/* Resolving ambiguity of argument lists: Progressive parsing of an
   argument list, keeping track of all possibilities.
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

#ifndef _XGETTEXT_ARGLIST_PARSER_H
#define _XGETTEXT_ARGLIST_PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "pos.h"
#include "rc-str-list.h"
#include "str-list.h"

#include "xg-mixed-string.h"
#include "xg-arglist-context.h"
#include "xg-arglist-callshape.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Represents the progressive parsing of an argument list w.r.t. a single
   'struct callshape'.  */
struct partial_call
{
  int argnumc;                  /* number of context argument, 0 when seen */
  int argnum1;                  /* number of singular argument, 0 when seen */
  int argnum2;                  /* number of plural argument, 0 when seen */
  bool argnum1_glib_context;    /* argument argnum1 has the syntax "ctxt|msgid" */
  bool argnum2_glib_context;    /* argument argnum2 has the syntax "ctxt|msgid" */
  int argtotal;                 /* total number of arguments, 0 if unspecified */
  string_list_ty xcomments;     /* auto-extracted comments */
  mixed_string_ty *msgctxt;     /* context - owned mixed_string, or NULL */
  lex_pos_ty msgctxt_pos;
  mixed_string_ty *msgid;       /* msgid - owned mixed_string, or NULL */
  flag_context_ty msgid_context;
  lex_pos_ty msgid_pos;
  refcounted_string_list_ty *msgid_comment;
  bool msgid_comment_is_utf8;
  mixed_string_ty *msgid_plural; /* msgid_plural - owned mixed_string, or NULL */
  flag_context_ty msgid_plural_context;
  lex_pos_ty msgid_plural_pos;
};

/* Represents the progressive parsing of an argument list w.r.t. an entire
   'struct callshapes'.  */
struct arglist_parser
{
  message_list_ty *mlp;         /* list where the message shall be added */
  const char *keyword;          /* the keyword, not NUL terminated */
  size_t keyword_len;           /* the keyword's length */
  bool next_is_msgctxt;         /* true if the next argument is the msgctxt */
  size_t nalternatives;         /* number of partial_call alternatives */
  struct partial_call alternative[FLEXIBLE_ARRAY_MEMBER]; /* partial_call alternatives */
};

/* Creates a fresh arglist_parser recognizing calls.
   You can pass shapes = NULL for a parser not recognizing any calls.  */
extern struct arglist_parser * arglist_parser_alloc (message_list_ty *mlp,
                                                     const struct callshapes *shapes);
/* Clones an arglist_parser.  */
extern struct arglist_parser * arglist_parser_clone (struct arglist_parser *ap);
/* Adds a string argument to an arglist_parser.  ARGNUM must be > 0.
   STRING must be a mixed_string; its ownership is passed to the callee.
   FILE_NAME must be allocated with indefinite extent.
   COMMENT may be savable_comment, or it may be a saved copy of savable_comment
   (then add_reference must be used when saving it, and drop_reference while
   dropping it).  Clear savable_comment.
   COMMENT_IS_UTF8 must be true if COMMENT has already been converted to UTF-8.
 */
extern void arglist_parser_remember (struct arglist_parser *ap,
                                     int argnum, mixed_string_ty *string,
                                     flag_context_ty context,
                                     const char *file_name, size_t line_number,
                                     refcounted_string_list_ty *comment,
                                     bool comment_is_utf8);
/* Adds a string argument as msgctxt to an arglist_parser, without incrementing
   the current argument number.
   STRING must be a mixed_string; its ownership is passed to the callee.
   FILE_NAME must be allocated with indefinite extent.  */
extern void arglist_parser_remember_msgctxt (struct arglist_parser *ap,
                                             mixed_string_ty *string,
                                             flag_context_ty context,
                                             const char *file_name, size_t line_number);
/* Tests whether an arglist_parser has is not waiting for more arguments after
   argument ARGNUM.  */
extern bool arglist_parser_decidedp (struct arglist_parser *ap, int argnum);
/* Terminates the processing of an arglist_parser after argument ARGNUM and
   deletes it.  */
extern void arglist_parser_done (struct arglist_parser *ap, int argnum);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_ARGLIST_PARSER_H */
