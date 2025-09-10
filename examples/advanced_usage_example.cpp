/**
 * @file advanced_usage_example.cpp
 * @brief Protocol 动态库高级使用示例
 *
 * 展示高级功能：直接访问组件、自定义配置、序列化等
 */

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include "protocol/adapter/protocol_adapter.h"
#include "protocol/transport/serial_transport.h"
#include <iostream>
#include "protocol/core/message_types.h"
#include "protocol/serialization/message_serializer.h"

using namespace Protocol;

class AdvancedExample : public QObject {
    Q_OBJECT

public:
    AdvancedExample(QObject* parent = nullptr) : QObject(parent) {
        setupProtocol();
        demonstrateAdvancedFeatures();
    }

private:
    void setupProtocol() {
        qInfo() << "=== Protocol 动态库高级示例 ===";

        // 创建传输层
        transport_ = new SerialTransport(this);
        transport_->setPortName("COM3");
        transport_->setBaudRate(115200);

        // 创建协议适配器
        adapter_ = new ProtocolAdapter(transport_, this);

        qInfo() << "协议适配器已创建";
    }

    void demonstrateAdvancedFeatures() {
        qInfo() << "\n=== 高级功能演示 ===";

        // 1. 直接访问内部组件
        demonstrateComponentAccess();

        // 2. 自定义参数映射配置
        demonstrateCustomMapping();

        // 3. 序列化和反序列化
        demonstrateSerialization();

        // 4. 连接统计和监控
        demonstrateConnectionStats();

        // 5. 版本管理
        demonstrateVersionManagement();
    }

    void demonstrateComponentAccess() {
        qInfo() << "\n1. 组件直接访问:";

        // 演示新的消息类型系统
        qInfo() << "   消息类型系统:";
        using namespace Protocol;
        
        // 展示新的消息类型
        QList<MessageType> newTypes = {
            MessageType::ANC_SWITCH,
            MessageType::VEHICLE_STATE,
            MessageType::CHANNEL_NUMBER,
            MessageType::ALPHA_PARAMS,
            MessageType::ORDER2_PARAMS
        };
        
        for (const auto& type : newTypes) {
            qInfo() << QString("     %1 (ProtoID: %2) - %3")
                      .arg(MessageTypeUtils::toString(type))
                      .arg(MessageTypeUtils::toProtoID(type))
                      .arg(MessageTypeUtils::getDescription(type));
        }

        // 访问协议适配器状态
        qInfo() << "   适配器状态:";
        qInfo() << "     连接状态:" << (adapter_->isConnected() ? "已连接" : "未连接");
        qInfo() << "     协议版本:" << adapter_->getProtocolVersion();
        qInfo() << "     传输层描述:" << adapter_->transportDescription();
        
        // 展示支持的参数
        auto supportedParams = adapter_->getSupportedParameters();
        qInfo() << "     支持的参数数量:" << supportedParams.size();
        if (supportedParams.size() > 0) {
            qInfo() << "     示例参数:" << supportedParams.mid(0, qMin(3, supportedParams.size())).join(", ");
        }
    }

    void demonstrateCustomMapping() {
        qInfo() << "\n2. 自定义参数映射:";

        // 创建自定义映射配置
        QJsonObject customMapping;
        customMapping["version"] = "1.0.0";
        customMapping["protocolVersion"] = "2.1.0";

        QJsonObject mappings;
        QJsonObject customParam;
        customParam["protobufPath"] = "custom_value";
        customParam["fieldType"] = "float";
        customParam["defaultValue"] = 0.5;
        customParam["messageType"] = "CUSTOM_MSG";
        customParam["description"] = "自定义参数示例";
        mappings["custom.parameter"] = customParam;

        customMapping["mappings"] = mappings;

        QJsonDocument doc(customMapping);
        QString jsonString = doc.toJson(QJsonDocument::Compact);

        qInfo() << "   自定义映射JSON:" << jsonString;

        // 使用适配器加载自定义映射
        bool loadResult = adapter_->loadProtocolMapping("protocol/config/parameter_mapping.json");
        qInfo() << "   参数映射加载:" << (loadResult ? "成功" : "失败");
        
        if (loadResult) {
            qInfo() << "   已加载的参数映射包含:";
            qInfo() << "     - ANC/ENC/RNC开关控制";
            qInfo() << "     - 车辆状态参数";
            qInfo() << "     - 通道配置参数";
            qInfo() << "     - 传函标定参数";
            qInfo() << "     - 系统配置参数";
            qInfo() << "     - ENC标定参数 (2/4/6阶)";
            qInfo() << "     - RNC步长/分频/阈值参数";
        }
    }

    void demonstrateSerialization() {
        qInfo() << "\n3. 序列化和反序列化:";

        // 准备新的参数数据（使用更新后的参数结构）
        QVariantMap parameters;
        
        // ANC开关控制参数
        QVariantMap ancParams;
        ancParams["anc_off"] = false;
        ancParams["enc_off"] = true;
        ancParams["rnc_off"] = false;
        parameters["anc.enabled"] = ancParams;
        
        // 车辆状态参数
        QVariantMap vehicleParams;
        vehicleParams["speed"] = 65;
        vehicleParams["engine_speed"] = 1800;
        parameters["vehicle.speed"] = vehicleParams;
        
        // RNC参数
        QVariantMap rncParams;
        rncParams["alpha1"] = 120;
        rncParams["alpha2"] = 180;
        parameters["rnc.alpha1"] = rncParams;

        qInfo() << "   新参数结构:" << parameters;

        // 序列化
        QByteArray serializedData = adapter_->serializeParameters(parameters);
        if (!serializedData.isEmpty()) {
            qInfo() << "   序列化成功，数据大小:" << serializedData.size() << "字节";
            qInfo() << "   序列化数据 (hex):" << serializedData.toHex();

            // 反序列化
            QVariantMap deserializedParams;
            bool success = adapter_->deserializeParameters(serializedData, deserializedParams);
            if (success) {
                qInfo() << "   反序列化成功:" << deserializedParams;

                // 验证数据一致性
                bool consistent = true;
                for (auto it = parameters.begin(); it != parameters.end(); ++it) {
                    if (!deserializedParams.contains(it.key()) ||
                        deserializedParams[it.key()] != it.value()) {
                        consistent = false;
                        break;
                    }
                }
                qInfo() << "   数据一致性:" << (consistent ? "通过" : "失败");
            } else {
                qInfo() << "   反序列化失败";
            }
        } else {
            qInfo() << "   序列化失败";
        }
    }

    void demonstrateConnectionStats() {
        qInfo() << "\n4. 连接统计信息:";

        qInfo() << "   协议适配器统计:";
        qInfo() << "     连接状态:" << (adapter_->isConnected() ? "已连接" : "未连接");
        qInfo() << "     支持的参数:" << adapter_->getSupportedParameters().size() << "个";
        
        // 演示消息类型统计
        qInfo() << "   消息类型统计:";
        using namespace Protocol;
        QMap<QString, int> typeStats;
        QList<MessageType> allTypes = {
            MessageType::CHANNEL_NUMBER, MessageType::CHANNEL_AMPLITUDE, MessageType::CHANNEL_SWITCH,
            MessageType::CHECK_MOD, MessageType::ANC_SWITCH, MessageType::VEHICLE_STATE,
            MessageType::TRAN_FUNC_FLAG, MessageType::TRAN_FUNC_STATE, MessageType::FILTER_RANGES,
            MessageType::SYSTEM_RANGES, MessageType::ORDER_FLAG, MessageType::ORDER2_PARAMS,
            MessageType::ORDER4_PARAMS, MessageType::ORDER6_PARAMS, MessageType::ALPHA_PARAMS,
            MessageType::FREQ_DIVISION, MessageType::THRESHOLDS, MessageType::GRAPH_DATA
        };
        
        int validCount = 0;
        for (const auto& type : allTypes) {
            if (MessageTypeUtils::isValid(type)) {
                validCount++;
            }
        }
        qInfo() << "     有效消息类型:" << validCount << "/" << allTypes.size();
        
        // 展示协议版本兼容性
        qInfo() << "     协议兼容性:";
        qInfo() << "       当前版本:" << adapter_->getProtocolVersion();
        qInfo() << "       ProtoID范围: 0-" << MessageTypeUtils::toProtoID(MessageType::ALPHA_PARAMS);
    }

    void demonstrateVersionManagement() {
        qInfo() << "\n5. 版本管理:";

        qInfo() << "   ERNC协议版本管理:";
        qInfo() << "     协议版本:" << adapter_->getProtocolVersion();
        qInfo() << "     Qt版本:" << qVersion();
        
        // 新协议特性演示
        qInfo() << "   新协议特性:";
        qInfo() << "     ✓ 支持18种消息类型";
        qInfo() << "     ✓ ProtoID完全映射 (0-158)";
        qInfo() << "     ✓ 层次化参数结构";
        qInfo() << "     ✓ 车辆状态集成";
        qInfo() << "     ✓ 通道配置管理";
        qInfo() << "     ✓ RNC/ENC参数优化";
        
        // 功能码演示
        using namespace Protocol;
        qInfo() << "   功能码支持:";
        qInfo() << QString("     REQUEST: %1").arg(static_cast<int>(FunctionCode::REQUEST));
        qInfo() << QString("     RESPONSE: %1").arg(static_cast<int>(FunctionCode::RESPONSE));
    }

private:
    SerialTransport* transport_ = nullptr;
    ProtocolAdapter* adapter_ = nullptr;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qInfo() << "Protocol 动态库高级使用示例";
    qInfo() << "============================";

    AdvancedExample example;

    std::cin.get();

    // 运行一小段时间后退出
    QTimer::singleShot(3000, &app, &QCoreApplication::quit);

    return app.exec();
}

#include "advanced_usage_example.moc"
