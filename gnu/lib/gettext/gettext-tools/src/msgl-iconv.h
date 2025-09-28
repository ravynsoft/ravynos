/* Message list character set conversion.
   Copyright (C) 2001-2003, 2005-2006, 2009, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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

#ifndef _MSGL_ICONV_H
#define _MSGL_ICONV_H

#include <stdbool.h>
#if HAVE_ICONV
#include <iconv.h>
#endif

#include "string-desc.h"

#include "message.h"


#ifdef __cplusplus
extern "C" {
#endif


#if HAVE_ICONV

/* A context, used for accurate error messages.  */
struct conversion_context
{
  const char *from_code;     /* canonicalized encoding name for input */
  const char *to_code;       /* canonicalized encoding name for output */
  const char *from_filename; /* file name where the input comes from */
  const message_ty *message; /* message being converted, or NULL */
};

/* Converts the STRING through the conversion descriptor CD.
   Assumes that either FROM_CODE or TO_CODE is UTF-8.  */
extern char *convert_string_directly (iconv_t cd, const char *string,
                                      const struct conversion_context* context);
extern string_desc_t
       convert_string_desc_directly (iconv_t cd, string_desc_t string,
                                     const struct conversion_context* context);

#endif

/* Converts the message list MLP to the (already canonicalized) encoding
   CANON_TO_CODE.  The (already canonicalized) encoding before conversion
   can be passed as CANON_FROM_CODE; if NULL is passed instead, the
   encoding is looked up in the header entry.  Returns true if and only if
   some msgctxt or msgid changed due to the conversion.  */
extern bool
       iconv_message_list (message_list_ty *mlp,
                           const char *canon_from_code,
                           const char *canon_to_code,
                           const char *from_filename);

/* Converts all the message lists in MDLP to the encoding TO_CODE.
   UPDATE_HEADER specifies whether to update the "charset=..." specification
   in the header; it should normally be true.  */
extern msgdomain_list_ty *
       iconv_msgdomain_list (msgdomain_list_ty *mdlp,
                             const char *to_code,
                             bool update_header,
                             const char *from_filename);

/* Tests whether the message list MLP could be converted to CANON_TO_CODE.
   The (already canonicalized) encoding before conversion can be passed as
   CANON_FROM_CODE; if NULL is passed instead, the encoding is looked up
   in the header entry.  */
extern bool
       is_message_list_iconvable (message_list_ty *mlp,
                                  const char *canon_from_code,
                                  const char *canon_to_code);


#ifdef __cplusplus
}
#endif


#endif /* _MSGL_ICONV_H */
