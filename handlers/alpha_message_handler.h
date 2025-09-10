#ifndef ALPHA_MESSAGE_HANDLER_H
#define ALPHA_MESSAGE_HANDLER_H

#include "../core/imessage_handler.h"

extern "C" {
#include "../nanopb/pb.h"
#include "../nanopb/pb_encode.h"
#include "../nanopb/pb_decode.h"
#include "../messages/ERNC_praram.pb.h"
}

namespace Protocol {

/**
 * @brief Alpha参数消息处理器 - 基于ERNC_praram.proto
 *
 * 负责处理RNC步长参数相关消息的序列化和反序列化
 */
class AlphaMessageHandler : public IMessageHandler {
public:
    AlphaMessageHandler() = default;
    ~AlphaMessageHandler() override = default;

    QByteArray serialize(const QVariantMap& parameters) override;
    bool deserialize(const QByteArray& data, QVariantMap& parameters) override;
    MessageType getMessageType() const override { return MessageType::ALPHA_PARAMS; }
    bool validateParameters(const QVariantMap& parameters) const override;
    QString getDescription() const override { return "RNC Alpha step parameter message handler"; }

private:
    static constexpr int MAX_BUFFER_SIZE = 256;
    static constexpr float MIN_ALPHA_VALUE = 0.0f;
    static constexpr float MAX_ALPHA_VALUE = 1.0f;
};

} // namespace Protocol

#endif // ALPHA_MESSAGE_HANDLER_H
