#ifndef IMESSAGE_HANDLER_H
#define IMESSAGE_HANDLER_H

#include <QByteArray>
#include <QVariantMap>
#include <QString>
#include "message_types.h"

namespace Protocol {

/**
 * @brief 消息处理器接口
 *
 * 定义了消息序列化和反序列化的统一接口
 * 每种消息类型都应该有对应的实现类
 */
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    /**
     * @brief 序列化参数到字节数组
     * @param parameters 参数映射
     * @return 序列化后的字节数组，失败返回空数组
     */
    virtual QByteArray serialize(const QVariantMap& parameters) = 0;

    /**
     * @brief 从字节数组反序列化参数
     * @param data 字节数组
     * @param parameters 输出参数映射
     * @return 成功返回true，失败返回false
     */
    virtual bool deserialize(const QByteArray& data, QVariantMap& parameters) = 0;

    /**
     * @brief 获取处理器支持的消息类型
     * @return 消息类型
     */
    virtual MessageType getMessageType() const = 0;

    /**
     * @brief 验证参数有效性
     * @param parameters 参数映射
     * @return 有效返回true，无效返回false
     */
    virtual bool validateParameters(const QVariantMap& parameters) const = 0;

    /**
     * @brief 获取处理器描述信息
     * @return 描述字符串
     */
    virtual QString getDescription() const = 0;
};

} // namespace Protocol

#endif // IMESSAGE_HANDLER_H
