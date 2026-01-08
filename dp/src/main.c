/*
 * Dpaper - A dynamic wallpaper changer for Kubuntu
 *
 * Copyright 2026 Dpaper Project
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the MIT License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <gtk/gtk.h>
#include <libayatana-appindicator3-0.1/libayatana-appindicator/app-indicator.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <strings.h>  // For strcasecmp
#include <glib.h>
#include "config.h"

// Global variables
static Config *app_config = NULL;
static guint auto_rotate_timer_id = 0;

// Forward declarations
AppIndicator* create_tray_icon(void);
static void set_random_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata);
static void set_all_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata);
static void set_selected_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata);
static void set_selected_all_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata);
static void find_photos_callback(GtkMenuItem *menuitem, gpointer userdata);
static void remove_photos_callback(GtkMenuItem *menuitem, gpointer userdata);
static void start_auto_rotate_callback(GtkMenuItem *menuitem, gpointer userdata);
static void stop_auto_rotate_callback(GtkMenuItem *menuitem, gpointer userdata);
static void configure_callback(GtkMenuItem *menuitem, gpointer userdata);
static void about_callback(GtkMenuItem *menuitem, gpointer userdata);
static void quit_callback(GtkMenuItem *menuitem, gpointer userdata);
static char* get_default_wallpaper_directory(void);
static int create_default_directory(void);
static char* get_random_image_from_directory(const char *directory);
static int is_image_file(const char *filename);
static int set_kde_wallpaper(const char *image_path);
static int set_kde_wallpaper_desktop(const char *image_path, int desktop_index);
static void copy_files_to_wallpaper_directory(GSList *file_list);
static void update_installed_photos_from_directory(void);
static void install_default_wallpapers(void);
static void show_configuration_dialog(void);
static gboolean auto_rotate_timer_callback(gpointer data);
static void start_auto_rotate(void);
static void stop_auto_rotate(void);
static void toggle_default_wallpapers_callback(GtkMenuItem *menuitem, gpointer userdata);
static void toggle_boot_screen_callback(GtkMenuItem *menuitem, gpointer userdata);
static void set_boot_screen_random_callback(GtkMenuItem *menuitem, gpointer userdata);
static void set_boot_screen_selected_callback(GtkMenuItem *menuitem, gpointer userdata);

int main(int argc, char *argv[]) {
    // Suppress libayatana-appindicator deprecation warnings
    g_setenv("G_DEBUG", "fatal-warnings", TRUE);

    // Initialize GTK
    gtk_init(&argc, &argv);

    // Load configuration
    app_config = config_new();
    char *config_path = config_get_config_path();
    if (!config_load(app_config, config_path)) {
        // Silently use defaults if config can't be loaded
    }
    g_free(config_path);

    // Restart auto-rotate if it was enabled (silently)
    if (app_config->auto_rotate_enabled) {
        start_auto_rotate();
    }

    // Set boot screen wallpaper if enabled (silently)
    if (app_config->boot_screen_enabled) {
        if (app_config->boot_screen_image && strlen(app_config->boot_screen_image) > 0) {
            // Use specific image
            set_kde_wallpaper(app_config->boot_screen_image);
        } else {
            // Use random image
            char *default_dir = get_default_wallpaper_directory();
            char *random_image = get_random_image_from_directory(default_dir);

            if (random_image != NULL) {
                set_kde_wallpaper(random_image);
                free(random_image);
            }

            free(default_dir);
        }
    }

    // Show welcome message
    printf("Welcome to Dynamic Wallpaper! Look for the camera icon in your system tray to select your photos.\n");

    // Create default directory if it doesn't exist
    create_default_directory();

    // Install default wallpapers if enabled
    install_default_wallpapers();

    // Scan and update installed photos
    update_installed_photos_from_directory();

    // Create the system tray icon
    AppIndicator *indicator = create_tray_icon();

    // Set status to active to show the icon
    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

    // Start the GTK main loop
    gtk_main();

    // Save configuration on exit
    config_path = config_get_config_path();
    config_save(app_config, config_path);
    g_free(config_path);

    // Cleanup
    config_free(app_config);

    return 0;
}

static char* get_default_wallpaper_directory(void) {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    
    char *default_dir = malloc(strlen(homedir) + strlen("/.dp") + 1);
    strcpy(default_dir, homedir);
    strcat(default_dir, "/.dp");
    
    return default_dir;
}

static int create_default_directory(void) {
    char *default_dir = get_default_wallpaper_directory();
    
    struct stat st = {0};
    if (stat(default_dir, &st) == -1) {
        // Directory doesn't exist, create it
        if (mkdir(default_dir, 0755) == 0) {
            printf("Created default wallpaper directory: %s\n", default_dir);
        } else {
            printf("Failed to create default wallpaper directory: %s\n", default_dir);
            free(default_dir);
            return -1;
        }
    }
    
    free(default_dir);
    return 0;
}

AppIndicator* create_tray_icon(void) {
    // Try to use custom icon if available, otherwise use default
    // System tray needs PNG at 48x48, desktop can use SVG
    char icon_dir[PATH_MAX];
    snprintf(icon_dir, sizeof(icon_dir), "/usr/share/icons/hicolor/48x48/apps/");
    char icon_file[PATH_MAX];
    strcpy(icon_file, icon_dir);
    strcat(icon_file, "dpaper.png");

    AppIndicator *indicator;
    if (access(icon_file, F_OK) == 0) {
        // Custom icon available
        indicator = app_indicator_new_with_path(
            "dpaper",
            "dpaper",
            APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
            icon_dir
        );
    } else {
        // Fall back to default icon
        indicator = app_indicator_new(
            "dpaper",
            "preferences-desktop-wallpaper",
            APP_INDICATOR_CATEGORY_APPLICATION_STATUS
        );
    }

    // Create menu
    GtkWidget *menu = gtk_menu_new();

    // Create menu items
    GtkWidget *set_random_item = gtk_menu_item_new_with_label("Set Random (Current Desktop)");
    GtkWidget *set_all_item = gtk_menu_item_new_with_label("Set Random (All Desktops)");
    GtkWidget *set_selected_item = gtk_menu_item_new_with_label("Set Selected (Current Desktop)");
    GtkWidget *set_selected_all_item = gtk_menu_item_new_with_label("Set Selected (All Desktops)");
    GtkWidget *find_photos_item = gtk_menu_item_new_with_label("Find Photos");
    GtkWidget *remove_photos_item = gtk_menu_item_new_with_label("Remove Photos");
    GtkWidget *start_auto_rotate_item = gtk_menu_item_new_with_label("Start Auto-Rotate (5 min)");
    GtkWidget *stop_auto_rotate_item = gtk_menu_item_new_with_label("Stop Auto-Rotate");
    GtkWidget *separator3 = gtk_separator_menu_item_new();
    GtkWidget *boot_screen_item = gtk_check_menu_item_new_with_label("Boot Screen Enabled");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(boot_screen_item), app_config->boot_screen_enabled);
    GtkWidget *boot_screen_random_item = gtk_menu_item_new_with_label("Set Boot Screen (Random)");
    GtkWidget *boot_screen_selected_item = gtk_menu_item_new_with_label("Set Boot Screen (Selected)");
    GtkWidget *separator4 = gtk_separator_menu_item_new();
    GtkWidget *default_wallpapers_item = gtk_check_menu_item_new_with_label("Use Default Wallpapers");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(default_wallpapers_item), app_config->use_default_wallpapers);
    GtkWidget *configure_item = gtk_menu_item_new_with_label("Configure");
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    GtkWidget *separator1 = gtk_separator_menu_item_new();
    GtkWidget *separator2 = gtk_separator_menu_item_new();
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");

    // Add items to menu
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), set_random_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), set_all_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), set_selected_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), set_selected_all_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), find_photos_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), remove_photos_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator1);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), start_auto_rotate_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), stop_auto_rotate_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator2);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator3);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), boot_screen_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), boot_screen_random_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), boot_screen_selected_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator4);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), default_wallpapers_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), configure_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), about_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);

    // Show all menu items
    gtk_widget_show(set_random_item);
    gtk_widget_show(set_all_item);
    gtk_widget_show(set_selected_item);
    gtk_widget_show(set_selected_all_item);
    gtk_widget_show(find_photos_item);
    gtk_widget_show(remove_photos_item);
    gtk_widget_show(separator1);
    gtk_widget_show(start_auto_rotate_item);
    gtk_widget_show(stop_auto_rotate_item);
    gtk_widget_show(separator2);
    gtk_widget_show(separator3);
    gtk_widget_show(boot_screen_item);
    gtk_widget_show(boot_screen_random_item);
    gtk_widget_show(boot_screen_selected_item);
    gtk_widget_show(separator4);
    gtk_widget_show(default_wallpapers_item);
    gtk_widget_show(configure_item);
    gtk_widget_show(about_item);
    gtk_widget_show(quit_item);

    // Attach menu to indicator
    app_indicator_set_menu(indicator, GTK_MENU(menu));

    // Connect signals
    g_signal_connect(set_random_item, "activate", G_CALLBACK(set_random_wallpaper_callback), NULL);
    g_signal_connect(set_all_item, "activate", G_CALLBACK(set_all_wallpaper_callback), NULL);
    g_signal_connect(set_selected_item, "activate", G_CALLBACK(set_selected_wallpaper_callback), NULL);
    g_signal_connect(set_selected_all_item, "activate", G_CALLBACK(set_selected_all_wallpaper_callback), NULL);
    g_signal_connect(find_photos_item, "activate", G_CALLBACK(find_photos_callback), NULL);
    g_signal_connect(remove_photos_item, "activate", G_CALLBACK(remove_photos_callback), NULL);
    g_signal_connect(start_auto_rotate_item, "activate", G_CALLBACK(start_auto_rotate_callback), NULL);
    g_signal_connect(stop_auto_rotate_item, "activate", G_CALLBACK(stop_auto_rotate_callback), NULL);
    g_signal_connect(boot_screen_item, "activate", G_CALLBACK(toggle_boot_screen_callback), NULL);
    g_signal_connect(boot_screen_random_item, "activate", G_CALLBACK(set_boot_screen_random_callback), NULL);
    g_signal_connect(boot_screen_selected_item, "activate", G_CALLBACK(set_boot_screen_selected_callback), NULL);
    g_signal_connect(default_wallpapers_item, "activate", G_CALLBACK(toggle_default_wallpapers_callback), NULL);
    g_signal_connect(configure_item, "activate", G_CALLBACK(configure_callback), NULL);
    g_signal_connect(about_item, "activate", G_CALLBACK(about_callback), NULL);
    g_signal_connect(quit_item, "activate", G_CALLBACK(quit_callback), NULL);

    return indicator;
}

static void set_random_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata) {
    (void)menuitem;
    (void)userdata;
    char *default_dir = get_default_wallpaper_directory();
    char *random_image = get_random_image_from_directory(default_dir);

    if (random_image != NULL) {
        // Set random wallpaper on current desktop (desktop 0)
        // Note: We use fallback logic, so this should always succeed
        set_kde_wallpaper_desktop(random_image, 0);
        free(random_image);
    } else {
        // Show error notification for no images
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_WARNING,
                                                   GTK_BUTTONS_OK,
                                                   "No images found in directory:\n%s\n\nPlease add some images using 'Find Photos'.",
                                                   default_dir);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }

    free(default_dir);
}

static void set_all_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata) {
    char *default_dir = get_default_wallpaper_directory();
    char *random_image = get_random_image_from_directory(default_dir);

    if (random_image != NULL) {
        // Set same random wallpaper on all desktops
        set_kde_wallpaper(random_image);
        free(random_image);
    } else {
        // Show error notification for no images
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_WARNING,
                                                   GTK_BUTTONS_OK,
                                                   "No images found in directory:\n%s\n\nPlease add some images using 'Find Photos'.",
                                                   default_dir);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }

    free(default_dir);
}

static void set_selected_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata) {
    // Create file chooser dialog for single selection
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Wallpaper",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    // Add image filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Image files");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_filter_add_pattern(filter, "*.gif");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Set default directory
    const char *default_dir = app_config ? app_config->wallpaper_directory : NULL;
    if (!default_dir) {
        char *fallback_dir = get_default_wallpaper_directory();
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), fallback_dir);
        free(fallback_dir);
    } else {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), default_dir);
    }

    // Run dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename) {
            // Set selected wallpaper on current desktop
            set_kde_wallpaper_desktop(filename, 0);
            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);
}

static void set_selected_all_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata) {
    // Create file chooser dialog for single selection
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Wallpaper",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    // Add image filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Image files");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_filter_add_pattern(filter, "*.gif");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Set default directory
    const char *default_dir = app_config ? app_config->wallpaper_directory : NULL;
    if (!default_dir) {
        char *fallback_dir = get_default_wallpaper_directory();
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), fallback_dir);
        free(fallback_dir);
    } else {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), default_dir);
    }

    // Run dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename) {
            // Set selected wallpaper on all desktops
            set_kde_wallpaper(filename);
            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);
}

static void configure_callback(GtkMenuItem *menuitem, gpointer userdata) {
    show_configuration_dialog();
}

static void about_callback(GtkMenuItem *menuitem, gpointer userdata) {
    // TODO: Implement about dialog
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_INFO,
                                              GTK_BUTTONS_OK,
                                              "Dynamic wallPaper\n"
                                              "Version 1.0\n\n"
                                              "A C-based dynamic wallpaper changer for Kubuntu\n\n"
                                              "Built by Bernie and his beagle Buddy\n\n"
                                              "www.CyberBoost.com\n\n"
                                              "Default directory: ~/.dp/");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void quit_callback(GtkMenuItem *menuitem, gpointer userdata) {
    stop_auto_rotate(); // Stop auto-rotate when quitting
    gtk_main_quit();
}

static void start_auto_rotate_callback(GtkMenuItem *menuitem, gpointer userdata) {
    start_auto_rotate();
}

static void stop_auto_rotate_callback(GtkMenuItem *menuitem, gpointer userdata) {
    stop_auto_rotate();
}

static void start_auto_rotate(void) {
    if (!app_config) {
        return;
    }

    if (app_config->auto_rotate_enabled && auto_rotate_timer_id != 0) {
        return;
    }

    // Set wallpaper immediately
    set_random_wallpaper_callback(NULL, NULL);

    // Start the timer (interval in milliseconds)
    auto_rotate_timer_id = g_timeout_add(app_config->auto_rotate_interval * 1000, auto_rotate_timer_callback, NULL);
    if (auto_rotate_timer_id == 0) {
        return;
    }

    app_config->auto_rotate_enabled = TRUE;

    // Save configuration
    char *config_path = config_get_config_path();
    config_save(app_config, config_path);
    g_free(config_path);
}

static void stop_auto_rotate(void) {
    if (!app_config) {
        return;
    }

    if (!app_config->auto_rotate_enabled) {
        return;
    }

    if (auto_rotate_timer_id > 0) {
        g_source_remove(auto_rotate_timer_id);
        auto_rotate_timer_id = 0;
    }

    app_config->auto_rotate_enabled = FALSE;

    // Save configuration
    char *config_path = config_get_config_path();
    config_save(app_config, config_path);
    g_free(config_path);
}

static gboolean auto_rotate_timer_callback(gpointer data) {
    // Set random wallpaper
    set_random_wallpaper_callback(NULL, NULL);

    // Return TRUE to continue the timer
    return TRUE;
}

static void find_photos_callback(GtkMenuItem *menuitem, gpointer userdata) {
    // Create file chooser dialog
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Photos",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    // Allow multiple selection
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    // Add image filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Image files");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_filter_add_pattern(filter, "*.gif");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Set default directory to Pictures folder
    const char *home = getenv("HOME");
    if (home) {
        char pictures_path[1024];
        snprintf(pictures_path, sizeof(pictures_path), "%s/Pictures", home);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), pictures_path);
    }

    // Run dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        // Get selected files
        GSList *file_list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));

        if (file_list) {
            // Copy files to wallpaper directory
            copy_files_to_wallpaper_directory(file_list);

            // Free the file list
            g_slist_free_full(file_list, g_free);
        }
    }

    gtk_widget_destroy(dialog);
}

static void remove_photos_callback(GtkMenuItem *menuitem, gpointer userdata) {
    (void)menuitem;
    (void)userdata;

    if (!app_config) return;

    // Check if there are any photos to remove
    if (app_config->installed_photos->len == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_INFO,
                                                   GTK_BUTTONS_OK,
                                                   "No photos to remove.\n\nUse 'Find Photos' to add photos first.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Create file chooser dialog for removing photos
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Remove Photos",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Remove", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    // Allow multiple selection
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    // Add image filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Image files");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_filter_add_pattern(filter, "*.gif");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Set current folder to wallpaper directory
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), app_config->wallpaper_directory);

    // Run dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        // Get selected files
        GSList *file_list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));

        if (file_list) {
            int removed_count = 0;

            for (GSList *l = file_list; l != NULL; l = l->next) {
                const char *src_path = (const char *)l->data;

                // Get filename from path
                const char *filename = strrchr(src_path, '/');
                if (filename) {
                    filename++; // Skip the '/'
                } else {
                    filename = src_path;
                }

                // Delete the file from disk
                if (remove(src_path) == 0) {
                    removed_count++;
                    printf("Removed: %s\n", src_path);

                    // Remove from configuration
                    config_remove_photo(app_config, filename);
                } else {
                    printf("Failed to remove: %s\n", src_path);
                }
            }

            // Show success message
            if (removed_count > 0) {
                char message[256];
                snprintf(message, sizeof(message), "Successfully removed %d photo%s from wallpaper collection.",
                        removed_count, removed_count == 1 ? "" : "s");

                GtkWidget *msg_dialog = gtk_message_dialog_new(NULL,
                                                               GTK_DIALOG_MODAL,
                                                               GTK_MESSAGE_INFO,
                                                               GTK_BUTTONS_OK,
                                                               "%s", message);
                gtk_dialog_run(GTK_DIALOG(msg_dialog));
                gtk_widget_destroy(msg_dialog);

                // Save configuration
                char *config_path = config_get_config_path();
                config_save(app_config, config_path);
                g_free(config_path);
            }

            // Free the file list
            g_slist_free_full(file_list, g_free);
        }
    }

    gtk_widget_destroy(dialog);
}

static void copy_files_to_wallpaper_directory(GSList *file_list) {
    const char *dest_dir = app_config ? app_config->wallpaper_directory : NULL;
    if (!dest_dir) {
        char *fallback_dir = get_default_wallpaper_directory();
        dest_dir = fallback_dir;
    }

    int copied_count = 0;

    for (GSList *l = file_list; l != NULL; l = l->next) {
        const char *src_path = (const char *)l->data;

        // Get filename from path
        const char *filename = strrchr(src_path, '/');
        if (filename) {
            filename++; // Skip the '/'
        } else {
            filename = src_path;
        }

        // Create destination path
        char dest_path[2048];
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, filename);

        // Check if file already exists
        if (access(dest_path, F_OK) == 0) {
            // File exists, ask to overwrite or skip
            char new_filename[1024];
            char *dot = strrchr(filename, '.');
            if (dot) {
                // Insert timestamp before extension
                time_t t = time(NULL);
                snprintf(new_filename, sizeof(new_filename), "%.*s_%ld%s",
                        (int)(dot - filename), filename, t, dot);
                filename = new_filename;
            } else {
                // No extension
                time_t t = time(NULL);
                snprintf(new_filename, sizeof(new_filename), "%s_%ld", filename, t);
                filename = new_filename;
            }

            snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, filename);
        }

        // Copy file
        FILE *src = fopen(src_path, "rb");
        if (src) {
            FILE *dest = fopen(dest_path, "wb");
            if (dest) {
                char buffer[8192];
                size_t bytes;
                while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                    fwrite(buffer, 1, bytes, dest);
                }
                fclose(dest);
                copied_count++;

                // Add to configuration
                if (app_config) {
                    config_add_photo(app_config, filename);
                }

                printf("Copied: %s -> %s\n", src_path, dest_path);
            } else {
                printf("Failed to create destination file: %s\n", dest_path);
            }
            fclose(src);
        } else {
            printf("Failed to open source file: %s\n", src_path);
        }
    }

    // Show success message
    if (copied_count > 0) {
        char message[256];
        snprintf(message, sizeof(message), "Successfully added %d photo%s to wallpaper collection.",
                copied_count, copied_count == 1 ? "" : "s");

        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_INFO,
                                                   GTK_BUTTONS_OK,
                                                   "%s", message);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }

    // Cleanup
    if (!app_config) {
        free((char*)dest_dir);
    }
}

static char* get_random_image_from_directory(const char *directory) {
    DIR *dir;
    struct dirent *ent;
    char **image_files = NULL;
    int image_count = 0;
    int image_capacity = 10;
    
    // Allocate initial array for image files
    image_files = malloc(image_capacity * sizeof(char*));
    
    // Open directory
    if ((dir = opendir(directory)) != NULL) {
        // Read all files in directory
        while ((ent = readdir(dir)) != NULL) {
            // Check if file is an image
            if (is_image_file(ent->d_name)) {
                // Expand array if needed
                if (image_count >= image_capacity) {
                    image_capacity *= 2;
                    image_files = realloc(image_files, image_capacity * sizeof(char*));
                }
                
                // Allocate memory for full path
                image_files[image_count] = malloc(strlen(directory) + strlen(ent->d_name) + 2);
                strcpy(image_files[image_count], directory);
                strcat(image_files[image_count], "/");
                strcat(image_files[image_count], ent->d_name);
                
                image_count++;
            }
        }
        closedir(dir);
    } else {
        // Could not open directory
        if (image_files) {
            free(image_files);
        }
        return NULL;
    }
    
    // If no images found, return NULL
    if (image_count == 0) {
        if (image_files) {
            free(image_files);
        }
        return NULL;
    }
    
    // Select random image
    srand(time(NULL));
    int random_index = rand() % image_count;
    
    // Copy the selected image path
    char *selected_image = malloc(strlen(image_files[random_index]) + 1);
    strcpy(selected_image, image_files[random_index]);
    
    // Free allocated memory
    for (int i = 0; i < image_count; i++) {
        free(image_files[i]);
    }
    free(image_files);
    
    return selected_image;
}

static int is_image_file(const char *filename) {
    // Get file extension
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    
    // Check if extension matches supported image formats (case insensitive)
    const char *ext = dot + 1;
    return (strcasecmp(ext, "jpg") == 0 || 
            strcasecmp(ext, "jpeg") == 0 || 
            strcasecmp(ext, "png") == 0 || 
            strcasecmp(ext, "bmp") == 0 || 
            strcasecmp(ext, "gif") == 0);
}

static int set_kde_wallpaper(const char *image_path) {
    return set_kde_wallpaper_desktop(image_path, -1); // -1 means all desktops
}

static int set_kde_wallpaper_desktop(const char *image_path, int desktop_index) {
    // Create error log file
    char *log_path = NULL;
    char *home_dir = getenv("HOME");
    if (home_dir) {
        log_path = malloc(strlen(home_dir) + strlen("/.dp/error.log") + 1);
        sprintf(log_path, "%s/.dp/error.log", home_dir);
    }

    FILE *log_file = log_path ? fopen(log_path, "a") : NULL;
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "[%s] Setting wallpaper: %s (desktop: %d)\n",
                ctime(&now), image_path, desktop_index);
    }

    if (desktop_index == -1) {
        // Set wallpaper for all desktops (default behavior)
        char command[1024];
        snprintf(command, sizeof(command),
                 "plasma-apply-wallpaperimage '%s' >/dev/null 2>&1",
                 image_path);

        if (log_file) {
            fprintf(log_file, "Executing: %s\n", command);
        }

        int result = system(command);

        if (log_file) {
            fprintf(log_file, "Result: %d\n", result);
            fclose(log_file);
        }
        free(log_path);

        return result == 0 ? 0 : -1;
    } else {
        // Set wallpaper for specific desktop using D-Bus
        char command[2048];
        snprintf(command, sizeof(command),
                 "qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript '"
                 "var allDesktops = desktops();"
                 "if (%d < allDesktops.length) {"
                 "  var desktop = allDesktops[%d];"
                 "  desktop.wallpaperPlugin = \"org.kde.image\";"
                 "  desktop.currentConfigGroup = [\"Wallpaper\", \"org.kde.image\", \"General\"];"
                 "  desktop.writeConfig(\"Image\", \"file://%s\");"
                 "}' >/dev/null 2>&1",
                 desktop_index, desktop_index, image_path);

        if (log_file) {
            fprintf(log_file, "Executing: %s\n", command);
        }

        int result = system(command);

        if (log_file) {
            fprintf(log_file, "Result: %d\n", result);
        }

        // If D-Bus fails, try fallback to all-desktops method
        if (result != 0) {
            if (log_file) {
                fprintf(log_file, "D-Bus failed, trying fallback to all-desktops\n");
            }

            char fallback_command[1024];
            snprintf(fallback_command, sizeof(fallback_command),
                     "plasma-apply-wallpaperimage '%s' >/dev/null 2>&1",
                     image_path);

            if (log_file) {
                fprintf(log_file, "Executing fallback: %s\n", fallback_command);
            }

            result = system(fallback_command);

            if (log_file) {
                fprintf(log_file, "Fallback result: %d\n", result);
            }
        }

        if (log_file) {
            fclose(log_file);
        }
        free(log_path);

        return result == 0 ? 0 : -1;
    }
}

// Update installed photos from directory scan
static void update_installed_photos_from_directory(void) {
    if (!app_config) return;

    // Clear existing photos
    g_ptr_array_set_size(app_config->installed_photos, 0);

    // Scan directory for images
    DIR *dir = opendir(app_config->wallpaper_directory);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (is_image_file(ent->d_name)) {
            config_add_photo(app_config, ent->d_name);
        }
    }
    closedir(dir);
}

// Copy default wallpapers from data/wallpaper to user directory
static void install_default_wallpapers(void) {
    if (!app_config->use_default_wallpapers) return;

    const char *default_wallpaper_dir = "/opt/dp/dp/data/wallpaper";
    const char *user_wallpaper_dir = app_config->wallpaper_directory;

    // Check if default wallpapers exist
    if (!g_file_test(default_wallpaper_dir, G_FILE_TEST_IS_DIR)) {
        return;
    }

    // Create user directory if needed
    if (g_mkdir_with_parents(user_wallpaper_dir, 0755) != 0) {
        return;
    }

    // Copy all wallpaper files
    DIR *dir = opendir(default_wallpaper_dir);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (is_image_file(ent->d_name)) {
            char src_path[PATH_MAX];
            char dst_path[PATH_MAX];
            snprintf(src_path, sizeof(src_path), "%s/%s", default_wallpaper_dir, ent->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", user_wallpaper_dir, ent->d_name);

            // Only copy if destination doesn't exist
            if (!g_file_test(dst_path, G_FILE_TEST_EXISTS)) {
                g_file_copy(g_file_new_for_path(src_path),
                           g_file_new_for_path(dst_path),
                           G_FILE_COPY_NONE, NULL, NULL, NULL, NULL);
            }
        }
    }
    closedir(dir);
}

// Toggle default wallpapers callback
static void toggle_default_wallpapers_callback(GtkMenuItem *menuitem, gpointer userdata) {
    if (!app_config) return;
    app_config->use_default_wallpapers = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));

    // If enabled, install wallpapers now
    if (app_config->use_default_wallpapers) {
        install_default_wallpapers();
        update_installed_photos_from_directory();
    }

    // Save config
    char *config_path = config_get_config_path();
    config_save(app_config, config_path);
    g_free(config_path);
}

// Toggle boot screen callback
static void toggle_boot_screen_callback(GtkMenuItem *menuitem, gpointer userdata) {
    if (!app_config) return;
    app_config->boot_screen_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));

    // Save config
    char *config_path = config_get_config_path();
    config_save(app_config, config_path);
    g_free(config_path);
}

// Set boot screen to random callback
static void set_boot_screen_random_callback(GtkMenuItem *menuitem, gpointer userdata) {
    if (!app_config) return;

    // Set to random mode (NULL means random)
    g_free(app_config->boot_screen_image);
    app_config->boot_screen_image = NULL;

    // Save config
    char *config_path = config_get_config_path();
    config_save(app_config, config_path);
    g_free(config_path);

    // Show confirmation
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "Boot screen set to random wallpaper mode.\n\nThe wallpaper will be randomly selected from your collection on system startup.");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Set boot screen to selected callback
static void set_boot_screen_selected_callback(GtkMenuItem *menuitem, gpointer userdata) {
    if (!app_config) return;

    // Create file chooser dialog
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Boot Screen Wallpaper",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Select", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    // Add image filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Image files");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_filter_add_pattern(filter, "*.gif");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Set default directory
    const char *default_dir = app_config ? app_config->wallpaper_directory : NULL;
    if (!default_dir) {
        char *fallback_dir = get_default_wallpaper_directory();
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), fallback_dir);
        free(fallback_dir);
    } else {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), default_dir);
    }

    // Run dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename) {
            // Set selected wallpaper for boot screen
            g_free(app_config->boot_screen_image);
            app_config->boot_screen_image = g_strdup(filename);

            // Save config
            char *config_path = config_get_config_path();
            config_save(app_config, config_path);
            g_free(config_path);

            // Show confirmation
            GtkWidget *msg_dialog = gtk_message_dialog_new(NULL,
                                                           GTK_DIALOG_MODAL,
                                                           GTK_MESSAGE_INFO,
                                                           GTK_BUTTONS_OK,
                                                           "Boot screen wallpaper set to:\n%s\n\nThis wallpaper will be used on system startup.", filename);
            gtk_dialog_run(GTK_DIALOG(msg_dialog));
            gtk_widget_destroy(msg_dialog);

            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);
}

// Show configuration dialog
static void show_configuration_dialog(void) {
    if (!app_config) return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Dpaper - Configuration",
        NULL,
        GTK_DIALOG_MODAL,
        "_OK", GTK_RESPONSE_OK,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 20);
    gtk_widget_set_margin_end(grid, 20);
    gtk_widget_set_margin_top(grid, 20);
    gtk_widget_set_margin_bottom(grid, 20);

    // Wallpaper directory
    GtkWidget *dir_label = gtk_label_new("Wallpaper Directory:");
    gtk_widget_set_halign(dir_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), dir_label, 0, 0, 1, 1);

    GtkWidget *dir_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(dir_entry), app_config->wallpaper_directory);
    gtk_grid_attach(GTK_GRID(grid), dir_entry, 1, 0, 1, 1);

    GtkWidget *dir_button = gtk_button_new_with_label("Browse...");
    gtk_grid_attach(GTK_GRID(grid), dir_button, 2, 0, 1, 1);

    // Auto-rotate interval
    GtkWidget *interval_label = gtk_label_new("Auto-Rotate Interval (seconds):");
    gtk_widget_set_halign(interval_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), interval_label, 0, 1, 1, 1);

    GtkWidget *interval_spin = gtk_spin_button_new_with_range(30, 3600, 30);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(interval_spin), app_config->auto_rotate_interval);
    gtk_grid_attach(GTK_GRID(grid), interval_spin, 1, 1, 1, 1);

    // Supported formats (display only for now)
    GtkWidget *formats_label = gtk_label_new("Supported Formats:");
    gtk_widget_set_halign(formats_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), formats_label, 0, 2, 1, 1);

    GtkWidget *formats_entry = gtk_entry_new();
    GString *formats_str = g_string_new("");
    for (guint i = 0; i < app_config->supported_formats->len; i++) {
        if (i > 0) g_string_append(formats_str, ", ");
        g_string_append(formats_str, (char*)g_ptr_array_index(app_config->supported_formats, i));
    }
    gtk_entry_set_text(GTK_ENTRY(formats_entry), formats_str->str);
    g_string_free(formats_str, TRUE);
    gtk_editable_set_editable(GTK_EDITABLE(formats_entry), FALSE);
    gtk_grid_attach(GTK_GRID(grid), formats_entry, 1, 2, 2, 1);

    // Installed photos count
    char photos_text[256];
    snprintf(photos_text, sizeof(photos_text), "Installed Photos: %d", app_config->installed_photos->len);
    GtkWidget *photos_label = gtk_label_new(photos_text);
    gtk_widget_set_halign(photos_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), photos_label, 0, 3, 3, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        // Update configuration
        const char *new_dir = gtk_entry_get_text(GTK_ENTRY(dir_entry));
        if (new_dir && strlen(new_dir) > 0) {
            g_free(app_config->wallpaper_directory);
            app_config->wallpaper_directory = g_strdup(new_dir);
        }

        app_config->auto_rotate_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(interval_spin));

        // Save configuration
        char *config_path = config_get_config_path();
        config_save(app_config, config_path);
        g_free(config_path);

        // Update installed photos
        update_installed_photos_from_directory();
    }

    gtk_widget_destroy(dialog);
}