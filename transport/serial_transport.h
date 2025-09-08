#ifndef SERIAL_TRANSPORT_H
#define SERIAL_TRANSPORT_H

#include "itransport.h"
#include <QSerialPort>
#include <QTimer>
#include <QScopedPointer>

/**
 * @brief 串口传输实现
 *
 * 实现基于QSerialPort的串口通信，提供：
 * - 串口连接管理
 * - 数据收发
 * - 连接状态监控
 * - 错误处理和重连机制
 */
class SerialTransport : public ITransport
{
    Q_OBJECT

public:
    explicit SerialTransport(QObject* parent = nullptr);
    explicit SerialTransport(const QString& portName, int baudRate = 115200, QObject* parent = nullptr);
    ~SerialTransport() override;

    /**
     * @brief 串口配置接口
     */

    // 设置串口参数
    void setPortName(const QString& portName);
    void setBaudRate(int baudRate);
    void setDataBits(QSerialPort::DataBits dataBits);
    void setParity(QSerialPort::Parity parity);
    void setStopBits(QSerialPort::StopBits stopBits);
    void setFlowControl(QSerialPort::FlowControl flowControl);

    // 获取串口参数
    QString portName() const;
    int baudRate() const;
    QSerialPort::DataBits dataBits() const;
    QSerialPort::Parity parity() const;
    QSerialPort::StopBits stopBits() const;
    QSerialPort::FlowControl flowControl() const;

    /**
     * @brief ITransport接口实现
     */
    bool open() override;
    void close() override;
    bool isOpen() const override;
    bool send(const QByteArray& data) override;
    QString description() const override;
    QString transportType() const override;

    /**
     * @brief 串口特有功能
     */

    // 设置发送超时时间
    void setSendTimeout(int timeoutMs);
    int sendTimeout() const;

    // 启用/禁用自动重连
    void setAutoReconnect(bool enable);
    bool autoReconnect() const;

    // 设置连接检测间隔
    void setConnectionCheckInterval(int intervalMs);
    int connectionCheckInterval() const;

    // 获取串口错误信息
    QString lastErrorString() const;

private slots:
    /**
     * @brief 内部信号处理
     */
    void handleSerialDataReceived();
    void handleSerialError(QSerialPort::SerialPortError error);
    void handleConnectionCheck();

private:
    /**
     * @brief 私有方法
     */
    void initializeSerialPort();
    void connectSerialSignals();
    void disconnectSerialSignals();
    bool attemptReconnection();

    /**
     * @brief 成员变量
     */
    QScopedPointer<QSerialPort> serialPort_;    // 串口对象
    QScopedPointer<QTimer> connectionTimer_;    // 连接检测定时器

    // 串口配置
    QString portName_;
    int baudRate_;
    QSerialPort::DataBits dataBits_;
    QSerialPort::Parity parity_;
    QSerialPort::StopBits stopBits_;
    QSerialPort::FlowControl flowControl_;

    // 传输配置
    int sendTimeoutMs_;
    bool autoReconnectEnabled_;
    int connectionCheckIntervalMs_;

    // 状态信息
    QString lastError_;
    bool wasConnected_;  // 用于检测连接状态变化

    // 常量
    static const int DEFAULT_BAUD_RATE;
    static const int DEFAULT_SEND_TIMEOUT_MS;
    static const int DEFAULT_CONNECTION_CHECK_INTERVAL_MS;
};

#endif // SERIAL_TRANSPORT_H
