/* Checking of messages in PO files.
   Copyright (C) 2005, 2008, 2010, 2015 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2005.

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

#ifndef _MSGL_CHECK_H
#define _MSGL_CHECK_H 1

#include "message.h"
#include "pos.h"
#include "plural-eval.h"
#include "plural-distrib.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Check the values returned by plural_eval.
   Signals the errors through po_xerror.
   Return the number of errors that were seen.
   If no errors, returns in *DISTRIBUTION information about the plural_eval
   values distribution.  */
extern int check_plural_eval (const struct expression *plural_expr,
                              unsigned long nplurals_value,
                              const message_ty *header,
                              struct plural_distribution *distribution);

/* Perform all checks on a non-obsolete message.  */
extern int check_message (const message_ty *mp,
                          const lex_pos_ty *msgid_pos,
                          int check_newlines,
                          int check_format_strings,
                          const struct plural_distribution *distribution,
                          int check_header,
                          int check_compatibility,
                          int check_accelerators, char accelerator_char);

/* Perform all checks on a message list.
   Return the number of errors that were seen.  */
extern int check_message_list (message_list_ty *mlp,
                               int ignore_untranslated_messages,
                               int ignore_fuzzy_messages,
                               int check_newlines,
                               int check_format_strings,
                               int check_header,
                               int check_compatibility,
                               int check_accelerators, char accelerator_char);

/* Perform all syntax checks on a message list.
   Return the number of errors that were seen.  */
extern int syntax_check_message_list (message_list_ty *mlp);

#ifdef __cplusplus
}
#endif

#endif /* _MSGL_CHECK_H */
