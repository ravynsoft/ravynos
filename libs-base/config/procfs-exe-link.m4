dnl AC_SYS_PROCFS_EXE_LINK
dnl This macro checks for the existence of a symlink in /proc to the executable
dnl file associated with the current process, and defines PROCFS_EXE_LINK to
dnl the path it finds.  Currently supports Linux and FreeBSD variants.
dnl  Copyright (C) 2005 Free Software Foundation
dnl  Copying and distribution of this file, with or without modification,
dnl  are permitted in any medium without royalty provided the copyright
dnl  notice and this notice are preserved.
AC_DEFUN(AC_SYS_PROCFS_EXE_LINK,
[ AC_REQUIRE([AC_SYS_PROCFS])

  AC_CACHE_CHECK([link to exe of process in /proc], ac_cv_sys_procfs_exe_link,
    [if test "$ac_cv_sys_procfs" = yes; then
      # Linux 2.2.x and up
      if test -L /proc/self/exe; then
        ac_cv_sys_procfs_exe_link=/proc/self/exe
      # FreeBSD 4.x and up
      elif test -L /proc/curproc/file; then
        ac_cv_sys_procfs_exe_link=/proc/curproc/file
      else
        ac_cv_sys_procfs_exe_link=no
      fi
    else
      ac_cv_sys_procfs_exe_link=no
    fi])

  if test "$ac_cv_sys_procfs_exe_link" != no; then
    AC_DEFINE_UNQUOTED(PROCFS_EXE_LINK, ["$ac_cv_sys_procfs_exe_link"],
      [Define as the link to exe of process in /proc filesystem.])
  fi
]) 
