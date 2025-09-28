dnl Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright (C) 2012-2023 Free Software Foundation, Inc.
dnl
dnl This file is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; see the file COPYING3.  If not see
dnl <http://www.gnu.org/licenses/>.
dnl

m4_include([../bfd/version.m4])
m4_include([../config/debuginfod.m4])
AC_INIT([binutils], BFD_VERSION)
AC_CONFIG_SRCDIR(ar.c)

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS

LT_INIT
ACX_LARGEFILE

ac_checking=
. ${srcdir}/../bfd/development.sh
test "$development" = true && ac_checking=yes
AC_ARG_ENABLE(checking,
[  --enable-checking       enable run-time checks],
[case "${enableval}" in
  no|none)  ac_checking= ;;
  *)	    ac_checking=yes ;;
esac])dnl
if test x$ac_checking != x ; then
  AC_DEFINE(ENABLE_CHECKING, 1, [Define if you want run-time sanity checks.])
fi

AC_ARG_ENABLE(targets,
[  --enable-targets        alternative target configurations],
[case "${enableval}" in
  yes | "") AC_MSG_ERROR(enable-targets option must specify target names or 'all')
            ;;
  no)       enable_targets= ;;
  *)        enable_targets=$enableval ;;
esac])dnl


AC_ARG_ENABLE(deterministic-archives,
[AS_HELP_STRING([--enable-deterministic-archives],
		[ar and ranlib default to -D behavior])], [
if test "${enableval}" = no; then
  default_ar_deterministic=0
else
  default_ar_deterministic=1
fi], [default_ar_deterministic=0])

AC_DEFINE_UNQUOTED(DEFAULT_AR_DETERMINISTIC, $default_ar_deterministic,
		   [Should ar and ranlib use -D behavior by default?])


AC_ARG_ENABLE(default-strings-all,
[AS_HELP_STRING([--disable-default-strings-all],
		[strings defaults to --data behavior])], [
if test "${enableval}" = no; then
  default_strings_all=0
else
  default_strings_all=1
fi], [default_strings_all=1])

AC_DEFINE_UNQUOTED(DEFAULT_STRINGS_ALL, $default_strings_all,
		   [Should strings use -a behavior by default?])


AC_ARG_ENABLE(f-for-ifunc-symbols,
[AS_HELP_STRING([--enable-f-for-ifunc-symbols],
	[Have nm use F and f for global and local ifunc symbols])], [
if test "${enableval}" = no; then
  default_f_for_ifunc=0
else
  default_f_for_ifunc=1
fi], [default_f_for_ifunc=0])

AC_DEFINE_UNQUOTED(DEFAULT_F_FOR_IFUNC_SYMBOLS, $default_f_for_ifunc,
		   [Have nm use F and f for global and local ifunc symbols])


AC_ARG_ENABLE(follow-debug-links,
[AS_HELP_STRING([--enable-follow-debug-links],
	[Have readelf and objdump follow debug links by default])], [
if test "${enableval}" = no; then
  default_for_follow_links=0
else
  default_for_follow_links=1
fi], [default_for_follow_links=1])

AC_DEFINE_UNQUOTED(DEFAULT_FOR_FOLLOW_LINKS, $default_for_follow_links,
		   [Have readelf and objdump follow debug links by default])




AC_ARG_ENABLE(colored-disassembly,
[AS_HELP_STRING([--enable-colored-disassembly],
	[Have objdump use colors in its disassembly by default])], [
if test "${enableval}" = no; then
  default_for_colored_disassembly=0
else
  default_for_colored_disassembly=1
fi], [default_for_colored_disassembly=0])

AC_DEFINE_UNQUOTED(DEFAULT_FOR_COLORED_DISASSEMBLY, $default_for_colored_disassembly,
		   [Have objdump use colors in its disassembly by default])


AC_DEBUGINFOD

GCC_ENABLE([libctf], [yes], [], [Handle .ctf type-info sections])
if test "${enable_libctf}" = yes; then
    AC_DEFINE(ENABLE_LIBCTF, 1, [Handle .ctf type-info sections])
fi
AM_CONDITIONAL(ENABLE_LIBCTF, test "${enable_libctf}" = yes)

AM_BINUTILS_WARNINGS

AC_CONFIG_HEADERS(config.h:config.in)

AH_VERBATIM([00_CONFIG_H_CHECK],
[/* Check that config.h is #included before system headers
   (this works only for glibc, but that should be enough).  */
#if defined(__GLIBC__) && !defined(__FreeBSD_kernel__) && !defined(__CONFIG_H__)
#  error config.h must be #included before system headers
#endif
#define __CONFIG_H__ 1])

if test -z "$target" ; then
    AC_MSG_ERROR(Unrecognized target system type; please check config.sub.)
fi
if test -z "$host" ; then
    AC_MSG_ERROR(Unrecognized host system type; please check config.sub.)
fi

AC_PROG_YACC
AM_PROG_LEX

ALL_LINGUAS="bg ca da es fi fr hr id it ja pt ro ru rw sk sr sv tr uk vi zh_CN zh_TW"
ZW_GNU_GETTEXT_SISTER_DIR
AM_PO_SUBDIRS

AM_MAINTAINER_MODE
AM_CONDITIONAL(GENINSRC_NEVER, false)
AC_EXEEXT
if test -n "$EXEEXT"; then
  AC_DEFINE(HAVE_EXECUTABLE_SUFFIX, 1,
	    [Does the platform use an executable suffix?])
fi
AC_DEFINE_UNQUOTED(EXECUTABLE_SUFFIX, "${EXEEXT}",
		   [Suffix used for executables, if any.])

# host-specific stuff:

HDEFINES=

. ${srcdir}/../bfd/configure.host

AC_SUBST(HDEFINES)
AR=${AR-ar}
AC_SUBST(AR)
AC_PROG_RANLIB
AC_PROG_INSTALL

BFD_CC_FOR_BUILD

DEMANGLER_NAME=c++filt
case "${host}" in
  *-*-go32* | *-*-msdos*)
    DEMANGLER_NAME=cxxfilt
esac
AC_SUBST(DEMANGLER_NAME)

# We use headers from include/ that check various HAVE_*_H macros, thus
# should ensure they are set by configure.  This is true even when C99
# guarantees they are available.
# plugin-api.h tests HAVE_STDINT_H and HAVE_INTTYPES_H
# Besides those, we need to check anything used in binutils/ not in C99.
AC_CHECK_HEADERS(fcntl.h inttypes.h stdint.h sys/file.h \
		 sys/stat.h sys/time.h sys/types.h unistd.h)
AC_HEADER_SYS_WAIT
AC_FUNC_MMAP
AC_CHECK_FUNCS(fseeko fseeko64 getc_unlocked mkdtemp mkstemp utimensat utimes)

AC_MSG_CHECKING([for mbstate_t])
AC_TRY_COMPILE([#include <wchar.h>],
[mbstate_t teststate;],
have_mbstate_t=yes, have_mbstate_t=no)
AC_MSG_RESULT($have_mbstate_t)
if test x"$have_mbstate_t" = xyes; then
  AC_DEFINE(HAVE_MBSTATE_T,1,[Define if mbstate_t exists in wchar.h.])
fi

# Copied from gnulib stat-time.m4.
# We should just switch over to using gnulib.
AC_CHECK_MEMBERS([struct stat.st_atim.tv_nsec],
  [AC_CACHE_CHECK([whether struct stat.st_atim is of type struct timespec],
     [ac_cv_typeof_struct_stat_st_atim_is_struct_timespec],
     [AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
        [[
          #include <sys/types.h>
          #include <sys/stat.h>
          #if HAVE_SYS_TIME_H
          # include <sys/time.h>
          #endif
          #include <time.h>
          struct timespec ts;
          struct stat st;
        ]],
        [[
          st.st_atim = ts;
        ]])],
        [ac_cv_typeof_struct_stat_st_atim_is_struct_timespec=yes],
        [ac_cv_typeof_struct_stat_st_atim_is_struct_timespec=no])])
   if test $ac_cv_typeof_struct_stat_st_atim_is_struct_timespec = yes; then
     AC_DEFINE([TYPEOF_STRUCT_STAT_ST_ATIM_IS_STRUCT_TIMESPEC], [1],
       [Define to 1 if the type of the st_atim member of a struct stat is
        struct timespec.])
   fi],
  [AC_CHECK_MEMBERS([struct stat.st_atimespec.tv_nsec], [],
     [AC_CHECK_MEMBERS([struct stat.st_atimensec], [],
        [AC_CHECK_MEMBERS([struct stat.st_atim.st__tim.tv_nsec], [], [],
           [#include <sys/types.h>
            #include <sys/stat.h>])],
        [#include <sys/types.h>
         #include <sys/stat.h>])],
     [#include <sys/types.h>
      #include <sys/stat.h>])],
  [#include <sys/types.h>
   #include <sys/stat.h>])

# Some systems have frexp only in -lm, not in -lc.
AC_SEARCH_LIBS(frexp, m)

AM_LC_MESSAGES

AC_MSG_CHECKING(for a known getopt prototype in unistd.h)
AC_CACHE_VAL(bu_cv_decl_getopt_unistd_h,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>], [extern int getopt (int, char *const*, const char *);])],
bu_cv_decl_getopt_unistd_h=yes, bu_cv_decl_getopt_unistd_h=no)])
AC_MSG_RESULT($bu_cv_decl_getopt_unistd_h)
if test $bu_cv_decl_getopt_unistd_h = yes; then
  AC_DEFINE([HAVE_DECL_GETOPT], 1,
	    [Is the prototype for getopt in <unistd.h> in the expected format?])
fi

# Under Next 3.2 <utime.h> apparently does not define struct utimbuf
# by default.
AC_MSG_CHECKING([for utime.h])
AC_CACHE_VAL(bu_cv_header_utime_h,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <utime.h>],
[struct utimbuf s;])],
bu_cv_header_utime_h=yes, bu_cv_header_utime_h=no)])
AC_MSG_RESULT($bu_cv_header_utime_h)
if test $bu_cv_header_utime_h = yes; then
  AC_DEFINE(HAVE_GOOD_UTIME_H, 1, [Does <utime.h> define struct utimbuf?])
fi

AC_CHECK_DECLS([asprintf, environ, getc_unlocked, stpcpy, strnlen])

# Link in zlib/zstd if we can.  This allows us to read compressed debug
# sections.  This is used only by readelf.c (objdump uses bfd for
# reading compressed sections).
AM_ZLIB
AC_ZSTD

BFD_BINARY_FOPEN

# Support for the msgpack C library.
AC_ARG_WITH([msgpack],
	    AC_HELP_STRING([--with-msgpack], [Enable msgpack support (auto/yes/no)]),
	    [],
	    [with_msgpack=auto])

AS_IF([test "$with_msgpack" != no],
  [PKG_CHECK_MODULES(MSGPACK, msgpack,
    [AC_DEFINE([HAVE_MSGPACK], [1], [Define to 1 if msgpack is available.])],
    [AS_IF([test "$with_msgpack" = yes],
      [AC_MSG_ERROR([--with-msgpack was given, but msgpack is missing or unusable.])])])])

# target-specific stuff:

# Canonicalize the secondary target names.
if test -n "$enable_targets"; then
    for targ in `echo $enable_targets | sed 's/,/ /g'`
    do
	result=`$ac_config_sub $targ 2>/dev/null`
	if test -n "$result"; then
	    canon_targets="$canon_targets $result"
	else
	    # Allow targets that config.sub doesn't recognize, like "all".
	    canon_targets="$canon_targets $targ"
	fi
    done
fi

AC_CHECK_HEADER(iconv.h)
AM_ICONV

all_targets=false
BUILD_SRCONV=
BUILD_DLLTOOL=
DLLTOOL_DEFS=
DLLTOOL_DEFAULT=
BUILD_WINDRES=
BUILD_WINDMC=
BUILD_DLLWRAP=
BUILD_MISC=
BUILD_INSTALL_MISC=
OBJDUMP_DEFS=
OBJDUMP_PRIVATE_VECTORS=
OBJDUMP_PRIVATE_OFILES=
od_vectors=

for targ in $target $canon_targets
do
    if test "x$targ" = "xall"; then
        all_targets=true
	BUILD_SRCONV='$(SRCONV_PROG)'
	BUILD_MISC="${BUILD_MISC} "'bin2c$(EXEEXT_FOR_BUILD)'
	BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
	BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
	BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	if test -z "$DLLTOOL_DEFAULT"; then
	  DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_I386"
	fi
	DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_I386"
	BUILD_DLLWRAP='$(DLLWRAP_PROG)$(EXEEXT)'
	od_vectors="$od_vectors objdump_private_desc_xcoff objdump_private_desc_pe"
    else
	case $targ in
	*-*-hms*) BUILD_SRCONV='$(SRCONV_PROG)' ;;
	esac

	case $targ in
	aarch64-*-mingw*)
	  BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_AARCH64"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_AARCH64"
	  BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
	  BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
	  BUILD_DLLWRAP='$(DLLWRAP_PROG)$(EXEEXT)'
	  ;;
	arm-wince-pe* | arm-*-wince | arm*-*-cegcc* | arm*-*-mingw32ce*)
  	  BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_ARM_WINCE"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_ARM_WINCE -DDLLTOOL_ARM"
	  BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
	  BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
	  ;;
	arm-*-pe*)
  	  BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_ARM"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_ARM"
	  BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
	  BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
	  ;;
	x86_64-*-mingw* | x86_64-*-cygwin*)
  	  BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_MX86_64"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_MX86_64"
	  BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
	  BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
	  BUILD_DLLWRAP='$(DLLWRAP_PROG)$(EXEEXT)'
	  ;;
changequote(,)dnl
	i[3-7]86-*-pe* | i[3-7]86-*-cygwin* | i[3-7]86-*-mingw32**)
changequote([,])dnl
  	  BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_I386"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_I386"
	  BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
	  BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
	  BUILD_DLLWRAP='$(DLLWRAP_PROG)$(EXEEXT)'
	  ;;
changequote(,)dnl
	i[3-7]86-*-interix)
changequote([,])dnl
	  BUILD_DLLTOOL='$(DLLTOOL_PROG)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_I386"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_I386"
	  ;;
changequote(,)dnl
	powerpc*-aix5.[01])
changequote([,])dnl
	  ;;
changequote(,)dnl
	powerpc*-aix[5-9].*)
changequote([,])dnl
	  OBJDUMP_DEFS="-DAIX_WEAK_SUPPORT"
	  ;;
	powerpc*-*-linux* | powerpc*-*-elf* | powerpc*-*-eabi*)
	  case "$BUILD_INSTALL_MISC" in
	    *embedspu*) ;;
	    *) BUILD_INSTALL_MISC="${BUILD_INSTALL_MISC} embedspu"
	  esac
	  ;;
	sh*-*-pe)
  	  BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_SH"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_SH"
	  BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
	  BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
	  ;;
	spu-*-*)
	  BUILD_MISC="${BUILD_MISC} "'bin2c$(EXEEXT_FOR_BUILD)'
	  ;;
	mcore-*-pe)
  	  BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_MCORE"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_MCORE"
	  BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
	  BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
	  ;;
	mcore-*-elf)
  	  BUILD_DLLTOOL='$(DLLTOOL_PROG)$(EXEEXT)'
	  if test -z "$DLLTOOL_DEFAULT"; then
	    DLLTOOL_DEFAULT="-DDLLTOOL_DEFAULT_MCORE_ELF"
	  fi
	  DLLTOOL_DEFS="$DLLTOOL_DEFS -DDLLTOOL_MCORE_ELF"
	  ;;
  	mep-*)
	  OBJDUMP_DEFS="-DSKIP_ZEROES=256 -DSKIP_ZEROES_AT_END=0"
	  ;;
	esac

	# Add objdump private vectors.
	case $targ in
	avr-*-*)
	  od_vectors="$od_vectors objdump_private_desc_elf32_avr"
	  ;;
	powerpc*-*-aix* | rs6000-*-aix*)
	  od_vectors="$od_vectors objdump_private_desc_xcoff"
	  ;;
	*-*-pe* | *-*-cygwin* | *-*-mingw*)
	  od_vectors="$od_vectors objdump_private_desc_pe"
	  ;;
        *-*-darwin*)
	  od_vectors="$od_vectors objdump_private_desc_mach_o"
	  ;;
	esac
    fi
done

# Uniq objdump private vector, build objdump target ofiles.
od_files=
f=""
for i in $od_vectors ; do
    case " $f " in
    *" $i "*) ;;
    *)
	f="$f $i"
	OBJDUMP_PRIVATE_VECTORS="$OBJDUMP_PRIVATE_VECTORS &$i,"
	case $i in
	objdump_private_desc_elf32_avr)
	    od_files="$od_files od-elf32_avr" ;;
	objdump_private_desc_xcoff)
	    od_files="$od_files od-xcoff" ;;
	objdump_private_desc_pe)
	    od_files="$od_files od-pe" ;;
	objdump_private_desc_mach_o)
	    od_files="$od_files od-macho" ;;
	*) AC_MSG_ERROR(*** unknown private vector $i) ;;
	esac
	;;
    esac
done

# Uniq objdump target ofiles
f=""
for i in $od_files ; do
    case " $f " in
    *" $i "*) ;;
    *)
	f="$f $i"
	OBJDUMP_PRIVATE_OFILES="$OBJDUMP_PRIVATE_OFILES $i.$objext"
	;;
    esac
done

DLLTOOL_DEFS="$DLLTOOL_DEFS $DLLTOOL_DEFAULT"

if test "${with_windres+set}" = set; then
	  BUILD_WINDRES='$(WINDRES_PROG)$(EXEEXT)'
fi

if test "${with_windmc+set}" = set; then
	  BUILD_WINDMC='$(WINDMC_PROG)$(EXEEXT)'
fi

OBJDUMP_DEFS="${OBJDUMP_DEFS} -DOBJDUMP_PRIVATE_VECTORS=\"${OBJDUMP_PRIVATE_VECTORS}\""

AC_SUBST(BUILD_SRCONV)
AC_SUBST(BUILD_DLLTOOL)
AC_SUBST(DLLTOOL_DEFS)
AC_SUBST(BUILD_WINDRES)
AC_SUBST(BUILD_WINDMC)
AC_SUBST(BUILD_DLLWRAP)
AC_SUBST(BUILD_MISC)
AC_SUBST(BUILD_INSTALL_MISC)
AC_SUBST(OBJDUMP_DEFS)
AC_SUBST(OBJDUMP_PRIVATE_OFILES)

AC_DEFINE_UNQUOTED(TARGET, "${target}", [Configured target name.])

targ=$target
. $srcdir/../bfd/config.bfd
if test "x$targ_underscore" = "xyes"; then
    UNDERSCORE=1
else
    UNDERSCORE=0
fi
AC_DEFINE_UNQUOTED(TARGET_PREPENDS_UNDERSCORE, $UNDERSCORE,
 [Define to 1 if user symbol names have a leading underscore, 0 if not.])

# Emulation
targ=$target
. ${srcdir}/configure.tgt
EMULATION=$targ_emul
EMULATION_VECTOR=$targ_emul_vector

AC_SUBST(EMULATION)
AC_SUBST(EMULATION_VECTOR)

# Required for html and install-html
AC_SUBST(datarootdir)
AC_SUBST(docdir)
AC_SUBST(htmldir)
AC_SUBST(pdfdir)

AC_CONFIG_FILES(Makefile po/Makefile.in:po/Make-in)
AC_OUTPUT

GNU_MAKE_JOBSERVER
