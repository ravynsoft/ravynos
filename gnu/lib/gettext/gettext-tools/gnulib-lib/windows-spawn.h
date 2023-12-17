/* Auxiliary functions for the creation of subprocesses.  Native Windows API.
   Copyright (C) 2001, 2003-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _WINDOWS_SPAWN_H
#define _WINDOWS_SPAWN_H

/* This file uses _GL_ATTRIBUTE_MALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdint.h>
#include <stdlib.h>

/* Get declarations of the native Windows API functions.  */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


/* Prepares an argument vector before calling spawn().

   Note that spawn() does not by itself call the command interpreter
     (getenv ("COMSPEC") != NULL ? getenv ("COMSPEC") :
      ({ OSVERSIONINFO v; v.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
         GetVersionEx(&v);
         v.dwPlatformId == VER_PLATFORM_WIN32_NT;
      }) ? "cmd.exe" : "command.com").
   Instead it simply concatenates the arguments, separated by ' ', and calls
   CreateProcess().  We must quote the arguments since Windows CreateProcess()
   interprets characters like ' ', '\t', '\\', '"' (but not '<' and '>') in a
   special way:
   - Space and tab are interpreted as delimiters. They are not treated as
     delimiters if they are surrounded by double quotes: "...".
   - Unescaped double quotes are removed from the input. Their only effect is
     that within double quotes, space and tab are treated like normal
     characters.
   - Backslashes not followed by double quotes are not special.
   - But 2*n+1 backslashes followed by a double quote become
     n backslashes followed by a double quote (n >= 0):
       \" -> "
       \\\" -> \"
       \\\\\" -> \\"
   - '*', '?' characters may get expanded through wildcard expansion in the
     callee: By default, in the callee, the initialization code before main()
     takes the result of GetCommandLine(), wildcard-expands it, and passes it
     to main(). The exceptions to this rule are:
       - programs that inspect GetCommandLine() and ignore argv,
       - mingw programs that have a global variable 'int _CRT_glob = 0;',
       - Cygwin programs, when invoked from a Cygwin program.

   prepare_spawn creates and returns a new argument vector, where the arguments
   are appropriately quoted and an additional argument "sh.exe" has been added
   at the beginning.  The new argument vector is freshly allocated.  The memory
   for all its elements is allocated within *MEM_TO_FREE, which is freshly
   allocated as well.  In case of memory allocation failure, NULL is returned,
   with errno set.
 */
extern const char ** prepare_spawn (const char * const *argv,
                                    char **mem_to_free);

/* Composes the command to be passed to CreateProcess().
   ARGV must contain appropriately quoted arguments, as returned by
   prepare_spawn.
   Returns a freshly allocated string.  In case of memory allocation failure,
   NULL is returned, with errno set.  */
extern char * compose_command (const char * const *argv)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;

/* Composes the block of memory that contains the environment variables.
   ENVP must contain an environment (a NULL-terminated array of string of the
   form VARIABLE=VALUE).
   Returns a freshly allocated block of memory.  In case of memory allocation
   failure, NULL is returned, with errno set.  */
extern char * compose_envblock (const char * const *envp)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;


/* An inheritable handle with some additional information.  */
struct IHANDLE
{
  /* Either INVALID_HANDLE_VALUE or an inheritable handle.  */
  HANDLE handle;
  /* Only relevant if handle != INVALID_HANDLE_VALUE.
     It is a bit mask consisting of:
       - 32 for O_APPEND.
       - KEEP_OPEN_IN_CHILD if the handle is scheduled to be preserved in the
         child process.
       - KEEP_OPEN_IN_PARENT if the handle is shared with (and needs to be kept
         open in) the parent process.
       - DELAYED_DUP2_OLDFD if there is a delayed dup2 (oldfd, newfd) and
         this IHANDLE is at position oldfd.
       - DELAYED_DUP2_NEWFD if there is a delayed dup2 (oldfd, newfd) and
         this IHANDLE is at position newfd.
     Note that DELAYED_DUP2_OLDFD and DELAYED_DUP2_NEWFD cannot be set in the
     same IHANDLE.  */
  unsigned short flags;
  #define KEEP_OPEN_IN_CHILD 0x100
  #define KEEP_OPEN_IN_PARENT 0x200
  #define DELAYED_DUP2_OLDFD 0x400
  #define DELAYED_DUP2_NEWFD 0x800
  /* Only relevant if handle != INVALID_HANDLE_VALUE and flags contains
     either DELAYED_DUP2_OLDFD or DELAYED_DUP2_NEWFD.
     It is the other fd of the delayed dup2 (oldfd, newfd), i.e.
       - for DELAYED_DUP2_OLDFD, the newfd,
       - for DELAYED_DUP2_NEWFD, the oldfd.  */
  int linked_fd;
};

/* This struct keeps track of which handles to potentially pass to a subprocess,
   and with which flags.  All of the handles here are inheritable.
   Regarding handle inheritance, see
   <https://docs.microsoft.com/en-us/windows/win32/sysinfo/handle-inheritance>.
   Whether a handle is actually scheduled for being preserved in the child
   process is determined by the KEEP_OPEN_IN_CHILD bit in the flags.  */
struct inheritable_handles
{
  /* The number of occupied entries in the two arrays below.
     3 <= count <= allocated.  */
  size_t count;
  /* The number of allocated entries in the two arrays below.  */
  size_t allocated;
  /* ih[0..count-1] are the occupied entries.  */
  struct IHANDLE *ih;
};

/* Initializes a set of inheritable handles, filling in all or part of the
   file descriptors of the current process.
   If DUPLICATE is true, the handles stored are those of all file descriptors,
   and we use DuplicateHandle to make sure that they are all inheritable.
   If DUPLICATE is false, the handles stored are only the inheritables ones;
   this is a shortcut for spawnpvech().
   Returns 0 upon success.  In case of failure, -1 is returned, with errno set.
 */
extern int init_inheritable_handles (struct inheritable_handles *inh_handles,
                                     bool duplicate);

/* Fills a set of inheritable handles into a STARTUPINFO for CreateProcess().
   Returns 0 upon success.  In case of failure, -1 is returned, with errno set.
 */
extern int compose_handles_block (const struct inheritable_handles *inh_handles,
                                  STARTUPINFOA *sinfo);

/* Frees the memory held by a set of inheritable handles.  */
extern void free_inheritable_handles (struct inheritable_handles *inh_handles);


/* Converts a CreateProcess() error code (retrieved through GetLastError()) to
   an errno value.  */
extern int convert_CreateProcess_error (DWORD error);


/* Creates a subprocess.
   MODE is either P_WAIT or P_NOWAIT.
   PROGNAME is the program to invoke.
   ARGV is the NULL-terminated array of arguments, ARGV[0] being PROGNAME by
   convention.
   ENVP is the NULL-terminated set of environment variable assignments, or NULL
   to inherit the initial environ variable assignments from the caller and
   ignore all calls to putenv(), setenv(), unsetenv() done in the caller.
   CURRDIR is the directory in which to start the program, or NULL to inherit
   the working directory from the caller.
   STDIN_HANDLE, STDOUT_HANDLE, STDERR_HANDLE are the handles to use for the
   first three file descriptors in the callee process.
   Returns
     - 0 for success (if MODE is P_WAIT), or
     - a handle that be passed to _cwait (on Windows) or waitpid (on OS/2), or
     - -1 upon error, with errno set.
 */
extern intptr_t spawnpvech (int mode,
                            const char *progname, const char * const *argv,
                            const char * const *envp,
                            const char *currdir,
                            HANDLE stdin_handle, HANDLE stdout_handle,
                            HANDLE stderr_handle);

#endif /* _WINDOWS_SPAWN_H */
