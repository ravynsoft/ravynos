#ifndef foogccmacrohfoo
#define foogccmacrohfoo

/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

/** \file gccmacro.h Defines some macros for GCC extensions */

#include <avahi-common/cdecl.h>

AVAHI_C_DECL_BEGIN

#if defined(__GNUC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 3)
#define AVAHI_GCC_ALLOC_SIZE(x) __attribute__ ((__alloc_size__(x)))
#define AVAHI_GCC_ALLOC_SIZE2(x,y) __attribute__ ((__alloc_size__(x,y)))
#else
/** Macro for usage of GCC's alloc_size attribute */
#define AVAHI_GCC_ALLOC_SIZE(x)
#define AVAHI_GCC_ALLOC_SIZE2(x,y)
#endif

#if defined(__GNUC__) && (__GNUC__ >= 4)
#define AVAHI_GCC_SENTINEL __attribute__ ((sentinel))
#else
/** Macro for usage of GCC's sentinel compilation warnings */
#define AVAHI_GCC_SENTINEL
#endif

#ifdef __GNUC__
#define AVAHI_GCC_PRINTF_ATTR(a,b) __attribute__ ((format (printf, a, b)))
#else
/** Macro for usage of GCC's printf compilation warnings */
#define AVAHI_GCC_PRINTF_ATTR(a,b)
#endif

/** Same as AVAHI_GCC_PRINTF_ATTR but hard coded to arguments 1 and 2 */
#define AVAHI_GCC_PRINTF_ATTR12 AVAHI_GCC_PRINTF_ATTR(1,2)

/** Same as AVAHI_GCC_PRINTF_ATTR but hard coded to arguments 2 and 3 */
#define AVAHI_GCC_PRINTF_ATTR23 AVAHI_GCC_PRINTF_ATTR(2,3)

#ifdef __GNUC__
#define AVAHI_GCC_NORETURN __attribute__((noreturn))
#else
/** Macro for no-return functions */
#define AVAHI_GCC_NORETURN
#endif

#ifdef __GNUC__
#define AVAHI_GCC_UNUSED __attribute__ ((unused))
#else
/** Macro for not used parameter */
#define AVAHI_GCC_UNUSED
#endif

AVAHI_C_DECL_END

#endif
