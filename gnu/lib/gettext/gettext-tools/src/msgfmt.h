/* msgfmt specifics
   Copyright (C) 1995-1998, 2000-2001, 2009, 2015 Free Software
   Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, April 1995.

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

#ifndef _MSGFMT_H
#define _MSGFMT_H

#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Be more verbose.  Use only 'fprintf' and 'multiline_warning' but not
   'error' or 'multiline_error' to emit verbosity messages, because 'error'
   and 'multiline_error' during PO file parsing cause the program to exit
   with EXIT_FAILURE.  See function lex_end().  */
extern int verbose;

/* Data types for bulk operation mode.  */
typedef struct msgfmt_operand_ty msgfmt_operand_ty;
struct msgfmt_operand_ty
{
  char *language;
  message_list_ty *mlp;
};

typedef struct msgfmt_operand_list_ty msgfmt_operand_list_ty;
struct msgfmt_operand_list_ty
{
  msgfmt_operand_ty *items;
  size_t nitems;
  size_t nitems_max;
};

#ifdef __cplusplus
}
#endif

#endif /* _MSGFMT_H */
