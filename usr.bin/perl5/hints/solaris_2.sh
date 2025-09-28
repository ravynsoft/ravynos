# hints/solaris_2.sh
# Contributions by (in alphabetical order) Alan Burlison, Andy Dougherty,
# Dean Roehrich, Jarkko Hietaniemi, Lupe Christoph, Richard Soderberg and
# many others.
#
# See README.solaris for additional information.
#
# For consistency with gcc, we do not adopt Sun Marketing's
# removal of the '2.' prefix from the Solaris version number.
# (Configure tries to detect an old fixincludes and needs
# this information.)

# If perl fails tests that involve dynamic loading of extensions, and
# you are using gcc, be sure that you are NOT using GNU as and ld.  One
# way to do that is to invoke Configure with
#
#     sh Configure -Dcc='gcc -B/usr/ccs/bin/'
#
#  (Note that the trailing slash is *required*.)
#  gcc will occasionally emit warnings about "unused prefix", but
#  these ought to be harmless.  See below for more details.

# Solaris has secure SUID scripts
d_suidsafe=${d_suidsafe:-define}

# Be paranoid about nm failing to find symbols
mistrustnm=${mistrustnm:-run}

# Several people reported problems with perl's malloc, especially
# when use64bitall is defined or when using gcc.
#     http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/2001-01/msg01318.html
#     http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/2001-01/msg00465.html
usemymalloc=${usemymalloc:-false}

# malloc wrap works
case "$usemallocwrap" in
'') usemallocwrap='define' ;;
esac

# Avoid all libraries in /usr/ucblib.
# /lib is just a symlink to /usr/lib
set `echo $glibpth | sed -e 's@/usr/ucblib@@' -e 's@ /lib @ @'`
glibpth="$*"

# Starting with Solaris 10, we don't want versioned shared libraries because
# those often indicate a private use only library.  Especially badly that would
# break things with SUNWbdb (Berkeley DB) being installed, which brings in
# /usr/lib/libdb.so.1, but that is not really meant for public consumption.
#  XXX Revisit after perl 5.10 -- should we apply this to older Solaris
# versions too?  (A.D. 11/2007).
case "`$run uname -r`" in
5.[0-9]) ;;
*) ignore_versioned_solibs=y ;;
esac

# Remove unwanted libraries.  -lucb contains incompatible routines.
# -lld and -lsec don't do anything useful. -lcrypt does not
# really provide anything we need over -lc, so we drop it, too.
# -lmalloc can cause a problem with GNU CC & Solaris.  Specifically,
# libmalloc.a may allocate memory that is only 4 byte aligned, but
# GNU CC on the Sparc assumes that doubles are 8 byte aligned.
# Thanks to  Hallvard B. Furuseth <h.b.furuseth@usit.uio.no>
set `echo " $libswanted " | sed -e 's@ ld @ @' -e 's@ malloc @ @' -e 's@ ucb @ @' -e 's@ sec @ @' -e 's@ crypt @ @'`
libswanted="$*"

# Add libnsl for networking support
set `echo " $libswanted " | sed -e 's@ inet @ inet nsl @'`
libswanted="$*"

# Look for architecture name.  We want to suggest a useful default.
case "$archname" in
'')
    if test -f /usr/bin/arch; then
	archname=`/usr/bin/arch`
	archname="${archname}-${osname}"
    elif test -f /usr/ucb/arch; then
	archname=`/usr/ucb/arch`
	archname="${archname}-${osname}"
    fi
    ;;
esac

#
# This extracts the library directories that will be searched by the Sun
# Workshop compiler, given the command-line supplied in $tryworkshopcc.
# Use thusly: loclibpth="`$getworkshoplibs` $loclibpth"
#
	getworkshoplibs=`cat <<'END'
eval $tryworkshopcc -### 2>&1 | \
sed -n '/ -Y /s!.* -Y "P,\([^"]*\)".*!\1!p' | tr ':' ' ' | \
sed -e 's!/usr/lib/sparcv9!!' -e 's!/usr/ccs/lib/sparcv9!!' \
    -e 's!/usr/lib!!g' -e 's!/usr/ccs/lib!!g'
END
`

case "$cc" in
'')    for i in `ls -r /opt/*studio*/bin/cc` /opt/SUNWspro/bin/cc \
		`which gcc`
       do
	       if test -f "$i"; then
		       cc=$i
		       cat <<EOF >&4

You specified no cc but you seem to have the Workshop compiler
($cc) installed, using that.
If you want something else, specify that in the command line,
e.g. Configure -Dcc=gcc

EOF
			break
		fi
	done
	;;
esac

######################################################
# General sanity testing.  See below for excerpts from the Solaris FAQ.
#
# From roehrich@ironwood-fddi.cray.com Wed Sep 27 12:51:46 1995
# Date: Thu, 7 Sep 1995 16:31:40 -0500
# From: Dean Roehrich <roehrich@ironwood-fddi.cray.com>
# To: perl5-porters@africa.nicoh.com
# Subject: Re: On perl5/solaris/gcc
#
# Here's another draft of the perl5/solaris/gcc sanity-checker.

case `type ${cc:-cc}` in
*/usr/ucb/cc*) cat <<END >&4

NOTE:  Some people have reported problems with /usr/ucb/cc.
If you have difficulties, please make sure the directory
containing your C compiler is before /usr/ucb in your PATH.

END
;;
esac


# Check that /dev/fd is mounted.  If it is not mounted, let the
# user know that suid scripts may not work.
$run mount | grep '^/dev/fd ' 2>&1 > /dev/null
case $? in
0) ;;
*)
	cat <<END >&4

NOTE: Your system does not have /dev/fd mounted.  If you want to
be able to use set-uid scripts you must ask your system administrator
to mount /dev/fd.

END
	;;
esac


# See if libucb can be found in /usr/lib.  If it is, warn the user
# that this may cause problems while building Perl extensions.
found_libucb=''
case "$run" in
'') /usr/bin/ls /usr/lib/libucb* >/dev/null 2>&1
    found_libucb=$?
    ;;
*)  $run /usr/bin/ls '/usr/lib/libucb*' >/dev/null 2>&1
    found_libucb=$?
    ;;
esac

case $found_libucb in
0)
	cat <<END >&4

NOTE: libucb has been found in /usr/lib.  libucb should reside in
/usr/ucblib.  You may have trouble while building Perl extensions.

END
;;
esac

# Use shell built-in 'type' command instead of /usr/bin/which to
# avoid possible csh start-up problems and also to use the same shell
# we'll be using to Configure and make perl.
# The path name is the last field in the output, but the type command
# has an annoying array of possible outputs, e.g.:
#	make is hashed (/opt/gnu/bin/make)
#	cc is /usr/ucb/cc
#	foo not found
# use a command like type make | awk '{print $NF}' | sed 's/[()]//g'

# See if make(1) is GNU make(1).
# If it is, make sure the setgid bit is not set.
make -v > make.vers 2>&1
if grep GNU make.vers > /dev/null 2>&1; then
    tmp=`type make | awk '{print $NF}' | sed 's/[()]//g'`
    case "`${ls:-'/usr/bin/ls'} -lL $tmp`" in
    ??????s*)
	    cat <<END >&2

NOTE: Your PATH points to GNU make, and your GNU make has the set-group-id
bit set.  You must either rearrange your PATH to put /usr/ccs/bin before the
GNU utilities or you must ask your system administrator to disable the
set-group-id bit on GNU make.

END
	    ;;
    esac
fi
rm -f make.vers

cat > UU/cc.cbu <<'EOCBU'
# This script UU/cc.cbu will get 'called-back' by Configure after it
# has prompted the user for the C compiler to use.

# If the C compiler is gcc:
#   - check the fixed-includes
#   - check as(1) and ld(1), they should not be GNU
#	(GNU as and ld 2.8.1 and later are reportedly ok, however.)
# If the C compiler is not gcc:
#   - Check if it is the Workshop/Forte compiler.
#     If it is, prepare for 64 bit and long doubles.
#   - check as(1) and ld(1), they should not be GNU
#	(GNU as and ld 2.8.1 and later are reportedly ok, however.)
#
# Watch out in case they have not set $cc.

# Perl compiled with some combinations of GNU as and ld may not
# be able to perform dynamic loading of extensions.  If you have a
# problem with dynamic loading, be sure that you are using the Solaris
# /usr/ccs/bin/as and /usr/ccs/bin/ld.  You can do that with
#	sh Configure -Dcc='gcc -B/usr/ccs/bin/'
# (note the trailing slash is required).
# Combinations that are known to work with the following hints:
#
#  gcc-2.7.2, GNU as 2.7, GNU ld 2.7
#  egcs-1.0.3, GNU as 2.9.1 and GNU ld 2.9.1
#	--Andy Dougherty  <doughera@lafayette.edu>
#	Tue Apr 13 17:19:43 EDT 1999

# Get gcc to share its secrets.
echo 'int main() { return 0; }' > try.c
	# Indent to avoid propagation to config.sh
	verbose=`${cc:-cc} $ccflags -v -o try try.c 2>&1`

# XXX TODO:  'specs' output changed from 'Reading specs from' in gcc-[23] to 'Using
# built-in specs' in gcc-4.  Perhaps we should just use the same gcc test as
# in Configure to see if we're using gcc.
if echo "$verbose" | egrep '(Reading specs from)|(Using built-in specs)' >/dev/null 2>&1; then
	#
	# Using gcc.
	#
	cc_name='gcc'

	# See if as(1) is GNU as(1).  GNU as(1) might not work for this job.
	if echo "$verbose" | grep ' /usr/ccs/bin/as ' >/dev/null 2>&1; then
	    :
	else
	    cat <<END >&2

NOTE: You are using GNU as(1).  GNU as(1) might not build Perl.  If you
have trouble, you can use /usr/ccs/bin/as by including -B/usr/ccs/bin/
in your ${cc:-cc} command.  (Note that the trailing "/" is required.)

END
	    # Apparently not needed, at least for as 2.7 and later.
	    # cc="${cc:-cc} $ccflags -B/usr/ccs/bin/"
	fi

	# See if ld(1) is GNU ld(1).  GNU ld(1) might not work for this job.
	# Recompute $verbose since we may have just changed $cc.
	verbose=`${cc:-cc} $ccflags -v -o try try.c 2>&1 | grep ld 2>&1`

	if echo "$verbose" | grep ' /usr/ccs/bin/ld ' >/dev/null 2>&1; then
	    # Ok, gcc directly calls the Solaris /usr/ccs/bin/ld.
	    :
	elif echo "$verbose" | grep "ld: Software Generation Utilities" >/dev/null 2>&1; then
	    # Hmm.  gcc doesn't call /usr/ccs/bin/ld directly, but it
	    # does appear to be using it eventually.  egcs-1.0.3's ld
	    # wrapper does this.
	    # Most Solaris versions of ld I've seen contain the magic
	    # string used in the grep.
	    :
	elif echo "$verbose" | grep "Solaris Link Editors" >/dev/null 2>&1; then
	    # However some Solaris 8 versions prior to ld 5.8-1.286 contain
	    # this string instead.
	    :
	else
	    # No evidence yet of /usr/ccs/bin/ld.  Some versions
	    # of egcs's ld wrapper call /usr/ccs/bin/ld in turn but
	    # apparently don't reveal that unless you pass in -V.
	    # (This may all depend on local configurations too.)

	    # Recompute verbose with -Wl,-v to find GNU ld if present
	    verbose=`${cc:-cc} $ccflags -Wl,-v -o try try.c 2>&1 | grep /ld 2>&1`

	    myld=`echo $verbose | awk '/\/ld/ {print $1}'`
	    # This assumes that gcc's output will not change, and that
	    # /full/path/to/ld will be the first word of the output.
	    # Thus myld is something like /opt/gnu/sparc-sun-solaris2.5/bin/ld

	    # Allow that $myld may be '', due to changes in gcc's output
	    if ${myld:-ld} -V 2>&1 |
		grep "ld: Software Generation Utilities" >/dev/null 2>&1; then
		# Ok, /usr/ccs/bin/ld eventually does get called.
		:
	    elif ${myld:-ld} -V 2>&1 |
		grep "Solaris Link Editors" >/dev/null 2>&1; then
		# Ok, /usr/ccs/bin/ld eventually does get called.
		:
	    else
		echo "Found GNU ld='$myld'" >&4
		cat <<END >&2

NOTE: You are using GNU ld(1).  GNU ld(1) might not build Perl.  If you
have trouble, you can use /usr/ccs/bin/ld by including -B/usr/ccs/bin/
in your ${cc:-cc} command.  (Note that the trailing "/" is required.)

I will try to use GNU ld by passing in the -Wl,-E flag, but if that
doesn't work, you should use -B/usr/ccs/bin/ instead.

END
		ccdlflags="$ccdlflags -Wl,-E"
		lddlflags="$lddlflags -Wl,-E -shared"
	    fi
	fi

else
	#
	# Not using gcc.
	#
	cat > try.c << 'EOM'
#include <stdio.h>
int main() {
#if defined(__SUNPRO_C)
	printf("workshop\n");
#else
#if defined(__SUNPRO_CC)
	printf("workshop CC\n");
#else
	printf("\n");
#endif
#endif
return(0);
}
EOM
	tryworkshopcc="${cc:-cc} $ccflags try.c -o try"
	if $tryworkshopcc >/dev/null 2>&1; then
		cc_name=`$run ./try`
		if test "$cc_name" = "workshop"; then
			ccversion="`${cc:-cc} -V 2>&1|sed -n -e '1s/^[Cc][Cc9]9*: //p'`"
		fi
		if test "$cc_name" = "workshop CC"; then
			ccversion="`${cc:-CC} -V 2>&1|sed -n -e '1s/^[Cc][C]: //p'`"
		fi
		case "$cc_name" in
		workshop*)
			# Settings for either cc or CC
			if test ! "$use64bitall_done"; then
				loclibpth="/usr/lib /usr/ccs/lib `$getworkshoplibs` $loclibpth"
			fi
			# Sun CC/cc don't support gcc attributes
			d_attribute_format='undef'
			d_attribute_malloc='undef'
			d_attribute_nonnull='undef'
			d_attribute_noreturn='undef'
			d_attribute_pure='undef'
			d_attribute_unused='undef'
			d_attribute_warn_unused_result='undef'
			case "$cc" in
			*c99)	# c99 rejects bare '-O'.
				case "$optimize" in
				''|-O) optimize=-O3 ;;
				esac
				# Without -Xa c99 doesn't see
				# many OS interfaces.
				case "$ccflags" in
				*-Xa*)	;;
				*) ccflags="$ccflags -Xa" ;;
				esac
				;;
			esac
			;;
		esac
	fi

	# See if as(1) is GNU as(1).  GNU might not work for this job.
	case `as --version < /dev/null 2>&1` in
	*GNU*)
		cat <<END >&2

NOTE: You are using GNU as(1).  GNU as(1) might not build Perl.
You must arrange to use /usr/ccs/bin/as, perhaps by adding /usr/ccs/bin
to the beginning of your PATH.

END
		;;
	esac

	# See if ld(1) is GNU ld(1).  GNU ld(1) might not work for this job.
	# ld --version doesn't properly report itself as a GNU tool,
	# as of ld version 2.6, so we need to be more strict. TWP 9/5/96
	# Sun's ld always emits the "Software Generation Utilities" string.
	if ld -V 2>&1 | grep "ld: Software Generation Utilities" >/dev/null 2>&1; then
	    # Ok, ld is /usr/ccs/bin/ld.
	    :
	else
	    cat <<END >&2

NOTE: You are apparently using GNU ld(1).  GNU ld(1) might not build Perl.
You should arrange to use /usr/ccs/bin/ld, perhaps by adding /usr/ccs/bin
to the beginning of your PATH.

END
	fi
fi

# as --version or ld --version might dump core.
rm -f try try.c core
EOCBU

cat > UU/usethreads.cbu <<'EOCBU'
# This script UU/usethreads.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use threads.
case "$usethreads" in
$define|true|[yY]*)
	ccflags="-D_REENTRANT $ccflags"

	# -lpthread overrides some lib C functions, so put it before c.
	set `echo X "$libswanted "| sed -e "s/ c / pthread c /"`
	shift
	libswanted="$*"

	# sched_yield is available in the -lrt library.  However,
	# we can also pick up the equivalent yield() function in the
	# normal C library.  To avoid pulling in unnecessary
	# libraries, we'll normally avoid sched_yield()/-lrt and
	# just use yield().  However, we'll honor a command-line
	# override : "-Dsched_yield=sched_yield".
	# If we end up using sched_yield, we're going to need -lrt.
	sched_yield=${sched_yield:-yield}
	if test "$sched_yield" = "sched_yield"; then
	    set `echo X "$libswanted "| sed -e "s/ pthread / rt pthread /"`
	    shift
	    libswanted="$*"
	fi

	# On Solaris 2.6 x86 there is a bug with sigsetjmp() and siglongjmp()
	# when linked with the threads library, such that whatever positive
	# value you pass to siglongjmp(), sigsetjmp() returns 1.
	# Thanks to Simon Parsons <S.Parsons@ftel.co.uk> for this report.
	# Sun BugID is 4117946, "sigsetjmp always returns 1 when called by
	# siglongjmp in a MT program". As of 19980622, there is no patch
	# available.
	cat >try.c <<'EOM'
	/* Test for sig(set|long)jmp bug. */
	#include <setjmp.h>

	int main()
	{
	    sigjmp_buf env;
	    int ret;

	    ret = sigsetjmp(env, 1);
	    if (ret) { return ret == 2; }
	    siglongjmp(env, 2);
	}
EOM
	if test "`arch`" = i86pc -a `uname -r` = 5.6 && \
	   ${cc:-cc} try.c -lpthread >/dev/null 2>&1 && ./a.out; then
	    d_sigsetjmp=$undef
	fi

	# These prototypes should be visible since we using
	# -D_REENTRANT, but that does not seem to work.
	# It does seem to work for getnetbyaddr_r, weirdly enough,
	# and other _r functions. (Solaris 8)

	d_ctermid_r_proto="$define"
	d_gethostbyaddr_r_proto="$define"
	d_gethostbyname_r_proto="$define"
	d_getnetbyname_r_proto="$define"
	d_getprotobyname_r_proto="$define"
	d_getprotobynumber_r_proto="$define"
	d_getservbyname_r_proto="$define"
	d_getservbyport_r_proto="$define"

	# Ditto. (Solaris 7)
	d_readdir_r_proto="$define"
	d_readdir64_r_proto="$define"
	d_tmpnam_r_proto="$define"
	d_ttyname_r_proto="$define"

	;;
esac
EOCBU

cat > UU/uselargefiles.cbu <<'EOCBU'
# This script UU/uselargefiles.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use large files.
case "$uselargefiles" in
''|$define|true|[yY]*)

# Keep these in the left margin.
ccflags_uselargefiles="`$run getconf LFS_CFLAGS 2>/dev/null`"
ldflags_uselargefiles="`$run getconf LFS_LDFLAGS 2>/dev/null`"
libswanted_uselargefiles="`$run getconf LFS_LIBS 2>/dev/null|sed -e 's@^-l@@' -e 's@ -l@ @g'`"

    ccflags="$ccflags $ccflags_uselargefiles"
    ldflags="$ldflags $ldflags_uselargefiles"
    libswanted="$libswanted $libswanted_uselargefiles"
    ;;
esac
EOCBU

# This is truly a mess.
case "$usemorebits" in
"$define"|true|[yY]*)
	use64bitint="$define"
	uselongdouble="$define"
	;;
esac

if test `$run uname -p` = i386; then
    case "$use64bitint" in
    "$define"|true|[yY]*)
            ccflags="$ccflags -DPTR_IS_LONG"
            ;;
    esac
fi

if test `$run uname -p` = sparc -o `$run uname -p` = i386; then
    cat > UU/use64bitint.cbu <<'EOCBU'
# This script UU/use64bitint.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use 64 bit integers.
case "$use64bitint" in
"$define"|true|[yY]*)
	    case "`$run uname -r`" in
	    5.[0-4])
		cat >&4 <<EOM
Solaris `uname -r|sed -e 's/^5\./2./'` does not support 64-bit integers.
You should upgrade to at least Solaris 2.5.
EOM
		exit 1
		;;
	    esac

# gcc-2.8.1 on Solaris 8 with -Duse64bitint fails op/pat.t test 822
# if we compile regexec.c with -O.  Turn off optimization for that one
# file.  See hints/README.hints , especially
# =head2 Propagating variables to config.sh, method 3.
#  A. Dougherty  May 24, 2002
    case "${gccversion}-${optimize}" in
    2.8*-O*)
	# Honor a command-line override (rather unlikely)
	case "$regexec_cflags" in
	'') echo "Disabling optimization on regexec.c for gcc $gccversion" >&4
	    regexec_cflags='optimize='
	    echo "regexec_cflags='optimize=\"\"'" >> config.sh
	    ;;
	esac
	;;
    esac
    ;;
esac
EOCBU

    cat > UU/use64bitall.cbu <<'EOCBU'
# This script UU/use64bitall.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to be maximally 64 bitty.
case "$use64bitall-$use64bitall_done" in
"$define-"|true-|[yY]*-)
	    case "`$run uname -r`" in
	    5.[0-6])
		cat >&4 <<EOM
Solaris `uname -r|sed -e 's/^5\./2./'` does not support 64-bit pointers.
You should upgrade to at least Solaris 2.7.
EOM
		exit 1
		;;
	    esac
	    processor=`$run uname -p`;
	    if test "$processor" = sparc; then
		libc='/usr/lib/sparcv9/libc.so'
		if test ! -f $libc; then
		    cat >&4 <<EOM

I do not see the 64-bit libc, $libc.
Cannot continue, aborting.

EOM
		    exit 1
		fi
	    fi
	    case "${cc:-cc} -v 2>/dev/null" in
	    *gcc*|*g++*)
		echo 'int main() { return 0; }' > try.c
		case "`${cc:-cc} $ccflags -mcpu=v9 -m64 -S try.c 2>&1 | grep 'm64 is not supported by this configuration'`" in
		*"m64 is not supported"*)
		    cat >&4 <<EOM

Full 64-bit build is not supported by this gcc configuration.
Check http://gcc.gnu.org/ for the latest news of availability
of gcc for 64-bit Sparc.

Cannot continue, aborting.

EOM
		    exit 1
		    ;;
		esac
		if test "$processor" = sparc; then
		    loclibpth="/usr/lib/sparcv9 $loclibpth"
		    ccflags="$ccflags -mcpu=v9"
		fi
		ccflags="$ccflags -m64"

		# This adds in -Wa,-xarch=v9.  I suspect that's superfluous,
		# since the -m64 above should do that already.  Someone
		# with gcc-3.x.x, please test with gcc -v.   A.D. 20-Nov-2003
#		if test $processor = sparc -a X`$run getconf XBS5_LP64_OFF64_CFLAGS 2>/dev/null` != X; then
#		    ccflags="$ccflags -Wa,`$run getconf XBS5_LP64_OFF64_CFLAGS 2>/dev/null`"
#		fi
		ldflags="$ldflags -m64"

		# See [perl #66604]:  On Solaris 11, gcc -m64 on amd64
		# appears not to understand -G.  (gcc -G has not caused
		# problems on other platforms in the past.)  gcc versions
		# at least as old as 3.4.3 support -shared, so just
		# use that with Solaris 11 and later, but keep
		# the old behavior for older Solaris versions.
		case "$osvers" in
			2.?|2.10) lddlflags="$lddlflags -G -m64" ;;
			*) lddlflags="$lddlflags -shared -m64" ;;
		esac
		;;
	    *)
		getconfccflags="`$run getconf XBS5_LP64_OFF64_CFLAGS 2>/dev/null`"
		getconfldflags="`$run getconf XBS5_LP64_OFF64_LDFLAGS 2>/dev/null`"
		getconflddlflags="`$run getconf XBS5_LP64_OFF64_LDFLAGS 2>/dev/null`"
		echo "int main() { return(0); } " > try.c
		case "`${cc:-cc} $getconfccflags try.c 2>&1 | grep 'deprecated'`" in
		*" -xarch=generic64 is deprecated, use -m64 "*)
		    getconfccflags=`echo $getconfccflags | sed -e 's/xarch=generic64/m64/'`
		    getconfldflags=`echo $getconfldflags | sed -e 's/xarch=generic64/m64/'`
		    getconflddlflags=`echo $getconflddlflags | sed -e 's/xarch=generic64/m64/'`
		    ;;
		esac
		ccflags="$ccflags $getconfccflags"
		ldflags="$ldflags $getconfldflags"
		lddlflags="$lddlflags -G $getconflddlflags"

		echo "int main() { return(0); } " > try.c
		tryworkshopcc="${cc:-cc} try.c -o try $ccflags"
		if test "$processor" = sparc; then
		    loclibpth="/usr/lib/sparcv9 /usr/ccs/lib/sparcv9 $loclibpth"
		fi
		loclibpth="`$getworkshoplibs` $loclibpth"
		;;
	    esac
	    unset processor
	    use64bitall_done=yes
	    archname64=64
	    ;;
esac
EOCBU

    # Actually, we want to run this already now, if so requested,
    # because we need to fix up things right now.
    case "$use64bitall" in
    "$define"|true|[yY]*)
	# CBUs expect to be run in UU
	cd UU; . ./use64bitall.cbu; cd ..
	;;
    esac
fi

cat > UU/uselongdouble.cbu <<'EOCBU'
# This script UU/uselongdouble.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use long doubles.
case "$uselongdouble" in
"$define"|true|[yY]*)
	if test "$cc_name" = "workshop"; then
		cat > try.c << 'EOM'
#include <sunmath.h>
int main() { (void) powl(2, 256); return(0); }
EOM
		if ${cc:-cc} try.c -lsunmath -o try > /dev/null 2>&1 && ./try; then
			libswanted="$libswanted sunmath"
		fi
	else
		cat >&4 <<EOM

The Sun Workshop math library is either not available or not working,
so I do not know how to do long doubles, sorry.
I'm therefore disabling the use of long doubles.
EOM
		uselongdouble="$undef"
	fi
	;;
esac
EOCBU

rm -f try.c try.o try a.out

# If using C++, the Configure scan for dlopen() will fail in Solaris
# because one of the two (1) an extern "C" linkage definition is needed
# (2) #include <dlfcn.h> is needed, *and* a cast to (void*(*)())
# is needed for the &dlopen.  Adding any of these would require changing
# a delicate spot in Configure, so easier just to force our guess here
# for Solaris.  Much the same goes for dlerror().
case "$cc" in
*g++*|*CC*)
  d_dlopen='define'
  d_dlerror='define'
  ;;
esac

# Oracle/Sun builds their Perl shared since 5.6.1, and they also
# strongly recommend using shared libraries in general.
#
# Furthermore, OpenIndiana seems to effectively require building perl
# shared, or otherwise perl scripts won't even find the Perl library.
useshrplib='true'
