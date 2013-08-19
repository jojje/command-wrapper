// Note on memory leaks:
// The function setEnv will leak some 300 bytes for each call to this
// function, but there's no getting to zero leaking when the user wants to add
// set custom environment variables since we *must* pass the env structure to
// the child process as part of exec / spawn. The OS will clean up afterwards
// anyway, when the wrapped program has finished.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#ifdef IS_WINDOWS
#  include <process.h>
#else
#  include <unistd.h>
#endif

#include "file_utils.h"
#include "string_utils.h"

#define CONFIG_FILE_NAME  ".command_wrapper"
#define VERBOSE_FLAG      "--cw-verbose"

void split_pair(pair* kv, char* s ) {
  int pos = strcspn(s, "=");
  *(s + pos) = 0;
  kv->key = s;
  s += pos+1;
  kv->value = s;
}

// Parse the arguments to the wrapper and set the global verbose flag if the
// user specified that command argument.
void parse_verbose(int* argc, char* argv[]) {
  int dropped = 0;
  int i,j;
  for(i=*argc-1; i>0; i--) {
    if( strcmp(argv[i], VERBOSE_FLAG) == 0 ) {
      dropped++;
      verbose = TRUE;
      for(j=i; j < *argc-1; j++)      // Drag forward arguments after the dropped verbose arg
        argv[j] = argv[j+1];          // Except if the dropped arg is the last in the original list
    }
  }
  *argc = *argc - dropped;            // Cut off the dropped args by inserting null at the first in the remaining list
  argv[*argc] = 0;
}


// Sets an environment variable to key=value and returns a pointer to the
// string array containing the ammended environment list.
char** setEnv(char* key, char* value, char* envp[]) {
    int i, found = FALSE;
    char** new_envp;
    char lkey[MAX_PATH_LEN+1];
    size_t pos;

    for(i=0; envp[i] != 0; i++) {
        pos = strcspn(envp[i], "=");
        strncpy_null(lkey, envp[i], pos);
        if( strcmp(key, lkey) == 0 ) {
          found = TRUE;
          info("Replacing ENV: %s -> %s=%s\n", envp[i], key, value);
          envp[i] = (char*) malloc( strlen(key) * strlen(value) + 2 );
          sprintf(envp[i], "%s=%s", key, value);
        } 
    }

    if(!found) {
        new_envp = (char**) malloc( (i+2) * sizeof(char**) );
        for(i=0; envp[i] != 0; i++) {
          new_envp[i] = envp[i];
        }
        new_envp[i] = (char*) malloc( strlen(key) * strlen(value) + 2 );
        sprintf(new_envp[i], "%s=%s", key, value);
        info("Adding ENV: %s\n", new_envp[i]);
        new_envp[i+1] = 0;
        envp = new_envp;
    }

    return envp;
}

// Searches the ${HOME}/.command_wrapper files for user specified
// configuration for the command (key) specified.
//
// Returns the extracted configuration information for the command 
// if it was found in the config file, else FALSE.
config_entry* find_config(const char* cmd) {
  char cfg_file[MAX_PATH_LEN+1];
  char* home = getenv("HOME");
  config_entry* entry;

  if(!home)
    error("ERROR Environment variable HOME is not set. Something must be "
          "wrong with the environment configuration\n");

  join_path(cfg_file, home, CONFIG_FILE_NAME);
  if( !file_exists(cfg_file) )
    error("ERROR file \"%s\" not found.\n"
          "Please create it and add command mappings to it in the format\n\n"
          "COMMAND = DIRECTORY\n\n"
          "where COMMAND is the name of a command wrapper instance and "
          "DIRECTORY is the directory where the real command resides.\n", 
          cfg_file);

  entry = (config_entry*) malloc( sizeof(config_entry) );
  entry->cmd = (char*) cmd;      // Use memory from the stack, will survive until we spawn.
  entry->cmd = 0;
  entry->abscmd = 0;
  entry->envvars = 0;
  entry->c_envvars = 0;

  if( !read_config(cfg_file, cmd, entry) )
    error("ERROR no mapping defined for \"%s\" in configuration file: %s\n", cmd, cfg_file);

  return entry;
}

// Launches the actual command with the provided arguments and environment
// configuration.
//
// Note: a few bytes of additional leaking on windows for the quoting of
// arguments, but again, the OS will mop up after the "child" finishes.
int launch(int argc, char* argv[], char* envp[]) {
  int i, rc;
  char* infomsg;
#ifdef IS_WINDOWS
  char* s;
#endif

  if(! file_exists(argv[0]) )
    error("File command found: %s\n",argv[0]);

#ifdef IS_WINDOWS
  // msysgit on windows requires space separated args to be quoted for some
  // reason, so let's quote all args except the command name itself as
  // execv/spawn will bork at the latter.
  for(i=1; i<argc; i++) {
      s = (char*) malloc( strlen(argv[i]) + 3);
      quote(s,argv[i]);
      argv[i] = s;
  }
#endif

  if(verbose) {
    infomsg = (char*) malloc(1<<16);
    infomsg[0] = 0;
    if(infomsg) {
      info("Launching using environment variables:");
      for(i=0; envp[i] != 0; i++) info("  %s", envp[i] );

      info("");
      info("Command arguments: ");
      for(i=0; i<argc ; i++) {
        strcat(infomsg, argv[i] );
        if(i<argc-1) strcat(infomsg, " ");
      }
      info("  %s\n", infomsg);
    }
  }

# ifdef IS_WINDOWS
    rc = _spawnve(_P_WAIT, argv[0], (const char * const *)argv,(const char * const *)envp);
# else
    rc = execv(argv[0],(char * const *)argv);
# endif
  info("Return code: %d", rc);
  return rc;
}

int main(int argc, char* argv[], char* envp[]) {
  int i;
  char* cmd = argv[0];
  config_entry* entry;
  pair kv;

  parse_verbose(&argc, argv);

  basename(&cmd);
  entry = find_config(cmd);

  argv[0] = entry->abscmd;
  for(i=0; i<entry->c_envvars; i++){
    split_pair(&kv, entry->envvars[i] );
    envp = setEnv(kv.key, kv.value, envp);
  }

  free(entry);                       // No longer needed as all bits of data 
                                     // have gone into their target structures.
  return launch(argc, argv, envp);

  return 0;
}

