case "$optimize" in
'') optimize="-O1" ;;
esac
d_setregid='undef'
d_setreuid='undef'
case "$usemymalloc" in
'') usemymalloc='y'
    ccflags="$ccflags -DNO_RCHECK"
    ;;
esac
# If somebody ignores the Cray PATH.
case ":$PATH:" in
*:/opt/ctl/bin:*) ;;
'') case "$cc" in
    '') test -x /opt/ctl/bin/cc && cc=/opt/ctl/bin/cc ;;
    esac
    ;;
esac
# As of UNICOS/mk 2.0.5.24 the shm* are in libc but unimplemented
# (an attempt to use them causes a runtime error)
# XXX Configure probe for really functional shm*() is needed XXX
if test "$d_shm" = ""; then
    d_shmat=${d_shmat:-undef}
    d_shmdt=${d_shmdt:-undef}
    d_shmget=${d_shmget:-undef}
    d_shmctl=${d_shmctl:-undef}
    case "$d_shmat$d_shmctl$d_shmdt$d_shmget" in
    *"undef"*) d_shm="$undef" ;;
    esac
fi
# Otherwise the unpack %65c checksums will fail.
pp_pack_cflags='optimize="-h scalar0 -h vector0"'
# No shared libraries.
so='none'
# Threads call-back unit.
cat > UU/usethreads.cbu <<'EOCBU'
# This script UU/usethreads.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use threads.
case "$usethreads" in
$define|true|[yY]*)
        set `echo X "$libswanted "| sed -e "s/ c / pthread c /"`
        shift
        libswanted="$*"
	;;
esac
EOCBU
