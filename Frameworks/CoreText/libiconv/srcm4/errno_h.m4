# errno_h.m4
# serial 19
dnl Copyright (C) 2004, 2006, 2008-2026 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

AC_PREREQ([2.61])

AC_DEFUN_ONCE([gl_HEADER_ERRNO_H],
[
  AC_REQUIRE([AC_PROG_CC])

  dnl Through the dependency on module extensions-aix, _LINUX_SOURCE_COMPAT
  dnl gets defined already before this macro gets invoked.  This persuades
  dnl AIX 7.3 errno.h to assign ENOTEMPTY a value different than EEXIST.

  AC_CACHE_CHECK([for complete errno.h], [gl_cv_header_errno_h_complete], [
    AC_EGREP_CPP([booboo],[
#include <errno.h>
#if !defined ETXTBSY
booboo
#endif
#if !defined ENOMSG
booboo
#endif
#if !defined EIDRM
booboo
#endif
#if !defined ENOLINK
booboo
#endif
#if !defined EPROTO
booboo
#endif
#if !defined EMULTIHOP
booboo
#endif
#if !defined EBADMSG
booboo
#endif
#if !defined EOVERFLOW
booboo
#endif
#if !defined ENOTSUP
booboo
#endif
#if !defined ENETRESET
booboo
#endif
#if !defined ECONNABORTED
booboo
#endif
#if !defined ESTALE
booboo
#endif
#if !defined EDQUOT
booboo
#endif
#if !defined ECANCELED
booboo
#endif
#if !defined EOWNERDEAD
booboo
#endif
#if !defined ENOTRECOVERABLE
booboo
#endif
#if !defined EILSEQ
booboo
#endif
#if !defined ESOCKTNOSUPPORT
booboo
#endif
      ],
      [gl_cv_header_errno_h_complete=no],
      [gl_cv_header_errno_h_complete=yes])
  ])
  if test $gl_cv_header_errno_h_complete = yes; then
    GL_GENERATE_ERRNO_H=false
  else
    gl_NEXT_HEADERS([errno.h])
    GL_GENERATE_ERRNO_H=true
  fi
])
