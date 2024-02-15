AC_DEFUN([SUDO_SYMBOL_VISIBILITY], [
    dnl
    dnl Check for symbol visibility support.
    dnl This test relies on AC_LANG_WERROR
    dnl
    if test -n "$GCC"; then
	AX_CHECK_COMPILE_FLAG([-fvisibility=hidden], [
	    AC_DEFINE(HAVE_DSO_VISIBILITY)
	    AX_APPEND_FLAG([-fvisibility=hidden], [CFLAGS])
	    LT_LDEXPORTS=
	    LT_LDDEP=
	])
    else
	case "$host_os" in
	hpux*)
	    AX_CHECK_COMPILE_FLAG([-Bhidden_def], [
		# HP-UX cc may not allow __declspec(dllexport) to be
		# used in conjunction with #pragma HP_DEFINED_EXTERNAL
		# when redefining standard libc functions.
		AC_CACHE_CHECK([whether __declspec(dllexport) can be used when overriding libc functions],
		    [sudo_cv_var_hpux_declspec_libc_function],
		    [
			_CFLAGS="$CFLAGS"
			CFLAGS="${CFLAGS} -Bhidden_def"
			AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>
	__declspec(dllexport) char * getenv(const char *n) { return NULL; }]])], [
			    sudo_cv_var_hpux_declspec_libc_function=yes
			], [
			    sudo_cv_var_hpux_declspec_libc_function=no
			])
			CFLAGS="$_CFLAGS"
		    ]
		)
		if test "$sudo_cv_var_hpux_declspec_libc_function" = "yes"; then
		    AC_DEFINE(HAVE_DSO_VISIBILITY)
		    AX_APPEND_FLAG([-Bhidden_def], [CFLAGS])
		    LT_LDEXPORTS=
		    LT_LDDEP=
		fi
	    ])
	    ;;
	solaris2*)
	    AX_CHECK_COMPILE_FLAG([-xldscope=hidden], [
		AC_DEFINE(HAVE_DSO_VISIBILITY)
		AX_APPEND_FLAG([-xldscope=hidden], [CFLAGS])
		LT_LDEXPORTS=
		LT_LDDEP=
	    ])
	    ;;
	esac
    fi

    dnl
    dnl Check whether ld supports version scripts (most ELF linkers).
    dnl If possible, we use this even if the compiler has symbol visibility
    dnl support so we will notice mismatches between the exports file and
    dnl sudo_dso_public annotations in the source code.
    dnl This test relies on AC_LANG_WERROR
    dnl
    if test "$lt_cv_prog_gnu_ld" = "yes"; then
	AC_CACHE_CHECK([whether ld supports anonymous map files],
	    [sudo_cv_var_gnu_ld_anon_map],
	    [
		sudo_cv_var_gnu_ld_anon_map=no
		cat > conftest.map <<-EOF
		{
		    global: foo;
		    local:  *;
		};
		EOF
		_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $lt_prog_compiler_pic"
		_LDFLAGS="$LDFLAGS"
		LDFLAGS="$LDFLAGS $lt_prog_compiler_pic -shared -Wl,--version-script,./conftest.map"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[int foo;]], [[]])],
		    [sudo_cv_var_gnu_ld_anon_map=yes])
		CFLAGS="$_CFLAGS"
		LDFLAGS="$_LDFLAGS"
		rm -f conftest.map
	    ]
	)
	if test "$sudo_cv_var_gnu_ld_anon_map" = "yes"; then
	    LT_LDDEP="\$(shlib_map)"; LT_LDEXPORTS="-Wl,--version-script,\$(shlib_map)"
	fi
    else
	case "$host_os" in
	solaris2*)
	    AC_CACHE_CHECK([whether ld supports anonymous map files],
		[sudo_cv_var_solaris_ld_anon_map],
		[
		    sudo_cv_var_solaris_ld_anon_map=no
		    cat > conftest.map <<-EOF
			{
			    global: foo;
			    local:  *;
			};
			EOF
		    _CFLAGS="$CFLAGS"
		    CFLAGS="$CFLAGS $lt_prog_compiler_pic"
		    _LDFLAGS="$LDFLAGS"
		    LDFLAGS="$LDFLAGS -shared -Wl,-M,./conftest.map"
		    AC_LINK_IFELSE([AC_LANG_PROGRAM([[int foo;]], [[]])],
			[sudo_cv_var_solaris_ld_anon_map=yes])
		    CFLAGS="$_CFLAGS"
		    LDFLAGS="$_LDFLAGS"
		    rm -f conftest.map
		]
	    )
	    if test "$sudo_cv_var_solaris_ld_anon_map" = "yes"; then
		LT_LDDEP="\$(shlib_map)"; LT_LDEXPORTS="-Wl,-M,\$(shlib_map)"
	    fi
	    ;;
	hpux*)
	    AC_CACHE_CHECK([whether ld supports controlling exported symbols],
		[sudo_cv_var_hpux_ld_symbol_export],
		[
		    sudo_cv_var_hpux_ld_symbol_export=no
		    echo "+e foo" > conftest.opt
		    _CFLAGS="$CFLAGS"
		    CFLAGS="$CFLAGS $lt_prog_compiler_pic"
		    _LDFLAGS="$LDFLAGS"
		    if test -n "$GCC"; then
			LDFLAGS="$LDFLAGS -shared -Wl,-c,./conftest.opt"
		    else
			LDFLAGS="$LDFLAGS -b -Wl,-c,./conftest.opt"
		    fi
		    AC_LINK_IFELSE([AC_LANG_PROGRAM([[int foo;]], [[]])],
			[sudo_cv_var_hpux_ld_symbol_export=yes])
		    CFLAGS="$_CFLAGS"
		    LDFLAGS="$_LDFLAGS"
		    rm -f conftest.opt
		]
	    )
	    if test "$sudo_cv_var_hpux_ld_symbol_export" = "yes"; then
		LT_LDDEP="\$(shlib_opt)"; LT_LDEXPORTS="-Wl,-c,\$(shlib_opt)"
	    fi
	    ;;
	esac
    fi
])
