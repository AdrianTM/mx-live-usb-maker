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
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setup();
    ui->combo_Usb->addItems(buildUsbList());
    if (args.size() > 1) {
        ui->buttonSelectSource->setText(args.at(1));
        ui->buttonSelectSource->setToolTip(args.at(1));
        ui->buttonSelectSource->setIcon(QIcon::fromTheme("media-cdrom"));
        ui->buttonSelectSource->setStyleSheet("text-align: left;");
    }
    this->adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::isRunningLive()
{
    QString test = cmd->getOutput("df -T / |tail -n1 |awk '{print $2}'");
    return ( test == "aufs" || test == "overlay" );
}

// determine if it's running in "toram" mode
bool MainWindow::isToRam()
{
    return QFile("/live/config/did-toram").exists();
}

void MainWindow::makeUsb(const QString &options)
{
    device = ui->combo_Usb->currentText().split(" ").at(0);

    QString source;
    if (!ui->cb_clone_live->isChecked() && !ui->cb_clone_mode->isChecked()) {
        source = "\"" + ui->buttonSelectSource->text() + "\"";
        QString source_size = cmd->getOutput("du -m \"" + ui->buttonSelectSource->text() + "\" 2>/dev/null | cut -f1", QStringList() << "quiet");
        iso_sectors = source_size.toInt() * 1024 / 512 * 1024;
    } else if (ui->cb_clone_mode->isChecked()) {
        QString source_size = cmd->getOutput("du -m --summarize \"" + ui->buttonSelectSource->text() + "\" 2>/dev/null | cut -f1", QStringList() << "quiet");
        iso_sectors = source_size.toInt() * 1024 / 512 * 1024;
        source = "clone=\"" + ui->buttonSelectSource->text() + "\"";

        // check if source and destination are on the same drive
        QString root_partition = cmd->getOutput("df --output=source \"" + ui->buttonSelectSource->text() + "\"| awk 'END{print $1}'");
        if ("/dev/" + device == cmd->getOutput(cli_utils + "get_drive " + root_partition)) {
            QMessageBox::critical(this, tr("Failure"), tr("Source and destination are on the same device, please select again."));
            ui->stackedWidget->setCurrentWidget(ui->selectionPage);
            ui->buttonNext->setEnabled(true);
            setCursor(QCursor(Qt::ArrowCursor));
            return;
        }
    } else if (ui->cb_clone_live->isChecked()) {
        source = "clone";
        QString source_size = cmd->getOutput("du -m --summarize /live/boot-dev 2>/dev/null | cut -f1", QStringList() << "quiet");
        iso_sectors = source_size.toInt() * 1024 / 512 * 1024;
    }

    // check amount of io on device before copy, this is in sectors
    start_io = cmd->getOutput("cat /sys/block/" + device + "/stat |awk '{print $7}'", QStringList() << "quiet").toInt();
    ui->progressBar->setMinimum(start_io);
    qDebug() << "start io is " << start_io;
    ui->progressBar->setMaximum(iso_sectors+start_io);
    qDebug() << "max progress bar is " << ui->progressBar->maximum();

    QString cmdstr = QString("live-usb-maker gui " + options + "-C off --from=%1 -t /dev/%2").arg(source).arg(device);
    if (ui->rb_dd->isChecked()) {
        cmdstr = "live-usb-maker gui partition-clear -NC off --target " + device;
        connect(cmd, &Cmd::outputAvailable, this, &MainWindow::updateOutput);
        qDebug() << cmd->getOutput(cmdstr, QStringList() << "slowtick");
        cmdstr = "dd bs=1M if=" + source + " of=/dev/" + device;
        ui->outputBox->appendPlainText(tr("Writing %1 using 'dd' command to /dev/%2,\n\n"
                                          "Please wait until the the process is completed").arg(source).arg(device));
    }
    setConnections();
    qDebug() << cmd->getOutput(cmdstr, QStringList() << "slowtick");
}

// setup versious items first time program runs
void MainWindow::setup()
{
    cmd = new Cmd(this);
    cmdprog = new Cmd(this);
    connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::cleanup);
    this->setWindowTitle("Custom_Program_Name");
    ui->groupAdvOptions->hide();
    advancedOptions = false;
    ui->buttonBack->setHidden(true);;
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonCancel->setEnabled(true);
    ui->buttonNext->setEnabled(true);
    ui->outputBox->setCursorWidth(0);
    height = this->heightMM();

    QRegExp rx("\\w*");
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->edit_label->setValidator(validator);

    //set save boot directory option to disable unless update mode is checked
    ui->cb_save_boot->setEnabled(false);

    //check if running live or with "toram"
    ui->cb_clone_live->setEnabled(isRunningLive() && !isToRam());

    //check if datafirst option is available
    QString test = cmd->getOutput("live-usb-maker --help | grep data-first");
    if ( test.isEmpty()) {
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

}

// build the USB list
QStringList MainWindow::buildUsbList()
{
    QString drives = cmd->getOutput("lsblk --nodeps -nlo name,size,model,vendor -I 3,8,22,179,259");
    return removeUnsuitable(drives.split("\n"));
}

// remove unsuitable drives from the list (live and unremovable)
QStringList MainWindow::removeUnsuitable(const QStringList &devices)
{
    QStringList list;
    QString name;
    for (const QString &line : devices) {
        name = line.split(" ").at(0);
        if (system(cli_utils.toUtf8() + "is_usb_or_removable " + name.toUtf8()) == 0) {
            if (cmd->getOutput(cli_utils + "get_drive $(get_live_dev) ") != name) {
                list << line;
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
    ui->progressBar->setValue(ui->progressBar->maximum());
    setCursor(QCursor(Qt::ArrowCursor));
    ui->buttonBack->setEnabled(true);
    if (cmd->getExitCode() == 0) {
        QMessageBox::information(this, tr("Success"), tr("LiveUSB creation successful!"));
    } else {
        QMessageBox::critical(this, tr("Failure"), tr("Error encountered in the LiveUSB creation process"));
    }
    cmd->disconnect();
}

// set proc and timer connections
void MainWindow::setConnections()
{
    connect(cmd, &Cmd::outputAvailable, this, &MainWindow::updateOutput);
    connect(cmd, &Cmd::started, this, &MainWindow::cmdStart);
    connect(cmd, &Cmd::runTime, this, &MainWindow::updateBar);
    connect(cmd, &Cmd::finished, this, &MainWindow::cmdDone);

}

void MainWindow::updateBar()
{
    int current_io = cmdprog->getOutput("cat /sys/block/" + device + "/stat | awk '{print $7}'", QStringList() << "quiet" << "slowtick").toInt();
    ui->progressBar->setValue(current_io);
}

void MainWindow::updateOutput(QString out)
{
    // remove escape sequences that are not handled by code
    out.remove("[0m").remove("]0;").remove("").remove("").remove("[1000D").remove("[74C|").remove("[?25l").remove("[?25h").remove("[0;36m").remove("[1;37m");
    if (out.contains("[10D[K")) { // escape sequence used to display the progress percentage
        out.remove("[10D[K");
        ui->outputBox->moveCursor(QTextCursor::StartOfLine);
        QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_K, Qt::ControlModifier);
        QCoreApplication::postEvent(ui->outputBox, event);
        QString out_prog = out;
        ui->progressBar->setValue(out_prog.remove(" ").remove("%").toInt());
    }

    ui->outputBox->insertPlainText(out);

    QScrollBar *sb = ui->outputBox->verticalScrollBar();
    sb->setValue(sb->maximum());
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
        if (!(QFile(ui->buttonSelectSource->text()).exists() || ui->buttonSelectSource->text() == tr("clone"))) { // pop the selection box if no valid selection (or clone)
            ui->buttonSelectSource->clicked();
            return;
        }
        if (cmd->isRunning()) {
            ui->stackedWidget->setCurrentWidget(ui->outputPage);
            return;
        }
        ui->buttonBack->setHidden(false);
        ui->buttonBack->setEnabled(false);
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
    ui->buttonBack->setDisabled(true);
    ui->outputBox->clear();
    ui->progressBar->setValue(0);
}


// About button clicked
void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    QMessageBox msgBox(QMessageBox::NoIcon,
                       tr("About") + " Custom_Program_Name", "<p align=\"center\"><b><h2>Custom_Program_Name</h2></b></p><p align=\"center\">" +
                       tr("Version: ") + VERSION + "</p><p align=\"center\"><h3>" +
                       tr("Program for creating a live-usb from an iso-file, another live-usb, a live-cd/dvd, or a running live system.") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p><p align=\"center\">" +
                       tr("Copyright (c) MX Linux") + "<br /><br /></p>");
    QPushButton *btnLicense = msgBox.addButton(tr("License"), QMessageBox::HelpRole);
    QPushButton *btnChangelog = msgBox.addButton(tr("Changelog"), QMessageBox::HelpRole);
    QPushButton *btnCancel = msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme("window-close"));

    msgBox.exec();

    Cmd cmd;
    if (msgBox.clickedButton() == btnLicense) {

        QString user = cmd.getOutput("logname");
        if (system("command -v mx-viewer") == 0) { // use mx-viewer if available
            system("su " + user.toUtf8() + " -c \"mx-viewer file:///usr/share/doc/CUSTOMPROGRAMNAME/license.html 'Custom_Program_Name " + tr("License").toUtf8() + "'\"&");
        } else {
            system("su " + user.toUtf8() + " -c \"xdg-open file:///usr/share/doc/CUSTOMPROGRAMNAME/license.html\"&");
        }
    } else if (msgBox.clickedButton() == btnChangelog) {
    QDialog *changelog = new QDialog(this);
    changelog->resize(600, 500);

    QTextEdit *text = new QTextEdit;
    text->setReadOnly(true);
    text->setText(cmd.getOutput("zless /usr/share/doc/" + QFileInfo(QCoreApplication::applicationFilePath()).fileName()  + "/changelog.gz"));

    QPushButton *btnClose = new QPushButton(tr("&Close"));
    btnClose->setIcon(QIcon::fromTheme("window-close"));
    connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(text);
    layout->addWidget(btnClose);
    changelog->setLayout(layout);
    changelog->exec();
}
    this->show();
}

// Help button clicked
void MainWindow::on_buttonHelp_clicked()
{
    QString url = "https://mxlinux.org/wiki/help-files/help-mx-live-usb-maker";
    QString exec = "xdg-open";
    if (system("command -v mx-viewer") == 0) { // use mx-viewer if available
        exec = "mx-viewer";
    }

    Cmd c;
    QString user = c.getOutput("logname");
    QString cmd = QString("su " + user + " -c \"" + exec + " " + url + "\"&");
    system(cmd.toUtf8());
}


void MainWindow::on_buttonSelectSource_clicked()
{
    QString selected;
    Cmd c;
    QString user = c.getOutput("logname");
    QString home = "/home/" + user;

    if (!ui->cb_clone_live->isChecked() && !ui->cb_clone_mode->isChecked()) {
        selected = QFileDialog::getOpenFileName(this, tr("Select an ISO file to write to the USB drive"), home, QString("*.iso"));
        if (!selected.isEmpty()) {
            ui->buttonSelectSource->setText(selected);
            ui->buttonSelectSource->setToolTip(selected);
            ui->buttonSelectSource->setIcon(QIcon::fromTheme("media-cdrom"));
            ui->buttonSelectSource->setStyleSheet("text-align: left;");
        }
    } else if (ui->cb_clone_mode->isChecked()) {
        selected = QFileDialog::getExistingDirectory(this, tr("Select Source Directory"), QString(QDir::rootPath()), QFileDialog::ShowDirsOnly);
        if (QFile(selected + "/antiX/linuxfs").exists()) {
            ui->buttonSelectSource->setText(selected);
        } else {
            selected = (selected == "/") ? "" : selected;
            QMessageBox::critical(this, tr("Failure"), tr("Could not find %1/antiX/linuxfs file").arg(selected));
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
}

void MainWindow::on_cb_clone_live_clicked(bool checked)
{
    if (checked){
        ui->buttonSelectSource->setStyleSheet("text-align: center;");
        ui->cb_clone_mode->setChecked(false);
        ui->label_3->setText("<b>" + tr("Select Source") + "</b>");
        ui->buttonSelectSource->setEnabled(false);
        ui->buttonSelectSource->setText(tr("clone"));
        ui->buttonSelectSource->setIcon(QIcon::fromTheme("tools-media-optical-copy"));
        ui->buttonSelectSource->blockSignals(true);
    } else {
        ui->label_3->setText("<b>" + tr("Select ISO file") + "</b>");
        ui->buttonSelectSource->setEnabled(true);
        ui->buttonSelectSource->setText(tr("Select ISO"));
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
