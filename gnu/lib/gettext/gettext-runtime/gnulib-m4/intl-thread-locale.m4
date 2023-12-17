# intl-thread-locale.m4 serial 10
dnl Copyright (C) 2015-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl
dnl This file can be used in projects which are not available under
dnl the GNU General Public License or the GNU Lesser General Public
dnl License but which still want to provide support for the GNU gettext
dnl functionality.
dnl Please note that the actual code of the GNU gettext library is covered
dnl by the GNU Lesser General Public License, and the rest of the GNU
dnl gettext package is covered by the GNU General Public License.
dnl They are *not* in the public domain.

dnl Check how to retrieve the name of a per-thread locale (POSIX locale_t).
dnl Sets gt_nameless_locales.
AC_DEFUN([gt_INTL_THREAD_LOCALE_NAME],
[
  AC_REQUIRE([AC_CANONICAL_HOST])

  dnl Persuade Solaris <locale.h> to define 'locale_t'.
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  dnl Test whether uselocale() exists and works at all.
  gt_FUNC_USELOCALE

  dnl On OpenBSD >= 6.2, the locale_t type and the uselocale(), newlocale(),
  dnl duplocale(), freelocale() functions exist but are effectively useless,
  dnl because the locale_t value depends only on the LC_CTYPE category of the
  dnl locale and furthermore contains only one bit of information (it
  dnl distinguishes the "C" locale from the *.UTF-8 locales). See
  dnl <https://cvsweb.openbsd.org/src/lib/libc/locale/newlocale.c?rev=1.1&content-type=text/x-cvsweb-markup>.
  dnl In the setlocale() implementation they have thought about the programs
  dnl that use the API ("Even though only LC_CTYPE has any effect in the
  dnl OpenBSD base system, store complete information about the global locale,
  dnl such that third-party software can access it"), but for uselocale()
  dnl they did not think about the programs.
  dnl In this situation, even the HAVE_NAMELESS_LOCALES support does not work.
  dnl So, define HAVE_FAKE_LOCALES and disable all locale_t support.
  case "$gt_cv_func_uselocale_works" in
    *yes)
      AC_CHECK_HEADERS_ONCE([xlocale.h])
      AC_CACHE_CHECK([for fake locale system (OpenBSD)],
        [gt_cv_locale_fake],
        [AC_RUN_IFELSE(
           [AC_LANG_SOURCE([[
#include <locale.h>
#if HAVE_XLOCALE_H
# include <xlocale.h>
#endif
int main ()
{
  locale_t loc1, loc2;
  if (setlocale (LC_ALL, "de_DE.UTF-8") == NULL) return 1;
  if (setlocale (LC_ALL, "fr_FR.UTF-8") == NULL) return 1;
  loc1 = newlocale (LC_ALL_MASK, "de_DE.UTF-8", (locale_t)0);
  loc2 = newlocale (LC_ALL_MASK, "fr_FR.UTF-8", (locale_t)0);
  return !(loc1 == loc2);
}]])],
           [gt_cv_locale_fake=yes],
           [gt_cv_locale_fake=no],
           [dnl Guess the locale system is fake only on OpenBSD.
            case "$host_os" in
              openbsd*) gt_cv_locale_fake="guessing yes" ;;
              *)        gt_cv_locale_fake="guessing no" ;;
            esac
           ])
        ])
      ;;
    *) gt_cv_locale_fake=no ;;
  esac
  case "$gt_cv_locale_fake" in
    *yes)
      gt_fake_locales=yes
      AC_DEFINE([HAVE_FAKE_LOCALES], [1],
        [Define if the locale_t type contains insufficient information, as on OpenBSD.])
      ;;
    *)
      gt_fake_locales=no
      ;;
  esac

  case "$gt_cv_func_uselocale_works" in
    *yes)
      AC_CACHE_CHECK([for Solaris 11.4 locale system],
        [gt_cv_locale_solaris114],
        [case "$host_os" in
           solaris*)
             dnl Test whether <locale.h> defines locale_t as a typedef of
             dnl 'struct _LC_locale_t **' (whereas Illumos defines it as a
             dnl typedef of 'struct _locale *').
             dnl Another possible test would be to include <sys/localedef.h>
             dnl and test whether it defines the _LC_core_data_locale_t type.
             dnl This type was added in Solaris 11.4.
             AC_COMPILE_IFELSE(
               [AC_LANG_PROGRAM([[
                  #include <locale.h>
                  struct _LC_locale_t *x;
                  locale_t y;
                ]],
                [[*y = x;]])],
               [gt_cv_locale_solaris114=yes],
               [gt_cv_locale_solaris114=no])
             ;;
           *) gt_cv_locale_solaris114=no ;;
         esac
        ])
      ;;
    *) gt_cv_locale_solaris114=no ;;
  esac
  if test $gt_cv_locale_solaris114 = yes; then
    AC_DEFINE([HAVE_SOLARIS114_LOCALES], [1],
      [Define if the locale_t type is as on Solaris 11.4.])
  fi

  dnl Solaris 12 will maybe provide getlocalename_l.  If it does, it will
  dnl improve the implementation of gl_locale_name_thread(), by removing
  dnl the use of undocumented structures.
  case "$gt_cv_func_uselocale_works" in
    *yes)
      AC_CHECK_FUNCS([getlocalename_l])
      ;;
  esac

  dnl This code is for platforms where the locale_t type does not provide access
  dnl to the name of each locale category.  This code has the drawback that it
  dnl requires the gnulib overrides of 'newlocale', 'duplocale', 'freelocale',
  dnl which is a problem for GNU libunistring.  Therefore try hard to avoid
  dnl enabling this code!
  gt_nameless_locales=no
  case "$host_os" in
    dnl It's needed on AIX 7.2.
    aix*)
      gt_nameless_locales=yes
      AC_DEFINE([HAVE_NAMELESS_LOCALES], [1],
        [Define if the locale_t type does not contain the name of each locale category.])
      ;;
  esac

  dnl We cannot support uselocale() on platforms where the locale_t type is
  dnl fake.  So, set
  dnl   gt_good_uselocale = gt_working_uselocale && !gt_fake_locales.
  if test $gt_working_uselocale = yes && test $gt_fake_locales = no; then
    gt_good_uselocale=yes
    AC_DEFINE([HAVE_GOOD_USELOCALE], [1],
      [Define if the uselocale exists, may be safely called, and returns sufficient information.])
  else
    gt_good_uselocale=no
  fi

  dnl Set gt_localename_enhances_locale_funcs to indicate whether localename.c
  dnl overrides newlocale(), duplocale(), freelocale() to keep track of locale
  dnl names.
  if test $gt_good_uselocale = yes && test $gt_nameless_locales = yes; then
    gt_localename_enhances_locale_funcs=yes
    LOCALENAME_ENHANCE_LOCALE_FUNCS=1
    AC_DEFINE([LOCALENAME_ENHANCE_LOCALE_FUNCS], [1],
      [Define if localename.c overrides newlocale(), duplocale(), freelocale().])
  else
    gt_localename_enhances_locale_funcs=no
  fi
])

dnl Tests whether uselocale() exists and is usable.
dnl Sets gt_working_uselocale and defines HAVE_WORKING_USELOCALE.
AC_DEFUN([gt_FUNC_USELOCALE],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles

  dnl Persuade glibc and Solaris <locale.h> to define 'locale_t'.
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  gl_CHECK_FUNCS_ANDROID([uselocale], [[#include <locale.h>]])

  dnl On AIX 7.2, the uselocale() function is not documented and leads to
  dnl crashes in subsequent setlocale() invocations.
  dnl In 2019, some versions of z/OS lack the locale_t type and have a broken
  dnl uselocale function.
  if test $ac_cv_func_uselocale = yes; then
    AC_CHECK_HEADERS_ONCE([xlocale.h])
    AC_CACHE_CHECK([whether uselocale works],
      [gt_cv_func_uselocale_works],
      [AC_RUN_IFELSE(
         [AC_LANG_SOURCE([[
#include <locale.h>
#if HAVE_XLOCALE_H
# include <xlocale.h>
#endif
locale_t loc1;
int main ()
{
  uselocale (NULL);
  setlocale (LC_ALL, "en_US.UTF-8");
  return 0;
}]])],
         [gt_cv_func_uselocale_works=yes],
         [gt_cv_func_uselocale_works=no],
         [# Guess no on AIX and z/OS, yes otherwise.
          case "$host_os" in
            aix* | openedition*) gt_cv_func_uselocale_works="guessing no" ;;
            *)                   gt_cv_func_uselocale_works="guessing yes" ;;
          esac
         ])
      ])
  else
    gt_cv_func_uselocale_works=no
  fi
  case "$gt_cv_func_uselocale_works" in
    *yes)
      gt_working_uselocale=yes
      AC_DEFINE([HAVE_WORKING_USELOCALE], [1],
        [Define if the uselocale function exists and may safely be called.])
      ;;
    *)
      gt_working_uselocale=no
      ;;
  esac
])
