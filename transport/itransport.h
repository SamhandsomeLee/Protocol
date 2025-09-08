#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <QObject>
#include <QByteArray>
#include <QString>

/**
 * @brief 传输层抽象接口
 *
 * 定义了协议适配器与具体传输方式之间的抽象接口，支持：
 * - 串口通信 (SerialTransport)
 * - TCP通信 (TcpTransport)
 * - UDP通信 (UdpTransport)
 * - 模拟传输 (MockTransport) - 用于单元测试
 */
class ITransport : public QObject
{
    Q_OBJECT

public:
    explicit ITransport(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ITransport() = default;

    /**
     * @brief 传输层操作接口
     */

    // 打开连接
    virtual bool open() = 0;

    // 关闭连接
    virtual void close() = 0;

    // 检查连接状态
    virtual bool isOpen() const = 0;

    // 发送数据
    virtual bool send(const QByteArray& data) = 0;

    // 获取传输层描述信息
    virtual QString description() const = 0;

    // 获取传输层类型
    virtual QString transportType() const = 0;

signals:
    /**
     * @brief 传输层信号
     */

    // 接收到数据
    void dataReceived(const QByteArray& data);

    // 连接状态变化
    void connectionStatusChanged(bool connected);

    // 传输错误
    void transportError(const QString& error);

    // 连接建立
    void connected();

    // 连接断开
    void disconnected();

protected:
    /**
     * @brief 受保护的辅助方法
     */

    // 发射数据接收信号（供子类调用）
    void emitDataReceived(const QByteArray& data) {
        emit dataReceived(data);
    }

    // 发射连接状态变化信号
    void emitConnectionStatusChanged(bool connected) {
        emit connectionStatusChanged(connected);
        if (connected) {
            emit this->connected();
        } else {
            emit disconnected();
        }
    }

    // 发射传输错误信号
    void emitTransportError(const QString& error) {
        emit transportError(error);
    }
};

#endif // ITRANSPORT_H
