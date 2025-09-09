#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include "protocol/transport/itransport.h"

namespace Protocol {

/**
 * @brief 连接管理器
 *
 * 负责管理传输层连接，处理数据收发，错误重试等
 * 提供统一的连接管理接口，屏蔽底层传输细节
 */
class ConnectionManager : public QObject {
    Q_OBJECT

public:
    explicit ConnectionManager(QObject* parent = nullptr);
    ~ConnectionManager() override;

    /**
     * @brief 设置传输层（依赖注入）
     * @param transport 传输层对象，管理器不获得所有权
     */
    void setTransport(ITransport* transport);

    /**
     * @brief 获取当前传输层
     * @return 传输层对象指针，可能为nullptr
     */
    ITransport* transport() const { return transport_; }

    /**
     * @brief 检查是否已连接
     * @return 连接状态
     */
    bool isConnected() const;

    /**
     * @brief 获取传输层描述信息
     * @return 描述字符串
     */
    QString transportDescription() const;

    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 成功返回true，失败返回false
     */
    bool sendData(const QByteArray& data);

    /**
     * @brief 发送数据（带重试）
     * @param data 要发送的数据
     * @param maxRetries 最大重试次数
     * @return 成功返回true，失败返回false
     */
    bool sendDataWithRetry(const QByteArray& data, int maxRetries = 3);

    /**
     * @brief 设置接收缓冲区大小
     * @param size 缓冲区大小（字节）
     */
    void setReceiveBufferSize(int size);

    /**
     * @brief 获取接收缓冲区大小
     * @return 缓冲区大小
     */
    int receiveBufferSize() const { return maxBufferSize_; }

    /**
     * @brief 清除接收缓冲区
     */
    void clearReceiveBuffer();

    /**
     * @brief 获取连接统计信息
     */
    struct ConnectionStats {
        int bytesSent = 0;
        int bytesReceived = 0;
        int sendErrorCount = 0;
        int receiveErrorCount = 0;
        int retryCount = 0;
        QString lastError;
    };

    ConnectionStats getConnectionStats() const { return stats_; }

    /**
     * @brief 重置统计信息
     */
    void resetStats();

signals:
    /**
     * @brief 接收到数据信号
     * @param data 接收到的数据
     */
    void dataReceived(const QByteArray& data);

    /**
     * @brief 连接状态变化信号
     * @param connected 连接状态
     */
    void connectionStatusChanged(bool connected);

    /**
     * @brief 通信错误信号
     * @param error 错误信息
     */
    void communicationError(const QString& error);

    /**
     * @brief 数据发送完成信号
     * @param success 是否成功
     * @param bytesWritten 发送的字节数
     */
    void dataSent(bool success, int bytesWritten);

    /**
     * @brief 重试发送信号
     * @param attempt 当前重试次数
     * @param maxRetries 最大重试次数
     */
    void retryingSend(int attempt, int maxRetries);

private slots:
    /**
     * @brief 处理传输层数据接收
     * @param data 接收到的数据
     */
    void handleTransportDataReceived(const QByteArray& data);

    /**
     * @brief 处理传输层错误
     * @param error 错误信息
     */
    void handleTransportError(const QString& error);

    /**
     * @brief 处理传输层连接状态变化
     * @param connected 连接状态
     */
    void handleTransportConnectionChanged(bool connected);

    /**
     * @brief 重试定时器超时处理
     */
    void handleRetryTimeout();

private:
    /**
     * @brief 连接传输层信号
     */
    void connectTransportSignals();

    /**
     * @brief 断开传输层信号
     */
    void disconnectTransportSignals();

    /**
     * @brief 处理接收缓冲区数据
     */
    void processReceiveBuffer();

    /**
     * @brief 检查数据包完整性
     * @param data 数据
     * @return 完整返回true，不完整返回false
     */
    bool isPacketComplete(const QByteArray& data) const;

    /**
     * @brief 提取完整的数据包
     * @return 完整的数据包列表
     */
    QList<QByteArray> extractCompletePackets();

private:
    ITransport* transport_ = nullptr;       // 传输层对象
    QByteArray receiveBuffer_;              // 接收缓冲区
    int maxBufferSize_ = 4096;              // 最大缓冲区大小

    // 重试机制
    QTimer* retryTimer_;                    // 重试定时器
    QQueue<QByteArray> retryQueue_;         // 重试队列
    QMutex retryMutex_;                     // 重试队列互斥锁
    int currentRetryCount_ = 0;             // 当前重试次数
    int maxRetryCount_ = 3;                 // 最大重试次数
    int retryIntervalMs_ = 1000;            // 重试间隔（毫秒）

    // 统计信息
    mutable QMutex statsMutex_;             // 统计信息互斥锁
    ConnectionStats stats_;                 // 连接统计

    // 协议相关
    static constexpr uint8_t PACKET_HEADER = 0xAA;  // 数据包头
    static constexpr uint8_t PACKET_FOOTER = 0x55;  // 数据包尾
    static constexpr int MIN_PACKET_SIZE = 3;        // 最小数据包大小（头+长度+尾）
};

} // namespace Protocol

#endif // CONNECTION_MANAGER_H
