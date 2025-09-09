#ifndef MESSAGE_FACTORY_H
#define MESSAGE_FACTORY_H

#include <memory>
#include <QHash>
#include <QString>
#include "../core/message_types.h"
#include "../core/imessage_handler.h"

namespace Protocol {

/**
 * @brief 消息工厂
 *
 * 负责创建和管理各种消息处理器
 * 使用工厂模式和单例模式确保处理器的统一管理
 */
class MessageFactory {
public:
    MessageFactory();
    ~MessageFactory() = default;

    // 禁止拷贝和赋值
    MessageFactory(const MessageFactory&) = delete;
    MessageFactory& operator=(const MessageFactory&) = delete;

    /**
     * @brief 获取消息处理器
     * @param messageType 消息类型
     * @return 消息处理器指针，如果不支持该类型则返回nullptr
     */
    IMessageHandler* getHandler(MessageType messageType) const;

    /**
     * @brief 注册自定义消息处理器
     * @param messageType 消息类型
     * @param handler 处理器实例（工厂将获得所有权）
     * @return 成功返回true，失败返回false
     */
    bool registerHandler(MessageType messageType, std::shared_ptr<IMessageHandler> handler);

    /**
     * @brief 检查是否支持某种消息类型
     * @param messageType 消息类型
     * @return 支持返回true，不支持返回false
     */
    bool isSupported(MessageType messageType) const;

    /**
     * @brief 获取所有支持的消息类型
     * @return 消息类型列表
     */
    QList<MessageType> getSupportedTypes() const;

    /**
     * @brief 获取消息类型描述信息
     * @param messageType 消息类型
     * @return 描述信息，如果不支持则返回空字符串
     */
    QString getTypeDescription(MessageType messageType) const;

    /**
     * @brief 清除所有注册的处理器
     */
    void clear();

private:
    /**
     * @brief 初始化默认处理器
     */
    void initializeDefaultHandlers();

private:
    QHash<MessageType, std::shared_ptr<IMessageHandler>> handlers_;
};

} // namespace Protocol

#endif // MESSAGE_FACTORY_H
