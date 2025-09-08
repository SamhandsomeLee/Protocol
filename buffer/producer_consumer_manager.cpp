#include "producer_consumer_manager.h"
#include <QDebug>
#include <QDateTime>
#include <QCoreApplication>
#include <algorithm>

namespace Protocol {
namespace Buffer {

// =============================================================================
// ProducerConsumerManager 实现
// =============================================================================

ProducerConsumerManager::ProducerConsumerManager(QObject *parent)
    : QObject(parent)
    , consumerThread_(nullptr)
    , processingTimer_(nullptr)
    , statisticsTimer_(nullptr)
    , strategy_(ProcessingStrategy::FIFO)
    , running_(false)
    , paused_(false)
    , stopping_(false)
    , processedCount_(0)
    , droppedCount_(0)
    , totalProduced_(0)
{
    // 初始化默认配置
    setFlowControlConfig(FlowControlConfig{});

    // 初始化组件
    initializeComponents();
}

ProducerConsumerManager::~ProducerConsumerManager()
{
    stopConsumers();
    cleanupComponents();
}

void ProducerConsumerManager::initializeComponents()
{
    // 创建环形缓冲区
    dataQueue_ = std::make_unique<RingBuffer>(flowConfig_.maxQueueSize);

    // 设置默认的数据处理器
    dataProcessor_ = [this](const DataItem& item) -> bool {
        qDebug() << "Processing data item:" << item.type << "size:" << item.data.size();
        emit dataProcessed(item.type, item.timestamp);
        return true;
    };

    // 设置默认的批处理器
    batchProcessor_ = [this](const QList<DataItem>& items) -> bool {
        quint64 startTime = QDateTime::currentMSecsSinceEpoch();

        for (const auto& item : items) {
            if (!dataProcessor_(item)) {
                return false;
            }
        }

        quint64 processingTime = QDateTime::currentMSecsSinceEpoch() - startTime;
        emit batchProcessed(items.size(), processingTime);
        return true;
    };

    setupTimers();
}

void ProducerConsumerManager::setupTimers()
{
    // 创建消费者线程
    consumerThread_ = new QThread(this);

    // 创建处理定时器
    processingTimer_ = new QTimer();
    processingTimer_->moveToThread(consumerThread_);
    processingTimer_->setInterval(flowConfig_.processingIntervalMs);

    // 创建统计定时器
    statisticsTimer_ = new QTimer(this);
    statisticsTimer_->setInterval(1000); // 每秒更新统计

    // 连接信号槽
    connect(consumerThread_, &QThread::started, processingTimer_, [this]() {
        processingTimer_->start();
    });

    connect(consumerThread_, &QThread::finished, processingTimer_, &QTimer::stop);
    connect(processingTimer_, &QTimer::timeout, this, &ProducerConsumerManager::processData);
    connect(statisticsTimer_, &QTimer::timeout, this, &ProducerConsumerManager::updateStatistics);
}

void ProducerConsumerManager::cleanupComponents()
{
    if (consumerThread_) {
        consumerThread_->quit();
        consumerThread_->wait(3000);
    }

    if (processingTimer_) {
        processingTimer_->deleteLater();
        processingTimer_ = nullptr;
    }
}

void ProducerConsumerManager::setProcessingStrategy(ProcessingStrategy strategy)
{
    strategy_ = strategy;
}

void ProducerConsumerManager::setFlowControlConfig(const FlowControlConfig& config)
{
    flowConfig_ = config;

    // 如果环形缓冲区已存在，需要重新创建
    if (dataQueue_) {
        auto oldSize = dataQueue_->size();
        dataQueue_ = std::make_unique<RingBuffer>(config.maxQueueSize);
        qDebug() << "Ring buffer resized from" << oldSize << "to" << config.maxQueueSize;
    }
}

void ProducerConsumerManager::setDataProcessor(std::function<bool(const DataItem&)> processor)
{
    if (processor) {
        dataProcessor_ = processor;
    }
}

void ProducerConsumerManager::setBatchProcessor(std::function<bool(const QList<DataItem>&)> processor)
{
    if (processor) {
        batchProcessor_ = processor;
    }
}

bool ProducerConsumerManager::produceData(const QByteArray& data, const QString& type, quint32 priority)
{
    if (data.isEmpty()) {
        return false;
    }

    DataItem item(data, type, priority);

    // 检查队列是否已满
    if (dataQueue_->full()) {
        droppedCount_++;
        emit queueOverflow(1);
        return false;
    }

    // 将数据推入队列
    bool success = dataQueue_->push(item);
    if (success) {
        totalProduced_++;
        updateFlowControl(dataQueue_->size());
    } else {
        droppedCount_++;
    }

    return success;
}

bool ProducerConsumerManager::produceDataBatch(const QList<DataItem>& items)
{
    if (items.isEmpty()) {
        return true;
    }

    size_t dropped = 0;
    size_t produced = 0;

    for (const auto& item : items) {
        if (dataQueue_->full()) {
            dropped++;
            continue;
        }

        if (dataQueue_->push(item)) {
            produced++;
        } else {
            dropped++;
        }
    }

    totalProduced_ += produced;
    droppedCount_ += dropped;

    if (dropped > 0) {
        emit queueOverflow(dropped);
    }

    updateFlowControl(dataQueue_->size());
    return dropped == 0;
}

void ProducerConsumerManager::startConsumers()
{
    if (running_.load()) {
        return;
    }

    running_.store(true);
    paused_.store(false);
    stopping_.store(false);

    // 启动消费者线程
    if (!consumerThread_->isRunning()) {
        consumerThread_->start();
    }

    // 启动统计定时器
    statisticsTimer_->start();

    qDebug() << "Producer-Consumer manager started";
}

void ProducerConsumerManager::stopConsumers()
{
    if (!running_.load()) {
        return;
    }

    stopping_.store(true);
    running_.store(false);

    // 停止定时器
    statisticsTimer_->stop();

    // 停止消费者线程
    if (consumerThread_ && consumerThread_->isRunning()) {
        consumerThread_->quit();
        consumerThread_->wait(3000);
    }

    qDebug() << "Producer-Consumer manager stopped";
}

void ProducerConsumerManager::pauseConsumers()
{
    paused_.store(true);
    qDebug() << "Producer-Consumer manager paused";
}

void ProducerConsumerManager::resumeConsumers()
{
    paused_.store(false);
    qDebug() << "Producer-Consumer manager resumed";
}

size_t ProducerConsumerManager::getQueueSize() const
{
    return dataQueue_ ? dataQueue_->size() : 0;
}

ProducerConsumerManager::Statistics ProducerConsumerManager::getStatistics() const
{
    QMutexLocker locker(&statisticsMutex_);
    return currentStats_;
}

void ProducerConsumerManager::resetStatistics()
{
    QMutexLocker locker(&statisticsMutex_);
    processedCount_.store(0);
    droppedCount_.store(0);
    totalProduced_.store(0);
    currentStats_ = Statistics{};

    QMutexLocker perfLocker(&performanceMutex_);
    processingTimes_.clear();
}

void ProducerConsumerManager::processData()
{
    if (!running_.load() || paused_.load() || stopping_.load()) {
        return;
    }

    quint64 startTime = QDateTime::currentMSecsSinceEpoch();

    try {
        if (strategy_ == ProcessingStrategy::BATCH) {
            // 批量处理
            auto batch = extractBatch();
            if (!batch.isEmpty()) {
                bool success = processBatchItems(batch);
                if (success) {
                    processedCount_ += batch.size();
                } else {
                    emit processingError("Batch processing failed", "batch");
                }
            }
        } else {
            // 单项处理
            DataItem item;
            if (dataQueue_->pop(item)) {
                bool success = processDataItem(item);
                if (success) {
                    processedCount_++;
                } else {
                    emit processingError("Item processing failed", item.type);
                }
            }
        }
    } catch (const std::exception& e) {
        emit processingError(QString("Processing exception: %1").arg(e.what()), "unknown");
    }

    quint64 processingTime = QDateTime::currentMSecsSinceEpoch() - startTime;
    recordProcessingTime(processingTime);
}

void ProducerConsumerManager::updateStatistics()
{
    QMutexLocker locker(&statisticsMutex_);

    currentStats_.totalProduced = totalProduced_.load();
    currentStats_.totalConsumed = processedCount_.load();
    currentStats_.totalDropped = droppedCount_.load();
    currentStats_.currentQueueSize = getQueueSize();
    currentStats_.lastProcessTime = QDateTime::currentDateTime();

    // 计算平均处理时间
    QMutexLocker perfLocker(&performanceMutex_);
    if (!processingTimes_.isEmpty()) {
        quint64 sum = 0;
        for (auto time : processingTimes_) {
            sum += time;
        }
        currentStats_.averageProcessingTime = static_cast<double>(sum) / processingTimes_.size();
    }

    emit performanceReport(currentStats_);
}

bool ProducerConsumerManager::processDataItem(const DataItem& item)
{
    if (!dataProcessor_) {
        return false;
    }

    return dataProcessor_(item);
}

bool ProducerConsumerManager::processBatchItems(const QList<DataItem>& items)
{
    if (!batchProcessor_) {
        return false;
    }

    return batchProcessor_(items);
}

QList<DataItem> ProducerConsumerManager::extractBatch()
{
    QList<DataItem> batch;
    batch.reserve(flowConfig_.maxBatchSize);

    for (int i = 0; i < flowConfig_.maxBatchSize && !dataQueue_->empty(); ++i) {
        DataItem item;
        if (dataQueue_->pop(item)) {
            batch.append(item);
        } else {
            break;
        }
    }

    // 根据策略排序
    if (strategy_ == ProcessingStrategy::PRIORITY) {
        std::sort(batch.begin(), batch.end(), comparePriority);
    } else if (strategy_ == ProcessingStrategy::LIFO) {
        std::reverse(batch.begin(), batch.end());
    }

    return batch;
}

void ProducerConsumerManager::updateFlowControl(size_t currentSize)
{
    static size_t lastReportedLevel = 0;

    if (currentSize >= flowConfig_.highWaterMark && lastReportedLevel < flowConfig_.highWaterMark) {
        QMutexLocker locker(&statisticsMutex_);
        currentStats_.highWaterMarkHits++;
        emit highWaterMarkReached(currentSize);
    } else if (currentSize <= flowConfig_.lowWaterMark && lastReportedLevel > flowConfig_.lowWaterMark) {
        emit lowWaterMarkReached(currentSize);
    }

    lastReportedLevel = currentSize;
}

void ProducerConsumerManager::recordProcessingTime(quint64 timeMs)
{
    QMutexLocker locker(&performanceMutex_);
    processingTimes_.append(timeMs);

    // 保持最近1000个记录
    if (processingTimes_.size() > 1000) {
        processingTimes_.removeFirst();
    }
}

bool ProducerConsumerManager::comparePriority(const DataItem& a, const DataItem& b)
{
    return a.priority > b.priority; // 高优先级优先
}

// =============================================================================
// ProtocolDataManager 实现
// =============================================================================

ProtocolDataManager::ProtocolDataManager(QObject *parent)
    : ProducerConsumerManager(parent)
{
    initializeProtocolProcessors();
}

void ProtocolDataManager::initializeProtocolProcessors()
{
    // 设置协议数据处理器
    setDataProcessor([this](const DataItem& item) -> bool {
        return handleProtocolData(item);
    });

    // 配置适合协议处理的参数
    FlowControlConfig config;
    config.maxQueueSize = 5000;
    config.highWaterMark = 4000;
    config.lowWaterMark = 1000;
    config.maxBatchSize = 50;
    config.processingIntervalMs = 5; // 更快的处理间隔
    setFlowControlConfig(config);

    // 设置FIFO策略，确保协议数据顺序
    setProcessingStrategy(ProcessingStrategy::FIFO);
}

bool ProtocolDataManager::produceIncomingData(const QByteArray& rawData)
{
    return produceData(rawData, "incoming", 10);
}

bool ProtocolDataManager::produceOutgoingData(const QByteArray& protocolData, quint32 priority)
{
    return produceData(protocolData, "outgoing", priority);
}

bool ProtocolDataManager::produceControlData(const QByteArray& controlData, quint32 priority)
{
    return produceData(controlData, "control", priority);
}

void ProtocolDataManager::setIncomingDataHandler(std::function<bool(const QByteArray&)> handler)
{
    incomingHandler_ = handler;
}

void ProtocolDataManager::setOutgoingDataHandler(std::function<bool(const QByteArray&)> handler)
{
    outgoingHandler_ = handler;
}

void ProtocolDataManager::setControlDataHandler(std::function<bool(const QByteArray&)> handler)
{
    controlHandler_ = handler;
}

bool ProtocolDataManager::handleProtocolData(const DataItem& item)
{
    try {
        if (item.type == "incoming") {
            if (incomingHandler_) {
                bool success = incomingHandler_(item.data);
                emit incomingDataReady(item.data);
                return success;
            }
        } else if (item.type == "outgoing") {
            if (outgoingHandler_) {
                bool success = outgoingHandler_(item.data);
                emit outgoingDataProcessed(success);
                return success;
            }
        } else if (item.type == "control") {
            if (controlHandler_) {
                bool success = controlHandler_(item.data);
                emit controlDataExecuted(success);
                return success;
            }
        }

        // 默认处理
        emit dataProcessed(item.type, item.timestamp);
        return true;

    } catch (const std::exception& e) {
        emit protocolError(QString("Protocol processing error: %1").arg(e.what()));
        return false;
    }
}

} // namespace Buffer
} // namespace Protocol

#include "producer_consumer_manager.moc"
