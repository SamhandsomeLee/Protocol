#ifndef RNC_MESSAGE_HANDLER_H
#define RNC_MESSAGE_HANDLER_H

#include "../core/imessage_handler.h"

extern "C" {
#include "../nanopb/pb.h"
#include "../nanopb/pb_encode.h"
#include "../nanopb/pb_decode.h"
#include "../messages/ERNC_praram.pb.h"
}

namespace Protocol {

/**
 * @brief RNC消息处理器
 *
 * 负责处理RNC相关消息的序列化和反序列化
 */
class RncMessageHandler : public IMessageHandler {
public:
    RncMessageHandler() = default;
    ~RncMessageHandler() override = default;

    QByteArray serialize(const QVariantMap& parameters) override;
    bool deserialize(const QByteArray& data, QVariantMap& parameters) override;
    MessageType getMessageType() const override { return MessageType::ANC_SWITCH; }
    bool validateParameters(const QVariantMap& parameters) const override;
    QString getDescription() const override { return "RNC control message handler"; }

private:
    static constexpr int MAX_BUFFER_SIZE = 64;
};

} // namespace Protocol

#endif // RNC_MESSAGE_HANDLER_H