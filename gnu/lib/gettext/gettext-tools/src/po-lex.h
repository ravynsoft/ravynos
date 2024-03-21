/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2006, 2012 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

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

#ifndef _PO_LEX_H
#define _PO_LEX_H

#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include "error.h"
#include "error-progname.h"
#include "xerror.h"
#include "pos.h"

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || __STRICT_ANSI__
#  define __attribute__(Spec) /* empty */
# endif
/* The __-protected variants of 'format' and 'printf' attributes
   are accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#  define __format__ format
#  define __printf__ printf
# endif
#endif


#ifdef __cplusplus
extern "C" {
#endif


/* Lexical analyzer for reading PO files.  */


/* Global variables from po-lex.c.  */

/* Current position within the PO file.  */
extern DLL_VARIABLE lex_pos_ty gram_pos;
extern DLL_VARIABLE int gram_pos_column;

/* Number of parse errors within a PO file that cause the program to
   terminate.  Cf. error_message_count, declared in <error.h>.  */
extern DLL_VARIABLE unsigned int gram_max_allowed_errors;

/* True if obsolete entries shall be considered as valid.  */
extern DLL_VARIABLE bool pass_obsolete_entries;


/* Prepare lexical analysis.  */
extern void lex_start (FILE *fp, const char *real_filename,
                       const char *logical_filename);

/* Terminate lexical analysis.  */
extern void lex_end (void);

/* Return the next token in the PO file.  The return codes are defined
   in "po-gram-gen2.h".  Associated data is put in 'po_gram_lval.  */
extern int po_gram_lex (void);

/* po_gram_lex() can return comments as COMMENT.  Switch this on or off.  */
extern void po_lex_pass_comments (bool flag);

/* po_gram_lex() can return obsolete entries as if they were normal entries.
   Switch this on or off.  */
extern void po_lex_pass_obsolete_entries (bool flag);

extern void po_gram_error (const char *fmt, ...)
       __attribute__ ((__format__ (__printf__, 1, 2)));
extern void po_gram_error_at_line (const lex_pos_ty *pos, const char *fmt, ...)
       __attribute__ ((__format__ (__printf__, 2, 3)));


/* Contains information about the definition of one translation.  */
struct msgstr_def
{
  char *msgstr;
  size_t msgstr_len;
};


#ifdef __cplusplus
}
#endif


#endif
