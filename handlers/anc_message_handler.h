#ifndef ANC_MESSAGE_HANDLER_H
#define ANC_MESSAGE_HANDLER_H

#include "../core/imessage_handler.h"

extern "C" {
#include "../nanopb/pb.h"
#include "../nanopb/pb_encode.h"
#include "../nanopb/pb_decode.h"
#include "../messages/ERNC_praram.pb.h"
}

namespace Protocol {

/**
 * @brief ANC消息处理器 - 基于ERNC_praram.proto
 *
 * 负责处理ANC/ENC/RNC开关状态消息的序列化和反序列化
 */
class AncMessageHandler : public IMessageHandler {
public:
    AncMessageHandler() = default;
    ~AncMessageHandler() override = default;

    QByteArray serialize(const QVariantMap& parameters) override;
    bool deserialize(const QByteArray& data, QVariantMap& parameters) override;
    MessageType getMessageType() const override { return MessageType::ANC_SWITCH; }
    bool validateParameters(const QVariantMap& parameters) const override;
    QString getDescription() const override { return "ANC/ENC/RNC switch control message handler"; }

private:
    static constexpr int MAX_BUFFER_SIZE = 64;
};

} // namespace Protocol

#endif // ANC_MESSAGE_HANDLER_H
