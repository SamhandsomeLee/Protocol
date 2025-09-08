#ifndef MESSAGE_SERIALIZER_H
#define MESSAGE_SERIALIZER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <memory>
#include "../core/message_types.h"
#include "message_factory.h"

namespace Protocol {

/**
 * @brief 消息序列化器
 *
 * 统一的消息序列化和反序列化接口
 * 使用MessageFactory管理具体的处理器实现
 */
class MessageSerializer : public QObject {
    Q_OBJECT

public:
    explicit MessageSerializer(QObject* parent = nullptr);
    ~MessageSerializer() override = default;

    /**
     * @brief 序列化参数到字节数组
     * @param messageType 消息类型
     * @param parameters 参数映射
     * @return 序列化后的字节数组，失败返回空数组
     */
    QByteArray serialize(MessageType messageType, const QVariantMap& parameters);

    /**
     * @brief 从字节数组反序列化参数
     * @param messageType 消息类型
     * @param data 字节数组
     * @param parameters 输出参数映射
     * @return 成功返回true，失败返回false
     */
    bool deserialize(MessageType messageType, const QByteArray& data, QVariantMap& parameters);

    /**
     * @brief 检查是否支持某种消息类型
     * @param messageType 消息类型
     * @return 支持返回true，不支持返回false
     */
    bool isMessageTypeSupported(MessageType messageType) const;

    /**
     * @brief 获取所有支持的消息类型
     * @return 消息类型列表
     */
    QList<MessageType> getSupportedMessageTypes() const;

    /**
     * @brief 验证参数有效性
     * @param messageType 消息类型
     * @param parameters 参数映射
     * @return 有效返回true，无效返回false
     */
    bool validateParameters(MessageType messageType, const QVariantMap& parameters) const;

    /**
     * @brief 获取消息类型描述信息
     * @param messageType 消息类型
     * @return 描述信息
     */
    QString getMessageTypeDescription(MessageType messageType) const;

    /**
     * @brief 注册自定义消息处理器
     * @param messageType 消息类型
     * @param handler 处理器实例
     * @return 成功返回true，失败返回false
     */
    bool registerCustomHandler(MessageType messageType, std::unique_ptr<IMessageHandler> handler);

signals:
    /**
     * @brief 序列化完成信号
     * @param messageType 消息类型
     * @param success 是否成功
     * @param dataSize 数据大小（如果成功）
     */
    void serializationCompleted(MessageType messageType, bool success, int dataSize);

    /**
     * @brief 反序列化完成信号
     * @param messageType 消息类型
     * @param success 是否成功
     * @param parameterCount 参数数量（如果成功）
     */
    void deserializationCompleted(MessageType messageType, bool success, int parameterCount);

    /**
     * @brief 序列化错误信号
     * @param messageType 消息类型
     * @param errorMessage 错误信息
     */
    void serializationError(MessageType messageType, const QString& errorMessage);

private:
    /**
     * @brief 记录操作统计信息
     * @param messageType 消息类型
     * @param operation 操作类型（"serialize" 或 "deserialize"）
     * @param success 是否成功
     * @param dataSize 数据大小
     */
    void recordStatistics(MessageType messageType, const QString& operation, bool success, int dataSize);

private:
    std::unique_ptr<MessageFactory> messageFactory_;

    // 统计信息
    struct Statistics {
        int serializeCount = 0;
        int deserializeCount = 0;
        int serializeErrorCount = 0;
        int deserializeErrorCount = 0;
        int totalBytesProcessed = 0;
    };
    QHash<MessageType, Statistics> statistics_;
};

} // namespace Protocol

#endif // MESSAGE_SERIALIZER_H
