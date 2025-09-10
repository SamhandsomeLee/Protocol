#include "anc_message_handler.h"
#include <QDebug>

namespace Protocol {

QByteArray AncMessageHandler::serialize(const QVariantMap& parameters) {
    qDebug() << "=== AncMessageHandler::serialize START ===";
    qDebug() << "Input parameters:" << parameters;
    qDebug() << "Parameters keys:" << parameters.keys();
    qDebug() << "Parameters size:" << parameters.size();

    if (!validateParameters(parameters)) {
        qWarning() << "Invalid parameters for ANC message";
        qDebug() << "=== AncMessageHandler::serialize END (validation failed) ===";
        return QByteArray();
    }
    qDebug() << "Parameter validation passed";

    MSG_AncSwitch msg = MSG_AncSwitch_init_zero;
    qDebug() << "MSG_AncSwitch initialized";

    // 设置ANC开关状态 (注意：ANC_OFF消息中true表示关闭ANC)
    QVariant ancEnabledVariant = parameters.value("anc.enabled", false);
    qDebug() << "Raw anc.enabled value:" << ancEnabledVariant;
    qDebug() << "anc.enabled variant type:" << ancEnabledVariant.typeName();

    bool ancEnabled = ancEnabledVariant.toBool();
    qDebug() << "Converted anc.enabled to bool:" << ancEnabled;

    msg.anc_off = !ancEnabled; // 设置ANC_OFF字段（true=关闭，false=开启）
    qDebug() << "MSG_AncSwitch.anc_off set to:" << msg.anc_off << "(inverted from ancEnabled:" << ancEnabled << ")";

    // 序列化消息
    uint8_t buffer[MAX_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    qDebug() << "Protobuf stream initialized, buffer size:" << MAX_BUFFER_SIZE;

    qDebug() << "Starting protobuf encoding...";
    if (!pb_encode(&stream, MSG_AncSwitch_fields, &msg)) {
        qWarning() << "Failed to encode ANC message:" << PB_GET_ERROR(&stream);
        qDebug() << "Stream bytes written before failure:" << stream.bytes_written;
        qDebug() << "=== AncMessageHandler::serialize END (encoding failed) ===";
        return QByteArray();
    }
    qDebug() << "Protobuf encoding successful, bytes written:" << stream.bytes_written;

    QByteArray result(reinterpret_cast<char*>(buffer), stream.bytes_written);
    qDebug() << "Created QByteArray with size:" << result.size();
    qDebug() << "Result data (hex):" << result.toHex();
    qDebug() << "ANC message serialized:" << result.size() << "bytes, ANC enabled:" << ancEnabled;
    qDebug() << "=== AncMessageHandler::serialize END (success) ===";

    return result;
}

bool AncMessageHandler::deserialize(const QByteArray& data, QVariantMap& parameters) {
    if (data.isEmpty()) {
        qWarning() << "Empty data for ANC message deserialization";
        return false;
    }

    MSG_AncSwitch msg = MSG_AncSwitch_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()),
        data.size()
    );

    if (!pb_decode(&stream, MSG_AncSwitch_fields, &msg)) {
        qWarning() << "Failed to decode ANC message:" << PB_GET_ERROR(&stream);
        return false;
    }

    // 解析ANC状态 (注意：ANC_OFF消息中true表示关闭ANC)
    bool ancEnabled = !msg.anc_off; // 反转逻辑，anc_off=true表示关闭
    parameters["anc.enabled"] = ancEnabled;

    qDebug() << "ANC message deserialized: ANC enabled:" << ancEnabled;
    return true;
}

bool AncMessageHandler::validateParameters(const QVariantMap& parameters) const {
    // 检查必需的参数
    if (!parameters.contains("anc.enabled")) {
        qWarning() << "Missing required parameter: anc.enabled";
        return false;
    }

    // 检查参数类型
    QVariant enabledValue = parameters.value("anc.enabled");
    if (!enabledValue.canConvert<bool>()) {
        qWarning() << "Invalid type for anc.enabled, expected bool, got:" << enabledValue.typeName();
        return false;
    }

    return true;
}

} // namespace Protocol
