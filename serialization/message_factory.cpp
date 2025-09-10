#include "message_factory.h"
#include "../handlers/anc_message_handler.h"
#include "../handlers/enc_message_handler.h"
#include "../handlers/alpha_message_handler.h"
#include "../handlers/vehicle_message_handler.h"
#include "../handlers/channel_message_handler.h"
#include <QDebug>

namespace Protocol {

MessageFactory::MessageFactory() {
    initializeDefaultHandlers();
}

IMessageHandler* MessageFactory::getHandler(MessageType messageType) const {
    auto it = handlers_.find(messageType);
    return (it != handlers_.end()) ? it->get() : nullptr;
}

bool MessageFactory::registerHandler(MessageType messageType, std::shared_ptr<IMessageHandler> handler) {
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

    handlers_[messageType] = handler;
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
    // 注册ANC开关处理器（ANC/ENC/RNC组合开关）
    auto ancHandler = std::make_shared<AncMessageHandler>();
    registerHandler(MessageType::ANC_SWITCH, ancHandler);

    // 注册Alpha参数处理器（RNC步长参数）
    auto alphaHandler = std::make_shared<AlphaMessageHandler>();
    registerHandler(MessageType::ALPHA_PARAMS, alphaHandler);

    // 注册车辆状态处理器
    auto vehicleHandler = std::make_shared<VehicleMessageHandler>();
    registerHandler(MessageType::VEHICLE_STATE, vehicleHandler);

    // 注册通道处理器（实时数据流相关）
    auto channelNumberHandler = std::make_shared<ChannelMessageHandler>(ChannelMessageHandler::ChannelMessageSubType::CHANNEL_NUMBER);
    registerHandler(MessageType::CHANNEL_NUMBER, channelNumberHandler);

    auto channelAmplitudeHandler = std::make_shared<ChannelMessageHandler>(ChannelMessageHandler::ChannelMessageSubType::CHANNEL_AMPLITUDE);
    registerHandler(MessageType::CHANNEL_AMPLITUDE, channelAmplitudeHandler);

    auto channelSwitchHandler = std::make_shared<ChannelMessageHandler>(ChannelMessageHandler::ChannelMessageSubType::CHANNEL_SWITCH);
    registerHandler(MessageType::CHANNEL_SWITCH, channelSwitchHandler);

    // TODO: 可以根据需要添加更多处理器
    // - TRAN_FUNC_FLAG, TRAN_FUNC_STATE (传函标定)
    // - FILTER_RANGES, SYSTEM_RANGES (系统配置)
    // - ORDER_FLAG, ORDER2_PARAMS, ORDER4_PARAMS, ORDER6_PARAMS (ENC标定)
    // - FREQ_DIVISION, THRESHOLDS (RNC其他参数)
    // - CHECK_MOD, GRAPH_DATA (数据流控制)

    qInfo() << "ERNC Protocol message handlers initialized:" << handlers_.size() << "handlers";
    qInfo() << "Supported message types:";
    for (const auto& type : getSupportedTypes()) {
        qInfo() << "  -" << static_cast<int>(type) << ":" << getTypeDescription(type);
    }
}

} // namespace Protocol
