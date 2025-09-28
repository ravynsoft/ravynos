/* Keeping track of the encoding of strings to be extracted.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.

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

#ifndef _XGETTEXT_ENCODING_H
#define _XGETTEXT_ENCODING_H

#include <stddef.h>

#if HAVE_ICONV
#include <iconv.h>
#endif

#include "string-desc.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Context while building up lexical tokens.  */
typedef enum
  {
    lc_outside, /* Initial context: outside of comments and strings.  */
    lc_comment, /* Inside a comment.  */
    lc_string,  /* Inside a string literal.  */

    /* For embedded XML in programming code, like E4X in JavaScript.  */
    lc_xml_open_tag,   /* Inside an opening tag of an XML element.  */
    lc_xml_close_tag,  /* Inside a closing tag of an XML element.  */
    lc_xml_content     /* Inside an XML text node.  */
  }
lexical_context_ty;

/* Error message about non-ASCII character in a specific lexical context.  */
extern char *non_ascii_error_message (lexical_context_ty lcontext,
                                      const char *file_name,
                                      size_t line_number);


/* Canonicalized encoding name for all input files.
   It can be NULL when the --from-code option has not been specified.  In this
   case, the default (ASCII or UTF-8) depends on the programming language.  */
extern const char *xgettext_global_source_encoding;

#if HAVE_ICONV
/* Converter from xgettext_global_source_encoding to UTF-8 (except from
   ASCII or UTF-8, when this conversion is a no-op).  */
extern iconv_t xgettext_global_source_iconv;
#endif

/* Canonicalized encoding name for the current input file.  */
extern const char *xgettext_current_source_encoding;

#if HAVE_ICONV
/* Converter from xgettext_current_source_encoding to UTF-8 (except from
   ASCII or UTF-8, when this conversion is a no-op).  */
extern iconv_t xgettext_current_source_iconv;
#endif

/* Convert the given string from xgettext_current_source_encoding to
   the output file encoding (i.e. ASCII or UTF-8).
   The resulting string is either the argument string, or freshly allocated.
   The lcontext, file_name and line_number are only used for error message
   purposes.  */
extern char *from_current_source_encoding (const char *string,
                                           lexical_context_ty lcontext,
                                           const char *file_name,
                                           size_t line_number);

/* Like from_current_source_encoding, for a string that may contain NULs.  */
extern string_desc_t
       string_desc_from_current_source_encoding (string_desc_t string,
                                                 lexical_context_ty lcontext,
                                                 const char *file_name,
                                                 size_t line_number);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_ENCODING_H */
