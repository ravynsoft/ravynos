/* Guts of POSIX spawn interface.  Generic POSIX.1 version.
   Copyright (C) 2000-2006, 2008-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <spawn.h>
#include "spawn_int.h"

#include <alloca.h>
#include <errno.h>

#include <fcntl.h>
#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif

#if _LIBC || HAVE_PATHS_H
# include <paths.h>
#else
# define _PATH_BSHELL BOURNE_SHELL
#endif

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if _LIBC
# include <not-cancel.h>
#else
# define close_not_cancel close
# define open_not_cancel open
#endif

#if _LIBC
# include <local-setxid.h>
#else
# if !HAVE_SETEUID
#  define seteuid(id) setresuid (-1, id, -1)
# endif
# if !HAVE_SETEGID
#  define setegid(id) setresgid (-1, id, -1)
# endif
# define local_seteuid(id) seteuid (id)
# define local_setegid(id) setegid (id)
#endif

#if _LIBC
# define alloca __alloca
# define execve __execve
# define dup2 __dup2
# define fork __fork
# define getgid __getgid
# define getuid __getuid
# define sched_setparam __sched_setparam
# define sched_setscheduler __sched_setscheduler
# define setpgid __setpgid
# define sigaction __sigaction
# define sigismember __sigismember
# define sigprocmask __sigprocmask
# define strchrnul __strchrnul
# define vfork __vfork
#endif


/* The Unix standard contains a long explanation of the way to signal
   an error after the fork() was successful.  Since no new wait status
   was wanted there is no way to signal an error using one of the
   available methods.  The committee chose to signal an error by a
   normal program exit with the exit code 127.  */
#define SPAWN_ERROR     127


#if defined _WIN32 && ! defined __CYGWIN__
/* Native Windows API.  */

/* Define to 1 to enable DuplicateHandle optimization.
   Define to 0 to disable this optimization.  */
# ifndef SPAWN_INTERNAL_OPTIMIZE_DUPLICATEHANDLE
#  define SPAWN_INTERNAL_OPTIMIZE_DUPLICATEHANDLE 1
# endif

/* Get declarations of the native Windows API functions.  */
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

# include <stdio.h>

# include "filename.h"
# include "concat-filename.h"
# include "findprog.h"
# include "malloca.h"
# include "windows-spawn.h"

/* Don't assume that UNICODE is not defined.  */
# undef CreateFile
# define CreateFile CreateFileA
# undef STARTUPINFO
# define STARTUPINFO STARTUPINFOA
# undef CreateProcess
# define CreateProcess CreateProcessA

/* Grows inh_handles->count so that it becomes > newfd.
   Returns 0 upon success.  In case of failure, -1 is returned, with errno set.
 */
static int
grow_inheritable_handles (struct inheritable_handles *inh_handles, int newfd)
{
  if (inh_handles->allocated <= newfd)
    {
      size_t new_allocated = 2 * inh_handles->allocated + 1;
      if (new_allocated <= newfd)
        new_allocated = newfd + 1;
      struct IHANDLE *new_ih =
        (struct IHANDLE *)
        realloc (inh_handles->ih, new_allocated * sizeof (struct IHANDLE));
      if (new_ih == NULL)
        {
          errno = ENOMEM;
          return -1;
        }
      inh_handles->allocated = new_allocated;
      inh_handles->ih = new_ih;
    }

  struct IHANDLE *ih = inh_handles->ih;

  for (; inh_handles->count <= newfd; inh_handles->count++)
    ih[inh_handles->count].handle = INVALID_HANDLE_VALUE;

  return 0;
}

# if SPAWN_INTERNAL_OPTIMIZE_DUPLICATEHANDLE

/* Assuming inh_handles->ih[newfd].handle != INVALID_HANDLE_VALUE
   and      (inh_handles->ih[newfd].flags & DELAYED_DUP2_NEWFD) != 0,
   actually performs the delayed dup2 (oldfd, newfd).
   Returns 0 upon success.  In case of failure, -1 is returned, with errno set.
 */
static int
do_delayed_dup2 (int newfd, struct inheritable_handles *inh_handles,
                 HANDLE curr_process)
{
  int oldfd = inh_handles->ih[newfd].linked_fd;
  /* Check invariants.  */
  if (!((inh_handles->ih[oldfd].flags & DELAYED_DUP2_OLDFD) != 0
        && newfd == inh_handles->ih[oldfd].linked_fd
        && inh_handles->ih[newfd].handle == inh_handles->ih[oldfd].handle))
    abort ();
  /* Duplicate the handle now.  */
  if (!DuplicateHandle (curr_process, inh_handles->ih[oldfd].handle,
                        curr_process, &inh_handles->ih[newfd].handle,
                        0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      errno = EBADF; /* arbitrary */
      return -1;
    }
  inh_handles->ih[oldfd].flags &= ~DELAYED_DUP2_OLDFD;
  inh_handles->ih[newfd].flags =
    (unsigned char) inh_handles->ih[oldfd].flags | KEEP_OPEN_IN_CHILD;
  return 0;
}

/* Performs the remaining delayed dup2 (oldfd, newfd).
   Returns 0 upon success.  In case of failure, -1 is returned, with errno set.
 */
static int
do_remaining_delayed_dup2 (struct inheritable_handles *inh_handles,
                           HANDLE curr_process)
{
  size_t handles_count = inh_handles->count;
  int newfd;

  for (newfd = 0; newfd < handles_count; newfd++)
    if (inh_handles->ih[newfd].handle != INVALID_HANDLE_VALUE
        && (inh_handles->ih[newfd].flags & DELAYED_DUP2_NEWFD) != 0)
      if (do_delayed_dup2 (newfd, inh_handles, curr_process) < 0)
        return -1;
  return 0;
}

# endif

/* Closes the handles in inh_handles that are not meant to be preserved in the
   child process, and reduces inh_handles->count to the minimum needed.  */
static void
shrink_inheritable_handles (struct inheritable_handles *inh_handles)
{
  struct IHANDLE *ih = inh_handles->ih;
  size_t handles_count = inh_handles->count;
  unsigned int fd;

  for (fd = 0; fd < handles_count; fd++)
    {
      HANDLE handle = ih[fd].handle;

      if (handle != INVALID_HANDLE_VALUE
          && (ih[fd].flags & KEEP_OPEN_IN_CHILD) == 0)
        {
          if (!(ih[fd].flags & KEEP_OPEN_IN_PARENT))
            CloseHandle (handle);
          ih[fd].handle = INVALID_HANDLE_VALUE;
        }
    }

  while (handles_count > 3
         && ih[handles_count - 1].handle == INVALID_HANDLE_VALUE)
    handles_count--;

  inh_handles->count = handles_count;
}

/* Closes all handles in inh_handles.  */
static void
close_inheritable_handles (struct inheritable_handles *inh_handles)
{
  struct IHANDLE *ih = inh_handles->ih;
  size_t handles_count = inh_handles->count;
  unsigned int fd;

  for (fd = 0; fd < handles_count; fd++)
    {
      HANDLE handle = ih[fd].handle;

      if (handle != INVALID_HANDLE_VALUE
          && !(ih[fd].flags & DELAYED_DUP2_NEWFD)
          && !(ih[fd].flags & KEEP_OPEN_IN_PARENT))
        CloseHandle (handle);
    }
}

/* Tests whether a memory region, starting at P and N bytes long, contains only
   zeroes.  */
static bool
memiszero (const void *p, size_t n)
{
  const char *cp = p;
  for (; n > 0; cp++, n--)
    if (*cp != 0)
      return 0;
  return 1;
}

/* Tests whether *S contains no signals.  */
static bool
sigisempty (const sigset_t *s)
{
  return memiszero (s, sizeof (sigset_t));
}

/* Executes a 'close' action.
   Returns 0 upon success.  In case of failure, -1 is returned, with errno set.
 */
static int
do_close (struct inheritable_handles *inh_handles, int fd, bool ignore_EBADF)
{
  if (!(fd >= 0 && fd < inh_handles->count
        && inh_handles->ih[fd].handle != INVALID_HANDLE_VALUE))
    {
      if (ignore_EBADF)
        return 0;
      else
        {
          errno = EBADF;
          return -1;
        }
    }

# if SPAWN_INTERNAL_OPTIMIZE_DUPLICATEHANDLE
  if ((inh_handles->ih[fd].flags & DELAYED_DUP2_NEWFD) != 0)
    {
      int dup2_oldfd = inh_handles->ih[fd].linked_fd;
      /* Check invariants.  */
      if (!((inh_handles->ih[dup2_oldfd].flags & DELAYED_DUP2_OLDFD) != 0
            && fd == inh_handles->ih[dup2_oldfd].linked_fd
            && inh_handles->ih[fd].handle == inh_handles->ih[dup2_oldfd].handle))
        abort ();
      /* Annihilate a delayed dup2 (..., fd) call.  */
      inh_handles->ih[dup2_oldfd].flags &= ~DELAYED_DUP2_OLDFD;
    }
  else if ((inh_handles->ih[fd].flags & DELAYED_DUP2_OLDFD) != 0)
    {
      int dup2_newfd = inh_handles->ih[fd].linked_fd;
      /* Check invariants.  */
      if (!((inh_handles->ih[dup2_newfd].flags & DELAYED_DUP2_NEWFD) != 0
            && fd == inh_handles->ih[dup2_newfd].linked_fd
            && inh_handles->ih[fd].handle == inh_handles->ih[dup2_newfd].handle))
        abort ();
      /* Optimize a delayed dup2 (fd, ...) call.  */
      inh_handles->ih[dup2_newfd].flags =
        (inh_handles->ih[fd].flags & ~DELAYED_DUP2_OLDFD) | KEEP_OPEN_IN_CHILD;
    }
  else
# endif
    {
      if (!(inh_handles->ih[fd].flags & KEEP_OPEN_IN_PARENT)
          && !CloseHandle (inh_handles->ih[fd].handle))
        {
          inh_handles->ih[fd].handle = INVALID_HANDLE_VALUE;
          errno = EIO;
          return -1;
        }
    }
  inh_handles->ih[fd].handle = INVALID_HANDLE_VALUE;

  return 0;
}

/* Opens an inheritable HANDLE to a file.
   Upon failure, returns INVALID_HANDLE_VALUE with errno set.  */
static HANDLE
open_handle (const char *name, int flags, mode_t mode)
{
  /* To ease portability.  Like in open.c.  */
  if (strcmp (name, "/dev/null") == 0)
    name = "NUL";

  /* POSIX <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_13>
     specifies: "More than two leading <slash> characters shall be treated as
     a single <slash> character."  */
  if (ISSLASH (name[0]) && ISSLASH (name[1]) && ISSLASH (name[2]))
    {
      name += 2;
      while (ISSLASH (name[1]))
        name++;
    }

  size_t len = strlen (name);
  size_t drive_prefix_len = (HAS_DEVICE (name) ? 2 : 0);

  /* Remove trailing slashes (except the very first one, at position
     drive_prefix_len), but remember their presence.  */
  size_t rlen;
  bool check_dir = false;

  rlen = len;
  while (rlen > drive_prefix_len && ISSLASH (name[rlen-1]))
    {
      check_dir = true;
      if (rlen == drive_prefix_len + 1)
        break;
      rlen--;
    }

  /* Handle '' and 'C:'.  */
  if (!check_dir && rlen == drive_prefix_len)
    {
      errno = ENOENT;
      return INVALID_HANDLE_VALUE;
    }

  /* Handle '\\'.  */
  if (rlen == 1 && ISSLASH (name[0]) && len >= 2)
    {
      errno = ENOENT;
      return INVALID_HANDLE_VALUE;
    }

  const char *rname;
  char *malloca_rname;
  if (rlen == len)
    {
      rname = name;
      malloca_rname = NULL;
    }
  else
    {
      malloca_rname = malloca (rlen + 1);
      if (malloca_rname == NULL)
        {
          errno = ENOMEM;
          return INVALID_HANDLE_VALUE;
        }
      memcpy (malloca_rname, name, rlen);
      malloca_rname[rlen] = '\0';
      rname = malloca_rname;
    }

  /* For the meaning of the flags, see
     <https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/open-wopen>  */
  /* Open a handle to the file.
     CreateFile
     <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea>
     <https://docs.microsoft.com/en-us/windows/desktop/FileIO/creating-and-opening-files>  */
  SECURITY_ATTRIBUTES sec_attr;
  sec_attr.nLength = sizeof (SECURITY_ATTRIBUTES);
  sec_attr.lpSecurityDescriptor = NULL;
  sec_attr.bInheritHandle = TRUE;
  HANDLE handle =
    CreateFile (rname,
                ((flags & (O_WRONLY | O_RDWR)) != 0
                 ? GENERIC_READ | GENERIC_WRITE
                 : GENERIC_READ),
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                &sec_attr,
                ((flags & O_CREAT) != 0
                 ? ((flags & O_EXCL) != 0
                    ? CREATE_NEW
                    : ((flags & O_TRUNC) != 0 ? CREATE_ALWAYS : OPEN_ALWAYS))
                 : ((flags & O_TRUNC) != 0
                    ? TRUNCATE_EXISTING
                    : OPEN_EXISTING)),
                /* FILE_FLAG_BACKUP_SEMANTICS is useful for opening directories,
                   which is out-of-scope here.  */
                /* FILE_FLAG_POSIX_SEMANTICS (treat file names that differ only
                   in case as different) makes sense only when applied to *all*
                   filesystem operations.  */
                /* FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_POSIX_SEMANTICS */
                FILE_ATTRIBUTE_NORMAL
                | ((flags & O_TEMPORARY) != 0 ? FILE_FLAG_DELETE_ON_CLOSE : 0)
                | ((flags & O_SEQUENTIAL ) != 0 ? FILE_FLAG_SEQUENTIAL_SCAN : 0)
                | ((flags & O_RANDOM) != 0 ? FILE_FLAG_RANDOM_ACCESS : 0),
                NULL);
  if (handle == INVALID_HANDLE_VALUE)
    switch (GetLastError ())
      {
      /* Some of these errors probably cannot happen with the specific flags
         that we pass to CreateFile.  But who knows...  */
      case ERROR_FILE_NOT_FOUND: /* The last component of rname does not exist.  */
      case ERROR_PATH_NOT_FOUND: /* Some directory component in rname does not exist.  */
      case ERROR_BAD_PATHNAME:   /* rname is such as '\\server'.  */
      case ERROR_BAD_NETPATH:    /* rname is such as '\\nonexistentserver\share'.  */
      case ERROR_BAD_NET_NAME:   /* rname is such as '\\server\nonexistentshare'.  */
      case ERROR_INVALID_NAME:   /* rname contains wildcards, misplaced colon, etc.  */
      case ERROR_DIRECTORY:
        errno = ENOENT;
        break;

      case ERROR_ACCESS_DENIED:  /* rname is such as 'C:\System Volume Information\foo'.  */
      case ERROR_SHARING_VIOLATION: /* rname is such as 'C:\pagefile.sys'.  */
                                    /* XXX map to EACCES or EPERM? */
        errno = EACCES;
        break;

      case ERROR_OUTOFMEMORY:
        errno = ENOMEM;
        break;

      case ERROR_WRITE_PROTECT:
        errno = EROFS;
        break;

      case ERROR_WRITE_FAULT:
      case ERROR_READ_FAULT:
      case ERROR_GEN_FAILURE:
        errno = EIO;
        break;

      case ERROR_BUFFER_OVERFLOW:
      case ERROR_FILENAME_EXCED_RANGE:
        errno = ENAMETOOLONG;
        break;

      case ERROR_DELETE_PENDING: /* XXX map to EACCES or EPERM? */
        errno = EPERM;
        break;

      default:
        errno = EINVAL;
        break;
      }

  if (malloca_rname != NULL)
    {
      int saved_errno = errno;
      freea (malloca_rname);
      errno = saved_errno;
    }
  return handle;
}

/* Executes an 'open' action.
   Returns 0 upon success.  In case of failure, -1 is returned, with errno set.
 */
static int
do_open (struct inheritable_handles *inh_handles, int newfd,
         const char *filename, const char *directory,
         int flags, mode_t mode)
{
  if (!(newfd >= 0 && newfd < _getmaxstdio ()))
    {
      errno = EBADF;
      return -1;
    }
  if (grow_inheritable_handles (inh_handles, newfd) < 0)
    return -1;
  if (do_close (inh_handles, newfd, true) < 0)
    return -1;
  if (filename == NULL)
    {
      errno = EINVAL;
      return -1;
    }
  char *filename_to_free = NULL;
  if (directory != NULL && IS_RELATIVE_FILE_NAME (filename))
    {
      char *real_filename = concatenated_filename (directory, filename, NULL);
      if (real_filename == NULL)
        {
          errno = ENOMEM;
          return -1;
        }
      filename = real_filename;
      filename_to_free = real_filename;
    }
  HANDLE handle = open_handle (filename, flags, mode);
  if (handle == INVALID_HANDLE_VALUE)
    {
      free (filename_to_free);
      return -1;
    }
  free (filename_to_free);
  inh_handles->ih[newfd].handle = handle;
  inh_handles->ih[newfd].flags =
    ((flags & O_APPEND) != 0 ? 32 : 0) | KEEP_OPEN_IN_CHILD;
  return 0;
}

/* Executes a 'dup2' action.
   Returns 0 upon success.  In case of failure, -1 is returned, with errno set.
 */
static int
do_dup2 (struct inheritable_handles *inh_handles, int oldfd, int newfd,
         HANDLE curr_process)
{
  if (!(oldfd >= 0 && oldfd < inh_handles->count
        && inh_handles->ih[oldfd].handle != INVALID_HANDLE_VALUE))
    {
      errno = EBADF;
      return -1;
    }
  if (!(newfd >= 0 && newfd < _getmaxstdio ()))
    {
      errno = EBADF;
      return -1;
    }
  if (newfd != oldfd)
    {
      if (grow_inheritable_handles (inh_handles, newfd) < 0)
        return -1;
      if (do_close (inh_handles, newfd, true) < 0)
        return -1;
      /* We may need to duplicate the handle, so that a forthcoming do_close
         action on oldfd has no effect on newfd.  */
# if SPAWN_INTERNAL_OPTIMIZE_DUPLICATEHANDLE
      /* But try to not do it now; delay it if possible.  In many cases, the
         DuplicateHandle call can be optimized away.  */
      if ((inh_handles->ih[oldfd].flags & DELAYED_DUP2_NEWFD) != 0)
        if (do_delayed_dup2 (oldfd, inh_handles, curr_process) < 0)
          return -1;
      if ((inh_handles->ih[oldfd].flags & DELAYED_DUP2_OLDFD) != 0)
        {
          /* Check invariants.  */
          int dup2_newfd = inh_handles->ih[oldfd].linked_fd;
          if (!((inh_handles->ih[dup2_newfd].flags & DELAYED_DUP2_NEWFD) != 0
                && oldfd == inh_handles->ih[dup2_newfd].linked_fd
                && inh_handles->ih[oldfd].handle == inh_handles->ih[dup2_newfd].handle))
            abort ();
          /* We can't delay two or more dup2 calls from the same oldfd.  */
          if (!DuplicateHandle (curr_process, inh_handles->ih[oldfd].handle,
                                curr_process, &inh_handles->ih[newfd].handle,
                                0, TRUE, DUPLICATE_SAME_ACCESS))
            {
              errno = EBADF; /* arbitrary */
              return -1;
            }
          inh_handles->ih[newfd].flags =
            (unsigned char) inh_handles->ih[oldfd].flags | KEEP_OPEN_IN_CHILD;
        }
      else
        {
          /* Delay the dup2 (oldfd, newfd) action.  */
          inh_handles->ih[oldfd].flags |= DELAYED_DUP2_OLDFD;
          inh_handles->ih[oldfd].linked_fd = newfd;
          inh_handles->ih[newfd].handle = inh_handles->ih[oldfd].handle;
          inh_handles->ih[newfd].flags = DELAYED_DUP2_NEWFD;
          inh_handles->ih[newfd].linked_fd = oldfd;
        }
# else
      if (!DuplicateHandle (curr_process, inh_handles->ih[oldfd].handle,
                            curr_process, &inh_handles->ih[newfd].handle,
                            0, TRUE, DUPLICATE_SAME_ACCESS))
        {
          errno = EBADF; /* arbitrary */
          return -1;
        }
      inh_handles->ih[newfd].flags =
        (unsigned char) inh_handles->ih[oldfd].flags | KEEP_OPEN_IN_CHILD;
# endif
    }
  return 0;
}

int
__spawni (pid_t *pid, const char *prog_filename,
          const posix_spawn_file_actions_t *file_actions,
          const posix_spawnattr_t *attrp, const char *const prog_argv[],
          const char *const envp[], int use_path)
{
  /* Validate the arguments.  */
  if (prog_filename == NULL
      || (attrp != NULL
          && ((attrp->_flags & ~POSIX_SPAWN_SETPGROUP) != 0
              || attrp->_pgrp != 0
              || ! sigisempty (&attrp->_sd)
              || ! sigisempty (&attrp->_ss)
              || attrp->_sp.sched_priority != 0
              || attrp->_policy != 0)))
    return EINVAL;

  /* Process group handling:
     Native Windows does not have the concept of process group, but it has the
     concept of a console attached to a process.
     So, we interpret the three cases as follows:
       - Flag POSIX_SPAWN_SETPGROUP not set: Means, the child process is in the
         same process group as the parent process.  We interpret this as a
         request to reuse the same console.
       - Flag POSIX_SPAWN_SETPGROUP set with attrp->_pgrp == 0: Means the child
         process starts a process group of its own.  See
         <https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_getpgroup.html>
         <https://pubs.opengroup.org/onlinepubs/9699919799/functions/setpgrp.html>
         We interpret this as a request to detach from the current console.
       - Flag POSIX_SPAWN_SETPGROUP set with attrp->_pgrp != 0: Means the child
         process joins another, existing process group.  See
         <https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_getpgroup.html>
         <https://pubs.opengroup.org/onlinepubs/9699919799/functions/setpgid.html>
         We don't support this case; it produces error EINVAL above.  */
  /* <https://docs.microsoft.com/en-us/windows/win32/procthread/process-creation-flags>  */
  DWORD process_creation_flags =
    (attrp != NULL && (attrp->_flags & POSIX_SPAWN_SETPGROUP) != 0 ? DETACHED_PROCESS : 0);

  char *argv_mem_to_free;
  const char **argv = prepare_spawn (prog_argv, &argv_mem_to_free);
  if (argv == NULL)
    return errno; /* errno is set here */
  argv++;

  /* Compose the command.  */
  char *command = compose_command (argv);
  if (command == NULL)
    {
      free (argv_mem_to_free);
      return ENOMEM;
    }

  /* Copy *ENVP into a contiguous block of memory.  */
  char *envblock;
  if (envp == NULL)
    envblock = NULL;
  else
    {
      envblock = compose_envblock (envp);
      if (envblock == NULL)
        {
          free (command);
          free (argv_mem_to_free);
          return ENOMEM;
        }
    }

  /* Set up the array of handles to inherit.
     Duplicate each handle, so that a spawn_do_close action (below) has no
     effect on the file descriptors of the current process.  Alternatively,
     we could store, for each handle, a bit that tells whether it is shared
     with the current process.  But this is simpler.  */
  struct inheritable_handles inh_handles;
  if (init_inheritable_handles (&inh_handles, true) < 0)
    goto failed_1;

  /* Directory in which to execute the new process.  */
  const char *directory = NULL;

  /* Execute the file_actions, modifying the inh_handles instead of the
     file descriptors of the current process.  */
  if (file_actions != NULL)
    {
      HANDLE curr_process = GetCurrentProcess ();
      int cnt;

      for (cnt = 0; cnt < file_actions->_used; ++cnt)
        {
          struct __spawn_action *action = &file_actions->_actions[cnt];

          switch (action->tag)
            {
            case spawn_do_close:
              {
                int fd = action->action.close_action.fd;
                if (do_close (&inh_handles, fd, false) < 0)
                  goto failed_2;
              }
              break;

            case spawn_do_open:
              {
                int newfd = action->action.open_action.fd;
                const char *filename = action->action.open_action.path;
                int flags = action->action.open_action.oflag;
                mode_t mode = action->action.open_action.mode;
                if (do_open (&inh_handles, newfd, filename, directory,
                             flags, mode)
                    < 0)
                  goto failed_2;
              }
              break;

            case spawn_do_dup2:
              {
                int oldfd = action->action.dup2_action.fd;
                int newfd = action->action.dup2_action.newfd;
                if (do_dup2 (&inh_handles, oldfd, newfd, curr_process) < 0)
                  goto failed_2;
              }
              break;

            case spawn_do_chdir:
              {
                char *newdir = action->action.chdir_action.path;
                if (directory != NULL && IS_RELATIVE_FILE_NAME (newdir))
                  {
                    newdir = concatenated_filename (directory, newdir, NULL);
                    if (newdir == NULL)
                      {
                        errno = ENOMEM;
                        goto failed_2;
                      }
                  }
                directory = newdir;
              }
              break;

            case spawn_do_fchdir:
              /* Not supported in this implementation.  */
              errno = EINVAL;
              goto failed_2;
            }
        }

# if SPAWN_INTERNAL_OPTIMIZE_DUPLICATEHANDLE
      /* Do the remaining delayed dup2 invocations.  */
      if (do_remaining_delayed_dup2 (&inh_handles, curr_process) < 0)
        goto failed_2;
# endif
    }

  /* Close the handles in inh_handles that are not meant to be preserved in the
     child process, and reduce inh_handles.count to the minimum needed.  */
  shrink_inheritable_handles (&inh_handles);

  /* CreateProcess
     <https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa>  */
  /* STARTUPINFO
     <https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-startupinfoa>  */
  STARTUPINFO sinfo;
  sinfo.cb = sizeof (STARTUPINFO);
  sinfo.lpReserved = NULL;
  sinfo.lpDesktop = NULL;
  sinfo.lpTitle = NULL;
  if (compose_handles_block (&inh_handles, &sinfo) < 0)
    goto failed_2;

  /* Perform the PATH search now, considering the final DIRECTORY.  */
  char *resolved_prog_filename_to_free = NULL;
  {
    const char *resolved_prog_filename =
      find_in_given_path (prog_filename, use_path ? getenv ("PATH") : "",
                          directory, false);
    if (resolved_prog_filename == NULL)
      goto failed_3;
    if (resolved_prog_filename != prog_filename)
      resolved_prog_filename_to_free = (char *) resolved_prog_filename;
    prog_filename = resolved_prog_filename;
  }

  PROCESS_INFORMATION pinfo;
  if (!CreateProcess (prog_filename, command, NULL, NULL, TRUE,
                      process_creation_flags, envblock, directory, &sinfo,
                      &pinfo))
    {
      DWORD error = GetLastError ();

      free (resolved_prog_filename_to_free);
      free (sinfo.lpReserved2);
      close_inheritable_handles (&inh_handles);
      free_inheritable_handles (&inh_handles);
      free (envblock);
      free (command);
      free (argv_mem_to_free);

      return convert_CreateProcess_error (error);
    }

  if (pinfo.hThread)
    CloseHandle (pinfo.hThread);

  free (resolved_prog_filename_to_free);
  free (sinfo.lpReserved2);
  close_inheritable_handles (&inh_handles);
  free_inheritable_handles (&inh_handles);
  free (envblock);
  free (command);
  free (argv_mem_to_free);

  if (pid != NULL)
    *pid = (intptr_t) pinfo.hProcess;
  return 0;

 failed_3:
  {
    int saved_errno = errno;
    free (sinfo.lpReserved2);
    close_inheritable_handles (&inh_handles);
    free_inheritable_handles (&inh_handles);
    free (envblock);
    free (command);
    free (argv_mem_to_free);
    return saved_errno;
  }

 failed_2:
  {
    int saved_errno = errno;
    close_inheritable_handles (&inh_handles);
    free_inheritable_handles (&inh_handles);
    free (envblock);
    free (command);
    free (argv_mem_to_free);
    return saved_errno;
  }

 failed_1:
  free (envblock);
  free (command);
  free (argv_mem_to_free);
  return errno;
}

#else


/* The warning "warning: 'vfork' is deprecated: Use posix_spawn or fork" seen
   on macOS 12 is pointless, as we use vfork only when it is safe or when the
   user has explicitly requested it.  Silence this warning.  */
#if _GL_GNUC_PREREQ (4, 2)
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS. */
int
__spawni (pid_t *pid, const char *file,
          const posix_spawn_file_actions_t *file_actions,
          const posix_spawnattr_t *attrp, const char *const argv[],
          const char *const envp[], int use_path)
{
  pid_t new_pid;
  char *path, *p, *name;
  size_t len;
  size_t pathlen;

  /* Do this once.  */
  short int flags = attrp == NULL ? 0 : attrp->_flags;

  /* Avoid gcc warning
       "variable 'flags' might be clobbered by 'longjmp' or 'vfork'"  */
  (void) &flags;

  /* Generate the new process.  */
#if HAVE_VFORK
  if ((flags & POSIX_SPAWN_USEVFORK) != 0
      /* If no major work is done, allow using vfork.  Note that we
         might perform the path searching.  But this would be done by
         a call to execvp(), too, and such a call must be OK according
         to POSIX.  */
      || ((flags & (POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF
                    | POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER
                    | POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_RESETIDS)) == 0
          && file_actions == NULL))
    new_pid = vfork ();
  else
#endif
    new_pid = fork ();

  if (new_pid != 0)
    {
      if (new_pid < 0)
        return errno;

      /* The call was successful.  Store the PID if necessary.  */
      if (pid != NULL)
        *pid = new_pid;

      return 0;
    }

  /* Set signal mask.  */
  if ((flags & POSIX_SPAWN_SETSIGMASK) != 0
      && sigprocmask (SIG_SETMASK, &attrp->_ss, NULL) != 0)
    _exit (SPAWN_ERROR);

  /* Set signal default action.  */
  if ((flags & POSIX_SPAWN_SETSIGDEF) != 0)
    {
      /* We have to iterate over all signals.  This could possibly be
         done better but it requires system specific solutions since
         the sigset_t data type can be very different on different
         architectures.  */
      int sig;
      struct sigaction sa;

      memset (&sa, '\0', sizeof (sa));
      sa.sa_handler = SIG_DFL;

      for (sig = 1; sig <= NSIG; ++sig)
        if (sigismember (&attrp->_sd, sig) != 0
            && sigaction (sig, &sa, NULL) != 0)
          _exit (SPAWN_ERROR);

    }

#if (_LIBC ? defined _POSIX_PRIORITY_SCHEDULING : HAVE_SCHED_SETPARAM && HAVE_SCHED_SETSCHEDULER)
  /* Set the scheduling algorithm and parameters.  */
  if ((flags & (POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER))
      == POSIX_SPAWN_SETSCHEDPARAM)
    {
      if (sched_setparam (0, &attrp->_sp) == -1)
        _exit (SPAWN_ERROR);
    }
  else if ((flags & POSIX_SPAWN_SETSCHEDULER) != 0)
    {
      if (sched_setscheduler (0, attrp->_policy,
                              (flags & POSIX_SPAWN_SETSCHEDPARAM) != 0
                              ? &attrp->_sp : NULL) == -1)
        _exit (SPAWN_ERROR);
    }
#endif

  /* Set the process group ID.  */
  if ((flags & POSIX_SPAWN_SETPGROUP) != 0
      && setpgid (0, attrp->_pgrp) != 0)
    _exit (SPAWN_ERROR);

  /* Set the effective user and group IDs.  */
  if ((flags & POSIX_SPAWN_RESETIDS) != 0
      && (local_seteuid (getuid ()) != 0
          || local_setegid (getgid ()) != 0))
    _exit (SPAWN_ERROR);

  /* Execute the file actions.  */
  if (file_actions != NULL)
    {
      int cnt;

      for (cnt = 0; cnt < file_actions->_used; ++cnt)
        {
          struct __spawn_action *action = &file_actions->_actions[cnt];

          switch (action->tag)
            {
            case spawn_do_close:
              if (close_not_cancel (action->action.close_action.fd) != 0)
                /* Signal the error.  */
                _exit (SPAWN_ERROR);
              break;

            case spawn_do_open:
              {
                int new_fd = open_not_cancel (action->action.open_action.path,
                                              action->action.open_action.oflag
                                              | O_LARGEFILE,
                                              action->action.open_action.mode);

                if (new_fd == -1)
                  /* The 'open' call failed.  */
                  _exit (SPAWN_ERROR);

                /* Make sure the desired file descriptor is used.  */
                if (new_fd != action->action.open_action.fd)
                  {
                    if (dup2 (new_fd, action->action.open_action.fd)
                        != action->action.open_action.fd)
                      /* The 'dup2' call failed.  */
                      _exit (SPAWN_ERROR);

                    if (close_not_cancel (new_fd) != 0)
                      /* The 'close' call failed.  */
                      _exit (SPAWN_ERROR);
                  }
              }
              break;

            case spawn_do_dup2:
              if (dup2 (action->action.dup2_action.fd,
                        action->action.dup2_action.newfd)
                  != action->action.dup2_action.newfd)
                /* The 'dup2' call failed.  */
                _exit (SPAWN_ERROR);
              break;

            case spawn_do_chdir:
              if (chdir (action->action.chdir_action.path) < 0)
                /* The 'chdir' call failed.  */
                _exit (SPAWN_ERROR);
              break;

            case spawn_do_fchdir:
              if (fchdir (action->action.fchdir_action.fd) < 0)
                /* The 'fchdir' call failed.  */
                _exit (SPAWN_ERROR);
              break;
            }
        }
    }

  if (! use_path || strchr (file, '/') != NULL)
    {
      /* The FILE parameter is actually a path.  */
      execve (file, (char * const *) argv, (char * const *) envp);

      /* Oh, oh.  'execve' returns.  This is bad.  */
      _exit (SPAWN_ERROR);
    }

  /* We have to search for FILE on the path.  */
  path = getenv ("PATH");
  if (path == NULL)
    {
#if HAVE_CONFSTR
      /* There is no 'PATH' in the environment.
         The default search path is the current directory
         followed by the path 'confstr' returns for '_CS_PATH'.  */
      len = confstr (_CS_PATH, (char *) NULL, 0);
      path = (char *) alloca (1 + len);
      path[0] = ':';
      (void) confstr (_CS_PATH, path + 1, len);
#else
      /* Pretend that the PATH contains only the current directory.  */
      path = "";
#endif
    }

  len = strlen (file) + 1;
  pathlen = strlen (path);
  name = alloca (pathlen + len + 1);
  /* Copy the file name at the top.  */
  name = (char *) memcpy (name + pathlen + 1, file, len);
  /* And add the slash.  */
  *--name = '/';

  p = path;
  do
    {
      char *startp;

      path = p;
      p = strchrnul (path, ':');

      if (p == path)
        /* Two adjacent colons, or a colon at the beginning or the end
           of 'PATH' means to search the current directory.  */
        startp = name + 1;
      else
        startp = (char *) memcpy (name - (p - path), path, p - path);

      /* Try to execute this name.  If it works, execv will not return.  */
      execve (startp, (char * const *) argv, (char * const *) envp);

      switch (errno)
        {
        case EACCES:
        case ENOENT:
        case ESTALE:
        case ENOTDIR:
          /* Those errors indicate the file is missing or not executable
             by us, in which case we want to just try the next path
             directory.  */
          break;

        default:
          /* Some other error means we found an executable file, but
             something went wrong executing it; return the error to our
             caller.  */
          _exit (SPAWN_ERROR);
        }
    }
  while (*p++ != '\0');

  /* Return with an error.  */
  _exit (SPAWN_ERROR);
}

#endif
