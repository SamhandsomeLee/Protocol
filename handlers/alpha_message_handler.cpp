#include "alpha_message_handler.h"
#include <QDebug>

namespace Protocol {

QByteArray AlphaMessageHandler::serialize(const QVariantMap& parameters) {
    if (!validateParameters(parameters)) {
        qWarning() << "Invalid parameters for Alpha message";
        return QByteArray();
    }

    MSG_Alpha msg = MSG_Alpha_init_zero;

    // 设置Alpha值 - 映射到alpha1字段作为主要alpha值
    float alphaValue = parameters.value("processing.alpha", 0.5f).toFloat();
    msg.alpha1 = static_cast<uint32_t>(alphaValue * 1000); // 转换为整数，假设乘以1000保持精度

    // 如果有其他alpha相关参数，可以映射到其他字段
    if (parameters.contains("processing.alpha2")) {
        msg.alpha2 = static_cast<uint32_t>(parameters.value("processing.alpha2").toFloat() * 1000);
    }
    if (parameters.contains("processing.alpha3")) {
        msg.alpha3 = static_cast<uint32_t>(parameters.value("processing.alpha3").toFloat() * 1000);
    }
    if (parameters.contains("processing.alpha4")) {
        msg.alpha4 = static_cast<uint32_t>(parameters.value("processing.alpha4").toFloat() * 1000);
    }
    if (parameters.contains("processing.alpha5")) {
        msg.alpha5 = static_cast<uint32_t>(parameters.value("processing.alpha5").toFloat() * 1000);
    }

    // 序列化消息
    uint8_t buffer[MAX_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, MSG_Alpha_fields, &msg)) {
        qWarning() << "Failed to encode Alpha message:" << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    QByteArray result(reinterpret_cast<char*>(buffer), stream.bytes_written);
    qDebug() << "Alpha message serialized:" << result.size() << "bytes, alpha:" << alphaValue;

    return result;
}

bool AlphaMessageHandler::deserialize(const QByteArray& data, QVariantMap& parameters) {
    if (data.isEmpty()) {
        qWarning() << "Empty data for Alpha message deserialization";
        return false;
    }

    MSG_Alpha msg = MSG_Alpha_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()),
        data.size()
    );

    if (!pb_decode(&stream, MSG_Alpha_fields, &msg)) {
        qWarning() << "Failed to decode Alpha message:" << PB_GET_ERROR(&stream);
        return false;
    }

    // 解析Alpha参数 - 从alpha1字段获取主要alpha值
    parameters["processing.alpha"] = static_cast<float>(msg.alpha1) / 1000.0f; // 转换回浮点数

    // 解析其他alpha字段
    if (msg.alpha2 > 0) {
        parameters["processing.alpha2"] = static_cast<float>(msg.alpha2) / 1000.0f;
    }
    if (msg.alpha3 > 0) {
        parameters["processing.alpha3"] = static_cast<float>(msg.alpha3) / 1000.0f;
    }
    if (msg.alpha4 > 0) {
        parameters["processing.alpha4"] = static_cast<float>(msg.alpha4) / 1000.0f;
    }
    if (msg.alpha5 > 0) {
        parameters["processing.alpha5"] = static_cast<float>(msg.alpha5) / 1000.0f;
    }

    qDebug() << "Alpha message deserialized: alpha1:" << msg.alpha1;
    return true;
}

bool AlphaMessageHandler::validateParameters(const QVariantMap& parameters) const {
    // 检查必需的参数
    if (!parameters.contains("processing.alpha")) {
        qWarning() << "Missing required parameter: processing.alpha";
        return false;
    }

    // 检查Alpha值的类型和范围
    QVariant alphaVariant = parameters.value("processing.alpha");
    if (!alphaVariant.canConvert<float>()) {
        qWarning() << "Invalid type for processing.alpha, expected float, got:" << alphaVariant.typeName();
        return false;
    }

    float alphaValue = alphaVariant.toFloat();
    if (alphaValue < MIN_ALPHA_VALUE || alphaValue > MAX_ALPHA_VALUE) {
        qWarning() << "Alpha value out of range [" << MIN_ALPHA_VALUE << "," << MAX_ALPHA_VALUE << "], got:" << alphaValue;
        return false;
    }

    // 验证其他可选alpha参数
    QStringList alphaParams = {"processing.alpha2", "processing.alpha3", "processing.alpha4", "processing.alpha5"};
    for (const QString& param : alphaParams) {
        if (parameters.contains(param)) {
            QVariant alphaVariant = parameters.value(param);
            if (!alphaVariant.canConvert<float>()) {
                qWarning() << "Invalid type for" << param << ", expected float, got:" << alphaVariant.typeName();
                return false;
            }
            float alphaValue = alphaVariant.toFloat();
            if (alphaValue < MIN_ALPHA_VALUE || alphaValue > MAX_ALPHA_VALUE) {
                qWarning() << param << "value out of range [" << MIN_ALPHA_VALUE << "," << MAX_ALPHA_VALUE << "], got:" << alphaValue;
                return false;
            }
        }
    }

    return true;
}

} // namespace Protocol
