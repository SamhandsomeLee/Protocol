#ifndef PROTOCOL_PACKAGER_H
#define PROTOCOL_PACKAGER_H

#include <QByteArray>
#include "../core/message_types.h"

namespace Protocol {

/**
 * @brief 协议封装器
 *
 * 负责将具体消息数据封装成完整的MsgRequestResponse格式
 * 包含ProtoID、FunCode和payload字段
 */
class ProtocolPackager {
public:
    ProtocolPackager() = default;
    ~ProtocolPackager() = default;

    /**
     * @brief 将消息数据封装成MsgRequestResponse格式
     * @param messageType 消息类型（用于确定ProtoID）
     * @param functionCode 功能码（REQUEST或RESPONSE）
     * @param payloadData 具体消息的序列化数据
     * @return 完整的MsgRequestResponse字节数组
     */
    QByteArray packageMessage(MessageType messageType, FunctionCode functionCode, const QByteArray& payloadData);

    /**
     * @brief 从MsgRequestResponse格式中解包消息
     * @param data 完整的MsgRequestResponse数据
     * @param messageType 输出：解析出的消息类型
     * @param functionCode 输出：解析出的功能码
     * @param payloadData 输出：具体消息数据
     * @return 成功返回true，失败返回false
     */
    bool unpackageMessage(const QByteArray& data, MessageType& messageType, FunctionCode& functionCode, QByteArray& payloadData);

private:
    /**
     * @brief 编码varint格式的整数
     * @param value 要编码的值
     * @return 编码后的字节数组
     */
    QByteArray encodeVarint(quint32 value);

    /**
     * @brief 解码varint格式的整数
     * @param data 数据
     * @param offset 偏移量（会被更新）
     * @param value 输出：解码的值
     * @return 成功返回true，失败返回false
     */
    bool decodeVarint(const QByteArray& data, int& offset, quint32& value);

    /**
     * @brief 编码带长度前缀的字节数组（protobuf的bytes类型）
     * @param data 要编码的数据
     * @return 编码后的字节数组（长度+数据）
     */
    QByteArray encodeLengthPrefixed(const QByteArray& data);

    /**
     * @brief 解码带长度前缀的字节数组
     * @param data 源数据
     * @param offset 偏移量（会被更新）
     * @param result 输出：解码的数据
     * @return 成功返回true，失败返回false
     */
    bool decodeLengthPrefixed(const QByteArray& data, int& offset, QByteArray& result);
};

} // namespace Protocol

#endif // PROTOCOL_PACKAGER_H
