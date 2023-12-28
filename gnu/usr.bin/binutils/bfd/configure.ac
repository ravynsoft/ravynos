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

m4_include([version.m4])
AC_INIT([bfd], BFD_VERSION)
AC_CONFIG_SRCDIR([libbfd.c])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

dnl These must be called before LT_INIT, because it may want
dnl to call AC_CHECK_PROG.
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib, :)

dnl Default to a non shared library.  This may be overridden by the
dnl configure option --enable-shared.
AC_DISABLE_SHARED

AC_PROG_CC
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS

LT_INIT([dlopen])

# AC_PLUGINS setting $plugins is called by ACX_LARGEFILE.
ACX_LARGEFILE

changequote(,)dnl
case "${target}" in
    hppa*64*-*-*) ;;
    *-*-*aout*| i[3-7]86-*-msdos* | ns32k-*-* | pdp11-*-*)
changequote([,])dnl
	if test "$plugins" = "yes"; then
	    if test "${enable_plugins+set}" = set; then
		AC_MSG_WARN(Enabling plugins for AOUT is experimental)
	    else
		plugins=no
	    fi
	fi ;;
    hppa*-*-hpux* | *-*-*vms* | \
    powerpc*-*-aix* | powerpc-*-beos* | powerpc-*-macos* | rs6000-*-*)
	if test "$plugins" = "yes"; then
	    if test "${enable_plugins+set}" = set; then
		AC_MSG_WARN(Enabling plugins may result in ar creating non-standard archives for ${target})
	    else
		plugins=no
	    fi
	fi ;;
esac

AM_CONDITIONAL(PLUGINS, test "$plugins" = "yes")

ac_checking=
. ${srcdir}/development.sh
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

BFD_64_BIT
if test $enable_64_bit_bfd = yes ; then
  want64=true
else
  want64=false
fi

AC_ARG_ENABLE(targets,
[  --enable-targets        alternative target configurations],
[case "${enableval}" in
  yes | "") AC_MSG_ERROR([enable-targets option must specify target names or 'all'])
            ;;
  no)       enable_targets= ;;
  *)        enable_targets=$enableval ;;
esac])dnl

AC_ARG_ENABLE(64_bit_archive,
	      AS_HELP_STRING([--enable-64-bit-archive],
	      [force 64-bit archives]),
[case "${enableval}" in
  yes)  want_64_bit_archive=true  ;;
  no)   want_64_bit_archive=false ;;
  *)    AC_MSG_ERROR(bad value ${enableval} for 64-bit-archive option) ;;
esac],[want_64_bit_archive=unset])dnl

AC_ARG_WITH(mmap,
[  --with-mmap             try using mmap for BFD input files if available],
[case "${withval}" in
  yes)  want_mmap=true ;;
  no)   want_mmap=false ;;
  *)    AC_MSG_ERROR(bad value ${withval} for BFD with-mmap option) ;;
esac],[want_mmap=false])dnl

AC_ARG_ENABLE(secureplt,
[  --enable-secureplt      Default to creating read-only plt entries],
[case "${enableval}" in
  yes)  use_secureplt=true  ;;
  no)   use_secureplt=false ;;
  *)    AC_MSG_ERROR(bad value ${enableval} for secureplt option) ;;
esac],[use_secureplt=true])dnl
if test $use_secureplt = true; then
  AC_DEFINE(USE_SECUREPLT, 1,
    [Define if we should default to creating read-only plt entries])
fi

# Decide if -z separate-code should be enabled in ELF linker by default.
ac_default_ld_z_separate_code=unset
AC_ARG_ENABLE(separate-code,
	      AS_HELP_STRING([--enable-separate-code],
	      [enable -z separate-code in ELF linker by default]),
[case "${enableval}" in
  yes) ac_default_ld_z_separate_code=1 ;;
  no) ac_default_ld_z_separate_code=0 ;;
esac])
# Enable -z separate-code by default for Linux/x86.
changequote(,)dnl
case "${target}" in
i[3-7]86-*-linux-* | x86_64-*-linux-*)
changequote([,])dnl
  if test ${ac_default_ld_z_separate_code} = unset; then
    ac_default_ld_z_separate_code=1
  fi
  ;;
esac
if test "${ac_default_ld_z_separate_code}" = unset; then
  ac_default_ld_z_separate_code=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_LD_Z_SEPARATE_CODE,
  $ac_default_ld_z_separate_code,
  [Define to 1 if you want to enable -z separate-code in ELF linker by default.])

AC_ARG_ENABLE(leading-mingw64-underscores,
  AS_HELP_STRING([--enable-leading-mingw64-underscores],
                 [Enable leading underscores on 64 bit mingw targets]),
  [],[])
AS_IF([ test x"$enable_leading_mingw64_underscores" = xyes ],
  [AC_DEFINE(USE_MINGW64_LEADING_UNDERSCORES, 1,
    [Define if we should use leading underscore on 64 bit mingw targets])])

DEBUGDIR=${libdir}/debug
AC_ARG_WITH(separate-debug-dir,
  AS_HELP_STRING([--with-separate-debug-dir=DIR],
                 [Look for global separate debug info in DIR [[default=LIBDIR/debug]]]),
[DEBUGDIR="${withval}"])
AC_SUBST(DEBUGDIR)

ACX_PKGVERSION([GNU Binutils])
ACX_BUGURL([https://sourceware.org/bugzilla/])

AM_BINUTILS_WARNINGS

AC_CONFIG_HEADERS(config.h:config.in)

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

AM_MAINTAINER_MODE
AM_CONDITIONAL(GENINSRC_NEVER, false)
AM_INSTALL_LIBBFD
AC_EXEEXT

AC_C_BIGENDIAN

host64=false
target64=false
bfd_default_target_size=32

# host stuff:

ALL_LINGUAS="da es fi fr hr id ja ro ru rw sr sv tr uk vi zh_CN pt"
ZW_GNU_GETTEXT_SISTER_DIR
AM_PO_SUBDIRS

# Permit host specific settings.
. ${srcdir}/configure.host

AC_SUBST(HDEFINES)
AC_PROG_INSTALL

AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)

if test "x${ac_cv_sizeof_void_p}" = "x8"; then
  host64=true
fi

BFD_CC_FOR_BUILD

AC_CHECK_HEADERS(fcntl.h sys/file.h sys/resource.h sys/stat.h sys/types.h \
		 unistd.h)

AC_CHECK_FUNCS(fcntl fdopen fileno fls getgid getpagesize getrlimit getuid \
	       sysconf)

AC_CHECK_DECLS([basename, ffs, stpcpy, asprintf, vasprintf, strnlen])
AC_CHECK_DECLS([___lc_codepage_func], [], [], [[#include <locale.h>]])

BFD_BINARY_FOPEN

# Link in zlib/zstd if we can.  This allows us to read compressed debug sections.
# This is used only by compress.c.
AM_ZLIB
AC_ZSTD

save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Werror"
AC_CACHE_CHECK([compiler support for hidden visibility], bfd_cv_hidden,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
const char hw[] __attribute__ ((__visibility__ ("hidden"))) = "Hello, World\n";
extern void print (const char *) __attribute__ ((__visibility__ ("hidden")));]],
	[[print (hw);]])],
  [bfd_cv_hidden=yes], [bfd_cv_hidden=no])])
CFLAGS="$save_CFLAGS"
if test $bfd_cv_hidden = yes; then
  AC_DEFINE(HAVE_HIDDEN, 1,
	    [Define if your compiler supports hidden visibility.])
fi

# Check if linker supports --as-needed and --no-as-needed options
AC_CACHE_CHECK(linker --as-needed support, bfd_cv_ld_as_needed,
	[bfd_cv_ld_as_needed=no
	if $LD --help 2>/dev/null | grep as-needed > /dev/null; then
		bfd_cv_ld_as_needed=yes
	fi
	])

LT_LIB_M

# When building a shared libbfd, link against the pic version of libiberty
# so that apps that use libbfd won't need libiberty just to satisfy any
# libbfd references.
# We can't do that if a pic libiberty is unavailable since including non-pic
# code would insert text relocations into libbfd.
SHARED_LIBADD=
SHARED_LDFLAGS=
if test "$enable_shared" = "yes"; then
changequote(,)dnl
  x=`sed -n -e 's/^[ 	]*PICFLAG[ 	]*=[ 	]*//p' < ../libiberty/Makefile | sed -n '$p'`
changequote([,])dnl
  if test -n "$x"; then
    SHARED_LIBADD="-L`pwd`/../libiberty/pic -liberty"
  fi
fi

SHARED_LIBADD="$SHARED_LIBADD $LIBINTL"

if test "$enable_shared" = "yes"; then
  case "${host}" in
  # More hacks to build DLLs on Windows.
  *-*-cygwin*)
    SHARED_LDFLAGS="-no-undefined"
    SHARED_LIBADD="-L`pwd`/../libiberty -liberty $SHARED_LIBADD -lcygwin -lkernel32"
  ;;
  esac

  if test -n "$SHARED_LIBADD"; then
    if test -n "$LIBM"; then
      if test x"$bfd_cv_ld_as_needed" = xyes; then
	# Link against libm only when needed.  Put -lc, -lm inside -Wl
	# to stop libtool reordering these options.
	SHARED_LIBADD="$SHARED_LIBADD -Wl,-lc,--as-needed,`echo $LIBM | sed 's/ /,/g'`,--no-as-needed"
      else
	SHARED_LIBADD="$SHARED_LIBADD $LIBM"
      fi
    fi
  fi
fi
AC_SUBST(SHARED_LDFLAGS)
AC_SUBST(SHARED_LIBADD)

# target stuff:

# Canonicalize the secondary target names.
if test -n "$enable_targets" ; then
    for targ in `echo $enable_targets | sed 's/,/ /g'`
    do
	result=`$ac_config_sub $targ 2>/dev/null`
	if test -n "$result" ; then
	    canon_targets="$canon_targets $result"
	else
	    # Allow targets that config.sub doesn't recognize, like "all".
	    canon_targets="$canon_targets $targ"
	fi
    done
fi

all_targets=false
defvec=
selvecs=
assocvecs=
selarchs=
TDEFINES=
for targ in $target $canon_targets
do
    if test $targ = all; then
        all_targets=true
	assocvecs="$assocvecs $targ_defvec $targ_selvecs"
    elif test $targ != plugin; then
	. $srcdir/config.bfd
	if test $targ = $target; then
	    defvec=$targ_defvec
	fi
	selvecs="$selvecs $targ_defvec $targ_selvecs"
	selarchs="$selarchs $targ_archs"
	TDEFINES="$TDEFINES $targ_cflags"
    fi
done
AC_SUBST(TDEFINES)

# This processing still needs to be done if we're to decide properly whether
# 64-bit support needs to be compiled in.  Currently, it will be included if
# the default or any other explicitly requested target requires it; it
# will not be included on a 32-bit host if no 64-bit target is requested, and
# no "--with-64-bit-bfd" option is given, even if "--enable-targets=all" is
# used.

# uniq the default and selected vectors in all the configured targets.
f=""
for i in $selvecs ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
selvecs="$f"


# uniq the associated vectors in all the configured targets.
f=""
for i in $assocvecs ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
assocvecs="$f"


# uniq the architectures in all the configured targets.
f=""
for i in $selarchs ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
selarchs="$f"

# Target backend .o files.
tb=

elf="elf.lo elflink.lo elf-attrs.lo elf-strtab.lo elf-eh-frame.lo
     elf-sframe.lo dwarf1.lo dwarf2.lo"
coffgen="coffgen.lo dwarf2.lo"
coff="cofflink.lo $coffgen"
ecoff="ecofflink.lo $coffgen"
xcoff="xcofflink.lo $coffgen"

elfxx_x86="elfxx-x86.lo elf-ifunc.lo elf-vxworks.lo"
for vec in $selvecs
do
    target_size=32
    case "$vec" in
    # This list is alphabetized to make it easy to compare
    # with the two vector lists in targets.c.  For the same reason,
    # use one entry per line, even though this leads to long lines.
    aarch64_elf32_be_vec)	 tb="$tb elf32-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf32.lo $elf"; target_size=64 ;;
    aarch64_elf32_le_vec)	 tb="$tb elf32-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf32.lo $elf"; target_size=64 ;;
    aarch64_elf64_be_vec)	   tb="$tb elf64-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf64.lo $elf"; target_size=64 ;;
    aarch64_elf64_be_cloudabi_vec) tb="$tb elf64-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf64.lo $elf"; target_size=64 ;;
    aarch64_elf64_le_vec)	   tb="$tb elf64-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf64.lo $elf"; target_size=64 ;;
    aarch64_elf64_le_cloudabi_vec) tb="$tb elf64-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf64.lo $elf"; target_size=64 ;;
    aarch64_mach_o_vec)		 tb="$tb mach-o-aarch64.lo"; target_size=64 ;;
    aarch64_pei_le_vec)		 tb="$tb pei-aarch64.lo pe-aarch64igen.lo $coff"; target_size=64 ;;
    aarch64_pe_le_vec)		 tb="$tb pe-aarch64.lo pe-aarch64igen.lo $coff"; target_size=64 ;;
    alpha_ecoff_le_vec)		 tb="$tb coff-alpha.lo ecoff.lo $ecoff"; target_size=64 ;;
    alpha_elf64_vec)		 tb="$tb elf64-alpha.lo elf64.lo $elf"; target_size=64 ;;
    alpha_elf64_fbsd_vec)	 tb="$tb elf64-alpha.lo elf64.lo $elf"; target_size=64 ;;
    alpha_vms_vec)		 tb="$tb vms-alpha.lo vms-misc.lo vms-lib.lo"; target_size=64 ;;
    alpha_vms_lib_txt_vec)	 tb="$tb vms-lib.lo vms-misc.lo" ;;
    am33_elf32_linux_vec)	 tb="$tb elf32-am33lin.lo elf32.lo $elf" ;;
    amdgcn_elf64_le_vec)	 tb="$tb elf64-amdgcn.lo elf64.lo $elf"; target_size=64 ;;
    aout0_be_vec)		 tb="$tb aout0.lo aout32.lo" ;;
    aout64_vec)			 tb="$tb demo64.lo aout64.lo"; target_size=64 ;;
    aout_vec)			 tb="$tb host-aout.lo aout32.lo" ;;
    arc_elf32_be_vec)		 tb="$tb elf32-arc.lo elf32.lo $elf" ;;
    arc_elf32_le_vec)		 tb="$tb elf32-arc.lo elf32.lo $elf" ;;
    arm_elf32_be_vec)		 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_le_vec)		 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_fdpic_be_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_fdpic_le_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_nacl_be_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_nacl_le_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_vxworks_be_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_vxworks_le_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_pe_be_vec)		 tb="$tb pe-arm.lo peigen.lo $coff" ;;
    arm_pe_le_vec)		 tb="$tb pe-arm.lo peigen.lo $coff" ;;
    arm_pe_wince_be_vec)	 tb="$tb pe-arm-wince.lo pe-arm.lo peigen.lo $coff" ;;
    arm_pe_wince_le_vec)	 tb="$tb pe-arm-wince.lo pe-arm.lo peigen.lo $coff" ;;
    arm_pei_be_vec)		 tb="$tb pei-arm.lo peigen.lo $coff" ;;
    arm_pei_le_vec)		 tb="$tb pei-arm.lo peigen.lo $coff" ;;
    arm_pei_wince_be_vec)	 tb="$tb pei-arm-wince.lo pei-arm.lo peigen.lo $coff" ;;
    arm_pei_wince_le_vec)	 tb="$tb pei-arm-wince.lo pei-arm.lo peigen.lo $coff" ;;
    arm_mach_o_vec)		 tb="$tb mach-o-arm.lo" ;;
    avr_elf32_vec)		 tb="$tb elf32-avr.lo elf32.lo $elf" ;;
    bfin_elf32_vec)		 tb="$tb elf32-bfin.lo elf32.lo $elf" ;;
    bfin_elf32_fdpic_vec)	 tb="$tb elf32-bfin.lo elf32.lo $elf" ;;
    cr16_elf32_vec)		 tb="$tb elf32-cr16.lo elf32.lo $elf" ;;
    cris_aout_vec)		 tb="$tb aout-cris.lo" ;;
    cris_elf32_vec)		 tb="$tb elf32-cris.lo elf32.lo $elf" ;;
    cris_elf32_us_vec)		 tb="$tb elf32-cris.lo elf32.lo $elf" ;;
    crx_elf32_vec)		 tb="$tb elf32-crx.lo elf32.lo $elf" ;;
    csky_elf32_be_vec)		 tb="$tb elf32-csky.lo elf32.lo $elf" ;;
    csky_elf32_le_vec)		 tb="$tb elf32-csky.lo elf32.lo $elf" ;;
    d10v_elf32_vec)		 tb="$tb elf32-d10v.lo elf32.lo $elf" ;;
    d30v_elf32_vec)		 tb="$tb elf32-d30v.lo elf32.lo $elf" ;;
    dlx_elf32_be_vec)		 tb="$tb elf32-dlx.lo elf32.lo $elf" ;;
    elf32_be_vec)		 tb="$tb elf32-gen.lo elf32.lo $elf" ;;
    elf32_le_vec)		 tb="$tb elf32-gen.lo elf32.lo $elf" ;;
    elf64_be_vec)		 tb="$tb elf64-gen.lo elf64.lo $elf"; target_size=64 ;;
    elf64_le_vec)		 tb="$tb elf64-gen.lo elf64.lo $elf"; target_size=64 ;;
    bpf_elf64_le_vec)		 tb="$tb elf64-bpf.lo elf64.lo $elf"; target_size=64;;
    bpf_elf64_be_vec)		 tb="$tb elf64-bpf.lo elf64.lo $elf"; target_size=64 ;;
    epiphany_elf32_vec)		 tb="$tb elf32-epiphany.lo elf32.lo $elf" ;;
    fr30_elf32_vec)		 tb="$tb elf32-fr30.lo elf32.lo $elf" ;;
    frv_elf32_vec)		 tb="$tb elf32-frv.lo elf32.lo $elf" ;;
    frv_elf32_fdpic_vec)	 tb="$tb elf32-frv.lo elf32.lo $elf" ;;
    h8300_elf32_vec)		 tb="$tb elf32-h8300.lo elf32.lo $elf" ;;
    h8300_elf32_linux_vec)	 tb="$tb elf32-h8300.lo elf32.lo $elf" ;;
    hppa_elf32_vec)		 tb="$tb elf32-hppa.lo elf32.lo $elf" ;;
    hppa_elf32_linux_vec)	 tb="$tb elf32-hppa.lo elf32.lo $elf" ;;
    hppa_elf32_nbsd_vec)	 tb="$tb elf32-hppa.lo elf32.lo $elf" ;;
    hppa_elf64_vec)		 tb="$tb elf64-hppa.lo elf64.lo $elf"; target_size=64 ;;
    hppa_elf64_linux_vec)	 tb="$tb elf64-hppa.lo elf64.lo $elf"; target_size=64 ;;
    hppa_som_vec)		 tb="$tb som.lo" ;;
    i386_aout_vec)		 tb="$tb i386aout.lo aout32.lo" ;;
    i386_aout_bsd_vec)		 tb="$tb i386bsd.lo aout32.lo" ;;
    i386_aout_lynx_vec)		 tb="$tb i386lynx.lo lynx-core.lo aout32.lo" ;;
    i386_coff_vec)		 tb="$tb coff-i386.lo $coff" ;;
    i386_coff_go32_vec)		 tb="$tb coff-go32.lo $coff" ;;
    i386_coff_go32stubbed_vec)	 tb="$tb coff-stgo32.lo $coff" ;;
    i386_coff_lynx_vec)		 tb="$tb cf-i386lynx.lo lynx-core.lo $coff" ;;
    i386_elf32_vec)		 tb="$tb elf32-i386.lo $elfxx_x86 elf32.lo $elf" ;;
    i386_elf32_fbsd_vec)	 tb="$tb elf32-i386.lo $elfxx_x86 elf32.lo $elf" ;;
    i386_elf32_sol2_vec)	 tb="$tb elf32-i386.lo $elfxx_x86 elf32.lo $elf" ;;
    i386_elf32_vxworks_vec)	 tb="$tb elf32-i386.lo $elfxx_x86 elf32.lo $elf" ;;
    i386_mach_o_vec)		 tb="$tb mach-o-i386.lo" ;;
    i386_msdos_vec)		 tb="$tb i386msdos.lo" ;;
    i386_pe_vec)		 tb="$tb pe-i386.lo peigen.lo $coff" ;;
    i386_pe_big_vec)		 tb="$tb pe-i386.lo peigen.lo $coff" ;;
    i386_pei_vec)		 tb="$tb pei-i386.lo peigen.lo $coff" ;;
    iamcu_elf32_vec)		 tb="$tb elf32-i386.lo $elfxx_x86 elf32.lo $elf" ;;
    ia64_elf32_be_vec)		 tb="$tb elf32-ia64.lo elfxx-ia64.lo elf32.lo $elf" ;;
    ia64_elf32_hpux_be_vec)	 tb="$tb elf32-ia64.lo elfxx-ia64.lo elf32.lo $elf" ;;
    ia64_elf64_be_vec)		 tb="$tb elf64-ia64.lo elfxx-ia64.lo elf64.lo $elf"; target_size=64 ;;
    ia64_elf64_le_vec)		 tb="$tb elf64-ia64.lo elfxx-ia64.lo elf64.lo $elf"; target_size=64 ;;
    ia64_elf64_hpux_be_vec)	 tb="$tb elf64-ia64.lo elfxx-ia64.lo elf64.lo $elf"; target_size=64 ;;
    ia64_elf64_vms_vec)		 tb="$tb elf64-ia64-vms.lo elf64-ia64.lo elfxx-ia64.lo elf64.lo vms-lib.lo vms-misc.lo $elf"; target_size=64 ;;
    ia64_pei_vec)		 tb="$tb pei-ia64.lo pepigen.lo $coff"; target_size=64 ;;
    ip2k_elf32_vec)		 tb="$tb elf32-ip2k.lo elf32.lo $elf" ;;
    iq2000_elf32_vec)		 tb="$tb elf32-iq2000.lo elf32.lo $elf" ;;
    lm32_elf32_vec)		 tb="$tb elf32-lm32.lo elf32.lo $elf" ;;
    lm32_elf32_fdpic_vec)	 tb="$tb elf32-lm32.lo elf32.lo $elf" ;;
    loongarch_elf32_vec)	 tb="$tb elf32-loongarch.lo elfxx-loongarch.lo elf32.lo elf-ifunc.lo $elf" ;;
    loongarch_elf64_vec)	 tb="$tb elf64-loongarch.lo elf64.lo elfxx-loongarch.lo elf32.lo elf-ifunc.lo $elf"; target_size=64 ;;
    loongarch64_pei_vec)	 tb="$tb pei-loongarch64.lo pe-loongarch64igen.lo $coff"; target_size=64 ;;
    m32c_elf32_vec)		 tb="$tb elf32-m32c.lo elf32.lo $elf" ;;
    m32r_elf32_vec)		 tb="$tb elf32-m32r.lo elf32.lo $elf" ;;
    m32r_elf32_le_vec)		 tb="$tb elf32-m32r.lo elf32.lo $elf" ;;
    m32r_elf32_linux_vec)	 tb="$tb elf32-m32r.lo elf32.lo $elf" ;;
    m32r_elf32_linux_le_vec)	 tb="$tb elf32-m32r.lo elf32.lo $elf" ;;
    m68hc11_elf32_vec)		 tb="$tb elf32-m68hc11.lo elf32-m68hc1x.lo elf32.lo $elf" ;;
    m68hc12_elf32_vec)		 tb="$tb elf32-m68hc12.lo elf32-m68hc1x.lo elf32.lo $elf" ;;
    m68k_elf32_vec)		 tb="$tb elf32-m68k.lo elf32.lo $elf" ;;
    s12z_elf32_vec)		 tb="$tb elf32-s12z.lo elf32.lo $elf" ;;
    mach_o_be_vec)		 tb="$tb mach-o.lo dwarf2.lo" ;;
    mach_o_le_vec)		 tb="$tb mach-o.lo dwarf2.lo" ;;
    mach_o_fat_vec)		 tb="$tb mach-o.lo dwarf2.lo" ;;
    mcore_elf32_be_vec)		 tb="$tb elf32-mcore.lo elf32.lo $elf" ;;
    mcore_elf32_le_vec)		 tb="$tb elf32-mcore.lo elf32.lo $elf" ;;
    mcore_pe_be_vec)		 tb="$tb pe-mcore.lo peigen.lo $coff" ;;
    mcore_pe_le_vec)		 tb="$tb pe-mcore.lo peigen.lo $coff" ;;
    mcore_pei_be_vec)		 tb="$tb pei-mcore.lo peigen.lo $coff" ;;
    mcore_pei_le_vec)		 tb="$tb pei-mcore.lo peigen.lo $coff" ;;
    mep_elf32_vec)		 tb="$tb elf32-mep.lo elf32.lo $elf" ;;
    mep_elf32_le_vec)		 tb="$tb elf32-mep.lo elf32.lo $elf" ;;
    metag_elf32_vec)		 tb="$tb elf32-metag.lo elf32.lo $elf" ;;
    microblaze_elf32_vec)	 tb="$tb elf32-microblaze.lo elf32.lo $elf" ;;
    microblaze_elf32_le_vec)	 tb="$tb elf32-microblaze.lo elf32.lo $elf" ;;
    mips_ecoff_be_vec)		 tb="$tb coff-mips.lo ecoff.lo $ecoff" ;;
    mips_ecoff_le_vec)		 tb="$tb coff-mips.lo ecoff.lo $ecoff" ;;
    mips_ecoff_bele_vec)	 tb="$tb coff-mips.lo ecoff.lo $ecoff" ;;
    mips_elf32_be_vec)		 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff" ;;
    mips_elf32_le_vec)		 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff" ;;
    mips_elf32_n_be_vec)	 tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf32_n_le_vec)	 tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf32_ntrad_be_vec)	 tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf32_ntrad_le_vec)	 tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf32_ntradfbsd_be_vec) tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf32_ntradfbsd_le_vec) tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf32_trad_be_vec)	 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff" ;;
    mips_elf32_trad_le_vec)	 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff" ;;
    mips_elf32_tradfbsd_be_vec)  tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff" ;;
    mips_elf32_tradfbsd_le_vec)  tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff" ;;
    mips_elf32_vxworks_be_vec)	 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff" ;;
    mips_elf32_vxworks_le_vec)	 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff" ;;
    mips_elf64_be_vec)		 tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf64_le_vec)		 tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf64_trad_be_vec)	 tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf64_trad_le_vec)	 tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf64_tradfbsd_be_vec)  tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mips_elf64_tradfbsd_le_vec)  tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf $ecoff"; target_size=64 ;;
    mmix_elf64_vec)		 tb="$tb elf64-mmix.lo elf64.lo $elf" target_size=64 ;;
    mmix_mmo_vec)			 tb="$tb mmo.lo" target_size=64 ;;
    mn10200_elf32_vec)		 tb="$tb elf-m10200.lo elf32.lo $elf" ;;
    mn10300_elf32_vec)		 tb="$tb elf-m10300.lo elf32.lo $elf" ;;
    moxie_elf32_be_vec)		 tb="$tb elf32-moxie.lo elf32.lo $elf" ;;
    moxie_elf32_le_vec)		 tb="$tb elf32-moxie.lo elf32.lo $elf" ;;
    msp430_elf32_vec)		 tb="$tb elf32-msp430.lo elf32.lo $elf" ;;
    msp430_elf32_ti_vec)	 tb="$tb elf32-msp430.lo elf32.lo $elf" ;;
    mt_elf32_vec)		 tb="$tb elf32-mt.lo elf32.lo $elf" ;;
    nds32_elf32_be_vec)		 tb="$tb elf32-nds32.lo elf32.lo $elf" ;;
    nds32_elf32_le_vec)		 tb="$tb elf32-nds32.lo elf32.lo $elf" ;;
    nds32_elf32_linux_be_vec)	 tb="$tb elf32-nds32.lo elf32.lo $elf" ;;
    nds32_elf32_linux_le_vec)	 tb="$tb elf32-nds32.lo elf32.lo $elf" ;;
    nfp_elf64_vec)		 tb="$tb elf64-nfp.lo elf64.lo $elf" ;;
    nios2_elf32_be_vec)		 tb="$tb elf32-nios2.lo elf32.lo $elf" ;;
    nios2_elf32_le_vec)		 tb="$tb elf32-nios2.lo elf32.lo $elf" ;;
    ns32k_aout_pc532mach_vec)	 tb="$tb pc532-mach.lo aout-ns32k.lo" ;;
    ns32k_aout_pc532nbsd_vec)	 tb="$tb ns32knetbsd.lo aout-ns32k.lo" ;;
    or1k_elf32_vec)		 tb="$tb elf32-or1k.lo elf32.lo $elf" ;;
    pdb_vec)			 tb="$tb pdb.lo" ;;
    pdp11_aout_vec)		 tb="$tb pdp11.lo" ;;
    pef_vec)			 tb="$tb pef.lo" ;;
    pef_xlib_vec)		 tb="$tb pef.lo" ;;
    pj_elf32_vec)		 tb="$tb elf32-pj.lo elf32.lo $elf" ;;
    pj_elf32_le_vec)		 tb="$tb elf32-pj.lo elf32.lo $elf" ;;
    powerpc_boot_vec)		 tb="$tb ppcboot.lo" ;;
    powerpc_elf32_vec)		 tb="$tb elf32-ppc.lo elf-vxworks.lo elf32.lo $elf" ;;
    powerpc_elf32_le_vec)	 tb="$tb elf32-ppc.lo elf-vxworks.lo elf32.lo $elf" ;;
    powerpc_elf32_fbsd_vec)	 tb="$tb elf32-ppc.lo elf-vxworks.lo elf32.lo $elf" ;;
    powerpc_elf32_vxworks_vec)	 tb="$tb elf32-ppc.lo elf-vxworks.lo elf32.lo $elf" ;;
    powerpc_elf64_vec)		 tb="$tb elf64-ppc.lo elf64-gen.lo elf64.lo $elf"; target_size=64 ;;
    powerpc_elf64_le_vec)	 tb="$tb elf64-ppc.lo elf64-gen.lo elf64.lo $elf" target_size=64 ;;
    powerpc_elf64_fbsd_vec)	 tb="$tb elf64-ppc.lo elf64-gen.lo elf64.lo $elf" target_size=64 ;;
    powerpc_elf64_fbsd_le_vec)	 tb="$tb elf64-ppc.lo elf64-gen.lo elf64.lo $elf" target_size=64 ;;
    powerpc_xcoff_vec)		 tb="$tb coff-rs6000.lo $xcoff" ;;
    pru_elf32_vec)		 tb="$tb elf32-pru.lo elf32.lo $elf" ;;
    riscv_elf32_vec)		 tb="$tb elf32-riscv.lo elfxx-riscv.lo elf-ifunc.lo elf32.lo $elf" ;;
    riscv_elf64_vec)		 tb="$tb elf64-riscv.lo elf64.lo elfxx-riscv.lo elf-ifunc.lo elf32.lo $elf"; target_size=64 ;;
    riscv_elf32_be_vec)		 tb="$tb elf32-riscv.lo elfxx-riscv.lo elf-ifunc.lo elf32.lo $elf" ;;
    riscv_elf64_be_vec)		 tb="$tb elf64-riscv.lo elf64.lo elfxx-riscv.lo elf-ifunc.lo elf32.lo $elf"; target_size=64 ;;
    rl78_elf32_vec)		 tb="$tb elf32-rl78.lo elf32.lo $elf" ;;
    rs6000_xcoff64_vec)		 tb="$tb coff64-rs6000.lo aix5ppc-core.lo $xcoff"; target_size=64 ;;
    rs6000_xcoff64_aix_vec)	 tb="$tb coff64-rs6000.lo aix5ppc-core.lo $xcoff"; target_size=64 ;;
    rs6000_xcoff_vec)		 tb="$tb coff-rs6000.lo $xcoff" ;;
    rx_elf32_be_vec)		 tb="$tb elf32-rx.lo elf32.lo $elf" ;;
    rx_elf32_be_ns_vec)		 tb="$tb elf32-rx.lo elf32.lo $elf" ;;
    rx_elf32_le_vec)		 tb="$tb elf32-rx.lo elf32.lo $elf" ;;
    rx_elf32_linux_le_vec)	 tb="$tb elf32-rx.lo elf32.lo $elf" ;;
    s390_elf32_vec)		 tb="$tb elf32-s390.lo elf32.lo $elf" ;;
    s390_elf64_vec)		 tb="$tb elf64-s390.lo elf64.lo $elf"; target_size=64 ;;
    score_elf32_be_vec)		 tb="$tb elf32-score.lo elf32-score7.lo elf32.lo elf64.lo $elf"; want64=true; target_size=64 ;;
    score_elf32_le_vec)		 tb="$tb elf32-score.lo elf32-score7.lo elf32.lo elf64.lo $elf"; want64=true; target_size=64 ;;
    sh_coff_vec)		 tb="$tb coff-sh.lo $coff" ;;
    sh_coff_le_vec)		 tb="$tb coff-sh.lo $coff" ;;
    sh_coff_small_vec)		 tb="$tb coff-sh.lo $coff" ;;
    sh_coff_small_le_vec)	 tb="$tb coff-sh.lo $coff" ;;
    sh_elf32_vec)		 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_le_vec)		 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_fdpic_be_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_fdpic_le_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_linux_vec)		 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_linux_be_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_nbsd_vec)		 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_nbsd_le_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_vxworks_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_elf32_vxworks_le_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo $coff" ;;
    sh_pe_le_vec)		 tb="$tb pe-sh.lo coff-sh.lo peigen.lo $coff" ;;
    sh_pei_le_vec)		 tb="$tb pei-sh.lo coff-sh.lo peigen.lo $coff" ;;
    sparc_elf32_vec)		 tb="$tb elf32-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf32.lo $elf" ;;
    sparc_elf32_sol2_vec)	 tb="$tb elf32-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf32.lo $elf" ;;
    sparc_elf32_vxworks_vec)	 tb="$tb elf32-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf32.lo $elf" ;;
    sparc_elf64_vec)		 tb="$tb elf64-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf64.lo $elf"; target_size=64 ;;
    sparc_elf64_fbsd_vec)	 tb="$tb elf64-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf64.lo $elf"; target_size=64 ;;
    sparc_elf64_sol2_vec)	 tb="$tb elf64-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf64.lo $elf"; target_size=64 ;;
    spu_elf32_vec)		 tb="$tb elf32-spu.lo elf32.lo $elf" ;;
    sym_vec)			 tb="$tb xsym.lo" ;;
    tic30_coff_vec)		 tb="$tb coff-tic30.lo $coffgen" ;;
    tic4x_coff0_vec)		 tb="$tb coff-tic4x.lo $coffgen" ;;
    tic4x_coff0_beh_vec)	 tb="$tb coff-tic4x.lo $coffgen" ;;
    tic4x_coff1_vec)		 tb="$tb coff-tic4x.lo $coffgen" ;;
    tic4x_coff1_beh_vec)	 tb="$tb coff-tic4x.lo $coffgen" ;;
    tic4x_coff2_vec)		 tb="$tb coff-tic4x.lo $coffgen" ;;
    tic4x_coff2_beh_vec)	 tb="$tb coff-tic4x.lo $coffgen" ;;
    tic54x_coff0_vec)		 tb="$tb coff-tic54x.lo $coffgen" ;;
    tic54x_coff0_beh_vec)	 tb="$tb coff-tic54x.lo $coffgen" ;;
    tic54x_coff1_vec)		 tb="$tb coff-tic54x.lo $coffgen" ;;
    tic54x_coff1_beh_vec)	 tb="$tb coff-tic54x.lo $coffgen" ;;
    tic54x_coff2_vec)		 tb="$tb coff-tic54x.lo $coffgen" ;;
    tic54x_coff2_beh_vec)	 tb="$tb coff-tic54x.lo $coffgen" ;;
    tic6x_elf32_be_vec)		 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_le_vec)		 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_c6000_be_vec)	 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_c6000_le_vec)	 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_linux_be_vec)	 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_linux_le_vec)	 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tilegx_elf32_be_vec)	 tb="$tb elf32-tilegx.lo elfxx-tilegx.lo elf32.lo $elf" ; target_size=32 ;;
    tilegx_elf32_le_vec)	 tb="$tb elf32-tilegx.lo elfxx-tilegx.lo elf32.lo $elf" ; target_size=32 ;;
    tilegx_elf64_be_vec)	 tb="$tb elf64-tilegx.lo elfxx-tilegx.lo elf64.lo $elf" ; target_size=64 ;;
    tilegx_elf64_le_vec)	 tb="$tb elf64-tilegx.lo elfxx-tilegx.lo elf64.lo $elf" ; target_size=64 ;;
    tilepro_elf32_vec)		 tb="$tb elf32-tilepro.lo elf32.lo $elf" ;;
    v800_elf32_vec)		 tb="$tb elf32-v850.lo elf32.lo $elf" ;;
    v850_elf32_vec)		 tb="$tb elf32-v850.lo elf32.lo $elf" ;;
    vax_aout_1knbsd_vec)	 tb="$tb vax1knetbsd.lo aout32.lo" ;;
    vax_aout_nbsd_vec)		 tb="$tb vaxnetbsd.lo aout32.lo" ;;
    vax_elf32_vec)		 tb="$tb elf32-vax.lo elf32.lo $elf" ;;
    ft32_elf32_vec)		 tb="$tb elf32-ft32.lo elf32.lo $elf" ;;
    visium_elf32_vec)		 tb="$tb elf32-visium.lo elf32.lo $elf" ;;
    wasm_vec)                    tb="$tb wasm-module.lo" ;;
    wasm32_elf32_vec)            tb="$tb elf32-wasm32.lo elf32.lo $elf" ;;
    x86_64_coff_vec)		 tb="$tb coff-x86_64.lo $coff"; target_size=64 ;;
    x86_64_elf32_vec)		 tb="$tb elf64-x86-64.lo $elfxx_x86 elf64.lo elf32.lo $elf"; target_size=64 ;;
    x86_64_elf64_vec)		 tb="$tb elf64-x86-64.lo $elfxx_x86 elf64.lo $elf"; target_size=64 ;;
    x86_64_elf64_cloudabi_vec)	 tb="$tb elf64-x86-64.lo $elfxx_x86 elf64.lo $elf"; target_size=64 ;;
    x86_64_elf64_fbsd_vec)	 tb="$tb elf64-x86-64.lo $elfxx_x86 elf64.lo $elf"; target_size=64 ;;
    x86_64_elf64_sol2_vec)	 tb="$tb elf64-x86-64.lo $elfxx_x86 elf64.lo $elf"; target_size=64 ;;
    x86_64_mach_o_vec)		 tb="$tb mach-o-x86-64.lo" ;;
    x86_64_pe_vec)		 tb="$tb pe-x86_64.lo pex64igen.lo $coff"; target_size=64 ;;
    x86_64_pe_big_vec)		 tb="$tb pe-x86_64.lo pex64igen.lo $coff"; target_size=64 ;;
    x86_64_pei_vec)		 tb="$tb pei-x86_64.lo pex64igen.lo $coff"; target_size=64 ;;
    xgate_elf32_vec)		 tb="$tb elf32-xgate.lo elf32.lo $elf" ;;
    xstormy16_elf32_vec)	 tb="$tb elf32-xstormy16.lo elf32.lo $elf" ;;
    xtensa_elf32_be_vec)	 tb="$tb xtensa-dynconfig.lo xtensa-isa.lo xtensa-modules.lo elf32-xtensa.lo elf32.lo $elf" ;;
    xtensa_elf32_le_vec)	 tb="$tb xtensa-dynconfig.lo xtensa-isa.lo xtensa-modules.lo elf32-xtensa.lo elf32.lo $elf" ;;
    z80_coff_vec)		 tb="$tb coff-z80.lo reloc16.lo $coffgen" ;;
    z80_elf32_vec)		 tb="$tb elf32-z80.lo elf32.lo $elf" ;;
    z8k_coff_vec)		 tb="$tb coff-z8k.lo reloc16.lo $coff" ;;

    # These appear out of order in targets.c
    srec_vec)			 tb="$tb srec.lo" ;;
    symbolsrec_vec)		 tb="$tb srec.lo" ;;
    tekhex_vec)			 tb="$tb tekhex.lo" ;;
    core_cisco_be_vec)		 tb="$tb cisco-core.lo" ;;
    core_cisco_le_vec)		 tb="$tb cisco-core.lo" ;;

    "")				 ;;
    *) AC_MSG_ERROR(*** unknown target vector $vec) ;;
    esac

    if test ${target_size} = 64; then
	target64=true
    fi
    if test x"${vec}" = x"${defvec}"; then
	bfd_default_target_size=${target_size}
    fi
done

if test "$plugins" = "yes"; then
     tb="$tb plugin.lo"
fi

# Target architecture .o files.
# A couple of CPUs use shorter file names to avoid problems on DOS
# filesystems.
ta=`echo $selarchs | sed -e s/bfd_/cpu-/g -e s/_arch/.lo/g -e s/mn10200/m10200/ -e s/mn10300/m10300/`

# Weed out duplicate .o files.
f=""
for i in $tb ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
tb="$f"

f=""
for i in $ta ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
ta="$f"

bfd_backends="$tb"
bfd_machines="$ta"

if test x${all_targets} = xtrue ; then
  bfd_backends="${bfd_backends}"' $(ALL_BACKENDS)'
  bfd_machines="${bfd_machines}"' $(ALL_MACHINES)'
  selvecs=
  havevecs=-DHAVE_all_vecs
  selarchs=
  test -n "$assocvecs" &&
    assocvecs=`echo $assocvecs | sed -e 's/^/\&/' -e 's/ \(.\)/,\&\1/g'`
else	# all_targets is true
  # Only set these if they will be nonempty, for the clever echo.
  havevecs=
  assocvecs=
  test -n "$selvecs" &&
    havevecs=`echo $selvecs | sed -e 's/^/-DHAVE_/' -e 's/ \(.\)/ -DHAVE_\1/g'`
  test -n "$selvecs" &&
    selvecs=`echo $selvecs | sed -e 's/^/\&/' -e 's/ \(.\)/,\&\1/g'`
  test -n "$selarchs" &&
    selarchs=`echo $selarchs | sed -e 's/^/\&/' -e 's/ \(.\)/,\&\1/g'`
fi	# all_targets is true

# 64-bit archives need a 64-bit bfd_vma.
if test "x$want_64_bit_archive" = xtrue; then
  want64=true
  AC_DEFINE(USE_64_BIT_ARCHIVE, 1,
    [Define if 64-bit archives should always be used.])
fi

case ${host64}-${target64}-${want64} in
  *true*)
    wordsize=64
    bfd64_libs='$(BFD64_LIBS)'
    all_backends='$(BFD64_BACKENDS) $(BFD32_BACKENDS)'
    if test -n "$GCC" ; then
       bad_64bit_gcc=no;
       AC_MSG_CHECKING([for gcc version with buggy 64-bit support])
       # Add more tests for gcc versions with non-working 64-bit support here.
       AC_EGREP_CPP([: 2 : 91 : 1 :],[:__GNUC__:__GNUC_MINOR__:__i386__:],
		    bad_64bit_gcc=yes;
		    AC_MSG_RESULT([yes: egcs-1.1.2 on ix86 spotted]),
		    AC_MSG_RESULT(no))
       if test $bad_64bit_gcc = yes ; then
	 AC_MSG_ERROR([A newer version of gcc is needed for the requested 64-bit BFD configuration])
       fi
    fi
    ;;
  false-false-false)
    wordsize=32
    all_backends='$(BFD32_BACKENDS)'
    ;;
esac

tdefaults=""
test -n "${defvec}" && tdefaults="${tdefaults} -DDEFAULT_VECTOR=${defvec}"
test -n "${selvecs}" && tdefaults="${tdefaults} -DSELECT_VECS='${selvecs}'"
test -n "${assocvecs}" && tdefaults="${tdefaults} -DASSOCIATED_VECS='${assocvecs}'"
test -n "${selarchs}" && tdefaults="${tdefaults} -DSELECT_ARCHITECTURES='${selarchs}'"

AC_SUBST(wordsize)
AC_SUBST(bfd64_libs)
AC_SUBST(all_backends)
AC_SUBST(bfd_backends)
AC_SUBST(bfd_machines)
AC_SUBST(bfd_default_target_size)
AC_SUBST(tdefaults)
AC_SUBST(havevecs)

# If we are configured native, pick a core file support file.
COREFILE=
COREFLAG=
CORE_HEADER=
TRAD_HEADER=
if test "${target}" = "${host}"; then
  case "${host}" in
  alpha*-*-freebsd* | alpha*-*-kfreebsd*-gnu | alpha*-*-*vms*)
	COREFILE=''
	;;
  alpha*-*-linux-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/alphalinux.h"'
	;;
  alpha*-*-netbsd* | alpha*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  alpha*-*-*)
        COREFILE=osf-core.lo
	;;
  arm-*-freebsd* | arm-*-kfreebsd*-gnu)
	COREFILE='' ;;
  arm*-*-netbsd* | arm-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  hppa*-*-hpux*)	COREFILE=hpux-core.lo ;;
  hppa*-*-hiux*)	COREFILE=hpux-core.lo ;;
  hppa*-*-mpeix*)	COREFILE=hpux-core.lo ;;
  hppa*-*-bsd*)		COREFILE="hpux-core.lo hppabsd-core.lo"
			COREFLAG="-DHPUX_CORE -DHPPABSD_CORE" ;;
  hppa*-*-netbsd* | hppa*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;

changequote(,)dnl
  i[3-7]86-*-bsdi)
changequote([,])dnl
	COREFILE=
	;;
changequote(,)dnl
  i[3-7]86-*-bsd* | i[3-7]86-*-freebsd[123] | i[3-7]86-*-freebsd[123]\.* | i[3-7]86-*-freebsd4\.[01234] | i[3-7]86-*-freebsd4\.[01234]\.*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i386bsd.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-freebsd* | i[3-7]86-*-kfreebsd*-gnu | i[3-7]86-*-dragonfly*)
changequote([,])dnl
	COREFILE=''
	TRAD_HEADER='"hosts/i386bsd.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-netbsd* | i[3-7]86-*-knetbsd*-gnu | i[3-7]86-*-openbsd*)
changequote([,])dnl
	COREFILE=netbsd-core.lo
	;;
changequote(,)dnl
  i[3-7]86-esix-sysv3*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/esix.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-mach3*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i386mach3.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-linux-*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i386linux.h"'
	case "$enable_targets"-"$want64" in
	*x86_64-*linux*|*-true)
	  CORE_HEADER='"hosts/x86-64linux.h"'
	esac
	;;
  mips*-*-netbsd* | mips*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  mips-*-irix*)
	COREFILE=irix-core.lo
	;;
  m68*-*-linux-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/m68klinux.h"'
	;;
  m68*-*-netbsd*)
	COREFILE=netbsd-core.lo
	;;
  ns32k-pc532-mach)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/pc532mach.h"'
	;;
  ns32k-*-netbsd* | ns32k-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  rs6000-*-lynx*)
	COREFILE=lynx-core.lo
	;;
changequote(,)dnl
  rs6000-*-aix[5-9].* | powerpc-*-aix[5-9].* | powerpc64-*-aix[5-9].*)
changequote([,])dnl
        COREFILE=rs6000-core.lo
	COREFLAG="$COREFLAG -DAIX_5_CORE -DAIX_CORE_DUMPX_CORE"
	;;
changequote(,)dnl
  rs6000-*-aix4.[3-9]* | powerpc-*-aix4.[3-9]*)
changequote([,])dnl
        COREFILE=rs6000-core.lo
	COREFLAG="$COREFLAG -DAIX_CORE_DUMPX_CORE"
	# Not all versions of AIX with -DAIX_CORE_DUMPX_CORE
	# have c_impl as a member of struct core_dumpx
	AC_MSG_CHECKING([for c_impl in struct core_dumpx])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <core.h>]], [[struct core_dumpx c; c.c_impl = 0;]])],[AC_DEFINE(HAVE_ST_C_IMPL, 1,
	    [Define if struct core_dumpx has member c_impl])
	  AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])
	;;
  rs6000-*-aix4*)	COREFILE=rs6000-core.lo ;;
  rs6000-*-*)		COREFILE=rs6000-core.lo ;;
  powerpc64-*-aix*)	COREFILE=rs6000-core.lo ;;
  powerpc-*-aix4*)	COREFILE=rs6000-core.lo ;;
  powerpc-*-aix*)	COREFILE=rs6000-core.lo ;;
  powerpc-*-beos*)	;;
  powerpc-*-freebsd* | powerpc-*-kfreebsd*-gnu)
		        COREFILE='' ;;
  powerpc-*-netbsd*)	COREFILE=netbsd-core.lo ;;
  powerpc-*-*bsd*)	COREFILE=netbsd-core.lo ;;
  s390*-*-*)            COREFILE=trad-core.lo ;;
  sh*-*-netbsd* | sh*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  sparc-*-netbsd* | sparc*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  vax-*-netbsd* | vax-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  vax-*-linux-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/vaxlinux.h"'
	;;
  vax-*-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/vaxbsd.h"'
	;;
  x86_64-*-linux*)
	CORE_HEADER='"hosts/x86-64linux.h"'
	;;
  x86_64-*-netbsd* | x86_64-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  esac

  case "$COREFILE" in
  hppabsd-core.lo)	COREFLAG=-DHPPABSD_CORE ;;
  hpux-core.lo)		COREFLAG=-DHPUX_CORE ;;
  irix-core.lo)		COREFLAG=-DIRIX_CORE ;;
  lynx-core.lo)		COREFLAG=-DLYNX_CORE ;;
  netbsd-core.lo)	COREFLAG=-DNETBSD_CORE ;;
  osf-core.lo)		COREFLAG=-DOSF_CORE ;;
  ptrace-core.lo)	COREFLAG=-DPTRACE_CORE ;;
  rs6000-core.lo)	COREFLAG="$COREFLAG -DAIX_CORE" ;;
  trad-core.lo)		COREFLAG="$COREFLAG -DTRAD_CORE" ;;
  esac

  # ELF corefile support has several flavors, but all of
  # them use something called <sys/procfs.h>
  BFD_SYS_PROCFS_H
  if test "$ac_cv_header_sys_procfs_h" = yes; then
    BFD_HAVE_SYS_PROCFS_TYPE(prstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE(prstatus32_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(prstatus_t, pr_who)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(prstatus32_t, pr_who)
    BFD_HAVE_SYS_PROCFS_TYPE(pstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE(pxstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE(pstatus32_t)
    BFD_HAVE_SYS_PROCFS_TYPE(prpsinfo_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(prpsinfo_t, pr_pid)
    BFD_HAVE_SYS_PROCFS_TYPE(prpsinfo32_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(prpsinfo32_t, pr_pid)
    BFD_HAVE_SYS_PROCFS_TYPE(psinfo_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(psinfo_t, pr_pid)
    BFD_HAVE_SYS_PROCFS_TYPE(psinfo32_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(psinfo32_t, pr_pid)
    BFD_HAVE_SYS_PROCFS_TYPE(lwpstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE(lwpxstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(lwpstatus_t, pr_context)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(lwpstatus_t, pr_reg)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(lwpstatus_t, pr_fpreg)
    BFD_HAVE_SYS_PROCFS_TYPE(win32_pstatus_t)
  fi
fi
AC_SUBST(COREFILE)
AC_SUBST(COREFLAG)
if test -n "$CORE_HEADER"; then
  AC_DEFINE_UNQUOTED(CORE_HEADER, $CORE_HEADER,
    [Name of host specific core header file to include in elf.c.])
fi
if test -n "$TRAD_HEADER"; then
  AC_DEFINE_UNQUOTED(TRAD_HEADER, $TRAD_HEADER,
    [Name of host specific header file to include in trad-core.c.])
fi

if test "$plugins" = "yes"; then
  supports_plugins=1
else
  supports_plugins=0
fi
AC_SUBST(supports_plugins)
AC_SUBST(lt_cv_dlopen_libs)

# Determine the host dependent file_ptr a.k.a. off_t type.  In order
# prefer: off64_t - if ftello64 and fseeko64, off_t - if ftello and
# fseeko, long.  This assumes that sizeof off_t is .ge. sizeof long.
# Hopefully a reasonable assumption since fseeko et.al. should be
# upward compatible.
AC_CHECK_FUNCS(ftello ftello64 fseeko fseeko64 fopen64)
AC_CHECK_DECLS([ftello, ftello64, fseeko, fseeko64, fopen64])
if test x"$ac_cv_func_ftello" = xyes -a x"$ac_cv_func_fseeko" = xyes; then
    AC_CHECK_SIZEOF(off_t)
fi
AC_MSG_CHECKING([file_ptr type])
bfd_file_ptr="long"
bfd_ufile_ptr="unsigned long"
if test x"$ac_cv_func_ftello64" = xyes -a x"$ac_cv_func_fseeko64" = xyes \
	-o x"${ac_cv_sizeof_off_t}" = x8; then
    bfd_file_ptr=int64_t
    bfd_ufile_ptr=uint64_t
fi
AC_MSG_RESULT($bfd_file_ptr)
AC_SUBST(bfd_file_ptr)
AC_SUBST(bfd_ufile_ptr)

AC_FUNC_MMAP
AC_CHECK_FUNCS(madvise mprotect)
case ${want_mmap}+${ac_cv_func_mmap_fixed_mapped} in
  true+yes )  AC_DEFINE(USE_MMAP, 1, [Use mmap if it's available?]) ;;
esac

AC_CONFIG_FILES([Makefile bfd-in3.h:bfd-in2.h po/Makefile.in:po/Make-in])

dnl We need this duplication, even though we use AM_PO_SUBDIRS, because of
dnl our two separate POTFILES.  Yuck.
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
sed -e '/SRC-POTFILES =/r po/SRC-POTFILES' \
    -e '/BLD-POTFILES =/r po/BLD-POTFILES' \
    -e "s,@POFILES@,$POFILES," \
    -e "s,@GMOFILES@,$GMOFILES," \
    po/Makefile.in > po/Makefile]],[[]])

dnl Required by html, pdf, install-pdf and install-html
AC_SUBST(datarootdir)
AC_SUBST(docdir)
AC_SUBST(htmldir)
AC_SUBST(pdfdir)

AC_OUTPUT

GNU_MAKE_JOBSERVER
