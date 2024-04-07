# $MirOS: src/gnu/usr.bin/perl/hints/mirbsd.sh,v 1.3 2007/05/07 20:32:09 tg Exp $
#
# hints file for MirOS by Thorsten Glaser <tg@mirbsd.de>
# based upon hints for OpenBSD

[ -z "$cc" ] && cc="${CC:-mgcc}"

# MirOS has a better malloc than perl...
usemymalloc='n'

# Added by bingos. 
loclibpth="/usr/mpkg/lib /usr/local/lib"
locincpath="/usr/mpkg/include /usr/local/include"
ccflags="$ccflags -fhonour-copts -I/usr/mpkg/include"
cppflags="$cppflags -fhonour-copts -I/usr/mpkg/include"

# Currently, vfork(2) is not a real win over fork(2).
usevfork="$undef"

test -z "$usedl" && usedl=$define
# We use -fPIC here because -fpic is *NOT* enough for some of the
# extensions like Tk on some platforms (ie: sparc)
cccdlflags="-DPIC -fPIC $cccdlflags"
ld=$cc
lddlflags="-shared -fPIC $lddlflags"
libswanted=$(echo $libswanted | sed 's/ dl / /')

# We need to force ld to export symbols on ELF platforms.
# Without this, dlopen() is crippled. All platforms are ELF.
ldflags="-Wl,-E $ldflags"

# malloc wrap works
case "$usemallocwrap" in
'') usemallocwrap='define' ;;
esac

# MirOS doesn't need libcrypt
libswanted=$(echo $libswanted | sed 's/ crypt / /')

# Configure can't figure this out non-interactively
d_suidsafe=$define

# cc is gcc so we can do better than -O
# Allow a command-line override, such as -Doptimize=-g
test "$optimize" || optimize='-O2'

# This script UU/usethreads.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use threads.
cat > UU/usethreads.cbu <<'EOCBU'
case "$usethreads" in
$define|true|[yY]*)
	ccflags="-pthread $ccflags"
	ldflags="-pthread $ldflags"
esac
EOCBU

# When building in the MirOS tree we use different paths
# This is only part of the story, the rest comes from config.over
case "$mirbsd_distribution" in
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
	if [ "$usedl" = "$define" -a -r shlib_version ]; then
		useshrplib=true
		libperl=$(. ./shlib_version; echo libperl.so.${major}.${minor})
	fi
	;;
esac

# end
