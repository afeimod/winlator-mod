#!/bin/bash
#
# arch-bootstrap: Bootstrap a base Arch Linux system using any GNU distribution.
#
# Dependencies: bash >= 4, coreutils, curl, sed, gawk, tar, gzip, chroot, xz, zstd.
# Project: https://github.com/tokland/arch-bootstrap
#
# Install:
#
#   # install -m 755 arch-bootstrap.sh /usr/local/bin/arch-bootstrap
#
# Usage:
#
#   # arch-bootstrap destination
#   # arch-bootstrap -a x86_64 -r ftp://ftp.archlinux.org destination-64
#
# And then you can chroot to the destination directory (user: root, password: root):
#
#   # chroot destination

set -e -u -o pipefail

# Packages needed by pacman (see get-pacman-dependencies.sh)
PACMAN_PACKAGES=(
  acl archlinux-keyring attr brotli bzip2 curl expat glibc gpgme libarchive
  libassuan libgpg-error libnghttp2 libnghttp3 libssh2 lzo openssl pacman pacman-mirrorlist xz zlib
  krb5 e2fsprogs keyutils libidn2 libunistring gcc-libs lz4 libpsl icu libunistring zstd libxml2
)
BASIC_PACKAGES=(${PACMAN_PACKAGES[*]} filesystem base)
EXTRA_PACKAGES=(coreutils bash grep gawk file tar gzip systemd sed)
DEFAULT_REPO_URL="http://mirrors.kernel.org/archlinux"
DEFAULT_ARM_REPO_URL="http://mirror.archlinuxarm.org"
DEFAULT_X86_REPO_URL="http://mirror.archlinux32.org"

stderr() {
  echo "$@" >&2
}

debug() {
  stderr "--- $@"
}

extract_href() {
  sed -n '/<a / s/^.*<a [^>]*href="\([^\"]*\)".*$/\1/p'
}

fetch() {
  curl -L -s "$@"
}

fetch_file() {
  local FILEPATH=$1
  shift
  if [[ -e "$FILEPATH" ]]; then
    curl -L -z "$FILEPATH" -o "$FILEPATH" "$@"
  else
    curl -L -o "$FILEPATH" "$@"
  fi
}

uncompress() {
  local FILEPATH=$1 DEST=$2

  case "$FILEPATH" in
    *.gz)
      tar xzf "$FILEPATH" -C "$DEST";;
    *.xz)
      xz -dc "$FILEPATH" | tar x -C "$DEST";;
    *.zst)
      zstd -dc "$FILEPATH" | tar x -C "$DEST";;
    *)
      debug "Error: unknown package format: $FILEPATH"
      return 1;;
  esac
}

###

get_default_repo() {
  local ARCH=$1
  if [[ "$ARCH" == arm* || "$ARCH" == aarch64 ]]; then
    echo $DEFAULT_ARM_REPO_URL
  elif [[ "$ARCH" == i*86 || "$ARCH" == pentium4 ]]; then
    echo $DEFAULT_X86_REPO_URL
  else
    echo $DEFAULT_REPO_URL
  fi
}

get_core_repo_url() {
  local REPO_URL=$1 ARCH=$2
  if [[ "$ARCH" == arm* || "$ARCH" == aarch64 || "$ARCH" == i*86 || "$ARCH" == pentium4 ]]; then
    echo "${REPO_URL%/}/$ARCH/core"
  else
    echo "${REPO_URL%/}/core/os/$ARCH"
  fi
}

get_template_repo_url() {
  local REPO_URL=$1 ARCH=$2
  if [[ "$ARCH" == arm* || "$ARCH" == aarch64 || "$ARCH" == i*86 || "$ARCH" == pentium4 ]]; then
    echo "${REPO_URL%/}/$ARCH/\$repo"
  else
    echo "${REPO_URL%/}/\$repo/os/$ARCH"
  fi
}

configure_pacman() {
  local DEST=$1 ARCH=$2
  debug "configure DNS and pacman"
  cp "/etc/resolv.conf" "$DEST/etc/resolv.conf"
  SERVER=$(get_template_repo_url "$REPO_URL" "$ARCH")
  echo "Server = $SERVER" > "$DEST/etc/pacman.d/mirrorlist"
}

configure_minimal_system() {
  local DEST=$1

  mkdir -p "$DEST/dev"
  sed -ie 's/^root:.*$/root:$1$GT9AUpJe$oXANVIjIzcnmOpY07iaGi\/:14657::::::/' "$DEST/etc/shadow"
  touch "$DEST/etc/group"
  echo "bootstrap" > "$DEST/etc/hostname"

  rm -f "$DEST/etc/mtab"
  echo "rootfs / rootfs rw 0 0" > "$DEST/etc/mtab"
  test -e "$DEST/dev/null" || mknod "$DEST/dev/null" c 1 3
  test -e "$DEST/dev/random" || mknod -m 0644 "$DEST/dev/random" c 1 8
  test -e "$DEST/dev/urandom" || mknod -m 0644 "$DEST/dev/urandom" c 1 9

  sed -i 's/^DownloadUser/#DownloadUser/' "$DEST/etc/pacman.conf"
  sed -i "s/^[[:space:]]*\(CheckSpace\)/# \1/" "$DEST/etc/pacman.conf"
  sed -i "s/^[[:space:]]*SigLevel[[:space:]]*=.*$/SigLevel = Never/" "$DEST/etc/pacman.conf"
}

fetch_packages_list() {
  local REPO=$1

  debug "fetch packages list: $REPO/"
  fetch "$REPO/" | extract_href | awk -F"/" '{print $NF}' | sort -rn ||
    { debug "Error: cannot fetch packages list: $REPO"; return 1; }
}

install_pacman_packages() {
  local BASIC_PACKAGES=$1 DEST=$2 LIST=$3 DOWNLOAD_DIR=$4
  debug "pacman package and dependencies: $BASIC_PACKAGES"

  for PACKAGE in $BASIC_PACKAGES; do
    local FILE=$(echo "$LIST" | grep -m1 "^$PACKAGE-[[:digit:]].*\(\.gz\|\.xz\|\.zst\)$")
    test "$FILE" || { debug "Error: cannot find package: $PACKAGE"; return 1; }
    local FILEPATH="$DOWNLOAD_DIR/$FILE"

    debug "download package: $REPO/$FILE"
    fetch_file "$FILEPATH" "$REPO/$FILE"
    debug "uncompress package: $FILEPATH"
    uncompress "$FILEPATH" "$DEST"
  done
}

configure_static_qemu() {
  local ARCH=$1 DEST=$2
  [[ "$ARCH" == arm* ]] && ARCH=arm
  QEMU_STATIC_BIN=$(which qemu-$ARCH-static || echo )
  [[ -e "$QEMU_STATIC_BIN" ]] ||\
    { debug "no static qemu for $ARCH, ignoring"; return 0; }
  cp "$QEMU_STATIC_BIN" "$DEST/usr/bin"
}

install_packages() {
  local ARCH=$1 DEST=$2 PACKAGES=$3
  debug "install packages: $PACKAGES"
  LC_ALL=C chroot "$DEST" /usr/bin/pacman \
    --noconfirm --arch $ARCH -Sy --overwrite \* $PACKAGES
}

show_usage() {
  stderr "Usage: $(basename "$0") [-q] [-a i486|i686|pentium4|x86_64|arm|aarch64] [-r REPO_URL] [-d DOWNLOAD_DIR] DESTDIR"
}

main() {
  # Process arguments and options
  test $# -eq 0 && set -- "-h"
  local ARCH=
  local REPO_URL=
  local USE_QEMU=
  local DOWNLOAD_DIR=
  local PRESERVE_DOWNLOAD_DIR=

  while getopts "qa:r:d:h" ARG; do
    case "$ARG" in
      a) ARCH=$OPTARG;;
      r) REPO_URL=$OPTARG;;
      q) USE_QEMU=true;;
      d) DOWNLOAD_DIR=$OPTARG
         PRESERVE_DOWNLOAD_DIR=true;;
      *) show_usage; return 1;;
    esac
  done
  shift $(($OPTIND-1))
  test $# -eq 1 || { show_usage; return 1; }

  [[ -z "$ARCH" ]] && ARCH=$(uname -m)
  [[ -z "$REPO_URL" ]] &&REPO_URL=$(get_default_repo "$ARCH")

  local DEST=$1
  local REPO=$(get_core_repo_url "$REPO_URL" "$ARCH")
  [[ -z "$DOWNLOAD_DIR" ]] && DOWNLOAD_DIR=$(mktemp -d)
  mkdir -p "$DOWNLOAD_DIR"
  [[ -z "$PRESERVE_DOWNLOAD_DIR" ]] && trap "rm -rf '$DOWNLOAD_DIR'" KILL TERM EXIT
  debug "destination directory: $DEST"
  debug "core repository: $REPO"
  debug "temporary directory: $DOWNLOAD_DIR"

  # Fetch packages, install system and do a minimal configuration
  mkdir -p "$DEST"
  local LIST=$(fetch_packages_list $REPO)
  install_pacman_packages "${BASIC_PACKAGES[*]}" "$DEST" "$LIST" "$DOWNLOAD_DIR"
  configure_pacman "$DEST" "$ARCH"
  configure_minimal_system "$DEST"
  [[ -n "$USE_QEMU" ]] && configure_static_qemu "$ARCH" "$DEST"
  install_packages "$ARCH" "$DEST" "${BASIC_PACKAGES[*]} ${EXTRA_PACKAGES[*]}"
  configure_pacman "$DEST" "$ARCH" # Pacman must be re-configured
  [[ -z "$PRESERVE_DOWNLOAD_DIR" ]] && rm -rf "$DOWNLOAD_DIR"

  debug "Done!"
  debug
  debug "You may now chroot or arch-chroot from package arch-install-scripts:"
  debug "$ sudo arch-chroot $DEST"
}

main "$@"
