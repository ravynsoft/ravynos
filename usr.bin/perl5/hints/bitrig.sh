# hints/openbsd.sh
#
# hints file for OpenBSD; Todd Miller <millert@openbsd.org>
# Edited to allow Configure command-line overrides by
#  Andy Dougherty <doughera@lafayette.edu>
#
# To build with distribution paths, use:
#	./Configure -des -Dopenbsd_distribution=defined
#

# OpenBSD has a better malloc than perl...
test "$usemymalloc" || usemymalloc='n'

# Currently, vfork(2) is not a real win over fork(2).
usevfork="$undef"

#
# Not all platforms support dynamic loading...
# For the case of "$openbsd_distribution", the hints file
# needs to know whether we are using dynamic loading so that
# it can set the libperl name appropriately.
# Allow command line overrides.
#
#ARCH=`arch | sed 's/^OpenBSD.//'`
ARCH=`arch | sed 's/^Bitrig.//'`
case "${ARCH}-${osvers}" in
*)
	test -z "$usedl" && usedl=$define
	# We use -fPIC here because -fpic is *NOT* enough for some of the
	# extensions like Tk on some OpenBSD platforms (ie: sparc)
	cccdlflags="-DPIC -fPIC $cccdlflags"
	case "$osvers" in
	*) # from 3.1 onwards
		ld=${cc:-cc}
		lddlflags="-shared -fPIC $lddlflags"
		libswanted=`echo $libswanted | sed 's/ dl / /'`
		;;
	esac

	# We need to force ld to export symbols on ELF platforms.
	# Without this, dlopen() is crippled.
	ELF=`${cc:-cc} -dM -E - </dev/null | grep __ELF__`
	test -n "$ELF" && ldflags="-Wl,-E $ldflags"
	;;
esac

# malloc wrap causes problems on m68k
if [ X"$usemallocwrap" = X"" ]; then
	case "${ARCH}" in
	*)    usemallocwrap="define" ;;
	esac
fi

# OpenBSD doesn't need libcrypt but many folks keep a stub lib
# around for old NetBSD binaries.
libswanted=`echo $libswanted | sed 's/ crypt / /'`

# Configure can't figure this out non-interactively
d_suidsafe=$define

# cc is gcc so we can do better than -O
# Allow a command-line override, such as -Doptimize=-g
case "${ARCH}-${osvers}" in
*)
   test "$optimize" || optimize='-O2'
   ;;
esac

# This script UU/usethreads.cbu will get 'called-back' by Configure 
# after it has prompted the user for whether to use threads.
cat > UU/usethreads.cbu <<'EOCBU'
case "$usethreads" in
$define|true|[yY]*)
	# any openbsd version dependencies with pthreads?
	ccflags="-pthread $ccflags"
	ldflags="-pthread $ldflags"
esac
EOCBU

# When building in the OpenBSD tree we use different paths
# This is only part of the story, the rest comes from config.over
case "$openbsd_distribution" in
''|$undef|false) ;;
*)
	# We put things in /usr, not /usr/local
	prefix='/usr'
	prefixexp='/usr'
	sysman='/usr/share/man/man1'
	libpth='/usr/lib'
	glibpth='/usr/lib'
	# Local things, however, do go in /usr/local
	siteprefix='/usr/local'
	siteprefixexp='/usr/local'
	# Ports installs non-std libs in /usr/local/lib so look there too
	locincpth='/usr/local/include'
	loclibpth='/usr/local/lib'
	# Link perl with shared libperl
	if [ "$usedl" = "$define" -a -r $src/shlib_version ]; then
		useshrplib=true
		libperl=`. $src/shlib_version; echo libperl.so.${major}.${minor}`
	fi
	;;
esac

# end
