/* Internal functions for line breaking of Unicode strings.
   Copyright (C) 2001-2003, 2005-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2021.

   This file is free software.
   It is dual-licensed under "the GNU LGPLv3+ or the GNU GPLv2+".
   You can redistribute it and/or modify it under either
     - the terms of the GNU Lesser General Public License as published
       by the Free Software Foundation, either version 3, or (at your
       option) any later version, or
     - the terms of the GNU General Public License as published by the
       Free Software Foundation; either version 2, or (at your option)
       any later version, or
     - the same dual license "the GNU LGPLv3+ or the GNU GPLv2+".

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License and the GNU General Public License
   for more details.

   You should have received a copy of the GNU Lesser General Public
   License and of the GNU General Public License along with this
   program.  If not, see <https://www.gnu.org/licenses/>.  */

extern void
       u8_possible_linebreaks_loop (const uint8_t *s, size_t n,
                                    const char *encoding, int cr,
                                    char *_UC_RESTRICT p);
extern void
       u16_possible_linebreaks_loop (const uint16_t *s, size_t n,
                                     const char *encoding, int cr,
                                     char *_UC_RESTRICT p);
extern void
       u32_possible_linebreaks_loop (const uint32_t *s, size_t n,
                                     const char *encoding, int cr,
                                     char *_UC_RESTRICT p);

extern int
       u8_width_linebreaks_internal (const uint8_t *s, size_t n,
                                     int width, int start_column, int at_end_columns,
                                     const char *o, const char *encoding, int cr,
                                     char *p);
