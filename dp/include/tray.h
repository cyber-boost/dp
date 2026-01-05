#ifndef TRAY_H
#define TRAY_H

#include <gtk/gtk.h>
#include <libayatana-appindicator3-0.1/libayatana-appindicator/app-indicator.h>

// Function declarations for system tray functionality
AppIndicator* create_tray_icon(void);
void show_notification(const char* title, const char* message);
void set_random_wallpaper_callback(GtkMenuItem *menuitem, gpointer userdata);
void set_wallpaper_from_app_callback(GtkMenuItem *menuitem, gpointer userdata);
void configure_callback(GtkMenuItem *menuitem, gpointer userdata);
void about_callback(GtkMenuItem *menuitem, gpointer userdata);
void quit_callback(GtkMenuItem *menuitem, gpointer userdata);

#endif // TRAY_H