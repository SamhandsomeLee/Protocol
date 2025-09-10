#include "realtime_data_handler.h"
#include "messages/ERNC_praram.pb.h"
#include <QDebug>
#include <QDataStream>
#include <QBuffer>

namespace Protocol {

QByteArray RealtimeDataHandler::serialize(const QVariantMap& parameters) {
    if (!validateParameters(parameters)) {
        qWarning() << "RealtimeDataHandler: Invalid parameters for serialization";
        return QByteArray();
    }

    QByteArray result;
    QBuffer buffer(&result);
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    // 序列化实时数据参数
    if (parameters.contains("channel_count")) {
        int channelCount = parameters["channel_count"].toInt();
        stream << static_cast<quint32>(channelCount);
    }

    if (parameters.contains("sample_rate")) {
        int sampleRate = parameters["sample_rate"].toInt();
        stream << static_cast<quint32>(sampleRate);
    }

    if (parameters.contains("data_format")) {
        int dataFormat = parameters["data_format"].toInt();
        stream << static_cast<quint32>(dataFormat);
    }

    // 序列化通道数据
    if (parameters.contains("channel_data")) {
        QVariantList channelData = parameters["channel_data"].toList();
        stream << static_cast<quint32>(channelData.size());

        for (const auto& data : channelData) {
            QVariantMap channelInfo = data.toMap();
            if (channelInfo.contains("channel_id")) {
                stream << static_cast<quint32>(channelInfo["channel_id"].toInt());
            }
            if (channelInfo.contains("amplitude")) {
                stream << channelInfo["amplitude"].toFloat();
            }
            if (channelInfo.contains("frequency")) {
                stream << channelInfo["frequency"].toFloat();
            }
        }
    }

    // 序列化时间戳
    if (parameters.contains("timestamp")) {
        quint64 timestamp = parameters["timestamp"].toULongLong();
        stream << timestamp;
    }

    buffer.close();
    return result;
}

bool RealtimeDataHandler::deserialize(const QByteArray& data, QVariantMap& parameters) {
    if (data.isEmpty() || data.size() > MAX_BUFFER_SIZE) {
        qWarning() << "RealtimeDataHandler: Invalid data size for deserialization";
        return false;
    }

    QBuffer buffer(const_cast<QByteArray*>(&data));
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    try {
        // 反序列化基本参数
        quint32 channelCount = 0;
        if (stream.atEnd()) return false;
        stream >> channelCount;
        parameters["channel_count"] = static_cast<int>(channelCount);

        quint32 sampleRate = 0;
        if (stream.atEnd()) return false;
        stream >> sampleRate;
        parameters["sample_rate"] = static_cast<int>(sampleRate);

        quint32 dataFormat = 0;
        if (stream.atEnd()) return false;
        stream >> dataFormat;
        parameters["data_format"] = static_cast<int>(dataFormat);

        // 反序列化通道数据
        quint32 dataCount = 0;
        if (stream.atEnd()) return false;
        stream >> dataCount;

        if (dataCount > MAX_CHANNELS) {
            qWarning() << "RealtimeDataHandler: Too many channels:" << dataCount;
            return false;
        }

        QVariantList channelDataList;
        for (quint32 i = 0; i < dataCount && !stream.atEnd(); ++i) {
            QVariantMap channelInfo;

            quint32 channelId = 0;
            stream >> channelId;
            channelInfo["channel_id"] = static_cast<int>(channelId);

            float amplitude = 0.0f;
            stream >> amplitude;
            channelInfo["amplitude"] = amplitude;

            float frequency = 0.0f;
            stream >> frequency;
            channelInfo["frequency"] = frequency;

            channelDataList.append(channelInfo);
        }
        parameters["channel_data"] = channelDataList;

        // 反序列化时间戳
        if (!stream.atEnd()) {
            quint64 timestamp = 0;
            stream >> timestamp;
            parameters["timestamp"] = timestamp;
        }

    } catch (...) {
        qWarning() << "RealtimeDataHandler: Exception during deserialization";
        return false;
    }

    buffer.close();
    return validateParameters(parameters);
}

bool RealtimeDataHandler::validateParameters(const QVariantMap& parameters) const {
    // 检查基本参数
    if (!parameters.contains("channel_count") ||
        !parameters.contains("sample_rate") ||
        !parameters.contains("data_format")) {
        return false;
    }

    int channelCount = parameters["channel_count"].toInt();
    if (channelCount < 0 || channelCount > MAX_CHANNELS) {
        return false;
    }

    int sampleRate = parameters["sample_rate"].toInt();
    if (sampleRate <= 0 || sampleRate > 48000) {
        return false;
    }

    int dataFormat = parameters["data_format"].toInt();
    if (dataFormat < 0 || dataFormat > 3) {
        return false;
    }

    // 验证通道数据
    if (parameters.contains("channel_data")) {
        if (!validateChannelData(parameters)) {
            return false;
        }
    }

    return true;
}

bool RealtimeDataHandler::validateChannelData(const QVariantMap& parameters) const {
    QVariantList channelData = parameters["channel_data"].toList();

    if (channelData.size() > MAX_CHANNELS) {
        return false;
    }

    for (const auto& data : channelData) {
        QVariantMap channelInfo = data.toMap();

        if (!channelInfo.contains("channel_id") ||
            !channelInfo.contains("amplitude")) {
            return false;
        }

        int channelId = channelInfo["channel_id"].toInt();
        if (channelId < 0 || channelId >= MAX_CHANNELS) {
            return false;
        }

        float amplitude = channelInfo["amplitude"].toFloat();
        if (amplitude < -100.0f || amplitude > 100.0f) {
            return false;
        }
    }

    return true;
}

bool RealtimeDataHandler::validateAmplitudeData(const QVariantMap& parameters) const {
    if (!parameters.contains("amplitude_data")) {
        return true; // 可选参数
    }

    QVariantList amplitudeData = parameters["amplitude_data"].toList();

    for (const auto& data : amplitudeData) {
        float amplitude = data.toFloat();
        if (amplitude < -120.0f || amplitude > 20.0f) {
            return false;
        }
    }

    return true;
}

} // namespace Protocol

