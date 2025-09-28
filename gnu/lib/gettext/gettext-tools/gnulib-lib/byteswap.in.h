/* byteswap.h - Byte swapping
   Copyright (C) 2005, 2007, 2009-2023 Free Software Foundation, Inc.
   Written by Oskar Liljeblad <oskar@osk.mine.nu>, 2005.

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

#ifndef _GL_BYTESWAP_H
#define _GL_BYTESWAP_H

/* Given an unsigned 16-bit argument X, return the value corresponding to
   X with reversed byte order.  */
#define bswap_16(x) ((((x) & 0x00FF) << 8) | \
                     (((x) & 0xFF00) >> 8))

/* Given an unsigned 32-bit argument X, return the value corresponding to
   X with reversed byte order.  */
#define bswap_32(x) ((((x) & 0x000000FF) << 24) | \
                     (((x) & 0x0000FF00) << 8) | \
                     (((x) & 0x00FF0000) >> 8) | \
                     (((x) & 0xFF000000) >> 24))

/* Given an unsigned 64-bit argument X, return the value corresponding to
   X with reversed byte order.  */
#define bswap_64(x) ((((x) & 0x00000000000000FFULL) << 56) | \
                     (((x) & 0x000000000000FF00ULL) << 40) | \
                     (((x) & 0x0000000000FF0000ULL) << 24) | \
                     (((x) & 0x00000000FF000000ULL) << 8) | \
                     (((x) & 0x000000FF00000000ULL) >> 8) | \
                     (((x) & 0x0000FF0000000000ULL) >> 24) | \
                     (((x) & 0x00FF000000000000ULL) >> 40) | \
                     (((x) & 0xFF00000000000000ULL) >> 56))

#endif /* _GL_BYTESWAP_H */
