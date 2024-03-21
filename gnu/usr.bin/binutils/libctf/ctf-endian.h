/* Interface to endianness-neutrality functions.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef _CTF_ENDIAN_H
#define _CTF_ENDIAN_H

#include "config.h"
#include <stdint.h>
#include "swap.h"

#if !defined (HAVE_ENDIAN_H) || !defined (htole64)
#ifndef WORDS_BIGENDIAN
# define htole64(x) (x)
# define le64toh(x) (x)
#else
# define htole64(x) bswap_64 ((x))
# define le64toh(x) bswap_64 ((x))
#endif /* WORDS_BIGENDIAN */
#endif /* !defined(HAVE_ENDIAN_H) */

#endif /* !defined(_CTF_ENDIAN_H) */
