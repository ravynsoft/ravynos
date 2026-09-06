/* Set the error indicator of a stream.
   Copyright (C) 2007, 2009-2026 Free Software Foundation, Inc.

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

#ifndef _FSETERR_H
#define _FSETERR_H

/* This file uses HAVE___FSETERR.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdio.h>

/* Set the error indicator of the stream FP.
   The "error indicator" is set when an I/O operation on the stream fails, and
   is cleared (together with the "end-of-file" indicator) by clearerr (FP).  */

#if HAVE___FSETERR /* musl libc */

/* Haiku has __fseterr but does not declare it.  */
# if defined __HAIKU__
extern void __fseterr (FILE *fp);
# endif

# include <stdio_ext.h>
# define fseterr(fp) __fseterr (fp)

#else

# ifdef __cplusplus
extern "C" {
# endif

extern void fseterr (FILE *fp);

# ifdef __cplusplus
}
# endif

#endif

#endif /* _FSETERR_H */
