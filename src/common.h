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

#include <QRegularExpression>
#include <sys/stat.h>

inline const QString startingHome {qEnvironmentVariable("HOME")};

// Size conversion constants
inline const quint64 BYTES_PER_GB = 1024 * 1024 * 1024;
inline const quint64 SECTORS_PER_MB = 2048; // 1024 KB/MB / 512 bytes/sector * 1024

// Disk and partition constants
inline const int SECTOR_SIZE_BYTES = 512;  // Standard sector size
inline const int PARTITION_TABLE_SIZE_BYTES = 17 * 1024;  // Partition table size
inline const int MBR_BOOT_CODE_SIZE_BYTES = 440;  // MBR boot code size (first 440 bytes of 512-byte MBR)
inline const int SNEAKY_OFFSET_BYTES = 32 * 1024;  // Offset for partition alignment

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

// Device utility functions
namespace DeviceUtils
{
// Validate that a device name is well-formed and safe to use
// Returns true if the device name contains only valid characters and patterns
[[nodiscard]] inline bool isValidDeviceName(const QString &device)
{
    if (device.isEmpty()) {
        return false;
    }

    // Check for path traversal attempts
    if (device.contains(QStringLiteral("..")) || device.contains(QStringLiteral("//"))) {
        return false;
    }

    // Extract just the device name part (without /dev/ prefix)
    QString name = device.trimmed();
    if (name.startsWith(QStringLiteral("/dev/"))) {
        name = name.mid(5);
    } else if (name.startsWith(QStringLiteral("dev/"))) {
        name = name.mid(4);
    }

    // Device names should only contain alphanumeric characters, dashes, and underscores
    // Valid patterns: sda, sda1, nvme0n1, nvme0n1p1, mmcblk0, mmcblk0p1, etc.
    static const QRegularExpression validDeviceRe(QStringLiteral("^[a-zA-Z0-9_-]+$"));
    return validDeviceRe.match(name).hasMatch();
}

// Normalize a device path to /dev/xxx format
// Examples: "sda" -> "/dev/sda", "dev/sdb" -> "/dev/sdb", "/dev/sdc" -> "/dev/sdc"
// Returns empty string if device name is invalid
[[nodiscard]] inline QString normalizePath(const QString &device)
{
    QString dev = device.trimmed();

    // Validate device name before normalizing
    if (!isValidDeviceName(dev)) {
        return {};
    }

    if (dev.startsWith(QStringLiteral("/dev/"))) {
        return dev;
    }
    if (dev.startsWith(QStringLiteral("dev/"))) {
        return QStringLiteral("/") + dev;
    }
    if (dev.startsWith(QLatin1Char('/'))) {
        return QStringLiteral("/dev") + dev;
    }
    return QStringLiteral("/dev/") + dev;
}

// Extract base drive name from device path (strips partition numbers)
// Examples: "sda1" -> "sda", "/dev/mmcblk0p1" -> "mmcblk0", "nvme0n1p2" -> "nvme0n1"
[[nodiscard]] inline QString baseDriveName(const QString &device)
{
    QString name = device.trimmed();

    // Strip /dev/ prefix if present
    if (name.startsWith(QStringLiteral("/dev/"))) {
        name = name.mid(5);
    } else if (name.startsWith(QStringLiteral("dev/"))) {
        name = name.mid(4);
    } else if (name.startsWith(QLatin1Char('/'))) {
        name = name.mid(1);
    }

    // Handle mmcblk and nvme devices (partition format: mmcblk0p1, nvme0n1p1)
    // For these devices, partitions are named with 'p' followed by number
    if (name.contains(QStringLiteral("mmcblk")) || name.contains(QStringLiteral("nvme"))) {
        // Remove trailing 'pN' where N is one or more digits
        static const QRegularExpression pDigitRe(QStringLiteral("p\\d+$"));
        name.remove(pDigitRe);
        return name;
    }

    // Handle regular devices (partition format: sda1, hda2)
    // Remove all trailing digits
    while (!name.isEmpty() && name.at(name.size() - 1).isDigit()) {
        name.chop(1);
    }

    return name;
}

// Get full drive path from device (e.g., "/dev/sda1" -> "/dev/sda", "mmcblk0p2" -> "/dev/mmcblk0")
[[nodiscard]] inline QString baseDrivePath(const QString &device)
{
    const QString name = baseDriveName(device);
    if (name.isEmpty()) {
        return {};
    }
    return QStringLiteral("/dev/") + name;
}

// Validate that a path is a block device
[[nodiscard]] inline bool isBlockDevice(const QString &path)
{
    struct stat st {};
    const QByteArray pathBytes = path.toLocal8Bit();
    if (::stat(pathBytes.constData(), &st) != 0) {
        return false;
    }
    return S_ISBLK(st.st_mode);
}
} // namespace DeviceUtils

// Shell utility functions
namespace ShellUtils
{
// Properly escape a string for use in shell commands using single quotes
// This prevents shell injection by escaping all special characters
// Example: "foo'bar" -> 'foo'\''bar'
[[nodiscard]] inline QString quote(const QString &value)
{
    if (value.isEmpty()) {
        return QStringLiteral("''");
    }

    // Use single quotes and escape any existing single quotes
    QString escaped = value;
    escaped.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QStringLiteral("'") + escaped + QStringLiteral("'");
}

// Escape multiple arguments and join them with spaces
[[nodiscard]] inline QString quoteArgs(const QStringList &args)
{
    QStringList quoted;
    quoted.reserve(args.size());
    for (const QString &arg : args) {
        quoted.append(quote(arg));
    }
    return quoted.join(QLatin1Char(' '));
}
} // namespace ShellUtils

// Validation utilities
namespace ValidationUtils
{
// Validate that a string matches UUID format (8-4-4-4-12 hex digits)
// Example: "550e8400-e29b-41d4-a716-446655440000"
[[nodiscard]] inline bool isValidUuid(const QString &uuid)
{
    if (uuid.isEmpty()) {
        return false;
    }

    // UUID format: 8-4-4-4-12 hexadecimal digits separated by hyphens
    static const QRegularExpression uuidRe(
        QStringLiteral("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$"));
    return uuidRe.match(uuid).hasMatch();
}

// Validate that lsblk tabular output has the expected number of columns
// Returns true if each non-empty line has at least minColumns fields
[[nodiscard]] inline bool validateLsblkColumns(const QString &output, int minColumns)
{
    if (output.isEmpty()) {
        return true; // Empty output is acceptable (no devices/partitions)
    }

    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        // Count fields separated by whitespace
        const QStringList fields = trimmed.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (fields.size() < minColumns) {
            return false;
        }
    }
    return true;
}
} // namespace ValidationUtils
