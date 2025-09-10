#include "rnc_message_handler.h"
#include <QDebug>

namespace Protocol {

QByteArray RncMessageHandler::serialize(const QVariantMap& parameters) {
    if (!validateParameters(parameters)) {
        qWarning() << "Invalid parameters for RNC message";
        return QByteArray();
    }

    MSG_AncSwitch msg = MSG_AncSwitch_init_zero;

    // 设置RNC开关状态
    bool rncEnabled = parameters.value("rnc.enabled", false).toBool();
    msg.rnc_off = !rncEnabled; // 设置RNC_OFF字段（true=关闭，false=开启）

    // 序列化消息
    uint8_t buffer[MAX_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, MSG_AncSwitch_fields, &msg)) {
        qWarning() << "Failed to encode RNC message:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    QByteArray result(reinterpret_cast<char*>(buffer), stream.bytes_written);
    qDebug() << "RNC message serialized:" << result.size() << "bytes, RNC enabled:" << rncEnabled;

    return result;
}

bool RncMessageHandler::deserialize(const QByteArray& data, QVariantMap& parameters) {
    if (data.isEmpty()) {
        qWarning() << "Empty data for RNC message deserialization";
        return false;
    }

    MSG_AncSwitch msg = MSG_AncSwitch_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()),
        data.size()
    );

    if (!pb_decode(&stream, MSG_AncSwitch_fields, &msg)) {
        qWarning() << "Failed to decode RNC message:" << PB_GET_ERROR(&stream);
        return false;
    }

    // 解析RNC状态
    bool rncEnabled = !msg.rnc_off; // 反转逻辑，rnc_off=true表示关闭
    parameters["rnc.enabled"] = rncEnabled;

    qDebug() << "RNC message deserialized: RNC enabled:" << rncEnabled;
    return true;
}

bool RncMessageHandler::validateParameters(const QVariantMap& parameters) const {
    // 检查必需的参数
    if (!parameters.contains("rnc.enabled")) {
        qWarning() << "Missing required parameter: rnc.enabled";
        return false;
    }

    // 检查参数类型
    QVariant enabledValue = parameters.value("rnc.enabled");
    if (!enabledValue.canConvert<bool>()) {
        qWarning() << "Invalid type for rnc.enabled, expected bool, got:" << enabledValue.typeName();
        return false;
    }

    return true;
}

} // namespace Protocol
