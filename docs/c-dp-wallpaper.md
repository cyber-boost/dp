# Dpaper Application for Kubuntu

## Project Overview

A C-based application for dynamically changing Kubuntu desktop wallpapers with a system tray icon interface. The application will allow users to select image folders, set wallpapers based on various criteria, and configure scheduling options.

## Features

1. System tray icon with menu options
2. Folder selection dialog (defaulting to `/opt/.dp/`)
3. File filtering for image formats (jpg, png, etc.)
4. Wallpaper setting modes:
   - Set all monitors to same wallpaper
   - Set random wallpaper from folder
   - Set based on running applications
   - Set based on time of day
5. Configuration options:
   - Source photos folder
   - Refresh interval
   - Mode selection
6. Snap package distribution

## Technical Approach

### Language Choice: C

Despite C++ being more common for KDE applications, we'll use C to meet your 2026 learning goal while still achieving full functionality.

### Dependencies

- GTK 3.0: For GUI elements (system tray icon, dialogs)
- Ayatana AppIndicator: For system tray integration (replacement for deprecated GtkStatusIcon)
- GLib: For file operations and utilities
- D-Bus GLib bindings: For communicating with KDE Plasma

### Core Components

1. **System Tray Interface**
   - Use libayatana-appindicator for tray icon
   - Menu with options: Set Wallpaper, Configure, About, Quit
   - Left-click to set random wallpaper
   - Right-click for menu

2. **File Management**
   - Directory scanning with GDir
   - File filtering with GtkFileFilter
   - Image format detection
   - Configuration file handling

3. **Wallpaper Setting**
   - Use kwriteconfig5 to set wallpaper in KDE
   - Use qdbus to refresh Plasma shell
   - Support for multiple monitor configurations

4. **Configuration**
   - JSON or INI format config file
   - Store user preferences (folder path, mode, etc.)
   - Default configuration with sensible defaults

5. **Scheduling**
   - GTimer for periodic wallpaper changes
   - Time-based triggers
   - Application monitoring (optional advanced feature)

## Implementation Plan

### Phase 1: Basic System Tray Application
- Create minimal C application with system tray icon
- Implement basic menu with quit option
- Test appindicator integration

### Phase 2: File Selection and Management
- Implement folder selection dialog
- Add file filtering for image formats
- Create image scanning functionality
- Implement configuration file handling

### Phase 3: Wallpaper Setting Functionality
- Integrate with KDE wallpaper system
- Implement basic "set wallpaper" functionality
- Add support for random selection
- Test wallpaper refresh mechanism

### Phase 4: Advanced Features
- Add time-based scheduling
- Implement application-based wallpaper selection
- Add configuration dialog
- Create preferences management

### Phase 5: Polish and Distribution
- Add error handling and logging
- Create user documentation
- Package as Snap application
- Test installation and functionality

## Build and Dependencies

### Required Development Packages
```bash
sudo apt install libgtk-3-dev libayatana-appindicator3-dev libdbus-glib-1-dev
```

### Build System
```bash
gcc -o c-dp-wallpaper main.c $(pkg-config --cflags --libs gtk+-3.0 ayatana-appindicator3-0.1 dbus-glib-1)
```

### Runtime Dependencies
- kwriteconfig5 (part of kde-cli-tools)
- qdbus (part of qttools5-dev-tools)
- KDE Plasma environment

## File Structure
```
c-dp-wallpaper/
├── src/
│   ├── main.c              # Entry point and main application logic
│   ├── tray.c              # System tray implementation
│   ├── config.c            # Configuration handling
│   ├── wallpaper.c         # Wallpaper setting functionality
│   ├── filemanager.c       # File and directory operations
│   └── scheduler.c         # Timing and scheduling functions
├── include/
│   ├── tray.h
│   ├── config.h
│   ├── wallpaper.h
│   ├── filemanager.h
│   └── scheduler.h
├── data/
│   ├── config.json         # Default configuration
│   └── icons/              # Application icons
├── snap/
│   └── snapcraft.yaml      # Snap package definition
├── Makefile
├── README.md
└── LICENSE
```

## Key Technical Details

### Setting Wallpaper in KDE
```bash
# Set wallpaper
kwriteconfig5 --file kdeglobals --group General --key Wallpaper "/path/to/image.jpg"

# Refresh Plasma shell
qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript '...'
```

### System Tray Implementation
Use libayatana-appindicator which provides:
- `app_indicator_new()` to create indicator
- `app_indicator_set_status()` to show/hide
- `app_indicator_set_menu()` to attach menu

### Configuration File Format
JSON format for easy parsing in C:
```json
{
  "image_directory": "/opt/.dp/",
  "file_extensions": ["jpg", "png", "jpeg"],
  "mode": "random",
  "interval": 300,
  "time_based": {
    "morning": "morning.jpg",
    "afternoon": "afternoon.jpg",
    "evening": "evening.jpg",
    "night": "night.jpg"
  }
}
```

## Challenges and Considerations

1. **C Language Complexity**: Manual memory management, no built-in high-level data structures
2. **KDE Integration**: Working with KDE-specific tools from C requires careful process management
3. **System Tray Limitations**: Modern Linux desktops have restrictions on system tray icons
4. **Cross-Platform Compatibility**: Focused on KDE/Kubuntu, may need adjustments for other environments

## Timeline

- Phase 1: 1-2 weeks
- Phase 2: 2-3 weeks
- Phase 3: 2-3 weeks
- Phase 4: 3-4 weeks
- Phase 5: 1-2 weeks

Total estimated development time: 3-4 months for a fully functional application.