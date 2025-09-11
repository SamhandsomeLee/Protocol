#include "protocol_packager.h"
#include "../core/message_types.h"
#include <QDebug>

namespace Protocol {

QByteArray ProtocolPackager::packageMessage(MessageType messageType, FunctionCode functionCode, const QByteArray& payloadData) {
    QByteArray result;

    // 获取ProtoID
    int protoID = MessageTypeUtils::toProtoID(messageType);

    qDebug() << "Packaging message - Type:" << static_cast<int>(messageType)
             << "ProtoID:" << protoID
             << "FunCode:" << static_cast<int>(functionCode)
             << "Payload size:" << payloadData.size();

    // 构建MsgRequestResponse结构：
    // 字段1: ProtoID (tag=08, varint)
    result.append(0x08);  // tag for field 1 (ProtoID), wire type 0 (varint)
    result.append(encodeVarint(static_cast<quint32>(protoID)));

    // 字段2: FunCode (tag=10, varint)
    result.append(0x10);  // tag for field 2 (FunCode), wire type 0 (varint)
    result.append(encodeVarint(static_cast<quint32>(functionCode)));

    // 根据消息类型确定具体的oneof字段标签
    quint8 fieldTag;
    switch (messageType) {
        case MessageType::CHANNEL_NUMBER:
            fieldTag = 0x1A;  // tag for field 3 (msg_channel_number)
            break;
        case MessageType::CHANNEL_AMPLITUDE:
            fieldTag = 0x22;  // tag for field 4 (msg_channel_amplitude)
            break;
        case MessageType::CHANNEL_SWITCH:
            fieldTag = 0x2A;  // tag for field 5 (msg_channel_switch)
            break;
        case MessageType::CHECK_MOD:
            fieldTag = 0x32;  // tag for field 6 (msg_check_mod)
            break;
        case MessageType::ANC_SWITCH:
            fieldTag = 0x3A;  // tag for field 7 (msg_anc_switch)
            break;
        case MessageType::VEHICLE_STATE:
            fieldTag = 0x42;  // tag for field 8 (msg_vehicle_state)
            break;
        case MessageType::TRAN_FUNC_FLAG:
            fieldTag = 0x4A;  // tag for field 9 (msg_tran_func_flag)
            break;
        case MessageType::TRAN_FUNC_STATE:
            fieldTag = 0x52;  // tag for field 10 (msg_tran_func_state)
            break;
        case MessageType::FILTER_RANGES:
            fieldTag = 0x5A;  // tag for field 11 (msg_filter_ranges)
            break;
        case MessageType::SYSTEM_RANGES:
            fieldTag = 0x62;  // tag for field 12 (msg_system_ranges)
            break;
        case MessageType::ORDER_FLAG:
            fieldTag = 0x6A;  // tag for field 13 (msg_order_flag)
            break;
        case MessageType::ORDER2_PARAMS:
            fieldTag = 0x72;  // tag for field 14 (msg_order2_params)
            break;
        case MessageType::ORDER4_PARAMS:
            fieldTag = 0x7A;  // tag for field 15 (msg_order4_params)
            break;
        case MessageType::ORDER6_PARAMS:
            fieldTag = 0x82;  // tag for field 16 (msg_order6_params), need varint encoding
            break;
        case MessageType::ALPHA_PARAMS:
            fieldTag = 0x8A;  // tag for field 17 (msg_alpha_params), need varint encoding
            break;
        case MessageType::FREQ_DIVISION:
            fieldTag = 0x92;  // tag for field 18 (msg_freq_division), need varint encoding
            break;
        case MessageType::THRESHOLDS:
            fieldTag = 0x9A;  // tag for field 19 (msg_thresholds), need varint encoding
            break;
        default:
            qWarning() << "Unsupported message type for packaging:" << static_cast<int>(messageType);
            return QByteArray();
    }

    // 对于字段号大于15的，需要使用varint编码tag
    if (fieldTag >= 0x80) {
        // 字段16-19需要特殊处理tag编码
        switch (messageType) {
            case MessageType::ORDER6_PARAMS:
                result.append(0x82); result.append(0x01); // field 16, wire type 2
                break;
            case MessageType::ALPHA_PARAMS:
                result.append(0x8A); result.append(0x01); // field 17, wire type 2
                break;
            case MessageType::FREQ_DIVISION:
                result.append(0x92); result.append(0x01); // field 18, wire type 2
                break;
            case MessageType::THRESHOLDS:
                result.append(0x9A); result.append(0x01); // field 19, wire type 2
                break;
            default:
                break;
        }
    } else {
        result.append(fieldTag);
    }

    result.append(encodeLengthPrefixed(payloadData));

    qDebug() << "Packaged message size:" << result.size() << "bytes";
    qDebug() << "Packaged data:" << result.toHex(' ');

    return result;
}

bool ProtocolPackager::unpackageMessage(const QByteArray& data, MessageType& messageType, FunctionCode& functionCode, QByteArray& payloadData) {
    if (data.isEmpty()) {
        qWarning() << "Cannot unpackage empty data";
        return false;
    }

    int offset = 0;
    quint32 protoID = 0;
    quint32 funCode = 0;
    bool foundProtoID = false;
    bool foundFunCode = false;
    bool foundPayload = false;

    while (offset < data.size()) {
        // 读取tag
        quint32 tag;
        if (!decodeVarint(data, offset, tag)) {
            qWarning() << "Failed to decode tag at offset" << offset;
            return false;
        }

        int fieldNumber = tag >> 3;
        int wireType = tag & 0x07;

        switch (fieldNumber) {
        case 1: // ProtoID
            if (wireType != 0) { // varint
                qWarning() << "Invalid wire type for ProtoID field";
                return false;
            }
            if (!decodeVarint(data, offset, protoID)) {
                qWarning() << "Failed to decode ProtoID";
                return false;
            }
            foundProtoID = true;
            break;

        case 2: // FunCode
            if (wireType != 0) { // varint
                qWarning() << "Invalid wire type for FunCode field";
                return false;
            }
            if (!decodeVarint(data, offset, funCode)) {
                qWarning() << "Failed to decode FunCode";
                return false;
            }
            foundFunCode = true;
            break;

        case 3:  // msg_channel_number
        case 4:  // msg_channel_amplitude
        case 5:  // msg_channel_switch
        case 6:  // msg_check_mod
        case 7:  // msg_anc_switch
        case 8:  // msg_vehicle_state
        case 9:  // msg_tran_func_flag
        case 10: // msg_tran_func_state
        case 11: // msg_filter_ranges
        case 12: // msg_system_ranges
        case 13: // msg_order_flag
        case 14: // msg_order2_params
        case 15: // msg_order4_params
        case 16: // msg_order6_params
        case 17: // msg_alpha_params
        case 18: // msg_freq_division
        case 19: // msg_thresholds
            if (wireType != 2) { // length-delimited
                qWarning() << "Invalid wire type for payload field" << fieldNumber;
                return false;
            }
            if (!decodeLengthPrefixed(data, offset, payloadData)) {
                qWarning() << "Failed to decode payload for field" << fieldNumber;
                return false;
            }
            foundPayload = true;
            break;

        default:
            // 跳过未知字段
            qWarning() << "Skipping unknown field" << fieldNumber;
            if (wireType == 0) { // varint
                quint32 dummy;
                if (!decodeVarint(data, offset, dummy)) return false;
            } else if (wireType == 2) { // length-delimited
                QByteArray dummy;
                if (!decodeLengthPrefixed(data, offset, dummy)) return false;
            } else {
                qWarning() << "Unsupported wire type" << wireType;
                return false;
            }
            break;
        }
    }

    if (!foundProtoID || !foundFunCode || !foundPayload) {
        qWarning() << "Missing required fields - ProtoID:" << foundProtoID
                   << "FunCode:" << foundFunCode << "Payload:" << foundPayload;
        return false;
    }

    // 转换ProtoID到MessageType
    messageType = MessageTypeUtils::fromProtoID(static_cast<int>(protoID));
    functionCode = static_cast<FunctionCode>(funCode);

    qDebug() << "Unpackaged message - ProtoID:" << protoID
             << "Type:" << static_cast<int>(messageType)
             << "FunCode:" << funCode
             << "Payload size:" << payloadData.size();

    return true;
}

QByteArray ProtocolPackager::encodeVarint(quint32 value) {
    QByteArray result;
    while (value >= 0x80) {
        result.append(static_cast<char>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    result.append(static_cast<char>(value & 0x7F));
    return result;
}

bool ProtocolPackager::decodeVarint(const QByteArray& data, int& offset, quint32& value) {
    value = 0;
    int shift = 0;

    while (offset < data.size()) {
        quint8 byte = static_cast<quint8>(data[offset++]);
        value |= static_cast<quint32>(byte & 0x7F) << shift;

        if ((byte & 0x80) == 0) {
            return true; // 最后一个字节
        }

        shift += 7;
        if (shift >= 32) {
            qWarning() << "Varint too long";
            return false;
        }
    }

    qWarning() << "Incomplete varint";
    return false;
}

QByteArray ProtocolPackager::encodeLengthPrefixed(const QByteArray& data) {
    QByteArray result;
    result.append(encodeVarint(static_cast<quint32>(data.size())));
    result.append(data);
    return result;
}

bool ProtocolPackager::decodeLengthPrefixed(const QByteArray& data, int& offset, QByteArray& result) {
    quint32 length;
    if (!decodeVarint(data, offset, length)) {
        return false;
    }

    if (offset + static_cast<int>(length) > data.size()) {
        qWarning() << "Length-prefixed data exceeds buffer size";
        return false;
    }

    result = data.mid(offset, static_cast<int>(length));
    offset += static_cast<int>(length);
    return true;
}

} // namespace Protocol
