# hints/dec_osf.sh

#	* If you want to debug perl or want to send a
#	stack trace for inclusion into an bug report, call
#	Configure with the additional argument  -Doptimize=-g2
#	or uncomment this assignment to "optimize":
#
#optimize=-g2
#
#	If you want both to optimise and debug with the DEC cc
#	you must have -g3, e.g. "-O4 -g3", and (re)run Configure.
#
#	* gcc can always have both -g and optimisation on.
#
#	* debugging optimised code, no matter what compiler
#	one is using, can be surprising and confusing because of
#	the optimisation tricks like code motion, code removal,
#	loop unrolling, and inlining. The source code and the
#	executable code simply do not agree any more while in
#	mid-execution, the optimiser only cares about the results.
#
#	* Configure will automatically add the often quoted
#	-DDEBUGGING for you if the -g is specified.
#
#	* There is even more optimisation available in the new
#	(GEM) DEC cc: -O5 and -fast. "man cc" will tell more about them.
#	The jury is still out whether either or neither help for Perl
#	and how much. Based on very quick testing, -fast boosts
#	raw data copy by about 5-15% (-fast brings in, among other
#	things, inlined, ahem, fast memcpy()), while on the other
#	hand searching things (index, m//, s///), seems to get slower.
#	Your mileage will vary.
#
#	* The -std is needed because the following compiled
#	without the -std and linked with -lm
#
#	#include <math.h>
#	#include <stdio.h>
#	int main(){short x=10,y=sqrt(x);printf("%d\n",y);}
#
#	will in Digital UNIX 3.* and 4.0b print 0 -- and in Digital
#	UNIX 4.0{,a} dump core: Floating point exception in the printf(),
#	the y has become a signaling NaN.
#
#	* Compilation warnings like:
#
#	"Undefined the ANSI standard macro ..."
#
#	can be ignored, at least while compiling the POSIX extension
#	and especially if using the sfio (the latter is not a standard
#	part of Perl, never mind if it says little to you).
#

# If using the DEC compiler we must find out the DEC compiler style:
# the style changed between Digital UNIX (aka DEC OSF/1) 3 and
# Digital UNIX 4. The old compiler was originally from Ultrix and
# the MIPS company, the new compiler is originally from the VAX world
# and it is called GEM. Many of the options we are going to use depend
# on the compiler style.

cc=${cc:-cc}

# Intentional leading tabs.
	myosvers="`/usr/sbin/sizer -v 2>/dev/null || uname -r`"
	unamer="`uname -r`"

# Fancy compiler suites use optimising linker as well as compiler.
# <spider@Orb.Nashua.NH.US>
case "$unamer" in
*[123].*)	# old loader
		lddlflags="$lddlflags -O3"
		;;
*)            if $test "X$optimize" = "X$undef"; then
                      lddlflags="$lddlflags -msym"
              else
		  case "$myosvers" in
		  *4.0D*)
		      # QAR 56761: -O4 + .so may produce broken code,
		      # fixed in 4.0E or better.
		      ;;
		  *)
                      lddlflags="$lddlflags $optimize"
		      ;;
		  esac
		  # -msym: If using a sufficiently recent /sbin/loader,
		  # keep the module symbols with the modules.
                  lddlflags="$lddlflags -msym $_lddlflags_strict_ansi"
              fi
		;;
esac
# Yes, the above loses if gcc does not use the system linker.
# If that happens, let me know about it. <jhi@iki.fi>

# Because there is no other handy way to recognize 3.X.
case "$unamer" in
*3.*)	ccflags="$ccflags -DDEC_OSF1_3_X" ;;
esac

case "`$cc -v 2>&1 | grep cc`" in
*gcc*) isgcc=gcc ;;
esac

# do NOT, I repeat, *NOT* take away the leading tabs
# Configure Black Magic (TM)
	# reset
	_DEC_cc_style=
case "$isgcc" in
gcc)	if [ "X$gccversion" = "X" ]; then
	    # Done too late in Configure if hinted
	    gccversion=`$cc -dumpversion`
	fi
	set $gccversion
	if test "$1" -lt 2 -o \( "$1" -eq 2 -a \( "$2" -lt 95 -o \( "$2" -eq 95 -a "$3" -lt 3 \) \) \); then
	    cat >&4 <<EOF

*** Your cc seems to be gcc and its version ($gccversion) seems to be
*** less than 2.95.3.  This is not a good idea since old versions of gcc
*** are known to produce buggy code when compiling Perl (and no doubt for
*** other programs, too).
***
*** Therefore, I strongly suggest upgrading your gcc.  (Why don't you use
*** the vendor cc is also a good question.  It comes with the operating
*** system, produces good code, and is very ANSI C fastidious.)

Cannot continue, aborting.

EOF
	    exit 1
	fi
	if test "$1" -eq 2 -a "$2" -eq 95 -a "$3" -le 2; then
	    cat >&4 <<EOF

*** Note that as of gcc 2.95.2 (19991024) and Perl 5.6.0 (March 2000)
*** if the said Perl is compiled with the said gcc the lib/sdbm test
*** may dump core (meaning that the SDBM_File extension is unusable).
*** As this core dump never happens with the vendor cc, this is most
*** probably a lingering bug in gcc.  Therefore unless you have a better
*** gcc installation you are still better off using the vendor cc.

Since you explicitly chose gcc, I assume that you know what are doing.

EOF
	fi
	# -ansi is fine for gcc in Tru64 (-ansi is not universally so).
	_ccflags_strict_ansi="-ansi"
        ;;
*)	# compile something.
	cat >try.c <<EOF
int main() { return 0; }
EOF
	ccversion=`cc -V | awk '/(Compaq|DEC) C/ {print $3}' | grep '^V'`
	# the main point is the '-v' flag of 'cc'.
	case "`cc -v -c try.c 2>&1`" in
	*/gemc_cc*)	# we have the new DEC GEM CC
			_DEC_cc_style=new
			;;
	*)		# we have the old MIPS CC
			_DEC_cc_style=old
			;;
	esac
	# We need to figure out whether -c99 is a valid flag to use.
	# If it is, we can use it for being nauseatingly C99 ANSI --
	# but even then the lddlflags needs to stay -std1.
	# If it is not, we must use -std1 for both flags.
	#
	case "`cc -c99 try.c 2>&1`" in
	*"-c99: Unknown flag"*)
		_ccflags_strict_ansi="-std1"
		;;
	*)	_ccflags_strict_ansi="-c99"
		;;
	esac
	_lddlflags_strict_ansi="-std1"
	# -no_ansi_alias because Perl code is not that strict
	# (also gcc uses by default -fno-strict-aliasing).
	case "$unamer" in
	*[1234].*) ;;
	*5.*)	_ccflags_strict_ansi="$_ccflags_strict_ansi -no_ansi_alias" ;;
	esac
	# Cleanup.
	rm -f try.c try.o
	;;
esac

# Be nauseatingly ANSI
ccflags="$ccflags $_ccflags_strict_ansi"

# g++ needs a lot of definitions to see the same set of
# prototypes from <unistd.h> et alia as cxx/cc see.
# Note that we cannot define _XOPEN_SOURCE_EXTENDED or
# its moral equivalent, _XOPEN_SOURCE=500 (which would
# define a lot of the required prototypes for us), because
# the gcc-processed version of <sys/wait.h> contains fatally
# conflicting prototypes for wait3().  The _SOCKADDR_LEN is
# needed to get struct sockaddr and struct sockaddr_in to align.
case "$cc" in
*g++*) ccflags="$ccflags -D_XOPEN_SOURCE -D_OSF_SOURCE -D_AES_SOURCE -D_BSD -D_POSIX_C_SOURCE=199309L -D_POSIX_PII_SOCKET -D_SOCKADDR_LEN" ;;
esac

# for gcc the Configure knows about the -fpic:
# position-independent code for dynamic loading

# we want optimisation

case "$optimize" in
'')	case "$isgcc" in
	gcc)	optimize='-O3'				;;
	*)	case "$_DEC_cc_style" in
		new)	optimize='-O4'			;;
		old)	optimize='-O2 -Olimit 3200'	;;
		esac
		ccflags="$ccflags -D_INTRINSICS"
		;;
	esac
	;;
esac

case "$isgcc" in
gcc)	;;
*)	case "$optimize" in
	*-O*)	# With both -O and -g, the -g must be -g3.
		optimize="`echo $optimize | sed 's/-g[1-4]*/-g3/'`"
		;;
	esac
	;;
esac

## Optimization limits
case "$isgcc" in
gcc) #  gcc 3.2.1 wants a lot of memory for -O3'ing toke.c
cat >try.c <<EOF
#include <stdio.h>
#include <sys/resource.h>

int main ()
{
    struct rlimit rl;
    int i = getrlimit (RLIMIT_DATA, &rl);
    printf ("%d\n", rl.rlim_cur / (1024 * 1024));
    } /* main */
EOF
$cc -o try $ccflags $ldflags try.c
	maxdsiz=`./try`
rm -f try try.c core
if [ $maxdsiz -lt 256 ]; then
    # less than 256 MB is probably not enough to optimize toke.c with gcc -O3
    cat <<EOM >&4

Your process datasize is limited to $maxdsiz MB, which is (sadly) not
always enough to fully optimize some source code files of Perl,
at least 256 MB seems to be necessary as of Perl 5.8.0.  I'll try to
use a lower optimization level for those parts.  You could either try
using your shell's ulimit/limit/limits command to raise your datasize
(assuming the system-wide hard resource limits allow you to go higher),
or if you can't go higher and if you are a sysadmin, and you *do* want
the full optimization, you can tune the 'max_per_proc_data_size'
kernel parameter: see man sysconfigtab, and man sys_attrs_proc.

EOM
toke_cflags='optimize=-O2'
    fi
;;
esac

# The patch 23787
# https://github.com/Perl/perl5/commit/73cb726371990cd489597c4fee405a9815abf4da
# broke things for gcc (at least gcc 3.3) so that many of the pack()
# checksum tests for formats L, j, J, especially when combined
# with the < and > specifiers, started to fail if compiled with plain -O3.
case "$isgcc" in
gcc)
pp_pack_cflags='optimize="-O3 -fno-cse-skip-blocks"'
;;
esac

# we want dynamic fp rounding mode, and we want ieee exception semantics
case "$isgcc" in
gcc)	ccflags="$ccflags -mfp-rounding-mode=d -mieee" ;;
*)	case "$_DEC_cc_style" in
	new)	ccflags="$ccflags -fprm d -ieee"	;;
	esac
	;;
esac

# Make glibpth agree with the compiler suite.  Note that /shlib
# is not here.  That's on purpose.  Even though that's where libc
# really lives from V4.0 on, the linker (and /sbin/loader) won't
# look there by default.  The sharable /sbin utilities were all
# built with "-Wl,-rpath,/shlib" to get around that.  This makes
# no attempt to figure out the additional location(s) searched by
# gcc, since not all versions of gcc are easily coerced into
# revealing that information.
glibpth="/usr/shlib /usr/ccs/lib /usr/lib/cmplrs/cc"
glibpth="$glibpth /usr/lib /usr/local/lib /var/shlib"

# dlopen() is in libc
libswanted="`echo $libswanted | sed -e 's/ dl / /'`"

# libPW contains nothing useful for perl
libswanted="`echo $libswanted | sed -e 's/ PW / /'`"

# libnet contains nothing useful for perl here, and doesn't work
libswanted="`echo $libswanted | sed -e 's/ net / /'`"

# libbsd contains nothing used by perl that is not already in libc
libswanted="`echo $libswanted | sed -e 's/ bsd / /'`"

# libc need not be separately listed
libswanted="`echo $libswanted | sed -e 's/ c / /'`"

# ndbm is already in libc
libswanted="`echo $libswanted | sed -e 's/ ndbm / /'`"

# the basic lddlflags used always
lddlflags='-shared -expect_unresolved "*"'

# If debugging or (old systems and doing shared)
# then do not strip the lib, otherwise, strip.
# As noted above the -DDEBUGGING is added automagically by Configure if -g.
case "$optimize" in
	*-g*) ;; # left intentionally blank
*)	case "$unamer" in
	*[123].*)
		case "$useshrplib" in
		false|undef|'')	lddlflags="$lddlflags -s"	;;
		esac
		;;
        *) lddlflags="$lddlflags -s"
	        ;;
	esac
	;;
esac

#
# Make embedding in things like INN and Apache more memory friendly.
# Keep it overridable on the Configure command line, though, so that
# "-Uuseshrplib" prevents this default.
#

case "$_DEC_cc_style.$useshrplib" in
	new.)	useshrplib="$define"	;;
esac

# The EFF_ONLY_OK from <sys/access.h> is present but dysfunctional for
# [RWX]_OK as of Digital UNIX 4.0[A-D]?.  If and when this gets fixed,
# please adjust this appropriately.  See also pp_sys.c just before the
# emulate_eaccess().

# Fixed in V5.0A.
case "$myosvers" in
*5.0[A-Z]*|*5.[1-9]*|*[6-9].[0-9]*)
	: ok
	;;
*)
# V5.0 or previous
pp_sys_cflags='ccflags="$ccflags -DNO_EFF_ONLY_OK"'
	;;
esac

# The off_t is already 8 bytes, so we do have largefileness.

cat > UU/usethreads.cbu <<'EOCBU'
# This script UU/usethreads.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use threads.
case "$usethreads" in
$define|true|[yY]*)
	# In Tru64 V5 (at least V5.1A, V5.1B) gcc (at least 3.2.2)
	# cannot be used to compile a threaded Perl.
	cat > pthread.c <<EOF
#include <pthread.h>
extern int foo;
EOF
	$cc -c pthread.c 2> pthread.err
	if egrep -q "unrecognized compiler|syntax error" pthread.err; then
	    cat >&4 <<EOF
***
*** I'm sorry but your C compiler ($cc) cannot be used to
*** compile Perl with threads.  The system C compiler should work.
***

Cannot continue, aborting.

EOF
	    rm -f pthread.*
	    exit 1
	fi
	rm -f pthread.*
	# Threads interfaces changed with V4.0.
	case "$isgcc" in
	gcc)
	    ccflags="-D_REENTRANT $ccflags"
	    ;;
	*)  case "$unamer" in
	    *[123].*)	ccflags="-threads $ccflags" ;;
	    *)          ccflags="-pthread $ccflags" ;;
	    esac
	    ;;
	esac
	case "$unamer" in
	*[123].*) libswanted="$libswanted pthreads mach exc c_r" ;;
	*)        libswanted="$libswanted pthread exc" ;;
	esac

	case "$usemymalloc" in
	'')
		usemymalloc='n'
		;;
	esac
	# These symbols are renamed in <time.h> so
	# that the Configure hasproto doesn't see them.
	d_asctime_r_proto="$define"
	d_ctime_r_proto="$define"
	d_gmtime_r_proto="$define"
	d_localtime_r_proto="$define"
	;;
esac
EOCBU

# malloc wrap works
case "$usemallocwrap" in
'') usemallocwrap='define' ;;
esac

cat > UU/uselongdouble.cbu <<'EOCBU'
# This script UU/uselongdouble.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use long doubles.
case "$uselongdouble" in
$define|true|[yY]*)
	case "$myosvers" in
	*[1-4].0*)	cat >&4 <<EOF

***
*** Sorry, you cannot use long doubles in pre-V5.0 releases of Tru64.
***

Cannot continue, aborting.

EOF
		exit 1
		;;
	*)
		# Test whether libc's been fixed yet for long doubles.
		cat >try.c <<\TRY
#include <stdio.h>
int main(int argc, char **argv)
{
	unsigned long uvmax = ~0UL;
	long double ld = uvmax + 0.0L;
	char buf1[30], buf2[30];

	(void) sprintf(buf1, "%lu", uvmax);
	(void) sprintf(buf2, "%.0Lf", ld);
	return strcmp(buf1, buf2) != 0;
}
TRY
		# Don't bother trying to work with Configure's idea of
		# cc and the various flags.  This might not work as-is
		# with gcc -- but we're testing libc, not the compiler.
		if cc -o try $_ccflags_strict_ansi try.c && ./try
		then
			: ok
		else
			cat <<\UGLY >&4
!
Warning!  Your libc has not yet been patched so that its "%Lf" format for
printing long doubles shows all the significant digits.  You will get errors
in the t/op/numconvert test because of this.  (The data is still good
internally, and the "%e" format of printf() or sprintf() in perl will still
produce valid results.)  See README.tru64 for additional details.

Continuing anyway.
!
UGLY
		fi
		$rm -f try try.c
	esac
	;;
esac
EOCBU

case "$myosvers" in
*[1-4].0*) d_modfl=undef ;; # must wait till 5.0
esac

# Keep that leading tab.
	old_LD_LIBRARY_PATH=$LD_LIBRARY_PATH
for p in $loclibpth
do
	if test -d $p; then
	    echo "Appending $p to LD_LIBRARY_PATH." >&4
	    case "$LD_LIBRARY_PATH" in
	    '') LD_LIBRARY_PATH=$p                  ;;
	    *)  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$p ;;
	    esac
	fi
done
case "$LD_LIBRARY_PATH" in
"$old_LD_LIBRARY_PATH") ;;
*) echo "LD_LIBRARY_PATH is now $LD_LIBRARY_PATH." >&4 ;;
esac
case "$LD_LIBRARY_PATH" in
'') ;;
* ) export LD_LIBRARY_PATH ;;
esac

# Enforce strict data.
case "$isgcc" in
gcc)   ;;
*)     # -trapuv poisons uninitialized stack with
       #  0xfff58005fff58005 which is as a pointer a segmentation fault and
       #  as a floating point a signaling NaN.  As integers/longs that causes
       #  no traps but at least it is not zero.
       # -readonly_strings moves string constants into read-only section
       #  which hopefully means that modifying them leads into segmentation
       #  faults.
       for i in -trapuv -readonly_strings
       do
               case "$ccflags" in
               *$i*) ;;
               *) ccflags="$ccflags $i" ;;
               esac
       done
       ;;
esac

# In Tru64 several slightly incompatible socket APIs are supported,
# which one applies is chosen with a set of defines:
# -D_SOCKADDR_LEN enables 4.4BSD and IPv6 interfaces
# -D_POSIX_PII_SOCKET enables socklen_t instead of size_t
for i in -D_SOCKADDR_LEN -D_POSIX_PII_SOCKET
do
    case "$ccflags" in
    *$i*) ;;
    *) ccflags="$ccflags $i" ;;
    esac
done
# For OSF/1 3.2, however, defining _SOCKADDR_LEN would be
# a bad idea since it breaks send() and recv().
case "$ccflags" in
*DEC_OSF1_3_X*SOCKADDR_LEN*)
 ccflags=`echo " $ccflags " | sed -e 's/ -D_SOCKADDR_LEN / /'`
 ;;
esac

# These are in libm, but seem broken (there are no protos in headers,
# or man pages, either)
d_fdim='undef'
d_fma='undef'
d_fmax='undef'
d_fmin='undef'
d_llrint='undef'
d_llround='undef'
d_lrint='undef'
d_lround='undef'
d_nan='undef'
d_nearbyint='undef'
d_round='undef'
d_scalbn='undef'
d_tgamma='undef'

#
# Unset temporary variables no more needed.
#

unset _DEC_cc_style

#
# History:
#
# perl5.005_51:
#
#	September-1998 Jarkko Hietaniemi <jhi@iki.fi>
#
#	* Added the -DNO_EFF_ONLY_OK flag ('use filetest;' support).
#
# perl5.004_57:
#
#	19-Dec-1997 Spider Boardman <spider@Orb.Nashua.NH.US>
#
#	* Newer Digital UNIX compilers enforce signaling for NaN without
#	  -ieee.  Added -fprm d at the same time since it's friendlier for
#	  embedding.
#
#	* Fixed the library search path to match cc, ld, and /sbin/loader.
#
#	* Default to building -Duseshrplib on newer systems.  -Uuseshrplib
#	  still overrides.
#
#	* Fix -pthread additions for useshrplib.  ld has no -pthread option.
#
#
# perl5.004_04:
#
#       19-Sep-1997 Spider Boardman <spider@Orb.Nashua.NH.US>
#
#	* libnet on Digital UNIX is for JAVA, not for sockets.
#
#
# perl5.003_28:
#
#       22-Feb-1997 Jarkko Hietaniemi <jhi@iki.fi>
#
#	* Restructuring Spider's suggestions.
#
#	* Older Digital UNIXes cannot handle -Olimit ... for $lddlflags.
#
#	* ld -s cannot be used in older Digital UNIXes when doing shared.
#
#
#       21-Feb-1997 Spider Boardman <spider@Orb.Nashua.NH.US>
#
#	* -hidden removed.
#
#	* -DSTANDARD_C removed.
#
#	* -D_INTRINSICS added. (that -fast does not seem to buy much confirmed)
#
#	* odbm not in libc, only ndbm. Therefore dbm back to $libswanted.
#
#	* -msym for the newer runtime loaders.
#
#	* $optimize also in $lddflags.
#
#
# perl5.003_27:
#
#	18-Feb-1997 Jarkko Hietaniemi <jhi@iki.fi>
#
#	* unset _DEC_cc_style and more commentary on -std.
#
#
# perl5.003_26:
#
#	15-Feb-1997 Jarkko Hietaniemi <jhi@iki.fi>
#
#	* -std and -ansi.
#
#
# perl5.003_24:
#
#	30-Jan-1997 Jarkko Hietaniemi <jhi@iki.fi>
#
#	* Fixing the note on -DDEBUGGING.
#
#	* Note on -O5 -fast.
#
#
# perl5.003_23:
#
#	26-Jan-1997 Jarkko Hietaniemi <jhi@iki.fi>
#
#	* Notes on how to do both optimisation and debugging.
#
#
#	25-Jan-1997 Jarkko Hietaniemi <jhi@iki.fi>
#
#	* Remove unneeded libraries from $libswanted: PW, bsd, c, dbm
#
#	* Restructure the $lddlflags build.
#
#	* $optimize based on which compiler we have.
#
#
# perl5.003_22:
#
#	23-Jan-1997 Achim Bohnet <ach@rosat.mpe-garching.mpg.de>
#
#	* Added comments 'how to create a debugging version of perl'
#
#	* Fixed logic of this script to prevent stripping of shared
#         objects by the loader (see ld man page for -s) is debugging
#         is set via the -g switch.
#
#
#	21-Jan-1997 Achim Bohnet <ach@rosat.mpe-garching.mpg.de>
#
#	* now 'dl' is always removed from libswanted. Not only if
#	  optimize is an empty string.
#
#
#	17-Jan-1997 Achim Bohnet <ach@rosat.mpe-garching.mpg.de>
#
#	* Removed 'dl' from libswanted: When the FreePort binary
#	  translator for Sun binaries is installed Configure concludes
#	  that it should use libdl.x.yz.fpx.so :-(
#	  Because the dlopen, dlclose,... calls are in the
#	  C library it not necessary at all to check for the
#	  dl library.  Therefore dl is removed from libswanted.
#
#
#	1-Jan-1997 Achim Bohnet <ach@rosat.mpe-garching.mpg.de>
#
#	* Set -Olimit to 3200 because perl_yylex.c got too big
#	  for the optimizer.
#

