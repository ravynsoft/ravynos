#! /bin/sh
# cygwin.sh - hints for building perl using the Cygwin environment for Win32
#

# not otherwise settable
exe_ext='.exe'
firstmakefile='GNUmakefile'
case "$ldlibpthname" in
'') ldlibpthname=PATH ;;
esac
archobjs='cygwin.o'

# mandatory (overrides incorrect defaults)
test -z "$cc" && cc='gcc'
if test -z "$plibpth"
then
    plibpth=`gcc -print-file-name=libc.a`
    plibpth=`dirname $plibpth`
    plibpth=`cd $plibpth && pwd`
fi
so='dll'
# - eliminate -lc, implied by gcc and a symlink to libcygwin.a
libswanted=`echo " $libswanted " | sed -e 's/ c / /g'`
# - eliminate -lm, symlink to libcygwin.a
libswanted=`echo " $libswanted " | sed -e 's/ m / /g'`
# - eliminate -lutil, symbols are all in libcygwin.a
libswanted=`echo " $libswanted " | sed -e 's/ util / /g'`
test -z "$ignore_versioned_solibs" && ignore_versioned_solibs='y'
test -z "$usenm" && usenm='no'
test -z "$libc" && libc='/usr/lib/libcygwin.a'
test -z "$loclibpth" && loclibpth=' '
test -z "$glibpth" && glibpth=' '
test -z "$plibpth" && plibpth=' '
test -z "$libpth" && libpth=' '
PATH='.:/usr/bin/'
# - add libgdbm_compat $libswanted
libswanted="$libswanted gdbm_compat"
test -z "$optimize" && optimize='-O3'
man3ext='3pm'
test -z "$use64bitint" && use64bitint='define'
test -z "$useithreads" && useithreads='define'
test -z "$usemymalloc" && usemymalloc='undef'
ccflags="$ccflags -U__STRICT_ANSI__ -D_GNU_SOURCE"
# - otherwise i686-cygwin
archname='cygwin'

# dynamic loading
# - otherwise -fpic
cccdlflags=' '
lddlflags=' --shared'
test -z "$ld" && ld='g++'

case "$osvers" in
    # Configure gets these wrong if the IPC server isn't yet running:
    # only use for 1.5.7 and onwards
    [2-9]*|1.[6-9]*|1.[1-5][0-9]*|1.5.[7-9]*|1.5.[1-6][0-9]*)
        d_semctl_semid_ds='define'
        d_semctl_semun='define'
        ;;
esac

case "$osvers" in
    [2-9]*|1.[6-9]*)
        # IPv6 only since 1.7
        d_inetntop='define'
        d_inetpton='define'
        ;;
    *)
        # IPv6 not implemented before cygwin-1.7
        d_inetntop='undef'
        d_inetpton='undef'
esac

# compile Win32CORE "module" as static. try to avoid the space.
if test -z "$static_ext"; then
  static_ext="Win32CORE"
else
  static_ext="$static_ext Win32CORE"
fi

# Win9x problem with non-blocking read from a closed pipe
d_eofnblk='define'

# suppress auto-import warnings
ldflags="$ldflags -Wl,--enable-auto-import -Wl,--export-all-symbols -Wl,--enable-auto-image-base"
lddlflags="$lddlflags $ldflags"

# strip exe's and dll's, better do it afterwards
#ldflags="$ldflags -s"
#ccdlflags="$ccdlflags -s"
#lddlflags="$lddlflags -s"

# Seems that exporting _Thread_local doesn't work on cygwin. This 6 year old
# gcc bug suggests that maybe the problem really is binutils, but either way
# it still doesn't work, despite our probes looking good:
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64697
d_thread_local=undef

# Broken: https://sourceware.org/pipermail/cygwin/2022-August/252043.html */
d_newlocale=undef
