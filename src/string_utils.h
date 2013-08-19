#ifndef STRING_UTILS_H
#define STRING_UTILS_H

// Removes leading and trailing white spaces from the supplied string
void trim(char** sptr);

// like strncpy but also inserts a null character at the end of the dst string
char* strncpy_null(char* dst, const char* src, size_t num);

// Quote src and store the resilt into dst. Remember to allocate a string
// 3 characters larger than src for the two quotes and the terminating null
// character.
void quote(char* dst, const char* src);

#endif // STRING_UTILS_H
