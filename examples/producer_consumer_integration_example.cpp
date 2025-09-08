/**
 * @file producer_consumer_integration_example.cpp
 * @brief 协议系统生产者消费者模型集成示例
 *
 * 本文件展示如何在现有协议系统中集成生产者消费者模型，
 * 包括配置、使用和监控
 */

#include "buffer/protocol_system_integrator.h"
#include "adapter/protocol_adapter.h"
#include "adapter/protocol_adapter_refactored.h"
#include "connection/connection_manager.h"
#include "buffer/protocol_buffer_adapter.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <memory>

using namespace Protocol::Buffer;
using namespace Protocol::Adapter;
using namespace Protocol::Connection;

class IntegrationExample : public QObject
{
    Q_OBJECT

public:
    IntegrationExample(QObject* parent = nullptr) : QObject(parent)
    {
        setupComponents();
        setupIntegration();
        connectSignals();
    }

    void runExample()
    {
        qInfo() << "=== Starting Producer-Consumer Integration Example ===";

        // 启动集成系统
        integrator_->startIntegration();

        // 模拟数据生产
        simulateDataProduction();

        // 设置定时器来展示统计信息
        auto statsTimer = new QTimer(this);
        connect(statsTimer, &QTimer::timeout, this, &IntegrationExample::showStatistics);
        statsTimer->start(2000);

        qInfo() << "Example started. Use Ctrl+C to stop.";
    }

private slots:
    void onIncomingDataReceived(const QByteArray& data)
    {
        qDebug() << "Received incoming data:" << data.size() << "bytes" << data.left(20).toHex();
    }

    void onOutgoingDataSent(const QByteArray& data, bool success)
    {
        qDebug() << "Outgoing data" << (success ? "sent successfully" : "failed")
                 << ":" << data.size() << "bytes";
    }

    void onDataProcessingError(const QString& error)
    {
        qWarning() << "Data processing error:" << error;
    }

    void onStatisticsReport(const ProtocolSystemIntegrator::IntegratedStatistics& stats)
    {
        qInfo() << "=== Performance Statistics ===";
        qInfo() << "Producer-Consumer Stats:";
        qInfo() << "  Total Produced:" << stats.producerConsumerStats.totalProduced;
        qInfo() << "  Total Consumed:" << stats.producerConsumerStats.totalConsumed;
        qInfo() << "  Total Dropped:" << stats.producerConsumerStats.totalDropped;
        qInfo() << "  Current Queue Size:" << stats.producerConsumerStats.currentQueueSize;
        qInfo() << "  Average Processing Time:" << stats.producerConsumerStats.averageProcessingTime << "ms";

        qInfo() << "System Stats:";
        qInfo() << "  Total Data Received:" << stats.systemStats.totalDataReceived;
        qInfo() << "  Total Data Sent:" << stats.systemStats.totalDataSent;
        qInfo() << "  Total Errors:" << stats.systemStats.totalErrors;
    }

    void showStatistics()
    {
        auto stats = integrator_->getIntegratedStatistics();
        qInfo() << "\n=== Current Statistics ===";
        qInfo() << "Queue Size:" << stats.producerConsumerStats.currentQueueSize;
        qInfo() << "Processed:" << stats.producerConsumerStats.totalConsumed;
        qInfo() << "Dropped:" << stats.producerConsumerStats.totalDropped;

        if (stats.producerConsumerStats.currentQueueSize > 100) {
            qWarning() << "High queue size detected, consider adjusting processing parameters";
        }
    }

private:
    void setupComponents()
    {
        // 创建协议适配器
        protocolAdapter_ = std::make_unique<ProtocolAdapter>(this);

        // 创建连接管理器
        connectionManager_ = std::make_unique<ConnectionManager>(this);

        // 创建缓冲适配器
        bufferAdapter_ = std::make_unique<ProtocolBufferAdapter>(2048, this);

        qDebug() << "Protocol components created";
    }

    void setupIntegration()
    {
        // 创建标准集成器
        integrator_ = ProtocolSystemIntegratorFactory::createStandardIntegrator(this);

        // 集成各个组件
        integrator_->integrateProtocolAdapter(protocolAdapter_.get());
        integrator_->integrateConnectionManager(connectionManager_.get());
        integrator_->integrateBufferAdapter(bufferAdapter_.get());

        // 设置自定义数据处理器
        setupCustomProcessors();

        qDebug() << "Integration setup completed";
    }

    void setupCustomProcessors()
    {
        // 设置入站数据处理器
        integrator_->setIncomingDataProcessor([this](const QByteArray& data) {
            // 自定义处理逻辑
            if (data.size() > 1024) {
                qDebug() << "Processing large incoming packet:" << data.size() << "bytes";
            }

            // 模拟数据解析
            processIncomingProtocolData(data);

            emit dataProcessed("incoming", data.size());
        });

        // 设置出站数据处理器
        integrator_->setOutgoingDataProcessor([this](const QByteArray& data) -> bool {
            // 自定义发送逻辑
            qDebug() << "Sending data:" << data.size() << "bytes";

            // 模拟发送延迟
            QThread::msleep(1);

            return simulateDataTransmission(data);
        });

        // 设置错误处理器
        integrator_->setErrorHandler([this](const QString& error) {
            qCritical() << "System Error:" << error;

            // 可以在这里实现错误恢复逻辑
            if (error.contains("overflow")) {
                qInfo() << "Attempting to recover from buffer overflow...";
                integrator_->getDataManager()->resetStatistics();
            }
        });
    }

    void connectSignals()
    {
        // 连接集成器信号
        connect(integrator_.get(), &ProtocolSystemIntegrator::incomingDataReceived,
                this, &IntegrationExample::onIncomingDataReceived);

        connect(integrator_.get(), &ProtocolSystemIntegrator::outgoingDataSent,
                this, &IntegrationExample::onOutgoingDataSent);

        connect(integrator_.get(), &ProtocolSystemIntegrator::dataProcessingError,
                this, &IntegrationExample::onDataProcessingError);

        connect(integrator_.get(), &ProtocolSystemIntegrator::statisticsReport,
                this, &IntegrationExample::onStatisticsReport);

        // 连接性能警告
        connect(integrator_.get(), &ProtocolSystemIntegrator::performanceWarning,
                [](const QString& warning) {
                    qWarning() << "Performance Warning:" << warning;
                });
    }

    void simulateDataProduction()
    {
        // 创建定时器来模拟数据生产
        auto productionTimer = new QTimer(this);
        connect(productionTimer, &QTimer::timeout, [this]() {
            static int counter = 0;

            // 模拟不同类型的数据
            if (counter % 3 == 0) {
                // 控制消息（高优先级）
                QByteArray controlData = generateControlMessage(counter);
                auto dataManager = integrator_->getDataManager();
                if (dataManager) {
                    dataManager->produceControlData(controlData, 100);
                }
            } else if (counter % 3 == 1) {
                // 传感器数据（中优先级）
                QByteArray sensorData = generateSensorData(counter);
                auto dataManager = integrator_->getDataManager();
                if (dataManager) {
                    dataManager->produceIncomingData(sensorData);
                }
            } else {
                // 常规通信数据（低优先级）
                QByteArray regularData = generateRegularData(counter);
                auto dataManager = integrator_->getDataManager();
                if (dataManager) {
                    dataManager->produceOutgoingData(regularData, 10);
                }
            }

            counter++;

            // 偶尔产生数据突发
            if (counter % 50 == 0) {
                simulateDataBurst();
            }
        });

        productionTimer->start(100); // 每100ms产生一次数据
    }

    void simulateDataBurst()
    {
        qInfo() << "Simulating data burst...";

        auto dataManager = integrator_->getDataManager();
        if (!dataManager) return;

        // 创建一批数据
        QList<DataItem> burstData;
        for (int i = 0; i < 50; ++i) {
            QByteArray data = QString("Burst data packet %1").arg(i).toUtf8();
            burstData.append(DataItem(data, "burst", i % 10)); // 不同优先级
        }

        // 批量生产
        bool success = dataManager->produceDataBatch(burstData);
        qInfo() << "Data burst" << (success ? "successful" : "partially failed");
    }

    QByteArray generateControlMessage(int counter)
    {
        return QString("CTRL_%1:enable_anc=true").arg(counter).toUtf8();
    }

    QByteArray generateSensorData(int counter)
    {
        // 模拟传感器数据包
        QByteArray data;
        data.append("SENSOR_DATA:");
        data.append(QString("temp=%1,").arg(20.0 + (counter % 100) / 10.0));
        data.append(QString("pressure=%1,").arg(1013.25 + (counter % 50)));
        data.append(QString("timestamp=%1").arg(QDateTime::currentMSecsSinceEpoch()));
        return data;
    }

    QByteArray generateRegularData(int counter)
    {
        return QString("Regular message %1 with some payload data").arg(counter).toUtf8();
    }

    void processIncomingProtocolData(const QByteArray& data)
    {
        // 模拟协议数据解析
        if (data.startsWith("CTRL_")) {
            qDebug() << "Processing control message";
        } else if (data.startsWith("SENSOR_DATA:")) {
            qDebug() << "Processing sensor data";
        } else {
            qDebug() << "Processing regular data";
        }
    }

    bool simulateDataTransmission(const QByteArray& data)
    {
        // 模拟网络传输（偶尔失败）
        static int failCounter = 0;
        bool success = (++failCounter % 20) != 0; // 5% 失败率

        if (!success) {
            qWarning() << "Simulated transmission failure for" << data.size() << "bytes";
        }

        return success;
    }

signals:
    void dataProcessed(const QString& type, int size);

private:
    std::unique_ptr<ProtocolAdapter> protocolAdapter_;
    std::unique_ptr<ConnectionManager> connectionManager_;
    std::unique_ptr<ProtocolBufferAdapter> bufferAdapter_;
    std::unique_ptr<ProtocolSystemIntegrator> integrator_;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qInfo() << "Producer-Consumer Integration Example";
    qInfo() << "====================================";

    IntegrationExample example;
    example.runExample();

    return app.exec();
}

#include "producer_consumer_integration_example.moc"
