/* Define at-style functions like fstatat, unlinkat, fchownat, etc.
   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.

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

#include "filename.h" /* solely for definition of IS_ABSOLUTE_FILE_NAME */

#ifdef GNULIB_SUPPORT_ONLY_AT_FDCWD
# include <errno.h>
# ifndef ENOTSUP
#  define ENOTSUP EINVAL
# endif
#else
# include "openat.h"
# include "openat-priv.h"
# include "save-cwd.h"
#endif

#ifdef AT_FUNC_USE_F1_COND
# define CALL_FUNC(F)                           \
  (flag == AT_FUNC_USE_F1_COND                  \
    ? AT_FUNC_F1 (F AT_FUNC_POST_FILE_ARGS)     \
    : AT_FUNC_F2 (F AT_FUNC_POST_FILE_ARGS))
# define VALIDATE_FLAG(F)                       \
  if (flag & ~AT_FUNC_USE_F1_COND)              \
    {                                           \
      errno = EINVAL;                           \
      return FUNC_FAIL;                         \
    }
#else
# define CALL_FUNC(F) (AT_FUNC_F1 (F AT_FUNC_POST_FILE_ARGS))
# define VALIDATE_FLAG(F) /* empty */
#endif

#ifdef AT_FUNC_RESULT
# define FUNC_RESULT AT_FUNC_RESULT
#else
# define FUNC_RESULT int
#endif

#ifdef AT_FUNC_FAIL
# define FUNC_FAIL AT_FUNC_FAIL
#else
# define FUNC_FAIL -1
#endif

/* Call AT_FUNC_F1 to operate on FILE, which is in the directory
   open on descriptor FD.  If AT_FUNC_USE_F1_COND is defined to a value,
   AT_FUNC_POST_FILE_PARAM_DECLS must include a parameter named flag;
   call AT_FUNC_F2 if FLAG is 0 or fail if FLAG contains more bits than
   AT_FUNC_USE_F1_COND.  Return int and fail with -1 unless AT_FUNC_RESULT
   or AT_FUNC_FAIL are defined.  If possible, do it without changing the
   working directory.  Otherwise, resort to using save_cwd/fchdir,
   then AT_FUNC_F?/restore_cwd.  If either the save_cwd or the restore_cwd
   fails, then give a diagnostic and exit nonzero.  */
FUNC_RESULT
AT_FUNC_NAME (int fd, char const *file AT_FUNC_POST_FILE_PARAM_DECLS)
{
  VALIDATE_FLAG (flag);

  if (fd == AT_FDCWD || IS_ABSOLUTE_FILE_NAME (file))
    return CALL_FUNC (file);

#ifdef GNULIB_SUPPORT_ONLY_AT_FDCWD
  errno = ENOTSUP;
  return FUNC_FAIL;
#else
  {
  /* Be careful to choose names unlikely to conflict with
     AT_FUNC_POST_FILE_PARAM_DECLS.  */
  struct saved_cwd saved_cwd;
  int saved_errno;
  FUNC_RESULT err;

  {
    char proc_buf[OPENAT_BUFFER_SIZE];
    char *proc_file = openat_proc_name (proc_buf, fd, file);
    if (proc_file)
      {
        FUNC_RESULT proc_result = CALL_FUNC (proc_file);
        int proc_errno = errno;
        if (proc_file != proc_buf)
          free (proc_file);
        /* If the syscall succeeds, or if it fails with an unexpected
           errno value, then return right away.  Otherwise, fall through
           and resort to using save_cwd/restore_cwd.  */
        if (FUNC_FAIL != proc_result)
          return proc_result;
        if (! EXPECTED_ERRNO (proc_errno))
          {
            errno = proc_errno;
            return proc_result;
          }
      }
  }

  if (save_cwd (&saved_cwd) != 0)
    openat_save_fail (errno);
  if (0 <= fd && fd == saved_cwd.desc)
    {
      /* If saving the working directory collides with the user's
         requested fd, then the user's fd must have been closed to
         begin with.  */
      free_cwd (&saved_cwd);
      errno = EBADF;
      return FUNC_FAIL;
    }

  if (fchdir (fd) != 0)
    {
      saved_errno = errno;
      free_cwd (&saved_cwd);
      errno = saved_errno;
      return FUNC_FAIL;
    }

  err = CALL_FUNC (file);
  saved_errno = (err == FUNC_FAIL ? errno : 0);

  if (restore_cwd (&saved_cwd) != 0)
    openat_restore_fail (errno);

  free_cwd (&saved_cwd);

  if (saved_errno)
    errno = saved_errno;
  return err;
  }
#endif
}
#undef CALL_FUNC
#undef FUNC_RESULT
#undef FUNC_FAIL
