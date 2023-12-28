/* compress-debug.h - Header file for compressed debug sections.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef COMPRESS_DEBUG_H
#define COMPRESS_DEBUG_H

#include <stdbool.h>

struct z_stream_s;

/* Initialize the compression engine.  */
extern void *compress_init (bool);

/* Stream the contents of a frag to the compression engine.  Output
   from the engine goes into the current frag on the obstack.  */
extern int compress_data (bool, void *, const char **, int *, char **, int *);

/* Finish the compression and consume the remaining compressed output.  */
extern int
compress_finish (bool, void *, char **, int *, int *);

#endif /* COMPRESS_DEBUG_H */
