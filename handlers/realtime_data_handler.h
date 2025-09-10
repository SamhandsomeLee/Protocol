#ifndef REALTIME_DATA_HANDLER_H
#define REALTIME_DATA_HANDLER_H

#include "core/imessage_handler.h"
#include "core/message_types.h"
#include <QVariantMap>
#include <QByteArray>

namespace Protocol {

/**
 * @brief 实时数据处理器
 *
 * 处理实时数据流消息，包括通道数据、幅值数据等
 * 支持CHECK_MOD消息类型(ProtoID: 150)
 */
class RealtimeDataHandler : public IMessageHandler {
public:
    RealtimeDataHandler() = default;
    ~RealtimeDataHandler() override = default;

    QByteArray serialize(const QVariantMap& parameters) override;
    bool deserialize(const QByteArray& data, QVariantMap& parameters) override;
    MessageType getMessageType() const override { return MessageType::CHECK_MOD; }
    bool validateParameters(const QVariantMap& parameters) const override;
    QString getDescription() const override { return "Realtime data stream message handler"; }

private:
    static constexpr int MAX_BUFFER_SIZE = 512;
    static constexpr int MAX_CHANNELS = 32;

    bool validateChannelData(const QVariantMap& parameters) const;
    bool validateAmplitudeData(const QVariantMap& parameters) const;
};

} // namespace Protocol

#endif // REALTIME_DATA_HANDLER_H

