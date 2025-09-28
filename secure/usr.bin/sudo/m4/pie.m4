AC_DEFUN([SUDO_CHECK_PIE_SUPPORT], [
    dnl
    dnl Check for PIE executable support if using gcc.
    dnl This test relies on AC_LANG_WERROR
    dnl
    if test -n "$GCC"; then
	if test X"$enable_pie" = X""; then
	    case "$host_os" in
		linux*)
		    # Attempt to build with PIE support
		    enable_pie="maybe"
		    ;;
	    esac
	fi
	if test X"$enable_pie" != X""; then
	    if test "$enable_pie" = "no"; then
		AX_CHECK_COMPILE_FLAG([-fno-pie], [
		    _CFLAGS="$CFLAGS"
		    CFLAGS="$CFLAGS -fno-pie"
		    AX_CHECK_LINK_FLAG([-nopie], [
			PIE_CFLAGS="-fno-pie"
			PIE_LDFLAGS="-nopie"
		    ])
		    CFLAGS="$_CFLAGS"
		])
	    else
		AX_CHECK_COMPILE_FLAG([-fPIE], [
		    _CFLAGS="$CFLAGS"
		    CFLAGS="$CFLAGS -fPIE"
		    _LDFLAGS="$LDFLAGS"
		    AX_CHECK_LINK_FLAG([-pie], [
			if test "$enable_pie" = "maybe"; then
			    LDFLAGS="$LDFLAGS -pie"
			    SUDO_WORKING_PIE([enable_pie=yes], [])
			fi
			if test "$enable_pie" = "yes"; then
			    PIE_CFLAGS="-fPIE"
			    PIE_LDFLAGS="-Wc,-fPIE -pie"
			fi
		    ])
		    CFLAGS="$_CFLAGS"
		    LDFLAGS="$_LDFLAGS"
		])
	    fi
	fi
    fi
    if test X"$enable_pie" != X"no" -a X"$with_gnu_ld" = X"no"; then
	# Solaris 11.1 and higher ld supports PIE executables, ASLR,
	# non-executable stack and non-executable heap.
	case "$host_os" in
	    solaris2.1[[1-9]]|solaris2.[[2-9]][[0-9]])
		# This assumes lt_prog_compiler_pic is a single flag,
		# which is the case on Solaris.
		if test -n "$lt_prog_compiler_pic"; then
		    _CFLAGS="$CFLAGS"
		    CFLAGS="$CFLAGS $lt_prog_compiler_pic"
		    _LDFLAGS="$LDFLAGS"
		    AX_CHECK_LINK_FLAG([-Wl,-ztype=pie], [
			# Try building PIE if not disabled.
			if test X"$enable_pie" = X""; then
			    LDFLAGS="$LDFLAGS -Wl,-ztype=pie"
			    SUDO_WORKING_PIE([enable_pie=yes], [])
			fi
			if test "$enable_pie" = "yes"; then
			    PIE_CFLAGS="$lt_prog_compiler_pic"
			    PIE_LDFLAGS="-Wc,$lt_prog_compiler_pic -Wl,-ztype=pie"
			fi
		    ])
		    CFLAGS="$_CFLAGS"
		    LDFLAGS="$_LDFLAGS"
		fi
		# These flags are only valid when linking an executable
		# so we cannot add them to HARDENING_LDFLAGS.
		AX_CHECK_LINK_FLAG([-Wl,-zaslr], [
		    AX_APPEND_FLAG([-Wl,-zaslr], [PIE_LDFLAGS])
		])
		AX_CHECK_LINK_FLAG([-Wl,-znxheap], [
		    AX_APPEND_FLAG([-Wl,-znxheap], [PIE_LDFLAGS])
		])
		AX_CHECK_LINK_FLAG([-Wl,-znxstack], [
		    AX_APPEND_FLAG([-Wl,-znxstack], [PIE_LDFLAGS])
		])
		;;
	esac
    fi
])
