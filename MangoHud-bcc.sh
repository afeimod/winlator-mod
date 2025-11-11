#!/bin/bash
# Chroot Container IS default root user
allPkg=(
  base-devel
  git
  cmake
  meson
  ninja
  glib2-devel
  libx11
  mesa
  vulkan-devel
  libglvnd
  dbus
  gcc
  make
  pkg-config
  python3
  patchelf
  wget
  ca-certificates
  spirv-tools
  glslang
)
pacman -Syu --noconfirm
pacman-key --init
pacman -S --noconfirm --needed ${allPkg[@]}
exit 0