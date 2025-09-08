#include <QtTest>
#include <QSignalSpy>
#include "../serialization/message_serializer.h"

using namespace Protocol;

class TestMessageSerializer : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础序列化测试
    void testAncSerialization();
    void testEncSerialization();
    void testAlphaSerialization();

    // 反序列化测试
    void testAncDeserialization();
    void testInvalidDeserialization();

    // 参数验证测试
    void testParameterValidation();
    void testInvalidParameters();

    // 信号测试
    void testSerializationSignals();

    // 边界情况测试
    void testEmptyParameters();
    void testUnsupportedMessageType();

private:
    MessageSerializer* serializer_;
};

void TestMessageSerializer::initTestCase()
{
    qDebug() << "Starting MessageSerializer tests";
}

void TestMessageSerializer::cleanupTestCase()
{
    qDebug() << "MessageSerializer tests completed";
}

void TestMessageSerializer::init()
{
    serializer_ = new MessageSerializer();
}

void TestMessageSerializer::cleanup()
{
    delete serializer_;
    serializer_ = nullptr;
}

void TestMessageSerializer::testAncSerialization()
{
    // 准备ANC参数
    QVariantMap parameters;
    parameters["anc.enabled"] = true;

    // 测试序列化
    QByteArray result = serializer_->serialize(MessageType::ANC_OFF, parameters);
    QVERIFY(!result.isEmpty());
    QVERIFY(result.size() > 0);

    // 验证可以反序列化
    QVariantMap deserializedParams;
    bool success = serializer_->deserialize(MessageType::ANC_OFF, result, deserializedParams);
    QVERIFY(success);
    QVERIFY(deserializedParams.contains("anc.enabled"));
}

void TestMessageSerializer::testEncSerialization()
{
    // 准备ENC参数
    QVariantMap parameters;
    parameters["enc.enabled"] = false;

    // 测试序列化
    QByteArray result = serializer_->serialize(MessageType::ENC_OFF, parameters);
    QVERIFY(!result.isEmpty());

    // 验证往返序列化
    QVariantMap deserializedParams;
    bool success = serializer_->deserialize(MessageType::ENC_OFF, result, deserializedParams);
    QVERIFY(success);
    QCOMPARE(deserializedParams["enc.enabled"].toBool(), false);
}

void TestMessageSerializer::testAlphaSerialization()
{
    // 准备Alpha参数
    QVariantMap parameters;
    parameters["processing.alpha"] = 0.75f;

    // 测试序列化
    QByteArray result = serializer_->serialize(MessageType::ALPHA, parameters);
    QVERIFY(!result.isEmpty());

    // 验证往返序列化
    QVariantMap deserializedParams;
    bool success = serializer_->deserialize(MessageType::ALPHA, result, deserializedParams);
    QVERIFY(success);
    QVERIFY(deserializedParams.contains("processing.alpha"));

    float alphaValue = deserializedParams["processing.alpha"].toFloat();
    QCOMPARE(alphaValue, 0.75f);
}

void TestMessageSerializer::testAncDeserialization()
{
    // 首先序列化一个已知的参数
    QVariantMap originalParams;
    originalParams["anc.enabled"] = true;

    QByteArray data = serializer_->serialize(MessageType::ANC_OFF, originalParams);
    QVERIFY(!data.isEmpty());

    // 测试反序列化
    QVariantMap deserializedParams;
    bool success = serializer_->deserialize(MessageType::ANC_OFF, data, deserializedParams);

    QVERIFY(success);
    QVERIFY(deserializedParams.contains("anc.enabled"));
    QCOMPARE(deserializedParams["anc.enabled"].toBool(), true);
}

void TestMessageSerializer::testInvalidDeserialization()
{
    // 测试空数据反序列化
    QVariantMap parameters;
    bool success = serializer_->deserialize(MessageType::ANC_OFF, QByteArray(), parameters);
    QVERIFY(!success);

    // 测试无效数据反序列化
    QByteArray invalidData("invalid_protobuf_data");
    success = serializer_->deserialize(MessageType::ANC_OFF, invalidData, parameters);
    QVERIFY(!success);
}

void TestMessageSerializer::testParameterValidation()
{
    // 测试有效参数验证
    QVariantMap validParams;
    validParams["anc.enabled"] = true;

    bool isValid = serializer_->validateParameters(MessageType::ANC_OFF, validParams);
    QVERIFY(isValid);

    // 测试Alpha参数验证
    QVariantMap alphaParams;
    alphaParams["processing.alpha"] = 0.5f;

    isValid = serializer_->validateParameters(MessageType::ALPHA, alphaParams);
    QVERIFY(isValid);
}

void TestMessageSerializer::testInvalidParameters()
{
    // 测试缺少必需参数
    QVariantMap emptyParams;
    bool isValid = serializer_->validateParameters(MessageType::ANC_OFF, emptyParams);
    QVERIFY(!isValid);

    // 测试错误的参数类型
    QVariantMap invalidParams;
    invalidParams["anc.enabled"] = "not_a_bool"; // 应该是bool类型

    isValid = serializer_->validateParameters(MessageType::ANC_OFF, invalidParams);
    QVERIFY(!isValid);

    // 测试Alpha参数范围验证
    QVariantMap outOfRangeParams;
    outOfRangeParams["processing.alpha"] = 2.0f; // 超出[0.0, 1.0]范围

    isValid = serializer_->validateParameters(MessageType::ALPHA, outOfRangeParams);
    QVERIFY(!isValid);
}

void TestMessageSerializer::testSerializationSignals()
{
    // 创建信号监听器
    QSignalSpy serializationSpy(serializer_, &MessageSerializer::serializationCompleted);
    QSignalSpy errorSpy(serializer_, &MessageSerializer::serializationError);

    // 测试成功序列化的信号
    QVariantMap validParams;
    validParams["anc.enabled"] = true;

    QByteArray result = serializer_->serialize(MessageType::ANC_OFF, validParams);
    QVERIFY(!result.isEmpty());

    // 验证成功信号
    QCOMPARE(serializationSpy.count(), 1);
    QList<QVariant> arguments = serializationSpy.takeFirst();
    QCOMPARE(arguments.at(0).value<MessageType>(), MessageType::ANC_OFF);
    QCOMPARE(arguments.at(1).toBool(), true); // success
    QVERIFY(arguments.at(2).toInt() > 0); // dataSize

    // 测试失败序列化的信号
    QVariantMap invalidParams;
    // 不提供必需的参数

    result = serializer_->serialize(MessageType::ANC_OFF, invalidParams);
    QVERIFY(result.isEmpty());

    // 验证错误信号
    QCOMPARE(errorSpy.count(), 1);
}

void TestMessageSerializer::testEmptyParameters()
{
    // 测试空参数映射
    QVariantMap emptyParams;

    QByteArray result = serializer_->serialize(MessageType::ANC_OFF, emptyParams);
    QVERIFY(result.isEmpty()); // 应该失败

    // 测试参数验证
    bool isValid = serializer_->validateParameters(MessageType::ANC_OFF, emptyParams);
    QVERIFY(!isValid);
}

void TestMessageSerializer::testUnsupportedMessageType()
{
    // 获取支持的消息类型
    auto supportedTypes = serializer_->getSupportedMessageTypes();
    QVERIFY(!supportedTypes.isEmpty());

    // 验证已知的类型被支持
    QVERIFY(serializer_->isMessageTypeSupported(MessageType::ANC_OFF));
    QVERIFY(serializer_->isMessageTypeSupported(MessageType::ENC_OFF));
    QVERIFY(serializer_->isMessageTypeSupported(MessageType::ALPHA));

    // 测试消息类型描述
    QString description = serializer_->getMessageTypeDescription(MessageType::ANC_OFF);
    QVERIFY(!description.isEmpty());
}

QTEST_MAIN(TestMessageSerializer)
#include "test_message_serializer.moc"
