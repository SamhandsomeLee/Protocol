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

    // 初始化消息，默认保持所有字段未设置状态
    // 仅设置用户明确提供的参数对应的字段

    // 设置ANC开关状态 (注意：消息中true表示关闭，false表示开启)
    if (parameters.contains("anc.enabled")) {
        QVariant ancEnabledVariant = parameters.value("anc.enabled");
        qDebug() << "Raw anc.enabled value:" << ancEnabledVariant;
        qDebug() << "anc.enabled variant type:" << ancEnabledVariant.typeName();
        bool ancEnabled = ancEnabledVariant.toBool();
        qDebug() << "Converted anc.enabled to bool:" << ancEnabled;
        msg.anc_off = !ancEnabled; // 设置ANC_OFF字段（true=关闭，false=开启）
        qDebug() << "MSG_AncSwitch.anc_off set to:" << msg.anc_off << "(inverted from ancEnabled:" << ancEnabled << ")";
    } else {
        qDebug() << "anc.enabled not provided, field not modified";
    }

    // 设置ENC开关状态
    if (parameters.contains("enc.enabled")) {
        QVariant encEnabledVariant = parameters.value("enc.enabled");
        qDebug() << "Raw enc.enabled value:" << encEnabledVariant;
        bool encEnabled = encEnabledVariant.toBool();
        msg.enc_off = !encEnabled; // 设置ENC_OFF字段（true=关闭，false=开启）
        qDebug() << "MSG_AncSwitch.enc_off set to:" << msg.enc_off << "(inverted from encEnabled:" << encEnabled << ")";
    } else {
        qDebug() << "enc.enabled not provided, field not modified";
    }

    // 设置RNC开关状态
    if (parameters.contains("rnc.enabled")) {
        QVariant rncEnabledVariant = parameters.value("rnc.enabled");
        qDebug() << "Raw rnc.enabled value:" << rncEnabledVariant;
        bool rncEnabled = rncEnabledVariant.toBool();
        msg.rnc_off = !rncEnabled; // 设置RNC_OFF字段（true=关闭，false=开启）
        qDebug() << "MSG_AncSwitch.rnc_off set to:" << msg.rnc_off << "(inverted from rncEnabled:" << rncEnabled << ")";
    } else {
        qDebug() << "rnc.enabled not provided, field not modified";
    }

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
    qDebug() << "ANC message serialized:" << result.size() << "bytes, ANC state:"
             << (parameters.contains("anc.enabled") ?
                 (parameters.value("anc.enabled").toBool() ? "enabled" : "disabled") : "unchanged")
             << ", ENC state:"
             << (parameters.contains("enc.enabled") ?
                 (parameters.value("enc.enabled").toBool() ? "enabled" : "disabled") : "unchanged")
             << ", RNC state:"
             << (parameters.contains("rnc.enabled") ?
                 (parameters.value("rnc.enabled").toBool() ? "enabled" : "disabled") : "unchanged");
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

    // 解析ANC状态 (注意：消息中true表示关闭，false表示开启)
    bool ancEnabled = !msg.anc_off; // 反转逻辑，anc_off=true表示关闭
    parameters["anc.enabled"] = ancEnabled;

    // 解析ENC状态
    bool encEnabled = !msg.enc_off; // 反转逻辑，enc_off=true表示关闭
    parameters["enc.enabled"] = encEnabled;

    // 解析RNC状态
    bool rncEnabled = !msg.rnc_off; // 反转逻辑，rnc_off=true表示关闭
    parameters["rnc.enabled"] = rncEnabled;

    qDebug() << "ANC message deserialized: ANC enabled:" << ancEnabled
             << ", ENC enabled:" << encEnabled
             << ", RNC enabled:" << rncEnabled;
    return true;
}

bool AncMessageHandler::validateParameters(const QVariantMap& parameters) const {
    // 要求至少提供一个参数
    if (!parameters.contains("anc.enabled") &&
        !parameters.contains("enc.enabled") &&
        !parameters.contains("rnc.enabled")) {
        qWarning() << "At least one parameter must be provided (anc.enabled, enc.enabled, or rnc.enabled)";
        return false;
    }

    // 检查ANC参数 (可选参数，仅在提供时检查类型)
    if (parameters.contains("anc.enabled")) {
        QVariant ancValue = parameters.value("anc.enabled");
        if (!ancValue.canConvert<bool>()) {
            qWarning() << "Invalid type for anc.enabled, expected bool, got:" << ancValue.typeName();
            return false;
        }
    }

    // 检查ENC参数 (可选参数，仅在提供时检查类型)
    if (parameters.contains("enc.enabled")) {
        QVariant encValue = parameters.value("enc.enabled");
        if (!encValue.canConvert<bool>()) {
            qWarning() << "Invalid type for enc.enabled, expected bool, got:" << encValue.typeName();
            return false;
        }
    }

    // 检查RNC参数 (可选参数，仅在提供时检查类型)
    if (parameters.contains("rnc.enabled")) {
        QVariant rncValue = parameters.value("rnc.enabled");
        if (!rncValue.canConvert<bool>()) {
            qWarning() << "Invalid type for rnc.enabled, expected bool, got:" << rncValue.typeName();
            return false;
        }
    }

    return true;
}

} // namespace Protocol
