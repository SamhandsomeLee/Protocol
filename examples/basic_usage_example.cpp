/**
 * @file basic_usage_example.cpp
 * @brief Protocol 动态库基础使用示例
 *
 * 展示如何在客户端项目中使用 Protocol 动态库
 */

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

// 使用动态库的头文件
#include "protocol/adapter/protocol_adapter_refactored.h"
#include "protocol/transport/serial_transport.h"

using namespace Protocol;

class BasicExample : public QObject {
    Q_OBJECT

public:
    BasicExample(QObject* parent = nullptr) : QObject(parent) {
        setupProtocol();
        demonstrateBasicOperations();
    }

private slots:
    void onParameterAcknowledged(const QString& path) {
        qInfo() << "[ACK]" << path;
    }

    void onCommunicationError(const QString& error) {
        qWarning() << "[ERROR]" << error;
    }

    void onConnectionStatusChanged(bool connected) {
        qInfo() << "[CONNECTION]" << (connected ? "Connected" : "Disconnected");
    }

private:
    void setupProtocol() {
        qInfo() << "=== Protocol 动态库基础示例 ===";
        qInfo() << "库版本:" << getProtocolVersion();

        // 创建串口传输层
        transport_ = new SerialTransport(this);
        transport_->setPortName("COM3");  // 根据实际情况修改
        transport_->setBaudRate(115200);

        // 创建协议适配器
        adapter_ = new ProtocolAdapterRefactored(transport_, this);

        // 连接信号
        connect(adapter_, &ProtocolAdapterRefactored::parameterAcknowledged,
                this, &BasicExample::onParameterAcknowledged);
        connect(adapter_, &ProtocolAdapterRefactored::communicationError,
                this, &BasicExample::onCommunicationError);
        connect(adapter_, &ProtocolAdapterRefactored::connectionStatusChanged,
                this, &BasicExample::onConnectionStatusChanged);

        qInfo() << "协议适配器已创建";
    }

    void demonstrateBasicOperations() {
        qInfo() << "\n=== 基础操作演示 ===";

        // 1. 检查连接状态
        qInfo() << "1. 连接状态:" << (adapter_->isConnected() ? "已连接" : "未连接");

        // 2. 获取支持的参数列表
        qInfo() << "2. 支持的参数:";
        auto supportedParams = adapter_->getSupportedParameters();
        for (const auto& param : supportedParams) {
            qInfo() << "   -" << param;
        }

        // 3. 发送单个参数更新
        qInfo() << "3. 发送参数更新...";
        bool success = adapter_->sendParameterUpdate("anc.enabled", true);
        qInfo() << "   结果:" << (success ? "成功" : "失败");

        // 4. 发送参数组更新
        qInfo() << "4. 发送参数组更新...";
        QStringList paths = {"anc.enabled", "enc.enabled", "rnc.enabled"};
        QVariantMap values = {
            {"anc.enabled", false},
            {"enc.enabled", true},
            {"rnc.enabled", false}
        };
        success = adapter_->sendParameterGroup(paths, values);
        qInfo() << "   结果:" << (success ? "成功" : "失败");

        // 5. 检查特定参数支持
        qInfo() << "5. 参数支持检查:";
        QStringList testParams = {"anc.enabled", "processing.alpha", "unknown.param"};
        for (const auto& param : testParams) {
            bool supported = adapter_->isParameterSupported(param);
            qInfo() << "   " << param << ":" << (supported ? "支持" : "不支持");
        }

        // 6. 获取传输层信息
        qInfo() << "6. 传输层信息:" << adapter_->transportDescription();

        // 7. 获取协议版本
        qInfo() << "7. 协议版本:" << adapter_->getProtocolVersion();
    }

    QString getProtocolVersion() const {
        // 这里可以访问动态库的版本信息
        return "1.0.0";  // 实际应该从动态库获取
    }

private:
    SerialTransport* transport_ = nullptr;
    ProtocolAdapterRefactored* adapter_ = nullptr;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qInfo() << "Protocol 动态库基础使用示例";
    qInfo() << "============================";

    BasicExample example;

    // 运行一小段时间后退出
    QTimer::singleShot(2000, &app, &QCoreApplication::quit);

    return app.exec();
}

#include "basic_usage_example.moc"
