/* provide a replacement openat function
   Copyright (C) 2004-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* written by Jim Meyering */

/* If the user's config.h happens to include <fcntl.h>, let it include only
   the system's <fcntl.h> here, so that orig_openat doesn't recurse to
   rpl_openat.  */
#define __need_system_fcntl_h
#include <config.h>

/* Get the original definition of open.  It might be defined as a macro.  */
#include <fcntl.h>
#include <sys/types.h>
#undef __need_system_fcntl_h

#if HAVE_OPENAT
static int
orig_openat (int fd, char const *filename, int flags, mode_t mode)
{
  return openat (fd, filename, flags, mode);
}
#endif

/* Write "fcntl.h" here, not <fcntl.h>, otherwise OSF/1 5.1 DTK cc eliminates
   this include because of the preliminary #include <fcntl.h> above.  */
#include "fcntl.h"

#include "openat.h"

#include "cloexec.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#if HAVE_OPENAT

/* Like openat, but support O_CLOEXEC and work around Solaris 9 bugs
   with trailing slash.  */
int
rpl_openat (int dfd, char const *filename, int flags, ...)
{
  /* 0 = unknown, 1 = yes, -1 = no.  */
#if GNULIB_defined_O_CLOEXEC
  int have_cloexec = -1;
#else
  static int have_cloexec;
#endif

  mode_t mode;
  int fd;

  mode = 0;
  if (flags & O_CREAT)
    {
      va_list arg;
      va_start (arg, flags);

      /* We have to use PROMOTED_MODE_T instead of mode_t, otherwise GCC 4
         creates crashing code when 'mode_t' is smaller than 'int'.  */
      mode = va_arg (arg, PROMOTED_MODE_T);

      va_end (arg);
    }

# if OPEN_TRAILING_SLASH_BUG
  /* Fail if one of O_CREAT, O_WRONLY, O_RDWR is specified and the filename
     ends in a slash, as POSIX says such a filename must name a directory
     <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_13>:
       "A pathname that contains at least one non-<slash> character and that
        ends with one or more trailing <slash> characters shall not be resolved
        successfully unless the last pathname component before the trailing
        <slash> characters names an existing directory"
     If the named file already exists as a directory, then
       - if O_CREAT is specified, open() must fail because of the semantics
         of O_CREAT,
       - if O_WRONLY or O_RDWR is specified, open() must fail because POSIX
         <https://pubs.opengroup.org/onlinepubs/9699919799/functions/openat.html>
         says that it fails with errno = EISDIR in this case.
     If the named file does not exist or does not name a directory, then
       - if O_CREAT is specified, open() must fail since open() cannot create
         directories,
       - if O_WRONLY or O_RDWR is specified, open() must fail because the
         file does not contain a '.' directory.  */
  if ((flags & O_CREAT)
      || (flags & O_ACCMODE) == O_RDWR
      || (flags & O_ACCMODE) == O_WRONLY)
    {
      size_t len = strlen (filename);
      if (len > 0 && filename[len - 1] == '/')
        {
          errno = EISDIR;
          return -1;
        }
    }
# endif

  fd = orig_openat (dfd, filename,
                    flags & ~(have_cloexec < 0 ? O_CLOEXEC : 0), mode);

  if (flags & O_CLOEXEC)
    {
      if (! have_cloexec)
        {
          if (0 <= fd)
            have_cloexec = 1;
          else if (errno == EINVAL)
            {
              fd = orig_openat (dfd, filename, flags & ~O_CLOEXEC, mode);
              have_cloexec = -1;
            }
        }
      if (have_cloexec < 0 && 0 <= fd)
        set_cloexec_flag (fd, true);
    }


# if OPEN_TRAILING_SLASH_BUG
  /* If the filename ends in a slash and fd does not refer to a directory,
     then fail.
     Rationale: POSIX says such a filename must name a directory
     <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_13>:
       "A pathname that contains at least one non-<slash> character and that
        ends with one or more trailing <slash> characters shall not be resolved
        successfully unless the last pathname component before the trailing
        <slash> characters names an existing directory"
     If the named file without the slash is not a directory, open() must fail
     with ENOTDIR.  */
  if (fd >= 0)
    {
      /* We know len is positive, since open did not fail with ENOENT.  */
      size_t len = strlen (filename);
      if (filename[len - 1] == '/')
        {
          struct stat statbuf;

          if (fstat (fd, &statbuf) >= 0 && !S_ISDIR (statbuf.st_mode))
            {
              close (fd);
              errno = ENOTDIR;
              return -1;
            }
        }
    }
# endif

  return fd;
}

#else /* !HAVE_OPENAT */

# include "filename.h" /* solely for definition of IS_ABSOLUTE_FILE_NAME */
# include "openat-priv.h"
# include "save-cwd.h"

/* Replacement for Solaris' openat function.
   <https://www.google.com/search?q=openat+site:docs.oracle.com>
   First, try to simulate it via open ("/proc/self/fd/FD/FILE").
   Failing that, simulate it by doing save_cwd/fchdir/open/restore_cwd.
   If either the save_cwd or the restore_cwd fails (relatively unlikely),
   then give a diagnostic and exit nonzero.
   Otherwise, upon failure, set errno and return -1, as openat does.
   Upon successful completion, return a file descriptor.  */
int
openat (int fd, char const *file, int flags, ...)
{
  mode_t mode = 0;

  if (flags & O_CREAT)
    {
      va_list arg;
      va_start (arg, flags);

      /* We have to use PROMOTED_MODE_T instead of mode_t, otherwise GCC 4
         creates crashing code when 'mode_t' is smaller than 'int'.  */
      mode = va_arg (arg, PROMOTED_MODE_T);

      va_end (arg);
    }

  return openat_permissive (fd, file, flags, mode, NULL);
}

/* Like openat (FD, FILE, FLAGS, MODE), but if CWD_ERRNO is
   nonnull, set *CWD_ERRNO to an errno value if unable to save
   or restore the initial working directory.  This is needed only
   the first time remove.c's remove_dir opens a command-line
   directory argument.

   If a previous attempt to restore the current working directory
   failed, then we must not even try to access a '.'-relative name.
   It is the caller's responsibility not to call this function
   in that case.  */

int
openat_permissive (int fd, char const *file, int flags, mode_t mode,
                   int *cwd_errno)
{
  struct saved_cwd saved_cwd;
  int saved_errno;
  int err;
  bool save_ok;

  if (fd == AT_FDCWD || IS_ABSOLUTE_FILE_NAME (file))
    return open (file, flags, mode);

  {
    char buf[OPENAT_BUFFER_SIZE];
    char *proc_file = openat_proc_name (buf, fd, file);
    if (proc_file)
      {
        int open_result = open (proc_file, flags, mode);
        int open_errno = errno;
        if (proc_file != buf)
          free (proc_file);
        /* If the syscall succeeds, or if it fails with an unexpected
           errno value, then return right away.  Otherwise, fall through
           and resort to using save_cwd/restore_cwd.  */
        if (0 <= open_result || ! EXPECTED_ERRNO (open_errno))
          {
            errno = open_errno;
            return open_result;
          }
      }
  }

  save_ok = (save_cwd (&saved_cwd) == 0);
  if (! save_ok)
    {
      if (! cwd_errno)
        openat_save_fail (errno);
      *cwd_errno = errno;
    }
  if (0 <= fd && fd == saved_cwd.desc)
    {
      /* If saving the working directory collides with the user's
         requested fd, then the user's fd must have been closed to
         begin with.  */
      free_cwd (&saved_cwd);
      errno = EBADF;
      return -1;
    }

  err = fchdir (fd);
  saved_errno = errno;

  if (! err)
    {
      err = open (file, flags, mode);
      saved_errno = errno;
      if (save_ok && restore_cwd (&saved_cwd) != 0)
        {
          if (! cwd_errno)
            {
              /* Don't write a message to just-created fd 2.  */
              saved_errno = errno;
              if (err == STDERR_FILENO)
                close (err);
              openat_restore_fail (saved_errno);
            }
          *cwd_errno = errno;
        }
    }

  free_cwd (&saved_cwd);
  errno = saved_errno;
  return err;
}

/* Return true if our openat implementation must resort to
   using save_cwd and restore_cwd.  */
bool
openat_needs_fchdir (void)
{
  bool needs_fchdir = true;
  int fd = open ("/", O_SEARCH | O_CLOEXEC);

  if (0 <= fd)
    {
      char buf[OPENAT_BUFFER_SIZE];
      char *proc_file = openat_proc_name (buf, fd, ".");
      if (proc_file)
        {
          needs_fchdir = false;
          if (proc_file != buf)
            free (proc_file);
        }
      close (fd);
    }

  return needs_fchdir;
}

#endif /* !HAVE_OPENAT */
