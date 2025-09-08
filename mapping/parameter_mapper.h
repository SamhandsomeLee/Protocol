#ifndef PARAMETER_MAPPER_H
#define PARAMETER_MAPPER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QStringList>
#include <QHash>
#include <QJsonObject>
#include "../core/message_types.h"

namespace Protocol {

/**
 * @brief 参数映射信息
 */
struct ParameterInfo {
    QString logicalPath;        // 逻辑参数路径 (例如: "anc.enabled")
    QString protobufPath;       // protobuf 字段路径 (例如: "value")
    QString fieldType;          // 字段类型 (bool, int32, float, string)
    QVariant defaultValue;      // 默认值
    MessageType messageType;    // 对应的消息类型
    bool deprecated = false;    // 是否已弃用
    QString replacedBy;         // 替换参数（如果已弃用）
    QString description;        // 参数描述

    bool isValid() const { return !logicalPath.isEmpty() && !protobufPath.isEmpty(); }
};

/**
 * @brief 参数映射器
 *
 * 负责管理参数路径到protobuf字段的映射关系
 * 支持从JSON配置文件加载映射信息
 */
class ParameterMapper : public QObject {
    Q_OBJECT

public:
    explicit ParameterMapper(QObject* parent = nullptr);
    ~ParameterMapper() override = default;

    /**
     * @brief 从JSON文件加载参数映射
     * @param configFile JSON配置文件路径
     * @return 成功返回true，失败返回false
     */
    bool loadMapping(const QString& configFile);

    /**
     * @brief 从JSON对象加载参数映射
     * @param jsonObject JSON对象
     * @return 成功返回true，失败返回false
     */
    bool loadMappingFromJson(const QJsonObject& jsonObject);

    /**
     * @brief 获取参数信息
     * @param parameterPath 参数路径
     * @return 参数信息，不存在则返回无效的ParameterInfo
     */
    ParameterInfo getParameterInfo(const QString& parameterPath) const;

    /**
     * @brief 检查参数是否支持
     * @param parameterPath 参数路径
     * @return 支持返回true，不支持返回false
     */
    bool isParameterSupported(const QString& parameterPath) const;

    /**
     * @brief 获取所有支持的参数列表
     * @return 参数路径列表
     */
    QStringList getSupportedParameters() const;

    /**
     * @brief 根据消息类型获取相关参数
     * @param messageType 消息类型
     * @return 该消息类型相关的参数路径列表
     */
    QStringList getParametersForMessageType(MessageType messageType) const;

    /**
     * @brief 检查参数是否已弃用
     * @param parameterPath 参数路径
     * @return 已弃用返回true，否则返回false
     */
    bool isParameterDeprecated(const QString& parameterPath) const;

    /**
     * @brief 获取弃用参数的替换参数
     * @param parameterPath 参数路径
     * @return 替换参数路径，如果不是弃用参数则返回空字符串
     */
    QString getReplacementParameter(const QString& parameterPath) const;

    /**
     * @brief 清除所有映射
     */
    void clear();

    /**
     * @brief 获取映射数量
     * @return 映射数量
     */
    int mappingCount() const;

signals:
    /**
     * @brief 映射加载完成信号
     * @param success 是否成功
     * @param errorMessage 错误信息（如果失败）
     */
    void mappingLoaded(bool success, const QString& errorMessage = QString());

    /**
     * @brief 发现弃用参数使用信号
     * @param deprecatedPath 弃用的参数路径
     * @param replacementPath 替换参数路径
     */
    void deprecatedParameterUsed(const QString& deprecatedPath, const QString& replacementPath);

private:
    /**
     * @brief 初始化默认映射
     */
    void initializeDefaultMappings();

    /**
     * @brief 验证参数信息有效性
     * @param info 参数信息
     * @return 有效返回true，无效返回false
     */
    bool validateParameterInfo(const ParameterInfo& info) const;

    /**
     * @brief 从JSON对象解析参数信息
     * @param jsonParam JSON参数对象
     * @return 参数信息
     */
    ParameterInfo parseParameterFromJson(const QJsonObject& jsonParam) const;

private:
    QHash<QString, ParameterInfo> mappings_;  // 参数路径到信息的映射
    bool initialized_ = false;                // 是否已初始化
};

} // namespace Protocol

#endif // PARAMETER_MAPPER_H
