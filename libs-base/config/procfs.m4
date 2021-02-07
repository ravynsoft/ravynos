dnl procfs macros
dnl  Copyright (C) 2005 Free Software Foundation
dnl  Copying and distribution of this file, with or without modification,
dnl  are permitted in any medium without royalty provided the copyright
dnl  notice and this notice are preserved.
dnl
dnl AC_SYS_PROCFS
dnl This macro defines HAVE_PROCFS if either it finds a mounted /proc
dnl or the user explicitly enables it for cross-compiles.
AC_DEFUN(AC_SYS_PROCFS,
[ AC_ARG_ENABLE(procfs,
    [  --enable-procfs               Use /proc filesystem (default)],
    enable_procfs="$enableval", if test "$cross_compiling" = yes; then enable_procfs=cross; else enable_procfs=yes; fi;)

  AC_CACHE_CHECK([kernel support for /proc filesystem], ac_cv_sys_procfs,
  [if test "$enable_procfs" = yes; then
  # Suggested change for the following line was 
  #  if test -d /proc/0; then
  # but it doesn't work on my linux - /proc/0 does not exist, but /proc
  # works fine
    if grep 'proc' /proc/mounts >/dev/null 2>/dev/null; then
      ac_cv_sys_procfs=yes
    else
      ac_cv_sys_procfs=no
    fi
    case "$target_os" in
      # Solaris has proc, but it is not readable
      solaris*)    ac_cv_sys_procfs=no;;
      irix*)       ac_cv_sys_procfs=no;;
      # Cygwin does have proc, but it does not show with mount
      cygwin*)     ac_cv_sys_procfs=yes;;
    esac
  elif test "$enable_procfs" = cross; then
    ac_cv_sys_procfs=no
  else
    ac_cv_sys_procfs=no
  fi])

  if test "$enable_procfs" = cross; then
    AC_MSG_WARN(Cross-compiling: Pass --enable-procfs argument to enable use of /proc filesystem.)
  fi
  if test $ac_cv_sys_procfs = yes; then
    AC_DEFINE(HAVE_PROCFS, 1, [Define if system supports the /proc filesystem])
  fi
]
)

dnl AC_SYS_PROCFS_PSINFO
dnl This macro defines HAVE_PROCFS_PSINFO if it can read the psinfo 
dnl structure from the /proc/%pid% directory
AC_DEFUN(AC_SYS_PROCFS_PSINFO,
[ AC_ARG_ENABLE(procfs-psinfo,
    [  --enable-procfs-psinfo         Use /proc/%pid% to get info],
    enable_procfs_psinfo="$enableval", if test "$cross_compiling" = yes; then enable_procfs_psinfo=cross; else enable_procfs_psinfo=yes; fi;)

  AC_CACHE_CHECK([support for /proc psinfo struct], ac_cv_sys_procfs_psinfo,
  [if test "$enable_procfs_psinfo" = yes; then
    AC_TRY_RUN([#include "$srcdir/config/config.psinfo.c"],
	ac_cv_sys_procfs_psinfo=yes, ac_cv_sys_procfs_psinfo=no, 
	ac_cv_sys_procfs_psinfo=yes)
  elif test "$enable_procfs" = cross; then
    ac_cv_sys_procfs_psinfo=no
  else
    ac_cv_sys_procfs_psinfo=no
  fi])

  if test "$enable_procfs" = cross; then
    AC_MSG_WARN(Cross-compiling: Pass --enable-procfs-psinfo argument to enable use of /proc psinfo information.)
  fi
  if test $ac_cv_sys_procfs_psinfo = yes; then
    AC_DEFINE(HAVE_PROCFS_PSINFO, 1, [Define if system supports reading psinfo from /proc])
  fi
]
)
