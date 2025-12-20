# Live-usb-maker migration plan

Goal: remove reliance on `/usr/bin/live-usb-maker` and the embedded script copy,
replacing it with a Qt/C++ backend while keeping the GUI-exposed behavior and
the `/var/log/live-usb-maker.log` output.

Scope decisions:
- Cover only the features exposed by the current GUI.
- Keep clone support (`clone`, `clone=<dir>`, `--clone-persist`) if possible.
- Prefer external system tools where needed; replace only when it is a
  straightforward swap that does not change behavior.
- Keep pkexec + helper for elevation unless a better option appears.
- Preserve the live-usb-maker log file location and separation from GUI logs.
- Encryption keeps the GUI-default first-boot passphrase behavior.

Planned migration stages:
- Map GUI options to script behaviors and note the exact command paths that need
  native replacements.
- Implement Qt/C++ backend modules for partitioning, filesystem creation,
  copy/clone, bootloader updates, encryption hooks, and progress logging.
- Remove `src/live-usb-maker.sh`, `scripts.qrc`, and the `cli-shell-utils`
  dependency once the backend is wired in.
- Update documentation with any deltas and preserved quirks.

Backend implementation status:
- Qt backend executable: `mx-live-usb-maker-backend`, invoked via pkexec helper.
- GUI writes a JSON config to `/tmp` and passes it to the backend.
- The backend logs to `/var/log/live-usb-maker.log` and streams output to stdout.
- Embedded `src/live-usb-maker.sh` and `scripts.qrc` are removed.

GUI option mapping (planned backend behavior):
- Normal mode: create partitions, format, copy ISO/clone, install bootloader.
- dd mode: run partition-clear, then raw `dd` of the ISO to the target device.
- Update: skip re-partitioning and formatting, copy main files only.
- GPT + pmbr: use GPT partitioning; optionally set `pmbr_boot` disk flag.
- ESP size: set UEFI partition size in MiB.
- Main size percent: allocate the main partition percentage (or data-first).
- Data-first: create a leading data partition with selected filesystem.
- Keep syslinux: skip syslinux module updates.
- Save boot: preserve existing `/boot` when updating.
- Encrypt: create LUKS main partition and enable initrd encryption, defaulting
  to first-boot passphrase creation.
- Force flags: `usb`, `automount`, `makefs`, `nofuse` map to their existing
  behaviors in the script.

External tools expected to remain:
- Partitioning: `parted`, `sfdisk`, `partprobe`, `lsblk`, `blkid`.
- Filesystems: `mkfs.ext4`, `mkfs.vfat`, `mkfs.exfat`, `mkfs.ntfs`, `tune2fs`.
- ISO/clone: `mount`, `umount`, `rsync`/`cp`, `xorriso`, `dd`.
- Bootloader: `extlinux`, `syslinux`, `grub-install`/`grub` config edits.
- Encryption/initrd: `cryptsetup`, `dmsetup`, `cpio`, `gzip` (initrd edits).

Legacy quirks to preserve (do not fix):
- Default partition table remains `msdos` (Dell BIOS workaround).
- UEFI memtest workaround remains (fallback copy of `grubx64.efi`).
- Reliance on parsing `parted` output and external tools remains, including the
  same failure modes when tools are missing or output changes.

Decision:
- For dd mode, keep the preliminary partition-clear step to remove trailing
  old partitions before the raw dd write.

Known deltas to document:
- `--force=nofuse` is ignored because the backend mounts ISOs with loop+ro.
- Clone boot directory detection is fixed to `antiX` (no `find_live_boot_dir`
  heuristics yet).
- Update mode does not auto-detect encryption; it relies on the GUI checkbox.
