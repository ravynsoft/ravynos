/* watch.h - definitions for tables for keeping track of allocated memory */

/*  Copyright (C) 2001-2020 Free Software Foundation, Inc.

    This file is part of GNU Bash, the Bourne-Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MWATCH_H
#define _MWATCH_H

#include "imalloc.h"

#ifdef MALLOC_WATCH

/* `Events' for watchpoints */

#define W_ALLOC		0x01
#define W_FREE		0x02
#define W_REALLOC	0x04
#define W_RESIZED	0x08

extern int _malloc_nwatch;

extern void _malloc_ckwatch PARAMS((PTR_T, const char *, int, int, unsigned long));
                    
#endif /* MALLOC_WATCH */

#endif /* _MWATCH_H */
