/* hppa-aux.h -- Assembler for the PA - PA-RISC specific support routines
   Copyright (C) 1989 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*
   HP PA-RISC support was contributed by the Center for Software Science
   at the University of Utah.
 */

/* HP-PA support for Mach-O    ... USV */

#ifndef HPPA_AUX_DEFINED
#define HPPA_AUX_DEFINED

typedef enum FPOF { SGL, DBL, ILLEGAL_FMT, QUAD } FP_Operand_Format;

int pa_parse_number();

int pa_parse_fp_cmp_cond();

FP_Operand_Format pa_parse_fp_format();

int pa_parse_nullif();

int pa_parse_nonneg_cmpsub_cmpltr();

int pa_parse_neg_cmpsub_cmpltr();

int pa_parse_nonneg_add_cmpltr();

int pa_parse_neg_add_cmpltr();

/* A structure used during assembly of individual instructions */

struct pa_it {
    char    *error;
    uint32_t opcode;
#ifdef undef
    symbol_dictS *nlistp;      /*** used to be:    struct nlist *nlistp; */
#else
    nlist_t *nlistp;
#endif
    expressionS exp;
    int pcrel;
    int pcrel_reloc;
    FP_Operand_Format fpof1;   /* Floating Point Operand Format, operand 1 */
    FP_Operand_Format fpof2;   /* Floating Point Operand Format, operand 2 */
                               /* (used only for class 1 instructions --   */
			       /* the conversion instructions)             */
    int32_t field_selector;
    unsigned int reloc;
/*	enum reloc_type_hppa reloc; */
    int code;
    int32_t arg_reloc;
};

extern struct pa_it the_insn;
  
/*
  PA-89 floating point registers are arranged like this:


  +--------------+--------------+
  |   0 or 16L   |  16 or 16R   |
  +--------------+--------------+
  |   1 or 17L   |  17 or 17R   |
  +--------------+--------------+
  |              |              |

  .              .              .
  .              .              .
  .              .              .

  |              |              |
  +--------------+--------------+
  |  14 or 30L   |  30 or 30R   |
  +--------------+--------------+
  |  15 or 31L   |  31 or 31R   |
  +--------------+--------------+


  The following is a version of pa_parse_number that
  handles the L/R notation and returns the correct
  value to put into the instruction register field.
  The correct value to put into the instruction is
  encoded in the structure 'pa_89_fp_reg_struct'.

 */

struct pa_89_fp_reg_struct {
  char number_part;
  char L_R_select;
};

int need_89_opcode();
int pa_89_parse_number();

extern int getAbsoluteExpression(
    char *str);
extern int evaluateAbsolute(
    expressionS exp,
    int field_selector);
extern int getExpression(
    char *str);

#endif /* HPPA_AUX_DEFINED */

/* end hppa-aux.h */
