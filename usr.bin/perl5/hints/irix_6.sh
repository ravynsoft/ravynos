# hints/irix_6.sh
#
# original from Krishna Sethuraman, krishna@sgi.com
#
# Modified Mon Jul 22 14:52:25 EDT 1996
# 	Andy Dougherty <doughera@lafayette.edu>
# 	with help from Dean Roehrich <roehrich@cray.com>.
#   cc -n32 update info from Krishna Sethuraman, krishna@sgi.com.
#       additional update from Scott Henry, scotth@sgi.com

# Futzed with by John Stoffel <jfs@fluent.com> on 4/24/1997
#    - assumes 'cc -n32' by default
#    - tries to check for various compiler versions and do the right 
#      thing when it can
#    - warnings turned off (-n32 messages):
#       1184 - "=" is used where where "==" may have been intended
#       1552 - variable "foo" set but never used

# Tweaked by Chip Salzenberg <chip@perl.com> on 5/13/97
#    - don't assume 'cc -n32' if the n32 libm.so is missing

# Threaded by Jarkko Hietaniemi <jhi@iki.fi> on 11/18/97
#    - POSIX threads knowledge by IRIX version

# gcc-enabled by Kurt Starsinic <kstar@isinet.com> on 3/24/1998

# 64-bitty by Jarkko Hietaniemi on 9/1998

# Martin Pool added -shared for gcc on 2004-01-27

# Use   sh Configure -Dcc='cc -n32' to try compiling with -n32.
#     or -Dcc='cc -n32 -mips3' (or -mips4) to force (non)portability
# Don't bother with -n32 unless you have the 7.1 or later compilers.
#     But there's no quick and light-weight way to check in 6.2.

# NOTE: some IRIX cc versions, e.g. 7.3.1.1m (try cc -version) have
# been known to have issues (coredumps) when compiling perl.c.
# If you've used -OPT:fast_io=ON and this happens, try removing it.
# If that fails, or you didn't use that, then try adjusting other
# optimization options (-LNO, -INLINE, -O3 to -O2, etcetera).
# The compiler bug has been reported to SGI.
# -- Allen Smith <allens@cpan.org>

# Modified (10/30/04) to turn off usemallocwrap (PERL_MALLOC_WRAP) in -n32
# mode - Allen.

case "$use64bitall" in
$define|true|[yY]*)
    case "`uname -s`" in
       IRIX)
           cat <<END >&2
You have asked for use64bitall but you aren't running on 64-bit IRIX.
I'll try changing it to use64bitint.
END
       use64bitall="$undef"

       case "`uname -r`" in
           [1-5]*|6.[01])
               cat <<END >&2
Sorry, can't do use64bitint either. Try upgrading to IRIX 6.2 or later.
END
               use64bitint="$undef"
           ;;
           *) use64bitint="$define"
           ;;
       esac
       ;;
    esac
    ;;
esac

# Until we figure out what to be probed for in Configure (ditto for hpux.sh)
case "$usemorebits" in # Need to expand this now, then.
$define|true|[yY]*)
    case "`uname -r`" in
           [1-5]*|6.[01])
               uselongdouble="$define"
               ;;
           *) use64bitint="$define" uselongdouble="$define" ;;
    esac
esac

# Let's assume we want to use 'cc -n32' by default, unless the
# necessary libm is missing (which has happened at least twice)
case "$cc" in
'') case "$use64bitall" in
    "$define"|true|[yY]*) test -f /usr/lib64/libm.so && cc='cc -64' ;;
    *) test -f /usr/lib32/libm.so && cc='cc -n32' ;;
    esac    	
esac

case "$use64bitint" in
    "$define"|true|[yY]*) ;;
    *)  d_casti32="$undef" ;;
esac

cc=${cc:-cc}
cat=${cat:-cat}

$cat > UU/cc.cbu <<'EOCCBU'
# This script UU/cc.cbu will get 'called-back' by Configure after it
# has prompted the user for the C compiler to use.

case "$cc" in
*gcc*)
  # With cc we can use -c99, but with gcc we just can't use C99 headers.
  # (There is a hidden define __c99 that cc uses, but trying to use that
  # with gcc leads into magnificent explosions.)
  i_stdint='undef'
  ;;
*) ccversion=`cc -version 2>&1` ;;
esac

# Check for which compiler we're using

case "$cc" in
*"cc -n32"*)
    test -z "$ldlibpthname" && ldlibpthname='LD_LIBRARYN32_PATH'

	# If a library is requested to link against, make sure the
	# objects in the library are of the same ABI we are compiling
	# against. Albert Chin-A-Young <china@thewrittenword.com>

       # In other words, you no longer have to worry regarding having old
       # library paths (/usr/lib) in the searchpath for -n32 or -64; thank
       # you very much, Albert! Now if we could just get more module authors
       # to use something like this... - Allen

	libscheck='case "$xxx" in
*.a) /bin/ar p $xxx `/bin/ar t $xxx | sed q` >$$.o;
  case "`/usr/bin/file $$.o`" in
  *N32*) rm -f $$.o ;;
  *) rm -f $$.o; xxx=/no/n32$xxx ;;
  esac ;;
*) case "`/usr/bin/file $xxx`" in
  *N32*) ;;
  *) xxx=/no/n32$xxx ;;
  esac ;;
esac'

	# NOTE: -L/usr/lib32 -L/lib32 are automatically selected by the linker
       test -z "$ldflags" && ldflags=' -L/usr/local/lib32 -L/usr/local/lib'
	cccdlflags=' '
    # From: David Billinghurst <David.Billinghurst@riotinto.com.au>
    # If you get complaints about so_locations then change the following
    # line to something like:
    #	lddlflags="-n32 -shared -check_registry /usr/lib32/so_locations"
       test -z "$lddlflags" && lddlflags="-n32 -shared"
       test -z "$libc" && libc='/usr/lib32/libc.so'
       test -z "$plibpth" && plibpth='/usr/lib32 /lib32 /usr/ccs/lib'

       # PERL_MALLOC_WRAP gives false alarms ("panic: memory wrap") in IRIX
       # -n32 mode, resulting in perl compiles never getting further than
       # miniperl. I am not sure whether it actually does any good in -32 or
       # -64 mode, especially the latter, but it does not give false
       # alarms (in testing). -Allen

       usemallocwrap=${usemallocwrap:-false}
       ;;
*"cc -64"*)
    case "`uname -s`" in
    IRIX)
	$cat >&4 <<EOM
You cannot use cc -64 or -Duse64bitall in 32-bit IRIX, sorry.
Cannot continue, aborting.
EOM
       exit 1
       ;;
    esac
       test -z "$ldlibpthname" && ldlibpthname='LD_LIBRARY64_PATH'
       test -z "$use64bitall" && use64bitall="$define"
       test -z "$use64bitint" && use64bitint="$define"
	loclibpth="$loclibpth /usr/lib64"
	libscheck='case "`/usr/bin/file $xxx`" in
*64-bit*) ;;
*) xxx=/no/64-bit$xxx ;;
esac'
	# NOTE: -L/usr/lib64 -L/lib64 are automatically selected by the linker
       test -z "$ldflags" && ldflags=' -L/usr/local/lib64 -L/usr/local/lib'
	cccdlflags=' '
       test -z "$archname64" && archname64='64all'
    # From: David Billinghurst <David.Billinghurst@riotinto.com.au>
    # If you get complaints about so_locations then change the following
    # line to something like:
    #	lddlflags="-64 -shared -check_registry /usr/lib64/so_locations"
       test -z lddlflags="-64 -shared"
       test -z "$libc" && libc='/usr/lib64/libc.so'
       test -z "$plibpth" && plibpth='/usr/lib64 /lib64 /usr/ccs/lib'
	;;
*gcc*)
	ccflags="$ccflags -D_BSD_TYPES -D_BSD_TIME"
       test -z "$optimize" && optimize="-O3"
	usenm='undef'
	# It seems gcc can build Irix shlibs, but of course it needs
	# -shared.  Otherwise you get link errors looking for main().
	lddlflags="$lddlflags -shared"
	case "`uname -s`" in
	# Without the -mabi=64 gcc in 64-bit IRIX has problems passing
	# and returning small structures.  This affects inet_*() and semctl().
	# See http://groups.google.com/group/comp.sys.sgi.admin/msg/3ad8353bc4ce3cb0
	# for more information.  Reported by Lionel Cons <lionel.cons@cern.ch>.
	IRIX64)	ccflags="$ccflags -mabi=64"
		ldflags="$ldflags -mabi=64 -L/usr/lib64"
		lddlflags="$lddlflags -mabi=64"
		;;
	*)	ccflags="$ccflags -DIRIX32_SEMUN_BROKEN_BY_GCC"
                # XXX Note: It is possible that turning off usemallocwrap is
                # needed here; insufficient data! - Allen
		;;
	esac
	;;
*)
	# this is needed to force the old-32 paths
	#  since the system default can be changed.
	ccflags="$ccflags -32 -D_BSD_TYPES -D_BSD_TIME -Olimit 3100"
	optimize='-O'	  
	;;
esac

# Settings common to both native compiler modes.
case "$cc" in
*"cc -n32"*|*"cc -64"*)
       test -z "$ld" && ld=$cc

	# perl's malloc can return improperly aligned buffer
	# which (under 5.6.0RC1) leads into really bizarre bus errors
	# and freak test failures (lib/safe1 #18, for example),
	# even more so with -Duse64bitall: for example lib/io_linenumtb.
	# fails under the harness but succeeds when run separately,
	# under make test pragma/warnings #98 fails, and lib/io_dir
	# apparently coredumps (the last two don't happen under
    	# the harness.  Helmut Jarausch is seeing bus errors from
        # miniperl, as was Scott Henry with snapshots from just before
	# the RC1. --jhi
	usemymalloc='undef'

       # Was at the first of the line - Allen
       #malloc_cflags='ccflags="-DSTRICT_ALIGNMENT $ccflags"'

       nm_opt="$nm_opt -p"
       nm_so_opt="$nm_so_opt -p"

	# Warnings to turn off because the source code hasn't
	# been cleaned up enough yet to satisfy the IRIX cc.
	# 1047: macro redefinitions (in IRIX' own system headers!)
	# 1184: "=" is used where where "==" may have been intended.
	# 1552: The variable "foobar" is set but never used.
	woff=1184,1552

	# Perl 5.004_57 introduced new qsort code into pp_ctl.c that
	# makes IRIX  cc prior to 7.2.1 to emit bad code.
	# so some serious hackery follows to set pp_ctl flags correctly.

	# Check for which version of the compiler we're running
	case "`$cc -version 2>&1`" in
	*7.0*)                        # Mongoose 7.0
	     ccflags="$ccflags -D_BSD_TYPES -D_BSD_TIME -woff $woff -OPT:Olimit=0"
	     optimize='none'
	     ;;
	*7.1*|*7.2|*7.20)             # Mongoose 7.1+
            ccflags="$ccflags -D_BSD_TYPES -D_BSD_TIME -woff $woff"
            case "$optimize" in
               '') optimize='-O3 -OPT:Olimit=0' ;;
               '-O') optimize='-O3 -OPT:Olimit=0' ;;
               *) ;;
            esac

           # This is a temporary fix for 5.005+.
           # See hints/README.hints, especially the section
           # =head2 Propagating variables to config.sh

           # Note the part about case statements not working without
           # weirdness like the below echo statement... and, since
           # we're in a callback unit, it's to config.sh, not UU/config.sh
           # - Allen


           pp_ctl_cflags="$pp_ctl_flags optimize=\"$optimize -O1\""
           echo "pp_ctl_cflags=\"$pp_ctl_flags optimize=\\\"\$optimize -O1\\\"\"" >> config.sh
	     ;;



# XXX What is space=ON doing in here? Could someone ask Scott Henry? - Allen

	*7.*)                         # Mongoose 7.2.1+
            ccflags="$ccflags -D_BSD_TYPES -D_BSD_TIME -woff $woff"
            case "$optimize" in
               '') optimize='-O3 -OPT:Olimit=0:space=ON' ;;
               '-O') optimize='-O3 -OPT:Olimit=0:space=ON' ;;
               *) ;;
            esac
	    # Perl source has just grown too chummy with c99
	    # (headerwise, not code-wise: we use <stdint.h> and such)
	    ccflags="$ccflags -c99"
	     ;;
	*6.2*)                        # Ragnarok 6.2
	     ccflags="$ccflags -D_BSD_TYPES -D_BSD_TIME -woff $woff"
	     optimize='none'
	     ;;
	*)                            # Be safe and not optimize
	     ccflags="$ccflags -D_BSD_TYPES -D_BSD_TIME -woff $woff"
	     optimize='none'
	     ;;
	esac

# this is to accommodate the 'modules' capability of the 
# 7.2 MIPSPro compilers, which allows for the compilers to be installed
# in a nondefault location.  Almost everything works as expected, but
# /usr/include isn't caught properly.  Hence see the /usr/include/pthread.h
# change below to include TOOLROOT (a modules environment variable),
# and the following code.  Additional
# code to accommodate the 'modules' environment should probably be added
# here if possible, or be inserted as a ${TOOLROOT} reference before
# absolute paths (again, see the pthread.h change below). 
# -- krishna@sgi.com, 8/23/98

	if [ "X${TOOLROOT}" != "X" ]; then
	# we cant set cppflags because it gets overwritten
	# we dont actually need $TOOLROOT/usr/include on the cc line cuz the 
	# modules functionality already includes it but
	# XXX - how do I change cppflags in the hints file?
		ccflags="$ccflags -I${TOOLROOT}/usr/include"
	usrinc="${TOOLROOT}/usr/include"
        fi

	;;
esac

# workaround for an optimizer bug
# Made to work via UU/config.sh thing (or, rather, config.sh, since we're in
# a callback) from README.hints, plus further stuff; doesn't handle -g still,
# unfortunately - Allen
case "`$cc -version 2>&1`" in
*7.2.*)
    test -z "$op_cflags" && echo "op_cflags=\"optimize=\\\"\$optimize -O1\\\"\"" >> config.sh
    test -z "$op_cflags" && op_cflags="optimize=\"\$optimize -O1\""
    test -z "$opmini_cflags" && echo "opmini_cflags=\"optimize=\\\"\$optimize -O1\\\"\"" >> config.sh
    test -z "$opmini_cflags" && opmini_cflags="optimize=\"\$optimize -O1\""
    ;;
*7.3.1.*)
    test -z "$op_cflags" && echo "op_cflags=\"optimize=\\\"\$optimize -O2\\\"\"" >> config.sh
    test -z "$op_cflags" && op_cflags="$op_cflags optimize=\"\$optimize -O2\""
    test -z "$opmini_cflags" && echo "opmini_cflags=\"optimize=\\\"\$optimize -O2\\\"\"" >> config.sh
    test -z "$opmini_cflags" && opmini_cflags="optimize=\"\$optimize -O2\""
    ;;
esac


# Workaround [perl #33849]: perl 5.8.6 fails to build on IRIX 6.5 due to
# bizarre preprocessor bug:  cc -E - unfortunately goes into K&R mode, but
# cc -E file.c doesn't.  Force a wrapper to always get the ANSI mode.
# (We only need to do this for cc, not for gcc.  ccversion is computed above.)
case "$ccversion" in
'')  ;; # gcc.  Do nothing.
*)  # Inside this call-back unit, we are down in the UU/ subdirectory,
    # but Configure will look for cppstdin one level up.
    cd ..; cppstdin=`pwd`/cppstdin; cd UU
    cpprun="$cppstdin"
    ;;
esac

# There is a devious bug in the MIPSpro 7.4 compiler:
# memcmp() is an inlined intrinsic, and "sometimes" it gets compiled wrong.
#
# In Perl the most obvious hit is regcomp.c:S_regpposixcc(),
# causing bus errors when compiling the POSIX character classes like
# /[[:digit:]], which means that miniperl cannot build perl.
# (That is almost only the one victim: one single test in re/pat fails, also.)
#
# Therefore let's turn the inline intrinsics off and let the normal
# libc versions be used instead. This may cause a performance hit
# but a little slower is better than zero speed.
#
# MIPSpro C 7.4.1m is supposed to have fixed this bug.
#
case "$ccversion" in
"MIPSpro Compilers: Version 7.4")
  ccflags="$ccflags -U__INLINE_INTRINSICS"
  ;;
esac

EOCCBU

# End of cc.cbu callback unit. - Allen

# We don't want these libraries.
# Socket networking is in libc, these are not installed by default,
# and just slow perl down. (scotth@sgi.com)
# librt contains nothing we need (some places need it for Time::HiRes) --jhi
set `echo X "$libswanted "|sed -e 's/ socket / /' -e 's/ nsl / /' -e 's/ dl / /' -e 's/ rt / /'`
shift
libswanted="$*"

# I have conflicting reports about the sun, crypt, bsd, and PW
# libraries on Irix 6.2.
#
# One user reports:
# Don't need sun crypt bsd PW under 6.2.  You *may* need to link
# with these if you want to run perl built under 6.2 on a 5.3 machine
# (I haven't checked)
#
# Another user reported that if he included those libraries, a large number
# of the tests failed (approx. 20-25) and he would get a core dump. To
# make things worse, test results were inconsistent, i.e., some of the
# tests would pass some times and fail at other times.
# The safest thing to do seems to be to eliminate them.
#
#  Actually, the only libs that you want are '-lm'.  Everything else
# you need is in libc.  You do also need '-lbsd' if you choose not
# to use the -D_BSD_* defines.  Note that as of 6.2 the only
# difference between '-lmalloc' and '-lc' malloc is the debugging
# and control calls, which aren't used by perl. -- scotth@sgi.com

set `echo X "$libswanted "|sed -e 's/ sun / /' -e 's/ crypt / /' -e 's/ bsd / /' -e 's/ PW / /' -e 's/ malloc / /'`
shift
libswanted="$*"

# libbind.{so|a} would be from a BIND/named installation - IRIX 6.5.* has
# pretty much everything that would be useful in libbind in libc, including
# accessing a local caching server (nsd) that will also look in /etc/hosts,
# NIS (yuck!), etcetera. libbind also doesn't have the _r (thread-safe
# reentrant) functions.
# - Allen <easmith@beatrice.rutgers.edu>

case "`uname -r`" in
6.5)
    set `echo X "$libswanted "|sed -e 's/ bind / /'`
    shift
    libswanted="$*"
    ;;
esac

# Don't groan about unused libraries.
case "$ldflags" in
    *-Wl,-woff,84*) ;;
    *) ldflags="$ldflags -Wl,-woff,84" ;;
esac

# IRIX freeware kits sometimes have only o32 libraries for gdbm.
# You can try Configure ... -Dlibswanted='m' -Dnoextensions='GDBM_File'
# since the libm seems to be pretty much the only really needed library.

# Irix 6.5.6 seems to have a broken header <sys/mode.h>
# don't include that (it doesn't contain S_IFMT, S_IFREG, et al)

i_sysmode="$undef"

$cat > UU/usethreads.cbu <<'EOCBU'
# This script UU/usethreads.cbu will get 'called-back' by Configure 
# after it has prompted the user for whether to use threads.
case "$usethreads" in
$define|true|[yY]*)
        if test ! -f ${TOOLROOT}/usr/include/pthread.h -o ! -f /usr/lib/libpthread.so; then
            case "`uname -r`" in
            [1-5].*|6.[01])
 	        cat >&4 <<EOM
IRIX `uname -r` does not support POSIX threads.
You should upgrade to at least IRIX 6.2 with pthread patches.
EOM
	        ;;
	    6.2)
 	        cat >&4 <<EOM
IRIX 6.2 can have the POSIX threads.
However, the following IRIX patches (or their replacements) MUST be installed:
        1404 Irix 6.2 Posix 1003.1b man pages
        1645 IRIX 6.2 & 6.3 POSIX header file updates
        2000 Irix 6.2 Posix 1003.1b support modules
        2254 Pthread library fixes
	2401 6.2 all platform kernel rollup
IMPORTANT:
	Without patch 2401, a kernel bug in IRIX 6.2 will
	cause your machine to panic and crash when running
	threaded perl. IRIX 6.3 and up should be OK.
EOM
	        ;;
  	    [67].*)
	        cat >&4 <<EOM
IRIX `uname -r` should have the POSIX threads.
But, somehow, you do not seem to have them installed.
EOM
	        ;;
	    esac
            cat >&4 <<EOM
Cannot continue, aborting.
EOM
            exit 1
        fi
        set `echo X "$libswanted "| sed -e 's/ c / pthread /'`
        shift
        libswanted="$*"

        usemymalloc='n'

	# These are hidden behind a _POSIX1C ifdef that would
	# require including <pthread.h> for the Configure hasproto
	# to see these.

#      d_asctime_r_proto="$define"
#      d_ctime_r_proto="$define"
#      d_gmtime_r_proto="$define"
#      d_localtime_r_proto="$define"

       # Safer just to go ahead and include it, for other ifdefs like them
       # (there are a lot, such as in netdb.h). - Allen
       ccflags="$ccflags -DPTHREAD_H_FIRST"

       pthread_h_first="$define"
       echo "pthread_h_first='define'" >> config.sh

	;;

esac
EOCBU

# The -n32 makes off_t to be 8 bytes, so we should have largefileness.

$cat > UU/use64bitint.cbu <<'EOCBU'
# This script UU/use64bitint.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use 64 bit integers.

case "$use64bitint" in
$define|true|[yY]*)
           case "`uname -r`" in
           [1-5]*|6.[01])
               cat >&4 <<EOM
IRIX `uname -r` does not support 64-bit types.
You should upgrade to at least IRIX 6.2.
Cannot continue, aborting.
EOM
               exit 1
               ;;
            esac
    usemymalloc="$undef"
    ;;
*) d_casti32="$undef" ;;
esac

EOCBU

$cat > UU/use64bitall.cbu <<'EOCBU'
# This script UU/use64bitall.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to be maximally 64 bitty.

case "$use64bitall" in
$define|true|[yY]*)
    case "$cc" in
       *-n32*|*-32*)
           cat >&4 <<EOM
You cannot use a non-64 bit cc for -Duse64bitall, sorry.
Cannot continue, aborting.
EOM
           exit 1
       ;;
    esac
    ;;
esac

EOCBU

$cat > UU/uselongdouble.cbu <<'EOCBU'
# This script UU/uselongdouble.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use long doubles.

# This script is designed to test IRIX (and other machines, once it's put into
# Configure) for a bug in which they fail to round correctly when using
# sprintf/printf/etcetera on a long double with precision specified (%.0Lf or
# whatever). Sometimes, this only happens when the number in question is
# between 1 and -1, weirdly enough. - Allen

case "$uselongdouble" in
$define|true|[yY]*)

case "$d_PRIfldbl" in
$define|true|[yY]*)

    echo " " >try.c
    $cat >>try.c <<EOP
#include <stdio.h>

#define sPRIfldbl $sPRIfldbl

#include <stdlib.h>

int main()
{ 
        char buf1[64];
 	char buf2[64];
        buf1[63] = '\0';
	buf2[63] = '\0';

	(void)sprintf(buf1,"%.0"sPRIfldbl,(long double)0.6L);
	(void)sprintf(buf2,"%.0f",(double)0.6);
	if (strcmp(buf1,buf2)) {
	    exit(1);
	}
	(void)sprintf(buf1,"%.0"sPRIfldbl,(long double)-0.6L);
	(void)sprintf(buf2,"%.0f",(double)-0.6);
	if (strcmp(buf1,buf2)) {
	    exit(1);
	} else {
	    exit(0);
	}
}

EOP

    set try
    if eval $compile && $run ./try; then
	rm -f try try.* >/dev/null
    else
	rm -f try try.* core a.out >/dev/null
	ccflags="$ccflags -DHAS_LDBL_SPRINTF_BUG"
	cppflags="$cppflags -DHAS_LDBL_SPRINTF_BUG"

        echo " " >try.c
    $cat >>try.c <<EOP
#include <stdio.h>

#define sPRIfldbl $sPRIfldbl

#include <stdlib.h>

int main()
{ 
        char buf1[64];
 	char buf2[64];
        buf1[63] = '\0';
	buf2[63] = '\0';

	(void)sprintf(buf1,"%.0"sPRIfldbl,(long double)1.6L);
	(void)sprintf(buf2,"%.0f",(double)1.6);
	if (strcmp(buf1,buf2)) {
	    exit(1);
	}
	(void)sprintf(buf1,"%.0"sPRIfldbl,(long double)-1.6L);
	(void)sprintf(buf2,"%.0f",(double)-1.6);
	if (strcmp(buf1,buf2)) {
	    exit(1);
	} else {
	    exit(0);
	}
}

EOP

	set try
	if eval $compile && $run ./try; then
	    rm -f try try.c >/dev/null
	    ccflags="$ccflags -DHAS_LDBL_SPRINTF_BUG_LESS1"
	    cppflags="$cppflags -DHAS_LDBL_SPRINTF_BUG_LESS1"
	else
	    rm -f try try.c core try.o a.out >/dev/null
	fi
    fi
;;
*) # Can't tell!
   ccflags="$ccflags -DHAS_LDBL_SPRINTF_BUG"
   cppflags="$cppflags -DHAS_LDBL_SPRINTF_BUG"
   ;;
esac

# end of case statement for how to print ldbl with 'f'
;;
*) ;;
esac

# end of case statement for whether to do long doubles

EOCBU

# Helmut Jarausch reports that Perl's malloc is rather unusable
# with IRIX, and SGI confirms the problem.
usemymalloc=${usemymalloc:-false}

# Configure finds <fcntl.h> but then thinks it can use <sys/file.h>
# instead; in IRIX this is not true because the prototype of fcntl()
# requires explicit include of <fcntl.h>
i_fcntl=define

# There is <prctl.h> but it's not the Linux one that Configure expects.
d_prctl="$undef"
