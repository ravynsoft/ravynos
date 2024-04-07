# hints/minix.sh
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
#
case "$osvers" in
*)
	if test -f /usr/libexec/ld.elf_so; then
		# ELF
		d_dlopen=$define
		d_dlerror=$define
		cccdlflags="-DPIC -fPIC $cccdlflags"
		lddlflags="-shared $lddlflags"
		rpathflag="-Wl,-rpath,"
		ccdlflags="-Wl,-E $ccdlflags"
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
esac
EOCBU

# Set sensible defaults for Minix: look for local software in
# /usr/local, plus the build prefix, which might or might not be
# /usr/pkg.
#
loclibpth="/usr/local/lib ${prefix}/lib"
locincpth="/usr/local/include ${prefix}/include"

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

# malloc wrap works
case "$usemallocwrap" in
'') usemallocwrap='define' ;;
esac

# don't use perl malloc by default
case "$usemymalloc" in
'') usemymalloc=n ;;
esac
