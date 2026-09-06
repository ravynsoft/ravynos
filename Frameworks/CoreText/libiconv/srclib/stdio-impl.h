/* Implementation details of FILE streams.
   Copyright (C) 2007-2008, 2010-2026 Free Software Foundation, Inc.

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

/* Many stdio implementations have the same logic and therefore can share
   the same implementation of stdio extension API, except that some fields
   have different naming conventions, or their access requires some casts.  */

/* Glibc 2.28 made _IO_UNBUFFERED and _IO_IN_BACKUP private.  For now, work
   around this problem by defining them ourselves.  FIXME: Do not rely on glibc
   internals.  */
#if defined _IO_EOF_SEEN
# if !defined _IO_UNBUFFERED
#  define _IO_UNBUFFERED 0x2
# endif
# if !defined _IO_IN_BACKUP
#  define _IO_IN_BACKUP 0x100
# endif
#endif

/* Haiku stdio implementation.  */
#if defined __HAIKU__
# include <stdint.h>
/* This FILE structure was made into an incomplete type in 2025.
   See <https://cgit.haiku-os.org/haiku/tree/src/system/libroot/posix/glibc/libio/libio.h>.  */
#  define fp_ ((struct { int _flags; \
                         char *_IO_read_ptr; \
                         char *_IO_read_end; \
                         char *_IO_read_base; \
                         char *_IO_write_base; \
                         char *_IO_write_ptr; \
                         char *_IO_write_end; \
                         char *_IO_buf_base; \
                         char *_IO_buf_end; \
                         char *_IO_save_base; \
                         char *_IO_backup_base; \
                         char *_IO_save_end; \
                         void *_markers; \
                         void *_chain; \
                         int _fileno; \
                         int _flags2; \
                         off_t _old_offset; \
                         unsigned short _cur_column; \
                         signed char _vtable_offset; \
                         char _shortbuf[1]; \
                         void *_lock; \
                         int64_t _offset; \
                         /* More fields, not relevant here.  */ \
                       } *) fp)
# if !defined _IO_UNBUFFERED
#  define _IO_UNBUFFERED 0x2
# endif
# if !defined _IO_EOF_SEEN
#  define _IO_EOF_SEEN 0x10
# endif
# if !defined _IO_IN_BACKUP
#  define _IO_IN_BACKUP 0x100
# endif
# if !defined _IO_LINE_BUF
#  define _IO_LINE_BUF 0x200
# endif
#endif

/* BSD stdio derived implementations.  */

#if defined __NetBSD__                         /* NetBSD */
/* Get __NetBSD_Version__.  */
# include <sys/param.h>
#endif

#include <errno.h>                             /* For detecting Plan9.  */

#if defined __sferror || defined __OpenBSD__ || defined __DragonFly__ || defined __ANDROID__
  /* FreeBSD, NetBSD, OpenBSD, DragonFly, Mac OS X, Cygwin, Minix 3, Android */

# if defined __DragonFly__          /* DragonFly */
  /* See <https://gitweb.dragonflybsd.org/dragonfly.git/blob_plain/HEAD:/lib/libc/stdio/priv_stdio.h>.  */
#  define fp_ ((struct { struct __FILE_public pub; \
                         struct { unsigned char *_base; int _size; } _bf; \
                         void *cookie; \
                         void *_close; \
                         void *_read; \
                         void *_seek; \
                         void *_write; \
                         struct { unsigned char *_base; int _size; } _ub; \
                         int _ur; \
                         unsigned char _ubuf[3]; \
                         unsigned char _nbuf[1]; \
                         struct { unsigned char *_base; int _size; } _lb; \
                         int _blksize; \
                         fpos_t _offset; \
                         /* More fields, not relevant here.  */ \
                       } *) fp)
  /* See <https://gitweb.dragonflybsd.org/dragonfly.git/blob_plain/HEAD:/include/stdio.h>.  */
#  define _p pub._p
#  define _flags pub._flags
#  define _r pub._r
#  define _w pub._w
# elif defined __OpenBSD__          /* OpenBSD */
#  if defined __sferror             /* OpenBSD <= 7.7 */
#   define _gl_flags_file_t short
#  else                             /* OpenBSD >= 7.8 */
#   define _gl_flags_file_t int
#  endif
  /* Up to this commit from 2025-07-16
     <https://github.com/openbsd/src/commit/b7f6c2eb760a2da367dd51d539ef06f5f3553790>
     the innards of FILE were public.  After this commit, the innards of FILE
     are hidden.  In this commit
     <https://github.com/openbsd/src/commit/9063a2f1ec94013fb0e2c7ec851495108e788a6e>
     they were reshuffled.  */
#  if defined __sferror             /* OpenBSD <= 7.7 */
#   define fp_ ((struct { unsigned char *_p; \
                          int _r; \
                          int _w; \
                          _gl_flags_file_t _flags; \
                          _gl_flags_file_t _file; \
                          struct { unsigned char *_base; size_t _size; } _bf; \
                          int _lbfsize; \
                          void *_cookie; \
                          void *_close; \
                          void *_read; \
                          void *_seek; \
                          void *_write; \
                          struct { unsigned char *_base; size_t _size; } _ext; \
                          unsigned char *_up; \
                          int _ur; \
                          unsigned char _ubuf[3]; \
                          unsigned char _nbuf[1]; \
                          struct { unsigned char *_base; size_t _size; } _lb; \
                          int _blksize; \
                          fpos_t _offset; \
                          /* More fields, not relevant here.  */ \
                        } *) fp)
#  else                             /* OpenBSD >= 7.8 */
#   define fp_ ((struct { _gl_flags_file_t _flags; \
                          _gl_flags_file_t _file; \
                          unsigned char *_p; \
                          int _r; \
                          int _w; \
                          struct { unsigned char *_base; size_t _size; } _bf; \
                          int _lbfsize; \
                          /* More fields, not relevant here.  */ \
                        } *) fp)
#  endif
# elif defined __ANDROID__          /* Android */
#  if defined __LP64__
#   define _gl_flags_file_t int
#  else
#   define _gl_flags_file_t short
#  endif
#  if defined __LP64__
#   define _gl_file_offset_t int64_t
#  else
    /* see https://android.googlesource.com/platform/bionic/+/master/docs/32-bit-abi.md */
#   define _gl_file_offset_t __kernel_off_t
#  endif
  /* Up to this commit from 2015-10-12
     <https://android.googlesource.com/platform/bionic.git/+/f0141dfab10a4b332769d52fa76631a64741297a>
     the innards of FILE were public,
     see <https://android.googlesource.com/platform/bionic.git/+/e78392637d5086384a5631ddfdfa8d7ec8326ee3/libc/stdio/fileext.h>
     and <https://android.googlesource.com/platform/bionic.git/+/e78392637d5086384a5631ddfdfa8d7ec8326ee3/libc/stdio/local.h>.
     After this commit, the innards of FILE are hidden.  */
#  define fp_ ((struct { unsigned char *_p; \
                         int _r; \
                         int _w; \
                         _gl_flags_file_t _flags; \
                         _gl_flags_file_t _file; \
                         struct { unsigned char *_base; size_t _size; } _bf; \
                         int _lbfsize; \
                         void *_cookie; \
                         void *_close; \
                         void *_read; \
                         void *_seek; \
                         void *_write; \
                         struct { unsigned char *_base; size_t _size; } _ext; \
                         unsigned char *_up; \
                         int _ur; \
                         unsigned char _ubuf[3]; \
                         unsigned char _nbuf[1]; \
                         struct { unsigned char *_base; size_t _size; } _lb; \
                         int _blksize; \
                         _gl_file_offset_t _offset; \
                         /* More fields, not relevant here.  */ \
                       } *) fp)
# else
#  define fp_ fp
# endif

# if (defined __NetBSD__ && __NetBSD_Version__ >= 105270000) || defined __minix /* NetBSD >= 1.5ZA, Minix 3 */
  /* See <https://cvsweb.netbsd.org/bsdweb.cgi/src/lib/libc/stdio/fileext.h?rev=HEAD&content-type=text/x-cvsweb-markup>
     and <https://github.com/Stichting-MINIX-Research-Foundation/minix/blob/master/lib/libc/stdio/fileext.h> */
  struct __sfileext
    {
      struct  __sbuf _ub; /* ungetc buffer */
      /* More fields, not relevant here.  */
    };
#  define fp_ub ((struct __sfileext *) fp->_ext._base)->_ub
# elif defined __ANDROID__ || defined __OpenBSD__ /* Android, OpenBSD */
  struct __sfileext
    {
      struct { unsigned char *_base; size_t _size; } _ub; /* ungetc buffer */
      /* More fields, not relevant here.  */
    };
#  define fp_ub ((struct __sfileext *) fp_->_ext._base)->_ub
# else                                         /* FreeBSD, NetBSD <= 1.5Z, DragonFly, Mac OS X, Cygwin */
#  define fp_ub fp_->_ub
# endif

# define HASUB(fp) (fp_ub._base != NULL)

# if defined __ANDROID__ || defined __OpenBSD__ /* Android, OpenBSD */
  /* Needed after this Android commit from 2016-01-25
     <https://android.googlesource.com/platform/bionic.git/+/e70e0e9267d069bf56a5078c99307e08a7280de7>
     And after this OpenBSD commit from 2025-07-16
     <https://github.com/openbsd/src/commit/b7f6c2eb760a2da367dd51d539ef06f5f3553790>.  */
#  ifndef __SEOF
#   define __SLBF 1
#   define __SNBF 2
#   define __SRD 4
#   define __SWR 8
#   define __SRW 0x10
#   define __SEOF 0x20
#   define __SERR 0x40
#  endif
#  ifndef __SOFF
#   define __SOFF 0x1000
#  endif
# endif

#endif


/* SystemV derived implementations.  */

#ifdef __TANDEM                     /* NonStop Kernel */
# ifndef _IOERR
/* These values were determined by the program 'stdioext-flags' at
   <https://lists.gnu.org/r/bug-gnulib/2010-12/msg00165.html>.  */
#  define _IOERR   0x40
#  define _IOREAD  0x80
#  define _IOWRT    0x4
#  define _IORW   0x100
# endif
#endif

#if defined _IOERR

# if defined __sun && defined _LP64 /* Solaris/{SPARC,AMD64} 64-bit */
#  define fp_ ((struct { unsigned char *_ptr; \
                         unsigned char *_base; \
                         unsigned char *_end; \
                         long _cnt; \
                         int _file; \
                         unsigned int _flag; \
                       } *) fp)
# elif defined __VMS                /* OpenVMS */
#  define fp_ ((struct _iobuf *) fp)
# else
#  define fp_ fp
# endif

# if defined _SCO_DS || (defined __SCO_VERSION__ || defined __sysv5__)  /* OpenServer 5, OpenServer 6, UnixWare 7 */
#  define _cnt __cnt
#  define _ptr __ptr
#  define _base __base
#  define _flag __flag
# endif

#elif defined _WIN32 && ! defined __CYGWIN__  /* newer Windows with MSVC */

/* <stdio.h> does not define the innards of FILE any more.  */
# define WINDOWS_OPAQUE_FILE

struct _gl_real_FILE
{
  /* Note: Compared to older Windows and to mingw, it has the fields
     _base and _cnt swapped. */
  unsigned char *_ptr;
  unsigned char *_base;
  int _cnt;
  int _flag;
  int _file;
  int _charbuf;
  int _bufsiz;
};
# define fp_ ((struct _gl_real_FILE *) fp)

/* These values were determined by a program similar to the one at
   <https://lists.gnu.org/r/bug-gnulib/2010-12/msg00165.html>.  */
# define _IOREAD   0x1
# define _IOWRT    0x2
# define _IORW     0x4
# define _IOEOF    0x8
# define _IOERR   0x10

#endif
