# woe32-dll.m4 serial 6
dnl Copyright (C) 2005-2006, 2011, 2018, 2020 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

# Add --disable-auto-import to the LDFLAGS if the linker supports it.
# GNU ld has an --enable-auto-import option, and it is the default on Cygwin
# since July 2005. But it has three fatal drawbacks:
#   - It produces executables and shared libraries with relocations in the
#     .text segment, defeating the principles of virtual memory.
#   - For some constructs such as
#         extern int var;
#         int * const b = &var;
#     it creates an executable that will give an error at runtime, rather
#     than either a compile-time or link-time error or a working executable.
#     (This is with both gcc and g++.) Whereas this code, not relying on
#     auto-import:
#         extern __declspec (dllimport) int var;
#         int * const b = &var;
#     gives a compile-time error with gcc and works with g++.
#   - It doesn't work in some cases (references to a member field of an
#     exported struct variable, or to a particular element of an exported
#     array variable), requiring code modifications.  One platform
#     dictates code modifications on all platforms.
# See <https://haible.de/bruno/woe32dll.html> for more details.
AC_DEFUN([gl_WOE32_DLL],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | cygwin*)
      AC_CACHE_CHECK([for auto-import of symbols],
        [gl_cv_ld_autoimport],
        [dnl --disable-auto-import is unsupported in MSVC and in MSVC/clang.
         dnl We need to sort out this case explicitly, because with clang,
         dnl -Wl,--disable-auto-import does not yield an error, however later
         dnl libtool turns it into --disable-auto-import, which does produce
         dnl an error.
         AC_EGREP_CPP([Known], [
            #ifdef _MSC_VER
             Known
            #endif
           ],
           [gl_cv_ld_autoimport=no],
           [gl_save_LDFLAGS="$LDFLAGS"
            LDFLAGS="$LDFLAGS -Wl,--disable-auto-import"
            AC_LINK_IFELSE([], [gl_cv_ld_autoimport=yes], [gl_cv_ld_autoimport=no])
            LDFLAGS="$gl_save_LDFLAGS"
           ])
        ])
      if test $gl_cv_ld_autoimport = yes; then
        LDFLAGS="$LDFLAGS -Wl,--disable-auto-import"
      fi
      ;;
  esac
])
