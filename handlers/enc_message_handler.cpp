#include "enc_message_handler.h"
#include <QDebug>

namespace Protocol {

QByteArray EncMessageHandler::serialize(const QVariantMap& parameters) {
    if (!validateParameters(parameters)) {
        qWarning() << "Invalid parameters for ENC message";
        return QByteArray();
    }

    MSG_AncSwitch msg = MSG_AncSwitch_init_zero;

    // 设置ENC开关状态
    bool encEnabled = parameters.value("enc.enabled", false).toBool();
    msg.enc_off = !encEnabled; // 设置ENC_OFF字段（true=关闭，false=开启）

    // 序列化消息
    uint8_t buffer[MAX_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, MSG_AncSwitch_fields, &msg)) {
        qWarning() << "Failed to encode ENC message:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    QByteArray result(reinterpret_cast<char*>(buffer), stream.bytes_written);
    qDebug() << "ENC message serialized:" << result.size() << "bytes, ENC enabled:" << encEnabled;

    return result;
}

bool EncMessageHandler::deserialize(const QByteArray& data, QVariantMap& parameters) {
    if (data.isEmpty()) {
        qWarning() << "Empty data for ENC message deserialization";
        return false;
    }

    MSG_AncSwitch msg = MSG_AncSwitch_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()),
        data.size()
    );

    if (!pb_decode(&stream, MSG_AncSwitch_fields, &msg)) {
        qWarning() << "Failed to decode ENC message:" << PB_GET_ERROR(&stream);
        return false;
    }

    // 解析ENC状态
    bool encEnabled = !msg.enc_off; // 反转逻辑，enc_off=true表示关闭
    parameters["enc.enabled"] = encEnabled;

    qDebug() << "ENC message deserialized: ENC enabled:" << encEnabled;
    return true;
}

bool EncMessageHandler::validateParameters(const QVariantMap& parameters) const {
    // 检查必需的参数
    if (!parameters.contains("enc.enabled")) {
        qWarning() << "Missing required parameter: enc.enabled";
        return false;
    }

    // 检查参数类型
    QVariant enabledValue = parameters.value("enc.enabled");
    if (!enabledValue.canConvert<bool>()) {
        qWarning() << "Invalid type for enc.enabled, expected bool, got:" << enabledValue.typeName();
        return false;
    }

    return true;
}

} // namespace Protocol
