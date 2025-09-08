/**
 * @file usage_example.cpp
 * @brief 重构后的ProtocolAdapter使用示例
 *
 * 展示如何使用新的职责分离架构
 */

#include <QCoreApplication>
#include <QDebug>
#include "../adapter/protocol_adapter_refactored.h"
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
        adapter_ = new ProtocolAdapterRefactored(transport_, this);

        // 连接信号
        connect(adapter_, &ProtocolAdapterRefactored::parameterAcknowledged,
                this, &ProtocolExample::onParameterAcknowledged);
        connect(adapter_, &ProtocolAdapterRefactored::communicationError,
                this, &ProtocolExample::onCommunicationError);
        connect(adapter_, &ProtocolAdapterRefactored::connectionStatusChanged,
                this, &ProtocolExample::onConnectionStatusChanged);

        // 打开连接
        transport_->open();

        qInfo() << "Protocol version:" << adapter_->getProtocolVersion();
        qInfo() << "Transport:" << adapter_->transportDescription();
    }

    void demonstrateBasicUsage() {
        qInfo() << "\n=== Basic Usage Examples ===";

        // 1. 发送单个参数
        qInfo() << "1. Sending single parameter update...";
        bool success = adapter_->sendParameterUpdate("anc.enabled", true);
        qInfo() << "   Result:" << (success ? "Success" : "Failed");

        // 2. 检查参数支持
        qInfo() << "2. Checking parameter support...";
        QStringList testParams = {"anc.enabled", "enc.enabled", "unknown.param"};
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

        // 4. 发送参数组
        qInfo() << "4. Sending parameter group...";
        QStringList paths = {"anc.enabled", "enc.enabled"};
        QVariantMap values = {
            {"anc.enabled", false},
            {"enc.enabled", true}
        };
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

        // 获取详细的参数信息
        if (parameterMapper) {
            auto paramInfo = parameterMapper->getParameterInfo("anc.enabled");
            if (paramInfo.isValid()) {
                qInfo() << "   ANC parameter info:";
                qInfo() << "     Logical path:" << paramInfo.logicalPath;
                qInfo() << "     Protobuf path:" << paramInfo.protobufPath;
                qInfo() << "     Field type:" << paramInfo.fieldType;
                qInfo() << "     Default value:" << paramInfo.defaultValue;
            }
        }

        // 获取连接统计信息
        if (connectionManager) {
            auto stats = connectionManager->getConnectionStats();
            qInfo() << "   Connection statistics:";
            qInfo() << "     Bytes sent:" << stats.bytesSent;
            qInfo() << "     Bytes received:" << stats.bytesReceived;
            qInfo() << "     Send errors:" << stats.sendErrorCount;
        }

        // 获取版本信息
        if (versionManager) {
            qInfo() << "   Version info:" << versionManager->getVersionSummary();
        }

        // 2. 直接序列化/反序列化
        qInfo() << "2. Direct serialization example...";
        QVariantMap params = {{"processing.alpha", 0.8f}};
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
    ProtocolAdapterRefactored* adapter_ = nullptr;
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
