/* Read symbolic links without size limitation.

   Copyright (C) 2001, 2003-2004, 2007, 2009-2023 Free Software Foundation,
   Inc.

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

/* Written by Jim Meyering <jim@meyering.net>  */

#include <stdlib.h>

extern char *areadlink (char const *filename)
  _GL_ATTRIBUTE_DEALLOC_FREE;
extern char *areadlink_with_size (char const *filename, size_t size_hint)
  _GL_ATTRIBUTE_DEALLOC_FREE;

#if GNULIB_AREADLINKAT
extern char *areadlinkat (int fd, char const *filename)
  _GL_ATTRIBUTE_DEALLOC_FREE;
#endif

#if GNULIB_AREADLINKAT_WITH_SIZE
extern char *areadlinkat_with_size (int fd, char const *filename,
                                    size_t size_hint)
  _GL_ATTRIBUTE_DEALLOC_FREE;
#endif
