/**********************************************************************
 *  liveusbmaker_config.h
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

#include <QJsonObject>
#include <QString>

struct LiveUsbMakerConfig
{
    enum class Mode { Normal, Dd };
    enum class SourceMode { Iso, Clone, CloneDir };

    Mode mode {Mode::Normal};
    SourceMode sourceMode {SourceMode::Iso};
    QString sourcePath;
    QString targetDevice;

    bool pretend {false};
    bool update {false};
    bool keepSyslinux {false};
    bool saveBoot {false};
    bool encrypt {false};
    bool gpt {false};
    bool pmbr {false};
    bool forceUsb {false};
    bool forceAutomount {false};
    bool forceMakefs {false};
    bool forceNofuse {false};

    int espSizeMiB {50};
    int mainPercent {100};
    QString label;

    bool dataFirst {false};
    int dataPercent {0};
    QString dataFs;

    int verbosity {0};
    bool clonePersist {true};

    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] static LiveUsbMakerConfig fromJson(const QJsonObject &object, QString *error);
    [[nodiscard]] static QString backendExecutablePath();
    [[nodiscard]] static QString writeToTempFile(const LiveUsbMakerConfig &config, QString *error);
    static bool readFromFile(const QString &path, LiveUsbMakerConfig *config, QString *error);
};
