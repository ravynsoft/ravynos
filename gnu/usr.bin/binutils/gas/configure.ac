dnl Process this file with autoconf to produce a configure script.
dnl
dnl And be careful when changing it!  If you must add tests with square
dnl brackets, be sure changequote invocations surround it.
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
dnl v2.5 needed for --bindir et al
m4_include([../bfd/version.m4])
AC_INIT([gas], BFD_VERSION)
AC_CONFIG_SRCDIR(as.h)

dnl Autoconf 2.57 will find the aux dir without this.  However, unless
dnl we specify this explicitly, automake-1.7 will assume that ylwrap is in
dnl gas/ instead of gas/../.
AC_CONFIG_AUX_DIR(..)
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS

LT_INIT
ACX_LARGEFILE
ACX_PROG_CMP_IGNORE_INITIAL

AC_ARG_ENABLE(targets,
[  --enable-targets        alternative target configurations besides the primary],
[case "${enableval}" in
  yes | "") AC_MSG_ERROR(enable-targets option must specify target names or 'all')
	    ;;
  no)	    enable_targets= ;;
  *)	    enable_targets=$enableval ;;
esac])dnl

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

# PR gas/19109
# Decide the default method for compressing debug sections.
ac_default_compressed_debug_sections=unset
# Provide a configure time option to override our default.
AC_ARG_ENABLE(compressed_debug_sections,
	      AS_HELP_STRING([--enable-compressed-debug-sections={all,gas,none}],
	      [compress debug sections by default]),
[case ,"${enableval}", in
  ,yes, | ,all, | *,gas,*) ac_default_compressed_debug_sections=yes ;;
  ,no, | ,none,)  ac_default_compressed_debug_sections=no ;;
  *)   ac_default_compressed_debug_sections=unset ;;
esac])dnl

# Select default compression algorithm.
ac_default_compressed_debug_sections_algorithm=COMPRESS_DEBUG_GABI_ZLIB
AC_ARG_ENABLE(default_compressed_debug_sections_algorithm,
	      AS_HELP_STRING([--enable-default-compressed-debug-sections-algorithm={zlib,zstd}],
	      [Default compression algorithm for --enable-compressed-debug-sections.]),
[case "${enableval}" in
   zstd) ac_default_compressed_debug_sections_algorithm=COMPRESS_DEBUG_ZSTD ;;
esac])dnl

# PR gas/19520
# Decide if x86 assembler should generate relax relocations.
ac_default_x86_relax_relocations=unset
# Provide a configure time option to override our default.
AC_ARG_ENABLE(x86_relax_relocations,
	      AS_HELP_STRING([--enable-x86-relax-relocations],
	      [generate x86 relax relocations by default]),
[case "${enableval}" in
  no)  ac_default_x86_relax_relocations=0 ;;
esac])dnl

# Decide if ELF assembler should generate common symbols with the
# STT_COMMON type.
ac_default_elf_stt_common=unset
# Provide a configure time option to override our default.
AC_ARG_ENABLE(elf_stt_common,
	      AS_HELP_STRING([--enable-elf-stt-common],
	      [generate ELF common symbols with STT_COMMON type by default]),
[case "${enableval}" in
  yes)  ac_default_elf_stt_common=1 ;;
esac])dnl


# Decide if the ELF assembler should default to generating
# GNU Build notes if none are provided by the input.
ac_default_generate_build_notes=0
# Provide a configuration option to override the default.
AC_ARG_ENABLE(generate_build_notes,
	      AS_HELP_STRING([--enable-generate-build-notes],
	      [generate GNU Build notes if none are provided by the input]),
[case "${enableval}" in
  yes)  ac_default_generate_build_notes=1 ;;
  no)   ac_default_generate_build_notes=0 ;;
esac])dnl

# Decide if the MIPS assembler should default to enable MIPS fix Loongson3
# LLSC errata.
ac_default_mips_fix_loongson3_llsc=unset
# Provide a configuration option to override the default.
AC_ARG_ENABLE(mips-fix-loongson3-llsc,
             AS_HELP_STRING([--enable-mips-fix-loongson3-llsc],
             [enable MIPS fix Loongson3 LLSC errata]),
[case "${enableval}" in
  yes)  ac_default_mips_fix_loongson3_llsc=1 ;;
  no)   ac_default_mips_fix_loongson3_llsc=0 ;;
esac])dnl

# Decide if the x86 ELF assembler should default to generating GNU x86
# used ISA and feature properties.
ac_default_generate_x86_used_note=unset
# Provide a configuration option to override the default.
AC_ARG_ENABLE(x86-used-note,
	      AS_HELP_STRING([--enable-x86-used-note],
	      [generate GNU x86 used ISA and feature properties]),
[case "${enableval}" in
  yes)  ac_default_generate_x86_used_note=1 ;;
  no)   ac_default_generate_x86_used_note=0 ;;
esac])dnl

# Decide if the RISC-V ELF assembler should default to generating attribute.
ac_default_generate_riscv_attr=unset
# Provide a configuration option to override the default.
AC_ARG_ENABLE(default-riscv-attribute,
	      AS_HELP_STRING([--enable-default-riscv-attribute],
	      [generate RISC-V arch attribute by default]),
[case "${enableval}" in
  yes)  ac_default_generate_riscv_attr=1 ;;
  no)   ac_default_generate_riscv_attr=0 ;;
esac])dnl

using_cgen=no

AM_BINUTILS_WARNINGS

# Generate a header file
AC_CONFIG_HEADERS(config.h:config.in)

dnl Option --with-cpu=TYPE allows configure type control of the default
dnl cpu type within the assembler.  Currently only the ARC target
dnl supports this feature, but others may be added in the future.
AC_ARG_WITH(cpu,
            AS_HELP_STRING([--with-cpu=CPU],
            [default cpu variant is CPU (currently only supported on ARC)]),
            [AC_DEFINE_UNQUOTED(TARGET_WITH_CPU,
                                "${with_cpu}",
                                [Target specific CPU.])],
            [])

# PR 14072
AH_VERBATIM([00_CONFIG_H_CHECK],
[/* Check that config.h is #included before system headers
   (this works only for glibc, but that should be enough).  */
#if defined(__GLIBC__) && !defined(__FreeBSD_kernel__) && !defined(__CONFIG_H__)
#  error config.h must be #included before system headers
#endif
#define __CONFIG_H__ 1])

# If we are on a DOS filesystem, we must use gdb.ini rather than
# .gdbinit.
case "${host}" in
  *-*-msdos* | *-*-go32* | *-*-mingw32* | *-*-windows*)
    GDBINIT="gdb.ini"
    AC_CONFIG_FILES(gdb.ini:gdbinit.in)
    ;;
  *)
    GDBINIT=".gdbinit"
    AC_CONFIG_FILES(.gdbinit:gdbinit.in)
    ;;
esac
AC_SUBST(GDBINIT)

#We need this for the host.
AC_C_BIGENDIAN

te_file=generic

# Makefile target for installing gas in $(tooldir)/bin.
install_tooldir=install-exec-tooldir

canon_targets=""
all_targets=no
if test -n "$enable_targets" ; then
  for t in `echo $enable_targets | sed 's/,/ /g'`; do
    if test $t = "all"; then
      all_targets=yes
      continue
    fi
    result=`$ac_config_sub $t 2>/dev/null`
    if test -n "$result" ; then
      canon_targets="$canon_targets $result"
#    else
#      # Permit "all", etc.  We don't support it yet though.
#      canon_targets="$canon_targets $t"
    fi
  done
  GAS_UNIQ(canon_targets)
fi

emulations=""

for this_target in $target $canon_targets ; do

    targ=${this_target}
    . ${srcdir}/configure.tgt

    case ${target_cpu} in
      crisv32)
	AC_DEFINE_UNQUOTED(DEFAULT_CRIS_ARCH, $arch,
			   [Default CRIS architecture.])
	;;
    esac

    if test ${this_target} = $target ; then
      target_cpu_type=${cpu_type}
    elif test ${target_cpu_type} != ${cpu_type} ; then
      continue
    fi

    generic_target=${cpu_type}-${target_vendor}-${target_os}
    case ${generic_target} in
      i386-*-msdosdjgpp* \
      | i386-*-go32*)
	AC_DEFINE(STRICTCOFF, 1, [Using strict COFF?])
	;;

      i386-*-linux-* | x86_64-*-linux-*)
	if test ${this_target} = $target \
	   && test ${ac_default_generate_x86_used_note} = unset; then
	  ac_default_generate_x86_used_note=1
	fi
	;;

      i386-*-solaris2 \
      | x86_64-*-solaris2 \
      | i386-*-solaris2.[[0-9]] \
      | i386-*-solaris2.1[[01]] \
      | x86_64-*-solaris2.1[[01]])
	if test ${this_target} = $target \
	   && test ${ac_default_x86_relax_relocations} = unset; then
	  ac_default_x86_relax_relocations=0
	fi
	;;

      microblaze*)
        ;;

changequote(,)dnl
      ppc-*-aix[5-9].*)
changequote([,])dnl
	AC_DEFINE(AIX_WEAK_SUPPORT, 1,
		  [Define if using AIX 5.2 value for C_WEAKEXT.])
	;;
      ppc-*-solaris*)
	if test ${this_target} = $target; then
	  AC_DEFINE(TARGET_SOLARIS_COMMENT, 1,
		    [Define if default target is PowerPC Solaris.])
	fi
	if test x${endian} = xbig; then
	  AC_MSG_ERROR(Solaris must be configured little endian)
	fi
	;;
    esac

    if test ${this_target} = $target ; then
      endian_def=
      if test x${endian} = xbig; then
	endian_def=1
      elif test x${endian} = xlittle; then
	endian_def=0
      fi
      if test x${endian_def} != x; then
	AC_DEFINE_UNQUOTED(TARGET_BYTES_BIG_ENDIAN, $endian_def,
			   [Define as 1 if big endian.])
      fi
    fi

# Other random stuff.

    case ${cpu_type} in
      mips)
	# Set mips_cpu to the name of the default CPU.
	case ${target_cpu} in
	  mips | mipsbe | mipseb | mipsle | mipsel | mips64 | mips64el)
	    mips_cpu=from-abi
	    ;;
	  mipsisa32 | mipsisa32el)
	    mips_cpu=mips32
	    ;;
	  mipsisa32r2 | mipsisa32r2el)
	    mips_cpu=mips32r2
	    ;;
	  mipsisa32r3 | mipsisa32r3el)
	    mips_cpu=mips32r3
	    ;;
	  mipsisa32r5 | mipsisa32r5el)
	    mips_cpu=mips32r5
	    ;;
	  mipsisa32r6 | mipsisa32r6el)
	    mips_cpu=mips32r6
	    ;;
	  mipsisa64 | mipsisa64el)
	    mips_cpu=mips64
	    ;;
	  mipsisa64r2 | mipsisa64r2el)
	    mips_cpu=mips64r2
	    ;;
	  mipsisa64r3 | mipsisa64r3el)
	    mips_cpu=mips64r3
	    ;;
	  mipsisa64r5 | mipsisa64r5el)
	    mips_cpu=mips64r5
	    ;;
	  mipsisa64r6 | mipsisa64r6el)
	    mips_cpu=mips64r6
	    ;;
	  mipstx39 | mipstx39el)
	    mips_cpu=r3900
	    ;;
	  mips64vr | mips64vrel)
	    mips_cpu=vr4100
	    ;;
	  mipsisa32r2* | mipsisa64r2*)
changequote(,)dnl
	    mips_cpu=`echo $target_cpu | sed -e 's/[a-z]*..r2//' -e 's/el$//'`
changequote([,])dnl
	    ;;
	  mipsisa32r6* | mipsisa64r6*)
changequote(,)dnl
	    mips_cpu=`echo $target_cpu | sed -e 's/[a-z]*..r6//' -e 's/el$//'`
changequote([,])dnl
	    ;;
	  mips64* | mipsisa64* | mipsisa32*)
changequote(,)dnl
	    mips_cpu=`echo $target_cpu | sed -e 's/[a-z]*..//' -e 's/el$//'`
changequote([,])dnl
	    ;;
	  mips*)
changequote(,)dnl
	    mips_cpu=`echo $target_cpu | sed -e 's/^mips//' -e 's/el$//'`
changequote([,])dnl
	    ;;
	  *)
	    AC_MSG_ERROR($target_cpu isn't a supported MIPS CPU name)
	    ;;
	esac
	# See whether it's appropriate to set E_MIPS_ABI_O32 for o32
	# binaries.  It's a GNU extension that some OSes don't understand.
	case ${target} in
	  *-*-irix*)
	    use_e_mips_abi_o32=0
	    ;;
	  *)
	    use_e_mips_abi_o32=1
	    ;;
	esac
	# Decide whether to generate 32-bit or 64-bit code by default.
	# Used to resolve -march=from-abi when an embedded ABI is selected.
	case ${target} in
	  mips64*-*-* | mipsisa64*-*-*)
	    mips_default_64bit=1
	    ;;
	  *)
	    mips_default_64bit=0
	    ;;
	esac
	# Decide which ABI to target by default.
	case ${target} in
	  mips64*-openbsd* | mips64*-linux-gnuabi64)
	    mips_default_abi=N64_ABI
	    ;;
	  mips64*-linux* | mips-sgi-irix6* | mips64*-freebsd* \
	  | mips64*-kfreebsd*-gnu | mips64*-ps2-elf*)
	    mips_default_abi=N32_ABI
	    ;;
	  mips*-linux* | mips*-freebsd* | mips*-kfreebsd*-gnu)
	    mips_default_abi=O32_ABI
	    ;;
	  *)
	    mips_default_abi=NO_ABI
	    ;;
	esac
	AC_DEFINE_UNQUOTED(MIPS_CPU_STRING_DEFAULT, "$mips_cpu",
			   [Default CPU for MIPS targets. ])
	AC_DEFINE_UNQUOTED(USE_E_MIPS_ABI_O32, $use_e_mips_abi_o32,
			   [Allow use of E_MIPS_ABI_O32 on MIPS targets. ])
	AC_DEFINE_UNQUOTED(MIPS_DEFAULT_64BIT, $mips_default_64bit,
			   [Generate 64-bit code by default on MIPS targets. ])
	AC_DEFINE_UNQUOTED(MIPS_DEFAULT_ABI, $mips_default_abi,
			   [Choose a default ABI for MIPS targets. ])
	;;
    esac

    # Do we need the opcodes library?
    case ${cpu_type} in
      vax | tic30 | i386 | arm)
	;;

      *)
	need_opcodes=yes

	case "${enable_shared}" in
	yes) shared_opcodes=true ;;
	*opcodes*) shared_opcodes=true ;;
	*) shared_opcodes=false ;;
	esac
	;;
    esac

    # Any other special object files needed ?
    case ${cpu_type} in

      bfin)
	for f in config/bfin-parse.o config/bfin-lex-wrapper.o; do
	  case " $extra_objects " in
	    *" $f "*) ;;
	    *) extra_objects="$extra_objects $f" ;;
	  esac
	done
	;;

      bpf)
      	if test $this_target = $target ; then
	  AC_DEFINE_UNQUOTED(DEFAULT_ARCH, "${arch}", [Default architecture.])
	fi
        using_cgen=yes
        ;;
      epiphany | fr30 | ip2k | iq2000 | lm32 | m32r | or1k)
	using_cgen=yes
	;;

      loongarch)
	for f in config/loongarch-parse.o config/loongarch-lex-wrapper.o; do
	  case " $extra_objects " in
	    *" $f "*) ;;
	    *) extra_objects="$extra_objects $f" ;;
	  esac
	done
        # --target=loongarch[32|64]-*-*.  */
        if test $this_target = $target ; then
          AC_DEFINE_UNQUOTED(DEFAULT_ARCH, "${arch}", [Default architecture.])
        fi
	;;

      m32c)
	using_cgen=yes
	;;
      frv)
	using_cgen=yes
	;;
      m68k)
	f=config/m68k-parse.o
	case " $extra_objects " in
	  *" $f "*) ;;
	  *) extra_objects="$extra_objects $f" ;;
	esac
	;;

      mep)
	using_cgen=yes
	;;

      mips)
	for f in itbl-parse.o itbl-lex-wrapper.o itbl-ops.o; do
	  case " $extra_objects " in
	    *" $f "*) ;;
	    *) extra_objects="$extra_objects $f" ;;
	  esac
	done
	;;

      mt)
	using_cgen=yes
	;;

      nds32)
	# setup NDS32_LINUX_TOOLCHAIN definition
	if test "linux" = $em; then
	  AC_DEFINE(NDS32_LINUX_TOOLCHAIN, 1,
	    [Define value for nds32_linux_toolchain])
	else
	  AC_DEFINE(NDS32_LINUX_TOOLCHAIN, 0,
	    [Define default value for nds32_linux_toolchain])
	fi

	# Decide BASELINE, REDUCED_REGS, FPU_DP_EXT, FPU_SP_EXT features
	# based on arch_name.
	AC_MSG_CHECKING(for default configuration of --with-arch)
	if test "x${with_arch}" != x; then
	  case ${with_arch} in
	    v2j | v2s | v2f | v2 | v3m | v3j | v3s | v3f | v3 )
	      AC_DEFINE_UNQUOTED(NDS32_DEFAULT_ARCH_NAME, "$with_arch",
				 [Define value for nds32_arch_name])
	      ;;
	    *)
	      AC_MSG_ERROR(This kind of arch name does *NOT* exist!)
	      ;;
	  esac
	fi
	AC_MSG_RESULT($with_arch)

	# Decide features one by one.
	AC_MSG_CHECKING(for default configuration of --enable-dx-regs)
	if test "x${enable_dx_regs}" = xyes; then
	  AC_DEFINE(NDS32_DEFAULT_DX_REGS, 1,
		    [Define value for nds32_dx_regs])
	else
	  AC_DEFINE(NDS32_DEFAULT_DX_REGS, 0,
		    [Define default value for nds32_dx_regs])
	fi
	AC_MSG_RESULT($enable_dx_regs)

	AC_MSG_CHECKING(for default configuration of --enable-perf-ext)
	if test "x${enable_perf_ext}" = xno; then
	  AC_DEFINE(NDS32_DEFAULT_PERF_EXT, 0,
		    [Define value for nds32_perf_ext])
	else
	  AC_DEFINE(NDS32_DEFAULT_PERF_EXT, 1,
		    [Define default value for nds32_perf_ext])
	fi
	AC_MSG_RESULT($enable_perf_ext)

	AC_MSG_CHECKING(for default configuration of --enable-perf-ext2)
	if test "x${enable_perf_ext2}" = xno; then
	  AC_DEFINE(NDS32_DEFAULT_PERF_EXT2, 0,
		    [Define value for nds32_perf_ext2])
	else
	  AC_DEFINE(NDS32_DEFAULT_PERF_EXT2, 1,
		    [Define default value for nds32_perf_ext2])
	fi
	AC_MSG_RESULT($enable_perf_ext2)

	AC_MSG_CHECKING(for default configuration of --enable-string-ext)
	if test "x${enable_string_ext}" = xno; then
	  AC_DEFINE(NDS32_DEFAULT_STRING_EXT, 0,
		    [Define value for nds32_string_ext])
	else
	  AC_DEFINE(NDS32_DEFAULT_STRING_EXT, 1,
		    [Define default value for nds32_string_ext])
	fi
	AC_MSG_RESULT($enable_string_ext)

	AC_MSG_CHECKING(for default configuration of --enable-audio-ext)
	if test "x${enable_audio_ext}" = xno; then
	  AC_DEFINE(NDS32_DEFAULT_AUDIO_EXT, 0,
		    [Define value for nds32_audio_ext])
	else
	  AC_DEFINE(NDS32_DEFAULT_AUDIO_EXT, 1,
		    [Define default value for nds32_audio_ext])
	fi
	AC_MSG_RESULT($enable_audio_ext)

	AC_MSG_CHECKING(for default configuration of --enable-dsp-ext)
	if test "x${enable_dsp_ext}" = xno; then
	  AC_DEFINE(NDS32_DEFAULT_DSP_EXT, 0,
		    [Define value for nds32_dsp_ext])
	else
	  AC_DEFINE(NDS32_DEFAULT_DSP_EXT, 1,
		    [Define default value for nds32_dsp_ext])
	fi
	AC_MSG_RESULT($enable_dsp_ext)

	AC_MSG_CHECKING(for default configuration of --enable-zol-ext)
	if test "x${enable_zol_ext}" = xno; then
	  AC_DEFINE(NDS32_DEFAULT_ZOL_EXT, 0,
		    [Define value for nds32_zol_ext])
	else
	  AC_DEFINE(NDS32_DEFAULT_ZOL_EXT, 1,
		    [Define default value for nds32_zol_ext])
	fi
	AC_MSG_RESULT($enable_zol_ext)
	;;

      aarch64 | i386 | s390 | sparc)
	if test $this_target = $target ; then
	  AC_DEFINE_UNQUOTED(DEFAULT_ARCH, "${arch}", [Default architecture.])
	fi
	;;

      riscv)
        # --target=riscv[32|64]-*-*.  */
        if test $this_target = $target ; then
          AC_DEFINE_UNQUOTED(DEFAULT_ARCH, "${arch}", [Default architecture.])
        fi

        # --with-arch=<value>.  The syntax of <value> is same as Gas option -march.
        AC_MSG_CHECKING(for default configuration of --with-arch)
        if test "x${with_arch}" != x; then
        AC_DEFINE_UNQUOTED(DEFAULT_RISCV_ARCH_WITH_EXT, "$with_arch",
                           [Define default value for RISC-V -march.])
        fi
        AC_MSG_RESULT($with_arch)

        # --with-isa-spec=[2.2|20190608|20191213].
        AC_MSG_CHECKING(for default configuration of --with-isa-spec)
        if test "x${with_isa_spec}" != x; then
          AC_DEFINE_UNQUOTED(DEFAULT_RISCV_ISA_SPEC, "$with_isa_spec",
                             [Define default value for RISC-V -misa-spec.])
        fi
        AC_MSG_RESULT($with_isa_spec)

        # --with-priv-spec=[1.9.1|1.10|1.11|1.12].
        AC_MSG_CHECKING(for default configuration of --with-priv-spec)
        if test "x${with_priv_spec}" != x; then
          AC_DEFINE_UNQUOTED(DEFAULT_RISCV_PRIV_SPEC, "$with_priv_spec",
                             [Define default value for RISC-V -mpriv-spec])
        fi
        AC_MSG_RESULT($with_priv_spec)
        ;;

      rl78)
	f=config/rl78-parse.o
	case " $extra_objects " in
	  *" $f "*) ;;
	  *) extra_objects="$extra_objects $f" ;;
	esac
	;;

      rx)
	f=config/rx-parse.o
	case " $extra_objects " in
	  *" $f "*) ;;
	  *) extra_objects="$extra_objects $f" ;;
	esac
	;;

      xstormy16)
	using_cgen=yes
	;;

      xtensa)
	f=config/xtensa-relax.o
	case " $extra_objects " in
	  *" $f "*) ;;
	  *) extra_objects="$extra_objects $f" ;;
	esac
	;;

      *)
	;;
    esac

    if test $using_cgen = yes ; then
	f=cgen.o
	case " $extra_objects " in
	  *" $f "*) ;;
	  *) extra_objects="$extra_objects $f" ;;
	esac
    fi

# See if we really can support this configuration with the emulation code.

    if test $this_target = $target ; then
      obj_format=$fmt
      te_file=$em
    fi

    case ${te_file} in
      vms)
	f=config/te-vms.o
	case " $extra_objects " in
	  *" $f "*) ;;
	  *) extra_objects="$extra_objects $f" ;;
	esac ;;
    esac

# From target name and format, produce a list of supported emulations.

    case ${generic_target}-${fmt} in
      mips-*-*-*)	case "$endian" in
			big)	emulation="mipsbelf mipslelf mipself" ;;
			*)	emulation="mipslelf mipsbelf mipself" ;;
			esac ;;
      # i386-pc-pe-coff != i386-pc-coff.
      i386-*-pe-coff)	;;
      # Uncommenting the next line will turn on support for i386 AOUT
      # for the default linux configuration
      # i386-*-linux*-elf) emulation="i386elf i386aout" ;;
      #
      i386-*-aout)	emulation="i386aout" ;;
      i386-*-coff)	emulation="i386coff" ;;
      i386-*-elf)	emulation="i386elf" ;;

      # Always all formats.  The first stated emulation becomes the default.
      cris-*-*aout*)	emulation="crisaout criself" ;;
      cris-*-*)		emulation="criself crisaout" ;;
    esac

    emulations="$emulations $emulation"

done

if test ${ac_default_x86_relax_relocations} = unset; then
  ac_default_x86_relax_relocations=1
fi
AC_DEFINE_UNQUOTED(DEFAULT_GENERATE_X86_RELAX_RELOCATIONS,
  $ac_default_x86_relax_relocations,
  [Define to 1 if you want to generate x86 relax relocations by default.])

if test ${ac_default_elf_stt_common} = unset; then
  ac_default_elf_stt_common=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_GENERATE_ELF_STT_COMMON,
  $ac_default_elf_stt_common,
  [Define to 1 if you want to generate ELF common symbols with the
   STT_COMMON type by default.])

AC_DEFINE_UNQUOTED(DEFAULT_GENERATE_BUILD_NOTES,
  $ac_default_generate_build_notes,
  [Define to 1 if you want to generate GNU Build attribute notes
   by default, if none are contained in the input.])

if test ${ac_default_generate_x86_used_note} = unset; then
  ac_default_generate_x86_used_note=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_X86_USED_NOTE,
  $ac_default_generate_x86_used_note,
  [Define to 1 if you want to generate GNU x86 used ISA and feature
   properties by default.])

if test ${ac_default_generate_riscv_attr} = unset; then
  ac_default_generate_riscv_attr=1
fi

AC_DEFINE_UNQUOTED(DEFAULT_RISCV_ATTR,
  $ac_default_generate_riscv_attr,
  [Define to 1 if you want to generate RISC-V arch attribute by default.])

if test ${ac_default_mips_fix_loongson3_llsc} = unset; then
  ac_default_mips_fix_loongson3_llsc=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_MIPS_FIX_LOONGSON3_LLSC,
  $ac_default_mips_fix_loongson3_llsc,
  [Define to 1 if you want to fix Loongson3 LLSC Errata by default.])

if test x$ac_default_compressed_debug_sections = xyes ; then
  AC_DEFINE(DEFAULT_FLAG_COMPRESS_DEBUG, 1, [Define if you want compressed debug sections by default.])
fi

AC_DEFINE_UNQUOTED(DEFAULT_COMPRESSED_DEBUG_ALGORITHM, $ac_default_compressed_debug_sections_algorithm,
		   [Default compression algorithm for --enable-compressed-debug-sections.])

# Turn on all targets if possible
if test ${all_targets} = "yes"; then
  case ${target_cpu_type} in
  i386)
    case ${obj_format} in
    aout)
      emulations="$emulations i386coff i386elf"
      ;;
    coff)
      emulations="$emulations i386aout i386elf"
    ;;
    elf)
      emulations="$emulations i386aout i386coff"
      ;;
    esac
  ;;
  x86_64)
    case ${obj_format} in
    aout)
      emulations="$emulations i386coff i386elf"
      ;;
    coff)
      emulations="$emulations i386aout i386elf"
    ;;
    elf)
      emulations="$emulations i386aout i386coff"
      ;;
    esac
  ;;
  esac
fi

# PE code has way too many macros tweaking behaviour
case ${te_file} in
  pe*) emulations="" ;;
esac

# Assign floating point type.  Most processors with FP support
# IEEE FP.  On those that don't support FP at all, usually IEEE
# is emulated.
case ${target_cpu} in
  vax | pdp11 )	atof=vax ;;
  *)		atof=ieee ;;
esac

case "${obj_format}" in
  "") AC_MSG_ERROR(GAS does not know what format to use for target ${target}) ;;
esac

# Unfortunately the cpu in cpu-opc.h file isn't always $(TARGET_CPU).
cgen_cpu_prefix=""
if test $using_cgen = yes ; then
  case ${target_cpu} in
    or1knd)
       cgen_cpu_prefix=or1k ;;
    *) cgen_cpu_prefix=${target_cpu} ;;
  esac
  AC_SUBST(cgen_cpu_prefix)
  AC_DEFINE(USING_CGEN, 1, [Using cgen code?])
fi

dnl
dnl Make sure the desired support files exist.
dnl

if test ! -r ${srcdir}/config/tc-${target_cpu_type}.c; then
  AC_MSG_ERROR(GAS does not support target CPU ${target_cpu_type})
fi

if test ! -r ${srcdir}/config/obj-${obj_format}.c; then
  AC_MSG_ERROR(GAS does not have support for object file format ${obj_format})
fi

# Some COFF configurations want these random other flags set.
case ${obj_format} in
  coff)
    case ${target_cpu_type} in
      i386) AC_DEFINE(I386COFF, 1, [Using i386 COFF?]) ;;
      x86_64) AC_DEFINE(I386COFF, 1, [Using i386 COFF?]) ;;
    esac
    ;;
esac

# Getting this done right is going to be a bitch.  Each configuration specified
# with --enable-targets=... should be checked for environment, format, cpu
# setting.
#
# For each configuration, the necessary object file support code must be linked
# in.  This might be only one, it might be up to four.  The necessary emulation
# code needs to be provided, too.
#
# And then there's "--enable-targets=all"....
#
# For now, just always do it for MIPS ELF configurations.  Sigh.

formats="${obj_format}"
emfiles=""
EMULATIONS=""
GAS_UNIQ(emulations)
for em in . $emulations ; do
  case $em in
    .)	continue ;;
    mipsbelf | mipslelf | mipself)
	fmt=elf   file=mipself ;;
    *coff)
	fmt=coff  file=$em ;;
    *aout)
	fmt=aout file=$em ;;
    *elf)
	fmt=elf file=$em ;;
  esac
  formats="$formats $fmt"
  emfiles="$emfiles config/e-$file.o"
  EMULATIONS="$EMULATIONS &$em,"
done
GAS_UNIQ(formats)
GAS_UNIQ(emfiles)
if test `set . $formats ; shift ; echo $#` -gt 1 ; then
  for fmt in $formats ; do
    case $fmt in
      aout)	AC_DEFINE(OBJ_MAYBE_AOUT, 1,    [a.out support?])   ;;
      coff)	AC_DEFINE(OBJ_MAYBE_COFF, 1,    [COFF support?])    ;;
      ecoff)	AC_DEFINE(OBJ_MAYBE_ECOFF, 1,   [ECOFF support?])   ;;
      elf)	AC_DEFINE(OBJ_MAYBE_ELF, 1,     [ELF support?])     ;;
      generic)	AC_DEFINE(OBJ_MAYBE_GENERIC, 1, [generic support?]) ;;
      som)	AC_DEFINE(OBJ_MAYBE_SOM, 1,     [SOM support?])     ;;
    esac
    extra_objects="$extra_objects config/obj-$fmt.o"
  done
  obj_format=multi
fi
if test `set . $emfiles ; shift ; echo $#` -gt 0 ; then
  DEFAULT_EMULATION=`set . $emulations ; echo $2`
  # e-mipself has more than one emulation per file, e-i386* has just one at the
  # moment.  If only one emulation is specified, then don't define
  # USE_EMULATIONS or include any of the e-files as they will only be bloat.
  case "${obj_format}${emfiles}" in
    multi* | *mipself*)
      extra_objects="$extra_objects $emfiles"
      AC_DEFINE(USE_EMULATIONS, 1, [Use emulation support?]) ;;
  esac
fi
AC_SUBST(extra_objects)
AC_DEFINE_UNQUOTED(EMULATIONS, $EMULATIONS, [Supported emulations.])
AC_DEFINE_UNQUOTED(DEFAULT_EMULATION, "$DEFAULT_EMULATION",
		   [Default emulation.])

reject_dev_configs=yes

case ${reject_dev_configs}-${dev} in
  yes-yes) # Oops.
    AC_MSG_ERROR(GAS does not support the ${generic_target} configuration.)
    ;;
esac

AC_SUBST(target_cpu_type)
AC_SUBST(obj_format)
AC_SUBST(te_file)
AC_SUBST(install_tooldir)
AC_SUBST(atof)
dnl AC_SUBST(emulation)

# do we need the opcodes library?
case "${need_opcodes}" in
yes)
  OPCODES_LIB=../opcodes/libopcodes.la
  ;;
esac

AC_SUBST(OPCODES_LIB)

AC_DEFINE_UNQUOTED(TARGET_ALIAS,	"${target_alias}", [Target alias.])
AC_DEFINE_UNQUOTED(TARGET_CANONICAL,	"${target}",       [Canonical target.])
AC_DEFINE_UNQUOTED(TARGET_CPU,		"${target_cpu}",   [Target CPU.])
AC_DEFINE_UNQUOTED(TARGET_VENDOR,	"${target_vendor}", [Target vendor.])
AC_DEFINE_UNQUOTED(TARGET_OS,		"${target_os}",    [Target OS.])

AC_PROG_YACC
AM_PROG_LEX

ALL_LINGUAS="es fi fr id ja ru rw sv tr uk zh_CN"
ZW_GNU_GETTEXT_SISTER_DIR
AM_PO_SUBDIRS

AM_MAINTAINER_MODE
AM_CONDITIONAL(GENINSRC_NEVER, false)
AC_EXEEXT

AC_CHECK_HEADERS(memory.h sys/stat.h sys/types.h unistd.h)

# Put this here so that autoconf's "cross-compiling" message doesn't confuse
# people who are not cross-compiling but are compiling cross-assemblers.
AC_MSG_CHECKING(whether compiling a cross-assembler)
if test "${host}" = "${target}"; then
  cross_gas=no
else
  cross_gas=yes
  AC_DEFINE(CROSS_COMPILE, 1, [Compiling cross-assembler?])
fi
AC_MSG_RESULT($cross_gas)

AC_CHECK_FUNCS(strsignal)

AM_LC_MESSAGES

# do we need the math library?
case "${need_libm}" in
yes)
  LT_LIB_M
  AC_SUBST(LIBM)
  ;;
esac

# Some non-ANSI preprocessors botch requoting inside strings.  That's bad
# enough, but on some of those systems, the assert macro relies on requoting
# working properly!
GAS_WORKING_ASSERT

# On some systems, the system header files may not declare malloc, realloc,
# and free.  There are places where gas needs these functions to have been
# declared -- such as when taking their addresses.
gas_test_headers="
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
"

AC_MSG_CHECKING(for a known getopt prototype in unistd.h)
AC_CACHE_VAL(gas_cv_decl_getopt_unistd_h,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>], [extern int getopt (int, char *const*, const char *);])],
gas_cv_decl_getopt_unistd_h=yes, gas_cv_decl_getopt_unistd_h=no)])
AC_MSG_RESULT($gas_cv_decl_getopt_unistd_h)
if test $gas_cv_decl_getopt_unistd_h = yes; then
  AC_DEFINE([HAVE_DECL_GETOPT], 1,
	    [Is the prototype for getopt in <unistd.h> in the expected format?])
fi

GAS_CHECK_DECL_NEEDED(environ, f, char **f, $gas_test_headers)
GAS_CHECK_DECL_NEEDED(ffs, f, int (*f)(int), $gas_test_headers)

AC_CHECK_DECLS([asprintf, mempcpy, stpcpy])

BFD_BINARY_FOPEN

# Link in zlib/zstd if we can.  This allows us to write compressed debug sections.
AM_ZLIB
AC_ZSTD

# Support for VMS timestamps via cross compile

if test "$ac_cv_header_time_h" = yes; then
  GAS_HAVE_TIME_TYPE_MEMBER(struct tm, tm_gmtoff)
fi

if test "$ac_cv_header_sys_stat_h" = yes; then
    GAS_HAVE_SYS_STAT_TYPE_MEMBER(struct stat, st_mtim, tv_sec)
    GAS_HAVE_SYS_STAT_TYPE_MEMBER(struct stat, st_mtim, tv_nsec)
fi


dnl Required for html, pdf, install-pdf and install-html targets.
AC_SUBST(datarootdir)
AC_SUBST(docdir)
AC_SUBST(htmldir)
AC_SUBST(pdfdir)

dnl This must come last.

dnl We used to make symlinks to files in the source directory, but now
dnl we just use the right name for .c files, and create .h files in
dnl the build directory which include the right .h file.  Make sure
dnl the old symlinks don't exist, so that a reconfigure in an existing
dnl directory behaves reasonably.

AC_CONFIG_FILES(Makefile po/Makefile.in:po/Make-in)
AC_CONFIG_COMMANDS([default],
[rm -f targ-cpu.c targ-cpu.h obj-format.h obj-format.c targ-env.h atof-targ.c itbl-cpu.h
 echo '#include "tc-'"${target_cpu_type}"'.h"' > targ-cpu.h
 echo '#include "obj-'"${obj_format}"'.h"' > obj-format.h
 echo '#include "te-'"${te_file}"'.h"' > targ-env.h
 echo '#include "itbl-'"${target_cpu_type}"'.h"' > itbl-cpu.h
 if test "x$cgen_cpu_prefix" != x ; then
   echo '#include "opcodes/'"${cgen_cpu_prefix}"'-desc.h"' > cgen-desc.h
 fi],
[target_cpu_type=${target_cpu_type}
 cgen_cpu_prefix=${cgen_cpu_prefix}
 obj_format=${obj_format}
 te_file=${te_file}])

AC_OUTPUT

GNU_MAKE_JOBSERVER
