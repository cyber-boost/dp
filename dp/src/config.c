#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib/gstdio.h>

// Create new configuration with defaults
Config* config_new(void) {
    Config *config = g_new0(Config, 1);
    config_set_defaults(config);
    return config;
}

// Set default configuration values
void config_set_defaults(Config *config) {
    // Default wallpaper directory
    config->wallpaper_directory = g_strdup(g_build_filename(g_get_home_dir(), ".dp", NULL));

    // Supported formats
    config->supported_formats = g_ptr_array_new_with_free_func(g_free);
    g_ptr_array_add(config->supported_formats, g_strdup("jpg"));
    g_ptr_array_add(config->supported_formats, g_strdup("jpeg"));
    g_ptr_array_add(config->supported_formats, g_strdup("png"));
    g_ptr_array_add(config->supported_formats, g_strdup("bmp"));
    g_ptr_array_add(config->supported_formats, g_strdup("gif"));

    // Installed photos
    config->installed_photos = g_ptr_array_new_with_free_func(g_free);

    // Auto-rotate settings
    config->auto_rotate_interval = 300; // 5 minutes
    config->auto_rotate_enabled = FALSE;

    // Desktop settings
    config->last_desktop_index = 0;

    // Default wallpapers
    config->use_default_wallpapers = TRUE;

    // Boot screen settings
    config->boot_screen_enabled = FALSE;
    config->boot_screen_image = NULL; // NULL means random
}

// Free configuration memory
void config_free(Config *config) {
    if (!config) return;

    g_free(config->wallpaper_directory);
    g_free(config->boot_screen_image);
    g_ptr_array_free(config->supported_formats, TRUE);
    g_ptr_array_free(config->installed_photos, TRUE);
    g_free(config);
}

// Get configuration file path
char* config_get_config_path(void) {
    return g_build_filename(g_get_home_dir(), ".dp", "config.json", NULL);
}

// Ensure directory exists
void config_ensure_directory(const char *path) {
    char *dir = g_path_get_dirname(path);
    g_mkdir_with_parents(dir, 0755);
    g_free(dir);
}

// Load configuration from JSON file
gboolean config_load(Config *config, const char *filename) {
    if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
        // File doesn't exist, use defaults
        config_set_defaults(config);
        return TRUE;
    }

    GError *error = NULL;
    char *contents = NULL;

    if (!g_file_get_contents(filename, &contents, NULL, &error)) {
        g_warning("Failed to load config file %s: %s", filename, error->message);
        g_error_free(error);
        config_set_defaults(config);
        return FALSE;
    }

    // Parse basic JSON fields manually (simple implementation)
    config_set_defaults(config);

    // Parse auto_rotate_interval
    char *interval_line = g_strstr_len(contents, -1, "auto_rotate_interval");
    if (interval_line) {
        int interval = 0;
        if (sscanf(interval_line, "auto_rotate_interval%*[^0-9]%d", &interval) == 1) {
            config->auto_rotate_interval = interval;
        }
    }

    // Parse auto_rotate_enabled (boolean)
    if (g_strstr_len(contents, -1, "\"auto_rotate_enabled\": true")) {
        config->auto_rotate_enabled = TRUE;
    } else if (g_strstr_len(contents, -1, "\"auto_rotate_enabled\": false")) {
        config->auto_rotate_enabled = FALSE;
    }

    // Parse use_default_wallpapers (boolean)
    if (g_strstr_len(contents, -1, "\"use_default_wallpapers\": false")) {
        config->use_default_wallpapers = FALSE;
    } else {
        config->use_default_wallpapers = TRUE;
    }

    // Parse boot_screen_enabled (boolean)
    if (g_strstr_len(contents, -1, "\"boot_screen_enabled\": true")) {
        config->boot_screen_enabled = TRUE;
    } else if (g_strstr_len(contents, -1, "\"boot_screen_enabled\": false")) {
        config->boot_screen_enabled = FALSE;
    }

    // Parse boot_screen_image
    char *boot_img_start = g_strstr_len(contents, -1, "\"boot_screen_image\": \"");
    if (boot_img_start) {
        boot_img_start += strlen("\"boot_screen_image\": \"");
        char *boot_img_end = g_strstr_len(boot_img_start, -1, "\"");
        if (boot_img_end) {
            gsize boot_img_len = boot_img_end - boot_img_start;
            char *boot_image = g_strndup(boot_img_start, boot_img_len);
            g_free(config->boot_screen_image);
            config->boot_screen_image = boot_image;
        }
    }

    // Parse wallpaper_directory
    char *dir_start = g_strstr_len(contents, -1, "\"wallpaper_directory\": \"");
    if (dir_start) {
        dir_start += strlen("\"wallpaper_directory\": \"");
        char *dir_end = g_strstr_len(dir_start, -1, "\"");
        if (dir_end) {
            gsize dir_len = dir_end - dir_start;
            char *directory = g_strndup(dir_start, dir_len);
            g_free(config->wallpaper_directory);
            config->wallpaper_directory = directory;
        }
    }

    g_free(contents);
    return TRUE;
}

// Save configuration to JSON file
gboolean config_save(const Config *config, const char *filename) {
    config_ensure_directory(filename);

    GString *json = g_string_new("{\n");

    // Wallpaper directory
    g_string_append_printf(json, "  \"wallpaper_directory\": \"%s\",\n",
                          config->wallpaper_directory);

    // Supported formats
    g_string_append(json, "  \"supported_formats\": [");
    for (guint i = 0; i < config->supported_formats->len; i++) {
        if (i > 0) g_string_append(json, ", ");
        g_string_append_printf(json, "\"%s\"",
                              (char*)g_ptr_array_index(config->supported_formats, i));
    }
    g_string_append(json, "],\n");

    // Installed photos
    g_string_append(json, "  \"installed_photos\": [");
    for (guint i = 0; i < config->installed_photos->len; i++) {
        if (i > 0) g_string_append(json, ", ");
        g_string_append_printf(json, "\"%s\"",
                              (char*)g_ptr_array_index(config->installed_photos, i));
    }
    g_string_append(json, "],\n");

    // Auto-rotate settings
    g_string_append_printf(json, "  \"auto_rotate_interval\": %d,\n",
                          config->auto_rotate_interval);
    g_string_append_printf(json, "  \"auto_rotate_enabled\": %s,\n",
                          config->auto_rotate_enabled ? "true" : "false");

    // Default wallpapers setting
    g_string_append_printf(json, "  \"use_default_wallpapers\": %s,\n",
                          config->use_default_wallpapers ? "true" : "false");

    // Boot screen settings
    g_string_append_printf(json, "  \"boot_screen_enabled\": %s,\n",
                          config->boot_screen_enabled ? "true" : "false");
    g_string_append_printf(json, "  \"boot_screen_image\": \"%s\",\n",
                          config->boot_screen_image ? config->boot_screen_image : "");

    // Last desktop index
    g_string_append_printf(json, "  \"last_desktop_index\": %d\n",
                          config->last_desktop_index);

    g_string_append(json, "}\n");

    GError *error = NULL;
    gboolean success = g_file_set_contents(filename, json->str, -1, &error);

    if (!success) {
        g_warning("Failed to save config file %s: %s", filename, error->message);
        g_error_free(error);
    }

    g_string_free(json, TRUE);
    return success;
}

// Photo management functions
void config_add_photo(Config *config, const char *filename) {
    if (!config_has_photo(config, filename)) {
        g_ptr_array_add(config->installed_photos, g_strdup(filename));
    }
}

void config_remove_photo(Config *config, const char *filename) {
    for (guint i = 0; i < config->installed_photos->len; i++) {
        const char *photo = (const char*)g_ptr_array_index(config->installed_photos, i);
        if (g_strcmp0(photo, filename) == 0) {
            g_ptr_array_remove_index(config->installed_photos, i);
            break;
        }
    }
}

gboolean config_has_photo(const Config *config, const char *filename) {
    for (guint i = 0; i < config->installed_photos->len; i++) {
        const char *photo = (const char*)g_ptr_array_index(config->installed_photos, i);
        if (g_strcmp0(photo, filename) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

GPtrArray* config_get_photos(const Config *config) {
    return config->installed_photos;
}

// Legacy compatibility functions
Config* config_load_legacy(const char *config_path) {
    Config *config = config_new();
    config_load(config, config_path);
    return config;
}

void config_save_legacy(Config *config, const char *config_path) {
    config_save(config, config_path);
}

Config* config_new_default(void) {
    return config_new();
}