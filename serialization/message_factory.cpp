#include "message_factory.h"
#include "../handlers/anc_message_handler.h"
#include "../handlers/enc_message_handler.h"
#include "../handlers/alpha_message_handler.h"
#include <QDebug>

namespace Protocol {

MessageFactory::MessageFactory() {
    initializeDefaultHandlers();
}

IMessageHandler* MessageFactory::getHandler(MessageType messageType) const {
    auto it = handlers_.find(messageType);
    return (it != handlers_.end()) ? it->get() : nullptr;
}

bool MessageFactory::registerHandler(MessageType messageType, std::unique_ptr<IMessageHandler> handler) {
    if (!handler) {
        qWarning() << "Cannot register null handler for message type:" << static_cast<int>(messageType);
        return false;
    }

    // 验证处理器的消息类型是否匹配
    if (handler->getMessageType() != messageType) {
        qWarning() << "Handler message type mismatch. Expected:" << static_cast<int>(messageType)
                   << "Got:" << static_cast<int>(handler->getMessageType());
        return false;
    }

    handlers_[messageType] = std::move(handler);
    qDebug() << "Registered message handler for type:" << static_cast<int>(messageType);
    return true;
}

bool MessageFactory::isSupported(MessageType messageType) const {
    return handlers_.contains(messageType);
}

QList<MessageType> MessageFactory::getSupportedTypes() const {
    return handlers_.keys();
}

QString MessageFactory::getTypeDescription(MessageType messageType) const {
    auto handler = getHandler(messageType);
    return handler ? handler->getDescription() : QString();
}

void MessageFactory::clear() {
    handlers_.clear();
    qDebug() << "All message handlers cleared";
}

void MessageFactory::initializeDefaultHandlers() {
    // 注册ANC处理器
    auto ancHandler = std::make_unique<AncMessageHandler>();
    registerHandler(MessageType::ANC_OFF, std::move(ancHandler));

    // 注册ENC处理器
    auto encHandler = std::make_unique<EncMessageHandler>();
    registerHandler(MessageType::ENC_OFF, std::move(encHandler));

    // 注册Alpha处理器
    auto alphaHandler = std::make_unique<AlphaMessageHandler>();
    registerHandler(MessageType::ALPHA, std::move(alphaHandler));

    // TODO: 添加其他消息处理器
    // RNC, SET1, CALIBRATION等处理器可以后续添加

    qInfo() << "Default message handlers initialized:" << handlers_.size() << "handlers";
}

} // namespace Protocol
