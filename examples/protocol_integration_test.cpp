#include <QCoreApplication>
#include <QDebug>
#include <QVariantMap>
#include "../core/message_types.h"
#include "../serialization/message_serializer.h"
#include "../adapter/protocol_adapter.h"

/**
 * @brief ERNC协议模块集成测试
 *
 * 验证更新后的protocol模块功能是否正常：
 * 1. 消息类型枚举
 * 2. 参数映射
 * 3. 消息序列化器
 * 4. 协议适配器
 */

using namespace Protocol;

void testMessageTypes() {
    qDebug() << "=== 测试消息类型 ===";

    // 测试新的消息类型
    QList<MessageType> testTypes = {
        MessageType::CHANNEL_NUMBER,
        MessageType::CHANNEL_AMPLITUDE,
        MessageType::ANC_SWITCH,
        MessageType::VEHICLE_STATE,
        MessageType::ALPHA_PARAMS,
        MessageType::ORDER2_PARAMS,
        MessageType::FREQ_DIVISION
    };

    for (const auto& type : testTypes) {
        QString name = MessageTypeUtils::toString(type);
        int protoID = MessageTypeUtils::toProtoID(type);
        QString desc = MessageTypeUtils::getDescription(type);
        bool valid = MessageTypeUtils::isValid(type);

        qDebug() << QString("类型: %1, ProtoID: %2, 描述: %3, 有效: %4")
                    .arg(name).arg(protoID).arg(desc).arg(valid);
    }

    // 测试ProtoID到MessageType的转换
    qDebug() << "ProtoID转换测试:";
    QList<int> testProtoIDs = {0, 25, 119, 150, 151, 138, 158, 78};
    for (int id : testProtoIDs) {
        MessageType type = MessageTypeUtils::fromProtoID(id);
        qDebug() << QString("ProtoID %1 -> %2").arg(id).arg(MessageTypeUtils::toString(type));
    }
}

void testMessageSerializer() {
    qDebug() << "\n=== 测试消息序列化器 ===";

    MessageSerializer serializer;

    // 获取支持的消息类型
    auto supportedTypes = serializer.getSupportedMessageTypes();
    qDebug() << "支持的消息类型数量:" << supportedTypes.size();

    for (const auto& type : supportedTypes) {
        qDebug() << "支持:" << MessageTypeUtils::toString(type)
                 << "(" << serializer.getMessageTypeDescription(type) << ")";
    }

    // 测试参数验证
    QVariantMap testParams;
    testParams["anc_off"] = false;
    testParams["enc_off"] = true;
    testParams["rnc_off"] = false;

    bool valid = serializer.validateParameters(MessageType::ANC_SWITCH, testParams);
    qDebug() << "ANC_SWITCH参数验证结果:" << valid;
}

void testProtocolAdapter() {
    qDebug() << "\n=== 测试协议适配器 ===";

    ProtocolAdapter adapter;

    // 测试协议版本
    QString version = adapter.getProtocolVersion();
    qDebug() << "协议版本:" << version;

    // 测试参数支持检查
    QStringList testPaths = {
        "anc.enabled",
        "channel.refer_num",
        "vehicle.speed",
        "rnc.alpha1",
        "invalid.parameter"
    };

    qDebug() << "参数支持测试:";
    for (const auto& path : testPaths) {
        bool supported = adapter.isParameterSupported(path);
        qDebug() << QString("参数 '%1': %2").arg(path).arg(supported ? "支持" : "不支持");
    }

    // 获取所有支持的参数
    QStringList supportedParams = adapter.getSupportedParameters();
    qDebug() << "支持的参数总数:" << supportedParams.size();
    qDebug() << "部分支持的参数:" << supportedParams.mid(0, qMin(5, supportedParams.size()));
}

void testParameterMapping() {
    qDebug() << "\n=== 测试参数映射 ===";

    ProtocolAdapter adapter;

    // 尝试加载参数映射配置
    QString mappingFile = "protocol/config/parameter_mapping.json";
    bool loaded = adapter.loadProtocolMapping(mappingFile);
    qDebug() << "参数映射文件加载:" << (loaded ? "成功" : "失败");

    if (loaded) {
        // 测试参数路径到protobuf路径的映射
        QStringList testPaths = {
            "anc.enabled",
            "channel.refer_num",
            "vehicle.speed",
            "rnc.alpha1"
        };

        for (const auto& path : testPaths) {
            QString protobufPath = adapter.getProtobufPath(path);
            qDebug() << QString("'%1' -> '%2'").arg(path).arg(protobufPath);
        }
    }
}

void runFunctionCodeTests() {
    qDebug() << "\n=== 测试功能码 ===";

    // 测试FunctionCode枚举
    QList<FunctionCode> codes = {FunctionCode::REQUEST, FunctionCode::RESPONSE};

    for (const auto& code : codes) {
        QString name = MessageTypeUtils::toString(code);
        qDebug() << "功能码:" << static_cast<int>(code) << "名称:" << name;
    }

    // 测试从字符串转换
    QString reqStr = MessageTypeUtils::toString(FunctionCode::REQUEST);
    FunctionCode reqCode = MessageTypeUtils::functionCodeFromString(reqStr);
    qDebug() << QString("字符串 '%1' -> 功能码 %2").arg(reqStr).arg(static_cast<int>(reqCode));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "ERNC协议模块集成测试开始";
    qDebug() << "=====================================";

    try {
        testMessageTypes();
        testMessageSerializer();
        testProtocolAdapter();
        testParameterMapping();
        runFunctionCodeTests();

        qDebug() << "\n=====================================";
        qDebug() << "所有测试完成 ✓";

    } catch (const std::exception& e) {
        qCritical() << "测试过程中发生异常:" << e.what();
        return 1;
    }

    return 0;
}
