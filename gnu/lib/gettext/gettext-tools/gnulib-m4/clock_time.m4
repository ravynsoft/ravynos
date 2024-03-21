# clock_time.m4 serial 14
dnl Copyright (C) 2002-2006, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Check for clock_getres, clock_gettime and clock_settime,
# and set CLOCK_TIME_LIB.
# For a program named, say foo, you should add a line like the following
# in the corresponding Makefile.am file:
# foo_LDADD = $(LDADD) $(CLOCK_TIME_LIB)

AC_DEFUN([gl_CLOCK_TIME],
[
  AC_REQUIRE([AC_CANONICAL_HOST])

  dnl Persuade glibc and Solaris <time.h> to declare these functions.
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])

  # On mingw, these functions are defined in the libwinpthread library,
  # which is better avoided.  In fact, the clock_gettime function is buggy
  # in 32-bit mingw, when -D__MINGW_USE_VC2005_COMPAT is used (which Gnulib's
  # year2038 module does): It leaves the upper 32 bits of the tv_sec field
  # of the result uninitialized.

  # Solaris 2.5.1 needs -lposix4 to get the clock_gettime function.
  # Solaris 7 prefers the library name -lrt to the obsolescent name -lposix4.

  # Save and restore LIBS so e.g., -lrt, isn't added to it.  Otherwise, *all*
  # programs in the package would end up linked with that potentially-shared
  # library, inducing unnecessary run-time overhead.
  CLOCK_TIME_LIB=
  AC_SUBST([CLOCK_TIME_LIB])
  case "$host_os" in
    mingw* | windows*)
      ac_cv_func_clock_getres=no
      ac_cv_func_clock_gettime=no
      ac_cv_func_clock_settime=no
      ;;
    *)
      gl_saved_libs=$LIBS
        AC_SEARCH_LIBS([clock_gettime], [rt posix4],
                       [test "$ac_cv_search_clock_gettime" = "none required" ||
                        CLOCK_TIME_LIB=$ac_cv_search_clock_gettime])
        AC_CHECK_FUNCS([clock_getres clock_gettime clock_settime])
      LIBS=$gl_saved_libs
      ;;
  esac

  # For backward compatibility.
  LIB_CLOCK_GETTIME="$CLOCK_TIME_LIB"
  AC_SUBST([LIB_CLOCK_GETTIME])
])
