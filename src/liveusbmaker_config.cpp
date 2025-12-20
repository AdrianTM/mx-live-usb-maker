/**********************************************************************
 *  liveusbmaker_config.cpp
 **********************************************************************
 * Copyright (C) 2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryFile>

#include "liveusbmaker_config.h"

namespace
{
QString modeToString(LiveUsbMakerConfig::Mode mode)
{
    switch (mode) {
    case LiveUsbMakerConfig::Mode::Dd:
        return QStringLiteral("dd");
    case LiveUsbMakerConfig::Mode::Normal:
    default:
        return QStringLiteral("normal");
    }
}

LiveUsbMakerConfig::Mode modeFromString(const QString &mode)
{
    if (mode == QLatin1String("dd")) {
        return LiveUsbMakerConfig::Mode::Dd;
    }
    return LiveUsbMakerConfig::Mode::Normal;
}

QString sourceModeToString(LiveUsbMakerConfig::SourceMode mode)
{
    switch (mode) {
    case LiveUsbMakerConfig::SourceMode::Clone:
        return QStringLiteral("clone");
    case LiveUsbMakerConfig::SourceMode::CloneDir:
        return QStringLiteral("clone-dir");
    case LiveUsbMakerConfig::SourceMode::Iso:
    default:
        return QStringLiteral("iso");
    }
}

LiveUsbMakerConfig::SourceMode sourceModeFromString(const QString &mode)
{
    if (mode == QLatin1String("clone")) {
        return LiveUsbMakerConfig::SourceMode::Clone;
    }
    if (mode == QLatin1String("clone-dir")) {
        return LiveUsbMakerConfig::SourceMode::CloneDir;
    }
    return LiveUsbMakerConfig::SourceMode::Iso;
}

QString jsonString(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (!value.isString()) {
        return {};
    }
    return value.toString();
}

bool jsonBool(const QJsonObject &object, const QString &key, bool fallback)
{
    const QJsonValue value = object.value(key);
    return value.isBool() ? value.toBool() : fallback;
}

int jsonInt(const QJsonObject &object, const QString &key, int fallback)
{
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(fallback) : fallback;
}
} // namespace

QJsonObject LiveUsbMakerConfig::toJson() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("mode"), modeToString(mode));
    obj.insert(QStringLiteral("source_mode"), sourceModeToString(sourceMode));
    obj.insert(QStringLiteral("source_path"), sourcePath);
    obj.insert(QStringLiteral("target_device"), targetDevice);

    obj.insert(QStringLiteral("pretend"), pretend);
    obj.insert(QStringLiteral("update"), update);
    obj.insert(QStringLiteral("keep_syslinux"), keepSyslinux);
    obj.insert(QStringLiteral("save_boot"), saveBoot);
    obj.insert(QStringLiteral("encrypt"), encrypt);
    obj.insert(QStringLiteral("gpt"), gpt);
    obj.insert(QStringLiteral("pmbr"), pmbr);
    obj.insert(QStringLiteral("force_usb"), forceUsb);
    obj.insert(QStringLiteral("force_automount"), forceAutomount);
    obj.insert(QStringLiteral("force_makefs"), forceMakefs);
    obj.insert(QStringLiteral("force_nofuse"), forceNofuse);

    obj.insert(QStringLiteral("esp_size_mib"), espSizeMiB);
    obj.insert(QStringLiteral("main_percent"), mainPercent);
    obj.insert(QStringLiteral("label"), label);

    obj.insert(QStringLiteral("data_first"), dataFirst);
    obj.insert(QStringLiteral("data_percent"), dataPercent);
    obj.insert(QStringLiteral("data_fs"), dataFs);

    obj.insert(QStringLiteral("verbosity"), verbosity);
    obj.insert(QStringLiteral("clone_persist"), clonePersist);
    return obj;
}

LiveUsbMakerConfig LiveUsbMakerConfig::fromJson(const QJsonObject &object, QString *error)
{
    if (error) {
        error->clear();
    }
    LiveUsbMakerConfig config;
    config.mode = modeFromString(jsonString(object, QStringLiteral("mode")));
    config.sourceMode = sourceModeFromString(jsonString(object, QStringLiteral("source_mode")));
    config.sourcePath = jsonString(object, QStringLiteral("source_path"));
    config.targetDevice = jsonString(object, QStringLiteral("target_device"));

    if (config.targetDevice.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Missing target device.");
        }
        return {};
    }

    config.pretend = jsonBool(object, QStringLiteral("pretend"), false);
    config.update = jsonBool(object, QStringLiteral("update"), false);
    config.keepSyslinux = jsonBool(object, QStringLiteral("keep_syslinux"), false);
    config.saveBoot = jsonBool(object, QStringLiteral("save_boot"), false);
    config.encrypt = jsonBool(object, QStringLiteral("encrypt"), false);
    config.gpt = jsonBool(object, QStringLiteral("gpt"), false);
    config.pmbr = jsonBool(object, QStringLiteral("pmbr"), false);
    config.forceUsb = jsonBool(object, QStringLiteral("force_usb"), false);
    config.forceAutomount = jsonBool(object, QStringLiteral("force_automount"), false);
    config.forceMakefs = jsonBool(object, QStringLiteral("force_makefs"), false);
    config.forceNofuse = jsonBool(object, QStringLiteral("force_nofuse"), false);

    config.espSizeMiB = jsonInt(object, QStringLiteral("esp_size_mib"), 50);
    config.mainPercent = jsonInt(object, QStringLiteral("main_percent"), 100);
    config.label = jsonString(object, QStringLiteral("label"));

    config.dataFirst = jsonBool(object, QStringLiteral("data_first"), false);
    config.dataPercent = jsonInt(object, QStringLiteral("data_percent"), 0);
    config.dataFs = jsonString(object, QStringLiteral("data_fs"));

    config.verbosity = jsonInt(object, QStringLiteral("verbosity"), 0);
    config.clonePersist = jsonBool(object, QStringLiteral("clone_persist"), true);
    return config;
}

QString LiveUsbMakerConfig::backendExecutablePath()
{
    const QString fromPath = QStandardPaths::findExecutable(QStringLiteral("mx-live-usb-maker-backend"));
    if (!fromPath.isEmpty()) {
        return fromPath;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString localCandidate = QDir(appDir).filePath(QStringLiteral("mx-live-usb-maker-backend"));
    if (QFile::exists(localCandidate)) {
        return localCandidate;
    }

    const QString fallback = QStringLiteral("/usr/lib/mx-live-usb-maker/mx-live-usb-maker-backend");
    if (QFile::exists(fallback)) {
        return fallback;
    }
    return {};
}

QString LiveUsbMakerConfig::writeToTempFile(const LiveUsbMakerConfig &config, QString *error)
{
    QTemporaryFile temp(QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                            .filePath(QStringLiteral("mx-live-usb-maker-XXXXXX.json")));
    temp.setAutoRemove(false);
    if (!temp.open()) {
        if (error) {
            *error = QStringLiteral("Unable to create temporary config file.");
        }
        return {};
    }

    const QJsonDocument doc(config.toJson());
    const QByteArray data = doc.toJson(QJsonDocument::Compact);
    if (temp.write(data) != data.size()) {
        if (error) {
            *error = QStringLiteral("Unable to write config file.");
        }
        return {};
    }
    temp.flush();
    return temp.fileName();
}

bool LiveUsbMakerConfig::readFromFile(const QString &path, LiveUsbMakerConfig *config, QString *error)
{
    if (!config) {
        if (error) {
            *error = QStringLiteral("Missing config output.");
        }
        return false;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = QStringLiteral("Unable to open config file.");
        }
        return false;
    }
    if (error) {
        error->clear();
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        if (error) {
            *error = QStringLiteral("Config file is not valid JSON.");
        }
        return false;
    }
    *config = LiveUsbMakerConfig::fromJson(doc.object(), error);
    if (!error || error->isEmpty()) {
        return true;
    }
    return false;
}
