#!/bin/sh
# genscripts.sh - generate the ld-emulation-target specific files
# Copyright (C) 2004-2023 Free Software Foundation, Inc.
#
# This file is part of the Gnu Linker.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GLD; see the file COPYING.  If not, write to the Free
# Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
# 02110-1301, USA.
#
# Usage: genscripts_extra.sh \
#          srcdir \
#          libdir \
#          prefix \
#          exec_prefix \
#          host \
#          target \
#          target_alias \
#          default_emulation \
#          native_lib_dirs \
#          use_sysroot \
#          enable_initfini_array \
#          this_emulation \
# optional:
#          tool_dir
#
# Sample usage:
#
#   genscripts_extra.sh \
#    /sources/ld \
#    /usr/local/lib \
#    /usr/local \
#    /usr/local \
#    sparc-sun-sunos4.1.3 \
#    sparc-sun-sunos4.1.3 \
#    sparc-sun-sunos4.1.3 \
#    sun4 \
#    "" \
#    no \
#    sun3 \
#    sparc-sun-sunos4.1.3 \
#    sparc.sh
#
# produces the linker scripts:
#
#   sun3.x       [default linker script]
#   sun3.xbn     [used when the linker is invoked with "-N"]
#   sun3.xn      [used when the linker is invoked with "-n"]
#   sun3.xr      [used when the linker is invoked with "-r"]
#   sun3.xu      [used when the linker is invoked with "-Ur"]
#
# depending on platform specific settings linker scripts with the
# following suffixes might be generated as well:
#
# xdwe:   -pie    -z combreloc -z separate-code -z relro -z now
# xdw:    -pie    -z combreloc                  -z relro -z now
# xdceo:  -pie    -z combreloc -z separate-code -z relro
# xdce:   -pie    -z combreloc -z separate-code
# xdco:   -pie    -z combreloc                  -z relro
# xdc:    -pie    -z combreloc
# xdeo:   -pie                 -z separate-code -z relro
# xde:    -pie                 -z separate-code
# xdo:    -pie                                  -z relro
# xd:     -pie
#
# xswe:   -shared -z combreloc -z separate-code -z relro -z now
# xsw:    -shared -z combreloc                  -z relro -z now
# xsceo:  -shared -z combreloc -z separate-code -z relro
# xsce:   -shared -z combreloc -z separate-code
# xsco:   -shared -z combreloc                  -z relro
# xsc:    -shared -z combreloc
# xseo:   -shared              -z separate-code -z relro
# xse:    -shared              -z separate-code
# xso:    -shared                               -z relro
# xs:     -shared
#
# xwe:            -z combreloc -z separate-code -z relro -z now
# xw:             -z combreloc                  -z relro -z now
# xceo:           -z combreloc -z separate-code -z relro
# xce:            -z combreloc -z separate-code
# xco:            -z combreloc                  -z relro
# xc:             -z combreloc
# xeo:                         -z separate-code -z relro
# xe:                          -z separate-code
# xo:                                           -z relro
#
#
# It also produced the C source file:
#
#   em_sun3.c
#
# which is then compiled into the linker.
#
# The linker scripts are created by running the shell script
# /sources/ld/emulparams/sparc.sh to set the value of ${SCRIPT_NAME}
# (and any other variables it wants to).  ${SCRIPT_NAME} is then
# invoked with a variable called ${LD_FLAG} to tell it which version
# of the linker script to create.


srcdir=$1
libdir=$2
prefix=$3
exec_prefix=$4
host=$5
target=$6
target_alias=$7
DEPDIR=$8
shift 8
LIB_PATH=$1
EMULATION_LIBPATH=$2
NATIVE_LIB_DIRS=$3
use_sysroot=$4
ENABLE_INITFINI_ARRAY=$5
shift 5
EMULATION_NAME=$1
TOOL_LIB=$2

source_sh()
{
  if test -n "${DEPDIR}"; then
    echo $1 >> ${DEPDIR}/e${EMULATION_NAME}.Tc
  fi
  . $1
}

if test -n "${DEPDIR}"; then
  rm -f ${DEPDIR}/e${EMULATION_NAME}.Tc
fi

# Include the emulation-specific parameters:
CUSTOMIZER_SCRIPT="${srcdir}/emulparams/${EMULATION_NAME}.sh"
source_sh ${CUSTOMIZER_SCRIPT}

if test -d ldscripts; then
  true
else
  mkdir ldscripts
fi

# Set some flags for the emultempl scripts.  USE_LIBPATH will
# be set for any libpath-using emulation; NATIVE will be set for a
# libpath-using emulation where ${host} = ${target}.  NATIVE
# may already have been set by the emulparams file, but that's OK
# (it'll just get set to "yes" twice).

case " $EMULATION_LIBPATH " in
  *" ${EMULATION_NAME} "*)
    if [ "x${host}" = "x${target}" ] ; then
      NATIVE=yes
      USE_LIBPATH=yes
    elif [ "x${use_sysroot}" = "xyes" ] ; then
      USE_LIBPATH=yes
    fi
    ;;
esac

# If the emulparams file sets NATIVE, make sure USE_LIBPATH is set also.
if test "x$NATIVE" = "xyes" ; then
  USE_LIBPATH=yes
fi

# Set the library search path, for libraries named by -lfoo.
# If LIB_PATH is defined (e.g., by Makefile) and non-empty, it is used.
# Otherwise, the default is set here.
#
# The format is the usual list of colon-separated directories.
# To force a logically empty LIB_PATH, do LIBPATH=":".
#
# If we are using a sysroot, prefix library paths with "=" to indicate this.
#
# If the emulparams file set LIBPATH_SUFFIX, prepend an extra copy of
# the library path with the suffix applied.

# Paths with LIBPATH_SUFFIX
lib_path1=
# Paths without LIBPATH_SUFFIX
lib_path2=
if [ "${LIB_PATH}" != ":" ] ; then
  lib_path2=${LIB_PATH}
fi

# Add args to lib_path1 and lib_path2, discarding any duplicates
append_to_lib_path()
{
  if [ $# != 0 ]; then
    for lib in "$@"; do
      # The "=" is harmless if we aren't using a sysroot, but also needless.
      if [ "x${use_sysroot}" = "xyes" ] ; then
	lib="=${lib}"
      fi
      skip_lib=no
      for libpath_suffix in ${LIBPATH_SUFFIX}; do
	case "${lib}" in
	  *${libpath_suffix})
	    case :${lib_path1}: in
	      *:${lib}:*) ;;
	      ::) lib_path1=${lib} ;;
	      *) lib_path1=${lib_path1}:${lib} ;;
	    esac ;;
	  *)
	    if test -n "${LIBPATH_SUFFIX_SKIP}"; then
	      case "${lib}" in
		*${LIBPATH_SUFFIX_SKIP}) skip_lib=yes ;;
	      esac
	    fi
	    if test "${skip_lib}" = "no"; then
	      case :${lib_path1}: in
		*:${lib}${libpath_suffix}:*) ;;
		::) lib_path1=${lib}${libpath_suffix} ;;
		*) lib_path1=${lib_path1}:${lib}${libpath_suffix} ;;
	      esac
	    fi ;;
	esac
      done
      if test "${skip_lib}" = "no"; then
	case :${lib_path1}:${lib_path2}: in
	  *:${lib}:*) ;;
	  *::) lib_path2=${lib} ;;
	  *) lib_path2=${lib_path2}:${lib} ;;
	esac
      fi
    done
  fi
}

# Always search $(tooldir)/lib, aka /usr/local/TARGET/lib when native
# except when LIBPATH=":".
if [ "${LIB_PATH}" != ":" ] ; then
  libs=
  if [ "x${TOOL_LIB}" = "x" ] ; then
    if [ "x${NATIVE}" = "xyes" ] ; then
      libs="${exec_prefix}/${target_alias}/lib"
    fi
  else
    # For multilib'ed targets, ensure both ${target_alias}/lib${LIBPATH_SUFFIX}
    # and ${TOOL_LIB}/lib${LIBPATH_SUFFIX} are in the default search path,
    # because 64bit libraries may be in both places, depending on
    # cross-development setup method (e.g.: /usr/s390x-linux/lib64
    # vs. /usr/s390-linux/lib64)
    for libpath_suffix in ${LIBPATH_SUFFIX}; do
      case "${NATIVE}:${libpath_suffix}:${TOOL_LIB}" in
	:* | *::* | *:*:*${libpath_suffix}) ;;
	*) libs="${exec_prefix}/${target_alias}/lib${libpath_suffix}" ;;
      esac
    done
    libs="${exec_prefix}/${TOOL_LIB}/lib ${libs}"
  fi
  append_to_lib_path ${libs}
fi

if [ "x${LIB_PATH}" = "x" ] && [ "x${USE_LIBPATH}" = xyes ] ; then
  libs=${NATIVE_LIB_DIRS}
  if [ "x${NATIVE}" = "xyes" ] ; then
    case " ${libs} " in
      *" ${libdir} "*) ;;
      *) libs="${libdir} ${libs}" ;;
    esac
  fi
  append_to_lib_path ${libs}
fi

case :${lib_path1}:${lib_path2}: in
  *:: | ::*) LIB_PATH=${lib_path1}${lib_path2} ;;
  *) LIB_PATH=${lib_path1}:${lib_path2} ;;
esac

LIB_SEARCH_DIRS=`echo ${LIB_PATH} | sed -e 's/:/ /g' -e 's/\([^ ][^ ]*\)/SEARCH_DIR(\\"\1\\");/g'`

# We need it for testsuite.
set $EMULATION_LIBPATH
if [ "x$1" = "x$EMULATION_NAME" ]; then
    test -d tmpdir || mkdir tmpdir
    rm -f tmpdir/libpath.exp
    echo "set libpath \"${LIB_PATH}\"" | sed -e 's/:/ /g' > tmpdir/libpath.exp
fi

if [ "x$SCRIPT_NAME" = "xelf" ]; then
  GENERATE_COMBRELOC_SCRIPT=yes
fi

SEGMENT_SIZE=${SEGMENT_SIZE-${MAXPAGESIZE-${TARGET_PAGE_SIZE}}}

# Determine DATA_ALIGNMENT for the 5 variants, using
# values specified in the emulparams/<script_to_run>.sh file or default.

DATA_ALIGNMENT_="${DATA_ALIGNMENT_-${DATA_ALIGNMENT-ALIGN(${SEGMENT_SIZE})}}"
DATA_ALIGNMENT_n="${DATA_ALIGNMENT_n-${DATA_ALIGNMENT_}}"
DATA_ALIGNMENT_N="${DATA_ALIGNMENT_N-${DATA_ALIGNMENT-.}}"
DATA_ALIGNMENT_r="${DATA_ALIGNMENT_r-${DATA_ALIGNMENT-}}"
DATA_ALIGNMENT_u="${DATA_ALIGNMENT_u-${DATA_ALIGNMENT_r}}"

LD_FLAG=r
DATA_ALIGNMENT=${DATA_ALIGNMENT_r}
DEFAULT_DATA_ALIGNMENT="ALIGN(${SEGMENT_SIZE})"
( echo "/* Script for -r */"
  source_sh ${CUSTOMIZER_SCRIPT}
  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xr

LD_FLAG=u
DATA_ALIGNMENT=${DATA_ALIGNMENT_u}
CONSTRUCTING=" "
( echo "/* Script for -Ur */"
  source_sh ${CUSTOMIZER_SCRIPT}
  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xu

DATA_ALIGNMENT=${DATA_ALIGNMENT_}
RELOCATING=" "
LD_FLAG=
( echo "/* Default linker script, for normal executables */"
  source_sh ${CUSTOMIZER_SCRIPT}
  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.x

LD_FLAG=textonly
( echo "/* Script for -z separate-code */"
  source_sh ${CUSTOMIZER_SCRIPT}
  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xe

if test -n "$GENERATE_RELRO_SCRIPT"; then
    LD_FLAG=
    RELRO=" "
    ( echo "/* Script for -z relro */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xo
    LD_FLAG=textonly
    ( echo "/* Script for -z separate-code -z relro */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xeo
    unset RELRO
fi
LD_FLAG=n
DATA_ALIGNMENT=${DATA_ALIGNMENT_n}
( echo "/* Script for -n */"
  source_sh ${CUSTOMIZER_SCRIPT}
  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xn

LD_FLAG=N
DATA_ALIGNMENT=${DATA_ALIGNMENT_N}
( echo "/* Script for -N */"
  source_sh ${CUSTOMIZER_SCRIPT}
  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xbn

if test -n "$GENERATE_COMBRELOC_SCRIPT"; then
  DATA_ALIGNMENT=${DATA_ALIGNMENT_c-${DATA_ALIGNMENT_}}
  LD_FLAG=c
  COMBRELOC=ldscripts/${EMULATION_NAME}.xc.tmp
  ( echo "/* Script for -z combreloc */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xc
  rm -f ${COMBRELOC}
  LD_FLAG=ctextonly
  COMBRELOC=ldscripts/${EMULATION_NAME}.xce.tmp
  ( echo "/* Script for -z combreloc -z separate-code */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xce
  rm -f ${COMBRELOC}
  RELRO_NOW=" "
  LD_FLAG=w
  COMBRELOC=ldscripts/${EMULATION_NAME}.xw.tmp
  ( echo "/* Script for -z combreloc -z relro -z now */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xw
  rm -f ${COMBRELOC}
  LD_FLAG=wtextonly
  COMBRELOC=ldscripts/${EMULATION_NAME}.xwe.tmp
  ( echo "/* Script for -z combreloc -z separate-code -z relro -z now */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xwe
  rm -f ${COMBRELOC}
  COMBRELOC=
  unset RELRO_NOW
  if test -n "$GENERATE_RELRO_SCRIPT"; then
      LD_FLAG=c
      RELRO=" "
      COMBRELOC=ldscripts/${EMULATION_NAME}.xco.tmp
      ( echo "/* Script for -z combreloc -z relro */"
	source_sh ${CUSTOMIZER_SCRIPT}
	source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
      ) | sed -e '/^ *$/d;s/[    ]*$//' > ldscripts/${EMULATION_NAME}.xco
      rm -f ${COMBRELOC}
      LD_FLAG=ctextonly
      COMBRELOC=ldscripts/${EMULATION_NAME}.xceo.tmp
      ( echo "/* Script for -z combreloc -z separate-code -z relro */"
	source_sh ${CUSTOMIZER_SCRIPT}
	source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
      ) | sed -e '/^ *$/d;s/[    ]*$//' > ldscripts/${EMULATION_NAME}.xceo
      rm -f ${COMBRELOC}
      COMBRELOC=
      unset RELRO
  fi
fi

if test -n "$GENERATE_SHLIB_SCRIPT"; then
  DATA_ALIGNMENT=${DATA_ALIGNMENT_s-${DATA_ALIGNMENT_}}
  CREATE_SHLIB=" "
  LD_FLAG=shared
  ( echo "/* Script for -shared */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xs
  LD_FLAG=sharedtextonly
  ( echo "/* Script for -shared -z separate-code */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xse

  if test -n "$GENERATE_RELRO_SCRIPT"; then
      RELRO=" "
      LD_FLAG=shared
      ( echo "/* Script for -shared -z relro */"
	source_sh ${CUSTOMIZER_SCRIPT}
	source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
      ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xso
      LD_FLAG=sharedtextonly
      ( echo "/* Script for -shared -z separate-code -z relro */"
	source_sh ${CUSTOMIZER_SCRIPT}
	source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
      ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xseo
      unset RELRO
  fi
  if test -n "$GENERATE_COMBRELOC_SCRIPT"; then
    DATA_ALIGNMENT=${DATA_ALIGNMENT_sc-${DATA_ALIGNMENT}}
    LD_FLAG=cshared
    COMBRELOC=ldscripts/${EMULATION_NAME}.xsc.tmp
    ( echo "/* Script for -shared -z combreloc */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xsc
    rm -f ${COMBRELOC}
    LD_FLAG=csharedtextonly
    COMBRELOC=ldscripts/${EMULATION_NAME}.xsce.tmp
    ( echo "/* Script for -shared -z combreloc -z separate-code */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xsce
    rm -f ${COMBRELOC}
    RELRO_NOW=" "
    LD_FLAG=wshared
    COMBRELOC=ldscripts/${EMULATION_NAME}.xsw.tmp
    ( echo "/* Script for -shared -z combreloc -z relro -z now */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xsw
    rm -f ${COMBRELOC}
    LD_FLAG=wsharedtextonly
    COMBRELOC=ldscripts/${EMULATION_NAME}.xswe.tmp
    ( echo "/* Script for -shared -z combreloc -z separate-code -z relro -z now */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xswe
    rm -f ${COMBRELOC}
    unset RELRO_NOW

    if test -n "$GENERATE_RELRO_SCRIPT"; then
	LD_FLAG=wshared
	RELRO=" "
	COMBRELOC=ldscripts/${EMULATION_NAME}.xsco.tmp
	( echo "/* Script for -shared -z combreloc -z relro */"
	  source_sh ${CUSTOMIZER_SCRIPT}
	  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
	) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xsco
	rm -f ${COMBRELOC}
	LD_FLAG=wsharedtextonly
	COMBRELOC=ldscripts/${EMULATION_NAME}.xsceo.tmp
	( echo "/* Script for -shared -z combreloc -z separate-code -z relro */"
	  source_sh ${CUSTOMIZER_SCRIPT}
	  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
	) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xsceo
	rm -f ${COMBRELOC}
	unset RELRO
    fi
    COMBRELOC=
  fi
  unset CREATE_SHLIB
fi

if test -n "$GENERATE_PIE_SCRIPT"; then
  DATA_ALIGNMENT=${DATA_ALIGNMENT_s-${DATA_ALIGNMENT_}}
  CREATE_PIE=" "
  LD_FLAG=pie
  ( echo "/* Script for -pie */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xd
  LD_FLAG=pietextonly
  ( echo "/* Script for -pie -z separate-code */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xde
  if test -n "$GENERATE_RELRO_SCRIPT"; then
      RELRO=" "
      LD_FLAG=pie
      ( echo "/* Script for -pie -z relro */"
	source_sh ${CUSTOMIZER_SCRIPT}
	source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
      ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xdo
      LD_FLAG=pietextonly
      ( echo "/* Script for -pie -z separate-code -z relro */"
	source_sh ${CUSTOMIZER_SCRIPT}
	source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
      ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xdeo
      unset RELRO
  fi
  if test -n "$GENERATE_COMBRELOC_SCRIPT"; then
    DATA_ALIGNMENT=${DATA_ALIGNMENT_sc-${DATA_ALIGNMENT}}
    COMBRELOC=ldscripts/${EMULATION_NAME}.xdc.tmp
    LD_FLAG=cpie
    ( echo "/* Script for -pie -z combreloc */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xdc
    rm -f ${COMBRELOC}
    LD_FLAG=cpietextonly
    COMBRELOC=ldscripts/${EMULATION_NAME}.xdce.tmp
    ( echo "/* Script for -pie -z combreloc -z separate-code */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xdce
    rm -f ${COMBRELOC}
    RELRO_NOW=" "
    LD_FLAG=wpie
    COMBRELOC=ldscripts/${EMULATION_NAME}.xdw.tmp
    ( echo "/* Script for -pie -z combreloc -z relro -z now */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xdw
    rm -f ${COMBRELOC}
    LD_FLAG=wpietextonly
    COMBRELOC=ldscripts/${EMULATION_NAME}.xdwe.tmp
    ( echo "/* Script for -pie -z combreloc -z separate-code -z relro -z now */"
      source_sh ${CUSTOMIZER_SCRIPT}
      source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
    ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xdwe
    rm -f ${COMBRELOC}
    unset RELRO_NOW

    if test -n "$GENERATE_RELRO_SCRIPT"; then
	LD_FLAG=wpie
	RELRO=" "
	COMBRELOC=ldscripts/${EMULATION_NAME}.xdco.tmp
	( echo "/* Script for -pie -z combreloc -z relro */"
	  source_sh ${CUSTOMIZER_SCRIPT}
	  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
	) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xdco
	rm -f ${COMBRELOC}
	LD_FLAG=wpietextonly
	COMBRELOC=ldscripts/${EMULATION_NAME}.xdceo.tmp
	( echo "/* Script for -pie -z combreloc -z separate-code -z relro */"
	  source_sh ${CUSTOMIZER_SCRIPT}
	  source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
	) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xdceo
	rm -f ${COMBRELOC}

	unset RELRO
    fi
    COMBRELOC=
  fi
  unset CREATE_PIE
fi

if test -n "$GENERATE_AUTO_IMPORT_SCRIPT"; then
  LD_FLAG=auto_import
  DATA_ALIGNMENT=${DATA_ALIGNMENT_}
  ( echo "/* Script for --enable-auto-import */"
    source_sh ${CUSTOMIZER_SCRIPT}
    source_sh ${srcdir}/scripttempl/${SCRIPT_NAME}.sc
  ) | sed -e '/^ *$/d;s/[	 ]*$//' > ldscripts/${EMULATION_NAME}.xa
fi

case "$COMPILE_IN: $EMULATION_LIBPATH " in
    :*" ${EMULATION_NAME} "*) COMPILE_IN=yes;;
esac

# PR ld/5652:
# Determine if the shell has support for the variable BASH_LINENO.
# When it is the case, it is only available inside functions.
has_lineno()
{
  test "x$BASH_LINENO" != "x"
}

# Enable accruate error source in the compiler error messages, if possible.
if has_lineno; then
  . ${srcdir}/genscrba.sh
else
  source_em()
  {
    source_sh $1
  }
  fragment()
  {
    cat >> e${EMULATION_NAME}.c
  }
fi

# Generate e${EMULATION_NAME}.c.
# Start with an empty file, then the sourced .em script
# can use the "fragment" function to append.
> e${EMULATION_NAME}.c
source_em ${srcdir}/emultempl/${TEMPLATE_NAME-generic}.em

if test -n "${DEPDIR}"; then
  ecdeps=
  for dep in `cat ${DEPDIR}/e${EMULATION_NAME}.Tc`; do
    case " $ecdeps " in
      *" $dep "*): ;;
      *) ecdeps="$ecdeps $dep" ;;
    esac
  done
  rm -f ${DEPDIR}/e${EMULATION_NAME}.Tc
  echo "e${EMULATION_NAME}.c:${ecdeps}" > ${DEPDIR}/e${EMULATION_NAME}.Pc
fi
