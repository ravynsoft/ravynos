# hints/netbsd.sh
#
# Please check with packages@netbsd.org before making modifications
# to this file.

case "$archname" in
'')
    archname=`uname -m`-${osname}
    ;;
esac

# NetBSD keeps dynamic loading dl*() functions in /usr/lib/crt0.o,
# so Configure doesn't find them (unless you abandon the nm scan).
# Also, NetBSD 0.9a was the first release to introduce shared
# libraries.
#
case "$osvers" in
0.9|0.8*)
	usedl="$undef"
	;;
*)
	case `uname -m` in
	pmax)
		# NetBSD 1.3 and 1.3.1 on pmax shipped an `old' ld.so,
		# which will not work.
		case "$osvers" in
		1.3|1.3.1)
			d_dlopen=$undef
			;;
		esac
		;;
	esac
	if test -f /usr/libexec/ld.elf_so; then
		# ELF
		d_dlopen=$define
		d_dlerror=$define
		cccdlflags="-DPIC -fPIC $cccdlflags"
		lddlflags="-shared $lddlflags"
		cat >UU/cc.cbu <<'EOCBU'
# gcc 4.6 doesn't support --whole-archive, but it's required for the
# system gcc to build correctly, so check for it
echo 'int f(void) { return 0; }' >try.c
if ${cc:-cc} $cccdlflags -c try.c -otry.o 2>&1 &&
   ${cc:-cc} --whole-archive $lddlflags try.o -otry.so 2>&1 ; then
    lddlflags="--whole-archive $lddlflags"
fi
rm try.c try.o try.so 2>/dev/null
EOCBU
		rpathflag="-Wl,-rpath,"
		case "$osvers" in
		1.[0-5]*)
			#
			# Include the whole libgcc.a into the perl executable
			# so that certain symbols needed by loadable modules
			# built as C++ objects (__eh_alloc, __pure_virtual,
			# etc.) will always be defined.
			#
			ccdlflags="-Wl,-whole-archive -lgcc \
				-Wl,-no-whole-archive -Wl,-E $ccdlflags"
			;;
		*)
			ccdlflags="-Wl,-E $ccdlflags"
			;;
		esac
	elif test -f /usr/libexec/ld.so; then
		# a.out
		d_dlopen=$define
		d_dlerror=$define
		cccdlflags="-DPIC -fPIC $cccdlflags"
		lddlflags="-Bshareable $lddlflags"
		rpathflag="-R"
	else
		d_dlopen=$undef
		rpathflag=
	fi
	;;
esac

# netbsd had these but they don't really work as advertised, in the
# versions listed below.  if they are defined, then there isn't a
# way to make perl call setuid() or setgid().  if they aren't, then
# ($<, $>) = ($u, $u); will work (same for $(/$)).  this is because
# you can not change the real userid of a process under 4.4BSD.
# netbsd fixed this in 1.3.2.
case "$osvers" in
0.9*|1.[012]*|1.3|1.3.1)
	d_setregid="$undef"
	d_setreuid="$undef"
	;;
esac
case "$osvers" in
0.8*)
	;;
*)
	d_getprotoent_r="$undef"
	d_getprotobyname_r="$undef"
	d_getprotobynumber_r="$undef"
	d_setprotoent_r="$undef"
	d_endprotoent_r="$undef"
	d_getservent_r="$undef"
	d_getservbyname_r="$undef"
	d_getservbyport_r="$undef"
	d_setservent_r="$undef"
	d_endservent_r="$undef"
	d_gethostbyname_r="$undef"
	d_gethostbyaddr2_r="$undef"
	d_gethostbyaddr_r="$undef"
	d_sethostent_r="$undef"
	d_gethostent_r="$undef"
	d_endhostent_r="$undef"
	d_getprotoent_r_proto="0"
	d_getprotobyname_r_proto="0"
	d_getprotobynumber_r_proto="0"
	d_setprotoent_r_proto="0"
	d_endprotoent_r_proto="0"
	d_getservent_r_proto="0"
	d_getservbyname_r_proto="0"
	d_getservbyport_r_proto="0"
	d_setservent_r_proto="0"
	d_endservent_r_proto="0"
	d_gethostbyname_r_proto="0"
	d_gethostbyaddr2_r_proto="0"
	d_gethostbyaddr_r_proto="0"
	d_sethostent_r_proto="0"
	d_endhostent_r_proto="0"
	d_gethostent_r_proto="0"
	;;
esac

# These are obsolete in any netbsd.
d_setrgid="$undef"
d_setruid="$undef"

# there's no problem with vfork.
usevfork=true

# This is there but in machine/ieeefp_h.
ieeefp_h="define"

# This script UU/usethreads.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use threads.
cat > UU/usethreads.cbu <<'EOCBU'
case "$usethreads" in
$define|true|[yY]*)
	lpthread=
	for xxx in pthread; do
		for yyy in $loclibpth $plibpth $glibpth dummy; do
			zzz=$yyy/lib$xxx.a
			if test -f "$zzz"; then
				lpthread=$xxx
				break;
			fi
			zzz=$yyy/lib$xxx.so
			if test -f "$zzz"; then
				lpthread=$xxx
				break;
			fi
			zzz=`ls $yyy/lib$xxx.so.* 2>/dev/null`
			if test "X$zzz" != X; then
				lpthread=$xxx
				break;
			fi
		done
		if test "X$lpthread" != X; then
			break;
		fi
	done
	if test "X$lpthread" != X; then
		# Add -lpthread.
		libswanted="$libswanted $lpthread"
		# There is no libc_r as of NetBSD 1.5.2, so no c -> c_r.
		# This will be revisited when NetBSD gains a native pthreads
		# implementation.
	else
		echo "$0: No POSIX threads library (-lpthread) found.  " \
		     "You may want to install GNU pth.  Aborting." >&4
		exit 1
	fi
	unset lpthread

	# several reentrant functions are embedded in libc, but haven't
	# been added to the header files yet.  Let's hold off on using
	# them until they are a valid part of the API
	case "$osvers" in
	[012].*|3.[0-1])
		d_getprotobyname_r=$undef
		d_getprotobynumber_r=$undef
		d_getprotoent_r=$undef
		d_getservbyname_r=$undef
		d_getservbyport_r=$undef
		d_getservent_r=$undef
		d_setprotoent_r=$undef
		d_setservent_r=$undef
		d_endprotoent_r=$undef
		d_endservent_r=$undef ;;
	esac
	;;

esac
EOCBU

# Set sensible defaults for NetBSD: look for local software in
# /usr/pkg (NetBSD Packages Collection) and in /usr/local.
#
loclibpth="/usr/pkg/lib /usr/local/lib"
locincpth="/usr/pkg/include /usr/local/include"
case "$rpathflag" in
'')
	ldflags=
	;;
*)
	ldflags=
	for yyy in $loclibpth; do
		ldflags="$ldflags $rpathflag$yyy"
	done
	;;
esac

case `uname -m` in
alpha)
    echo 'int main() {}' > try.c
    gcc=`${cc:-cc} -v -c try.c 2>&1|grep 'gcc version egcs-2'`
    case "$gcc" in
    '' | "gcc version egcs-2.95."[3-9]*) ;; # 2.95.3 or better okay
    *)	cat >&4 <<EOF
***
*** Your gcc ($gcc) is known to be
*** too buggy on netbsd/alpha to compile Perl with optimization.
*** It is suggested you install the lang/gcc package which should
*** have at least gcc 2.95.3 which should work okay: use for example
*** Configure -Dcc=/usr/pkg/gcc-2.95.3/bin/cc.  You could also
*** Configure -Doptimize=-O0 to compile Perl without any optimization
*** but that is not recommended.
***
EOF
	exit 1
	;;
    esac
    rm -f try.*
    ;;
esac

# NetBSD/sparc 1.5.3/1.6.1 dumps core in the semid_ds test of Configure.
case `uname -m` in
sparc) d_semctl_semid_ds=undef ;;
esac

# malloc wrap works
case "$usemallocwrap" in
'') usemallocwrap='define' ;;
esac

# don't use perl malloc by default
case "$usemymalloc" in
'') usemymalloc=n ;;
esac

# NetBSD 6 defines the *at() functions in libc, but either doesn't
# implement them, or implements them only for AT_FDCWD
case "$osver" in
[1-6].*)
        d_unlinkat="$undef"
        d_renameat="$undef"
        d_linkat="$undef"
        d_fchmodat="$undef"
        ;;
esac

cat >UU/uselongdouble.cbu <<'EOCBU'
# This script UU/uselongdouble.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use long doubles.
#
# See https://github.com/Perl/perl5/issues/17853 and https://github.com/Perl/perl5/issues/17854
case "$uselongdouble" in
$define|true|[yY]*)
    cat >try.c <<\TRY
#include <stdio.h>
#include <math.h>

long double x = 1.0;

int main(int argc, char **argv) {
    double e1 = exp(1.0);
    /* as of NetBSD 9.0 expl() just calls exp(),
       Fail here if they're equal. */
    return expl(x) == (long double)e1;
}
TRY
    if $cc -o try $ccflags $ldflags try.c -lm && $run ./try; then
        echo "NetBSD seem to have fixed expl (and hopefully more)" >&4
    else
        cat <<EOM >&4

Warning! NetBSD's long double support is limited enough that it will cause
test failures, and possibly build failures, and this doesn't appear to have
been fixed in the release you're running.

EOM
    fi
;;
esac
EOCBU
