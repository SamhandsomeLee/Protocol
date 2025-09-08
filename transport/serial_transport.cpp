#include "serial_transport.h"
#include <QDebug>
#include <QSerialPortInfo>

// 常量定义
const int SerialTransport::DEFAULT_BAUD_RATE = 115200;
const int SerialTransport::DEFAULT_SEND_TIMEOUT_MS = 3000;
const int SerialTransport::DEFAULT_CONNECTION_CHECK_INTERVAL_MS = 5000;

SerialTransport::SerialTransport(QObject* parent)
    : ITransport(parent)
    , serialPort_(new QSerialPort(this))
    , connectionTimer_(new QTimer(this))
    , portName_()
    , baudRate_(DEFAULT_BAUD_RATE)
    , dataBits_(QSerialPort::Data8)
    , parity_(QSerialPort::NoParity)
    , stopBits_(QSerialPort::OneStop)
    , flowControl_(QSerialPort::NoFlowControl)
    , sendTimeoutMs_(DEFAULT_SEND_TIMEOUT_MS)
    , autoReconnectEnabled_(false)
    , connectionCheckIntervalMs_(DEFAULT_CONNECTION_CHECK_INTERVAL_MS)
    , wasConnected_(false)
{
    initializeSerialPort();
}

SerialTransport::SerialTransport(const QString& portName, int baudRate, QObject* parent)
    : SerialTransport(parent)
{
    portName_ = portName;
    baudRate_ = baudRate;
}

SerialTransport::~SerialTransport()
{
    close();
}

void SerialTransport::initializeSerialPort()
{
    // 设置连接检测定时器
    connectionTimer_->setInterval(connectionCheckIntervalMs_);
    connect(connectionTimer_.data(), &QTimer::timeout, this, &SerialTransport::handleConnectionCheck);
}

void SerialTransport::connectSerialSignals()
{
    if (serialPort_) {
        connect(serialPort_.data(), &QSerialPort::readyRead,
                this, &SerialTransport::handleSerialDataReceived);
        connect(serialPort_.data(), QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
                this, &SerialTransport::handleSerialError);
    }
}

void SerialTransport::disconnectSerialSignals()
{
    if (serialPort_) {
        disconnect(serialPort_.data(), nullptr, this, nullptr);
    }
}

void SerialTransport::setPortName(const QString& portName)
{
    if (isOpen()) {
        qWarning() << "Cannot change port name while connection is open";
        return;
    }
    portName_ = portName;
}

void SerialTransport::setBaudRate(int baudRate)
{
    if (isOpen()) {
        qWarning() << "Cannot change baud rate while connection is open";
        return;
    }
    baudRate_ = baudRate;
}

void SerialTransport::setDataBits(QSerialPort::DataBits dataBits)
{
    dataBits_ = dataBits;
}

void SerialTransport::setParity(QSerialPort::Parity parity)
{
    parity_ = parity;
}

void SerialTransport::setStopBits(QSerialPort::StopBits stopBits)
{
    stopBits_ = stopBits;
}

void SerialTransport::setFlowControl(QSerialPort::FlowControl flowControl)
{
    flowControl_ = flowControl;
}

QString SerialTransport::portName() const
{
    return portName_;
}

int SerialTransport::baudRate() const
{
    return baudRate_;
}

QSerialPort::DataBits SerialTransport::dataBits() const
{
    return dataBits_;
}

QSerialPort::Parity SerialTransport::parity() const
{
    return parity_;
}

QSerialPort::StopBits SerialTransport::stopBits() const
{
    return stopBits_;
}

QSerialPort::FlowControl SerialTransport::flowControl() const
{
    return flowControl_;
}

bool SerialTransport::open()
{
    if (isOpen()) {
        qDebug() << "Serial port already open:" << portName_;
        return true;
    }

    if (portName_.isEmpty()) {
        lastError_ = "Port name is empty";
        emitTransportError(lastError_);
        return false;
    }

    // 重新创建串口对象以确保干净的状态
    if (serialPort_) {
        disconnectSerialSignals();
        serialPort_.reset(new QSerialPort(this));
    }

    // 配置串口参数
    serialPort_->setPortName(portName_);
    serialPort_->setBaudRate(baudRate_);
    serialPort_->setDataBits(dataBits_);
    serialPort_->setParity(parity_);
    serialPort_->setStopBits(stopBits_);
    serialPort_->setFlowControl(flowControl_);

    // 连接信号
    connectSerialSignals();

    // 尝试打开串口
    bool success = serialPort_->open(QIODevice::ReadWrite);
    if (success) {
        lastError_.clear();
        wasConnected_ = true;

        // 启动连接检测定时器
        connectionTimer_->start();

        qDebug() << "Serial port opened successfully:" << portName_ << "at" << baudRate_ << "bps";
        emitConnectionStatusChanged(true);

    } else {
        lastError_ = serialPort_->errorString();
        qWarning() << "Failed to open serial port:" << portName_ << "-" << lastError_;
        emitTransportError(QString("Failed to open serial port: %1").arg(lastError_));

        disconnectSerialSignals();
        serialPort_.reset();
    }

    return success;
}

void SerialTransport::close()
{
    if (serialPort_ && serialPort_->isOpen()) {
        // 停止连接检测
        connectionTimer_->stop();

        // 断开信号连接
        disconnectSerialSignals();

        // 关闭串口
        serialPort_->close();

        qDebug() << "Serial port closed:" << portName_;

        if (wasConnected_) {
            wasConnected_ = false;
            emitConnectionStatusChanged(false);
        }
    }
}

bool SerialTransport::isOpen() const
{
    return serialPort_ && serialPort_->isOpen();
}

bool SerialTransport::send(const QByteArray& data)
{
    if (!isOpen()) {
        lastError_ = "Serial port is not open";
        emitTransportError(lastError_);
        return false;
    }

    qint64 bytesWritten = serialPort_->write(data);
    if (bytesWritten == -1) {
        lastError_ = serialPort_->errorString();
        emitTransportError(QString("Failed to write data: %1").arg(lastError_));
        return false;
    }

    bool success = serialPort_->waitForBytesWritten(sendTimeoutMs_);
    if (!success) {
        lastError_ = "Write timeout or error occurred";
        emitTransportError(lastError_);
        return false;
    }

    if (bytesWritten != data.size()) {
        lastError_ = QString("Incomplete write: %1/%2 bytes written").arg(bytesWritten).arg(data.size());
        emitTransportError(lastError_);
        return false;
    }

    qDebug() << "Serial data sent:" << data.size() << "bytes";
    return true;
}

QString SerialTransport::description() const
{
    return QString("Serial Port: %1 (%2 bps)").arg(portName_).arg(baudRate_);
}

QString SerialTransport::transportType() const
{
    return "Serial";
}

void SerialTransport::setSendTimeout(int timeoutMs)
{
    sendTimeoutMs_ = timeoutMs;
}

int SerialTransport::sendTimeout() const
{
    return sendTimeoutMs_;
}

void SerialTransport::setAutoReconnect(bool enable)
{
    autoReconnectEnabled_ = enable;
}

bool SerialTransport::autoReconnect() const
{
    return autoReconnectEnabled_;
}

void SerialTransport::setConnectionCheckInterval(int intervalMs)
{
    connectionCheckIntervalMs_ = intervalMs;
    connectionTimer_->setInterval(intervalMs);
}

int SerialTransport::connectionCheckInterval() const
{
    return connectionCheckIntervalMs_;
}

QString SerialTransport::lastErrorString() const
{
    return lastError_;
}

void SerialTransport::handleSerialDataReceived()
{
    if (!serialPort_) {
        return;
    }

    QByteArray data = serialPort_->readAll();
    if (!data.isEmpty()) {
        qDebug() << "Serial data received:" << data.size() << "bytes";
        emitDataReceived(data);
    }
}

void SerialTransport::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }

    QString errorString;
    switch (error) {
    case QSerialPort::DeviceNotFoundError:
        errorString = "Device not found";
        break;
    case QSerialPort::PermissionError:
        errorString = "Permission denied";
        break;
    case QSerialPort::OpenError:
        errorString = "Failed to open device";
        break;
    case QSerialPort::WriteError:
        errorString = "Write error";
        break;
    case QSerialPort::ReadError:
        errorString = "Read error";
        break;
    case QSerialPort::ResourceError:
        errorString = "Resource error (device disconnected?)";
        break;
    case QSerialPort::UnsupportedOperationError:
        errorString = "Unsupported operation";
        break;
    case QSerialPort::TimeoutError:
        errorString = "Timeout error";
        break;
    default:
        errorString = "Unknown serial port error";
        break;
    }

    lastError_ = errorString;
    qWarning() << "Serial port error:" << errorString;
    emitTransportError(errorString);

    // 严重错误时关闭连接
    if (error == QSerialPort::ResourceError ||
        error == QSerialPort::DeviceNotFoundError ||
        error == QSerialPort::PermissionError) {

        if (wasConnected_) {
            wasConnected_ = false;
            emitConnectionStatusChanged(false);
        }

        // 如果启用了自动重连，尝试重连
        if (autoReconnectEnabled_) {
            QTimer::singleShot(2000, this, &SerialTransport::attemptReconnection);
        }
    }
}

void SerialTransport::handleConnectionCheck()
{
    bool currentlyConnected = isOpen();

    if (wasConnected_ != currentlyConnected) {
        wasConnected_ = currentlyConnected;
        emitConnectionStatusChanged(currentlyConnected);

        if (!currentlyConnected && autoReconnectEnabled_) {
            QTimer::singleShot(1000, this, &SerialTransport::attemptReconnection);
        }
    }
}

bool SerialTransport::attemptReconnection()
{
    if (isOpen()) {
        return true;  // 已经连接了
    }

    qDebug() << "Attempting to reconnect to serial port:" << portName_;

    bool success = open();
    if (success) {
        qDebug() << "Serial port reconnection successful";
    } else {
        qDebug() << "Serial port reconnection failed, will retry later";
    }

    return success;
}
