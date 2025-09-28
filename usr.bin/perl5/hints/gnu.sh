# hints/gnu.sh
# Originally contributed by:  Mark Kettenis <kettenis@phys.uva.nl> Dec 10 1998

# libnsl is unusable on the Hurd.
# XXX remove this once SUNRPC is implemented.
set `echo X "$libswanted "| sed -e 's/ bsd / /' -e 's/ nsl / /' -e 's/ c / pthread /'`
shift
libswanted="$*"

# Debian 4.0 puts ndbm in the -lgdbm_compat library.
libswanted="$libswanted gdbm_compat"

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

case "$optimize" in
'') optimize='-O2' ;;
esac

case "$plibpth" in
'') plibpth=`gcc -print-search-dirs | grep libraries |
        cut -f2- -d= | tr ':' $trnl | grep -v 'gcc' | sed -e 's:/$::'`
    set X $plibpth # Collapse all entries on one line
    shift
    plibpth="$*"
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
        for trylib in libc.so.0.3 libc.so
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

# Flags needed to produce shared libraries.
lddlflags='-shared'

# Flags needed by programs that use dynamic linking.
ccdlflags='-Wl,-E'

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

# The following routines are only available as stubs in GNU libc.
# XXX remove this once metaconf detects the GNU libc stubs.
d_msgctl='undef'
d_msgget='undef'
d_msgrcv='undef'
d_msgsnd='undef'
d_semctl='undef'
d_semget='undef'
d_semop='undef'
d_shmat='undef'
d_shmctl='undef'
d_shmdt='undef'
d_shmget='undef'
