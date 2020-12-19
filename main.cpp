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
#include <QCommandLineParser>
#include <QDateTime>
#include <QIcon>
#include <QLibraryInfo>
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
    QApplication app(argc, argv);
    app.setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("Program for creating a live-usb from an iso-file, another live-usb, a live-cd/dvd, or a running live system."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QCoreApplication::tr("filename"), QCoreApplication::tr("Name of .iso file to open"), QCoreApplication::tr("[filename]"));
    parser.process(app);

    app.setWindowIcon(QIcon::fromTheme(app.applicationName()));

    QTranslator qtTran;
    if (qtTran.load(QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTran);

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtBaseTran);

    QTranslator appTran;
    if (appTran.load(app.applicationName() + "_" + QLocale::system().name(), "/usr/share/" + app.applicationName() + "/locale"))
        app.installTranslator(&appTran);

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

        qDebug().noquote() << app.applicationName() << QApplication::tr("version:") << app.applicationVersion();
        MainWindow w(app.arguments());
        w.show();
        return app.exec();
    } else {
        system("su-to-root -X -c " + app.applicationFilePath().toUtf8() + "&");
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
    out << context.category << ": " << msg << endl;
}
