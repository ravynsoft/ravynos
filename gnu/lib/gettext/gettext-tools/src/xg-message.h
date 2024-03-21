/* Extracting a message.  Accumulating the message list.
   Copyright (C) 2001-2020 Free Software Foundation, Inc.

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

#ifndef _XGETTEXT_MESSAGE_H
#define _XGETTEXT_MESSAGE_H

#include <stdbool.h>

#include "message.h"
#include "pos.h"
#include "rc-str-list.h"

#include "xg-arglist-context.h"
#include "xg-encoding.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Add a message to the list of extracted messages.
   MSGCTXT must be either NULL or a malloc()ed string; its ownership is passed
   to the callee.
   MSGID must be a malloc()ed string; its ownership is passed to the callee.
   IS_UTF8 must be true if MSGCTXT and MSGID have already been converted to
   UTF-8.
   PLURALP must be true if and only if a call to remember_a_message_plural will
   follow.
   POS->file_name must be allocated with indefinite extent.
   EXTRACTED_COMMENT is a comment that needs to be copied into the POT file,
   or NULL.
   COMMENT may be savable_comment, or it may be a saved copy of savable_comment
   (then add_reference must be used when saving it, and drop_reference while
   dropping it).  Clear savable_comment.
   COMMENT_IS_UTF8 must be true if COMMENT has already been converted to UTF-8.
   Return the new or found message, or NULL if the message is excluded.  */
extern message_ty *remember_a_message (message_list_ty *mlp,
                                       char *msgctxt,
                                       char *msgid,
                                       bool is_utf8,
                                       bool pluralp,
                                       flag_context_ty context,
                                       lex_pos_ty *pos,
                                       const char *extracted_comment,
                                       refcounted_string_list_ty *comment,
                                       bool comment_is_utf8);

/* Add an msgid_plural to a message previously returned by
   remember_a_message.
   STRING must be a malloc()ed string; its ownership is passed to the callee.
   IS_UTF8 must be true if STRING has already been converted to UTF-8.
   POS->file_name must be allocated with indefinite extent.
   COMMENT may be savable_comment, or it may be a saved copy of savable_comment
   (then add_reference must be used when saving it, and drop_reference while
   dropping it).  Clear savable_comment.
   COMMENT_IS_UTF8 must be true if COMMENT has already been converted to UTF-8.
 */
extern void remember_a_message_plural (message_ty *mp,
                                       char *string,
                                       bool is_utf8,
                                       flag_context_ty context,
                                       lex_pos_ty *pos,
                                       refcounted_string_list_ty *comment,
                                       bool comment_is_utf8);

/* The following functions are used by remember_a_message.
   Most extractors don't need to invoke them explicitly.  */

/* Eliminates the 'undecided' values in mp->is_format.  */
extern void decide_is_format (message_ty *mp);

/* Adds a range restriction to mp->range.  */
extern void intersect_range (message_ty *mp, const struct argument_range *range);

/* Eliminates the 'undecided' value in mp->do_wrap.  */
extern void decide_do_wrap (message_ty *mp);

/* Eliminates the 'undecided' values in mp->syntax_check.  */
extern void decide_syntax_check (message_ty *mp);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_MESSAGE_H */
