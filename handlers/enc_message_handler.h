#ifndef ENC_MESSAGE_HANDLER_H
#define ENC_MESSAGE_HANDLER_H

#include "../core/imessage_handler.h"

extern "C" {
#include "../nanopb/pb.h"
#include "../nanopb/pb_encode.h"
#include "../nanopb/pb_decode.h"
#include "../messages/ernc.pb.h"
}

namespace Protocol {

/**
 * @brief ENC消息处理器
 *
 * 负责处理ENC相关消息的序列化和反序列化
 */
class EncMessageHandler : public IMessageHandler {
public:
    EncMessageHandler() = default;
    ~EncMessageHandler() override = default;

    QByteArray serialize(const QVariantMap& parameters) override;
    bool deserialize(const QByteArray& data, QVariantMap& parameters) override;
    MessageType getMessageType() const override { return MessageType::ENC_OFF; }
    bool validateParameters(const QVariantMap& parameters) const override;
    QString getDescription() const override { return "ENC control message handler"; }

private:
    static constexpr int MAX_BUFFER_SIZE = 64;
};

} // namespace Protocol

#endif // ENC_MESSAGE_HANDLER_H
