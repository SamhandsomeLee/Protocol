#include "vehicle_message_handler.h"
#include <QLoggingCategory>
#include <QVariantList>

Q_LOGGING_CATEGORY(vehicleHandler, "protocol.handler.vehicle")

namespace Protocol {

QByteArray VehicleMessageHandler::serialize(const QVariantMap& parameters)
{
    MSG_VehicleState vehicleState = MSG_VehicleState_init_zero;

    // 基本车辆信息
    if (parameters.contains("speed")) {
        vehicleState.speed = parameters["speed"].toUInt();
    }

    if (parameters.contains("engine_speed")) {
        vehicleState.EngineSpeed = parameters["engine_speed"].toUInt();
    }

    if (parameters.contains("ac")) {
        vehicleState.AC = parameters["ac"].toUInt();
    }

    if (parameters.contains("gear")) {
        vehicleState.gear = parameters["gear"].toUInt();
    }

    if (parameters.contains("drive_mod")) {
        vehicleState.drive_mod = parameters["drive_mod"].toUInt();
    }

    // 车门状态
    if (parameters.contains("door")) {
        QVariantList doorStates = parameters["door"].toList();
        int doorCount = qMin(doorStates.size(), 5); // 最多5个车门
        for (int i = 0; i < doorCount; ++i) {
            vehicleState.door[i] = doorStates[i].toUInt();
        }
    }

    // 车窗状态
    if (parameters.contains("window")) {
        QVariantList windowStates = parameters["window"].toList();
        int windowCount = qMin(windowStates.size(), 4); // 最多4个车窗
        for (int i = 0; i < windowCount; ++i) {
            vehicleState.window[i] = windowStates[i].toUInt();
        }
    }

    // 媒体参数
    if (parameters.contains("media")) {
        QVariantList mediaParams = parameters["media"].toList();
        int mediaCount = qMin(mediaParams.size(), 8); // 最多8个媒体参数
        for (int i = 0; i < mediaCount; ++i) {
            vehicleState.media[i] = mediaParams[i].toUInt();
        }
    }

    // 序列化消息
    uint8_t buffer[MAX_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool status = pb_encode(&stream, MSG_VehicleState_fields, &vehicleState);
    if (!status) {
        qCWarning(vehicleHandler) << "Failed to encode vehicle state message:"
                                  << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    QByteArray result(reinterpret_cast<const char*>(buffer),
                      static_cast<int>(stream.bytes_written));

    qCDebug(vehicleHandler) << "Serialized vehicle state message, size:" << result.size();
    return result;
}

bool VehicleMessageHandler::deserialize(const QByteArray& data, QVariantMap& parameters)
{
    if (data.isEmpty()) {
        qCWarning(vehicleHandler) << "Empty data received";
        return false;
    }

    // 解码车辆状态消息
    MSG_VehicleState vehicleState = MSG_VehicleState_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()),
        static_cast<size_t>(data.size())
    );

    bool status = pb_decode(&stream, MSG_VehicleState_fields, &vehicleState);
    if (!status) {
        qCWarning(vehicleHandler) << "Failed to decode vehicle state message:"
                                  << PB_GET_ERROR(&stream);
        return false;
    }

    // 提取基本车辆信息
    parameters["speed"] = vehicleState.speed;
    parameters["engine_speed"] = vehicleState.EngineSpeed;
    parameters["ac"] = vehicleState.AC;
    parameters["gear"] = vehicleState.gear;
    parameters["drive_mod"] = vehicleState.drive_mod;

    // 提取车门状态 (固定5个车门)
    QVariantList doorStates;
    for (size_t i = 0; i < 5; ++i) {
        doorStates.append(vehicleState.door[i]);
    }
    parameters["door"] = doorStates;

    // 提取车窗状态 (固定4个车窗)
    QVariantList windowStates;
    for (size_t i = 0; i < 4; ++i) {
        windowStates.append(vehicleState.window[i]);
    }
    parameters["window"] = windowStates;

    // 提取媒体参数 (固定8个参数)
    QVariantList mediaParams;
    for (size_t i = 0; i < 8; ++i) {
        mediaParams.append(vehicleState.media[i]);
    }
    parameters["media"] = mediaParams;

    qCDebug(vehicleHandler) << "Successfully deserialized vehicle state message";
    return true;
}

bool VehicleMessageHandler::validateParameters(const QVariantMap& parameters) const
{
    // 验证车速
    if (parameters.contains("speed")) {
        uint32_t speed = parameters["speed"].toUInt();
        if (!isValidSpeed(speed)) {
            qCWarning(vehicleHandler) << "Invalid speed value:" << speed;
            return false;
        }
    }

    // 验证发动机转速
    if (parameters.contains("engine_speed")) {
        uint32_t engineSpeed = parameters["engine_speed"].toUInt();
        if (!isValidEngineSpeed(engineSpeed)) {
            qCWarning(vehicleHandler) << "Invalid engine speed value:" << engineSpeed;
            return false;
        }
    }

    // 验证车门状态
    if (parameters.contains("door")) {
        QVariantList doorStates = parameters["door"].toList();
        if (!isValidDoorStates(doorStates)) {
            qCWarning(vehicleHandler) << "Invalid door states";
            return false;
        }
    }

    // 验证车窗状态
    if (parameters.contains("window")) {
        QVariantList windowStates = parameters["window"].toList();
        if (!isValidWindowStates(windowStates)) {
            qCWarning(vehicleHandler) << "Invalid window states";
            return false;
        }
    }

    qCDebug(vehicleHandler) << "All vehicle parameters validated successfully";
    return true;
}

bool VehicleMessageHandler::isValidSpeed(uint32_t speed) const
{
    return speed <= MAX_SPEED;
}

bool VehicleMessageHandler::isValidEngineSpeed(uint32_t engineSpeed) const
{
    return engineSpeed <= MAX_ENGINE_SPEED;
}

bool VehicleMessageHandler::isValidDoorStates(const QVariantList& doors) const
{
    if (doors.size() > MAX_DOORS) {
        return false;
    }

    for (const QVariant& door : doors) {
        if (!door.canConvert<uint32_t>()) {
            return false;
        }
        uint32_t doorValue = door.toUInt();
        // 车门状态：0=关，1=开，允许其他状态值
        if (doorValue > 10) {  // 合理的上限
            return false;
        }
    }

    return true;
}

bool VehicleMessageHandler::isValidWindowStates(const QVariantList& windows) const
{
    if (windows.size() > MAX_WINDOWS) {
        return false;
    }

    for (const QVariant& window : windows) {
        if (!window.canConvert<uint32_t>()) {
            return false;
        }
        uint32_t windowValue = window.toUInt();
        // 车窗状态：0=关，1=开，允许其他状态值
        if (windowValue > 10) {  // 合理的上限
            return false;
        }
    }

    return true;
}

} // namespace Protocol
