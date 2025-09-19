#include "QSerialTransport.h"

namespace ernc::v2 {

QSerialTransport::QSerialTransport(const QString& portName,
                                   int baudRate,
                                   QObject* parent)
    : QObject(parent) {
    serial_.setPortName(portName);
    serial_.setBaudRate(baudRate);
    serial_.setDataBits(QSerialPort::Data8);
    serial_.setParity(QSerialPort::NoParity);
    serial_.setStopBits(QSerialPort::OneStop);
    serial_.setFlowControl(QSerialPort::NoFlowControl);
    connect(&serial_, &QSerialPort::readyRead, this, &QSerialTransport::onReadyRead);
}

bool QSerialTransport::open() {
    bool ok = serial_.open(QIODevice::ReadWrite);
    if (ok) {
        serial_.setDataTerminalReady(true);
        serial_.setRequestToSend(true);
    }
    return ok;
}

void QSerialTransport::close() {
    if (serial_.isOpen()) serial_.close();
}

bool QSerialTransport::send(const uint8_t* data, size_t len) {
    if (!serial_.isOpen()) return false;
    const char* ptr = reinterpret_cast<const char*>(data);
    qint64 total = 0;
    const qint64 target = static_cast<qint64>(len);
    while (total < target) {
        qint64 n = serial_.write(ptr + total, target - total);
        if (n < 0) return false;
        total += n;
        if (!serial_.waitForBytesWritten(1000)) return false;
    }
    return true;
}

void QSerialTransport::setReceiveHandler(std::function<void(const uint8_t*, size_t)> handler) {
    receiveHandler_ = std::move(handler);
}

void QSerialTransport::onReadyRead() {
    // 直接把当前可读数据上报为一条完整protobuf消息（无长度前缀）
    rxBuffer_.append(serial_.readAll());
    if (receiveHandler_ && !rxBuffer_.isEmpty()) {
        const uint8_t* raw = reinterpret_cast<const uint8_t*>(rxBuffer_.constData());
        receiveHandler_(raw, static_cast<size_t>(rxBuffer_.size()));
        rxBuffer_.clear();
    }
}

} // namespace ernc::v2


