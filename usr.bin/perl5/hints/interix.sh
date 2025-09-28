# hints/interix.sh
#
# Please check with tech-pkg@netbsd.org before making modifications
# to this file.

cc='gcc'
cccdlflags="-DPIC $cccdlflags"
ccdlflags='-Wl,-E'
ccflags="-D_ALL_SOURCE $ccflags"
d_poll="$undef"
ld='gcc'
lddlflags="-shared $lddlflags"
rpathflag='-Wl,-R'
sharpbang='#!'
usemymalloc='false'
usenm='false'
plibpth=''

case "$plibpth" in
'') plibpth=`LANG=C LC_ALL=C $cc -print-search-dirs | grep libraries |
        cut -f2- -d= | tr ':' $trnl | grep -v 'gcc' | sed -e 's:/$::'`
    set X $plibpth # Collapse all entries on one line
    shift
    plibpth="$*"
    ;;
esac

# This script UU/usethreads.cbu will get 'called-back' by Configure 
# after it has prompted the user for whether to use threads. 
cat > UU/usethreads.cbu <<'EOCBU' 
case "$usethreads" in
$define|true|[yY]*)
	ccflags="-D_REENTRANT $ccflags"
	libswanted="$libswanted pthread"
        ;; 
esac 
EOCBU
