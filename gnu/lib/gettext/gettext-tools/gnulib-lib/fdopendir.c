/* provide a replacement fdopendir function
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

#include <config.h>

#include <dirent.h>

#include <stdlib.h>
#include <unistd.h>

#if !HAVE_FDOPENDIR

# if GNULIB_defined_DIR
/* We are in control of the file descriptor of a DIR.  */

#  include "dirent-private.h"

#  if !REPLACE_FCHDIR
#   error "unexpected configuration: GNULIB_defined_DIR but fchdir not replaced"
#  endif

DIR *
fdopendir (int fd)
{
  char const *name = _gl_directory_name (fd);
  DIR *dirp = name ? opendir (name) : NULL;
  if (dirp != NULL)
    dirp->fd_to_close = fd;
  return dirp;
}

# else
/* We are not in control of the file descriptor of a DIR, and therefore have to
   play tricks with file descriptors before and after a call to opendir().  */

#  include "openat.h"
#  include "openat-priv.h"
#  include "save-cwd.h"

#  if GNULIB_DIRENT_SAFER
#   include "dirent--.h"
#  endif

#  ifndef REPLACE_FCHDIR
#   define REPLACE_FCHDIR 0
#  endif

static DIR *fdopendir_with_dup (int, int, struct saved_cwd const *);
static DIR *fd_clone_opendir (int, struct saved_cwd const *);

/* Replacement for POSIX fdopendir.

   First, try to simulate it via opendir ("/proc/self/fd/...").  Failing
   that, simulate it by using fchdir metadata, or by doing
   save_cwd/fchdir/opendir(".")/restore_cwd.
   If either the save_cwd or the restore_cwd fails (relatively unlikely),
   then give a diagnostic and exit nonzero.

   If successful, the resulting stream is based on FD in
   implementations where streams are based on file descriptors and in
   applications where no other thread or signal handler allocates or
   frees file descriptors.  In other cases, consult dirfd on the result
   to find out whether FD is still being used.

   Otherwise, this function works just like POSIX fdopendir.

   W A R N I N G:

   Unlike other fd-related functions, this one places constraints on FD.
   If this function returns successfully, FD is under control of the
   dirent.h system, and the caller should not close or modify the state of
   FD other than by the dirent.h functions.  */
DIR *
fdopendir (int fd)
{
  DIR *dir = fdopendir_with_dup (fd, -1, NULL);

  if (! REPLACE_FCHDIR && ! dir)
    {
      int saved_errno = errno;
      if (EXPECTED_ERRNO (saved_errno))
        {
          struct saved_cwd cwd;
          if (save_cwd (&cwd) != 0)
            openat_save_fail (errno);
          dir = fdopendir_with_dup (fd, -1, &cwd);
          saved_errno = errno;
          free_cwd (&cwd);
          errno = saved_errno;
        }
    }

  return dir;
}

/* Like fdopendir, except that if OLDER_DUPFD is not -1, it is known
   to be a dup of FD which is less than FD - 1 and which will be
   closed by the caller and not otherwise used by the caller.  This
   function makes sure that FD is closed and all file descriptors less
   than FD are open, and then calls fd_clone_opendir on a dup of FD.
   That way, barring race conditions, fd_clone_opendir returns a
   stream whose file descriptor is FD.

   If REPLACE_FCHDIR or CWD is null, use opendir ("/proc/self/fd/...",
   falling back on fchdir metadata.  Otherwise, CWD is a saved version
   of the working directory; use fchdir/opendir(".")/restore_cwd(CWD).  */
static DIR *
fdopendir_with_dup (int fd, int older_dupfd, struct saved_cwd const *cwd)
{
  int dupfd = dup (fd);
  if (dupfd < 0 && errno == EMFILE)
    dupfd = older_dupfd;
  if (dupfd < 0)
    return NULL;
  else
    {
      DIR *dir;
      int saved_errno;
      if (dupfd < fd - 1 && dupfd != older_dupfd)
        {
          dir = fdopendir_with_dup (fd, dupfd, cwd);
          saved_errno = errno;
        }
      else
        {
          close (fd);
          dir = fd_clone_opendir (dupfd, cwd);
          saved_errno = errno;
          if (! dir)
            {
              int fd1 = dup (dupfd);
              if (fd1 != fd)
                openat_save_fail (fd1 < 0 ? errno : EBADF);
            }
        }

      if (dupfd != older_dupfd)
        close (dupfd);
      errno = saved_errno;
      return dir;
    }
}

/* Like fdopendir, except the result controls a clone of FD.  It is
   the caller's responsibility both to close FD and (if the result is
   not null) to closedir the result.  */
static DIR *
fd_clone_opendir (int fd, struct saved_cwd const *cwd)
{
  if (REPLACE_FCHDIR || ! cwd)
    {
      DIR *dir = NULL;
      int saved_errno = EOPNOTSUPP;
      char buf[OPENAT_BUFFER_SIZE];
      char *proc_file = openat_proc_name (buf, fd, ".");
      if (proc_file)
        {
          dir = opendir (proc_file);
          saved_errno = errno;
          if (proc_file != buf)
            free (proc_file);
        }
#  if REPLACE_FCHDIR
      if (! dir && EXPECTED_ERRNO (saved_errno))
        {
          char const *name = _gl_directory_name (fd);
          DIR *dp = name ? opendir (name) : NULL;

          /* The caller has done an elaborate dance to arrange for opendir to
             consume just the right file descriptor.  If dirfd returns -1,
             though, we're on a system like mingw where opendir does not
             consume a file descriptor.  Consume it via 'dup' instead.  */
          if (dp && dirfd (dp) < 0)
            dup (fd);

          return dp;
        }
#  endif
      errno = saved_errno;
      return dir;
    }
  else
    {
      if (fchdir (fd) != 0)
        return NULL;
      else
        {
          DIR *dir = opendir (".");
          int saved_errno = errno;
          if (restore_cwd (cwd) != 0)
            openat_restore_fail (errno);
          errno = saved_errno;
          return dir;
        }
    }
}

# endif

#else /* HAVE_FDOPENDIR */

# include <errno.h>
# include <sys/stat.h>

# undef fdopendir

/* Like fdopendir, but work around GNU/Hurd bug by validating FD.  */

DIR *
rpl_fdopendir (int fd)
{
  struct stat st;
  if (fstat (fd, &st))
    return NULL;
  if (!S_ISDIR (st.st_mode))
    {
      errno = ENOTDIR;
      return NULL;
    }
  return fdopendir (fd);
}

#endif /* HAVE_FDOPENDIR */
