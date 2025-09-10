/**
 * @file basic_usage_example.cpp
 * @brief Protocol 动态库基础使用示例
 *
 * 展示如何在客户端项目中使用 Protocol 动态库
 */

#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include <QTimer>

// 使用动态库的头文件
#include "protocol/adapter/protocol_adapter.h"
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

        transport_->open();

        // 创建协议适配器
        adapter_ = new ProtocolAdapter(transport_, this);

        // 连接信号
        connect(adapter_, &ProtocolAdapter::parameterAcknowledged,
                this, &BasicExample::onParameterAcknowledged);
        connect(adapter_, &ProtocolAdapter::communicationError,
                this, &BasicExample::onCommunicationError);
        connect(adapter_, &ProtocolAdapter::connectionStatusChanged,
                this, &BasicExample::onConnectionStatusChanged);

        qInfo() << "协议适配器已创建";

        // 加载参数映射文件 (如果需要的话)
        adapter_->loadProtocolMapping("H:/ERNC_Clion/protocol/config/parameter_mapping.json");

        // 检查初始化和连接状态
        qDebug() << "初始化状态检查:";
        qDebug() << "  - 适配器已初始化:" << (adapter_ != nullptr);
        qDebug() << "  - 传输层连接状态:" << adapter_->isConnected();
        qDebug() << "  - 支持的参数数量:" << adapter_->getSupportedParameters().size();
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

        // 3. 发送ANC/ENC/RNC开关状态更新
        qInfo() << "3. 发送ANC开关状态更新...";
        QVariantMap ancSwitchParams;
        ancSwitchParams["anc_off"] = false;  // 开启ANC
        ancSwitchParams["enc_off"] = true;   // 关闭ENC
        ancSwitchParams["rnc_off"] = false;  // 开启RNC
        bool success = adapter_->sendParameterUpdate("anc.enabled", ancSwitchParams);
        qInfo() << "   结果:" << (success ? "成功" : "失败");

        // 4. 发送车辆状态参数
        qInfo() << "4. 发送车辆状态参数...";
        QVariantMap vehicleParams;
        vehicleParams["speed"] = 80;              // 车速80km/h
        vehicleParams["engine_speed"] = 2000;     // 发动机转速2000rpm
        success = adapter_->sendParameterUpdate("vehicle.speed", vehicleParams);
        qInfo() << "   结果:" << (success ? "成功" : "失败");

        // 5. 发送通道配置参数
        qInfo() << "5. 发送通道配置参数...";
        QVariantMap channelParams;
        channelParams["refer_num"] = 4;           // 参考通道数量
        channelParams["error_num"] = 8;           // 误差通道数量
        success = adapter_->sendParameterUpdate("channel.refer_num", channelParams);
        qInfo() << "   结果:" << (success ? "成功" : "失败");

        // 6. 检查特定参数支持
        qInfo() << "6. 参数支持检查:";
        QStringList testParams = {
            "anc.enabled",        // ANC开关状态
            "vehicle.speed",      // 车辆状态
            "channel.refer_num",  // 通道配置
            "rnc.alpha1",         // RNC步长参数
            "order2.params",      // ENC 2阶参数
            "unknown.param"       // 未知参数
        };
        for (const auto& param : testParams) {
            bool supported = adapter_->isParameterSupported(param);
            qInfo() << "   " << param << ":" << (supported ? "支持" : "不支持");
        }

        // 7. 发送RNC参数组更新
        qInfo() << "7. 发送RNC参数组更新...";
        QStringList rncPaths = {"rnc.alpha1", "rnc.alpha2", "rnc.alpha3"};
        QVariantMap rncValues = {
            {"rnc.alpha1", 100},
            {"rnc.alpha2", 150},
            {"rnc.alpha3", 200}
        };
        success = adapter_->sendParameterGroup(rncPaths, rncValues);
        qInfo() << "   结果:" << (success ? "成功" : "失败");

        // 8. 获取传输层信息
        qInfo() << "8. 传输层信息:" << adapter_->transportDescription();

        // 9. 获取协议版本
        qInfo() << "9. 协议版本:" << adapter_->getProtocolVersion();
        
        // 10. 展示新支持的消息类型
        qInfo() << "10. 新支持的消息类型演示:";
        qInfo() << "   - ANC_SWITCH (ProtoID: 151): ANC/ENC/RNC开关控制";
        qInfo() << "   - VEHICLE_STATE (ProtoID: 138): 车辆状态信息";
        qInfo() << "   - CHANNEL_NUMBER (ProtoID: 0): 通道数量配置";
        qInfo() << "   - ALPHA_PARAMS (ProtoID: 158): RNC步长参数";
        qInfo() << "   - ORDER2_PARAMS (ProtoID: 78): ENC 2阶参数";
    }

    QString getProtocolVersion() const {
        // 这里可以访问动态库的版本信息
        return "1.0.0";  // 实际应该从动态库获取
    }

private:
    SerialTransport* transport_ = nullptr;
    ProtocolAdapter* adapter_ = nullptr;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qInfo() << "Protocol 动态库基础使用示例";
    qInfo() << "============================";

    BasicExample example;

    std::cin.get();

    // 运行一小段时间后退出
    QTimer::singleShot(2000, &app, &QCoreApplication::quit);

    return app.exec();
}

#include "basic_usage_example.moc"
