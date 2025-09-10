#ifndef CHANNEL_MESSAGE_HANDLER_H
#define CHANNEL_MESSAGE_HANDLER_H

#include "../core/imessage_handler.h"

extern "C" {
#include "../nanopb/pb.h"
#include "../nanopb/pb_encode.h"
#include "../nanopb/pb_decode.h"
#include "../messages/ERNC_praram.pb.h"
}

namespace Protocol {

/**
 * @brief 通道消息处理器 - 基于ERNC_praram.proto
 *
 * 负责处理实时数据流相关消息的序列化和反序列化
 * 支持通道数量、通道幅值、通道开关等消息类型
 */
class ChannelMessageHandler : public IMessageHandler {
public:
    enum class ChannelMessageSubType {
        CHANNEL_NUMBER,
        CHANNEL_AMPLITUDE,
        CHANNEL_SWITCH
    };

    explicit ChannelMessageHandler(ChannelMessageSubType subType);
    ~ChannelMessageHandler() override = default;

    QByteArray serialize(const QVariantMap& parameters) override;
    bool deserialize(const QByteArray& data, QVariantMap& parameters) override;
    MessageType getMessageType() const override;
    bool validateParameters(const QVariantMap& parameters) const override;
    QString getDescription() const override;

private:
    ChannelMessageSubType subType_;

    static constexpr int MAX_BUFFER_SIZE = 256;
    static constexpr uint32_t MAX_CHANNEL_COUNT = 32;
    static constexpr uint32_t MAX_INPUT_AMPLITUDE_COUNT = 13;
    static constexpr uint32_t MAX_INPUT_SWITCH_COUNT = 20;
    static constexpr uint32_t MAX_OUTPUT_SWITCH_COUNT = 8;
    static constexpr uint32_t MAX_AMPLITUDE_VALUE = 65535;

    // 子消息处理方法
    QByteArray serializeChannelNumber(const QVariantMap& parameters);
    QByteArray serializeChannelAmplitude(const QVariantMap& parameters);
    QByteArray serializeChannelSwitch(const QVariantMap& parameters);

    bool deserializeChannelNumber(const QByteArray& data, QVariantMap& parameters);
    bool deserializeChannelAmplitude(const QByteArray& data, QVariantMap& parameters);
    bool deserializeChannelSwitch(const QByteArray& data, QVariantMap& parameters);

    // 验证辅助方法
    bool isValidChannelCount(uint32_t count) const;
    bool isValidAmplitudeArray(const QVariantList& amplitudes) const;
    bool isValidSwitchArray(const QVariantList& switches, uint32_t maxCount) const;
};

} // namespace Protocol

#endif // CHANNEL_MESSAGE_HANDLER_H
