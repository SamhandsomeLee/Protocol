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

#include "protocol/adapter/protocol_adapter_refactored.h"
#include "protocol/transport/serial_transport.h"
#include <iostream>
#include "protocol/mapping/parameter_mapper.h"
#include "protocol/connection/connection_manager.h"
#include "protocol/version/version_manager.h"

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
        adapter_ = new ProtocolAdapterRefactored(transport_, this);

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

        // 访问参数映射器
        auto* paramMapper = adapter_->parameterMapper();
        if (paramMapper) {
            qInfo() << "   参数映射器可用";

            // 获取详细参数信息
            auto paramInfo = paramMapper->getParameterInfo("anc.enabled");
            if (paramInfo.isValid()) {
                qInfo() << "   ANC 参数详情:";
                qInfo() << "     逻辑路径:" << paramInfo.logicalPath;
                qInfo() << "     Protobuf路径:" << paramInfo.protobufPath;
                qInfo() << "     字段类型:" << paramInfo.fieldType;
                qInfo() << "     默认值:" << paramInfo.defaultValue;
                qInfo() << "     消息类型:" << static_cast<int>(paramInfo.messageType);
                qInfo() << "     描述:" << paramInfo.description;
            }
        }

        // 访问连接管理器
        auto* connManager = adapter_->connectionManager();
        if (connManager) {
            qInfo() << "   连接管理器可用";
            qInfo() << "   当前状态:" << (connManager->isConnected() ? "已连接" : "未连接");
        }

        // 访问版本管理器
        auto* versionManager = adapter_->versionManager();
        if (versionManager) {
            qInfo() << "   版本管理器可用";
            qInfo() << "   版本摘要:" << versionManager->getVersionSummary();
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

        // 加载自定义映射（如果支持的话）
        auto* paramMapper = adapter_->parameterMapper();
        if (paramMapper) {
            // 这里应该有加载自定义映射的方法
            qInfo() << "   (自定义映射加载功能需要在 ParameterMapper 中实现)";
        }
    }

    void demonstrateSerialization() {
        qInfo() << "\n3. 序列化和反序列化:";

        // 准备参数数据
        QVariantMap parameters;
        parameters["anc.enabled"] = false;
        parameters["enc.enabled"] = true;
        parameters["processing.alpha"] = 0.8f;

        qInfo() << "   原始参数:" << parameters;

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

        auto* connManager = adapter_->connectionManager();
        if (connManager) {
            auto stats = connManager->getConnectionStats();

            qInfo() << "   连接统计:";
            qInfo() << "     发送字节数:" << stats.bytesSent;
            qInfo() << "     接收字节数:" << stats.bytesReceived;
            qInfo() << "     发送错误:" << stats.sendErrorCount;
            qInfo() << "     接收错误:" << stats.receiveErrorCount;
            qInfo() << "     重试次数:" << stats.retryCount;
            if (!stats.lastError.isEmpty()) {
                qInfo() << "     最后错误:" << stats.lastError;
            }

            // 计算传输效率（使用可用的统计信息）
            if (stats.bytesSent > 0 && stats.sendErrorCount >= 0) {
                int successfulSends = qMax(1, stats.bytesSent / 50); // 假设平均包大小50字节
                double avgSendSize = static_cast<double>(stats.bytesSent) / successfulSends;
                qInfo() << "     估计平均发送大小:" << QString::number(avgSendSize, 'f', 2) << "字节";
            }

            if (stats.bytesReceived > 0) {
                int successfulReceives = qMax(1, stats.bytesReceived / 50); // 假设平均包大小50字节
                double avgReceiveSize = static_cast<double>(stats.bytesReceived) / successfulReceives;
                qInfo() << "     估计平均接收大小:" << QString::number(avgReceiveSize, 'f', 2) << "字节";
            }
        }
    }

    void demonstrateVersionManagement() {
        qInfo() << "\n5. 版本管理:";

        auto* versionManager = adapter_->versionManager();
        if (versionManager) {
            qInfo() << "   版本信息:";
            qInfo() << "     当前版本:" << versionManager->getCurrentVersion();
            qInfo() << "     支持的版本:" << versionManager->getSupportedVersions().join(", ");
            qInfo() << "     Qt版本:" << qVersion();

            // 版本兼容性检查
            bool compatible = versionManager->isCompatible("1.0.1");
            qInfo() << "   协议版本 1.0.1 兼容性:" << (compatible ? "兼容" : "不兼容");

            // 检查另一个版本
            QString reason;
            bool compatible2 = versionManager->isCompatible("1.1.0", reason);
            qInfo() << "   协议版本 1.1.0 兼容性:" << (compatible2 ? "兼容" : "不兼容");
            if (!compatible2 && !reason.isEmpty()) {
                qInfo() << "     原因:" << reason;
            }
        }
    }

private:
    SerialTransport* transport_ = nullptr;
    ProtocolAdapterRefactored* adapter_ = nullptr;
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
