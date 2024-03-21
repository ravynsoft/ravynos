# Enable large files on systems where this is not the default.
# Enable support for files on Linux file systems with 64-bit inode numbers.

# Copyright 1992-1996, 1998-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# The following macro works around a problem in Autoconf's AC_FUNC_FSEEKO:
# It does not set _LARGEFILE_SOURCE=1 on HP-UX/ia64 32-bit, although this
# setting of _LARGEFILE_SOURCE is needed so that <stdio.h> declares fseeko
# and ftello in C++ mode as well.
# This problem occurs in Autoconf 2.71 and earlier, which lack AC_SYS_YEAR2038.
AC_DEFUN([gl_SET_LARGEFILE_SOURCE],
 m4_ifndef([AC_SYS_YEAR2038], [[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_FUNC_FSEEKO
  case "$host_os" in
    hpux*)
      AC_DEFINE([_LARGEFILE_SOURCE], [1],
        [Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2).])
      ;;
  esac
 ]])
)

m4_ifndef([AC_SYS_YEAR2038_RECOMMENDED], [
# Support AC_SYS_YEAR2038_RECOMMENDED and related macros, even if
# Autoconf 2.71 or earlier.  This code is taken from Autoconf master.

# _AC_SYS_YEAR2038_TEST_CODE
# --------------------------
# C code used to probe for time_t that can represent time points more
# than 2**31 - 1 seconds after the epoch.  With the usual Unix epoch,
# these correspond to dates after 2038-01-18 22:14:07 +0000 (Gregorian),
# hence the name.
AC_DEFUN([_AC_SYS_YEAR2038_TEST_CODE],
[[
  #include <time.h>
  /* Check that time_t can represent 2**32 - 1 correctly.  */
  #define LARGE_TIME_T \\
    ((time_t) (((time_t) 1 << 30) - 1 + 3 * ((time_t) 1 << 30)))
  int verify_time_t_range[(LARGE_TIME_T / 65537 == 65535
                           && LARGE_TIME_T % 65537 == 0)
                          ? 1 : -1];
]])

# _AC_SYS_YEAR2038_OPTIONS
# ------------------------
# List of known ways to enable support for large time_t.  If you change
# this list you probably also need to change the AS_CASE at the end of
# _AC_SYS_YEAR2038_PROBE.
m4_define([_AC_SYS_YEAR2038_OPTIONS], m4_normalize(
    ["none needed"]                   dnl 64-bit and newer 32-bit Unix
    ["-D_TIME_BITS=64"]               dnl glibc 2.34 with some 32-bit ABIs
    ["-D__MINGW_USE_VC2005_COMPAT"]   dnl 32-bit MinGW
    ["-U_USE_32_BIT_TIME_T -D__MINGW_USE_VC2005_COMPAT"]
                                      dnl 32-bit MinGW (misconfiguration)
))

# _AC_SYS_YEAR2038_PROBE
# ----------------------
# Subroutine of AC_SYS_YEAR2038.  Probe for time_t that can represent
# time points more than 2**31 - 1 seconds after the epoch (dates after
# 2038-01-18, see above) and set the cache variable ac_cv_sys_year2038_opts
# to one of the values in the _AC_SYS_YEAR2038_OPTIONS list, or to
# "support not detected" if none of them worked.  Then, set compilation
# options and #defines as necessary to enable large time_t support.
#
# Note that we do not test whether mktime, localtime, etc. handle
# large values of time_t correctly, as that would require use of
# AC_TRY_RUN.  Note also that some systems only support large time_t
# together with large off_t.
#
# If you change this macro you may also need to change
# _AC_SYS_YEAR2038_OPTIONS.
AC_DEFUN([_AC_SYS_YEAR2038_PROBE],
[AC_CACHE_CHECK([for $CC option for timestamps after 2038],
  [ac_cv_sys_year2038_opts],
  [ac_save_CPPFLAGS="$CPPFLAGS"
  ac_opt_found=no
  for ac_opt in _AC_SYS_YEAR2038_OPTIONS; do
    AS_IF([test x"$ac_opt" != x"none needed"],
      [CPPFLAGS="$ac_save_CPPFLAGS $ac_opt"])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([_AC_SYS_YEAR2038_TEST_CODE])],
      [ac_cv_sys_year2038_opts="$ac_opt"
      ac_opt_found=yes])
    test $ac_opt_found = no || break
  done
  CPPFLAGS="$ac_save_CPPFLAGS"
  test $ac_opt_found = yes || ac_cv_sys_year2038_opts="support not detected"])

ac_have_year2038=yes
AS_CASE([$ac_cv_sys_year2038_opts],
  ["none needed"], [],
  ["support not detected"],
    [ac_have_year2038=no],

  ["-D_TIME_BITS=64"],
    [AC_DEFINE([_TIME_BITS], [64],
      [Number of bits in time_t, on hosts where this is settable.])],

  ["-D__MINGW_USE_VC2005_COMPAT"],
    [AC_DEFINE([__MINGW_USE_VC2005_COMPAT], [1],
      [Define to 1 on platforms where this makes time_t a 64-bit type.])],

  ["-U_USE_32_BIT_TIME_T"*],
    [AC_MSG_FAILURE(m4_text_wrap(
      [the 'time_t' type is currently forced to be 32-bit.
       It will stop working after mid-January 2038.
       Remove _USE_32BIT_TIME_T from the compiler flags.],
      [], [], [55]))],

  [AC_MSG_ERROR(
    [internal error: bad value for \$ac_cv_sys_year2038_opts])])
])

# _AC_SYS_YEAR2038_ENABLE
# -----------------------
# Depending on which of the YEAR2038 macros was used, add either an
# --enable-year2038 or a --disable-year2038 to
# the configure script.  This is expanded very late and
# therefore there cannot be any code in the AC_ARG_ENABLE.  The
# default value for 'enable_year2038' is emitted unconditionally
# because the generated code always looks at this variable.
m4_define([_AC_SYS_YEAR2038_ENABLE],
[m4_divert_text([DEFAULTS],
  m4_provide_if([AC_SYS_YEAR2038],
    [enable_year2038=yes],
    [enable_year2038=no]))]dnl
[AC_ARG_ENABLE([year2038],
  m4_provide_if([AC_SYS_YEAR2038],
    [AS_HELP_STRING([--disable-year2038],
      [don't support timestamps after 2038])],
    [AS_HELP_STRING([--enable-year2038],
      [support timestamps after 2038])]))])

# AC_SYS_YEAR2038
# ---------------
# Attempt to detect and activate support for large time_t.
# On systems where time_t is not always 64 bits, this probe can be
# skipped by passing the --disable-year2038 option to configure.
AC_DEFUN([AC_SYS_YEAR2038],
[AC_REQUIRE([AC_SYS_LARGEFILE])dnl
AS_IF([test "$enable_year2038,$ac_have_year2038,$cross_compiling" = yes,no,no],
 [# If we're not cross compiling and 'touch' works with a large
  # timestamp, then we can presume the system supports wider time_t
  # *somehow* and we just weren't able to detect it.  One common
  # case that we deliberately *don't* probe for is a system that
  # supports both 32- and 64-bit ABIs but only the 64-bit ABI offers
  # wide time_t.  (It would be inappropriate for us to override an
  # intentional use of -m32.)  Error out, demanding use of
  # --disable-year2038 if this is intentional.
  AS_IF([TZ=UTC0 touch -t 210602070628.15 conftest.time 2>/dev/null],
    [AS_CASE([`TZ=UTC0 LC_ALL=C ls -l conftest.time 2>/dev/null`],
       [*'Feb  7  2106'* | *'Feb  7 17:10'*],
       [AC_MSG_FAILURE(m4_text_wrap(
	  [this system appears to support timestamps after mid-January 2038,
	   but no mechanism for enabling wide 'time_t' was detected.
	   Did you mean to build a 64-bit binary? (E.g., 'CC="${CC} -m64"'.)
	   To proceed with 32-bit time_t, configure with '--disable-year2038'.],
	  [], [], [55]))])])])])

# AC_SYS_YEAR2038_RECOMMENDED
# ---------------------------
# Same as AC_SYS_YEAR2038, but recommend support for large time_t.
# If we cannot find any way to make time_t capable of representing
# values larger than 2**31 - 1, error out unless --disable-year2038 is given.
AC_DEFUN([AC_SYS_YEAR2038_RECOMMENDED],
[AC_REQUIRE([AC_SYS_YEAR2038])dnl
AS_IF([test "$enable_year2038,$ac_have_year2038" = yes,no],
   [AC_MSG_FAILURE(m4_text_wrap(
      [could not enable timestamps after mid-January 2038.
       This package recommends support for these later timestamps.
       However, to proceed with signed 32-bit time_t even though it
       will fail then, configure with '--disable-year2038'.],
      [], [], [55]))])])

# _AC_SYS_LARGEFILE_TEST_CODE
# ---------------------------
# C code used to probe for large file support.
m4_define([_AC_SYS_LARGEFILE_TEST_CODE],
[@%:@include <sys/types.h>
@%:@ifndef FTYPE
@%:@ define FTYPE off_t
@%:@endif
 /* Check that FTYPE can represent 2**63 - 1 correctly.
    We can't simply define LARGE_FTYPE to be 9223372036854775807,
    since some C++ compilers masquerading as C compilers
    incorrectly reject 9223372036854775807.  */
@%:@define LARGE_FTYPE (((FTYPE) 1 << 31 << 31) - 1 + ((FTYPE) 1 << 31 << 31))
  int FTYPE_is_large[[(LARGE_FTYPE % 2147483629 == 721
		       && LARGE_FTYPE % 2147483647 == 1)
		      ? 1 : -1]];[]dnl
])
# Defined by Autoconf 2.71 and circa 2022 Gnulib unwisely depended on it.
m4_define([_AC_SYS_LARGEFILE_TEST_INCLUDES], [_AC_SYS_LARGEFILE_TEST_CODE])

# _AC_SYS_LARGEFILE_OPTIONS
# -------------------------
# List of known ways to enable support for large files.  If you change
# this list you probably also need to change the AS_CASE at the end of
# _AC_SYS_LARGEFILE_PROBE.
m4_define([_AC_SYS_LARGEFILE_OPTIONS], m4_normalize(
    ["none needed"]                   dnl Most current systems
    ["-D_FILE_OFFSET_BITS=64"]        dnl X/Open LFS spec
    ["-D_LARGE_FILES=1"]              dnl 32-bit AIX 4.2.1+, 32-bit z/OS
    ["-n32"]                          dnl 32-bit IRIX 6, SGI cc (obsolete)
))

# _AC_SYS_LARGEFILE_PROBE
# -----------------------
# Subroutine of AC_SYS_LARGEFILE. Probe for large file support and set
# the cache variable ac_cv_sys_largefile_opts to one of the values in
# the _AC_SYS_LARGEFILE_OPTIONS list, or to "support not detected" if
# none of the options in that list worked.  Then, set compilation
# options and #defines as necessary to enable large file support.
#
# If large file support is not detected, the behavior depends on which of
# the top-level AC_SYS_LARGEFILE macros was used (see below).
#
# If you change this macro you may also need to change
# _AC_SYS_LARGEFILE_OPTIONS.
AC_DEFUN([_AC_SYS_LARGEFILE_PROBE],
[AC_CACHE_CHECK([for $CC option to enable large file support],
  [ac_cv_sys_largefile_opts],
  [ac_save_CC="$CC"
  ac_opt_found=no
  for ac_opt in _AC_SYS_LARGEFILE_OPTIONS; do
    AS_IF([test x"$ac_opt" != x"none needed"],
      [CC="$ac_save_CC $ac_opt"])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([_AC_SYS_LARGEFILE_TEST_CODE])],
     [AS_IF([test x"$ac_opt" = x"none needed"],
	[# GNU/Linux s390x and alpha need _FILE_OFFSET_BITS=64 for wide ino_t.
	 CC="$CC -DFTYPE=ino_t"
	 AC_COMPILE_IFELSE([], [],
	   [CC="$CC -D_FILE_OFFSET_BITS=64"
	    AC_COMPILE_IFELSE([], [ac_opt='-D_FILE_OFFSET_BITS=64'])])])
      ac_cv_sys_largefile_opts=$ac_opt
      ac_opt_found=yes])
    test $ac_opt_found = no || break
  done
  CC="$ac_save_CC"
  dnl Gnulib implements large file support for native Windows, based on the
  dnl variables WINDOWS_64_BIT_OFF_T, WINDOWS_64_BIT_ST_SIZE.
  m4_ifdef([gl_LARGEFILE], [
    AC_REQUIRE([AC_CANONICAL_HOST])
    if test $ac_opt_found != yes; then
      AS_CASE([$host_os],
        [mingw* | windows*],
          [ac_cv_sys_largefile_opts="supported through gnulib"
           ac_opt_found=yes]
      )
    fi
  ])
  test $ac_opt_found = yes || ac_cv_sys_largefile_opts="support not detected"])

ac_have_largefile=yes
AS_CASE([$ac_cv_sys_largefile_opts],
  ["none needed"], [],
  ["supported through gnulib"], [],
  ["support not detected"],
    [ac_have_largefile=no],

  ["-D_FILE_OFFSET_BITS=64"],
    [AC_DEFINE([_FILE_OFFSET_BITS], [64],
      [Number of bits in a file offset, on hosts where this is settable.])],

  ["-D_LARGE_FILES=1"],
    [AC_DEFINE([_LARGE_FILES], [1],
      [Define to 1 on platforms where this makes off_t a 64-bit type.])],

  ["-n32"],
    [CC="$CC -n32"],

  [AC_MSG_ERROR(
    [internal error: bad value for \$ac_cv_sys_largefile_opts])])

AS_IF([test "$enable_year2038" != no],
  [_AC_SYS_YEAR2038_PROBE])
AC_CONFIG_COMMANDS_PRE([_AC_SYS_YEAR2038_ENABLE])])

# AC_SYS_LARGEFILE
# ----------------
# By default, many hosts won't let programs access large files;
# one must use special compiler options to get large-file access to work.
# For more details about this brain damage please see:
# http://www.unix.org/version2/whatsnew/lfs20mar.html
# Additionally, on Linux file systems with 64-bit inodes a file that happens
# to have a 64-bit inode number cannot be accessed by 32-bit applications on
# Linux x86/x86_64.  This can occur with file systems such as XFS and NFS.
AC_DEFUN([AC_SYS_LARGEFILE],
[AC_ARG_ENABLE([largefile],
   [AS_HELP_STRING([--disable-largefile],
      [omit support for large files])])dnl
AS_IF([test "$enable_largefile,$enable_year2038" != no,no],
  [_AC_SYS_LARGEFILE_PROBE])])
])# m4_ifndef AC_SYS_YEAR2038_RECOMMENDED

# Enable large files on systems where this is implemented by Gnulib, not by the
# system headers.
# Set the variables WINDOWS_64_BIT_OFF_T, WINDOWS_64_BIT_ST_SIZE if Gnulib
# overrides ensure that off_t or 'struct size.st_size' are 64-bit, respectively.
AC_DEFUN([gl_LARGEFILE],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | windows*)
      dnl Native Windows.
      dnl mingw64 defines off_t to a 64-bit type already, if
      dnl _FILE_OFFSET_BITS=64, which is ensured by AC_SYS_LARGEFILE.
      AC_CACHE_CHECK([for 64-bit off_t], [gl_cv_type_off_t_64],
        [AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM(
              [[#include <sys/types.h>
                int verify_off_t_size[sizeof (off_t) >= 8 ? 1 : -1];
              ]],
              [[]])],
           [gl_cv_type_off_t_64=yes], [gl_cv_type_off_t_64=no])
        ])
      if test $gl_cv_type_off_t_64 = no; then
        WINDOWS_64_BIT_OFF_T=1
      else
        WINDOWS_64_BIT_OFF_T=0
      fi
      dnl Some mingw versions define, if _FILE_OFFSET_BITS=64, 'struct stat'
      dnl to 'struct _stat32i64' or 'struct _stat64' (depending on
      dnl _USE_32BIT_TIME_T), which has a 32-bit st_size member.
      AC_CACHE_CHECK([for 64-bit st_size], [gl_cv_member_st_size_64],
        [AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM(
              [[#include <sys/types.h>
                struct stat buf;
                int verify_st_size_size[sizeof (buf.st_size) >= 8 ? 1 : -1];
              ]],
              [[]])],
           [gl_cv_member_st_size_64=yes], [gl_cv_member_st_size_64=no])
        ])
      if test $gl_cv_member_st_size_64 = no; then
        WINDOWS_64_BIT_ST_SIZE=1
      else
        WINDOWS_64_BIT_ST_SIZE=0
      fi
      ;;
    *)
      dnl Nothing to do on gnulib's side.
      dnl A 64-bit off_t is
      dnl   - already the default on Mac OS X, FreeBSD, NetBSD, OpenBSD, IRIX,
      dnl     OSF/1, Cygwin,
      dnl   - enabled by _FILE_OFFSET_BITS=64 (ensured by AC_SYS_LARGEFILE) on
      dnl     glibc, HP-UX, Solaris,
      dnl   - enabled by _LARGE_FILES=1 (ensured by AC_SYS_LARGEFILE) on AIX,
      dnl   - impossible to achieve on Minix 3.1.8.
      WINDOWS_64_BIT_OFF_T=0
      WINDOWS_64_BIT_ST_SIZE=0
      ;;
  esac
])
