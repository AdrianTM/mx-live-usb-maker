/**********************************************************************
 *  liveusbmaker_backend_main.cpp
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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

#include "liveusbmaker_backend.h"
#include "liveusbmaker_config.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("MX Live USB Maker backend"));
    parser.addHelpOption();
    QCommandLineOption configOption(QStringLiteral("config"),
                                    QStringLiteral("Path to JSON config file."),
                                    QStringLiteral("path"));
    parser.addOption(configOption);
    parser.process(app);

    const QString configPath = parser.value(configOption);
    if (configPath.isEmpty()) {
        QTextStream(stderr) << "Missing --config argument.\n";
        return 2;
    }

    LiveUsbMakerConfig config;
    QString error;
    if (!LiveUsbMakerConfig::readFromFile(configPath, &config, &error)) {
        QTextStream(stderr) << "Failed to read config: " << error << "\n";
        return 2;
    }

    LiveUsbMakerBackend backend(config);
    if (!backend.run(&error)) {
        if (!error.isEmpty()) {
            QTextStream(stderr) << "Failure: " << error << "\n";
        }
        return 1;
    }
    return 0;
}
