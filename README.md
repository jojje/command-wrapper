Command Wrapper
===============

A generic wrapper command that delegates to another command

DESCRIPTION
-----------
The intended use is to reduce the `PATH` pollution, so that you can quarantine
off large applications to their separate "jails" (directories) and only expose
the individual command(s) you use on the path.

It works by you making a copy of the wrapper executable and giving it the name
of a command you'd like to have on the PATH.

When you launch the renamed wrapper command it will look for a configuration
entry in the file `$(HOME)/.command_wrapper` (or `%HOME%/.command_wrapper` on
Windows) for an entry with the same name as you gave the copy of the wrapper,
read what actual command to call and optionally add any environment variables
you may have specified for this specific wrapper.

SETUP
-----
You need to create the file `$(HOME)/.command_wrapper` and populate it with an
entry (a line) for each wrapper name you want to use. The configuration line
has the following syntax:

    NAME = ACTUAL_COMMAND | ENV=VALUE | ... | ENV=VALUE

 * `NAME` is the basename of the command you want to wrap (without extension
   or directory)

 * `ACTUAL_COMMAND` is the absolute path (complete file name) for the command
   the wrapper should launch when invoked.

 * `ENV` is zero or more environment variables that you want set before the
   launching the actual command.

Each environment variable is delimited by a pipe (|) character. If you don't
want to set any environment variables, you don't specify any pipes either. The
pipe delimiter denotes the start of an environment variable, so the rule is to
have as many pipe characters as environment variables (to keep it simple).

Usage
----
Invoke the wrapper as you would the actual command.

There is one parameter that has a special meaning and which the wrapper will
gobble up and not pass along, and that is `--cw-verbose`. This option makes
the wrapper print out some information about what it's doing prior to the
handover to the actual command, and could be useful during problem
determination or simply as a result of curiosity.

The arcane option flag was chosen to make it highly unlikely that it should
ever conflict with options from actual commands. If you find a conflict, then
you should first let the authors of the actual command know that they have a
pretty poor taste in naming, and that they are not very "client focused". If
that doesn't mitigate the problem, open a bug report here and I'll change the
option name to something even worse.

EXAMPLE
-------
Following is a sample configuration

    > cat %HOME%\.command_wrapper
    git = c:\app\pgit\bin\git.exe | TERM=msys
    vim = C:\app\vim73\vim.exe

_windows example as I'm on such a box atm, but it works the same for gnu/linux
and osx._

With this configuration if I were to rename the `cwrapper` binary to `git` and
execute the wrapper with this new name, it would find the name it was invokes
with in the configuration file, see what actual command corresponds to that
alias, set the TERM environment variable and then spawn/exec the actual git
command.

Any arguments passed to the wrapper are delegated to the mapped executable, so
its use should be transparent to the actual command.

COMPILING
---------
### Prerequisites ###

 * C-Compiler, I've tested mingw, cygwin and visual studio on windows. On Mac
   clang from xcode and on linux gcc.

 * GNU Make

With the pre-reqs in place, just type `make` and it should compile the wrapper
as `cwrapper` on *nix and `cwrapper.exe` on windows.
