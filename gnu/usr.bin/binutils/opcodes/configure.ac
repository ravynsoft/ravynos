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
AC_INIT([opcodes], BFD_VERSION)
AC_CONFIG_SRCDIR([z8k-dis.c])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS

dnl These must be called before LT_INIT, because it may want
dnl to call AC_CHECK_PROG.
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib, :)

dnl Default to a non shared library.  This may be overridden by the
dnl configure option --enable-shared.
AC_DISABLE_SHARED

LT_INIT

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
  yes | "") AC_MSG_ERROR([enable-targets option must specify target names or 'all'])
            ;;
  no)       enable_targets= ;;
  *)        enable_targets=$enableval ;;
esac])dnl

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
AM_INSTALL_LIBBFD
AC_EXEEXT

# host-specific stuff:

ALL_LINGUAS="da de es fi fr ga id it nl pt_BR ro sr sv tr uk vi zh_CN"
ZW_GNU_GETTEXT_SISTER_DIR
AM_PO_SUBDIRS

. ${srcdir}/../bfd/configure.host

BFD_CC_FOR_BUILD
BFD_64_BIT

AC_SUBST(HDEFINES)
AC_PROG_INSTALL

AC_CHECK_DECLS([basename, stpcpy])

# Check if sigsetjmp is available.  Using AC_CHECK_FUNCS won't do
# since sigsetjmp might only be defined as a macro.
AC_CACHE_CHECK([for sigsetjmp], gdb_cv_func_sigsetjmp,
[AC_TRY_COMPILE([
#include <setjmp.h>
], [sigjmp_buf env; while (! sigsetjmp (env, 1)) siglongjmp (env, 1);],
bfd_cv_func_sigsetjmp=yes, bfd_cv_func_sigsetjmp=no)])
if test $bfd_cv_func_sigsetjmp = yes; then
  AC_DEFINE(HAVE_SIGSETJMP, 1, [Define if sigsetjmp is available. ])
fi

cgen_maint=no
cgendir='$(srcdir)/../cgen'

AC_ARG_ENABLE(cgen-maint,
[  --enable-cgen-maint[=dir]    build cgen generated files],
[case "${enableval}" in
  yes)	cgen_maint=yes ;;
  no)	cgen_maint=no ;;
  *)
	# Argument is a directory where cgen can be found.  In some
	# future world cgen could be installable, but right now this
	# is not the case.  Instead we assume the directory is a path
	# to the cgen source tree.
	cgen_maint=yes
        if test -r ${enableval}/iformat.scm; then
          # This looks like a cgen source tree.
	  cgendir=${enableval}
        else
	  AC_MSG_ERROR(${enableval} doesn't look like a cgen source tree)
        fi
	;;
esac])dnl
AM_CONDITIONAL(CGEN_MAINT, test x${cgen_maint} = xyes)
AC_SUBST(cgendir)

using_cgen=no

# Check if linker supports --as-needed and --no-as-needed options
AC_CACHE_CHECK(linker --as-needed support, bfd_cv_ld_as_needed,
	[bfd_cv_ld_as_needed=no
	if $LD --help 2>/dev/null | grep as-needed > /dev/null; then
		bfd_cv_ld_as_needed=yes
	fi
	])

LT_LIB_M

#Libs for generator progs
if test "x$cross_compiling" = "xno"; then
  BUILD_LIBS=../libiberty/libiberty.a
  BUILD_LIB_DEPS=$BUILD_LIBS
else
  # if cross-compiling, assume that the system provides -liberty
  # and that the version is compatible with new headers.
  BUILD_LIBS=-liberty
  BUILD_LIB_DEPS=
fi
BUILD_LIBS="$BUILD_LIBS $LIBINTL"
BUILD_LIB_DEPS="$BUILD_LIB_DEPS $LIBINTL_DEP"

AC_SUBST(BUILD_LIBS)
AC_SUBST(BUILD_LIB_DEPS)

# Horrible hacks to build DLLs on Windows and a shared library elsewhere.
SHARED_LDFLAGS=
SHARED_LIBADD=
SHARED_DEPENDENCIES=
if test "$enable_shared" = "yes"; then
# When building a shared libopcodes, link against the pic version of libiberty
# so that apps that use libopcodes won't need libiberty just to satisfy any
# libopcodes references.
# We can't do that if a pic libiberty is unavailable since including non-pic
# code would insert text relocations into libopcodes.
# Note that linking against libbfd as we do here, which is itself linked
# against libiberty, may not satisfy all the libopcodes libiberty references
# since libbfd may not pull in the entirety of libiberty.
# Also, jam libintl into the right place in all of this: after libiberty,
# which uses it, but before -lcygwin, which it uses.
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
    *-*-cygwin*)
      SHARED_LDFLAGS="-no-undefined"
      SHARED_LIBADD="-L`pwd`/../bfd -lbfd -L`pwd`/../libiberty -liberty $SHARED_LIBADD"
      ;;
    *)
      SHARED_LIBADD="../bfd/libbfd.la ${SHARED_LIBADD}"
      SHARED_DEPENDENCIES="../bfd/libbfd.la"
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
AC_SUBST(SHARED_DEPENDENCIES)

# target-specific stuff:

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
selarchs=
for targ in $target $canon_targets
do
    if test "x$targ" = "xall" ; then
        all_targets=true
    else
	. $srcdir/../bfd/config.bfd
	selarchs="$selarchs $targ_archs"
    fi
done

# Utility var, documents generic cgen support files.

cgen_files="cgen-opc.lo cgen-asm.lo cgen-dis.lo cgen-bitset.lo"

# We don't do any links based on the target system, just makefile config.

if test x${all_targets} = xfalse ; then

    # Target architecture .o files.
    ta=

    for arch in $selarchs
    do
	ad=`echo $arch | sed -e s/bfd_//g -e s/_arch//g`
	archdefs="$archdefs -DARCH_$ad"
	case "$arch" in
	bfd_aarch64_arch)	ta="$ta aarch64-asm.lo aarch64-dis.lo aarch64-opc.lo aarch64-asm-2.lo aarch64-dis-2.lo aarch64-opc-2.lo" ;;
	bfd_alpha_arch)		ta="$ta alpha-dis.lo alpha-opc.lo" ;;
	bfd_amdgcn_arch)	;;
	bfd_arc_arch)		ta="$ta arc-dis.lo arc-opc.lo arc-ext.lo" ;;
	bfd_arm_arch)		ta="$ta arm-dis.lo" ;;
	bfd_avr_arch)		ta="$ta avr-dis.lo" ;;
	bfd_bfin_arch)		ta="$ta bfin-dis.lo" ;;
	bfd_cr16_arch)		ta="$ta cr16-dis.lo cr16-opc.lo" ;;
	bfd_cris_arch)		ta="$ta cris-desc.lo cris-dis.lo cris-opc.lo cgen-bitset.lo" ;;
	bfd_crx_arch)		ta="$ta crx-dis.lo crx-opc.lo" ;;
	bfd_csky_arch)		ta="$ta csky-dis.lo" ;;
	bfd_d10v_arch)		ta="$ta d10v-dis.lo d10v-opc.lo" ;;
	bfd_d30v_arch)		ta="$ta d30v-dis.lo d30v-opc.lo" ;;
	bfd_dlx_arch)		ta="$ta dlx-dis.lo" ;;
	bfd_fr30_arch)		ta="$ta fr30-asm.lo fr30-desc.lo fr30-dis.lo fr30-ibld.lo fr30-opc.lo" using_cgen=yes ;;
	bfd_frv_arch)		ta="$ta frv-asm.lo frv-desc.lo frv-dis.lo frv-ibld.lo frv-opc.lo" using_cgen=yes ;;
	bfd_ft32_arch)		ta="$ta ft32-opc.lo ft32-dis.lo" ;;
	bfd_moxie_arch)		ta="$ta moxie-dis.lo moxie-opc.lo" ;;
	bfd_h8300_arch)		ta="$ta h8300-dis.lo" ;;
	bfd_hppa_arch)		ta="$ta hppa-dis.lo" ;;
	bfd_i386_arch|bfd_iamcu_arch)
				ta="$ta i386-dis.lo" ;;
	bfd_ia64_arch)		ta="$ta ia64-dis.lo ia64-opc.lo" ;;
	bfd_ip2k_arch)		ta="$ta ip2k-asm.lo ip2k-desc.lo ip2k-dis.lo ip2k-ibld.lo ip2k-opc.lo" using_cgen=yes ;;
	bfd_epiphany_arch)	ta="$ta epiphany-asm.lo epiphany-desc.lo epiphany-dis.lo epiphany-ibld.lo epiphany-opc.lo" using_cgen=yes ;;
	bfd_iq2000_arch)	ta="$ta iq2000-asm.lo iq2000-desc.lo iq2000-dis.lo iq2000-ibld.lo iq2000-opc.lo" using_cgen=yes ;;
	bfd_lm32_arch)		ta="$ta lm32-asm.lo lm32-desc.lo lm32-dis.lo lm32-ibld.lo lm32-opc.lo lm32-opinst.lo" using_cgen=yes ;;
	bfd_m32c_arch)		ta="$ta m32c-asm.lo m32c-desc.lo m32c-dis.lo m32c-ibld.lo m32c-opc.lo" using_cgen=yes ;;
	bfd_m32r_arch)		ta="$ta m32r-asm.lo m32r-desc.lo m32r-dis.lo m32r-ibld.lo m32r-opc.lo m32r-opinst.lo" using_cgen=yes ;;
	bfd_m68hc11_arch)	ta="$ta m68hc11-dis.lo m68hc11-opc.lo" ;;
	bfd_m68hc12_arch)	ta="$ta m68hc11-dis.lo m68hc11-opc.lo" ;;
	bfd_m9s12x_arch)	ta="$ta m68hc11-dis.lo m68hc11-opc.lo" ;;
	bfd_m9s12xg_arch)	ta="$ta m68hc11-dis.lo m68hc11-opc.lo" ;;
	bfd_s12z_arch)		ta="$ta s12z-dis.lo s12z-opc.lo" ;;
	bfd_m68k_arch)		ta="$ta m68k-dis.lo m68k-opc.lo" ;;
	bfd_mcore_arch)		ta="$ta mcore-dis.lo" ;;
	bfd_mep_arch)		ta="$ta mep-asm.lo mep-desc.lo mep-dis.lo mep-ibld.lo mep-opc.lo" using_cgen=yes ;;
	bfd_metag_arch)		ta="$ta metag-dis.lo" ;;
	bfd_microblaze_arch)	ta="$ta microblaze-dis.lo" ;;
	bfd_mips_arch)		ta="$ta mips-dis.lo mips-opc.lo mips16-opc.lo micromips-opc.lo" ;;
	bfd_mmix_arch)		ta="$ta mmix-dis.lo mmix-opc.lo" ;;
	bfd_mn10200_arch)	ta="$ta m10200-dis.lo m10200-opc.lo" ;;
	bfd_mn10300_arch)	ta="$ta m10300-dis.lo m10300-opc.lo" ;;
	bfd_mt_arch)		ta="$ta mt-asm.lo mt-desc.lo mt-dis.lo mt-ibld.lo mt-opc.lo" using_cgen=yes ;;
	bfd_msp430_arch)	ta="$ta msp430-dis.lo msp430-decode.lo" ;;
	bfd_nds32_arch)		ta="$ta nds32-asm.lo nds32-dis.lo" ;;
	bfd_nfp_arch)		ta="$ta nfp-dis.lo" ;;
	bfd_nios2_arch)		ta="$ta nios2-dis.lo nios2-opc.lo" ;;
	bfd_ns32k_arch)		ta="$ta ns32k-dis.lo" ;;
	bfd_or1k_arch)		ta="$ta or1k-asm.lo or1k-desc.lo or1k-dis.lo or1k-ibld.lo or1k-opc.lo" using_cgen=yes ;;
	bfd_pdp11_arch)		ta="$ta pdp11-dis.lo pdp11-opc.lo" ;;
	bfd_pj_arch)		ta="$ta pj-dis.lo pj-opc.lo" ;;
	bfd_powerpc_arch)	ta="$ta ppc-dis.lo ppc-opc.lo" ;;
	bfd_powerpc_64_arch)	ta="$ta ppc-dis.lo ppc-opc.lo" ;;
	bfd_pru_arch)		ta="$ta pru-dis.lo pru-opc.lo" ;;
	bfd_pyramid_arch)	;;
	bfd_romp_arch)		;;
	bfd_riscv_arch)         ta="$ta riscv-dis.lo riscv-opc.lo" ;;
	bfd_rs6000_arch)	ta="$ta ppc-dis.lo ppc-opc.lo" ;;
	bfd_rl78_arch)		ta="$ta rl78-dis.lo rl78-decode.lo";;
	bfd_rx_arch)		ta="$ta rx-dis.lo rx-decode.lo";;
	bfd_s390_arch)		ta="$ta s390-dis.lo s390-opc.lo" ;;
	bfd_score_arch)		ta="$ta score-dis.lo score7-dis.lo" ;;
	bfd_sh_arch)		ta="$ta sh-dis.lo cgen-bitset.lo" ;;
	bfd_sparc_arch)		ta="$ta sparc-dis.lo sparc-opc.lo" ;;
	bfd_spu_arch)		ta="$ta spu-dis.lo spu-opc.lo" ;;
	bfd_tic30_arch)		ta="$ta tic30-dis.lo" ;;
	bfd_tic4x_arch)		ta="$ta tic4x-dis.lo" ;;
	bfd_tic54x_arch)	ta="$ta tic54x-dis.lo tic54x-opc.lo" ;;
	bfd_tic6x_arch)		ta="$ta tic6x-dis.lo" ;;
	bfd_tilegx_arch)	ta="$ta tilegx-dis.lo tilegx-opc.lo" ;;
	bfd_tilepro_arch)	ta="$ta tilepro-dis.lo tilepro-opc.lo" ;;
	bfd_v850_arch)		ta="$ta v850-opc.lo v850-dis.lo" ;;
	bfd_v850e_arch)		ta="$ta v850-opc.lo v850-dis.lo" ;;
	bfd_v850ea_arch)	ta="$ta v850-opc.lo v850-dis.lo" ;;
	bfd_v850_rh850_arch)	ta="$ta v850-opc.lo v850-dis.lo" ;;
	bfd_vax_arch)		ta="$ta vax-dis.lo" ;;
	bfd_visium_arch)	ta="$ta visium-dis.lo visium-opc.lo" ;;
        bfd_wasm32_arch)        ta="$ta wasm32-dis.lo" ;;
	bfd_xgate_arch)		ta="$ta xgate-dis.lo xgate-opc.lo" ;;
	bfd_xstormy16_arch)	ta="$ta xstormy16-asm.lo xstormy16-desc.lo xstormy16-dis.lo xstormy16-ibld.lo xstormy16-opc.lo" using_cgen=yes ;;
	bfd_xtensa_arch)	ta="$ta xtensa-dis.lo" ;;
	bfd_z80_arch)		ta="$ta z80-dis.lo" ;;
	bfd_z8k_arch)		ta="$ta z8k-dis.lo" ;;
	bfd_bpf_arch)		ta="$ta bpf-asm.lo bpf-desc.lo bpf-dis.lo bpf-ibld.lo bpf-opc.lo" using_cgen=yes ;;
	bfd_loongarch_arch)	ta="$ta loongarch-dis.lo loongarch-opc.lo loongarch-coder.lo" ;;

	"")			;;
	*)		AC_MSG_ERROR(*** unknown target architecture $arch) ;;
	esac
    done

    if test $using_cgen = yes ; then
	ta="$ta $cgen_files"
    fi

    # Weed out duplicate .o files.
    f=""
    for i in $ta ; do
	case " $f " in
	*" $i "*) ;;
	*) f="$f $i" ;;
	esac
    done
    ta="$f"

    # And duplicate -D flags.
    f=""
    for i in $archdefs ; do
	case " $f " in
	*" $i "*) ;;
	*) f="$f $i" ;;
	esac
    done
    archdefs="$f"

    BFD_MACHINES="$ta"

else	# all_targets is true
    archdefs=-DARCH_all
    if test "$enable_64_bit_bfd" = "yes" ; then
	BFD_MACHINES='$(ALL32_MACHINES) $(ALL64_MACHINES)'
    else
	BFD_MACHINES='$(ALL32_MACHINES)'
    fi
fi

AC_SUBST(archdefs)
AC_SUBST(BFD_MACHINES)

AC_CONFIG_FILES([Makefile po/Makefile.in:po/Make-in])
AC_OUTPUT

GNU_MAKE_JOBSERVER
