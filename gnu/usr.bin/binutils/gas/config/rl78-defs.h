/* rl78-defs.h Renesas RL78 internal definitions
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef RL78_DEFS_H
#define RL78_DEFS_H

/* Third operand to rl78_op.  */
#define RL78REL_DATA		0
#define RL78REL_PCREL		1

#define RL78_RELAX_NONE		0
#define RL78_RELAX_BRANCH	1

extern int    rl78_error (const char *);
extern void   rl78_lex_init (char *, char *);
extern void   rl78_prefix (int);
extern int    rl78_has_prefix (void);
extern void   rl78_base1 (int);
extern void   rl78_base2 (int, int);
extern void   rl78_base3 (int, int, int);
extern void   rl78_base4 (int, int, int, int);
extern void   rl78_field (int, int, int);
extern void   rl78_op (expressionS, int, int);
extern void   rl78_disp3 (expressionS, int);
extern void   rl78_field5s (expressionS);
extern void   rl78_field5s2 (expressionS);
extern void   rl78_relax (int, int);
extern void   rl78_linkrelax_addr16 (void);
extern void   rl78_linkrelax_branch (void);
extern int    rl78_parse (void);
extern int    rl78_wrap (void);

extern int    rl78_isa_g10 (void);
extern int    rl78_isa_g13 (void);
extern int    rl78_isa_g14 (void);

extern char * rl78_lex_start;
extern char * rl78_lex_end;
#endif
