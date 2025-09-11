#include "message_serializer.h"
#include "protocol_packager.h"
#include <QDebug>

namespace Protocol {

MessageSerializer::MessageSerializer(QObject* parent)
    : QObject(parent)
    , messageFactory_(std::make_shared<MessageFactory>())
    , protocolPackager_(std::make_unique<ProtocolPackager>())
{
    qDebug() << "MessageSerializer initialized";
}

MessageSerializer::~MessageSerializer() = default;

QByteArray MessageSerializer::serialize(MessageType messageType, const QVariantMap& parameters) {
    auto handler = messageFactory_->getHandler(messageType);
    if (!handler) {
        QString error = QString("No handler found for message type: %1").arg(static_cast<int>(messageType));
        qWarning() << error;
        emit serializationError(messageType, error);
        recordStatistics(messageType, "serialize", false, 0);
        return QByteArray();
    }

    // 验证参数
    if (!handler->validateParameters(parameters)) {
        QString error = QString("Parameter validation failed for message type: %1").arg(static_cast<int>(messageType));
        qWarning() << error;
        emit serializationError(messageType, error);
        recordStatistics(messageType, "serialize", false, 0);
        return QByteArray();
    }

    // 执行序列化
    QByteArray result = handler->serialize(parameters);
    bool success = !result.isEmpty();

    if (success) {
        qDebug() << "Message serialized successfully. Type:" << static_cast<int>(messageType)
                 << "Size:" << result.size() << "bytes";
        emit serializationCompleted(messageType, true, result.size());
    } else {
        QString error = QString("Serialization failed for message type: %1").arg(static_cast<int>(messageType));
        qWarning() << error;
        emit serializationError(messageType, error);
    }

    recordStatistics(messageType, "serialize", success, result.size());
    return result;
}

bool MessageSerializer::deserialize(MessageType messageType, const QByteArray& data, QVariantMap& parameters) {
    if (data.isEmpty()) {
        QString error = "Cannot deserialize empty data";
        qWarning() << error;
        emit serializationError(messageType, error);
        recordStatistics(messageType, "deserialize", false, 0);
        return false;
    }

    auto handler = messageFactory_->getHandler(messageType);
    if (!handler) {
        QString error = QString("No handler found for message type: %1").arg(static_cast<int>(messageType));
        qWarning() << error;
        emit serializationError(messageType, error);
        recordStatistics(messageType, "deserialize", false, data.size());
        return false;
    }

    // 执行反序列化
    parameters.clear(); // 清除之前的参数
    bool success = handler->deserialize(data, parameters);

    if (success) {
        qDebug() << "Message deserialized successfully. Type:" << static_cast<int>(messageType)
                 << "Parameters:" << parameters.size();
        emit deserializationCompleted(messageType, true, parameters.size());
    } else {
        QString error = QString("Deserialization failed for message type: %1").arg(static_cast<int>(messageType));
        qWarning() << error;
        emit serializationError(messageType, error);
    }

    recordStatistics(messageType, "deserialize", success, data.size());
    return success;
}

bool MessageSerializer::isMessageTypeSupported(MessageType messageType) const {
    return messageFactory_->isSupported(messageType);
}

QList<MessageType> MessageSerializer::getSupportedMessageTypes() const {
    return messageFactory_->getSupportedTypes();
}

bool MessageSerializer::validateParameters(MessageType messageType, const QVariantMap& parameters) const {
    auto handler = messageFactory_->getHandler(messageType);
    return handler ? handler->validateParameters(parameters) : false;
}

QString MessageSerializer::getMessageTypeDescription(MessageType messageType) const {
    return messageFactory_->getTypeDescription(messageType);
}

bool MessageSerializer::registerCustomHandler(MessageType messageType, std::shared_ptr<IMessageHandler> handler) {
    if (!handler) {
        qWarning() << "Cannot register null handler";
        return false;
    }

    bool result = messageFactory_->registerHandler(messageType, handler);
    if (result) {
        qInfo() << "Custom handler registered for message type:" << static_cast<int>(messageType);
    }

    return result;
}

QByteArray MessageSerializer::serialize(MessageType messageType, const QVariantMap& parameters, FunctionCode functionCode, bool useProtocolPackaging) {
    // 首先使用原有方法序列化具体消息
    QByteArray payloadData = serialize(messageType, parameters);

    if (payloadData.isEmpty()) {
        qWarning() << "Failed to serialize payload for message type:" << static_cast<int>(messageType);
        return QByteArray();
    }

    // 如果不使用协议封装，直接返回payload数据（向后兼容）
    if (!useProtocolPackaging) {
        return payloadData;
    }

    // 使用协议封装器包装成完整的MsgRequestResponse格式
    QByteArray result = protocolPackager_->packageMessage(messageType, functionCode, payloadData);

    if (result.isEmpty()) {
        QString error = QString("Protocol packaging failed for message type: %1").arg(static_cast<int>(messageType));
        qWarning() << error;
        emit serializationError(messageType, error);
        recordStatistics(messageType, "serialize", false, 0);
        return QByteArray();
    }

    qDebug() << "Message packaged successfully. Type:" << static_cast<int>(messageType)
             << "FunCode:" << static_cast<int>(functionCode)
             << "Total size:" << result.size() << "bytes";

    emit serializationCompleted(messageType, true, result.size());
    recordStatistics(messageType, "serialize", true, result.size());

    return result;
}

bool MessageSerializer::deserialize(const QByteArray& data, MessageType& messageType, FunctionCode& functionCode, QVariantMap& parameters) {
    if (data.isEmpty()) {
        QString error = "Cannot deserialize empty data";
        qWarning() << error;
        return false;
    }

    // 使用协议封装器解包MsgRequestResponse格式
    QByteArray payloadData;
    if (!protocolPackager_->unpackageMessage(data, messageType, functionCode, payloadData)) {
        QString error = "Failed to unpackage MsgRequestResponse format";
        qWarning() << error;
        emit serializationError(messageType, error);
        recordStatistics(messageType, "deserialize", false, data.size());
        return false;
    }

    // 使用原有方法反序列化具体消息
    bool success = deserialize(messageType, payloadData, parameters);

    if (success) {
        qDebug() << "Message unpackaged and deserialized successfully. Type:" << static_cast<int>(messageType)
                 << "FunCode:" << static_cast<int>(functionCode)
                 << "Parameters:" << parameters.size();
        emit deserializationCompleted(messageType, true, parameters.size());
    }

    recordStatistics(messageType, "deserialize", success, data.size());
    return success;
}

void MessageSerializer::recordStatistics(MessageType messageType, const QString& operation, bool success, int dataSize) {
    auto& stats = statistics_[messageType];

    if (operation == "serialize") {
        stats.serializeCount++;
        if (!success) {
            stats.serializeErrorCount++;
        }
    } else if (operation == "deserialize") {
        stats.deserializeCount++;
        if (!success) {
            stats.deserializeErrorCount++;
        }
    }

    if (success) {
        stats.totalBytesProcessed += dataSize;
    }
}

} // namespace Protocol
