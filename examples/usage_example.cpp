/**
 * @file usage_example.cpp
 * @brief 重构后的ProtocolAdapter使用示例
 *
 * 展示如何使用新的职责分离架构
 */

#include <QCoreApplication>
#include <QDebug>
#include "../adapter/protocol_adapter.h"
#include "../transport/itransport.h" // 假设存在具体的传输实现

using namespace Protocol;

// 模拟传输层实现（用于演示）
class MockTransport : public ITransport {
    Q_OBJECT
public:
    MockTransport() = default;

    bool open() override {
        connected_ = true;
        emit connectionStatusChanged(true);
        return true;
    }

    void close() override {
        connected_ = false;
        emit connectionStatusChanged(false);
    }

    bool isConnected() const override { return connected_; }

    bool write(const QByteArray& data) override {
        qDebug() << "MockTransport: Sending" << data.size() << "bytes";
        // 模拟发送成功
        return true;
    }

    QString transportType() const override { return "Mock"; }

private:
    bool connected_ = false;
};

class ProtocolExample : public QObject {
    Q_OBJECT

public:
    ProtocolExample() {
        setupProtocolAdapter();
        demonstrateBasicUsage();
        demonstrateAdvancedUsage();
    }

private slots:
    void onParameterAcknowledged(const QString& path) {
        qInfo() << "Parameter acknowledged:" << path;
    }

    void onCommunicationError(const QString& error) {
        qWarning() << "Communication error:" << error;
    }

    void onConnectionStatusChanged(bool connected) {
        qInfo() << "Connection status:" << (connected ? "Connected" : "Disconnected");
    }

private:
    void setupProtocolAdapter() {
        qInfo() << "=== Setting up Protocol Adapter ===";

        // 创建传输层
        transport_ = new MockTransport();

        // 创建协议适配器
        adapter_ = new ProtocolAdapter(transport_, this);

        // 连接信号
        connect(adapter_, &ProtocolAdapter::parameterAcknowledged,
                this, &ProtocolExample::onParameterAcknowledged);
        connect(adapter_, &ProtocolAdapter::communicationError,
                this, &ProtocolExample::onCommunicationError);
        connect(adapter_, &ProtocolAdapter::connectionStatusChanged,
                this, &ProtocolExample::onConnectionStatusChanged);

        // 打开连接
        transport_->open();

        qInfo() << "Protocol version:" << adapter_->getProtocolVersion();
        qInfo() << "Transport:" << adapter_->transportDescription();
    }

    void demonstrateBasicUsage() {
        qInfo() << "\n=== Basic Usage Examples ===";

        // 1. 发送ANC开关参数
        qInfo() << "1. Sending ANC switch parameter update...";
        QVariantMap ancParams;
        ancParams["anc_off"] = false;  // 开启ANC
        ancParams["enc_off"] = true;   // 关闭ENC
        ancParams["rnc_off"] = false;  // 开启RNC
        bool success = adapter_->sendParameterUpdate("anc.enabled", ancParams);
        qInfo() << "   Result:" << (success ? "Success" : "Failed");

        // 2. 检查新参数支持
        qInfo() << "2. Checking new parameter support...";
        QStringList testParams = {
            "anc.enabled",        // ANC/ENC/RNC开关
            "vehicle.speed",      // 车辆状态
            "channel.refer_num",  // 通道数量
            "rnc.alpha1",         // RNC参数
            "order2.params",      // ENC 2阶参数
            "unknown.param"       // 未知参数
        };
        for (const QString& param : testParams) {
            bool supported = adapter_->isParameterSupported(param);
            qInfo() << "   " << param << ":" << (supported ? "Supported" : "Not supported");
        }

        // 3. 获取所有支持的参数
        qInfo() << "3. Supported parameters:";
        QStringList supportedParams = adapter_->getSupportedParameters();
        for (const QString& param : supportedParams) {
            qInfo() << "   -" << param;
        }

        // 4. 发送新的参数组（车辆状态 + RNC参数）
        qInfo() << "4. Sending new parameter group...";
        QStringList paths = {"vehicle.speed", "rnc.alpha1"};
        QVariantMap values;
        
        // 车辆状态参数
        QVariantMap vehicleParams;
        vehicleParams["speed"] = 75;
        vehicleParams["engine_speed"] = 1900;
        values["vehicle.speed"] = vehicleParams;
        
        // RNC参数
        QVariantMap rncParams;
        rncParams["alpha1"] = 110;
        rncParams["alpha2"] = 160;
        values["rnc.alpha1"] = rncParams;
        
        success = adapter_->sendParameterGroup(paths, values);
        qInfo() << "   Result:" << (success ? "Success" : "Failed");
    }

    void demonstrateAdvancedUsage() {
        qInfo() << "\n=== Advanced Usage Examples ===";

        // 1. 直接访问组件（高级用法）
        qInfo() << "1. Accessing components directly...";
        auto parameterMapper = adapter_->parameterMapper();
        auto messageSerializer = adapter_->messageSerializer();
        auto connectionManager = adapter_->connectionManager();
        auto versionManager = adapter_->versionManager();

        // 获取协议适配器状态信息
        qInfo() << "   Protocol adapter status:";
        qInfo() << "     Is connected:" << adapter_->isConnected();
        qInfo() << "     Protocol version:" << adapter_->getProtocolVersion();
        qInfo() << "     Transport description:" << adapter_->transportDescription();
        
        // 展示新支持的参数类型
        qInfo() << "   New supported parameter types:";
        qInfo() << "     - ANC/ENC/RNC switches (anc.enabled)";
        qInfo() << "     - Vehicle state (vehicle.speed)";
        qInfo() << "     - Channel configuration (channel.refer_num)";
        qInfo() << "     - RNC parameters (rnc.alpha1)";
        qInfo() << "     - ENC parameters (order2.params)";

        // 获取适配器统计信息
        qInfo() << "   Adapter statistics:";
        qInfo() << "     Connection status:" << (adapter_->isConnected() ? "Connected" : "Disconnected");
        qInfo() << "     Supported parameters:" << adapter_->getSupportedParameters().size();
        qInfo() << "     Transport type:" << (transport_ ? transport_->transportType() : "None");
        
        // 展示消息类型统计
        qInfo() << "   Message type coverage:";
        qInfo() << "     Total message types: 18";
        qInfo() << "     ProtoID range: 0-158";
        qInfo() << "     Categories: Real-time, Vehicle, Transfer Function, System, ENC, RNC";

        // 获取ERNC协议版本信息
        qInfo() << "   ERNC Protocol version info:";
        qInfo() << "     Protocol version:" << adapter_->getProtocolVersion();
        qInfo() << "     Proto file: ERNC_praram.proto";
        qInfo() << "     Supported features: 18 message types, hierarchical parameters";

        // 2. 直接序列化/反序列化（使用新参数结构）
        qInfo() << "2. Direct serialization example with new structure...";
        QVariantMap params;
        QVariantMap rncParams;
        rncParams["alpha1"] = 95;
        rncParams["alpha2"] = 145;
        params["rnc.alpha1"] = rncParams;
        
        QByteArray serializedData = adapter_->serializeParameters(params);

        if (!serializedData.isEmpty()) {
            qInfo() << "   Serialized" << params.size() << "parameters to" << serializedData.size() << "bytes";

            // 反序列化
            QVariantMap deserializedParams;
            bool success = adapter_->deserializeParameters(serializedData, deserializedParams);
            if (success) {
                qInfo() << "   Deserialized successfully:" << deserializedParams.size() << "parameters";
                for (auto it = deserializedParams.begin(); it != deserializedParams.end(); ++it) {
                    qInfo() << "     " << it.key() << "=" << it.value();
                }
            }
        }

        // 3. 自定义消息处理器示例
        demonstrateCustomHandler();
    }

    void demonstrateCustomHandler() {
        qInfo() << "3. Custom message handler example...";

        // 这里可以展示如何添加自定义消息处理器
        // 例如：adapter_->messageSerializer()->registerCustomHandler(...)

        qInfo() << "   (Custom handlers can be registered for new message types)";
    }

private:
    MockTransport* transport_ = nullptr;
    ProtocolAdapter* adapter_ = nullptr;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qInfo() << "Protocol Adapter Refactored - Usage Example";
    qInfo() << "==========================================";

    ProtocolExample example;

    return 0; // 不运行事件循环，直接退出
}

#include "usage_example.moc"
