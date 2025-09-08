#ifndef PROTOCOL_BUFFER_ADAPTER_H
#define PROTOCOL_BUFFER_ADAPTER_H

#include "common/utils/ring_buffer.h"
#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <memory>

namespace Protocol {
namespace Buffer {

/**
 * @brief 协议数据包结构
 */
struct ProtocolPacket {
    QByteArray data;
    qint64 timestamp;
    int priority;
    QString messageType;

    ProtocolPacket() : timestamp(0), priority(0) {}

    ProtocolPacket(const QByteArray& d, const QString& type = "", int p = 0)
        : data(d), timestamp(QDateTime::currentMSecsSinceEpoch()), priority(p), messageType(type) {}
};

/**
 * @brief 协议缓冲区适配器
 *
 * 将通用环形缓冲区适配为协议层专用的缓冲区，
 * 提供协议相关的便利方法和Qt信号槽支持
 */
class ProtocolBufferAdapter : public QObject {
    Q_OBJECT

public:
    using BufferType = Common::Utils::ThreadSafeRingBuffer<ProtocolPacket>;
    using StatsType = Common::Utils::BufferStats;

    explicit ProtocolBufferAdapter(size_t capacity = 1024, QObject* parent = nullptr)
        : QObject(parent)
        , buffer_(capacity)
        , maxPacketSize_(0)
        , totalDataSize_(0) {

        setupBufferHandlers();
    }

    /**
     * @brief 推送协议数据包
     * @param data 数据内容
     * @param messageType 消息类型
     * @param priority 优先级
     * @param timeoutMs 超时时间
     * @return 成功返回true
     */
    bool pushPacket(const QByteArray& data, const QString& messageType = "",
                   int priority = 0, int timeoutMs = 0) {

        ProtocolPacket packet(data, messageType, priority);
        bool success = (timeoutMs == 0) ? buffer_.tryPush(packet) : buffer_.push(packet, timeoutMs);

        if (success) {
            updateDataStats(data.size(), true);
            emit packetPushed(messageType, data.size());
        } else {
            emit pushFailed(messageType, data.size());
        }

        return success;
    }

    /**
     * @brief 弹出协议数据包
     * @param packet 输出参数
     * @param timeoutMs 超时时间
     * @return 成功返回true
     */
    bool popPacket(ProtocolPacket& packet, int timeoutMs = 0) {
        bool success = (timeoutMs == 0) ? buffer_.tryPop(packet) : buffer_.pop(packet, timeoutMs);

        if (success) {
            updateDataStats(packet.data.size(), false);
            emit packetPopped(packet.messageType, packet.data.size());
        }

        return success;
    }

    /**
     * @brief 批量弹出数据包
     * @param packets 输出容器
     * @param maxCount 最大数量
     * @return 实际弹出数量
     */
    size_t popPacketBatch(QList<ProtocolPacket>& packets, size_t maxCount) {
        std::vector<ProtocolPacket> temp;
        size_t count = buffer_.popBatch(temp, maxCount);

        for (const auto& packet : temp) {
            packets.append(packet);
            updateDataStats(packet.data.size(), false);
            emit packetPopped(packet.messageType, packet.data.size());
        }

        if (count > 0) {
            emit batchPopped(static_cast<int>(count));
        }

        return count;
    }

    /**
     * @brief 设置覆盖策略
     */
    void setOverwritePolicy(bool overwrite) {
        buffer_.setOverwritePolicy(overwrite);
    }

    /**
     * @brief 获取缓冲区统计信息
     */
    StatsType getBufferStats() const {
        return buffer_.getStats();
    }

    /**
     * @brief 获取协议专用统计信息
     */
    struct ProtocolStats {
        StatsType bufferStats;
        size_t maxPacketSize;
        qint64 totalDataSize;
        double averagePacketSize;
    };

    ProtocolStats getProtocolStats() const {
        ProtocolStats stats;
        stats.bufferStats = buffer_.getStats();
        stats.maxPacketSize = maxPacketSize_;
        stats.totalDataSize = totalDataSize_;

        if (stats.bufferStats.totalPushed > 0) {
            stats.averagePacketSize = static_cast<double>(totalDataSize_) / stats.bufferStats.totalPushed;
        } else {
            stats.averagePacketSize = 0.0;
        }

        return stats;
    }

    /**
     * @brief 清空缓冲区
     */
    void clear() {
        buffer_.clear();
        resetDataStats();
        emit bufferCleared();
    }

    /**
     * @brief 关闭缓冲区
     */
    void close() {
        buffer_.close();
        emit bufferClosed();
    }

    /**
     * @brief 重新打开缓冲区
     */
    void reopen() {
        buffer_.reopen();
        emit bufferReopened();
    }

    /**
     * @brief 检查是否已关闭
     */
    bool isClosed() const {
        return buffer_.isClosed();
    }

    /**
     * @brief 获取当前大小
     */
    size_t size() const {
        return buffer_.size();
    }

    /**
     * @brief 获取容量
     */
    size_t capacity() const {
        return buffer_.capacity();
    }

    /**
     * @brief 获取使用率
     */
    double usage() const {
        return buffer_.usage();
    }

    /**
     * @brief 检查是否为空
     */
    bool empty() const {
        return buffer_.empty();
    }

    /**
     * @brief 检查是否已满
     */
    bool full() const {
        return buffer_.full();
    }

signals:
    void packetPushed(const QString& messageType, int dataSize);
    void packetPopped(const QString& messageType, int dataSize);
    void pushFailed(const QString& messageType, int dataSize);
    void batchPopped(int count);
    void bufferOverflow(const QString& messageType, int droppedDataSize);
    void bufferUnderflow();
    void bufferCleared();
    void bufferClosed();
    void bufferReopened();

private slots:
    void onBufferOverflow(const ProtocolPacket& droppedPacket) {
        emit bufferOverflow(droppedPacket.messageType, droppedPacket.data.size());
    }

    void onBufferUnderflow() {
        emit bufferUnderflow();
    }

private:
    void setupBufferHandlers() {
        buffer_.setOverflowHandler([this](const ProtocolPacket& packet) {
            QMetaObject::invokeMethod(this, "onBufferOverflow",
                Qt::QueuedConnection, Q_ARG(ProtocolPacket, packet));
        });

        buffer_.setUnderflowHandler([this]() {
            QMetaObject::invokeMethod(this, "onBufferUnderflow", Qt::QueuedConnection);
        });
    }

    void updateDataStats(size_t dataSize, bool isPush) {
        if (isPush) {
            totalDataSize_ += dataSize;
            if (dataSize > maxPacketSize_) {
                maxPacketSize_ = dataSize;
            }
        } else {
            totalDataSize_ -= dataSize;
        }
    }

    void resetDataStats() {
        maxPacketSize_ = 0;
        totalDataSize_ = 0;
    }

private:
    BufferType buffer_;
    std::atomic<size_t> maxPacketSize_;
    std::atomic<qint64> totalDataSize_;
};

} // namespace Buffer
} // namespace Protocol

// 注册Q_ARG类型
Q_DECLARE_METATYPE(Protocol::Buffer::ProtocolPacket)

#endif // PROTOCOL_BUFFER_ADAPTER_H
