/* xgettext common functions.
   Copyright (C) 2001-2003, 2005-2006, 2008-2009, 2011, 2013-2014, 2018, 2020, 2023 Free Software Foundation, Inc.
   Written by Peter Miller <millerp@canb.auug.org.au>
   and Bruno Haible <haible@clisp.cons.org>, 2001.

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

#ifndef _XGETTEXT_H
#define _XGETTEXT_H

#include <stdbool.h>
#include <stddef.h>

#include "message.h"
#include "rc-str-list.h"

#ifdef __cplusplus
extern "C" {
#endif


/* If true, add all comments immediately preceding one of the keywords. */
extern bool add_all_comments;

/* Tag used in comment of prevailing domain.  */
extern char *comment_tag;

/* List of messages whose msgids must not be extracted, or NULL.
   Used by remember_a_message().  */
extern message_list_ty *exclude;

/* String used as prefix for msgstr.  */
extern const char *msgstr_prefix;

/* String used as suffix for msgstr.  */
extern const char *msgstr_suffix;

/* If true, omit the header entry.
   If false, keep the header entry present in the input.  */
extern int xgettext_omit_header;

/* Be more verbose.  */
extern int verbose;

extern enum is_syntax_check default_syntax_check[NSYNTAXCHECKS];

/* Language dependent format string parser.
   NULL if the language has no notion of format strings.  */
extern struct formatstring_parser *current_formatstring_parser1;
extern struct formatstring_parser *current_formatstring_parser2;
extern struct formatstring_parser *current_formatstring_parser3;
extern struct formatstring_parser *current_formatstring_parser4;


/* Record a flag in the appropriate backend's table.
   OPTIONSTRING has the syntax WORD:ARG:FLAG (as documented)
   or                          WORD:ARG:FLAG!BACKEND.
   The latter syntax is undocumented and only needed for format string types
   that are used by multiple backends.  */
extern void xgettext_record_flag (const char *optionstring);


extern const char * xgettext_comment (size_t n);
extern void xgettext_comment_reset (void);

/* Comment handling for backends which support combining adjacent strings
   even across lines.
   In these backends we cannot use the xgettext_comment* functions directly,
   because in multiline string expressions like
           "string1" +
           "string2"
   the newline between "string1" and "string2" would cause a call to
   xgettext_comment_reset(), thus destroying the accumulated comments
   that we need a little later, when we have concatenated the two strings
   and pass them to remember_a_message().
   Instead, we do the bookkeeping of the accumulated comments directly,
   and save a pointer to the accumulated comments when we read "string1".
   In order to avoid excessive copying of strings, we use reference
   counting.  */

extern refcounted_string_list_ty *savable_comment;
extern void savable_comment_add (const char *str);
extern void savable_comment_reset (void);
extern void
       savable_comment_to_xgettext_comment (refcounted_string_list_ty *rslp);


extern bool recognize_qt_formatstrings (void);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_H */
