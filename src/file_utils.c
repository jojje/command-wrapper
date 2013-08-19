// Note on memory leaks:
// parse_env_vars and parse_config_entry will leak ~ 50 bytes per config entry, so unless
// you have millions of command entries, the machine should still be able to launch and 
// run the wrapped process. These elements need to be passed to the child process and 
// we don't know how big they are in advance, so we can't use stack space and we can't
// free them either before invoking the child.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // XXX REMOVE

#include "common.h"
#include "string_utils.h"
#include "file_utils.h"

char* malloc_string(size_t size) {
  char* s = (char*) malloc(size);
  if(!s) {
     error("Failed to allocate memory\n");
  }
  return s;
}

void join_path(char* dst, const char* path1, const char* path2) {
  snprintf(dst, MAX_PATH_LEN, "%s%s%s", path1, FILE_SEP, path2);
}

void basename(char** ppath) {
  char* path = *ppath;
  int i, last_sep, len=strlen(path);

  for(i=0, last_sep=0; i<len; i++) {          // Remove leading dirs
    if(path[i] == '/' || path[i] == '\\') {
      last_sep = i+1;
    }
  }
  path += last_sep;
  *ppath = path;

  len = strlen(path);
#ifdef IS_WINDOWS
    for(i=0, last_sep=0; i<len; i++) {         // extension if any
      if(path[i] == '.') {
        last_sep = i-1;
      }
    }
    if(last_sep > 0) {
      path[last_sep+1] = 0;
    }
#endif
}


BOOL read_line(char* buff, size_t maxlen, FILE* fp) {
  int ch = 0;
  size_t i = 0;
  while(i < maxlen && (ch = getc(fp)) != EOF && ch != '\r' && ch != '\n') {
    buff[i++] = ch;
  }
  buff[i] = 0;
  return ch != EOF;
}

BOOL file_exists(const char* path) {
  FILE* fp;
  if( (fp = fopen(path,"r")) ) {
    fclose(fp);
    return TRUE;
  }
  return FALSE;
}

// Extracts config name = value pairs into the respective *name* and *value*
// arrays. Returns TRUE if successful, FALSE otherwise.
BOOL get_name_value(const char* line, char** pname, char** pvalue){
  size_t pos = 0, len = strlen(line);
  pos = strcspn(line, "=");
  if(pos == len)
    return FALSE;
  
  strncpy_null(*pname, line, pos);
  trim(pname);
  
  line += pos+1;
  strncpy_null(*pvalue, line, strlen(line));
  trim(pvalue);
  
  return TRUE;
}


// E
//
BOOL parse_env_vars(config_entry* entry, const char* line, char delim) {
  char* s = malloc_string(strlen(line)+1);
  size_t i, j, len = strlen(line);
  char* start;

  strcpy(s,line);

  for(i=0, j=0; i<len; i++) {
    if( s[i] == delim ) j++;
  }

  entry->c_envvars = j;
  entry->envvars = (char**) malloc(entry->c_envvars * sizeof(char**));
  if(!entry->envvars)
    error("Failed to allocate memory");

  s++;
  start = s;
  for(i=0; *s; s++) {
    if( *s == '|' ) { 
      *s = 0;       // Replace | with null to simplify extracting the token
      entry->envvars[i] = (char*) malloc(strlen(start) + 1);
      strcpy(entry->envvars[i], start);
      trim(&entry->envvars[i]);
      i++;
      start = s+1;
    }
  }
  if(strlen(start) > 3) {
    entry->envvars[i] = (char*) malloc(strlen(start) + 1);
    strcpy(entry->envvars[i], start);
    trim(&entry->envvars[i]);
  }

  return TRUE;
}

// Extracts config name = value pairs into the respective *name* and *value*
// arrays. Returns TRUE if successful, FALSE otherwise.
BOOL parse_config_entry(config_entry* entry, char* line){
  size_t pos = 0, len = strlen(line);

  // Command name
  pos = strcspn(line, "=");

  if(pos == len)
    return FALSE;
  
  entry->cmd = malloc_string( pos+1 );

  strncpy_null(entry->cmd, line, pos);
  trim(&entry->cmd);
  
  // Absolute path to command
  line += pos+1;
  pos = strcspn(line, "|");
  entry->abscmd = malloc_string( pos+1 );
  strncpy_null(entry->abscmd, line, pos);
  trim(&entry->abscmd);

  // Environment variables
  line += pos;
  trim(&line);
  len = strlen(line);
  if(len > 4) {
    parse_env_vars(entry, line, '|');
  }

  return TRUE;
}

// Extracts a configuration parameter value from a configuration file if found,
// stores the value in *value* and returns TRUE.
//
// If the configuration file doesn't exist, the parameter name is not found
// in the config file, or the parameter's value is empty, then FALSE is
// returned.
BOOL read_config(const char* config_path, const char* cmd, config_entry* entry) {
  FILE* fp = 0;
  char line[1+MAX_PATH_LEN*2];

  BOOL ret = FALSE;

  if(! (fp = fopen(config_path,"r")) ) {
    return FALSE;
  }
  while ( read_line(line, MAX_PATH_LEN, fp) ) {
    if( parse_config_entry(entry, line) ) {
      if (strcmp(entry->cmd, cmd) == 0) {
        ret = TRUE;
        break;
      }
    }
  }
  fclose(fp);
  return ret;
}
