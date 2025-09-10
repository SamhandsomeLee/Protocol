#include "protocol_adapter_refactored.h"
#include <QDebug>

namespace Protocol {

const QString ProtocolAdapterRefactored::PROTOCOL_VERSION = "1.0.0";
const int ProtocolAdapterRefactored::DEFAULT_TIMEOUT_MS = 5000;

ProtocolAdapterRefactored::ProtocolAdapterRefactored(QObject *parent)
    : QObject(parent)
{
    initializeComponents();
    qDebug() << "ProtocolAdapterRefactored created without transport";
}

ProtocolAdapterRefactored::ProtocolAdapterRefactored(ITransport* transport, QObject *parent)
    : QObject(parent)
{
    initializeComponents();
    setTransport(transport);
    qDebug() << "ProtocolAdapterRefactored created with transport:" << transportDescription();
}

ProtocolAdapterRefactored::~ProtocolAdapterRefactored() {
    disconnectComponentSignals();
    qDebug() << "ProtocolAdapterRefactored destroyed";
}

bool ProtocolAdapterRefactored::sendParameterUpdate(const QString& parameterPath, const QVariant& value) {
    qDebug() << "=== Starting parameter update ===";
    qDebug() << "Input parameter:" << parameterPath << "=" << value;

    // 检查初始化状态
    qDebug() << "Checking initialization status...";
    if (!initialized_) {
        qDebug() << "✗ EARLY RETURN: ProtocolAdapter not initialized";
        qWarning() << "ProtocolAdapter not initialized";
        emit communicationError("ProtocolAdapter not initialized");
        return false;
    }
    qDebug() << "✓ ProtocolAdapter is initialized";

    // 检查连接状态
    qDebug() << "Checking connection status...";
    if (!isConnected()) {
        qDebug() << "✗ EARLY RETURN: Not connected, cannot send parameter update";
        qWarning() << "Not connected, cannot send parameter update";
        emit communicationError("Not connected");
        return false;
    }
    qDebug() << "✓ Connection is active";

    // 检查参数是否支持
    qDebug() << "Checking parameter support for:" << parameterPath;
    if (!parameterMapper_->isParameterSupported(parameterPath)) {
        qDebug() << "✗ EARLY RETURN: Parameter not supported:" << parameterPath;
        qDebug() << "Available parameters:" << parameterMapper_->getSupportedParameters();
        QString error = QString("Unsupported parameter: %1").arg(parameterPath);
        qWarning() << error;
        emit communicationError(error);
        return false;
    }
    qDebug() << "✓ Parameter is supported";

    // 获取参数信息
    qDebug() << "Getting parameter information for:" << parameterPath;
    auto paramInfo = parameterMapper_->getParameterInfo(parameterPath);
    if (!paramInfo.isValid()) {
        qDebug() << "✗ EARLY RETURN: Invalid parameter info for:" << parameterPath;
        qDebug() << "Parameter info details: messageType=" << static_cast<int>(paramInfo.messageType)
                 << "protobufPath=" << paramInfo.protobufPath;
        QString error = QString("Invalid parameter info for: %1").arg(parameterPath);
        qWarning() << error;
        emit communicationError(error);
        return false;
    }
    qDebug() << "✓ Parameter info valid - messageType:" << static_cast<int>(paramInfo.messageType)
             << "protobufPath:" << paramInfo.protobufPath;

    // 准备参数映射
    QVariantMap parameters;
    parameters[parameterPath] = value;

    // 序列化消息
    qDebug() << "=== Starting parameter serialization ===";
    qDebug() << "Parameter path:" << parameterPath;
    qDebug() << "Message type:" << static_cast<int>(paramInfo.messageType);
    qDebug() << "Parameter value:" << parameters.value(parameterPath)
             << QString("(type: %1)").arg(parameters.value(parameterPath).typeName());
    qDebug() << "Parameter map size:" << parameters.size();

    // 打印完整的参数映射
    qDebug() << "Complete parameter map:";
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        qDebug() << "  -" << it.key() << "=" << it.value()
                 << QString("(type: %1)").arg(it.value().typeName());
    }

    qDebug() << "Attempting serialization with message serializer...";
    QByteArray data = messageSerializer_->serialize(paramInfo.messageType, parameters);

    if (data.isEmpty()) {
        qDebug() << "✗ SERIALIZATION FAILED";
        qDebug() << "Failed parameter path:" << parameterPath;
        qDebug() << "Failed message type:" << static_cast<int>(paramInfo.messageType);
        qDebug() << "Parameter value that failed:" << parameters.value(parameterPath);
        QString error = QString("Failed to serialize parameter: %1").arg(parameterPath);
        qWarning() << error;
        emit communicationError(error);
        return false;
    }

    qDebug() << "✓ SERIALIZATION SUCCESSFUL";
    qDebug() << "Serialized data size:" << data.size() << "bytes";
    qDebug() << "Serialized data (hex):" << data.toHex(' ');
    qDebug() << "First 16 bytes:" << data.left(16).toHex(' ');

    // 发送数据
    qDebug() << "=== Starting data transmission ===";
    qDebug() << "Sending data via connection manager...";
    qDebug() << "Data to send size:" << data.size() << "bytes";
    bool success = connectionManager_->sendData(data);

    qDebug() << "=== Data transmission result ===";
    if (success) {
        qDebug() << "✓ TRANSMISSION SUCCESSFUL";
        qDebug() << "Successfully sent parameter:" << parameterPath;
        qDebug() << "Parameter value:" << value;
        qDebug() << "Message type:" << static_cast<int>(paramInfo.messageType);
        qDebug() << "Data size transmitted:" << data.size() << "bytes";
        qDebug() << "Emitting parameterAcknowledged signal for:" << parameterPath;
        qDebug() << "Parameter update sent successfully:" << parameterPath << "=" << value;
        emit parameterAcknowledged(parameterPath);
        qDebug() << "✓ Parameter acknowledgment signal emitted";
    } else {
        qDebug() << "✗ TRANSMISSION FAILED";
        qDebug() << "Failed to send parameter:" << parameterPath;
        qDebug() << "Parameter value:" << value;
        qDebug() << "Message type:" << static_cast<int>(paramInfo.messageType);
        qDebug() << "Data size that failed:" << data.size() << "bytes";
        qDebug() << "Failed data (hex):" << data.toHex(' ');
        QString error = QString("Failed to send parameter update: %1").arg(parameterPath);
        qWarning() << error;
        qDebug() << "Emitting communicationError signal with error:" << error;
        emit communicationError(error);
    }
    qDebug() << "=== End of parameter transmission ===";
    qDebug() << "";

    return success;
}

bool ProtocolAdapterRefactored::sendParameterGroup(const QStringList& paths, const QVariantMap& values) {
    if (!initialized_) {
        qWarning() << "ProtocolAdapter not initialized";
        emit communicationError("ProtocolAdapter not initialized");
        return false;
    }

    if (!isConnected()) {
        qWarning() << "Not connected, cannot send parameter group";
        emit communicationError("Not connected");
        return false;
    }

    if (paths.isEmpty()) {
        qWarning() << "Empty parameter path list";
        return false;
    }

    // 按消息类型分组参数
    QHash<MessageType, QVariantMap> messageGroups;

    for (const QString& path : paths) {
        if (!parameterMapper_->isParameterSupported(path)) {
            QString error = QString("Unsupported parameter in group: %1").arg(path);
            qWarning() << error;
            emit communicationError(error);
            return false;
        }

        auto paramInfo = parameterMapper_->getParameterInfo(path);
        if (!paramInfo.isValid()) {
            QString error = QString("Invalid parameter info for: %1").arg(path);
            qWarning() << error;
            emit communicationError(error);
            return false;
        }

        // 将参数添加到对应的消息组
        if (values.contains(path)) {
            messageGroups[paramInfo.messageType][path] = values[path];
        }
    }

    // 逐个发送不同类型的消息
    bool allSuccess = true;
    for (auto it = messageGroups.begin(); it != messageGroups.end(); ++it) {
        MessageType messageType = it.key();
        const QVariantMap& groupParams = it.value();

        QByteArray data = messageSerializer_->serialize(messageType, groupParams);
        if (data.isEmpty()) {
            QString error = QString("Failed to serialize message type: %1").arg(static_cast<int>(messageType));
            qWarning() << error;
            emit communicationError(error);
            allSuccess = false;
            continue;
        }

        bool success = connectionManager_->sendData(data);
        if (!success) {
            QString error = QString("Failed to send message type: %1").arg(static_cast<int>(messageType));
            qWarning() << error;
            emit communicationError(error);
            allSuccess = false;
        } else {
            // 发送成功，通知每个参数
            for (const QString& path : groupParams.keys()) {
                emit parameterAcknowledged(path);
            }
        }
    }

    if (allSuccess) {
        qDebug() << "Parameter group sent successfully:" << paths.size() << "parameters";
    } else {
        qWarning() << "Some parameters in group failed to send";
    }

    return allSuccess;
}

QByteArray ProtocolAdapterRefactored::serializeParameters(const QVariantMap& parameters) {
    if (!initialized_) {
        qWarning() << "ProtocolAdapter not initialized";
        return QByteArray();
    }

    if (parameters.isEmpty()) {
        qWarning() << "Empty parameters map";
        return QByteArray();
    }

    // 简化实现：假设所有参数属于同一消息类型
    // 实际使用中可能需要更复杂的逻辑
    QString firstPath = parameters.keys().first();
    auto paramInfo = parameterMapper_->getParameterInfo(firstPath);

    if (!paramInfo.isValid()) {
        qWarning() << "Invalid parameter info for:" << firstPath;
        return QByteArray();
    }

    return messageSerializer_->serialize(paramInfo.messageType, parameters);
}

bool ProtocolAdapterRefactored::deserializeParameters(const QByteArray& data, QVariantMap& parameters) {
    if (!initialized_) {
        qWarning() << "ProtocolAdapter not initialized";
        return false;
    }

    if (data.isEmpty()) {
        qWarning() << "Empty data for deserialization";
        return false;
    }

    // 简化实现：尝试各种消息类型进行反序列化
    // 实际实现中可能需要根据数据头部确定消息类型
    auto supportedTypes = messageSerializer_->getSupportedMessageTypes();

    for (MessageType messageType : supportedTypes) {
        QVariantMap tempParams;
        if (messageSerializer_->deserialize(messageType, data, tempParams)) {
            parameters = tempParams;
            qDebug() << "Successfully deserialized as message type:" << static_cast<int>(messageType);
            return true;
        }
    }

    qWarning() << "Failed to deserialize data with any supported message type";
    return false;
}

QString ProtocolAdapterRefactored::getProtocolVersion() const {
    return versionManager_ ? versionManager_->getCurrentVersion() : PROTOCOL_VERSION;
}

bool ProtocolAdapterRefactored::isParameterSupported(const QString& parameterPath) const {
    return parameterMapper_ ? parameterMapper_->isParameterSupported(parameterPath) : false;
}

QStringList ProtocolAdapterRefactored::getSupportedParameters() const {
    return parameterMapper_ ? parameterMapper_->getSupportedParameters() : QStringList();
}

void ProtocolAdapterRefactored::setTransport(ITransport* transport) {
    if (connectionManager_) {
        connectionManager_->setTransport(transport);
        qInfo() << "Transport set:" << transportDescription();
    } else {
        qWarning() << "ConnectionManager not initialized";
    }
}

ITransport* ProtocolAdapterRefactored::transport() const {
    return connectionManager_ ? connectionManager_->transport() : nullptr;
}

bool ProtocolAdapterRefactored::isConnected() const {
    return connectionManager_ ? connectionManager_->isConnected() : false;
}

QString ProtocolAdapterRefactored::transportDescription() const {
    return connectionManager_ ? connectionManager_->transportDescription() : "No connection manager";
}

bool ProtocolAdapterRefactored::loadProtocolMapping(const QString& mappingFile) {
    if (!parameterMapper_) {
        qWarning() << "ParameterMapper not initialized";
        return false;
    }

    return parameterMapper_->loadMapping(mappingFile);
}

QString ProtocolAdapterRefactored::getProtobufPath(const QString& parameterPath) const {
    if (!parameterMapper_) {
        return QString();
    }

    auto paramInfo = parameterMapper_->getParameterInfo(parameterPath);
    return paramInfo.isValid() ? paramInfo.protobufPath : QString();
}

void ProtocolAdapterRefactored::handleConnectionDataReceived(const QByteArray& data) {
    qDebug() << "Data received:" << data.size() << "bytes";

    // 验证协议版本（如果数据包含版本信息）
    if (!validateProtocolVersion(data)) {
        return; // 版本验证失败，已发出相应信号
    }

    // 处理协议数据
    processProtocolData(data);

    // 转发原始数据信号
    emit dataReceived(data);
}

void ProtocolAdapterRefactored::handleConnectionError(const QString& error) {
    qWarning() << "Connection error:" << error;
    emit communicationError(error);
}

void ProtocolAdapterRefactored::handleConnectionStatusChanged(bool connected) {
    qInfo() << "Connection status changed:" << connected;
    emit connectionStatusChanged(connected);
}

void ProtocolAdapterRefactored::handleVersionIncompatible(const QString& currentVersion,
                                                         const QString& remoteVersion,
                                                         const QString& reason) {
    qWarning() << "Version incompatible:" << reason;
    emit protocolVersionMismatch(currentVersion, remoteVersion);
}

void ProtocolAdapterRefactored::handleMappingLoaded(bool success, const QString& errorMessage) {
    qInfo() << "Parameter mapping loaded:" << success;
    if (!success) {
        qWarning() << "Mapping load error:" << errorMessage;
    }
    emit mappingLoaded(success, errorMessage);
}

void ProtocolAdapterRefactored::initializeComponents() {
    // 创建组件
    parameterMapper_ = std::make_unique<ParameterMapper>(this);
    messageSerializer_ = std::make_unique<MessageSerializer>(this);
    connectionManager_ = std::make_unique<ConnectionManager>(this);
    versionManager_ = std::make_unique<VersionManager>(this);

    // 设置版本管理器
    versionManager_->setCurrentVersion(PROTOCOL_VERSION);

    // 连接组件信号
    connectComponentSignals();

    initialized_ = true;
    qInfo() << "ProtocolAdapter components initialized successfully";
}

void ProtocolAdapterRefactored::connectComponentSignals() {
    if (!initialized_) {
        return;
    }

    // 连接ConnectionManager信号
    connect(connectionManager_.get(), &ConnectionManager::dataReceived,
            this, &ProtocolAdapterRefactored::handleConnectionDataReceived);
    connect(connectionManager_.get(), &ConnectionManager::communicationError,
            this, &ProtocolAdapterRefactored::handleConnectionError);
    connect(connectionManager_.get(), &ConnectionManager::connectionStatusChanged,
            this, &ProtocolAdapterRefactored::handleConnectionStatusChanged);

    // 连接VersionManager信号
    connect(versionManager_.get(), &VersionManager::versionIncompatible,
            this, &ProtocolAdapterRefactored::handleVersionIncompatible);

    // 连接ParameterMapper信号
    connect(parameterMapper_.get(), &ParameterMapper::mappingLoaded,
            this, &ProtocolAdapterRefactored::handleMappingLoaded);

    qDebug() << "Component signals connected";
}

void ProtocolAdapterRefactored::disconnectComponentSignals() {
    if (!initialized_) {
        return;
    }

    // 断开所有组件信号
    if (connectionManager_) {
        disconnect(connectionManager_.get(), nullptr, this, nullptr);
    }
    if (versionManager_) {
        disconnect(versionManager_.get(), nullptr, this, nullptr);
    }
    if (parameterMapper_) {
        disconnect(parameterMapper_.get(), nullptr, this, nullptr);
    }

    qDebug() << "Component signals disconnected";
}

MessageType ProtocolAdapterRefactored::getMessageTypeFromPath(const QString& parameterPath) const {
    if (!parameterMapper_) {
        return MessageType::ANC_SWITCH; // 默认类型
    }

    auto paramInfo = parameterMapper_->getParameterInfo(parameterPath);
    return paramInfo.isValid() ? paramInfo.messageType : MessageType::ANC_SWITCH;
}

void ProtocolAdapterRefactored::processProtocolData(const QByteArray& data) {
    // 尝试反序列化数据
    QVariantMap parameters;
    if (deserializeParameters(data, parameters)) {
        qDebug() << "Protocol data processed successfully, parameters:" << parameters.size();

        // 可以在这里添加特定的协议处理逻辑
        // 例如自动回复、状态更新等
    } else {
        qWarning() << "Failed to process protocol data";
    }
}

bool ProtocolAdapterRefactored::validateProtocolVersion(const QByteArray& data) {
    // 简化实现：假设数据包不包含版本信息
    // 实际实现中可能需要解析数据包头部的版本字段
    Q_UNUSED(data)

    // 这里可以添加版本验证逻辑
    // 例如：解析数据包中的版本字段，然后调用versionManager_->isCompatible()

    return true; // 暂时总是返回true
}

} // namespace Protocol
