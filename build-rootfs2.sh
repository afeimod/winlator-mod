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
    )
    
    for pkg in "${essential_packages[@]}"; do
        echo "安装 $pkg..."
        if ! pacman -S --noconfirm --needed "$pkg"; then
            echo "⚠️ $pkg 安装失败，尝试继续..."
        fi
    done
    
    echo "✅ 最小化依赖安装完成"
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
    
    # 下载必要的附加文件
    if [[ -f "data.tar.xz" ]]; then
        tar -xf data.tar.xz -C "$rootfs"
    fi
    
    if [[ -f "tzdata-"*".pkg.tar.xz" ]]; then
        tar -xf tzdata-*.pkg.tar.xz -C "$rootfs"
    fi
    
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
    
    if ../configure --prefix=/usr --libdir=/lib; then
        make -j2 && make DESTDIR="/data/data/com.winlator/files/rootfs" install
    else
        echo "❌ xz 配置失败"
        return 1
    fi
    
    echo "✅ xz 构建完成"
}

# 构建 libxkbcommon (简化版本)
build_libxkbcommon_simple() {
    echo "构建 libxkbcommon..."
    
    cd /tmp
    
    # 克隆源码
    if [[ ! -d "libxkbcommon-src" ]]; then
        if ! git clone -b "$libxkbcommonVer" https://github.com/xkbcommon/libxkbcommon.git libxkbcommon-src; then
            echo "❌ libxkbcommon 源码克隆失败"
            return 1
        fi
    fi
    
    cd libxkbcommon-src
    
    # 简化构建配置
    meson setup builddir \
        -Denable-xkbregistry=false \
        -Denable-bash-completion=false \
        -Denable-docs=false \
        --prefix=/usr \
        --libdir=lib \
        --buildtype=release
    
    if [[ -d "builddir" ]]; then
        meson compile -C builddir && \
        meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir
    else
        echo "❌ libxkbcommon 构建目录创建失败"
        return 1
    fi
    
    echo "✅ libxkbcommon 构建完成"
}

# 构建 MangoHud (简化版本)
build_mangohud_simple() {
    echo "构建 MangoHud..."
    
    cd /tmp
    
    # 克隆源码
    if [[ ! -d "MangoHud-src" ]]; then
        if ! git clone -b "$mangohudVer" https://github.com/flightlessmango/MangoHud.git MangoHud-src; then
            echo "❌ MangoHud 源码克隆失败"
            return 1
        fi
    fi
    
    cd MangoHud-src
    
    # 极简配置
    meson setup builddir \
        --prefix=/usr \
        --libdir=lib \
        -Dbuildtype=release \
        -Dwith_x11=enabled \
        -Dwith_wayland=disabled \
        -Dwith_xnvctrl=disabled \
        -Dwith_dbus=disabled \
        -Dmangoplot=disabled \
        -Dmangoapp=false \
        -Dmangohudctl=false \
        -Dtests=disabled
    
    if [[ -d "builddir" ]]; then
        meson compile -C builddir && \
        meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir
    else
        echo "❌ MangoHud 构建目录创建失败"
        return 1
    fi
    
    echo "✅ MangoHud 构建完成"
}

# 构建 GStreamer (简化版本)
build_gstreamer_simple() {
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
    
    # 简化配置
    meson setup builddir \
        --buildtype=release \
        -Dintrospection=disabled \
        -Dgst-full-libraries=app,video,player \
        -Dprefix=/usr \
        -Dlibdir=lib \
        -Dauto_features=disabled \
        -Dgst-plugins-base:app=enabled \
        -Dgst-plugins-base:video=enabled
    
    if [[ -d "builddir" ]]; then
        meson compile -C builddir && \
        meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir
    else
        echo "❌ GStreamer 构建目录创建失败"
        return 1
    fi
    
    echo "✅ GStreamer 构建完成"
}

# 修复 ELF 文件 (简化版本)
fix_elf_files() {
    echo "修复 ELF 文件..."
    
    local rootfs="/data/data/com.winlator/files/rootfs"
    local interpreter="$rootfs/lib/ld-linux-aarch64.so.1"
    
    if [[ ! -f "$interpreter" ]]; then
        echo "⚠️ 解释器不存在，跳过 ELF 修复"
        return 0
    fi
    
    # 只修复可执行文件，不修复库文件
    find "$rootfs/usr/bin" "$rootfs/bin" -type f -executable 2>/dev/null | while read -r file; do
        if file "$file" | grep -q "ELF"; then
            echo "修复: $file"
            patchelf --set-interpreter "$interpreter" "$file" 2>/dev/null || true
            patchelf --set-rpath "/data/data/com.winlator/files/rootfs/lib" "$file" 2>/dev/null || true
        fi
    done
    
    echo "✅ ELF 文件修复完成"
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
  libxkbcommon=> $libxkbcommonVer
  MangoHud=> $mangohudVer
  rootfs-tag=> $customTag
Repo:
  [Waim908/rootfs-custom-winlator](https://github.com/Waim908/rootfs-custom-winlator)
Built with simplified script
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
        tar -cf - ./* | xz -T0 -c > "$output_dir/output-lite.tar.xz"
    else
        tar -czf "$output_dir/output-lite.tar.gz" ./*
    fi
    
    # 创建完整版（如果有附加数据）
    if [[ -f "/tmp/data.tar.xz" ]]; then
        tar -xf /tmp/data.tar.xz -C "$rootfs"
        create_version_info
        
        echo "创建完整版包..."
        if command -v xz >/dev/null 2>&1; then
            tar -cf - ./* | xz -T0 -c > "$output_dir/output-full.tar.xz"
        else
            tar -czf "$output_dir/output-full.tar.gz" ./*
        fi
    fi
    
    # 创建最终的 rootfs.tzst
    echo "创建 rootfs.tzst..."
    if command -v zstd >/dev/null 2>&1; then
        tar -cf - ./* | zstd -T0 -c > "$output_dir/rootfs.tzst"
    else
        tar -czf "$output_dir/rootfs.tar.gz" ./*
    fi
    
    echo "✅ 打包完成"
    echo "输出文件在: $output_dir"
    ls -la "$output_dir"
}

# 主构建流程
main() {
    echo "开始简化构建流程..."
    
    # 初始化环境
    if [[ ! -f /tmp/init.sh ]]; then
        echo "❌ 初始化脚本不存在"
        exit 1
    fi
    
    source /tmp/init.sh
    echo "版本信息:"
    echo "  gstreamer=> $gstVer"
    echo "  xz=> $xzVer"
    echo "  libxkbcommon=> $libxkbcommonVer"
    echo "  MangoHud=> $mangohudVer"
    
    # 设置镜像源
    setup_mirrors
    
    # 安装最小化依赖
    install_minimal_deps
    
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
    
    if ! build_libxkbcommon_simple; then
        echo "⚠️ libxkbcommon 构建失败，继续其他组件"
    fi
    
    if ! build_mangohud_simple; then
        echo "⚠️ MangoHud 构建失败，继续其他组件"
    fi
    
    if ! build_gstreamer_simple; then
        echo "⚠️ GStreamer 构建失败，继续其他组件"
    fi
    
    # 修复 ELF 文件
    fix_elf_files
    
    # 创建版本信息
    create_version_info
    
    # 打包成品
    package_results
    
    echo "🎉 构建流程完成！"
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