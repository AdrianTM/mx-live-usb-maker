/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
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

inline const QString startingHome {qEnvironmentVariable("HOME")};

// Size conversion constants
inline const quint64 BYTES_PER_GB = 1024 * 1024 * 1024;
inline const quint64 SECTORS_PER_MB = 2048; // 1024 KB/MB / 512 bytes/sector * 1024

// Live system paths
namespace LivePaths
{
inline const QString CONFIG_DIR = QStringLiteral("/live/config");
inline const QString INITRD_OUT = QStringLiteral("/live/config/initrd.out");
inline const QString DID_TORAM = QStringLiteral("/live/config/did-toram");
inline const QString ENCRYPTED = QStringLiteral("/live/config/encrypted");
inline const QString STATIC_ROOT = QStringLiteral("/live/config/static-root");
inline const QString BOOT_DEV = QStringLiteral("/live/boot-dev");
inline const QString TO_RAM = QStringLiteral("/live/to-ram");
inline const QString LINUX = QStringLiteral("/live/linux");
} // namespace LivePaths

// System paths
namespace SystemPaths
{
inline const QString PROC_MOUNTS = QStringLiteral("/proc/mounts");
inline const QString PROC_SELF_LOGINUID = QStringLiteral("/proc/self/loginuid");
inline const QString SYS_BLOCK = QStringLiteral("/sys/block");
inline const QString DEV_DISK_BY_UUID = QStringLiteral("/dev/disk/by-uuid");
inline const QString DEV_MAPPER = QStringLiteral("/dev/mapper");
inline const QString ROOT_HOME = QStringLiteral("/root");
inline const QString TMP_DIR = QStringLiteral("/tmp");
} // namespace SystemPaths

// Application paths
namespace AppPaths
{
inline const QString WORK_DIR = QStringLiteral("/run/live-usb-maker");
inline const QString CONFIG_FILE = QStringLiteral("/etc/mx-live-usb-maker/mx-live-usb-maker.conf");
inline const QString LOG_FILE = QStringLiteral("/var/log/live-usb-maker.log");
} // namespace AppPaths

// Syslinux paths
namespace SyslinuxPaths
{
inline const QString SHARE = QStringLiteral("/usr/share/syslinux");
inline const QString LIB = QStringLiteral("/usr/lib/syslinux");
inline const QString MBR = QStringLiteral("/usr/lib/syslinux/mbr");
inline const QString MODULES_BIOS = QStringLiteral("/usr/lib/syslinux/modules/bios");
} // namespace SyslinuxPaths

// Dictionary paths
namespace DictPaths
{
inline const QString AMERICAN_ENGLISH = QStringLiteral("/usr/share/dict/american-english");
inline const QString WORDS = QStringLiteral("/usr/share/dict/words");
} // namespace DictPaths

// Documentation paths
namespace DocPaths
{
inline const QString SHARE_DOC = QStringLiteral("/usr/share/doc");
} // namespace DocPaths
