#include "version_manager.h"
#include <QDebug>

namespace Protocol {

const QString VersionManager::DEFAULT_VERSION = "1.0.0";
const QStringList VersionManager::DEFAULT_SUPPORTED_VERSIONS = {
    "1.0.0", "1.0.1", "1.0.2", "1.1.0"
};

VersionManager::VersionManager(QObject* parent)
    : QObject(parent)
    , currentVersion_(QVersionNumber::fromString(DEFAULT_VERSION))
    , compatibilityMode_(CompatibilityMode::Minor)
{
    initializeDefaultSupportedVersions();
    qDebug() << "VersionManager initialized with version:" << DEFAULT_VERSION;
}

bool VersionManager::setCurrentVersion(const QString& version) {
    if (!isValidVersionFormat(version)) {
        qWarning() << "Invalid version format:" << version;
        return false;
    }

    QVersionNumber newVersion = QVersionNumber::fromString(version);
    if (newVersion.isNull()) {
        qWarning() << "Failed to parse version:" << version;
        return false;
    }

    currentVersion_ = newVersion;
    qInfo() << "Current version set to:" << version;
    return true;
}

QString VersionManager::getCurrentVersion() const {
    return currentVersion_.toString();
}

bool VersionManager::isCompatible(const QString& remoteVersion) const {
    QString reason;
    return isCompatible(remoteVersion, reason);
}

bool VersionManager::isCompatible(const QString& remoteVersion, QString& reason) const {
    if (!isValidVersionFormat(remoteVersion)) {
        reason = QString("Invalid remote version format: %1").arg(remoteVersion);
        qWarning() << reason;
        return false;
    }

    QVersionNumber remote = QVersionNumber::fromString(remoteVersion);
    if (remote.isNull()) {
        reason = QString("Failed to parse remote version: %1").arg(remoteVersion);
        qWarning() << reason;
        return false;
    }

    bool compatible = false;

    switch (compatibilityMode_) {
    case CompatibilityMode::Strict:
        compatible = checkStrictCompatibility(remote, reason);
        break;
    case CompatibilityMode::Backward:
        compatible = checkBackwardCompatibility(remote, reason);
        break;
    case CompatibilityMode::Forward:
        compatible = checkForwardCompatibility(remote, reason);
        break;
    case CompatibilityMode::Minor:
        compatible = checkMinorCompatibility(remote, reason);
        break;
    }

    if (!compatible) {
        qWarning() << "Version incompatibility:" << reason;
        emit const_cast<VersionManager*>(this)->versionIncompatible(
            getCurrentVersion(), remoteVersion, reason);
    } else {
        // 检查是否需要发出警告
        if (currentVersion_ != remote) {
            QString warning = QString("Version mismatch (compatible): local=%1, remote=%2")
                             .arg(getCurrentVersion())
                             .arg(remoteVersion);
            qInfo() << warning;
            emit const_cast<VersionManager*>(this)->versionCompatibilityWarning(
                getCurrentVersion(), remoteVersion, warning);
        }
    }

    return compatible;
}

int VersionManager::compareVersions(const QString& version1, const QString& version2) {
    QVersionNumber v1 = QVersionNumber::fromString(version1);
    QVersionNumber v2 = QVersionNumber::fromString(version2);

    if (v1.isNull() || v2.isNull()) {
        qWarning() << "Invalid version format in comparison:" << version1 << "vs" << version2;
        return 0;
    }

    return QVersionNumber::compare(v1, v2);
}

bool VersionManager::isValidVersionFormat(const QString& version) {
    if (version.isEmpty()) {
        return false;
    }

    QVersionNumber v = QVersionNumber::fromString(version);
    return !v.isNull() && v.segmentCount() >= 2; // 至少要有主版本号和次版本号
}

QStringList VersionManager::getSupportedVersions() const {
    QStringList result;
    for (const QVersionNumber& version : supportedVersions_) {
        result << version.toString();
    }
    return result;
}

bool VersionManager::addSupportedVersion(const QString& version) {
    if (!isValidVersionFormat(version)) {
        qWarning() << "Cannot add invalid version:" << version;
        return false;
    }

    QVersionNumber versionNumber = QVersionNumber::fromString(version);
    if (supportedVersions_.contains(versionNumber)) {
        qDebug() << "Version already supported:" << version;
        return true;
    }

    supportedVersions_.append(versionNumber);
    qInfo() << "Added supported version:" << version;
    return true;
}

bool VersionManager::removeSupportedVersion(const QString& version) {
    QVersionNumber versionNumber = QVersionNumber::fromString(version);
    int removed = supportedVersions_.removeAll(versionNumber);

    if (removed > 0) {
        qInfo() << "Removed supported version:" << version;
        return true;
    } else {
        qWarning() << "Version not found in supported list:" << version;
        return false;
    }
}

void VersionManager::clearSupportedVersions() {
    supportedVersions_.clear();
    qInfo() << "All supported versions cleared";
}

QString VersionManager::getVersionSummary() const {
    QString modeStr;
    switch (compatibilityMode_) {
    case CompatibilityMode::Strict:
        modeStr = "Strict";
        break;
    case CompatibilityMode::Backward:
        modeStr = "Backward";
        break;
    case CompatibilityMode::Forward:
        modeStr = "Forward";
        break;
    case CompatibilityMode::Minor:
        modeStr = "Minor";
        break;
    }

    return QString("Current: %1, Mode: %2, Supported: [%3]")
           .arg(getCurrentVersion())
           .arg(modeStr)
           .arg(getSupportedVersions().join(", "));
}

void VersionManager::initializeDefaultSupportedVersions() {
    for (const QString& version : DEFAULT_SUPPORTED_VERSIONS) {
        QVersionNumber versionNumber = QVersionNumber::fromString(version);
        if (!versionNumber.isNull()) {
            supportedVersions_.append(versionNumber);
        }
    }

    qDebug() << "Default supported versions initialized:" << supportedVersions_.size() << "versions";
}

bool VersionManager::checkStrictCompatibility(const QVersionNumber& remoteVersion, QString& reason) const {
    if (currentVersion_ == remoteVersion) {
        return true;
    }

    reason = QString("Strict mode requires exact version match. Local: %1, Remote: %2")
             .arg(currentVersion_.toString())
             .arg(remoteVersion.toString());
    return false;
}

bool VersionManager::checkBackwardCompatibility(const QVersionNumber& remoteVersion, QString& reason) const {
    // 向后兼容：远程版本不能高于当前版本
    if (QVersionNumber::compare(remoteVersion, currentVersion_) <= 0) {
        return true;
    }

    reason = QString("Backward compatibility mode: remote version too high. Local: %1, Remote: %2")
             .arg(currentVersion_.toString())
             .arg(remoteVersion.toString());
    return false;
}

bool VersionManager::checkForwardCompatibility(const QVersionNumber& remoteVersion, QString& reason) const {
    // 向前兼容：远程版本不能低于当前版本
    if (QVersionNumber::compare(remoteVersion, currentVersion_) >= 0) {
        return true;
    }

    reason = QString("Forward compatibility mode: remote version too low. Local: %1, Remote: %2")
             .arg(currentVersion_.toString())
             .arg(remoteVersion.toString());
    return false;
}

bool VersionManager::checkMinorCompatibility(const QVersionNumber& remoteVersion, QString& reason) const {
    // 次版本兼容：主版本号必须相同
    if (currentVersion_.majorVersion() == remoteVersion.majorVersion()) {
        return true;
    }

    reason = QString("Minor compatibility mode: major version mismatch. Local: %1.x.x, Remote: %2.x.x")
             .arg(currentVersion_.majorVersion())
             .arg(remoteVersion.majorVersion());
    return false;
}

} // namespace Protocol
