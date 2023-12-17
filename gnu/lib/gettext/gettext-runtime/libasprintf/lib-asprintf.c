/* Library functions for class autosprintf.
   Copyright (C) 2002-2003, 2006, 2018-2019, 2021 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2002.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#if !(HAVE_VASPRINTF && HAVE_POSIX_PRINTF)

/* Define to the same symbols as in lib-asprintf.h.  */
#define asprintf libasprintf_asprintf
#define vasprintf libasprintf_vasprintf

/* Define functions declared in "vasprintf.h".  */
#include "vasprintf.c"
#include "asprintf.c"

/* Define the same functions also without the 'libasprintf_' prefix,
   for binary backward-compatibility.
   But don't redefine functions already defined by mingw.  */
#if !(defined __MINGW32__ && __USE_MINGW_ANSI_STDIO)
#undef asprintf
#undef vasprintf
#include "vasprintf.c"
#include "asprintf.c"
#endif

#endif
