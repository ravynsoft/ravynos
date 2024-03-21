/* Open a file, without destroying an old file with the same name.

   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible, 2020.  */

#include <config.h>

/* Specification.  */
#include "supersede.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined _WIN32 && !defined __CYGWIN__
/* A native Windows platform.  */
# define WIN32_LEAN_AND_MEAN  /* avoid including junk */
# include <windows.h>
# include <io.h>
#else
# include <unistd.h>
#endif

#include "canonicalize.h"
#include "clean-temp.h"
#include "ignore-value.h"
#include "stat-time.h"
#include "utimens.h"
#include "acl.h"

#if defined _WIN32 && !defined __CYGWIN__
/* Don't assume that UNICODE is not defined.  */
# undef MoveFileEx
# define MoveFileEx MoveFileExA
#endif

static int
create_temp_file (char *canon_filename, int flags, mode_t mode,
                  struct supersede_final_action *action)
{
  /* Use a temporary file always.  */
  size_t canon_filename_length = strlen (canon_filename);

  /* The temporary file needs to be in the same directory, otherwise the
     final rename may fail.  */
  char *temp_filename = (char *) malloc (canon_filename_length + 7 + 1);
  if (temp_filename == NULL)
    return -1;
  memcpy (temp_filename, canon_filename, canon_filename_length);
  memcpy (temp_filename + canon_filename_length, ".XXXXXX", 7 + 1);

  int fd = gen_register_open_temp (temp_filename, 0, flags, mode);
  if (fd < 0)
    return -1;

  action->final_rename_temp = temp_filename;
  action->final_rename_dest = canon_filename;
  return fd;
}

int
open_supersede (const char *filename, int flags, mode_t mode,
                bool supersede_if_exists, bool supersede_if_does_not_exist,
                struct supersede_final_action *action)
{
  int fd;
  /* Extra flags for existing devices.  */
  int extra_flags =
    #if defined __sun || (defined _WIN32 && !defined __CYGWIN__)
    /* open ("/dev/null", O_TRUNC | O_WRONLY) fails on Solaris zones:
         - with error EINVAL on Illumos, see
           <https://www.illumos.org/issues/13035>,
         - with error EACCES on Solaris 11.3.
       Likewise, open ("NUL", O_TRUNC | O_RDWR) fails with error EINVAL on
       native Windows.
       As a workaround, add the O_CREAT flag, although it ought not to be
       necessary.  */
    O_CREAT;
    #else
    0;
    #endif

#if defined _WIN32 && ! defined __CYGWIN__
  if (strcmp (filename, "/dev/null") == 0)
    filename = "NUL";
#endif

  if (supersede_if_exists)
    {
      if (supersede_if_does_not_exist)
        {
          struct stat statbuf;

          if (stat (filename, &statbuf) >= 0
              && ! S_ISREG (statbuf.st_mode)
              /* The file exists and is possibly a character device, socket, or
                 something like that.  */
              && ((fd = open (filename, flags | extra_flags, mode)) >= 0
                  || errno != ENOENT))
            {
              if (fd >= 0)
                {
                  action->final_rename_temp = NULL;
                  action->final_rename_dest = NULL;
                }
            }
          else
            {
              /* The file does not exist or is a regular file.
                 Use a temporary file.  */
              char *canon_filename =
                canonicalize_filename_mode (filename, CAN_ALL_BUT_LAST);
              if (canon_filename == NULL)
                fd = -1;
              else
                {
                  fd = create_temp_file (canon_filename, flags, mode, action);
                  if (fd < 0)
                    free (canon_filename);
                }
            }
        }
      else
        {
          fd = open (filename, flags | O_CREAT | O_EXCL, mode);
          if (fd >= 0)
            {
              /* The file did not exist.  */
              action->final_rename_temp = NULL;
              action->final_rename_dest = NULL;
            }
          else
            {
              /* The file exists or is a symbolic link to a nonexistent
                 file.  */
              char *canon_filename =
                canonicalize_filename_mode (filename, CAN_ALL_BUT_LAST);
              if (canon_filename == NULL)
                fd = -1;
              else
                {
                  fd = open (canon_filename, flags | O_CREAT | O_EXCL, mode);
                  if (fd >= 0)
                    {
                      /* It was a symbolic link to a nonexistent file.  */
                      free (canon_filename);
                      action->final_rename_temp = NULL;
                      action->final_rename_dest = NULL;
                    }
                  else
                    {
                      /* The file exists.  */
                      struct stat statbuf;

                      if (stat (canon_filename, &statbuf) >= 0
                          && S_ISREG (statbuf.st_mode))
                        {
                          /* It is a regular file.  Use a temporary file.  */
                          fd = create_temp_file (canon_filename, flags, mode,
                                                 action);
                          if (fd < 0)
                            free (canon_filename);
                        }
                      else
                        {
                          /* It is possibly a character device, socket, or
                             something like that.  */
                          fd = open (canon_filename, flags | extra_flags, mode);
                          free (canon_filename);
                          if (fd >= 0)
                            {
                              action->final_rename_temp = NULL;
                              action->final_rename_dest = NULL;
                            }
                        }
                    }
                }
            }
        }
    }
  else
    {
      if (supersede_if_does_not_exist)
        {
          fd = open (filename, flags, mode);
          if (fd >= 0)
            {
              /* The file exists.  */
              action->final_rename_temp = NULL;
              action->final_rename_dest = NULL;
            }
          #if defined __sun || (defined _WIN32 && !defined __CYGWIN__)
          /* See the comment regarding extra_flags, above.  */
          else if (errno == EINVAL || errno == EACCES)
            {
              struct stat statbuf;

              if (stat (filename, &statbuf) >= 0
                  && ! S_ISREG (statbuf.st_mode))
                {
                  /* The file exists and is possibly a character device, socket,
                     or something like that.  As a workaround, add the O_CREAT
                     flag, although it ought not to be necessary.*/
                  fd = open (filename, flags | extra_flags, mode);
                  if (fd >= 0)
                    {
                      /* The file exists.  */
                      action->final_rename_temp = NULL;
                      action->final_rename_dest = NULL;
                    }
                }
            }
          #endif
          else if (errno == ENOENT)
            {
              /* The file does not exist.  Use a temporary file.  */
              char *canon_filename =
                canonicalize_filename_mode (filename, CAN_ALL_BUT_LAST);
              if (canon_filename == NULL)
                fd = -1;
              else
                {
                  fd = create_temp_file (canon_filename, flags, mode, action);
                  if (fd < 0)
                    free (canon_filename);
                }
            }
        }
      else
        {
          /* Never use a temporary file.  */
          fd = open (filename, flags | O_CREAT, mode);
          action->final_rename_temp = NULL;
          action->final_rename_dest = NULL;
        }
    }
  return fd;
}

static int
after_close_actions (int ret, const struct supersede_final_action *action)
{
  if (ret < 0)
    {
      /* There was an error writing.  Erase the temporary file.  */
      if (action->final_rename_temp != NULL)
        {
          int saved_errno = errno;
          ignore_value (unlink (action->final_rename_temp));
          free (action->final_rename_temp);
          free (action->final_rename_dest);
          errno = saved_errno;
        }
      return ret;
    }

  if (action->final_rename_temp != NULL)
    {
      struct stat temp_statbuf;
      struct stat dest_statbuf;

      if (stat (action->final_rename_temp, &temp_statbuf) < 0)
        {
          /* We just finished writing the temporary file, but now cannot access
             it.  There's something wrong.  */
          int saved_errno = errno;
          ignore_value (unlink (action->final_rename_temp));
          free (action->final_rename_temp);
          free (action->final_rename_dest);
          errno = saved_errno;
          return -1;
        }

      if (stat (action->final_rename_dest, &dest_statbuf) >= 0)
        {
          /* Copy the access time from the destination file to the temporary
             file.  */
          {
            struct timespec ts[2];

            ts[0] = get_stat_atime (&dest_statbuf);
            ts[1] = get_stat_mtime (&temp_statbuf);
            ignore_value (utimens (action->final_rename_temp, ts));
          }

#if HAVE_CHOWN
          /* Copy the owner and group from the destination file to the
             temporary file.  */
          ignore_value (chown (action->final_rename_temp,
                               dest_statbuf.st_uid, dest_statbuf.st_gid));
#endif

          /* Copy the access permissions from the destination file to the
             temporary file.  */
#if USE_ACL
          switch (qcopy_acl (action->final_rename_dest, -1,
                             action->final_rename_temp, -1,
                             dest_statbuf.st_mode))
            {
            case -2:
              /* Could not get the ACL of the destination file.  */
            case -1:
              /* Could not set the ACL on the temporary file.  */
              ignore_value (unlink (action->final_rename_temp));
              free (action->final_rename_temp);
              free (action->final_rename_dest);
              errno = EPERM;
              return -1;
            }
#else
          chmod (action->final_rename_temp, dest_statbuf.st_mode);
#endif
        }
      else
        /* No chmod needed, since the mode was already passed to
           gen_register_open_temp.  */
        ;

      /* Rename the temporary file to the destination file.  */
#if defined _WIN32 && !defined __CYGWIN__
      /* A native Windows platform.  */
      /* ReplaceFile
         <https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-replacefilea>
         is atomic regarding the file's contents, says
         https://stackoverflow.com/questions/167414/is-an-atomic-file-rename-with-overwrite-possible-on-windows>
         But it fails with GetLastError () == ERROR_FILE_NOT_FOUND if
         action->final_rename_dest does not exist.  So better use
         MoveFileEx
         <https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexa>.  */
      if (!MoveFileEx (action->final_rename_temp, action->final_rename_dest,
                       MOVEFILE_REPLACE_EXISTING))
        {
          int saved_errno;
          switch (GetLastError ())
            {
            case ERROR_INVALID_PARAMETER:
              saved_errno = EINVAL; break;
            default:
              saved_errno = EIO; break;
            }
          ignore_value (unlink (action->final_rename_temp));
          free (action->final_rename_temp);
          free (action->final_rename_dest);
          errno = saved_errno;
          return -1;
        }
#else
      if (rename (action->final_rename_temp, action->final_rename_dest) < 0)
        {
          int saved_errno = errno;
          ignore_value (unlink (action->final_rename_temp));
          free (action->final_rename_temp);
          free (action->final_rename_dest);
          errno = saved_errno;
          return -1;
        }
#endif

      unregister_temporary_file (action->final_rename_temp);

      free (action->final_rename_temp);
      free (action->final_rename_dest);
    }

  return ret;
}

int
close_supersede (int fd, const struct supersede_final_action *action)
{
  if (fd < 0)
    {
      free (action->final_rename_temp);
      free (action->final_rename_dest);
      return fd;
    }

  int ret;
  if (action->final_rename_temp != NULL)
    ret = close_temp (fd);
  else
    ret = close (fd);
  return after_close_actions (ret, action);
}

FILE *
fopen_supersede (const char *filename, const char *mode,
                 bool supersede_if_exists, bool supersede_if_does_not_exist,
                 struct supersede_final_action *action)
{
  /* Parse the mode.  */
  int open_direction = 0;
  int open_flags = 0;
  {
    const char *p = mode;

    for (; *p != '\0'; p++)
      {
        switch (*p)
          {
          case 'r':
            open_direction = O_RDONLY;
            continue;
          case 'w':
            open_direction = O_WRONLY;
            open_flags |= /* not! O_CREAT | */ O_TRUNC;
            continue;
          case 'a':
            open_direction = O_WRONLY;
            open_flags |= /* not! O_CREAT | */ O_APPEND;
            continue;
          case 'b':
            /* While it is non-standard, O_BINARY is guaranteed by
               gnulib <fcntl.h>.  */
            open_flags |= O_BINARY;
            continue;
          case '+':
            open_direction = O_RDWR;
            continue;
          case 'x':
            /* not! open_flags |= O_EXCL; */
            continue;
          case 'e':
            open_flags |= O_CLOEXEC;
            continue;
          default:
            break;
          }
        break;
      }
  }

  mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
  int fd = open_supersede (filename, open_direction | open_flags, open_mode,
                           supersede_if_exists, supersede_if_does_not_exist,
                           action);
  if (fd < 0)
    return NULL;

  FILE *stream = fdopen (fd, mode);
  if (stream == NULL)
    {
      int saved_errno = errno;
      close (fd);
      close_supersede (-1, action);
      errno = saved_errno;
    }
  return stream;
}

int
fclose_supersede (FILE *stream, const struct supersede_final_action *action)
{
  if (stream == NULL)
    return -1;
  int ret;
  if (action->final_rename_temp != NULL)
    ret = fclose_temp (stream);
  else
    ret = fclose (stream);
  return after_close_actions (ret, action);
}

#if GNULIB_FWRITEERROR
int
fwriteerror_supersede (FILE *stream, const struct supersede_final_action *action)
{
  if (stream == NULL)
    return -1;
  int ret;
  if (action->final_rename_temp != NULL)
    ret = fclose_temp (stream);
  else
    ret = fclose (stream);
  return after_close_actions (ret, action);
}
#endif
