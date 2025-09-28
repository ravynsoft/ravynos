/* Take file names apart into directory and base names.

   Copyright (C) 1998, 2001, 2003-2006, 2009-2023 Free Software Foundation,
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

#ifndef DIRNAME_H_
# define DIRNAME_H_ 1

/* This file uses _GL_ATTRIBUTE_MALLOC, _GL_ATTRIBUTE_PURE,
   _GL_ATTRIBUTE_RETURNS_NONNULL.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

# include <stdlib.h>
# include "filename.h"
# include "basename-lgpl.h"

# ifndef DIRECTORY_SEPARATOR
#  define DIRECTORY_SEPARATOR '/'
# endif

#ifdef __cplusplus
extern "C" {
#endif

# if GNULIB_DIRNAME
char *base_name (char const *file)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_RETURNS_NONNULL;
char *dir_name (char const *file)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_RETURNS_NONNULL;
# endif

char *mdir_name (char const *file)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;
size_t dir_len (char const *file) _GL_ATTRIBUTE_PURE;

bool strip_trailing_slashes (char *file);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* not DIRNAME_H_ */
