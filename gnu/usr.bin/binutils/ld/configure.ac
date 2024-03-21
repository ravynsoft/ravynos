dnl Process this file with autoconf to produce a configure script
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
AC_INIT([ld], BFD_VERSION)
AC_CONFIG_SRCDIR(ldmain.c)

AC_CANONICAL_TARGET
AC_CANONICAL_BUILD

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_CXX
AC_PROG_GREP
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG

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

AC_ARG_WITH(lib-path, [  --with-lib-path=dir1:dir2...  set default LIB_PATH],LIB_PATH=$withval)
AC_ARG_ENABLE(targets,
[  --enable-targets        alternative target configurations],
[case "${enableval}" in
  yes | "") AC_MSG_ERROR(enable-targets option must specify target names or 'all')
	    ;;
  no)       enable_targets= ;;
  *)        enable_targets=$enableval ;;
esac])dnl

BFD_64_BIT

AC_ARG_WITH(sysroot,
[  --with-sysroot[=DIR] Search for usr/lib et al within DIR.],
[
 case ${with_sysroot} in
 yes) TARGET_SYSTEM_ROOT='${exec_prefix}/${target_alias}/sys-root' ;;
 *) TARGET_SYSTEM_ROOT=$with_sysroot ;;
 esac

 TARGET_SYSTEM_ROOT_DEFINE='-DTARGET_SYSTEM_ROOT=\"$(TARGET_SYSTEM_ROOT)\"'
 use_sysroot=yes

 if test "x$prefix" = xNONE; then
  test_prefix=/usr/local
 else
  test_prefix=$prefix
 fi
 if test "x$exec_prefix" = xNONE; then
  test_exec_prefix=$test_prefix
 else
  test_exec_prefix=$exec_prefix
 fi
 case ${TARGET_SYSTEM_ROOT} in
 "${test_prefix}"|"${test_prefix}/"*|\
 "${test_exec_prefix}"|"${test_exec_prefix}/"*|\
 '${prefix}'|'${prefix}/'*|\
 '${exec_prefix}'|'${exec_prefix}/'*)
   t="$TARGET_SYSTEM_ROOT_DEFINE -DTARGET_SYSTEM_ROOT_RELOCATABLE"
   TARGET_SYSTEM_ROOT_DEFINE="$t"
   ;;
 esac
], [
 use_sysroot=no
 TARGET_SYSTEM_ROOT=
 TARGET_SYSTEM_ROOT_DEFINE='-DTARGET_SYSTEM_ROOT=\"\"'
])
AC_SUBST(use_sysroot)
AC_SUBST(TARGET_SYSTEM_ROOT)
AC_SUBST(TARGET_SYSTEM_ROOT_DEFINE)

dnl Use --enable-gold to decide if this linker should be the default.
dnl "install_as_default" is set to false if gold is the default linker.
dnl "installed_linker" is the installed BFD linker name.
AC_ARG_ENABLE(gold,
[[  --enable-gold[=ARG]     build gold [ARG={default,yes,no}]]],
[case "${enableval}" in
 default)
   install_as_default=no
   installed_linker=ld.bfd
   ;;
 yes|no)
   install_as_default=yes
   installed_linker=ld.bfd
   ;;
 *)
   AC_MSG_ERROR([invalid --enable-gold argument])
   ;;
 esac],
[install_as_default=yes
 installed_linker=ld.bfd])
AC_SUBST(install_as_default)
AC_SUBST(installed_linker)

AC_ARG_ENABLE([got],
AS_HELP_STRING([--enable-got=<type>],
	       [GOT handling scheme (target, single, negative, multigot)]),
[case "${enableval}" in
  target | single | negative | multigot)  got_handling=$enableval ;;
  *)  AC_MSG_ERROR(bad value ${enableval} for --enable-got option) ;;
esac],
[got_handling=target])

case "${got_handling}" in
  target)
    AC_DEFINE([GOT_HANDLING_DEFAULT], [GOT_HANDLING_TARGET_DEFAULT],
	      [Define to choose default GOT handling scheme]) ;;
  single)
    AC_DEFINE([GOT_HANDLING_DEFAULT], [GOT_HANDLING_SINGLE],
	      [Define to choose default GOT handling scheme]) ;;
  negative)
    AC_DEFINE([GOT_HANDLING_DEFAULT], [GOT_HANDLING_NEGATIVE],
	      [Define to choose default GOT handling scheme]) ;;
  multigot)
    AC_DEFINE([GOT_HANDLING_DEFAULT], [GOT_HANDLING_MULTIGOT],
	      [Define to choose default GOT handling scheme]) ;;
  *)  AC_MSG_ERROR(bad value ${got_handling} for --enable-got option) ;;
esac

# PR gas/19109
# Decide the default method for compressing debug sections.
ac_default_compressed_debug_sections=unset
# Provide a configure time option to override our default.
AC_ARG_ENABLE(compressed_debug_sections,
	      AS_HELP_STRING([--enable-compressed-debug-sections={all,ld,none}],
	      [compress debug sections by default])],
[case ,"${enableval}", in
  ,yes, | ,all, | *,ld,*) ac_default_compressed_debug_sections=yes ;;
  ,no, | ,none,)  ac_default_compressed_debug_sections=no ;;
esac])dnl

# Select default compression algorithm.
ac_default_compressed_debug_sections_algorithm=COMPRESS_DEBUG_GABI_ZLIB
AC_ARG_ENABLE(default_compressed_debug_sections_algorithm,
	      AS_HELP_STRING([--enable-default-compressed-debug-sections-algorithm={zlib,zstd}],
	      [Default compression algorithm for --enable-compressed-debug-sections.]),
[case "${enableval}" in
   zstd) ac_default_compressed_debug_sections_algorithm=COMPRESS_DEBUG_ZSTD ;;
esac])dnl

# Decide setting DT_RUNPATH instead of DT_RPATH by default
ac_default_new_dtags=unset
# Provide a configure time option to override our default.
AC_ARG_ENABLE(new_dtags,
	      AS_HELP_STRING([--enable-new-dtags],
	      [set DT_RUNPATH instead of DT_RPATH by default])],
[case "${enableval}" in
  yes) ac_default_new_dtags=1 ;;
  no) ac_default_new_dtags=0 ;;
esac])dnl

# Decide if -z relro should be enabled in ELF linker by default.
ac_default_ld_z_relro=unset
# Provide a configure time option to override our default.
AC_ARG_ENABLE(relro,
	      AS_HELP_STRING([--enable-relro],
	      [enable -z relro in ELF linker by default]),
[case "${enableval}" in
  yes)  ac_default_ld_z_relro=1 ;;
  no)  ac_default_ld_z_relro=0 ;;
esac])dnl

# Decide if DT_TEXTREL check should be enabled in ELF linker.
ac_default_ld_textrel_check=unset
AC_ARG_ENABLE([textrel-check],
	      AC_HELP_STRING([--enable-textrel-check=@<:@yes|no|warning|error@:>@],
			     [enable DT_TEXTREL check in ELF linker]),
[case "${enableval}" in
  yes|no|warning|error) ac_default_ld_textrel_check=${enableval} ;;
esac])

# Decide if -z separate-code should be enabled in ELF linker by default.
ac_default_ld_z_separate_code=unset
AC_ARG_ENABLE(separate-code,
	      AS_HELP_STRING([--enable-separate-code],
	      [enable -z separate-code in ELF linker by default]),
[case "${enableval}" in
  yes) ac_default_ld_z_separate_code=1 ;;
  no) ac_default_ld_z_separate_code=0 ;;
esac])


# By default warn when an executable stack is created due to object files
# requesting such, not when the user specifies -z execstack.
ac_default_ld_warn_execstack=2
AC_ARG_ENABLE(warn-execstack,
	      AS_HELP_STRING([--enable-warn-execstack],
	      [enable warnings when creating an executable stack]),
[case "${enableval}" in
  yes) ac_default_ld_warn_execstack=1 ;;
  no)  ac_default_ld_warn_execstack=0 ;;
esac])

ac_default_ld_warn_rwx_segments=unset
AC_ARG_ENABLE(warn-rwx-segments,
	      AS_HELP_STRING([--enable-warn-rwx-segments],
	      [enable warnings when creating segements with RWX permissions]),
[case "${enableval}" in
  yes) ac_default_ld_warn_rwx_segments=1 ;;
  no)  ac_default_ld_warn_rwx_segments=0 ;;
esac])

ac_default_ld_default_execstack=unset
AC_ARG_ENABLE(default-execstack,
	      AS_HELP_STRING([--enable-default-execstack],
	      [create an executable stack if an input file is missing a .note.GNU-stack section]),
[case "${enableval}" in
  yes) ac_default_ld_default_execstack=1 ;;
  no)  ac_default_ld_default_execstack=0 ;;
esac])


# Decide if --error-handling-script should be supported.
ac_support_error_handling_script=unset
AC_ARG_ENABLE(error-handling-script,
	      AS_HELP_STRING([--enable-error-handling-script],
	      [enable/disable support for the --error-handling-script option]),
[case "${enableval}" in
  yes) ac_support_error_handling_script=1 ;;
  no)  ac_support_error_handling_script=0 ;;
esac])

# Decide which "--hash-style" to use by default
# Provide a configure time option to override our default.
AC_ARG_ENABLE([default-hash-style],
AS_HELP_STRING([--enable-default-hash-style={sysv,gnu,both}],
	       [use this default hash style]),
[case "${enable_default_hash_style}" in
  sysv | gnu | both) ;;
  *) AC_MSG_ERROR([bad value ${enable_default_hash_style} for enable-default-hash-style option]) ;;
esac],
[case "${target}" in
  # Enable gnu hash only on GNU targets, but not mips
  mips*-*-*) enable_default_hash_style=sysv ;;
  *-*-gnu* | *-*-linux* | *-*-nacl*) enable_default_hash_style=both ;;
  *) enable_default_hash_style=sysv ;;
esac])

case "${enable_default_hash_style}" in
  sysv | both) ac_default_emit_sysv_hash=1 ;;
  *) ac_default_emit_sysv_hash=0 ;;
esac

case "${enable_default_hash_style}" in
  gnu | both) ac_default_emit_gnu_hash=1 ;;
  *) ac_default_emit_gnu_hash=0 ;;
esac

AC_ARG_ENABLE(initfini-array,
[  --disable-initfini-array do not use .init_array/.fini_array sections],
[case "${enableval}" in
 yes|no) ;;
 *) AC_MSG_ERROR([invalid --enable-initfini-array argument]) ;;
 esac], [enable_initfini_array=yes])
AC_SUBST(enable_initfini_array)
if test $enable_initfini_array = yes; then
  AC_DEFINE(HAVE_INITFINI_ARRAY, 1,
    [Define .init_array/.fini_array sections are available and working.])
fi

GCC_ENABLE([libctf], [yes], [], [Handle .ctf type-info sections])
if test "${enable_libctf}" = yes; then
    AC_DEFINE(ENABLE_LIBCTF, 1, [Handle .ctf type-info sections])
fi
AM_CONDITIONAL(ENABLE_LIBCTF, test "${enable_libctf}" = yes)
AC_SUBST(enable_libctf)

# Used to validate --package-metadata= input. Disabled by default.
AC_ARG_ENABLE([jansson],
  [AS_HELP_STRING([--enable-jansson],
    [enable jansson [default=no]])],
  [enable_jansson=$enableval],
  [enable_jansson="no"])

AS_IF([test "x$enable_jansson" != "xno"],
  [PKG_CHECK_MODULES(JANSSON, [jansson],
    [
      AC_DEFINE(HAVE_JANSSON, 1, [The jansson library is to be used])
      AC_SUBST([JANSSON_CFLAGS])
      AC_SUBST([JANSSON_LIBS])
    ],
    [AC_MSG_ERROR([Cannot find jansson library])])
  ])

AM_BINUTILS_WARNINGS

AM_LC_MESSAGES

AC_CONFIG_HEADERS([config.h:config.in])

# PR 14072
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

# host-specific stuff:

ALL_LINGUAS="bg da de es fi fr ga id it ja pt_BR ru sr sv tr uk vi zh_CN zh_TW"
ZW_GNU_GETTEXT_SISTER_DIR
AM_PO_SUBDIRS

AC_EXEEXT

AC_PROG_YACC
AM_PROG_LEX

AM_MAINTAINER_MODE
AM_CONDITIONAL(GENINSRC_NEVER, false)
ACX_PROG_CMP_IGNORE_INITIAL

. ${srcdir}/configure.host

AC_SUBST(HDEFINES)
AC_SUBST(NATIVE_LIB_DIRS)

# We use headers from include/ that check various HAVE_*_H macros, thus
# should ensure they are set by configure.  This is true even when C99
# guarantees they are available.
# sha1.h and md4.h test HAVE_LIMITS_H, HAVE_SYS_TYPES_H and HAVE_STDINT_H
# plugin-api.h tests HAVE_STDINT_H and HAVE_INTTYPES_H
# Besides those, we need to check anything used in ld/ not in C99.
AC_CHECK_HEADERS(fcntl.h elf-hints.h limits.h inttypes.h stdint.h \
		 sys/file.h sys/mman.h sys/param.h sys/stat.h sys/time.h \
		 sys/types.h unistd.h)
AC_CHECK_FUNCS(close glob lseek mkstemp open realpath waitpid)

BFD_BINARY_FOPEN

AC_CHECK_DECLS([asprintf, environ])

AC_FUNC_MMAP

AC_SEARCH_LIBS([dlopen], [dl])

AC_MSG_CHECKING(for a known getopt prototype in unistd.h)
AC_CACHE_VAL(ld_cv_decl_getopt_unistd_h,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>], [extern int getopt (int, char *const*, const char *);])],
ld_cv_decl_getopt_unistd_h=yes, ld_cv_decl_getopt_unistd_h=no)])
AC_MSG_RESULT($ld_cv_decl_getopt_unistd_h)
if test $ld_cv_decl_getopt_unistd_h = yes; then
  AC_DEFINE([HAVE_DECL_GETOPT], 1,
	    [Is the prototype for getopt in <unistd.h> in the expected format?])
fi

# Link in zlib/zstd if we can.  This allows us to read and write
# compressed debug sections.
AM_ZLIB
AC_ZSTD

# When converting linker scripts into strings for use in emulation
# files, use astring.sed if the compiler supports ANSI string
# concatenation, or ostring.sed otherwise.  This is to support the
# broken Microsoft MSVC compiler, which limits the length of string
# constants, while still supporting pre-ANSI compilers which do not
# support string concatenation.
AC_MSG_CHECKING([whether ANSI C string concatenation works])
AC_CACHE_VAL(ld_cv_string_concatenation,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [char *a = "a" "a";])],
  ld_cv_string_concatenation=yes,
  ld_cv_string_concatenation=no)])
AC_MSG_RESULT($ld_cv_string_concatenation)
if test "$ld_cv_string_concatenation" = "yes"; then
  STRINGIFY=astring.sed
else
  STRINGIFY=ostring.sed
fi
AC_SUBST(STRINGIFY)

# target-specific stuff:

all_targets=
EMUL=
all_emuls=
all_emul_extras=
all_libpath=
TDIRS=

elf_list_options=false
elf_shlib_list_options=false
elf_plt_unwind_list_options=false
for targ_alias in `echo $target_alias $enable_targets | sed 's/,/ /g'`
do
  if test "$targ_alias" = "all"; then
    all_targets=true
    elf_list_options=true
    elf_shlib_list_options=true
    elf_plt_unwind_list_options=true
  else
    # Canonicalize the secondary target names.
    result=`$ac_config_sub $targ_alias 2>/dev/null`
    if test -n "$result"; then
	targ=$result
    else
	targ=$targ_alias
    fi

    . ${srcdir}/configure.tgt

    if test "$targ" = "$target"; then
      EMUL=$targ_emul
    fi

    if test x${enable_64_bit_bfd} = xno; then
      . ${srcdir}/../bfd/config.bfd
    fi

    if test x${enable_64_bit_bfd} = xyes; then
      targ_extra_emuls="$targ_extra_emuls $targ64_extra_emuls"
      targ_extra_libpath="$targ_extra_libpath $targ64_extra_libpath"
    fi

    for i in $targ_emul $targ_extra_emuls $targ_extra_libpath; do
	case " $all_emuls " in
	*" e${i}.o "*) ;;
	*)
	  all_emuls="$all_emuls e${i}.o"
	  eval result=\$tdir_$i
	  test -z "$result" && result=$targ_alias
	  TDIRS="$TDIRS
tdir_$i=$result"
	  case "${i}" in
	  *elf*)
	    elf_list_options=true
	    ;;
	  *)
	    if $GREP "TEMPLATE_NAME=elf" ${srcdir}/emulparams/${i}.sh >/dev/null 2>/dev/null; then
	      elf_list_options=true
	    fi
	    ;;
	  esac
	  if test "$elf_list_options" = "true"; then
	    source_sh()
	    {
	      . $1
	    }
	    source_sh ${srcdir}/emulparams/${i}.sh
	    if test x${GENERATE_SHLIB_SCRIPT} = xyes; then
	      elf_shlib_list_options=true
	    fi
	    if test x${PLT_UNWIND} = xyes; then
	      elf_plt_unwind_list_options=true
	    fi
	  fi
	  ;;
	esac
    done

    for i in $targ_emul $targ_extra_libpath; do
	case " $all_libpath " in
	*" ${i} "*) ;;
	*)
	  if test -z "$all_libpath"; then
	    all_libpath=${i}
	  else
	    all_libpath="$all_libpath ${i}"
	  fi
	  ;;
	esac
    done

    for i in $targ_extra_ofiles; do
	case " $all_emul_extras " in
	*" ${i} "*) ;;
	*)
	  all_emul_extras="$all_emul_extras ${i}"
	  ;;
	esac
    done

  fi
done

if test x$ac_default_compressed_debug_sections = xyes ; then
  AC_DEFINE(DEFAULT_FLAG_COMPRESS_DEBUG, 1, [Define if you want compressed debug sections by default.])
fi

AC_DEFINE_UNQUOTED(DEFAULT_COMPRESSED_DEBUG_ALGORITHM, $ac_default_compressed_debug_sections_algorithm,
		   [Default compression algorithm for --enable-compressed-debug-sections.])

if test "${ac_default_new_dtags}" = unset; then
  ac_default_new_dtags=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_NEW_DTAGS,
  $ac_default_new_dtags,
  [Define to 1 if you want to set DT_RUNPATH instead of DT_RPATH by default.])

if test "${ac_default_ld_z_relro}" = unset; then
  ac_default_ld_z_relro=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_LD_Z_RELRO,
  $ac_default_ld_z_relro,
  [Define to 1 if you want to enable -z relro in ELF linker by default.])

ac_default_ld_textrel_check_warning=0
case "${ac_default_ld_textrel_check}" in
  unset|no)
    ac_default_ld_textrel_check=textrel_check_none
    ;;
  yes|warning)
    ac_default_ld_textrel_check=textrel_check_warning
    ac_default_ld_textrel_check_warning=1
    ;;
  error)
    ac_default_ld_textrel_check=textrel_check_error
    ;;
esac
AC_DEFINE_UNQUOTED(DEFAULT_LD_TEXTREL_CHECK,
  $ac_default_ld_textrel_check,
  [The default method for DT_TEXTREL check in ELF linker.])
AC_DEFINE_UNQUOTED(DEFAULT_LD_TEXTREL_CHECK_WARNING,
  $ac_default_ld_textrel_check_warning,
  [Define to 1 if DT_TEXTREL check is warning in ELF linker by default.])

if test "${ac_default_ld_z_separate_code}" = unset; then
  ac_default_ld_z_separate_code=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_LD_Z_SEPARATE_CODE,
  $ac_default_ld_z_separate_code,
  [Define to 1 if you want to enable -z separate-code in ELF linker by default.])


AC_DEFINE_UNQUOTED(DEFAULT_LD_WARN_EXECSTACK,
  $ac_default_ld_warn_execstack,
  [Define to 1 if you want to enable --warn-execstack in ELF linker by default.])

if test "${ac_default_ld_warn_rwx_segments}" = unset; then
  ac_default_ld_warn_rwx_segments=1
fi
AC_DEFINE_UNQUOTED(DEFAULT_LD_WARN_RWX_SEGMENTS,
  $ac_default_ld_warn_rwx_segments,
  [Define to 0 if you want to disable --warn-rwx-segments in ELF linker by default.])

if test "${ac_default_ld_default_execstack}" = unset; then
  ac_default_ld_default_execstack=1
fi
AC_DEFINE_UNQUOTED(DEFAULT_LD_EXECSTACK,
  $ac_default_ld_default_execstack,
  [Define to 0 if you want to disable the generation of an executable stack when a .note-GNU-stack section is missing.])


if test "${ac_support_error_handling_script}" = unset; then
  ac_support_error_handling_script=1
fi
AC_DEFINE_UNQUOTED(SUPPORT_ERROR_HANDLING_SCRIPT,
  $ac_support_error_handling_script,
  [Define to 1 if you want to support the --error-handling-script command line option.])

AC_DEFINE_UNQUOTED([DEFAULT_EMIT_SYSV_HASH],
  [$ac_default_emit_sysv_hash],
  [Define to 1 if you want to emit sysv hash in the ELF linker by default.])

AC_DEFINE_UNQUOTED([DEFAULT_EMIT_GNU_HASH],
  [$ac_default_emit_gnu_hash],
  [Define to 1 if you want to emit gnu hash in the ELF linker by default.])

AC_SUBST(elf_list_options)
AC_SUBST(elf_shlib_list_options)
AC_SUBST(elf_plt_unwind_list_options)
AC_SUBST(EMUL)

AC_SUBST(TDIRS)
AM_SUBST_NOTMAKE(TDIRS)

if test x${all_targets} = xtrue; then
  if test x${enable_64_bit_bfd} = xyes; then
    EMULATION_OFILES='$(ALL_EMULATIONS) $(ALL_64_EMULATIONS)'
    EMUL_EXTRA_OFILES='$(ALL_EMUL_EXTRA_OFILES) $(ALL_64_EMUL_EXTRA_OFILES)'
  else
    EMULATION_OFILES='$(ALL_EMULATIONS)'
    EMUL_EXTRA_OFILES='$(ALL_EMUL_EXTRA_OFILES)'
  fi
else
  EMULATION_OFILES=$all_emuls
  EMUL_EXTRA_OFILES=$all_emul_extras
fi
AC_SUBST(EMULATION_OFILES)
AC_SUBST(EMUL_EXTRA_OFILES)
AC_SUBST(LIB_PATH)

EMULATION_LIBPATH=$all_libpath
AC_SUBST(EMULATION_LIBPATH)

if test x${enable_static} = xno; then
  TESTBFDLIB="-Wl,--rpath,../bfd/.libs ../bfd/.libs/libbfd.so"
  TESTCTFLIB="-Wl,--rpath,../libctf/.libs ../libctf/.libs/libctf.so"
  TESTSFRAMELIB="-Wl,--rpath,../libsframe/.libs ../libsframe/.libs/libsframe.so"
else
  TESTBFDLIB="../bfd/.libs/libbfd.a"
  TESTCTFLIB="../libctf/.libs/libctf.a"
  TESTSFRAMELIB="../libsframe/.libs/libsframe.a"
fi
if test "${enable_libctf}" = no; then
    TESTCTFLIB=
fi
AC_SUBST(TESTBFDLIB)
AC_SUBST(TESTCTFLIB)
AC_SUBST(TESTSFRAMELIB)

target_vendor=${target_vendor=$host_vendor}
case "$target_vendor" in
  hp) EXTRA_SHLIB_EXTENSION=".sl" ;;
  *)  EXTRA_SHLIB_EXTENSION= ;;
esac

case "$target_os" in
  lynxos) EXTRA_SHLIB_EXTENSION=".a" ;;
esac

if test x${EXTRA_SHLIB_EXTENSION} != x ; then
  AC_DEFINE_UNQUOTED(EXTRA_SHLIB_EXTENSION, "$EXTRA_SHLIB_EXTENSION",
   [Additional extension a shared object might have.])
fi

AC_CONFIG_COMMANDS([default],
[[
case "$srcdir" in
  .) srcdirpre= ;;
  *) srcdirpre='$(srcdir)/' ;;
esac
POFILES=
GMOFILES=
for lang in dummy $OBSOLETE_ALL_LINGUAS; do
  if test $lang != dummy; then
    POFILES="$POFILES $srcdirpre$lang.po"
    GMOFILES="$GMOFILES $srcdirpre$lang.gmo"
  fi
done
sed -e '/^SRC-POTFILES =/r po/SRC-POTFILES' \
    -e '/^BLD-POTFILES =/r po/BLD-POTFILES' \
    -e "s,@POFILES@,$POFILES," \
    -e "s,@GMOFILES@,$GMOFILES," \
    po/Makefile.in > po/Makefile]],[[]])

dnl Required by html, pdf, install-pdf and install-html
AC_SUBST(datarootdir)
AC_SUBST(docdir)
AC_SUBST(htmldir)
AC_SUBST(pdfdir)

AC_CONFIG_FILES(Makefile po/Makefile.in:po/Make-in)
AC_OUTPUT

GNU_MAKE_JOBSERVER
