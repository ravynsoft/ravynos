# hints/linux.sh
# Original version by rsanders
# Additional support by Kenneth Albanowski <kjahds@kjahds.com>
#
# ELF support by H.J. Lu <hjl@nynexst.com>
# Additional info from Nigel Head <nhead@ESOC.bitnet>
# and Kenneth Albanowski <kjahds@kjahds.com>
#
# Consolidated by Andy Dougherty <doughera@lafayette.edu>
#
# Updated Thu Feb  8 11:56:10 EST 1996

# Updated Thu May 30 10:50:22 EDT 1996 by <doughera@lafayette.edu>

# Updated Fri Jun 21 11:07:54 EDT 1996
# NDBM support for ELF re-enabled by <kjahds@kjahds.com>

# No version of Linux supports setuid scripts.
d_suidsafe='undef'

# No version of Linux needs libutil for perl.
i_libutil='undef'

# Debian and Red Hat, and perhaps other vendors, provide both runtime and
# development packages for some libraries.  The runtime packages contain shared
# libraries with version information in their names (e.g., libgdbm.so.1.7.3);
# the development packages supplement this with versionless shared libraries
# (e.g., libgdbm.so).
#
# If you want to link against such a library, you must install the development
# version of the package.
#
# These packages use a -dev naming convention in both Debian and Red Hat:
#   libgdbmg1  (non-development version of GNU libc 2-linked GDBM library)
#   libgdbmg1-dev (development version of GNU libc 2-linked GDBM library)
# So make sure that for any libraries you wish to link Perl with under
# Debian or Red Hat you have the -dev packages installed.

# SuSE Linux can be used as cross-compilation host for Cray XT4 Catamount/Qk.
if test -d /opt/xt-pe
then
  case "`${cc:-cc} -V 2>&1`" in
  *catamount*) . hints/catamount.sh; return ;;
  esac
fi

# Some operating systems (e.g., Solaris 2.6) will link to a versioned shared
# library implicitly.  For example, on Solaris, `ld foo.o -lgdbm' will find an
# appropriate version of libgdbm, if one is available; Linux, however, doesn't
# do the implicit mapping.
ignore_versioned_solibs='y'

# BSD compatibility library no longer needed
# 'kaffe' has a /usr/lib/libnet.so which is not at all relevant for perl.
# bind causes issues with several reentrant functions
set `echo X "$libswanted "| sed -e 's/ bsd / /' -e 's/ net / /' -e 's/ bind / /'`
shift
libswanted="$*"

# Debian 4.0 puts ndbm in the -lgdbm_compat library.
echo $libs
if echo " $libswanted " | grep -q ' gdbm '; then
    # Only add if gdbm is in libswanted.
    libswanted="$libswanted gdbm_compat"
fi

# Configure may fail to find lstat() since it's a static/inline
# function in <sys/stat.h>.
d_lstat=define

# malloc wrap works
case "$usemallocwrap" in
'') usemallocwrap='define' ;;
esac

# The system malloc() is about as fast and as frugal as perl's.
# Since the system malloc() has been the default since at least
# 5.001, we might as well leave it that way.  --AD  10 Jan 2002
case "$usemymalloc" in
'') usemymalloc='n' ;;
esac

uname_minus_m="`$run uname -m 2>/dev/null`"
uname_minus_m="${uname_minus_m:-"$targetarch"}"

# Check if we're about to use Intel's ICC compiler
case "`${cc:-cc} -V 2>&1`" in
*"Intel(R) C"*" Compiler"*)
    # record the version, formats:
    # icc (ICC) 10.1 20080801
    # icpc (ICC) 10.1 20080801
    # followed by a copyright on the second line
    ccversion=`${cc:-cc} --version | sed -n -e 's/^icp\?c \((ICC) \)\?//p'`
    # This is needed for Configure's prototype checks to work correctly
    # The -mp flag is needed to pass various floating point related tests
    # The -no-gcc flag is needed otherwise, icc pretends (poorly) to be gcc
    ccflags="-we147 -mp -no-gcc $ccflags"
    # Prevent relocation errors on 64bits arch
    case "$uname_minus_m" in
	*ia64*|*x86_64*)
	    cccdlflags='-fPIC'
	;;
    esac
    # If we're using ICC, we usually want the best performance
    case "$optimize" in
    '') optimize='-O3' ;;
    esac
    ;;
*" Sun "*"C"*)
    # Sun's C compiler, which might have a 'tag' name between
    # 'Sun' and the 'C':  Examples:
    # cc: Sun C 5.9 Linux_i386 Patch 124871-01 2007/07/31
    # cc: Sun Ceres C 5.10 Linux_i386 2008/07/10
    test "$optimize" || optimize='-xO2'
    cccdlflags='-KPIC'
    lddlflags='-G -Bdynamic'
    # Sun C doesn't support gcc attributes, but, in many cases, doesn't
    # complain either.  Not all cases, though.
    d_attribute_format='undef'
    d_attribute_malloc='undef'
    d_attribute_nonnull='undef'
    d_attribute_noreturn='undef'
    d_attribute_pure='undef'
    d_attribute_unused='undef'
    d_attribute_warn_unused_result='undef'
    ;;
esac

case "$optimize" in
# use -O2 by default ; -O3 doesn't seem to bring significant benefits with gcc
'')
    optimize='-O2'
    case "$uname_minus_m" in
        ppc*)
            # on ppc, it seems that gcc (at least gcc 3.3.2) isn't happy
            # with -O2 ; so downgrade to -O1.
            optimize='-O1'
        ;;
        ia64*)
            # This architecture has had various problems with gcc's
            # in the 3.2, 3.3, and 3.4 releases when optimized to -O2.  See
            # RT #37156 for a discussion of the problem.
            case "`${cc:-gcc} -v 2>&1`" in
            *"version 3.2"*|*"version 3.3"*|*"version 3.4"*)
                ccflags="-fno-delete-null-pointer-checks $ccflags"
            ;;
            esac
        ;;
    esac
    ;;
esac

# Ubuntu 11.04 (and later, presumably) doesn't keep most libraries
# (such as -lm) in /lib or /usr/lib.  So we have to ask gcc to tell us
# where to look.  We don't want gcc's own libraries, however, so we
# filter those out.
# This could be conditional on Ubuntu, but other distributions may
# follow suit, and this scheme seems to work even on rather old gcc's.
# This unconditionally uses gcc because even if the user is using another
# compiler, we still need to find the math library and friends, and I don't
# know how other compilers will cope with that situation.
# Morever, if the user has their own gcc earlier in $PATH than the system gcc,
# we don't want its libraries. So we try to prefer the system gcc
# Still, as an escape hatch, allow Configure command line overrides to
# plibpth to bypass this check.
if [ -x /usr/bin/gcc ] ; then
    gcc=/usr/bin/gcc
# clang also provides -print-search-dirs
elif ${cc:-cc} --version 2>/dev/null | grep -q '^clang ' ; then
    gcc=${cc:-cc}
else
    gcc=gcc
fi

case "$plibpth" in
'') plibpth=`LANG=C LC_ALL=C $gcc $ccflags $ldflags -print-search-dirs | grep libraries |
	cut -f2- -d= | tr ':' $trnl | grep -v 'gcc' | sed -e 's:/$::'`
    set X $plibpth # Collapse all entries on one line
    shift
    plibpth="$*"
    ;;
esac

# For the musl libc, perl should #define _GNU_SOURCE.  Otherwise, some
# available functions, like memem, won't be used.  See the discussion in
# [perl #133760].  musl doesn't offer an easy way to identify it, but,
# at least on alpine linux, the ldd --version output contains the
# string 'musl.'
case `ldd --version 2>&1` in
    musl*)  ccflags="$ccflags -D_GNU_SOURCE" ;;
        *) ;;
esac

# libquadmath is sometimes installed as gcc internal library,
# so contrary to our usual policy of *not* looking at gcc internal
# directories we now *do* look at them, in case they contain
# the quadmath library.
# XXX This may apply to other gcc internal libraries, if such exist.
# XXX This could be at Configure level, but then the $gcc is messy.
case "$usequadmath" in
"$define")
  for d in `LANG=C LC_ALL=C $gcc $ccflags $ldflags -print-search-dirs | grep libraries | cut -f2- -d= | tr ':' $trnl | grep 'gcc' | sed -e 's:/$::'`
  do
    case `ls $d/*libquadmath*$so* 2>/dev/null` in
    $d/*libquadmath*$so*) xlibpth="$xlibpth $d" ;;
    esac
  done
  ;;
esac

case "$libc" in
'')
# If you have glibc, then report the version for ./myconfig bug reporting.
# (Configure doesn't need to know the specific version since it just uses
# gcc to load the library for all tests.)
# We don't use __GLIBC__ and  __GLIBC_MINOR__ because they
# are insufficiently precise to distinguish things like
# libc-2.0.6 and libc-2.0.7.
    for p in $plibpth
    do
        for trylib in libc.so.6 libc.so
        do
            if $test -e $p/$trylib; then
                libc=`ls -l $p/$trylib | awk '{print $NF}'`
                if $test "X$libc" != X; then
                    break
                fi
            fi
        done
        if $test "X$libc" != X; then
            break
        fi
    done
    ;;
esac

if ${sh:-/bin/sh} -c exit; then
  echo ''
  echo 'You appear to have a working bash.  Good.'
else
  cat << 'EOM' >&4

*********************** Warning! *********************
It would appear you have a defective bash shell installed. This is likely to
give you a failure of op/exec test #5 during the test phase of the build,
Upgrading to a recent version (1.14.4 or later) should fix the problem.
******************************************************
EOM

fi

# On SPARClinux,
# The following csh consistently coredumped in the test directory
# "/home/mikedlr/perl5.003_94/t", though not most other directories.

#Name        : csh                    Distribution: Red Hat Linux (Rembrandt)
#Version     : 5.2.6                        Vendor: Red Hat Software
#Release     : 3                        Build Date: Fri May 24 19:42:14 1996
#Install date: Thu Jul 11 16:20:14 1996 Build Host: itchy.redhat.com
#Group       : Shells                   Source RPM: csh-5.2.6-3.src.rpm
#Size        : 184417
#Description : BSD c-shell

# For this reason I suggest using the much bug-fixed tcsh for globbing
# where available.

# November 2001:  That warning's pretty old now and probably not so
# relevant, especially since perl now uses File::Glob for globbing.
# We'll still look for tcsh, but tone down the warnings.
# Andy Dougherty, Nov. 6, 2001
if $csh -c 'echo $version' >/dev/null 2>&1; then
    echo 'Your csh is really tcsh.  Good.'
else
    if xxx=`./UU/loc tcsh blurfl $pth`; $test -f "$xxx"; then
	echo "Found tcsh.  I'll use it for globbing."
	# We can't change Configure's setting of $csh, due to the way
	# Configure handles $d_portable and commands found in $loclist.
	# We can set the value for CSH in config.h by setting full_csh.
	full_csh=$xxx
    elif [ -f "$csh" ]; then
	echo "Couldn't find tcsh.  Csh-based globbing might be broken."
    fi
fi

# Shimpei Yamashita <shimpei@socrates.patnet.caltech.edu>
# Message-Id: <33EF1634.B36B6500@pobox.com>
#
# The DR2 of MkLinux (osname=linux,archname=ppc-linux) may need
# special flags passed in order for dynamic loading to work.
# instead of the recommended:
#
# ccdlflags='-rdynamic'
#
# it should be:
# ccdlflags='-Wl,-E'
#
# So if your DR2 (DR3 came out summer 1998, consider upgrading)
# has problems with dynamic loading, uncomment the
# following three lines, make distclean, and re-Configure:
#case "`uname -r | sed 's/^[0-9.-]*//'``arch`" in
#'osfmach3ppc') ccdlflags='-Wl,-E' ;;
#esac

case "$uname_minus_m" in
sparc*)
	case "$cccdlflags" in
	*-fpic*) cccdlflags="`echo $cccdlflags|sed 's/-fpic/-fPIC/'`" ;;
	*-fPIC*) ;;
	*)	 cccdlflags="$cccdlflags -fPIC" ;;
	esac
	;;
esac

# SuSE8.2 has /usr/lib/libndbm* which are ld scripts rather than
# true libraries. The scripts cause binding against static
# version of -lgdbm which is a bad idea. So if we have 'nm'
# make sure it can read the file
# NI-S 2003/08/07
case "$nm" in
    '') ;;
    *)
    for p in $plibpth
    do
        if $test -r $p/libndbm.so; then
            if $nm $p/libndbm.so >/dev/null 2>&1 ; then
                echo 'Your shared -lndbm seems to be a real library.'
                _libndbm_real=1
                break
            fi
        fi
    done
    if $test "X$_libndbm_real" = X; then
        echo 'Your shared -lndbm is not a real library.'
        set `echo X "$libswanted "| sed -e 's/ ndbm / /'`
        shift
        libswanted="$*"
    fi
    ;;
esac

# Linux on Synology.
if [ -f /etc/synoinfo.conf -a -d /usr/syno ]; then
    # Tested on Synology DS213 and DS413
    #  OS version info in /etc.defaults/VERSION
    #  http://forum.synology.com/wiki/index.php/What_kind_of_CPU_does_my_NAS_have
    # Synology DS213 running DSM 4.3-3810-0 (2013-11-06)
    #  CPU model Marvell Kirkwood mv6282 ARMv5te
    #  Linux 2.6.32.12 #3810 Wed Nov 6 05:13:41 CST 2013 armv5tel GNU/Linux
    # Synology DS413 running DSM 4.3-3810-0 (2013-11-06)
    #  CPU model Freescale QorIQ P1022 ppc (e500v2)
    #  linux 2.6.32.12 #3810 ppc GNU/Linux
    # All development stuff installed with ipkg is in /opt
    if [ "$LANG" = "" -o "$LANG" = "C" ]; then
	echo 'Your LANG is safe'
    else
	echo 'Please set $LANG to "C". All other $LANG settings will cause havoc' >&4
	LANG=C
    fi
    echo 'Setting up to use /opt/*' >&4
    locincpth="/opt/include $locincpth"
    libpth="/opt/lib $libpth"
    libspth="/opt/lib $libspth"
    loclibpth="/opt/lib $loclibpth"
    # POSIX will not link without the pthread lib
    libswanted="$libswanted pthread"
    echo "$libswanted" >&4
fi

# This script UU/usethreads.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use threads.
cat > UU/usethreads.cbu <<'EOCBU'
case "$usethreads" in
$define|true|[yY]*)
        ccflags="-D_REENTRANT -D_GNU_SOURCE $ccflags"
        if echo $libswanted | grep -v pthread >/dev/null
        then
            set `echo X "$libswanted "| sed -e 's/ c / pthread c /'`
            shift
            libswanted="$*"
        fi

	# Somehow at least in Debian 2.2 these manage to escape
	# the #define forest of <features.h> and <time.h> so that
	# the hasproto macro of Configure doesn't see these protos,
	# even with the -D_GNU_SOURCE.

	d_asctime_r_proto="$define"
	d_crypt_r_proto="$define"
	d_ctime_r_proto="$define"
	d_gmtime_r_proto="$define"
	d_localtime_r_proto="$define"
	d_random_r_proto="$define"

	;;
esac
EOCBU

cat > UU/uselargefiles.cbu <<'EOCBU'
# This script UU/uselargefiles.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use large files.
case "$uselargefiles" in
''|$define|true|[yY]*)
# Keep this in the left margin.
ccflags_uselargefiles="-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"

	ccflags="$ccflags $ccflags_uselargefiles"
	;;
esac
EOCBU

# Purify fails to link Perl if a "-lc" is passed into its linker
# due to duplicate symbols.
case "$PURIFY" in
$define|true|[yY]*)
    set `echo X "$libswanted "| sed -e 's/ c / /'`
    shift
    libswanted="$*"
    ;;
esac

# If using g++, the Configure scan for dlopen() and (especially)
# dlerror() might fail, easier just to forcibly hint them in.
case "$cc" in
*g++*)
  d_dlopen='define'
  d_dlerror='define'
  ;;
esac

# Under some circumstances libdb can get built in such a way as to
# need pthread explicitly linked.

libdb_needs_pthread="N"

if echo " $libswanted " | grep -v " pthread " >/dev/null
then
   if echo " $libswanted " | grep " db " >/dev/null
   then
     for DBDIR in $glibpth
     do
       DBLIB="$DBDIR/libdb.so"
       if [ -f $DBLIB ]
       then
         if ${nm:-nm} -u $DBLIB 2>/dev/null | grep pthread >/dev/null
         then
           if ldd $DBLIB | grep pthread >/dev/null
           then
             libdb_needs_pthread="N"
           else
             libdb_needs_pthread="Y"
           fi
         fi
       fi
     done
   fi
fi

case "$libdb_needs_pthread" in
  "Y")
    libswanted="$libswanted pthread"
    ;;
esac
