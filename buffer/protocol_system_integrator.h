#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include <functional>

#include "producer_consumer_manager.h"
#include "protocol_buffer_adapter.h"

// 前向声明
class ProtocolAdapter;  // 全局命名空间中
namespace Protocol {
    class ProtocolAdapterRefactored;  // Protocol命名空间中
    class ConnectionManager;          // Protocol命名空间中
}

namespace Protocol {
namespace Buffer {

/**
 * @brief 协议系统集成器
 *
 * 将生产者消费者模型集成到现有的协议系统中，
 * 提供统一的数据流管理和处理
 */
class ProtocolSystemIntegrator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 集成配置
     */
    struct IntegrationConfig {
        bool enableLegacyBuffer = true;         // 启用传统缓冲区兼容
        bool enableProducerConsumer = true;     // 启用生产者消费者模型
        bool enableDataForwarding = true;       // 启用数据转发
        bool enableStatisticsReporting = true;  // 启用统计报告
        int statisticsReportInterval = 5000;    // 统计报告间隔（毫秒）
    };

    explicit ProtocolSystemIntegrator(QObject *parent = nullptr);
    ~ProtocolSystemIntegrator();

    // === 组件集成 ===
    void integrateProtocolAdapter(ProtocolAdapter* adapter);
    void integrateProtocolAdapterRefactored(Protocol::ProtocolAdapterRefactored* adapter);
    void integrateConnectionManager(Protocol::ConnectionManager* connectionManager);
    void integrateBufferAdapter(ProtocolBufferAdapter* bufferAdapter);

    // === 配置接口 ===
    void setIntegrationConfig(const IntegrationConfig& config);
    IntegrationConfig getIntegrationConfig() const { return config_; }

    // === 数据处理器设置 ===
    void setIncomingDataProcessor(std::function<void(const QByteArray&)> processor);
    void setOutgoingDataProcessor(std::function<bool(const QByteArray&)> processor);
    void setErrorHandler(std::function<void(const QString&)> handler);

    // === 控制接口 ===
    void startIntegration();
    void stopIntegration();
    void pauseProcessing();
    void resumeProcessing();

    // === 统计接口 ===
    struct IntegratedStatistics {
        ProducerConsumerManager::Statistics producerConsumerStats;
        ProtocolBufferAdapter::ProtocolStats bufferStats;
        struct {
            size_t totalDataReceived = 0;
            size_t totalDataSent = 0;
            size_t totalErrors = 0;
            double averageLatency = 0.0;
        } systemStats;
    };

    IntegratedStatistics getIntegratedStatistics() const;
    void resetStatistics();

    // === 访问器 ===
    ProtocolDataManager* getDataManager() const;
    ProtocolBufferAdapter* getBufferAdapter() const { return bufferAdapter_; }

signals:
    // === 数据流信号 ===
    void incomingDataReceived(const QByteArray& data);
    void outgoingDataSent(const QByteArray& data, bool success);
    void dataProcessingError(const QString& error);

    // === 系统状态信号 ===
    void integrationStarted();
    void integrationStopped();
    void processingPaused();
    void processingResumed();

    // === 性能监控信号 ===
    void statisticsReport(const IntegratedStatistics& stats);
    void performanceWarning(const QString& warning);

private slots:
    // === 协议适配器信号处理 ===
    void handleProtocolDataReceived(const QByteArray& data);
    void handleProtocolDataSent(const QByteArray& data);
    void handleProtocolError(const QString& error);
    void handleConnectionStatusChanged(bool connected);

    // === 连接管理器信号处理 ===
    void handleConnectionDataReceived(const QByteArray& data);
    void handleConnectionDataSent(bool success, int bytesWritten);
    void handleConnectionError(const QString& error);

    // === 缓冲适配器信号处理 ===
    void handleBufferPacketPushed(const QString& messageType, int dataSize);
    void handleBufferOverflow(const QString& messageType, int droppedDataSize);

    // === 生产者消费者信号处理 ===
    void handleDataProcessed(const QString& type, quint64 timestamp);
    void handleProcessingError(const QString& error, const QString& dataType);
    void handlePerformanceReport(const ProducerConsumerManager::Statistics& stats);

    // === 统计报告 ===
    void generateStatisticsReport();

private:
    // === 组件引用 ===
    ProtocolAdapter* protocolAdapter_;
    Protocol::ProtocolAdapterRefactored* protocolAdapterRefactored_;
    Protocol::ConnectionManager* connectionManager_;
    ProtocolBufferAdapter* bufferAdapter_;

    // === 生产者消费者管理器 ===
    std::unique_ptr<ProtocolDataManager> dataManager_;

    // === 配置 ===
    IntegrationConfig config_;

    // === 数据处理器 ===
    std::function<void(const QByteArray&)> incomingProcessor_;
    std::function<bool(const QByteArray&)> outgoingProcessor_;
    std::function<void(const QString&)> errorHandler_;

    // === 统计信息 ===
    mutable QMutex statisticsMutex_;
    IntegratedStatistics currentStats_;
    QTimer* statisticsTimer_;

    // === 状态 ===
    bool integrationStarted_;
    bool processingPaused_;

    // === 私有方法 ===
    void initializeComponents();
    void setupConnections();
    void setupDefaultProcessors();
    void connectProtocolAdapter();
    void connectProtocolAdapterRefactored();
    void connectConnectionManager();
    void connectBufferAdapter();
    void connectDataManager();

    void updateSystemStatistics();
    void forwardDataToLegacyBuffer(const QByteArray& data, const QString& type);
    void processIncomingData(const QByteArray& data);
    bool processOutgoingData(const QByteArray& data);
    void handleSystemError(const QString& error);
};

/**
 * @brief 协议系统集成器工厂
 */
class ProtocolSystemIntegratorFactory
{
public:
    /**
     * @brief 创建标准集成器
     * @param parent 父对象
     * @return 集成器实例
     */
    static std::unique_ptr<ProtocolSystemIntegrator> createStandardIntegrator(QObject* parent = nullptr);

    /**
     * @brief 创建高性能集成器
     * @param parent 父对象
     * @return 集成器实例
     */
    static std::unique_ptr<ProtocolSystemIntegrator> createHighPerformanceIntegrator(QObject* parent = nullptr);

    /**
     * @brief 创建兼容性集成器（仅传统缓冲区）
     * @param parent 父对象
     * @return 集成器实例
     */
    static std::unique_ptr<ProtocolSystemIntegrator> createCompatibilityIntegrator(QObject* parent = nullptr);
};

} // namespace Buffer
} // namespace Protocol
