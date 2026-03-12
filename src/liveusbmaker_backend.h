/**********************************************************************
 *  liveusbmaker_backend.h
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
#pragma once

#include <QString>

#include "liveusbmaker_config.h"

class LiveUsbMakerBackend
{
public:
    explicit LiveUsbMakerBackend(const LiveUsbMakerConfig &config);

    [[nodiscard]] bool run(QString *error);

private:
    struct Paths {
        QString workDir;
        QString isoDir;
        QString mainDir;
        QString biosDir;
        QString uefiDir;
        QString dataDir;
        QString initrdDir;
        QString linuxDir;
    };

    struct DeviceLayout {
        QString drive;
        QString mainDev;
        QString biosDev;
        QString uefiDev;
        QString dataDev;
        int biosPart{0};
        int mainPart{0};
        int uefiPart{0};
        int biosSizeMiB{0};
        int mainSizeMiB{0};
        int uefiSizeMiB{0};
        int dataSizeMiB{0};
    };

    const LiveUsbMakerConfig config;
    Paths paths;
    DeviceLayout layout;
    bool archIso{};
    QString archIsoArch;

    bool suspendAutomount();
    void resumeAutomount();
    bool prepareWorkDirs(QString *error);
    bool prepareSource(QString *error);
    bool cleanup();

    bool runDd(QString *error);
    bool runNormal(QString *error);

    bool checkTargetDevice(QString *error);
    bool isUsbOrRemovable(const QString &device) const;

    bool clearPartitionTable(QString *error);
    bool computeLayout(QString *error);
    bool partitionDevice(QString *error);
    bool makeFileSystems(QString *error);
    bool mountTargets(bool mountMain, QString *error);
    bool unmountTargets();

    bool copyMain(QString *error);
    bool copyBios(QString *error);
    bool copyUefi(QString *error);
    bool checkUsbMd5(QString *error);

    bool installBootloader(QString *error);
    bool installArchIsoBootloader(QString *error) const;
    bool updateUuids(QString *error);
    bool updateArchIsoBootConfig(QString *error) const;
    bool setupArchLiveUsbStorage();
    bool writeDataUuid(QString *error);
    bool fixUefiMemtest(QString *error);

    bool prepareInitrdEncryption(QString *error);
    bool finalizeInitrdEncryption(QString *error);
    bool encryptMainPartition(QString *error);

    QString logPath() const;
    bool runCommand(const QString &program, const QStringList &args, QString *error, bool allowFail = false) const;
    bool runCommandShell(const QString &command, QString *error, bool allowFail = false) const;
    bool runCommandOutput(const QString &program, const QStringList &args, QString *output, QString *error) const;
    void logLine(const QString &line) const;
    void logWarn(const QString &line) const;
    void logError(const QString &line) const;

    [[nodiscard]] QString devicePath(const QString &device) const;
    [[nodiscard]] QString partitionPath(const QString &drive, int index) const;
    [[nodiscard]] int totalSizeMiB(QString *error) const;
    [[nodiscard]] int duApparentSizeMiB(const QString &dir, const QString &spec, QString *error) const;
    [[nodiscard]] int extOverheadMiB(int sizeMiB) const;
    bool detectArchIsoLayout();

    [[nodiscard]] QString findSyslinuxMbr(const QString &name, QString *error) const;
    [[nodiscard]] QString findSyslinuxModuleDir(QString *error) const;
    [[nodiscard]] QString findSyslinuxDir(const QString &biosDir) const;
    [[nodiscard]] QString randomString(int bytes) const;

    bool copyFileTree(const QString &source, const QString &destination, QString *error) const;
    bool copyFilesSpec(const QString &source, const QString &spec, const QString &destination, QString *error) const;
    bool makeFs(const QString &device, const QString &type, const QString &label, QString *error) const;
    bool mountDevice(const QString &device, const QString &mountPoint, const QString &fsType, QString *error) const;
    bool unmountPath(const QString &mountPoint, QString *error) const;
    bool isMountpoint(const QString &path) const;

    bool unpackInitrd(const QString &initrdPath, const QString &destDir, QString *error) const;
    bool repackInitrd(const QString &initrdPath, const QString &srcDir, QString *error) const;
    bool copyInitrdPrograms(const QString &linuxDir, const QString &initrdDir, QString *error) const;
    bool copyInitrdModules(const QString &linuxDir, const QString &initrdDir, QString *error) const;
    bool writePassphraseFile(const QString &path, QString *error) const;
};
