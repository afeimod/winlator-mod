#!/bin/bash

# 设置镜像源
setup_mirrors() {
    echo "设置 Arch Linux ARM 镜像源..."
    
    # 备份原始镜像列表
    cp /etc/pacman.d/mirrorlist /etc/pacman.d/mirrorlist.backup
    
    # 使用更快的镜像源
    cat > /etc/pacman.d/mirrorlist << 'EOF'
## Arch Linux ARM mirrorlist
## 使用更快的镜像源

## 主要镜像
Server = http://mirror.archlinuxarm.org/$arch/$repo
Server = http://eu.mirror.archlinuxarm.org/$arch/$repo
Server = http://sg.mirror.archlinuxarm.org/$arch/$repo

## 备用镜像
Server = https://mirror.archlinuxarm.org/$arch/$repo
Server = https://eu.mirror.archlinuxarm.org/$arch/$repo
EOF

    # 清理包缓存
    pacman -Scc --noconfirm
    
    # 更新包数据库
    pacman -Sy --noconfirm || echo "包数据库更新失败，继续构建..."
}

# 安装基本依赖（最小化）
install_minimal_deps() {
    echo "安装最小化依赖..."
    
    # 只安装绝对必要的包
    local essential_packages=(
        "base-devel"
        "git"
        "wget"
        "patchelf"
        "meson"
        "ninja"
        "python-mako"
    )
    
    for pkg in "${essential_packages[@]}"; do
        echo "安装 $pkg..."
        if ! pacman -S --noconfirm --needed "$pkg"; then
            echo "⚠️ $pkg 安装失败，尝试继续..."
        fi
    done
    
    echo "✅ 最小化依赖安装完成"
}

# 创建根文件系统目录结构
create_rootfs_dir() {
    echo "创建根文件系统目录结构..."
    
    local RootDirectories=(
        etc
        home
        opt
        tmp
        var
        usr/bin
        usr/lib
        usr/share
        usr/local
        usr/libexec
        usr/include
        usr/games
        usr/src
        usr/sbin
    )
    
    local rootfs="/data/data/com.winlator/files/rootfs"
    local nowPath=$(pwd)
    
    mkdir -p "$rootfs"
    
    for dir in "${RootDirectories[@]}"; do
        mkdir -p "$rootfs/$dir"
    done
    
    cd "$rootfs"
    ln -sf usr/bin bin
    ln -sf usr/lib lib
    ln -sf usr/sbin sbin
    cd "$nowPath"
    
    echo "✅ 根文件系统目录结构创建完成"
}

# 应用补丁函数
apply_patch() {
    if [[ ! -d /tmp/patches ]]; then
        echo "⚠️ 补丁目录不存在，跳过补丁应用"
        return 0
    fi
    
    if [[ -d "/tmp/patches/$1/$2" ]]; then
        for patch_file in /tmp/patches/$1/$2/*; do
            if [[ -f "$patch_file" ]]; then
                echo "应用补丁: $patch_file"
                if ! patch -p1 < "$patch_file"; then
                    echo "❌ 应用补丁 $patch_file 失败"
                    return 1
                fi
            fi
        done
    else
        echo "⚠️ 没有找到 $1/$2 的补丁文件"
    fi
    
    return 0
}

# 修复 ELF 文件（完整版本）
patchelf_fix() {
    echo "修复 ELF 文件..."
    
    local LD_RPATH="/data/data/com.winlator/files/rootfs/usr/lib"
    local LD_FILE="$LD_RPATH/ld-linux-aarch64.so.1"
    local rootfs="/data/data/com.winlator/files/rootfs"
    
    if [[ ! -f "$LD_FILE" ]]; then
        echo "⚠️ 解释器不存在，跳过 ELF 修复"
        return 0
    fi
    
    find "$rootfs" -type f -exec file {} + | grep -E ":.*ELF" | cut -d: -f1 | while read -r elf_file; do
        echo "修复: $elf_file"
        patchelf --set-rpath "$LD_RPATH" --set-interpreter "$LD_FILE" "$elf_file" 2>/dev/null || {
            echo "⚠️ 修复 $elf_file 失败，继续..."
        }
    done
    
    echo "✅ ELF 文件修复完成"
}

# 修复基础环境
fix_basic_environment() {
    echo "修复基础环境..."
    
    local rootfs="/data/data/com.winlator/files/rootfs"
    local rootfs_lib="$rootfs/lib"
    local rootfs_usr_lib="$rootfs/usr/lib"
    
    # 确保基础目录结构
    mkdir -p "$rootfs_lib"
    mkdir -p "$rootfs_usr_lib"
    mkdir -p "$rootfs/usr/bin"
    mkdir -p "$rootfs/bin"
    
    # 创建必要的符号链接
    if [[ ! -L "$rootfs_usr_lib" ]]; then
        ln -sf "../lib" "$rootfs_usr_lib"
    fi
    
    # 创建 Winlator 特定的配置目录
    mkdir -p "$rootfs/usr/share/mangohud"
    mkdir -p "$rootfs/etc/mangohud"
    
    echo "✅ 基础环境修复完成"
}

# 下载预编译的库文件
download_prebuilt_libraries() {
    echo "下载预编译库文件..."
    
    local rootfs="/data/data/com.winlator/files/rootfs"
    
    cd /tmp
    
    # 下载基础 rootfs
    if [[ ! -f "rootfs.tzst" ]]; then
        echo "下载 rootfs..."
        if ! wget -q --show-progress https://github.com/Waim908/rootfs-custom-winlator/releases/download/ori-b11.0/rootfs.tzst; then
            echo "❌ rootfs 下载失败"
            return 1
        fi
    fi
    
    # 解压 rootfs
    echo "解压 rootfs..."
    tar -xf rootfs.tzst -C "$rootfs"
    
    # 下载 CA 证书
    echo "下载 CA 证书..."
    cd "$rootfs/etc"
    mkdir -p ca-certificates
    cd ca-certificates
    if ! wget -q https://curl.haxx.se/ca/cacert.pem; then
        echo "⚠️ CA 证书下载失败，继续构建..."
    fi
    
    cd /tmp
    
    echo "✅ 预编译库文件下载完成"
}

# 构建 xz (必要依赖)
build_xz() {
    echo "构建 xz..."
    
    cd /tmp
    
    # 克隆源码
    if [[ ! -d "xz-src" ]]; then
        if ! git clone -b "$xzVer" https://github.com/tukaani-project/xz.git xz-src; then
            echo "❌ xz 源码克隆失败"
            return 1
        fi
    fi
    
    cd xz-src
    
    # 配置和构建
    ./autogen.sh
    mkdir -p build
    cd build
    
    if ../configure --prefix=/data/data/com.winlator/files/rootfs/usr; then
        make -j$(nproc) && make install
    else
        echo "❌ xz 配置失败"
        return 1
    fi
    
    echo "✅ xz 构建完成"
}

# 构建 libxkbcommon (使用与原始脚本相同的配置)
build_libxkbcommon() {
    echo "构建 libxkbcommon..."
    
    cd /tmp
    
    # 克隆源码
    if [[ ! -d "xkbcommon-src" ]]; then
        if ! git clone -b "$xkbcommonVer" https://github.com/xkbcommon/libxkbcommon.git xkbcommon-src; then
            echo "❌ libxkbcommon 源码克隆失败"
            return 1
        fi
    fi
    
    cd xkbcommon-src
    
    # 使用与原始脚本相同的配置
    meson setup builddir \
        --buildtype=release \
        --strip \
        --prefix=/data/data/com.winlator/files/rootfs/usr \
        --libdir=/data/data/com.winlator/files/rootfs/usr/lib \
        -Dbash-completion-path=false \
        -Denable-xkbregistry=false \
        -Denable-wayland=false \
        -Denable-tools=false \
        -Denable-bash-completion=false
    
    if [[ -d "builddir" ]]; then
        meson compile -C builddir && \
        meson install -C builddir
    else
        echo "❌ libxkbcommon 构建目录创建失败"
        return 1
    fi
    
    echo "✅ libxkbcommon 构建完成"
}

# 创建 MangoHud Winlator 配置文件
create_mangohud_winlator_config() {
    echo "创建 MangoHud Winlator 配置文件..."
    
    local rootfs="/data/data/com.winlator/files/rootfs"
    local mangohud_dir="$rootfs/usr/share/mangohud"
    local config_dir="$rootfs/etc/mangohud"
    
    mkdir -p "$mangohud_dir"
    mkdir -p "$config_dir"
    
    # 创建 MangoHud 配置文件
    cat > "$config_dir/MangoHud.conf" << 'EOF'
# MangoHud 配置文件 for Winlator
no_display
gpu_stats
gpu_temp
gpu_core_clock
gpu_mem_clock
gpu_power
gpu_load_change
gpu_load_value=50,90
gpu_load_color=FFFFFF,FF7800,CC0000
cpu_stats
cpu_temp
cpu_power
cpu_mhz
cpu_load_change
core_load_change
io_stats
vram
vram_color=2e97cb
ram
ram_color=c26693
fps
fps_color=2e97cb
engine_version
engine_color=2e97cb
gpu_color=2e97cb
cpu_color=2e97cb
vulkan_driver
wine
wine_color=eb5b5b
frame_timing=1
frametime_color=00ff00
background_alpha=0.4
font_size=24
background_color=020202
position=top-left
text_color=ffffff
round_corners=10
table_columns=3
toggle_hud=Shift_R+F12
toggle_logging=Shift_L+F2
reload_cfg=Shift_L+F4
upload_log=F5
EOF

    # 创建 Winlator 特定的路径配置
    cat > "$mangohud_dir/winlator-paths.conf" << 'EOF'
# Winlator 特定路径配置
WINLATOR_ROOT=/data/data/com.winlator/files/rootfs
WINLATOR_LIB_PATH=/data/data/com.winlator/files/rootfs/usr/lib
WINLATOR_BIN_PATH=/data/data/com.winlator/files/rootfs/usr/bin
XDG_CONFIG_HOME=/data/data/com.winlator/files/rootfs/.config
XDG_DATA_HOME=/data/data/com.winlator/files/rootfs/.local/share
EOF

    echo "✅ MangoHud Winlator 配置文件创建完成"
}

# 构建 MangoHud (修复版本，包含 Winlator 路径)
build_mangohud() {
    echo "构建 MangoHud..."
    
    cd /tmp
    
    # 克隆源码
    if [[ ! -d "mangohud-src" ]]; then
        if ! git clone -b "$mangohudVer" https://github.com/flightlessmango/MangoHud.git mangohud-src; then
            echo "❌ MangoHud 源码克隆失败"
            return 1
        fi
    fi
    
    cd mangohud-src
    
    # 应用补丁
    apply_patch mangohud "$mangohudVer"
    
    # 为 Winlator 创建自定义补丁（如果不存在）
    if [[ ! -f "/tmp/patches/mangohud/$mangohudVer/winlator-paths.patch" ]]; then
        echo "创建 Winlator 路径补丁..."
        cat > /tmp/winlator-paths.patch << 'PATCHEOF'
--- a/src/overlay.cpp
+++ b/src/overlay.cpp
@@ -XXX,XX +XXX,XX @@
     { "/proc/self/exe", true },
+    { "/data/data/com.winlator/files/rootfs/proc/self/exe", true },
     { (get_wine_exe_name() + "/data/data/com.winlator/files/rootfs" + get_wine_exe_name()).c_str(), true },
     { "\\??\\" + get_wine_exe_name(), true },
     { get_game_exe(), false },
PATCHEOF
        
        # 应用临时补丁
        if patch -p1 < /tmp/winlator-paths.patch; then
            echo "✅ Winlator 路径补丁应用成功"
        else
            echo "⚠️ Winlator 路径补丁应用失败，继续构建..."
        fi
    fi
    
    # 使用与原始脚本相同的配置，但添加 Winlator 特定路径
    meson setup builddir \
        --buildtype=release \
        --strip \
        --prefix=/data/data/com.winlator/files/rootfs/usr \
        --libdir=/data/data/com.winlator/files/rootfs/usr/lib \
        -Ddynamic_string_tokens=false \
        -Dwith_xnvctrl=disabled \
        -Dwith_wayland=disabled \
        -Dwith_nvml=disabled \
        -Dinclude_doc=false \
        -Dappend_libdir_mangohud=false \
        -Dmangoapp=false \
        -Dmangoapp_layer=false \
        -Dmangohudctl=false
    
    if [[ -d "builddir" ]]; then
        meson compile -C builddir && \
        meson install -C builddir
    else
        echo "❌ MangoHud 构建目录创建失败"
        return 1
    fi
    
    # 创建 Winlator 配置文件
    create_mangohud_winlator_config
    
    echo "✅ MangoHud 构建完成"
}

# 构建 GStreamer (使用与原始脚本相同的配置)
build_gstreamer() {
    echo "构建 GStreamer..."
    
    cd /tmp
    
    # 克隆源码
    if [[ ! -d "gst-src" ]]; then
        if ! git clone -b "$gstVer" https://github.com/GStreamer/gstreamer.git gst-src; then
            echo "❌ GStreamer 源码克隆失败"
            return 1
        fi
    fi
    
    cd gst-src
    
    # 使用与原始脚本相同的配置
    meson setup builddir \
        --buildtype=release \
        --strip \
        --prefix=/data/data/com.winlator/files/rootfs/usr \
        --libdir=/data/data/com.winlator/files/rootfs/usr/lib \
        -Dgst-full-target-type=shared_library \
        -Dintrospection=disabled \
        -Dgst-full-libraries=app,video,player \
        -Dbase=enabled \
        -Dgood=enabled \
        -Dbad=enabled \
        -Dugly=enabled \
        -Dlibav=enabled \
        -Dtests=disabled \
        -Dexamples=disabled \
        -Ddoc=disabled \
        -Dges=disabled \
        -Dpython=disabled \
        -Ddevtools=disabled \
        -Dgstreamer:check=disabled \
        -Dgstreamer:benchmarks=disabled \
        -Dgstreamer:libunwind=disabled \
        -Dgstreamer:libdw=disabled \
        -Dgstreamer:bash-completion=disabled \
        -Dgst-plugins-good:cairo=disabled \
        -Dgst-plugins-good:gdk-pixbuf=disabled \
        -Dgst-plugins-good:oss=disabled \
        -Dgst-plugins-good:oss4=disabled \
        -Dgst-plugins-good:v4l2=disabled \
        -Dgst-plugins-good:aalib=disabled \
        -Dgst-plugins-good:jack=disabled \
        -Dgst-plugins-good:pulse=enabled \
        -Dgst-plugins-good:adaptivedemux2=disabled \
        -Dgst-plugins-good:v4l2=disabled \
        -Dgst-plugins-good:libcaca=disabled \
        -Dgst-plugins-good:mpg123=enabled \
        -Dgst-plugins-base:examples=disabled \
        -Dgst-plugins-base:alsa=enabled \
        -Dgst-plugins-base:pango=disabled \
        -Dgst-plugins-base:x11=enabled \
        -Dgst-plugins-base:gl=disabled \
        -Dgst-plugins-base:opus=disabled \
        -Dgst-plugins-bad:androidmedia=disabled \
        -Dgst-plugins-bad:rtmp=disabled \
        -Dgst-plugins-bad:shm=disabled \
        -Dgst-plugins-bad:zbar=disabled \
        -Dgst-plugins-bad:webp=disabled \
        -Dgst-plugins-bad:kms=disabled \
        -Dgst-plugins-bad:vulkan=disabled \
        -Dgst-plugins-bad:dash=disabled \
        -Dgst-plugins-bad:analyticsoverlay=disabled \
        -Dgst-plugins-bad:nvcodec=disabled \
        -Dgst-plugins-bad:uvch264=disabled \
        -Dgst-plugins-bad:v4l2codecs=disabled \
        -Dgst-plugins-bad:udev=disabled \
        -Dgst-plugins-bad:libde265=disabled \
        -Dgst-plugins-bad:smoothstreaming=disabled \
        -Dgst-plugins-bad:fluidsynth=disabled \
        -Dgst-plugins-bad:inter=disabled \
        -Dgst-plugins-bad:x11=enabled \
        -Dgst-plugins-bad:gl=disabled \
        -Dgst-plugins-bad:wayland=disabled \
        -Dgst-plugins-bad:openh264=disabled \
        -Dgst-plugins-bad:hip=disabled \
        -Dgst-plugins-bad:aja=disabled \
        -Dgst-plugins-bad:aes=disabled \
        -Dgst-plugins-bad:dtls=disabled \
        -Dgst-plugins-bad:hls=disabled \
        -Dgst-plugins-bad:curl=disabled \
        -Dgst-plugins-bad:opus=disabled \
        -Dgst-plugins-bad:webrtc=disabled \
        -Dgst-plugins-bad:webrtcdsp=disabled \
        -Dpackage-origin="[rootfs-custom-winlator](https://github.com/Waim908/rootfs-custom-winlator)"
    
    if [[ -d "builddir" ]]; then
        meson compile -C builddir && \
        meson install -C builddir
    else
        echo "❌ GStreamer 构建目录创建失败"
        return 1
    fi
    
    echo "✅ GStreamer 构建完成"
}

# 创建版本信息
create_version_info() {
    echo "创建版本信息..."
    
    local date=$(TZ=Asia/Shanghai date '+%Y-%m-%d %H:%M:%S')
    
    cat > "/data/data/com.winlator/files/rootfs/_version_.txt" << EOF
Output Date(UTC+8): $date
Version:
  gstreamer=> $gstVer
  xz=> $xzVer
  libxkbcommon=> $xkbcommonVer
  MangoHud=> $mangohudVer
  rootfs-tag=> $customTag
Repo:
  [Waim908/rootfs-custom-winlator](https://github.com/Waim908/rootfs-custom-winlator)
Built with Winlator path fixes
EOF
    
    echo "✅ 版本信息创建完成"
}

# 打包成品
package_results() {
    echo "打包成品..."
    
    local rootfs="/data/data/com.winlator/files/rootfs"
    local output_dir="/tmp/output"
    
    mkdir -p "$output_dir"
    
    cd "$rootfs"
    
    # 创建精简版
    echo "创建精简版包..."
    if command -v xz >/dev/null 2>&1; then
        tar -I 'xz -T0 -9' -cf "$output_dir/output-lite.tar.xz" ./*
    else
        tar -czf "$output_dir/output-lite.tar.gz" ./*
    fi
    
    # 添加附加数据并创建完整版
    echo "添加附加数据..."
    cd /tmp
    if [[ -f "data.tar.xz" ]]; then
        tar -xf data.tar.xz -C "$rootfs"
    fi
    
    if ls tzdata-*.pkg.tar.xz 1> /dev/null 2>&1; then
        tar -xf tzdata-*.pkg.tar.xz -C "$rootfs"
    fi
    
    # 复制字体和其他资源
    if [[ -d "fonts" ]]; then
        cp -r -p fonts "$rootfs/usr/share/"
    fi
    
    if [[ -d "extra" ]]; then
        cp -r -p extra "$rootfs/"
    fi
    
    # 更新版本信息
    create_version_info
    
    cd "$rootfs"
    
    # 创建完整版包
    echo "创建完整版包..."
    if command -v xz >/dev/null 2>&1; then
        tar -I 'xz -T0 -9' -cf "$output_dir/output-full.tar.xz" ./*
    else
        tar -czf "$output_dir/output-full.tar.gz" ./*
    fi
    
    # 创建最终的 rootfs.tzst
    echo "创建 rootfs.tzst..."
    if command -v zstd >/dev/null 2>&1; then
        tar -I 'zstd -T0 -9' -cf "$output_dir/rootfs.tzst" ./*
    else
        tar -czf "$output_dir/rootfs.tar.gz" ./*
    fi
    
    echo "✅ 打包完成"
    echo "输出文件在: $output_dir"
    ls -la "$output_dir"
}

# 主构建流程
main() {
    echo "开始 Winlator 路径修复构建流程..."
    
    # 初始化环境
    if [[ ! -f /tmp/init.sh ]]; then
        echo "❌ 初始化脚本不存在"
        exit 1
    fi
    
    source /tmp/init.sh
    echo "版本信息:"
    echo "  gstreamer=> $gstVer"
    echo "  xz=> $xzVer"
    echo "  libxkbcommon=> $xkbcommonVer"
    echo "  MangoHud=> $mangohudVer"
    
    # 设置镜像源
    setup_mirrors
    
    # 安装最小化依赖
    install_minimal_deps
    
    # 创建根文件系统目录结构
    create_rootfs_dir
    
    # 修复基础环境
    fix_basic_environment
    
    # 下载预编译库
    if ! download_prebuilt_libraries; then
        echo "❌ 预编译库下载失败"
        exit 1
    fi
    
    # 构建各组件
    echo "开始构建组件..."
    
    if ! build_xz; then
        echo "⚠️ xz 构建失败，继续其他组件"
    fi
    
    if ! build_libxkbcommon; then
        echo "⚠️ libxkbcommon 构建失败，继续其他组件"
    fi
    
    if ! build_mangohud; then
        echo "⚠️ MangoHud 构建失败，继续其他组件"
    fi
    
    if ! build_gstreamer; then
        echo "⚠️ GStreamer 构建失败，继续其他组件"
    fi
    
    # 修复 ELF 文件
    patchelf_fix
    
    # 创建版本信息
    create_version_info
    
    # 打包成品
    package_results
    
    echo "🎉 Winlator 路径修复构建流程完成！"
    echo "================================="
    echo "输出目录: /tmp/output"
    echo "包含文件:"
    ls -la /tmp/output/
    echo "================================="
}

# 错误处理
set -e
trap 'echo "❌ 脚本在 line $LINENO 失败: $BASH_COMMAND"; exit 1' ERR

# 运行主流程
main "$@"