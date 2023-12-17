/* inttostr.h -- convert integers to printable strings

   Copyright (C) 2001-2006, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Paul Eggert */

/* This file uses _GL_ATTRIBUTE_NODISCARD.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdint.h>
#include <sys/types.h>

#include "intprops.h"

_GL_ATTRIBUTE_NODISCARD char *imaxtostr (intmax_t, char *);
_GL_ATTRIBUTE_NODISCARD char *inttostr (int, char *);
_GL_ATTRIBUTE_NODISCARD char *offtostr (off_t, char *);
_GL_ATTRIBUTE_NODISCARD char *uinttostr (unsigned int, char *);
_GL_ATTRIBUTE_NODISCARD char *umaxtostr (uintmax_t, char *);
