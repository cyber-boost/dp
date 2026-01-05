#ifndef WALLPAPER_H
#define WALLPAPER_H

// Function declarations for wallpaper functionality
int set_wallpaper(const char *image_path);
int set_random_wallpaper(const char *directory, char **extensions, int extension_count);
char* get_random_image_from_directory(const char *directory, char **extensions, int extension_count);
int refresh_wallpaper(void);

#endif // WALLPAPER_H