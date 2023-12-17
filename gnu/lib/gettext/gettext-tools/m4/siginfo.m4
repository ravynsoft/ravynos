# siginfo.m4 serial 2 (gettext-0.21.1)
dnl Copyright (C) 2001-2002, 2020 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Determine how to determine the precise cause of a signal, for example
# division by zero.
# - SUSV2 and POSIX specify the use of sigaction with SA_SIGINFO and a member
#   void (*)(int sig, siginfo_t *info, void *context) sa_sigaction.
#   Linux (2.2.x and newer) and Solaris implement this.
#   Linux (2.4.x and newer) on i386, m68k, sparc, sparc64, ia64 actually
#   deliver FPE_INTDIV.
# - Without SA_SIGINFO:
#   - Linux on m68k calls the handler as
#     void (*)(int sig, int code, struct sigcontext* scp).
#     For division by zero, code would be VEC_ZERODIV<<2.
#   - Linux on sparc calls the handler either as
#     void (*)(int sig, int code, struct sigcontext* scp),
#     code for division by zero would be SUBSIG_IDIVZERO, or as
#     void (*)(int sig, siginfo_t *info, void *context).
#     Which one depends on a process specific flag in the kernel.
#   - Linux on sparc64 always calls the handler as
#     void (*)(int sig, siginfo_t *info, void *context).
#   - FreeBSD on i386 calls the handler as
#     void (*)(int sig, int code, void* scp, char* addr).
#     For division by zero, code would be FPE_INTDIV.
#   - SunOS 4 calls the handler as
#     void (*)(int sig, int code, void* scp, char* addr).
#   - Solaris?
#   - Irix 5, OSF/1, AIX call the handler as
#     void (*)(int sig, int code, struct sigcontext *scp).
# These are so many OS and CPU dependencies that we don't bother, and rely
# only on SA_SIGINFO.
AC_DEFUN([gt_SIGINFO],
[
  AC_CACHE_CHECK([for signal handlers with siginfo_t], gt_cv_siginfo_t,
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <signal.h>]],
          [[struct sigaction action;
            siginfo_t info;
            action.sa_flags = SA_SIGINFO;
            action.sa_sigaction = (void *) 0;
          ]])],
       [gt_cv_siginfo_t=yes],
       [gt_cv_siginfo_t=no])])
  if test $gt_cv_siginfo_t = yes; then
    AC_DEFINE([HAVE_SIGINFO], [1],
      [Define to 1 if <signal.h> defines the siginfo_t type and
       struct sigaction has the sa_sigaction member and the SA_SIGINFO flag.])
  fi
])
