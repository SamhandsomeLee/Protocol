#ifndef VEHICLE_MESSAGE_HANDLER_H
#define VEHICLE_MESSAGE_HANDLER_H

#include "../core/imessage_handler.h"

extern "C" {
#include "../nanopb/pb.h"
#include "../nanopb/pb_encode.h"
#include "../nanopb/pb_decode.h"
#include "../messages/ERNC_praram.pb.h"
}

namespace Protocol {

/**
 * @brief 车辆状态消息处理器 - 基于ERNC_praram.proto
 *
 * 负责处理车辆状态信息（车速/转速/空调等）的序列化和反序列化
 */
class VehicleMessageHandler : public IMessageHandler {
public:
    VehicleMessageHandler() = default;
    ~VehicleMessageHandler() override = default;

    QByteArray serialize(const QVariantMap& parameters) override;
    bool deserialize(const QByteArray& data, QVariantMap& parameters) override;
    MessageType getMessageType() const override { return MessageType::VEHICLE_STATE; }
    bool validateParameters(const QVariantMap& parameters) const override;
    QString getDescription() const override { return "Vehicle state information message handler"; }

private:
    static constexpr int MAX_BUFFER_SIZE = 512;
    static constexpr uint32_t MAX_SPEED = 300;        // 最大车速 (km/h)
    static constexpr uint32_t MAX_ENGINE_SPEED = 8000; // 最大发动机转速 (rpm)
    static constexpr uint32_t MAX_DOORS = 5;           // 车门数量
    static constexpr uint32_t MAX_WINDOWS = 4;         // 车窗数量
    static constexpr uint32_t MAX_MEDIA_PARAMS = 8;    // 媒体参数数量

    // 验证辅助方法
    bool isValidSpeed(uint32_t speed) const;
    bool isValidEngineSpeed(uint32_t engineSpeed) const;
    bool isValidDoorStates(const QVariantList& doors) const;
    bool isValidWindowStates(const QVariantList& windows) const;
};

} // namespace Protocol

#endif // VEHICLE_MESSAGE_HANDLER_H
