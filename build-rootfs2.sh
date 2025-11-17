#!/bin/bash

# 修复符号链接问题
fix_symlink_issues() {
  echo "修复符号链接问题..."
  
  local rootfs_lib="/data/data/com.winlator/files/rootfs/lib"
  local rootfs_usr_lib="/data/data/com.winlator/files/rootfs/usr/lib"
  
  # 检查并修复符号链接循环
  if [[ -L "$rootfs_usr_lib" ]]; then
    local link_target=$(readlink "$rootfs_usr_lib")
    if [[ "$link_target" == "../lib" ]]; then
      echo "检查到正确的符号链接: $rootfs_usr_lib -> $link_target"
    else
      echo "修复符号链接: $rootfs_usr_lib"
      rm -f "$rootfs_usr_lib"
      ln -sf "../lib" "$rootfs_usr_lib"
    fi
  fi
  
  # 确保关键目录存在
  mkdir -p "$rootfs_lib"
  mkdir -p "$(dirname "$rootfs_usr_lib")"
  
  echo "✅ 符号链接问题修复完成"
}

# 修复 GLIBC 兼容性问题 - 重新实现
fix_glibc_compatibility() {
  echo "修复 GLIBC 兼容性问题..."
  
  local rootfs_lib="/data/data/com.winlator/files/rootfs/lib"
  local rootfs_usr_lib="/data/data/com.winlator/files/rootfs/usr/lib"
  
  # 首先，确保我们使用宿主系统的工具而不是rootfs中的工具
  export PATH="/usr/bin:/bin:/usr/sbin:/sbin:$PATH"
  
  echo "恢复原始 GLIBC 库文件..."
  
  # 如果存在备份目录，从备份恢复
  if [[ -d "${rootfs_usr_lib}.backup" ]]; then
    echo "从备份恢复关键库文件..."
    # 只恢复关键的 GLIBC 库
    local critical_libs=(
      "libc.so.6"
      "libpthread.so.0"
      "ld-linux-aarch64.so.1"
      "libm.so.6"
      "libdl.so.2"
      "librt.so.1"
    )
    
    for lib in "${critical_libs[@]}"; do
      if [[ -f "${rootfs_usr_lib}.backup/$lib" ]]; then
        cp "${rootfs_usr_lib}.backup/$lib" "$rootfs_lib/" 2>/dev/null || true
        echo "✅ 恢复 $lib"
      fi
    done
  fi
  
  # 确保关键库文件存在
  local critical_libs=(
    "libc.so.6"
    "libpthread.so.0" 
    "ld-linux-aarch64.so.1"
    "libm.so.6"
    "libdl.so.2"
    "librt.so.1"
  )
  
  for lib in "${critical_libs[@]}"; do
    if [[ ! -f "$rootfs_lib/$lib" ]]; then
      echo "⚠️ 缺少关键库: $lib"
      # 尝试从宿主系统复制兼容版本
      if [[ -f "/lib/aarch64-linux-gnu/$lib" ]]; then
        cp "/lib/aarch64-linux-gnu/$lib" "$rootfs_lib/"
        echo "✅ 从系统复制: $lib"
      elif [[ -f "/usr/lib/aarch64-linux-gnu/$lib" ]]; then
        cp "/usr/lib/aarch64-linux-gnu/$lib" "$rootfs_lib/"
        echo "✅ 从系统复制: $lib"
      fi
    fi
  done
  
  # 修复符号链接
  echo "修复库文件符号链接..."
  
  # libc.so.6
  local libc_target=$(find "$rootfs_lib" -name "libc-*.so" -type f | head -1)
  if [[ -n "$libc_target" ]] && [[ -f "$rootfs_lib/libc.so.6" ]]; then
    if [[ ! -L "$rootfs_lib/libc.so.6" ]] || [[ "$(readlink "$rootfs_lib/libc.so.6")" != "$(basename "$libc_target")" ]]; then
      rm -f "$rootfs_lib/libc.so.6"
      ln -sf "$(basename "$libc_target")" "$rootfs_lib/libc.so.6"
      echo "✅ 修复 libc.so.6 符号链接"
    fi
  fi
  
  # libpthread.so.0
  local pthread_target=$(find "$rootfs_lib" -name "libpthread-*.so" -type f | head -1)
  if [[ -n "$pthread_target" ]] && [[ -f "$rootfs_lib/libpthread.so.0" ]]; then
    if [[ ! -L "$rootfs_lib/libpthread.so.0" ]] || [[ "$(readlink "$rootfs_lib/libpthread.so.0")" != "$(basename "$pthread_target")" ]]; then
      rm -f "$rootfs_lib/libpthread.so.0"
      ln -sf "$(basename "$pthread_target")" "$rootfs_lib/libpthread.so.0"
      echo "✅ 修复 libpthread.so.0 符号链接"
    fi
  fi
  
  echo "✅ GLIBC 兼容性修复完成"
}

# 修改 patchelf_fix 函数，完全避免修改系统库
patchelf_fix() {
  LD_RPATH="/data/data/com.winlator/files/rootfs/lib"
  LD_FILE="$LD_RPATH/ld-linux-aarch64.so.1"
  
  echo "开始修补 ELF 文件..."
  
  # 创建关键系统库列表
  local system_libs=(
    "libc.so.6"
    "libpthread.so.0"
    "ld-linux-aarch64.so.1"
    "libm.so.6"
    "libdl.so.2"
    "librt.so.1"
    "libgcc_s.so.1"
    "libstdc++.so.6"
  )
  
  find /data/data/com.winlator/files/rootfs -type f -executable | while read -r elf_file; do
    if [[ -f "$elf_file" && -w "$elf_file" ]]; then
      # 跳过系统库目录
      if [[ "$elf_file" == *"/lib/"* ]] && [[ "$(basename "$elf_file")" =~ ^lib.*\.so(\.[0-9]+)*$ ]]; then
        echo "跳过系统库: $elf_file"
        continue
      fi
      
      # 检查是否是关键系统库
      local skip_file=0
      for lib in "${system_libs[@]}"; do
        if [[ "$elf_file" == *"$lib" ]]; then
          skip_file=1
          break
        fi
      done
      
      if [[ $skip_file -eq 1 ]]; then
        echo "跳过关键系统库: $elf_file"
        continue
      fi
      
      echo "修补: $elf_file"
      # 设置解释器
      patchelf --set-interpreter "$LD_FILE" "$elf_file" 2>/dev/null || true
      # 设置 rpath
      patchelf --set-rpath "$LD_RPATH" "$elf_file" 2>/dev/null || true
    fi
  done
  echo "ELF 文件修补完成"
}

# 修复库链接 - 简化版本
fix_library_links() {
  echo "修复库文件链接..."
  
  local rootfs_lib="/data/data/com.winlator/files/rootfs/lib"
  local rootfs_usr_lib="/data/data/com.winlator/files/rootfs/usr/lib"
  
  # 确保使用宿主系统的工具
  export PATH="/usr/bin:/bin:/usr/sbin:/sbin:$PATH"
  
  # 修复符号链接问题
  fix_symlink_issues
  
  # 如果 /usr/lib 不存在，创建符号链接
  if [[ ! -e "$rootfs_usr_lib" ]]; then
    echo "创建 /usr/lib 符号链接..."
    mkdir -p "$(dirname "$rootfs_usr_lib")"
    ln -sf "../lib" "$rootfs_usr_lib"
    echo "✅ 创建 /usr/lib -> /lib 符号链接"
  fi
  
  echo "✅ 库文件链接修复完成"
}

# 修改构建配置，确保库文件安装到正确位置
fix_build_install_paths() {
  echo "修复构建安装路径..."
  
  # 确保所有构建都安装到正确的前缀
  export DESTDIR="/data/data/com.winlator/files/rootfs"
  export PREFIX="/usr"
  
  # 设置环境变量确保库文件安装到正确位置
  export PKG_CONFIG_PATH="/data/data/com.winlator/files/rootfs/usr/lib/pkgconfig:/data/data/com.winlator/files/rootfs/lib/pkgconfig:$PKG_CONFIG_PATH"
  export LD_LIBRARY_PATH="/data/data/com.winlator/files/rootfs/usr/lib:/data/data/com.winlator/files/rootfs/lib:$LD_LIBRARY_PATH"
  
  echo "✅ 构建安装路径修复完成"
}

# 在构建完成后移动库文件到正确位置
move_built_libraries() {
  echo "移动构建的库文件到正确位置..."
  
  local rootfs_lib="/data/data/com.winlator/files/rootfs/lib"
  local rootfs_usr_lib="/data/data/com.winlator/files/rootfs/usr/lib"
  
  # 确保使用宿主系统的工具
  export PATH="/usr/bin:/bin:/usr/sbin:/sbin:$PATH"
  
  # 移动新构建的库文件从 /usr/lib 到 /lib
  if [[ -d "$rootfs_usr_lib" ]]; then
    echo "移动新构建的库文件..."
    
    # 移动 MangoHud 库
    if [[ -d "$rootfs_usr_lib/mangohud" ]]; then
      echo "移动 MangoHud 库..."
      mkdir -p "$rootfs_lib/mangohud"
      cp -r "$rootfs_usr_lib/mangohud"/* "$rootfs_lib/mangohud"/ 2>/dev/null || true
      # 移除原始目录避免符号链接问题
      rm -rf "$rootfs_usr_lib/mangohud"
    fi
    
    # 移动 gstreamer 库
    if [[ -d "$rootfs_usr_lib/gstreamer-1.0" ]]; then
      mkdir -p "$rootfs_lib/gstreamer-1.0"
      cp -r "$rootfs_usr_lib/gstreamer-1.0"/* "$rootfs_lib/gstreamer-1.0"/ 2>/dev/null || true
    fi
    
    # 移动其他库文件
    find "$rootfs_usr_lib" -maxdepth 1 -name "*.so*" -type f | while read -r lib_file; do
      local lib_name=$(basename "$lib_file")
      if [[ ! -f "$rootfs_lib/$lib_name" ]]; then
        cp "$lib_file" "$rootfs_lib/" 2>/dev/null || true
      fi
    done
    
    echo "✅ 库文件移动完成"
  fi
}

# 清理备份目录
cleanup_backups() {
  echo "清理备份目录..."
  
  local rootfs_usr_lib="/data/data/com.winlator/files/rootfs/usr/lib"
  
  if [[ -d "${rootfs_usr_lib}.backup" ]]; then
    # 检查是否还有文件在备份目录中需要保留
    if [[ $(find "${rootfs_usr_lib}.backup" -type f | wc -l) -eq 0 ]]; then
      rm -rf "${rootfs_usr_lib}.backup"
      echo "✅ 清理备份目录完成"
    else
      echo "⚠️ 备份目录中还有文件，保留备份"
    fi
  fi
}

# 修复 MangoHud 安装问题
fix_mangohud_installation() {
  echo "修复 MangoHud 安装问题..."
  
  local rootfs_lib="/data/data/com.winlator/files/rootfs/lib"
  local rootfs_usr_lib="/data/data/com.winlator/files/rootfs/usr/lib"
  
  # 确保使用宿主系统的工具
  export PATH="/usr/bin:/bin:/usr/sbin:/sbin:$PATH"
  
  # 检查 MangoHud 是否安装成功
  if [[ ! -f "$rootfs_lib/libMangoHud.so" ]] && [[ -f "$rootfs_usr_lib/libMangoHud.so" ]]; then
    echo "移动 MangoHud 库文件..."
    cp "$rootfs_usr_lib/libMangoHud.so" "$rootfs_lib/" 2>/dev/null || true
  fi
  
  # 修复 MangoHud 目录
  if [[ -d "$rootfs_usr_lib/mangohud" ]]; then
    echo "修复 MangoHud 插件目录..."
    mkdir -p "$rootfs_lib/mangohud"
    cp -r "$rootfs_usr_lib/mangohud"/* "$rootfs_lib/mangohud"/ 2>/dev/null || true
    rm -rf "$rootfs_usr_lib/mangohud"
  fi
  
  # 创建必要的符号链接
  if [[ ! -L "$rootfs_usr_lib/mangohud" ]] && [[ -d "$rootfs_lib/mangohud" ]]; then
    ln -sf "../lib/mangohud" "$rootfs_usr_lib/mangohud"
  fi
  
  echo "✅ MangoHud 安装修复完成"
}

# 修复 MangoHud 构建配置
fix_mangohud_build() {
  echo "修复 MangoHud 构建配置..."
  
  cd /tmp/MangoHud-src
  
  # 清理可能的构建残留
  rm -rf builddir 2>/dev/null || true
  
  # 使用更简化的配置
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
    -Dtests=disabled \
    -Duse_system_spdlog=disabled \
    -Duse_system_imgui=disabled || {
    echo "❌ MangoHud Meson 配置失败"
    return 1
  }
  
  if [[ ! -d builddir ]]; then
    echo "❌ builddir 未创建"
    return 1
  fi
  
  if ! meson compile -C builddir; then
    echo "❌ MangoHud 编译失败"
    return 1
  fi
  
  # 安装前确保目标目录存在
  mkdir -p "/data/data/com.winlator/files/rootfs/usr/lib"
  mkdir -p "/data/data/com.winlator/files/rootfs/lib"
  
  meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir
  echo "✅ MangoHud 构建完成"
}

# 修复 libxkbcommon 构建配置
fix_libxkbcommon_build() {
  echo "修复 libxkbcommon 构建配置..."
  cd /tmp/libxkbcommon-src
  
  # 使用正确的 Meson 配置选项
  meson setup builddir \
    -Denable-xkbregistry=false \
    -Denable-bash-completion=false \
    -Denable-docs=false \
    --prefix=/usr \
    --libdir=lib \
    --buildtype=release || {
      echo "❌ libxkbcommon Meson 配置失败"
      return 1
    }
  
  if [[ ! -d builddir ]]; then
    echo "❌ builddir 未创建"
    return 1
  fi
  
  if ! meson compile -C builddir; then
    echo "❌ libxkbcommon 编译失败"
    return 1
  fi
  
  meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir
  echo "✅ libxkbcommon 构建完成"
}

# 修复 GStreamer 构建配置
fix_gstreamer_build() {
  echo "修复 GStreamer 构建配置..."
  cd /tmp/gst-src
  
  # 使用正确的 Meson 配置选项
  meson setup builddir \
    --buildtype=release \
    --strip \
    -Dgst-full-target-type=shared_library \
    -Dintrospection=disabled \
    -Dgst-full-libraries=app,video,player \
    -Dprefix=/usr \
    -Dlibdir=lib \
    -Dauto_features=disabled \
    -Dgst-plugins-base:app=enabled \
    -Dgst-plugins-base:video=enabled \
    -Dgst-plugins-good:player=enabled || {
      echo "❌ GStreamer Meson 配置失败"
      return 1
    }
  
  if [[ ! -d builddir ]]; then
    echo "❌ builddir 未创建"
    return 1
  fi
  
  if ! meson compile -C builddir; then
    echo "❌ GStreamer 编译失败"
    return 1
  fi
  
  meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir
  echo "✅ GStreamer 构建完成"
}

# 初始化构建环境
init_build_environment() {
  echo "初始化构建环境..."
  
  # 确保使用宿主系统的工具
  export PATH="/usr/bin:/bin:/usr/sbin:/sbin:$PATH"
  
  # 设置构建环境变量
  export PKG_CONFIG_PATH="/data/data/com.winlator/files/rootfs/lib/pkgconfig:$PKG_CONFIG_PATH"
  export LD_LIBRARY_PATH="/data/data/com.winlator/files/rootfs/lib:$LD_LIBRARY_PATH"
  export C_INCLUDE_PATH="/data/data/com.winlator/files/rootfs/include:$C_INCLUDE_PATH"
  export CPLUS_INCLUDE_PATH="/data/data/com.winlator/files/rootfs/include:$CPLUS_INCLUDE_PATH"
  
  # 使用更简单的编译标志
  export CFLAGS="-mlittle-endian -mabi=lp64 -g -O2 -std=gnu11 -fPIC"
  export CXXFLAGS="$CFLAGS"
  export LDFLAGS="-L/data/data/com.winlator/files/rootfs/lib -Wl,-rpath,/data/data/com.winlator/files/rootfs/lib"
  
  echo "✅ 构建环境初始化完成"
}

# 主构建流程
main() {
  # 修改主构建流程
  if [[ ! -f /tmp/init.sh ]]; then
    exit 1
  else
    source /tmp/init.sh
    echo "gst=> $gstVer"
    echo "xz=> $xzVer"
    echo "libxkbcommon=> $libxkbcommonVer"
    echo "MangoHud=> $mangohudVer"
  fi

  # 安装依赖
  echo "安装构建依赖..."
  pacman -R --noconfirm libvorbis flac lame
  pacman -S --noconfirm --needed libdrm glm nlohmann-json libxcb python3 python-mako xorgproto wayland wayland-protocols libglvnd libxrandr libxinerama libxdamage libxfixes patchelf meson ninja

  mkdir -p /data/data/com.winlator/files/rootfs/
  cd /tmp
  if ! wget https://github.com/Waim908/rootfs-custom-winlator/releases/download/ori-b11.0/rootfs.tzst; then
    exit 1
  fi

  echo "解压 rootfs..."
  tar -xf rootfs.tzst -C /data/data/com.winlator/files/rootfs/
  tar -xf data.tar.xz -C /data/data/com.winlator/files/rootfs/
  tar -xf tzdata-*-.pkg.tar.xz -C /data/data/com.winlator/files/rootfs/

  # 安装 CA 证书
  cd /data/data/com.winlator/files/rootfs/etc
  mkdir -p ca-certificates
  if ! wget https://curl.haxx.se/ca/cacert.pem; then
    echo "⚠️ CA 证书下载失败，继续构建..."
  fi

  # 初始化构建环境
  init_build_environment

  # 首先修复符号链接和 GLIBC 问题
  echo "开始修复系统库问题..."
  fix_symlink_issues
  fix_glibc_compatibility
  fix_library_links
  fix_build_install_paths

  cd /tmp
  rm -rf /data/data/com.winlator/files/rootfs/lib/libgst*
  rm -rf /data/data/com.winlator/files/rootfs/lib/gstreamer-1.0

  # 克隆源码
  echo "克隆源代码..."
  if ! git clone -b "$xzVer" https://github.com/tukaani-project/xz.git xz-src; then
    exit 1
  fi

  if ! git clone -b "$gstVer" https://github.com/GStreamer/gstreamer.git gst-src; then
    exit 1
  fi

  # Build xz
  echo "Build and Compile xz(liblzma)"
  cd /tmp/xz-src
  ./autogen.sh
  mkdir build
  cd build
  if ! ../configure --prefix=/usr --libdir=/lib --datarootdir=/usr/share; then
    exit 1
  fi
  if ! make -j"$(nproc)"; then
    exit 1
  fi
  make DESTDIR="/data/data/com.winlator/files/rootfs" install

  # Build libxkbcommon
  echo "Build and Compile libxkbcommon"
  cd /tmp
  if ! git clone -b "$libxkbcommonVer" https://github.com/xkbcommon/libxkbcommon.git libxkbcommon-src; then
    exit 1
  fi

  # 使用修复的构建函数
  if ! fix_libxkbcommon_build; then
    echo "❌ libxkbcommon 构建失败"
    exit 1
  fi

  # Build MangoHud
  echo "Build and Compile MangoHud"
  cd /tmp
  if ! git clone -b "$mangohudVer" https://github.com/flightlessmango/MangoHud.git MangoHud-src; then
    exit 1
  fi

  # 使用修复的 MangoHud 构建
  if ! fix_mangohud_build; then
    echo "❌ MangoHud 构建失败"
    exit 1
  fi

  # 修复 MangoHud 安装
  fix_mangohud_installation

  # 移动构建的库文件到正确位置
  move_built_libraries

  # 验证安装
  if [[ -f "/data/data/com.winlator/files/rootfs/lib/libMangoHud.so" ]]; then
    echo "✅ MangoHud 库文件安装成功"
  else
    echo "❌ MangoHud 库文件安装失败"
  fi

  # 再次修复库链接
  fix_library_links

  # Build GStreamer
  cd /tmp/gst-src
  echo "Build and Compile gstreamer"
  if ! fix_gstreamer_build; then
    echo "❌ GStreamer 构建失败"
    exit 1
  fi

  # 再次移动 GStreamer 库文件
  move_built_libraries

  export date=$(TZ=Asia/Shanghai date '+%Y-%m-%d %H:%M:%S')

  # Package
  echo "Package"
  mkdir /tmp/output
  cd /data/data/com.winlator/files/rootfs/

  # 最终修复
  patchelf_fix
  fix_library_links

  # 清理备份目录
  cleanup_backups

  create_ver_txt

  if ! tar -I 'xz -T8' -cf /tmp/output/output-lite.tar.xz ./*; then
    exit 1
  fi

  cd /tmp
  tar -xf data.tar.xz -C /data/data/com.winlator/files/rootfs/
  tar -xf tzdata-2025b-1-aarch64.pkg.tar.xz -C /data/data/com.winlator/files/rootfs/

  cd /data/data/com.winlator/files/rootfs/
  create_ver_txt

  if ! tar -I 'xz -T8' -cf /tmp/output/output-full.tar.xz ./*; then
    exit 1
  fi

  # 重新创建 rootfs.tzst
  rm -rf /data/data/com.winlator/files/rootfs/*
  tar -xf rootfs.tzst -C /data/data/com.winlator/files/rootfs/
  tar -xf /tmp/output/output-full.tar.xz -C /data/data/com.winlator/files/rootfs/

  cd /data/data/com.winlator/files/rootfs/
  # 最终修补确保所有文件都正确
  patchelf_fix
  fix_library_links

  # 最终清理
  cleanup_backups

  create_ver_txt

  if ! tar -I 'zstd -T8' -cf /tmp/output/rootfs.tzst ./*; then
    exit 1
  fi

  echo "✅ 所有构建步骤完成"
}

# 运行主构建流程
main "$@"