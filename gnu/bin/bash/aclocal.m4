dnl
dnl Bash specific tests
dnl
dnl Some derived from PDKSH 5.1.3 autoconf tests
dnl
dnl Copyright (C) 1987-2021 Free Software Foundation, Inc.
dnl

dnl
dnl Check for <inttypes.h>.  This is separated out so that it can be
dnl AC_REQUIREd.
dnl
dnl BASH_HEADER_INTTYPES
AC_DEFUN(BASH_HEADER_INTTYPES,
[
 AC_CHECK_HEADERS(inttypes.h)
])

dnl
dnl check for typedef'd symbols in header files, but allow the caller to
dnl specify the include files to be checked in addition to the default
dnl
dnl This could be changed to use AC_COMPILE_IFELSE instead of AC_EGREP_CPP
dnl 
dnl BASH_CHECK_TYPE(TYPE, HEADERS, DEFAULT[, VALUE-IF-FOUND])
AC_DEFUN(BASH_CHECK_TYPE,
[
AC_REQUIRE([BASH_HEADER_INTTYPES])
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(bash_cv_type_$1,
[AC_EGREP_CPP($1, [#include <sys/types.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STDDEF_H
#include <stddef.h>
#endif
#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#if HAVE_STDINT_H
#include <stdint.h>
#endif
$2
], bash_cv_type_$1=yes, bash_cv_type_$1=no)])
AC_MSG_RESULT($bash_cv_type_$1)
ifelse($#, 4, [if test $bash_cv_type_$1 = yes; then
	AC_DEFINE($4)
	fi])
if test $bash_cv_type_$1 = no; then
  AC_DEFINE_UNQUOTED($1, $3)
fi
])

dnl
dnl BASH_CHECK_DECL(FUNC)
dnl
dnl Check for a declaration of FUNC in stdlib.h and inttypes.h like
dnl AC_CHECK_DECL
dnl
AC_DEFUN(BASH_CHECK_DECL,
[
AC_REQUIRE([BASH_HEADER_INTTYPES])
AC_CHECK_DECLS([$1])
])

AC_DEFUN(BASH_DECL_PRINTF,
[AC_MSG_CHECKING(for declaration of printf in <stdio.h>)
AC_CACHE_VAL(bash_cv_printf_declared,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#ifdef __STDC__
typedef int (*_bashfunc)(const char *, ...);
#else
typedef int (*_bashfunc)();
#endif
#include <stdlib.h>
int
main()
{
_bashfunc pf;
pf = (_bashfunc) printf;
exit(pf == 0);
}
]])], [bash_cv_printf_declared=yes], [bash_cv_printf_declared=no],
   [AC_MSG_WARN(cannot check printf declaration if cross compiling -- defaulting to yes)
    bash_cv_printf_declared=yes]
)])
AC_MSG_RESULT($bash_cv_printf_declared)
if test $bash_cv_printf_declared = yes; then
AC_DEFINE(PRINTF_DECLARED)
fi
])

AC_DEFUN(BASH_DECL_SBRK,
[AC_MSG_CHECKING(for declaration of sbrk in <unistd.h>)
AC_CACHE_VAL(bash_cv_sbrk_declared,
[AC_EGREP_HEADER(sbrk, unistd.h,
 bash_cv_sbrk_declared=yes, bash_cv_sbrk_declared=no)])
AC_MSG_RESULT($bash_cv_sbrk_declared)
if test $bash_cv_sbrk_declared = yes; then
AC_DEFINE(SBRK_DECLARED)
fi
])

dnl
dnl Check for sys_siglist[] or _sys_siglist[]
dnl
AC_DEFUN(BASH_DECL_UNDER_SYS_SIGLIST,
[AC_MSG_CHECKING([for _sys_siglist in signal.h or unistd.h])
AC_CACHE_VAL(bash_cv_decl_under_sys_siglist,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif]], [[ char *msg = _sys_siglist[2]; ]])],
  [bash_cv_decl_under_sys_siglist=yes], [bash_cv_decl_under_sys_siglist=no],
  [AC_MSG_WARN(cannot check for _sys_siglist[] if cross compiling -- defaulting to no)])])dnl
AC_MSG_RESULT($bash_cv_decl_under_sys_siglist)
if test $bash_cv_decl_under_sys_siglist = yes; then
AC_DEFINE(UNDER_SYS_SIGLIST_DECLARED)
fi
])

AC_DEFUN(BASH_UNDER_SYS_SIGLIST,
[AC_REQUIRE([BASH_DECL_UNDER_SYS_SIGLIST])
AC_MSG_CHECKING([for _sys_siglist in system C library])
AC_CACHE_VAL(bash_cv_under_sys_siglist,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#ifndef UNDER_SYS_SIGLIST_DECLARED
extern char *_sys_siglist[];
#endif
int
main()
{
char *msg = (char *)_sys_siglist[2];
exit(msg == 0);
}
]])],
	[bash_cv_under_sys_siglist=yes], [bash_cv_under_sys_siglist=no],
	[AC_MSG_WARN(cannot check for _sys_siglist[] if cross compiling -- defaulting to no)
	 bash_cv_under_sys_siglist=no]
)])
AC_MSG_RESULT($bash_cv_under_sys_siglist)
if test $bash_cv_under_sys_siglist = yes; then
AC_DEFINE(HAVE_UNDER_SYS_SIGLIST)
fi
])

dnl this defines HAVE_DECL_SYS_SIGLIST
AC_DEFUN([BASH_DECL_SYS_SIGLIST],
[AC_CHECK_DECLS([sys_siglist],,,
[#include <signal.h>
/* NetBSD declares sys_siglist in unistd.h.  */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
])
])

AC_DEFUN(BASH_SYS_SIGLIST,
[AC_REQUIRE([BASH_DECL_SYS_SIGLIST])
AC_MSG_CHECKING([for sys_siglist in system C library])
AC_CACHE_VAL(bash_cv_sys_siglist,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#if !HAVE_DECL_SYS_SIGLIST
extern char *sys_siglist[];
#endif
int
main()
{
char *msg = sys_siglist[2];
exit(msg == 0);
}
]])], [bash_cv_sys_siglist=yes], [bash_cv_sys_siglist=no],
	[AC_MSG_WARN(cannot check for sys_siglist if cross compiling -- defaulting to no)
	 bash_cv_sys_siglist=no]
)])
AC_MSG_RESULT($bash_cv_sys_siglist)
if test $bash_cv_sys_siglist = yes; then
AC_DEFINE(HAVE_SYS_SIGLIST)
fi
])

dnl Check for the various permutations of sys_siglist and make sure we
dnl compile in siglist.o if they're not defined
AC_DEFUN(BASH_CHECK_SYS_SIGLIST, [
AC_REQUIRE([BASH_SYS_SIGLIST])
AC_REQUIRE([BASH_DECL_UNDER_SYS_SIGLIST])
AC_REQUIRE([BASH_FUNC_STRSIGNAL])
if test "$bash_cv_sys_siglist" = no && test "$bash_cv_under_sys_siglist" = no && test "$bash_cv_have_strsignal" = no; then
  SIGLIST_O=siglist.o
else
  SIGLIST_O=
fi
AC_SUBST([SIGLIST_O])
])

dnl Check for sys_errlist[] and sys_nerr, check for declaration
AC_DEFUN(BASH_SYS_ERRLIST,
[AC_MSG_CHECKING([for sys_errlist and sys_nerr])
AC_CACHE_VAL(bash_cv_sys_errlist,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <errno.h>
]],[[
extern char *sys_errlist[];
extern int sys_nerr;
char *msg = sys_errlist[sys_nerr - 1];
]] )],
[bash_cv_sys_errlist=yes], [bash_cv_sys_errlist=no]
)])
AC_MSG_RESULT($bash_cv_sys_errlist)
if test $bash_cv_sys_errlist = yes; then
AC_DEFINE(HAVE_SYS_ERRLIST)
fi
])

dnl
dnl Check if dup2() does not clear the close on exec flag
dnl
AC_DEFUN(BASH_FUNC_DUP2_CLOEXEC_CHECK,
[AC_MSG_CHECKING(if dup2 fails to clear the close-on-exec flag)
AC_CACHE_VAL(bash_cv_dup2_broken,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
int
main()
{
  int fd1, fd2, fl;
  fd1 = open("/dev/null", 2);
  if (fcntl(fd1, 2, 1) < 0)
    exit(1);
  fd2 = dup2(fd1, 1);
  if (fd2 < 0)
    exit(2);
  fl = fcntl(fd2, 1, 0);
  /* fl will be 1 if dup2 did not reset the close-on-exec flag. */
  exit(fl != 1);
}
]])], [bash_cv_dup2_broken=yes], [bash_cv_dup2_broken=no],
    [AC_MSG_WARN(cannot check dup2 if cross compiling -- defaulting to no)
     bash_cv_dup2_broken=no]
)])
AC_MSG_RESULT($bash_cv_dup2_broken)
if test $bash_cv_dup2_broken = yes; then
AC_DEFINE(DUP2_BROKEN)
fi
])

AC_DEFUN(BASH_FUNC_STRSIGNAL,
[AC_MSG_CHECKING([for the existence of strsignal])
AC_CACHE_VAL(bash_cv_have_strsignal,
[AC_LINK_IFELSE(
	[AC_LANG_PROGRAM([[#include <sys/types.h>
#include <signal.h>
#include <string.h>]],
[[char *s = (char *)strsignal(2);]])],
 [bash_cv_have_strsignal=yes], [bash_cv_have_strsignal=no])])
AC_MSG_RESULT($bash_cv_have_strsignal)
if test $bash_cv_have_strsignal = yes; then
AC_DEFINE(HAVE_STRSIGNAL)
fi
])

dnl Check to see if opendir will open non-directories (not a nice thing)
AC_DEFUN(BASH_FUNC_OPENDIR_CHECK,
[AC_REQUIRE([AC_HEADER_DIRENT])dnl
AC_MSG_CHECKING(if opendir() opens non-directories)
AC_CACHE_VAL(bash_cv_opendir_not_robust,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if defined(HAVE_DIRENT_H)
# include <dirent.h>
#else
# define dirent direct
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif /* SYSNDIR */
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif /* SYSDIR */
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif /* HAVE_DIRENT_H */
#include <stdlib.h>
int
main()
{
DIR *dir;
int fd, err;
err = mkdir("bash-aclocal", 0700);
if (err < 0) {
  perror("mkdir");
  exit(1);
}
unlink("bash-aclocal/not_a_directory");
fd = open("bash-aclocal/not_a_directory", O_WRONLY|O_CREAT|O_EXCL, 0666);
write(fd, "\n", 1);
close(fd);
dir = opendir("bash-aclocal/not_a_directory");
unlink("bash-aclocal/not_a_directory");
rmdir("bash-aclocal");
exit (dir == 0);
}
]])], [bash_cv_opendir_not_robust=yes], [bash_cv_opendir_not_robust=no],
    [AC_MSG_WARN(cannot check opendir if cross compiling -- defaulting to no)
     bash_cv_opendir_not_robust=no]
)])
AC_MSG_RESULT($bash_cv_opendir_not_robust)
if test $bash_cv_opendir_not_robust = yes; then
AC_DEFINE(OPENDIR_NOT_ROBUST)
fi
])

dnl
dnl A signed 16-bit integer quantity
dnl
AC_DEFUN(BASH_TYPE_BITS16_T,
[
if test "$ac_cv_sizeof_short" = 2; then
  AC_CHECK_TYPE(bits16_t, short)
elif test "$ac_cv_sizeof_char" = 2; then
  AC_CHECK_TYPE(bits16_t, char)
else
  AC_CHECK_TYPE(bits16_t, short)
fi
])

dnl
dnl An unsigned 16-bit integer quantity
dnl
AC_DEFUN(BASH_TYPE_U_BITS16_T,
[
if test "$ac_cv_sizeof_short" = 2; then
  AC_CHECK_TYPE(u_bits16_t, unsigned short)
elif test "$ac_cv_sizeof_char" = 2; then
  AC_CHECK_TYPE(u_bits16_t, unsigned char)
else
  AC_CHECK_TYPE(u_bits16_t, unsigned short)
fi
])

dnl
dnl A signed 32-bit integer quantity
dnl
AC_DEFUN(BASH_TYPE_BITS32_T,
[
if test "$ac_cv_sizeof_int" = 4; then
  AC_CHECK_TYPE(bits32_t, int)
elif test "$ac_cv_sizeof_long" = 4; then
  AC_CHECK_TYPE(bits32_t, long)
else
  AC_CHECK_TYPE(bits32_t, int)
fi
])

dnl
dnl An unsigned 32-bit integer quantity
dnl
AC_DEFUN(BASH_TYPE_U_BITS32_T,
[
if test "$ac_cv_sizeof_int" = 4; then
  AC_CHECK_TYPE(u_bits32_t, unsigned int)
elif test "$ac_cv_sizeof_long" = 4; then
  AC_CHECK_TYPE(u_bits32_t, unsigned long)
else
  AC_CHECK_TYPE(u_bits32_t, unsigned int)
fi
])

AC_DEFUN(BASH_TYPE_PTRDIFF_T,
[
if test "$ac_cv_sizeof_int" = "$ac_cv_sizeof_char_p"; then
  AC_CHECK_TYPE(ptrdiff_t, int)
elif test "$ac_cv_sizeof_long" = "$ac_cv_sizeof_char_p"; then
  AC_CHECK_TYPE(ptrdiff_t, long)
elif test "$ac_cv_type_long_long" = yes && test "$ac_cv_sizeof_long_long" = "$ac_cv_sizeof_char_p"; then
  AC_CHECK_TYPE(ptrdiff_t, [long long])
else
  AC_CHECK_TYPE(ptrdiff_t, int)
fi
])

dnl
dnl A signed 64-bit quantity
dnl
AC_DEFUN(BASH_TYPE_BITS64_T,
[
if test "$ac_cv_sizeof_char_p" = 8; then
  AC_CHECK_TYPE(bits64_t, char *)
elif test "$ac_cv_sizeof_double" = 8; then
  AC_CHECK_TYPE(bits64_t, double)
elif test -n "$ac_cv_type_long_long" && test "$ac_cv_sizeof_long_long" = 8; then
  AC_CHECK_TYPE(bits64_t, [long long])
elif test "$ac_cv_sizeof_long" = 8; then
  AC_CHECK_TYPE(bits64_t, long)
else
  AC_CHECK_TYPE(bits64_t, double)
fi
])

AC_DEFUN(BASH_SIZEOF_RLIMIT,
[AC_MSG_CHECKING(for size of struct rlimit fields)
AC_CACHE_VAL(bash_cv_sizeof_rlim_cur,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <sys/resource.h>
main()
{
struct rlimit r;
exit(sizeof (r.rlim_cur));
}
]])], [bash_cv_sizeof_rlim_cur=$?], [bash_cv_sizeof_rlim_cur=$?],
	[AC_MSG_WARN(cannot check size of rlimit fields if cross compiling -- defaulting to long)
	 bash_cv_sizeof_rlim_cur=$ac_cv_sizeof_long]
)])
AC_MSG_RESULT($bash_cv_sizeof_rlim_cur)
])

AC_DEFUN(BASH_SIZEOF_QUAD_T,
[AC_MSG_CHECKING(for size of quad_t)
AC_CACHE_VAL(bash_cv_sizeof_quad_t,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <stdlib.h>
#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#if HAVE_STDINT_H
#include <stdint.h>
#endif

main()
{
#if HAVE_QUAD_T
quad_t x;
exit(sizeof (x));
#else
exit (0);
#endif
}
]])], [bash_cv_sizeof_quad_t=$?], [bash_cv_sizeof_quad_t=$?],
	[AC_MSG_WARN(cannot check size of quad_t if cross compiling -- defaulting to 0)
	 bash_cv_sizeof_quad_t=0]
)])
AC_MSG_RESULT($bash_cv_sizeof_quad_t)
])

dnl
dnl Type of struct rlimit fields: updated to check POSIX rlim_t and
dnl if it doesn't exist determine the best guess based on sizeof(r.rlim_cur)
dnl
AC_DEFUN(BASH_TYPE_RLIMIT,
[AC_MSG_CHECKING(for type of struct rlimit fields)
AC_CACHE_VAL(bash_cv_type_rlimit,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/resource.h>]],
[[rlim_t xxx;]]
)],
	[bash_cv_type_rlimit=rlim_t], [
BASH_SIZEOF_RLIMIT
BASH_SIZEOF_QUAD_T
if test $bash_cv_sizeof_rlim_cur = $ac_cv_sizeof_long; then
  bash_cv_type_rlimit='unsigned long'
elif test $bash_cv_sizeof_rlim_cur = $ac_cv_sizeof_long_long; then
  bash_cv_type_rlimit='unsigned long long'
elif test $bash_cv_sizeof_rlim_cur = $ac_cv_sizeof_int; then
  bash_cv_type_rlimit='unsigned int'
elif test $bash_cv_sizeof_rlim_cur = $bash_cv_sizeof_quad_t; then
  bash_cv_type_rlimit='quad_t'
else
  bash_cv_type_rlimit='unsigned long'
fi
]
)])
AC_MSG_RESULT($bash_cv_type_rlimit)
AC_DEFINE_UNQUOTED([RLIMTYPE], [$bash_cv_type_rlimit])
])

AC_DEFUN(BASH_TYPE_SIG_ATOMIC_T,
[AC_CACHE_CHECK([for sig_atomic_t in signal.h], ac_cv_have_sig_atomic_t,
[AC_LINK_IFELSE(
	[AC_LANG_PROGRAM(
		[[ #include <signal.h> ]],
		[[ sig_atomic_t x; ]])],
	[ac_cv_have_sig_atomic_t=yes],[ac_cv_have_sig_atomic_t=no])])
if test "$ac_cv_have_sig_atomic_t" = "no"
then
    BASH_CHECK_TYPE(sig_atomic_t, [#include <signal.h>], int)
fi
])

AC_DEFUN(BASH_FUNC_LSTAT,
[dnl Cannot use AC_CHECK_FUNCS(lstat) because Linux defines lstat() as an
dnl inline function in <sys/stat.h>.
AC_CACHE_CHECK([for lstat], bash_cv_func_lstat,
[AC_LINK_IFELSE(
	[AC_LANG_PROGRAM([[
		#include <sys/types.h>
		#include <sys/stat.h>
		]],
		[[ lstat(".",(struct stat *)0); ]])],
	[bash_cv_func_lstat=yes],[bash_cv_func_lstat=no])])
if test $bash_cv_func_lstat = yes; then
  AC_DEFINE(HAVE_LSTAT)
fi
])

AC_DEFUN(BASH_FUNC_INET_ATON,
[
AC_CACHE_CHECK([for inet_aton], bash_cv_func_inet_aton,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
struct in_addr ap;]], [[ inet_aton("127.0.0.1", &ap); ]])],
[bash_cv_func_inet_aton=yes], [bash_cv_func_inet_aton=no])])
if test $bash_cv_func_inet_aton = yes; then
  AC_DEFINE(HAVE_INET_ATON)
else
  AC_LIBOBJ(inet_aton)
fi
])

AC_DEFUN(BASH_FUNC_GETENV,
[AC_MSG_CHECKING(to see if getenv can be redefined)
AC_CACHE_VAL(bash_cv_getenv_redef,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <stdlib.h>
#ifndef __STDC__
#  ifndef const
#    define const
#  endif
#endif
char *
getenv (name)
#if defined (__linux__) || defined (__bsdi__) || defined (convex)
     const char *name;
#else
     char const *name;
#endif /* !__linux__ && !__bsdi__ && !convex */
{
return "42";
}
int
main()
{
char *s;
/* The next allows this program to run, but does not allow bash to link
   when it redefines getenv.  I'm not really interested in figuring out
   why not. */
#if defined (NeXT)
exit(1);
#endif
s = getenv("ABCDE");
exit(s == 0);	/* force optimizer to leave getenv in */
}
]])], [bash_cv_getenv_redef=yes], [bash_cv_getenv_redef=no],
   [AC_MSG_WARN(cannot check getenv redefinition if cross compiling -- defaulting to yes)
    bash_cv_getenv_redef=yes]
)])
AC_MSG_RESULT($bash_cv_getenv_redef)
if test $bash_cv_getenv_redef = yes; then
AC_DEFINE(CAN_REDEFINE_GETENV)
fi
])

# We should check for putenv before calling this
AC_DEFUN(BASH_FUNC_STD_PUTENV,
[
AC_REQUIRE([AC_C_PROTOTYPES])
AC_CACHE_CHECK([for standard-conformant putenv declaration], bash_cv_std_putenv,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STDDEF_H
#include <stddef.h>
#endif
#ifndef __STDC__
#  ifndef const
#    define const
#  endif
#endif
#ifdef PROTOTYPES
extern int putenv (char *);
#else
extern int putenv ();
#endif
]], [[return (putenv == 0);]] )],
[bash_cv_std_putenv=yes], [bash_cv_std_putenv=no]
)])
if test $bash_cv_std_putenv = yes; then
AC_DEFINE(HAVE_STD_PUTENV)
fi
])

# We should check for unsetenv before calling this
AC_DEFUN(BASH_FUNC_STD_UNSETENV,
[
AC_REQUIRE([AC_C_PROTOTYPES])
AC_CACHE_CHECK([for standard-conformant unsetenv declaration], bash_cv_std_unsetenv,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STDDEF_H
#include <stddef.h>
#endif
#ifndef __STDC__
#  ifndef const
#    define const
#  endif
#endif
#ifdef PROTOTYPES
extern int unsetenv (const char *);
#else
extern int unsetenv ();
#endif
]], [[return (unsetenv == 0);]] )],
[bash_cv_std_unsetenv=yes], [bash_cv_std_unsetenv=no]
)])
if test $bash_cv_std_unsetenv = yes; then
AC_DEFINE(HAVE_STD_UNSETENV)
fi
])

AC_DEFUN(BASH_FUNC_ULIMIT_MAXFDS,
[AC_MSG_CHECKING(whether ulimit can substitute for getdtablesize)
AC_CACHE_VAL(bash_cv_ulimit_maxfds,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdlib.h>
#ifdef HAVE_ULIMIT_H
#include <ulimit.h>
#endif
int
main()
{
long maxfds = ulimit(4, 0L);
exit (maxfds == -1L);
}
]])], [bash_cv_ulimit_maxfds=yes], [bash_cv_ulimit_maxfds=no],
   [AC_MSG_WARN(cannot check ulimit if cross compiling -- defaulting to no)
    bash_cv_ulimit_maxfds=no]
)])
AC_MSG_RESULT($bash_cv_ulimit_maxfds)
if test $bash_cv_ulimit_maxfds = yes; then
AC_DEFINE(ULIMIT_MAXFDS)
fi
])

AC_DEFUN(BASH_FUNC_GETCWD,
[AC_MSG_CHECKING([if getcwd() will dynamically allocate memory with 0 size])
AC_CACHE_VAL(bash_cv_getcwd_malloc,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>

int
main()
{
	char	*xpwd;
	xpwd = getcwd(0, 0);
	exit (xpwd == 0);
}
]])], [bash_cv_getcwd_malloc=yes], [bash_cv_getcwd_malloc=no],
   [AC_MSG_WARN(cannot check whether getcwd allocates memory when cross-compiling -- defaulting to no)
    bash_cv_getcwd_malloc=no]
)])
AC_MSG_RESULT($bash_cv_getcwd_malloc)
if test $bash_cv_getcwd_malloc = no; then
AC_DEFINE(GETCWD_BROKEN)
AC_LIBOBJ(getcwd)
fi
])

dnl
dnl This needs BASH_CHECK_SOCKLIB, but since that's not called on every
dnl system, we can't use AC_PREREQ. Only called if we need the socket library
dnl
AC_DEFUN(BASH_FUNC_GETHOSTBYNAME,
[if test "X$bash_cv_have_gethostbyname" = "X"; then
_bash_needmsg=yes
else
AC_MSG_CHECKING(for gethostbyname in socket library)
_bash_needmsg=
fi
AC_CACHE_VAL(bash_cv_have_gethostbyname,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <netdb.h>
]], [[
struct hostent *hp;
hp = gethostbyname("localhost");
]] )],
[bash_cv_have_gethostbyname=yes], [bash_cv_have_gethostbyname=no]
)])
if test "X$_bash_needmsg" = Xyes; then
    AC_MSG_CHECKING(for gethostbyname in socket library)
fi
AC_MSG_RESULT($bash_cv_have_gethostbyname)
if test "$bash_cv_have_gethostbyname" = yes; then
AC_DEFINE(HAVE_GETHOSTBYNAME)
fi
])

AC_DEFUN(BASH_FUNC_FNMATCH_EXTMATCH,
[AC_MSG_CHECKING(if fnmatch does extended pattern matching with FNM_EXTMATCH)
AC_CACHE_VAL(bash_cv_fnm_extmatch,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <fnmatch.h>

int
main()
{
#ifdef FNM_EXTMATCH
  return (0);
#else
  return (1);
#endif
}
]])], [bash_cv_fnm_extmatch=yes], [bash_cv_fnm_extmatch=no],
    [AC_MSG_WARN(cannot check FNM_EXTMATCH if cross compiling -- defaulting to no)
     bash_cv_fnm_extmatch=no]
)])
AC_MSG_RESULT($bash_cv_fnm_extmatch)
if test $bash_cv_fnm_extmatch = yes; then
AC_DEFINE(HAVE_LIBC_FNM_EXTMATCH)
fi
])

AC_DEFUN(BASH_FUNC_POSIX_SETJMP,
[AC_REQUIRE([BASH_SYS_SIGNAL_VINTAGE])
AC_MSG_CHECKING(for presence of POSIX-style sigsetjmp/siglongjmp)
AC_CACHE_VAL(bash_cv_func_sigsetjmp,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>

int
main()
{
#if !defined (_POSIX_VERSION) || !defined (HAVE_POSIX_SIGNALS)
exit (1);
#else

int code;
sigset_t set, oset, nset;
sigjmp_buf xx;

/* get the mask */
sigemptyset(&set);
sigemptyset(&oset);

sigprocmask(SIG_BLOCK, (sigset_t *)NULL, &oset);
/* paranoia -- make sure SIGINT is not blocked */
sigdelset (&oset, SIGINT);
sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);

/* save it */
code = sigsetjmp(xx, 1);
if (code)
{
  sigprocmask(SIG_BLOCK, (sigset_t *)NULL, &nset);
  /* could compare nset to oset here, but we just look for SIGINT */
  if (sigismember (&nset, SIGINT))
    exit(1);
  exit(0);
}

/* change it so that SIGINT is blocked */
sigaddset(&set, SIGINT);
sigprocmask(SIG_BLOCK, &set, (sigset_t *)NULL);

/* and siglongjmp */
siglongjmp(xx, 10);
exit(1);
#endif
}
]])], [bash_cv_func_sigsetjmp=present], [bash_cv_func_sigsetjmp=missing],
    [AC_MSG_WARN(cannot check for sigsetjmp/siglongjmp if cross-compiling -- defaulting to $bash_cv_posix_signals)
     if test "$bash_cv_posix_signals" = "yes" ; then
	bash_cv_func_sigsetjmp=present
     else
	bash_cv_func_sigsetjmp=missing
     fi]
)])
AC_MSG_RESULT($bash_cv_func_sigsetjmp)
if test $bash_cv_func_sigsetjmp = present; then
AC_DEFINE(HAVE_POSIX_SIGSETJMP)
fi
])

AC_DEFUN(BASH_FUNC_STRCOLL,
[AC_MSG_CHECKING(whether or not strcoll and strcmp differ)
AC_CACHE_VAL(bash_cv_func_strcoll_broken,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#if defined (HAVE_LOCALE_H)
#include <locale.h>
#endif
#include <string.h>
#include <stdlib.h>

int
main(c, v)
int     c;
char    *v[];
{
        int     r1, r2;
        char    *deflocale, *defcoll;

#ifdef HAVE_SETLOCALE
        deflocale = setlocale(LC_ALL, "");
	defcoll = setlocale(LC_COLLATE, "");
#endif

#ifdef HAVE_STRCOLL
	/* These two values are taken from tests/glob-test. */
        r1 = strcoll("abd", "aXd");
#else
	r1 = 0;
#endif
        r2 = strcmp("abd", "aXd");

	/* These two should both be greater than 0.  It is permissible for
	   a system to return different values, as long as the sign is the
	   same. */

        /* Exit with 1 (failure) if these two values are both > 0, since
	   this tests whether strcoll(3) is broken with respect to strcmp(3)
	   in the default locale. */
	exit (r1 > 0 && r2 > 0);
}
]])], [bash_cv_func_strcoll_broken=yes], [bash_cv_func_strcoll_broken=no],
   [AC_MSG_WARN(cannot check strcoll if cross compiling -- defaulting to no)
    bash_cv_func_strcoll_broken=no]
)])
AC_MSG_RESULT($bash_cv_func_strcoll_broken)
if test $bash_cv_func_strcoll_broken = yes; then
AC_DEFINE(STRCOLL_BROKEN)
fi
])

AC_DEFUN(BASH_FUNC_PRINTF_A_FORMAT,
[AC_MSG_CHECKING([for printf floating point output in hex notation])
AC_CACHE_VAL(bash_cv_printf_a_format,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
main()
{
	double y = 0.0;
	char abuf[1024];

	sprintf(abuf, "%A", y);
	exit(strchr(abuf, 'P') == (char *)0);
}
]])], [bash_cv_printf_a_format=yes], [bash_cv_printf_a_format=no],
   [AC_MSG_WARN(cannot check printf if cross compiling -- defaulting to no)
    bash_cv_printf_a_format=no]
)])
AC_MSG_RESULT($bash_cv_printf_a_format)
if test $bash_cv_printf_a_format = yes; then
AC_DEFINE(HAVE_PRINTF_A_FORMAT)
fi
])

AC_DEFUN(BASH_STRUCT_TERMIOS_LDISC,
[
AC_CHECK_MEMBER(struct termios.c_line, AC_DEFINE(TERMIOS_LDISC), ,[
#include <sys/types.h>
#include <termios.h>
])
])

AC_DEFUN(BASH_STRUCT_TERMIO_LDISC,
[
AC_CHECK_MEMBER(struct termio.c_line, AC_DEFINE(TERMIO_LDISC), ,[
#include <sys/types.h>
#include <termio.h>
])
])

dnl
dnl Like AC_STRUCT_ST_BLOCKS, but doesn't muck with LIBOBJS
dnl
dnl sets bash_cv_struct_stat_st_blocks
dnl
dnl unused for now; we'll see how AC_CHECK_MEMBERS works
dnl
AC_DEFUN(BASH_STRUCT_ST_BLOCKS,
[
AC_MSG_CHECKING([for struct stat.st_blocks])
AC_CACHE_VAL(bash_cv_struct_stat_st_blocks,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/stat.h>
]], [[
int
main()
{
static struct stat a;
if (a.st_blocks) return 0;
return 0;
}
]])], [bash_cv_struct_stat_st_blocks=yes], [bash_cv_struct_stat_st_blocks=no])
])
AC_MSG_RESULT($bash_cv_struct_stat_st_blocks)
if test "$bash_cv_struct_stat_st_blocks" = "yes"; then
AC_DEFINE(HAVE_STRUCT_STAT_ST_BLOCKS)
fi
])

AC_DEFUN([BASH_CHECK_LIB_TERMCAP],
[
if test "X$bash_cv_termcap_lib" = "X"; then
_bash_needmsg=yes
else
AC_MSG_CHECKING(which library has the termcap functions)
_bash_needmsg=
fi
AC_CACHE_VAL(bash_cv_termcap_lib,
[AC_CHECK_FUNC(tgetent, bash_cv_termcap_lib=libc,
  [AC_CHECK_LIB(termcap, tgetent, bash_cv_termcap_lib=libtermcap,
    [AC_CHECK_LIB(tinfo, tgetent, bash_cv_termcap_lib=libtinfo,
        [AC_CHECK_LIB(curses, tgetent, bash_cv_termcap_lib=libcurses,
	    [AC_CHECK_LIB(ncurses, tgetent, bash_cv_termcap_lib=libncurses,
                [AC_CHECK_LIB(ncursesw, tgetent, bash_cv_termcap_lib=libncursesw,
	            bash_cv_termcap_lib=gnutermcap)])])])])])])
if test "X$_bash_needmsg" = "Xyes"; then
AC_MSG_CHECKING(which library has the termcap functions)
fi
AC_MSG_RESULT(using $bash_cv_termcap_lib)
if test $bash_cv_termcap_lib = gnutermcap && test -z "$prefer_curses"; then
LDFLAGS="$LDFLAGS -L./lib/termcap"
TERMCAP_LIB="./lib/termcap/libtermcap.a"
TERMCAP_DEP="./lib/termcap/libtermcap.a"
elif test $bash_cv_termcap_lib = libtermcap && test -z "$prefer_curses"; then
TERMCAP_LIB=-ltermcap
TERMCAP_DEP=
elif test $bash_cv_termcap_lib = libtinfo; then
TERMCAP_LIB=-ltinfo
TERMCAP_DEP=
elif test $bash_cv_termcap_lib = libncurses; then
TERMCAP_LIB=-lncurses
TERMCAP_DEP=
elif test $bash_cv_termcap_lib = libc; then
TERMCAP_LIB=
TERMCAP_DEP=
else
# we assume ncurses is installed somewhere the linker can find it
TERMCAP_LIB=-lncurses
TERMCAP_DEP=
fi
])

dnl
dnl Check for the presence of getpeername in libsocket.
dnl If libsocket is present, check for libnsl and add it to LIBS if
dnl it's there, since most systems with libsocket require linking
dnl with libnsl as well.  This should only be called if getpeername
dnl was not found in libc.
dnl
dnl NOTE: IF WE FIND GETPEERNAME, WE ASSUME THAT WE HAVE BIND/CONNECT
dnl	  AS WELL
dnl
AC_DEFUN(BASH_CHECK_LIB_SOCKET,
[
if test "X$bash_cv_have_socklib" = "X"; then
_bash_needmsg=
else
AC_MSG_CHECKING(for socket library)
_bash_needmsg=yes
fi
AC_CACHE_VAL(bash_cv_have_socklib,
[AC_CHECK_LIB(socket, getpeername,
        bash_cv_have_socklib=yes, bash_cv_have_socklib=no, -lnsl)])
if test "X$_bash_needmsg" = Xyes; then
  AC_MSG_RESULT($bash_cv_have_socklib)
  _bash_needmsg=
fi
if test $bash_cv_have_socklib = yes; then
  # check for libnsl, add it to LIBS if present
  if test "X$bash_cv_have_libnsl" = "X"; then
    _bash_needmsg=
  else
    AC_MSG_CHECKING(for libnsl)
    _bash_needmsg=yes
  fi
  AC_CACHE_VAL(bash_cv_have_libnsl,
	   [AC_CHECK_LIB(nsl, t_open,
		 bash_cv_have_libnsl=yes, bash_cv_have_libnsl=no)])
  if test "X$_bash_needmsg" = Xyes; then
    AC_MSG_RESULT($bash_cv_have_libnsl)
    _bash_needmsg=
  fi
  if test $bash_cv_have_libnsl = yes; then
    LIBS="-lsocket -lnsl $LIBS"
  else
    LIBS="-lsocket $LIBS"
  fi
  AC_DEFINE(HAVE_LIBSOCKET)
  AC_DEFINE(HAVE_GETPEERNAME)
fi
])

dnl like _AC_STRUCT_DIRENT(MEMBER) but public
AC_DEFUN(BASH_STRUCT_DIRENT,
[
AC_REQUIRE([AC_HEADER_DIRENT])
AC_CHECK_MEMBERS(struct dirent.$1, bash_cv_dirent_has_$1=yes, bash_cv_dirent_has_$1=no,
[[
#include <stdio.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if defined(HAVE_DIRENT_H)
# include <dirent.h>
#else
# define dirent direct
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif /* SYSNDIR */
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif /* SYSDIR */
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif /* HAVE_DIRENT_H */
]])
])

AC_DEFUN(BASH_STRUCT_DIRENT_D_INO,
[AC_REQUIRE([AC_HEADER_DIRENT])
AC_MSG_CHECKING(for struct dirent.d_ino)
AC_CACHE_VAL(bash_cv_dirent_has_d_ino, [BASH_STRUCT_DIRENT([d_ino])])
AC_MSG_RESULT($bash_cv_dirent_has_d_ino)
if test $bash_cv_dirent_has_d_ino = yes; then
AC_DEFINE(HAVE_STRUCT_DIRENT_D_INO)
fi
])

AC_DEFUN(BASH_STRUCT_DIRENT_D_FILENO,
[AC_REQUIRE([AC_HEADER_DIRENT])
AC_MSG_CHECKING(for struct dirent.d_fileno)
AC_CACHE_VAL(bash_cv_dirent_has_d_fileno, [BASH_STRUCT_DIRENT([d_fileno])])
AC_MSG_RESULT($bash_cv_dirent_has_d_fileno)
if test $bash_cv_dirent_has_d_fileno = yes; then
AC_DEFINE(HAVE_STRUCT_DIRENT_D_FILENO)
fi
])

AC_DEFUN(BASH_STRUCT_DIRENT_D_NAMLEN,
[AC_REQUIRE([AC_HEADER_DIRENT])
AC_MSG_CHECKING(for struct dirent.d_namlen)
AC_CACHE_VAL(bash_cv_dirent_has_d_namlen, [BASH_STRUCT_DIRENT([d_namlen])])
AC_MSG_RESULT($bash_cv_dirent_has_d_namlen)
if test $bash_cv_dirent_has_d_namlen = yes; then
AC_DEFINE(HAVE_STRUCT_DIRENT_D_NAMLEN)
fi
])

AC_DEFUN(BASH_STRUCT_TIMEVAL,
[AC_MSG_CHECKING(for struct timeval in sys/time.h and time.h)
AC_CACHE_VAL(bash_cv_struct_timeval,
[AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#if HAVE_SYS_TIME_H
		  #include <sys/time.h>
		  #endif
		  #include <time.h>
		]],
		[[static struct timeval x; x.tv_sec = x.tv_usec;]]
	)],
	bash_cv_struct_timeval=yes,
	bash_cv_struct_timeval=no)
])
AC_MSG_RESULT($bash_cv_struct_timeval)
if test $bash_cv_struct_timeval = yes; then
  AC_DEFINE(HAVE_TIMEVAL)
fi
])

AC_DEFUN(BASH_STRUCT_TIMEZONE,
[AC_MSG_CHECKING(for struct timezone in sys/time.h and time.h)
AC_CACHE_VAL(bash_cv_struct_timezone,
[
AC_EGREP_HEADER(struct timezone, sys/time.h,
		bash_cv_struct_timezone=yes,
		AC_EGREP_HEADER(struct timezone, time.h,
			bash_cv_struct_timezone=yes,
			bash_cv_struct_timezone=no))
])
AC_MSG_RESULT($bash_cv_struct_timezone)
if test $bash_cv_struct_timezone = yes; then
  AC_DEFINE(HAVE_STRUCT_TIMEZONE)
fi
])

AC_DEFUN(BASH_CHECK_WINSIZE_IOCTL,
[AC_CACHE_VAL(bash_cv_struct_winsize_ioctl,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/ioctl.h>
]],
[[
struct winsize x;
if (sizeof (x) > 0) return (0);
]] )], [bash_cv_struct_winsize_ioctl=yes], [bash_cv_struct_winsize_ioctl=no])
])
])

AC_DEFUN(BASH_CHECK_WINSIZE_TERMIOS,
[AC_CACHE_VAL(bash_cv_struct_winsize_termios,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/termios.h>
]],
[[
struct winsize x;
if (sizeof (x) > 0) return (0);
]] )], [bash_cv_struct_winsize_termios=yes], [bash_cv_struct_winsize_termios=no])
])
])

AC_DEFUN(BASH_STRUCT_WINSIZE,
[AC_MSG_CHECKING(for struct winsize in sys/ioctl.h and termios.h)
AC_CACHE_VAL(bash_cv_struct_winsize_header,
[
BASH_CHECK_WINSIZE_IOCTL
BASH_CHECK_WINSIZE_TERMIOS

if test $bash_cv_struct_winsize_ioctl = yes; then
  bash_cv_struct_winsize_header=ioctl_h
elif test $bash_cv_struct_winsize_termios = yes; then
  bash_cv_struct_winsize_header=termios_h
else
  bash_cv_struct_winsize_header=other
fi
])
if test $bash_cv_struct_winsize_header = ioctl_h; then
  AC_MSG_RESULT(sys/ioctl.h)
  AC_DEFINE(STRUCT_WINSIZE_IN_SYS_IOCTL)
elif test $bash_cv_struct_winsize_header = termios_h; then
  AC_MSG_RESULT(termios.h)
  AC_DEFINE(STRUCT_WINSIZE_IN_TERMIOS)
else
  AC_MSG_RESULT(not found)
fi
])

AC_DEFUN(BASH_HAVE_POSIX_SIGNALS,
[AC_CACHE_VAL(bash_cv_posix_signals,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <signal.h>
]], [[
    sigset_t ss;
    struct sigaction sa;
    sigemptyset(&ss); sigsuspend(&ss);
    sigaction(SIGINT, &sa, (struct sigaction *) 0);
    sigprocmask(SIG_BLOCK, &ss, (sigset_t *) 0);
]] )],
[bash_cv_posix_signals=yes], [bash_cv_posix_signals=no]
)])
])

AC_DEFUN(BASH_HAVE_BSD_SIGNALS,
[AC_CACHE_VAL(bash_cv_bsd_signals,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <signal.h>
]], [[
int mask = sigmask(SIGINT);
sigsetmask(mask); sigblock(mask); sigpause(mask);
]] )],
[bash_cv_bsd_signals=yes], [bash_cv_bsd_signals=no]
)])
])

AC_DEFUN(BASH_HAVE_SYSV_SIGNALS,
[AC_CACHE_VAL(bash_cv_sysv_signals,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <signal.h>
void foo() { }
]], [[
int mask = sigmask(SIGINT);
sigset(SIGINT, foo); sigrelse(SIGINT);
sighold(SIGINT); sigpause(SIGINT);
]] )],
[bash_cv_sysv_signals=yes], [bash_cv_sysv_signals=no]
)])
])

dnl Check type of signal routines (posix, 4.2bsd, 4.1bsd or v7)
AC_DEFUN(BASH_SYS_SIGNAL_VINTAGE,
[AC_MSG_CHECKING(for type of signal functions)
AC_CACHE_VAL(bash_cv_signal_vintage,
[
BASH_HAVE_POSIX_SIGNALS
if test $bash_cv_posix_signals = yes; then
  bash_cv_signal_vintage=posix
else
  BASH_HAVE_BSD_SIGNALS
  if test $bash_cv_bsd_signals = yes; then
    bash_cv_signal_vintage=4.2bsd
  else
    BASH_HAVE_SYSV_SIGNALS
    if test $bash_cv_sysv_signals = yes; then
      bash_cv_signal_vintage=svr3
    else
      bash_cv_signal_vintage=v7
    fi
  fi
fi
])
AC_MSG_RESULT($bash_cv_signal_vintage)
if test "$bash_cv_signal_vintage" = posix; then
AC_DEFINE(HAVE_POSIX_SIGNALS)
elif test "$bash_cv_signal_vintage" = "4.2bsd"; then
AC_DEFINE(HAVE_BSD_SIGNALS)
elif test "$bash_cv_signal_vintage" = svr3; then
AC_DEFINE(HAVE_USG_SIGHOLD)
fi
])

dnl Check if the pgrp of setpgrp() can't be the pid of a zombie process.
AC_DEFUN(BASH_SYS_PGRP_SYNC,
[AC_REQUIRE([AC_FUNC_GETPGRP])
AC_MSG_CHECKING(whether pgrps need synchronization)
AC_CACHE_VAL(bash_cv_pgrp_pipe,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif
#include <stdlib.h>
int
main()
{
# ifdef GETPGRP_VOID
#  define getpgID()	getpgrp()
# else
#  define getpgID()	getpgrp(0)
#  define setpgid(x,y)	setpgrp(x,y)
# endif
	int pid1, pid2, fds[2];
	int status;
	char ok;

	switch (pid1 = fork()) {
	  case -1:
	    exit(1);
	  case 0:
	    setpgid(0, getpid());
	    exit(0);
	}
	setpgid(pid1, pid1);

	sleep(2);	/* let first child die */

	if (pipe(fds) < 0)
	  exit(2);

	switch (pid2 = fork()) {
	  case -1:
	    exit(3);
	  case 0:
	    setpgid(0, pid1);
	    ok = getpgID() == pid1;
	    write(fds[1], &ok, 1);
	    exit(0);
	}
	setpgid(pid2, pid1);

	close(fds[1]);
	if (read(fds[0], &ok, 1) != 1)
	  exit(4);
	wait(&status);
	wait(&status);
	exit(ok ? 0 : 5);
}
]])], [bash_cv_pgrp_pipe=no], [bash_cv_pgrp_pipe=yes],
   [AC_MSG_WARN(cannot check pgrp synchronization if cross compiling -- defaulting to no)
    bash_cv_pgrp_pipe=no]
)])
AC_MSG_RESULT($bash_cv_pgrp_pipe)
if test $bash_cv_pgrp_pipe = yes; then
AC_DEFINE(PGRP_PIPE)
fi
])

AC_DEFUN(BASH_SYS_REINSTALL_SIGHANDLERS,
[AC_REQUIRE([BASH_SYS_SIGNAL_VINTAGE])
AC_MSG_CHECKING([if signal handlers must be reinstalled when invoked])
AC_CACHE_VAL(bash_cv_must_reinstall_sighandlers,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>

typedef void sigfunc();

volatile int nsigint;

#ifdef HAVE_POSIX_SIGNALS
sigfunc *
set_signal_handler(sig, handler)
     int sig;
     sigfunc *handler;
{
  struct sigaction act, oact;
  act.sa_handler = handler;
  act.sa_flags = 0;
  sigemptyset (&act.sa_mask);
  sigemptyset (&oact.sa_mask);
  sigaction (sig, &act, &oact);
  return (oact.sa_handler);
}
#else
#define set_signal_handler(s, h) signal(s, h)
#endif

void
sigint(s)
int s;
{
  nsigint++;
}

int
main()
{
	nsigint = 0;
	set_signal_handler(SIGINT, sigint);
	kill((int)getpid(), SIGINT);
	kill((int)getpid(), SIGINT);
	exit(nsigint != 2);
}
]])], [bash_cv_must_reinstall_sighandlers=no], [bash_cv_must_reinstall_sighandlers=yes],
   [AC_MSG_WARN(cannot check signal handling if cross compiling -- defaulting to no)
    bash_cv_must_reinstall_sighandlers=no]
)])
AC_MSG_RESULT($bash_cv_must_reinstall_sighandlers)
if test $bash_cv_must_reinstall_sighandlers = yes; then
AC_DEFINE(MUST_REINSTALL_SIGHANDLERS)
fi
])

dnl check that some necessary job control definitions are present
AC_DEFUN(BASH_SYS_JOB_CONTROL_MISSING,
[AC_REQUIRE([BASH_SYS_SIGNAL_VINTAGE])
AC_MSG_CHECKING(for presence of necessary job control definitions)
AC_CACHE_VAL(bash_cv_job_control_missing,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <signal.h>

/* add more tests in here as appropriate */

/* signal type */
#if !defined (HAVE_POSIX_SIGNALS) && !defined (HAVE_BSD_SIGNALS)
#error
#endif

/* signals and tty control. */
#if !defined (SIGTSTP) || !defined (SIGSTOP) || !defined (SIGCONT)
#error
#endif

/* process control */
#if !defined (WNOHANG) || !defined (WUNTRACED) 
#error
#endif

/* Posix systems have tcgetpgrp and waitpid. */
#if defined (_POSIX_VERSION) && !defined (HAVE_TCGETPGRP)
#error
#endif

#if defined (_POSIX_VERSION) && !defined (HAVE_WAITPID)
#error
#endif

/* Other systems have TIOCSPGRP/TIOCGPRGP and wait3. */
#if !defined (_POSIX_VERSION) && !defined (HAVE_WAIT3)
#error
#endif

]], [[ int x; ]] )],
[bash_cv_job_control_missing=present], [bash_cv_job_control_missing=missing]
)])
AC_MSG_RESULT($bash_cv_job_control_missing)
if test $bash_cv_job_control_missing = missing; then
AC_DEFINE(JOB_CONTROL_MISSING)
fi
])

dnl check whether named pipes are present
dnl this requires a previous check for mkfifo, but that is awkward to specify
AC_DEFUN(BASH_SYS_NAMED_PIPES,
[AC_MSG_CHECKING(for presence of named pipes)
AC_CACHE_VAL(bash_cv_sys_named_pipes,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Add more tests in here as appropriate. */
int
main()
{
int fd, err;

#if defined (HAVE_MKFIFO)
exit (0);
#endif

#if !defined (S_IFIFO) && (defined (_POSIX_VERSION) && !defined (S_ISFIFO))
exit (1);
#endif

#if defined (NeXT)
exit (1);
#endif
err = mkdir("bash-aclocal", 0700);
if (err < 0) {
  perror ("mkdir");
  exit(1);
}
fd = mknod ("bash-aclocal/sh-np-autoconf", 0666 | S_IFIFO, 0);
if (fd == -1) {
  rmdir ("bash-aclocal");
  exit (1);
}
close(fd);
unlink ("bash-aclocal/sh-np-autoconf");
rmdir ("bash-aclocal");
exit(0);
}
]])], [bash_cv_sys_named_pipes=present], [bash_cv_sys_named_pipes=missing],
    [AC_MSG_WARN(cannot check for named pipes if cross-compiling -- defaulting to missing)
     bash_cv_sys_named_pipes=missing]
)])
AC_MSG_RESULT($bash_cv_sys_named_pipes)
if test $bash_cv_sys_named_pipes = missing; then
AC_DEFINE(NAMED_PIPES_MISSING)
fi
])

AC_DEFUN(BASH_SYS_DEFAULT_MAIL_DIR,
[AC_MSG_CHECKING(for default mail directory)
AC_CACHE_VAL(bash_cv_mail_dir,
[if test -d /var/mail; then
   bash_cv_mail_dir=/var/mail
 elif test -d /var/spool/mail; then
   bash_cv_mail_dir=/var/spool/mail
 elif test -d /usr/mail; then
   bash_cv_mail_dir=/usr/mail
 elif test -d /usr/spool/mail; then
   bash_cv_mail_dir=/usr/spool/mail
 else
   bash_cv_mail_dir=unknown
 fi
])
AC_MSG_RESULT($bash_cv_mail_dir)
AC_DEFINE_UNQUOTED(DEFAULT_MAIL_DIRECTORY, "$bash_cv_mail_dir")
])

AC_DEFUN(BASH_HAVE_TIOCGWINSZ,
[AC_MSG_CHECKING(for TIOCGWINSZ in sys/ioctl.h)
AC_CACHE_VAL(bash_cv_tiocgwinsz_in_ioctl,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/ioctl.h>]], [[int x = TIOCGWINSZ;]] )],
  [bash_cv_tiocgwinsz_in_ioctl=yes], [bash_cv_tiocgwinsz_in_ioctl=no]
)])
AC_MSG_RESULT($bash_cv_tiocgwinsz_in_ioctl)
if test $bash_cv_tiocgwinsz_in_ioctl = yes; then   
AC_DEFINE(GWINSZ_IN_SYS_IOCTL)
fi
])

AC_DEFUN(BASH_HAVE_TIOCSTAT,
[AC_MSG_CHECKING(for TIOCSTAT in sys/ioctl.h)
AC_CACHE_VAL(bash_cv_tiocstat_in_ioctl,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/ioctl.h>]], [[int x = TIOCSTAT;]] )],
  [bash_cv_tiocstat_in_ioctl=yes], [bash_cv_tiocstat_in_ioctl=no]
)])
AC_MSG_RESULT($bash_cv_tiocstat_in_ioctl)
if test $bash_cv_tiocstat_in_ioctl = yes; then   
AC_DEFINE(TIOCSTAT_IN_SYS_IOCTL)
fi
])

AC_DEFUN(BASH_HAVE_FIONREAD,
[AC_MSG_CHECKING(for FIONREAD in sys/ioctl.h)
AC_CACHE_VAL(bash_cv_fionread_in_ioctl,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/ioctl.h>]], [[int x = FIONREAD;]] )],
  [bash_cv_fionread_in_ioctl=yes], [bash_cv_fionread_in_ioctl=no]
)])
AC_MSG_RESULT($bash_cv_fionread_in_ioctl)
if test $bash_cv_fionread_in_ioctl = yes; then   
AC_DEFINE(FIONREAD_IN_SYS_IOCTL)
fi
])

dnl
dnl See if speed_t is declared in <sys/types.h>.  Some versions of linux
dnl require a definition of speed_t each time <termcap.h> is included,
dnl but you can only get speed_t if you include <termios.h> (on some
dnl versions) or <sys/types.h> (on others).
dnl
AC_DEFUN(BASH_CHECK_SPEED_T,
[AC_MSG_CHECKING(for speed_t in sys/types.h)
AC_CACHE_VAL(bash_cv_speed_t_in_sys_types,
[AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <sys/types.h>]],
		[[speed_t x;]])],
	[bash_cv_speed_t_in_sys_types=yes],[bash_cv_speed_t_in_sys_types=no])])
AC_MSG_RESULT($bash_cv_speed_t_in_sys_types)
if test $bash_cv_speed_t_in_sys_types = yes; then   
AC_DEFINE(SPEED_T_IN_SYS_TYPES)
fi
])

AC_DEFUN(BASH_CHECK_GETPW_FUNCS,
[AC_MSG_CHECKING(whether getpw functions are declared in pwd.h)
AC_CACHE_VAL(bash_cv_getpw_declared,
[AC_EGREP_CPP(getpwuid,
[
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <pwd.h>
],
bash_cv_getpw_declared=yes,bash_cv_getpw_declared=no)])
AC_MSG_RESULT($bash_cv_getpw_declared)
if test $bash_cv_getpw_declared = yes; then
AC_DEFINE(HAVE_GETPW_DECLS)
fi
])

AC_DEFUN(BASH_CHECK_DEV_FD,
[AC_MSG_CHECKING(whether /dev/fd is available)
AC_CACHE_VAL(bash_cv_dev_fd,
[bash_cv_dev_fd=""
if test -d /dev/fd  && (exec test -r /dev/fd/0 < /dev/null) ; then
# check for systems like FreeBSD 5 that only provide /dev/fd/[012]
   if (exec test -r /dev/fd/3 3</dev/null) ; then
     bash_cv_dev_fd=standard
   else
     bash_cv_dev_fd=absent
   fi
fi
if test -z "$bash_cv_dev_fd" ; then 
  if test -d /proc/self/fd && (exec test -r /proc/self/fd/0 < /dev/null) ; then
    bash_cv_dev_fd=whacky
  else
    bash_cv_dev_fd=absent
  fi
fi
])
AC_MSG_RESULT($bash_cv_dev_fd)
if test $bash_cv_dev_fd = "standard"; then
  AC_DEFINE(HAVE_DEV_FD)
  AC_DEFINE(DEV_FD_PREFIX, "/dev/fd/")
elif test $bash_cv_dev_fd = "whacky"; then
  AC_DEFINE(HAVE_DEV_FD)
  AC_DEFINE(DEV_FD_PREFIX, "/proc/self/fd/")
fi
])

AC_DEFUN(BASH_CHECK_DEV_STDIN,
[AC_MSG_CHECKING(whether /dev/stdin stdout stderr are available)
AC_CACHE_VAL(bash_cv_dev_stdin,
[if (exec test -r /dev/stdin < /dev/null) ; then
   bash_cv_dev_stdin=present
 else
   bash_cv_dev_stdin=absent
 fi
])
AC_MSG_RESULT($bash_cv_dev_stdin)
if test $bash_cv_dev_stdin = "present"; then
  AC_DEFINE(HAVE_DEV_STDIN)
fi
])


AC_DEFUN(BASH_CHECK_RLIMIT,
[AC_CACHE_VAL(bash_cv_rlimit,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/resource.h>
]],
[[
  int f;
  f = RLIMIT_DATA;
]] )],
[bash_cv_rlimit=yes], [bash_cv_rlimit=no]
)])
])

dnl
dnl Check if HPUX needs _KERNEL defined for RLIMIT_* definitions
dnl
AC_DEFUN(BASH_CHECK_KERNEL_RLIMIT,
[AC_MSG_CHECKING([whether $host_os needs _KERNEL for RLIMIT defines])
AC_CACHE_VAL(bash_cv_kernel_rlimit,
[BASH_CHECK_RLIMIT
if test $bash_cv_rlimit = no; then
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#define _KERNEL
#include <sys/resource.h>
#undef _KERNEL
]],
[[
  int f;
  f = RLIMIT_DATA;
]] )], [bash_cv_kernel_rlimit=yes], [bash_cv_kernel_rlimit=no] )
fi
])
AC_MSG_RESULT($bash_cv_kernel_rlimit)
if test $bash_cv_kernel_rlimit = yes; then
AC_DEFINE(RLIMIT_NEEDS_KERNEL)
fi
])

dnl
dnl Check for 64-bit off_t -- used for malloc alignment
dnl
dnl C does not allow duplicate case labels, so the compile will fail if
dnl sizeof(off_t) is > 4.
dnl
AC_DEFUN(BASH_CHECK_OFF_T_64,
[AC_CACHE_CHECK(for 64-bit off_t, bash_cv_off_t_64,
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
]],[[
switch (0) case 0: case (sizeof (off_t) <= 4):;
]] )], [bash_cv_off_t_64=no], [bash_cv_off_t_64=yes]
))
if test $bash_cv_off_t_64 = yes; then
        AC_DEFINE(HAVE_OFF_T_64)
fi])

AC_DEFUN(BASH_CHECK_RTSIGS,
[AC_MSG_CHECKING(for unusable real-time signals due to large values)
AC_CACHE_VAL(bash_cv_unusable_rtsigs,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

#ifndef NSIG
#  define NSIG 64
#endif

int
main ()
{
  int n_sigs = 2 * NSIG;
#ifdef SIGRTMIN
  int rtmin = SIGRTMIN;
#else
  int rtmin = 0;
#endif

  exit(rtmin < n_sigs);
}
]])], [bash_cv_unusable_rtsigs=yes], [bash_cv_unusable_rtsigs=no],
    [AC_MSG_WARN(cannot check real-time signals if cross compiling -- defaulting to yes)
     bash_cv_unusable_rtsigs=yes]
)])
AC_MSG_RESULT($bash_cv_unusable_rtsigs)
if test $bash_cv_unusable_rtsigs = yes; then
AC_DEFINE(UNUSABLE_RT_SIGNALS)
fi
])

dnl
dnl check for availability of multibyte characters and functions
dnl
dnl geez, I wish I didn't have to check for all of this stuff separately
dnl
AC_DEFUN(BASH_CHECK_MULTIBYTE,
[
AC_CHECK_HEADERS(wctype.h)
AC_CHECK_HEADERS(wchar.h)
AC_CHECK_HEADERS(langinfo.h)

AC_CHECK_HEADERS(mbstr.h)

AC_CHECK_FUNC(mbrlen, AC_DEFINE(HAVE_MBRLEN))
AC_CHECK_FUNC(mbscasecmp, AC_DEFINE(HAVE_MBSCMP))
AC_CHECK_FUNC(mbscmp, AC_DEFINE(HAVE_MBSCMP))
AC_CHECK_FUNC(mbsnrtowcs, AC_DEFINE(HAVE_MBSNRTOWCS))
AC_CHECK_FUNC(mbsrtowcs, AC_DEFINE(HAVE_MBSRTOWCS))

AC_REPLACE_FUNCS(mbschr)

AC_CHECK_FUNC(wcrtomb, AC_DEFINE(HAVE_WCRTOMB))
AC_CHECK_FUNC(wcscoll, AC_DEFINE(HAVE_WCSCOLL))
AC_CHECK_FUNC(wcsdup, AC_DEFINE(HAVE_WCSDUP))
AC_CHECK_FUNC(wcwidth, AC_DEFINE(HAVE_WCWIDTH))
AC_CHECK_FUNC(wctype, AC_DEFINE(HAVE_WCTYPE))

AC_REPLACE_FUNCS(wcswidth)

dnl checks for both mbrtowc and mbstate_t
AC_FUNC_MBRTOWC
if test $ac_cv_func_mbrtowc = yes; then
	AC_DEFINE(HAVE_MBSTATE_T)
fi

AC_CHECK_FUNCS(iswlower iswupper towlower towupper iswctype)

AC_REQUIRE([AM_LANGINFO_CODESET])

dnl check for wchar_t in <wchar.h>
AC_CACHE_CHECK([for wchar_t in wchar.h], bash_cv_type_wchar_t,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
[#include <wchar.h>]],
[[
        wchar_t foo;
        foo = 0;
]] )], [bash_cv_type_wchar_t=yes], [bash_cv_type_wchar_t=no]
)])
if test $bash_cv_type_wchar_t = yes; then
        AC_DEFINE(HAVE_WCHAR_T, 1, [systems should define this type here])
fi

dnl check for wctype_t in <wctype.h>
AC_CACHE_CHECK([for wctype_t in wctype.h], bash_cv_type_wctype_t,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
[#include <wctype.h>]],
[[
        wctype_t foo;
        foo = 0;
]] )], [bash_cv_type_wctype_t=yes], [bash_cv_type_wctype_t=no]
)])
if test $bash_cv_type_wctype_t = yes; then
        AC_DEFINE(HAVE_WCTYPE_T, 1, [systems should define this type here])
fi

dnl check for wint_t in <wctype.h>
AC_CACHE_CHECK([for wint_t in wctype.h], bash_cv_type_wint_t,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
[#include <wctype.h>]],
[[
        wint_t foo;
        foo = 0;
]] )], [bash_cv_type_wint_t=yes], [bash_cv_type_wint_t=no]
)])
if test $bash_cv_type_wint_t = yes; then
        AC_DEFINE(HAVE_WINT_T, 1, [systems should define this type here])
fi

dnl check for broken wcwidth
AC_CACHE_CHECK([for wcwidth broken with unicode combining characters],
bash_cv_wcwidth_broken,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <locale.h>
#include <wchar.h>

int
main(c, v)
int     c;
char    **v;
{
        int     w;

        setlocale(LC_ALL, "en_US.UTF-8");
        w = wcwidth (0x0301);
        exit (w == 0);  /* exit 0 if wcwidth broken */
}
]])], [bash_cv_wcwidth_broken=yes], [bash_cv_wcwidth_broken=no],
      [bash_cv_wcwidth_broken=no]
)])
if test "$bash_cv_wcwidth_broken" = yes; then
        AC_DEFINE(WCWIDTH_BROKEN, 1, [wcwidth is usually not broken])
fi

if test "$am_cv_func_iconv" = yes; then
	OLDLIBS="$LIBS"
	LIBS="$LIBS $LIBINTL $LIBICONV"
	AC_CHECK_FUNCS(locale_charset)
	LIBS="$OLDLIBS"
fi

AC_CHECK_SIZEOF(wchar_t, 4)

])

dnl need: prefix exec_prefix libdir includedir CC TERMCAP_LIB
dnl require:
dnl	AC_PROG_CC
dnl	BASH_CHECK_LIB_TERMCAP

AC_DEFUN([RL_LIB_READLINE_VERSION],
[
AC_REQUIRE([BASH_CHECK_LIB_TERMCAP])

AC_MSG_CHECKING([version of installed readline library])

# What a pain in the ass this is.

# save cpp and ld options
_save_CFLAGS="$CFLAGS"
_save_LDFLAGS="$LDFLAGS"
_save_LIBS="$LIBS"

# Don't set ac_cv_rl_prefix if the caller has already assigned a value.  This
# allows the caller to do something like $_rl_prefix=$withval if the user
# specifies --with-installed-readline=PREFIX as an argument to configure

if test -z "$ac_cv_rl_prefix"; then
test "x$prefix" = xNONE && ac_cv_rl_prefix=$ac_default_prefix || ac_cv_rl_prefix=${prefix}
fi

eval ac_cv_rl_includedir=${ac_cv_rl_prefix}/include
eval ac_cv_rl_libdir=${ac_cv_rl_prefix}/lib

LIBS="$LIBS -lreadline ${TERMCAP_LIB}"
CFLAGS="$CFLAGS -I${ac_cv_rl_includedir}"
LDFLAGS="$LDFLAGS -L${ac_cv_rl_libdir}"

AC_CACHE_VAL(ac_cv_rl_version,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <readline/readline.h>
#include <stdlib.h>

extern int rl_gnu_readline_p;

int
main()
{
	FILE *fp;
	fp = fopen("conftest.rlv", "w");
	if (fp == 0)
		exit(1);
	if (rl_gnu_readline_p != 1)
		fprintf(fp, "0.0\n");
	else
		fprintf(fp, "%s\n", rl_library_version ? rl_library_version : "0.0");
	fclose(fp);
	exit(0);
}
]])],
[ac_cv_rl_version=`cat conftest.rlv`],
[ac_cv_rl_version='0.0'],
[ac_cv_rl_version='8.0']
)])

CFLAGS="$_save_CFLAGS"
LDFLAGS="$_save_LDFLAGS"
LIBS="$_save_LIBS"

RL_MAJOR=0
RL_MINOR=0

# (
case "$ac_cv_rl_version" in
2*|3*|4*|5*|6*|7*|8*|9*)
	RL_MAJOR=`echo $ac_cv_rl_version | sed 's:\..*$::'`
	RL_MINOR=`echo $ac_cv_rl_version | sed -e 's:^.*\.::' -e 's:[[a-zA-Z]]*$::'`
	;;
esac

# (((
case $RL_MAJOR in
[[0-9][0-9]])	_RL_MAJOR=$RL_MAJOR ;;
[[0-9]])	_RL_MAJOR=0$RL_MAJOR ;;
*)		_RL_MAJOR=00 ;;
esac

# (((
case $RL_MINOR in
[[0-9][0-9]])	_RL_MINOR=$RL_MINOR ;;
[[0-9]])	_RL_MINOR=0$RL_MINOR ;;
*)		_RL_MINOR=00 ;;
esac

RL_VERSION="0x${_RL_MAJOR}${_RL_MINOR}"

# Readline versions greater than 4.2 have these defines in readline.h

if test $ac_cv_rl_version = '0.0' ; then
	AC_MSG_WARN([Could not test version of installed readline library.])
elif test $RL_MAJOR -gt 4 || { test $RL_MAJOR = 4 && test $RL_MINOR -gt 2 ; } ; then
	# set these for use by the caller
	RL_PREFIX=$ac_cv_rl_prefix
	RL_LIBDIR=$ac_cv_rl_libdir
	RL_INCLUDEDIR=$ac_cv_rl_includedir
	AC_MSG_RESULT($ac_cv_rl_version)
else

AC_DEFINE_UNQUOTED(RL_READLINE_VERSION, $RL_VERSION, [encoded version of the installed readline library])
AC_DEFINE_UNQUOTED(RL_VERSION_MAJOR, $RL_MAJOR, [major version of installed readline library])
AC_DEFINE_UNQUOTED(RL_VERSION_MINOR, $RL_MINOR, [minor version of installed readline library])

AC_SUBST(RL_VERSION)
AC_SUBST(RL_MAJOR)
AC_SUBST(RL_MINOR)

# set these for use by the caller
RL_PREFIX=$ac_cv_rl_prefix
RL_LIBDIR=$ac_cv_rl_libdir
RL_INCLUDEDIR=$ac_cv_rl_includedir

AC_MSG_RESULT($ac_cv_rl_version)

fi
])

AC_DEFUN(BASH_CHECK_WCONTINUED,
[
AC_MSG_CHECKING(whether WCONTINUED flag to waitpid is unavailable or available but broken)
AC_CACHE_VAL(bash_cv_wcontinued_broken,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#ifndef errno
extern int errno;
#endif
int
main()
{
	int	x;

	x = waitpid(-1, (int *)0, WNOHANG|WCONTINUED);
	if (x == -1 && errno == EINVAL)
		exit (1);
	else
		exit (0);
}
]])], [bash_cv_wcontinued_broken=no], [bash_cv_wcontinued_broken=yes],
   [AC_MSG_WARN(cannot check WCONTINUED if cross compiling -- defaulting to no)
    bash_cv_wcontinued_broken=no]
)])
AC_MSG_RESULT($bash_cv_wcontinued_broken)
if test $bash_cv_wcontinued_broken = yes; then
AC_DEFINE(WCONTINUED_BROKEN)
fi
])

dnl
dnl tests added for bashdb
dnl


AC_DEFUN([AM_PATH_LISPDIR],
 [AC_ARG_WITH(lispdir, AS_HELP_STRING([--with-lispdir], [override the default lisp directory]),
  [ lispdir="$withval" 
    AC_MSG_CHECKING([where .elc files should go])
    AC_MSG_RESULT([$lispdir])],
  [
  # If set to t, that means we are running in a shell under Emacs.
  # If you have an Emacs named "t", then use the full path.
  test x"$EMACS" = xt && EMACS=
  AC_CHECK_PROGS(EMACS, emacs xemacs, no)
  if test $EMACS != "no"; then
    if test x${lispdir+set} != xset; then
      AC_CACHE_CHECK([where .elc files should go], [am_cv_lispdir], [dnl
	am_cv_lispdir=`$EMACS -batch -q -eval '(while load-path (princ (concat (car load-path) "\n")) (setq load-path (cdr load-path)))' | sed -n -e 's,/$,,' -e '/.*\/lib\/\(x\?emacs\/site-lisp\)$/{s,,${libdir}/\1,;p;q;}' -e '/.*\/share\/\(x\?emacs\/site-lisp\)$/{s,,${datadir}/\1,;p;q;}'`
	if test -z "$am_cv_lispdir"; then
	  am_cv_lispdir='${datadir}/emacs/site-lisp'
	fi
      ])
      lispdir="$am_cv_lispdir"
    fi
  fi
 ])
 AC_SUBST(lispdir)
])

dnl From gnulib
AC_DEFUN([BASH_FUNC_FPURGE],
[
  AC_CHECK_FUNCS_ONCE([fpurge])
  AC_CHECK_FUNCS_ONCE([__fpurge])
  AC_CHECK_DECLS([fpurge], , , [#include <stdio.h>])
])

AC_DEFUN([BASH_FUNC_SNPRINTF],
[
  AC_CHECK_FUNCS_ONCE([snprintf])
  if test X$ac_cv_func_snprintf = Xyes; then
    AC_CACHE_CHECK([for standard-conformant snprintf], [bash_cv_func_snprintf],
      [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <stdlib.h>

int
main()
{
  int n;
  n = snprintf (0, 0, "%s", "0123456");
  exit(n != 7);
}
]])], [bash_cv_func_snprintf=yes], [bash_cv_func_snprintf=no],
   [AC_MSG_WARN([cannot check standard snprintf if cross-compiling])
    bash_cv_func_snprintf=yes]
)])
    if test $bash_cv_func_snprintf = no; then
      ac_cv_func_snprintf=no
    fi
  fi
  if test $ac_cv_func_snprintf = no; then
    AC_DEFINE(HAVE_SNPRINTF, 0,
      [Define if you have a standard-conformant snprintf function.])
  fi
])

AC_DEFUN([BASH_FUNC_VSNPRINTF],
[
  AC_CHECK_FUNCS_ONCE([vsnprintf])
  if test X$ac_cv_func_vsnprintf = Xyes; then
    AC_CACHE_CHECK([for standard-conformant vsnprintf], [bash_cv_func_vsnprintf],
      [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#if HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdio.h>
#include <stdlib.h>

static int
#if HAVE_STDARG_H
foo(const char *fmt, ...)
#else
foo(format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;
  int n;

#if HAVE_STDARG_H
  va_start(args, fmt);
#else
  va_start(args);
#endif
  n = vsnprintf(0, 0, fmt, args);
  va_end (args);
  return n;
}

int
main()
{
  int n;
  n = foo("%s", "0123456");
  exit(n != 7);
}
]])], [bash_cv_func_vsnprintf=yes], [bash_cv_func_vsnprintf=no],
   [AC_MSG_WARN([cannot check standard vsnprintf if cross-compiling])
    bash_cv_func_vsnprintf=yes]
)])
    if test $bash_cv_func_vsnprintf = no; then
      ac_cv_func_vsnprintf=no
    fi
  fi
  if test $ac_cv_func_vsnprintf = no; then
    AC_DEFINE(HAVE_VSNPRINTF, 0,
      [Define if you have a standard-conformant vsnprintf function.])
  fi
])

AC_DEFUN(BASH_STRUCT_WEXITSTATUS_OFFSET,
[AC_MSG_CHECKING(for offset of exit status in return status from wait)
AC_CACHE_VAL(bash_cv_wexitstatus_offset,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

int
main(c, v)
     int c;
     char **v;
{
  pid_t pid, p;
  int s, i, n;

  s = 0;
  pid = fork();
  if (pid == 0)
    exit (42);

  /* wait for the process */
  p = wait(&s);
  if (p != pid)
    exit (255);

  /* crack s */
  for (i = 0; i < (sizeof(s) * 8); i++)
    {
      n = (s >> i) & 0xff;
      if (n == 42)
	exit (i);
    }

  exit (254);
}
]])], [bash_cv_wexitstatus_offset=0], [bash_cv_wexitstatus_offset=$?],
   [AC_MSG_WARN(cannot check WEXITSTATUS offset if cross compiling -- defaulting to 0)
    bash_cv_wexitstatus_offset=0]
)])
if test "$bash_cv_wexitstatus_offset" -gt 32 ; then
  AC_MSG_WARN(bad exit status from test program -- defaulting to 0)
  bash_cv_wexitstatus_offset=0
fi
AC_MSG_RESULT($bash_cv_wexitstatus_offset)
AC_DEFINE_UNQUOTED([WEXITSTATUS_OFFSET], [$bash_cv_wexitstatus_offset], [Offset of exit status in wait status word])
])

AC_DEFUN([BASH_FUNC_SBRK],
[
  AC_MSG_CHECKING([for sbrk])
  AC_CACHE_VAL(ac_cv_func_sbrk,
  [AC_LINK_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <unistd.h>]],
		[[ void *x = sbrk (4096); ]])],
	[ac_cv_func_sbrk=yes],[ac_cv_func_sbrk=no])])
  AC_MSG_RESULT($ac_cv_func_sbrk)
  if test X$ac_cv_func_sbrk = Xyes; then
    AC_CACHE_CHECK([for working sbrk], [bash_cv_func_sbrk],
      [AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdlib.h>
#include <unistd.h>

int
main(int c, char **v)
{
	void *x;

	x = sbrk (4096);
	exit ((x == (void *)-1) ? 1 : 0);
}
]])],[bash_cv_func_sbrk=yes],[bash_cv_func_sbrk=no],[AC_MSG_WARN([cannot check working sbrk if cross-compiling])
    bash_cv_func_sbrk=yes
])])
    if test $bash_cv_func_sbrk = no; then
      ac_cv_func_sbrk=no
    fi
  fi
  if test $ac_cv_func_sbrk = yes; then
    AC_DEFINE(HAVE_SBRK, 1,
      [Define if you have a working sbrk function.])
  fi
])

AC_DEFUN(BASH_FUNC_FNMATCH_EQUIV_FALLBACK,
[AC_MSG_CHECKING(whether fnmatch can be used to check bracket equivalence classes)
AC_CACHE_VAL(bash_cv_fnmatch_equiv_fallback,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fnmatch.h>
#include <locale.h>

char *pattern = "[[=a=]]";

/* char *string = "ä"; */
unsigned char string[4] = { '\xc3', '\xa4', '\0' };

int
main (int c, char **v)
{
  setlocale (LC_ALL, "en_US.UTF-8");
  if (fnmatch (pattern, (const char *)string, 0) != FNM_NOMATCH)
    exit (0);
  exit (1);
}
]])], [bash_cv_fnmatch_equiv_fallback=yes], [bash_cv_fnmatch_equiv_fallback=no],
   [AC_MSG_WARN(cannot check fnmatch if cross compiling -- defaulting to no)
    bash_cv_fnmatch_equiv_fallback=no]
)])
AC_MSG_RESULT($bash_cv_fnmatch_equiv_fallback)
if test "$bash_cv_fnmatch_equiv_fallback" = "yes" ; then
    bash_cv_fnmatch_equiv_value=1
else
    bash_cv_fnmatch_equiv_value=0
fi
AC_DEFINE_UNQUOTED([FNMATCH_EQUIV_FALLBACK], [$bash_cv_fnmatch_equiv_value], [Whether fnmatch can be used for bracket equivalence classes])
])
