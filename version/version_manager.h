#ifndef VERSION_MANAGER_H
#define VERSION_MANAGER_H

#include <QObject>
#include <QString>
#include <QVersionNumber>
#include <QStringList>

namespace Protocol {

/**
 * @brief 协议版本管理器
 *
 * 负责管理协议版本信息，检查版本兼容性
 * 支持语义化版本控制（Semantic Versioning）
 */
class VersionManager : public QObject {
    Q_OBJECT

public:
    explicit VersionManager(QObject* parent = nullptr);
    ~VersionManager() override = default;

    /**
     * @brief 设置当前协议版本
     * @param version 版本字符串（例如："1.2.3"）
     * @return 成功返回true，失败返回false
     */
    bool setCurrentVersion(const QString& version);

    /**
     * @brief 获取当前协议版本
     * @return 版本字符串
     */
    QString getCurrentVersion() const;

    /**
     * @brief 获取当前版本号对象
     * @return 版本号对象
     */
    QVersionNumber getCurrentVersionNumber() const { return currentVersion_; }

    /**
     * @brief 检查版本兼容性
     * @param remoteVersion 远程版本字符串
     * @return 兼容返回true，不兼容返回false
     */
    bool isCompatible(const QString& remoteVersion) const;

    /**
     * @brief 检查版本兼容性（详细信息）
     * @param remoteVersion 远程版本字符串
     * @param reason 不兼容原因（输出参数）
     * @return 兼容返回true，不兼容返回false
     */
    bool isCompatible(const QString& remoteVersion, QString& reason) const;

    /**
     * @brief 比较两个版本
     * @param version1 版本1
     * @param version2 版本2
     * @return 小于返回-1，等于返回0，大于返回1
     */
    static int compareVersions(const QString& version1, const QString& version2);

    /**
     * @brief 检查版本字符串格式是否有效
     * @param version 版本字符串
     * @return 有效返回true，无效返回false
     */
    static bool isValidVersionFormat(const QString& version);

    /**
     * @brief 获取支持的版本列表
     * @return 支持的版本字符串列表
     */
    QStringList getSupportedVersions() const;

    /**
     * @brief 添加支持的版本
     * @param version 版本字符串
     * @return 成功返回true，失败返回false
     */
    bool addSupportedVersion(const QString& version);

    /**
     * @brief 移除支持的版本
     * @param version 版本字符串
     * @return 成功返回true，失败返回false
     */
    bool removeSupportedVersion(const QString& version);

    /**
     * @brief 清除所有支持的版本
     */
    void clearSupportedVersions();

    /**
     * @brief 设置兼容性策略
     */
    enum class CompatibilityMode {
        Strict,      // 严格模式：主版本号必须相同
        Backward,    // 向后兼容：远程版本不能高于当前版本
        Forward,     // 向前兼容：远程版本不能低于当前版本
        Minor        // 次版本兼容：主版本号相同即可
    };

    void setCompatibilityMode(CompatibilityMode mode) { compatibilityMode_ = mode; }
    CompatibilityMode getCompatibilityMode() const { return compatibilityMode_; }

    /**
     * @brief 获取版本信息摘要
     * @return 版本信息字符串
     */
    QString getVersionSummary() const;

signals:
    /**
     * @brief 版本不兼容信号
     * @param currentVersion 当前版本
     * @param remoteVersion 远程版本
     * @param reason 不兼容原因
     */
    void versionIncompatible(const QString& currentVersion, const QString& remoteVersion, const QString& reason);

    /**
     * @brief 版本兼容性警告信号
     * @param currentVersion 当前版本
     * @param remoteVersion 远程版本
     * @param warning 警告信息
     */
    void versionCompatibilityWarning(const QString& currentVersion, const QString& remoteVersion, const QString& warning);

private:
    /**
     * @brief 初始化默认支持版本
     */
    void initializeDefaultSupportedVersions();

    /**
     * @brief 检查严格兼容性
     * @param remoteVersion 远程版本
     * @param reason 不兼容原因
     * @return 兼容性结果
     */
    bool checkStrictCompatibility(const QVersionNumber& remoteVersion, QString& reason) const;

    /**
     * @brief 检查向后兼容性
     * @param remoteVersion 远程版本
     * @param reason 不兼容原因
     * @return 兼容性结果
     */
    bool checkBackwardCompatibility(const QVersionNumber& remoteVersion, QString& reason) const;

    /**
     * @brief 检查向前兼容性
     * @param remoteVersion 远程版本
     * @param reason 不兼容原因
     * @return 兼容性结果
     */
    bool checkForwardCompatibility(const QVersionNumber& remoteVersion, QString& reason) const;

    /**
     * @brief 检查次版本兼容性
     * @param remoteVersion 远程版本
     * @param reason 不兼容原因
     * @return 兼容性结果
     */
    bool checkMinorCompatibility(const QVersionNumber& remoteVersion, QString& reason) const;

private:
    QVersionNumber currentVersion_;          // 当前版本
    QList<QVersionNumber> supportedVersions_; // 支持的版本列表
    CompatibilityMode compatibilityMode_;    // 兼容性模式

    // 默认版本信息
    static const QString DEFAULT_VERSION;
    static const QStringList DEFAULT_SUPPORTED_VERSIONS;
};

} // namespace Protocol

#endif // VERSION_MANAGER_H
