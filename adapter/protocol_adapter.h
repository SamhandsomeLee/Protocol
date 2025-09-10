#ifndef PROTOCOL_ADAPTER_H
#define PROTOCOL_ADAPTER_H

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QStringList>
#include <QByteArray>
#include <QString>
#include <QHash>
#include <QTimer>
#include <QScopedPointer>
#include "protocol/transport/itransport.h"
#include "protocol/core/message_types.h"

extern "C" {
#include "protocol/nanopb/pb.h"
#include "protocol/nanopb/pb_encode.h"
#include "protocol/nanopb/pb_decode.h"
#include "protocol/messages/ERNC_praram.pb.h"
#include "protocol/messages/basic.pb.h"
}

/**
 * @brief 协议适配器 - 封装nanopb操作，提供高级接口
 *
 * 根据参数绑定设计文档实现的协议适配器，负责：
 * - 封装 nanopb 的序列化/反序列化逻辑
 * - 提供参数路径到 protobuf 字段的映射
 * - 处理协议版本兼容性
 * - 通过依赖注入的传输层进行通信
 *
 * 使用依赖注入模式，支持多种传输方式：
 * - 串口通信 (SerialTransport)
 * - TCP通信 (TcpTransport)
 * - 模拟传输 (MockTransport) - 用于单元测试
 */
class ProtocolAdapter : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolAdapter(QObject *parent = nullptr);
    explicit ProtocolAdapter(ITransport* transport, QObject *parent = nullptr);
    ~ProtocolAdapter();

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
     * @brief 使用Protocol命名空间的MessageType
     */
    using MessageType = Protocol::MessageType;
    using FunctionCode = Protocol::FunctionCode;

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

private slots:
    // 传输层数据接收处理
    void handleTransportDataReceived(const QByteArray& data);

    // 传输层错误处理
    void handleTransportError(const QString& error);

    // 传输层连接状态变化处理
    void handleTransportConnectionChanged(bool connected);

private:
    /**
     * @brief 内部数据结构
     */

    // 参数映射信息
    struct ParameterMapping {
        QString logicalPath;        // 逻辑参数路径
        QString protobufPath;       // protobuf 字段路径
        QString fieldType;          // 字段类型
        QVariant defaultValue;      // 默认值
        MessageType messageType;    // 消息类型
        bool deprecated;            // 是否已弃用
        QString replacedBy;         // 替换参数
    };

    /**
     * @brief 私有方法
     */

    // 初始化默认参数映射
    void initializeDefaultMappings();

    // 序列化具体消息类型
    QByteArray serializeMessage(MessageType type, const QVariantMap& parameters);

    // 反序列化具体消息类型
    bool deserializeMessage(MessageType type, const QByteArray& data, QVariantMap& parameters);

    // 参数路径到消息类型的映射
    MessageType getMessageTypeFromPath(const QString& parameterPath) const;

    // 设置消息字段值
    bool setMessageField(void* message, const ParameterMapping& mapping, const QVariant& value);

    // 获取消息字段值
    QVariant getMessageField(const void* message, const ParameterMapping& mapping);

    // 发送原始数据
    bool sendRawData(const QByteArray& data);

    // 处理接收的数据
    void processReceivedData(const QByteArray& data);

    // 连接传输层信号
    void connectTransportSignals();

    // 断开传输层信号
    void disconnectTransportSignals();

    /**
     * @brief 具体消息序列化方法 - 基于ERNC_praram.proto
     */
    // 实时数据流相关
    QByteArray serializeChannelNumber(const QVariantMap& parameters);
    QByteArray serializeChannelAmplitude(const QVariantMap& parameters);
    QByteArray serializeChannelSwitch(const QVariantMap& parameters);
    QByteArray serializeCheckMod(uint32_t checkMod);

    // 车辆CAN信息相关
    QByteArray serializeAncSwitch(const QVariantMap& parameters);
    QByteArray serializeVehicleState(const QVariantMap& parameters);

    // 传函标定相关
    QByteArray serializeTranFuncFlag(bool value);
    QByteArray serializeTranFuncState(uint32_t state);
    QByteArray serializeFilterRanges(const QVariantMap& parameters);

    // 系统配置相关
    QByteArray serializeSystemRanges(const QVariantMap& parameters);

    // ENC标定相关
    QByteArray serializeOrderFlag(const QVariantMap& parameters);
    QByteArray serializeOrder2Params(const QVariantMap& parameters);
    QByteArray serializeOrder4Params(const QVariantMap& parameters);
    QByteArray serializeOrder6Params(const QVariantMap& parameters);

    // RNC标定相关
    QByteArray serializeAlphaParams(const QVariantMap& parameters);
    QByteArray serializeFreqDivision(const QVariantMap& parameters);
    QByteArray serializeThresholds(const QVariantMap& parameters);

    /**
     * @brief 具体消息反序列化方法 - 基于ERNC_praram.proto
     */
    // 实时数据流相关
    bool deserializeChannelNumber(const QByteArray& data, QVariantMap& parameters);
    bool deserializeChannelAmplitude(const QByteArray& data, QVariantMap& parameters);
    bool deserializeChannelSwitch(const QByteArray& data, QVariantMap& parameters);
    bool deserializeCheckMod(const QByteArray& data, QVariantMap& parameters);

    // 车辆CAN信息相关
    bool deserializeAncSwitch(const QByteArray& data, QVariantMap& parameters);
    bool deserializeVehicleState(const QByteArray& data, QVariantMap& parameters);

    // 传函标定相关
    bool deserializeTranFuncFlag(const QByteArray& data, QVariantMap& parameters);
    bool deserializeTranFuncState(const QByteArray& data, QVariantMap& parameters);
    bool deserializeFilterRanges(const QByteArray& data, QVariantMap& parameters);

    // 系统配置相关
    bool deserializeSystemRanges(const QByteArray& data, QVariantMap& parameters);

    // ENC标定相关
    bool deserializeOrderFlag(const QByteArray& data, QVariantMap& parameters);
    bool deserializeOrder2Params(const QByteArray& data, QVariantMap& parameters);
    bool deserializeOrder4Params(const QByteArray& data, QVariantMap& parameters);
    bool deserializeOrder6Params(const QByteArray& data, QVariantMap& parameters);

    // RNC标定相关
    bool deserializeAlphaParams(const QByteArray& data, QVariantMap& parameters);
    bool deserializeFreqDivision(const QByteArray& data, QVariantMap& parameters);
    bool deserializeThresholds(const QByteArray& data, QVariantMap& parameters);

    /**
     * @brief 成员变量
     */
    ITransport* transport_;                             // 传输层对象（依赖注入）
    QHash<QString, ParameterMapping> pathMapping_;      // 参数路径映射
    QString protocolVersion_;                           // 协议版本
    QByteArray receiveBuffer_;                          // 接收缓冲区
    bool transportOwned_;                               // 是否拥有传输层对象的所有权

    // 协议常量
    static const QString PROTOCOL_VERSION;
    static const int SERIAL_TIMEOUT_MS;
    static const int MAX_RETRY_COUNT;
};

#endif // PROTOCOL_ADAPTER_H
