/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2008, 2009-2023
 *	Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

#ifndef SUDO_COMPAT_H
#define SUDO_COMPAT_H

#include <sys/types.h>	/* for gid_t, mode_t, size_t, ssize_t, time_t, uid_t */
#include <sys/stat.h>	/* to avoid problems with mismatched headers and libc */
#include <unistd.h>	/* to avoid problems with mismatched headers and libc */
#include <stdio.h>
#if !defined(HAVE_VSNPRINTF) || !defined(HAVE_VASPRINTF) || \
    !defined(HAVE_VSYSLOG) || defined(PREFER_PORTABLE_SNPRINTF)
# include <stdarg.h>
#endif

/*
 * Macros and functions that may be missing on some operating systems.
 */

/*
 * Given the pointer x to the member m of the struct s, return
 * a pointer to the containing structure.
 */
#ifndef __containerof
# define __containerof(x, s, m)	((s *)((char *)(x) - offsetof(s, m)))
#endif

/*
 * Pre-C99 compilers may lack a va_copy macro.
 */
#ifndef HAVE_VA_COPY
# ifdef HAVE___VA_COPY
#  define va_copy(d, s) __va_copy(d, s)
# else
#  define va_copy(d, s) memcpy(&(d), &(s), sizeof(d));
# endif
#endif

/*
 * Some systems lack full limit definitions.
 */
#if defined(HAVE_DECL_LLONG_MAX) && !HAVE_DECL_LLONG_MAX
# if defined(HAVE_DECL_QUAD_MAX) && HAVE_DECL_QUAD_MAX
#  define LLONG_MAX	QUAD_MAX
# else
#  define LLONG_MAX	0x7fffffffffffffffLL
# endif
#endif

#if defined(HAVE_DECL_LLONG_MIN) && !HAVE_DECL_LLONG_MIN
# if defined(HAVE_DECL_QUAD_MIN) && HAVE_DECL_QUAD_MIN
#  define LLONG_MIN	QUAD_MIN
# else
#  define LLONG_MIN	(-0x7fffffffffffffffLL-1)
# endif
#endif

#if defined(HAVE_DECL_ULLONG_MAX) && !HAVE_DECL_ULLONG_MAX
# if defined(HAVE_DECL_UQUAD_MAX) && HAVE_DECL_UQUAD_MAX
#  define ULLONG_MAX	UQUAD_MAX
# else
#  define ULLONG_MAX	0xffffffffffffffffULL
# endif
#endif

#if defined(HAVE_DECL_SIZE_MAX) && !HAVE_DECL_SIZE_MAX
# if defined(HAVE_DECL_SIZE_T_MAX) && HAVE_DECL_SIZE_T_MAX
#  define SIZE_MAX	SIZE_T_MAX
# else
#  define SIZE_MAX	ULONG_MAX
# endif
#endif

#if defined(HAVE_DECL_SSIZE_MAX) && !HAVE_DECL_SSIZE_MAX
# define SSIZE_MAX	LONG_MAX
#endif

#if defined(HAVE_DECL_PATH_MAX) && !HAVE_DECL_PATH_MAX
# if defined(HAVE_DECL__POSIX_PATH_MAX) && HAVE_DECL__POSIX_PATH_MAX
#  define PATH_MAX		_POSIX_PATH_MAX
# else
#  define PATH_MAX		256
# endif
#endif

/* ACCESSPERMS and ALLPERMS are handy BSDisms. */
#ifndef ACCESSPERMS
# define ACCESSPERMS	00777
#endif /* ACCESSPERMS */
#ifndef ALLPERMS
# define ALLPERMS	07777
#endif /* ALLPERMS */

/* For futimens() and utimensat() emulation. */
#if !defined(HAVE_FUTIMENS) && !defined(HAVE_UTIMENSAT)
# ifndef UTIME_OMIT
#  define UTIME_OMIT	-1L
# endif
# ifndef UTIME_NOW
#  define UTIME_NOW	-2L
# endif
#endif
#if !defined(HAVE_OPENAT) || (!defined(HAVE_FUTIMENS) && !defined(HAVE_UTIMENSAT)) || !defined(HAVE_FCHMODAT) || !defined(HAVE_FSTATAT) || !defined(HAVE_UNLINKAT)
# ifndef AT_FDCWD
#  define AT_FDCWD		-100
# endif
# ifndef AT_SYMLINK_NOFOLLOW
#  define AT_SYMLINK_NOFOLLOW	0x02
# endif
#endif

/* For dup3() and pipe2() emulation. */
#if (!defined(HAVE_PIPE2) || !defined(HAVE_DUP3)) && defined(O_NONBLOCK)
# if !defined(O_CLOEXEC) || O_CLOEXEC > 0xffffffff
#  undef O_CLOEXEC
#  define O_CLOEXEC	0x80000000
# endif
#endif

/*
 * BSD defines these in <sys/param.h> but we don't include that anymore.
 */
#ifndef MIN
# define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
# define MAX(a,b) (((a)>(b))?(a):(b))
#endif

/* Macros to set/clear/test flags. */
#undef SET
#define SET(t, f)	((t) |= (f))
#undef CLR
#define CLR(t, f)	((t) &= ~(f))
#undef ISSET
#define ISSET(t, f)     ((t) & (f))

/*
 * Some systems define this in <sys/param.h> but we don't include that anymore.
 */
#ifndef howmany
# define howmany(x, y)	(((x) + ((y) - 1)) / (y))
#endif

/*
 * Simple isblank() macro and function for systems without it.
 */
#ifndef HAVE_ISBLANK
sudo_dso_public int isblank(int);
# define isblank(_x)	((_x) == ' ' || (_x) == '\t')
#endif

/*
 * NCR's SVr4 has _innetgr(3) instead of innetgr(3) for some reason.
 */
#ifdef HAVE__INNETGR
# define innetgr(n, h, u, d)	(_innetgr(n, h, u, d))
# define HAVE_INNETGR 1
#endif /* HAVE__INNETGR */

/*
 * The nitems macro may be defined in sys/param.h
 */
#ifndef nitems
# define nitems(_a)	(sizeof((_a)) / sizeof((_a)[0]))
#endif

/*
 * If dirfd() does not exists, hopefully dd_fd does.
 */
#if !defined(HAVE_DIRFD) && defined(HAVE_DD_FD)
# define dirfd(_d)	((_d)->dd_fd)
# define HAVE_DIRFD
#endif

#if !defined(HAVE_KILLPG) && !defined(killpg)
# define killpg(p, s)	kill(-(p), (s))
#endif

/*
 * Declare errno if errno.h doesn't do it for us.
 */
#if defined(HAVE_DECL_ERRNO) && !HAVE_DECL_ERRNO
extern int errno;
#endif /* !HAVE_DECL_ERRNO */

/* Not all systems define NSIG in signal.h */
#if defined(HAVE_DECL_NSIG) && !HAVE_DECL_NSIG
# if defined(HAVE_DECL__NSIG) && HAVE_DECL__NSIG
#  define NSIG _NSIG
# elif defined(HAVE_DECL___NSIG) && HAVE_DECL___NSIG
#  define NSIG __NSIG
# else
#  define NSIG 64
# endif
#endif

/* For sig2str() */
#if !defined(HAVE_DECL_SIG2STR_MAX) || !HAVE_DECL_SIG2STR_MAX
# define SIG2STR_MAX 32
#endif

/* WCOREDUMP is not POSIX, this usually works (verified on AIX). */
#ifndef WCOREDUMP
# define WCOREDUMP(x)	((x) & 0x80)
#endif

/* W_EXITCODE is not POSIX but the encoding of wait status is. */
#ifndef W_EXITCODE
# define W_EXITCODE(ret, sig)	((ret) << 8 | (sig))
#endif

/* Number of bits in a byte. */
#ifndef NBBY
# ifdef __NBBY
#  define NBBY __NBBY
# else
#  define NBBY 8
# endif
#endif

#ifndef HAVE_SETEUID
#  if defined(HAVE_SETRESUID)
#    define seteuid(u)	setresuid(-1, (u), -1)
#    define setegid(g)	setresgid(-1, (g), -1)
#    define HAVE_SETEUID 1
#  elif defined(HAVE_SETREUID)
#    define seteuid(u)	setreuid(-1, (u))
#    define setegid(g)	setregid(-1, (g))
#    define HAVE_SETEUID 1
#  endif
#endif /* HAVE_SETEUID */

/*
 * Older HP-UX does not declare setresuid() or setresgid().
 */
#if defined(HAVE_DECL_SETRESUID) && !HAVE_DECL_SETRESUID
int setresuid(uid_t, uid_t, uid_t);
int setresgid(gid_t, gid_t, gid_t);
#endif
#if defined(HAVE_DECL_GETRESUID) && !HAVE_DECL_GETRESUID
int getresuid(uid_t *, uid_t *, uid_t *);
int getresgid(gid_t *, gid_t *, gid_t *);
#endif

/*
 * HP-UX does not declare innetgr() or getdomainname().
 * Solaris does not declare getdomainname().
 */
#if defined(HAVE_DECL_INNETGR) && !HAVE_DECL_INNETGR
int innetgr(const char *, const char *, const char *, const char *);
#endif
#if defined(HAVE_DECL__INNETGR) && !HAVE_DECL__INNETGR
int _innetgr(const char *, const char *, const char *, const char *);
#endif
#if defined(HAVE_DECL_GETDOMAINNAME) && !HAVE_DECL_GETDOMAINNAME
int getdomainname(char *, size_t);
#endif

/*
 * HP-UX 11.00 has broken pread/pwrite on 32-bit machines when
 * _FILE_OFFSET_BITS == 64.  Use pread64/pwrite64 instead.
 */
#if defined(__hpux) && !defined(__LP64__)
# ifdef HAVE_PREAD64
#  undef pread
#  define pread(_a, _b, _c, _d) pread64((_a), (_b), (_c), (_d))
#  if defined(HAVE_DECL_PREAD64) && !HAVE_DECL_PREAD64
    ssize_t pread64(int fd, void *buf, size_t nbytes, off64_t offset);
#  endif
# endif
# ifdef HAVE_PWRITE64
#  undef pwrite
#  define pwrite(_a, _b, _c, _d) pwrite64((_a), (_b), (_c), (_d))
#  if defined(HAVE_DECL_PWRITE64) && !HAVE_DECL_PWRITE64
    ssize_t pwrite64(int fd, const void *buf, size_t nbytes, off64_t offset);
#  endif
# endif
#endif /* __hpux && !__LP64__ */

/*
 * Older systems may lack fseeko(3), just use fseek(3) instead.
 */
#ifndef HAVE_FSEEKO
# define fseeko(f, o, w)	fseek((f), (long)(o), (w))
#endif

/*
 * Functions "missing" from libc.
 * All libc replacements are prefixed with "sudo_" to avoid namespace issues.
 */

struct passwd;
struct stat;
struct timespec;
struct termios;
struct tm;

#ifndef HAVE_CFMAKERAW
sudo_dso_public void sudo_cfmakeraw(struct termios *term);
# undef cfmakeraw
# define cfmakeraw(_a) sudo_cfmakeraw((_a))
#endif /* HAVE_CFMAKERAW */
#ifndef HAVE_CLOSEFROM
sudo_dso_public void sudo_closefrom(int);
# undef closefrom
# define closefrom(_a) sudo_closefrom((_a))
#endif /* HAVE_CLOSEFROM */
#ifndef HAVE_EXPLICIT_BZERO
sudo_dso_public void sudo_explicit_bzero(void *s, size_t n);
# undef explicit_bzero
# define explicit_bzero(_a, _b) sudo_explicit_bzero((_a), (_b))
#endif /* HAVE_EXPLICIT_BZERO */
#ifndef HAVE_FREEZERO
sudo_dso_public void sudo_freezero(void *p, size_t n);
# undef freezero
# define freezero(_a, _b) sudo_freezero((_a), (_b))
#endif /* HAVE_FREEZERO */
#ifndef HAVE_GETGROUPLIST
sudo_dso_public int sudo_getgrouplist(const char *name, GETGROUPS_T basegid, GETGROUPS_T *groups, int *ngroupsp);
# undef getgrouplist
# define getgrouplist(_a, _b, _c, _d) sudo_getgrouplist((_a), (_b), (_c), (_d))
#endif /* GETGROUPLIST */
#if !defined(HAVE_GETDELIM)
sudo_dso_public ssize_t sudo_getdelim(char ** restrict bufp, size_t * restrict bufsizep, int delim, FILE * restrict fp);
# undef getdelim
# define getdelim(_a, _b, _c, _d) sudo_getdelim((_a), (_b), (_c), (_d))
#elif defined(HAVE_DECL_GETDELIM) && !HAVE_DECL_GETDELIM
/* getdelim present in libc but missing prototype (old gcc fixed includes?) */
ssize_t getdelim(char **bufp, size_t *bufsizep, int delim, FILE *fp);
#endif /* HAVE_GETDELIM */
#ifndef HAVE_GETUSERSHELL
sudo_dso_public char *sudo_getusershell(void);
# undef getusershell
# define getusershell() sudo_getusershell()
sudo_dso_public void sudo_setusershell(void);
# undef setusershell
# define setusershell() sudo_setusershell()
sudo_dso_public void sudo_endusershell(void);
# undef endusershell
# define endusershell() sudo_endusershell()
#elif HAVE_DECL_GETUSERSHELL == 0
/* Older Solaris has getusershell() et al but does not declare it. */
char *getusershell(void);
void setusershell(void);
void endusershell(void);
#endif /* HAVE_GETUSERSHELL */
#ifndef HAVE_GMTIME_R
sudo_dso_public struct tm *sudo_gmtime_r(const time_t *, struct tm *);
# undef gmtime_r
# define gmtime_r(_a, _b) sudo_gmtime_r((_a), (_b))
#endif /* HAVE_GMTIME_R */
#ifndef HAVE_LOCALTIME_R
sudo_dso_public struct tm *sudo_localtime_r(const time_t *, struct tm *);
# undef localtime_r
# define localtime_r(_a, _b) sudo_localtime_r((_a), (_b))
#endif /* HAVE_LOCALTIME_R */
#ifndef HAVE_TIMEGM
sudo_dso_public time_t sudo_timegm(struct tm *);
#endif /* HAVE_TIMEGM */
#ifndef HAVE_UTIMENSAT
sudo_dso_public int sudo_utimensat(int fd, const char *file, const struct timespec *times, int flag);
# undef utimensat
# define utimensat(_a, _b, _c, _d) sudo_utimensat((_a), (_b), (_c), (_d))
#endif /* HAVE_UTIMENSAT */
#ifndef HAVE_FCHMODAT
sudo_dso_public int sudo_fchmodat(int dfd, const char *path, mode_t mode, int flag);
# undef fchmodat
# define fchmodat(_a, _b, _c, _d) sudo_fchmodat((_a), (_b), (_c), (_d))
#endif /* HAVE_FCHMODAT */
#ifndef HAVE_FSTATAT
sudo_dso_public int sudo_fstatat(int dfd, const char *path, struct stat *sb, int flag);
# undef fstatat
# define fstatat(_a, _b, _c, _d) sudo_fstatat((_a), (_b), (_c), (_d))
#endif /* HAVE_FSTATAT */
#ifndef HAVE_FUTIMENS
sudo_dso_public int sudo_futimens(int fd, const struct timespec *times);
# undef futimens
# define futimens(_a, _b) sudo_futimens((_a), (_b))
#endif /* HAVE_FUTIMENS */
#if !defined(HAVE_SNPRINTF) || defined(PREFER_PORTABLE_SNPRINTF)
sudo_dso_public int sudo_snprintf(char * restrict str, size_t n, char const * restrict fmt, ...) sudo_printflike(3, 4);
# undef snprintf
# define snprintf sudo_snprintf
#endif /* HAVE_SNPRINTF */
#if !defined(HAVE_VSNPRINTF) || defined(PREFER_PORTABLE_SNPRINTF)
sudo_dso_public int sudo_vsnprintf(char * restrict str, size_t n, const char * restrict fmt, va_list ap) sudo_printflike(3, 0);
# undef vsnprintf
# define vsnprintf sudo_vsnprintf
#endif /* HAVE_VSNPRINTF */
#if !defined(HAVE_ASPRINTF) || defined(PREFER_PORTABLE_SNPRINTF)
sudo_dso_public int sudo_asprintf(char ** restrict str, char const * restrict fmt, ...) sudo_printflike(2, 3);
# undef asprintf
# define asprintf sudo_asprintf
#endif /* HAVE_ASPRINTF */
#if !defined(HAVE_VASPRINTF) || defined(PREFER_PORTABLE_SNPRINTF)
sudo_dso_public int sudo_vasprintf(char ** restrict str, const char * restrict fmt, va_list ap) sudo_printflike(2, 0);
# undef vasprintf
# define vasprintf sudo_vasprintf
#endif /* HAVE_VASPRINTF */
#ifndef HAVE_STRLCAT
sudo_dso_public size_t sudo_strlcat(char * restrict dst, const char * restrict src, size_t siz);
# undef strlcat
# define strlcat(_a, _b, _c) sudo_strlcat((_a), (_b), (_c))
#endif /* HAVE_STRLCAT */
#ifndef HAVE_STRLCPY
sudo_dso_public size_t sudo_strlcpy(char * restrict dst, const char * restrict src, size_t siz);
# undef strlcpy
# define strlcpy(_a, _b, _c) sudo_strlcpy((_a), (_b), (_c))
#endif /* HAVE_STRLCPY */
#ifndef HAVE_STRNDUP
sudo_dso_public char *sudo_strndup(const char *str, size_t maxlen);
# undef strndup
# define strndup(_a, _b) sudo_strndup((_a), (_b))
#endif /* HAVE_STRNDUP */
#ifndef HAVE_STRNLEN
sudo_dso_public size_t sudo_strnlen(const char *str, size_t maxlen);
# undef strnlen
# define strnlen(_a, _b) sudo_strnlen((_a), (_b))
#endif /* HAVE_STRNLEN */
#ifndef HAVE_FCHOWNAT
sudo_dso_public int sudo_fchownat(int dfd, const char *path, uid_t uid, gid_t gid, int flag);
# undef fchownat
# define fchownat(_a, _b, _c, _d, _e) sudo_fchownat((_a), (_b), (_c), (_d), (_e))
#endif /* HAVE_FCHOWNAT */
#ifndef HAVE_MEMRCHR
sudo_dso_public void *sudo_memrchr(const void *s, int c, size_t n);
# undef memrchr
# define memrchr(_a, _b, _c) sudo_memrchr((_a), (_b), (_c))
#endif /* HAVE_MEMRCHR */
#ifndef HAVE_MKDIRAT
sudo_dso_public int sudo_mkdirat(int dfd, const char *path, mode_t mode);
# undef mkdirat
# define mkdirat(_a, _b, _c) sudo_mkdirat((_a), (_b), (_c))
#endif /* HAVE_MKDIRAT */
#if !defined(HAVE_MKDTEMPAT) || !defined(HAVE_MKOSTEMPSAT)
# if defined(HAVE_MKDTEMPAT_NP) && defined(HAVE_MKOSTEMPSAT_NP)
#  undef mkdtempat
#  define mkdtempat mkdtempat_np
#  undef mkostempsat
#  define mkostempsat mkostempsat_np
# else
sudo_dso_public char *sudo_mkdtemp(char *path);
#  undef mkdtemp
#  define mkdtemp(_a) sudo_mkdtemp((_a))
sudo_dso_public char *sudo_mkdtempat(int dfd, char *path);
#  undef mkdtempat
#  define mkdtempat(_a, _b) sudo_mkdtempat((_a), (_b))
sudo_dso_public int sudo_mkostempsat(int dfd, char *path, int slen, int flags);
#  undef mkostempsat
#  define mkostempsat(_a, _b, _c, _d) sudo_mkostempsat((_a), (_b), (_c), (_d))
sudo_dso_public int sudo_mkstemp(char *path);
#  undef mkstemp
#  define mkstemp(_a) sudo_mkstemp((_a))
sudo_dso_public int sudo_mkstemps(char *path, int slen);
#  undef mkstemps
#  define mkstemps(_a, _b) sudo_mkstemps((_a), (_b))
# endif /* HAVE_MKDTEMPAT_NP || HAVE_MKOSTEMPSAT_NP */
#endif /* !HAVE_MKDTEMPAT || !HAVE_MKOSTEMPSAT */
#ifndef HAVE_NANOSLEEP
sudo_dso_public int sudo_nanosleep(const struct timespec *timeout, struct timespec *remainder);
#undef nanosleep
# define nanosleep(_a, _b) sudo_nanosleep((_a), (_b))
#endif /* HAVE_NANOSLEEP */
#ifndef HAVE_OPENAT
sudo_dso_public int sudo_openat(int dfd, const char *path, int flags, mode_t mode);
# undef openat
# define openat(_a, _b, _c, _d) sudo_openat((_a), (_b), (_c), (_d))
#endif /* HAVE_OPENAT */
#ifndef HAVE_PW_DUP
sudo_dso_public struct passwd *sudo_pw_dup(const struct passwd *pw);
# undef pw_dup
# define pw_dup(_a) sudo_pw_dup((_a))
#endif /* HAVE_PW_DUP */
#ifndef HAVE_STRSIGNAL
sudo_dso_public char *sudo_strsignal(int signo);
# undef strsignal
# define strsignal(_a) sudo_strsignal((_a))
#endif /* HAVE_STRSIGNAL */
#ifndef HAVE_SIG2STR
sudo_dso_public int sudo_sig2str(int signo, char *signame);
# undef sig2str
# define sig2str(_a, _b) sudo_sig2str((_a), (_b))
#endif /* HAVE_SIG2STR */
#ifndef HAVE_STR2SIG
sudo_dso_public int sudo_str2sig(const char *signame, int *signum);
# undef str2sig
# define str2sig(_a, _b) sudo_str2sig((_a), (_b))
#endif /* HAVE_STR2SIG */
#if !defined(HAVE_INET_NTOP) && defined(NEED_INET_NTOP)
sudo_dso_public char *sudo_inet_ntop(int af, const void *src, char *dst, socklen_t size);
# undef inet_ntop
# define inet_ntop(_a, _b, _c, _d) sudo_inet_ntop((_a), (_b), (_c), (_d))
#endif /* HAVE_INET_NTOP */
#ifndef HAVE_INET_PTON
sudo_dso_public int sudo_inet_pton(int af, const char *src, void *dst);
# undef inet_pton
# define inet_pton(_a, _b, _c) sudo_inet_pton((_a), (_b), (_c))
#endif /* HAVE_INET_PTON */
#ifndef HAVE_GETPROGNAME
sudo_dso_public const char *sudo_getprogname(void);
# undef getprogname
# define getprogname() sudo_getprogname()
#endif /* HAVE_GETPROGNAME */
#ifndef HAVE_SETPROGNAME
sudo_dso_public void sudo_setprogname(const char *name);
# undef setprogname
# define setprogname(_a) sudo_setprogname(_a)
#endif /* HAVE_SETPROGNAME */
#ifndef HAVE_REALLOCARRAY
sudo_dso_public void *sudo_reallocarray(void *ptr, size_t nmemb, size_t size);
# undef reallocarray
# define reallocarray(_a, _b, _c) sudo_reallocarray((_a), (_b), (_c))
#endif /* HAVE_REALLOCARRAY */
#ifndef HAVE_REALPATH
sudo_dso_public char *sudo_realpath(const char * restrict path, char * restrict resolved);
# undef realpath
# define realpath(_a, _b) sudo_realpath((_a), (_b))
#endif /* HAVE_REALPATH */
#ifndef HAVE_DUP3
sudo_dso_public int sudo_dup3(int oldd, int newd, int flags);
# undef dup3
# define dup3(_a, _b, _c) sudo_dup3((_a), (_b), (_c))
#endif /* HAVE_DUP3 */
#ifndef HAVE_PIPE2
sudo_dso_public int sudo_pipe2(int fildes[2], int flags);
# undef pipe2
# define pipe2(_a, _b) sudo_pipe2((_a), (_b))
#endif /* HAVE_PIPE2 */
#ifndef HAVE_PREAD
sudo_dso_public ssize_t sudo_pread(int fd, void *buf, size_t nbytes, off_t offset);
# undef pread
# define pread(_a, _b, _c, _d) sudo_pread((_a), (_b), (_c), (_d))
#endif /* HAVE_PREAD */
#ifndef HAVE_PWRITE
sudo_dso_public ssize_t sudo_pwrite(int fd, const void *buf, size_t nbytes, off_t offset);
# undef pwrite
# define pwrite(_a, _b, _c, _d) sudo_pwrite((_a), (_b), (_c), (_d))
#endif /* HAVE_PWRITE */
#ifndef HAVE_UNLINKAT
sudo_dso_public int sudo_unlinkat(int dfd, const char *path, int flag);
# undef unlinkat
# define unlinkat(_a, _b, _c) sudo_unlinkat((_a), (_b), (_c))
#endif /* HAVE_UNLINKAT */

#endif /* SUDO_COMPAT_H */
