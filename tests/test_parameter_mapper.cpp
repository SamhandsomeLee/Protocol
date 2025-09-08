#include <QtTest>
#include <QJsonObject>
#include <QJsonDocument>
#include "../mapping/parameter_mapper.h"

using namespace Protocol;

class TestParameterMapper : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础功能测试
    void testDefaultMappings();
    void testParameterSupport();
    void testParameterInfo();

    // JSON加载测试
    void testLoadFromJson();
    void testInvalidJson();

    // 参数管理测试
    void testParametersByMessageType();
    void testDeprecatedParameters();

    // 边界情况测试
    void testEmptyMapping();
    void testInvalidParameters();

private:
    ParameterMapper* mapper_;
    QJsonObject createTestMapping();
};

void TestParameterMapper::initTestCase()
{
    qDebug() << "Starting ParameterMapper tests";
}

void TestParameterMapper::cleanupTestCase()
{
    qDebug() << "ParameterMapper tests completed";
}

void TestParameterMapper::init()
{
    mapper_ = new ParameterMapper();
}

void TestParameterMapper::cleanup()
{
    delete mapper_;
    mapper_ = nullptr;
}

void TestParameterMapper::testDefaultMappings()
{
    // 测试默认映射是否正确加载
    QVERIFY(mapper_->isParameterSupported("anc.enabled"));
    QVERIFY(mapper_->isParameterSupported("enc.enabled"));
    QVERIFY(mapper_->isParameterSupported("rnc.enabled"));

    // 测试参数数量
    QVERIFY(mapper_->mappingCount() >= 5); // 至少应该有5个默认参数

    // 测试参数信息
    auto ancInfo = mapper_->getParameterInfo("anc.enabled");
    QVERIFY(ancInfo.isValid());
    QCOMPARE(ancInfo.logicalPath, QString("anc.enabled"));
    QCOMPARE(ancInfo.protobufPath, QString("value"));
    QCOMPARE(ancInfo.fieldType, QString("bool"));
    QCOMPARE(ancInfo.messageType, MessageType::ANC_OFF);
}

void TestParameterMapper::testParameterSupport()
{
    // 测试支持的参数
    QVERIFY(mapper_->isParameterSupported("anc.enabled"));

    // 测试不支持的参数
    QVERIFY(!mapper_->isParameterSupported("unknown.parameter"));
    QVERIFY(!mapper_->isParameterSupported(""));

    // 测试获取支持的参数列表
    QStringList supportedParams = mapper_->getSupportedParameters();
    QVERIFY(!supportedParams.isEmpty());
    QVERIFY(supportedParams.contains("anc.enabled"));
}

void TestParameterMapper::testParameterInfo()
{
    // 测试有效参数的信息
    auto info = mapper_->getParameterInfo("anc.enabled");
    QVERIFY(info.isValid());
    QCOMPARE(info.logicalPath, QString("anc.enabled"));

    // 测试无效参数的信息
    auto invalidInfo = mapper_->getParameterInfo("invalid.parameter");
    QVERIFY(!invalidInfo.isValid());
}

void TestParameterMapper::testLoadFromJson()
{
    QJsonObject testMapping = createTestMapping();

    // 测试成功加载
    bool success = mapper_->loadMappingFromJson(testMapping);
    QVERIFY(success);

    // 验证加载的参数
    QVERIFY(mapper_->isParameterSupported("test.parameter1"));
    QVERIFY(mapper_->isParameterSupported("test.parameter2"));

    auto info1 = mapper_->getParameterInfo("test.parameter1");
    QVERIFY(info1.isValid());
    QCOMPARE(info1.fieldType, QString("bool"));
    QCOMPARE(info1.messageType, MessageType::ANC_OFF);
}

void TestParameterMapper::testInvalidJson()
{
    // 测试空JSON对象
    QJsonObject emptyJson;
    bool success = mapper_->loadMappingFromJson(emptyJson);
    QVERIFY(!success);

    // 测试没有parameters字段的JSON
    QJsonObject invalidJson;
    invalidJson["version"] = "1.0.0";
    success = mapper_->loadMappingFromJson(invalidJson);
    QVERIFY(!success);
}

void TestParameterMapper::testParametersByMessageType()
{
    // 测试根据消息类型获取参数
    QStringList ancParams = mapper_->getParametersForMessageType(MessageType::ANC_OFF);
    QVERIFY(ancParams.contains("anc.enabled"));

    QStringList encParams = mapper_->getParametersForMessageType(MessageType::ENC_OFF);
    QVERIFY(encParams.contains("enc.enabled"));
}

void TestParameterMapper::testDeprecatedParameters()
{
    // 创建包含弃用参数的测试映射
    QJsonObject testMapping;
    QJsonArray parameters;

    QJsonObject deprecatedParam;
    deprecatedParam["logical_path"] = "old.parameter";
    deprecatedParam["protobuf_path"] = "old_value";
    deprecatedParam["field_type"] = "bool";
    deprecatedParam["message_type"] = "ANC_OFF";
    deprecatedParam["deprecated"] = true;
    deprecatedParam["replaced_by"] = "new.parameter";
    parameters.append(deprecatedParam);

    testMapping["parameters"] = parameters;

    // 加载测试映射
    bool success = mapper_->loadMappingFromJson(testMapping);
    QVERIFY(success);

    // 测试弃用参数检测
    QVERIFY(mapper_->isParameterDeprecated("old.parameter"));
    QCOMPARE(mapper_->getReplacementParameter("old.parameter"), QString("new.parameter"));

    // 测试非弃用参数
    QVERIFY(!mapper_->isParameterDeprecated("anc.enabled"));
}

void TestParameterMapper::testEmptyMapping()
{
    // 清空映射
    mapper_->clear();
    QCOMPARE(mapper_->mappingCount(), 0);
    QVERIFY(mapper_->getSupportedParameters().isEmpty());
}

void TestParameterMapper::testInvalidParameters()
{
    // 测试无效的参数路径
    QVERIFY(!mapper_->isParameterSupported(""));
    QVERIFY(!mapper_->isParameterSupported(" "));

    auto info = mapper_->getParameterInfo("");
    QVERIFY(!info.isValid());
}

QJsonObject TestParameterMapper::createTestMapping()
{
    QJsonObject mapping;
    QJsonArray parameters;

    // 测试参数1
    QJsonObject param1;
    param1["logical_path"] = "test.parameter1";
    param1["protobuf_path"] = "test_value1";
    param1["field_type"] = "bool";
    param1["message_type"] = "ANC_OFF";
    param1["description"] = "Test parameter 1";
    parameters.append(param1);

    // 测试参数2
    QJsonObject param2;
    param2["logical_path"] = "test.parameter2";
    param2["protobuf_path"] = "test_value2";
    param2["field_type"] = "float";
    param2["message_type"] = "ALPHA";
    param2["description"] = "Test parameter 2";
    parameters.append(param2);

    mapping["parameters"] = parameters;
    return mapping;
}

QTEST_MAIN(TestParameterMapper)
#include "test_parameter_mapper.moc"
