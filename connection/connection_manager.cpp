#include "connection_manager.h"
#include <QDebug>
#include <QMutexLocker>

namespace Protocol {

ConnectionManager::ConnectionManager(QObject* parent)
    : QObject(parent)
    , retryTimer_(new QTimer(this))
{
    retryTimer_->setSingleShot(true);
    connect(retryTimer_, &QTimer::timeout, this, &ConnectionManager::handleRetryTimeout);

    qDebug() << "ConnectionManager initialized";
}

ConnectionManager::~ConnectionManager() {
    disconnectTransportSignals();
    qDebug() << "ConnectionManager destroyed";
}

void ConnectionManager::setTransport(ITransport* transport) {
    if (transport_ == transport) {
        return; // 相同的传输层，无需重新设置
    }

    // 断开之前的传输层信号
    disconnectTransportSignals();

    transport_ = transport;

    // 连接新的传输层信号
    if (transport_) {
        connectTransportSignals();
        qInfo() << "Transport set:" << transportDescription();
    } else {
        qInfo() << "Transport cleared";
    }

    // 清除缓冲区和重置状态
    clearReceiveBuffer();
    resetStats();

    // 通知连接状态变化
    emit connectionStatusChanged(isConnected());
}

bool ConnectionManager::isConnected() const {
    return transport_ && transport_->isOpen();
}

QString ConnectionManager::transportDescription() const {
    if (!transport_) {
        return "No transport";
    }

    return QString("%1 (%2)")
           .arg(transport_->transportType())
           .arg(isConnected() ? "Connected" : "Disconnected");
}

bool ConnectionManager::sendData(const QByteArray& data) {
    if (!transport_) {
        QString error = "No transport available";
        qWarning() << error;
        emit communicationError(error);

        QMutexLocker locker(&statsMutex_);
        stats_.sendErrorCount++;
        stats_.lastError = error;
        return false;
    }

    if (!transport_->isOpen()) {
        QString error = "Transport not connected";
        qWarning() << error;
        emit communicationError(error);

        QMutexLocker locker(&statsMutex_);
        stats_.sendErrorCount++;
        stats_.lastError = error;
        return false;
    }

    if (data.isEmpty()) {
        QString error = "Cannot send empty data";
        qWarning() << error;
        emit communicationError(error);

        QMutexLocker locker(&statsMutex_);
        stats_.sendErrorCount++;
        stats_.lastError = error;
        return false;
    }

    // 构造带协议头尾的数据包
    QByteArray packet;
    packet.append(PACKET_HEADER);                    // 包头
    packet.append(static_cast<uint8_t>(data.size())); // 数据长度
    packet.append(data);                            // 数据内容
    packet.append(PACKET_FOOTER);                   // 包尾

    // 发送数据
    bool success = transport_->send(packet);

    {
        QMutexLocker locker(&statsMutex_);
        if (success) {
            stats_.bytesSent += packet.size();
            qDebug() << "Data sent successfully:" << packet.size() << "bytes";
        } else {
            stats_.sendErrorCount++;
            stats_.lastError = "Transport write failed";
            qWarning() << "Failed to send data:" << packet.size() << "bytes";
        }
    }

    emit dataSent(success, success ? packet.size() : 0);
    return success;
}

bool ConnectionManager::sendDataWithRetry(const QByteArray& data, int maxRetries) {
    maxRetryCount_ = maxRetries;
    currentRetryCount_ = 0;

    // 先尝试直接发送
    if (sendData(data)) {
        return true; // 发送成功，无需重试
    }

    // 发送失败，添加到重试队列
    {
        QMutexLocker locker(&retryMutex_);
        retryQueue_.enqueue(data);
    }

    // 启动重试定时器
    if (!retryTimer_->isActive()) {
        retryTimer_->start(retryIntervalMs_);
        qDebug() << "Started retry timer for data sending";
    }

    return false; // 首次发送失败，等待重试结果
}

void ConnectionManager::setReceiveBufferSize(int size) {
    if (size <= 0) {
        qWarning() << "Invalid buffer size:" << size;
        return;
    }

    maxBufferSize_ = size;

    // 如果当前缓冲区超过新大小，截断它
    if (receiveBuffer_.size() > maxBufferSize_) {
        receiveBuffer_ = receiveBuffer_.left(maxBufferSize_);
        qWarning() << "Receive buffer truncated to" << maxBufferSize_ << "bytes";
    }

    qDebug() << "Receive buffer size set to:" << maxBufferSize_ << "bytes";
}

void ConnectionManager::clearReceiveBuffer() {
    receiveBuffer_.clear();
    qDebug() << "Receive buffer cleared";
}

void ConnectionManager::resetStats() {
    QMutexLocker locker(&statsMutex_);
    stats_ = ConnectionStats();
    qDebug() << "Connection statistics reset";
}

void ConnectionManager::handleTransportDataReceived(const QByteArray& data) {
    if (data.isEmpty()) {
        return;
    }

    // 添加到接收缓冲区
    receiveBuffer_.append(data);

    // 检查缓冲区大小
    if (receiveBuffer_.size() > maxBufferSize_) {
        qWarning() << "Receive buffer overflow, clearing buffer";
        receiveBuffer_.clear();

        QMutexLocker locker(&statsMutex_);
        stats_.receiveErrorCount++;
        stats_.lastError = "Receive buffer overflow";
        emit communicationError("Receive buffer overflow");
        return;
    }

    {
        QMutexLocker locker(&statsMutex_);
        stats_.bytesReceived += data.size();
    }

    // 处理缓冲区中的完整数据包
    processReceiveBuffer();
}

void ConnectionManager::handleTransportError(const QString& error) {
    qWarning() << "Transport error:" << error;

    {
        QMutexLocker locker(&statsMutex_);
        stats_.lastError = error;
        stats_.receiveErrorCount++;
    }

    emit communicationError(error);
}

void ConnectionManager::handleTransportConnectionChanged(bool connected) {
    qInfo() << "Transport connection status changed:" << connected;

    if (!connected) {
        // 连接断开时清除缓冲区
        clearReceiveBuffer();

        // 停止重试
        retryTimer_->stop();
        {
            QMutexLocker locker(&retryMutex_);
            retryQueue_.clear();
        }
        currentRetryCount_ = 0;
    }

    emit connectionStatusChanged(connected);
}

void ConnectionManager::handleRetryTimeout() {
    QByteArray dataToRetry;

    {
        QMutexLocker locker(&retryMutex_);
        if (retryQueue_.isEmpty()) {
            return; // 没有数据需要重试
        }
        dataToRetry = retryQueue_.dequeue();
    }

    currentRetryCount_++;
    emit retryingSend(currentRetryCount_, maxRetryCount_);

    qDebug() << "Retrying send, attempt:" << currentRetryCount_ << "/" << maxRetryCount_;

    if (sendData(dataToRetry)) {
        // 重试成功
        qInfo() << "Retry send successful on attempt:" << currentRetryCount_;
        currentRetryCount_ = 0;

        QMutexLocker locker(&statsMutex_);
        stats_.retryCount++;
        return;
    }

    // 重试失败
    if (currentRetryCount_ < maxRetryCount_) {
        // 还有重试机会，重新加入队列
        {
            QMutexLocker locker(&retryMutex_);
            retryQueue_.enqueue(dataToRetry);
        }
        retryTimer_->start(retryIntervalMs_);
    } else {
        // 重试次数用完
        qWarning() << "Send retry failed after" << maxRetryCount_ << "attempts";
        currentRetryCount_ = 0;

        QMutexLocker locker(&statsMutex_);
        stats_.sendErrorCount++;
        stats_.lastError = QString("Send failed after %1 retries").arg(maxRetryCount_);

        emit communicationError(stats_.lastError);
    }
}

void ConnectionManager::connectTransportSignals() {
    if (!transport_) {
        return;
    }

    connect(transport_, &ITransport::dataReceived,
            this, &ConnectionManager::handleTransportDataReceived);
    connect(transport_, &ITransport::transportError,
            this, &ConnectionManager::handleTransportError);
    connect(transport_, &ITransport::connectionStatusChanged,
            this, &ConnectionManager::handleTransportConnectionChanged);

    qDebug() << "Transport signals connected";
}

void ConnectionManager::disconnectTransportSignals() {
    if (!transport_) {
        return;
    }

    disconnect(transport_, nullptr, this, nullptr);
    qDebug() << "Transport signals disconnected";
}

void ConnectionManager::processReceiveBuffer() {
    auto packets = extractCompletePackets();

    for (const QByteArray& packet : packets) {
        qDebug() << "Complete packet received:" << packet.size() << "bytes";
        emit dataReceived(packet);
    }
}

bool ConnectionManager::isPacketComplete(const QByteArray& data) const {
    if (data.size() < MIN_PACKET_SIZE) {
        return false; // 数据太短
    }

    // 检查包头
    if (static_cast<uint8_t>(data[0]) != PACKET_HEADER) {
        return false;
    }

    // 获取数据长度
    uint8_t dataLength = static_cast<uint8_t>(data[1]);
    int expectedPacketSize = 2 + dataLength + 1; // 头+长度+数据+尾

    if (data.size() < expectedPacketSize) {
        return false; // 数据不完整
    }

    // 检查包尾
    if (static_cast<uint8_t>(data[expectedPacketSize - 1]) != PACKET_FOOTER) {
        return false;
    }

    return true;
}

QList<QByteArray> ConnectionManager::extractCompletePackets() {
    QList<QByteArray> packets;

    while (receiveBuffer_.size() >= MIN_PACKET_SIZE) {
        // 查找包头
        int headerIndex = receiveBuffer_.indexOf(PACKET_HEADER);
        if (headerIndex == -1) {
            // 没有找到包头，清除缓冲区
            receiveBuffer_.clear();
            break;
        }

        // 移除包头之前的无效数据
        if (headerIndex > 0) {
            receiveBuffer_.remove(0, headerIndex);
            qWarning() << "Removed" << headerIndex << "bytes of invalid data";
        }

        // 检查是否有足够的数据来解析包长度
        if (receiveBuffer_.size() < 2) {
            break; // 等待更多数据
        }

        // 获取数据长度
        uint8_t dataLength = static_cast<uint8_t>(receiveBuffer_[1]);
        int expectedPacketSize = 2 + dataLength + 1; // 头+长度+数据+尾

        // 检查是否有完整的数据包
        if (receiveBuffer_.size() < expectedPacketSize) {
            break; // 等待更多数据
        }

        // 提取可能的数据包
        QByteArray potentialPacket = receiveBuffer_.left(expectedPacketSize);

        if (isPacketComplete(potentialPacket)) {
            // 提取数据内容（去除包头、长度和包尾）
            QByteArray packetData = potentialPacket.mid(2, dataLength);
            packets.append(packetData);

            // 从缓冲区移除已处理的数据包
            receiveBuffer_.remove(0, expectedPacketSize);
        } else {
            // 无效的数据包，移除包头并继续搜索
            receiveBuffer_.remove(0, 1);
            qWarning() << "Invalid packet format, removed 1 byte";
        }
    }

    return packets;
}

} // namespace Protocol
