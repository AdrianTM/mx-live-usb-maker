#!/bin/bash

# inject-live-usb-storage.sh
#
# Injects live-usb-storage support into an Arch live USB.
#
# Usage: inject-live-usb-storage.sh <mounted-main-partition>
#
# Strategy:
#   1. Copies live-usb-storage script + service to <main>/arch/live-usb-storage/
#   2. Fully extracts the compressed initrd (preserving early CPIOs like microcode)
#   3. Patches the archiso hook with our snippet
#   4. Repacks and recompresses the initrd using the original compression format

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

die() { echo "Error: $*" >&2; exit 1; }
msg() { echo "==> $*" >&2; }
msg2() { echo "  -> $*" >&2; }

# Auto-select cpio implementation (GNU cpio or bsdcpio from libarchive)
if command -v cpio &>/dev/null; then
    CPIO=cpio
elif command -v bsdcpio &>/dev/null; then
    CPIO=bsdcpio
else
    die "No cpio implementation found (need cpio or bsdcpio)"
fi

cleanup() {
    [ -n "${WORK:-}" ] && [ -d "${WORK:-}" ] && rm -rf "$WORK"
}
trap cleanup EXIT

# --------------------------------------------------------------------------
# Hook snippet: copies service files from boot partition into the live root.
# Injected inside archiso_mount_handler(), before the copytoram unmount,
# so /run/archiso/bootmnt is still available and /new_root is set up.
# --------------------------------------------------------------------------
HOOK_SNIPPET='
    # live-usb-storage: install service files into overlay
    if [ -d /run/archiso/bootmnt/arch/live-usb-storage ]; then
        mkdir -p /new_root/usr/bin
        mkdir -p /new_root/etc/systemd/system/multi-user.target.wants
        cp /run/archiso/bootmnt/arch/live-usb-storage/live-usb-storage \
           /new_root/usr/bin/live-usb-storage
        chmod 755 /new_root/usr/bin/live-usb-storage
        cp /run/archiso/bootmnt/arch/live-usb-storage/live-usb-storage.service \
           /new_root/etc/systemd/system/live-usb-storage.service
        ln -sf ../live-usb-storage.service \
           /new_root/etc/systemd/system/multi-user.target.wants/live-usb-storage.service
    fi
'

# --------------------------------------------------------------------------
# Detect compression format of a file
# --------------------------------------------------------------------------
detect_compression() {
    local magic
    magic=$(od -A n -t x1 -N 4 "$1" | tr -d ' ')
    case "$magic" in
        1f8b*)     echo "gzip" ;;
        28b52ffd)  echo "zstd" ;;
        fd377a58)  echo "xz"   ;;
        04224d18)  echo "lz4"  ;;
        *)         echo "unknown" ;;
    esac
}

decompress_cmd() {
    case "$1" in
        gzip) echo "gzip -dc"  ;;
        zstd) echo "zstd -dc"  ;;
        xz)   echo "xz -dc"    ;;
        lz4)  echo "lz4 -dc"   ;;
        *)    die "Unsupported compression: $1" ;;
    esac
}

recompress_cmd() {
    case "$1" in
        gzip) echo "gzip -c"         ;;
        zstd) echo "zstd -c -T0"     ;;
        xz)   echo "xz -c --check=crc32 -T0" ;;
        lz4)  echo "lz4 -c -l"       ;;
        *)    die "Unsupported compression: $1" ;;
    esac
}

# --------------------------------------------------------------------------
# Skip past prepended uncompressed CPIO archives (e.g. microcode) to find
# the offset of the main (compressed) initramfs.
# --------------------------------------------------------------------------
find_main_offset() {
    local file=$1
    local filesize
    filesize=$(stat -c%s "$file")
    local offset=0

    while [ "$offset" -lt "$filesize" ]; do
        local magic
        magic=$(od -A n -t x1 -j "$offset" -N 6 "$file" | tr -d ' ')

        # If not a CPIO magic (070701/070702), we found the compressed part
        if [ "$magic" != "303730373031" ] && [ "$magic" != "303730373032" ]; then
            echo "$offset"
            return
        fi

        # Find TRAILER!!! marker from current offset
        local trailer_pos
        trailer_pos=$(grep -aboP 'TRAILER!!!' "$file" \
            | awk -F: -v start="$offset" '$1 >= start {print $1; exit}')

        [ -n "$trailer_pos" ] || break

        # Skip past trailer + alignment padding (newc uses 4-byte alignment)
        local after=$(( trailer_pos + 10 ))
        local aligned=$(( (after + 3) & ~3 ))

        # Skip null padding bytes
        while [ "$aligned" -lt "$filesize" ]; do
            local byte
            byte=$(od -A n -t x1 -j "$aligned" -N 1 "$file" | tr -d ' ')
            [ "$byte" = "00" ] && aligned=$((aligned + 1)) || break
        done

        offset=$aligned
    done

    echo "$offset"
}

# --------------------------------------------------------------------------
# Extract the full compressed initrd and find the archiso hook
# Sets: HOOK_FILE_PATH, HOOK_RELATIVE_PATH, INITRD_COMP_FMT, INITRD_MAIN_OFFSET
# --------------------------------------------------------------------------
extract_archiso_hook() {
    local initrd=$1
    local work=$2

    INITRD_MAIN_OFFSET=$(find_main_offset "$initrd")
    msg2 "Main initramfs at offset $INITRD_MAIN_OFFSET"

    # Get compressed data
    local compressed="$work/initrd.compressed"
    dd if="$initrd" bs=1 skip="$INITRD_MAIN_OFFSET" of="$compressed" 2>/dev/null

    INITRD_COMP_FMT=$(detect_compression "$compressed")
    [ "$INITRD_COMP_FMT" != "unknown" ] || die "Could not detect initrd compression format"
    msg2 "Compression: $INITRD_COMP_FMT"

    local decomp
    decomp=$(decompress_cmd "$INITRD_COMP_FMT")

    # Full extraction of the initramfs
    local extract_dir="$work/extract"
    mkdir -p "$extract_dir"
    $decomp < "$compressed" | (cd "$extract_dir" && $CPIO -id 2>/dev/null)

    HOOK_FILE_PATH=""
    local try
    for try in hooks/archiso usr/lib/initcpio/hooks/archiso; do
        if [ -f "$extract_dir/$try" ]; then
            HOOK_FILE_PATH="$extract_dir/$try"
            HOOK_RELATIVE_PATH="$try"
            break
        fi
    done

    [ -n "$HOOK_FILE_PATH" ] || die "Could not find archiso hook in initrd"
}

# --------------------------------------------------------------------------
# Main
# --------------------------------------------------------------------------
main() {
    local main_part="${1:-}"
    [ -n "$main_part" ] || die "Usage: $0 <mounted-main-partition>"
    [ -d "$main_part" ] || die "'$main_part' is not a directory"

    # CPIO auto-selected at top of script

    # Verify source files
    local src_script="$SCRIPT_DIR/live-usb-storage"
    local src_service="$SCRIPT_DIR/live-usb-storage.service"
    [ -f "$src_script" ]  || die "Missing $src_script"
    [ -f "$src_service" ] || die "Missing $src_service"

    # Step 1: Copy files to boot partition
    msg "Installing live-usb-storage files to boot partition"
    local dest_dir="$main_part/arch/live-usb-storage"
    mkdir -p "$dest_dir"
    cp "$src_script"  "$dest_dir/live-usb-storage"
    cp "$src_service" "$dest_dir/live-usb-storage.service"
    chmod 755 "$dest_dir/live-usb-storage"
    msg2 "Installed to $dest_dir"

    # Step 2: Find the initrd
    msg "Locating Arch initrd image"
    local initrd=""
    local pattern
    for pattern in \
        "$main_part"/arch/boot/*/initramfs*.img \
        "$main_part"/arch/boot/*/archiso.img \
        "$main_part"/boot/*/initramfs*.img \
        "$main_part"/boot/*/archiso.img; do
        for f in $pattern; do
            [ -f "$f" ] && initrd="$f" && break 2
        done
    done
    [ -n "$initrd" ] || die "Could not find Arch initrd image under $main_part"
    msg2 "Found: $initrd"

    # Step 3: Extract the archiso hook
    WORK=$(mktemp -d /tmp/inject-lus.XXXXXX)
    HOOK_FILE_PATH=""
    HOOK_RELATIVE_PATH=""
    INITRD_COMP_FMT=""
    INITRD_MAIN_OFFSET=""

    msg "Extracting initrd contents"
    extract_archiso_hook "$initrd" "$WORK"
    msg2 "Found hook: $HOOK_RELATIVE_PATH"

    # Step 4: Check if already patched
    if grep -q 'live-usb-storage' "$HOOK_FILE_PATH"; then
        msg2 "Hook already patched, skipping"
        msg "Done (already injected)."
        return 0
    fi

    # Step 5: Patch the hook — inject snippet inside archiso_mount_handler()
    # Insert before the final 'copytoram' unmount so /run/archiso/bootmnt is
    # still mounted and /new_root is already set up.
    local insert_marker='if \[ "\${copytoram}" = "y" \]; then'
    # Find the LAST occurrence (the unmount block at the end of archiso_mount_handler)
    local last_line
    last_line=$(grep -n 'if \[ "${copytoram}" = "y" \]; then' "$HOOK_FILE_PATH" | tail -1 | cut -d: -f1)
    if [ -z "$last_line" ]; then
        # Fallback: insert before closing brace of archiso_mount_handler
        last_line=$(grep -n '^}' "$HOOK_FILE_PATH" | tail -1 | cut -d: -f1)
    fi
    if [ -z "$last_line" ]; then
        die "Could not find insertion point in archiso hook"
    fi
    # Insert snippet before the target line
    local tmp_hook="$WORK/hook_patched"
    {
        head -n "$((last_line - 1))" "$HOOK_FILE_PATH"
        echo "$HOOK_SNIPPET"
        tail -n "+${last_line}" "$HOOK_FILE_PATH"
    } > "$tmp_hook"
    cp "$tmp_hook" "$HOOK_FILE_PATH"
    msg2 "Injected live-usb-storage snippet into archiso_mount_handler()"

    # Step 6: Recompress and rebuild the initrd
    msg "Rebuilding initrd ($INITRD_COMP_FMT)"
    local orig_size
    orig_size=$(stat -c%s "$initrd")

    local recomp
    recomp=$(recompress_cmd "$INITRD_COMP_FMT")

    # Preserve the early CPIO archives (microcode etc.) before the compressed part
    local new_initrd="$WORK/initrd.new"
    if [ "$INITRD_MAIN_OFFSET" -gt 0 ]; then
        dd if="$initrd" bs="$INITRD_MAIN_OFFSET" count=1 of="$new_initrd" 2>/dev/null
    else
        : > "$new_initrd"
    fi

    # Repack and recompress the full initramfs
    (cd "$WORK/extract" && find . -print0 | $CPIO --null -o -H newc 2>/dev/null) \
        | $recomp >> "$new_initrd" \
        || die "Failed to recompress initrd"

    cp "$new_initrd" "$initrd" || die "Failed to write new initrd"

    local new_size
    new_size=$(stat -c%s "$initrd")
    msg2 "Initrd: $orig_size -> $new_size bytes"

    msg "Done! Live-usb-storage injection complete."
}

main "$@"
