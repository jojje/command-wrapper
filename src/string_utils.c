#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "common.h"
#include "string_utils.h"

void trim(char** sptr) {
  char* s = *sptr;
  char* ptr = s;
  
  ptr = s + strlen(s)-1;
  while(isspace((int)*ptr)) ptr--;
  *(ptr+1) = '\0';
  
  ptr = s;
  while(isspace((int)*ptr)) ptr++;
  
  *sptr = ptr;
}

char* strncpy_null(char* dst, const char* src, size_t num) {
  strncpy(dst, src, num);
  dst[num] = 0;
  return dst;
}

void quote(char* dst, const char* src) {
  sprintf(dst, "\"%s\"", src);
}
