/* String descriptors.
   Copyright (C) 2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#ifndef _STRING_DESC_H
#define _STRING_DESC_H 1

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE,
   _GL_ATTRIBUTE_NODISCARD.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get ptrdiff_t.  */
#include <stddef.h>

/* Get FILE.  */
#include <stdio.h>

/* Get abort(), free().  */
#include <stdlib.h>

/* Get idx_t.  */
#include "idx.h"


_GL_INLINE_HEADER_BEGIN
#ifndef GL_STRING_DESC_INLINE
# define GL_STRING_DESC_INLINE _GL_INLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* Type describing a string that may contain NUL bytes.
   It's merely a descriptor of an array of bytes.  */
typedef struct string_desc_t string_desc_t;
struct string_desc_t
{
  /* The fields of this struct should be considered private.  */
  idx_t _nbytes;
  char *_data;
};

/* String descriptors can be passed and returned by value.

   String descriptors and NUL-terminated 'const char *'/'char *' C strings
   cannot be used interchangeably.  You will get compilation errors if you
   attempt to assign a string descriptor to a C string or vice versa.  */


/* ==== Side-effect-free operations on string descriptors ==== */

/* Return the length of the string S.  */
#if 0 /* Defined inline below.  */
extern idx_t string_desc_length (string_desc_t s);
#endif

/* Return the byte at index I of string S.
   I must be < length(S).  */
#if 0 /* Defined inline below.  */
extern char string_desc_char_at (string_desc_t s, idx_t i);
#endif

/* Return a read-only view of the bytes of S.  */
#if 0 /* Defined inline below.  */
extern const char * string_desc_data (string_desc_t s);
#endif

/* Return true if S is the empty string.  */
#if 0 /* Defined inline below.  */
extern bool string_desc_is_empty (string_desc_t s);
#endif

/* Return true if A and B are equal.  */
extern bool string_desc_equals (string_desc_t a, string_desc_t b);

/* Return true if S starts with PREFIX.  */
extern bool string_desc_startswith (string_desc_t s, string_desc_t prefix);

/* Return true if S ends with SUFFIX.  */
extern bool string_desc_endswith (string_desc_t s, string_desc_t suffix);

/* Return > 0, == 0, or < 0 if A > B, A == B, A < B.
   This uses a lexicographic ordering, where the bytes are compared as
   'unsigned char'.  */
extern int string_desc_cmp (string_desc_t a, string_desc_t b);

/* Return the index of the first occurrence of C in S,
   or -1 if there is none.  */
extern ptrdiff_t string_desc_index (string_desc_t s, char c);

/* Return the index of the last occurrence of C in S,
   or -1 if there is none.  */
extern ptrdiff_t string_desc_last_index (string_desc_t s, char c);

/* Return the index of the first occurrence of NEEDLE in HAYSTACK,
   or -1 if there is none.  */
extern ptrdiff_t string_desc_contains (string_desc_t haystack, string_desc_t needle);

/* Return an empty string.  */
extern string_desc_t string_desc_new_empty (void);

/* Return a string that represents the C string S, of length strlen (S).  */
extern string_desc_t string_desc_from_c (const char *s);

/* Return the substring of S, starting at offset START and ending at offset END.
   START must be <= END.
   The result is of length END - START.
   The result must not be freed (since its storage is part of the storage
   of S).  */
extern string_desc_t string_desc_substring (string_desc_t s, idx_t start, idx_t end);

/* Output S to the file descriptor FD.
   Return 0 if successful.
   Upon error, return -1 with errno set.  */
extern int string_desc_write (int fd, string_desc_t s);

/* Output S to the FILE stream FP.
   Return 0 if successful.
   Upon error, return -1.  */
extern int string_desc_fwrite (FILE *fp, string_desc_t s);


/* ==== Memory-allocating operations on string descriptors ==== */

/* Construct a string of length N, with uninitialized contents.
   Return 0 if successful.
   Upon error, return -1 with errno set.  */
_GL_ATTRIBUTE_NODISCARD
extern int string_desc_new (string_desc_t *resultp, idx_t n);

/* Construct and return a string of length N, at the given memory address.  */
extern string_desc_t string_desc_new_addr (idx_t n, char *addr);

/* Construct a string of length N, filled with C.
   Return 0 if successful.
   Upon error, return -1 with errno set.  */
_GL_ATTRIBUTE_NODISCARD
extern int string_desc_new_filled (string_desc_t *resultp, idx_t n, char c);

/* Construct a copy of string S.
   Return 0 if successful.
   Upon error, return -1 with errno set.  */
_GL_ATTRIBUTE_NODISCARD
extern int string_desc_copy (string_desc_t *resultp, string_desc_t s);

/* Construct the concatenation of N strings.  N must be > 0.
   Return 0 if successful.
   Upon error, return -1 with errno set.  */
_GL_ATTRIBUTE_NODISCARD
extern int string_desc_concat (string_desc_t *resultp, idx_t n, string_desc_t string1, ...);

/* Construct a copy of string S, as a NUL-terminated C string.
   Return it is successful.
   Upon error, return NULL with errno set.  */
extern char * string_desc_c (string_desc_t s) _GL_ATTRIBUTE_DEALLOC_FREE;


/* ==== Operations with side effects on string descriptors ==== */

/* Overwrite the byte at index I of string S with C.
   I must be < length(S).  */
extern void string_desc_set_char_at (string_desc_t s, idx_t i, char c);

/* Fill part of S, starting at offset START and ending at offset END,
   with copies of C.
   START must be <= END.  */
extern void string_desc_fill (string_desc_t s, idx_t start, idx_t end, char c);

/* Overwrite part of S with T, starting at offset START.
   START + length(T) must be <= length (S).  */
extern void string_desc_overwrite (string_desc_t s, idx_t start, string_desc_t t);

/* Free S.  */
extern void string_desc_free (string_desc_t s);


/* ==== Inline function definitions ==== */

GL_STRING_DESC_INLINE idx_t
string_desc_length (string_desc_t s)
{
  return s._nbytes;
}

GL_STRING_DESC_INLINE char
string_desc_char_at (string_desc_t s, idx_t i)
{
  if (!(i >= 0 && i < s._nbytes))
    /* Invalid argument.  */
    abort ();
  return s._data[i];
}

GL_STRING_DESC_INLINE const char *
string_desc_data (string_desc_t s)
{
  return s._data;
}

GL_STRING_DESC_INLINE bool
string_desc_is_empty (string_desc_t s)
{
  return s._nbytes == 0;
}


#ifdef __cplusplus
}
#endif

_GL_INLINE_HEADER_END


#endif /* _STRING_DESC_H */
