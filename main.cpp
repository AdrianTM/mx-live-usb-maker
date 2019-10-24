/**********************************************************************
 *  main.cpp
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

#include <QApplication>
#include <QDateTime>
#include <QIcon>
#include <QLocale>
#include <QScopedPointer>
#include <QTranslator>
#include <QDebug>

#include "mainwindow.h"
#include <version.h>
#include <unistd.h>


static QScopedPointer<QFile> logFile;
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (a.arguments().contains("--version") || a.arguments().contains("-v") ) {
       qDebug() << "Version:" << VERSION;
       return EXIT_SUCCESS;
    }

    a.setWindowIcon(QIcon::fromTheme("CUSTOMPROGRAMNAME"));

    QTranslator qtTran;
    qtTran.load(QString("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QString("CUSTOMPROGRAMNAME_") + QLocale::system().name(), "/usr/share/CUSTOMPROGRAMNAME/locale");
    a.installTranslator(&appTran);

    if (getuid() == 0) {
        QString log_name= "/var/log/mx-live-usb-maker.log";
        // archive old log
        system("[ -f " + log_name.toUtf8() + " ] && mv " + log_name.toUtf8() + " " + log_name.toUtf8() + ".old");
        // Set the logging files
        logFile.reset(new QFile(log_name));
        // Open the file logging
        logFile.data()->open(QFile::Append | QFile::Text);
        // Set handler
        qInstallMessageHandler(messageHandler);
        MainWindow w(a.arguments());
        w.show();
        return a.exec();
    } else {
        QApplication::beep();
        QMessageBox::critical(nullptr, QApplication::tr("Error"),
                              QApplication::tr("You must run this program as root."));
        return EXIT_FAILURE;
    }
}


// The implementation of the handler
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Write to terminal
    QTextStream term_out(stdout);
    term_out << msg << endl;

    // Open stream file writes
    QTextStream out(logFile.data());

    // Write the date of recording
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    // By type determine to what level belongs message
    switch (type)
    {
    case QtInfoMsg:     out << "INF "; break;
    case QtDebugMsg:    out << "DBG "; break;
    case QtWarningMsg:  out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg:    out << "FTL "; break;
    }
    // Write to the output category of the message and the message itself
    out << context.category << ": "
        << msg << endl;
    out.flush();    // Clear the buffered data
}
