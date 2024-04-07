##
# Darwin (Mac OS) hints
# Wilfredo Sanchez <wsanchez@wsanchez.net>
##

##
# Paths
##

# Configure hasn't figured out the version number yet.  Bummer.
perl_revision=`awk '/define[ 	]+PERL_REVISION/ {print $3}' $src/patchlevel.h`
perl_version=`awk '/define[ 	]+PERL_VERSION/ {print $3}' $src/patchlevel.h`
perl_subversion=`awk '/define[ 	]+PERL_SUBVERSION/ {print $3}' $src/patchlevel.h`
version="${perl_revision}.${perl_version}.${perl_subversion}"

# Pretend that Darwin doesn't know about those system calls in Tiger
# (10.4/darwin 8) and earlier [perl #24122]
case "$osvers" in
[1-8].*)
    d_setregid='undef'
    d_setreuid='undef'
    d_setrgid='undef'
    d_setruid='undef'
    ;;
esac

# finite() deprecated in 10.9, use isfinite() instead.
case "$osvers" in
[1-8].*) ;;
*) d_finite='undef' ;;
esac

# This was previously used in all but causes three cases
# (no -Ddprefix=, -Dprefix=/usr, -Dprefix=/some/thing/else)
# but that caused too much grief.
# vendorlib="/System/Library/Perl/${version}"; # Apple-supplied modules

case "$darwin_distribution" in
$define) # We are building/replacing the built-in perl
	prefix='/usr';
	installprefix='/usr';
	bin='/usr/bin';
	siteprefix='/usr/local';
	# We don't want /usr/bin/HEAD issues.
	sitebin='/usr/local/bin';
	sitescript='/usr/local/bin';
	installusrbinperl='define'; # You knew what you were doing.
	privlib="/System/Library/Perl/${version}";
	sitelib="/Library/Perl/${version}";
	vendorprefix='/';
	usevendorprefix='define';
	vendorbin='/usr/bin';
	vendorscript='/usr/bin';
	vendorlib="/Network/Library/Perl/${version}";
	# 4BSD uses ${prefix}/share/man, not ${prefix}/man.
	man1dir='/usr/share/man/man1';
	man3dir='/usr/share/man/man3';
	# But users' installs shouldn't touch the system man pages.
	# Transient obsoleted style.
	siteman1='/usr/local/share/man/man1';
	siteman3='/usr/local/share/man/man3';
	# New style.
	siteman1dir='/usr/local/share/man/man1';
	siteman3dir='/usr/local/share/man/man3';
	;;
esac

##
# Tool chain settings
##

# Since we can build fat, the archname doesn't need the processor type
archname='darwin';

# nm isn't known to work after Snow Leopard and XCode 4; testing with OS X 10.5
# and Xcode 3 shows a working nm, but pretending it doesn't work produces no
# problems.
usenm='false';

case "$optimize" in
'')
#    Optimizing for size also mean less resident memory usage on the part
# of Perl.  Apple asserts that this is a more important optimization than
# saving on CPU cycles.  Given that memory speed has not increased at
# pace with CPU speed over time (on any platform), this is probably a
# reasonable assertion.
if [ -z "${optimize}" ]; then
  case "`${cc:-gcc} -v 2>&1`" in
    *"gcc version 3."*) optimize='-Os' ;;
    *) optimize='-O3' ;;
  esac
else
  optimize='-O3'
fi
;;
esac

# -fno-common because common symbols are not allowed in MH_DYLIB
# -DPERL_DARWIN: apparently the __APPLE__ is not sanctioned by Apple
# as the way to differentiate Mac OS X.  (The official line is that
# *no* cpp symbol does differentiate Mac OS X.)
ccflags="${ccflags} -fno-common -DPERL_DARWIN"

# At least on Darwin 1.3.x:
#
# # define INT32_MIN -2147483648
# int main () {
#  double a = INT32_MIN;
#  printf ("INT32_MIN=%g\n", a);
#  return 0;
# }
# will output:
# INT32_MIN=2.14748e+09
# Note that the INT32_MIN has become positive.
# INT32_MIN is set in /usr/include/stdint.h by:
# #define INT32_MIN        -2147483648
# which seems to break the gcc.  Defining INT32_MIN as (-2147483647-1)
# seems to work.  INT64_MIN seems to be similarly broken.
# -- Nicholas Clark, Ken Williams, and Edward Moy
#
# This seems to have been fixed since at least Mac OS X 10.1.3,
# stdint.h defining INT32_MIN as (-INT32_MAX-1)
# -- Edward Moy
#
if test -f /usr/include/stdint.h; then
  case "$(grep '^#define INT32_MIN' /usr/include/stdint.h)" in
  *-2147483648) ccflags="${ccflags} -DINT32_MIN_BROKEN -DINT64_MIN_BROKEN" ;;
  esac
fi

# Avoid Apple's cpp precompiler, better for extensions
if [ "X`echo | ${cc} -no-cpp-precomp -E - 2>&1 >/dev/null`" = "X" ]; then
    cppflags="${cppflags} -no-cpp-precomp"

    # This is necessary because perl's build system doesn't
    # apply cppflags to cc compile lines as it should.
    ccflags="${ccflags} ${cppflags}"
fi

# Known optimizer problems.
case "`cc -v 2>&1`" in
  *"3.1 20020105"*) toke_cflags='optimize=""' ;;
esac

# Shared library extension is .dylib.
# Bundle extension is .bundle.
so='dylib';
dlext='bundle';
usedl='define';

# 10.4 can use dlopen.
# 10.4 broke poll().
case "$osvers" in
[1-7].*)
    dlsrc='dl_dyld.xs';
    ;;
*)
    dlsrc='dl_dlopen.xs';
    d_poll='undef';
    i_poll='undef';
    ;;
esac

case "$ccdlflags" in		# If passed in from command line, presume user knows best
'')
   cccdlflags=' '; # space, not empty, because otherwise we get -fpic
;;
esac

# Allow the user to override ld, but modify it as necessary below
case "$ld" in
    '') case "$cc" in
        # If the cc is explicitly something else than cc (or empty),
        # set the ld to be that explicitly something else.  Conversely,
        # if the cc is 'cc' (or empty), set the ld to be 'cc'.
        cc|'') ld='cc';;
        *) ld="$cc" ;;
        esac
        ;;
esac

# From http://ftp.netbsd.org/pub/pkgsrc/current/pkgsrc/mk/platform/Darwin.mk
# and https://trac.macports.org/wiki/XcodeVersionInfo
# and https://trac.macports.org/wiki/UsingTheRightCompiler
# and https://gist.github.com/yamaya/2924292
# and http://opensource.apple.com/source/clang/
#
# Note that Xcode gets updates on older systems sometimes, and in
# general that the OS levels and XCode levels are not synchronized
# since new releases of XCode usually support both some new and some
# old OS releases.
#
# Note that Apple hijacks the clang preprocessor symbols __clang_major__
# and __clang_minor__ so they cannot be used (easily) to detect the
# actual clang release.  For example:
#
# "Yosemite 10.10.x 14.x.y 6.3 (clang 3.6 as 6.1/602.0.49)"
#
# means that the Xcode 6.3 provided the clang 6.3 but called it 6.1
# (__clang_major__, __clang_minor__) and in addition the preprocessor
# symbol __apple_build_version__ was 6020049.
#
# Codename        OS      Kernel  Xcode
#
# Cheetah         10.0.x  1.3.1
# Puma            10.1    1.4.1
#                 10.1.x  5.x.y
# Jaguar          10.2.x  6.x.y
# Panther         10.3.x  7.x.y
# Tiger           10.4.x  8.x.y   2.0   (gcc4 4.0.0)
#                                 2.2   (gcc4 4.0.1)
#                                 2.2.1 (gcc 3.3)
#                                 2.5 ?
# Leopard         10.5.x  9.x.y   3.0   (gcc 4.0.1 default)
#                                 3.1   (gcc 4.2.1)
# Snow Leopard    10.6.x  10.x.y  3.2   (llvm gcc 4.2, clang 2.3 as 1.0)
#                                 3.2.1 (clang 1.0.1 as 1.0.1/24)
#                                 3.2.2 (clang 1.0.2 as 1.0.2/32)
#                                 3.2.3 (clang 1.5 as 1.5/60)
#                                 4.0.1 (clang 2.9 as 2.0/138)
# Lion            10.7.x  11.x.y  4.1   (llvm gcc 4.2.1, clang 3.0 as 2.1/163.7.1)
#                                 4.2   (clang 3.0 as 3.0/211.10.1)
#                                 4.3.3 (clang 3.1 as 3.1/318.0.61)
#                                 4.4   (clang 3.1 as 4.0/421.0.57)
# Mountain Lion   10.8.x  12.x.y  4.5   (clang 3.1 as 4.1/421.11.65, real gcc removed, there is gcc but it's really clang)
#                                 4.6   (clang 3.2 as 4.2/425.0.24)
#                                 5.0   (clang 3.3 as 5.0/500.2.75)
#                                 5.1   (clang 3.4 as 5.1/503.0.38)
#                                 5.1.1 (clang 3.4 as 5.1/503.0.40)
# Mavericks       10.9.x  13.x.y  6.0.1 (clang 3.5 as 6.0/600.0.51)
#                                 6.1   (clang 3.5 as 6.0/600.0.54)
#                                 6.1.1 (clang 3.5 as 6.0/600.0.56)
#                                 6.2   (clang 3.5 as 6.0/600.0.57)
# Yosemite        10.10.x 14.x.y  6.3   (clang 3.6 as 6.1/602.0.49)
#                                 6.3.1 (clang 3.6 as 6.1/602.0.49)
#                                 6.3.2 (clang 3.6 as 6.1/602.0.53)
# El Capitan      10.11.x 15.x.y  7.0   (clang 3.7 as 7.0/700.0.72)
#                                 7.1   (clang 3.7 as 7.0/700.1.76)
#                                 7.2   (clang 3.7 as 7.0.2/700.1.81)
#                                 7.2.1 (clang 3.7 as 7.0.2/700.1.81)
#                                 7.3   (clang 3.8 as 7.3.0/703.0.29)
# Sierra          10.12.x 16.x.y  8.0.0 (clang 3.8 as 8.0/800.0.38)
#

# Processors Supported
#
# PowerPC (PPC):       10.0.x - 10.5.8 (final 10.5.x)
# PowerPC via Rosetta: 10.4.4 - 10.6.8 (final 10.6.x)
# IA-32:               10.4.4 - 10.6.8 (though still supported on x86-64)
# x86-64:              10.4.7 - current

# MACOSX_DEPLOYMENT_TARGET selects the minimum OS level we want to support
#
# It is needed for OS releases before 10.6.
#
# https://developer.apple.com/library/mac/documentation/DeveloperTools/Conceptual/cross_development/Configuring/configuring.html
#
# If it is set, we also propagate its value to ccflags and ldflags
# using the -mmacosx-version-min flag.  If it is not set, we use
# the OS X release as the min value for the flag.

# Adds "-mmacosx-version-min=$2" to "$1" unless it already is there.
add_macosx_version_min () {
  local v
  eval "v=\$$1"
  case " $v " in
  *"-mmacosx-version-min"*)
     echo "NOT adding -mmacosx-version-min=$2 to $1 ($v)" >&4
     ;;
  *) echo "Adding -mmacosx-version-min=$2 to $1" >&4
     eval "$1='$v -mmacosx-version-min=$2'"
     ;;
  esac
}

# Perl bundles do not expect two-level namespace, added in Darwin 1.4.
# But starting from perl 5.8.1/Darwin 7 the default is the two-level.
case "$osvers" in  # Note: osvers is the kernel version, not the 10.x
1.[0-3].*) # OS X 10.0.x
   lddlflags="${ldflags} -bundle -undefined suppress"
   ;;
1.*)       # OS X 10.1
   ldflags="${ldflags} -flat_namespace"
   lddlflags="${ldflags} -bundle -undefined suppress"
   ;;
[2-6].*)   # OS X 10.1.x - 10.2.x (though [2-4] never existed publicly)
   ldflags="${ldflags} -flat_namespace"
   lddlflags="${ldflags} -bundle -undefined suppress"
   ;;
[7-9].*)   # OS X 10.3.x - 10.5.x
   lddlflags="${ldflags} -bundle -undefined dynamic_lookup"
   case "$ld" in
       *MACOSX_DEPLOYMENT_TARGET*) ;;
       *) ld="env MACOSX_DEPLOYMENT_TARGET=10.3 ${ld}" ;;
   esac
   ;;
*)        # OS X 10.6.x - current
   # The MACOSX_DEPLOYMENT_TARGET is not needed,
   # but the -mmacosx-version-min option is always used.

   # We now use MACOSX_DEPLOYMENT_TARGET, if set, as an override by
   # capturing its value and adding it to the flags.
    case "$MACOSX_DEPLOYMENT_TARGET" in
    [1-9][0-9].*)
      add_macosx_version_min ccflags $MACOSX_DEPLOYMENT_TARGET
      add_macosx_version_min ldflags $MACOSX_DEPLOYMENT_TARGET
      ;;
    '')
      # Empty MACOSX_DEPLOYMENT_TARGET is okay.
      ;;
    *)
      cat <<EOM >&4

*** Unexpected MACOSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET
***
*** Please either set it to a valid macOS version number (e.g., 10.15) or to empty.

EOM
      exit 1
      ;;
    esac

    # Keep the prodvers leading whitespace (Configure magic).
    # Cannot use $osvers here since that is the kernel version.
    # sw_vers output                 what we want
    # "ProductVersion:    10.10.5"   "10.10"
    # "ProductVersion:    10.11"     "10.11"
        prodvers=`sw_vers|awk '/^ProductVersion:/{print $2}'|awk -F. '{print $1"."$2}'`
    case "$prodvers" in
    [1-9][0-9].*)
      add_macosx_version_min ccflags $prodvers
      add_macosx_version_min ldflags $prodvers
      ;;
    *)
      cat <<EOM >&4

*** Unexpected product version $prodvers.
***
*** Try running sw_vers and see what its ProductVersion says.

EOM
      exit 1
    esac

    darwin_major=$(echo $osvers|awk -F. '{print $1}')

    # macOS 10.12 (darwin 16.0.0) deprecated syscall().
    if [ "$darwin_major" -ge 16 ]; then
        d_syscall='undef'
        # If deploying to pre-10.12, suppress Time::HiRes's detection of the system clock_gettime()
        case "$MACOSX_DEPLOYMENT_TARGET" in
          10.[6-9]|10.10|10.11)
          ccflags="$ccflags -Werror=partial-availability -D_DARWIN_FEATURE_CLOCK_GETTIME=0"
          ;;
        *)
          ;;
        esac
    fi

    # The OS is buggy with respect to this.
    ccflags="$ccflags -DNO_POSIX_2008_LOCALE"

   lddlflags="${ldflags} -bundle -undefined dynamic_lookup"
   ;;
esac

ldlibpthname='DYLD_LIBRARY_PATH';

# useshrplib=true results in much slower startup times.
# 'false' is the default value.  Use Configure -Duseshrplib to override.

cat > UU/archname.cbu <<'EOCBU'
# This script UU/archname.cbu will get 'called-back' by Configure 
# after it has otherwise determined the architecture name.
case "$ldflags" in
*"-flat_namespace"*) ;; # Backward compat, be flat.
# If we are using two-level namespace, we will munge the archname to show it.
*) archname="${archname}-2level" ;;
esac
EOCBU

# 64-bit addressing support. Currently strictly experimental. DFD 2005-06-06
case "$use64bitall" in
$define|true|[yY]*)
case "$osvers" in
[1-7].*)
     cat <<EOM >&4



*** 64-bit addressing is not supported for Mac OS X versions
*** below 10.4 ("Tiger") or Darwin versions below 8. Please try
*** again without -Duse64bitall. (-Duse64bitint will work, however.)

EOM
     exit 1
  ;;
*)
    case "$osvers" in
    8.*)
        cat <<EOM >&4



*** Perl 64-bit addressing support is experimental for Mac OS X
*** 10.4 ("Tiger") and Darwin version 8. System V IPC is disabled
*** due to problems with the 64-bit versions of msgctl, semctl,
*** and shmctl. You should also expect the following test failures:
***
***    ext/threads-shared/t/wait (threaded builds only)

EOM

        [ "$d_msgctl" ] || d_msgctl='undef'
        [ "$d_semctl" ] || d_semctl='undef'
        [ "$d_shmctl" ] || d_shmctl='undef'
    ;;
    esac

    case `uname -p` in 
    powerpc) arch=ppc64 ;;
    i386) arch=x86_64 ;;
    *) cat <<EOM >&4

*** Don't recognize processor, can't specify 64 bit compilation.

EOM
    ;;
    esac
    for var in ccflags cppflags ld ldflags
    do
       eval $var="\$${var}\ -arch\ $arch"
    done

    ;;
esac
;;
esac

##
# System libraries
##

# vfork works
usevfork='true';

# malloc wrap works
case "$usemallocwrap" in
'') usemallocwrap='define' ;;
esac

# our malloc works (but allow users to override)
case "$usemymalloc" in
'') usemymalloc='n' ;;
esac
# However sbrk() returns -1 (failure) somewhere in lib/unicore/mktables at
# around 14M, so we need to use system malloc() as our sbrk()
#
# sbrk() in Darwin deprecated since Mavericks (10.9), it still exists
# in Yosemite (10.10) but that is just an emulation, and fails for
# allocations beyond 4MB.  One should use e.g. mmap instead (or system
# malloc, as suggested above, that but is kind of backward).
malloc_cflags='ccflags="-DUSE_PERL_SBRK -DPERL_SBRK_VIA_MALLOC $ccflags"'

# Locales aren't feeling well.
LC_ALL=C; export LC_ALL;
LANG=C; export LANG;

#
# The libraries are not threadsafe as of OS X 10.1.
#
# Fix when Apple fixes libc.
#
case "$usethreads$useithreads" in
  *define*)
  case "$osvers" in
    [12345].*)     cat <<EOM >&4



*** Warning, there might be problems with your libraries with
*** regards to threading.  The test ext/threads/t/libc.t is likely
*** to fail.

EOM
    ;;
    *) usereentrant='define';;
  esac

esac

# Fink can install a GDBM library that claims to have the ODBM interfaces
# but Perl dynaloader cannot for some reason use that library.  We don't
# really need ODBM_FIle, though, so let's just hint ODBM away.
i_dbm=undef;

# Configure doesn't detect ranlib on Tiger properly.
# NeilW says this should be acceptable on all darwin versions.
ranlib='ranlib'

# Catch MacPorts gcc/g++ extra libdir
case "$($cc -v 2>&1)" in
*"MacPorts gcc"*) loclibpth="$loclibpth /opt/local/lib/libgcc" ;;
esac

##
# Build process
##

# Case-insensitive filesystems don't get along with Makefile and
# makefile in the same place.  Since Darwin uses GNU make, this dodges
# the problem.
firstmakefile=GNUmakefile;

# if you use a newer toolchain before OS X 10.9 these functions may be
# incorrectly detected, so disable them
# OS X 10.10.x corresponds to kernel 14.x
case "$osvers" in
    [1-9].*|1[0-3].*)
	d_linkat=undef
	d_openat=undef
	d_renameat=undef
	d_unlinkat=undef
	d_fchmodat=undef
	;;
esac

# mkostemp() was autodetected as present but found to not be linkable
# on 15.6.0.  Unknown what other OS versions are affected.
d_mkostemp=undef

# Apparently the MACH-O format can't support _Thread_local in shared objects,
# but clang isn't wise to this, so our probe works but the build fails...
d_thread_local=undef
