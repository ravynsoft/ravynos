/* Provide a more complete sys/stat.h header file.
   Copyright (C) 2005-2026 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Eric Blake, Paul Eggert, and Jim Meyering.  */

/* This file is supposed to be used on platforms where <sys/stat.h> is
   incomplete.  It is intended to provide definitions and prototypes
   needed by an application.  Start with what the system provides.  */

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* This file uses #include_next of a system file that defines time_t.
   For the 'year2038' module to work right, <config.h> needs to have been
   included before.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#if defined __need_system_sys_stat_h
/* Special invocation convention.  */

#@INCLUDE_NEXT@ @NEXT_SYS_STAT_H@

#else
/* Normal invocation convention.  */

#ifndef _@GUARD_PREFIX@_SYS_STAT_H

/* Get nlink_t.
   May also define off_t to a 64-bit type on native Windows.  */
#include <sys/types.h>

/* Get struct timespec.  */
#include <time.h>

/* The include_next requires a split double-inclusion guard.  */
#@INCLUDE_NEXT@ @NEXT_SYS_STAT_H@

#ifndef _@GUARD_PREFIX@_SYS_STAT_H
#define _@GUARD_PREFIX@_SYS_STAT_H

/* This file uses _GL_ATTRIBUTE_NODISCARD, _GL_ATTRIBUTE_NOTHROW,
   GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif


/* _GL_ATTRIBUTE_NOTHROW declares that the function does not throw exceptions.
 */
#ifndef _GL_ATTRIBUTE_NOTHROW
# if defined __cplusplus
#  if (__GNUC__ + (__GNUC_MINOR__ >= 8) > 2) || __clang_major__ >= 4
#   if __cplusplus >= 201103L
#    define _GL_ATTRIBUTE_NOTHROW noexcept (true)
#   else
#    define _GL_ATTRIBUTE_NOTHROW throw ()
#   endif
#  else
#   define _GL_ATTRIBUTE_NOTHROW
#  endif
# else
#  if (__GNUC__ + (__GNUC_MINOR__ >= 3) > 3) || defined __clang__
#   define _GL_ATTRIBUTE_NOTHROW __attribute__ ((__nothrow__))
#  else
#   define _GL_ATTRIBUTE_NOTHROW
#  endif
# endif
#endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


/* Before doing "#define mknod rpl_mknod" below, we need to include all
   headers that may declare mknod().  OS/2 kLIBC declares mknod() in
   <unistd.h>, not in <sys/stat.h>.  */
#ifdef __KLIBC__
# include <unistd.h>
#endif

/* Before doing "#define mkdir rpl_mkdir" below, we need to include all
   headers that may declare mkdir().  Native Windows platforms declare mkdir
   in <io.h> and/or <direct.h>, not in <sys/stat.h>.  */
#if defined _WIN32 && ! defined __CYGWIN__
# include <io.h>     /* mingw32, mingw64 */
# include <direct.h> /* mingw64, MSVC 9 */
#endif

/* Native Windows platforms declare umask() in <io.h>.  */
#if 0 && (defined _WIN32 && ! defined __CYGWIN__)
# include <io.h>
#endif

/* Large File Support on native Windows.  */
#if @WINDOWS_64_BIT_ST_SIZE@
# define stat _stati64
#endif

/* Optionally, override 'struct stat' on native Windows.  */
#if @GNULIB_OVERRIDES_STRUCT_STAT@

# undef stat
# if @GNULIB_STAT@
#  define stat rpl_stat
# else
#  if !GNULIB_STAT
    /* Provoke a clear link error if stat() is used as a function and
       module 'stat' is not in use.  */
#   define stat stat_used_without_requesting_gnulib_module_stat
#  endif
# endif

# if !GNULIB_defined_struct_stat
struct stat
{
  dev_t st_dev;
  ino_t st_ino;
  mode_t st_mode;
  nlink_t st_nlink;
#  if 0
  uid_t st_uid;
#  else /* uid_t is not defined by default on native Windows.  */
  short st_uid;
#  endif
#  if 0
  gid_t st_gid;
#  else /* gid_t is not defined by default on native Windows.  */
  short st_gid;
#  endif
  dev_t st_rdev;
  off_t st_size;
#  if 0
  blksize_t st_blksize;
  blkcnt_t st_blocks;
#  endif

#  if @WINDOWS_STAT_TIMESPEC@
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
#  else
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
#  endif
};
#  if @WINDOWS_STAT_TIMESPEC@
#   define st_atime st_atim.tv_sec
#   define st_mtime st_mtim.tv_sec
#   define st_ctime st_ctim.tv_sec
    /* Indicator, for gnulib internal purposes.  */
#   define _GL_WINDOWS_STAT_TIMESPEC 1
#  endif
#  define GNULIB_defined_struct_stat 1
# endif

/* Other possible values of st_mode.  */
# if 0
#  define _S_IFBLK  0x6000
# endif
# if 0
#  define _S_IFLNK  0xA000
# endif
# if 0
#  define _S_IFSOCK 0xC000
# endif

#endif

#ifndef S_IFIFO
# ifdef _S_IFIFO
#  define S_IFIFO _S_IFIFO
# endif
#endif

#ifndef S_IFMT
# define S_IFMT 0170000
#endif

#if STAT_MACROS_BROKEN
# undef S_ISBLK
# undef S_ISCHR
# undef S_ISDIR
# undef S_ISFIFO
# undef S_ISLNK
# undef S_ISNAM
# undef S_ISMPB
# undef S_ISMPC
# undef S_ISNWK
# undef S_ISREG
# undef S_ISSOCK
#endif

#ifndef S_ISBLK
# ifdef S_IFBLK
#  define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
# else
#  define S_ISBLK(m) 0
# endif
#endif

#ifndef S_ISCHR
# ifdef S_IFCHR
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
# else
#  define S_ISCHR(m) 0
# endif
#endif

#ifndef S_ISDIR
# ifdef S_IFDIR
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
# else
#  define S_ISDIR(m) 0
# endif
#endif

#ifndef S_ISDOOR /* Solaris 2.5 and up */
# define S_ISDOOR(m) 0
#endif

#ifndef S_ISFIFO
# ifdef S_IFIFO
#  define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
# else
#  define S_ISFIFO(m) 0
# endif
#endif

#ifndef S_ISLNK
# ifdef S_IFLNK
#  define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
# else
#  define S_ISLNK(m) 0
# endif
#endif

#ifndef S_ISMPB /* V7 */
# ifdef S_IFMPB
#  define S_ISMPB(m) (((m) & S_IFMT) == S_IFMPB)
#  define S_ISMPC(m) (((m) & S_IFMT) == S_IFMPC)
# else
#  define S_ISMPB(m) 0
#  define S_ISMPC(m) 0
# endif
#endif

#ifndef S_ISMPX /* AIX */
# define S_ISMPX(m) 0
#endif

#ifndef S_ISNAM /* Xenix */
# ifdef S_IFNAM
#  define S_ISNAM(m) (((m) & S_IFMT) == S_IFNAM)
# else
#  define S_ISNAM(m) 0
# endif
#endif

#ifndef S_ISNWK /* HP/UX */
# ifdef S_IFNWK
#  define S_ISNWK(m) (((m) & S_IFMT) == S_IFNWK)
# else
#  define S_ISNWK(m) 0
# endif
#endif

#ifndef S_ISPORT /* Solaris 10 and up */
# define S_ISPORT(m) 0
#endif

#ifndef S_ISREG
# ifdef S_IFREG
#  define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
# else
#  define S_ISREG(m) 0
# endif
#endif

#ifndef S_ISSOCK
# ifdef S_IFSOCK
#  define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
# else
#  define S_ISSOCK(m) 0
# endif
#endif


#ifndef S_TYPEISMQ
# define S_TYPEISMQ(p) 0
#endif

#ifndef S_TYPEISTMO
# define S_TYPEISTMO(p) 0
#endif


#ifndef S_TYPEISSEM
# ifdef S_INSEM
#  define S_TYPEISSEM(p) (S_ISNAM ((p)->st_mode) && (p)->st_rdev == S_INSEM)
# else
#  define S_TYPEISSEM(p) 0
# endif
#endif

#ifndef S_TYPEISSHM
# ifdef S_INSHD
#  define S_TYPEISSHM(p) (S_ISNAM ((p)->st_mode) && (p)->st_rdev == S_INSHD)
# else
#  define S_TYPEISSHM(p) 0
# endif
#endif

/* high performance ("contiguous data") */
#ifndef S_ISCTG
# define S_ISCTG(p) 0
#endif

/* Cray DMF (data migration facility): off line, with data  */
#ifndef S_ISOFD
# define S_ISOFD(p) 0
#endif

/* Cray DMF (data migration facility): off line, with no data  */
#ifndef S_ISOFL
# define S_ISOFL(p) 0
#endif

/* 4.4BSD whiteout */
#ifndef S_ISWHT
# define S_ISWHT(m) 0
#endif

/* If any of the following are undefined,
   define them to their de facto standard values.  */
#if !S_ISUID
# define S_ISUID 04000
#endif
#if !S_ISGID
# define S_ISGID 02000
#endif

/* S_ISVTX is a common extension to POSIX.  */
#ifndef S_ISVTX
# define S_ISVTX 01000
#endif

#if !S_IRUSR && S_IREAD
# define S_IRUSR S_IREAD
#endif
#if !S_IRUSR
# define S_IRUSR 00400
#endif
#if !S_IRGRP
# define S_IRGRP (S_IRUSR >> 3)
#endif
#if !S_IROTH
# define S_IROTH (S_IRUSR >> 6)
#endif

#if !S_IWUSR && S_IWRITE
# define S_IWUSR S_IWRITE
#endif
#if !S_IWUSR
# define S_IWUSR 00200
#endif
#if !S_IWGRP
# define S_IWGRP (S_IWUSR >> 3)
#endif
#if !S_IWOTH
# define S_IWOTH (S_IWUSR >> 6)
#endif

#if !S_IXUSR && S_IEXEC
# define S_IXUSR S_IEXEC
#endif
#if !S_IXUSR
# define S_IXUSR 00100
#endif
#if !S_IXGRP
# define S_IXGRP (S_IXUSR >> 3)
#endif
#if !S_IXOTH
# define S_IXOTH (S_IXUSR >> 6)
#endif

#if !S_IRWXU
# define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#endif
#if !S_IRWXG
# define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#endif
#if !S_IRWXO
# define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)
#endif

/* Although S_IXUGO and S_IRWXUGO are not specified by POSIX and are
   not implemented in GNU/Linux, some Gnulib-using apps use the macros.  */
#if !S_IXUGO
# define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif
#ifndef S_IRWXUGO
# define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO)
#endif

/* Macros for futimens and utimensat.  */
#ifndef UTIME_NOW
# define UTIME_NOW (-1)
# define UTIME_OMIT (-2)
#endif


#if @GNULIB_CHMOD@
# if @REPLACE_CHMOD@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef chmod
#   define chmod rpl_chmod
#  endif
_GL_FUNCDECL_RPL (chmod, int, (const char *filename, mode_t mode),
                               _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (chmod, int, (const char *filename, mode_t mode));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef chmod
#   define chmod _chmod
#  endif
/* Need to cast, because in mingw the last argument is 'int mode'.  */
_GL_CXXALIAS_MDA_CAST (chmod, int, (const char *filename, mode_t mode));
# else
_GL_CXXALIAS_SYS (chmod, int, (const char *filename, mode_t mode));
# endif
_GL_CXXALIASWARN (chmod);
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_CHMOD
_GL_WARN_ON_USE (chmod, "chmod has portability problems - "
                 "use gnulib module chmod for portability");
# endif
#elif @GNULIB_MDA_CHMOD@
/* On native Windows, map 'chmod' to '_chmod', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::chmod always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef chmod
#   define chmod _chmod
#  endif
/* Need to cast, because in mingw the last argument is 'int mode'.  */
_GL_CXXALIAS_MDA_CAST (chmod, int, (const char *filename, mode_t mode));
# else
_GL_CXXALIAS_SYS (chmod, int, (const char *filename, mode_t mode));
# endif
_GL_CXXALIASWARN (chmod);
#endif


#if @GNULIB_FCHMODAT@
# if @REPLACE_FCHMODAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef fchmodat
#   define fchmodat rpl_fchmodat
#  endif
_GL_FUNCDECL_RPL (fchmodat, int,
                  (int fd, char const *file, mode_t mode, int flag),
                  _GL_ARG_NONNULL ((2)) _GL_ATTRIBUTE_NODISCARD);
_GL_CXXALIAS_RPL (fchmodat, int,
                  (int fd, char const *file, mode_t mode, int flag));
# else
#  if !@HAVE_FCHMODAT@
_GL_FUNCDECL_SYS (fchmodat, int,
                  (int fd, char const *file, mode_t mode, int flag),
                  _GL_ARG_NONNULL ((2)) _GL_ATTRIBUTE_NODISCARD);
#  endif
_GL_CXXALIAS_SYS (fchmodat, int,
                  (int fd, char const *file, mode_t mode, int flag));
# endif
_GL_CXXALIASWARN (fchmodat);
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_FCHMODAT
_GL_WARN_ON_USE (fchmodat, "fchmodat is not portable - "
                 "use gnulib module openat for portability");
# endif
#endif


#if @GNULIB_FSTAT@
# if @REPLACE_FSTAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef fstat
#   define fstat rpl_fstat
#  endif
_GL_FUNCDECL_RPL (fstat, int, (int fd, struct stat *buf),
                              _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (fstat, int, (int fd, struct stat *buf));
# else
_GL_CXXALIAS_SYS (fstat, int, (int fd, struct stat *buf));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (fstat);
# endif
#elif @GNULIB_OVERRIDES_STRUCT_STAT@
# if !GNULIB_FSTAT
#  undef fstat
#  define fstat fstat_used_without_requesting_gnulib_module_fstat
# endif
#elif @WINDOWS_64_BIT_ST_SIZE@
/* Above, we define stat to _stati64.  */
# define fstat _fstati64
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_FSTAT
_GL_WARN_ON_USE (fstat, "fstat has portability problems - "
                 "use gnulib module fstat for portability");
# endif
#endif


#if @GNULIB_FSTATAT@
# if @REPLACE_FSTATAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef fstatat
#   define fstatat rpl_fstatat
#  endif
_GL_FUNCDECL_RPL (fstatat, int,
                  (int fd, char const *restrict name, struct stat *restrict st,
                   int flags),
                  _GL_ARG_NONNULL ((2, 3)));
_GL_CXXALIAS_RPL (fstatat, int,
                  (int fd, char const *restrict name, struct stat *restrict st,
                   int flags));
# else
#  if !@HAVE_FSTATAT@
_GL_FUNCDECL_SYS (fstatat, int,
                  (int fd, char const *restrict name, struct stat *restrict st,
                   int flags),
                  _GL_ARG_NONNULL ((2, 3)));
#  endif
_GL_CXXALIAS_SYS (fstatat, int,
                  (int fd, char const *restrict name, struct stat *restrict st,
                   int flags));
# endif
_GL_CXXALIASWARN (fstatat);
#elif @GNULIB_OVERRIDES_STRUCT_STAT@
# if !GNULIB_FSTATAT
#  undef fstatat
#  define fstatat fstatat_used_without_requesting_gnulib_module_fstatat
# endif
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_FSTATAT
_GL_WARN_ON_USE (fstatat, "fstatat is not portable - "
                 "use gnulib module openat for portability");
# endif
#endif


#if @GNULIB_FUTIMENS@
/* Use the rpl_ prefix also on Solaris <= 9, because on Solaris 9 our futimens
   implementation relies on futimesat, which on Solaris 10 makes an invocation
   to futimens that is meant to invoke the libc's futimens(), not gnulib's
   futimens().  */
# if @REPLACE_FUTIMENS@ || (!@HAVE_FUTIMENS@ && defined __sun)
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef futimens
#   define futimens rpl_futimens
#  endif
_GL_FUNCDECL_RPL (futimens, int, (int fd, struct timespec const times[2]), );
_GL_CXXALIAS_RPL (futimens, int, (int fd, struct timespec const times[2]));
# else
#  if !@HAVE_FUTIMENS@
_GL_FUNCDECL_SYS (futimens, int, (int fd, struct timespec const times[2]), );
#  endif
_GL_CXXALIAS_SYS (futimens, int, (int fd, struct timespec const times[2]));
# endif
# if __GLIBC__ >= 2 && @HAVE_FUTIMENS@
_GL_CXXALIASWARN (futimens);
# endif
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_FUTIMENS
_GL_WARN_ON_USE (futimens, "futimens is not portable - "
                 "use gnulib module futimens for portability");
# endif
#endif


#if @GNULIB_GETUMASK@
# if !@HAVE_GETUMASK@
#  if __GLIBC__ + (__GLIBC_MINOR__ >= 2) > 2
_GL_FUNCDECL_SYS (getumask, mode_t, (void), ) _GL_ATTRIBUTE_NOTHROW;
#  else
_GL_FUNCDECL_SYS (getumask, mode_t, (void), );
#  endif
# endif
_GL_CXXALIAS_SYS (getumask, mode_t, (void));
# if @HAVE_GETUMASK@
_GL_CXXALIASWARN (getumask);
# endif
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_GETUMASK
_GL_WARN_ON_USE (getumask, "getumask is not portable - "
                 "use gnulib module getumask for portability");
# endif
#endif


#if @GNULIB_LCHMOD@
/* Change the mode of FILENAME to MODE, without dereferencing it if FILENAME
   denotes a symbolic link.  */
# if !@HAVE_LCHMOD@ || defined __hpux
_GL_FUNCDECL_SYS (lchmod, int, (const char *filename, mode_t mode),
                               _GL_ARG_NONNULL ((1)));
# endif
_GL_CXXALIAS_SYS (lchmod, int, (const char *filename, mode_t mode));
_GL_CXXALIASWARN (lchmod);
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_LCHMOD
_GL_WARN_ON_USE (lchmod, "lchmod is unportable - "
                 "use gnulib module lchmod for portability");
# endif
#endif


#if @GNULIB_MKDIR@
# if @REPLACE_MKDIR@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef mkdir
#   define mkdir rpl_mkdir
#  endif
_GL_FUNCDECL_RPL (mkdir, int, (char const *name, mode_t mode),
                               _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (mkdir, int, (char const *name, mode_t mode));
# elif defined _WIN32 && !defined __CYGWIN__
/* mingw's _mkdir() function has 1 argument, but we pass 2 arguments.
   Additionally, it declares _mkdir (and depending on compile flags, an
   alias mkdir), only in the nonstandard includes <direct.h> and <io.h>,
   which are included above.  */
#  if !GNULIB_defined_rpl_mkdir
static int
rpl_mkdir (char const *name, mode_t mode)
{
  return _mkdir (name);
}
#   define GNULIB_defined_rpl_mkdir 1
#  endif
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef mkdir
#   define mkdir rpl_mkdir
#  endif
_GL_CXXALIAS_RPL (mkdir, int, (char const *name, mode_t mode));
# else
_GL_CXXALIAS_SYS (mkdir, int, (char const *name, mode_t mode));
# endif
_GL_CXXALIASWARN (mkdir);
#elif @GNULIB_MDA_MKDIR@
/* On native Windows, map 'mkdir' to '_mkdir', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::mkdir always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !GNULIB_defined_rpl_mkdir
static int
rpl_mkdir (char const *name, mode_t mode)
{
  return _mkdir (name);
}
#   define GNULIB_defined_rpl_mkdir 1
#  endif
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef mkdir
#   define mkdir rpl_mkdir
#  endif
_GL_CXXALIAS_RPL (mkdir, int, (char const *name, mode_t mode));
# else
_GL_CXXALIAS_SYS (mkdir, int, (char const *name, mode_t mode));
# endif
_GL_CXXALIASWARN (mkdir);
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_MKDIR
_GL_WARN_ON_USE (mkdir, "mkdir does not always support two parameters - "
                 "use gnulib module mkdir for portability");
# endif
#endif


#if @GNULIB_MKDIRAT@
# if !@HAVE_MKDIRAT@
_GL_FUNCDECL_SYS (mkdirat, int, (int fd, char const *file, mode_t mode),
                                _GL_ARG_NONNULL ((2)));
# endif
_GL_CXXALIAS_SYS (mkdirat, int, (int fd, char const *file, mode_t mode));
_GL_CXXALIASWARN (mkdirat);
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_MKDIRAT
_GL_WARN_ON_USE (mkdirat, "mkdirat is not portable - "
                 "use gnulib module openat for portability");
# endif
#endif


#if @GNULIB_MKFIFO@
# if @REPLACE_MKFIFO@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef mkfifo
#   define mkfifo rpl_mkfifo
#  endif
_GL_FUNCDECL_RPL (mkfifo, int, (char const *file, mode_t mode),
                               _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (mkfifo, int, (char const *file, mode_t mode));
# else
#  if !@HAVE_MKFIFO@
_GL_FUNCDECL_SYS (mkfifo, int, (char const *file, mode_t mode),
                               _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (mkfifo, int, (char const *file, mode_t mode));
# endif
_GL_CXXALIASWARN (mkfifo);
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_MKFIFO
_GL_WARN_ON_USE (mkfifo, "mkfifo is not portable - "
                 "use gnulib module mkfifo for portability");
# endif
#endif


#if @GNULIB_MKFIFOAT@
# if @REPLACE_MKFIFOAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef mkfifoat
#   define mkfifoat rpl_mkfifoat
#  endif
_GL_FUNCDECL_RPL (mkfifoat, int, (int fd, char const *file, mode_t mode),
                                 _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (mkfifoat, int, (int fd, char const *file, mode_t mode));
# else
#  if !@HAVE_MKFIFOAT@
_GL_FUNCDECL_SYS (mkfifoat, int, (int fd, char const *file, mode_t mode),
                                 _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (mkfifoat, int, (int fd, char const *file, mode_t mode));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (mkfifoat);
# endif
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_MKFIFOAT
_GL_WARN_ON_USE (mkfifoat, "mkfifoat is not portable - "
                 "use gnulib module mkfifoat for portability");
# endif
#endif


#if @GNULIB_MKNOD@
# if @REPLACE_MKNOD@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef mknod
#   define mknod rpl_mknod
#  endif
_GL_FUNCDECL_RPL (mknod, int, (char const *file, mode_t mode, dev_t dev),
                              _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (mknod, int, (char const *file, mode_t mode, dev_t dev));
# else
#  if !@HAVE_MKNOD@
_GL_FUNCDECL_SYS (mknod, int, (char const *file, mode_t mode, dev_t dev),
                              _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (mknod, int, (char const *file, mode_t mode, dev_t dev));
# endif
_GL_CXXALIASWARN (mknod);
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_MKNOD
_GL_WARN_ON_USE (mknod, "mknod is not portable - "
                 "use gnulib module mknod for portability");
# endif
#endif


#if @GNULIB_MKNODAT@
# if @REPLACE_MKNODAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef mknodat
#   define mknodat rpl_mknodat
#  endif
_GL_FUNCDECL_RPL (mknodat, int,
                  (int fd, char const *file, mode_t mode, dev_t dev),
                  _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (mknodat, int,
                  (int fd, char const *file, mode_t mode, dev_t dev));
# else
#  if !@HAVE_MKNODAT@
_GL_FUNCDECL_SYS (mknodat, int,
                  (int fd, char const *file, mode_t mode, dev_t dev),
                  _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (mknodat, int,
                  (int fd, char const *file, mode_t mode, dev_t dev));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (mknodat);
# endif
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_MKNODAT
_GL_WARN_ON_USE (mknodat, "mknodat is not portable - "
                 "use gnulib module mkfifoat for portability");
# endif
#endif


#if @GNULIB_STAT@
# if @REPLACE_STAT@
#  if !@GNULIB_OVERRIDES_STRUCT_STAT@
    /* We can't use the object-like #define stat rpl_stat, because of
       struct stat.  This means that rpl_stat will not be used if the user
       does (stat)(a,b).  Oh well.  */
#   if defined _AIX && defined stat && defined _LARGE_FILES
     /* With _LARGE_FILES defined, AIX (only) defines stat to stat64,
        so we have to replace stat64() instead of stat(). */
#    undef stat64
#    define stat64(name, st) rpl_stat (name, st)
#   elif @WINDOWS_64_BIT_ST_SIZE@
     /* Above, we define stat to _stati64.  */
#    if defined __MINGW32__ && defined _stati64
#     ifdef _USE_32BIT_TIME_T
       /* The system headers possibly define _stati64 to _stat32i64.  */
#      undef _stat32i64
#      define _stat32i64(name, st) rpl_stat (name, st)
#     else
       /* The system headers define _stati64 to _stat64.  */
#      undef _stat64
#      define _stat64(name, st) rpl_stat (name, st)
#     endif
#    elif defined _MSC_VER && defined _stati64
#     ifdef _USE_32BIT_TIME_T
       /* The system headers define _stati64 to _stat32i64.  */
#      undef _stat32i64
#      define _stat32i64(name, st) rpl_stat (name, st)
#     else
       /* The system headers define _stati64 to _stat64.  */
#      undef _stat64
#      define _stat64(name, st) rpl_stat (name, st)
#     endif
#    else
#     undef _stati64
#     define _stati64(name, st) rpl_stat (name, st)
#    endif
#   elif defined __MINGW32__ && defined stat
#    ifdef _USE_32BIT_TIME_T
      /* The system headers define stat to _stat32i64.  */
#     undef _stat32i64
#     define _stat32i64(name, st) rpl_stat (name, st)
#    else
      /* The system headers define stat to _stat64.  */
#     undef _stat64
#     define _stat64(name, st) rpl_stat (name, st)
#    endif
#   elif defined _MSC_VER && defined stat
#    ifdef _USE_32BIT_TIME_T
      /* The system headers define stat to _stat32.  */
#     undef _stat32
#     define _stat32(name, st) rpl_stat (name, st)
#    else
      /* The system headers define stat to _stat64i32.  */
#     undef _stat64i32
#     define _stat64i32(name, st) rpl_stat (name, st)
#    endif
#   else /* !(_AIX || __MINGW32__ || _MSC_VER) */
#    undef stat
#    define stat(name, st) rpl_stat (name, st)
#   endif /* !_LARGE_FILES */
#  endif /* !@GNULIB_OVERRIDES_STRUCT_STAT@ */
_GL_EXTERN_C int stat (const char *restrict name, struct stat *restrict buf)
                      _GL_ARG_NONNULL ((1, 2));
# endif
#elif @GNULIB_OVERRIDES_STRUCT_STAT@
/* see above:
  #define stat stat_used_without_requesting_gnulib_module_stat
 */
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_STAT
_GL_WARN_ON_USE (stat, "stat is unportable - "
                 "use gnulib module stat for portability");
# endif
#endif


#if @GNULIB_LSTAT@
# if ! @HAVE_LSTAT@
/* mingw does not support symlinks, therefore it does not have lstat.  But
   without links, stat does just fine.  */
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define lstat stat
#  endif
_GL_CXXALIAS_RPL_1 (lstat, stat, int,
                    (const char *restrict name, struct stat *restrict buf));
# elif @REPLACE_LSTAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef lstat
#   define lstat rpl_lstat
#  endif
_GL_FUNCDECL_RPL (lstat, int,
                  (const char *restrict name, struct stat *restrict buf),
                  _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (lstat, int,
                  (const char *restrict name, struct stat *restrict buf));
# else
_GL_CXXALIAS_SYS (lstat, int,
                  (const char *restrict name, struct stat *restrict buf));
# endif
# if @HAVE_LSTAT@
_GL_CXXALIASWARN (lstat);
# endif
#elif @GNULIB_OVERRIDES_STRUCT_STAT@
# if !GNULIB_LSTAT
#  undef lstat
#  define lstat lstat_used_without_requesting_gnulib_module_lstat
# endif
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_LSTAT
_GL_WARN_ON_USE (lstat, "lstat is unportable - "
                 "use gnulib module lstat for portability");
# endif
#endif


#if @GNULIB_MDA_UMASK@
/* On native Windows, map 'umask' to '_umask', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::umask always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef umask
#   define umask _umask
#  endif
/* Need to cast, because in mingw the last argument is 'int mode'.  */
_GL_CXXALIAS_MDA_CAST (umask, mode_t, (mode_t mask));
# else
_GL_CXXALIAS_SYS (umask, mode_t, (mode_t mask));
# endif
_GL_CXXALIASWARN (umask);
#endif


#if @GNULIB_UTIMENSAT@
/* Use the rpl_ prefix also on Solaris <= 9, because on Solaris 9 our utimensat
   implementation relies on futimesat, which on Solaris 10 makes an invocation
   to utimensat that is meant to invoke the libc's utimensat(), not gnulib's
   utimensat().  */
# if @REPLACE_UTIMENSAT@ || (!@HAVE_UTIMENSAT@ && defined __sun)
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef utimensat
#   define utimensat rpl_utimensat
#  endif
_GL_FUNCDECL_RPL (utimensat, int, (int fd, char const *name,
                                   struct timespec const times[2], int flag),
                                  _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (utimensat, int, (int fd, char const *name,
                                   struct timespec const times[2], int flag));
# else
#  if !@HAVE_UTIMENSAT@
_GL_FUNCDECL_SYS (utimensat, int, (int fd, char const *name,
                                   struct timespec const times[2], int flag),
                                  _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (utimensat, int, (int fd, char const *name,
                                   struct timespec const times[2], int flag));
# endif
# if __GLIBC__ >= 2 && @HAVE_UTIMENSAT@
_GL_CXXALIASWARN (utimensat);
# endif
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_UTIMENSAT
_GL_WARN_ON_USE (utimensat, "utimensat is not portable - "
                 "use gnulib module utimensat for portability");
# endif
#endif


#endif /* _@GUARD_PREFIX@_SYS_STAT_H */
#endif /* _@GUARD_PREFIX@_SYS_STAT_H */
#endif
