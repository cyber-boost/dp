#ifndef FILEMANAGER_H
#define FILEMANAGER_H

// Function declarations for file management
char** scan_image_directory(const char *directory, const char **extensions, int extension_count, int *file_count);
int is_valid_image_file(const char *filepath, const char **extensions, int extension_count);
void free_string_array(char **array, int count);

#endif // FILEMANAGER_H