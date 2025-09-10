/**
 * @file producer_consumer_integration_example.cpp
 * @brief 协议系统生产者消费者模型集成示例
 *
 * 本文件展示如何在现有协议系统中集成生产者消费者模型，
 * 包括配置、使用和监控
 */

#include "buffer/protocol_system_integrator.h"
#include "adapter/protocol_adapter.h"
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
        qInfo() << "=== Starting ERNC Producer-Consumer Integration Example ===";
        qInfo() << "Protocol version:" << (protocolAdapter_ ? protocolAdapter_->getProtocolVersion() : "Unknown");
        qInfo() << "Supported message types: 18 (ProtoID range: 0-158)";
        qInfo() << "New features: ANC/ENC/RNC control, Vehicle state, Channel config, RNC params";

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
        // 创建协议适配器（支持ERNC协议）
        protocolAdapter_ = std::make_unique<ProtocolAdapter>(this);
        
        // 加载新的参数映射配置
        protocolAdapter_->loadProtocolMapping("protocol/config/parameter_mapping.json");

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
        // 生成新的ERNC控制消息格式
        QJsonObject controlMsg;
        controlMsg["type"] = "ANC_SWITCH";
        controlMsg["proto_id"] = 151;
        controlMsg["counter"] = counter;
        
        QJsonObject params;
        params["anc_off"] = (counter % 4 == 0);  // 周期性切换ANC
        params["enc_off"] = (counter % 3 == 0);  // 周期性切换ENC
        params["rnc_off"] = (counter % 5 == 0);  // 周期性切换RNC
        controlMsg["params"] = params;
        
        QJsonDocument doc(controlMsg);
        return doc.toJson(QJsonDocument::Compact);
    }

    QByteArray generateSensorData(int counter)
    {
        // 生成ERNC车辆状态数据
        QJsonObject sensorData;
        sensorData["type"] = "VEHICLE_STATE";
        sensorData["proto_id"] = 138;
        sensorData["counter"] = counter;
        
        QJsonObject vehicleParams;
        vehicleParams["speed"] = 60 + (counter % 80);           // 车速60-140km/h
        vehicleParams["engine_speed"] = 1500 + (counter % 1000); // 发动机转速1500-2500rpm
        vehicleParams["temperature"] = 20.0 + (counter % 50) / 10.0; // 温度
        vehicleParams["pressure"] = 1013.25 + (counter % 50);    // 气压
        vehicleParams["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        sensorData["params"] = vehicleParams;
        
        QJsonDocument doc(sensorData);
        return doc.toJson(QJsonDocument::Compact);
    }

    QByteArray generateRegularData(int counter)
    {
        // 生成RNC参数数据
        QJsonObject regularData;
        
        // 交替生成不同类型的RNC数据
        if (counter % 3 == 0) {
            regularData["type"] = "ALPHA_PARAMS";
            regularData["proto_id"] = 158;
            QJsonObject alphaParams;
            alphaParams["alpha1"] = 50 + (counter % 100);
            alphaParams["alpha2"] = 75 + (counter % 150);
            alphaParams["alpha3"] = 100 + (counter % 200);
            regularData["params"] = alphaParams;
        } else if (counter % 3 == 1) {
            regularData["type"] = "FREQ_DIVISION";
            regularData["proto_id"] = 27;
            QJsonObject freqParams;
            freqParams["division_factor"] = 2 + (counter % 8);
            freqParams["cutoff_freq"] = 100 + (counter % 400);
            regularData["params"] = freqParams;
        } else {
            regularData["type"] = "CHANNEL_NUMBER";
            regularData["proto_id"] = 0;
            QJsonObject channelParams;
            channelParams["refer_num"] = 4 + (counter % 4);
            channelParams["error_num"] = 8 + (counter % 8);
            regularData["params"] = channelParams;
        }
        
        regularData["counter"] = counter;
        QJsonDocument doc(regularData);
        return doc.toJson(QJsonDocument::Compact);
    }

    void processIncomingProtocolData(const QByteArray& data)
    {
        // 解析ERNC协议数据
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "Processing raw data (" << data.size() << "bytes)";
            return;
        }
        
        QJsonObject obj = doc.object();
        QString msgType = obj["type"].toString();
        int protoId = obj["proto_id"].toInt();
        
        if (msgType == "ANC_SWITCH") {
            qDebug() << "Processing ANC switch control (ProtoID:" << protoId << ")";
            QJsonObject params = obj["params"].toObject();
            qDebug() << "  ANC:" << (!params["anc_off"].toBool() ? "ON" : "OFF");
            qDebug() << "  ENC:" << (!params["enc_off"].toBool() ? "ON" : "OFF");
            qDebug() << "  RNC:" << (!params["rnc_off"].toBool() ? "ON" : "OFF");
        } else if (msgType == "VEHICLE_STATE") {
            qDebug() << "Processing vehicle state (ProtoID:" << protoId << ")";
            QJsonObject params = obj["params"].toObject();
            qDebug() << "  Speed:" << params["speed"].toInt() << "km/h";
            qDebug() << "  Engine:" << params["engine_speed"].toInt() << "rpm";
        } else if (msgType == "ALPHA_PARAMS") {
            qDebug() << "Processing RNC alpha parameters (ProtoID:" << protoId << ")";
        } else if (msgType == "CHANNEL_NUMBER") {
            qDebug() << "Processing channel configuration (ProtoID:" << protoId << ")";
        } else {
            qDebug() << "Processing unknown message type:" << msgType;
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
