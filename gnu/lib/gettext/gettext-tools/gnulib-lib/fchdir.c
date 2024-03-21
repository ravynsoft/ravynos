/* fchdir replacement.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <unistd.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "assure.h"
#include "filename.h"
#include "filenamecat.h"

#ifndef REPLACE_OPEN_DIRECTORY
# define REPLACE_OPEN_DIRECTORY 0
#endif

/* This replacement assumes that a directory is not renamed while opened
   through a file descriptor.

   FIXME: On mingw, this would be possible to enforce if we were to
   also open a HANDLE to each directory currently visited by a file
   descriptor, since mingw refuses to rename any in-use file system
   object.  */

/* Array of file descriptors opened.  If REPLACE_OPEN_DIRECTORY or if it points
   to a directory, it stores info about this directory.  */
typedef struct
{
  char *name;       /* Absolute name of the directory, or NULL.  */
} dir_info_t;
static dir_info_t *dirs;
static size_t dirs_allocated;

/* Try to ensure dirs has enough room for a slot at index fd; free any
   contents already in that slot.  Return false and set errno to
   ENOMEM on allocation failure.  */
static bool
ensure_dirs_slot (size_t fd)
{
  if (fd < dirs_allocated)
    free (dirs[fd].name);
  else
    {
      size_t new_allocated;
      dir_info_t *new_dirs;

      new_allocated = 2 * dirs_allocated + 1;
      if (new_allocated <= fd)
        new_allocated = fd + 1;
      new_dirs =
        (dirs != NULL
         ? (dir_info_t *) realloc (dirs, new_allocated * sizeof *dirs)
         : (dir_info_t *) malloc (new_allocated * sizeof *dirs));
      if (new_dirs == NULL)
        return false;
      memset (new_dirs + dirs_allocated, 0,
              (new_allocated - dirs_allocated) * sizeof *dirs);
      dirs = new_dirs;
      dirs_allocated = new_allocated;
    }
  return true;
}

/* Return an absolute name of DIR in malloc'd storage.
   Upon failure, return NULL with errno set.  */
static char *
get_name (char const *dir)
{
  char *cwd;
  char *result;

  if (IS_ABSOLUTE_FILE_NAME (dir))
    return strdup (dir);

  /* We often encounter "."; treat it as a special case.  */
  cwd = getcwd (NULL, 0);
  if (!cwd || (dir[0] == '.' && dir[1] == '\0'))
    return cwd;

  result = mfile_name_concat (cwd, dir, NULL);
  free (cwd);
  return result;
}

/* Hook into the gnulib replacements for open() and close() to keep track
   of the open file descriptors.  */

/* Close FD, cleaning up any fd to name mapping if fd was visiting a
   directory.  */
void
_gl_unregister_fd (int fd)
{
  if (fd >= 0 && fd < dirs_allocated)
    {
      free (dirs[fd].name);
      dirs[fd].name = NULL;
    }
}

/* Mark FD as visiting FILENAME.  FD must be non-negative, and refer
   to an open file descriptor.  If REPLACE_OPEN_DIRECTORY is non-zero,
   this should only be called if FD is visiting a directory.  Close FD
   and return -1 with errno set if there is insufficient memory to track
   the directory name; otherwise return FD.  */
int
_gl_register_fd (int fd, const char *filename)
{
  struct stat statbuf;

  assure (0 <= fd);
  if (REPLACE_OPEN_DIRECTORY
      || (fstat (fd, &statbuf) == 0 && S_ISDIR (statbuf.st_mode)))
    {
      if (!ensure_dirs_slot (fd)
          || (dirs[fd].name = get_name (filename)) == NULL)
        {
          int saved_errno = errno;
          close (fd);
          errno = saved_errno;
          return -1;
        }
    }
  return fd;
}

/* Mark NEWFD as a duplicate of OLDFD; useful from dup, dup2, dup3,
   and fcntl.  Both arguments must be valid and distinct file
   descriptors.  Close NEWFD and return -1 if OLDFD is tracking a
   directory, but there is insufficient memory to track the same
   directory in NEWFD; otherwise return NEWFD.  */
int
_gl_register_dup (int oldfd, int newfd)
{
  assure (0 <= oldfd && 0 <= newfd && oldfd != newfd);
  if (oldfd < dirs_allocated && dirs[oldfd].name)
    {
      /* Duplicated a directory; must ensure newfd is allocated.  */
      if (!ensure_dirs_slot (newfd)
          || (dirs[newfd].name = strdup (dirs[oldfd].name)) == NULL)
        {
          int saved_errno = errno;
          close (newfd);
          errno = saved_errno;
          newfd = -1;
        }
    }
  else if (newfd < dirs_allocated)
    {
      /* Duplicated a non-directory; ensure newfd is cleared.  */
      free (dirs[newfd].name);
      dirs[newfd].name = NULL;
    }
  return newfd;
}

/* If FD is currently visiting a directory, then return the name of
   that directory.  Otherwise, return NULL and set errno.  */
const char *
_gl_directory_name (int fd)
{
  if (0 <= fd && fd < dirs_allocated && dirs[fd].name != NULL)
    return dirs[fd].name;
  /* At this point, fd is either invalid, or open but not a directory.
     If dup2 fails, errno is correctly EBADF.  */
  if (0 <= fd)
    {
      if (dup2 (fd, fd) == fd)
        errno = ENOTDIR;
    }
  else
    errno = EBADF;
  return NULL;
}


/* Implement fchdir() in terms of chdir().  */

int
fchdir (int fd)
{
  const char *name = _gl_directory_name (fd);
  return name ? chdir (name) : -1;
}
