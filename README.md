# Dpaper

A complete dynamic wallpaper management application for KDE Plasma, written entirely in C. Features a system tray interface, automatic directory management, and native KDE integration.

## ✨ Features

- **System Tray Interface** - Clean, minimal GUI with right-click menu
- **Automatic Directory Management** - Creates `~/.dp` directory on first run
- **Multi-Format Support** - JPG, PNG, JPEG, BMP, GIF image formats
- **Random Wallpaper Selection** - Smart algorithm for picking wallpapers
- **Per-Desktop Wallpapers** - Set different wallpapers on individual desktops
- **Set All Desktops** - Apply same wallpaper to all virtual desktops
- **Find Photos Dialog** - Browse and select images to add to collection
- **Remove Photos Dialog** - Browse and select images to remove from collection
- **Default Wallpapers** - 34 bundled Linux-themed wallpapers (auto-installed)
- **Default Wallpapers Toggle** - Enable/disable bundled wallpapers via tray menu
- **Auto-Rotate** - Configurable interval automatic changes
- **Configuration System** - JSON-based settings with GUI editor
- **Photo Management** - Track installed wallpapers in configuration
- **Boot Screen Wallpaper** - Automatically set wallpaper on system startup (set or random)
- **KDE Integration** - Native Plasma wallpaper setting using `plasma-apply-wallpaperimage` and D-Bus
- **Smart Packaging** - Complete distribution system with installer
- **Error-Only Notifications** - Clean UX with error dialogs only

## 📦 Installation

### Option 1: Quick Download (Recommended)

```bash
# Download and install in one command
curl -sL http://cyberboost.com/latest/dp.tar.gz | tar -xzf -
cd dpaper-1.0
./install.sh
```

Or use the download script:
```bash
./down.sh
./install.sh
```

The installer will:
- Try to compile from source if dependencies are available
- Fall back to pre-compiled binary if compilation fails
- Install the application and desktop integration
- Create the `~/.dp` wallpaper directory

### Option 2: From Source (Development)

1. Install dependencies:
   ```bash
   sudo apt install libgtk-3-dev libayatana-appindicator3-dev
   ```

2. Clone and build:
   ```bash
   git clone <repository-url>
   cd dpaper
   ./build.sh clean && ./build.sh
   ```

3. Install system-wide:
   ```bash
   cd dp && sudo make install
   ```

### Option 3: Snap Package (Future)

```bash
snap install dpaper
```

## 🚀 Usage

1. **Add Wallpapers**: Place images in `~/.dp/` directory (auto-created)
2. **Run Application**: Execute `dpaper`, `dp` (short alias), or find it in your applications menu
3. **Access Features**: Right-click the system tray icon to open the menu:
   - **Set Random (Current Desktop)** - Change wallpaper on active desktop
   - **Set Random (All Desktops)** - Change wallpaper on all desktops
   - **Set Selected (Current Desktop)** - Choose a specific wallpaper for current desktop
   - **Set Selected (All Desktops)** - Choose a specific wallpaper for all desktops
   - **Find Photos** - Browse and select images to add to collection
   - **Remove Photos** - Browse and select images to remove from collection
   - **Start Auto-Rotate** - Automatic changes every 5 minutes
   - **Stop Auto-Rotate** - Stop automatic changes
   - **Boot Screen Enabled** - Enable/disable automatic wallpaper on system startup
   - **Set Boot Screen (Random)** - Use random wallpaper on system startup
   - **Set Boot Screen (Selected)** - Choose specific wallpaper for system startup
   - **Use Default Wallpapers** - Toggle bundled Linux-themed wallpapers (34 images)
   - **Configure** - Open settings dialog to customize behavior
   - **About** - Show application information
   - **Quit** - Exit the application

## ⚙️ Configuration

The application stores settings in `~/.dp/config.json`:

```json
{
  "wallpaper_directory": "/home/user/.dp",
  "supported_formats": ["jpg", "jpeg", "png", "bmp", "gif"],
  "installed_photos": ["wallpaper1.jpg", "wallpaper2.png"],
  "auto_rotate_interval": 300,
  "auto_rotate_enabled": false,
  "use_default_wallpapers": true,
  "boot_screen_enabled": false,
  "boot_screen_image": "",
  "last_desktop_index": 0
}
```

**Configuration Dialog** (accessible via tray menu):
- **Wallpaper Directory**: Change the folder where wallpapers are stored
- **Auto-Rotate Interval**: Set time between automatic wallpaper changes (30-3600 seconds)
- **Supported Formats**: View image formats the application recognizes
- **Installed Photos**: See how many wallpapers are currently managed

**Default Wallpapers**:
- 34 bundled Linux-themed wallpapers included in `/opt/dp/dp/data/wallpaper/`
- Automatically installed to `~/.dp/` when "Use Default Wallpapers" is enabled
- Toggle via checkbox in tray menu
- Can be combined with user-added photos

**Boot Screen Settings**:
- **Boot Screen Enabled**: Checkbox to enable/disable automatic wallpaper setting on system startup
- **Boot Screen Image**: Path to specific wallpaper image (leave empty for random selection)
- Toggle via checkbox in tray menu, set mode via "Set Boot Screen" submenu options

Settings are automatically saved when you exit the application.

## 🏗️ Development

### Building
```bash
./build.sh clean && ./build.sh
```

### Debug Build
```bash
./build.sh debug
```

### Testing
```bash
# Run (multiple options)
./dpaper
# or
dp  # short alias
```

### Packaging for Distribution
```bash
# Create distribution package
./pkg/pkg.sh

# Result: pkg/kde-dp-1.0.tar.gz with smart installer
```

## 🔧 Technical Details

- **Language**: Pure C (C99 standard)
- **GUI**: GTK 3.0 with Ayatana AppIndicator
- **KDE Integration**: Direct `kwriteconfig5` and `qdbus` calls
- **Memory**: Manual management with proper cleanup
- **Build**: GCC with `-Wall -Wextra -Wno-deprecated-declarations`
- **Size**: ~17KB compiled binary

## 📁 Directory Structure

```
dpaper/
├── README.md              # Main user documentation
├── AGENTS.md              # Developer/AI agent guide
├── build.sh               # Build entry point script
├── docs/                  # Additional documentation
│   ├── PROJECT-COMPLETION.md
│   ├── FINAL-SUMMARY.md
│   ├── INSTALL.md
│   └── LICENSE
├── dp/                    # Main application code
│   ├── src/main.c         # Complete application implementation
│   ├── include/           # Header files for future modules
│   ├── data/              # Desktop integration and icons
│   ├── snap/              # Snap package definition
│   ├── Makefile           # Build system
│   └── dpaper     # Compiled executable
└── pkg/                   # Packaging and distribution
    ├── pkg.sh             # Package creation script
    ├── down.sh            # Download script
    └── kde-dp-1.0.tar.gz  # Generated packages
```

## 🐛 Known Limitations

- **Deprecation Warning**: `libayatana-appindicator` shows runtime warning (functionality unaffected)
- **KDE Plasma Required**: Designed specifically for KDE Plasma environment
- **Single Monitor Focus**: Current version optimized for single-monitor setups

## 🎯 Roadmap

- **✅ Configuration System** - JSON-based settings persistence
- **✅ Default Wallpapers** - 34 bundled wallpapers with toggle option
- **✅ Photo Management** - Add/remove photos with config updates
- **✅ Boot Screen Wallpaper** - Automatic wallpaper setting on system startup
- **Time-Based Selection** - Different wallpapers for different times
- **Application Triggers** - Wallpapers based on running applications
- **Multi-Monitor Support** - Individual wallpapers per monitor
- **Snap Distribution** - Complete snap packaging
- **Debian/RPM Packages** - .deb and .rpm package generation

## 📄 License

MIT License - Free for personal and commercial use.

---

**Built with ❤️ for KDE Plasma users who want more control over their desktop experience.**