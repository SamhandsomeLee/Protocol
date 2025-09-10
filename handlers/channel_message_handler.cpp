#include "channel_message_handler.h"
#include <QLoggingCategory>
#include <QVariantList>

Q_LOGGING_CATEGORY(channelHandler, "protocol.handler.channel")

namespace Protocol {

ChannelMessageHandler::ChannelMessageHandler(ChannelMessageSubType subType)
    : subType_(subType)
{
    qCDebug(channelHandler) << "ChannelMessageHandler initialized with subtype:"
                            << static_cast<int>(subType);
}

QByteArray ChannelMessageHandler::serialize(const QVariantMap& parameters)
{
    switch (subType_) {
        case ChannelMessageSubType::CHANNEL_NUMBER:
            return serializeChannelNumber(parameters);
        case ChannelMessageSubType::CHANNEL_AMPLITUDE:
            return serializeChannelAmplitude(parameters);
        case ChannelMessageSubType::CHANNEL_SWITCH:
            return serializeChannelSwitch(parameters);
        default:
            qCWarning(channelHandler) << "Unknown channel message subtype";
            return QByteArray();
    }
}

bool ChannelMessageHandler::deserialize(const QByteArray& data, QVariantMap& parameters)
{
    switch (subType_) {
        case ChannelMessageSubType::CHANNEL_NUMBER:
            return deserializeChannelNumber(data, parameters);
        case ChannelMessageSubType::CHANNEL_AMPLITUDE:
            return deserializeChannelAmplitude(data, parameters);
        case ChannelMessageSubType::CHANNEL_SWITCH:
            return deserializeChannelSwitch(data, parameters);
        default:
            qCWarning(channelHandler) << "Unknown channel message subtype";
            return false;
    }
}

MessageType ChannelMessageHandler::getMessageType() const
{
    switch (subType_) {
        case ChannelMessageSubType::CHANNEL_NUMBER:
            return MessageType::CHANNEL_NUMBER;
        case ChannelMessageSubType::CHANNEL_AMPLITUDE:
            return MessageType::CHANNEL_AMPLITUDE;
        case ChannelMessageSubType::CHANNEL_SWITCH:
            return MessageType::CHANNEL_SWITCH;
        default:
            return MessageType::CHANNEL_NUMBER; // 默认返回通道数量类型
    }
}

QString ChannelMessageHandler::getDescription() const
{
    switch (subType_) {
        case ChannelMessageSubType::CHANNEL_NUMBER:
            return "Channel number configuration message handler";
        case ChannelMessageSubType::CHANNEL_AMPLITUDE:
            return "Channel amplitude configuration message handler";
        case ChannelMessageSubType::CHANNEL_SWITCH:
            return "Channel switch configuration message handler";
        default:
            return "Unknown channel message handler";
    }
}

bool ChannelMessageHandler::validateParameters(const QVariantMap& parameters) const
{
    switch (subType_) {
        case ChannelMessageSubType::CHANNEL_NUMBER:
            // 验证通道数量参数
            if (parameters.contains("refer_num") || parameters.contains("err_num") || parameters.contains("spk_num")) {
                if (parameters.contains("refer_num")) {
                    uint32_t count = parameters["refer_num"].toUInt();
                    if (!isValidChannelCount(count)) return false;
                }
                if (parameters.contains("err_num")) {
                    uint32_t count = parameters["err_num"].toUInt();
                    if (!isValidChannelCount(count)) return false;
                }
                if (parameters.contains("spk_num")) {
                    uint32_t count = parameters["spk_num"].toUInt();
                    if (!isValidChannelCount(count)) return false;
                }
                return true;
            }
            break;
        case ChannelMessageSubType::CHANNEL_AMPLITUDE:
            if (parameters.contains("input_amplitude")) {
                QVariantList amplitudes = parameters["input_amplitude"].toList();
                if (!isValidAmplitudeArray(amplitudes)) return false;
            }
            if (parameters.contains("output_amplitude")) {
                uint32_t amplitude = parameters["output_amplitude"].toUInt();
                if (amplitude > MAX_AMPLITUDE_VALUE) return false;
            }
            return true;
        case ChannelMessageSubType::CHANNEL_SWITCH:
            if (parameters.contains("f_input_poi")) {
                QVariantList switches = parameters["f_input_poi"].toList();
                if (!isValidSwitchArray(switches, MAX_INPUT_SWITCH_COUNT)) {
                    return false;
                }
            }
            if (parameters.contains("f_output_poi")) {
                QVariantList switches = parameters["f_output_poi"].toList();
                if (!isValidSwitchArray(switches, MAX_OUTPUT_SWITCH_COUNT)) {
                    return false;
                }
            }
            return true;
    }
    return true;
}

QByteArray ChannelMessageHandler::serializeChannelNumber(const QVariantMap& parameters)
{
    MSG_ChannelNumber channelNumber = MSG_ChannelNumber_init_zero;

    if (parameters.contains("refer_num")) {
        channelNumber.ReferNum = parameters["refer_num"].toUInt();
    }

    if (parameters.contains("err_num")) {
        channelNumber.ErrNum = parameters["err_num"].toUInt();
    }

    if (parameters.contains("spk_num")) {
        channelNumber.SpkNum = parameters["spk_num"].toUInt();
    }

    // 序列化消息
    uint8_t buffer[MAX_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool status = pb_encode(&stream, MSG_ChannelNumber_fields, &channelNumber);
    if (!status) {
        qCWarning(channelHandler) << "Failed to encode channel number message:"
                                  << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    QByteArray result(reinterpret_cast<const char*>(buffer),
                      static_cast<int>(stream.bytes_written));

    qCDebug(channelHandler) << "Serialized channel number message, size:" << result.size();
    return result;
}

QByteArray ChannelMessageHandler::serializeChannelAmplitude(const QVariantMap& parameters)
{
    MSG_ChannelAmplitude channelAmplitude = MSG_ChannelAmplitude_init_zero;

    if (parameters.contains("input_amplitude")) {
        QVariantList amplitudes = parameters["input_amplitude"].toList();
        int amplitudeCount = qMin(amplitudes.size(), 13); // 最多13个输入幅值
        for (int i = 0; i < amplitudeCount; ++i) {
            channelAmplitude.InputAmplitude[i] = amplitudes[i].toUInt();
        }
    }

    if (parameters.contains("output_amplitude")) {
        channelAmplitude.OutputAmplitude = parameters["output_amplitude"].toUInt();
    }

    // 序列化消息
    uint8_t buffer[MAX_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool status = pb_encode(&stream, MSG_ChannelAmplitude_fields, &channelAmplitude);
    if (!status) {
        qCWarning(channelHandler) << "Failed to encode channel amplitude message:"
                                  << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    QByteArray result(reinterpret_cast<const char*>(buffer),
                      static_cast<int>(stream.bytes_written));

    qCDebug(channelHandler) << "Serialized channel amplitude message, size:" << result.size();
    return result;
}

QByteArray ChannelMessageHandler::serializeChannelSwitch(const QVariantMap& parameters)
{
    MSG_ChannelSwitch channelSwitch = MSG_ChannelSwitch_init_zero;

    if (parameters.contains("f_input_poi")) {
        QVariantList switches = parameters["f_input_poi"].toList();
        int inputSwitchCount = qMin(switches.size(), 20); // 最多20个输入开关
        for (int i = 0; i < inputSwitchCount; ++i) {
            channelSwitch.FInputPoi[i] = switches[i].toUInt();
        }
    }

    if (parameters.contains("f_output_poi")) {
        QVariantList switches = parameters["f_output_poi"].toList();
        int outputSwitchCount = qMin(switches.size(), 8); // 最多8个输出开关
        for (int i = 0; i < outputSwitchCount; ++i) {
            channelSwitch.FOutputPoi[i] = switches[i].toUInt();
        }
    }

    // 序列化消息
    uint8_t buffer[MAX_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool status = pb_encode(&stream, MSG_ChannelSwitch_fields, &channelSwitch);
    if (!status) {
        qCWarning(channelHandler) << "Failed to encode channel switch message:"
                                  << PB_GET_ERROR(&stream);
        return QByteArray();
    }

    QByteArray result(reinterpret_cast<const char*>(buffer),
                      static_cast<int>(stream.bytes_written));

    qCDebug(channelHandler) << "Serialized channel switch message, size:" << result.size();
    return result;
}

bool ChannelMessageHandler::deserializeChannelNumber(const QByteArray& data, QVariantMap& parameters)
{
    MSG_ChannelNumber channelNumber = MSG_ChannelNumber_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()),
        static_cast<size_t>(data.size())
    );

    bool status = pb_decode(&stream, MSG_ChannelNumber_fields, &channelNumber);
    if (!status) {
        qCWarning(channelHandler) << "Failed to decode channel number message:"
                                  << PB_GET_ERROR(&stream);
        return false;
    }

    parameters["refer_num"] = channelNumber.ReferNum;
    parameters["err_num"] = channelNumber.ErrNum;
    parameters["spk_num"] = channelNumber.SpkNum;

    qCDebug(channelHandler) << "Successfully deserialized channel number message";
    return true;
}

bool ChannelMessageHandler::deserializeChannelAmplitude(const QByteArray& data, QVariantMap& parameters)
{
    MSG_ChannelAmplitude channelAmplitude = MSG_ChannelAmplitude_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()),
        static_cast<size_t>(data.size())
    );

    bool status = pb_decode(&stream, MSG_ChannelAmplitude_fields, &channelAmplitude);
    if (!status) {
        qCWarning(channelHandler) << "Failed to decode channel amplitude message:"
                                  << PB_GET_ERROR(&stream);
        return false;
    }

    // 提取输入幅值 (固定13个)
    QVariantList amplitudes;
    for (size_t i = 0; i < 13; ++i) {
        amplitudes.append(channelAmplitude.InputAmplitude[i]);
    }
    parameters["input_amplitude"] = amplitudes;

    parameters["output_amplitude"] = channelAmplitude.OutputAmplitude;

    qCDebug(channelHandler) << "Successfully deserialized channel amplitude message";
    return true;
}

bool ChannelMessageHandler::deserializeChannelSwitch(const QByteArray& data, QVariantMap& parameters)
{
    MSG_ChannelSwitch channelSwitch = MSG_ChannelSwitch_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(data.constData()),
        static_cast<size_t>(data.size())
    );

    bool status = pb_decode(&stream, MSG_ChannelSwitch_fields, &channelSwitch);
    if (!status) {
        qCWarning(channelHandler) << "Failed to decode channel switch message:"
                                  << PB_GET_ERROR(&stream);
        return false;
    }

    // 提取输入开关 (固定20个)
    QVariantList inputSwitches;
    for (size_t i = 0; i < 20; ++i) {
        inputSwitches.append(channelSwitch.FInputPoi[i]);
    }
    parameters["f_input_poi"] = inputSwitches;

    // 提取输出开关 (固定8个)
    QVariantList outputSwitches;
    for (size_t i = 0; i < 8; ++i) {
        outputSwitches.append(channelSwitch.FOutputPoi[i]);
    }
    parameters["f_output_poi"] = outputSwitches;

    qCDebug(channelHandler) << "Successfully deserialized channel switch message";
    return true;
}

bool ChannelMessageHandler::isValidChannelCount(uint32_t count) const
{
    return count > 0 && count <= MAX_CHANNEL_COUNT;
}

bool ChannelMessageHandler::isValidAmplitudeArray(const QVariantList& amplitudes) const
{
    if (amplitudes.size() > MAX_INPUT_AMPLITUDE_COUNT) {
        return false;
    }

    for (const QVariant& amplitude : amplitudes) {
        if (!amplitude.canConvert<uint32_t>()) {
            return false;
        }
        uint32_t value = amplitude.toUInt();
        if (value > MAX_AMPLITUDE_VALUE) {
            return false;
        }
    }

    return true;
}

bool ChannelMessageHandler::isValidSwitchArray(const QVariantList& switches, uint32_t maxCount) const
{
    if (switches.size() > maxCount) {
        return false;
    }

    for (const QVariant& switchValue : switches) {
        if (!switchValue.canConvert<uint32_t>()) {
            return false;
        }
        // 开关值应该是合理范围内的整数
        uint32_t value = switchValue.toUInt();
        if (value > 1000) {  // 合理的上限
            return false;
        }
    }

    return true;
}

} // namespace Protocol
