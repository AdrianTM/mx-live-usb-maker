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
    static bool isantiX_mx_family(const QString &selected);
    void cleanup();
    void cmdDone();
    void setConnections();
    void setDefaultMode(const QString &isoName);
    void updateBar();
    void updateOutput();

    void checkCloneLive_clicked(bool checked);
    void checkCloneMode_clicked(bool checked);
    void checkDataFirst_clicked(bool checked);
    void checkUpdate_clicked(bool checked);
    void pushAbout_clicked();
    void pushBack_clicked();
    void pushHelp_clicked();
    void pushLumLogFile_clicked();
    void pushNext_clicked();
    void pushOptions_clicked();
    void pushRefresh_clicked();
    void pushSelectSource_clicked();
    void radioDd_clicked();
    void radioNormal_clicked();
    void spinBoxSize_valueChanged(int arg1);
    void textLabel_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    Cmd cmd;
    QString backendPath;
    QString device;
    QString elevate;
    QTimer timer;
    bool advancedOptions {};
    int defaultHeight {};
    int height {};
    uint sizeCheck;

    [[nodiscard]] QString buildOptionList();
    [[nodiscard]] QStringList buildUsbList();
    [[nodiscard]] QStringList removeUnsuitable(const QStringList &devices); // remove live or unremovable
    [[nodiscard]] bool checkDestSize();
    [[nodiscard]] bool confirmLargeDeviceWarning(const quint64 diskSize);
    [[nodiscard]] bool validateSizeCompatibility(const quint64 sourceSize, const quint64 diskSize);
    [[nodiscard]] quint64 calculateSourceSize();
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
    [[nodiscard]] static QString shellQuote(const QString &value);
    void makeUsb(const QString &options);
    void progress();
    void setGeneralConnections();
    void setSourceFile(const QString &fileName);
    void setup();
    void showErrorAndReset(const QString &message = QString());
};
