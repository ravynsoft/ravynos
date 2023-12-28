/* rx-defs.h Renesas RX internal definitions
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

#ifndef RX_DEFS_H
#define RX_DEFS_H

/* Third operand to rx_op.  */
#define RXREL_SIGNED		0
#define RXREL_UNSIGNED		1
#define RXREL_NEGATIVE		2
#define RXREL_PCREL		3
#define RXREL_NEGATIVE_BORROW	4

#define RX_RELAX_NONE	0
#define RX_RELAX_BRANCH	1
#define RX_RELAX_IMM	2
#define RX_RELAX_DISP	3

enum rx_cpu_types
{
  RX600,
  RX610,
  RX200,
  RX100,
  RXV2,
  RXV3,
  RXV3FPU,
};

extern int rx_pid_register;
extern int rx_gp_register;
extern enum rx_cpu_types rx_cpu;

extern int    rx_error (const char *);
extern void   rx_lex_init (char *, char *);
extern void   rx_base1 (int);
extern void   rx_base2 (int, int);
extern void   rx_base3 (int, int, int);
extern void   rx_base4 (int, int, int, int);
extern void   rx_field (int, int, int);
extern void   rx_op (expressionS, int, int);
extern void   rx_disp3 (expressionS, int);
extern void   rx_field5s (expressionS);
extern void   rx_field5s2 (expressionS);
extern void   rx_bfield (expressionS, expressionS, expressionS);
extern void   rx_relax (int, int);
extern void   rx_linkrelax_dsp (int);
extern void   rx_linkrelax_imm (int);
extern void   rx_linkrelax_branch (void);
extern int    rx_parse (void);
extern int    rx_wrap (void);
extern void   rx_note_string_insn_use (void);
extern void   rx_post (char);

extern char * rx_lex_start;
extern char * rx_lex_end;

#endif /* RX_DEFS_H */
