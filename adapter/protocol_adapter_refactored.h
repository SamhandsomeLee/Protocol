#ifndef PROTOCOL_ADAPTER_REFACTORED_H
#define PROTOCOL_ADAPTER_REFACTORED_H

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QStringList>
#include <QByteArray>
#include <QString>
#include <memory>

#include "../transport/itransport.h"
#include "../core/message_types.h"
#include "../mapping/parameter_mapper.h"
#include "../serialization/message_serializer.h"
#include "../connection/connection_manager.h"
#include "../version/version_manager.h"

namespace Protocol {

/**
 * @brief 重构后的协议适配器 - 轻量级协调者
 *
 * 职责：
 * - 协调各个组件工作
 * - 提供统一的外部接口
 * - 管理组件间的数据流
 *
 * 不再包含：
 * - 具体的序列化逻辑
 * - 参数映射逻辑
 * - 连接管理逻辑
 * - 版本管理逻辑
 */
class ProtocolAdapterRefactored : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolAdapterRefactored(QObject *parent = nullptr);
    explicit ProtocolAdapterRefactored(ITransport* transport, QObject *parent = nullptr);
    ~ProtocolAdapterRefactored() override;

    /**
     * @brief 协议操作接口
     */

    // 发送单个参数更新
    bool sendParameterUpdate(const QString& parameterPath, const QVariant& value);

    // 发送参数组更新
    bool sendParameterGroup(const QStringList& paths, const QVariantMap& values);

    // 序列化参数到字节数组
    QByteArray serializeParameters(const QVariantMap& parameters);

    // 从字节数组反序列化参数
    bool deserializeParameters(const QByteArray& data, QVariantMap& parameters);

    /**
     * @brief 协议信息接口
     */

    // 获取协议版本
    QString getProtocolVersion() const;

    // 检查参数是否支持
    bool isParameterSupported(const QString& parameterPath) const;

    // 获取支持的参数列表
    QStringList getSupportedParameters() const;

    /**
     * @brief 传输层管理接口
     */

    // 设置传输层（依赖注入）
    void setTransport(ITransport* transport);

    // 获取当前传输层
    ITransport* transport() const;

    // 检查传输层是否已设置且连接
    bool isConnected() const;

    // 获取传输层描述信息
    QString transportDescription() const;

    /**
     * @brief 映射管理接口
     */

    // 加载协议映射配置
    bool loadProtocolMapping(const QString& mappingFile);

    // 获取参数路径对应的protobuf路径
    QString getProtobufPath(const QString& parameterPath) const;

    /**
     * @brief 组件访问接口（用于高级用法）
     */

    // 获取参数映射器
    ParameterMapper* parameterMapper() const { return parameterMapper_.get(); }

    // 获取消息序列化器
    MessageSerializer* messageSerializer() const { return messageSerializer_.get(); }

    // 获取连接管理器
    ConnectionManager* connectionManager() const { return connectionManager_.get(); }

    // 获取版本管理器
    VersionManager* versionManager() const { return versionManager_.get(); }

signals:
    // 参数确认信号
    void parameterAcknowledged(const QString& path);

    // 通信错误信号
    void communicationError(const QString& error);

    // 协议版本不匹配信号
    void protocolVersionMismatch(const QString& expected, const QString& actual);

    // 连接状态变化信号
    void connectionStatusChanged(bool connected);

    // 接收到数据信号
    void dataReceived(const QByteArray& data);

    // 参数映射加载完成信号
    void mappingLoaded(bool success, const QString& errorMessage = QString());

private slots:
    // 处理连接管理器的数据接收
    void handleConnectionDataReceived(const QByteArray& data);

    // 处理连接管理器的错误
    void handleConnectionError(const QString& error);

    // 处理连接状态变化
    void handleConnectionStatusChanged(bool connected);

    // 处理版本不兼容
    void handleVersionIncompatible(const QString& currentVersion,
                                  const QString& remoteVersion,
                                  const QString& reason);

    // 处理参数映射加载结果
    void handleMappingLoaded(bool success, const QString& errorMessage);

private:
    /**
     * @brief 初始化组件
     */
    void initializeComponents();

    /**
     * @brief 连接组件信号
     */
    void connectComponentSignals();

    /**
     * @brief 断开组件信号
     */
    void disconnectComponentSignals();

    /**
     * @brief 根据参数路径确定消息类型
     * @param parameterPath 参数路径
     * @return 消息类型
     */
    MessageType getMessageTypeFromPath(const QString& parameterPath) const;

    /**
     * @brief 处理接收到的协议数据
     * @param data 接收的数据
     */
    void processProtocolData(const QByteArray& data);

    /**
     * @brief 验证协议版本
     * @param data 接收的数据
     * @return 版本验证成功返回true
     */
    bool validateProtocolVersion(const QByteArray& data);

private:
    // 核心组件（使用智能指针管理生命周期）
    std::unique_ptr<ParameterMapper> parameterMapper_;
    std::unique_ptr<MessageSerializer> messageSerializer_;
    std::unique_ptr<ConnectionManager> connectionManager_;
    std::unique_ptr<VersionManager> versionManager_;

    // 状态信息
    bool initialized_ = false;

    // 协议常量
    static const QString PROTOCOL_VERSION;
    static const int DEFAULT_TIMEOUT_MS;
};

} // namespace Protocol

#endif // PROTOCOL_ADAPTER_REFACTORED_H
