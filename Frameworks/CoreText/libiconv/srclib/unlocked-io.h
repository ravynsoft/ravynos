/* Prefer faster, non-thread-safe stdio functions if available.

   Copyright (C) 2001-2004, 2009-2026 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Jim Meyering.  */

#ifndef UNLOCKED_IO_H
# define UNLOCKED_IO_H 1

/* These are wrappers for functions/macros from the GNU C library, and
   from other C libraries supporting POSIX's optional thread-safe functions.

   The standard I/O functions are thread-safe.  These *_unlocked ones are
   more efficient but not thread-safe.  That they're not thread-safe is
   fine since all of the applications in this package are single threaded.

   Also, some code that is shared with the GNU C library may invoke
   the *_unlocked functions directly.  On hosts that lack those
   functions, invoke the non-thread-safe versions instead.  */

/* This file uses HAVE_DECL_*_UNLOCKED.  */
# if !_GL_CONFIG_H_INCLUDED
#  error "Please include config.h first."
# endif

# include <stdio.h>

# if HAVE_DECL_CLEARERR_UNLOCKED || defined clearerr_unlocked
#  undef clearerr
#  define clearerr(x) clearerr_unlocked (x)
# else
#  define clearerr_unlocked(x) clearerr (x)
# endif

# if HAVE_DECL_FEOF_UNLOCKED || defined feof_unlocked
#  undef feof
#  define feof(x) feof_unlocked (x)
# else
#  define feof_unlocked(x) feof (x)
# endif

# if HAVE_DECL_FERROR_UNLOCKED || defined ferror_unlocked
#  undef ferror
#  define ferror(x) ferror_unlocked (x)
# else
#  define ferror_unlocked(x) ferror (x)
# endif

# if HAVE_DECL_FFLUSH_UNLOCKED || defined fflush_unlocked
#  undef fflush
#  define fflush(x) fflush_unlocked (x)
# else
#  define fflush_unlocked(x) fflush (x)
# endif

# if HAVE_DECL_FGETS_UNLOCKED || defined fgets_unlocked
#  undef fgets
#  define fgets(x,y,z) fgets_unlocked (x,y,z)
# else
#  define fgets_unlocked(x,y,z) fgets (x,y,z)
# endif

# if HAVE_DECL_FILENO_UNLOCKED || defined fileno_unlocked
#  undef fileno
#  define fileno(x) fileno_unlocked (x)
# else
#  define fileno_unlocked(x) fileno (x)
# endif

# if HAVE_DECL_FPUTC_UNLOCKED || defined fputc_unlocked
#  undef fputc
#  define fputc(x,y) fputc_unlocked (x,y)
# else
#  define fputc_unlocked(x,y) fputc (x,y)
# endif

# if HAVE_DECL_FPUTS_UNLOCKED || defined fputs_unlocked
#  undef fputs
#  define fputs(x,y) fputs_unlocked (x,y)
# else
#  define fputs_unlocked(x,y) fputs (x,y)
# endif

# if HAVE_DECL_FREAD_UNLOCKED || defined fread_unlocked
#  undef fread
#  define fread(w,x,y,z) fread_unlocked (w,x,y,z)
# else
#  define fread_unlocked(w,x,y,z) fread (w,x,y,z)
# endif

# if HAVE_DECL_FWRITE_UNLOCKED || defined fwrite_unlocked
#  undef fwrite
#  define fwrite(w,x,y,z) fwrite_unlocked (w,x,y,z)
# else
#  define fwrite_unlocked(w,x,y,z) fwrite (w,x,y,z)
# endif

# if HAVE_DECL_GETC_UNLOCKED || defined getc_unlocked
#  undef getc
#  define getc(x) getc_unlocked (x)
# else
#  define getc_unlocked(x) getc (x)
# endif

# if HAVE_DECL_GETCHAR_UNLOCKED || defined getchar_unlocked
#  undef getchar
#  define getchar() getchar_unlocked ()
# else
#  define getchar_unlocked() getchar ()
# endif

# if HAVE_DECL_PUTC_UNLOCKED || defined putc_unlocked
#  undef putc
#  define putc(x,y) putc_unlocked (x,y)
# else
#  define putc_unlocked(x,y) putc (x,y)
# endif

# if HAVE_DECL_PUTCHAR_UNLOCKED || defined putchar_unlocked
#  undef putchar
#  define putchar(x) putchar_unlocked (x)
# else
#  define putchar_unlocked(x) putchar (x)
# endif

# undef flockfile
# define flockfile(x) ((void) 0)

# undef ftrylockfile
# define ftrylockfile(x) 0

# undef funlockfile
# define funlockfile(x) ((void) 0)

#endif /* UNLOCKED_IO_H */
