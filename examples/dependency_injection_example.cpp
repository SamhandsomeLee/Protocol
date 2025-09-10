#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "protocol/adapter/protocol_adapter.h"
#include "protocol/transport/serial_transport.h"

/**
 * @brief 依赖注入架构使用示例
 *
 * 展示如何使用新的依赖注入架构：
 * 1. 创建传输层对象（SerialTransport）
 * 2. 创建协议适配器并注入传输层
 * 3. 使用协议适配器进行参数操作
 * 4. 处理传输层的连接状态和错误
 */

class ExampleApplication : public QObject
{
    Q_OBJECT

public:
    ExampleApplication(QObject* parent = nullptr)
        : QObject(parent)
        , transport_(nullptr)
        , adapter_(nullptr)
    {
        setupTransportAndAdapter();
        connectSignals();
    }

    ~ExampleApplication() {
        cleanup();
    }

    void run() {
        qDebug() << "=== 依赖注入架构示例 ===";

        // 1. 连接到串口设备
        connectToDevice();

        // 2. 延迟执行参数操作示例
        QTimer::singleShot(2000, this, &ExampleApplication::demonstrateParameterOperations);

        // 3. 延迟执行传输层切换示例
        QTimer::singleShot(5000, this, &ExampleApplication::demonstrateTransportSwitching);

        // 4. 程序退出
        QTimer::singleShot(8000, qApp, &QCoreApplication::quit);
    }

private slots:
    void onTransportConnected() {
        qDebug() << "✅ Transport connected:" << transport_->description();
    }

    void onTransportDisconnected() {
        qDebug() << "❌ Transport disconnected";
    }

    void onTransportError(const QString& error) {
        qWarning() << "🔥 Transport error:" << error;
    }

    void onParameterAcknowledged(const QString& path) {
        qDebug() << "✅ Parameter acknowledged:" << path;
    }

    void onCommunicationError(const QString& error) {
        qWarning() << "🔥 Communication error:" << error;
    }

private:
    void setupTransportAndAdapter() {
        // 创建串口传输层
        transport_ = new SerialTransport("COM3", 115200, this);
        transport_->setAutoReconnect(true);

        // 创建协议适配器并注入传输层
        adapter_ = new ProtocolAdapter(transport_, this);

        qDebug() << "📦 Transport and adapter created";
    }

    void connectSignals() {
        // 连接传输层信号
        connect(transport_, &ITransport::connected,
                this, &ExampleApplication::onTransportConnected);
        connect(transport_, &ITransport::disconnected,
                this, &ExampleApplication::onTransportDisconnected);
        connect(transport_, &ITransport::transportError,
                this, &ExampleApplication::onTransportError);

        // 连接协议适配器信号
        connect(adapter_, &ProtocolAdapter::parameterAcknowledged,
                this, &ExampleApplication::onParameterAcknowledged);
        connect(adapter_, &ProtocolAdapter::communicationError,
                this, &ExampleApplication::onCommunicationError);
    }

    void connectToDevice() {
        qDebug() << "🔌 Attempting to connect to device...";

        bool success = transport_->open();
        if (success) {
            qDebug() << "✅ Connection successful!";
        } else {
            qWarning() << "❌ Connection failed:" << transport_->lastErrorString();

            // 演示：即使连接失败，我们仍可以继续其他操作
            qDebug() << "💡 Continuing with mock operations...";
        }
    }

    void demonstrateParameterOperations() {
        qDebug() << "\n=== 参数操作示例 ===";

        // 1. 单个参数更新
        qDebug() << "📤 Sending single parameter update...";
        adapter_->sendParameterUpdate("anc.enabled", true);

        // 2. 参数组更新
        qDebug() << "📤 Sending parameter group update...";
        QStringList paths = {"tuning.alpha.alpha1", "tuning.alpha.alpha2", "tuning.alpha.alpha3"};
        QVariantMap values = {
            {"tuning.alpha.alpha1", 0.5},
            {"tuning.alpha.alpha2", 0.7},
            {"tuning.alpha.alpha3", 0.9}
        };
        adapter_->sendParameterGroup(paths, values);

        // 3. 查询协议信息
        qDebug() << "📋 Protocol version:" << adapter_->getProtocolVersion();
        qDebug() << "📋 Supported parameters:" << adapter_->getSupportedParameters().size();
        qDebug() << "📋 Transport description:" << adapter_->transportDescription();
    }

    void demonstrateTransportSwitching() {
        qDebug() << "\n=== 传输层切换示例 ===";

        // 演示如何在运行时切换传输层
        qDebug() << "🔄 Creating new transport instance...";

        SerialTransport* newTransport = new SerialTransport("COM4", 9600, this);

        qDebug() << "🔄 Switching transport layer...";
        adapter_->setTransport(newTransport);

        qDebug() << "📋 New transport description:" << adapter_->transportDescription();

        // 注意：旧的传输层对象会自动管理生命周期
        transport_ = newTransport;  // 更新引用
    }

    void cleanup() {
        qDebug() << "\n=== 清理资源 ===";

        if (transport_ && transport_->isOpen()) {
            transport_->close();
            qDebug() << "🔌 Transport closed";
        }
        
        // 清理新的参数映射缓存
        qDebug() << "🧹 Clearing parameter mapping cache...";

        qDebug() << "✅ ERNC Protocol cleanup completed";
    }

private:
    SerialTransport* transport_;
    ProtocolAdapter* adapter_;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 创建并运行示例应用
    ExampleApplication example;
    example.run();

    return app.exec();
}

#include "dependency_injection_example.moc"
