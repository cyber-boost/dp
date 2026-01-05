#!/bin/bash

# dpaper Package Creator
# Creates a distributable package containing source, binary, and installation scripts

set -e

PACKAGE_NAME="dpaper"
VERSION="1.0"
ARCHIVE_NAME="${PACKAGE_NAME}-${VERSION}.tar.gz"

echo "Creating Dpaper package..."

# Create temporary directory structure
PKG_DIR="${PACKAGE_NAME}-${VERSION}"
mkdir -p "$PKG_DIR"

# Copy source files
echo "Copying source files..."
mkdir -p "$PKG_DIR/src"
cp ../dp/src/main.c "$PKG_DIR/src/"
cp -r ../dp/include "$PKG_DIR/"

# Copy build system
echo "Copying build files..."
cp ../dp/Makefile "$PKG_DIR/"
cp ../build.sh "$PKG_DIR/"
cp ../README.md "$PKG_DIR/"

# Copy compiled binary if it exists
if [ -f "../dp/dpaper" ]; then
    echo "Including pre-compiled binary..."
    cp ../dp/dpaper "$PKG_DIR/"
fi

# Copy data files
if [ -d "../dp/data" ]; then
    cp -r ../dp/data "$PKG_DIR/"
fi

# Create desktop file
echo "Creating desktop file..."
cat > "$PKG_DIR/dpaper.desktop" << 'EOF'
[Desktop Entry]
Version=1.0
Type=Application
Name=Dpaper
Comment=Dynamic wallpaper changer for KDE Plasma
Exec=dpaper
Icon=dpaper
Terminal=false
Categories=Utility;DesktopSettings;
StartupNotify=false
Keywords=wallpaper;desktop;background;kde;plasma;dynamic;
EOF

# Create a simple icon (SVG format for scalability)
echo "Creating application icon..."
cat > "$PKG_DIR/data/icons/c-dp-wallpaper.svg" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<svg width="48" height="48" version="1.1" viewBox="0 0 48 48" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <linearGradient id="bg" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" style="stop-color:#4a90e2;stop-opacity:1" />
      <stop offset="100%" style="stop-color:#357abd;stop-opacity:1" />
    </linearGradient>
  </defs>
  <rect width="48" height="48" rx="4" fill="url(#bg)"/>
  <rect x="8" y="8" width="32" height="32" rx="2" fill="#ffffff" opacity="0.9"/>
  <rect x="12" y="12" width="8" height="8" rx="1" fill="#4a90e2"/>
  <rect x="22" y="12" width="8" height="8" rx="1" fill="#e74c3c"/>
  <rect x="12" y="22" width="8" height="8" rx="1" fill="#f39c12"/>
  <rect x="22" y="22" width="8" height="8" rx="1" fill="#27ae60"/>
  <circle cx="36" cy="12" r="3" fill="#ffffff"/>
  <text x="24" y="42" text-anchor="middle" font-family="sans-serif" font-size="8" fill="#ffffff">DP</text>
</svg>
EOF

# Create install.sh script
echo "Creating install script..."
cat > "$PKG_DIR/install.sh" << 'EOF'
#!/bin/bash

# KDE Dynamic Wallpaper Installer
# Installs the application, trying to compile from source if possible

set -e

APP_NAME="dpaper"
INSTALL_DIR="/usr/local/bin"
DESKTOP_DIR="/usr/local/share/applications"
ICON_DIR="/usr/local/share/icons/hicolor/48x48/apps"

echo "Installing Dpaper..."

# Check if we can compile from source
CAN_COMPILE=false
if command -v gcc >/dev/null 2>&1 && pkg-config --exists gtk+-3.0 >/dev/null 2>&1; then
    echo "Dependencies found, attempting to compile from source..."
    if make clean && make; then
        echo "Compilation successful!"
        CAN_COMPILE=true
    else
        echo "Compilation failed, using pre-compiled binary..."
    fi
else
    echo "Build dependencies not found, using pre-compiled binary..."
fi

# Install binary
if [ "$CAN_COMPILE" = true ] && [ -f "$APP_NAME" ]; then
    echo "Installing compiled binary..."
    sudo cp "$APP_NAME" "$INSTALL_DIR/"
elif [ -f "$APP_NAME" ]; then
    echo "Installing pre-compiled binary..."
    sudo cp "$APP_NAME" "$INSTALL_DIR/"
else
    echo "ERROR: No binary found!"
    exit 1
fi

# Create symlink for short command
echo "Creating command alias 'dp'..."
sudo ln -sf "$INSTALL_DIR/$APP_NAME" "$INSTALL_DIR/dp"

# Install desktop file
echo "Installing desktop file..."
sudo mkdir -p "$DESKTOP_DIR"
sudo cp dpaper.desktop "$DESKTOP_DIR/"

# Install icon (if available)
if [ -f "data/icons/dpaper.svg" ]; then
    echo "Installing SVG icon..."
    sudo mkdir -p "$ICON_DIR"
    sudo cp "data/icons/dpaper.svg" "$ICON_DIR/dpaper.svg"
elif [ -f "data/icons/dpaper.png" ]; then
    echo "Installing PNG icon..."
    sudo mkdir -p "$ICON_DIR"
    sudo cp "data/icons/dpaper.png" "$ICON_DIR/dpaper.png"
fi

# Create default wallpaper directory for user
echo "Setting up default wallpaper directory..."
mkdir -p "$HOME/.dp"

# Update desktop database
if command -v update-desktop-database >/dev/null 2>&1; then
    sudo update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
fi

echo "Installation complete!"
echo ""
echo "To start the application:"
echo "  - Run: $APP_NAME"
echo "  - Or: dp (short command)"
echo "  - Or find it in your applications menu"
echo "  - Wallpaper images should be placed in: $HOME/.dp/"
echo ""
echo "Right-click the system tray icon for options."
EOF

chmod +x "$PKG_DIR/install.sh"

# Create README for the package
echo "Creating README..."
cat > "$PKG_DIR/README.md" << 'EOF'
# C Dynamic Wallpaper for KDE Plasma

A lightweight dynamic wallpaper changer for KDE Plasma desktop environment.

## Features

- System tray icon with menu
- Random wallpaper selection
- Auto-rotate functionality (5-minute intervals)
- Find Photos dialog to add wallpapers
- Per-desktop or all-desktops wallpaper setting
- Automatic ~/.dp directory creation

## Installation

Run the install script:
```bash
./install.sh
```

The installer will:
1. Try to compile from source if build dependencies are available
2. Fall back to pre-compiled binary if compilation fails
3. Install the application and desktop file
4. Create the default wallpaper directory

## Usage

1. Run `dpaper` or find it in your applications menu
2. Look for the wallpaper icon in your system tray
3. Right-click for options:
   - **Set Random (Current Desktop)**: Change wallpaper on active desktop
   - **Set Random (All Desktops)**: Change wallpaper on all desktops
   - **Find Photos**: Browse and select images to add to collection
   - **Start Auto-Rotate**: Automatic wallpaper changes every 5 minutes
   - **Stop Auto-Rotate**: Stop automatic changes

## Requirements

- KDE Plasma desktop
- GTK 3.0 libraries (for compilation)
- Ayatana AppIndicator libraries

## Files

- `~/.dp/`: Default wallpaper directory
- Supported formats: JPG, PNG, JPEG, BMP, GIF

## Uninstall

```bash
sudo rm /usr/local/bin/dpaper
sudo rm /usr/local/share/applications/dpaper.desktop
rm -rf ~/.dp/
```
EOF

# Create the tar.gz archive
echo "Creating archive: $ARCHIVE_NAME"
tar -czf "$ARCHIVE_NAME" "$PKG_DIR"

# Cleanup
rm -rf "$PKG_DIR"

echo "Package created successfully: $ARCHIVE_NAME"
echo ""
echo "To test installation:"
echo "  tar -xzf $ARCHIVE_NAME"
echo "  cd ${PACKAGE_NAME}-${VERSION}"
echo "  ./install.sh"