# terminfo.m4 serial 6
dnl Copyright (C) 2000-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

AC_DEFUN([gl_TERMINFO],
[
  AC_REQUIRE([gl_TERMINFO_BODY])
  if test $gl_cv_terminfo_tparam = no && test $gl_cv_terminfo_tparm = no; then
    AC_LIBOBJ([tparm])
  fi
  case "$gl_cv_terminfo" in
    no*)
      case "$gl_cv_termcap" in
        no*)
          AC_LIBOBJ([tputs])
          ;;
      esac
      ;;
  esac
])

AC_DEFUN([gl_TERMINFO_BODY],
[
  dnl Some systems have setupterm(), tigetnum(), tigetstr(), tigetflag(),
  dnl tputs(), tgoto() in the same library as tgetent(), tgetnum(), tgetstr(),
  dnl tgetflag(), e.g. Linux (in libncurses) or Solaris (in libtermcap =
  dnl libncurses).
  dnl Some systems have them in a different library, e.g. OSF/1 (in libcurses,
  dnl not in libtermcap) or AIX, HP-UX (in libxcurses, not in libtermcap).
  dnl Some systems, like NetBSD or BeOS, don't have these functions at all;
  dnl they have only a libtermcap.
  dnl Some systems, like BeOS, use GNU termcap, which has tparam() instead of
  dnl tparm().
  dnl Some systems, like mingw, have nothing at all.

  dnl Some people want to avoid these libraries, in special situations such
  dnl as when cross-compiling.
  AC_REQUIRE([gl_CURSES])

  dnl Prerequisites of AC_LIB_LINKFLAGS_BODY.
  AC_REQUIRE([AC_LIB_PREPARE_PREFIX])
  AC_REQUIRE([AC_LIB_RPATH])

  dnl Avoid disturbing the gl_TERMCAP_BODY macro.
  gl_save_LIBTERMCAP="$LIBTERMCAP"
  gl_save_LTLIBTERMCAP="$LTLIBTERMCAP"
  gl_save_INCTERMCAP="$INCTERMCAP"

  if test "$gl_curses_allowed" != no; then

    dnl Search for libncurses and define LIBNCURSES, LTLIBNCURSES and INCNCURSES
    dnl accordingly.
    AC_LIB_LINKFLAGS_BODY([ncurses])

    dnl Search for libtermcap and define LIBTERMCAP, LTLIBTERMCAP and INCTERMCAP
    dnl accordingly.
    AC_LIB_LINKFLAGS_BODY([termcap])

    dnl Search for libxcurses and define LIBXCURSES, LTLIBXCURSES and INCXCURSES
    dnl accordingly.
    AC_LIB_LINKFLAGS_BODY([xcurses])

    dnl Search for libcurses and define LIBCURSES, LTLIBCURSES and INCCURSES
    dnl accordingly.
    AC_LIB_LINKFLAGS_BODY([curses])

  else

    LIBNCURSES=
    LTLIBNCURSES=
    INCNCURSES=

    LIBTERMCAP=
    LTLIBTERMCAP=
    INCTERMCAP=

    LIBXCURSES=
    LTLIBXCURSES=
    INCXCURSES=

    LIBCURSES=
    LTLIBCURSES=
    INCCURSES=

  fi

  dnl When searching for the terminfo functions, prefer libtermcap over
  dnl libxcurses and libcurses, because it is smaller.
  AC_CACHE_CHECK([where terminfo library functions come from], [gl_cv_terminfo], [
    gl_cv_terminfo="not found, consider installing GNU ncurses"
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
         [[extern
           #ifdef __cplusplus
           "C"
           #endif
           int setupterm (const char *, int, int *);
           extern
           #ifdef __cplusplus
           "C"
           #endif
           int tigetnum (const char *);
           extern
           #ifdef __cplusplus
           "C"
           #endif
           int tigetflag (const char *);
           extern
           #ifdef __cplusplus
           "C"
           #endif
           const char * tigetstr (const char *);
         ]],
         [[return setupterm ("xterm", 0, (int *)0)
                  + tigetnum ("colors")
                  + tigetflag ("hc") + * tigetstr ("oc");]])],
      [gl_cv_terminfo=libc])
    if test "$gl_cv_terminfo" != libc; then
      gl_save_LIBS="$LIBS"
      LIBS="$LIBS $LIBNCURSES"
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
           [[extern
             #ifdef __cplusplus
             "C"
             #endif
             int setupterm (const char *, int, int *);
             extern
             #ifdef __cplusplus
             "C"
             #endif
             int tigetnum (const char *);
             extern
             #ifdef __cplusplus
             "C"
             #endif
             int tigetflag (const char *);
             extern
             #ifdef __cplusplus
             "C"
             #endif
             const char * tigetstr (const char *);
           ]],
           [[return setupterm ("xterm", 0, (int *)0)
                    + tigetnum ("colors")
                    + tigetflag ("hc") + * tigetstr ("oc");]])],
        [gl_cv_terminfo=libncurses])
      LIBS="$gl_save_LIBS"
      if test "$gl_cv_terminfo" != libncurses; then
        gl_save_LIBS="$LIBS"
        LIBS="$LIBS $LIBTERMCAP"
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM(
             [[extern
               #ifdef __cplusplus
               "C"
               #endif
               int setupterm (const char *, int, int *);
               extern
               #ifdef __cplusplus
               "C"
               #endif
               int tigetnum (const char *);
               extern
               #ifdef __cplusplus
               "C"
               #endif
               int tigetflag (const char *);
               extern
               #ifdef __cplusplus
               "C"
               #endif
               const char * tigetstr (const char *);
             ]],
             [[return setupterm ("xterm", 0, (int *)0)
                      + tigetnum ("colors")
                      + tigetflag ("hc") + * tigetstr ("oc");]])],
          [gl_cv_terminfo=libtermcap])
        LIBS="$gl_save_LIBS"
        if test "$gl_cv_terminfo" != libtermcap; then
          gl_save_LIBS="$LIBS"
          LIBS="$LIBS $LIBXCURSES"
          AC_LINK_IFELSE(
            [AC_LANG_PROGRAM(
               [[extern
                 #ifdef __cplusplus
                 "C"
                 #endif
                 int setupterm (const char *, int, int *);
                 extern
                 #ifdef __cplusplus
                 "C"
                 #endif
                 int tigetnum (const char *);
                 extern
                 #ifdef __cplusplus
                 "C"
                 #endif
                 int tigetflag (const char *);
                 extern
                 #ifdef __cplusplus
                 "C"
                 #endif
                 const char * tigetstr (const char *);
               ]],
               [[return setupterm ("xterm", 0, (int *)0)
                        + tigetnum ("colors")
                        + tigetflag ("hc") + * tigetstr ("oc");]])],
            [gl_cv_terminfo=libxcurses])
          LIBS="$gl_save_LIBS"
          if test "$gl_cv_terminfo" != libxcurses; then
            gl_save_LIBS="$LIBS"
            LIBS="$LIBS $LIBCURSES"
            AC_LINK_IFELSE(
              [AC_LANG_PROGRAM(
                 [[extern
                   #ifdef __cplusplus
                   "C"
                   #endif
                   int setupterm (const char *, int, int *);
                   extern
                   #ifdef __cplusplus
                   "C"
                   #endif
                   int tigetnum (const char *);
                   extern
                   #ifdef __cplusplus
                   "C"
                   #endif
                   int tigetflag (const char *);
                   extern
                   #ifdef __cplusplus
                   "C"
                   #endif
                   const char * tigetstr (const char *);
                 ]],
                 [[return setupterm ("xterm", 0, (int *)0)
                          + tigetnum ("colors")
                          + tigetflag ("hc") + * tigetstr ("oc");]])],
              [gl_cv_terminfo=libcurses])
            LIBS="$gl_save_LIBS"
          fi
        fi
      fi
    fi
  ])
  case "$gl_cv_terminfo" in
    libc)
      LIBTERMINFO=
      LTLIBTERMINFO=
      INCTERMINFO=
      ;;
    libncurses)
      LIBTERMINFO="$LIBNCURSES"
      LTLIBTERMINFO="$LTLIBNCURSES"
      INCTERMINFO="$INCNCURSES"
      ;;
    libtermcap)
      LIBTERMINFO="$LIBTERMCAP"
      LTLIBTERMINFO="$LTLIBTERMCAP"
      INCTERMINFO="$INCTERMCAP"
      ;;
    libxcurses)
      LIBTERMINFO="$LIBXCURSES"
      LTLIBTERMINFO="$LTLIBXCURSES"
      INCTERMINFO="$INCXCURSES"
      ;;
    libcurses)
      LIBTERMINFO="$LIBCURSES"
      LTLIBTERMINFO="$LTLIBCURSES"
      INCTERMINFO="$INCCURSES"
      ;;
  esac
  case "$gl_cv_terminfo" in
    libc | libncurses | libtermcap | libxcurses | libcurses)
      AC_DEFINE([HAVE_TERMINFO], 1,
        [Define if setupterm(), tigetnum(), tigetstr(), tigetflag()
         are among the termcap library functions.])
      ;;
    *)
      dnl Use the termcap functions as a fallback.
      AC_CACHE_CHECK([where termcap library functions come from], [gl_cv_termcap], [
        gl_cv_termcap="not found, consider installing GNU ncurses"
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM(
             [[extern
               #ifdef __cplusplus
               "C"
               #endif
               int tgetent (char *, const char *);
             ]],
             [[return tgetent ((char *) 0, "xterm");]])],
          [gl_cv_termcap=libc])
        if test "$gl_cv_termcap" != libc; then
          gl_save_LIBS="$LIBS"
          LIBS="$LIBS $LIBNCURSES"
          AC_LINK_IFELSE(
            [AC_LANG_PROGRAM(
               [[extern
                 #ifdef __cplusplus
                 "C"
                 #endif
                 int tgetent (char *, const char *);
               ]],
               [[return tgetent ((char *) 0, "xterm");]])],
            [gl_cv_termcap=libncurses])
          LIBS="$gl_save_LIBS"
          if test "$gl_cv_termcap" != libncurses; then
            gl_save_LIBS="$LIBS"
            LIBS="$LIBS $LIBTERMCAP"
            AC_LINK_IFELSE(
              [AC_LANG_PROGRAM(
                 [[extern
                   #ifdef __cplusplus
                   "C"
                   #endif
                   int tgetent (char *, const char *);
                 ]],
                 [[return tgetent ((char *) 0, "xterm");]])],
              [gl_cv_termcap=libtermcap])
            LIBS="$gl_save_LIBS"
          fi
        fi
      ])
      case "$gl_cv_termcap" in
        libc)
          LIBTERMINFO=
          LTLIBTERMINFO=
          INCTERMINFO=
          ;;
        libncurses)
          LIBTERMINFO="$LIBNCURSES"
          LTLIBTERMINFO="$LTLIBNCURSES"
          INCTERMINFO="$INCNCURSES"
          ;;
        libtermcap)
          LIBTERMINFO="$LIBTERMCAP"
          LTLIBTERMINFO="$LTLIBTERMCAP"
          INCTERMINFO="$INCTERMCAP"
          ;;
      esac
      case "$gl_cv_termcap" in
        libc | libncurses | libtermcap)
          AC_DEFINE([HAVE_TERMCAP], 1,
            [Define if tgetent(), tgetnum(), tgetstr(), tgetflag()
             are among the termcap library functions.])
          ;;
      esac
      ;;
  esac
  AC_SUBST([LIBTERMINFO])
  AC_SUBST([LTLIBTERMINFO])
  AC_SUBST([INCTERMINFO])

  dnl Test against the old GNU termcap, which provides a tparam() function
  dnl instead of the classical tparm() function.
  AC_CACHE_CHECK([for tparam], [gl_cv_terminfo_tparam], [
    gl_save_LIBS="$LIBS"
    LIBS="$LIBS $LIBTERMINFO"
    gl_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $INCTERMINFO"
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
         [[extern
           #ifdef __cplusplus
           "C"
           #endif
           char * tparam (const char *, void *, int, ...);
           char buf;
         ]],
         [[return ! tparam ("\033\133%dm", &buf, 1, 8);]])],
      [gl_cv_terminfo_tparam=yes],
      [gl_cv_terminfo_tparam=no])
    CPPFLAGS="$gl_save_CPPFLAGS"
    LIBS="$gl_save_LIBS"
  ])
  if test $gl_cv_terminfo_tparam = yes; then
    AC_DEFINE([HAVE_TPARAM], 1,
      [Define if tparam() is among the termcap library functions.])
  else
    dnl Test whether a tparm() function is provided. It is missing e.g.
    dnl in NetBSD 3.0 libtermcap.
    AC_CACHE_CHECK([for tparm], [gl_cv_terminfo_tparm], [
      gl_save_LIBS="$LIBS"
      LIBS="$LIBS $LIBTERMINFO"
      gl_save_CPPFLAGS="$CPPFLAGS"
      CPPFLAGS="$CPPFLAGS $INCTERMINFO"
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
           [[extern
             #ifdef __cplusplus
             "C"
             #endif
             char * tparm (const char *, ...);
           ]],
           [[return ! tparm ("\033\133%dm", 8);]])],
        [gl_cv_terminfo_tparm=yes], [gl_cv_terminfo_tparm=no])
      CPPFLAGS="$gl_save_CPPFLAGS"
      LIBS="$gl_save_LIBS"
    ])
  fi

  dnl Avoid disturbing the gl_TERMCAP_BODY macro.
  LIBTERMCAP="$gl_save_LIBTERMCAP"
  LTLIBTERMCAP="$gl_save_LTLIBTERMCAP"
  INCTERMCAP="$gl_save_INCTERMCAP"
])
