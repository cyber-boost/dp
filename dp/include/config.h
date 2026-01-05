#ifndef CONFIG_H
#define CONFIG_H

#include <glib.h>

// Configuration structure
typedef struct {
    char *wallpaper_directory;      // Main wallpaper directory
    GPtrArray *supported_formats;   // Array of supported file extensions
    GPtrArray *installed_photos;    // Array of installed photo filenames
    int auto_rotate_interval;       // Auto-rotate interval in seconds
    gboolean auto_rotate_enabled;   // Whether auto-rotate is enabled
    int last_desktop_index;         // Last used desktop index
    gboolean use_default_wallpapers; // Whether to use bundled default wallpapers
} Config;

// Configuration functions
Config* config_new(void);
void config_free(Config *config);
gboolean config_load(Config *config, const char *filename);
gboolean config_save(const Config *config, const char *filename);
void config_set_defaults(Config *config);

// Photo management
void config_add_photo(Config *config, const char *filename);
void config_remove_photo(Config *config, const char *filename);
gboolean config_has_photo(const Config *config, const char *filename);
GPtrArray* config_get_photos(const Config *config);

// Utility functions
char* config_get_config_path(void);
void config_ensure_directory(const char *path);

// Legacy functions for compatibility
Config* config_load_legacy(const char *config_path);
void config_save_legacy(Config *config, const char *config_path);
Config* config_new_default(void);

#endif // CONFIG_H