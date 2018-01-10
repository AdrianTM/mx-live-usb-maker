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

#include <QFileDialog>
#include <QScrollBar>
#include <QTextStream>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    height_group = ui->groupBox->height();
    setup();
    ui->combo_Usb->addItems(buildUsbList());
    this->adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::makeUsb(const QString &options)
{
    QString iso_name = ui->buttonSelectIso->text();
    QString device = ui->combo_Usb->currentText().split(" ").at(0);

    QString cmdstr = QString("live-usb-maker " + options + " -C off --percent-prog --from=%1 -t /dev/%2").arg(iso_name).arg(device);
    setConnections();
    qDebug() << cmd->getOutput(cmdstr);
}

// setup versious items first time program runs
void MainWindow::setup()
{
    cmd = new Cmd(this);
    connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::cleanup);
    this->setWindowTitle("Custom_Program_Name");
    ui->groupBox->setFixedHeight(0);
    ui->buttonBack->setHidden(true);;
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonCancel->setEnabled(true);
    ui->buttonNext->setEnabled(true);
    ui->outputBox->setCursorWidth(0);
}

// Build the option list to be passed to live-usb-maker
QString MainWindow::buildOptionList()
{
    QString options("-N");
    if (ui->cb_encrypt->isChecked()) {
        options += "E";
    }
    if (ui->cb_gpt->isChecked()) {
        options += "g";
    }
    if (ui->cb_keep->isChecked()) {
        options += "k";
    }
    if (ui->cb_pretend->isChecked()) {
        options += "p";
    }
    if (ui->cb_save_boot->isChecked()) {
        options += "S";
    }
    if (ui->cb_update->isChecked()) {
        options += "u";
    }
    if (ui->cb_set_pmbr_boot->isChecked()) {
        options += " --gpt-pmbr";
    }
    if (ui->spinBoxEsp->value() != 50) {
        options += " --esp-size=" + ui->spinBoxEsp->cleanText();
    }
    if (ui->spinBoxSize->value() < 100) {
        options += " --size=" + ui->spinBoxSize->cleanText();
    }
    if (!ui->edit_label->text().isEmpty()) {
        options += " --label=" + ui->edit_label->text();
    }
    switch(ui->sliderVerbosity->value()) {
    case 1 : options += " -V";
        break;
    case 2 : options += " -VV";
        break;
    }
    qDebug() << "Options: " << options;
    return options;
}

// cleanup environment when window is closed
void MainWindow::cleanup()
{

}


// Get version of the program
QString MainWindow::getVersion(QString name)
{
    Cmd cmd;
    return cmd.getOutput("dpkg -l "+ name + "| awk 'NR==6 {print $3}'");
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
    foreach (const QString line, devices) {
        name = line.split(" ").at(0);
        if (system(cli_utils.toUtf8() + "is_usb_or_removable " + name.toUtf8()) == 0) {
            list << line;
        }
    }
    return list;
}

void MainWindow::cmdStart()
{
    //setCursor(QCursor(Qt::BusyCursor));
    ui->lineEdit->setFocus();
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
    connect(cmd, &Cmd::finished, this, &MainWindow::cmdDone);
}

void MainWindow::updateOutput(QString out)
{
    // remove escape sequences that are not handled by code
    out.remove("[0m").remove("]0;").remove("").remove("").remove("[1000D").remove("[74C|").remove("[?25l").remove("[?25h");
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
        if (!QFile(ui->buttonSelectIso->text()).exists()) {
            QMessageBox::critical(this, tr("Error"), tr("Please select an ISO file"));
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
                       tr("Version: ") + getVersion("CUSTOMPROGRAMNAME") + "</p><p align=\"center\"><h3>" +
                       tr("Program for creating creating a live-usb from an iso-file, another live-usb, a live-cd/dvd, or a running live system.") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p><p align=\"center\">" +
                       tr("Copyright (c) MX Linux") + "<br /><br /></p>");
    msgBox.addButton(tr("License"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
    if (msgBox.exec() == QMessageBox::AcceptRole) {
        system("mx-viewer file:///usr/share/doc/CUSTOMPROGRAMNAME/license.html 'Custom_Program_Name " + tr("License").toUtf8() + "'");
    }
    this->show();
}

// Help button clicked
void MainWindow::on_buttonHelp_clicked()
{
    QString url = "google.com";
    QString exec = "xdg-open";
    if (system("command -v mx-viewer") == 0) { // use mx-viewer if available
        exec = "mx-viewer";
        url += " Custom_Program_Name";
    }
    QString cmd = QString(exec + " " + url + "&");
    system(cmd.toUtf8());
}


void MainWindow::on_buttonSelectIso_clicked()
{
    QFileDialog dialog;
    QString selected = dialog.getOpenFileName(this, tr("Select a ISO file to write to the USB drive"), QString(QDir::homePath()), QString("*.iso"));
    if (selected != "") {
        ui->buttonSelectIso->setText(selected);
        ui->buttonSelectIso->setIcon(QIcon::fromTheme("media-cdrom"));
    }
}

void MainWindow::on_buttonRefresh_clicked()
{
    ui->combo_Usb->clear();
    ui->combo_Usb->addItems(buildUsbList());
}


void MainWindow::on_buttonOptions_clicked()
{
    if (ui->buttonOptions->text() == tr("Show options")) {
        ui->groupBox->setFixedHeight(height_group);
        ui->buttonOptions->setText(tr("Hide options"));
        ui->buttonOptions->setIcon(QIcon::fromTheme("up"));
    } else {
       ui->groupBox->setFixedHeight(0);
       ui->buttonOptions->setText(tr("Show options"));
       ui->buttonOptions->setIcon(QIcon::fromTheme("down"));
    }
    this->repaint();
}

void MainWindow::on_buttonEnter_clicked()
{
    on_lineEdit_returnPressed();
}

void MainWindow::on_edit_label_textChanged(QString arg1)
{
    ui->edit_label->setText(arg1.remove(" "));
    ui->edit_label->setCursorPosition(arg1.length());
}

void MainWindow::on_lineEdit_returnPressed()
{
    cmd->writeToProc(ui->lineEdit->text());
    if (!(ui->lineEdit->text().size() == 1 && (ui->lineEdit->text() == "q" || ui->lineEdit->text() == "h"))) {
        cmd->writeToProc("\n"); // don't send new line for q and h interactive options
    }
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}
