/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2017 MX Authors
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

#include <QElapsedTimer>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTimer>

#include "cmd.h"
#include "common.h"
#include "liveusbmaker_config.h"

class QFile;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(const QStringList &args, QDialog *parent = nullptr);
    ~MainWindow() override;

public slots:

private slots:
    static bool isAntiXMxFamily(const QString &selected);
    static bool isArchIsoFamily(const QString &selected);
    void cleanup();
    void cmdDone();
    void setConnections();
    void setDefaultMode(const QString &isoName);
    void updateBar();
    void updateOutput();

    void checkCloneLiveClicked(bool checked);
    void checkCloneModeClicked(bool checked);
    void checkDataFirstClicked(bool checked);
    void checkUpdateClicked(bool checked);
    void pushAboutClicked();
    void pushBackClicked();
    void pushHelpClicked();
    void pushLumLogFileClicked();
    void pushNextClicked();
    void pushOptionsClicked();
    void pushRefreshClicked();
    void pushSelectSourceClicked();
    void radioDdClicked();
    void radioNormalClicked();
    void spinBoxSizeValueChanged(int value);
    void textLabelTextChanged(const QString &text);

private:
    Ui::MainWindow *ui;
    Cmd cmd;
    QString backendPath;
    QString device;
    QString lastConfigPath;
    QTimer timer;
    QElapsedTimer elapsedTimer;
    bool advancedOptions{};
    bool operationInProgress{};
    int defaultHeight{};
    int height{};
    uint sizeCheck;

    [[nodiscard]] QString buildOptionList();
    [[nodiscard]] QStringList buildUsbList();
    [[nodiscard]] QStringList removeUnsuitable(const QStringList &devices); // remove live or unremovable
    [[nodiscard]] bool checkDestSize();
    [[nodiscard]] bool confirmLargeDeviceWarning(const quint64 diskSize);
    [[nodiscard]] bool validateSizeCompatibility(const quint64 sourceSize, const quint64 diskSize);
    [[nodiscard]] quint64 calculateSourceSize();
    [[nodiscard]] QString getLinuxfsPath(const QString &sourceFilename);
    [[nodiscard]] quint64 calculateLinuxfsSize(const QString &linuxfsPath);
    [[nodiscard]] quint64 calculateIsoSize(const QString &isoFilename);
    [[nodiscard]] static QString expandDevicePath(const QString &device);
    [[nodiscard]] static QString getDriveName(const QString &device);
    [[nodiscard]] static QString getDrivePath(const QString &device);
    [[nodiscard]] static QString getLiveDeviceName();
    [[nodiscard]] static QString readInitrdParam(const QString &name,
                                                 const QString &filePath = LivePaths::INITRD_OUT);
    [[nodiscard]] static bool isRunningLive();
    [[nodiscard]] static bool isToRam();
    [[nodiscard]] static bool isUsbOrRemovable(const QString &device);
    [[nodiscard]] LiveUsbMakerConfig buildConfig() const;
    [[nodiscard]] QString writeBackendConfig(QString *error) const;
    void makeUsb(const QString &options);
    void progress();
    void setGeneralConnections();
    void setSourceFile(const QString &fileName);
    void setup();
    void showErrorAndReset(const QString &message = QString());
};
