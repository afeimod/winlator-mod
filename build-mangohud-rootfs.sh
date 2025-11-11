#!/bin/bash

# Function to patch ELF files
patchelf_fix() {
  local dir="$1"
  local interpreter="$2"
  
  echo "Patching ELF files in $dir..."
  find "$dir" -type f -exec file {} \; | grep -E ":.*ELF" | cut -d: -f1 | while read -r elf_file; do
    echo "  Patching $elf_file..."
    patchelf --set-interpreter "$interpreter" "$elf_file" 2>/dev/null || echo "    Warning: Failed to patch $elf_file"
  done
}

# Function to create version info
create_version_info() {
  local dir="$1"
  cat > "$dir/version-info.txt" << EOF
Build Date: $(TZ=Asia/Shanghai date '+%Y-%m-%d %H:%M:%S')
MangoHud Version: $mangohudVer
Commit: $mangohudCommit
Custom Tag: $customTag
Repository: https://github.com/Waim908/rootfs-custom-winlator
EOF
}

# Load environment variables
if [[ ! -f /tmp/init.sh ]]; then
  echo "Error: init.sh not found"
  exit 1
else
  source /tmp/init.sh
  echo "=== Build Configuration ==="
  echo "MangoHud Version: $mangohudVer"
  echo "Commit: $mangohudCommit"
  echo "Custom Tag: $customTag"
  echo "==========================="
fi

# Set installation directory
INSTALL_PREFIX="/opt/mangohud"
echo "Installation prefix: $INSTALL_PREFIX"
mkdir -p "$INSTALL_PREFIX"

# Install necessary build tools
echo "Installing build tools..."
pacman -Syu --noconfirm
pacman -S --noconfirm --needed wget ca-certificates git

cd /tmp

# Clone MangoHud source
if [[ -n "$mangohudCommit" && "$mangohudCommit" != "" ]]; then
  echo "Using specific commit: $mangohudCommit"
  if ! git clone https://github.com/flightlessmango/MangoHud.git mangohud-src; then
    echo "Error: Failed to clone MangoHud repository"
    exit 1
  fi
  cd mangohud-src
  git checkout "$mangohudCommit"
  cd /tmp
else
  echo "Using version tag: $mangohudVer"
  if ! git clone -b "$mangohudVer" https://github.com/flightlessmango/MangoHud.git mangohud-src; then
    echo "Error: Failed to clone MangoHud repository"
    exit 1
  fi
fi

# Build MangoHud
echo "=== Building MangoHud ==="
cd /tmp/mangohud-src

# Check build tools
echo "Checking build tools..."
for tool in meson ninja glslangValidator; do
    if command -v "$tool" &> /dev/null; then
        echo "✓ Found $tool: $(command -v $tool)"
    else
        echo "✗ Error: $tool not found"
        exit 1
    fi
done

# Configure build
echo "Configuring MangoHud build..."
meson setup build \
  --prefix="$INSTALL_PREFIX" \
  --libdir=lib \
  --buildtype=release \
  -Dwith_x11=enabled \
  -Dwith_wayland=disabled \
  -Dwith_xnvctrl=disabled \
  -Dwith_dbus=enabled \
  -Dmangoplot=enabled \
  -Dmangoapp=false \
  -Dmangohudctl=false \
  -Dtests=disabled

if [[ $? -ne 0 ]]; then
    echo "Error: Meson configuration failed"
    if [[ -f build/meson-logs/meson-log.txt ]]; then
        echo "=== Meson Log ==="
        cat build/meson-logs/meson-log.txt
        echo "================="
    fi
    exit 1
fi

if [[ ! -d build ]]; then
  echo "Error: Build directory not created"
  exit 1
fi

# Compile
echo "Compiling MangoHud..."
if ninja -C build -j$(nproc); then
    echo "✓ Compilation successful"
else
    echo "Multi-threaded compilation failed, trying single-threaded..."
    if ninja -C build -j1; then
        echo "✓ Single-threaded compilation successful"
    else
        echo "✗ Compilation failed"
        exit 1
    fi
fi

# Install
echo "Installing MangoHud..."
if ninja -C build install; then
    echo "✓ Installation successful"
else
    echo "✗ Installation failed"
    exit 1
fi

# Verify installation
echo "Verifying installation..."
if [[ -f "$INSTALL_PREFIX/lib/mangohud/libMangoHud.so" ]]; then
    echo "✓ MangoHud library installed"
else
    echo "✗ MangoHud library not found"
    echo "Searching for MangoHud files:"
    find "$INSTALL_PREFIX" -name "*mangohud*" -o -name "*MangoHud*" | head -10
    exit 1
fi

# Fix $LIB variable in startup script
echo "Fixing startup script..."
if [[ -f "$INSTALL_PREFIX/bin/mangohud" ]]; then
    sed -i 's|/\\$LIB|/lib|g' "$INSTALL_PREFIX/bin/mangohud"
    echo "✓ Startup script fixed"
else
    echo "Warning: mangohud binary not found"
fi

# Patch ELF files for Winlator compatibility
echo "Patching ELF files for Winlator..."
WINLATOR_INTERPRETER="/data/data/com.winlator/files/rootfs/lib/ld-linux-aarch64.so.1"
patchelf_fix "$INSTALL_PREFIX" "$WINLATOR_INTERPRETER"

# Create MangoHud configuration
echo "Creating MangoHud configuration..."
mkdir -p "$INSTALL_PREFIX/etc/mangohud/"
cat > "$INSTALL_PREFIX/etc/mangohud/MangoHud.conf" << 'EOF'
# MangoHud configuration file
# System-wide configuration for Winlator

# Basic display options
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
echo "✓ Configuration created"

# Create version info
create_version_info "$INSTALL_PREFIX"

# Package MangoHud build artifacts
echo "=== Packaging Artifacts ==="
mkdir -p /tmp/output
cd /opt

# Create standalone package
STANDALONE_TAR="mangohud-$mangohudVer-aarch64.tar.gz"
echo "Creating standalone package: $STANDALONE_TAR"
tar -czvf "/tmp/output/$STANDALONE_TAR" mangohud/
echo "✓ Standalone package created"

# Create Winlator integration package
echo "Creating Winlator integration package..."
mkdir -p /tmp/winlator-integration
cd /tmp/winlator-integration

# Create Winlator directory structure
mkdir -p data/data/com.winlator/files/rootfs/usr/lib/
mkdir -p data/data/com.winlator/files/rootfs/usr/bin/
mkdir -p data/data/com.winlator/files/rootfs/etc/mangohud/

# Copy MangoHud files to Winlator structure
echo "Copying MangoHud files..."
cp -r "$INSTALL_PREFIX/lib/"* data/data/com.winlator/files/rootfs/usr/lib/
cp "$INSTALL_PREFIX/bin/mangohud" data/data/com.winlator/files/rootfs/usr/bin/ 2>/dev/null || echo "mangohud binary not found, skipping"
cp -r "$INSTALL_PREFIX/etc/mangohud/"* data/data/com.winlator/files/rootfs/etc/mangohud/

# Create version info for Winlator package
create_version_info data/data/com.winlator/files/rootfs/

# Package Winlator integration
WINLATOR_TAR="mangohud-winlator-$mangohudVer.tar.gz"
echo "Creating Winlator package: $WINLATOR_TAR"
tar -czvf "/tmp/output/$WINLATOR_TAR" data/
echo "✓ Winlator package created"

# Create installation script
echo "Creating installation script..."
cat > "/tmp/output/install-mangohud.sh" << 'EOF'
#!/bin/bash
echo "MangoHud Installer for Winlator"
echo "================================"
echo ""
echo "This script will install MangoHud to your Winlator installation."
echo ""
echo "Usage:"
echo "  ./install-mangohud.sh [WINLATOR_PATH]"
echo ""
echo "If WINLATOR_PATH is not provided, it will use the default path:"
echo "/data/data/com.winlator/files"
echo ""

WINLATOR_PATH="${1:-/data/data/com.winlator/files}"

if [ ! -d "$WINLATOR_PATH" ]; then
    echo "Error: Winlator directory not found: $WINLATOR_PATH"
    echo "Please provide the correct path to your Winlator installation."
    exit 1
fi

echo "Installing MangoHud to: $WINLATOR_PATH"
echo "Extracting files..."

# Extract the Winlator package
tar -xzf mangohud-winlator-*.tar.gz -C "$WINLATOR_PATH"

if [ $? -eq 0 ]; then
    echo "✓ MangoHud installed successfully!"
    echo ""
    echo "Usage examples:"
    echo "  mangohud wine game.exe"
    echo "  mangohud %command%"
    echo ""
    echo "Configuration files are located at:"
    echo "  $WINLATOR_PATH/rootfs/etc/mangohud/"
else
    echo "✗ Installation failed!"
    exit 1
fi
EOF

chmod +x "/tmp/output/install-mangohud.sh"
echo "✓ Installation script created"

# Final output
echo ""
echo "=== Build Complete ==="
echo "Artifacts created in /tmp/output/:"
ls -la /tmp/output/
echo ""
echo "Packages:"
echo "  - $STANDALONE_TAR (Standalone MangoHud)"
echo "  - $WINLATOR_TAR (Winlator integrated)"
echo "  - install-mangohud.sh (Installation script)"
echo ""
echo "To use with Winlator:"
echo "  1. Extract the Winlator package to your Winlator rootfs"
echo "  2. Run: mangohud wine yourapp.exe"
echo "========================="