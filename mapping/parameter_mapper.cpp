#include "parameter_mapper.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

namespace Protocol {

ParameterMapper::ParameterMapper(QObject* parent)
    : QObject(parent)
{
    initializeDefaultMappings();
}

bool ParameterMapper::loadMapping(const QString& configFile) {
    QFile file(configFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QString error = QString("Cannot open mapping file: %1").arg(configFile);
        qWarning() << error;
        emit mappingLoaded(false, error);
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QString error = QString("JSON parse error: %1").arg(parseError.errorString());
        qWarning() << error;
        emit mappingLoaded(false, error);
        return false;
    }

    if (!doc.isObject()) {
        QString error = "Root JSON element must be an object";
        qWarning() << error;
        emit mappingLoaded(false, error);
        return false;
    }

    bool result = loadMappingFromJson(doc.object());
    emit mappingLoaded(result, result ? QString() : "Failed to load mapping from JSON");
    return result;
}

bool ParameterMapper::loadMappingFromJson(const QJsonObject& jsonObject) {
    QJsonArray parameters = jsonObject["parameters"].toArray();

    if (parameters.isEmpty()) {
        qWarning() << "No parameters found in mapping configuration";
        return false;
    }

    // 清除现有映射（保留默认映射）
    auto defaultMappings = mappings_;
    clear();

    // 重新添加默认映射
    mappings_ = defaultMappings;

    // 加载新映射
    for (const QJsonValue& value : parameters) {
        if (!value.isObject()) {
            qWarning() << "Invalid parameter entry (not an object)";
            continue;
        }

        ParameterInfo info = parseParameterFromJson(value.toObject());
        if (validateParameterInfo(info)) {
            mappings_[info.logicalPath] = info;
            qDebug() << "Loaded parameter mapping:" << info.logicalPath << "->" << info.protobufPath;
        } else {
            qWarning() << "Invalid parameter info for:" << info.logicalPath;
        }
    }

    initialized_ = true;
    qInfo() << "Parameter mapping loaded successfully:" << mappings_.size() << "parameters";
    return true;
}

ParameterInfo ParameterMapper::getParameterInfo(const QString& parameterPath) const {
    auto it = mappings_.find(parameterPath);
    if (it != mappings_.end()) {
        // 检查是否为弃用参数
        if (it->deprecated) {
            emit const_cast<ParameterMapper*>(this)->deprecatedParameterUsed(parameterPath, it->replacedBy);
        }
        return *it;
    }

    return ParameterInfo(); // 返回无效的参数信息
}

bool ParameterMapper::isParameterSupported(const QString& parameterPath) const {
    return mappings_.contains(parameterPath);
}

QStringList ParameterMapper::getSupportedParameters() const {
    return mappings_.keys();
}

QStringList ParameterMapper::getParametersForMessageType(MessageType messageType) const {
    QStringList result;
    for (auto it = mappings_.begin(); it != mappings_.end(); ++it) {
        if (it->messageType == messageType) {
            result << it.key();
        }
    }
    return result;
}

bool ParameterMapper::isParameterDeprecated(const QString& parameterPath) const {
    auto info = getParameterInfo(parameterPath);
    return info.isValid() && info.deprecated;
}

QString ParameterMapper::getReplacementParameter(const QString& parameterPath) const {
    auto info = getParameterInfo(parameterPath);
    return info.isValid() && info.deprecated ? info.replacedBy : QString();
}

void ParameterMapper::clear() {
    mappings_.clear();
    initialized_ = false;
}

int ParameterMapper::mappingCount() const {
    return mappings_.size();
}

void ParameterMapper::initializeDefaultMappings() {
    // ANC 相关参数
    ParameterInfo ancEnabledInfo;
    ancEnabledInfo.logicalPath = "anc.enabled";
    ancEnabledInfo.protobufPath = "value";
    ancEnabledInfo.fieldType = "bool";
    ancEnabledInfo.defaultValue = false;
    ancEnabledInfo.messageType = MessageType::ANC_OFF;
    ancEnabledInfo.description = "ANC enable/disable control";
    mappings_["anc.enabled"] = ancEnabledInfo;

    // ENC 相关参数
    ParameterInfo encEnabledInfo;
    encEnabledInfo.logicalPath = "enc.enabled";
    encEnabledInfo.protobufPath = "value";
    encEnabledInfo.fieldType = "bool";
    encEnabledInfo.defaultValue = false;
    encEnabledInfo.messageType = MessageType::ENC_OFF;
    encEnabledInfo.description = "ENC enable/disable control";
    mappings_["enc.enabled"] = encEnabledInfo;

    // RNC 相关参数
    ParameterInfo rncEnabledInfo;
    rncEnabledInfo.logicalPath = "rnc.enabled";
    rncEnabledInfo.protobufPath = "value";
    rncEnabledInfo.fieldType = "bool";
    rncEnabledInfo.defaultValue = false;
    rncEnabledInfo.messageType = MessageType::RNC_OFF;
    rncEnabledInfo.description = "RNC enable/disable control";
    mappings_["rnc.enabled"] = rncEnabledInfo;

    // 检查模式
    ParameterInfo checkModeInfo;
    checkModeInfo.logicalPath = "system.check_mode";
    checkModeInfo.protobufPath = "value";
    checkModeInfo.fieldType = "bool";
    checkModeInfo.defaultValue = false;
    checkModeInfo.messageType = MessageType::CHECK_MODE;
    checkModeInfo.description = "System check mode control";
    mappings_["system.check_mode"] = checkModeInfo;

    // Alpha 参数
    ParameterInfo alphaInfo;
    alphaInfo.logicalPath = "processing.alpha";
    alphaInfo.protobufPath = "alpha_value";
    alphaInfo.fieldType = "float";
    alphaInfo.defaultValue = 0.5f;
    alphaInfo.messageType = MessageType::ALPHA;
    alphaInfo.description = "Processing alpha parameter";
    mappings_["processing.alpha"] = alphaInfo;

    initialized_ = true;
    qDebug() << "Default parameter mappings initialized:" << mappings_.size() << "parameters";
}

bool ParameterMapper::validateParameterInfo(const ParameterInfo& info) const {
    if (!info.isValid()) {
        return false;
    }

    // 检查字段类型是否有效
    QStringList validTypes = {"bool", "int32", "uint32", "float", "double", "string", "bytes"};
    if (!validTypes.contains(info.fieldType)) {
        qWarning() << "Invalid field type:" << info.fieldType << "for parameter:" << info.logicalPath;
        return false;
    }

    // 检查弃用参数的替换参数是否有效
    if (info.deprecated && info.replacedBy.isEmpty()) {
        qWarning() << "Deprecated parameter must have replacement:" << info.logicalPath;
        return false;
    }

    return true;
}

ParameterInfo ParameterMapper::parseParameterFromJson(const QJsonObject& jsonParam) const {
    ParameterInfo info;

    info.logicalPath = jsonParam["logical_path"].toString();
    info.protobufPath = jsonParam["protobuf_path"].toString();
    info.fieldType = jsonParam["field_type"].toString();
    info.defaultValue = jsonParam["default_value"].toVariant();
    info.deprecated = jsonParam["deprecated"].toBool(false);
    info.replacedBy = jsonParam["replaced_by"].toString();
    info.description = jsonParam["description"].toString();

    // 解析消息类型
    QString messageTypeStr = jsonParam["message_type"].toString();
    info.messageType = MessageTypeUtils::fromString(messageTypeStr);

    return info;
}

} // namespace Protocol
