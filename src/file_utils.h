#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#define MAX_PATH_LEN 32767

#ifdef IS_WINDOWS
#  define FILE_SEP "\\"
#else
#  define FILE_SEP "//"
#endif

// Checks whether or not a file exists
BOOL file_exists(const char* path);

// Extracts the config entry for a given command if it has been provided in the config file
BOOL read_config(const char* config_path, const char* cmd, config_entry* entry);

// Strips leading directories and possible extension from a file name
void basename(char** path);

// Joins two strings with the OS native path separator.
void join_path(char* dst, const char* path1, const char* path2);

#endif // FILE_UTILS_H
