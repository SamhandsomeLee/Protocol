#include "alpha_message_handler.h"
#include <QDebug>

namespace Protocol {

QByteArray AlphaMessageHandler::serialize(const QVariantMap& parameters) {
    if (!validateParameters(parameters)) {
        qWarning() << "Invalid parameters for Alpha message";
        return QByteArray();
    }

    MSG_Alpha msg = MSG_Alpha_init_zero;

    // 设置Alpha值
    float alphaValue = parameters.value("processing.alpha", 0.5f).toFloat();
    msg.alpha_value = alphaValue;

    // 如果有其他相关参数，也可以在这里设置
    if (parameters.contains("processing.beta")) {
        msg.has_beta_value = true;
        msg.beta_value = parameters.value("processing.beta").toFloat();
    }

    if (parameters.contains("processing.gamma")) {
        msg.has_gamma_value = true;
        msg.gamma_value = parameters.value("processing.gamma").toFloat();
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

    // 解析Alpha参数
    parameters["processing.alpha"] = msg.alpha_value;

    // 解析可选参数
    if (msg.has_beta_value) {
        parameters["processing.beta"] = msg.beta_value;
    }

    if (msg.has_gamma_value) {
        parameters["processing.gamma"] = msg.gamma_value;
    }

    qDebug() << "Alpha message deserialized: alpha:" << msg.alpha_value;
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

    // 验证可选参数
    if (parameters.contains("processing.beta")) {
        QVariant betaVariant = parameters.value("processing.beta");
        if (!betaVariant.canConvert<float>()) {
            qWarning() << "Invalid type for processing.beta, expected float, got:" << betaVariant.typeName();
            return false;
        }
    }

    if (parameters.contains("processing.gamma")) {
        QVariant gammaVariant = parameters.value("processing.gamma");
        if (!gammaVariant.canConvert<float>()) {
            qWarning() << "Invalid type for processing.gamma, expected float, got:" << gammaVariant.typeName();
            return false;
        }
    }

    return true;
}

} // namespace Protocol
