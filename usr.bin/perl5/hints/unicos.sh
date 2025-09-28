case `uname -r` in
6.1*) shellflags="-m+65536" ;;
esac

case "$optimize" in
# If we used fastmd (the default) integer values would be limited to 46 bits.
# --Mark P. Lutz
'') optimize="$optimize -O1 -h nofastmd" ;;
esac

# At least in the following environment
# uname -a: snxxxx xxxx 9.0.2.2 sin.0 CRAY Y-MP
# cc -V:    Cray Standard C Version 4.0.3  (057126) Jan 29 2006  07:27:26
# one has to drop optimisation from perl.c or otherwise
# the resulting miniperl executable does nothing (visible)
# but always exits with zero (success) exit status, this
# making it impossible to build the perl executable. --jhi
perl_cflags='optimize="-O0"'

# The default is to die in runtime on math overflows.
# Let's not do that. --jhi
ccflags="$ccflags -h matherror=errno" 

# Cray floating point (cfp) CPUs need -h rounddiv
# (It gives int((2/3)*3) a chance to be 2, not 1. --jhi)
# (but IEEE CPUs, IEEE/ieee/CPE1 CPUs should not have -h rounddiv,
#  since the compiler on those CPUs doesn't even support the option.)
if /etc/cpu -i | grep -q cfp
then
    ccflags="$ccflags -h rounddiv"
fi

# Avoid an optimizer bug where a volatile variables
# isn't correctly saved and restored --Mark P. Lutz 
pp_ctl_cflags='ccflags="$ccflags -h scalar0 -h vector0"'
# Otherwise the unpack %65c checksums will fail.
pp_pack_cflags='optimize="$ccflags -h scalar0 -h vector0"'
case "$usemymalloc" in
'') # The perl malloc.c SHOULD work says Ilya.
    # But for the time being (5.004_68), alas, it doesn't. --jhi
    # usemymalloc='y'
    # ccflags="$ccflags -DNO_RCHECK"
    usemymalloc='n'
    ;;
esac
# Configure gets fooled for some reason, these do not exist.
d_getpgid='undef'
d_setitimer='undef'
# These exist but do not really work.
d_setregid='undef'
d_setreuid='undef'
# No shared libraries.
so='none'
# No dynaloading.
d_dlopen='undef'
i_dlfcn='undef'
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
