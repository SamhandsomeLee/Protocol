#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <memory>
#include <atomic>
#include <functional>

#include "common/utils/ring_buffer.h"

namespace Protocol {
namespace Buffer {

/**
 * @brief 数据项结构，包含数据和元信息
 */
struct DataItem {
    QByteArray data;
    quint64 timestamp;
    quint32 priority;
    QString type;

    DataItem() = default;
    DataItem(const QByteArray& d, const QString& t = "default", quint32 p = 0)
        : data(d), timestamp(QDateTime::currentMSecsSinceEpoch()), priority(p), type(t) {}
};

/**
 * @brief 生产者消费者管理器
 *
 * 该类管理协议系统中的数据生产和消费，使用高性能环形缓冲区
 * 支持多个生产者和消费者，并提供优先级处理和流量控制
 */
class ProducerConsumerManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 处理策略枚举
     */
    enum class ProcessingStrategy {
        FIFO,           // 先进先出
        LIFO,           // 后进先出
        PRIORITY,       // 优先级处理
        BATCH           // 批量处理
    };

    /**
     * @brief 流量控制配置
     */
    struct FlowControlConfig {
        size_t maxQueueSize = 10000;        // 最大队列大小
        size_t highWaterMark = 8000;        // 高水位标记
        size_t lowWaterMark = 2000;         // 低水位标记
        int maxBatchSize = 100;             // 最大批处理大小
        int processingIntervalMs = 10;      // 处理间隔（毫秒）
    };

    explicit ProducerConsumerManager(QObject *parent = nullptr);
    ~ProducerConsumerManager();

    // === 配置接口 ===
    void setProcessingStrategy(ProcessingStrategy strategy);
    void setFlowControlConfig(const FlowControlConfig& config);
    void setDataProcessor(std::function<bool(const DataItem&)> processor);
    void setBatchProcessor(std::function<bool(const QList<DataItem>&)> processor);

    // === 生产者接口 ===
    bool produceData(const QByteArray& data, const QString& type = "default", quint32 priority = 0);
    bool produceDataBatch(const QList<DataItem>& items);

    // === 消费者控制 ===
    void startConsumers();
    void stopConsumers();
    void pauseConsumers();
    void resumeConsumers();

    // === 状态查询 ===
    bool isRunning() const { return running_.load(); }
    bool isPaused() const { return paused_.load(); }
    size_t getQueueSize() const;
    size_t getProcessedCount() const { return processedCount_.load(); }
    size_t getDroppedCount() const { return droppedCount_.load(); }

    // === 统计信息 ===
    struct Statistics {
        size_t totalProduced = 0;
        size_t totalConsumed = 0;
        size_t totalDropped = 0;
        size_t currentQueueSize = 0;
        double averageProcessingTime = 0.0;
        size_t highWaterMarkHits = 0;
        QDateTime lastProcessTime;
    };

    Statistics getStatistics() const;
    void resetStatistics();

signals:
    // === 流量控制信号 ===
    void highWaterMarkReached(size_t currentSize);
    void lowWaterMarkReached(size_t currentSize);
    void queueOverflow(size_t droppedItems);

    // === 处理状态信号 ===
    void dataProcessed(const QString& type, quint64 timestamp);
    void batchProcessed(int batchSize, quint64 totalTime);
    void processingError(const QString& error, const QString& dataType);

    // === 性能监控信号 ===
    void performanceReport(const Statistics& stats);

private slots:
    void processData();
    void updateStatistics();

private:
    // === 核心组件 ===
    using RingBuffer = Common::Utils::ThreadSafeRingBuffer<DataItem>;
    std::unique_ptr<RingBuffer> dataQueue_;

    // === 工作线程 ===
    QThread* consumerThread_;
    QTimer* processingTimer_;
    QTimer* statisticsTimer_;

    // === 配置 ===
    ProcessingStrategy strategy_;
    FlowControlConfig flowConfig_;

    // === 处理器 ===
    std::function<bool(const DataItem&)> dataProcessor_;
    std::function<bool(const QList<DataItem>&)> batchProcessor_;

    // === 状态控制 ===
    std::atomic<bool> running_;
    std::atomic<bool> paused_;
    std::atomic<bool> stopping_;

    // === 统计计数器 ===
    std::atomic<size_t> processedCount_;
    std::atomic<size_t> droppedCount_;
    std::atomic<size_t> totalProduced_;
    mutable QMutex statisticsMutex_;
    Statistics currentStats_;

    // === 性能监控 ===
    QList<quint64> processingTimes_;
    mutable QMutex performanceMutex_;

    // === 私有方法 ===
    void initializeComponents();
    void setupTimers();
    void cleanupComponents();

    bool processDataItem(const DataItem& item);
    bool processBatchItems(const QList<DataItem>& items);
    QList<DataItem> extractBatch();

    void updateFlowControl(size_t currentSize);
    void recordProcessingTime(quint64 timeMs);

    // === 优先级排序 ===
    static bool comparePriority(const DataItem& a, const DataItem& b);
};

/**
 * @brief 专门用于协议数据的生产者消费者管理器
 */
class ProtocolDataManager : public ProducerConsumerManager
{
    Q_OBJECT

public:
    explicit ProtocolDataManager(QObject *parent = nullptr);

    // === 协议特定接口 ===
    bool produceIncomingData(const QByteArray& rawData);
    bool produceOutgoingData(const QByteArray& protocolData, quint32 priority = 0);
    bool produceControlData(const QByteArray& controlData, quint32 priority = 100);

    // === 数据分类处理 ===
    void setIncomingDataHandler(std::function<bool(const QByteArray&)> handler);
    void setOutgoingDataHandler(std::function<bool(const QByteArray&)> handler);
    void setControlDataHandler(std::function<bool(const QByteArray&)> handler);

signals:
    // === 协议特定信号 ===
    void incomingDataReady(const QByteArray& data);
    void outgoingDataProcessed(bool success);
    void controlDataExecuted(bool success);
    void protocolError(const QString& error);

private:
    // === 数据处理器 ===
    std::function<bool(const QByteArray&)> incomingHandler_;
    std::function<bool(const QByteArray&)> outgoingHandler_;
    std::function<bool(const QByteArray&)> controlHandler_;

    // === 私有方法 ===
    bool handleProtocolData(const DataItem& item);
    void initializeProtocolProcessors();
};

} // namespace Buffer
} // namespace Protocol
