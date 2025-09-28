dnl
dnl  configure.ac: Configure template for zsh.
dnl  Process this file with autoconf to produce a configure script.
dnl
dnl  Copyright (c) 1995-1997 Richard Coleman
dnl  All rights reserved.
dnl
dnl  Permission is hereby granted, without written agreement and without
dnl  license or royalty fees, to use, copy, modify, and distribute this
dnl  software and to distribute modified versions of this software for any
dnl  purpose, provided that the above copyright notice and the following
dnl  two paragraphs appear in all copies of this software.
dnl
dnl  In no event shall Richard Coleman or the Zsh Development Group be liable
dnl  to any party for direct, indirect, special, incidental, or consequential
dnl  damages arising out of the use of this software and its documentation,
dnl  even if Richard Coleman and the Zsh Development Group have been advised of
dnl  the possibility of such damage.
dnl
dnl  Richard Coleman and the Zsh Development Group specifically disclaim any
dnl  warranties, including, but not limited to, the implied warranties of
dnl  merchantability and fitness for a particular purpose.  The software
dnl  provided hereunder is on an "as is" basis, and Richard Coleman and the
dnl  Zsh Development Group have no obligation to provide maintenance,
dnl  support, updates, enhancements, or modifications.
dnl

AC_INIT
AC_CONFIG_SRCDIR([Src/zsh.h])
AC_PREREQ([2.69])
AC_CONFIG_HEADERS([config.h])

dnl What version of zsh are we building ?
. ${srcdir}/Config/version.mk
echo "configuring for zsh $VERSION"

dnl ----------------------------------------------
dnl CHECK FOR MACHINE/VENDOR/OPERATING SYSTEM TYPE
dnl ----------------------------------------------
dnl Find out machine type, vendor, and operating system
dnl What type of host is this?
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED(MACHTYPE, "$host_cpu",
[Define to be the machine type (microprocessor class or machine model).])
AC_DEFINE_UNQUOTED(VENDOR,   "$host_vendor",
[Define to be a string corresponding the vendor of the machine.])
AC_DEFINE_UNQUOTED(OSTYPE,   "$host_os",
[Define to be the name of the operating system.])

dnl -----------------------------
dnl CHECKING COMMAND LINE OPTIONS
dnl -----------------------------
dnl Handle --program-prefix, --program-suffix, etc.
zsh_ARG_PROGRAM

dnl Handle setting of compile flags (CPPFLAGS, CFLAGS, LDFLAGS, LIBS).
zsh_COMPILE_FLAGS($CPPFLAGS, $CFLAGS, $LDFLAGS, $LIBS)

dnl Do you want to debug zsh?
ifdef([zsh-debug],[undefine([zsh-debug])])dnl
AH_TEMPLATE([DEBUG],
[Define to 1 if you want to debug zsh.])
AC_ARG_ENABLE(zsh-debug,
AS_HELP_STRING([--enable-zsh-debug],[compile with debug code and debugger symbols]),
[if test x$enableval = xyes; then
  AC_DEFINE(DEBUG)
fi])

dnl Do you want zsh memory allocation routines.
ifdef([zsh-mem],[undefine([zsh-mem])])dnl
AH_TEMPLATE([ZSH_MEM],
[Define to 1 if you want to use zsh's own memory allocation routines])
AC_ARG_ENABLE(zsh-mem,
AS_HELP_STRING([--enable-zsh-mem],[compile with zsh memory allocation routines]),
[if test x$enableval = xyes; then
  AC_DEFINE(ZSH_MEM)
fi])

dnl Do you want to debug zsh memory allocation routines.
ifdef([zsh-mem-debug],[undefine([zsh-mem-debug])])dnl
AH_TEMPLATE([ZSH_MEM_DEBUG],
[Define to 1 if you want to debug zsh memory allocation routines.])
AC_ARG_ENABLE(zsh-mem-debug,
AS_HELP_STRING([--enable-zsh-mem-debug],[debug zsh memory allocation routines]),
[if test x$enableval = xyes; then
  AC_DEFINE(ZSH_MEM_DEBUG)
fi])

dnl Do you want to print warnings when errors in memory allocation.
AH_TEMPLATE([ZSH_MEM_WARNING],
[Define to 1 if you want to turn on warnings of memory allocation errors])
ifdef([zsh-mem-warning],[undefine([zsh-mem-warning])])dnl
AC_ARG_ENABLE(zsh-mem-warning,
AS_HELP_STRING([--enable-zsh-mem-warning],[print warnings for errors in memory allocation]),
[if test x$enableval = xyes; then
  AC_DEFINE(ZSH_MEM_WARNING)
fi])

dnl Do you want to turn on error checking for free().
ifdef([zsh-secure-free],[undefine([zsh-secure-free])])dnl
AH_TEMPLATE([ZSH_SECURE_FREE],
[Define to 1 if you want to turn on memory checking for free().])
AC_ARG_ENABLE(zsh-secure-free,
AS_HELP_STRING([--enable-zsh-secure-free],[turn on error checking for free()]),
[if test x$enableval = xyes; then
  AC_DEFINE(ZSH_SECURE_FREE)
fi])

dnl Do you want to debug zsh heap allocation?
dnl Does not depend on zsh-mem.
ifdef([zsh-heap-debug],[undefine([zsh-heap-debug])])dnl
AH_TEMPLATE([ZSH_HEAP_DEBUG],
[Define to 1 if you want to turn on error checking for heap allocation.])
AC_ARG_ENABLE(zsh-heap-debug,
AS_HELP_STRING([--enable-zsh-heap-debug],[turn on error checking for heap allocation]),
[if test x$enableval = xyes; then
  AC_DEFINE(ZSH_HEAP_DEBUG)
fi])

dnl Do you want to allow Valgrind to debug heap allocation?
ifdef([zsh-valgrind],[undefine([zsh-valgrind])])dnl
AH_TEMPLATE([ZSH_VALGRIND],
[Define to 1 if you want to add code for valgrind to debug heap memory.])
AC_ARG_ENABLE(zsh-valgrind,
AS_HELP_STRING([--enable-zsh-valgrind],[turn on support for valgrind debugging of heap memory]),
[if test x$enableval = xyes;  then
  AC_DEFINE(ZSH_VALGRIND)
fi])

dnl Do you want debugging information on internal hash tables.
dnl This turns on the `hashinfo' builtin command.
ifdef([zsh-hash-debug],[undefine([zsh-hash-debug])])dnl
AH_TEMPLATE([ZSH_HASH_DEBUG],
[Define to 1 if you want to get debugging information on internal
 hash tables.  This turns on the `hashinfo' builtin.])
AC_ARG_ENABLE(zsh-hash-debug,
AS_HELP_STRING([--enable-zsh-hash-debug],[turn on debugging of internal hash tables]),
[if test x$enableval = xyes; then
  AC_DEFINE(ZSH_HASH_DEBUG)
fi])

dnl Do you want to dynamically allocate memory on the stack where possible?
ifdef([stack-allocation],[undefine([stack-allocation])])dnl
AH_TEMPLATE([USE_STACK_ALLOCATION],
[Define to 1 if you want to allocate stack memory e.g. with `alloca'.])
AC_ARG_ENABLE(stack-allocation,
AS_HELP_STRING([--enable-stack-allocation],[allocate stack memory e.g. with `alloca']),
[if test x$enableval = xyes; then
  AC_DEFINE(USE_STACK_ALLOCATION)
fi])

dnl Pathnames for global zsh scripts
ifdef([etcdir],[undefine([etcdir])])dnl
AC_ARG_ENABLE(etcdir,
AS_HELP_STRING([--enable-etcdir=DIR],[the default directory for global zsh scripts]),
[etcdir="$enableval"], [etcdir=/etc])

ifdef([zshenv],[undefine([zshenv])])dnl
AC_ARG_ENABLE(zshenv,
AS_HELP_STRING([--enable-zshenv=FILE],[the full pathname of the global zshenv script]),
[zshenv="$enableval"],
[if test "x$etcdir" = xno; then
  zshenv=no
else
  zshenv="$etcdir/zshenv"
fi])
AH_TEMPLATE([GLOBAL_ZSHENV],
[The global file to source absolutely first whenever zsh is run;
 if undefined, don't source anything.])
if test "x$zshenv" != xno; then
  AC_DEFINE_UNQUOTED(GLOBAL_ZSHENV, "$zshenv")
fi

ifdef([zshrc],[undefine([zshrc])])dnl
AC_ARG_ENABLE(zshrc,
AS_HELP_STRING([--enable-zshrc=FILE],[the full pathname of the global zshrc script]),
[zshrc="$enableval"],
[if test "x$etcdir" = xno; then
  zshrc=no
else
  zshrc="$etcdir/zshrc"
fi])
AH_TEMPLATE([GLOBAL_ZSHRC],
[The global file to source whenever zsh is run;
 if undefined, don't source anything])
if test "x$zshrc" != xno; then
  AC_DEFINE_UNQUOTED(GLOBAL_ZSHRC, "$zshrc")
fi

ifdef([zprofile],[undefine([zprofile])])dnl
AC_ARG_ENABLE(zprofile,
AS_HELP_STRING([--enable-zprofile=FILE],[the full pathname of the global zprofile script]),
[zprofile="$enableval"],
[if test "x$etcdir" = xno; then
  zprofile=no
else
  zprofile="$etcdir/zprofile"
fi])
AH_TEMPLATE([GLOBAL_ZPROFILE],
[The global file to source whenever zsh is run as a login shell,
 before zshrc is read; if undefined, don't source anything.])
if test "x$zprofile" != xno; then
  AC_DEFINE_UNQUOTED(GLOBAL_ZPROFILE, "$zprofile")
fi

ifdef([zlogin],[undefine([zlogin])])dnl
AC_ARG_ENABLE(zlogin,
AS_HELP_STRING([--enable-zlogin=FILE],[the full pathname of the global zlogin script]),
[zlogin="$enableval"],
[if test "x$etcdir" = xno; then
  zlogin=no
else
  zlogin="$etcdir/zlogin"
fi])
AH_TEMPLATE([GLOBAL_ZLOGIN],
[The global file to source whenever zsh is run as a login shell;
 if undefined, don't source anything])
if test "x$zlogin" != xno; then
  AC_DEFINE_UNQUOTED(GLOBAL_ZLOGIN, "$zlogin")
fi

ifdef([zlogout],[undefine([zlogout])])dnl
AC_ARG_ENABLE(zlogout,
AS_HELP_STRING([--enable-zlogout=FILE],[the full pathname of the global zlogout script]),
[zlogout="$enableval"],
[if test "x$etcdir" = xno; then
  zlogout=no
else
  zlogout="$etcdir/zlogout"
fi])
AH_TEMPLATE([GLOBAL_ZLOGOUT],
[The global file to source whenever zsh was run as a login shell.
 This is sourced right before exiting.  If undefined, don't source
 anything.])
if test "x$zlogout" != xno; then
  AC_DEFINE_UNQUOTED(GLOBAL_ZLOGOUT, "$zlogout")
fi

AC_SUBST(zshenv)dnl
AC_SUBST(zshrc)dnl
AC_SUBST(zprofile)dnl
AC_SUBST(zlogin)dnl
AC_SUBST(zlogout)dnl

dnl Do you want dynamically loaded binary modules.
ifdef([dynamic],[undefine([dynamic])])dnl
AC_ARG_ENABLE(dynamic,
AS_HELP_STRING([--disable-dynamic],[turn off dynamically loaded binary modules]),
[dynamic="$enableval"], [dynamic=yes])

dnl Do you want to disable restricted on r* commands
ifdef([restricted-r],[undefine([restricted-r])])dnl
AH_TEMPLATE([RESTRICTED_R],
[Undefine this if you don't want to get a restricted shell
 when zsh is exec'd with basename that starts with r.
 By default this is defined.])
AC_ARG_ENABLE(restricted-r,
AS_HELP_STRING([--disable-restricted-r],[turn off r* invocation for restricted shell]),
[if test x$enableval = xyes; then
  AC_DEFINE(RESTRICTED_R)
fi],
AC_DEFINE(RESTRICTED_R)
)

dnl Do you want to disable use of locale functions
AH_TEMPLATE([CONFIG_LOCALE],
[Undefine if you don't want local features.  By default this is defined.])
AC_ARG_ENABLE([locale],
AS_HELP_STRING([--disable-locale],[turn off locale features]),
[if test x$enableval = xyes; then
  AC_DEFINE(CONFIG_LOCALE)
fi],
AC_DEFINE(CONFIG_LOCALE)
)

dnl Do you want to compile as K&R C.
AC_ARG_ENABLE(ansi2knr,
AS_HELP_STRING([--enable-ansi2knr],[translate source to K&R C before compiling]),
[ansi2knr="$enableval"], [ansi2knr=default])

ifdef([runhelpdir],[undefine([runhelpdir])])dnl
AC_ARG_ENABLE(runhelpdir,
AS_HELP_STRING([--enable-runhelpdir=DIR],[the directory in which to install run-help files]),
[if test x"$enableval" = xno; then
  runhelpdir=
else
  runhelpdir="$enableval"
fi], [runhelpdir=yes])
if test x"$runhelpdir" = xyes; then
  runhelpdir=${datadir}/${tzsh_name}/'${VERSION}'/help
fi
if test x"$runhelpdir" = x; then
  runhelp=
else
  runhelp=runhelp
fi

ifdef([fndir],[undefine([fndir])])dnl
AC_ARG_ENABLE(fndir,
AS_HELP_STRING([--enable-fndir=DIR],[the directory in which to install functions]),
dnl ${VERSION} to be determined at compile time.
[if test x$enableval = xyes; then
  fndir=${datadir}/${tzsh_name}/'${VERSION}'/functions
else
  fndir="$enableval"
fi], [fndir=${datadir}/${tzsh_name}/'${VERSION}'/functions])

ifdef([sitefndir],[undefine([sitefndir])])dnl
AC_ARG_ENABLE(site-fndir,
AS_HELP_STRING([--enable-site-fndir=DIR],[same for site functions (not version specific)]),
[if test x$enableval = xyes; then
  sitefndir=${datadir}/${tzsh_name}/site-functions
else
  sitefndir="$enableval"
fi], [sitefndir=${datadir}/${tzsh_name}/site-functions])

dnl Add /usr/local/share/zsh/site-functions if not yet present
dnl owing to $sitefndir, whether or not explicitly given.
dnl If not explicitly given, it hasn't been expanded yet.
if test X$sitefndir = X/usr/local/share/zsh/site-functions || \
   test X$sitefndir = Xno
then fixed_sitefndir=''
elif test X$prefix != X/usr/local; then
  if test X$prefix = XNONE && test X$ac_default_prefix = X/usr/local; then
    if test X$tzsh_name != Xzsh
    then fixed_sitefndir=/usr/local/share/zsh/site-functions
    else fixed_sitefndir=''
    fi
  else fixed_sitefndir=/usr/local/share/zsh/site-functions
  fi
elif test X$tzsh_name != Xzsh
then fixed_sitefndir=/usr/local/share/zsh/site-functions
else fixed_sitefndir=''
fi

ifdef([function_subdirs],[undefine([function_subdirs])])
AC_ARG_ENABLE(function-subdirs,
AS_HELP_STRING([--enable-function-subdirs],[install functions in subdirectories]))

if test "x${enable_function_subdirs}" != x &&
  test "x${enable_function_subdirs}" != xno; then
  FUNCTIONS_SUBDIRS=yes
else
  FUNCTIONS_SUBDIRS=no
fi

ifdef([additionalfpath],[undefine([additionalfpath])])dnl
AC_ARG_ENABLE(additional-fpath,
AS_HELP_STRING([--enable-additional-fpath=DIR],[add directories to default function path]),
[if test x$enableval = xyes; then
  additionalfpath=""
else
  additionalfpath="${enableval}"
fi], [additionalfpath=""])

AC_SUBST(runhelpdir)dnl
AC_SUBST(runhelp)dnl
AC_SUBST(additionalfpath)dnl
AC_SUBST(fndir)dnl
AC_SUBST(sitefndir)dnl
AC_SUBST(fixed_sitefndir)dnl
AC_SUBST(FUNCTIONS_SUBDIRS)dnl

dnl Directories for scripts such as newuser.

ifdef([scriptdir],[undefine([scriptdir])])dnl
AC_ARG_ENABLE(scriptdir,
AS_HELP_STRING([--enable-scriptdir=DIR],[the directory in which to install scripts]),
dnl ${VERSION} to be determined at compile time.
[if test x$enableval = xyes; then
  scriptdir=${datadir}/${tzsh_name}/'${VERSION}'/scripts
else
  scriptdir="$enableval"
fi], [scriptdir=${datadir}/${tzsh_name}/'${VERSION}'/scripts])

ifdef([sitescriptdir],[undefine([sitescriptdir])])dnl
AC_ARG_ENABLE(site-scriptdir,
AS_HELP_STRING([--enable-site-scriptdir=DIR],[same for site scripts (not version specific)]),
[if test x$enableval = xyes; then
  sitescriptdir=${datadir}/${tzsh_name}/scripts
else
  sitescriptdir="$enableval"
fi], [sitescriptdir=${datadir}/${tzsh_name}/scripts])

AC_SUBST(scriptdir)dnl
AC_SUBST(sitescriptdir)dnl

dnl htmldir is already handled, but if it wasn't set, use
dnl the standard zsh default.
if test x$htmldir = x'${docdir}' || test x$htmldir = x; then
  htmldir='$(datadir)/$(tzsh)/htmldoc'
fi

AH_TEMPLATE([CUSTOM_PATCHLEVEL],
[Define to a custom value for the ZSH_PATCHLEVEL parameter])
AC_ARG_ENABLE(custom-patchlevel,
AS_HELP_STRING([--enable-custom-patchlevel],[set a custom ZSH_PATCHLEVEL value]),
[if test x$enableval != x && test x$enableval != xno; then
  AC_DEFINE_UNQUOTED([CUSTOM_PATCHLEVEL], ["$enableval"])
fi])

dnl Do you want maildir support?
ifdef([maildir_support],[undefine([maildir_support])])dnl
AH_TEMPLATE([MAILDIR_SUPPORT],
[Define for Maildir support])
AC_ARG_ENABLE(maildir-support,
AS_HELP_STRING([--enable-maildir-support],[enable maildir support in MAIL and MAILPATH]),
[if test x$enableval = xyes; then
  AC_DEFINE(MAILDIR_SUPPORT)
fi])

dnl Do you want to set a maximum function depth?
ifdef([max_function_depth],[undefine([max_function_depth])])dnl
AH_TEMPLATE([MAX_FUNCTION_DEPTH],
[Define for function depth limits])
AC_ARG_ENABLE(max-function-depth,
AS_HELP_STRING([--enable-max-function-depth=MAX],[limit function depth to MAX, default 500]),
[if test x$enableval = xyes; then
  AC_DEFINE(MAX_FUNCTION_DEPTH, 500)
elif test x$enableval != xno; then
  AC_DEFINE_UNQUOTED(MAX_FUNCTION_DEPTH, $enableval)
fi],
[AC_DEFINE(MAX_FUNCTION_DEPTH, 500)]
)

ifdef([default_readnullcmd],[undefine([default_readnullcmd])])dnl
AH_TEMPLATE([DEFAULT_READNULLCMD],
[Define default pager used by readnullcmd])
AC_ARG_ENABLE(readnullcmd,
AS_HELP_STRING([--enable-readnullcmd=PAGER],[pager used when READNULLCMD is not set]),
[if test x$enableval = xyes; then
  AC_DEFINE(DEFAULT_READNULLCMD,"more")
elif test x$enableval != xno; then
  AC_DEFINE_UNQUOTED(DEFAULT_READNULLCMD,"$enableval")
fi],
[AC_DEFINE(DEFAULT_READNULLCMD,"more")]
)

dnl Do you want to look for pcre support?
AC_ARG_ENABLE(pcre,
AS_HELP_STRING([--enable-pcre],[enable the search for the pcre library (may create run-time library dependencies)]))

dnl Do you want to look for capability support?
AC_ARG_ENABLE(cap,
AS_HELP_STRING([--enable-cap],[enable the search for POSIX capabilities (may require additional headers to be added by hand)]))

# Default off for licensing reasons
AC_ARG_ENABLE(gdbm,
AS_HELP_STRING([--enable-gdbm],[enable the search for the GDBM library (see the zsh/db/gdbm module)]),
[gdbm="$enableval"], [gdbm=no])

dnl ------------------
dnl CHECK THE COMPILER
dnl ------------------
dnl We want these before the checks, so the checks can modify their values.
test -z "${CFLAGS+set}"  && CFLAGS=  auto_cflags=1
test -z "${LDFLAGS+set}" && LDFLAGS= auto_ldflags=1

AC_PROG_CC

dnl Check for large file support.

dnl Gross hack for ReliantUNIX - GCC does not understand getconf options
dnl For now just disable LFS in this case
dnl Any takers?
if test "$host" = mips-sni-sysv4 && test -n "$GCC"; then
  : 
else
  AC_SYS_LARGEFILE
fi

dnl if the user hasn't specified CFLAGS, then
dnl   if compiler is gcc, then use -O2 and some warning flags
dnl   else use -O
if test -n "$auto_cflags" && test ."$ansi2knr" != .yes; then
  if test "${enable_zsh_debug}" = yes; then
    if test -n "$GCC"; then
      CFLAGS="$CFLAGS -Wall -Wmissing-prototypes -ggdb"
    else
      CFLAGS="$CFLAGS -g"
    fi
  else
    if test -n "$GCC"; then
      CFLAGS="$CFLAGS -Wall -Wmissing-prototypes -O2"
    else
      CFLAGS="$CFLAGS -O"
    fi
  fi
fi
if test -n "$auto_ldflags"; then
  case "${enable_zsh_debug}$host_os" in
    yesaix*|yeshpux*|yesnetbsd*|yesopenbsd*) ;;  # "ld -g" is not valid on these systems
    darwin*) LDFLAGS=-Wl,-x ;;
    yes*)    LDFLAGS=-g ;;
    *)       LDFLAGS=-s ;;
  esac
fi

dnl ----------
dnl SCO KLUDGE
dnl ----------
dnl Sco doesn't define any useful compiler symbol,
dnl so we will check for sco and define __sco if
dnl found.
case "$host_os" in
  sco*) CFLAGS="-D__sco $CFLAGS" ;;
esac

sed=':1
     s/ -s / /g
     t1
     s/^ *//
     s/ *$//'

case " $LDFLAGS " in
  *" -s "*) strip_exeldflags=true strip_libldflags=true
    LDFLAGS=`echo " $LDFLAGS " | sed "$sed"` ;;
  *) strip_exeldflags=false strip_libldflags=false ;;
esac

case " ${EXELDFLAGS+$EXELDFLAGS }" in
  " ") ;;
  *" -s "*) strip_exeldflags=true
    EXELDFLAGS=`echo " $EXELDFLAGS " | sed "$sed"` ;;
  *) strip_exeldflags=false ;;
esac

case " ${LIBLDFLAGS+$LIBLDFLAGS }" in
  " ") ;;
  *" -s "*) strip_libldflags=true
    LIBLDFLAGS=`echo " $LIBLDFLAGS " | sed "$sed"` ;;
  *) strip_libldflags=false ;;
esac

AC_SUBST(CFLAGS)dnl
AC_SUBST(LDFLAGS)dnl
AC_SUBST(EXELDFLAGS)dnl
AC_SUBST(LIBLDFLAGS)dnl

AC_PROG_CPP                 dnl Figure out how to run C preprocessor.
AC_C_CONST                  dnl Does compiler support `const'.

dnl Default preprocessing on Mac OS X produces warnings
dnl Mac OS X 10.6 (darwin10.x.x) does not need this.
case "$host_os" in
  darwin[[0-9]].*) CPP="$CPP -traditional-cpp" ;;
esac

fp_PROG_CC_STDC
AC_MSG_CHECKING([whether to use prototypes])
if test ."$ansi2knr" = .yes || test ."$ansi2knr" = .no; then
  msg="(overridden) "
else
  msg=
  if test ."$fp_cv_prog_cc_stdc" = .no; then
    ansi2knr=yes
  else
    ansi2knr=no
  fi
fi
AH_TEMPLATE([PROTOTYPES],
[Define to 1 if ANSI function prototypes are usable.])
if test "$ansi2knr" = yes; then
  AC_MSG_RESULT(${msg}no)
  U=_
else
  AC_MSG_RESULT(${msg}yes)
  AC_DEFINE(PROTOTYPES)
  U=
fi
AC_SUBST(U)

AC_FUNC_ALLOCA              dnl Check how to get `alloca'.

dnl If the compiler supports union initialisation
AC_CACHE_CHECK(if the compiler supports union initialisation,
zsh_cv_c_have_union_init,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[union{void *p;long l;}u={0};]], [[u.l=1;]])],[zsh_cv_c_have_union_init=yes],[zsh_cv_c_have_union_init=no])])
AH_TEMPLATE([HAVE_UNION_INIT],
[Define to 1 if the compiler can initialise a union.])
if test x$zsh_cv_c_have_union_init = xyes; then
  AC_DEFINE(HAVE_UNION_INIT)
fi

dnl  Checking if compiler correctly cast signed to unsigned.
AC_CACHE_CHECK(if signed to unsigned casting is broken,
zsh_cv_c_broken_signed_to_unsigned_casting,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[main(){return((int)(unsigned char)((char) -1) == 255);}]])],[zsh_cv_c_broken_signed_to_unsigned_casting=yes],[zsh_cv_c_broken_signed_to_unsigned_casting=no],[zsh_cv_c_broken_signed_to_unsigned_casting=no])])
AH_TEMPLATE([BROKEN_SIGNED_TO_UNSIGNED_CASTING],
[Define to 1 if compiler incorrectly cast signed to unsigned.])
if test x$zsh_cv_c_broken_signed_to_unsigned_casting = xyes; then
  AC_DEFINE(BROKEN_SIGNED_TO_UNSIGNED_CASTING)
fi

dnl Checking if the compiler supports variable-length arrays
AC_CACHE_CHECK(if the compiler supports variable-length arrays,
zsh_cv_c_variable_length_arrays,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[int foo(), n;]], [[int i[foo()], a[n+1];]])],[zsh_cv_c_variable_length_arrays=yes],[zsh_cv_c_variable_length_arrays=no])])
AH_TEMPLATE([HAVE_VARIABLE_LENGTH_ARRAYS],
[Define to 1 if compiler supports variable-length arrays])
if test x$zsh_cv_c_variable_length_arrays = xyes; then
  AC_DEFINE(HAVE_VARIABLE_LENGTH_ARRAYS)
fi

dnl ------------------
dnl CHECK FOR PROGRAMS
dnl ------------------
AC_PROG_MAKE_SET            dnl Does make define $MAKE
AC_PROG_INSTALL             dnl Check for BSD compatible `install'
AC_PROG_AWK                 dnl Check for mawk,gawk,nawk, then awk.
AC_PROG_LN                  dnl Check for working ln, for "make install"
AC_PROG_LN_S                dnl Use ln -s/ln/cp for "make install.runhelp"
AC_PROG_EGREP               dnl sets $EGREP to grep -E or egrep
AC_CHECK_PROGS([YODL], [yodl], [: yodl])

YODL_OPTIONS=''
if test "x$ac_cv_prog_YODL" = xyodl; then
    case `yodl --version` in
      *"version 2."*) YODL_OPTIONS='-k' ;;
      *"version 3."*) YODL_OPTIONS='-k -L' ;;
      *"version 4."*) YODL_OPTIONS='-k -L' ;;
    esac
fi
AC_SUBST(YODL_OPTIONS)

AC_CHECK_PROGS([TEXI2DVI], [texi2dvi], [: texi2dvi])
AC_CHECK_PROGS([TEXI2PDF], [texi2pdf], [: texi2pdf])
AC_CHECK_PROGS([TEXI2HTML], [texi2any texi2html], [: texi2html])

if test x"$TEXI2PDF" != xtexi2pdf && test x"$TEXI2DVI" = xtexi2dvi; then
  TEXI2PDF='texi2dvi --pdf'
fi

if test x"$TEXI2HTML" = xtexi2any; then
  TEXI2HTML='texi2any -c TEXI2HTML=1'
fi

case "$LC_PAPER" in
  ??_US*) PAPERSIZE=us ;;
  *)      PAPERSIZE=a4 ;;
esac
AC_SUBST(PAPERSIZE)

AC_CHECK_PROGS([ANSI2KNR], [ansi2knr], [: ansi2knr])

if test x"$ansi2knr" = xyes && test x"$ANSI2KNR" = x": ansi2knr"; then
    echo "----------"
    echo "configure fatal error:"
    echo "ansi2knr was specified (--enable-ansi2knr) but the program could not be found."
    echo "Either remove the configure option if it is not required or build the ansi2knr"
    echo "program before reconfiguring Zsh.  The source code for ansi2knr is also"
    echo "available in the GPL directory on Zsh distribution sites."
    exit 1
fi

dnl ------------------
dnl CHECK HEADER FILES
dnl ------------------
AC_HEADER_DIRENT
AC_HEADER_STAT
AC_HEADER_SYS_WAIT

oldcflags="$CFLAGS"
if test x$enable_pcre = xyes; then
AC_CHECK_PROG([PCRECONF], pcre-config, pcre-config)
dnl Typically (meaning on this single RedHat 9 box in front of me)
dnl pcre-config --cflags produces a -I output which needs to go into
dnl CPPFLAGS else configure's preprocessor tests don't pick it up,
dnl producing a warning.
if test "x$ac_cv_prog_PCRECONF" = xpcre-config; then
  CPPFLAGS="$CPPFLAGS `pcre-config --cflags`"
fi
fi

AC_CHECK_HEADERS(sys/time.h sys/times.h sys/select.h termcap.h termio.h \
		 termios.h sys/param.h sys/filio.h string.h memory.h \
		 limits.h fcntl.h libc.h sys/utsname.h sys/resource.h \
		 locale.h errno.h stdio.h stdarg.h varargs.h stdlib.h \
		 unistd.h sys/capability.h \
		 utmp.h utmpx.h sys/types.h pwd.h grp.h poll.h sys/mman.h \
		 netinet/in_systm.h pcre.h langinfo.h wchar.h stddef.h \
		 sys/stropts.h iconv.h ncurses.h ncursesw/ncurses.h \
		 ncurses/ncurses.h)
if test x$dynamic = xyes; then
  AC_CHECK_HEADERS(dlfcn.h)
  AC_CHECK_HEADERS(dl.h)
fi

dnl Some SCO systems cannot include both sys/time.h and sys/select.h
AH_TEMPLATE([TIME_H_SELECT_H_CONFLICTS],
[Define if sys/time.h and sys/select.h cannot be both included.])
if test x$ac_cv_header_sys_time_h = xyes && test x$ac_cv_header_sys_select_h = xyes; then
  AC_CACHE_CHECK(for conflicts in sys/time.h and sys/select.h,
  zsh_cv_header_time_h_select_h_conflicts,
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/time.h>
#include <sys/select.h>]], [[int i;]])],[zsh_cv_header_time_h_select_h_conflicts=no],[zsh_cv_header_time_h_select_h_conflicts=yes])])
  if test x$zsh_cv_header_time_h_select_h_conflicts = xyes; then
    AC_DEFINE(TIME_H_SELECT_H_CONFLICTS)
  fi
fi

AH_TEMPLATE([GWINSZ_IN_SYS_IOCTL],
[Define if TIOCGWINSZ is defined in sys/ioctl.h but not in termios.h.])
if test x$ac_cv_header_termios_h = xyes; then
  AC_CACHE_CHECK(TIOCGWINSZ in termios.h,
  zsh_cv_header_termios_h_tiocgwinsz,
  [AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <termios.h>]], [[int x = TIOCGWINSZ;]])],[zsh_cv_header_termios_h_tiocgwinsz=yes],[zsh_cv_header_termios_h_tiocgwinsz=no])])
else
  zsh_cv_header_termios_h_tiocgwinsz=no
fi
if test x$zsh_cv_header_termios_h_tiocgwinsz = xno; then
  AC_CACHE_CHECK(TIOCGWINSZ in sys/ioctl.h,
  zsh_cv_header_sys_ioctl_h_tiocgwinsz,
  [AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <sys/ioctl.h>]], [[int x = TIOCGWINSZ;]])],[zsh_cv_header_sys_ioctl_h_tiocgwinsz=yes],[zsh_cv_header_sys_ioctl_h_tiocgwinsz=no])])
  if test x$zsh_cv_header_sys_ioctl_h_tiocgwinsz = xyes; then
    AC_DEFINE(GWINSZ_IN_SYS_IOCTL)
  fi
fi
 
AH_TEMPLATE([WINSIZE_IN_PTEM],
[Define if your should include sys/stream.h and sys/ptem.h.])
AC_CACHE_CHECK(for streams headers including struct winsize,
ac_cv_winsize_in_ptem,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/stream.h>
#include <sys/ptem.h>]], [[struct winsize wsz]])],[ac_cv_winsize_in_ptem=yes],[ac_cv_winsize_in_ptem=no])])
if test x$ac_cv_winsize_in_ptem = xyes; then
  AC_DEFINE(WINSIZE_IN_PTEM)
fi

dnl -------------------
dnl CHECK FOR LIBRARIES
dnl -------------------

dnl On some systems, modules need to be linked against libc explicitly,
dnl in case they require objects that exist only in the static version
dnl and might not be compiled into the zsh executable.
dnl On ReliantUNIX -lc better be the last library, else funny things
dnl may happen.
AC_CHECK_LIB(c, printf, [LIBS="$LIBS -lc"])

AC_CHECK_LIB(m, pow)

AC_CHECK_LIB(rt, clock_gettime)

dnl Various features of ncurses depend on having the right header
dnl (the system's own curses.h may well not be good enough).
dnl So don't search for ncurses unless we found the header.
if test x$ac_cv_header_ncurses_h = xyes || test x$ac_cv_header_ncurses_ncurses_h = xyes || test x$ac_cv_header_ncursesw_ncurses_h = xyes; then
  ncursesw_test=ncursesw
  ncurses_test=ncurses
else
  ncursesw_test=
  ncurses_test=
fi

dnl Prefer BSD termcap library to SysV curses library, except on certain
dnl SYSV-derived systems.  However, if we find terminfo and termcap
dnl stuff in the same library we will use that; typically this
dnl is ncurses or curses.
dnl On pre-11.11 HPUX, Hcurses is reported to work better than curses.
dnl Prefer ncurses to curses on all systems.  tinfo isn't very common now.
AC_ARG_WITH(term-lib,
AS_HELP_STRING([--with-term-lib=LIBS],[search space-separated LIBS for terminal handling]),
[if test "x$withval" != xno && test "x$withval" != x ; then
  termcap_curses_order="$withval"
  AC_SEARCH_LIBS(tigetstr, [$termcap_curses_order])
else
  termcap_curses_order="$ncursesw_test $ncurses_test tinfow tinfo termcap curses"
fi],
[case "$host_os" in
  solaris*)
   termcap_curses_order="$ncursesw_test $ncurses_test curses termcap" ;;
  hpux10.*|hpux11.*)
   DL_EXT="${DL_EXT=sl}"
   termcap_curses_order="Hcurses $ncursesw_test $ncurses_test curses termcap" ;;
  *)
   termcap_curses_order="$ncursesw_test $ncurses_test tinfow tinfo termcap curses" ;;
esac])dnl

AH_TEMPLATE([ZSH_NO_XOPEN],
[Define if _XOPEN_SOURCE_EXTENDED should not be defined to avoid clashes])
AC_CACHE_CHECK(if _XOPEN_SOURCE_EXTENDED should not be defined,
zsh_cv_no_xopen,
[[case "$host_os" in
  *freebsd5*|*freebsd6.[012]*|*aix*)
  zsh_cv_no_xopen=yes
  ;;
  *)
  zsh_cv_no_xopen=no
  ;;
esac]])
if test x$zsh_cv_no_xopen = xyes; then
  AC_DEFINE(ZSH_NO_XOPEN)
fi

dnl Check for tigetflag (terminfo) before tgetent (termcap).
dnl That's so that on systems where termcap and [n]curses are
dnl both available and both contain termcap functions, while
dnl only [n]curses contains terminfo functions, we only link against
dnl [n]curses.
LIBS_save_pre_term="$LIBS"
AC_SEARCH_LIBS(tigetstr, [$termcap_curses_order])
AC_SEARCH_LIBS(tigetflag, [$termcap_curses_order])
AC_SEARCH_LIBS(tgetent, [$termcap_curses_order],
  true,
  AC_MSG_FAILURE(["No terminal handling library was found on your system.
This is probably a library called 'curses' or 'ncurses'.  You may
need to install a package called 'curses-devel' or 'ncurses-devel' on your
system."], 255))
AC_CHECK_HEADERS(curses.h, [],
[AC_CACHE_CHECK(for Solaris 8 curses.h mistake, ac_cv_header_curses_solaris,
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <curses.h>]], [[]])],[ac_cv_header_curses_h=yes
ac_cv_header_curses_solaris=yes],[ac_cv_header_curses_h=no
ac_cv_header_curses_solaris=no]))
if test x$ac_cv_header_curses_solaris = xyes; then
AC_DEFINE(HAVE_CURSES_H)
fi])

dnl If our terminal library is not ncurses, don't try including
dnl any ncurses headers.
AC_CACHE_CHECK(if we need to ignore ncurses, zsh_cv_ignore_ncurses,
[case $LIBS in
  *-lncurses*)
  zsh_cv_ignore_ncurses=no
  ;;
  *)
    dnl The lack of -lncurses in the $LIBS might be the result of passing
    dnl --with-term-lib=^ncurses option. To address this, a test for the tgetent
    dnl and other functions is ran here, possibly for the second time, just to
    dnl ensure that the ncurses library doesn't have them.
    LIBS_save="$LIBS"
    dnl Remember (the values are used later, around line 3005) and remove the cache
    ac_cv_search_tigetstr_SAVE="$ac_cv_search_tigetstr"
    ac_cv_search_tigetnum_SAVE="$ac_cv_search_tigetnum"
    ac_cv_search_tigetflag_SAVE="$ac_cv_search_tigetflag"
    ac_cv_search_tgetent_SAVE="$ac_cv_search_tgetent"
    unset ac_cv_search_tigetstr ac_cv_search_tigetnum ac_cv_search_tigetflag ac_cv_search_tgetent
    LIBS="$LIBS_save_pre_term"

    dnl Run the checks for all four used terminal functions
    AC_SEARCH_LIBS(tigetstr, [ncursesw ncurses curses])
    AC_SEARCH_LIBS(tigetnum, [ncursesw ncurses curses])
    AC_SEARCH_LIBS(tigetflag, [ncursesw ncurses curses])
    AC_SEARCH_LIBS(tgetent, [ncursesw ncurses curses])
    LIBS_result="$LIBS"

    LIBS="$LIBS_save"
    dnl Restore the cache
    ac_cv_search_tigetstr="$ac_cv_search_tigetstr_SAVE"
    ac_cv_search_tigetnum="$ac_cv_search_tigetnum_SAVE"
    ac_cv_search_tigetflag="$ac_cv_search_tigetflag_SAVE"
    ac_cv_search_tgetent="$ac_cv_search_tgetent_SAVE"

    case $LIBS_result in
      *-lncurses*|*-lcurses*)
        dnl Yes we need to ignore ncurses, its tgetent or tigetflag might
        dnl conflict with the one from the selected terminal library
        zsh_cv_ignore_ncurses=yes
        ;;
      *)
        dnl If the tgetent nor tigetflag weren't found in the libncurses*.so, then
        dnl there will be no conflict with the other terminal library selected (e.g.
        dnl libtinfo) and it's possible to link ncurses provided that it is working
        dnl - it is here verified that it has initscr() function to check that
        AC_SEARCH_LIBS(initscr, [ncursesw ncurses curses])
        case $LIBS in
          *-lncurses*|*-lcurses*)
            dnl No need to ignore curses - it is working and it doesn't
            dnl have tgetent nor tigetflag
            zsh_cv_ignore_ncurses=no
          ;;
          *)
            zsh_cv_ignore_ncurses=yes
          ;;
        esac
    esac
  ;;
esac])

AC_SEARCH_LIBS(getpwnam, nsl)

dnl I am told that told that unicos reqire these for nis_list
if test `echo $host_os | sed 's/^\(unicos\).*/\1/'` = unicos; then
  LIBS="-lcraylm -lkrb -lnisdb -lnsl -lrpcsvc $LIBS"
fi

if test "x$dynamic" = xyes; then
  AC_CHECK_LIB(dl, dlopen)
fi

if test x$enable_cap = xyes; then
  AC_CHECK_LIB(cap, cap_get_proc)
fi

AC_CHECK_LIB(socket, socket)
AC_SEARCH_LIBS(gethostbyname2, bind)

case $LIBS in
  *-lbind*)
  AC_CHECK_HEADERS(bind/netdb.h)
  ;;
esac

dnl ---------------
dnl CHECK FOR ICONV
dnl ---------------

dnl Find iconv. It may be in libiconv and may be iconv() or libiconv()
if test "x$ac_cv_header_iconv_h" = "xyes"; then
  AC_CHECK_FUNC(iconv, ac_found_iconv=yes, ac_found_iconv=no)
  if test "x$ac_found_iconv" = "xno"; then
    AC_CHECK_LIB(iconv, iconv, ac_found_iconv=yes)
    if test "x$ac_found_iconv" = "xno"; then
      AC_CHECK_LIB(iconv, libiconv, ac_found_iconv=yes)
    fi
    if test "x$ac_found_iconv" != "xno"; then
      LIBS="-liconv $LIBS"
    fi
  else
    dnl Handle case where there is a native iconv but iconv.h is from libiconv
    AC_CHECK_DECL(_libiconv_version,
      [ AC_CHECK_LIB(iconv, libiconv, LIBS="-liconv $LIBS") ],,
      [ #include <iconv.h> ])
  fi
fi
AH_TEMPLATE([ICONV_FROM_LIBICONV],
[Define to 1 if iconv() is linked from libiconv])
if test "x$ac_found_iconv" = xyes; then
  AC_DEFINE(HAVE_ICONV, 1, [Define if you have the iconv() function.])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <iconv.h>]], [[int myversion = _libiconv_version]])],[AC_DEFINE(ICONV_FROM_LIBICONV)],[])
fi

dnl Check if iconv uses const in prototype declaration
if test "x$ac_found_iconv" = "xyes"; then
  AC_CACHE_CHECK(for iconv declaration, ac_cv_iconv_const,
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>
        #include <iconv.h>]],
        [[#ifdef __cplusplus
          "C"
          #endif
          #if defined(__STDC__) || defined(__cplusplus)
          size_t iconv (iconv_t cd, char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);
          #else
          size_t iconv();
          #endif]])],
      [ac_cv_iconv_const=],
      [ac_cv_iconv_const=const])])
  AC_DEFINE_UNQUOTED([ICONV_CONST], $ac_cv_iconv_const,
    [Define as const if the declaration of iconv() needs const.])
fi

if test x$enable_pcre = xyes; then
dnl pcre-config should probably be employed here
dnl AC_SEARCH_LIBS(pcre_compile, pcre)
  LIBS="`$ac_cv_prog_PCRECONF --libs` $LIBS"
fi

dnl ---------------------
dnl CHECK TERMCAP LIBRARY
dnl ---------------------
dnl Checks for external variable ospeed in the termcap library.
AC_CACHE_CHECK(if an include file defines ospeed,
zsh_cv_decl_ospeed_include_defines,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#if HAVE_TERMIOS_H
#include <termios.h>
#endif
#if HAVE_TERMCAP_H
#include <termcap.h>
#endif]], [[ospeed = 0;]])],[zsh_cv_decl_ospeed_include_defines=yes],[zsh_cv_decl_ospeed_include_defines=no])])

if test x$zsh_cv_decl_ospeed_include_defines = xno; then
  AC_CACHE_CHECK(if you must define ospeed,
  zsh_cv_decl_ospeed_must_define,
  [AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[extern short ospeed; ospeed = 0;]])],[zsh_cv_decl_ospeed_must_define=yes],[zsh_cv_decl_ospeed_must_define=no])])
fi

AH_TEMPLATE([HAVE_OSPEED],
[Define to 1 if your termcap library has the ospeed variable])
AH_TEMPLATE([MUST_DEFINE_OSPEED],
[Define to 1 if you have ospeed, but it is not defined in termcap.h])
if test x$zsh_cv_decl_ospeed_include_defines = xyes; then
  AC_DEFINE(HAVE_OSPEED)
elif test x$zsh_cv_decl_ospeed_must_define = xyes; then
  AC_DEFINE(HAVE_OSPEED)
  AC_DEFINE(MUST_DEFINE_OSPEED)
fi

if test x$gdbm != xno; then
  AC_CHECK_HEADERS(gdbm.h)
  AC_CHECK_LIB(gdbm, gdbm_open)
fi

AC_CHECK_HEADERS(sys/xattr.h)

dnl --------------
dnl CHECK TYPEDEFS
dnl --------------

AC_TYPE_PID_T
AC_TYPE_OFF_T
AC_CHECK_TYPE(ino_t, unsigned long)
AC_TYPE_MODE_T
AC_TYPE_UID_T
AC_TYPE_SIZE_T

dnl ------------------------------------------------
dnl Check size of long and try to find a 64-bit type
dnl ------------------------------------------------
dnl AC_CHECK_SIZEOF is no good, because we need the result here,
dnl and that doesn't seem to define a shell parameter.
AC_CACHE_CHECK(if long is 64 bits, zsh_cv_long_is_64_bit,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[int main() { return sizeof(long) < 8; }]])],[zsh_cv_long_is_64_bit=yes],[zsh_cv_long_is_64_bit=no],[zsh_cv_long_is_64_bit=no])])

AH_TEMPLATE([ino_t],
[Define to `unsigned long' if <sys/types.h> doesn't define.])
AH_TEMPLATE([LONG_IS_64_BIT],
[Definitions used when a long is less than eight byte, to try to
 provide some support for eight byte operations.

 Note that ZSH_64_BIT_TYPE, OFF_T_IS_64_BIT, INO_T_IS_64_BIT do *not* get
 defined if long is already 64 bits, since in that case no special handling
 is required.

 Define to 1 if long is 64 bits])
AH_TEMPLATE([ZSH_64_BIT_TYPE],
[Define to a 64 bit integer type if there is one, but long is shorter.])
AH_TEMPLATE([ZSH_64_BIT_UTYPE],
[Define to an unsigned variant of ZSH_64_BIT_TYPE if that is defined.])
AH_TEMPLATE([OFF_T_IS_64_BIT],
[Define to 1 if off_t is 64 bit (for large file support)])
AH_TEMPLATE([INO_T_IS_64_BIT],
[Define to 1 if ino_t is 64 bit (for large file support).])
if test x$zsh_cv_long_is_64_bit = xyes; then
  AC_DEFINE(LONG_IS_64_BIT)
else
  AC_CACHE_CHECK(if off_t is 64 bit, zsh_cv_off_t_is_64_bit,
  [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>

main() { return sizeof(off_t) < 8; }
]])],[zsh_cv_off_t_is_64_bit=yes],[zsh_cv_off_t_is_64_bit=no],[zsh_cv_off_t_is_64_bit=no])])
  if test x$zsh_cv_off_t_is_64_bit = xyes; then
    AC_DEFINE(OFF_T_IS_64_BIT)
  fi

  AC_CACHE_CHECK(if ino_t is 64 bit, zsh_cv_ino_t_is_64_bit,
  [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>

main() { return sizeof(ino_t) < 8; }
]])],[zsh_cv_ino_t_is_64_bit=yes],[zsh_cv_ino_t_is_64_bit=no],[zsh_cv_ino_t_is_64_bit=no])])
  if test x$zsh_cv_ino_t_is_64_bit = xyes; then
    AC_DEFINE(INO_T_IS_64_BIT)
  fi

  if test x$enable_largefile != xno -o x$zsh_cv_off_t_is_64_bit = xyes \
  -o $zsh_cv_ino_t_is_64_bit = yes; then
    AC_CACHE_CHECK(if compiler has a 64 bit type, zsh_cv_64_bit_type,
    [zsh_64_BIT_TYPE(long long, zsh_cv_64_bit_type)
     if test "$zsh_cv_64_bit_type" = no; then
       zsh_64_BIT_TYPE(quad_t, zsh_cv_64_bit_type)
     fi
     if test "$zsh_cv_64_bit_type" = no; then
       zsh_64_BIT_TYPE(__int64_t, zsh_cv_64_bit_type)
     fi
     dnl As a last resort, if we know off_t has 64 bits, use that as
     dnl the 64-bit integer type.  I don't dare try ino_t since there's
     dnl probably nothing to stop that being unsigned.
     if test "$zsh_cv_64_bit_type" = no &&
        test "$zsh_cv_off_t_is_64_bit" = yes; then
       zsh_64_BIT_TYPE(off_t, zsh_cv_64_bit_type)
     fi])
    if test "$zsh_cv_64_bit_type" != no; then
      AC_DEFINE_UNQUOTED(ZSH_64_BIT_TYPE, $zsh_cv_64_bit_type)

      dnl Handle cases where unsigned type cannot be simply
      dnl `unsigned ZSH_64_BIT_TYPE'.  More tests may be required.
      AC_CACHE_CHECK(for a corresponding unsigned 64 bit type,
      zsh_cv_64_bit_utype,
      [zsh_64_BIT_TYPE(unsigned $zsh_cv_64_bit_type, zsh_cv_64_bit_utype,
       force)
       if test "$zsh_cv_64_bit_utype" = no; then
         zsh_64_BIT_TYPE(__uint64_t, zsh_cv_64_bit_utype)
       fi])
      if test "$zsh_cv_64_bit_utype" != no; then
        AC_DEFINE_UNQUOTED(ZSH_64_BIT_UTYPE, $zsh_cv_64_bit_utype)
      fi
    fi
  fi
fi
AH_TEMPLATE([ZLONG_IS_LONG_LONG],
[Define to 1 if the zlong type uses long long int.])
AH_TEMPLATE([ZLONG_IS_LONG_64],
[Define to 1 if the zlong type uses 64-bit long int.])
if test "$zsh_cv_64_bit_type" = "long long"; then
  dnl Remember this so we can get (s)printf output right.
  AC_DEFINE(ZLONG_IS_LONG_LONG)
else
  if test "$zsh_cv_64_bit_type" = "long"; then
      AC_DEFINE(ZLONG_IS_LONG_64)
  fi
fi

dnl We'll blithely assume (f)printf supports the same types as sprintf.
AC_CACHE_CHECK(for %lld printf support, zsh_cv_printf_has_lld,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <string.h>
int main(int argc, char **argv)
{
   long long foo = ((long long)0xdead << 40) | 0xf00d;
   char buf[80];
   sprintf(buf, "before%lldafter", foo);
   if (!strcmp(buf, "before62677660341432333after")) {
      return 0;
   }
   return 1;
}
]])],[zsh_cv_printf_has_lld=yes],[zsh_cv_printf_has_lld=no],[zsh_cv_printf_has_lld=no])])
AH_TEMPLATE(PRINTF_HAS_LLD,
[Define to 1 if printf and sprintf support %lld for long long.])
if test x$zsh_cv_printf_has_lld = xyes; then
  AC_DEFINE(PRINTF_HAS_LLD)
fi

dnl Check for sigset_t.  Currently I'm looking in
dnl <sys/types.h> and <signal.h>.  Others might need
dnl to be added.
AC_CACHE_CHECK(for sigset_t, zsh_cv_type_sigset_t,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <signal.h>]], [[sigset_t tempsigset;]])],[zsh_cv_type_sigset_t=yes],[zsh_cv_type_sigset_t=no])])
AH_TEMPLATE([sigset_t],
[Define to `unsigned int' if <sys/types.h> or <signal.h> doesn't define])
if test x$zsh_cv_type_sigset_t = xno; then
  AC_DEFINE(sigset_t, unsigned int)
fi

dnl check structures for high resolution timestamps
AC_CHECK_MEMBERS([struct stat.st_atim.tv_nsec,
                  struct stat.st_atimespec.tv_nsec,
                  struct stat.st_atimensec,
                  struct stat.st_mtim.tv_nsec,
                  struct stat.st_mtimespec.tv_nsec,
                  struct stat.st_mtimensec,
                  struct stat.st_ctim.tv_nsec,
                  struct stat.st_ctimespec.tv_nsec,
                  struct stat.st_ctimensec])

dnl Check for struct timezone since some old SCO versions do not define it
zsh_TYPE_EXISTS([
#define _GNU_SOURCE 1
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
], struct timezone)

dnl Check for struct timespec since POSIX only gained it in 2008
zsh_TYPE_EXISTS([
#define _GNU_SOURCE 1
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
], struct timespec)

dnl Check for utmp structures, for watch
zsh_TYPE_EXISTS([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UTMP_H
# include <utmp.h>
#endif
], struct utmp)
zsh_TYPE_EXISTS([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#endif
], struct utmpx)

dnl Check contents of utmp structures
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UTMP_H
# include <utmp.h>
#endif
], struct utmp, ut_host)
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#endif
], struct utmpx, ut_host)
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#endif
], struct utmpx, ut_xtime)
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#endif
], struct utmpx, ut_tv)

dnl Check for inode numbers in directory entry structures
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_DIRENT_H
# include <dirent.h>
#endif
], struct dirent, d_ino)
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_DIRENT_H
# include <dirent.h>
#endif
], struct dirent, d_stat)
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_NDIR_H
# include <sys/ndir.h>
#endif
#ifdef HAVE_SYS_DIR_H
# include <sys/dir.h>
#endif
#ifdef HAVE_NDIR_H
# include <ndir.h>
#endif
], struct direct, d_ino)
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_NDIR_H
# include <sys/ndir.h>
#endif
#ifdef HAVE_SYS_DIR_H
# include <sys/dir.h>
#endif
#ifdef HAVE_NDIR_H
# include <ndir.h>
#endif
], struct direct, d_stat)

dnl Check IPv6 socket address structure type
zsh_STRUCT_MEMBER([
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <netinet/in.h>
], struct sockaddr_in6, sin6_scope_id)

dnl Check for h_errno external variable
AH_TEMPLATE([USE_LOCAL_H_ERRNO],
[Define to 1 if h_errno is not defined by the system.])
AC_CACHE_CHECK(if we need our own h_errno,
  zsh_cv_decl_h_errno_use_local,
  [AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[extern int h_errno; h_errno = 0;]])],[zsh_cv_decl_h_errno_use_local=no],[zsh_cv_decl_h_errno_use_local=yes])])

if test x$zsh_cv_decl_h_errno_use_local = xyes; then
  AC_DEFINE(USE_LOCAL_H_ERRNO)
fi

dnl ---------------
dnl CHECK FUNCTIONS
dnl ---------------

dnl need to integrate this function
dnl AC_FUNC_STRFTIME

AC_CHECK_FUNCS(strftime strptime mktime timelocal \
	       difftime gettimeofday clock_gettime \
	       select poll \
	       readlink faccessx fchdir ftruncate \
	       fstat lstat lchown fchown fchmod \
	       fseeko ftello \
	       mkfifo _mktemp mkstemp \
	       waitpid wait3 \
	       sigaction sigblock sighold sigrelse sigsetmask sigprocmask \
	       killpg setpgid setpgrp tcsetpgrp tcgetattr nice \
	       gethostname gethostbyname2 getipnodebyname \
	       inet_aton inet_pton inet_ntop \
	       getlogin getpwent getpwnam getpwuid getgrgid getgrnam \
	       initgroups nis_list \
	       setuid seteuid setreuid setresuid setsid \
	       setgid setegid setregid setresgid \
	       memcpy memmove strstr strerror strtoul \
	       getrlimit getrusage \
	       setlocale \
	       isblank iswblank \
	       uname \
	       signgam tgamma \
	       log2 \
	       scalbn \
	       putenv getenv setenv unsetenv xw\
	       brk sbrk \
	       pathconf sysconf \
	       tgetent tigetflag tigetnum tigetstr setupterm initscr resize_term \
	       getcchar setcchar waddwstr wget_wch win_wch use_default_colors \
	       pcre_compile pcre_study pcre_exec \
	       nl_langinfo \
	       erand48 open_memstream \
	       posix_openpt \
	       wctomb iconv \
	       grantpt unlockpt ptsname \
	       htons ntohs \
	       regcomp regexec regerror regfree \
	       gdbm_open getxattr \
	       realpath canonicalize_file_name \
	       symlink getcwd \
	       cygwin_conv_path \
	       nanosleep \
	       srand_deterministic \
	       setutxent getutxent endutxent getutent)
AC_FUNC_STRCOLL

# isinf() and isnan() can exist as either functions or macros.
AH_TEMPLATE([HAVE_ISINF],
  [Define to 1 if you have the `isinf' macro or function.])
AC_MSG_CHECKING([for isinf])
AC_LINK_IFELSE([AC_LANG_SOURCE(
[[#include <math.h>
int main () { return (isinf(1.0) != 0); }]])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([HAVE_ISINF])],
  [AC_MSG_RESULT([no])])

AH_TEMPLATE([HAVE_ISNAN],
  [Define to 1 if you have the `isnan' macro or function.])
AC_MSG_CHECKING([for isnan])
AC_LINK_IFELSE([AC_LANG_SOURCE([[
#include <math.h>
int main () { return (isnan(1.0) != 0); }]])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([HAVE_ISNAN])],
  [AC_MSG_RESULT([no])])

AH_TEMPLATE([REALPATH_ACCEPTS_NULL],
[Define if realpath() accepts NULL as its second argument.])
AC_CACHE_CHECK([if realpath accepts NULL],
zsh_cv_func_realpath_accepts_null,
[AC_RUN_IFELSE([AC_LANG_PROGRAM([
#include <stdlib.h>
#include <limits.h>
],[
exit(!realpath("/", (char*)0));
])],
[zsh_cv_func_realpath_accepts_null=yes],
[zsh_cv_func_realpath_accepts_null=no],
[zsh_cv_func_realpath_accepts_null=$ac_cv_func_canonicalize_file_name])])
if test x$zsh_cv_func_realpath_accepts_null = xyes; then
  AC_DEFINE(REALPATH_ACCEPTS_NULL)
fi

if test x$enable_cap = xyes; then
  AC_CHECK_FUNCS(cap_get_proc)
fi

dnl  Check if tgetent accepts NULL (and will allocate its own termcap buffer)
dnl  Some termcaps reportedly accept a zero buffer, but then dump core
dnl  in tgetstr().
dnl  Under Cygwin test program crashes but exit code is still 0. So,
dnl  we test for a file that porgram should create
AH_TEMPLATE([TGETENT_ACCEPTS_NULL],
[Define to 1 if tgetent() accepts NULL as a buffer.])
AC_CACHE_CHECK(if tgetent accepts NULL,
zsh_cv_func_tgetent_accepts_null,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <fcntl.h>
#include <stdlib.h>
int tgetent(char *, char *);
char *tgetstr(char *, char **);
main()
{
    char buf[4096];
    int r1 = tgetent(buf, "vt100");
    int r2 = tgetent((char*)0,"vt100");
    if (r1 >= 0 && r1 == r2) {
        char tbuf[1024], *u;
        u = tbuf;
    	tgetstr("cl", &u);
	creat("conftest.tgetent", 0640);
    }
    exit((r1 != r2) || r2 == -1);
}
]])],[if test -f conftest.tgetent; then
    zsh_cv_func_tgetent_accepts_null=yes
  else
    zsh_cv_func_tgetent_accepts_null=no
  fi],[zsh_cv_func_tgetent_accepts_null=no],[zsh_cv_func_tgetent_accepts_null=no])])
if test x$zsh_cv_func_tgetent_accepts_null = xyes; then
  AC_DEFINE(TGETENT_ACCEPTS_NULL)
fi
AC_CACHE_CHECK(if tgetent returns 0 on success,
zsh_cv_func_tgetent_zero_success,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <fcntl.h>
#include <stdlib.h>
int tgetent(char *, char*);
char *tgetstr(char *, char **);
main()
{
    char buf[4096];
    int r1 = tgetent(buf, "!@#$%^&*");
    int r2 = tgetent(buf, "vt100");
    if (r1 < 0 && r2 == 0) {
        char tbuf[1024], *u;
        u = tbuf;
    	tgetstr("cl", &u);
	creat("conftest.tgetent0", 0640);
    }
    exit(r1 == r2);
}
]])],[if test -f conftest.tgetent0; then
    zsh_cv_func_tgetent_zero_success=yes
  else
    zsh_cv_func_tgetent_zero_success=no
  fi],[zsh_cv_func_tgetent_zero_success=no],[zsh_cv_func_tgetent_zero_success=no])])
AH_TEMPLATE([TGETENT_SUCCESS],
[Define to what tgetent() returns on success (0 on HP-UX X/Open curses).])
if test x$zsh_cv_func_tgetent_zero_success = xyes; then
  AC_DEFINE(TGETENT_SUCCESS, 0)
else
  AC_DEFINE(TGETENT_SUCCESS, 1)
fi

AC_FUNC_MMAP
if test x$ac_cv_func_mmap_fixed_mapped = xyes; then
  AC_CHECK_FUNCS(munmap msync)
fi

if test x$ac_cv_func_setpgrp = xyes; then
  AC_FUNC_GETPGRP
else
  dnl If there is no setpgrp, the test for getpgrp(void) will fail
  dnl because the program will not compile.  However, in that case
  dnl we can be reasonably confident we are not dealing with a
  dnl Berkeleyesque system, so assume getpgrp does take void.
  ac_cv_func_getpgrp_void=yes
  AC_DEFINE(GETPGRP_VOID)
fi

if test x$dynamic = xyes; then
  AC_CHECK_FUNCS(dlopen dlerror dlsym dlclose load loadquery loadbind unload \
		shl_load shl_unload shl_findsym)
fi

AH_TEMPLATE([XATTR_EXTRA_ARGS],
Define if getxattr() etc. require additional MacOS-style arguments)
if test x$ac_cv_func_getxattr = xyes && test x$ac_cv_header_sys_xattr_h = xyes
then
  AC_CACHE_CHECK(if getxattr etc. are Linux-like,
  zsh_cv_getxattr_linux,
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <sys/xattr.h>]],
  [[
  (void)listxattr("", 0, 0);
  (void)getxattr("", "", 0, 0);
  (void)setxattr("", "", "", 0, 0);
  (void)removexattr("", "");
  ]])],
  [zsh_cv_getxattr_linux=yes],
  [zsh_cv_getxattr_linux=no])])

  if test x$zsh_cv_getxattr_linux != xyes; then
    AC_CACHE_CHECK(if getxattr etc. are MAC-like,
    zsh_cv_getxattr_mac,
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <sys/xattr.h>]],
    [[(void)listxattr("", 0, 0, 0);
    (void)getxattr("", "", 0, 0, 0, 0);
    (void)setxattr("", "", "", 0, 0, 0);
    (void)removexattr("", "", 0);]])],
    [zsh_cv_getxattr_mac=yes],
    [zsh_cv_getxattr_mac=no])])

    if test x$zsh_cv_getxattr_mac = xyes; then
      AC_DEFINE(XATTR_EXTRA_ARGS)
    fi
  fi
fi

AC_CACHE_CHECK(if getxattr etc. are usable,
zsh_cv_use_xattr,
[if test x$zsh_cv_getxattr_linux = xyes || test x$zsh_cv_getxattr_mac = xyes
then
zsh_cv_use_xattr=yes
else
zsh_cv_use_xattr=no
fi])

dnl -------------
dnl CHECK SIGNALS
dnl -------------
dnl What style of signal do you have (POSIX, BSD, or SYSV)?
AH_TEMPLATE([POSIX_SIGNALS],
[Define to 1 if you use POSIX style signal handling.])
AH_TEMPLATE([BSD_SIGNALS],
[Define to 1 if you use BSD style signal handling (and can block signals).])
AH_TEMPLATE([SYSV_SIGNALS],
[Define to 1 if you use SYS style signal handling (and can block signals).])
AH_TEMPLATE([NO_SIGNAL_BLOCKING],
[Define to 1 if you have no signal blocking at all (bummer).])
AC_MSG_CHECKING(what style of signals to use)
if test x$ac_cv_func_sigaction = xyes && test x$ac_cv_func_sigprocmask = xyes; then
  signals_style=POSIX_SIGNALS
  AC_DEFINE(POSIX_SIGNALS)
elif test x$ac_cv_func_sigblock = xyes && test x$ac_cv_func_sigsetmask = xyes; then
  signals_style=BSD_SIGNALS
  AC_DEFINE(BSD_SIGNALS)
elif test x$ac_cv_func_sighold = xyes && test x$ac_cv_func_sigrelse = xyes; then
  signals_style=SYSV_SIGNALS
  AC_DEFINE(SYSV_SIGNALS)
else
  signals_style=NO_SIGNAL_BLOCKING
  AC_DEFINE(NO_SIGNAL_BLOCKING)
fi
AC_DEFINE_UNQUOTED($signals_style)
AC_MSG_RESULT($signals_style)

dnl Where is <signal.h> located?  Needed as input for signals.awk
AC_CACHE_CHECK(where signal.h is located, zsh_cv_path_signal_h,
[dnl Look at the output from the preprocessor.
dnl We should get lines of the form `# 1 "/usr/include/signal.h"'
dnl The following assumes the real definitions are in a file which
dnl contains the name `sig'; we could relax this if necessary,
dnl but then you can get a rather long list of files to test.
dnl The backslash substitution is to persuade cygwin to cough up
dnl slashes rather than doubled backslashes in the path.
echo "#include <signal.h>" > nametmp.c
sigfile_list="`$CPP $CPPFLAGS nametmp.c |
sed -n -e 's/^#line[ 	].*\"\(.*\)\"/\1/p' \
       -e 's/^#[ 	].*\"\(.*\)\"/\1/p' |
sed 's/\\\\\\\\/\//g' |
$AWK '{ if ($1 ~ /sig/) files[[$1]] = $1 }
  END { for (var in files) print var }'`"
rm -f nametmp.c
if test -z "$sigfile_list"; then
  dnl In case we don't get the stuff from the preprocesor, use the old
  dnl list of standard places.
  sigfile_list="/usr/include/sys/iso/signal_iso.h
/usr/include/bsd/sys/signal.h
/usr/include/signum.h
/usr/include/asm/signum.h
/usr/include/asm/signal.h
/usr/include/linux/signal.h
/usr/include/sys/signal.h
/usr/include/bits/signum.h
/dev/null"
fi
for SIGNAL_TRY_H in $sigfile_list
do
  dnl Try to make sure it doesn't get confused by files that don't
  dnl have real signal definitions in, but do #define SIG* by counting
  dnl the number of signals.  Maybe we could even check for e.g. SIGHUP?
  nsigs=`test -f $SIGNAL_TRY_H && \
  grep '#[ 	]*define[ 	][ 	]*SIG[0-9A-Z]*[ 	]*[0-9][0-9]*' $SIGNAL_TRY_H | \
  wc -l | sed 's/[ 	]//g'`
  if test "x$nsigs" != x && test "$nsigs" -ge 7
  then
    SIGNAL_H="$SIGNAL_H $SIGNAL_TRY_H"
  fi
done
if test "x$SIGNAL_H" = x; then
  AC_MSG_ERROR(SIGNAL MACROS NOT FOUND:  please report to developers)
fi
zsh_cv_path_signal_h="$SIGNAL_H"
])
SIGNAL_H="$zsh_cv_path_signal_h"
AC_SUBST(SIGNAL_H)dnl

dnl Where are error names located?  Needed as input for errnames1.awk
AC_CACHE_CHECK(where error names are located, zsh_cv_path_errno_h,
[dnl Look at the output from the preprocessor.
dnl We should get lines of the form `# 1 "/usr/include/errno.h"'
dnl The following assumes the real definitions are in a file which
dnl contains the name `err'; we could relax this if necessary,
dnl but then you can get a rather long list of files to test.
dnl The backslash substitution is to persuade cygwin to cough up
dnl slashes rather than doubled backslashes in the path.
echo "#include <errno.h>" > nametmp.c
errfile_list="`$CPP $CPPFLAGS nametmp.c |
sed -n -e 's/^#line[ 	].*\"\(.*\)\"/\1/p' \
       -e 's/^#[ 	0-9].*\"\(.*\)\"/\1/p' |
sed 's/\\\\\\\\/\//g' |
$AWK '{ if ($1 ~ /err/) files[[$1]] = $1 }
  END { for (var in files) print var }'`"
rm -f nametmp.c
for ERRNO_TRY_H in $errfile_list /dev/null
do
  dnl Try to make sure it doesn't get confused by files that don't
  dnl have real error definitions in.  Count definitions to make sure.
  dnl Definitions of error numbers have become more and more general, so
  dnl make a list of files containing any definitions in and keep them all.
  dnl Careful with cut and paste in the pattern: the square brackets
  dnl must contain a space and a tab.
  nerrs=`test -f $ERRNO_TRY_H && \
  $EGREP '#[ 	]*define[ 	][ 	]*E[0-9A-Z]*[ 	]*(_HURD_ERRNO )?\(?[_A-Z0-9]' $ERRNO_TRY_H | \
  wc -l | sed 's/[ 	]//g'`
  if test "x$nerrs" != x && test "$nerrs" -ge 1
  then
    ERRNO_H="$ERRNO_H $ERRNO_TRY_H"
  fi
done
if test x"$ERRNO_H" = x; then
  AC_MSG_ERROR(ERROR MACROS NOT FOUND:  please report to developers)
fi
zsh_cv_path_errno_h="$ERRNO_H"
])
ERRNO_H="$zsh_cv_path_errno_h"
AC_SUBST(ERRNO_H)dnl

AC_CACHE_CHECK(location of curses header, zsh_cv_path_curses_header,
[if test x$zsh_cv_ignore_ncurses = xyes; then
  if test x$ac_cv_header_curses_h = xyes; then
    zsh_cv_path_curses_header=curses.h
  else
    zsh_cv_path_curses_header=none
  fi
elif test x$ac_cv_header_ncursesw_ncurses_h = xyes; then
  zsh_cv_path_curses_header=ncursesw/ncurses.h
elif test x$ac_cv_header_ncurses_ncurses_h = xyes; then
  zsh_cv_path_curses_header=ncurses/ncurses.h
elif test x$ac_cv_header_ncurses_h = xyes; then
  zsh_cv_path_curses_header=ncurses.h
elif test x$ac_cv_header_curses_h = xyes; then
  zsh_cv_path_curses_header=curses.h
else
  zsh_cv_path_curses_header=none
fi])
AH_TEMPLATE([ZSH_HAVE_CURSES_H],
[Define to 1 if some variant of a curses header can be included])
if test x$zsh_cv_path_curses_header != xnone; then
  AC_DEFINE(ZSH_HAVE_CURSES_H)
  ZSH_CURSES_H=$zsh_cv_path_curses_header
else
  ZSH_CURSES_H=
fi
AC_SUBST(ZSH_CURSES_H)

dnl Where are curses key definitions located?  Need for keypad() mode.
AC_CACHE_CHECK(where curses key definitions are located, zsh_cv_path_curses_keys_h,
[dnl This is an identical trick to errno.h, except we use ncurses.h
dnl if we can.
if test x$zsh_cv_path_curses_header = xnone; then
  echo >nametmp.c
else
  echo "#include <$zsh_cv_path_curses_header>" >nametmp.c
fi

curses_list="`$CPP $CPPFLAGS nametmp.c |
sed -n -e 's/^#line[ 	].*\"\(.*\)\"/\1/p' \
       -e 's/^#[ 	0-9].*\"\(.*\)\"/\1/p' |
sed 's/\\\\\\\\/\//g' |
$AWK '{ if ($1 ~ /\.h/) files[[$1]] = $1 }
  END { for (var in files) print var }'`"
rm -f nametmp.c
for CURSES_TRY_H in $curses_list /dev/null
do
  nkeys=`test -f $CURSES_TRY_H && \
  $EGREP '#[ 	]*define[ 	][ 	]*KEY_' $CURSES_TRY_H | \
  wc -l | sed 's/[ 	]//g'`
  if test "x$nkeys" != x && test "$nkeys" -ge 10
  then
    CURSES_KEYS_H=$CURSES_TRY_H
    break
  fi
done
zsh_cv_path_curses_keys_h="$CURSES_KEYS_H"
])
CURSES_KEYS_H="$zsh_cv_path_curses_keys_h"
AC_SUBST(CURSES_KEYS_H)dnl

dnl See if there are variants of term.h.  For testing each one
dnl we include the most likely variant of the curses header.
AC_CHECK_HEADERS(ncursesw/term.h,
true, true,
[#include <ncursesw/ncurses.h>])
AC_CHECK_HEADERS(ncurses/term.h,
true, true,
[#include <ncurses/ncurses.h>])
AC_CHECK_HEADERS(term.h,
true, true,
[#include <curses.h>])

dnl See if term.h is bundled along with the curses library we
dnl are using.  If this isn't the default system curses, compilation
dnl could barf unless we include from the right subdirectory.
AC_CACHE_CHECK(where term.h is located, zsh_cv_path_term_header,
[case x$zsh_cv_path_curses_header in
  xncursesw/*)
  if test x$ac_cv_header_ncursesw_term_h = xyes; then
    zsh_cv_path_term_header=ncursesw/term.h
  fi
  ;;
  xncurses/*)
  if test x$ac_cv_header_ncurses_term_h = xyes; then
    zsh_cv_path_term_header=ncurses/term.h
  fi
  ;;
esac
if test x$zsh_cv_path_term_header = x; then
  if test x$ac_cv_header_term_h = xyes; then
    zsh_cv_path_term_header=term.h
  else
    zsh_cv_path_term_header=none
  fi
fi])

AH_TEMPLATE([ZSH_HAVE_TERM_H],
[Define to 1 if some variant of term.h can be included])
AH_TEMPLATE([HAVE_BOOLCODES],
[Define if you have the termcap boolcodes symbol.])
AH_TEMPLATE([HAVE_NUMCODES],
[Define if you have the termcap numcodes symbol.])
AH_TEMPLATE([HAVE_STRCODES],
[Define if you have the termcap strcodes symbol.])
AH_TEMPLATE([HAVE_BOOLNAMES],
[Define if you have the terminfo boolnames symbol.])
AH_TEMPLATE([HAVE_NUMNAMES],
[Define if you have the terminfo numnames symbol.])
AH_TEMPLATE([HAVE_STRNAMES],
[Define if you have the terminfo strnames symbol.])
AH_TEMPLATE([TGOTO_PROTO_MISSING],
[Define if there is no prototype for the tgoto() terminal function.])

if test x$zsh_cv_path_term_header != xnone; then
  AC_DEFINE(ZSH_HAVE_TERM_H)
  ZSH_TERM_H=$zsh_cv_path_term_header
  if test x$zsh_cv_path_curses_header != xnone; then
    term_includes="#include <$zsh_cv_path_curses_header>
#include <$zsh_cv_path_term_header>"
  else
    term_includes="#include <$zsh_cv_path_term_header>"
  fi

  AC_MSG_CHECKING(if boolcodes is available)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[$term_includes]], [[char **test = boolcodes; puts(*test);]])],[AC_DEFINE(HAVE_BOOLCODES) boolcodes=yes],[boolcodes=no])
  AC_MSG_RESULT($boolcodes)

  AC_MSG_CHECKING(if numcodes is available)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[$term_includes]], [[char **test = numcodes; puts(*test);]])],[AC_DEFINE(HAVE_NUMCODES) numcodes=yes],[numcodes=no])
  AC_MSG_RESULT($numcodes)

  AC_MSG_CHECKING(if strcodes is available)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[$term_includes]], [[char **test = strcodes; puts(*test);]])],[AC_DEFINE(HAVE_STRCODES) strcodes=yes],[strcodes=no])
  AC_MSG_RESULT($strcodes)

  AC_MSG_CHECKING(if boolnames is available)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[$term_includes]], [[char **test = boolnames; puts(*test);]])],[AC_DEFINE(HAVE_BOOLNAMES) boolnames=yes],[boolnames=no])
  AC_MSG_RESULT($boolnames)

  AC_MSG_CHECKING(if numnames is available)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[$term_includes]], [[char **test = numnames; puts(*test);]])],[AC_DEFINE(HAVE_NUMNAMES) numnames=yes],[numnames=no])
  AC_MSG_RESULT($numnames)

  AC_MSG_CHECKING(if strnames is available)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[$term_includes]], [[char **test = strnames; puts(*test);]])],[AC_DEFINE(HAVE_STRNAMES) strnames=yes],[strnames=no])
  AC_MSG_RESULT($strnames)

  dnl There are apparently defective terminal library headers on some
  dnl versions of Solaris before 11.
  AC_MSG_CHECKING(if tgoto prototype is missing)
  tgoto_includes="$term_includes
/* guaranteed to clash with any valid tgoto prototype */
extern void tgoto(int **stuff, float **more_stuff);"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[$tgoto_includes]], [[int *stuff; float *more_stuff; tgoto(&stuff, &more_stuff);]])],[AC_DEFINE(TGOTO_PROTO_MISSING) tgotoprotomissing=yes],[tgotoprotomissing=no])
  AC_MSG_RESULT($tgotoprotomissing)
else
  ZSH_TERM_H=
fi
AC_SUBST(ZSH_TERM_H)


dnl -----------------------------------------------------
dnl Look for the file containing the RLIMIT_* definitions
dnl -----------------------------------------------------
dnl CALL FOR MORE (FEWER?) LOCATIONS:  I've just copied the signal checking.
AC_CACHE_CHECK(where the RLIMIT macros are located,zsh_cv_path_rlimit_h,
[dnl Look at the output from the preprocessor.
dnl Copied from the search for the signal names above.
echo "#include <sys/resource.h>" >restmp.c
resourcefile_list="`$CPP $CPPFLAGS restmp.c |
sed -n -e 's/^#line[ 	].*\"\(.*\)\"/\1/p' \
       -e 's/^#[ 	].*\"\(.*\)\"/\1/p' |
sed 's/\\\\\\\\/\//g' |
$AWK '{ if ($1 ~ /resource/) files[[$1]] = $1 }
  END { for (var in files) print var }'`"
rm -f restmp.c
if test -z "$resourcefile_list"; then
  dnl No list: look at standard places.
  resourcefile_list="/usr/include/bsd/sys/resource.h
/usr/include/asm/resource.h
/usr/include/linux/resource.h
/usr/include/sys/resource.h
/usr/include/bits/resource.h
/usr/include/resourcebits.h"
fi
for RESOURCE_H in $resourcefile_list /dev/null;
do
  test -f $RESOURCE_H && \
  grep '#[ 	]*define[ 	][ 	]*RLIMIT_[A-Z]*[ 	]*[0-9A-Z][0-9]*' $RESOURCE_H > /dev/null && \
  break
done
zsh_cv_path_rlimit_h=$RESOURCE_H
if test x$RESOURCE_H = x"/dev/null" && test x$ac_cv_func_getrlimit = xyes; then
  AC_MSG_WARN(RLIMIT MACROS NOT FOUND:  please report to developers)
fi])
RLIMITS_INC_H=$zsh_cv_path_rlimit_h
if test "$RLIMITS_INC_H" = "/dev/null"; then
  RLIMITS_INC_H=''
fi
dnl rlimits.h only appears in dependencies if we are actually using it.
dnl We are using it any time we have getrlimit, though if the macros were
dnl not found we simply awk through /dev/null and fail to find them.
dnl Thus, limit won't work, but at least the shell will compile.
AC_SUBST(RLIMITS_INC_H)dnl

dnl ------------------
dnl rlimit type checks
dnl ------------------
AH_TEMPLATE([RLIM_T_IS_QUAD_T],
[Define to 1 if struct rlimit uses quad_t.])
AH_TEMPLATE([RLIM_T_IS_LONG_LONG],
[Define to 1 if struct rlimit uses long long])
AH_TEMPLATE([RLIM_T_IS_UNSIGNED],
[Define to 1 if struct rlimit uses unsigned.])
AH_TEMPLATE([rlim_t],
[Define to the type used in struct rlimit.])
DEFAULT_RLIM_T=long
AC_CACHE_CHECK(if rlim_t is longer than a long,
zsh_cv_rlim_t_is_longer,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>
#include <stdlib.h>
main(){struct rlimit r;exit(sizeof(r.rlim_cur) <= sizeof(long));}]])],[zsh_cv_rlim_t_is_longer=yes],[zsh_cv_rlim_t_is_longer=no],[zsh_cv_rlim_t_is_longer=yes])])
if test x$zsh_cv_rlim_t_is_longer = xyes; then
  AC_CACHE_CHECK(if rlim_t is a quad,
  zsh_cv_rlim_t_is_quad_t,
  [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <stdio.h>
#include <sys/resource.h>
#include <stdlib.h>
main() { 
  struct rlimit r;
  char buf[20];
  r.rlim_cur = 0;
  sprintf(buf, "%qd", r.rlim_cur);
  exit(strcmp(buf, "0"));
}]])],[zsh_cv_rlim_t_is_quad_t=yes],[zsh_cv_rlim_t_is_quad_t=no],[zsh_cv_rlim_t_is_quad_t=no])])
  if test x$zsh_cv_rlim_t_is_quad_t = xyes; then
    AC_DEFINE(RLIM_T_IS_QUAD_T)
    DEFAULT_RLIM_T=quad_t
  else
    AC_DEFINE(RLIM_T_IS_LONG_LONG)
    DEFAULT_RLIM_T='long long'
  fi
else
  AC_CACHE_CHECK(if the rlim_t is unsigned,
  zsh_cv_type_rlim_t_is_unsigned,
  [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>
#include <stdlib.h>
  main(){struct rlimit r;r.rlim_cur=-1;exit(r.rlim_cur<0);}]])],[zsh_cv_type_rlim_t_is_unsigned=yes],[zsh_cv_type_rlim_t_is_unsigned=no],[zsh_cv_type_rlim_t_is_unsigned=no])])
  if test x$zsh_cv_type_rlim_t_is_unsigned = xyes; then
    AC_DEFINE(RLIM_T_IS_UNSIGNED)
    DEFAULT_RLIM_T="unsigned $DEFAULT_RLIM_T"
  fi
fi

AC_CACHE_CHECK(for rlim_t, zsh_cv_type_rlim_t,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>]], [[rlim_t l;]])],[zsh_cv_type_rlim_t=yes],[zsh_cv_type_rlim_t=no])])
if test x$zsh_cv_type_rlim_t = xno; then
  AC_DEFINE_UNQUOTED(rlim_t, $DEFAULT_RLIM_T)
fi


dnl On some systems the RLIMIT_* don't evaluate to integers at compile time
dnl (they may be enums).  In this case we are not able to do preprocessor
dnl comparisons and need our tests to determine what values exist and
dnl if there are clashing definitions.

zsh_LIMIT_PRESENT(RLIMIT_AIO_MEM)
zsh_LIMIT_PRESENT(RLIMIT_AIO_OPS)
zsh_LIMIT_PRESENT(RLIMIT_AS)
zsh_LIMIT_PRESENT(RLIMIT_LOCKS)
zsh_LIMIT_PRESENT(RLIMIT_MEMLOCK)
zsh_LIMIT_PRESENT(RLIMIT_NPROC)
zsh_LIMIT_PRESENT(RLIMIT_NTHR)
zsh_LIMIT_PRESENT(RLIMIT_NOFILE)
zsh_LIMIT_PRESENT(RLIMIT_PTHREAD)
zsh_LIMIT_PRESENT(RLIMIT_RSS)
zsh_LIMIT_PRESENT(RLIMIT_SBSIZE)
zsh_LIMIT_PRESENT(RLIMIT_TCACHE)
zsh_LIMIT_PRESENT(RLIMIT_VMEM)
zsh_LIMIT_PRESENT(RLIMIT_SIGPENDING)
zsh_LIMIT_PRESENT(RLIMIT_MSGQUEUE)
zsh_LIMIT_PRESENT(RLIMIT_NICE)
zsh_LIMIT_PRESENT(RLIMIT_RTPRIO)
zsh_LIMIT_PRESENT(RLIMIT_RTTIME)
zsh_LIMIT_PRESENT(RLIMIT_POSIXLOCKS)
zsh_LIMIT_PRESENT(RLIMIT_NPTS)
zsh_LIMIT_PRESENT(RLIMIT_SWAP)
zsh_LIMIT_PRESENT(RLIMIT_KQUEUES)
zsh_LIMIT_PRESENT(RLIMIT_UMTXP)

zsh_LIMITS_EQUAL(VMEM, vmem, RSS, rss)
zsh_LIMITS_EQUAL(VMEM, vmem, AS, as)
zsh_LIMITS_EQUAL(RSS, rss, AS, as)

dnl --------------------------------------------
dnl Check for members of struct rusage
dnl --------------------------------------------
if test x$ac_cv_func_getrusage = xyes; then
  AC_CHECK_MEMBERS([struct rusage.ru_maxrss,
		    struct rusage.ru_ixrss,
		    struct rusage.ru_idrss,
		    struct rusage.ru_isrss,
		    struct rusage.ru_minflt,
		    struct rusage.ru_majflt,
		    struct rusage.ru_nswap,
		    struct rusage.ru_inblock,
		    struct rusage.ru_oublock,
		    struct rusage.ru_msgsnd,
		    struct rusage.ru_msgrcv,
		    struct rusage.ru_nsignals,
		    struct rusage.ru_nvcsw,
		    struct rusage.ru_nivcsw],,,
[#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>])
fi


dnl --------------------------------------------
dnl CHECK FOR DEFAULT PATH (used for command -p)
dnl --------------------------------------------
AC_CACHE_VAL(zsh_cv_cs_path,
[if getconf _CS_PATH >/dev/null 2>&1; then
  zsh_cv_cs_path=`getconf _CS_PATH`
elif getconf CS_PATH >/dev/null 2>&1; then
  zsh_cv_cs_path=`getconf CS_PATH`
elif getconf PATH >/dev/null 2>&1; then
  zsh_cv_cs_path=`getconf PATH`
else
  zsh_cv_cs_path="/bin:/usr/bin"
fi])
AC_DEFINE_UNQUOTED(DEFAULT_PATH, "$zsh_cv_cs_path",
[The default path; used when running commands with command -p])


dnl ----------------------------
dnl CHECK FOR /dev/fd FILESYSTEM
dnl ----------------------------
dnl FreeBSD 5 only supports /dev/fd/0 to /dev/fd/2 without mounting
dnl a special file system.  As zsh needs arbitrary /dev/fd (typically
dnl >10) for its own use, we need to make sure higher fd's are available.
dnl Since we're using the shell, we're restricted to 0 to 9 but 3 should
dnl be good enough.
AH_TEMPLATE([PATH_DEV_FD],
[Define to the path of the /dev/fd filesystem.])
AC_CACHE_CHECK(for /dev/fd filesystem, zsh_cv_sys_path_dev_fd,
[for zsh_cv_sys_path_dev_fd in /proc/self/fd /dev/fd no; do
   test x`echo ok|(exec 3<&0; cat $zsh_cv_sys_path_dev_fd/3 2>/dev/null;)` = xok && break
 done])
if test x$zsh_cv_sys_path_dev_fd != xno; then
  AC_DEFINE_UNQUOTED(PATH_DEV_FD, "$zsh_cv_sys_path_dev_fd")
fi

dnl ---------------------------------
dnl CHECK FOR RFS SUPERROOT DIRECTORY
dnl ---------------------------------
AC_CACHE_CHECK(for RFS superroot directory, zsh_cv_sys_superroot,
[test -d /../.LOCALROOT && zsh_cv_sys_superroot=yes || zsh_cv_sys_superroot=no])
AH_TEMPLATE([HAVE_SUPERROOT],
[Define to 1 if you have RFS superroot directory.])
if test x$zsh_cv_sys_superroot = xyes; then
  AC_DEFINE(HAVE_SUPERROOT)
fi

dnl CHECK FOR SYSTEMS REQUIRING GETCWD
AC_CACHE_CHECK(whether we should use the native getcwd,
zsh_cv_use_getcwd,
[case "${host_cpu}-${host_vendor}-${host_os}" in
    *QNX*) zsh_cv_use_getcwd=yes ;;
    *) zsh_cv_use_getcwd=no ;;
 esac])
AH_TEMPLATE([USE_GETCWD],
[Define to 1 if you need to use the native getcwd.])
if test x$zsh_cv_use_getcwd = xyes; then
  AC_DEFINE(USE_GETCWD)
fi

dnl GNU getcwd() can allocate as much space as necessary for a
dnl directory name, preventing guessing games.
AH_TEMPLATE([GETCWD_CALLS_MALLOC],
[Define to 1 if getcwd() calls malloc to allocate memory.])
if test x$ac_cv_func_getcwd = xyes; then
  AC_CACHE_CHECK(whether getcwd calls malloc to allocate memory,
  zsh_cv_getcwd_malloc,
  [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <unistd.h>
#include <string.h>
int main() {
    char buf[1024], *ptr1, *ptr2;
    ptr1 = getcwd(buf, 1024);
    ptr2 = getcwd(NULL, 0);
    if (ptr1 && ptr2 && !strcmp(ptr1, ptr2)) {
      return 0;
    }
    return 1;
}
]])],[zsh_cv_getcwd_malloc=yes],[zsh_cv_getcwd_malloc=no],[zsh_cv_getcwd_malloc=no])])
  if test x$zsh_cv_getcwd_malloc = xyes; then
    AC_DEFINE(GETCWD_CALLS_MALLOC)
  fi
fi

dnl CHECK FOR setproctitle() FOR jobs -Z / ARGV0
AH_TEMPLATE([HAVE_SETPROCTITLE],
[Define to 1 if the system supports `setproctitle' to change process name])
AC_CHECK_FUNC(setproctitle,AC_DEFINE(HAVE_SETPROCTITLE),
AC_SEARCH_LIBS(setproctitle,util,AC_DEFINE(HAVE_SETPROCTITLE)))

dnl CHECK FOR prctl() FOR jobs -Z / ARGV0 when checking with ps -e
AH_TEMPLATE([HAVE_PRCTL],
[Define to 1 if the system supports `prctl' to change process name])
AC_CHECK_FUNC(prctl,AC_DEFINE(HAVE_PRCTL),
AC_SEARCH_LIBS(prctl,c,AC_DEFINE(HAVE_PRCTL)))

dnl ----------------------------------------
dnl CHECK FOR LOCATION OF {U,W}TMP{,X} FILES
dnl ----------------------------------------
zsh_PATH_UTMP(utmp)
zsh_PATH_UTMP(wtmp)
zsh_PATH_UTMP(utmpx,utx.active)
zsh_PATH_UTMP(wtmpx)

dnl -------------------
dnl brk/sbrk PROTOTYPES
dnl -------------------
AC_CACHE_CHECK(for brk() prototype in <unistd.h>,
zsh_cv_header_unistd_h_brk_proto,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <unistd.h>
double brk();]], [[int i;]])],[zsh_cv_header_unistd_h_brk_proto=no],[zsh_cv_header_unistd_h_brk_proto=yes])])
AH_TEMPLATE([HAVE_BRK_PROTO],
[Define to 1 if there is a prototype defined for brk() on your system.])
if test x$zsh_cv_header_unistd_h_brk_proto = xyes; then
  AC_DEFINE(HAVE_BRK_PROTO)
fi

AC_CACHE_CHECK(for sbrk() prototype in <unistd.h>,
zsh_cv_header_unistd_h_sbrk_proto,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <unistd.h>
double sbrk();]], [[int i;]])],[zsh_cv_header_unistd_h_sbrk_proto=no],[zsh_cv_header_unistd_h_sbrk_proto=yes])])
AH_TEMPLATE([HAVE_SBRK_PROTO],
[Define to 1 if there is a prototype defined for sbrk() on your system.])
if test x$zsh_cv_header_unistd_h_sbrk_proto = xyes; then
  AC_DEFINE(HAVE_SBRK_PROTO)
fi

dnl -----------------------
dnl mknod prototype for OSF
dnl -----------------------
AH_TEMPLATE([HAVE_MKNOD_PROTO],
[Define to 1 if there is a prototype defined for mknod() on your system.])
if test "$ac_cv_prog_cc_stdc" != no; then
  AC_CACHE_CHECK(for mknod prototype in <sys/stat.h>,
  zsh_cv_header_sys_stat_h_mknod_proto,
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/stat.h>
   int mknod(double x);]], [[int i;]])],[zsh_cv_header_sys_stat_h_mknod_proto=no],[zsh_cv_header_sys_stat_h_mknod_proto=yes])])
  if test x$zsh_cv_header_sys_stat_h_mknod_proto = xyes; then
    AC_DEFINE(HAVE_MKNOD_PROTO)
  fi
fi

dnl ----------------------------------------
dnl presence and location of ioctl prototype
dnl ----------------------------------------
AC_CACHE_CHECK(for ioctl prototype in <unistd.h> or <termios.h>,
zsh_cv_header_unistd_h_termios_h_ioctl_proto,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
double ioctl();]], [[int i;]])],[zsh_cv_header_unistd_h_termios_h_ioctl_proto=no],[zsh_cv_header_unistd_h_termios_h_ioctl_proto=yes])])

if test x$zsh_cv_header_unistd_h_termios_h_ioctl_proto = xno; then
  AC_CACHE_CHECK(for ioctl prototype in <sys/ioctl.h>,
  zsh_cv_header_sys_ioctl_h_ioctl_proto,
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/ioctl.h>
  double ioctl();]], [[int i;]])],[zsh_cv_header_sys_ioctl_h_ioctl_proto=no],[zsh_cv_header_sys_ioctl_h_ioctl_proto=yes])])
else
  zsh_cv_header_sys_ioctl_h_ioctl_proto=no
fi

AH_TEMPLATE([HAVE_IOCTL_PROTO],
[Define to 1 if there is a prototype defined for ioctl() on your system.])
if test x$zsh_cv_header_unistd_h_termios_h_ioctl_proto = xyes || \
   test x$zsh_cv_header_sys_ioctl_h_ioctl_proto = xyes; then
  AC_DEFINE(HAVE_IOCTL_PROTO)
fi
AH_TEMPLATE([IOCTL_IN_SYS_IOCTL],
[Define to 1 if we must include <sys/ioctl.h> to get a prototype for ioctl().])
if test x$zsh_cv_header_sys_ioctl_h_ioctl_proto = xyes; then
  AC_DEFINE(IOCTL_IN_SYS_IOCTL)
fi

dnl -------------------
dnl select() defined in <sys/socket.h>, ie BeOS R4.51
dnl -------------------
AH_TEMPLATE([SELECT_IN_SYS_SOCKET_H],
[Define to 1 if select() is defined in <sys/socket.h>, ie BeOS R4.51])
if test x$ac_cv_header_sys_select_h != xyes; then
  AC_CACHE_CHECK(for select() in <sys/socket.h>,
  zsh_cv_header_socket_h_select_proto,
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/socket.h>]], [[fd_set fd;]])],[zsh_cv_header_socket_h_select_proto=yes],[zsh_cv_header_socket_h_select_proto=no])])
  if test x$zsh_cv_header_socket_h_select_proto = xyes; then
    AC_DEFINE(SELECT_IN_SYS_SOCKET_H)
  fi
fi

dnl -----------
dnl named FIFOs
dnl -----------
dnl
AC_CACHE_CHECK(if named FIFOs work,
zsh_cv_sys_fifo,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
main()
{
    char c;
    int fd;
    int pid, ret;
    unlink("/tmp/fifo$$");
#ifdef HAVE_MKFIFO
    if(mkfifo("/tmp/fifo$$", 0600) < 0)
#else
    if(mknod("/tmp/fifo$$", 0010600, 0) < 0)
#endif
	exit(1);
    pid = fork();
    if(pid < 0)
	exit(1);
    if(pid) {
	fd = open("/tmp/fifo$$", O_RDONLY);
	exit(fd < 0 || read(fd, &c, 1) != 1 || c != 'x');
    }
    fd = open("/tmp/fifo$$", O_WRONLY);
    ret = (fd < 0 || write(fd, "x", 1) < 1);
    unlink("/tmp/fifo$$");
    exit(ret);
}
]])],[zsh_cv_sys_fifo=yes],[zsh_cv_sys_fifo=no],[zsh_cv_sys_fifo=yes])
])
AH_TEMPLATE([HAVE_FIFOS],
[Define to 1 if system has working FIFOs.])
if test x$zsh_cv_sys_fifo = xyes; then
  AC_DEFINE(HAVE_FIFOS)
fi

dnl -----------
dnl check that lseek() correctly reports seekability.
dnl -----------
AC_CACHE_CHECK(if lseek() correctly reports seekability,
zsh_cv_sys_lseek,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
int main() {
    int pipefd[2], fd;
    off_t ret;
    char* tmpfile = "seekfiletest.tmp";
    if ((fd = open(tmpfile, O_CREAT, S_IRUSR)) < 0) {
	fprintf(stderr, "creating file failed\n");
	return 1;
    }
    ret = lseek(fd, 0, SEEK_CUR);
    close(fd);
    unlink(tmpfile);
    if (ret == (off_t)-1) {
	fprintf(stderr, "lseek on regular file failed\n");
	return 1;
    }
    if (pipe(pipefd) < 0) {
	fprintf(stderr, "creating pipe failed\n");
	return 1;
    }
    write(pipefd[1], "abcdefgh", 8);
    ret = lseek(pipefd[0], 0, SEEK_CUR);
    close(pipefd[0]);
    close(pipefd[1]);
    if (ret != (off_t)-1) {
	fprintf(stderr, "lseek on pipe succeeded\n");
	return 1;
    }
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	fprintf(stderr, "creating UNIX domain socket failed\n");
	return 1;
    }
    ret = lseek(fd, 0, SEEK_CUR);
    close(fd);
    if (ret != (off_t)-1) {
	fprintf(stderr, "lseek on UNIX domain socket succeeded\n");
	return 1;
    }
    return 0;
}
]])],[zsh_cv_sys_lseek=yes],[zsh_cv_sys_lseek=no],[zsh_cv_sys_lseek=yes])
])
AH_TEMPLATE([USE_LSEEK],
[Define to 1 if lseek() can be used for SHIN.])
if test x$zsh_cv_sys_lseek = xyes; then
  AC_DEFINE(USE_LSEEK)
fi

dnl -----------
dnl test for whether link() works
dnl for instance, BeOS R4.51 doesn't support hard links yet
dnl -----------
AC_CACHE_CHECK(if link() works,
zsh_cv_sys_link,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
main()
{
    int ret;
    char *tmpfile, *newfile;
    tmpfile="/tmp/zsh.linktest$$";
    newfile="/tmp/zsh.linktest2$$";
    unlink(tmpfile);
    unlink(newfile);
    if(creat(tmpfile, 0644) < 0)
	exit(1);
    ret = link(tmpfile, newfile);
    unlink(tmpfile);
    unlink(newfile);
    exit(ret<0);
}
]])],[zsh_cv_sys_link=yes],[zsh_cv_sys_link=no],[zsh_cv_sys_link=yes])])
AH_TEMPLATE([HAVE_LINK],
[Define to 1 if system has working link().])
if test x$zsh_cv_sys_link = xyes; then
  AC_DEFINE(HAVE_LINK)
fi

dnl -----------
dnl test for whether kill(pid, 0) where pid doesn't exit
dnl should set errno to ESRCH, but some like BeOS R4.51 set to EINVAL
dnl -----------
AC_CACHE_CHECK(if kill(pid, 0) returns ESRCH correctly,
zsh_cv_sys_killesrch,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
main()
{
    int pid = (getpid() + 10000) & 0xffffff;
    while (pid && (kill(pid, 0) == 0 || errno != ESRCH)) pid >>= 1;
    exit(errno!=ESRCH);
}
]])],[zsh_cv_sys_killesrch=yes],[zsh_cv_sys_killesrch=no],[zsh_cv_sys_killesrch=yes])])
AH_TEMPLATE([BROKEN_KILL_ESRCH],
[Define to 1 if kill(pid, 0) doesn't return ESRCH, ie BeOS R4.51.])
if test x$zsh_cv_sys_killesrch = xno; then
  AC_DEFINE(BROKEN_KILL_ESRCH)
fi

dnl -----------
dnl if POSIX, test for working sigsuspend().
dnl for instance, BeOS R4.51 is broken.
dnl -----------
AH_TEMPLATE([BROKEN_POSIX_SIGSUSPEND],
Define to 1 if sigsuspend() is broken, ie BeOS R4.51.])
if test x$signals_style = xPOSIX_SIGNALS; then
    AC_CACHE_CHECK(if POSIX sigsuspend() works,
    zsh_cv_sys_sigsuspend,
    [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
int child=0;
void handler(sig)
    int sig;
{if(sig==SIGCHLD) child=1;}
main() {
    struct sigaction act;
    sigset_t set;
    int pid, ret;
    act.sa_handler = &handler;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, 0);
    pid=fork();
    if(pid==0) return 0;
    if(pid>0) {
    sigemptyset(&set);
        ret=sigsuspend(&set);
        exit(child==0);
    }
}
]])],[zsh_cv_sys_sigsuspend=yes],[zsh_cv_sys_sigsuspend=no],[zsh_cv_sys_sigsuspend=yes])])
    if test x$zsh_cv_sys_sigsuspend = xno; then
      AC_DEFINE(BROKEN_POSIX_SIGSUSPEND)
    fi
fi

dnl -----------
dnl if found tcsetpgrp, test to see if it actually works
dnl for instance, BeOS R4.51 does not support it yet
dnl -----------
AH_TEMPLATE([BROKEN_TCSETPGRP],
[Define to 1 if tcsetpgrp() doesn't work, ie BeOS R4.51.])
AC_ARG_WITH(tcsetpgrp,
AS_HELP_STRING([--with-tcsetpgrp],[assumes that tcsetpgrp() exists and works correctly]),[
case "x$withval" in
    xyes) zsh_working_tcsetpgrp=yes;;
    xno)  zsh_working_tcsetpgrp=no;;
    *)    AC_MSG_ERROR(please use --with-tcsetpgrp=yes or --with-tcsetpgrp=no);;
esac],[zsh_working_tcsetpgrp=check])
if test "x$ac_cv_func_tcsetpgrp" = xyes; then
case "x$zsh_working_tcsetpgrp" in
  xcheck)
    trap "" TTOU > /dev/null 2>&1 || :
    AC_CACHE_CHECK(if tcsetpgrp() actually works,
    zsh_cv_sys_tcsetpgrp,
    [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
main() {
    int fd;
    int ret;
    fd=open("/dev/tty", O_RDWR);
    if (fd < 0) exit(2);
    ret=tcsetpgrp(fd, tcgetpgrp(fd));
    if (ret < 0) exit(1);
    exit(0);
}
]])],[zsh_cv_sys_tcsetpgrp=yes],[
case $? in
    1) zsh_cv_sys_tcsetpgrp=no;;
    2) zsh_cv_sys_tcsetpgrp=notty;;
    *) zsh_cv_sys_tcsetpgrp=error;;
esac
      ],[zsh_cv_sys_tcsetpgrp=yes])])
    case "x$zsh_cv_sys_tcsetpgrp" in
      xno)    AC_DEFINE(BROKEN_TCSETPGRP);;
      xyes)   :;;
      xnotty) AC_MSG_ERROR([no controlling tty
Try running configure with --with-tcsetpgrp or --without-tcsetpgrp]);;
      *)      AC_MSG_ERROR([unexpected return status]);;
    esac
    trap - TTOU > /dev/null 2>&1 || :
    ;;
  xyes) :;;
  xno)  AC_DEFINE(BROKEN_TCSETPGRP);;
  *)    AC_MSG_ERROR([unexpected value zsh_working_tcsetpgrp=$zsh_working_tcsetpgrp]);;
esac
fi

dnl -----------
dnl test for faked getpwnam() entry, ie a single entry returned for any username
dnl for instance, BeOS R4.51 is not multiuser yet, and fakes getpwnam()
dnl test by looking up two usernames that shouldn't succeed, and compare entry
dnl -----------
AH_TEMPLATE([GETPWNAM_FAKED],
[Define to 1 if getpwnam() is faked, ie BeOS R4.51.])
if test x$ac_cv_func_getpwnam = xyes; then
    AC_CACHE_CHECK(if getpwnam() is faked,
    zsh_cv_sys_getpwnam_faked,
    [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
main() {
    struct passwd *pw1, *pw2;
    char buf[1024], name[1024];
    sprintf(buf, "%d:%d", getpid(), rand());
    pw1=getpwnam(buf);
    if (pw1) strcpy(name, pw1->pw_name);
    sprintf(buf, "%d:%d", rand(), getpid());
    pw2=getpwnam(buf);
    exit(pw1!=0 && pw2!=0 && !strcmp(name, pw2->pw_name));
}
]])],[zsh_cv_sys_getpwnam_faked=no],[zsh_cv_sys_getpwnam_faked=yes],[zsh_cv_sys_getpwnam_faked=no])])
    if test x$zsh_cv_sys_getpwnam_faked = xyes; then
      AC_DEFINE(GETPWNAM_FAKED)
    fi
fi


dnl ---------------
dnl check for the type of third argument of accept
dnl ---------------

zsh_CHECK_SOCKLEN_T

dnl ---------------
dnl Check for pty multiplexer for use in pty module.
dnl We need to open it read/write, so make sure it is writeable.
dnl Yet another test which won't work when cross-compiling.
dnl ---------------
AC_CACHE_CHECK(if your system has /dev/ptmx,
ac_cv_have_dev_ptmx,
[if test -w /dev/ptmx; then
  ac_cv_have_dev_ptmx=yes
else
  ac_cv_have_dev_ptmx=no
fi])

dnl --------
dnl Check if the ptmx functions are usable.
dnl We need to be able to find the prototypes, which may
dnl require non-POSIX source definitions.  So test to see
dnl if ptsname is correctly recognised as returning a char *.
dnl We do this by making sure a program where ptsname() is declared
dnl as returning int does *not* compile.
dnl On Linux we need the XOPEN extensions.  The easiest way to get
dnl these is by defining _GNU_SOURCE.
dnl -------
AH_TEMPLATE([USE_DEV_PTMX],
[Define to 1 if all the kit for using /dev/ptmx for ptys is available.])
if test x$ac_cv_have_dev_ptmx = xyes -o x$ac_cv_func_posix_openpt = xyes && \
   test x$ac_cv_func_grantpt = xyes && \
   test x$ac_cv_func_unlockpt = xyes && \
   test x$ac_cv_func_ptsname = xyes; then
   AC_CACHE_CHECK([if /dev/ptmx is usable],
   ac_cv_use_dev_ptmx,
   [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if defined(__linux) || defined(__CYGWIN__)
#define _GNU_SOURCE 1
#endif
#include <stdlib.h>
int ptsname();]], [[]])],[ac_cv_use_dev_ptmx=no],[ac_cv_use_dev_ptmx=yes])])
   if test x$ac_cv_use_dev_ptmx = xyes; then
     AC_DEFINE(USE_DEV_PTMX)
   fi
fi

dnl -----------------
dnl multibyte support
dnl -----------------
AC_ARG_ENABLE(multibyte,
AS_HELP_STRING([--enable-multibyte],[support multibyte characters]),
[zsh_cv_c_unicode_support=$enableval],
[AC_CACHE_VAL(zsh_cv_c_unicode_support,
  AC_MSG_NOTICE([checking for functions supporting multibyte characters])
  [zfuncs_absent=
dnl
dnl Note that iswblank is not included and checked separately.
dnl As iswblank() was added to C long after the others, we still
dnl want to enabled unicode support even if iswblank is not available
dnl (we then just do the SPC+TAB approximation)
dnl
   for zfunc in iswalnum iswcntrl iswdigit iswgraph iswlower iswprint \
iswpunct iswspace iswupper iswxdigit mbrlen mbrtowc towupper towlower \
wcschr wcscpy wcslen wcsncmp wcsncpy wcrtomb wcwidth wmemchr wmemcmp \
wmemcpy wmemmove wmemset; do
     AC_CHECK_FUNC($zfunc,
     [:], [zfuncs_absent="$zfuncs_absent $zfunc"])
    done
    if test x"$zfuncs_absent" = x; then
      AC_MSG_NOTICE([all functions found, multibyte support enabled])
      zsh_cv_c_unicode_support=yes
    else
      # Warns at the end of configure
      AC_MSG_NOTICE([missing functions, multibyte support disabled])
      zsh_cv_c_unicode_support=no
    fi
  ])
])
AH_TEMPLATE([MULTIBYTE_SUPPORT],
[Define to 1 if you want support for multibyte character sets.])

dnl
dnl unicode9 support
dnl
AH_TEMPLATE([ENABLE_UNICODE9],
[Define to 1 if you want use unicode9 character widths.])
AC_ARG_ENABLE(unicode9,
AS_HELP_STRING([--enable-unicode9],[compile with unicode9 character widths]),
[if test x$enableval = xyes; then
  AC_DEFINE(ENABLE_UNICODE9)
fi])

AH_TEMPLATE([BROKEN_ISPRINT],
[Define to 1 if the isprint() function is broken under UTF-8 locale.])

if test x$zsh_cv_c_unicode_support = xyes; then
  AC_DEFINE(MULTIBYTE_SUPPORT)

  dnl Test if wcwidth() and/or iswprint() is broken for
  dnl   zero-width combining characters, or
  dnl   some characters in the Latin Extended-B.
  dnl If either of the functions is broken, both functions will be replaced
  dnl by the ones from wcwidth9.h by defining ENABLE_UNICODE9. We will do
  dnl this only if __STDC_ISO_10646__ is defined (or if building on macOS,
  dnl where __STDC_ISO_10646__ is not defined but wchar_t is UCS).
  dnl For the test we use a combining acute accent (\u0301) or
  dnl a LATIN SMALL LETTER L WITH CURL (\u0234).
  dnl We input it as UTF-8 since that is the standard we can rely
  dnl upon most:  we can't rely on a wchar_t being stored as a
  dnl Unicode code point on all systems.
  dnl The programme returns 0 only if all the conditions for brokenness
  dnl are met:
  dnl - the programme compiled, linked and ran
  dnl - we successfully set a UTF-8 locale
  dnl - the locale we set plausibly converted the UTF-8 string
  dnl   into the correct wide character
  dnl - but wcwidth() or iswprint() is broken for the converted wide character.
  dnl locale -a is a fallback; on most systems we should find en_US.UTF-8.
  [locale_prog='char *my_locales[] = {
  "en_US.UTF-8", "en_GB.UTF-8", "en.UTF-8", '
  locale_prog="$locale_prog"`locale -a 2>/dev/null | \
    sed -e 's/utf8/UTF-8/' | grep UTF-8 | \
    while read line; do echo " \"$line\","; done;`
  locale_prog="$locale_prog 0 };
  #define _XOPEN_SOURCE
  #include <stdlib.h>
  #include <locale.h>
  #include <wchar.h>
  #include <wctype.h>

  int main() {
    char **localep;
    char comb_acute_mb[] = { (char)0xcc, (char)0x81 };
    char u_0234[] = { (char)0xc8, (char)0xb4 };
    wchar_t wc;
  #if !defined(__STDC_ISO_10646__) && !defined(__APPLE__)
    return 1;
  #endif

    for (localep = my_locales; *localep; localep++)
      if (setlocale(LC_ALL, *localep))
	  break;
    if (!*localep)
      return 1;
    if (mbtowc(&wc, comb_acute_mb, 2) == 2 && (wcwidth(wc) != 0 || !iswprint(wc)))
      return 0;
    if (mbtowc(&wc, u_0234, 2) == 2 && (wcwidth(wc) != 1 || !iswprint(wc)))
      return 0;
    return 1;
  }
  "]

  AC_CACHE_CHECK(if the wcwidth() and/or iswprint() functions are broken,
  zsh_cv_c_broken_wcwidth,
  [AC_RUN_IFELSE([AC_LANG_SOURCE([[$locale_prog]])],[zsh_cv_c_broken_wcwidth=yes],[zsh_cv_c_broken_wcwidth=no],[zsh_cv_c_broken_wcwidth=no])])
  if test x$zsh_cv_c_broken_wcwidth = xyes; then
    AC_DEFINE(ENABLE_UNICODE9)
  fi

  dnl Check if isprint() behaves correctly under UTF-8 locale.
  dnl On some platform (maybe only on Mac OS X), isprint() returns
  dnl true for all characters in the range from 0xa0 to 0xff if
  dnl called under UTF-8 locale.
  [locale_prog='char *my_locales[] = {
  "en_US.UTF-8", "en_GB.UTF-8", "en.UTF-8", '
  locale_prog="$locale_prog"`locale -a 2>/dev/null | \
    sed -e 's/utf8/UTF-8/' | grep UTF-8 | \
    while read line; do echo " \"$line\","; done;`
  locale_prog="$locale_prog 0 };
  #include <locale.h>
  #include <ctype.h>

  int main() {
    char **localep;
    for (localep = my_locales; *localep; localep++)
      if (setlocale(LC_ALL, *localep) && isprint(0xa0))
	return 0;
    return 1;
  }
  "]

  AC_CACHE_CHECK(if the isprint() function is broken,
  zsh_cv_c_broken_isprint,
  [AC_RUN_IFELSE([AC_LANG_SOURCE([[$locale_prog]])],[zsh_cv_c_broken_isprint=yes],[zsh_cv_c_broken_isprint=no],[zsh_cv_c_broken_isprint=no])])
  if test x$zsh_cv_c_broken_isprint = xyes; then
    AC_DEFINE(BROKEN_ISPRINT)
  fi
fi

dnl
dnl musl support
dnl
AH_TEMPLATE([LIBC_MUSL],
[Define to 1 if musl is being used as the C library])
AC_ARG_ENABLE(libc-musl,
AS_HELP_STRING([--enable-libc-musl],[compile with musl as the C library]),
[if test x$enableval = xyes; then
  AC_DEFINE(LIBC_MUSL)
fi])

dnl
dnl static user lookup
dnl
AC_ARG_ENABLE(dynamic-nss,
	      AS_HELP_STRING([--disable-dynamic-nss],[do not call
			      functions that will require dynamic NSS
			      modules]),
[zsh_cv_c_dynamic_nss=$enableval],
[])

AH_TEMPLATE([DISABLE_DYNAMIC_NSS],
[Define to 1 if you want to avoid calling functions that will require
 dynamic NSS modules.])
if test x$zsh_cv_c_dynamic_nss = xno; then
  AC_DEFINE(DISABLE_DYNAMIC_NSS)
fi

dnl ---------------
dnl dynamic loading
dnl ---------------
AH_TEMPLATE([HPUX10DYNAMIC],
[Define to 1 if you want to use dynamically loaded modules on HPUX 10.])
L=N
INSTLIB="install.bin-\$(L)"
UNINSTLIB="uninstall.bin-\$(L)"
LINKMODS=NOLINKMODS
MOD_EXPORT=
MOD_IMPORT_VARIABLE=
MOD_IMPORT_FUNCTION=
aixdynamic=no
hpuxdynamic=no
if test "$ac_cv_func_load"      = yes &&
   test "$ac_cv_func_unload"    = yes &&
   test "$ac_cv_func_loadbind"  = yes &&
   test "$ac_cv_func_loadquery" = yes; then
  dnl Force AIXDYNAMIC even on newer versions that have dl family
  if test "x$dynamic" = xyes; then
    aixdynamic=yes
  fi
elif test "$ac_cv_func_dlopen"  != yes ||
     test "$ac_cv_func_dlsym"   != yes ||
     test "$ac_cv_func_dlerror" != yes; then
  if test "$ac_cv_func_shl_load" != yes ||
     test "$ac_cv_func_shl_unload" != yes ||
     test "$ac_cv_func_shl_findsym" != yes; then
    dynamic=no
  elif test "x$dynamic" = xyes; then
    hpuxdynamic=yes
    DL_EXT="${DL_EXT=sl}"
    dnl autoheader won't allow us to define anything which isn't
    dnl going into a header, and we can't undefine anything, so
    dnl just define this anyway and rely on the later tests to
    dnl define DYNAMIC or not.
    AC_DEFINE(HPUX10DYNAMIC)dnl
  fi
fi

test -n "$GCC" && LDARG=-Wl,

AH_TEMPLATE([DLSYM_NEEDS_UNDERSCORE],
[Define to 1 if an underscore has to be prepended to dlsym() argument.])
AH_TEMPLATE([DYNAMIC_NAME_CLASH_OK],
[Define to 1 if multiple modules defining the same symbol are OK.])
if test "x$aixdynamic" = xyes; then
  DL_EXT="${DL_EXT=so}"
  DLLD="${DLLD=$CC}"
  zsh_cv_func_dlsym_needs_underscore=no
  if test -n "$GCC"; then
    DLLDFLAGS=${DLLDFLAGS=-shared}
  else
    DLLDFLAGS=${DLLDFLAGS=-bM:SRE}
  fi
  DLLDFLAGS=${DLLDFLAGS=}
  EXTRA_LDFLAGS=${EXTRA_LDFLAGS=}
  EXPOPT=${LDARG}-bE:
  IMPOPT=${LDARG}-bI:
  zsh_cv_sys_dynamic_clash_ok="${zsh_cv_sys_dynamic_clash_ok=yes}"
  zsh_cv_sys_dynamic_rtld_global="${zsh_cv_sys_dynamic_rtld_global=yes}"
  zsh_cv_sys_dynamic_execsyms="${zsh_cv_sys_dynamic_execsyms=yes}"
  zsh_cv_sys_dynamic_strip_exe="${zsh_cv_sys_dynamic_strip_exe=yes}"
  zsh_cv_sys_dynamic_strip_lib="${zsh_cv_sys_dynamic_strip_lib=yes}"
  zsh_cv_shared_environ="${zsh_cv_shared_environ=yes}"
elif test "$host_os" = cygwin; then
  DL_EXT="${DL_EXT=dll}"
##DLLD="${DLLD=dllwrap}"
  DLLD="${DLLD=$CC}"
##DLLDFLAGS="${DLLDFLAGS=--export-all-symbols}"
  DLLDFLAGS=${DLLDFLAGS=-shared -Wl,--export-all-symbols}
  zsh_cv_func_dlsym_needs_underscore=no
  DLLDFLAGS=${DLLDFLAGS=}
  EXTRA_LDFLAGS=${EXTRA_LDFLAGS=}
  zsh_cv_sys_dynamic_clash_ok="${zsh_cv_sys_dynamic_clash_ok=no}"
  zsh_cv_sys_dynamic_rtld_global="${zsh_cv_sys_dynamic_rtld_global=yes}"
  zsh_cv_sys_dynamic_execsyms="${zsh_cv_sys_dynamic_execsyms=no}"
  zsh_cv_sys_dynamic_strip_exe="${zsh_cv_sys_dynamic_strip_exe=yes}"
  zsh_cv_sys_dynamic_strip_lib="${zsh_cv_sys_dynamic_strip_lib=yes}"
  #
  # THAT SUCKS! and must be changed
  #
  zsh_cv_shared_environ="${zsh_cv_shared_environ=yes}"
  LINKMODS=LINKMODS
  MOD_EXPORT="__attribute__((__dllexport__))"
  MOD_IMPORT_VARIABLE="__attribute__((__dllimport__))"
  MOD_IMPORT_FUNCTION=
elif test "x$dynamic" = xyes; then
  AC_CACHE_CHECK(if your system uses ELF binaries,
   zsh_cv_sys_elf,
   [AC_RUN_IFELSE([AC_LANG_SOURCE([[/* Test for whether ELF binaries are produced */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
main(argc, argv)
int argc;
char *argv[];
{
	char b[4];
	int i = open(argv[0],O_RDONLY);
	if(i == -1) 
		exit(1); /* fail */
	if(read(i,b,4)==4 && b[0]==127 && b[1]=='E' && b[2]=='L' && b[3]=='F')
		exit(0); /* succeed (yes, it's ELF) */
	else
		exit(1); /* fail */
}]])],[zsh_cv_sys_elf=yes],[zsh_cv_sys_elf=no],[zsh_cv_sys_elf=yes])])

  # We use [0-9]* in case statements, so need to change quoting
  changequote(, )

  DL_EXT="${DL_EXT=so}"
  if test x$zsh_cv_sys_elf = xyes; then
    case "$host" in
      mips-sni-sysv4*)
        # Forcibly set ld to native compiler to avoid obscure GCC problems
	DLLD="${DLLD=/usr/ccs/bin/cc}"
	DLLDARG="${LDARG}"
      ;;
      * )
	DLLD="${DLLD=$CC}"
	DLLDARG="${LDARG}"
      ;;
    esac
  else
    case "$host" in
      *openbsd*)
	case "$host_os" in
	  openbsd[01].* | openbsd2.[0-7] | openbsd2.[0-7].*)
	    DLLD="${DLLD=ld}"
	  ;;
	  *)
	    DLLD="${DLLD=$CC}"
	  ;;
	esac
	DLLDARG="${LDARG}"
      ;;
      *darwin*)
        DLLD="${DLLD=$CC}"
        DLLDARG=""
      ;;
      *interix*)
        DLLD="${DLLD=$CC}"
        DLLDARG=""
      ;;
      * )
	DLLD="${DLLD=ld}"
	DLLDARG=""
      ;;
    esac
  fi
  if test -n "$GCC"; then
    case "$host_os" in
      hpux*)   DLLDFLAGS="${DLLDFLAGS=-shared}" ;;
      darwin*) DLCFLAGS="${DLCFLAGS=-fno-common}" ;;
      interix*) DLCFLAGS="${DLCFLAGS=}" ;;
      *)       DLCFLAGS="${DLCFLAGS=-fPIC}" ;;
    esac
  else
    case "$host_os" in
      hpux*)
        DLCFLAGS="${DLCFLAGS=+z}"
        DLLDFLAGS="${DLLDFLAGS=-b}"
      ;;
      sunos*)                DLCFLAGS="${DLCFLAGS=-pic}" ;;
      solaris*|sysv4*|esix*) DLCFLAGS="${DLCFLAGS=-KPIC}" ;;
    esac
  fi
  case "$host_os" in
    osf*) DLLDFLAGS="${DLLDFLAGS=-shared -expect_unresolved '*'}" ;;
    *freebsd*|*netbsd*|linux*|irix*|gnu*|interix*|dragonfly*) DLLDFLAGS="${DLLDFLAGS=-shared}" ;;
    sunos*)       DLLDFLAGS="${DLLDFLAGS=-assert nodefinitions}" ;;
    sysv4*|esix*) DLLDFLAGS="${DLLDFLAGS=-G $ldflags}" ;;
    aix*)         DLLDFLAGS="${DLLDFLAGS=-G -bexpall -lc}" ;;
    solaris*|sysv4*|esix*) DLLDFLAGS="${DLLDFLAGS=-G}" ;;
    darwin*)      DLLDFLAGS="${DLLDFLAGS=-bundle -flat_namespace -undefined suppress}" ;;
    beos*|haiku*) DLLDFLAGS="${DLLDFLAGS=-nostart}" ;;
    openbsd*)
      if test x$zsh_cv_sys_elf = xyes; then
	DLLDFLAGS="${DLLDFLAGS=-shared -fPIC}"
      else
	case "$host_os" in
	  openbsd[01].* | openbsd2.[0-7] | openbsd2.[0-7].*)
	   DLLDFLAGS="${DLLDFLAGS=-Bshareable}"
	  ;;
	  *)
	    DLLDFLAGS="${DLLDFLAGS=-shared -fPIC}"
	  ;;
	esac
      fi
    ;;
  esac
  case "$host" in
    *-hpux*)  EXTRA_LDFLAGS="${EXTRA_LDFLAGS=-Wl,-E}" ;;
    *openbsd*)
       if test x$zsh_cv_sys_elf = xyes; then
	 EXTRA_LDFLAGS="${EXTRA_LDFLAGS=-Wl,-E}"
       fi
    ;;
    mips-sni-sysv4)
      #
      # unfortunately, we have different compilers
      # that need different flags
      #
      if test -n "$GCC"; then
        sni_cc_version=GCC
      else
        sni_cc_version=`$CC -V 2>&1 | head -1`
      fi
      case "$sni_cc_version" in
        *CDS*|GCC )
         EXTRA_LDFLAGS="${EXTRA_LDFLAGS=-Wl,-Blargedynsym}"
       ;;
       * )
         EXTRA_LDFLAGS="${EXTRA_LDFLAGS=-LD-Blargedynsym}"
       ;;
      esac
    ;;
    *-beos*)
      # gcc on BeOS doesn't like -rdynamic...
      EXTRA_LDFLAGS="${EXTRA_LDFLAGS= }"
      # also, dlopen() at least in Zeta respects $LIBRARY_PATH, so needs %A added to it.
      export LIBRARY_PATH="$LIBRARY_PATH:%A/"
    ;;
    *-haiku*)
      # 
    ;;
  esac

  # Done with our shell code, so restore autotools quoting
  changequote([, ])

AC_CACHE_CHECK(if we can use -rdynamic, zsh_cv_rdynamic_available,
old_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS -rdynamic"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[zsh_cv_rdynamic_available=yes
EXTRA_LDFLAGS="${EXTRA_LDFLAGS=-rdynamic}"],[zsh_cvs_rdynamic_available=no])
LDFLAGS="$old_LDFLAGS")
  AC_CACHE_CHECK(if your dlsym() needs a leading underscore,
   zsh_cv_func_dlsym_needs_underscore,
   [echo failed >conftestval && cat >conftest.c <<EOM
fred () { }
EOM
    AC_TRY_COMMAND($CC -c $CFLAGS $CPPFLAGS $DLCFLAGS conftest.c 1>&AS_MESSAGE_LOG_FD) &&
    AC_TRY_COMMAND($DLLD $LDFLAGS $DLLDFLAGS -o conftest.$DL_EXT conftest.o 1>&AS_MESSAGE_LOG_FD) &&
    AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <stdlib.h>
#ifdef HPUX10DYNAMIC
#include <dl.h>
#define RTLD_LAZY BIND_DEFERRED
#define RTLD_GLOBAL DYNAMIC_PATH

char *zsh_gl_sym_addr ;

#define dlopen(file,mode) (void *)shl_load((file), (mode), (long) 0)
#define dlclose(handle) shl_unload((shl_t)(handle))
#define dlsym(handle,name) (zsh_gl_sym_addr=0,shl_findsym((shl_t *)&(handle),name,TYPE_UNDEFINED,&zsh_gl_sym_addr), (void *)zsh_gl_sym_addr)
#define dlerror() 0
#else
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#else
#include <sys/types.h>
#include <nlist.h>
#include <link.h>
#endif
#endif
#ifndef RTLD_LAZY
#define RTLD_LAZY 1
#endif

extern int fred() ;

main()
{
    void * handle ;
    void * symbol ;
    FILE *f=fopen("conftestval", "w");
    if (!f) exit(1);
    handle = dlopen("./conftest.$DL_EXT", RTLD_LAZY) ;
    if (handle == NULL) {
        fprintf (f, "dlopen failed") ;
            exit(1);
    }
    symbol = dlsym(handle, "fred") ;
    if (symbol == NULL) {
                /* try putting a leading underscore */
        symbol = dlsym(handle, "_fred") ;
        if (symbol == NULL) {
            fprintf (f, "dlsym failed") ;
                exit(1);
                }
        fprintf (f, "yes") ;
    }
    else
        fprintf (f, "no") ;
    exit(0);
}]])],[zsh_cv_func_dlsym_needs_underscore=`cat conftestval`],[zsh_cv_func_dlsym_needs_underscore=failed
    dynamic=no],[zsh_cv_func_dlsym_needs_underscore=no])])
  if test "x$zsh_cv_func_dlsym_needs_underscore" = xyes; then
    AC_DEFINE(DLSYM_NEEDS_UNDERSCORE)
  elif test "x$zsh_cv_func_dlsym_needs_underscore" != xno; then
    dnl Do not cache failed value
    unset zsh_cv_func_dlsym_needs_underscore
  fi
fi

if test "x$dynamic" = xyes; then
  zsh_SHARED_VARIABLE([environ], [char **])
  test "$zsh_cv_shared_environ" = yes || dynamic=no
  if test "$ac_cv_func_tgetent" = yes; then
    zsh_SHARED_FUNCTION([tgetent])
  fi
  if test "$ac_cv_func_tigetstr" = yes; then
    zsh_SHARED_FUNCTION([tigetstr])
  fi
fi

if test "x$dynamic" = xyes; then
  zsh_SYS_DYNAMIC_CLASH
  zsh_SYS_DYNAMIC_GLOBAL
  RTLD_GLOBAL_OK=$zsh_cv_sys_dynamic_rtld_global
  zsh_SYS_DYNAMIC_EXECSYMS
  if test "$zsh_cv_sys_dynamic_execsyms" != yes; then
    L=L
  fi
  zsh_SYS_DYNAMIC_STRIP_EXE
  zsh_SYS_DYNAMIC_STRIP_LIB
  if $strip_exeldflags && test "$zsh_cv_sys_dynamic_strip_exe" = yes; then
    EXELDFLAGS="$EXELDFLAGS -s"
  fi
  if $strip_libldflags && test "$zsh_cv_sys_dynamic_strip_lib" = yes; then
    LIBLDFLAGS="$LIBLDFLAGS -s"
  fi
  if test "$host_os" = cygwin; then
    INSTLIB="install.cygwin-lib"
    UNINSTLIB="uninstall.cygwin-lib"
  fi
else
  $strip_exeldflags && EXELDFLAGS="$EXELDFLAGS -s"
  $strip_libldflags && LIBLDFLAGS="$LIBLDFLAGS -s"
  RTLD_GLOBAL_OK=no
fi

AH_TEMPLATE([DYNAMIC],
[Define to 1 if you want to use dynamically loaded modules.])
if test "x$dynamic" = xyes; then
  D=D
  AC_DEFINE(DYNAMIC)dnl
else
  D=N
fi

AH_TEMPLATE([AIXDYNAMIC],
[Define to 1 if you want to use dynamically loaded modules on AIX.])
if test "x$aixdynamic" = xyes; then
  E=E
  AC_DEFINE(AIXDYNAMIC)dnl
else
  E=N
fi

if test "x$zsh_cv_sys_dynamic_clash_ok" = xyes; then
  SHORTBOOTNAMES=yes
else
  SHORTBOOTNAMES=no
fi
AC_SUBST(SHORTBOOTNAMES)

AC_SUBST(INSTLIB)dnl
AC_SUBST(UNINSTLIB)dnl

if test "$host_os" = cygwin; then
  EXTRAZSHOBJS="$EXTRAZSHOBJS zsh.res.o"
fi

AC_DEFINE_UNQUOTED(DL_EXT, "$DL_EXT",
[The extension used for dynamically loaded modules.])dnl
AC_SUBST(D)dnl
AC_SUBST(DL_EXT)dnl
AC_SUBST(DLLD)dnl
AC_SUBST(DLCFLAGS)dnl
AC_SUBST(DLLDFLAGS)dnl
AC_SUBST(E)dnl
AC_SUBST(EXTRA_LDFLAGS)dnl
AC_SUBST(EXPOPT)dnl
AC_SUBST(IMPOPT)dnl
AC_SUBST(L)dnl
AC_SUBST(LINKMODS)dnl
AC_SUBST(MOD_EXPORT)dnl
AC_SUBST(MOD_IMPORT_VARIABLE)dnl
AC_SUBST(MOD_IMPORT_FUNCTION)dnl
AC_SUBST(EXTRAZSHOBJS)dnl

# Generate config.modules.  We look for *.mdd files in first and second
# level subdirectories.  Any existing line not containing 'auto=y' will be
# retained, provided the .mdd file itself was found.
CONFIG_MODULES=./config.modules
cat <<EOM > ${CONFIG_MODULES}.sh
srcdir="$srcdir"
dynamic="$dynamic"
CONFIG_MODULES="${CONFIG_MODULES}"
EOM
cat <<\EOM >> ${CONFIG_MODULES}.sh
echo "creating ${CONFIG_MODULES}"
userlist=" "
if test -f ${CONFIG_MODULES}; then
  userlist="`sed -e '/^#/d' -e '/auto=y/d' -e 's/ .*/ /' -e 's/^name=/ /' \
        ${CONFIG_MODULES}`"
  mv ${CONFIG_MODULES} ${CONFIG_MODULES}.old
else
  # Save testing for existence each time.
  echo > ${CONFIG_MODULES}.old
fi
(echo "# Edit this file to change the way modules are loaded."
echo "# The format is strict; do not break lines or add extra spaces."
echo "# Run \`make prep' if you change anything here after compiling"
echo "# (there is no need if you change this just after the first time"
echo "# you run \`configure')."
echo "#"
echo "# Values of \`link' are \`static', \`dynamic' or \`no' to compile the"
echo "# module into the shell, link it in at run time, or not use it at all."
echo "# In the final case, no attempt will be made to compile it."
echo "# Use \`static' or \`no' if you do not have dynamic loading."
echo "#"
echo "# Values of \`load' are \`yes' or \`no'; if yes, any builtins etc."
echo "# provided by the module will be autoloaded by the main shell"
echo "# (so long as \`link' is not set to \`no')."
echo "#"
echo "# Values of \`auto' are \`yes' or \`no'. configure sets the value to"
echo "# \`yes'.  If you set it by hand to \`no', the line will be retained"
echo "# when the file is regenerated in future."
echo "#"
echo "# Note that the \`functions' entry extends to the end of the line."
echo "# It should not be quoted; it is used verbatim to find files to install."
echo "#"
echo "# You will need to run \`config.status --recheck' if you add a new"
echo "# module."
echo "#"
echo "# You should not change the values for the pseudo-module zsh/main,"
echo "# which is the main shell (apart from the functions entry)."
EOM
dnl The autoconf macros are only available in configure, not
dnl config.status, and only change when configure is rerun.
dnl So we need to run the autoconf tests here and store the results.
dnl We then generate config.modules, preserving any user-generated
dnl information, from config.status.
for modfile in `cd ${srcdir}; echo */*.mdd */*/*.mdd`; do
  name=
  link=
  load=
  functions=
  result=
  . ${srcdir}/$modfile
  if test x$name != x && test x"$link" != x; then
    case "$link" in
      *\ *) eval "link=\`$link\`"
	    ;;
    esac
    case "${load}" in
      y*) load=" load=yes"
	  ;;
      *) load=" load=no"
	 ;;
    esac
    if test "x$functions" != x; then
      # N.B. no additional quotes
      f=" functions=$functions"
    else
      f=
    fi
    case "$link" in
      static) result="name=$name modfile=$modfile link=static auto=yes${load}$f"
	      ;;
      dynamic) if test x$dynamic != xno; then
		  result="name=$name modfile=$modfile link=dynamic\
 auto=yes${load}$f"
	       else
		 result="name=$name modfile=$modfile link=no\
 auto=yes load=no$f"
	       fi
	       ;;
      either) if test x$dynamic != xno; then
		result="name=$name modfile=$modfile link=dynamic\
 auto=yes${load}$f"
	      else
		result="name=$name modfile=$modfile link=static\
 auto=yes${load}$f"
	      fi
	      ;;
       *) result="name=$name modfile=$modfile link=no auto=yes load=no$f"
	  ;;
    esac
dnl $result is the default output for config.modules.  We generate
dnl code to check if we should use this.
cat <<EOM >> ${CONFIG_MODULES}.sh
case "\$userlist" in
  *" $name "*) grep "^name=$name " \${CONFIG_MODULES}.old;;
  *) echo "$result";;
esac
EOM
  fi
done
cat <<\EOM >> ${CONFIG_MODULES}.sh
) >${CONFIG_MODULES}
rm -f ${CONFIG_MODULES}.old
EOM

dnl AH_TOP replaces the code which used to appear at the top
dnl of acconfig.h.
AH_TOP([/***** begin user configuration section *****/

/* Define this to be the location of your password file */
#define PASSWD_FILE "/etc/passwd"

/* Define this to be the name of your NIS/YP password *
 * map (if applicable)                                */
#define PASSWD_MAP "passwd.byname"

/* Define to 1 if you want user names to be cached */
#define CACHE_USERNAMES 1

/* Define to 1 if system supports job control */
#define JOB_CONTROL 1

/* Define this if you use "suspended" instead of "stopped" */
#define USE_SUSPENDED 1
 
/* The default history buffer size in lines */
#define DEFAULT_HISTSIZE 30

/* The default editor for the fc builtin */
#define DEFAULT_FCEDIT "vi"

/* The default prefix for temporary files */
#define DEFAULT_TMPPREFIX "/tmp/zsh"

/***** end of user configuration section            *****/
/***** shouldn't have to change anything below here *****/

])

CLEAN_MK="${srcdir}/Config/clean.mk"
CONFIG_MK="${srcdir}/Config/config.mk"
dnl defs.mk is in the build tree, not the source tree
DEFS_MK="Config/defs.mk"
VERSION_MK="${srcdir}/Config/version.mk"

AC_SUBST_FILE(CLEAN_MK)dnl
AC_SUBST_FILE(CONFIG_MK)dnl
AC_SUBST_FILE(DEFS_MK)dnl
AC_SUBST_FILE(VERSION_MK)dnl

AC_CONFIG_FILES(Config/defs.mk Makefile Doc/Makefile Etc/Makefile \
Src/Makefile Test/Makefile)
AC_CONFIG_COMMANDS([config.modules], [. ./config.modules.sh])
AC_CONFIG_COMMANDS([stamp-h], [echo >stamp-h])

AC_OUTPUT

dnl Copy pre-built man pages and help files, for tarball out-of-tree builds.
for manpage in `cd ${srcdir} && echo Doc/*.1`; do
  if test x"$manpage" != x"Doc/*.1" && ! test -e "${manpage}"; then
    cp ${srcdir}/${manpage} ./Doc/
  fi
done
if ! test -e Doc/help.txt; then
  cp ${srcdir}/Doc/help.txt ./Doc/
fi
mkdir -p ./Doc/help
for helpfile in `cd ${srcdir} && echo Doc/help/*`; do
  if test x"$helpfile" != x"Doc/help/*" && ! test -e "${helpfile}"; then
    cp ${srcdir}/${helpfile} ./Doc/help/
  fi
done

eval "zshbin1=${bindir}"
eval "zshbin2=${zshbin1}"
eval "zshman1=${mandir}"
eval "zshman2=${zshman1}"
eval "zshinfo1=${infodir}"
eval "zshinfo2=${zshinfo1}"
eval "zshfndir1=${fndir}"
eval "zshfndir2=${zshfndir1}"

echo "
zsh configuration
-----------------
zsh version               : ${VERSION}
host operating system     : ${host_cpu}-${host_vendor}-${host_os}
source code location      : ${srcdir}
compiler                  : ${CC}
preprocessor flags        : ${CPPFLAGS}
executable compiler flags : ${CFLAGS}"
if test "x$dynamic" = xyes; then
  echo "\
module compiler flags     : ${CFLAGS} ${DLCFLAGS}"
fi
echo "\
executable linker flags   : ${LDFLAGS} ${EXELDFLAGS} ${EXTRA_LDFLAGS}"
if test "x$dynamic" = xyes; then
  echo "\
module linker flags       : ${LDFLAGS} ${LIBLDFLAGS} ${DLLDFLAGS}"
fi
echo "\
library flags             : ${LIBS}
installation basename     : ${tzsh_name}
binary install path       : ${zshbin2}
man page install path     : ${zshman2}
info install path         : ${zshinfo2}"
if test "$zshfndir2" != no; then
  echo "functions install path    : ${zshfndir2}"
fi
if test "x$additionalfpath" != x; then
  echo "additional fpath entries  : ${additionalfpath}"
fi
echo "See config.modules for installed modules and functions.
"

if test x$zsh_cv_c_unicode_support != xyes; then
  if test "x$zfuncs_absent" = x; then
    # The user opted out.
    AC_MSG_WARN([You have chosen to build without multibyte support.])
    AC_MSG_WARN([This configuration may not be suitable for production use. It is known to cause errors in 'make test'. We strongly recommend to re-run configure with --enable-multibyte.])
  else
    # Some requisite functions are missing.
    AC_MSG_WARN([Multibyte support cannot be enabled: some standard library functions are missing: $zfuncs_absent])
    AC_MSG_WARN([This configuration may not be suitable for production use. It is known to cause errors in 'make test'. If your system provides those functions, we recommend to re-run configure appropriately.])
    # If your system doesn't have those functions, consider patching the
    # test suite and sending the patch to zsh-workers@ for inclusion.
  fi
fi

exit 0
