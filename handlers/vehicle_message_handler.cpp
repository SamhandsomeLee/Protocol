#include "vehicle_message_handler.h"
#include <QLoggingCategory>
#include <QVariantList>

Q_LOGGING_CATEGORY(vehicleHandler, "protocol.handler.vehicle")

namespace Protocol {

QByteArray VehicleMessageHandler::serialize(const QVariantMap& parameters)
{
    qCDebug(vehicleHandler) << "=== VehicleMessageHandler::serialize() ===";
    qCDebug(vehicleHandler) << "Input parameters count:" << parameters.size();
    qCDebug(vehicleHandler) << "Input parameters keys:" << parameters.keys();

    // 打印所有参数的详细内容
    for (auto it = parameters.constBegin(); it != parameters.constEnd(); ++it) {
        const QString& key = it.key();
        const QVariant& value = it.value();
        qCDebug(vehicleHandler) << "Parameter [" << key << "]:"
                                << "Type:" << value.typeName()
                                << "Value:" << value.toString()
                                << "IsValid:" << value.isValid()
                                << "CanConvertToUInt:" << value.canConvert<uint32_t>()
                                << "CanConvertToList:" << value.canConvert<QVariantList>();

        // 如果是列表类型，打印列表内容
        if (value.canConvert<QVariantList>()) {
            QVariantList list = value.toList();
            QStringList listItems;
            for (int i = 0; i < list.size(); ++i) {
                const QVariant& item = list[i];
                listItems << QString("[%1]:%2(%3)").arg(i).arg(item.toString()).arg(item.typeName());
            }
            qCDebug(vehicleHandler) << "  List content:" << listItems.join(", ");
        }
    }

    MSG_VehicleState vehicleState = MSG_VehicleState_init_zero;

    // 基本车辆信息 - 使用标准化参数名称
    qCDebug(vehicleHandler) << "--- Processing Basic Vehicle Information ---";

    // 车速 (vehicle.speed)
    if (parameters.contains("vehicle.speed")) {
        QVariant speedVariant = parameters["vehicle.speed"];
        uint32_t speedValue = speedVariant.toUInt();
        vehicleState.speed = speedValue;
        qCDebug(vehicleHandler) << "Set vehicle.speed:" << speedValue
                                << "from variant type:" << speedVariant.typeName()
                                << "original value:" << speedVariant.toString();
    } else {
        qCDebug(vehicleHandler) << "No vehicle.speed parameter, using default: 0";
    }

    // 发动机转速 (vehicle.engine_speed)
    if (parameters.contains("vehicle.engine_speed")) {
        QVariant engineSpeedVariant = parameters["vehicle.engine_speed"];
        uint32_t engineSpeedValue = engineSpeedVariant.toUInt();
        vehicleState.EngineSpeed = engineSpeedValue;
        qCDebug(vehicleHandler) << "Set vehicle.engine_speed:" << engineSpeedValue
                                << "from variant type:" << engineSpeedVariant.typeName()
                                << "original value:" << engineSpeedVariant.toString();
    } else {
        qCDebug(vehicleHandler) << "No vehicle.engine_speed parameter, using default: 0";
    }

    // 空调状态 (vehicle.ac)
    if (parameters.contains("vehicle.ac")) {
        QVariant acVariant = parameters["vehicle.ac"];
        uint32_t acValue = acVariant.toUInt();
        vehicleState.AC = acValue;
        qCDebug(vehicleHandler) << "Set vehicle.ac:" << acValue
                                << "from variant type:" << acVariant.typeName()
                                << "original value:" << acVariant.toString();
    } else {
        qCDebug(vehicleHandler) << "No vehicle.ac parameter, using default: 0";
    }

    // 挡位 (vehicle.gear)
    if (parameters.contains("vehicle.gear")) {
        QVariant gearVariant = parameters["vehicle.gear"];
        uint32_t gearValue = gearVariant.toUInt();
        vehicleState.gear = gearValue;
        qCDebug(vehicleHandler) << "Set vehicle.gear:" << gearValue
                                << "from variant type:" << gearVariant.typeName()
                                << "original value:" << gearVariant.toString();
    } else {
        qCDebug(vehicleHandler) << "No vehicle.gear parameter, using default: 0";
    }

    // 驾驶模式 (vehicle.drive_mod)
    if (parameters.contains("vehicle.drive_mod")) {
        QVariant driveModVariant = parameters["vehicle.drive_mod"];
        uint32_t driveModValue = driveModVariant.toUInt();
        vehicleState.drive_mod = driveModValue;
        qCDebug(vehicleHandler) << "Set vehicle.drive_mod:" << driveModValue
                                << "from variant type:" << driveModVariant.typeName()
                                << "original value:" << driveModVariant.toString();
    } else {
        qCDebug(vehicleHandler) << "No vehicle.drive_mod parameter, using default: 0";
    }

    // 车门状态 - 使用标准化参数名称 (vehicle.doors)
    qCDebug(vehicleHandler) << "--- Processing Door States ---";
    if (parameters.contains("vehicle.doors")) {
        QVariant doorVariant = parameters["vehicle.doors"];
        qCDebug(vehicleHandler) << "vehicle.doors parameter type:" << doorVariant.typeName()
                                << "value:" << doorVariant.toString();

        QVariantList doorStates = doorVariant.toList();
        int doorCount = qMin(doorStates.size(), 5); // 最多5个车门
        qCDebug(vehicleHandler) << "Door states input size:" << doorStates.size()
                                << "processing count:" << doorCount;

        // 打印输入的每个车门状态
        for (int i = 0; i < doorStates.size(); ++i) {
            const QVariant& doorItem = doorStates[i];
            qCDebug(vehicleHandler) << "Input vehicle.doors[" << i << "]:"
                                    << "type:" << doorItem.typeName()
                                    << "value:" << doorItem.toString()
                                    << "toUInt:" << doorItem.toUInt();
        }

        for (int i = 0; i < doorCount; ++i) {
            uint32_t doorValue = doorStates[i].toUInt();
            vehicleState.door[i] = doorValue;
            qCDebug(vehicleHandler) << "Set door[" << i << "]:" << doorValue;
        }

        // 打印完整的车门状态数组
        QStringList doorStatusList;
        for (int i = 0; i < 5; ++i) {
            doorStatusList << QString::number(vehicleState.door[i]);
        }
        qCDebug(vehicleHandler) << "Final door array: [" << doorStatusList.join(", ") << "]";
    } else {
        qCDebug(vehicleHandler) << "No vehicle.doors parameter, using default: [0, 0, 0, 0, 0]";
    }

    // 车窗状态 - 使用标准化参数名称 (vehicle.windows)
    qCDebug(vehicleHandler) << "--- Processing Window States ---";
    if (parameters.contains("vehicle.windows")) {
        QVariant windowVariant = parameters["vehicle.windows"];
        qCDebug(vehicleHandler) << "vehicle.windows parameter type:" << windowVariant.typeName()
                                << "value:" << windowVariant.toString();

        QVariantList windowStates = windowVariant.toList();
        int windowCount = qMin(windowStates.size(), 4); // 最多4个车窗
        qCDebug(vehicleHandler) << "Window states input size:" << windowStates.size()
                                << "processing count:" << windowCount;

        // 打印输入的每个车窗状态
        for (int i = 0; i < windowStates.size(); ++i) {
            const QVariant& windowItem = windowStates[i];
            qCDebug(vehicleHandler) << "Input vehicle.windows[" << i << "]:"
                                    << "type:" << windowItem.typeName()
                                    << "value:" << windowItem.toString()
                                    << "toUInt:" << windowItem.toUInt();
        }

        for (int i = 0; i < windowCount; ++i) {
            uint32_t windowValue = windowStates[i].toUInt();
            vehicleState.window[i] = windowValue;
            qCDebug(vehicleHandler) << "Set window[" << i << "]:" << windowValue;
        }

        // 打印完整的车窗状态数组
        QStringList windowStatusList;
        for (int i = 0; i < 4; ++i) {
            windowStatusList << QString::number(vehicleState.window[i]);
        }
        qCDebug(vehicleHandler) << "Final window array: [" << windowStatusList.join(", ") << "]";
    } else {
        qCDebug(vehicleHandler) << "No vehicle.windows parameter, using default: [0, 0, 0, 0]";
    }

    // 注意：media参数未在参数映射配置中定义，因此不处理
    // 根据parameter_mapping.json，VEHICLE_STATE消息类型只包含：
    // vehicle.speed, vehicle.engine_speed, vehicle.ac, vehicle.gear, vehicle.drive_mod, vehicle.doors, vehicle.windows
    qCDebug(vehicleHandler) << "--- Media parameters are not supported in VEHICLE_STATE message type ---";

    // 序列化消息
    qCDebug(vehicleHandler) << "--- Starting Protobuf Serialization ---";
    qCDebug(vehicleHandler) << "Buffer size:" << MAX_BUFFER_SIZE;

    // 打印最终的vehicleState结构内容
    qCDebug(vehicleHandler) << "Final VehicleState structure:";
    qCDebug(vehicleHandler) << "  speed:" << vehicleState.speed;
    qCDebug(vehicleHandler) << "  EngineSpeed:" << vehicleState.EngineSpeed;
    qCDebug(vehicleHandler) << "  AC:" << vehicleState.AC;
    qCDebug(vehicleHandler) << "  gear:" << vehicleState.gear;
    qCDebug(vehicleHandler) << "  drive_mod:" << vehicleState.drive_mod;

    QStringList finalDoorList, finalWindowList;
    for (int i = 0; i < 5; ++i) finalDoorList << QString::number(vehicleState.door[i]);
    for (int i = 0; i < 4; ++i) finalWindowList << QString::number(vehicleState.window[i]);

    qCDebug(vehicleHandler) << "  door[5]:" << finalDoorList.join(",");
    qCDebug(vehicleHandler) << "  window[4]:" << finalWindowList.join(",");
    qCDebug(vehicleHandler) << "  media[8]: not processed (not in parameter mapping)";

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

    qCDebug(vehicleHandler) << "Protobuf encoding successful!";
    qCDebug(vehicleHandler) << "Bytes written:" << stream.bytes_written;
    qCDebug(vehicleHandler) << "Result size:" << result.size();

    // 打印序列化后的十六进制数据
    QString hexData = result.toHex(' ').toUpper();
    qCDebug(vehicleHandler) << "Serialized data (hex with spaces):" << hexData;

    // 打印无空格的十六进制数据（方便复制粘贴）
    QString hexDataNoSpaces = result.toHex().toUpper();
    qCDebug(vehicleHandler) << "Serialized data (hex no spaces):" << hexDataNoSpaces;

    // 打印每个字节的详细信息
    int printLen = qMin(result.size(), 64); // 增加到64字节
    QStringList byteDetails;
    for (int i = 0; i < printLen; ++i) {
        uint8_t byte = static_cast<uint8_t>(result[i]);
        byteDetails << QString("0x%1").arg(byte, 2, 16, QChar('0')).toUpper();
    }
    qCDebug(vehicleHandler) << "First" << printLen << "bytes detailed:" << byteDetails.join(" ");

    // 以16字节为一行打印
    for (int i = 0; i < printLen; i += 16) {
        QStringList lineBytes;
        for (int j = 0; j < 16 && (i + j) < printLen; ++j) {
            uint8_t byte = static_cast<uint8_t>(result[i + j]);
            lineBytes << QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
        }
        qCDebug(vehicleHandler) << QString("Offset %1:").arg(i, 4, 16, QChar('0')).toUpper()
                                << lineBytes.join(" ");
    }

    qCDebug(vehicleHandler) << "=== VehicleMessageHandler::serialize() END ===";
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

    // 提取基本车辆信息 - 使用标准化参数名称
    parameters["vehicle.speed"] = vehicleState.speed;
    parameters["vehicle.engine_speed"] = vehicleState.EngineSpeed;
    parameters["vehicle.ac"] = vehicleState.AC;
    parameters["vehicle.gear"] = vehicleState.gear;
    parameters["vehicle.drive_mod"] = vehicleState.drive_mod;

    // 提取车门状态 (固定5个车门)
    QVariantList doorStates;
    for (size_t i = 0; i < 5; ++i) {
        doorStates.append(vehicleState.door[i]);
    }
    parameters["vehicle.doors"] = doorStates;

    // 提取车窗状态 (固定4个车窗)
    QVariantList windowStates;
    for (size_t i = 0; i < 4; ++i) {
        windowStates.append(vehicleState.window[i]);
    }
    parameters["vehicle.windows"] = windowStates;

    // 注意：media参数未在参数映射配置中定义，因此不输出

    qCDebug(vehicleHandler) << "Successfully deserialized vehicle state message";
    return true;
}

bool VehicleMessageHandler::validateParameters(const QVariantMap& parameters) const
{
    // 验证车速 - 使用标准化参数名称
    if (parameters.contains("vehicle.speed")) {
        uint32_t speed = parameters["vehicle.speed"].toUInt();
        if (!isValidSpeed(speed)) {
            qCWarning(vehicleHandler) << "Invalid vehicle.speed value:" << speed;
            return false;
        }
    }

    // 验证发动机转速
    if (parameters.contains("vehicle.engine_speed")) {
        uint32_t engineSpeed = parameters["vehicle.engine_speed"].toUInt();
        if (!isValidEngineSpeed(engineSpeed)) {
            qCWarning(vehicleHandler) << "Invalid vehicle.engine_speed value:" << engineSpeed;
            return false;
        }
    }

    // 验证车门状态
    if (parameters.contains("vehicle.doors")) {
        QVariantList doorStates = parameters["vehicle.doors"].toList();
        if (!isValidDoorStates(doorStates)) {
            qCWarning(vehicleHandler) << "Invalid vehicle.doors states";
            return false;
        }
    }

    // 验证车窗状态
    if (parameters.contains("vehicle.windows")) {
        QVariantList windowStates = parameters["vehicle.windows"].toList();
        if (!isValidWindowStates(windowStates)) {
            qCWarning(vehicleHandler) << "Invalid vehicle.windows states";
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
