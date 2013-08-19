#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "common.h"

void error(const char* fmt, ...) {
  va_list args;
  int max = 1<<16;
  char* msg = (char*) malloc(max);
  if(msg) {
    strcpy(msg,"ERROR ");
    strcat(msg,fmt);
    strcat(msg,"\n");

    va_start(args, fmt);
    vfprintf(stderr, msg, args);
    va_end(args);
  }
  free(msg);
  exit(1);
}

void info(const char* fmt, ...) {
  va_list args;
  char* msg;
  if(verbose) {
    msg = (char*) malloc(1<<16);
    if(msg) {
      strcpy(msg,"INFO ");
      strcat(msg,fmt);
      strcat(msg,"\n");
  
      va_start(args, fmt);
      vfprintf(stdout, msg, args);
      va_end(args);
    }
    free(msg);
  }
}
