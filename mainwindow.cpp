/**********************************************************************
 *  mainwindow.cpp
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

#include "about.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"

#include <QFileDialog>
#include <QScrollBar>
#include <QTextStream>

#include <QDebug>

MainWindow::MainWindow(const QStringList& args) :
    ui(new Ui::MainWindow)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << VERSION;
    ui->setupUi(this);

    // setup options
    LUM.clear();
    QFileInfo settingsfile("/etc/CUSTOMPROGRAMNAME/CUSTOMPROGRAMNAME.conf");
    QSettings settings("/etc/CUSTOMPROGRAMNAME/CUSTOMPROGRAMNAME.conf", QSettings::NativeFormat);
    LUM = settings.value("LUM", "live-usb-maker").toString();
    size_check = settings.value("SizeCheck", 128).toInt();
    qDebug() << "LUM is:" << LUM;

    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setup();
    ui->combo_Usb->addItems(buildUsbList());
    if (args.size() > 1 && args.at(1) != "%f") {
        ui->buttonSelectSource->setText(args.at(1));
        ui->buttonSelectSource->setProperty("filename", args.at(1));
        ui->buttonSelectSource->setToolTip(args.at(1));
        ui->buttonSelectSource->setIcon(QIcon::fromTheme("media-cdrom"));
        ui->buttonSelectSource->setStyleSheet("text-align: left;");
        setDefaultMode(args.at(1));
    }

    this->adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::checkDestSize()
{
    int disk_size = cmd.getCmdOut("echo $(( $(sudo blockdev --getsize64 /dev/"+ device + ") / 1024**3 ))").toInt();

    if (disk_size > size_check) { // question when writing on large drives (potentially unintended)
        return (QMessageBox::Yes == QMessageBox::question(
                    this, tr("Confirmation"), tr("Target device %1 is larger than %2 GB. Do you wish to proceed?").arg(device).arg(size_check),
                    QMessageBox::No|QMessageBox::Yes, QMessageBox::No));
    } else {
        return true;
    }
}

bool MainWindow::isRunningLive()
{
    QString test = cmd.getCmdOut("df -T / |tail -n1 |awk '{print $2}'");
    return ( test == "aufs" || test == "overlay" );
}

// determine if it's running in "toram" mode
bool MainWindow::isToRam()
{
    return QFileInfo::exists("/live/config/did-toram");
}

void MainWindow::makeUsb(const QString &options)
{
    device = ui->combo_Usb->currentText().split(" ").at(0);

    QString source;
    if (!ui->cb_clone_live->isChecked() && !ui->cb_clone_mode->isChecked()) {
        source = "\"" + ui->buttonSelectSource->property("filename").toString() + "\"";
        QString source_size = cmd.getCmdOut("du -m \"" + ui->buttonSelectSource->property("filename").toString() + "\" 2>/dev/null | cut -f1", true);
        iso_sectors = source_size.toInt() * 1024 / 512 * 1024;
    } else if (ui->cb_clone_mode->isChecked()) {
        QString source_size = cmd.getCmdOut("du -m --summarize \"" + ui->buttonSelectSource->property("filename").toString() + "\" 2>/dev/null | cut -f1", true);
        iso_sectors = source_size.toInt() * 1024 / 512 * 1024;
        source = "clone=\"" + ui->buttonSelectSource->property("filename").toString() + "\"";

        // check if source and destination are on the same drive
        QString root_partition = cmd.getCmdOut("df --output=source \"" + ui->buttonSelectSource->property("filename").toString() + "\"| awk 'END{print $1}'");
        if ("/dev/" + device == cmd.getCmdOut(cli_utils + "get_drive " + root_partition)) {
            QMessageBox::critical(this, tr("Failure"), tr("Source and destination are on the same device, please select again."));
            ui->stackedWidget->setCurrentWidget(ui->selectionPage);
            ui->buttonNext->setEnabled(true);
            setCursor(QCursor(Qt::ArrowCursor));
            return;
        }
    } else if (ui->cb_clone_live->isChecked()) {
        source = "clone";
        QString source_size = cmd.getCmdOut("du -m --summarize /live/boot-dev 2>/dev/null | cut -f1", true);
        iso_sectors = source_size.toInt() * 1024 / 512 * 1024;
    }

    if(!checkDestSize()) {
        ui->stackedWidget->setCurrentWidget(ui->selectionPage);
        ui->buttonNext->setEnabled(true);
        setCursor(QCursor(Qt::ArrowCursor));
        return;
    }

    // check amount of io on device before copy, this is in sectors
    start_io = cmd.getCmdOut("cat /sys/block/" + device + "/stat |awk '{print $7}'", true).toInt();
    ui->progressBar->setMinimum(start_io);
    qDebug() << "start io is " << start_io;
    ui->progressBar->setMaximum(iso_sectors + start_io);
    qDebug() << "max progress bar is " << ui->progressBar->maximum();

    QString cmdstr = QString(LUM + " gui " + options + "-C off --from=%1 -t /dev/%2").arg(source).arg(device);
    if (ui->rb_dd->isChecked()) {
        cmdstr = LUM + " gui partition-clear -NC off --target " + device;
        connect(&cmd, &QProcess::readyRead, this, &MainWindow::updateOutput);
        qDebug() << cmd.getCmdOut(cmdstr);
        cmdstr = "dd bs=1M if=" + source + " of=/dev/" + device;
        ui->outputBox->appendPlainText(tr("Writing %1 using 'dd' command to /dev/%2,\n\n"
                                          "Please wait until the the process is completed").arg(source).arg(device));
    }
    setConnections();
    qDebug() << cmd.getCmdOut(cmdstr);
}

// setup versious items first time program runs
void MainWindow::setup()
{
    connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::cleanup);
    this->setWindowTitle("Custom_Program_Name");

    QFont font("monospace");
    font.setStyleHint(QFont::Monospace);
    ui->outputBox->setFont(font);

    ui->groupAdvOptions->hide();
    advancedOptions = false;
    ui->buttonBack->hide();
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonCancel->setEnabled(true);
    ui->buttonNext->setEnabled(true);
    ui->outputBox->setCursorWidth(0);
    height = this->heightMM();

    QRegularExpression rx("\\w*");
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->edit_label->setValidator(validator);

    //set save boot directory option to disable unless update mode is checked
    ui->cb_save_boot->setEnabled(false);

    //check if running live or with "toram"
    ui->cb_clone_live->setEnabled(isRunningLive() && !isToRam());

    //check if datafirst option is available
    if (!cmd.run(LUM + " --help | grep -q data-first", true)) {
        ui->comboBoxDataFormat->hide();
        ui->cb_data_first->hide();
        ui->spinBoxDataSize->hide();
        ui->label->hide();
    }
}

// Build the option list to be passed to live-usb-maker
QString MainWindow::buildOptionList()
{
    QString options("-N ");
    if (ui->cb_encrypt->isChecked()) {
        options += "-E ";
    }
    if (ui->cb_gpt->isChecked()) {
        options += "-g ";
    }
    if (ui->cb_keep->isChecked()) {
        options += "-k ";
    }
    if (ui->cb_pretend->isChecked()) {
        options += "-p ";
        if (ui->sliderVerbosity->value() == 0) { // add Verbosity of not selected, workaround for LUM bug
            ui->sliderVerbosity->setSliderPosition(1);
        }
    }
    if (ui->cb_save_boot->isChecked()) {
        options += "-S ";
    }
    if (ui->cb_update->isChecked()) {
        options += "-u ";
    }
    if (ui->cb_set_pmbr_boot->isChecked()) {
        options += "--gpt-pmbr ";
    }
    if (ui->spinBoxEsp->value() != 50) {
        options += "--esp-size=" + ui->spinBoxEsp->cleanText() + " ";
    }
    if (ui->spinBoxSize->value() < 100) {
        options += "--size=" + ui->spinBoxSize->cleanText() + " ";
    }
    if (!ui->edit_label->text().isEmpty()) {
        options += " --label=" + ui->edit_label->text() + " ";
    }
    if (ui->cb_force_usb->isChecked()) {
        options += "--force=usb ";
    }
    if (ui->cb_force_automount->isChecked()) {
        options += "--force=automount ";
    }
    if (ui->cb_force_makefs->isChecked()) {
        options += "--force=makefs ";
    }
    if (ui->cb_force_nofuse->isChecked()) {
        options += "--force=nofuse ";
    }
    if (ui->rb_dd->isChecked()) {

    }

    if (ui->cb_data_first->isChecked()) {
        options += "--data-first=" + ui->spinBoxDataSize->cleanText() + "," + ui->comboBoxDataFormat->currentText() + " ";
    }
    switch(ui->sliderVerbosity->value()) {
    case 1 : options += "-V ";
        break;
    case 2 : options += "-VV ";
        break;
    }
    qDebug() << "Options: " << options;
    return options;
}

// cleanup environment when window is closed
void MainWindow::cleanup()
{
    cmd.halt();
}

// build the USB list
QStringList MainWindow::buildUsbList()
{
    QString drives = cmd.getCmdOut("lsblk --nodeps -nlo NAME,SIZE,MODEL,VENDOR -I 3,8,22,179,259");
    return removeUnsuitable(drives.split("\n"));
}

// remove unsuitable drives from the list (live and unremovable)
QStringList MainWindow::removeUnsuitable(const QStringList &devices)
{
    QStringList list;
    QString name;
    for (const QString &line : devices) {
        name = line.split(" ").at(0);
        if (ui->cb_force_usb->isChecked()){
            if (cmd.getCmdOut(cli_utils + "get_drive $(get_live_dev) ") != name) {
                list << line;
            }
        } else {
            if (system(cli_utils.toUtf8() + "is_usb_or_removable " + name.toUtf8()) == 0) {
                if (cmd.getCmdOut(cli_utils + "get_drive $(get_live_dev) ") != name) {
                    list << line;
                }
            }
        }
    }
    return list;
}

void MainWindow::cmdStart()
{
    //setCursor(QCursor(Qt::BusyCursor));
    //ui->lineEdit->setFocus();
}


void MainWindow::cmdDone()
{
    timer.stop();
    ui->progressBar->setValue(ui->progressBar->maximum());
    setCursor(QCursor(Qt::ArrowCursor));
    ui->buttonBack->show();
    if (cmd.exitCode() == 0 && cmd.exitStatus() == QProcess::NormalExit) {
        QMessageBox::information(this, tr("Success"), tr("LiveUSB creation successful!"));
    } else {
        QMessageBox::critical(this, tr("Failure"), tr("Error encountered in the LiveUSB creation process"));
    }
    cmd.disconnect();
}

// set proc and timer connections
void MainWindow::setConnections()
{
    timer.start(1000);
    connect(&cmd, &QProcess::readyRead, this, &MainWindow::updateOutput);
    connect(&cmd, &QProcess::started, this, &MainWindow::cmdStart);
    connect(&timer, &QTimer::timeout, this, &MainWindow::updateBar);
    connect(&cmd, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &MainWindow::cmdDone);
}

// set proper default mode based on iso contents
void MainWindow::setDefaultMode(const QString &iso_name)
{
    if (!isantiX_mx_family(iso_name)){
        ui->rb_dd->click();
        ui->rb_normal->setChecked(false);
    } else {
        ui->rb_dd->setChecked(false);
        ui->rb_normal->click();
    }
}

void MainWindow::updateBar()
{
    int current_io = cmd2.getCmdOut("cat /sys/block/" + device + "/stat | awk '{print $7}'", true).toInt();
    ui->progressBar->setValue(current_io);
}

void MainWindow::updateOutput()
{
    // remove escape sequences that are not handled by code
    QString out = cmd.readAll();
    out.remove("[0m").remove("]0;").remove("").remove("").remove("[1000D").remove("[74C|").remove("[?25l").remove("[?25h").remove("[0;36m").remove("[1;37m");
//    if (out.contains("[10D[K")) { // escape sequence used to display the progress percentage
//        out.remove("[10D[K");
//        ui->outputBox->moveCursor(QTextCursor::StartOfLine);
//        QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_K, Qt::ControlModifier);
//        QCoreApplication::postEvent(ui->outputBox, event);
//        QString out_prog = out;
//        ui->progressBar->setValue(out_prog.remove(" ").remove("%").toInt());
//    }

    ui->outputBox->insertPlainText(out);

    QScrollBar *sb = ui->outputBox->verticalScrollBar();
    sb->setValue(sb->maximum());
    qApp->processEvents();
}

// Next button clicked
void MainWindow::on_buttonNext_clicked()
{
    // on first page
    if (ui->stackedWidget->currentIndex() == 0) {
        if (ui->combo_Usb->currentText() == "") {
            QMessageBox::critical(this, tr("Error"), tr("Please select a USB device to write to"));
            return;
        }
        if (!(QFileInfo::exists(ui->buttonSelectSource->property("filename").toString()) || ui->buttonSelectSource->property("filename").toString() == tr("clone"))) { // pop the selection box if no valid selection (or clone)
            ui->buttonSelectSource->clicked();
            return;
        }
        if (cmd.state() != QProcess::NotRunning) {
            ui->stackedWidget->setCurrentWidget(ui->outputPage);
            return;
        }
        ui->buttonNext->setEnabled(false);
        ui->stackedWidget->setCurrentWidget(ui->outputPage);

        makeUsb(buildOptionList());

    // on output page
    } else if (ui->stackedWidget->currentWidget() == ui->outputPage) {

    } else {
        return qApp->quit();
    }
}

void MainWindow::on_buttonBack_clicked()
{
    this->setWindowTitle("Custom_Program_Name");
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonNext->setEnabled(true);
    ui->buttonBack->hide();
    ui->outputBox->clear();
    ui->progressBar->setValue(0);
}


// About button clicked
void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(tr("About %1").arg(this->windowTitle()), "<p align=\"center\"><b><h2>" + this->windowTitle() +"</h2></b></p><p align=\"center\">" +
                       tr("Version: ") + VERSION + "</p><p align=\"center\"><h3>" +
                       tr("Program for creating a live-usb from an iso-file, another live-usb, a live-cd/dvd, or a running live system.") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p><p align=\"center\">" +
                       tr("Copyright (c) MX Linux") + "<br /><br /></p>",
                       "/usr/share/doc/CUSTOMPROGRAMNAME/license.html", tr("%1 License").arg(this->windowTitle()), true);
    this->show();
}

// Help button clicked
void MainWindow::on_buttonHelp_clicked()
{
    QString url = "/usr/share/doc/CUSTOMPROGRAMNAME/help/CUSTOMPROGRAMNAME.html";
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()), true);
}


void MainWindow::on_buttonSelectSource_clicked()
{
    QString selected;
    QString user = cmd.getCmdOut("logname", true);
    QString home = "/home/" + user;

    if (!ui->cb_clone_live->isChecked() && !ui->cb_clone_mode->isChecked()) {
        selected = QFileDialog::getOpenFileName(this, tr("Select an ISO file to write to the USB drive"), home, tr("ISO Files (*.iso);;All Files (*.*)"));
        if (!selected.isEmpty()) {
            ui->buttonSelectSource->setText(selected);
            ui->buttonSelectSource->setProperty("filename", selected);
            ui->buttonSelectSource->setToolTip(selected);
            ui->buttonSelectSource->setIcon(QIcon::fromTheme("media-cdrom"));
            ui->buttonSelectSource->setStyleSheet("text-align: left;");
            setDefaultMode(selected); // set proper default mode based on iso contents
        }
    } else if (ui->cb_clone_mode->isChecked()) {
        selected = QFileDialog::getExistingDirectory(this, tr("Select Source Directory"), QString(QDir::rootPath()), QFileDialog::ShowDirsOnly);
        if (QFileInfo::exists(selected + "/antiX/linuxfs")|| QFileInfo::exists(selected + "/linuxfs")) {
            ui->buttonSelectSource->setText(selected);
            ui->buttonSelectSource->setProperty("filename", selected);
        } else {
            selected = (selected == "/") ? "" : selected;
            QMessageBox::critical(this, tr("Failure"), tr("Could not find %1/antiX/linuxfs file").arg(selected)); // TODO -- the file might be in %/linuxfs too for frugal
        }
    }
}

void MainWindow::on_buttonRefresh_clicked()
{
    ui->combo_Usb->clear();
    ui->combo_Usb->addItems(buildUsbList());
}


void MainWindow::on_buttonOptions_clicked()
{
    if (advancedOptions) {
        ui->buttonOptions->setText(tr("Show advanced options"));
        ui->groupAdvOptions->hide();
        advancedOptions = false;
        ui->buttonOptions->setIcon(QIcon::fromTheme("go-down"));
        this->setMaximumHeight(height);
    } else {
        ui->buttonOptions->setText(tr("Hide advanced options"));
        ui->groupAdvOptions->show();
        advancedOptions = true;
        ui->buttonOptions->setIcon(QIcon::fromTheme("go-up"));
    }
}

//void MainWindow::on_buttonEnter_clicked()
//{
//    on_lineEdit_returnPressed();
//}

void MainWindow::on_edit_label_textChanged(QString arg1)
{
    ui->edit_label->setText(arg1.remove(" "));
    ui->edit_label->setCursorPosition(arg1.length());
}

//void MainWindow::on_lineEdit_returnPressed()
//{
//    cmd->writeToProc(ui->lineEdit->text());
//    if (!(ui->lineEdit->text().size() == 1 && (ui->lineEdit->text() == "q" || ui->lineEdit->text() == "h"))) {
//        cmd->writeToProc("\n"); // don't send new line for q and h interactive options
//    }
//    ui->lineEdit->clear();
//    ui->lineEdit->setFocus();
//}

void MainWindow::on_cb_update_clicked(bool checked)
{
    if (checked) {
        ui->cb_save_boot->setEnabled(true);
    } else {
        ui->cb_save_boot->setChecked(false);
        ui->cb_save_boot->setEnabled(false);
    }
}

void MainWindow::on_cb_clone_mode_clicked(bool checked)
{
     if (checked) {
        ui->buttonSelectSource->setStyleSheet("text-align: center;");
        ui->cb_clone_live->setChecked(false);
        on_cb_clone_live_clicked(false);
        ui->label_3->setText("<b>" + tr("Select Source") + "</b>");
        ui->buttonSelectSource->setText(tr("Select Source Directory"));
        ui->buttonSelectSource->setIcon(QIcon::fromTheme("folder"));
    } else {
        ui->label_3->setText("<b>" + tr("Select ISO file") + "</b>");
        ui->buttonSelectSource->setText(tr("Select ISO"));
        ui->buttonSelectSource->setIcon(QIcon::fromTheme("user-home"));
        ui->cb_clone_live->setEnabled(isRunningLive());
    }
     ui->buttonSelectSource->setProperty("filename", "");
}

void MainWindow::on_cb_clone_live_clicked(bool checked)
{
    if (checked){
        ui->buttonSelectSource->setStyleSheet("text-align: center;");
        ui->cb_clone_mode->setChecked(false);
        ui->label_3->setText("<b>" + tr("Select Source") + "</b>");
        ui->buttonSelectSource->setEnabled(false);
        ui->buttonSelectSource->setText(tr("clone"));
        ui->buttonSelectSource->setProperty("filename", "clone");
        ui->buttonSelectSource->setIcon(QIcon::fromTheme("tools-media-optical-copy"));
        ui->buttonSelectSource->blockSignals(true);
    } else {
        ui->label_3->setText("<b>" + tr("Select ISO file") + "</b>");
        ui->buttonSelectSource->setEnabled(true);
        ui->buttonSelectSource->setText(tr("Select ISO"));
        ui->buttonSelectSource->setProperty("filename", "");
        ui->buttonSelectSource->setIcon(QIcon::fromTheme("user-home"));
        ui->buttonSelectSource->blockSignals(false);
    }
}

void MainWindow::on_rb_dd_clicked()
{
    ui->rb_normal->setChecked(false);
    ui->cb_clone_live->setChecked(false);
    ui->cb_clone_mode->setChecked(false);
    ui->cb_encrypt->setChecked(false);
    ui->cb_clone_live->setEnabled(false);
    ui->cb_clone_mode->setEnabled(false);
    ui->cb_encrypt->setEnabled(false);
    ui->cb_pretend->setEnabled(false);
    if (ui->groupAdvOptions->isVisible()) {
        on_buttonOptions_clicked();
    }
    ui->buttonOptions->setEnabled(false);
    ui->label_percent->setEnabled(false);
    ui->label_part_label->setEnabled(false);
    ui->spinBoxSize->setEnabled(false);
    ui->edit_label->setEnabled(false);
}

void MainWindow::on_rb_normal_clicked()
{
    ui->cb_clone_live->setEnabled(isRunningLive());
    ui->cb_clone_mode->setEnabled(true);
    ui->cb_encrypt->setEnabled(true);
    ui->cb_pretend->setEnabled(true);
    ui->buttonOptions->setEnabled(true);
    ui->label_percent->setEnabled(true);
    ui->label_part_label->setEnabled(true);
    ui->spinBoxSize->setEnabled(true);
    ui->edit_label->setEnabled(true);
}

bool MainWindow::isantiX_mx_family(QString selected)
{
    QString cmdstr;

    //make temp folder
    cmdstr = "mkdir -p /tmp/testisomount";

    if ( system(cmdstr.toUtf8()) != 0 ){
        return false;
    }

    //mount the iso file
    cmdstr = "mount -o loop " + selected + " /tmp/testisomount";
    if (system(cmdstr.toUtf8()) != 0) {
        //cleanup mount point
        cmdstr = "rmdir /tmp/testisomount";
        system(cmdstr.toUtf8());
        return false;
    }

    //check for antiX folder - this is a BS check but works for now since no antiX family iso doens't have an antiX folder
    cmdstr = "ls /tmp/testisomount |grep antiX";
    if (system(cmdstr.toUtf8()) == 0){
        //mean found
        //cleanup mount
        cmdstr = "umount /tmp/testisomount; rmdir /tmp/testisomount";
        system(cmdstr.toUtf8());
        return true;
    } else {
        cmdstr = "umount /tmp/testisomount; rmdir /tmp/testisomount";
        system(cmdstr.toUtf8());
        return false;
    }
}
