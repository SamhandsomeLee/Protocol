#include "protocol_system_integrator.h"
#include "../adapter/protocol_adapter.h"
#include "../adapter/protocol_adapter_refactored.h"
#include "../connection/connection_manager.h"

#include <QDebug>
#include <QDateTime>
#include <QMetaObject>

namespace Protocol {
namespace Buffer {

// =============================================================================
// ProtocolSystemIntegrator 实现
// =============================================================================

ProtocolSystemIntegrator::ProtocolSystemIntegrator(QObject *parent)
    : QObject(parent)
    , protocolAdapter_(nullptr)
    , protocolAdapterRefactored_(nullptr)
    , connectionManager_(nullptr)
    , bufferAdapter_(nullptr)
    , integrationStarted_(false)
    , processingPaused_(false)
{
    initializeComponents();
}

ProtocolSystemIntegrator::~ProtocolSystemIntegrator()
{
    stopIntegration();
}

void ProtocolSystemIntegrator::initializeComponents()
{
    // 创建生产者消费者管理器
    dataManager_ = std::make_unique<ProtocolDataManager>(this);

    // 创建统计定时器
    statisticsTimer_ = new QTimer(this);
    statisticsTimer_->setInterval(config_.statisticsReportInterval);
    connect(statisticsTimer_, &QTimer::timeout, this, &ProtocolSystemIntegrator::generateStatisticsReport);

    // 设置默认配置
    config_ = IntegrationConfig{};

    // 设置默认处理器
    setupDefaultProcessors();

    qDebug() << "ProtocolSystemIntegrator initialized";
}

void ProtocolSystemIntegrator::setupDefaultProcessors()
{
    // 默认入站数据处理器
    incomingProcessor_ = [this](const QByteArray& data) {
        qDebug() << "Processing incoming data:" << data.size() << "bytes";

        // 转发到传统缓冲区（如果启用）
        if (config_.enableLegacyBuffer && bufferAdapter_) {
            forwardDataToLegacyBuffer(data, "incoming");
        }

        emit incomingDataReceived(data);
    };

    // 默认出站数据处理器
    outgoingProcessor_ = [this](const QByteArray& data) -> bool {
        qDebug() << "Processing outgoing data:" << data.size() << "bytes";

        // 转发到传统缓冲区（如果启用）
        if (config_.enableLegacyBuffer && bufferAdapter_) {
            forwardDataToLegacyBuffer(data, "outgoing");
        }

        // 模拟发送成功
        emit outgoingDataSent(data, true);
        return true;
    };

    // 默认错误处理器
    errorHandler_ = [this](const QString& error) {
        qWarning() << "Protocol system error:" << error;
        handleSystemError(error);
    };
}

void ProtocolSystemIntegrator::setIntegrationConfig(const IntegrationConfig& config)
{
    config_ = config;

    // 更新统计定时器间隔
    if (statisticsTimer_) {
        statisticsTimer_->setInterval(config_.statisticsReportInterval);
    }

    qDebug() << "Integration config updated";
}

void ProtocolSystemIntegrator::integrateProtocolAdapter(Protocol::Adapter::ProtocolAdapter* adapter)
{
    if (protocolAdapter_ == adapter) {
        return;
    }

    protocolAdapter_ = adapter;
    if (protocolAdapter_) {
        connectProtocolAdapter();
        qDebug() << "ProtocolAdapter integrated";
    }
}

void ProtocolSystemIntegrator::integrateProtocolAdapterRefactored(Protocol::Adapter::ProtocolAdapterRefactored* adapter)
{
    if (protocolAdapterRefactored_ == adapter) {
        return;
    }

    protocolAdapterRefactored_ = adapter;
    if (protocolAdapterRefactored_) {
        connectProtocolAdapterRefactored();
        qDebug() << "ProtocolAdapterRefactored integrated";
    }
}

void ProtocolSystemIntegrator::integrateConnectionManager(Protocol::Connection::ConnectionManager* connectionManager)
{
    if (connectionManager_ == connectionManager) {
        return;
    }

    connectionManager_ = connectionManager;
    if (connectionManager_) {
        connectConnectionManager();
        qDebug() << "ConnectionManager integrated";
    }
}

void ProtocolSystemIntegrator::integrateBufferAdapter(ProtocolBufferAdapter* bufferAdapter)
{
    if (bufferAdapter_ == bufferAdapter) {
        return;
    }

    bufferAdapter_ = bufferAdapter;
    if (bufferAdapter_) {
        connectBufferAdapter();
        qDebug() << "ProtocolBufferAdapter integrated";
    }
}

void ProtocolSystemIntegrator::setIncomingDataProcessor(std::function<void(const QByteArray&)> processor)
{
    if (processor) {
        incomingProcessor_ = processor;

        // 设置到数据管理器
        if (dataManager_) {
            dataManager_->setIncomingDataHandler([this](const QByteArray& data) -> bool {
                processIncomingData(data);
                return true;
            });
        }
    }
}

void ProtocolSystemIntegrator::setOutgoingDataProcessor(std::function<bool(const QByteArray&)> processor)
{
    if (processor) {
        outgoingProcessor_ = processor;

        // 设置到数据管理器
        if (dataManager_) {
            dataManager_->setOutgoingDataHandler([this](const QByteArray& data) -> bool {
                return processOutgoingData(data);
            });
        }
    }
}

void ProtocolSystemIntegrator::setErrorHandler(std::function<void(const QString&)> handler)
{
    if (handler) {
        errorHandler_ = handler;
    }
}

void ProtocolSystemIntegrator::startIntegration()
{
    if (integrationStarted_) {
        return;
    }

    // 启动生产者消费者管理器
    if (dataManager_ && config_.enableProducerConsumer) {
        setupConnections();
        dataManager_->startConsumers();
    }

    // 启动统计报告
    if (config_.enableStatisticsReporting) {
        statisticsTimer_->start();
    }

    integrationStarted_ = true;
    processingPaused_ = false;

    qInfo() << "Protocol system integration started";
    emit integrationStarted();
}

void ProtocolSystemIntegrator::stopIntegration()
{
    if (!integrationStarted_) {
        return;
    }

    // 停止统计报告
    statisticsTimer_->stop();

    // 停止生产者消费者管理器
    if (dataManager_) {
        dataManager_->stopConsumers();
    }

    integrationStarted_ = false;
    processingPaused_ = false;

    qInfo() << "Protocol system integration stopped";
    emit integrationStopped();
}

void ProtocolSystemIntegrator::pauseProcessing()
{
    if (!integrationStarted_ || processingPaused_) {
        return;
    }

    if (dataManager_) {
        dataManager_->pauseConsumers();
    }

    processingPaused_ = true;

    qInfo() << "Protocol processing paused";
    emit processingPaused();
}

void ProtocolSystemIntegrator::resumeProcessing()
{
    if (!integrationStarted_ || !processingPaused_) {
        return;
    }

    if (dataManager_) {
        dataManager_->resumeConsumers();
    }

    processingPaused_ = false;

    qInfo() << "Protocol processing resumed";
    emit processingResumed();
}

ProtocolSystemIntegrator::IntegratedStatistics ProtocolSystemIntegrator::getIntegratedStatistics() const
{
    QMutexLocker locker(&statisticsMutex_);

    IntegratedStatistics stats = currentStats_;

    if (dataManager_) {
        stats.producerConsumerStats = dataManager_->getStatistics();
    }

    if (bufferAdapter_) {
        stats.bufferStats = bufferAdapter_->getProtocolStats();
    }

    return stats;
}

void ProtocolSystemIntegrator::resetStatistics()
{
    QMutexLocker locker(&statisticsMutex_);

    currentStats_ = IntegratedStatistics{};

    if (dataManager_) {
        dataManager_->resetStatistics();
    }

    qDebug() << "Statistics reset";
}

ProtocolDataManager* ProtocolSystemIntegrator::getDataManager() const
{
    return dataManager_.get();
}

void ProtocolSystemIntegrator::setupConnections()
{
    if (!dataManager_) {
        return;
    }

    connectDataManager();
}

void ProtocolSystemIntegrator::connectProtocolAdapter()
{
    if (!protocolAdapter_) {
        return;
    }

    // 连接数据接收信号
    connect(protocolAdapter_, &Protocol::Adapter::ProtocolAdapter::dataReceived,
            this, &ProtocolSystemIntegrator::handleProtocolDataReceived);

    // 连接错误信号
    connect(protocolAdapter_, &Protocol::Adapter::ProtocolAdapter::communicationError,
            this, &ProtocolSystemIntegrator::handleProtocolError);

    // 连接连接状态变化信号
    connect(protocolAdapter_, &Protocol::Adapter::ProtocolAdapter::connectionStatusChanged,
            this, &ProtocolSystemIntegrator::handleConnectionStatusChanged);
}

void ProtocolSystemIntegrator::connectProtocolAdapterRefactored()
{
    if (!protocolAdapterRefactored_) {
        return;
    }

    // 连接数据接收信号
    connect(protocolAdapterRefactored_, &Protocol::Adapter::ProtocolAdapterRefactored::dataReceived,
            this, &ProtocolSystemIntegrator::handleProtocolDataReceived);

    // 连接错误信号
    connect(protocolAdapterRefactored_, &Protocol::Adapter::ProtocolAdapterRefactored::communicationError,
            this, &ProtocolSystemIntegrator::handleProtocolError);

    // 连接连接状态变化信号
    connect(protocolAdapterRefactored_, &Protocol::Adapter::ProtocolAdapterRefactored::connectionStatusChanged,
            this, &ProtocolSystemIntegrator::handleConnectionStatusChanged);
}

void ProtocolSystemIntegrator::connectConnectionManager()
{
    if (!connectionManager_) {
        return;
    }

    // 连接数据接收信号
    connect(connectionManager_, &Protocol::Connection::ConnectionManager::dataReceived,
            this, &ProtocolSystemIntegrator::handleConnectionDataReceived);

    // 连接数据发送信号
    connect(connectionManager_, &Protocol::Connection::ConnectionManager::dataSent,
            this, &ProtocolSystemIntegrator::handleConnectionDataSent);

    // 连接错误信号
    connect(connectionManager_, &Protocol::Connection::ConnectionManager::communicationError,
            this, &ProtocolSystemIntegrator::handleConnectionError);

    // 连接连接状态变化信号
    connect(connectionManager_, &Protocol::Connection::ConnectionManager::connectionStatusChanged,
            this, &ProtocolSystemIntegrator::handleConnectionStatusChanged);
}

void ProtocolSystemIntegrator::connectBufferAdapter()
{
    if (!bufferAdapter_) {
        return;
    }

    // 连接缓冲区信号
    connect(bufferAdapter_, &ProtocolBufferAdapter::packetPushed,
            this, &ProtocolSystemIntegrator::handleBufferPacketPushed);

    connect(bufferAdapter_, &ProtocolBufferAdapter::bufferOverflow,
            this, &ProtocolSystemIntegrator::handleBufferOverflow);
}

void ProtocolSystemIntegrator::connectDataManager()
{
    if (!dataManager_) {
        return;
    }

    // 连接生产者消费者信号
    connect(dataManager_.get(), &ProtocolDataManager::dataProcessed,
            this, &ProtocolSystemIntegrator::handleDataProcessed);

    connect(dataManager_.get(), &ProtocolDataManager::processingError,
            this, &ProtocolSystemIntegrator::handleProcessingError);

    connect(dataManager_.get(), &ProtocolDataManager::performanceReport,
            this, &ProtocolSystemIntegrator::handlePerformanceReport);

    // 设置数据处理器
    dataManager_->setIncomingDataHandler([this](const QByteArray& data) -> bool {
        processIncomingData(data);
        return true;
    });

    dataManager_->setOutgoingDataHandler([this](const QByteArray& data) -> bool {
        return processOutgoingData(data);
    });
}

// === 信号处理槽函数 ===

void ProtocolSystemIntegrator::handleProtocolDataReceived(const QByteArray& data)
{
    if (config_.enableProducerConsumer && dataManager_) {
        dataManager_->produceIncomingData(data);
    } else {
        processIncomingData(data);
    }

    // 更新统计
    QMutexLocker locker(&statisticsMutex_);
    currentStats_.systemStats.totalDataReceived += data.size();
}

void ProtocolSystemIntegrator::handleProtocolDataSent(const QByteArray& data)
{
    emit outgoingDataSent(data, true);

    // 更新统计
    QMutexLocker locker(&statisticsMutex_);
    currentStats_.systemStats.totalDataSent += data.size();
}

void ProtocolSystemIntegrator::handleProtocolError(const QString& error)
{
    handleSystemError(error);
}

void ProtocolSystemIntegrator::handleConnectionStatusChanged(bool connected)
{
    qInfo() << "Connection status changed:" << connected;

    if (!connected) {
        // 连接断开时，可以选择暂停处理
        if (config_.enableProducerConsumer && integrationStarted_) {
            pauseProcessing();
        }
    } else {
        // 连接恢复时，恢复处理
        if (config_.enableProducerConsumer && integrationStarted_ && processingPaused_) {
            resumeProcessing();
        }
    }
}

void ProtocolSystemIntegrator::handleConnectionDataReceived(const QByteArray& data)
{
    handleProtocolDataReceived(data);
}

void ProtocolSystemIntegrator::handleConnectionDataSent(bool success, int bytesWritten)
{
    if (success) {
        QMutexLocker locker(&statisticsMutex_);
        currentStats_.systemStats.totalDataSent += bytesWritten;
    } else {
        handleSystemError("Data transmission failed");
    }
}

void ProtocolSystemIntegrator::handleConnectionError(const QString& error)
{
    handleSystemError(error);
}

void ProtocolSystemIntegrator::handleBufferPacketPushed(const QString& messageType, int dataSize)
{
    Q_UNUSED(messageType)
    Q_UNUSED(dataSize)
    // 可以添加缓冲区相关的统计
}

void ProtocolSystemIntegrator::handleBufferOverflow(const QString& messageType, int droppedDataSize)
{
    QString warning = QString("Buffer overflow for message type %1, dropped %2 bytes")
                        .arg(messageType).arg(droppedDataSize);
    emit performanceWarning(warning);
}

void ProtocolSystemIntegrator::handleDataProcessed(const QString& type, quint64 timestamp)
{
    Q_UNUSED(type)
    Q_UNUSED(timestamp)
    // 数据处理完成的通知
}

void ProtocolSystemIntegrator::handleProcessingError(const QString& error, const QString& dataType)
{
    QString fullError = QString("Processing error for %1: %2").arg(dataType, error);
    handleSystemError(fullError);
}

void ProtocolSystemIntegrator::handlePerformanceReport(const ProducerConsumerManager::Statistics& stats)
{
    Q_UNUSED(stats)
    // 性能报告处理
}

void ProtocolSystemIntegrator::generateStatisticsReport()
{
    if (!config_.enableStatisticsReporting) {
        return;
    }

    auto stats = getIntegratedStatistics();
    emit statisticsReport(stats);
}

// === 私有方法 ===

void ProtocolSystemIntegrator::forwardDataToLegacyBuffer(const QByteArray& data, const QString& type)
{
    if (!bufferAdapter_) {
        return;
    }

    bufferAdapter_->pushPacket(data, type);
}

void ProtocolSystemIntegrator::processIncomingData(const QByteArray& data)
{
    if (incomingProcessor_) {
        incomingProcessor_(data);
    }
}

bool ProtocolSystemIntegrator::processOutgoingData(const QByteArray& data)
{
    if (outgoingProcessor_) {
        return outgoingProcessor_(data);
    }
    return false;
}

void ProtocolSystemIntegrator::handleSystemError(const QString& error)
{
    QMutexLocker locker(&statisticsMutex_);
    currentStats_.systemStats.totalErrors++;

    if (errorHandler_) {
        errorHandler_(error);
    }

    emit dataProcessingError(error);
}

// =============================================================================
// ProtocolSystemIntegratorFactory 实现
// =============================================================================

std::unique_ptr<ProtocolSystemIntegrator> ProtocolSystemIntegratorFactory::createStandardIntegrator(QObject* parent)
{
    auto integrator = std::make_unique<ProtocolSystemIntegrator>(parent);

    // 标准配置
    ProtocolSystemIntegrator::IntegrationConfig config;
    config.enableLegacyBuffer = true;
    config.enableProducerConsumer = true;
    config.enableDataForwarding = true;
    config.enableStatisticsReporting = true;
    config.statisticsReportInterval = 5000;

    integrator->setIntegrationConfig(config);

    return integrator;
}

std::unique_ptr<ProtocolSystemIntegrator> ProtocolSystemIntegratorFactory::createHighPerformanceIntegrator(QObject* parent)
{
    auto integrator = std::make_unique<ProtocolSystemIntegrator>(parent);

    // 高性能配置
    ProtocolSystemIntegrator::IntegrationConfig config;
    config.enableLegacyBuffer = false;     // 禁用传统缓冲区以提高性能
    config.enableProducerConsumer = true;
    config.enableDataForwarding = false;   // 禁用数据转发以减少开销
    config.enableStatisticsReporting = true;
    config.statisticsReportInterval = 1000; // 更频繁的统计报告

    integrator->setIntegrationConfig(config);

    return integrator;
}

std::unique_ptr<ProtocolSystemIntegrator> ProtocolSystemIntegratorFactory::createCompatibilityIntegrator(QObject* parent)
{
    auto integrator = std::make_unique<ProtocolSystemIntegrator>(parent);

    // 兼容性配置
    ProtocolSystemIntegrator::IntegrationConfig config;
    config.enableLegacyBuffer = true;
    config.enableProducerConsumer = false;  // 禁用生产者消费者，只使用传统缓冲区
    config.enableDataForwarding = true;
    config.enableStatisticsReporting = false;

    integrator->setIntegrationConfig(config);

    return integrator;
}

} // namespace Buffer
} // namespace Protocol

#include "protocol_system_integrator.moc"
