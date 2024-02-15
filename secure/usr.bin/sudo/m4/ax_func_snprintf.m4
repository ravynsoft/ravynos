# ===========================================================================
#     https://www.gnu.org/software/autoconf-archive/ax_func_snprintf.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_FUNC_SNPRINTF
#
# DESCRIPTION
#
#   Checks for a fully C99 compliant snprintf, in particular checks whether
#   it does bounds checking and returns the correct string length; does the
#   same check for vsnprintf. If no working snprintf or vsnprintf is found,
#   request a replacement and warn the user about it. Note: the mentioned
#   replacement is freely available and may be used in any project
#   regardless of it's license.
#
# LICENSE
#
#   Copyright (c) 2008 Ruediger Kuhlmann <info@ruediger-kuhlmann.de>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 7

AU_ALIAS([AC_FUNC_SNPRINTF], [AX_FUNC_SNPRINTF])
AC_DEFUN([AX_FUNC_SNPRINTF],
[AC_CHECK_FUNCS(snprintf vsnprintf)
AC_MSG_CHECKING(for working snprintf)
AC_CACHE_VAL(ac_cv_have_working_snprintf,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <string.h>

int main(void)
{
    char bufs[5] = { 'x', 'x', 'x', '\0', '\0' };
    char bufd[5] = { 'x', 'x', 'x', '\0', '\0' };
    int i;
    i = snprintf (bufs, 2, "%s", "111");
    if (strcmp (bufs, "1")) return (1);
    if (i != 3) return (1);
    i = snprintf (bufd, 2, "%d", 111);
    if (strcmp (bufd, "1")) return (1);
    if (i != 3) return (1);
    return(0);
}]])],[ac_cv_have_working_snprintf=yes],[ac_cv_have_working_snprintf=no],[ac_cv_have_working_snprintf=cross])])
AC_MSG_RESULT([$ac_cv_have_working_snprintf])
AC_MSG_CHECKING(for working vsnprintf)
AC_CACHE_VAL(ac_cv_have_working_vsnprintf,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <stdarg.h>
#include <string.h>

int my_vsnprintf (char *buf, const char *tmpl, ...)
{
    int i;
    va_list args;
    va_start (args, tmpl);
    i = vsnprintf (buf, 2, tmpl, args);
    va_end (args);
    return i;
}

int main(void)
{
    char bufs[5] = { 'x', 'x', 'x', '\0', '\0' };
    char bufd[5] = { 'x', 'x', 'x', '\0', '\0' };
    int i;
    i = my_vsnprintf (bufs, "%s", "111");
    if (strcmp (bufs, "1")) return (1);
    if (i != 3) return (1);
    i = my_vsnprintf (bufd, "%d", 111);
    if (strcmp (bufd, "1")) return (1);
    if (i != 3) return (1);
    return(0);
}]])],[ac_cv_have_working_vsnprintf=yes],[ac_cv_have_working_vsnprintf=no],[ac_cv_have_working_vsnprintf=cross])])
AC_MSG_RESULT([$ac_cv_have_working_vsnprintf])
if test x$ac_cv_have_working_snprintf$ac_cv_have_working_vsnprintf != "xyesyes"; then
  AC_LIBOBJ(snprintf)
  AC_MSG_WARN([Replacing missing/broken (v)snprintf() with sudo's version.])
  AC_DEFINE(PREFER_PORTABLE_SNPRINTF, 1, [Enable replacement (v)snprintf if system (v)snprintf is broken.])
fi])
