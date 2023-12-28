/* Main header file for the bfd library -- portable access to object files.

   Copyright (C) 1990-2023 Free Software Foundation, Inc.

   Contributed by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef __BFD_H_SEEN__
#define __BFD_H_SEEN__

/* PR 14072: Ensure that config.h is included first.  */
#if !defined PACKAGE && !defined PACKAGE_VERSION
#error config.h must be included before this header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ansidecl.h"
#include "symcat.h"
#include <stdint.h>
#include <stdbool.h>
#include "diagnostics.h"
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#if defined (__STDC__) || defined (ALMOST_STDC) || defined (HAVE_STRINGIZE)
#ifndef SABER
/* This hack is to avoid a problem with some strict ANSI C preprocessors.
   The problem is, "32_" is not a valid preprocessing token, and we don't
   want extra underscores (e.g., "nlm_32_").  The XCONCAT2 macro will
   cause the inner CONCAT2 macros to be evaluated first, producing
   still-valid pp-tokens.  Then the final concatenation can be done.  */
#undef CONCAT4
#define CONCAT4(a,b,c,d) XCONCAT2(CONCAT2(a,b),CONCAT2(c,d))
#endif
#endif

/* This is a utility macro to handle the situation where the code
   wants to place a constant string into the code, followed by a
   comma and then the length of the string.  Doing this by hand
   is error prone, so using this macro is safer.  */
#define STRING_COMMA_LEN(STR) (STR), (sizeof (STR) - 1)

#define BFD_SUPPORTS_PLUGINS @supports_plugins@

/* The word size used by BFD on the host.  This may be 64 with a 32
   bit target if the host is 64 bit, or if other 64 bit targets have
   been selected with --enable-targets, or if --enable-64-bit-bfd.  */
#define BFD_ARCH_SIZE @wordsize@

/* The word size of the default bfd target.  */
#define BFD_DEFAULT_TARGET_SIZE @bfd_default_target_size@

#include <inttypes.h>

#if BFD_ARCH_SIZE >= 64
#define BFD64
#endif

/* Boolean type used in bfd.
   General rule: Functions which are bfd_boolean return TRUE on
   success and FALSE on failure (unless they're a predicate).  */

#ifdef POISON_BFD_BOOLEAN
# pragma GCC poison bfd_boolean
#else
# define bfd_boolean bool
# undef FALSE
# undef TRUE
# define FALSE 0
# define TRUE 1
#endif

/* Silence "applying zero offset to null pointer" UBSAN warnings.  */
#define PTR_ADD(P,A) ((A) != 0 ? (P) + (A) : (P))
/* Also prevent non-zero offsets from being applied to a null pointer.  */
#define NPTR_ADD(P,A) ((P) != NULL ? (P) + (A) : (P))

#ifdef BFD64

/* Represent a target address.  Also used as a generic unsigned type
   which is guaranteed to be big enough to hold any arithmetic types
   we need to deal with.  */
typedef uint64_t bfd_vma;

/* A generic signed type which is guaranteed to be big enough to hold any
   arithmetic types we need to deal with.  Can be assumed to be compatible
   with bfd_vma in the same way that signed and unsigned ints are compatible
   (as parameters, in assignment, etc).  */
typedef int64_t bfd_signed_vma;

typedef uint64_t bfd_size_type;
typedef uint64_t symvalue;

#else /* not BFD64  */

typedef uint32_t bfd_vma;
typedef int32_t bfd_signed_vma;
typedef uint32_t bfd_size_type;
typedef uint32_t symvalue;

#endif /* not BFD64  */

#define HALF_BFD_SIZE_TYPE \
  (((bfd_size_type) 1) << (8 * sizeof (bfd_size_type) / 2))

/* An offset into a file.  BFD always uses the largest possible offset
   based on the build time availability of fseek, fseeko, or fseeko64.  */
typedef @bfd_file_ptr@ file_ptr;
typedef @bfd_ufile_ptr@ ufile_ptr;

typedef uint32_t flagword;	/* 32 bits of flags */
typedef uint8_t bfd_byte;

/* Forward declarations.  */
typedef struct bfd bfd;
struct bfd_link_info;
struct bfd_link_hash_entry;
typedef struct bfd_section *sec_ptr;
typedef struct reloc_cache_entry arelent;
struct orl;

#define	align_power(addr, align)	\
  (((addr) + ((bfd_vma) 1 << (align)) - 1) & (-((bfd_vma) 1 << (align))))

/* Align an address upward to a boundary, expressed as a number of bytes.
   E.g. align to an 8-byte boundary with argument of 8.  Take care never
   to wrap around if the address is within boundary-1 of the end of the
   address space.  */
#define BFD_ALIGN(this, boundary)					  \
  ((((bfd_vma) (this) + (boundary) - 1) >= (bfd_vma) (this))		  \
   ? (((bfd_vma) (this) + ((boundary) - 1)) & ~ (bfd_vma) ((boundary)-1)) \
   : ~ (bfd_vma) 0)

/* Return TRUE if the start of STR matches PREFIX, FALSE otherwise.  */

static inline bool
startswith (const char *str, const char *prefix)
{
  return strncmp (str, prefix, strlen (prefix)) == 0;
}

