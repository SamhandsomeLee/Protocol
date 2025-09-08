#include "protocol_adapter.h"
#include "protocol/transport/serial_transport.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

// 协议常量定义
const QString ProtocolAdapter::PROTOCOL_VERSION = "2.1.0";
const int ProtocolAdapter::SERIAL_TIMEOUT_MS = 3000;
const int ProtocolAdapter::MAX_RETRY_COUNT = 3;

ProtocolAdapter::ProtocolAdapter(QObject *parent)
    : QObject(parent)
    , transport_(nullptr)
    , protocolVersion_(PROTOCOL_VERSION)
    , transportOwned_(false)
{
    // 初始化默认参数映射
    initializeDefaultMappings();
}

ProtocolAdapter::ProtocolAdapter(ITransport* transport, QObject *parent)
    : QObject(parent)
    , transport_(transport)
    , protocolVersion_(PROTOCOL_VERSION)
    , transportOwned_(false)
{
    // 初始化默认参数映射
    initializeDefaultMappings();

    // 连接传输层信号
    if (transport_) {
        connectTransportSignals();
    }
}

ProtocolAdapter::~ProtocolAdapter()
{
    if (transport_) {
        disconnectTransportSignals();

        // 如果拥有传输层对象的所有权，则删除它
        if (transportOwned_) {
            transport_->deleteLater();
        }
    }
}

bool ProtocolAdapter::sendParameterUpdate(const QString& parameterPath, const QVariant& value)
{
    if (!isConnected()) {
        emit communicationError("Transport not connected");
        return false;
    }

    if (!isParameterSupported(parameterPath)) {
        emit communicationError(QString("Parameter not supported: %1").arg(parameterPath));
        return false;
    }

    // 创建参数映射
    QVariantMap parameters;
    parameters[parameterPath] = value;

    // 序列化并发送
    QByteArray data = serializeParameters(parameters);
    if (data.isEmpty()) {
        emit communicationError(QString("Failed to serialize parameter: %1").arg(parameterPath));
        return false;
    }

    bool success = sendRawData(data);
    if (success) {
        emit parameterAcknowledged(parameterPath);
    }

    return success;
}

bool ProtocolAdapter::sendParameterGroup(const QStringList& paths, const QVariantMap& values)
{
    if (!isConnected()) {
        emit communicationError("Transport not connected");
        return false;
    }

    // 检查所有参数是否支持
    for (const QString& path : paths) {
        if (!isParameterSupported(path)) {
            emit communicationError(QString("Parameter not supported: %1").arg(path));
            return false;
        }
    }

    // 创建参数映射
    QVariantMap parameters;
    for (const QString& path : paths) {
        if (values.contains(path)) {
            parameters[path] = values[path];
        }
    }

    // 序列化并发送
    QByteArray data = serializeParameters(parameters);
    if (data.isEmpty()) {
        emit communicationError("Failed to serialize parameter group");
        return false;
    }

    bool success = sendRawData(data);
    if (success) {
        for (const QString& path : paths) {
            emit parameterAcknowledged(path);
        }
    }

    return success;
}

QByteArray ProtocolAdapter::serializeParameters(const QVariantMap& parameters)
{
    if (parameters.isEmpty()) {
        return QByteArray();
    }

    // 根据参数类型确定消息类型
    QString firstPath = parameters.firstKey();
    MessageType msgType = getMessageTypeFromPath(firstPath);

    return serializeMessage(msgType, parameters);
}

bool ProtocolAdapter::deserializeParameters(const QByteArray& data, QVariantMap& parameters)
{
    if (data.isEmpty()) {
        return false;
    }

    // 尝试解析为不同的消息类型
    // 这里简化处理，实际应该根据消息头确定类型
    QList<MessageType> typesToTry = {
        MessageType::ANC_OFF,
        MessageType::ENC_OFF,
        MessageType::RNC_OFF,
        MessageType::CHECK_MODE,
        MessageType::ALPHA,
        MessageType::SET1,
        MessageType::CALIBRATION_AMP,
        MessageType::CALIBRATION_OTHER
    };

    for (MessageType type : typesToTry) {
        QVariantMap tempParams;
        if (deserializeMessage(type, data, tempParams)) {
            parameters = tempParams;
            return true;
        }
    }

    return false;
}

QString ProtocolAdapter::getProtocolVersion() const
{
    return protocolVersion_;
}

bool ProtocolAdapter::isParameterSupported(const QString& parameterPath) const
{
    return pathMapping_.contains(parameterPath);
}

QStringList ProtocolAdapter::getSupportedParameters() const
{
    return pathMapping_.keys();
}

void ProtocolAdapter::setTransport(ITransport* transport)
{
    if (transport_) {
        disconnectTransportSignals();

        // 如果拥有传输层对象的所有权，则删除它
        if (transportOwned_) {
            transport_->deleteLater();
        }
    }

    transport_ = transport;
    transportOwned_ = false;  // 外部注入的对象，不拥有所有权

    if (transport_) {
        connectTransportSignals();
    }
}

ITransport* ProtocolAdapter::transport() const
{
    return transport_;
}

bool ProtocolAdapter::isConnected() const
{
    return transport_ && transport_->isOpen();
}

QString ProtocolAdapter::transportDescription() const
{
    return transport_ ? transport_->description() : "No transport set";
}

bool ProtocolAdapter::loadProtocolMapping(const QString& mappingFile)
{
    QFile file(mappingFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open mapping file:" << mappingFile;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    QJsonObject mappings = root["mappings"].toObject();

    // 清除现有映射
    pathMapping_.clear();

    // 加载新映射
    for (auto it = mappings.begin(); it != mappings.end(); ++it) {
        QString logicalPath = it.key();
        QJsonObject mapping = it.value().toObject();

        ParameterMapping paramMapping;
        paramMapping.logicalPath = logicalPath;
        paramMapping.protobufPath = mapping["protobufPath"].toString();
        paramMapping.fieldType = mapping["fieldType"].toString();
        paramMapping.defaultValue = mapping["defaultValue"].toVariant();
        paramMapping.deprecated = mapping["deprecated"].toBool(false);
        paramMapping.replacedBy = mapping["replacedBy"].toString();

        // 确定消息类型
        QString msgTypeStr = mapping["messageType"].toString();
        paramMapping.messageType = MessageType::ANC_OFF; // 默认值

        if (msgTypeStr == "ANC_OFF") paramMapping.messageType = MessageType::ANC_OFF;
        else if (msgTypeStr == "ENC_OFF") paramMapping.messageType = MessageType::ENC_OFF;
        else if (msgTypeStr == "RNC_OFF") paramMapping.messageType = MessageType::RNC_OFF;
        else if (msgTypeStr == "CHECK_MODE") paramMapping.messageType = MessageType::CHECK_MODE;
        else if (msgTypeStr == "ALPHA") paramMapping.messageType = MessageType::ALPHA;
        else if (msgTypeStr == "SET1") paramMapping.messageType = MessageType::SET1;
        else if (msgTypeStr == "CALIBRATION_AMP") paramMapping.messageType = MessageType::CALIBRATION_AMP;
        else if (msgTypeStr == "CALIBRATION_OTHER") paramMapping.messageType = MessageType::CALIBRATION_OTHER;

        pathMapping_[logicalPath] = paramMapping;
    }

    qDebug() << "Loaded" << pathMapping_.size() << "parameter mappings";
    return true;
}

QString ProtocolAdapter::getProtobufPath(const QString& parameterPath) const
{
    if (pathMapping_.contains(parameterPath)) {
        return pathMapping_[parameterPath].protobufPath;
    }
    return QString();
}

void ProtocolAdapter::handleTransportDataReceived(const QByteArray& data)
{
    receiveBuffer_.append(data);

    // 处理完整的消息包
    processReceivedData(receiveBuffer_);
    receiveBuffer_.clear();

    // 转发数据接收信号
    emit dataReceived(data);
}

void ProtocolAdapter::handleTransportError(const QString& error)
{
    qWarning() << "Transport error:" << error;
    emit communicationError(error);
}

void ProtocolAdapter::handleTransportConnectionChanged(bool connected)
{
    qDebug() << "Transport connection status changed:" << connected;
    emit connectionStatusChanged(connected);
}

void ProtocolAdapter::initializeDefaultMappings()
{
    // 初始化默认的参数映射
    // ANC 控制参数
    ParameterMapping ancOffMapping;
    ancOffMapping.logicalPath = "anc.enabled";
    ancOffMapping.protobufPath = "value";
    ancOffMapping.fieldType = "bool";
    ancOffMapping.defaultValue = false;
    ancOffMapping.messageType = MessageType::ANC_OFF;
    ancOffMapping.deprecated = false;
    pathMapping_["anc.enabled"] = ancOffMapping;

    // ENC 控制参数
    ParameterMapping encOffMapping;
    encOffMapping.logicalPath = "enc.enabled";
    encOffMapping.protobufPath = "value";
    encOffMapping.fieldType = "bool";
    encOffMapping.defaultValue = false;
    encOffMapping.messageType = MessageType::ENC_OFF;
    encOffMapping.deprecated = false;
    pathMapping_["enc.enabled"] = encOffMapping;

    // RNC 控制参数
    ParameterMapping rncOffMapping;
    rncOffMapping.logicalPath = "rnc.enabled";
    rncOffMapping.protobufPath = "value";
    rncOffMapping.fieldType = "bool";
    rncOffMapping.defaultValue = false;
    rncOffMapping.messageType = MessageType::RNC_OFF;
    rncOffMapping.deprecated = false;
    pathMapping_["rnc.enabled"] = rncOffMapping;

    // 检测模式参数
    ParameterMapping checkModeMapping;
    checkModeMapping.logicalPath = "system.check_mode";
    checkModeMapping.protobufPath = "value";
    checkModeMapping.fieldType = "bool";
    checkModeMapping.defaultValue = false;
    checkModeMapping.messageType = MessageType::CHECK_MODE;
    checkModeMapping.deprecated = false;
    pathMapping_["system.check_mode"] = checkModeMapping;

    // Alpha 参数组
    QStringList alphaParams = {"alpha1", "alpha2", "alpha3", "alpha4", "alpha5",
                              "alpha1_10", "alpha2_10", "alpha3_10", "alpha4_10", "alpha5_10"};

    for (const QString& param : alphaParams) {
        ParameterMapping alphaMapping;
        alphaMapping.logicalPath = QString("tuning.alpha.%1").arg(param);
        alphaMapping.protobufPath = param;
        alphaMapping.fieldType = "uint32";
        alphaMapping.defaultValue = 0;
        alphaMapping.messageType = MessageType::ALPHA;
        alphaMapping.deprecated = false;
        pathMapping_[alphaMapping.logicalPath] = alphaMapping;
    }

    // Set1 参数组
    QStringList set1Params = {"gamma", "eta", "delta", "refer_num", "spk_num", "output_amplitude"};

    for (const QString& param : set1Params) {
        ParameterMapping set1Mapping;
        set1Mapping.logicalPath = QString("tuning.set1.%1").arg(param);
        set1Mapping.protobufPath = param;
        set1Mapping.fieldType = "uint32";
        set1Mapping.defaultValue = 0;
        set1Mapping.messageType = MessageType::SET1;
        set1Mapping.deprecated = false;
        pathMapping_[set1Mapping.logicalPath] = set1Mapping;
    }

    qDebug() << "Initialized" << pathMapping_.size() << "default parameter mappings";
}

ProtocolAdapter::MessageType ProtocolAdapter::getMessageTypeFromPath(const QString& parameterPath) const
{
    if (pathMapping_.contains(parameterPath)) {
        return pathMapping_[parameterPath].messageType;
    }
    return MessageType::ANC_OFF; // 默认类型
}

QByteArray ProtocolAdapter::serializeMessage(MessageType type, const QVariantMap& parameters)
{
    switch (type) {
    case MessageType::ANC_OFF:
        return serializeAncOff(parameters.value("anc.enabled", false).toBool());
    case MessageType::ENC_OFF:
        return serializeEncOff(parameters.value("enc.enabled", false).toBool());
    case MessageType::RNC_OFF:
        return serializeRncOff(parameters.value("rnc.enabled", false).toBool());
    case MessageType::CHECK_MODE:
        return serializeCheckMode(parameters.value("system.check_mode", false).toBool());
    case MessageType::ALPHA:
        return serializeAlpha(parameters);
    case MessageType::SET1:
        return serializeSet1(parameters);
    case MessageType::CALIBRATION_AMP:
        return serializeCalibrationAmp(parameters);
    case MessageType::CALIBRATION_OTHER:
        return serializeCalibrationOther(parameters);
    default:
        return QByteArray();
    }
}

bool ProtocolAdapter::deserializeMessage(MessageType type, const QByteArray& data, QVariantMap& parameters)
{
    switch (type) {
    case MessageType::ANC_OFF:
        return deserializeAncOff(data, parameters);
    case MessageType::ENC_OFF:
        return deserializeEncOff(data, parameters);
    case MessageType::RNC_OFF:
        return deserializeRncOff(data, parameters);
    case MessageType::CHECK_MODE:
        return deserializeCheckMode(data, parameters);
    case MessageType::ALPHA:
        return deserializeAlpha(data, parameters);
    case MessageType::SET1:
        return deserializeSet1(data, parameters);
    case MessageType::CALIBRATION_AMP:
        return deserializeCalibrationAmp(data, parameters);
    case MessageType::CALIBRATION_OTHER:
        return deserializeCalibrationOther(data, parameters);
    default:
        return false;
    }
}

bool ProtocolAdapter::sendRawData(const QByteArray& data)
{
    if (!transport_ || !transport_->isOpen()) {
        emit communicationError("Transport not connected");
        return false;
    }

    bool success = transport_->send(data);
    if (!success) {
        emit communicationError("Failed to send data through transport");
        return false;
    }

    qDebug() << "Raw data sent:" << data.size() << "bytes";
    return true;
}

void ProtocolAdapter::processReceivedData(const QByteArray& data)
{
    emit dataReceived(data);

    // 尝试反序列化接收到的数据
    QVariantMap parameters;
    if (deserializeParameters(data, parameters)) {
        // 发出参数变更信号
        for (auto it = parameters.begin(); it != parameters.end(); ++it) {
            emit parameterAcknowledged(it.key());
        }
    }
}

// 具体消息序列化实现
QByteArray ProtocolAdapter::serializeAncOff(bool value)
{
    MSG_AncOff message = MSG_AncOff_init_zero;
    message.value = value;

    uint8_t buffer[MSG_AncOff_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool success = pb_encode(&stream, MSG_AncOff_fields, &message);
    if (!success) {
        qWarning() << "Failed to encode MSG_AncOff:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(buffer), stream.bytes_written);
}

QByteArray ProtocolAdapter::serializeEncOff(bool value)
{
    MSG_EncOff message = MSG_EncOff_init_zero;
    message.value = value;

    uint8_t buffer[MSG_EncOff_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool success = pb_encode(&stream, MSG_EncOff_fields, &message);
    if (!success) {
        qWarning() << "Failed to encode MSG_EncOff:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(buffer), stream.bytes_written);
}

QByteArray ProtocolAdapter::serializeRncOff(bool value)
{
    MSG_RncOff message = MSG_RncOff_init_zero;
    message.value = value;

    uint8_t buffer[MSG_RncOff_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool success = pb_encode(&stream, MSG_RncOff_fields, &message);
    if (!success) {
        qWarning() << "Failed to encode MSG_RncOff:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(buffer), stream.bytes_written);
}

QByteArray ProtocolAdapter::serializeCheckMode(bool value)
{
    MSG_CheckMod message = MSG_CheckMod_init_zero;
    message.value = value;

    uint8_t buffer[MSG_CheckMod_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool success = pb_encode(&stream, MSG_CheckMod_fields, &message);
    if (!success) {
        qWarning() << "Failed to encode MSG_CheckMod:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(buffer), stream.bytes_written);
}

QByteArray ProtocolAdapter::serializeAlpha(const QVariantMap& parameters)
{
    MSG_Alpha message = MSG_Alpha_init_zero;

    message.alpha1 = parameters.value("tuning.alpha.alpha1", 0).toUInt();
    message.alpha2 = parameters.value("tuning.alpha.alpha2", 0).toUInt();
    message.alpha3 = parameters.value("tuning.alpha.alpha3", 0).toUInt();
    message.alpha4 = parameters.value("tuning.alpha.alpha4", 0).toUInt();
    message.alpha5 = parameters.value("tuning.alpha.alpha5", 0).toUInt();
    message.alpha1_10 = parameters.value("tuning.alpha.alpha1_10", 0).toUInt();
    message.alpha2_10 = parameters.value("tuning.alpha.alpha2_10", 0).toUInt();
    message.alpha3_10 = parameters.value("tuning.alpha.alpha3_10", 0).toUInt();
    message.alpha4_10 = parameters.value("tuning.alpha.alpha4_10", 0).toUInt();
    message.alpha5_10 = parameters.value("tuning.alpha.alpha5_10", 0).toUInt();

    uint8_t buffer[MSG_Alpha_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool success = pb_encode(&stream, MSG_Alpha_fields, &message);
    if (!success) {
        qWarning() << "Failed to encode MSG_Alpha:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(buffer), stream.bytes_written);
}

QByteArray ProtocolAdapter::serializeSet1(const QVariantMap& parameters)
{
    MSG_Set1 message = MSG_Set1_init_zero;

    message.gamma = parameters.value("tuning.set1.gamma", 0).toUInt();
    message.eta = parameters.value("tuning.set1.eta", 0).toUInt();
    message.delta = parameters.value("tuning.set1.delta", 0).toUInt();
    message.refer_num = parameters.value("tuning.set1.refer_num", 0).toUInt();
    message.spk_num = parameters.value("tuning.set1.spk_num", 0).toUInt();
    message.output_amplitude = parameters.value("tuning.set1.output_amplitude", 0).toUInt();

    uint8_t buffer[MSG_Set1_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool success = pb_encode(&stream, MSG_Set1_fields, &message);
    if (!success) {
        qWarning() << "Failed to encode MSG_Set1:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(buffer), stream.bytes_written);
}

QByteArray ProtocolAdapter::serializeCalibrationAmp(const QVariantMap& parameters)
{
    MSG_CalibrationAmp message = MSG_CalibrationAmp_init_zero;

    // 这里简化处理，实际需要根据具体的数组参数来填充
    // 示例代码，需要根据实际参数结构调整

    uint8_t buffer[MSG_CalibrationAmp_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool success = pb_encode(&stream, MSG_CalibrationAmp_fields, &message);
    if (!success) {
        qWarning() << "Failed to encode MSG_CalibrationAmp:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(buffer), stream.bytes_written);
}

QByteArray ProtocolAdapter::serializeCalibrationOther(const QVariantMap& parameters)
{
    MSG_CalibrationOther message = MSG_CalibrationOther_init_zero;

    // 这里简化处理，实际需要根据具体的参数来填充

    uint8_t buffer[MSG_CalibrationOther_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool success = pb_encode(&stream, MSG_CalibrationOther_fields, &message);
    if (!success) {
        qWarning() << "Failed to encode MSG_CalibrationOther:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(buffer), stream.bytes_written);
}

// 具体消息反序列化实现
bool ProtocolAdapter::deserializeAncOff(const QByteArray& data, QVariantMap& parameters)
{
    MSG_AncOff message = MSG_AncOff_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size());

    bool success = pb_decode(&stream, MSG_AncOff_fields, &message);
    if (!success) {
        qWarning() << "Failed to decode MSG_AncOff:" << PB_GET_ERROR(&stream);
        return false;
    }

    parameters["anc.enabled"] = message.value;
    return true;
}

bool ProtocolAdapter::deserializeEncOff(const QByteArray& data, QVariantMap& parameters)
{
    MSG_EncOff message = MSG_EncOff_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size());

    bool success = pb_decode(&stream, MSG_EncOff_fields, &message);
    if (!success) {
        qWarning() << "Failed to decode MSG_EncOff:" << PB_GET_ERROR(&stream);
        return false;
    }

    parameters["enc.enabled"] = message.value;
    return true;
}

bool ProtocolAdapter::deserializeRncOff(const QByteArray& data, QVariantMap& parameters)
{
    MSG_RncOff message = MSG_RncOff_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size());

    bool success = pb_decode(&stream, MSG_RncOff_fields, &message);
    if (!success) {
        qWarning() << "Failed to decode MSG_RncOff:" << PB_GET_ERROR(&stream);
        return false;
    }

    parameters["rnc.enabled"] = message.value;
    return true;
}

bool ProtocolAdapter::deserializeCheckMode(const QByteArray& data, QVariantMap& parameters)
{
    MSG_CheckMod message = MSG_CheckMod_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size());

    bool success = pb_decode(&stream, MSG_CheckMod_fields, &message);
    if (!success) {
        qWarning() << "Failed to decode MSG_CheckMod:" << PB_GET_ERROR(&stream);
        return false;
    }

    parameters["system.check_mode"] = message.value;
    return true;
}

bool ProtocolAdapter::deserializeAlpha(const QByteArray& data, QVariantMap& parameters)
{
    MSG_Alpha message = MSG_Alpha_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size());

    bool success = pb_decode(&stream, MSG_Alpha_fields, &message);
    if (!success) {
        qWarning() << "Failed to decode MSG_Alpha:" << PB_GET_ERROR(&stream);
        return false;
    }

    parameters["tuning.alpha.alpha1"] = message.alpha1;
    parameters["tuning.alpha.alpha2"] = message.alpha2;
    parameters["tuning.alpha.alpha3"] = message.alpha3;
    parameters["tuning.alpha.alpha4"] = message.alpha4;
    parameters["tuning.alpha.alpha5"] = message.alpha5;
    parameters["tuning.alpha.alpha1_10"] = message.alpha1_10;
    parameters["tuning.alpha.alpha2_10"] = message.alpha2_10;
    parameters["tuning.alpha.alpha3_10"] = message.alpha3_10;
    parameters["tuning.alpha.alpha4_10"] = message.alpha4_10;
    parameters["tuning.alpha.alpha5_10"] = message.alpha5_10;

    return true;
}

bool ProtocolAdapter::deserializeSet1(const QByteArray& data, QVariantMap& parameters)
{
    MSG_Set1 message = MSG_Set1_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size());

    bool success = pb_decode(&stream, MSG_Set1_fields, &message);
    if (!success) {
        qWarning() << "Failed to decode MSG_Set1:" << PB_GET_ERROR(&stream);
        return false;
    }

    parameters["tuning.set1.gamma"] = message.gamma;
    parameters["tuning.set1.eta"] = message.eta;
    parameters["tuning.set1.delta"] = message.delta;
    parameters["tuning.set1.refer_num"] = message.refer_num;
    parameters["tuning.set1.spk_num"] = message.spk_num;
    parameters["tuning.set1.output_amplitude"] = message.output_amplitude;

    return true;
}

bool ProtocolAdapter::deserializeCalibrationAmp(const QByteArray& data, QVariantMap& parameters)
{
    MSG_CalibrationAmp message = MSG_CalibrationAmp_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size());

    bool success = pb_decode(&stream, MSG_CalibrationAmp_fields, &message);
    if (!success) {
        qWarning() << "Failed to decode MSG_CalibrationAmp:" << PB_GET_ERROR(&stream);
        return false;
    }

    // 这里简化处理，实际需要根据数组结构来解析参数
    return true;
}

bool ProtocolAdapter::deserializeCalibrationOther(const QByteArray& data, QVariantMap& parameters)
{
    MSG_CalibrationOther message = MSG_CalibrationOther_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size());

    bool success = pb_decode(&stream, MSG_CalibrationOther_fields, &message);
    if (!success) {
        qWarning() << "Failed to decode MSG_CalibrationOther:" << PB_GET_ERROR(&stream);
        return false;
    }

    // 这里简化处理，实际需要根据具体结构来解析参数
    return true;
}

void ProtocolAdapter::connectTransportSignals()
{
    if (!transport_) {
        return;
    }

    // 连接传输层信号到协议适配器的槽函数
    connect(transport_, &ITransport::dataReceived,
            this, &ProtocolAdapter::handleTransportDataReceived);
    connect(transport_, &ITransport::transportError,
            this, &ProtocolAdapter::handleTransportError);
    connect(transport_, &ITransport::connectionStatusChanged,
            this, &ProtocolAdapter::handleTransportConnectionChanged);

    qDebug() << "Transport signals connected";
}

void ProtocolAdapter::disconnectTransportSignals()
{
    if (!transport_) {
        return;
    }

    // 断开传输层信号连接
    disconnect(transport_, nullptr, this, nullptr);

    qDebug() << "Transport signals disconnected";
}
