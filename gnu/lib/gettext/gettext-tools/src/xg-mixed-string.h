/* Handling strings that are given partially in the source encoding and
   partially in Unicode.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _XGETTEXT_MIXED_STRING_H
#define _XGETTEXT_MIXED_STRING_H

#include <stdbool.h>
#include <stddef.h>

#include "xg-encoding.h"

#ifdef __cplusplus
extern "C" {
#endif


/* A string that contains segments in the xgettext_current_source_encoding
   and segments in UTF-8, in an alternating way.  */

enum segment_type
{
  source_encoded,
  utf8_encoded
};

struct mixed_string_segment
{
  /*enum segment_type*/ unsigned char type;
  size_t length;
  char contents[FLEXIBLE_ARRAY_MEMBER];
};

typedef struct mixed_string mixed_string_ty;
struct mixed_string
{
  /* The alternating segments.  */
  struct mixed_string_segment **segments;
  size_t nsegments;
  /* The lexical context.  Used only for error message purposes.  */
  lexical_context_ty lcontext;
  const char *logical_file_name;
  int line_number;
};

/* Creates a mixed_string that contains just a string in the
   xgettext_current_source_encoding.  */
extern mixed_string_ty *
       mixed_string_alloc_simple (const char *string,
                                  lexical_context_ty lcontext,
                                  const char *logical_file_name,
                                  int line_number);

/* Creates a mixed_string that contains just a UTF-8 string.  */
extern mixed_string_ty *
       mixed_string_alloc_utf8 (const char *string,
                                lexical_context_ty lcontext,
                                const char *logical_file_name,
                                int line_number);

/* Creates a copy of a mixed_string.  */
extern mixed_string_ty *
       mixed_string_clone (const mixed_string_ty *ms1);

/* Returns the contents of a mixed_string as an UTF-8 encoded string.
   This may provoke an error if no source encoding has been specified
   through --from-code.  The result is freshly allocated.  */
extern char *
       mixed_string_contents (const mixed_string_ty *ms);

/* Frees a mixed_string.  */
extern void
       mixed_string_free (mixed_string_ty *ms);

/* Returns the contents of a mixed_string as an UTF-8 encoded string,
   and frees the argument.  */
extern char *
       mixed_string_contents_free1 (mixed_string_ty *ms);

/* Concatenates two mixed_strings.  */
extern mixed_string_ty *
       mixed_string_concat (const mixed_string_ty *ms1,
                            const mixed_string_ty *ms2);
/* Concatenates two mixed_strings, and frees the first argument.  */
extern mixed_string_ty *
       mixed_string_concat_free1 (mixed_string_ty *ms1,
                                  const mixed_string_ty *ms2);


/* A string buffer type that allows appending bytes (in the
   xgettext_current_source_encoding) or Unicode characters.
   When done, it returns the entire string as a mixed_string.  */

struct mixed_string_buffer
{
  /* The alternating segments that are already finished.  */
  struct mixed_string_segment **segments;
  size_t nsegments;
  size_t nsegments_allocated;
  /* The segment that is being accumulated.  */
  int curr_type; /* An enum segment_type, or -1. */
  char *curr_buffer;
  size_t curr_buflen;
  size_t curr_allocated;
  /* The first half of an UTF-16 surrogate character.  */
  unsigned short utf16_surr;
  /* The lexical context.  Used only for error message purposes.  */
  lexical_context_ty lcontext;
  const char *logical_file_name;
  int line_number;
};

/* Initializes a mixed_string_buffer.  */
extern void
       mixed_string_buffer_init (struct mixed_string_buffer *bp,
                                 lexical_context_ty lcontext,
                                 const char *logical_file_name,
                                 int line_number);

/* Determines whether a mixed_string_buffer is still empty.  */
extern bool
       mixed_string_buffer_is_empty (const struct mixed_string_buffer *bp);

/* Appends a character to a mixed_string_buffer.  */
extern void
       mixed_string_buffer_append_char (struct mixed_string_buffer *bp, int c);

/* Appends a Unicode character to a mixed_string_buffer.  */
extern void
       mixed_string_buffer_append_unicode (struct mixed_string_buffer *bp,
                                           int c);

/* Frees the memory pointed to by a 'struct mixed_string_buffer' and
   discards the accumulated string.  */
extern void
       mixed_string_buffer_destroy (struct mixed_string_buffer *bp);

/* Frees the memory pointed to by a 'struct mixed_string_buffer'
   and returns the accumulated string.  */
extern mixed_string_ty *
       mixed_string_buffer_result (struct mixed_string_buffer *bp);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_MIXED_STRING_H */
