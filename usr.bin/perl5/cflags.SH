#!/bin/sh

# Generate the cflags script, which is used to determine what cflags
# to pass to the compiler for compiling the core perl.
#
# This does NOT affect the XS compilation (ext, dist, cpan)
# since that uses %Config values directly.
#
# For example, since -Wall adds -Wunused-*, a bare -Wall (without
# amending that with -Wno-unused-..., or with the PERL_UNUSED_...)
# would be too much for XS code because there are too many generated
# but often unused things.
#
# We create a temporary test C program and repeatedly compile it with
# various candidate flags, and from the compiler output, determine what
# flags are supported.
#
# From this we initialise the following variables in the cflags script:
#
#   $myccflags (possibly edited version of $Config{ccflags})
#   $warn
#   $stdflags
#   $extra
#   $_exe

case $PERL_CONFIG_SH in
'')
	if test -f config.sh; then TOP=.;
	elif test -f ../config.sh; then TOP=..;
	elif test -f ../../config.sh; then TOP=../..;
	elif test -f ../../../config.sh; then TOP=../../..;
	elif test -f ../../../../config.sh; then TOP=../../../..;
	else
		echo "Can't find config.sh."; exit 1
	fi
	. $TOP/config.sh
	;;
esac
# This forces SH files to create target in same directory as SH file.
# This is so that make depend always knows where to find SH derivatives.
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac

if test -f config_h.SH -a ! -f config.h; then
    . ./config_h.SH
    CONFIG_H=already-done
fi

warn=''

# Add -Wall for the core modules iff gcc and not already -Wall
case "$gccversion" in
'') ;;
Intel*) ;; # The Intel C++ plays gcc on TV but is not really it.
*)  case "$ccflags" in
    *-Wall*) ;;
    *) warn="$warn -Wall" ;;
    esac
    ;;
esac

# Create a test source file for testing what options can be fed to
# gcc in this system; include a selection of most common and commonly
# hairy include files.

cat >_cflags.c <<__EOT__
#include "EXTERN.h"
#include "perl.h"
/* The stdio.h, errno.h, and setjmp.h should be there in any ANSI C89. */
#include <stdio.h>
#include <errno.h>
#include <setjmp.h>
/* Just in case the inclusion of perl.h did not
 * pull in enough system headers, let's try again. */
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#ifdef I_DIRENT
#include <dirent.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef I_SYS_PARAM
#include <sys/param.h>
#endif
#ifdef I_SYS_RESOURCE
#include <sys/resource.h>
#endif
#ifdef I_SYS_SELECT
#include <sys/select.h>
#endif
#if defined(HAS_SOCKET) && !defined(VMS) && !defined(WIN32) /* See perl.h. */
#include <sys/socket.h>
#endif
#ifdef I_SYS_STAT
#include <sys/stat.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#ifdef I_SYS_TIMES
#include <sys/times.h>
#endif
#ifdef I_SYS_WAIT
#include <sys/wait.h>
#endif
/* The gcc -ansi can cause a lot of noise in Solaris because of:
 /usr/include/sys/resource.h:148: warning: 'struct rlimit64' declared inside parameter list
 */
int main(int argc, char *argv[]) {

/* Add here test code found to be problematic in some gcc platform. */

/* Off_t/off_t is a struct in Solaris with largefiles, and with gcc -ansi
 * that struct cannot be compared in some gcc releases with a flat
 * integer, such as a STRLEN. */

  IV iv;
  Off_t t0a = 2;
  STRLEN t0b = 3;
  int t0c = (STRLEN)t0a == t0b;

  printf("%s: %d\n", argv[0], argc);

/* In FreeBSD 6.2 (and probably other releases too), with -Duse64bitint,
   perl will use atoll(3).  However, that declaration is hidden in <stdlib.h>
   if we force the compiler to use -std=c89 mode.
*/
  iv = Atol("42");

  return (!t0c && (iv == 42)) ? 0 : -1; /* Try to avoid 'unused' warnings. */
}
__EOT__

stdflags=''

# Further gcc warning options.  Build up a list of options that work.
# Note that some problems may only show up with combinations of options,
# e.g. a warning might show up only with -Wall -ansi, not with either
# one individually.
# TODO:  Ponder whether to migrate this back to Configure so hints files can
# tweak it.  Also, be paranoid about whether results we've deduced in Configure
# will still be valid if we now add flags like -std=c99.

pedantic=''
case "$gccansipedantic" in
define) pedantic='-pedantic' ;;
esac

case "$gccversion" in
'') ;;
[12].*) ;; # gcc versions 1 (gasp!) and 2 are not good for this.
Intel*) ;; # # Is that you, Intel C++?
#
# These comments are adapted from the originals, which were for -std=c89.
# I believe that my updates close to correct, and better than throwing the
# entire comments away, but please check for discrepencies.
#
# NOTE 1: the -std=c99 without -pedantic is a bit pointless.
# Just -std=c99 means "if there is room for interpretation,
# interpret the C99 way."  It does NOT mean "strict C99" on its own.
# You need to add the -pedantic for that.  To do this with Configure,
# do -Dgccansipedantic (note that this is named from the time when we also
# added the -ansi option. That forces -std=c89, so we no longer use it.)
# *Because* we aren't adding -std=c99 if we don't have to, but will add -W,
# some versions of gcc will accept C99 code but warn about not-C89 features.
# (If we added -std=c99 then the warnings enabled by -W would be consistent)
# Hence we add -Wno-long-long and -Wno-declaration-after-statement to cover
# these cases.
#
# NOTE 2: -pedantic necessitates adding a couple of flags:
# * -PERL_GCC_PEDANTIC so that the perl code can adapt: there's nothing
#   added by gcc itself to indicate pedanticness.
# * -Wno-overlength-strings under -DDEBUGGING because quite many of
#   the LEAVE_with_name() and assert() calls generate string literals
#   longer then the ANSI C99 minimum of 4095 bytes.
#
# NOTE 3: the relative order of these options matters:
# -Wextra before -W
# -W before -Wno-long-long -Wno-declaration-after-statement
#
*) warns="$pedantic \
	-Werror=pointer-arith \
	-Werror=vla \
	-Wextra -W \
	-Wno-long-long -Wno-declaration-after-statement \
	-Wc++-compat -Wwrite-strings"
   case " $ccflags " in
   *" -std="*) ;; # Already have -std=...
   *) warns="-std=c99 $warns" ;;
   esac
   for opt in $warns
    do
       case " $ccflags " in
       *" $opt "*) ;; # Skip if already there.
       *) rm -f _cflags$_exe
	  flags="-DPERL_NO_INLINE_FUNCTIONS $ccflags $warn $stdflags $opt"
	  case "$opt" in
	  *-pedantic*) flags="$flags -DPERL_GCC_PEDANTIC" ;;
	  esac
          # echo "opt = $opt, flags = $flags"
          cmd="$cc $flags _cflags.c -o _cflags$_exe"
          out="`$cmd 2>&1`"
          # echo "$cmd --> $out"
          case "$out" in
          *"unrecognized"*) ;;
          *"unknown"*) ;;
          *"implicit declaration"*) ;; # Was something useful hidden?
          *"Invalid"*) ;;
          *"is valid for C"*) ;;
          *) if test -x _cflags$_exe
             then
               case "$opt" in
               -std*)
                 echo "cflags.SH: Adding $opt."
                 stdflags="$stdflags $opt"
                 ;;
               -W)
                 # -Wextra is the modern form of -W, so add
                 # -W only if -Wextra is not there already.
                 case " $warn " in
                 *-Wextra*) ;;
                 *)
                   echo "cflags.SH: Adding $opt."
                   warn="$warn $opt"
                   ;;
                 esac
                 ;;
               -Werror=pointer-arith)
                  # -pedantic* covers -Werror=p-a
                  case "$warn" in
                  *-pedantic*) ;;
                  *)
                     echo "cflags.SH: Adding $opt."
                     warn="$warn $opt"
                     ;;
                  esac
                  ;;
               *)
                  echo "cflags.SH: Adding $opt."
                  warn="$warn $opt"
                  ;;
               esac
             fi
             ;;
          esac
          ;;
       esac
       case "$ccflags$warn" in
       *-pedantic*)
         overlength=''
         case "$ccflags$optimize" in
         *-DDEBUGGING*) overlength='-Wno-overlength-strings' ;;
         esac
         for opt2 in -DPERL_GCC_PEDANTIC $overlength
         do
	   case "$ccflags$warn" in
	   *"$opt2"*) ;;
	   *) echo "cflags.SH: Adding $opt2 because of -pedantic."
	      warn="$warn $opt2" ;;
           esac
         done
         ;;
       esac
    done
    ;;
esac
rm -f _cflags.c _cflags$_exe

case "$gccversion" in
'') ;;
*)
  case "$warn$ccflags" in
  *-pedantic*)
    # For -std=c99 -pedantic, only the %Ld format seems to be warn-worthy.
    # 'long long' and '%lld' are now kosher.
    #
    # usedtrace (DTrace) uses unportable features (dollars in identifiers,
    # and gcc statement expressions), it is just easier to turn off pedantic.
    remove=''
    case "$quadtype:$ivtype:$sPRId64:$usedtrace" in
    **Ld*) remove='Ld' ;;
    *) case "$usedtrace" in
       define) remove='usedtrace' ;;
       esac
       ;;
    esac
    case "$remove" in
    '') ;;
    *) echo "cflags.SH: Removing -pedantic* -ansi because of $remove."
      ccflags=`echo $ccflags|sed -e 's/-pedantic-errors/ /' -e 's/-pedantic/ /'`
      warn=`echo $warn|sed -e 's/-pedantic-errors/ /' -e 's/-pedantic/ /'`
      ;;
    esac
    ;;
  esac
  ;;
esac

# Older clang releases are not wise enough for -Wunused-value.
case "$gccversion" in
*"Apple LLVM "[34]*|*"Apple LLVM version "[34]*)
  for f in -Wno-unused-value
  do
    echo "cflags.SH: Adding $f because clang version '$gccversion'" 
    warn="$warn $f"
  done
  ;;
esac

# The quadmath Q format specifier will cause -Wformat to whine.
case "$gccversion" in
'') ;;
*) case "$usequadmath" in
   define)
     for f in -Wno-format
     do
       echo "cflags.SH: Adding $f because of usequadmath."
       warn="$warn $f"
     done
    ;;
  esac
  ;;
esac

case "$cc" in
*g++*)
  # Extra paranoia in case people have bad canned ccflags:
  # bad in the sense that the flags are accepted by g++,
  # but then whined about.
  for f in -Wc++-compat -std=c99
  do
    case "$ccflags$warn" in
    *"$f"*)
      echo "cflags.SH: Removing $f because of g++."
      ccflags=`echo $ccflags|sed 's/$f/ /'`
      warn=`echo $warn|sed 's/$f/ /'`
      ;;
    esac
  done
  ;;
esac

for f in -Wpointer-arith -Werror=pointer-arith
do
  case "$cppflags" in
  *"$f"*)
    echo "cflags.SH: Removing $f from cppflags."
    cppflags=`echo $cppflags|sed 's/$f/ /'` ;;
  esac
done

# If usethreads and clang, add -Wthread-safety for clang 3.6 or later.
# gccversion is defined also for clang, because compat, use that for matching.
# Apple overwrites clang version with XCode version, see hints/darwin.sh
# for the gory details.  Aggressively forward-proofing.
case "$usethreads" in
define)
case "$gccversion" in
*" Clang 3."[56789]*|*" Clang "[456]*|*"Apple LLVM 6.1"*|*"Apple LLVM "[789]*)
  for f in -Wthread-safety
  do
    case " $warn " in
    *" $f "*) ;; # Skip if already there.
    *)
      echo "cflags.SH: Adding $f because usethreads and clang and gccversion '$gccversion'"
      warn="$warn $f"
      ;;
    esac
  done
;;
esac
;;
esac

# gcc version 12 and 13 are overly aggressive with use-after-free warnings
# and have false positives on code that shouldn't warn, and they haven't
# sorted it out yet. See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108115
case "$gccversion" in
"1"[23]*)
    for f in -Wno-use-after-free
    do
        echo "cflags.SH: Adding $f because of false positives in gccversion '$gccversion'"
        warn="$warn $f"
    done
;;
esac

echo "cflags.SH: cc       = $cc"
echo "cflags.SH: ccflags  = $ccflags"
echo "cflags.SH: stdflags = $stdflags"
echo "cflags.SH: optimize = $optimize"
echo "cflags.SH: warn     = $warn"

# Code to set any extra flags here.
extra=''

# Protect double or single quotes for better restoring of ccflags.
myccflags=`echo $ccflags | sed -e 's/"/\\\"/g' -e "s/'/\\\'/g"`

echo "Extracting cflags (with variable substitutions)"
# This section of the file will have variable substitutions done on it.
# Move anything that needs config subs from !NO!SUBS! section to !GROK!THIS!.
# Protect any dollar signs and backticks that you do not want interpreted
# by putting a backslash in front.  You may delete these comments.
rm -f cflags
$spitshell >cflags <<!GROK!THIS!
$startsh

# !!!!!!!   DO NOT EDIT THIS FILE   !!!!!!!

# This file is generated by cflags.SH

# Used to restore possible edits by cflags.SH.
myccflags="$myccflags"

# Extra warnings, used e.g. for gcc.
warn="$warn"
# Extra standardness.
stdflags="$stdflags"
# Extra extra.
extra="$extra"
# what do executables look like?
_exe="$_exe"

!GROK!THIS!

# In the following dollars and backticks do not need the extra backslash.
$spitshell >>cflags <<'!NO!SUBS!'
case $PERL_CONFIG_SH in
'')
	if test -f config.sh; then TOP=.;
	elif test -f ../config.sh; then TOP=..;
	elif test -f ../../config.sh; then TOP=../..;
	elif test -f ../../../config.sh; then TOP=../../..;
	elif test -f ../../../../config.sh; then TOP=../../../..;
	else
		echo "Can't find config.sh."; exit 1
	fi
	. $TOP/config.sh
        ccflags="$myccflags"  # Restore possible edits by cflags.SH.
	;;
esac

# syntax: cflags [optimize=XXX] [file[.suffix]] ...
#   displays the proposed compiler command line for each 'file'
#
#   with no file, dispalys it for all *.c files.
#   The optimise=XXX arg (if present) is evalled, setting the default
#   value of the $optimise variable, which is output on the command line
#   (but which may be overridden for specific files below)

case "X$1" in
Xoptimize=*|X"optimize=*")
	eval "$1"
	shift
	;;
esac

case $# in
0) set *.c; echo "The current C flags are:" ;;
esac

set `echo "$* " | sed -e 's/\.[oc] / /g' -e 's/\.obj / /g' -e "s/\\$obj_ext / /g"`

for file do

    case "$#" in
    1) ;;
    *) echo $n "    $file.c	$c" ;;
    esac

    # allow variables like toke_cflags to be evaluated

    case "$file" in
    */*) ;;
    *) eval 'eval ${'"${file}_cflags"'-""}' ;;
    esac

    # or customize here

    case "$file" in
    regcomp) : work around http://bugs.debian.org/754054
        case $archname in
        mips-*|mipsel-*)
            optimize="$optimize -fno-tree-vrp";;
        esac;;
    *) ;;

    # Customization examples follow.
    #
    # The examples are intentionally unreachable as the '*)' case above always
    # matches. To use them, move before the '*)' and edit as appropriate.
    # It is not a good idea to set ccflags to an absolute value here, as it
    # often contains general -D defines which are needed for correct
    # compilation. It is better to edit ccflags as shown, using interpolation
    # to add flags, or sed to remove flags.

    av) ccflags=`echo $ccflags | sed -e s/-pipe//` ;;
    deb) ccflags="$ccflags -fno-jump-tables" ;;
    hv) warn=`echo $warn | sed -e s/-Wextra//` ;;
    toke) optimize=-O0 ;;
    esac

    echo "$cc -c -DPERL_CORE $ccflags $stdflags $optimize $warn $extra"

    . $TOP/config.sh

    # end per file behaviour
done
!NO!SUBS!
chmod 755 cflags
$eunicefix cflags
