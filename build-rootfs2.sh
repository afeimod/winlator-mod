patchelf_fix() {
  LD_RPATH="/data/data/com.winlator/files/rootfs/lib"
  LD_FILE="$LD_RPATH/ld-linux-aarch64.so.1"
  
  echo "开始修补 ELF 文件..."
  find /data/data/com.winlator/files/rootfs -type f -exec file {} + | grep -E ":.*ELF" | cut -d: -f1 | while read -r elf_file; do
    if [[ -f "$elf_file" && -w "$elf_file" ]]; then
      echo "修补: $elf_file"
      # 设置解释器
      patchelf --set-interpreter "$LD_FILE" "$elf_file" 2>/dev/null || true
      # 设置 rpath
      patchelf --set-rpath "$LD_RPATH" "$elf_file" 2>/dev/null || true
    fi
  done
  echo "ELF 文件修补完成"
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

create_ver_txt () {
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
pacman -R --noconfirm libvorbis flac lame
pacman -S --noconfirm --needed libdrm glm nlohmann-json libxcb python3 python-mako xorgproto wayland wayland-protocols libglvnd libxrandr libxinerama libxdamage libxfixes

mkdir -p /data/data/com.winlator/files/rootfs/
cd /tmp
if ! wget https://github.com/Waim908/rootfs-custom-winlator/releases/download/ori-b11.0/rootfs.tzst; then
  exit 1
fi
tar -xf rootfs.tzst -C /data/data/com.winlator/files/rootfs/
tar -xf data.tar.xz -C /data/data/com.winlator/files/rootfs/
tar -xf tzdata-*-.pkg.tar.xz -C /data/data/com.winlator/files/rootfs/

# 安装 CA 证书
cd /data/data/com.winlator/files/rootfs/etc
mkdir -p ca-certificates
if ! wget https://curl.haxx.se/ca/cacert.pem; then
  exit 1
fi

cd /tmp
rm -rf /data/data/com.winlator/files/rootfs/lib/libgst*
rm -rf /data/data/com.winlator/files/rootfs/lib/gstreamer-1.0

# 克隆源码
if ! git clone -b $xzVer https://github.com/tukaani-project/xz.git xz-src; then
  exit 1
fi

if ! git clone -b $gstVer https://github.com/GStreamer/gstreamer.git gst-src; then
  exit 1
fi

# Build xz
echo "Build and Compile xz(liblzma)"
cd /tmp/xz-src
./autogen.sh
mkdir build
cd build
if ! ../configure -prefix=/data/data/com.winlator/files/rootfs/; then
  exit 1
fi
if ! make -j$(nproc); then
  exit 1
fi
make install

# Build libxkbcommon
echo "Build and Compile libxkbcommon"
cd /tmp
if ! git clone -b $libxkbcommonVer https://github.com/xkbcommon/libxkbcommon.git libxkbcommon-src; then
  exit 1
fi
cd libxkbcommon-src
meson setup builddir \
  -Denable-xkbregistry=false \
  -Denable-bash-completion=false \
  --prefix=/data/data/com.winlator/files/rootfs/ || exit 1
if [[ ! -d builddir ]]; then
  exit 1
fi
if ! meson compile -C builddir; then
  exit 1
fi
meson install -C builddir

# Build MangoHud
echo "Build and Compile MangoHud"
cd /tmp
if ! git clone -b $mangohudVer https://github.com/flightlessmango/MangoHud.git MangoHud-src; then
  exit 1
fi

# 应用补丁
apply_mangohud_patch "/tmp/MangoHud-src"

cd MangoHud-src
meson setup builddir \
  -Dbuildtype=release \
  -Dwith_x11=enabled \
  -Dwith_wayland=disabled \
  -Dwith_xnvctrl=disabled \
  -Dwith_dbus=enabled \
  -Dmangoplot=enabled \
  -Dmangoapp=false \
  -Dmangohudctl=false \
  -Dtests=disabled
  --prefix=/data/data/com.winlator/files/rootfs/ || exit 1

if [[ ! -d builddir ]]; then
  exit 1
fi
if ! meson compile -C builddir; then
  exit 1
fi
meson install -C builddir

# 修复 MangoHud 脚本
fix_mangohud_script

# Build GStreamer
cd /tmp/gst-src
echo "Build and Compile gstreamer"
meson setup builddir \
  --buildtype=release \
  --strip \
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
  -Dpackage-origin="[gstremaer-build] (https://github.com/Waim908/gstreamer-build)" \
  --prefix=/data/data/com.winlator/files/rootfs/ || exit 1

if [[ ! -d builddir ]]; then
  exit 1
fi
if ! meson compile -C builddir; then
  exit 1
fi
meson install -C builddir

export date=$(TZ=Asia/Shanghai date '+%Y-%m-%d %H:%M:%S')

# Package
echo "Package"
mkdir /tmp/output
cd /data/data/com.winlator/files/rootfs/

# 修补所有 ELF 文件
patchelf_fix
create_ver_txt

if ! tar -I 'xz -T8' -cf /tmp/output/output-lite.tar.xz *; then
  exit 1
fi

cd /tmp
tar -xf data.tar.xz -C /data/data/com.winlator/files/rootfs/
tar -xf tzdata-2025b-1-aarch64.pkg.tar.xz -C /data/data/com.winlator/files/rootfs/

cd /data/data/com.winlator/files/rootfs/
create_ver_txt

if ! tar -I 'xz -T8' -cf /tmp/output/output-full.tar.xz *; then
  exit 1
fi

# 重新创建 rootfs.tzst
rm -rf /data/data/com.winlator/files/rootfs/*
tar -xf rootfs.tzst -C /data/data/com.winlator/files/rootfs/
tar -xf /tmp/output/output-full.tar.xz -C /data/data/com.winlator/files/rootfs/

cd /data/data/com.winlator/files/rootfs/
# 再次修补确保所有文件都正确
patchelf_fix
create_ver_txt

if ! tar -I 'zstd -T8' -cf /tmp/output/rootfs.tzst *; then
  exit 1
fi