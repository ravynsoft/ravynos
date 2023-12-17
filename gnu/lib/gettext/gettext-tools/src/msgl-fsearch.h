/* Fast fuzzy searching among messages.
   Copyright (C) 2006, 2008 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifndef _MSGL_FSEARCH_H
#define _MSGL_FSEARCH_H 1

#include "message.h"

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/* A fuzzy index is a data structure that corresponds to a set of messages,
   allowing for fuzzy searching of a message.  It is optimized for large sets
   of messages.  */
typedef struct message_fuzzy_index_ty message_fuzzy_index_ty;

/* Allocate a fuzzy index corresponding to a given list of messages.
   The list of messages and the msgctxt and msgid fields of the messages
   inside it must not be modified while the returned fuzzy index is in use.  */
extern message_fuzzy_index_ty *
       message_fuzzy_index_alloc (const message_list_ty *mlp,
                                  const char *canon_charset);

/* Find a good match for the given msgctxt and msgid in the given fuzzy index.
   The match does not need to be optimal.
   Ignore matches for which the fuzzy_search_goal_function is < LOWER_BOUND.
   LOWER_BOUND must be >= FUZZY_THRESHOLD.
   If HEURISTIC is true, only the few best messages among the list - according
   to a certain heuristic - are considered.  If HEURISTIC is false, all
   messages with a fuzzy_search_goal_function > FUZZY_THRESHOLD are considered,
   like in message_list_search_fuzzy (except that in ambiguous cases where
   several best matches exist, message_list_search_fuzzy chooses the one with
   the smallest index whereas message_fuzzy_index_search makes a better
   choice).  */
extern message_ty *
       message_fuzzy_index_search (message_fuzzy_index_ty *findex,
                                   const char *msgctxt, const char *msgid,
                                   double lower_bound,
                                   bool heuristic);

/* Free a fuzzy index.  */
extern void
       message_fuzzy_index_free (message_fuzzy_index_ty *findex);


#ifdef __cplusplus
}
#endif

#endif /* _MSGL_FSEARCH_H */
