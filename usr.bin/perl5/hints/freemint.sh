# hints/freemint.sh
#
# Contact alanh@freemint.org if you want to change this file.

archname="m68k-freemint"

cccdlflags=' '; # avoid -fPIC
ccdlflags="-Wl,-whole-archive -lgcc -lpthread -Wl,-no-whole-archive"

# libs
libpth="$prefix/lib /usr/local/lib /usr/lib"
glibpth="$libpth"
xlibpth="$libpth"

ccflags="$ccflags -D_GNU_SOURCE"
libswanted='m dld'
dl_src='dl_freemint.xs'
dlext='o'
lddlflags='-r'
ldflags='-static'
so='none'
useshrplib='false'

case "$usemymalloc" in
'') usemymalloc='n' ;;
esac
#sbrk() returns -1 (failure) somewhere in lib/unicore/mktables at
#around 14M, so we need to use system malloc() as our sbrk()
malloc_cflags='ccflags="-DUSE_PERL_SBRK -DPERL_SBRK_VIA_MALLOC $ccflags"'

# Locales aren't feeling well.
LC_ALL=C; export LC_ALL;
LANG=C; export LANG;

# We crash if -Ox used.
locale_cflags='optimize="-O0"'
