#!/bin/bash
# Chroot Container IS default root user
allPkg=(
  base-devel
  ninja
  meson
  git
  glib2-devel
  libx11
  mesa
  libpulse
  vulkan-devel
  nasm
  yasm
  glslang
  cmake
  patchelf
  libde265
)
pacman -Syu --noconfirm
pacman-key --init
pacman -S --noconfirm --needed ${allPkg[@]}
exit 0