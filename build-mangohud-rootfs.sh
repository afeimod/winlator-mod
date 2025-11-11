#!/bin/bash

patchelf_fix() {
LD_RPATH=/data/data/com.winlator/files/rootfs/lib
LD_FILE=$LD_RPATH/ld-linux-aarch64.so.1
  find . -type f -exec file {} + | grep -E ":.*ELF" | cut -d: -f1 | while read -r elf_file; do
    echo "Patching $elf_file..."
    patchelf --set-rpath "$LD_RPATH" --set-interpreter "$LD_FILE" "$elf_file" || {
      echo "Failed to patch $elf_file" >&2
      continue
    }
  done
}

create_ver_txt () {
  cat > '/data/data/com.winlator/files/rootfs/_version_.txt' << EOF
Output Date(UTC+8): $date
Version:
  mangohud=> $mangohudVer
  commit=> $mangohudCommit
  rootfs-tag=> $customTag
Repo:
  [Waim908/rootfs-custom-winlator](https://github.com/Waim908/rootfs-custom-winlator)
EOF
}

if [[ ! -f /tmp/init.sh ]]; then
  exit 1
else
  source /tmp/init.sh
  echo "mangohud=> $mangohudVer"
  echo "commit=> $mangohudCommit"
fi

# 安装必要的网络工具
echo "安装必要的网络工具..."
pacman -Syu --noconfirm
pacman -S --noconfirm --needed wget ca-certificates

mkdir -p /data/data/com.winlator/files/rootfs/
cd /tmp

# Download base rootfs
if ! wget https://github.com/Waim908/rootfs-custom-winlator/releases/download/ori-b11.0/rootfs.tzst; then
  exit 1
fi

# Extract base rootfs
tar -xf rootfs.tzst -C /data/data/com.winlator/files/rootfs/

# Install CA certificates
cd /data/data/com.winlator/files/rootfs/etc
mkdir -p ca-certificates
if ! wget https://curl.haxx.se/ca/cacert.pem; then
  exit 1
fi

cd /tmp

# Clone MangoHud source
if [[ -n "$mangohudCommit" && "$mangohudCommit" != "" ]]; then
  # Clone with specific commit
  if ! git clone https://github.com/flightlessmango/MangoHud.git mangohud-src; then
    exit 1
  fi
  cd mangohud-src
  git checkout $mangohudCommit
  cd /tmp
else
  # Clone with specific version tag
  if ! git clone -b $mangohudVer https://github.com/flightlessmango/MangoHud.git mangohud-src; then
    exit 1
  fi
fi

# Build MangoHud
echo "Build and Compile MangoHud"
cd /tmp/mangohud-src

# 检查必要的工具
echo "检查构建工具..."
if ! command -v glslangValidator &> /dev/null; then
    echo "错误: glslangValidator 未找到"
    exit 1
fi

if ! command -v meson &> /dev/null; then
    echo "错误: meson 未找到"
    exit 1
fi

# Configure MangoHud build
meson setup build \
  --buildtype=release \
  --strip \
  -Dwith_x11=enabled \
  -Dwith_wayland=enabled \
  -Dwith_xnvctrl=disabled \
  -Dwith_dbus=enabled \
  -Dmangoplot=enabled \
  -Dmangoapp=false \
  -Dmangohudctl=false \
  -Dtests=disabled \
  -Dprefix=/data/data/com.winlator/files/rootfs/ || {
    echo "Meson配置失败"
    if [[ -d build/meson-logs ]]; then
        cat build/meson-logs/meson-log.txt
    fi
    exit 1
}

if [[ ! -d build ]]; then
  echo "构建目录未创建"
  exit 1
fi

# Compile and install
echo "开始编译 MangoHud..."
if ! ninja -C build -j$(nproc); then
  echo "编译失败"
  exit 1
fi

echo "安装 MangoHud..."
ninja -C build install

# Install MangoHud configuration file
mkdir -p /data/data/com.winlator/files/rootfs/etc/mangohud/
cat > /data/data/com.winlator/files/rootfs/etc/mangohud/MangoHud.conf << 'EOF'
# MangoHud configuration file
# Place in /etc/mangohud/MangoHud.conf for system-wide settings
# or in ~/.config/MangoHud/MangoHud.conf for user-specific settings

# Basic display options
output_folder=/tmp
gpu_stats
gpu_temp
gpu_core_clock
gpu_mem_clock
gpu_power
cpu_stats
cpu_temp
cpu_power
io_stats
ram
vram
fps
frametime=0
frame_count
engine_version
gpu_name
vulkan_driver
histogram
gamemode
wine

# Visual options
no_display
background_alpha=0.4
font_size=24
position=top-left
text_color=FFFFFF
background_color=020202
round_corners=10
toggle_hud=Shift_R+F12
toggle_logging=Shift_L+F2

# Wine specific
wine
wine_color=EB5B5B
EOF

export date=$(TZ=Asia/Shanghai date '+%Y-%m-%d %H:%M:%S')

# Package
echo "Package"
mkdir -p /tmp/output
cd /data/data/com.winlator/files/rootfs/

# Fix ELF files
patchelf_fix
create_ver_txt

# Create lite package (MangoHud only)
if ! tar -I 'xz -T8' -cf /tmp/output/mangohud-output-lite.tar.xz *; then
  exit 1
fi

# Add additional data for full package
cd /tmp
tar -xf data.tar.xz -C /data/data/com.winlator/files/rootfs/
tar -xf tzdata-2025b-1-aarch64.pkg.tar.xz -C /data/data/com.winlator/files/rootfs/

cd /data/data/com.winlator/files/rootfs/
create_ver_txt

# Create full package
if ! tar -I 'xz -T8' -cf /tmp/output/mangohud-output-full.tar.xz *; then
  exit 1
fi

# Create complete rootfs package
rm -rf /data/data/com.winlator/files/rootfs/*/
tar -xf rootfs.tzst -C /data/data/com.winlator/files/rootfs/
tar -xf /tmp/output/mangohud-output-full.tar.xz -C /data/data/com.winlator/files/rootfs/

cd /data/data/com.winlator/files/rootfs/
create_ver_txt

# Create final compressed rootfs
if ! tar -I 'zstd -T8' -cf /tmp/output/mangohud-rootfs.tzst *; then
  exit 1
fi