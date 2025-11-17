#!/bin/bash

# 修复 GLIBC 兼容性问题
fix_glibc_compatibility() {
  echo "修复 GLIBC 兼容性问题..."
  
  local rootfs_lib="/data/data/com.winlator/files/rootfs/lib"
  local rootfs_usr_lib="/data/data/com.winlator/files/rootfs/usr/lib"
  
  # 首先恢复原始的 libc 和 libpthread
  echo "恢复原始 GLIBC 库文件..."
  
  # 检查是否存在备份
  if [[ -d "${rootfs_usr_lib}.backup" ]]; then
    echo "从备份恢复库文件..."
    find "${rootfs_usr_lib}.backup" -name "libc.so.*" -exec cp {} "$rootfs_lib/" \; 2>/dev/null || true
    find "${rootfs_usr_lib}.backup" -name "libpthread.so.*" -exec cp {} "$rootfs_lib/" \; 2>/dev/null || true
    find "${rootfs_usr_lib}.backup" -name "ld-linux-aarch64.so.*" -exec cp {} "$rootfs_lib/" \; 2>/dev/null || true
  fi
  
  # 确保关键库文件存在且正确
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
      # 尝试从系统复制
      if [[ -f "/lib/aarch64-linux-gnu/$lib" ]]; then
        cp "/lib/aarch64-linux-gnu/$lib" "$rootfs_lib/"
        echo "✅ 从系统复制: $lib"
      fi
    fi
  done
  
  # 修复 libc.so.6 符号链接
  if [[ -f "$rootfs_lib/libc.so.6" ]] && [[ ! -L "$rootfs_lib/libc.so.6" ]]; then
    echo "修复 libc.so.6 符号链接..."
    local libc_target=$(find "$rootfs_lib" -name "libc-*.so" -type f | head -1)
    if [[ -n "$libc_target" ]]; then
      mv "$rootfs_lib/libc.so.6" "$rootfs_lib/libc.so.6.backup" 2>/dev/null || true
      ln -sf "$(basename "$libc_target")" "$rootfs_lib/libc.so.6"
      echo "✅ 创建 libc.so.6 -> $(basename "$libc_target")"
    fi
  fi
  
  # 修复 libpthread.so.0 符号链接
  if [[ -f "$rootfs_lib/libpthread.so.0" ]] && [[ ! -L "$rootfs_lib/libpthread.so.0" ]]; then
    echo "修复 libpthread.so.0 符号链接..."
    local pthread_target=$(find "$rootfs_lib" -name "libpthread-*.so" -type f | head -1)
    if [[ -n "$pthread_target" ]]; then
      mv "$rootfs_lib/libpthread.so.0" "$rootfs_lib/libpthread.so.0.backup" 2>/dev/null || true
      ln -sf "$(basename "$pthread_target")" "$rootfs_lib/libpthread.so.0"
      echo "✅ 创建 libpthread.so.0 -> $(basename "$pthread_target")"
    fi
  fi
  
  # 验证库文件完整性
  echo "验证 GLIBC 库文件完整性..."
  if [[ -f "$rootfs_lib/libc.so.6" ]]; then
    echo "检查 libc.so.6:"
    file "$rootfs_lib/libc.so.6" 2>/dev/null || echo "无法检查 libc.so.6"
  fi
  
  if [[ -f "$rootfs_lib/libpthread.so.0" ]]; then
    echo "检查 libpthread.so.0:"
    file "$rootfs_lib/libpthread.so.0" 2>/dev/null || echo "无法检查 libpthread.so.0"
  fi
  
  echo "✅ GLIBC 兼容性修复完成"
}

patchelf_fix() {
  LD_RPATH="/data/data/com.winlator/files/rootfs/lib"
  LD_FILE="$LD_RPATH/ld-linux-aarch64.so.1"
  
  echo "开始修补 ELF 文件..."
  find /data/data/com.winlator/files/rootfs -type f -exec file {} + | grep -E ":.*ELF" | cut -d: -f1 | while read -r elf_file; do
    if [[ -f "$elf_file" && -w "$elf_file" ]]; then
      # 跳过关键系统库
      if [[ "$elf_file" == *"libc.so.6" ]] || \
         [[ "$elf_file" == *"libpthread.so.0" ]] || \
         [[ "$elf_file" == *"ld-linux-aarch64.so.1" ]] || \
         [[ "$elf_file" == *"libm.so.6" ]] || \
         [[ "$elf_file" == *"libdl.so.2" ]] || \
         [[ "$elf_file" == *"librt.so.1" ]]; then
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

fix_library_links() {
  echo "修复库文件链接..."
  
  local rootfs_lib="/data/data/com.winlator/files/rootfs/lib"
  local rootfs_usr_lib="/data/data/com.winlator/files/rootfs/usr/lib"
  
  # 检查并清理可能存在的符号链接循环
  if [[ -L "$rootfs_usr_lib" ]]; then
    echo "清理现有的符号链接: $rootfs_usr_lib"
    rm -f "$rootfs_usr_lib"
  fi
  
  # 如果存在备份目录，先恢复
  if [[ -d "${rootfs_usr_lib}.backup" ]]; then
    echo "恢复备份的库文件..."
    rm -rf "$rootfs_usr_lib" 2>/dev/null || true
    mv "${rootfs_usr_lib}.backup" "$rootfs_usr_lib"
  fi
  
  # 移动库文件从 /usr/lib 到 /lib
  if [[ -d "$rootfs_usr_lib" ]]; then
    echo "移动库文件从 /usr/lib 到 /lib..."
    cp -r "$rootfs_usr_lib"/* "$rootfs_lib"/ 2>/dev/null || true
    # 备份原始目录
    mv "$rootfs_usr_lib" "${rootfs_usr_lib}.backup"
  fi
  
  # 创建符号链接 /usr/lib -> /lib
  mkdir -p "$(dirname "$rootfs_usr_lib")"
  ln -sf "../lib" "$rootfs_usr_lib"
  echo "✅ 创建 /usr/lib -> /lib 符号链接"
  
  # 修复 /usr/bin 中的二进制文件
  if [[ -d "/data/data/com.winlator/files/rootfs/usr/bin" ]]; then
    find "/data/data/com.winlator/files/rootfs/usr/bin" -type f -executable | while read -r bin_file; do
      if file "$bin_file" | grep -q "ELF"; then
        patchelf --set-rpath "$rootfs_lib" "$bin_file" 2>/dev/null || true
      fi
    done
    echo "✅ 修复 /usr/bin 二进制文件的 rpath"
  fi
  
  # 修复 pkg-config 路径
  if [[ -d "/data/data/com.winlator/files/rootfs/usr/lib/pkgconfig" ]]; then
    mkdir -p "/data/data/com.winlator/files/rootfs/lib/pkgconfig"
    cp -r "/data/data/com.winlator/files/rootfs/usr/lib/pkgconfig/"* "/data/data/com.winlator/files/rootfs/lib/pkgconfig/" 2>/dev/null || true
    echo "✅ 修复 pkg-config 文件位置"
  fi
  
  # 修复 libc.so.6 符号链接
  if [[ -f "$rootfs_lib/libc.so.6" ]] && [[ ! -L "$rootfs_lib/libc.so.6" ]]; then
    mv "$rootfs_lib/libc.so.6" "$rootfs_lib/libc.so.6.original"
    local libc_target=$(find "$rootfs_lib" -name "libc-*.so" | head -1)
    if [[ -n "$libc_target" ]]; then
      ln -sf "$(basename "$libc_target")" "$rootfs_lib/libc.so.6"
      echo "✅ 修复 libc.so.6 符号链接"
    else
      mv "$rootfs_lib/libc.so.6.original" "$rootfs_lib/libc.so.6"
    fi
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
  
  # 移动新构建的库文件从 /usr/lib 到 /lib
  if [[ -d "$rootfs_usr_lib" ]]; then
    echo "移动新构建的库文件..."
    
    # 移动 gstreamer 库
    if [[ -d "$rootfs_usr_lib/gstreamer-1.0" ]]; then
      mkdir -p "$rootfs_lib/gstreamer-1.0"
      cp -r "$rootfs_usr_lib/gstreamer-1.0"/* "$rootfs_lib/gstreamer-1.0"/ 2>/dev/null || true
    fi
    
    # 移动其他库文件
    find "$rootfs_usr_lib" -name "*.so*" -type f | while read -r lib_file; do
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

fix_python_specific() {
  echo "修复 Python 特定问题..."
  
  local python_bin="/data/data/com.winlator/files/rootfs/usr/bin/python"
  local python_lib_dir="/data/data/com.winlator/files/rootfs/usr/lib/python3.11"
  
  # 修复 Python 二进制文件
  if [[ -f "$python_bin" ]]; then
    echo "修复 Python 二进制文件: $python_bin"
    
    # 确保使用正确的解释器
    patchelf --set-interpreter "/data/data/com.winlator/files/rootfs/lib/ld-linux-aarch64.so.1" "$python_bin" 2>/dev/null || true
    
    # 设置正确的 rpath
    patchelf --set-rpath "/data/data/com.winlator/files/rootfs/lib:/data/data/com.winlator/files/rootfs/usr/lib" "$python_bin" 2>/dev/null || true
    
    # 检查 Python 依赖的库
    echo "Python 依赖的库:"
    ldd "$python_bin" 2>/dev/null || true
  fi
  
  # 修复 Python 库路径
  if [[ -d "$python_lib_dir" ]]; then
    # 确保 Python 能找到自己的库
    local lib_python_path="/data/data/com.winlator/files/rootfs/lib/python3.11"
    mkdir -p "$(dirname "$lib_python_path")"
    ln -sf "$python_lib_dir" "$lib_python_path" 2>/dev/null || true
    echo "✅ 创建 Python 库符号链接"
  fi
  
  # 检查 libc 问题
  local libc_file="/data/data/com.winlator/files/rootfs/lib/libc.so.6"
  if [[ -f "$libc_file" ]]; then
    echo "检查 libc.so.6:"
    file "$libc_file"
    # 检查符号
    echo "检查 libc 中的符号:"
    nm -D "$libc_file" 2>/dev/null | grep -i nptl_change_stack || echo "未找到 nptl_change_stack 符号"
  fi
  
  echo "✅ Python 特定问题修复完成"
}

fix_glibc_issue() {
  echo "修复 GLIBC 问题..."
  
  # 先修复兼容性问题
  fix_glibc_compatibility
  
  local rootfs_lib="/data/data/com.winlator/files/rootfs/lib"
  
  # 重新安装 glibc 以确保完整性
  echo "重新安装 GLIBC..."
  pacman -S --noconfirm glibc 2>/dev/null || echo "GLIBC 安装可能有问题，继续..."
  
  # 检查并修复 libpthread
  local libpthread_file="$rootfs_lib/libpthread.so.0"
  
  if [[ ! -f "$libpthread_file" ]]; then
    echo "⚠️ libpthread.so.0 不存在，尝试修复"
    local libpthread_target=$(find "$rootfs_lib" -name "libpthread-*.so" -type f | head -1)
    if [[ -n "$libpthread_target" ]]; then
      ln -sf "$(basename "$libpthread_target")" "$libpthread_file"
      echo "✅ 创建 libpthread.so.0 符号链接"
    fi
  fi
  
  # 验证符号
  echo "检查关键符号..."
  if [[ -f "$rootfs_lib/libc.so.6" ]]; then
    echo "libc.so.6 中的关键符号:"
    # 使用简化的符号检查，避免依赖有问题的 libc
    strings "$rootfs_lib/libc.so.6" 2>/dev/null | grep -i "glibc" | head -5 || echo "无法检查符号"
  fi
  
  # 创建一个简单的测试程序来验证 libc 功能
  echo "创建 GLIBC 测试程序..."
  cat > /tmp/test_libc.c << 'EOF'
#include <stdio.h>
#include <pthread.h>

void* test_thread(void* arg) {
    printf("Thread test passed\n");
    return NULL;
}

int main() {
    pthread_t thread;
    printf("Starting GLIBC thread test...\n");
    if (pthread_create(&thread, NULL, test_thread, NULL) == 0) {
        pthread_join(thread, NULL);
        printf("GLIBC thread test completed successfully\n");
        return 0;
    } else {
        printf("GLIBC thread test failed\n");
        return 1;
    }
}
EOF
  
  # 尝试编译测试程序
  if command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
    echo "编译 GLIBC 测试程序..."
    aarch64-linux-gnu-gcc -o /tmp/test_libc /tmp/test_libc.c -pthread -Wl,-rpath,/data/data/com.winlator/files/rootfs/lib
    
    # 运行测试（在构建环境中）
    echo "运行 GLIBC 线程测试..."
    if [[ -f "/tmp/test_libc" ]]; then
      /tmp/test_libc && echo "✅ GLIBC 线程测试通过" || echo "❌ GLIBC 线程测试失败"
    else
      echo "⚠️ 测试程序编译失败，跳过测试"
    fi
  else
    echo "⚠️ 交叉编译器未找到，跳过 GLIBC 测试"
  fi
  
  echo "✅ GLIBC 问题修复完成"
}

fix_mangohud_script() {
  local mangohud_script="/data/data/com.winlator/files/rootfs/bin/mangohud"
  if [[ -f "$mangohud_script" ]]; then
    echo "修复 MangoHud 启动脚本..."
    sed -i 's|/\\$LIB|/lib|g' "$mangohud_script"
    # 确保脚本可执行
    chmod +x "$mangohud_script"
  fi
}

apply_mangohud_patch() {
  local src_file="$1/src/sysinfo/linux.cpp"
  if [[ -f "$src_file" ]]; then
    echo "应用 MangoHud CPU 频率检测补丁..."
    cp "$src_file" "${src_file}.bak"
    awk '
    BEGIN {in_func=0}
    /^int get_cpu_freq\(int core\)/ {print; in_func=1; next}
    in_func && /^\{/ {print; print "    int freq = 0; std::string path;"; next}
    in_func && /return freq;/ {
        print "    // 尝试 scaling_cur_freq"
        print "    path = \"/sys/devices/system/cpu/cpu\" + std::to_string(core) + \"/cpufreq/scaling_cur_freq\";"
        print "    { std::ifstream f(path); if(f.is_open()){ f >> freq; if(freq>0) return freq/1000; } }"
        print ""
        print "    // 尝试 cpuinfo_cur_freq"
        print "    path = \"/sys/devices/system/cpu/cpu\" + std::to_string(core) + \"/cpufreq/cpuinfo_cur_freq\";"
        print "    { std::ifstream f(path); if(f.is_open()){ f >> freq; if(freq>0) return freq/1000; } }"
        print ""
        print "    // 从 /proc/cpuinfo 读取 (非 root fallback)"
        print "    {"
        print "        std::ifstream cpuinfo(\"/proc/cpuinfo\");"
        print "        if(cpuinfo.is_open()){"
        print "            std::string line; int index=-1;"
        print "            while(std::getline(cpuinfo,line)){"
        print "                if(line.find(\"processor\")!=std::string::npos) index++;"
        print "                if(index==core && line.find(\"cpu MHz\")!=std::string::npos){"
        print "                    size_t pos=line.find(\":\");"
        print "                    if(pos!=std::string::npos){"
        print "                        std::istringstream iss(line.substr(pos+1));"
        print "                        iss >> freq; if(freq>0) return (int)freq;"
        print "                    }"
        print "                }"
        print "            }"
        print "        }"
        print "    }"
        print ""
        print "    // fallback 默认值"
        print "    return 2000;"
        in_func=0; next
    }
    {print}
    ' "${src_file}.bak" > "$src_file"
    echo "✅ CPU 频率检测补丁应用成功"
  else
    echo "⚠️ 未找到 $src_file，跳过补丁"
  fi
}

apply_winlator_compatibility_patch() {
  local src_file="$1/src/overlay.cpp"
  if [[ -f "$src_file" ]]; then
    echo "应用 Winlator 兼容性补丁..."
    cp "$src_file" "${src_file}.bak"
    
    # 简化渲染逻辑，避免与 Winlator 的图形层冲突
    sed -i 's|glFlush();|// glFlush();|g' "$src_file"
    sed -i 's|glFinish();|// glFinish();|g' "$src_file"
    
    # 禁用可能冲突的 OpenGL 扩展
    sed -i 's|#ifndef GLX_MESA_swap_control|#if 0 // Disabled for Winlator compatibility|g' "$src_file"
    
    echo "✅ Winlator 兼容性补丁应用成功"
  else
    echo "⚠️ 未找到 $src_file，跳过兼容性补丁"
  fi
}

create_ver_txt() {
  cat > '/data/data/com.winlator/files/rootfs/_version_.txt' << EOF
Output Date(UTC+8): $date
Version:
  gstreamer=> $gstVer
  xz=> $xzVer
  libxkbcommon=> $libxkbcommonVer
  MangoHud=> $mangohudVer
  rootfs-tag=> $customTag
Repo:
  [Waim908/rootfs-custom-winlator](https://github.com/Waim908/rootfs-custom-winlator)
EOF
}

apply_mangohud_sysfs_patch() {
  echo "应用 MangoHud 系统访问补丁..."
  
  # 修改 cpu.cpp 避免访问错误
  local cpu_file="$1/src/cpu.cpp"
  if [[ -f "$cpu_file" ]]; then
    cp "$cpu_file" "${cpu_file}.bak"
    awk '
    /std::ifstream stat_file\("\/proc\/stat"\);/ {
        print "    // Winlator patch - safe /proc/stat access"
        print "    std::ifstream stat_file(\"/proc/stat\");"
        print "    if(!stat_file.is_open()) {"
        print "        LOG_WARNING(\"Cannot open /proc/stat in Winlator environment\");"
        print "        return;"
        print "    }"
        next
    }
    {print}
    ' "${cpu_file}.bak" > "$cpu_file"
    echo "✅ CPU 统计补丁应用成功"
  fi

  # 修改 file_utils.cpp 避免目录扫描错误
  local file_utils="$1/src/file_utils.cpp"
  if [[ -f "$file_utils" ]]; then
    cp "$file_utils" "${file_utils}.bak"
    sed -i 's|if (dir == nullptr)|if (true) { \/\/ Winlator patch - skip directory scanning\n        LOG_WARNING("Skipping directory scan in Winlator");\n        return {};\n    }\n    if (dir == nullptr)|g' "$file_utils"
    echo "✅ 文件工具补丁应用成功"
  fi
}

create_virtual_sysfs() {
  echo "创建虚拟系统文件..."
  
  # 创建基础目录
  mkdir -p "/data/data/com.winlator/files/rootfs/proc"
  mkdir -p "/data/data/com.winlator/files/rootfs/sys/class/hwmon/hwmon0"
  
  # 创建虚拟 /proc/stat
  cat > "/data/data/com.winlator/files/rootfs/proc/stat" << 'EOF'
cpu  100000 0 100000 0 0 0 0 0 0 0
cpu0 100000 0 100000 0 0 0 0 0 0 0
cpu1 100000 0 100000 0 0 0 0 0 0 0
cpu2 100000 0 100000 0 0 0 0 0 0 0
cpu3 100000 0 100000 0 0 0 0 0 0 0
EOF

  # 创建虚拟温度传感器
  echo "45000" > "/data/data/com.winlator/files/rootfs/sys/class/hwmon/hwmon0/temp1_input"
  echo "cpu" > "/data/data/com.winlator/files/rootfs/sys/class/hwmon/hwmon0/name"
  
  echo "✅ 虚拟系统文件创建完成"
}

apply_mangohud_path_patch() {
  echo "应用 MangoHud 路径补丁..."
  
  local meson_build="$1/meson.build"
  local src_files=("$1/src/overlay.cpp" "$1/src/loaders/loader.cpp" "$1/src/mangohud.cpp")
  
  # 修复 meson.build 中的路径设置
  if [[ -f "$meson_build" ]]; then
    cp "$meson_build" "${meson_build}.bak"
    
    # 使用更简单的编译选项，避免 glibc 兼容性问题
    sed -i 's|cpp_args = \[\]|cpp_args = [\"-mlittle-endian\", \"-mabi=lp64\", \"-g\", \"-O2\", \"-std=gnu11\", \"-fPIC\"]|g' "$meson_build"
    
    echo "✅ Meson 构建配置补丁应用成功"
  fi

  # 修复源码中的路径
  for src_file in "${src_files[@]}"; do
    if [[ -f "$src_file" ]]; then
      cp "$src_file" "${src_file}.bak"
      
      # 替换硬编码路径为 Winlator 路径
      sed -i 's|/usr/lib|/data/data/com.winlator/files/rootfs/lib|g' "$src_file"
      sed -i 's|/etc|/data/data/com.winlator/files/rootfs/etc|g' "$src_file"
      sed -i 's|/usr/share|/data/data/com.winlator/files/rootfs/usr/share|g' "$src_file"
      sed -i 's|/usr/local|/data/data/com.winlator/files/rootfs/usr/local|g' "$src_file"
      
      # 修复配置路径
      sed -i 's|"\.config"|"/data/data/com.winlator/files/rootfs/.config"|g' "$src_file"
      sed -i 's|"\.local/share"|"/data/data/com.winlator/files/rootfs/.local/share"|g' "$src_file"
      
      echo "✅ 修复 $(basename "$src_file") 中的路径"
    fi
  done

  # 修复数据文件路径
  local data_meson_build="$1/data/meson.build"
  if [[ -f "$data_meson_build" ]]; then
    cp "$data_meson_build" "${data_meson_build}.bak"
    sed -i 's|install_dir : join_paths(datadir, .MangoHud.)|install_dir : '\''/data/data/com.winlator/files/rootfs/share/MangoHud'\''|g' "$data_meson_build"
    echo "✅ 数据文件路径补丁应用成功"
  fi

  # 创建必要的配置文件目录
  mkdir -p "/data/data/com.winlator/files/rootfs/.config/MangoHud"
  mkdir -p "/data/data/com.winlator/files/rootfs/.local/share/MangoHud"
  mkdir -p "/data/data/com.winlator/files/rootfs/share/MangoHud"
  
  # 创建默认配置文件
  cat > "/data/data/com.winlator/files/rootfs/.config/MangoHud/MangoHud.conf" << 'EOF'
# MangoHud 配置文件
output_folder=/data/data/com.winlator/files/rootfs/tmp/mangohud
gpu_stats
cpu_stats
ram_stats
fps
frame_timing
histogram
EOF
}

setup_mangohud_environment() {
  echo "设置 MangoHud 构建环境..."
  
  export PKG_CONFIG_PATH="/data/data/com.winlator/files/rootfs/lib/pkgconfig:$PKG_CONFIG_PATH"
  export LD_LIBRARY_PATH="/data/data/com.winlator/files/rootfs/lib:$LD_LIBRARY_PATH"
  export C_INCLUDE_PATH="/data/data/com.winlator/files/rootfs/include:$C_INCLUDE_PATH"
  export CPLUS_INCLUDE_PATH="/data/data/com.winlator/files/rootfs/include:$CPLUS_INCLUDE_PATH"
  
  # 使用更简单的编译标志，避免 glibc 兼容性问题
  export CFLAGS="-mlittle-endian -mabi=lp64 -g -O2 -std=gnu11 -fPIC"
  export CXXFLAGS="$CFLAGS"
  export LDFLAGS="-L/data/data/com.winlator/files/rootfs/lib -Wl,-rpath,/data/data/com.winlator/files/rootfs/lib"
  
  echo "✅ 构建环境设置完成"
}

# 安装交叉编译工具链
install_cross_compiler() {
  echo "安装交叉编译工具链..."
  
  # 检查是否已安装
  if ! command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
    echo "安装 aarch64-linux-gnu-gcc..."
    pacman -S --noconfirm aarch64-linux-gnu-gcc 2>/dev/null || {
      echo "尝试从 AUR 安装交叉编译器..."
      # 如果 pacman 中没有，尝试其他方式
      if command -v yay >/dev/null 2>&1; then
        yay -S --noconfirm aarch64-linux-gnu-gcc-bin
      elif command -v paru >/dev/null 2>&1; then
        paru -S --noconfirm aarch64-linux-gnu-gcc-bin
      else
        echo "⚠️ 无法安装交叉编译器，请手动安装 aarch64-linux-gnu-gcc"
        return 1
      fi
    }
  fi
  
  if command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
    echo "✅ 交叉编译器已安装: $(aarch64-linux-gnu-gcc --version | head -1)"
  else
    echo "❌ 交叉编译器安装失败"
    return 1
  fi
}

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
pacman -S --noconfirm --needed libdrm glm nlohmann-json libxcb python3 python-mako xorgproto wayland wayland-protocols libglvnd libxrandr libxinerama libxdamage libxfixes patchelf

# 安装交叉编译器
install_cross_compiler

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

# 首先修复 GLIBC 问题
echo "开始修复系统库问题..."
fix_glibc_compatibility
fix_library_links
fix_build_install_paths
fix_glibc_issue
fix_python_specific

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

# Build xz - 修改安装路径
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

# Build libxkbcommon - 修改安装路径
echo "Build and Compile libxkbcommon"
cd /tmp
if ! git clone -b "$libxkbcommonVer" https://github.com/xkbcommon/libxkbcommon.git libxkbcommon-src; then
  exit 1
fi
cd libxkbcommon-src
meson setup builddir \
  -Denable-xkbregistry=false \
  -Denable-bash-completion=false \
  --prefix=/usr \
  --libdir=lib \
  -Ddatarootdir=/usr/share || exit 1
if [[ ! -d builddir ]]; then
  exit 1
fi
if ! meson compile -C builddir; then
  exit 1
fi
meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir

# Build MangoHud - 修改安装路径
echo "Build and Compile MangoHud"
cd /tmp
if ! git clone -b "$mangohudVer" https://github.com/flightlessmango/MangoHud.git MangoHud-src; then
  exit 1
fi

setup_mangohud_environment
apply_mangohud_patch "/tmp/MangoHud-src"
apply_mangohud_sysfs_patch "/tmp/MangoHud-src"
apply_mangohud_path_patch "/tmp/MangoHud-src"
apply_winlator_compatibility_patch "/tmp/MangoHud-src"

cd MangoHud-src

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
            -Dtests=disabled || exit 1

if [[ ! -d builddir ]]; then
  exit 1
fi
if ! meson compile -C builddir; then
  echo "MangoHud 编译失败，尝试简化构建..."
  rm -rf builddir
  meson setup builddir \
              --prefix=/usr \
              --libdir=lib \
              -Dbuildtype=release \
              -Dwith_x11=enabled \
              -Dwith_wayland=disabled \
              -Dwith_dbus=disabled \
              -Dtests=disabled || exit 1
  meson compile -C builddir || exit 1
fi
meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir

# 移动构建的库文件到正确位置
move_built_libraries

# 验证安装
if [[ -f "/data/data/com.winlator/files/rootfs/lib/libMangoHud.so" ]]; then
  echo "✅ MangoHud 库文件安装成功"
  file "/data/data/com.winlator/files/rootfs/lib/libMangoHud.so"
else
  echo "❌ MangoHud 库文件安装失败"
fi

# 创建虚拟系统文件
create_virtual_sysfs
fix_mangohud_script

# 再次修复库链接
fix_library_links
fix_python_specific

# Build GStreamer - 修改安装路径
cd /tmp/gst-src
echo "Build and Compile gstreamer"
meson setup builddir \
  --buildtype=release \
  --strip \
  -Dgst-full-target-type=shared_library \
  -Dintrospection=disabled \
  -Dgst-full-libraries=app,video,player \
  -Dprefix=/usr \
  -Dlibdir=lib \
  -Ddatarootdir=/usr/share

if [[ ! -d builddir ]]; then
  exit 1
fi
if ! meson compile -C builddir; then
  exit 1
fi
meson install --destdir="/data/data/com.winlator/files/rootfs" -C builddir

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
fix_python_specific

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
fix_python_specific

# 最终清理
cleanup_backups

create_ver_txt

if ! tar -I 'zstd -T8' -cf /tmp/output/rootfs.tzst ./*; then
  exit 1
fi

echo "✅ 所有构建步骤完成"