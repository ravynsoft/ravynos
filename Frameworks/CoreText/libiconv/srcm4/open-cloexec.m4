# open-cloexec.m4
# serial 1
dnl Copyright 2017-2026 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

# Test whether O_CLOEXEC is defined.

AC_DEFUN([gl_PREPROC_O_CLOEXEC],
[
  AC_CACHE_CHECK([for O_CLOEXEC],
    [gl_cv_macro_O_CLOEXEC],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM([[#include <fcntl.h>
                          #ifndef O_CLOEXEC
                            choke me;
                          #endif
                        ]],
                        [[return O_CLOEXEC;]])],
       [gl_cv_macro_O_CLOEXEC=yes],
       [gl_cv_macro_O_CLOEXEC=no])])
])
