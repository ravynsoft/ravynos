# hints/aix_3.sh
#
# On Tue 03 Feb 2004 split off from aix.sh by H.Merijn Brand
# AIX 3.x.x hints thanks to Wayne Scott <wscott@ichips.intel.com>

# Notes:
#
#    - shared libperl support is tricky. if ever libperl.a ends up
#      in /usr/local/lib/* it can override any subsequent builds of
#      that same perl release. to make sure you know where the shared
#      libperl.a is coming from do a 'dump -Hv perl' and check all the
#      library search paths in the loader header.
#
#      it would be nice to warn the user if a libperl.a exists that is
#      going to override the current build, but that would be complex.
#
#      better yet, a solid fix for this situation should be developed.

alignbytes=8

case "$usemymalloc" in
    '')  usemymalloc='n' ;;
    esac

# Intuiting the existence of system calls under AIX is difficult,
# at best; the safest technique is to find them empirically.

case "$usenm" in
    '') usenm='undef'
    esac

so="a"
# AIX itself uses .o (libc.o) but we prefer compatibility
# with the rest of the world and with rest of the scripting
# languages (Tcl, Python) and related systems (SWIG).
# Stephanie Beals <bealzy@us.ibm.com>
dlext="so"

# Take possible hint from the environment.  If 32-bit is set in the
# environment, we can override it later.  If set for 64, the
# 'sizeof' test sees a native 64-bit architecture and never looks back.
case "$use64bitint$use64bitall$usemorebits" in
    *$define*) cat >&4 <<EOM

AIX `oslevel` does not support 64-bit interfaces.
You should upgrade to at least AIX 4.3.
EOM
	exit
	;;
    esac

case "$usemorebits" in
if [ "$OBJECT_MODE"  = "64" ] then
    cat >&4 <<EOF

You have OBJECT_MODE=64 set in the environment. 
This would force a full 64-bit build, but AIX 3
does not support 64bit.
EOF
    fi

# uname -m output is too specific and not appropriate here
case "$archname" in
    '') archname="$osname" ;;
    esac

cc=${cc:-cc}
d_fchmod='undef'
ccflags="$ccflags -D_ALL_SOURCE"

# These functions don't work like Perl expects them to.
d_setregid='undef'
d_setreuid='undef'

# Configure finds setrgid and setruid, but they're useless.  The man
# pages state:
#    setrgid: The EPERM error code is always returned.
#    setruid: The EPERM error code is always returned. Processes cannot
#	      reset only their real user IDs.
d_setrgid='undef'
d_setruid='undef'

# Changes for dynamic linking by Wayne Scott <wscott@ichips.intel.com>
#
# Tell perl which symbols to export for dynamic linking.
cccdlflags='none'	# All AIX code is position independent
   cc_type=xlc		# do not export to config.sh
case "$cc" in
    *gcc*)
        cc_type=gcc
        ccdlflags='-Xlinker'
        if [ "X$gccversion" = "X" ]; then
	    # Done too late in Configure if hinted
	    gccversion=`$cc -dumpversion`
	    fi
        ;;

    *)  ccversion=`lslpp -L | grep 'C for AIX Compiler$' | grep -v '\.msg\.[A-Za-z_]*\.' | head -1 | awk '{print $1,$2}'`
        case "$ccversion" in
	    '') ccversion=`lslpp -L | grep 'IBM C and C++ Compilers LUM$'`
	        ;;

	    *.*.*.*.*.*.*)		# Ahhrgg, more than one C compiler installed
	        first_cc_path=`which ${cc:-cc}`
	        case "$first_cc_path" in
	            *vac*)
			cc_type=vac
			;;
		    /usr/bin/cc)		# Check the symlink
			if [ -h $first_cc_path ]; then
			    ls -l $first_cc_path > reflect
			    if grep -i vac reflect >/dev/null 2>&1 ; then
				cc_type=vac
				fi
			    rm -f reflect
			    fi
			;;
		    esac
		ccversion=`lslpp -L | grep 'C for AIX Compiler$' | grep -i $cc_type | head -1`
		;;
	    vac*.*.*.*)
		cc_type=vac
		;;
	    esac

	ccversion=`echo "$ccversion" | awk '{print $2}'`
	case "$ccversion" in
	    3.6.6.0)
		optimize='none'
		;;
	    4.4.0.0|4.4.0.1|4.4.0.2)
		cat >&4 <<EOF
***
*** This C compiler ($ccversion) is outdated.
***
*** Please upgrade to at least 4.4.0.3.
***
EOF
		;;
	    5.0.0.0)
		cat >&4 <<EOF
***
*** This C compiler ($ccversion) is known to have too many optimizer
*** bugs to compile a working Perl.
***
*** Consider upgrading your C compiler, or getting the GNU cc (gcc).
***
*** Cannot continue, aborting.
EOF
		exit 1
		;;
	    5.0.1.0)
		cat >&4 <<EOF
***
*** This C compiler ($ccversion) is known to have optimizer problems
*** when compiling regcomp.c.
***
*** Disabling optimization for that file but consider upgrading
*** your C compiler.
***
EOF
regcomp_cflags='optimize='
		;;
	    esac
    esac
# the required -bE:$installarchlib/CORE/perl.exp is added by
# libperl.U (Configure) later.

# The first 3 options would not be needed if dynamic libs. could be linked
# with the compiler instead of ld.
# -bI:$(PERL_INC)/perl.exp  Read the exported symbols from the perl binary
# -bE:$(BASEEXT).exp	    Export these symbols.  This file contains only one
#			    symbol: boot_$(EXP)	 can it be auto-generated?
lddlflags="$lddlflags -H512 -T512 -bhalt:4 -bM:SRE -bI:\$(PERL_INC)/perl.exp -bE:\$(BASEEXT).exp -e _nostart -lc"

case $cc_type in
    vac|xlc)
	case "$uselongdouble" in
	    $define|true|[yY]*)
		ccflags="$ccflags -qlongdouble"
		libswanted="c128 $libswanted"
		lddlflags=`echo "$lddlflags " | sed -e 's/ -lc / -lc128 -lc /'`
		;;
	    esac
    esac

case "$cc" in
    *gcc*) ;;
    cc*|xlc*) # cc should've been set by line 116 or so if empty.
	if test ! -x /usr/bin/$cc -a -x /usr/vac/bin/$cc; then
	    case ":$PATH:" in
		*:/usr/vac/bin:*) ;;
		*) if test ! -x /QOpenSys/usr/bin/$cc; then
		       # The /QOpenSys/usr/bin/$cc saves us if we are
		       # building natively in OS/400 PASE.
		       cat >&4 <<EOF

***
*** You either implicitly or explicitly specified an IBM C compiler,
*** but you do not seem to have one in /usr/bin, but you seem to have
*** the VAC installed in /usr/vac, but you do not have the /usr/vac/bin
*** in your PATH.  I suggest adding that and retrying Configure.
***
EOF
		       exit 1
		       fi 
		   ;;
		esac
	    fi
	;;
    esac

case "$ldlibpthname" in
    '') ldlibpthname=LIBPATH ;;
    esac

# This script UU/usethreads.cbu will get 'called-back' by Configure 
# after it has prompted the user for whether to use threads.
cat > UU/usethreads.cbu <<'EOCBU'
case "$usethreads" in
    $define|true|[yY]*)
	d_drand48_r='undef'
	d_endgrent_r='undef'
	d_endpwent_r='undef'
	d_getgrent_r='undef'
	d_getpwent_r='undef'
	d_random_r='undef'
	d_setgrent_r='undef'
	d_setpwent_r='undef'
	d_srand48_r='undef'
	d_strerror_r='undef'
	ccflags="$ccflags -DNEED_PTHREAD_INIT"
	case "$cc" in
	    *gcc*)
		ccflags="-D_THREAD_SAFE $ccflags"
		;;
	    cc_r) ;;
	    cc|xl[cC]_r) 
		echo >&4 "Switching cc to cc_r because of POSIX threads."
		# xlc_r has been known to produce buggy code in AIX 4.3.2.
		# (e.g. pragma/overload core dumps)	 Let's suspect xlC_r, too.
		# --jhi@iki.fi
		cc=cc_r
		;;
	    '') 
		cc=cc_r
		;;
	    *)
		cat >&4 <<EOM
*** For pthreads you should use the AIX C compiler cc_r.
*** (now your compiler was set to '$cc')
*** Cannot continue, aborting.
EOM
		exit 1
		;;
	    esac

	# c_rify libswanted.
	set `echo X "$libswanted "| sed -e 's/ \([cC]\) / \1_r /g'`
	shift
	libswanted="$*"
	# c_rify lddlflags.
	set `echo X "$lddlflags "| sed -e 's/ \(-l[cC]\) / \1_r /g'`
	shift
	lddlflags="$*"

	# Insert pthreads to libswanted, before any libc or libC.
	set `echo X "$libswanted "| sed -e 's/ \([cC]_r\) / pthreads \1 /'`
	shift
	libswanted="$*"
	# Insert pthreads to lddlflags, before any libc or libC.
	set `echo X "$lddlflags " | sed -e 's/ \(-l[cC]_r\) / -lpthreads \1 /'`
	shift
	lddlflags="$*"
	;;
    esac
EOCBU

# This script UU/uselargefiles.cbu will get 'called-back' by Configure 
# after it has prompted the user for whether to use large files.
cat > UU/uselargefiles.cbu <<'EOCBU'
case "$uselargefiles" in
    ''|$define|true|[yY]*)
	# Configure should take care of use64bitint and use64bitall being
	# defined before uselargefiles.cbu is consulted.
	if test X"$use64bitint:$quadtype" = X"$define:long" -o X"$use64bitall" = Xdefine; then
# Keep these at the left margin.
ccflags_uselargefiles="`getconf XBS5_LP64_OFF64_CFLAGS 2>/dev/null`"
ldflags_uselargefiles="`getconf XBS5_LP64_OFF64_LDFLAGS 2>/dev/null`"
	else
# Keep these at the left margin.
ccflags_uselargefiles="`getconf XBS5_ILP32_OFFBIG_CFLAGS 2>/dev/null`"
ldflags_uselargefiles="`getconf XBS5_ILP32_OFFBIG_LDFLAGS 2>/dev/null`"
	    fi
	if test X"$use64bitint:$quadtype" = X"$define:long" -o X"$use64bitall" = Xdefine; then
# Keep this at the left margin.
libswanted_uselargefiles="`getconf XBS5_LP64_OFF64_LIBS 2>/dev/null|sed -e 's@^-l@@' -e 's@ -l@ @g'`"
	else
# Keep this at the left margin.
libswanted_uselargefiles="`getconf XBS5_ILP32_OFFBIG_LIBS 2>/dev/null|sed -e 's@^-l@@' -e 's@ -l@ @g'`"
	    fi
	case "$ccflags_uselargefiles$ldflags_uselargefiles$libs_uselargefiles" in
	    '') ;;
	    *)  ccflags="$ccflags $ccflags_uselargefiles"
	        ldflags="$ldflags $ldflags_uselargefiles"
	        libswanted="$libswanted $libswanted_uselargefiles"
	        ;;
	    esac
	case "$gccversion" in
	    '') ;;
	    *)  # Remove xlc-specific -qflags.
		ccflags="`echo $ccflags | sed -e 's@ -q[^ ]*@ @g' -e 's@^-q[^ ]* @@g'`"
		ldflags="`echo $ldflags | sed -e 's@ -q[^ ]*@ @g' -e 's@^-q[^ ]* @@g'`"
		# Move xlc-specific -bflags.
		ccflags="`echo $ccflags | sed -e 's@ -b@ -Wl,-b@g'`"
		ldflags="`echo ' '$ldflags | sed -e 's@ -b@ -Wl,-b@g'`"
		lddlflags="`echo ' '$lddlflags | sed -e 's@ -b@ -Wl,-b@g'`"
		ld='gcc'
		echo >&4 "(using ccflags   $ccflags)"
		echo >&4 "(using ldflags   $ldflags)"
		echo >&4 "(using lddlflags $lddlflags)"
		;; 
	    esac
        ;;
    esac
EOCBU

if test $usenativedlopen = 'true' ; then
    ccflags="$ccflags -DUSE_NATIVE_DLOPEN"
    case "$cc" in
        *gcc*) ldflags="$ldflags -Wl,-brtl" ;;
        *)     ldflags="$ldflags -brtl" ;;
        esac
else
    # If the C++ libraries, libC and libC_r, are available we will
    # prefer them over the vanilla libc, because the libC contain
    # loadAndInit() and terminateAndUnload() which work correctly
    # with C++ statics while libc load() and unload() do not. See
    # ext/DynaLoader/dl_aix.xs. The C-to-C_r switch is done by
    # usethreads.cbu, if needed.
    if test -f /lib/libC.a -a X"`$cc -v 2>&1 | grep gcc`" = X; then
	# Cify libswanted.
	set `echo X "$libswanted "| sed -e 's/ c / C c /'`
	shift
	libswanted="$*"
	# Cify lddlflags.
	set `echo X "$lddlflags "| sed -e 's/ -lc / -lC -lc /'`
	shift
	lddlflags="$*"
	fi
    fi

# EOF
