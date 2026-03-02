AC_DEFUN([SUDO_CHECK_SANITIZER], [
    if test X"${enable_sanitizer}{enable_fuzzer}" != X"nono"; then
	dnl
	dnl For fuzz_policy we redefine getaddrinfo() and freeaddrinfo(), but
	dnl this can cause problems with ld.lld when sanitizers are enabled.
	dnl
	AX_CHECK_LINK_FLAG([-Wl,--allow-multiple-definition], [AX_APPEND_FLAG([-Wl,--allow-multiple-definition], [ASAN_LDFLAGS])])
    fi

    dnl
    dnl Check for -fsanitize support
    dnl This test relies on AC_LANG_WERROR
    dnl
    if test X"$enable_sanitizer" != X"no"; then
	AX_CHECK_COMPILE_FLAG([$enable_sanitizer], [
	    AX_APPEND_FLAG([$enable_sanitizer], [ASAN_CFLAGS])
	    AX_APPEND_FLAG([-XCClinker], [ASAN_LDFLAGS])
	    AX_APPEND_FLAG([$enable_sanitizer], [ASAN_LDFLAGS])
	    AX_CHECK_COMPILE_FLAG([-fno-omit-frame-pointer], [
		AX_APPEND_FLAG([-fno-omit-frame-pointer], [CFLAGS])
	    ])
	    AC_DEFINE(NO_LEAKS)
	    dnl
	    dnl Check for libasan.so to preload it before sudo_intercept.so.
	    dnl gcc links asan dynamically, clang links it statically.
	    dnl
	    case `$CC --version 2>&1` in
	    *gcc*)
		libasan=`$CC -print-file-name=libasan.so 2>/dev/null`
		if test -n "$libasan" -a X"$libasan" != X"libasan.so"; then
		    # libasan.so may be a linker script
		    libasan="`awk 'BEGIN {lib=ARGV[[1]]} /^INPUT/ {lib=$[3]} END {print lib}' \"$libasan\"`"
		    SUDO_DEFINE_UNQUOTED(_PATH_ASAN_LIB, "$libasan", [Path to the libasan.so shared library])
		fi
		;;
	    esac
	], [
	    AC_MSG_ERROR([$CC does not support the $enable_sanitizer flag])
	])
    fi

    if test X"$enable_fuzzer" = X"yes"; then
	AX_CHECK_COMPILE_FLAG([-fsanitize=fuzzer-no-link], [
	    AX_APPEND_FLAG([-fsanitize=fuzzer-no-link], [ASAN_CFLAGS])
	    AX_APPEND_FLAG([-XCClinker], [ASAN_LDFLAGS])
	    AX_APPEND_FLAG([-fsanitize=fuzzer-no-link], [ASAN_LDFLAGS])
	    if test -z "$FUZZ_ENGINE"; then
		FUZZ_ENGINE="-fsanitize=fuzzer"
	    fi
	    AX_CHECK_COMPILE_FLAG([-fno-omit-frame-pointer], [
		AX_APPEND_FLAG([-fno-omit-frame-pointer], [CFLAGS])
	    ])
	    # Use CFLAGS, not CPPFLAGS to match oss-fuzz behavior
	    AX_APPEND_FLAG([-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION], [CFLAGS])
	    AC_DEFINE(NO_LEAKS)
	], [
	    AC_MSG_ERROR([$CC does not support the -fsanitize=fuzzer-no-link flag])
	])
    else
	# Not using compiler fuzzing support, link with stub library.
	FUZZ_ENGINE='$(top_builddir)/lib/fuzzstub/libsudo_fuzzstub.la'
    fi
])
