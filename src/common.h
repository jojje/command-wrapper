#ifndef COMMON_H
#define COMMON_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#  define IS_WINDOWS
#endif

#if defined(__CYGWIN__)
#  define IS_CYGWIN
#endif

#ifndef TRUE
#  define TRUE 1
#endif

#ifndef FALSE
#  define FALSE 0
#endif

#ifndef BOOL
#  define BOOL int
#endif

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

// global flag for controlling verbose messages printing (info statements)
BOOL verbose;

// Structure for holding configuration elements
// for the respective wrapped commands.
typedef struct {
  char* cmd;         // The command shortname/basename w.o. extension
  char* abscmd;      // The fully qualified name and path to the actual command to execute
  char** envvars;    // An array of environment variables that should be set / appended when launching this command
  int c_envvars;     // Number of environment variables of the envvars array
} config_entry;

// Generic key-value structure, used for environment
// variable "keys"/names and values.
typedef struct {
  char* key;
  char* value;
} pair;

// Print error message to STDERR and terminate with non-zero exit code
void error(const char* fmt, ...);

// Print the message to STDOUT if the verbose flag has been set
void info(const char* fmt, ...);

#endif  // COMMON_H

