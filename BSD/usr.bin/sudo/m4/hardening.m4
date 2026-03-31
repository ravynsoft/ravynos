AC_DEFUN([SUDO_CHECK_HARDENING], [
    if test "$enable_hardening" != "no"; then
	#
	# Attempt to use _FORTIFY_SOURCE with sprintf.  If the headers support
	# it but libc does not, __sprintf_chk should be an undefined symbol.
	#
	O_CPPFLAGS="$CPPFLAGS"
	AX_APPEND_FLAG([-D_FORTIFY_SOURCE=2], [CPPFLAGS])
	AC_CACHE_CHECK([whether _FORTIFY_SOURCE may be specified],
	    [sudo_cv_use_fortify_source],
	    [AC_LINK_IFELSE([
		    AC_LANG_PROGRAM(
			[[#include <stdio.h>]],
			[[char buf[4]; sprintf(buf, "%s", "foo"); return buf[0];]]
		    )],
		    [sudo_cv_use_fortify_source=yes],
		    [sudo_cv_use_fortify_source=no]
		)
	    ]
	)
	if test "$sudo_cv_use_fortify_source" != yes; then
	    CPPFLAGS="$O_CPPFLAGS"
	fi

	dnl
	dnl The following tests rely on AC_LANG_WERROR.
	dnl
	if test -n "$GCC" -a "$enable_ssp" != "no"; then
	    AC_CACHE_CHECK([for compiler stack protector support],
		[sudo_cv_var_stack_protector],
		[
		    # Avoid CFLAGS since the compiler might optimize away our
		    # test.  We don't want CPPFLAGS or LIBS to interfere with
		    # the test but keep LDFLAGS as it may have an rpath needed
		    # to find the ssp lib.
		    _CPPFLAGS="$CPPFLAGS"
		    _CFLAGS="$CFLAGS"
		    _LDFLAGS="$LDFLAGS"
		    _LIBS="$LIBS"
		    CPPFLAGS=
		    LIBS=

		    sudo_cv_var_stack_protector="-fstack-protector-strong"
		    CFLAGS="$sudo_cv_var_stack_protector"
		    LDFLAGS="$_LDFLAGS $sudo_cv_var_stack_protector"
		    AC_LINK_IFELSE([
			AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
			[[char buf[1024]; buf[1023] = '\0';]])
		    ], [], [
			sudo_cv_var_stack_protector="-fstack-protector-all"
			CFLAGS="$sudo_cv_var_stack_protector"
			LDFLAGS="$_LDFLAGS $sudo_cv_var_stack_protector"
			AC_LINK_IFELSE([
			    AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
			    [[char buf[1024]; buf[1023] = '\0';]])
			], [], [
			    sudo_cv_var_stack_protector="-fstack-protector"
			    CFLAGS="$sudo_cv_var_stack_protector"
			    LDFLAGS="$_LDFLAGS $sudo_cv_var_stack_protector"
			    AC_LINK_IFELSE([
				AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
				[[char buf[1024]; buf[1023] = '\0';]])
			    ], [], [
				sudo_cv_var_stack_protector=no
			    ])
			])
		    ])
		    CPPFLAGS="$_CPPFLAGS"
		    CFLAGS="$_CFLAGS"
		    LDFLAGS="$_LDFLAGS"
		    LIBS="$_LIBS"
		]
	    )
	    if test X"$sudo_cv_var_stack_protector" != X"no"; then
		HARDENING_CFLAGS="$sudo_cv_var_stack_protector"
		HARDENING_LDFLAGS="-Wc,$sudo_cv_var_stack_protector"
	    fi
	fi

	# The gcc front-end may accept -fstack-clash-protection even if the
	# machine-specific code does not support it.  We use a test program
	# with a large stack allocation to try to cause the compiler to
	# insert the stack clash protection code, or fail if not supported.
	if test -n "$GCC"; then
	    AC_CACHE_CHECK([whether C compiler supports -fstack-clash-protection],
		[sudo_cv_check_cflags___fstack_clash_protection],
		[
		    _CFLAGS="$CFLAGS"
		    CFLAGS="$CFLAGS -fstack-clash-protection"
		    AC_COMPILE_IFELSE([
			AC_LANG_SOURCE([[int main(int argc, char *argv[]) { char buf[16384], *src = argv[0], *dst = buf; while ((*dst++ = *src++) != '\0') { continue; } return buf[argc]; }]])
		    ], [sudo_cv_check_cflags___fstack_clash_protection=yes], [sudo_cv_check_cflags___fstack_clash_protection=no])
		    CFLAGS="$_CFLAGS"
		]
	    )
	    if test X"$sudo_cv_check_cflags___fstack_clash_protection" = X"yes"; then
		AX_CHECK_LINK_FLAG([-fstack-clash-protection], [
		    AX_APPEND_FLAG([-fstack-clash-protection], [HARDENING_CFLAGS])
		    AX_APPEND_FLAG([-Wc,-fstack-clash-protection], [HARDENING_LDFLAGS])
		])
	    fi

	    # Check for control-flow transfer instrumentation (Intel CET).
	    AX_CHECK_COMPILE_FLAG([-fcf-protection], [
		AX_CHECK_LINK_FLAG([-fcf-protection], [
		    AX_APPEND_FLAG([-fcf-protection], [HARDENING_CFLAGS])
		    AX_APPEND_FLAG([-Wc,-fcf-protection], [HARDENING_LDFLAGS])
		])
	    ])
	fi

	# Linker-specific hardening flags.
	if test X"$with_gnu_ld" = X"yes"; then
	    # GNU ld, and similar (gold, lld, etc).
	    AX_CHECK_LINK_FLAG([-Wl,-z,relro], [AX_APPEND_FLAG([-Wl,-z,relro], [HARDENING_LDFLAGS])])
	    AX_CHECK_LINK_FLAG([-Wl,-z,now], [AX_APPEND_FLAG([-Wl,-z,now], [HARDENING_LDFLAGS])])
	    AX_CHECK_LINK_FLAG([-Wl,-z,noexecstack], [AX_APPEND_FLAG([-Wl,-z,noexecstack], [HARDENING_LDFLAGS])])
	fi
    fi])
