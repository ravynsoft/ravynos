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

#include <config.h>

/* Specification.  */
#include "windows-spawn.h"

/* Get declarations of the native Windows API functions.  */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Get _get_osfhandle().  */
#if GNULIB_MSVC_NOTHROW
# include "msvc-nothrow.h"
#else
# include <io.h>
#endif
#include <process.h>

#include "findprog.h"

/* Don't assume that UNICODE is not defined.  */
#undef STARTUPINFO
#define STARTUPINFO STARTUPINFOA
#undef CreateProcess
#define CreateProcess CreateProcessA

#define SHELL_SPECIAL_CHARS "\"\\ \001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037*?"
#define SHELL_SPACE_CHARS " \001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037"

/* Returns the length of a quoted argument string.  */
static size_t
quoted_arg_length (const char *string)
{
  bool quote_around = (strpbrk (string, SHELL_SPACE_CHARS) != NULL);
  size_t length;
  unsigned int backslashes;
  const char *s;

  length = 0;
  backslashes = 0;
  if (quote_around)
    length++;
  for (s = string; *s != '\0'; s++)
    {
      char c = *s;
      if (c == '"')
        length += backslashes + 1;
      length++;
      if (c == '\\')
        backslashes++;
      else
        backslashes = 0;
    }
  if (quote_around)
    length += backslashes + 1;

  return length;
}

/* Produces a quoted argument string.
   Stores exactly quoted_arg_length (STRING) + 1 bytes, including the final
   NUL byte, at MEM.
   Returns a pointer past the stored quoted argument string.  */
static char *
quoted_arg_string (const char *string, char *mem)
{
  bool quote_around = (strpbrk (string, SHELL_SPACE_CHARS) != NULL);
  char *p;
  unsigned int backslashes;
  const char *s;

  p = mem;
  backslashes = 0;
  if (quote_around)
    *p++ = '"';
  for (s = string; *s != '\0'; s++)
    {
      char c = *s;
      if (c == '"')
        {
          unsigned int j;
          for (j = backslashes + 1; j > 0; j--)
            *p++ = '\\';
        }
      *p++ = c;
      if (c == '\\')
        backslashes++;
      else
        backslashes = 0;
    }
  if (quote_around)
    {
      unsigned int j;
      for (j = backslashes; j > 0; j--)
        *p++ = '\\';
      *p++ = '"';
    }
  *p++ = '\0';

  return p;
}

const char **
prepare_spawn (const char * const *argv, char **mem_to_free)
{
  size_t argc;
  const char **new_argv;
  size_t i;

  /* Count number of arguments.  */
  for (argc = 0; argv[argc] != NULL; argc++)
    ;

  /* Allocate new argument vector.  */
  new_argv = (const char **) malloc ((1 + argc + 1) * sizeof (const char *));

  /* Add an element upfront that can be used when argv[0] turns out to be a
     script, not a program.
     On Unix, this would be "/bin/sh". On native Windows, "sh" is actually
     "sh.exe".  We have to omit the directory part and rely on the search in
     PATH, because the mingw "mount points" are not visible inside Windows
     CreateProcess().  */
  new_argv[0] = "sh.exe";

  /* Put quoted arguments into the new argument vector.  */
  size_t needed_size = 0;
  for (i = 0; i < argc; i++)
    {
      const char *string = argv[i];
      size_t length;

      if (string[0] == '\0')
        length = strlen ("\"\"");
      else if (strpbrk (string, SHELL_SPECIAL_CHARS) != NULL)
        length = quoted_arg_length (string);
      else
        length = strlen (string);
      needed_size += length + 1;
    }

  char *mem;
  if (needed_size == 0)
    mem = NULL;
  else
    {
      mem = (char *) malloc (needed_size);
      if (mem == NULL)
        {
          /* Memory allocation failure.  */
          free (new_argv);
          errno = ENOMEM;
          return NULL;
        }
    }
  *mem_to_free = mem;

  for (i = 0; i < argc; i++)
    {
      const char *string = argv[i];

      new_argv[1 + i] = mem;
      if (string[0] == '\0')
        {
          size_t length = strlen ("\"\"");
          memcpy (mem, "\"\"", length + 1);
          mem += length + 1;
        }
      else if (strpbrk (string, SHELL_SPECIAL_CHARS) != NULL)
        {
          mem = quoted_arg_string (string, mem);
        }
      else
        {
          size_t length = strlen (string);
          memcpy (mem, string, length + 1);
          mem += length + 1;
        }
    }
  new_argv[1 + argc] = NULL;

  return new_argv;
}

char *
compose_command (const char * const *argv)
{
  /* Just concatenate the argv[] strings, separated by spaces.  */
  char *command;

  /* Determine the size of the needed block of memory.  */
  size_t total_size = 0;
  const char * const *ap;
  const char *p;
  for (ap = argv; (p = *ap) != NULL; ap++)
    total_size += strlen (p) + 1;
  size_t command_size = (total_size > 0 ? total_size : 1);

  /* Allocate the block of memory.  */
  command = (char *) malloc (command_size);
  if (command == NULL)
    {
      errno = ENOMEM;
      return NULL;
    }

  /* Fill it.  */
  if (total_size > 0)
    {
      char *cp = command;
      for (ap = argv; (p = *ap) != NULL; ap++)
        {
          size_t size = strlen (p) + 1;
          memcpy (cp, p, size - 1);
          cp += size;
          cp[-1] = ' ';
        }
      cp[-1] = '\0';
    }
  else
    *command = '\0';

  return command;
}

char *
compose_envblock (const char * const *envp)
{
  /* This is a bit hairy, because we don't have a lock that would prevent other
     threads from making modifications in ENVP.  So, just make sure we don't
     crash; but if other threads are making modifications, part of the result
     may be wrong.  */
 retry:
  {
    /* Guess the size of the needed block of memory.
       The guess will be exact if other threads don't make modifications.  */
    size_t total_size = 0;
    const char * const *ep;
    const char *p;
    for (ep = envp; (p = *ep) != NULL; ep++)
      total_size += strlen (p) + 1;
    size_t envblock_size = total_size;

    /* Allocate the block of memory.  */
    char *envblock = (char *) malloc (envblock_size + 1);
    if (envblock == NULL)
      {
        errno = ENOMEM;
        return NULL;
      }
    size_t envblock_used = 0;
    for (ep = envp; (p = *ep) != NULL; ep++)
      {
        size_t size = strlen (p) + 1;
        if (envblock_used + size > envblock_size)
          {
            /* Other threads did modifications.  Need more memory.  */
            envblock_size += envblock_size / 2;
            if (envblock_used + size > envblock_size)
              envblock_size = envblock_used + size;

            char *new_envblock = (char *) realloc (envblock, envblock_size + 1);
            if (new_envblock == NULL)
              {
                free (envblock);
                errno = ENOMEM;
                return NULL;
              }
            envblock = new_envblock;
          }
        memcpy (envblock + envblock_used, p, size);
        envblock_used += size;
        if (envblock[envblock_used - 1] != '\0')
          {
            /* Other threads did modifications.  Restart.  */
            free (envblock);
            goto retry;
          }
      }
    envblock[envblock_used] = '\0';
    return envblock;
  }
}

int
init_inheritable_handles (struct inheritable_handles *inh_handles,
                          bool duplicate)
{
  /* Determine the minimal count of handles we need to care about.  */
  size_t handles_count;
  {
    /* _getmaxstdio
       <https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/getmaxstdio>
       Default value is 512.  */
    unsigned int fdmax = _getmaxstdio ();
    if (fdmax < 3)
      fdmax = 3;
    for (; fdmax > 3; fdmax--)
      {
        unsigned int fd = fdmax - 1;
        /* _get_osfhandle
           <https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/get-osfhandle>  */
        HANDLE handle = (HANDLE) _get_osfhandle (fd);
        if (handle != INVALID_HANDLE_VALUE)
          {
            if (duplicate)
              /* We will add fd to the array, regardless of whether it is
                 inheritable or not.  */
              break;
            else
              {
                DWORD hflags;
                /* GetHandleInformation
                   <https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-gethandleinformation>  */
                if (GetHandleInformation (handle, &hflags))
                  {
                    if ((hflags & HANDLE_FLAG_INHERIT) != 0)
                      /* fd denotes an inheritable descriptor.  */
                      break;
                  }
              }
          }
      }
    handles_count = fdmax;
  }
  /* Note: handles_count >= 3.  */

  /* Allocate the array.  */
  size_t handles_allocated = handles_count;
  struct IHANDLE *ih =
    (struct IHANDLE *) malloc (handles_allocated * sizeof (struct IHANDLE));
  if (ih == NULL)
    {
      errno = ENOMEM;
      return -1;
    }

  /* Fill in the array.  */
  {
    HANDLE curr_process = (duplicate ? GetCurrentProcess () : INVALID_HANDLE_VALUE);
    unsigned int fd;
    for (fd = 0; fd < handles_count; fd++)
      {
        ih[fd].handle = INVALID_HANDLE_VALUE;
        /* _get_osfhandle
           <https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/get-osfhandle>  */
        HANDLE handle = (HANDLE) _get_osfhandle (fd);
        if (handle != INVALID_HANDLE_VALUE)
          {
            DWORD hflags;
            /* GetHandleInformation
               <https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-gethandleinformation>  */
            if (GetHandleInformation (handle, &hflags))
              {
                if (duplicate)
                  {
                    /* Add fd to the array, regardless of whether it is
                       inheritable or not.  */
                    if ((hflags & HANDLE_FLAG_INHERIT) != 0)
                      {
                        /* Instead of duplicating it, just mark it as shared.  */
                        ih[fd].handle = handle;
                        ih[fd].flags = KEEP_OPEN_IN_PARENT | KEEP_OPEN_IN_CHILD;
                      }
                    else
                      {
                        if (!DuplicateHandle (curr_process, handle,
                                              curr_process, &ih[fd].handle,
                                              0, TRUE, DUPLICATE_SAME_ACCESS))
                          {
                            unsigned int i;
                            for (i = 0; i < fd; i++)
                              if (ih[i].handle != INVALID_HANDLE_VALUE
                                  && !(ih[i].flags & KEEP_OPEN_IN_PARENT))
                                CloseHandle (ih[i].handle);
                            free (ih);
                            errno = EBADF; /* arbitrary */
                            return -1;
                          }
                        ih[fd].flags = 0;
                      }
                  }
                else
                  {
                    if ((hflags & HANDLE_FLAG_INHERIT) != 0)
                      {
                        /* fd denotes an inheritable descriptor.  */
                        ih[fd].handle = handle;
                        ih[fd].flags = KEEP_OPEN_IN_CHILD;
                      }
                  }
              }
          }
      }
  }

  /* Return the result.  */
  inh_handles->count = handles_count;
  inh_handles->allocated = handles_allocated;
  inh_handles->ih = ih;
  return 0;
}

int
compose_handles_block (const struct inheritable_handles *inh_handles,
                       STARTUPINFO *sinfo)
{
  /* STARTUPINFO
     <https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-startupinfoa>  */
  sinfo->dwFlags = STARTF_USESTDHANDLES;
  sinfo->hStdInput  = inh_handles->ih[0].handle;
  sinfo->hStdOutput = inh_handles->ih[1].handle;
  sinfo->hStdError  = inh_handles->ih[2].handle;

  /* On newer versions of Windows, more file descriptors / handles than the
     first three can be passed.
     The format is as follows: Let N be an exclusive upper bound for the file
     descriptors to be passed. Two arrays are constructed in memory:
       - flags[0..N-1], of element type 'unsigned char',
       - handles[0..N-1], of element type 'HANDLE' or 'intptr_t'.
     For used entries, handles[i] is the handle, and flags[i] is a set of flags,
     a combination of:
        1 for open file descriptors,
       64 for handles of type FILE_TYPE_CHAR,
        8 for handles of type FILE_TYPE_PIPE,
       32 for O_APPEND.
     For unused entries - this may include any of the first three, since they
     are already passed above -, handles[i] is INVALID_HANDLE_VALUE and flags[i]
     is zero.
     lpReserved2 now is a pointer to the concatenation (without padding) of:
       - an 'unsigned int' whose value is N,
       - the contents of the flags[0..N-1] array,
       - the contents of the handles[0..N-1] array.
     cbReserved2 is the size (in bytes) of the object at lpReserved2.  */

  size_t handles_count = inh_handles->count;

  sinfo->cbReserved2 =
    sizeof (unsigned int)
    + handles_count * sizeof (unsigned char)
    + handles_count * sizeof (HANDLE);
  /* Add some padding, so that we can work with a properly aligned HANDLE
     array.  */
  char *hblock = (char *) malloc (sinfo->cbReserved2 + (sizeof (HANDLE) - 1));
  if (hblock == NULL)
    {
      errno = ENOMEM;
      return -1;
    }
  unsigned char *flags = (unsigned char *) (hblock + sizeof (unsigned int));
  char *handles = (char *) (flags + handles_count);
  HANDLE *handles_aligned =
    (HANDLE *) (((uintptr_t) handles + (sizeof (HANDLE) - 1))
                & - (uintptr_t) sizeof (HANDLE));

  * (unsigned int *) hblock = handles_count;
  {
    unsigned int fd;
    for (fd = 0; fd < handles_count; fd++)
      {
        handles_aligned[fd] = INVALID_HANDLE_VALUE;
        flags[fd] = 0;

        HANDLE handle = inh_handles->ih[fd].handle;
        if (handle != INVALID_HANDLE_VALUE
            /* The first three are possibly already passed above.
               But they need to passed here as well, if they have some flags.  */
            && (fd >= 3 || (unsigned char) inh_handles->ih[fd].flags != 0))
          {
            DWORD hflags;
            /* GetHandleInformation
               <https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-gethandleinformation>  */
            if (GetHandleInformation (handle, &hflags))
              {
                if ((hflags & HANDLE_FLAG_INHERIT) != 0)
                  {
                    /* fd denotes an inheritable descriptor.  */
                    handles_aligned[fd] = handle;
                    /* On Microsoft Windows, it would be sufficient to set
                       flags[fd] = 1.  But on ReactOS or Wine, adding the bit
                       that indicates the handle type may be necessary.  So,
                       just do it everywhere.  */
                    flags[fd] = 1 | (unsigned char) inh_handles->ih[fd].flags;
                    switch (GetFileType (handle))
                      {
                      case FILE_TYPE_CHAR:
                        flags[fd] |= 64;
                        break;
                      case FILE_TYPE_PIPE:
                        flags[fd] |= 8;
                        break;
                      default:
                        break;
                      }
                  }
                else
                  /* We shouldn't have any non-inheritable handles in
                     inh_handles->handles.  */
                  abort ();
              }
          }
      }
  }
  if (handles != (char *) handles_aligned)
    memmove (handles, (char *) handles_aligned, handles_count * sizeof (HANDLE));

  sinfo->lpReserved2 = (BYTE *) hblock;

  return 0;
}

void
free_inheritable_handles (struct inheritable_handles *inh_handles)
{
  free (inh_handles->ih);
}

int
convert_CreateProcess_error (DWORD error)
{
  /* Some of these errors probably cannot happen.  But who knows...  */
  switch (error)
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_BAD_PATHNAME:
    case ERROR_BAD_NET_NAME:
    case ERROR_INVALID_NAME:
    case ERROR_DIRECTORY:
      return ENOENT;
      break;

    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
      return EACCES;
      break;

    case ERROR_OUTOFMEMORY:
      return ENOMEM;
      break;

    case ERROR_BUFFER_OVERFLOW:
    case ERROR_FILENAME_EXCED_RANGE:
      return ENAMETOOLONG;
      break;

    case ERROR_BAD_FORMAT:
    case ERROR_BAD_EXE_FORMAT:
      return ENOEXEC;
      break;

    default:
      return EINVAL;
      break;
    }
}

intptr_t
spawnpvech (int mode,
            const char *progname, const char * const *argv,
            const char * const *envp,
            const char *currdir,
            HANDLE stdin_handle, HANDLE stdout_handle, HANDLE stderr_handle)
{
  /* Validate the arguments.  */
  if (!(mode == P_WAIT
        || mode == P_NOWAIT
        || mode == P_DETACH
        || mode == P_OVERLAY)
      || progname == NULL || argv == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  /* Implement the 'p' letter: search for PROGNAME in getenv ("PATH").  */
  const char *resolved_progname =
    find_in_given_path (progname, getenv ("PATH"), NULL, false);
  if (resolved_progname == NULL)
    return -1;

  /* Compose the command.  */
  char *command = compose_command (argv);
  if (command == NULL)
    goto out_of_memory_1;

  /* Copy *ENVP into a contiguous block of memory.  */
  char *envblock;
  if (envp == NULL)
    envblock = NULL;
  else
    {
      envblock = compose_envblock (envp);
      if (envblock == NULL)
        goto out_of_memory_2;
    }

  /* Collect the inheritable handles.  */
  struct inheritable_handles inh_handles;
  if (init_inheritable_handles (&inh_handles, false) < 0)
    {
      int saved_errno = errno;
      if (envblock != NULL)
        free (envblock);
      free (command);
      if (resolved_progname != progname)
        free ((char *) resolved_progname);
      errno = saved_errno;
      return -1;
    }
  inh_handles.ih[0].handle = stdin_handle;
  inh_handles.ih[0].flags = KEEP_OPEN_IN_CHILD;
  inh_handles.ih[1].handle = stdout_handle;
  inh_handles.ih[1].flags = KEEP_OPEN_IN_CHILD;
  inh_handles.ih[2].handle = stderr_handle;
  inh_handles.ih[2].flags = KEEP_OPEN_IN_CHILD;

  /* CreateProcess
     <https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa>  */
  /* <https://docs.microsoft.com/en-us/windows/win32/procthread/process-creation-flags>  */
  DWORD process_creation_flags = (mode == P_DETACH ? DETACHED_PROCESS : 0);
  /* STARTUPINFO
     <https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-startupinfoa>  */
  STARTUPINFO sinfo;
  sinfo.cb = sizeof (STARTUPINFO);
  sinfo.lpReserved = NULL;
  sinfo.lpDesktop = NULL;
  sinfo.lpTitle = NULL;
  if (compose_handles_block (&inh_handles, &sinfo) < 0)
    {
      int saved_errno = errno;
      free_inheritable_handles (&inh_handles);
      if (envblock != NULL)
        free (envblock);
      free (command);
      if (resolved_progname != progname)
        free ((char *) resolved_progname);
      errno = saved_errno;
      return -1;
    }

  PROCESS_INFORMATION pinfo;
  if (!CreateProcess (resolved_progname, command, NULL, NULL, TRUE,
                      process_creation_flags, envblock, currdir, &sinfo,
                      &pinfo))
    {
      DWORD error = GetLastError ();

      free (sinfo.lpReserved2);
      free_inheritable_handles (&inh_handles);
      if (envblock != NULL)
        free (envblock);
      free (command);
      if (resolved_progname != progname)
        free ((char *) resolved_progname);

      errno = convert_CreateProcess_error (error);
      return -1;
    }

  if (pinfo.hThread)
    CloseHandle (pinfo.hThread);
  free (sinfo.lpReserved2);
  free_inheritable_handles (&inh_handles);
  if (envblock != NULL)
    free (envblock);
  free (command);
  if (resolved_progname != progname)
    free ((char *) resolved_progname);

  switch (mode)
    {
    case P_WAIT:
      {
        /* Wait until it terminates.  Then get its exit status code.  */
        switch (WaitForSingleObject (pinfo.hProcess, INFINITE))
          {
          case WAIT_OBJECT_0:
            break;
          case WAIT_FAILED:
            errno = ECHILD;
            return -1;
          default:
            abort ();
          }

        DWORD exit_code;
        if (!GetExitCodeProcess (pinfo.hProcess, &exit_code))
          {
            errno = ECHILD;
            return -1;
          }
        CloseHandle (pinfo.hProcess);
        return exit_code;
      }

    case P_NOWAIT:
      /* Return pinfo.hProcess, not pinfo.dwProcessId.  */
      return (intptr_t) pinfo.hProcess;

    case P_DETACH:
    case P_OVERLAY:
      CloseHandle (pinfo.hProcess);
      return 0;

    default:
      /* Already checked above.  */
      abort ();
    }

  /*NOTREACHED*/
 out_of_memory_2:
  free (command);
 out_of_memory_1:
  if (resolved_progname != progname)
    free ((char *) resolved_progname);
  errno = ENOMEM;
  return -1;
}
