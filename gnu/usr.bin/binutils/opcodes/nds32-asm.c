/* NDS32-specific support for 32-bit ELF.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Andes Technology Corporation.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */


#include "sysdep.h"

#include <stdint.h>
#include <assert.h>

#include "safe-ctype.h"
#include "libiberty.h"
#include "hashtab.h"
#include "bfd.h"
#include "opintl.h"

#include <config.h>
#include <stdlib.h>
#include <string.h>

#include "opcode/nds32.h"
#include "nds32-asm.h"

/* There at at most MAX_LEX_NUM lexical elements in a syntax.  */
#define MAX_LEX_NUM		32
/* A operand in syntax string should be at most this long.  */
#define MAX_LEX_LEN		64
/* The max length of a keyword can be.  */
#define MAX_KEYWORD_LEN		32
/* This LEX is a plain char or operand.  */
#define IS_LEX_CHAR(c)		(((c) >> 7) == 0)
#define LEX_SET_FIELD(k,c)	((c) | (((k) + 1) << 8))
#define LEX_GET_FIELD(k,c)	(nds32_field_table[k])[((c) & 0xff)]
/* Get the char in this lexical element.  */
#define LEX_CHAR(c)		((c) & 0xff)

#define USRIDX(group, usr)	((group) | ((usr) << 5))
#define SRIDX(major, minor, ext) \
                           (((major) << 7) | ((minor) << 3) | (ext))

static int parse_re (struct nds32_asm_desc *, struct nds32_asm_insn *,
		     char **, int64_t *);
static int parse_re2 (struct nds32_asm_desc *, struct nds32_asm_insn *,
		      char **, int64_t *);
static int parse_fe5 (struct nds32_asm_desc *, struct nds32_asm_insn *,
		      char **, int64_t *);
static int parse_pi5 (struct nds32_asm_desc *, struct nds32_asm_insn *,
		      char **, int64_t *);
static int parse_aext_reg (struct nds32_asm_desc *, char **,
			   int *, int);
static int parse_a30b20 (struct nds32_asm_desc *, struct nds32_asm_insn *,
			 char **, int64_t *);
static int parse_rt21 (struct nds32_asm_desc *, struct nds32_asm_insn *,
		       char **, int64_t *);
static int parse_rte_start (struct nds32_asm_desc *, struct nds32_asm_insn *,
			    char **, int64_t *);
static int parse_rte_end (struct nds32_asm_desc *, struct nds32_asm_insn *,
			  char **, int64_t *);
static int parse_rte69_start (struct nds32_asm_desc *, struct nds32_asm_insn *,
			      char **, int64_t *);
static int parse_rte69_end (struct nds32_asm_desc *, struct nds32_asm_insn *,
			    char **, int64_t *);
static int parse_im5_ip (struct nds32_asm_desc *, struct nds32_asm_insn *,
			 char **, int64_t *);
static int parse_im5_mr (struct nds32_asm_desc *, struct nds32_asm_insn *,
			 char **, int64_t *);
static int parse_im6_ip (struct nds32_asm_desc *, struct nds32_asm_insn *,
			 char **, int64_t *);
static int parse_im6_iq (struct nds32_asm_desc *, struct nds32_asm_insn *,
			 char **, int64_t *);
static int parse_im6_mr (struct nds32_asm_desc *, struct nds32_asm_insn *,
			 char **, int64_t *);
static int parse_im6_ms (struct nds32_asm_desc *, struct nds32_asm_insn *,
			 char **, int64_t *);

/* These are operand prefixes for input/output semantic.

     %   input
     =   output
     &   both
     {}  optional operand

   Field table for operands and bit-fields.  */

const field_t nds32_operand_fields[] =
{
  {"rt",	20, 5, 0, HW_GPR, NULL},
  {"ra",	15, 5, 0, HW_GPR, NULL},
  {"rb",	10, 5, 0, HW_GPR, NULL},
  {"rd",	5, 5, 0, HW_GPR, NULL},
  {"re",	10, 5, 0, HW_GPR, parse_re},	/* lmw smw lmwa smwa.  */
  {"fst",	20, 5, 0, HW_FSR, NULL},
  {"fsa",	15, 5, 0, HW_FSR, NULL},
  {"fsb",	10, 5, 0, HW_FSR, NULL},
  {"fdt",	20, 5, 0, HW_FDR, NULL},
  {"fda",	15, 5, 0, HW_FDR, NULL},
  {"fdb",	10, 5, 0, HW_FDR, NULL},
  {"cprt",	20, 5, 0, HW_CPR, NULL},
  {"cp",	13, 2, 0, HW_CP, NULL},
  {"sh",	5, 5, 0, HW_UINT, NULL},	/* sh in ALU instructions.  */
  {"sv",	8, 2, 0, HW_UINT, NULL},	/* sv in MEM instructions.  */
  {"dt",	21, 1, 0, HW_DXR, NULL},
  {"usr",	10, 10, 0, HW_USR, NULL},	/* User Special Registers.  */
  {"sr",	10, 10, 0, HW_SR, NULL},	/* System Registers.  */
  {"ridx",	10, 10, 0, HW_UINT, NULL},	/* Raw value for mfusr/mfsr.  */
  {"enb4",	6, 4, 0, HW_UINT, NULL},	/* Enable4 for LSMW.  */
  {"swid",	5, 15, 0, HW_UINT, NULL},
  {"stdby_st",	5, 2, 0, HW_STANDBY_ST, NULL},
  {"tlbop_st",	5, 5, 0, HW_TLBOP_ST, NULL},
  {"tlbop_stx",	5, 5, 0, HW_UINT, NULL},
  {"cctl_st0",	5, 5, 0, HW_CCTL_ST0, NULL},
  {"cctl_st1",	5, 5, 0, HW_CCTL_ST1, NULL},
  {"cctl_st2",	5, 5, 0, HW_CCTL_ST2, NULL},
  {"cctl_st3",	5, 5, 0, HW_CCTL_ST3, NULL},
  {"cctl_st4",	5, 5, 0, HW_CCTL_ST4, NULL},
  {"cctl_st5",	5, 5, 0, HW_CCTL_ST5, NULL},
  {"cctl_stx",	5, 5, 0, HW_UINT, NULL},
  {"cctl_lv",	10, 1, 0, HW_CCTL_LV, NULL},
  {"msync_st",	5, 3, 0, HW_MSYNC_ST, NULL},
  {"msync_stx",	5, 3, 0, HW_UINT, NULL},
  {"dpref_st",	20, 4, 0, HW_DPREF_ST, NULL},
  {"rt5",	5, 5, 0, HW_GPR, NULL},
  {"ra5",	0, 5, 0, HW_GPR, NULL},
  {"rt4",	5, 4, 0, HW_GPR, NULL},
  {"rt3",	6, 3, 0, HW_GPR, NULL},
  {"rt38",	8, 3, 0, HW_GPR, NULL},	/* rt3 used in 38 form.  */
  {"ra3",	3, 3, 0, HW_GPR, NULL},
  {"rb3",	0, 3, 0, HW_GPR, NULL},
  {"rt5e",	4, 4, 1, HW_GPR, NULL},	/* for movd44.  */
  {"ra5e",	0, 4, 1, HW_GPR, NULL},	/* for movd44.  */
  {"re2",	5, 2, 0, HW_GPR, parse_re2},	/* re in push25/pop25.  */
  {"fe5",	0, 5, 2, HW_UINT, parse_fe5},	/* imm5u in lwi45.fe.  */
  {"pi5",	0, 5, 0, HW_UINT, parse_pi5},	/* imm5u in movpi45.  */
  {"abdim",	2, 3, 0, HW_ABDIM, NULL},	/* Flags for LSMW.  */
  {"abm",	2, 3, 0, HW_ABM, NULL},	/* Flags for LSMWZB.  */
  {"dtiton",	8, 2, 0, HW_DTITON, NULL},
  {"dtitoff",	8, 2, 0, HW_DTITOFF, NULL},

  {"i5s",	0, 5, 0, HW_INT, NULL},
  {"i10s",	0, 10, 0, HW_INT, NULL},
  {"i15s",	0, 15, 0, HW_INT, NULL},
  {"i19s",	0, 19, 0, HW_INT, NULL},
  {"i20s",	0, 20, 0, HW_INT, NULL},
  {"i8s1",	0, 8, 1, HW_INT, NULL},
  {"i11br3",	8, 11, 0, HW_INT, NULL},
  {"i14s1",	0, 14, 1, HW_INT, NULL},
  {"i15s1",	0, 15, 1, HW_INT, NULL},
  {"i16s1",	0, 16, 1, HW_INT, NULL},
  {"i16u5",	5, 16, 0, HW_UINT, NULL},
  {"i18s1",	0, 18, 1, HW_INT, NULL},
  {"i24s1",	0, 24, 1, HW_INT, NULL},
  {"i8s2",	0, 8, 2, HW_INT, NULL},
  {"i12s2",	0, 12, 2, HW_INT, NULL},
  {"i15s2",	0, 15, 2, HW_INT, NULL},
  {"i17s2",	0, 17, 2, HW_INT, NULL},
  {"i19s2",	0, 19, 2, HW_INT, NULL},
  {"i3u",	0, 3, 0, HW_UINT, NULL},
  {"i5u",	0, 5, 0, HW_UINT, NULL},
  {"ib5u",	10, 5, 0, HW_UINT, NULL},	/* imm5 field in ALU.  */
  {"ib5s",	10, 5, 0, HW_INT, NULL},	/* imm5 field in ALU.  */
  {"ia3u",	3, 3, 0, HW_UINT, NULL},	/* for bmski33, fexti33.  */
  {"i8u",	0, 8, 0, HW_UINT, NULL},
  {"ib8u",	7, 8, 0, HW_UINT, NULL},	/* for ffbi.  */
  {"i15u",	0, 15, 0, HW_UINT, NULL},
  {"i20u",	0, 20, 0, HW_UINT, NULL},
  {"i3u1",	0, 3, 1, HW_UINT, NULL},
  {"i9u1",	0, 9, 1, HW_UINT, NULL},
  {"i3u2",	0, 3, 2, HW_UINT, NULL},
  {"i6u2",	0, 6, 2, HW_UINT, NULL},
  {"i7u2",	0, 7, 2, HW_UINT, NULL},
  {"i5u3",	0, 5, 3, HW_UINT, NULL},	/* for pop25/pop25.  */
  {"i15s3",	0, 15, 3, HW_INT, NULL},	/* for dprefi.d.  */
  {"ib4u",	10, 4, 0, HW_UINT, NULL},	/* imm5 field in ALU.  */
  {"ib2u",	10, 2, 0, HW_UINT, NULL},	/* imm5 field in ALU.  */

  {"a_rt",	15, 5, 0, HW_GPR, NULL},  /* for audio-extension.  */
  {"a_ru",	10, 5, 0, HW_GPR, NULL},  /* for audio-extension.  */
  {"a_dx",	9, 1, 0, HW_DXR, NULL},  /* for audio-extension.  */
  {"a_a30",	16, 4, 0, HW_GPR, parse_a30b20},  /* for audio-extension.  */
  {"a_b20",	12, 4, 0, HW_GPR, parse_a30b20},  /* for audio-extension.  */
  {"a_rt21",	5, 7, 0, HW_GPR, parse_rt21},  /* for audio-extension.  */
  {"a_rte",	5, 7, 0, HW_GPR, parse_rte_start},  /* for audio-extension.  */
  {"a_rte1",	5, 7, 0, HW_GPR, parse_rte_end},  /* for audio-extension.  */
  {"a_rte69",	6, 4, 0, HW_GPR, parse_rte69_start},  /* for audio-extension.  */
  {"a_rte69_1",	6, 4, 0, HW_GPR, parse_rte69_end},  /* for audio-extension.  */
  {"dhy",	5, 2, 0, HW_AEXT_ACC, NULL},  /* for audio-extension.  */
  {"dxh",	15, 2, 0, HW_AEXT_ACC, NULL},  /* for audio-extension.  */
  {"aridx",	0, 5, 0, HW_AEXT_ARIDX, NULL},  /* for audio-extension.  */
  {"aridx2",	0, 5, 0, HW_AEXT_ARIDX2, NULL},  /* for audio-extension.  */
  {"aridxi",	16, 4, 0, HW_AEXT_ARIDXI, NULL},  /* for audio-extension.  */
  {"aridxi_mx",	16, 4, 0, HW_AEXT_ARIDXI_MX, NULL},  /* for audio-extension.  */
  {"imm16s",	0, 16, 0, HW_INT, NULL},  /* for audio-extension.  */
  {"imm16u",	0, 16, 0, HW_UINT, NULL},  /* for audio-extension.  */
  {"im5_i",	0, 5, 0, HW_AEXT_IM_I, parse_im5_ip},  /* for audio-extension.  */
  {"im5_m",	0, 5, 0, HW_AEXT_IM_M, parse_im5_mr},  /* for audio-extension.  */
  {"im6_ip",	0, 2, 0, HW_AEXT_IM_I, parse_im6_ip},  /* for audio-extension.  */
  {"im6_iq",	0, 2, 0, HW_AEXT_IM_I, parse_im6_iq},  /* for audio-extension.  */
  {"im6_mr",	2, 2, 0, HW_AEXT_IM_M, parse_im6_mr},  /* for audio-extension.  */
  {"im6_ms",	4, 2, 0, HW_AEXT_IM_M, parse_im6_ms},  /* for audio-extension.  */
  {"cp45",	4, 2, 0, HW_CP, NULL},  /* for cop-extension.  */
  {"i12u",	8, 12, 0, HW_UINT, NULL},  /* for cop-extension.  */
  {"cpi19",	6, 19, 0, HW_UINT, NULL},  /* for cop-extension.  */
  {NULL, 0, 0, 0, 0, NULL}
};

#define DEF_REG(r)		(N32_BIT (r))
#define USE_REG(r)		(N32_BIT (r))
#define RT(r)			(r << 20)
#define RA(r)			(r << 15)
#define RB(r)			(r << 10)
#define RA5(r)			(r)

struct nds32_opcode nds32_opcodes[] =
{
  /* opc6_encoding table OPC_6.  */
  {"lbi", "=rt,[%ra{+%i15s}]",		OP6 (LBI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhi", "=rt,[%ra{+%i15s1}]",		OP6 (LHI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lwi", "=rt,[%ra{+%i15s2}]",		OP6 (LWI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lbi.bi", "=rt,[%ra],%i15s",		OP6 (LBI_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhi.bi", "=rt,[%ra],%i15s1",	OP6 (LHI_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lwi.bi", "=rt,[%ra],%i15s2",	OP6 (LWI_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sbi", "%rt,[%ra{+%i15s}]",		OP6 (SBI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"shi", "%rt,[%ra{+%i15s1}]",		OP6 (SHI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"swi", "%rt,[%ra{+%i15s2}]",		OP6 (SWI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sbi.bi", "%rt,[%ra],%i15s",		OP6 (SBI_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"shi.bi", "%rt,[%ra],%i15s1",	OP6 (SHI_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"swi.bi", "%rt,[%ra],%i15s2",	OP6 (SWI_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},

  {"lbsi", "=rt,[%ra{+%i15s}]",		OP6 (LBSI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhsi", "=rt,[%ra{+%i15s1}]",	OP6 (LHSI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lbsi.bi", "=rt,[%ra],%i15s",	OP6 (LBSI_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhsi.bi", "=rt,[%ra],%i15s1",	OP6 (LHSI_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"movi", "=rt,%i20s",		OP6 (MOVI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sethi", "=rt,%i20u",	OP6 (SETHI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"addi", "=rt,%ra,%i15s",	OP6 (ADDI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"subri", "=rt,%ra,%i15s",	OP6 (SUBRI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"andi", "=rt,%ra,%i15u",	OP6 (ANDI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"xori", "=rt,%ra,%i15u",	OP6 (XORI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"ori", "=rt,%ra,%i15u",	OP6 (ORI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"slti", "=rt,%ra,%i15s",	OP6 (SLTI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sltsi", "=rt,%ra,%i15s",	OP6 (SLTSI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"bitci", "=rt,%ra,%i15u",	OP6 (BITCI), 4, ATTR_V3, 0, NULL, 0, NULL},

  /* seg-DPREFI.  */
  {"dprefi.w", "%dpref_st,[%ra{+%i15s2}]",	OP6 (DPREFI), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"dprefi.d", "%dpref_st,[%ra{+%i15s3}]",	OP6 (DPREFI) | N32_BIT (24), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  /* seg-LBGP.  */
  {"lbi.gp", "=rt,[+%i19s]",	OP6 (LBGP), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},
  {"lbsi.gp", "=rt,[+%i19s]",	OP6 (LBGP) | N32_BIT (19), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},
  /* seg-LWC/0.  */
  {"cplwi", "%cp,=cprt,[%ra{+%i12s2}]",		OP6 (LWC), 4, 0, 0, NULL, 0, NULL},
  {"cplwi.bi", "%cp,=cprt,[%ra],%i12s2",	OP6 (LWC) | N32_BIT (12), 4, 0, 0, NULL, 0, NULL},
  /* seg-SWC/0.  */
  {"cpswi", "%cp,=cprt,[%ra{+%i12s2}]",		OP6 (SWC), 4, 0, 0, NULL, 0, NULL},
  {"cpswi.bi", "%cp,=cprt,[%ra],%i12s2",	OP6 (SWC) | N32_BIT (12), 4, 0, 0, NULL, 0, NULL},
  /* seg-LDC/0.  */
  {"cpldi", "%cp,%cprt,[%ra{+%i12s2}]",		OP6 (LDC), 4, 0, 0, NULL, 0, NULL},
  {"cpldi.bi", "%cp,%cprt,[%ra],%i12s2",	OP6 (LDC) | N32_BIT (12), 4, 0, 0, NULL, 0, NULL},
  /* seg-SDC/0.  */
  {"cpsdi", "%cp,%cprt,[%ra{+%i12s2}]",		OP6 (SDC), 4, 0, 0, NULL, 0, NULL},
  {"cpsdi.bi", "%cp,%cprt,[%ra],%i12s2",	OP6 (SDC) | N32_BIT (12), 4, 0, 0, NULL, 0, NULL},
  /* seg-LSMW.  */
  {"lmw", "%abdim %rt,[%ra],%re{,%enb4}",	LSMW (LSMW), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lmwa", "%abdim %rt,[%ra],%re{,%enb4}",	LSMW (LSMWA), 4, ATTR_V3MEX_V2, 0, NULL, 0, NULL},
  {"lmwzb", "%abm %rt,[%ra],%re{,%enb4}",	LSMW (LSMWZB), 4, ATTR (STR_EXT), 0, NULL, 0, NULL},
  {"smw", "%abdim %rt,[%ra],%re{,%enb4}",	LSMW (LSMW) | N32_BIT (5), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"smwa", "%abdim %rt,[%ra],%re{,%enb4}",	LSMW (LSMWA) | N32_BIT (5), 4, ATTR_V3MEX_V2, 0, NULL, 0, NULL},
  {"smwzb", "%abm %rt,[%ra],%re{,%enb4}",	LSMW (LSMWZB) | N32_BIT (5), 4, ATTR (STR_EXT), 0, NULL, 0, NULL},
  /* seg-HWGP.  */
  {"lhi.gp", "=rt,[+%i18s1]",	OP6 (HWGP), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},
  {"lhsi.gp", "=rt,[+%i18s1]",	OP6 (HWGP) | (2 << 17), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},
  {"shi.gp", "%rt,[+%i18s1]",	OP6 (HWGP) | (4 << 17), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},
  {"lwi.gp", "=rt,[+%i17s2]",	OP6 (HWGP) | (6 << 17), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},
  {"swi.gp", "%rt,[+%i17s2]",	OP6 (HWGP) | (7 << 17), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},

  /* seg-SBGP.  */
  {"sbi.gp", "%rt,[+%i19s]",	OP6 (SBGP), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},
  {"addi.gp", "=rt,%i19s",	OP6 (SBGP) | N32_BIT (19), 4, ATTR (GPREL) | ATTR_V2UP, USE_REG (29), NULL, 0, NULL},
  /* seg-JI.  */
  {"j", "%i24s1",	OP6 (JI), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"jal", "%i24s1",	OP6 (JI) | N32_BIT (24), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-JREG.  */
  {"jr", "%rb",			JREG (JR), 4, ATTR (BRANCH) | ATTR_ALL, 0, NULL, 0, NULL},
  {"jral", "%rt,%rb",		JREG (JRAL), 4, ATTR (BRANCH) | ATTR_ALL, 0, NULL, 0, NULL},
  {"jrnez", "%rb",		JREG (JRNEZ), 4, ATTR (BRANCH) | ATTR_V3, 0, NULL, 0, NULL},
  {"jralnez", "%rt,%rb",	JREG (JRALNEZ), 4, ATTR (BRANCH) | ATTR_V3, 0, NULL, 0, NULL},
  {"ret", "%rb",		JREG (JR) | JREG_RET, 4, ATTR (BRANCH) | ATTR_ALL, 0, NULL, 0, NULL},
  {"jral", "%rb",		JREG (JRAL) | RT (30), 4, ATTR (BRANCH) | ATTR_ALL, 0, NULL, 0, NULL},
  {"jralnez", "%rb",		JREG (JRALNEZ) | RT (30), 4, ATTR (BRANCH) | ATTR_V3, 0, NULL, 0, NULL},
  {"ret", "",			JREG (JR) | JREG_RET | RB (30), 4, ATTR (BRANCH) | ATTR_ALL, 0, NULL, 0, NULL},
  {"jr", "%dtitoff %rb",	JREG (JR), 4, ATTR (BRANCH) | ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"ret", "%dtitoff %rb",	JREG (JR) | JREG_RET, 4, ATTR (BRANCH) | ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"jral", "%dtiton %rt,%rb",	JREG (JRAL), 4, ATTR (BRANCH) | ATTR_ALL, 0, NULL, 0, NULL},
  {"jral", "%dtiton %rb",	JREG (JRAL) | RT (30), 4, ATTR (BRANCH) | ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-BR1.  */
  {"beq", "%rt,%ra,%i14s1",	OP6 (BR1), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"bne", "%rt,%ra,%i14s1",	OP6 (BR1) | N32_BIT (14), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-BR2.  */
  {"beqz", "%rt,%i16s1",	BR2 (BEQZ), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"bnez", "%rt,%i16s1",	BR2 (BNEZ), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"bgez", "%rt,%i16s1",	BR2 (BGEZ), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"bltz", "%rt,%i16s1",	BR2 (BLTZ), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"bgtz", "%rt,%i16s1",	BR2 (BGTZ), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"blez", "%rt,%i16s1",	BR2 (BLEZ), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"bgezal", "%rt,%i16s1",	BR2 (BGEZAL), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"bltzal", "%rt,%i16s1",	BR2 (BLTZAL), 4, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-BR3.  */
  {"beqc", "%rt,%i11br3,%i8s1",	OP6 (BR3), 4, ATTR_PCREL | ATTR_V3MUP, 0, NULL, 0, NULL},
  {"bnec", "%rt,%i11br3,%i8s1",	OP6 (BR3) | N32_BIT (19), 4, ATTR_PCREL | ATTR_V3MUP, 0, NULL, 0, NULL},
  /* seg-SIMD.  */
  {"pbsad", "%rt,%ra,%rb", SIMD (PBSAD), 4, ATTR (PERF2_EXT), 0, NULL, 0, NULL},
  {"pbsada", "%rt,%ra,%rb", SIMD (PBSADA), 4, ATTR (PERF2_EXT), 0, NULL, 0, NULL},
  /* seg-ALU1.  */
  {"add", "=rt,%ra,%rb",	ALU1 (ADD), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sub", "=rt,%ra,%rb",	ALU1 (SUB), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"and", "=rt,%ra,%rb",	ALU1 (AND), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"xor", "=rt,%ra,%rb",	ALU1 (XOR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"or", "=rt,%ra,%rb",		ALU1 (OR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"nor", "=rt,%ra,%rb",	ALU1 (NOR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"slt", "=rt,%ra,%rb",	ALU1 (SLT), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"slts", "=rt,%ra,%rb",	ALU1 (SLTS), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"slli", "=rt,%ra,%ib5u",	ALU1 (SLLI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"srli", "=rt,%ra,%ib5u",	ALU1 (SRLI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"srai", "=rt,%ra,%ib5u",	ALU1 (SRAI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"rotri", "=rt,%ra,%ib5u",	ALU1 (ROTRI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sll", "=rt,%ra,%rb",	ALU1 (SLL), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"srl", "=rt,%ra,%rb",	ALU1 (SRL), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sra", "=rt,%ra,%rb",	ALU1 (SRA), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"rotr", "=rt,%ra,%rb",	ALU1 (ROTR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"seb", "=rt,%ra",		ALU1 (SEB), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"seh", "=rt,%ra",		ALU1 (SEH), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"bitc", "=rt,%ra,%rb",	ALU1 (BITC), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"zeh", "=rt,%ra",		ALU1 (ZEH), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"wsbh", "=rt,%ra",		ALU1 (WSBH), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"divsr", "=rt,=rd,%ra,%rb",	ALU1 (DIVSR), 4, ATTR (DIV) | ATTR_V2UP, 0, NULL, 0, NULL},
  {"divr", "=rt,=rd,%ra,%rb",	ALU1 (DIVR), 4, ATTR (DIV) | ATTR_V2UP, 0, NULL, 0, NULL},
  {"sva", "=rt,%ra,%rb",	ALU1 (SVA), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"svs", "=rt,%ra,%rb",	ALU1 (SVS), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cmovz", "=rt,%ra,%rb",	ALU1 (CMOVZ), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cmovn", "=rt,%ra,%rb",	ALU1 (CMOVN), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"or_srli", "=rt,%ra,%rb,%sh",	ALU1 (OR_SRLI), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"add_srli", "=rt,%ra,%rb,%sh",	ALU1 (ADD_SRLI), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"sub_srli", "=rt,%ra,%rb,%sh",	ALU1 (SUB_SRLI), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"and_srli", "=rt,%ra,%rb,%sh",	ALU1 (AND_SRLI), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"xor_srli", "=rt,%ra,%rb,%sh",	ALU1 (XOR_SRLI), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"add_slli", "=rt,%ra,%rb,%sh",	ALU1 (ADD), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"sub_slli", "=rt,%ra,%rb,%sh",	ALU1 (SUB), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"and_slli", "=rt,%ra,%rb,%sh",	ALU1 (AND), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"xor_slli", "=rt,%ra,%rb,%sh",	ALU1 (XOR), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"or_slli", "=rt,%ra,%rb,%sh",	ALU1 (OR), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"nop", "",	ALU1 (SRLI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-ALU2.  */
  {"max", "=rt,%ra,%rb",	ALU2 (MAX), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"min", "=rt,%ra,%rb",	ALU2 (MIN), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"ave", "=rt,%ra,%rb",	ALU2 (AVE), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"abs", "=rt,%ra",		ALU2 (ABS), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"clips", "=rt,%ra,%ib5u",	ALU2 (CLIPS), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"clip", "=rt,%ra,%ib5u",	ALU2 (CLIP), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"clo", "=rt,%ra",		ALU2 (CLO), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"clz", "=rt,%ra",		ALU2 (CLZ), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"bset", "=rt,%ra,%ib5u",	ALU2 (BSET), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"bclr", "=rt,%ra,%ib5u",	ALU2 (BCLR), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"btgl", "=rt,%ra,%ib5u",	ALU2 (BTGL), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"btst", "=rt,%ra,%ib5u",	ALU2 (BTST), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"bse", "=rt,%ra,=rb",	ALU2 (BSE), 4, ATTR (PERF2_EXT), 0, NULL, 0, NULL},
  {"bsp", "=rt,%ra,=rb",	ALU2 (BSP), 4, ATTR (PERF2_EXT), 0, NULL, 0, NULL},
  {"ffzmism", "=rt,%ra,%rb",	ALU2 (FFZMISM), 4, ATTR (STR_EXT), 0, NULL, 0, NULL},
  {"mfusr", "=rt,%usr",		ALU2 (MFUSR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mtusr", "%rt,%usr",		ALU2 (MTUSR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mfusr", "=rt,%ridx",	ALU2 (MFUSR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mtusr", "%rt,%ridx",	ALU2 (MTUSR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mul", "=rt,%ra,%rb",	ALU2 (MUL), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"madds64", "=dt,%ra,%rb",	ALU2 (MADDS64), 4, ATTR (MAC) | ATTR_ALL, 0, NULL, 0, NULL},
  {"madd64", "=dt,%ra,%rb",	ALU2 (MADD64), 4, ATTR (MAC) | ATTR_ALL, 0, NULL, 0, NULL},
  {"msubs64", "=dt,%ra,%rb",	ALU2 (MSUBS64), 4, ATTR (MAC) | ATTR_ALL, 0, NULL, 0, NULL},
  {"msub64", "=dt,%ra,%rb",	ALU2 (MSUB64), 4, ATTR (MAC) | ATTR_ALL, 0, NULL, 0, NULL},
  {"divs", "=dt,%ra,%rb",	ALU2 (DIVS), 4, ATTR (DIV) | ATTR (DXREG), 0, NULL, 0, NULL},
  {"div", "=dt,%ra,%rb",	ALU2 (DIV), 4, ATTR (DIV) | ATTR (DXREG), 0, NULL, 0, NULL},
  {"mult32", "=dt,%ra,%rb",	ALU2 (MULT32), 4, ATTR (DXREG) | ATTR_ALL, 0, NULL, 0, NULL},

  /* seg-ALU2_FFBI.  */
  {"ffb", "=rt,%ra,%rb",	ALU2 (FFB), 4, ATTR (STR_EXT), 0, NULL, 0, NULL},
  {"ffbi", "=rt,%ra,%ib8u",	ALU2 (FFBI) | N32_BIT (6), 4, ATTR (STR_EXT), 0, NULL, 0, NULL},
  /* seg-ALU2_FLMISM.  */
  {"ffmism", "=rt,%ra,%rb",	ALU2 (FFMISM), 4, ATTR (STR_EXT), 0, NULL, 0, NULL},
  {"flmism", "=rt,%ra,%rb",	ALU2 (FLMISM) | N32_BIT (6), 4, ATTR (STR_EXT), 0, NULL, 0, NULL},
  /* seg-ALU2_MULSR64.  */
  {"mults64", "=dt,%ra,%rb",	ALU2 (MULTS64), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mulsr64", "=rt,%ra,%rb",	ALU2 (MULSR64)| N32_BIT (6), 4, ATTR_V3MEX_V2, 0, NULL, 0, NULL},
  /* seg-ALU2_MULR64.  */
  {"mult64", "=dt,%ra,%rb",	ALU2 (MULT64), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mulr64", "=rt,%ra,%rb",	ALU2 (MULR64) | N32_BIT (6), 4, ATTR_V3MEX_V2, 0, NULL, 0, NULL},
  /* seg-ALU2_MADDR32.  */
  {"madd32", "=dt,%ra,%rb",	ALU2 (MADD32), 4, ATTR (MAC) | ATTR (DXREG) | ATTR_ALL, 0, NULL, 0, NULL},
  {"maddr32", "=rt,%ra,%rb",	ALU2 (MADDR32) | N32_BIT (6), 4, ATTR (MAC) | ATTR_V2UP, 0, NULL, 0, NULL},
  /* seg-ALU2_MSUBR32.  */
  {"msub32", "=dt,%ra,%rb",	ALU2 (MSUB32), 4, ATTR (MAC) | ATTR (DXREG) | ATTR_ALL, 0, NULL, 0, NULL},
  {"msubr32", "=rt,%ra,%rb",	ALU2 (MSUBR32) | N32_BIT (6), 4, ATTR (MAC) | ATTR_V2UP, 0, NULL, 0, NULL},

  /* seg-MISC.  */
  {"standby", "%stdby_st",	MISC (STANDBY), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mfsr", "=rt,%sr",		MISC (MFSR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"iret", "",			MISC (IRET), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"trap", "%swid",		MISC (TRAP), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"teqz", "%rt{,%swid}",	MISC (TEQZ), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"tnez", "%rt{,%swid}",	MISC (TNEZ), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"dsb", "",			MISC (DSB), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"isb", "",			MISC (ISB), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"break", "%swid",		MISC (BREAK), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"syscall", "%swid",		MISC (SYSCALL), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"msync", "%msync_st",	MISC (MSYNC), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"isync", "%rt",		MISC (ISYNC), 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-MISC_MTSR.  */
  {"mtsr", "%rt,%sr",		MISC (MTSR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-MISC_SETEND.  */
  {"setend.l", "",	MISC (MTSR) | (SRIDX (1, 0, 0) << 10) | N32_BIT (5), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"setend.b", "",	MISC (MTSR) | (SRIDX (1, 0, 0) << 10) | N32_BIT (5) | N32_BIT (20), 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-MISC_SETGIE.  */
  {"setgie.d", "",	MISC (MTSR) | (SRIDX (1, 0, 0) << 10) | N32_BIT (6), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"setgie.e", "",	MISC (MTSR) | (SRIDX (1, 0, 0) << 10) | N32_BIT (6) | N32_BIT (20), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mfsr", "=rt,%ridx",		MISC (MFSR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mtsr", "%rt,%ridx",		MISC (MTSR), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"trap", "",			MISC (TRAP), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"break", "",			MISC (BREAK), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"msync", "",			MISC (MSYNC), 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-MISC_TLBOP.  */
  {"tlbop", "%ra,%tlbop_st",	MISC (TLBOP), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"tlbop", "%ra,%tlbop_stx",	MISC (TLBOP), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"tlbop", "%rt,%ra,pb",	MISC (TLBOP) | (5 << 5), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"tlbop", "%rt,%ra,probe",	MISC (TLBOP) | (5 << 5), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"tlbop", "flua",		MISC (TLBOP) | (7 << 5), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"tlbop", "flushall",		MISC (TLBOP) | (7 << 5), 4, ATTR_ALL, 0, NULL, 0, NULL},

  /* seg-MEM.  */
  {"lb", "=rt,[%ra+(%rb<<%sv)]",	MEM (LB), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lb", "=rt,[%ra+%rb{<<%sv}]",	MEM (LB), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lh", "=rt,[%ra+(%rb<<%sv)]",	MEM (LH), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lh", "=rt,[%ra+%rb{<<%sv}]",	MEM (LH), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lw", "=rt,[%ra+(%rb<<%sv)]",	MEM (LW), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lw", "=rt,[%ra+%rb{<<%sv}]",	MEM (LW), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"ld", "=rt,[%ra+(%rb<<%sv)]",	MEM (LD), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lb.bi", "=rt,[%ra],%rb{<<%sv}",	MEM (LB_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lb.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (LB_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lb.p", "=rt,[%ra],%rb{<<%sv}",	MEM (LB_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lh.bi", "=rt,[%ra],%rb{<<%sv}",	MEM (LH_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lh.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (LH_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lh.p", "=rt,[%ra],%rb{<<%sv}",	MEM (LH_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lw.bi", "=rt,[%ra],%rb{<<%sv}",	MEM (LW_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lw.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (LW_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lw.p", "=rt,[%ra],%rb{<<%sv}",	MEM (LW_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"ld.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (LD_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sb", "=rt,[%ra+(%rb<<%sv)]",	MEM (SB), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sb", "%rt,[%ra+%rb{<<%sv}]",	MEM (SB), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sh", "=rt,[%ra+(%rb<<%sv)]",	MEM (SH), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sh", "%rt,[%ra+%rb{<<%sv}]",	MEM (SH), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sw", "=rt,[%ra+(%rb<<%sv)]",	MEM (SW), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sw", "%rt,[%ra+%rb{<<%sv}]",	MEM (SW), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sd", "=rt,[%ra+(%rb<<%sv)]",	MEM (SD), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sb.bi", "%rt,[%ra],%rb{<<%sv}",	MEM (SB_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sb.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (SB_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sb.p", "%rt,[%ra],%rb{<<%sv}",	MEM (SB_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sh.bi", "%rt,[%ra],%rb{<<%sv}",	MEM (SH_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sh.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (SH_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sh.p", "%rt,[%ra],%rb{<<%sv}",	MEM (SH_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sw.bi", "%rt,[%ra],%rb{<<%sv}",	MEM (SW_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sw.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (SW_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sw.p", "%rt,[%ra],%rb{<<%sv}",	MEM (SW_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sd.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (SD_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},

  {"lbs", "=rt,[%ra+(%rb<<%sv)]",	MEM (LBS), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lbs", "=rt,[%ra+%rb{<<%sv}]",	MEM (LBS), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhs", "=rt,[%ra+(%rb<<%sv)]",	MEM (LHS), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhs", "=rt,[%ra+%rb{<<%sv}]",	MEM (LHS), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lbs.bi", "=rt,[%ra],%rb{<<%sv}",	MEM (LBS_BI),4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lbs.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (LBS_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lbs.p", "=rt,[%ra],%rb{<<%sv}",	MEM (LBS_BI),4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhs.bi", "=rt,[%ra],%rb{<<%sv}",	MEM (LHS_BI),4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhs.bi", "=rt,[%ra],(%rb<<%sv)",	MEM (LHS_BI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhs.p", "=rt,[%ra],%rb{<<%sv}",	MEM (LHS_BI),4, ATTR_ALL, 0, NULL, 0, NULL},
  {"llw", "=rt,[%ra+(%rb<<%sv)]",	MEM (LLW), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"llw", "=rt,[%ra+%rb{<<%sv}]",	MEM (LLW), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"scw", "%rt,[%ra+(%rb<<%sv)]",	MEM (SCW), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"scw", "%rt,[%ra+%rb{<<%sv}]",	MEM (SCW), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},

  {"lbup", "=rt,[%ra+(%rb<<%sv)]",	MEM (LBUP), 4, ATTR_V3MEX_V2, 0, NULL, 0, NULL},
  {"lbup", "=rt,[%ra+%rb{<<%sv}]",	MEM (LBUP), 4, ATTR_V3MEX_V2, 0, NULL, 0, NULL},
  {"lwup", "=rt,[%ra+(%rb<<%sv)]",	MEM (LWUP), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"lwup", "=rt,[%ra+%rb{<<%sv}]",	MEM (LWUP), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"sbup", "%rt,[%ra+(%rb<<%sv)]",	MEM (SBUP), 4, ATTR_V3MEX_V2, 0, NULL, 0, NULL},
  {"sbup", "%rt,[%ra+%rb{<<%sv}]",	MEM (SBUP), 4, ATTR_V3MEX_V2, 0, NULL, 0, NULL},
  {"swup", "%rt,[%ra+(%rb<<%sv)]",	MEM (SWUP), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"swup", "%rt,[%ra+%rb{<<%sv}]",	MEM (SWUP), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},

  {"dpref", "%dpref_st,[%ra]",	OP6 (DPREFI), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"dpref", "%dpref_st,[%ra+(%rb<<%sv)]",	MEM (DPREF), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"dpref", "%dpref_st,[%ra+%rb{<<%sv}]",	MEM (DPREF), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},

  /* For missing-operand-load/store instructions.  */
  {"lb", "=rt,[%ra]",	OP6 (LBI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lh", "=rt,[%ra]",	OP6 (LHI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lw", "=rt,[%ra]",	OP6 (LWI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lbs", "=rt,[%ra]",	OP6 (LBSI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhs", "=rt,[%ra]",	OP6 (LHSI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sb", "%rt,[%ra]",	OP6 (SBI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sh", "%rt,[%ra]",	OP6 (SHI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"sw", "%rt,[%ra]",	OP6 (SWI), 4, ATTR_ALL, 0, NULL, 0, NULL},

  /* seg-LWC0.  */
  {"flsi", "=fst,[%ra{+%i12s2}]",	OP6 (LWC), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"flsi.bi", "=fst,[%ra],%i12s2",	FPU_RA_IMMBI (LWC), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-SWC0.  */
  {"fssi", "=fst,[%ra{+%i12s2}]",	OP6 (SWC), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fssi.bi", "=fst,[%ra],%i12s2",	FPU_RA_IMMBI (SWC), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-LDC0.  */
  {"fldi", "=fdt,[%ra{+%i12s2}]",	OP6 (LDC), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fldi.bi", "=fdt,[%ra],%i12s2",	FPU_RA_IMMBI (LDC), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-SDC0.  */
  {"fsdi", "=fdt,[%ra{+%i12s2}]",	OP6 (SDC), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fsdi.bi", "=fdt,[%ra],%i12s2",	FPU_RA_IMMBI (SDC), 4, ATTR (FPU), 0, NULL, 0, NULL},

  /* seg-FPU_FS1.  */
  {"fadds", "=fst,%fsa,%fsb",	FS1 (FADDS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fsubs", "=fst,%fsa,%fsb",	FS1 (FSUBS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcpynss", "=fst,%fsa,%fsb",	FS1 (FCPYNSS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcpyss", "=fst,%fsa,%fsb",	FS1 (FCPYSS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fmadds", "=fst,%fsa,%fsb",	FS1 (FMADDS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fmsubs", "=fst,%fsa,%fsb",	FS1 (FMSUBS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmovns", "=fst,%fsa,%fsb",	FS1 (FCMOVNS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmovzs", "=fst,%fsa,%fsb",	FS1 (FCMOVZS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fnmadds", "=fst,%fsa,%fsb",	FS1 (FNMADDS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fnmsubs", "=fst,%fsa,%fsb",	FS1 (FNMSUBS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fmuls", "=fst,%fsa,%fsb",	FS1 (FMULS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fdivs", "=fst,%fsa,%fsb",	FS1 (FDIVS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},

  /* seg-FPU_FS1_F2OP.  */
  {"fs2d", "=fdt,%fsa",		FS1_F2OP (FS2D), 4, ATTR (FPU) | ATTR (FPU_SP_EXT) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fsqrts", "=fst,%fsa",	FS1_F2OP (FSQRTS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fabss", "=fst,%fsa",	FS1_F2OP (FABSS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fui2s", "=fst,%fsa",	FS1_F2OP (FUI2S), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fsi2s", "=fst,%fsa",	FS1_F2OP (FSI2S), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fs2ui", "=fst,%fsa",	FS1_F2OP (FS2UI), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fs2ui.z", "=fst,%fsa",	FS1_F2OP (FS2UI_Z), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fs2si", "=fst,%fsa",	FS1_F2OP (FS2SI), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fs2si.z", "=fst,%fsa",	FS1_F2OP (FS2SI_Z), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  /* seg-FPU_FS2.  */
  {"fcmpeqs", "=fst,%fsa,%fsb",		FS2 (FCMPEQS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmpeqs.e", "=fst,%fsa,%fsb",	FS2 (FCMPEQS_E), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmplts", "=fst,%fsa,%fsb",		FS2 (FCMPLTS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmplts.e", "=fst,%fsa,%fsb",	FS2 (FCMPLTS_E), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmples", "=fst,%fsa,%fsb",		FS2 (FCMPLES), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmples.e", "=fst,%fsa,%fsb",	FS2 (FCMPLES_E), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmpuns", "=fst,%fsa,%fsb",		FS2 (FCMPUNS), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  {"fcmpuns.e", "=fst,%fsa,%fsb",	FS2 (FCMPUNS_E), 4, ATTR (FPU) | ATTR (FPU_SP_EXT), 0, NULL, 0, NULL},
  /* seg-FPU_FD1.  */
  {"faddd", "=fdt,%fda,%fdb",	FD1 (FADDD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fsubd", "=fdt,%fda,%fdb",	FD1 (FSUBD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcpynsd", "=fdt,%fda,%fdb",	FD1 (FCPYNSD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcpysd", "=fdt,%fda,%fdb",	FD1 (FCPYSD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fmaddd", "=fdt,%fda,%fdb",	FD1 (FMADDD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fmsubd", "=fdt,%fda,%fdb",	FD1 (FMSUBD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmovnd", "=fdt,%fda,%fsb",	FD1 (FCMOVND), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmovzd", "=fdt,%fda,%fsb",	FD1 (FCMOVZD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fnmaddd", "=fdt,%fda,%fdb",	FD1 (FNMADDD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fnmsubd", "=fdt,%fda,%fdb",	FD1 (FNMSUBD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fmuld", "=fdt,%fda,%fdb",	FD1 (FMULD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fdivd", "=fdt,%fda,%fdb",	FD1 (FDIVD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  /* seg-FPU_FD1_F2OP.  */
  {"fd2s", "=fst,%fda",		FD1_F2OP (FD2S), 4, ATTR (FPU) | ATTR (FPU_SP_EXT) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fsqrtd", "=fdt,%fda",	FD1_F2OP (FSQRTD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fabsd", "=fdt,%fda",	FD1_F2OP (FABSD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fui2d", "=fdt,%fsa",	FD1_F2OP (FUI2D), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fsi2d", "=fdt,%fsa",	FD1_F2OP (FSI2D), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fd2ui", "=fst,%fda",	FD1_F2OP (FD2UI), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fd2ui.z", "=fst,%fda",	FD1_F2OP (FD2UI_Z), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fd2si", "=fst,%fda",	FD1_F2OP (FD2SI), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fd2si.z", "=fst,%fda",	FD1_F2OP (FD2SI_Z), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  /* seg-FPU_FD2.  */
  {"fcmpeqd", "=fst,%fda,%fdb",		FD2 (FCMPEQD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmpeqd.e", "=fst,%fda,%fdb",	FD2 (FCMPEQD_E), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmpltd", "=fst,%fda,%fdb",		FD2 (FCMPLTD), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmpltd.e", "=fst,%fda,%fdb",	FD2 (FCMPLTD_E), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmpled", "=fst,%fda,%fdb",		FD2 (FCMPLED), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmpled.e", "=fst,%fda,%fdb",	FD2 (FCMPLED_E), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmpund", "=fst,%fda,%fdb",		FD2 (FCMPUND), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  {"fcmpund.e", "=fst,%fda,%fdb",	FD2 (FCMPUND_E), 4, ATTR (FPU) | ATTR (FPU_DP_EXT), 0, NULL, 0, NULL},
  /* seg-FPU_MFCP.  */
  {"fmfsr", "=rt,%fsa",	MFCP (FMFSR), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fmfdr", "=rt,%fda",	MFCP (FMFDR), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-FPU_MFCP_XR.  */
  {"fmfcfg", "=rt",	MFCP_XR(FMFCFG), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fmfcsr", "=rt",	MFCP_XR(FMFCSR), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-FPU_MTCP.  */

  {"fmtsr", "%rt,=fsa",	MTCP (FMTSR), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fmtdr", "%rt,=fda",	MTCP (FMTDR), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-FPU_MTCP_XR.  */
  {"fmtcsr", "%rt",	MTCP_XR(FMTCSR), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-FPU_FLS.  */
  {"fls", "=fst,[%ra+(%rb<<%sv)]",	FPU_MEM(FLS), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fls.bi", "=fst,[%ra],(%rb<<%sv)",	FPU_MEMBI(FLS), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-FPU_FLD.  */
  {"fld", "=fdt,[%ra+(%rb<<%sv)]",	FPU_MEM(FLD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fld.bi", "=fdt,[%ra],(%rb<<%sv)",	FPU_MEMBI(FLD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-FPU_FSS.  */
  {"fss", "=fst,[%ra+(%rb<<%sv)]",	FPU_MEM(FSS), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fss.bi", "=fst,[%ra],(%rb<<%sv)",	FPU_MEMBI(FSS), 4, ATTR (FPU), 0, NULL, 0, NULL},
  /* seg-FPU_FSD.  */
  {"fsd", "=fdt,[%ra+(%rb<<%sv)]",	FPU_MEM(FSD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fsd.bi", "=fdt,[%ra],(%rb<<%sv)",	FPU_MEMBI(FSD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fls", "=fst,[%ra+%rb{<<%sv}]",	FPU_MEM(FLS), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fls.bi", "=fst,[%ra],%rb{<<%sv}",	FPU_MEMBI(FLS), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fld", "=fdt,[%ra+%rb{<<%sv}]",	FPU_MEM(FLD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fld.bi", "=fdt,[%ra],%rb{<<%sv}",	FPU_MEMBI(FLD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fss", "=fst,[%ra+%rb{<<%sv}]",	FPU_MEM(FSS), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fss.bi", "=fst,[%ra],%rb{<<%sv}",	FPU_MEMBI(FSS), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fsd", "=fdt,[%ra+%rb{<<%sv}]",	FPU_MEM(FSD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"fsd.bi", "=fdt,[%ra],%rb{<<%sv}",	FPU_MEMBI(FSD), 4, ATTR (FPU), 0, NULL, 0, NULL},
  {"cctl", "%ra,%cctl_st0",		MISC (CCTL), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"cctl", "%ra,%cctl_st1{,%cctl_lv}",	MISC (CCTL), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"cctl", "=rt,%ra,%cctl_st2",		MISC (CCTL), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"cctl", "%rt,%ra,%cctl_st3",		MISC (CCTL), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"cctl", "%cctl_st4",			MISC (CCTL), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  {"cctl", "%cctl_st5{,%cctl_lv}",	MISC (CCTL), 4, ATTR_V3, 0, NULL, 0, NULL},
  {"cctl", "=rt,%ra,%cctl_stx,%cctl_lv",	MISC (CCTL), 4, ATTR_V3MEX_V1, 0, NULL, 0, NULL},
  /* seg-Alias instructions.  */
  {"neg", "=rt,%ra",	OP6 (SUBRI), 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"zeb", "=rt,%ra",	OP6 (ANDI) | 0xff, 4, ATTR_ALL, 0, NULL, 0, NULL},

  /* seg-COP.  */
  {"cpe1", "%cp45,%cpi19",	OP6 (COP) | 0x00, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cpe2", "%cp45,%cpi19",	OP6 (COP) | 0x04, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cpe3", "%cp45,%cpi19",	OP6 (COP) | 0x08, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cpe4", "%cp45,%cpi19",	OP6 (COP) | 0x0C, 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-COP-MFCPX.  */
  {"mfcpw", "%cp45,=rt,%i12u",	OP6 (COP) | 0x01, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mfcpd", "%cp45,=rt,%i12u",	OP6 (COP) | 0x41, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mfcppw", "%cp45,=rt,%i12u",	OP6 (COP) | 0xc1, 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-COP-CPLW.  */
  {"cplw", "%cp45,%cprt,[%ra+%rb<<%sv]",	OP6 (COP) | 0x02, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cplw.bi", "%cp45,%cprt,[%ra],%rb<<%sv",	OP6 (COP) | 0x82, 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-COP-CPLD.  */
  {"cpld", "%cp45,%cprt,[%ra+%rb<<%sv]",	OP6 (COP) | 0x03, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cpld.bi", "%cp45,%cprt,[%ra],%rb<<%sv",	OP6 (COP) | 0x83, 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-COP-MTCPX.  */
  {"mtcpw", "%cp45,%rt,%i12u",	OP6 (COP) | 0x09, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mtcpd", "%cp45,%rt,%i12u",	OP6 (COP) | 0x49, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"mtcppw", "%cp45,%rt,%i12u",	OP6 (COP) | 0xc9, 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-COP-CPSW.  */
  {"cpsw", "%cp45,%cprt,[%ra+%rb<<%sv]",	OP6 (COP) | 0x0a, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cpsw.bi", "%cp45,%cprt,[%ra],%rb<<%sv",	OP6 (COP) | 0x8a, 4, ATTR_ALL, 0, NULL, 0, NULL},
  /* seg-COP-CPSD.  */
  {"cpsd", "%cp45,%cprt,[%ra+%rb<<%sv]",	OP6 (COP) | 0x0b, 4, ATTR_ALL, 0, NULL, 0, NULL},
  {"cpsd.bi", "%cp45,%cprt,[%ra],%rb<<%sv",	OP6 (COP) | 0x8b, 4, ATTR_ALL, 0, NULL, 0, NULL},

  /* 16-bit instructions.  */
  /* get bit14~bit11 of 16-bit instruction.  */
  {"beqz38", "%rt38,%i8s1",	0xc000, 2, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"bnez38", "%rt38,%i8s1",	0xc800, 2, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  {"beqs38", "%rt38,%i8s1",	0xd000, 2, ATTR_PCREL | ATTR_ALL, USE_REG (5), NULL, 0, NULL},
  {"bnes38", "%rt38,%i8s1",	0xd800, 2, ATTR_PCREL | ATTR_ALL, USE_REG (5), NULL, 0, NULL},

  /* SEG00, get bit10.  */
  {"mov55", "=rt5,%ra5",	0x8000, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"movi55", "=rt5,%i5s",	0x8400, 2, ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG01  bit10~bit9.  */
  {"add45", "=rt4,%ra5",	0x8800, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"sub45", "=rt4,%ra5",	0x8a00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"addi45", "=rt4,%i5u",	0x8c00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"subi45", "=rt4,%i5u",	0x8e00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG02  bit10~bit9.  */
  {"srai45", "=rt4,%i5u",	0x9000, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"srli45", "=rt4,%i5u",	0x9200, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"slli333", "=rt3,%ra3,%i3u",	0x9400, 2, ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG03  bit10~bit9.  */
  {"add333", "=rt3,%ra3,%rb3",	0x9800, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"sub333", "=rt3,%ra3,%rb3",	0x9a00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"addi333", "=rt3,%ra3,%i3u",	0x9c00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"subi333", "=rt3,%ra3,%i3u",	0x9e00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG04  bit10~bit9.  */
  {"lwi333", "=rt3,[%ra3{+%i3u2}]",	0xa000, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"lwi333.bi", "=rt3,[%ra3],%i3u2",	0xa200, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"lhi333", "=rt3,[%ra3{+%i3u1}]",	0xa400, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"lbi333", "=rt3,[%ra3{+%i3u}]",	0xa600, 2, ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG05  bit10~bit9.  */
  {"swi333", "%rt3,[%ra3{+%i3u2}]",	0xa800, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"swi333.bi", "%rt3,[%ra3],%i3u2",	0xaa00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"shi333", "%rt3,[%ra3{+%i3u1}]",	0xac00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"sbi333", "%rt3,[%ra3{+%i3u}]",	0xae00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG06  bit10~bit9.  */
  {"addri36.sp", "%rt3,%i6u2",	0xb000, 2, ATTR_V3MUP, USE_REG (31), NULL, 0, NULL},
  {"lwi45.fe", "=rt4,%fe5",	0xb200, 2, ATTR_V3MUP, USE_REG (8), NULL, 0, NULL},
  {"lwi450", "=rt4,[%ra5]",	0xb400, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"swi450", "%rt4,[%ra5]",	0xb600, 2, ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG07  bit7.  */
  {"lwi37", "=rt38,[$fp{+%i7u2}]",	0xb800, 2, ATTR_ALL, USE_REG (28), NULL, 0, NULL},
  {"swi37", "%rt38,[$fp{+%i7u2}]",	0xb880, 2, ATTR_ALL, USE_REG (28), NULL, 0, NULL},
  /* SEG10_1 if Rt3=5.  */
  {"j8", "%i8s1",	0xd500, 2, ATTR_PCREL | ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG11_2  bit7~bit5.  */
  {"jr5", "%ra5",	0xdd00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"jral5", "%ra5",	0xdd20, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"ret5", "%ra5",	0xdd80, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"add5.pc", "%ra5",	0xdda0, 2, ATTR_V3, 0, NULL, 0, NULL},
  /* SEG11_3  if Ra5=30.  */
  {"ret5", "",	0xdd80 | RA5 (30), 2, ATTR_ALL, 0, NULL, 0, NULL},
  /* SEG12  bit10~bit9.  */
  {"slts45", "%rt4,%ra5",	0xe000, 2, ATTR_ALL, DEF_REG (15), NULL, 0, NULL},
  {"slt45", "%rt4,%ra5",	0xe200, 2, ATTR_ALL, DEF_REG (15), NULL, 0, NULL},
  {"sltsi45", "%rt4,%i5u",	0xe400, 2, ATTR_ALL, DEF_REG (15), NULL, 0, NULL},
  {"slti45", "%rt4,%i5u",	0xe600, 2, ATTR_ALL, DEF_REG (15), NULL, 0, NULL},
  /* SEG13  bit10~bit9.  */
  {"break16", "%i5u",	0xea00, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"addi10.sp", "%i10s",	0xec00, 2, ATTR_V2UP, USE_REG (31) | DEF_REG (31), NULL, 0, NULL},
  {"addi10.sp", "%i10s",	0xec00, 2, ATTR_V2UP, USE_REG (31) | DEF_REG (31), NULL, 0, NULL},
  /* SEG13_1  bit8.  */
  {"beqzs8", "%i8s1",	0xe800, 2, ATTR_PCREL | ATTR_ALL, USE_REG (15), NULL, 0, NULL},
  {"bnezs8", "%i8s1",	0xe900, 2, ATTR_PCREL | ATTR_ALL, USE_REG (15), NULL, 0, NULL},
  /* SEG14  bit7.  */
  {"lwi37.sp", "=rt38,[+%i7u2]",	0xf000, 2, ATTR_V2UP, USE_REG (31), NULL, 0, NULL},
  {"swi37.sp", "%rt38,[+%i7u2]",	0xf080, 2, ATTR_V2UP, USE_REG (31), NULL, 0, NULL},
  /* SEG15  bit10~bit9.  */
  {"movpi45", "=rt4,%pi5",	0xfa00, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  /* SEG15_1  bit8.  */
  {"movd44", "=rt5e,%ra5e",	0xfd00, 2, ATTR_V3MUP, 0, NULL, 0, NULL},

  /* SEG-BFMI333 bit2~bit0.  */
  {"zeb33", "=rt3,%ra3",	0x9600, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"zeh33", "=rt3,%ra3",	0x9601, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"seb33", "=rt3,%ra3",	0x9602, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"seh33", "=rt3,%ra3",	0x9603, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"xlsb33", "=rt3,%ra3",	0x9604, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"x11b33", "=rt3,%ra3",	0x9605, 2, ATTR_ALL, 0, NULL, 0, NULL},
  {"bmski33", "=rt3,%ia3u",	0x9606, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  {"fexti33", "=rt3,%ia3u",	0x9607, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  /* SEG-PUSHPOP25 bit8~bit7.  */
  {"push25", "%re2,%i5u3",	0xfc00, 2, ATTR_V3MUP, USE_REG (31) | DEF_REG (31), NULL, 0, NULL},
  {"pop25", "%re2,%i5u3",	0xfc80, 2, ATTR_V3MUP, USE_REG (31) | DEF_REG (31), NULL, 0, NULL},
  /* SEG-MISC33 bit2~bit0.  */
  {"neg33", "=rt3,%ra3",	0xfe02, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  {"not33", "=rt3,%ra3",	0xfe03, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  {"mul33", "=rt3,%ra3",	0xfe04, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  {"xor33", "=rt3,%ra3",	0xfe05, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  {"and33", "=rt3,%ra3",	0xfe06, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  {"or33", "=rt3,%ra3",	0xfe07, 2, ATTR_V3MUP, 0, NULL, 0, NULL},
  /* SEG-Alias instructions.  */
  {"nop16", "",	0x9200, 2, ATTR_ALL, 0, NULL, 0, NULL},

  /* Saturation ext ISA.  */
  {"kaddw", "=rt,%ra,%rb",	ALU2 (KADD), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"ksubw", "=rt,%ra,%rb",	ALU2 (KSUB), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"kaddh", "=rt,%ra,%rb",	ALU2 (KADD) | N32_BIT (6), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"ksubh", "=rt,%ra,%rb",	ALU2 (KSUB) | N32_BIT (6), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"kdmbb", "=rt,%ra,%rb",	ALU2 (KMxy), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"kdmbt", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (6), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"kdmtb", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (7), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"kdmtt", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (6) | N32_BIT (7), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"khmbb", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (8), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"khmbt", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (8) | N32_BIT (6), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"khmtb", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (8) | N32_BIT (7), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"khmtt", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (8) | N32_BIT (6) | N32_BIT (7), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"kslraw", "=rt,%ra,%rb",	ALU2 (KSLRAW), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"ksll", "=rt,%ra,%rb",	ALU2 (KSLRAW), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"kslraw.u", "=rt,%ra,%rb",	ALU2 (KSLRAWu), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"rdov", "=rt",		ALU2 (MFUSR) | N32_BIT (6) | ( 0x1e << 15), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},
  {"clrov", "",			ALU2 (MTUSR) | N32_BIT (6) | ( 0x1e << 15), 4, ATTR (SATURATION_EXT), 0, NULL, 0, NULL},

  /* Audio ext. instructions.  */

  {"amtari", "%aridxi,%imm16u", AUDIO (AMTARI), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtari", "%aridxi_mx,%imm16s", AUDIO (AMTARI), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMADD */
  {"alr2", "=a_rt,=a_ru,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMADD) | (0x1 << 6), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMADD) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMADD) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMADD) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMADD) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddsa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMADD) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"alr", "=a_rt,[%im5_i],%im5_m", AUDIO (AMADD) | (0x01 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amadd", "=a_dx,%ra,%rb", AUDIO (AMADD), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabbs", "=a_dx,%ra,%rb", AUDIO (AMADD) | 0x01, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMSUB */
  {"amsubl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMSUB) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMSUB) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMSUB) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMSUB) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubsa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMSUB) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"asr", "%ra,[%im5_i],%im5_m", AUDIO (AMSUB) | (0x01 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsub", "=a_dx,%ra,%rb", AUDIO (AMSUB), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabts", "=a_dx,%ra,%rb", AUDIO (AMSUB) |0x01, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMULT */
  {"amultl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMULT) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amultl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMULT) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amultl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMULT) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amultl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMULT) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amultsa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMULT) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ala", "=dxh,[%im5_i],%im5_m", AUDIO (AMULT) | (0x01 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amult", "=a_dx,%ra,%rb", AUDIO (AMULT), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amatbs", "=a_dx,%ra,%rb", AUDIO (AMULT) |0x01, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"asats48", "=a_dx", AUDIO (AMULT) | (0x02 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"awext", "%ra,%a_dx,%i5u", AUDIO (AMULT) | (0x03 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMFAR */
  {"amatts", "=a_dx,%ra,%rb", AUDIO (AMFAR) | 0x01, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"asa", "=dxh,[%im5_i],%im5_m", AUDIO (AMFAR) | (0x01 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtar", "%ra,%aridx", AUDIO (AMFAR) | (0x02 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtar2", "%ra,%aridx2", AUDIO (AMFAR) | (0x12 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amfar", "=ra,%aridx", AUDIO (AMFAR) | (0x03 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amfar2", "=ra,%aridx2", AUDIO (AMFAR) | (0x13 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMADDS */
  {"amaddsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMADDS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMADDS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMADDS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMADDS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amaddssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMADDS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"aupi", "%im5_i,%im5_m", AUDIO (AMADDS) | (0x01 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amadds", "=a_dx,%ra,%rb", AUDIO (AMADDS), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambbs", "=a_dx,%ra,%rb", AUDIO (AMADDS) |0x01, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawbs", "=a_dx,%ra,%rb", AUDIO (AMADDS) |0x02, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMSUBS */
  {"amsubsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMSUBS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMSUBS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMSUBS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMSUBS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMSUBS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amsubs", "=a_dx,%ra,%rb", AUDIO (AMSUBS), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambts", "=a_dx,%ra,%rb", AUDIO (AMSUBS) |0x01, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawts", "=a_dx,%ra,%rb", AUDIO (AMSUBS) |0x02, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMULTS */
  {"amultsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMULTS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amultsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMULTS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amultsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMULTS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amultsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMULTS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amultssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMULTS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amults", "=a_dx,%ra,%rb", AUDIO (AMULTS), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtbs", "=a_dx,%ra,%rb", AUDIO (AMULTS) |0x01, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwbs", "=a_dx,%ra,%rb", AUDIO (AMULTS) |0x02, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMNEGS */
  {"amnegsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMNEGS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amnegsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMNEGS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amnegsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMNEGS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amnegsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMNEGS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amnegssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMNEGS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amnegs", "=a_dx,%ra,%rb", AUDIO (AMNEGS), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtts", "=a_dx,%ra,%rb", AUDIO (AMNEGS) |0x01, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwts", "=a_dx,%ra,%rb", AUDIO (AMNEGS) |0x02, 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AADDL */
  {"aaddl", "=a_rte69,%ra,%rb,%a_rte69_1,[%im5_i],%im5_m", AUDIO (AADDL), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"asubl", "=a_rte69,%ra,%rb,%a_rte69_1,[%im5_i],%im5_m", AUDIO (AADDL) | (0x01 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMAWBS */
  {"amawbsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMAWBS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawbsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMAWBS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawbsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMAWBS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawbsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMAWBS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawbssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMAWBS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMAWTS */
  {"amawtsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMAWTS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawtsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMAWTS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawtsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMAWTS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawtsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMAWTS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amawtssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMAWTS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMWBS */
  {"amwbsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMWBS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwbsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMWBS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwbsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMWBS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwbsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMWBS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwbssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMWBS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwbssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMWBS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMWTS */
  {"amwtsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMWTS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwtsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMWTS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwtsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMWTS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwtsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMWTS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amwtssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMWTS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMABBS */
  {"amabbsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMABBS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabbsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMABBS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabbsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMABBS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabbsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMABBS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabbssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMABBS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMABTS */
  {"amabtsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMABTS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabtsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMABTS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabtsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMABTS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabtsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMABTS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amabtssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMABTS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMATBS */
  {"amatbsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMATBS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amatbsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMATBS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amatbsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMATBS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amatbsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMATBS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amatbssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMATBS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMATTS */
  {"amattsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMATTS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amattsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMATTS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amattsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMATTS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amattsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMATTS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amattssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMATTS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMBBS */
  {"ambbsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMBBS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambbsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMBBS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambbsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMBBS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambbsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMBBS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambbssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMBBS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMBTS */
  {"ambtsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMBTS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambtsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMBTS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambtsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMBTS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambtsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMBTS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"ambtssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMBTS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMTBS */
  {"amtbsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMTBS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtbsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMTBS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtbsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMTBS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtbsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMTBS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amtbssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMTBS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  /* N32_AEXT_AMTTS */
  {"amttsl.s", "=a_dx,%ra,%rb,[%im5_i],%im5_m", AUDIO (AMTTS) | (0x04 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amttsl.l", "=a_dx,%a_a30,%a_b20,%a_rt21,[%im5_i],%im5_m", AUDIO (AMTTS) | (0x06 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amttsl2.s", "=a_dx,%ra,%rb,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMTTS) | (0x08 << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amttsl2.l", "=a_dx,%a_a30,%a_b20,%a_rte,%a_rte1,[%im6_ip],[%im6_iq],%im6_mr,%im6_ms", AUDIO (AMTTS) | (0x0A << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},
  {"amttssa", "=a_dx,%ra,%rb,%dhy,[%im5_i],%im5_m", AUDIO (AMTTS) | (0x0C << 5), 4, ATTR (AUDIO_ISAEXT), 0, NULL, 0, NULL},

  /* DSP ISA.  */
  /* ALU2 Bit 9-6 = 0000.  */
  {"add64", "=rt,%ra,%rb",	ALU2 (ADD64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sub64", "=rt,%ra,%rb",	ALU2 (SUB64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smal", "=rt,%ra,%rb",	ALU2 (SMAL), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"radd64", "=rt,%ra,%rb",	ALU2 (RADD64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"rsub64", "=rt,%ra,%rb",	ALU2 (RSUB64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"uradd64", "=rt,%ra,%rb",	ALU2 (URADD64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ursub64", "=rt,%ra,%rb",	ALU2 (URSUB64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kadd64", "=rt,%ra,%rb",	ALU2 (KADD64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ksub64", "=rt,%ra,%rb",	ALU2 (KSUB64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ukadd64", "=rt,%ra,%rb",	ALU2 (UKADD64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"uksub64", "=rt,%ra,%rb",	ALU2 (UKSUB64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  /* ALU2 Bit 9-6 = 0001.  */
  {"smar64", "=rt,%ra,%rb",	ALU2_1 (SMAR64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"umar64", "=rt,%ra,%rb",	ALU2_1 (UMAR64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smsr64", "=rt,%ra,%rb",	ALU2_1 (SMSR64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"umsr64", "=rt,%ra,%rb",	ALU2_1 (UMSR64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmar64", "=rt,%ra,%rb",	ALU2_1 (KMAR64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ukmar64", "=rt,%ra,%rb",	ALU2_1 (UKMAR64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmsr64", "=rt,%ra,%rb",	ALU2_1 (KMSR64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ukmsr64", "=rt,%ra,%rb",	ALU2_1 (UKMSR64), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smalda", "=rt,%ra,%rb",	ALU2_1 (SMALDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smslda", "=rt,%ra,%rb",	ALU2_1 (SMSLDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smalds", "=rt,%ra,%rb",	ALU2_1 (SMALDS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smalbb", "=rt,%ra,%rb",	ALU2_1 (SMALBB), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smalxda", "=rt,%ra,%rb",	ALU2_1 (SMALXDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smslxda", "=rt,%ra,%rb",	ALU2_1 (SMSLXDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smalxds", "=rt,%ra,%rb",	ALU2_1 (SMALXDS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smalbt", "=rt,%ra,%rb",	ALU2_1 (SMALBT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smalbt", "=rt,%ra,%rb",	ALU2_1 (SMALBT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smaldrs", "=rt,%ra,%rb",	ALU2_1 (SMALDRS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smaltt", "=rt,%ra,%rb",	ALU2_1 (SMALTT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smds", "=rt,%ra,%rb",	ALU2_1 (SMDS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smxds", "=rt,%ra,%rb",	ALU2_1 (SMXDS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smdrs", "=rt,%ra,%rb",	ALU2_1 (SMDRS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmadrs", "=rt,%ra,%rb",	ALU2_1 (KMADRS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmads", "=rt,%ra,%rb",	ALU2_1 (KMADS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmaxds", "=rt,%ra,%rb",	ALU2_1 (KMAXDS), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  /* DSP MISC.  */
  {"bpick", "=rt,%ra,%rb,%rd",	MISC (BPICK), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  /* ALU_2 KMxy.  */
  {"khm16", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (9) | N32_BIT (8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"khmx16", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (9) | N32_BIT (8) | N32_BIT (6), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smul16", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (9), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smulx16", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (9) | N32_BIT (6), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"umul16", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (9) | N32_BIT (7), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"umulx16", "=rt,%ra,%rb",	ALU2 (KMxy) | N32_BIT (9) | N32_BIT (7) | N32_BIT (6), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  /* ALU2 Bit 9-6 = 0010.  */
  {"kadd16", "=rt,%ra,%rb",	ALU2_2 (KADD16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ksub16", "=rt,%ra,%rb",	ALU2_2 (KSUB16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kcras16", "=rt,%ra,%rb",	ALU2_2 (KCRAS16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kcrsa16", "=rt,%ra,%rb",	ALU2_2 (KCRSA16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kadd8", "=rt,%ra,%rb",	ALU2_2 (KADD8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ksub8", "=rt,%ra,%rb",	ALU2_2 (KSUB8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"wext", "=rt,%ra,%rb",	ALU2_2 (WEXT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"wexti", "=rt,%ra,%ib5u",	ALU2_2 (WEXTI), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ukadd16", "=rt,%ra,%rb",	ALU2_2 (UKADD16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"uksub16", "=rt,%ra,%rb",	ALU2_2 (UKSUB16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ukcras16", "=rt,%ra,%rb",	ALU2_2 (UKCRAS16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ukcrsa16", "=rt,%ra,%rb",	ALU2_2 (UKCRSA16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ukadd8", "=rt,%ra,%rb",	ALU2_2 (UKADD8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"uksub8", "=rt,%ra,%rb",	ALU2_2 (UKSUB8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  /* ONEOP.  */
#define DSP_ONEOP(n) ((n) << 10)
  {"sunpkd810", "=rt,%ra",	ALU2_2 (ONEOP) | DSP_ONEOP (0x0), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sunpkd820", "=rt,%ra",	ALU2_2 (ONEOP) | DSP_ONEOP (0x1), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sunpkd830", "=rt,%ra",	ALU2_2 (ONEOP) | DSP_ONEOP (0x2), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sunpkd831", "=rt,%ra",	ALU2_2 (ONEOP) | DSP_ONEOP (0x3), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"zunpkd810", "=rt,%ra",	ALU2_2 (ONEOP) | DSP_ONEOP (0x4), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"zunpkd820", "=rt,%ra",	ALU2_2 (ONEOP) | DSP_ONEOP (0x5), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"zunpkd830", "=rt,%ra",	ALU2_2 (ONEOP) | DSP_ONEOP (0x6), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"zunpkd831", "=rt,%ra",	ALU2_2 (ONEOP) | DSP_ONEOP (0x7), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kabs", "=rt,%ra",		ALU2 (ABS), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"kabs16", "=rt,%ra",		ALU2_2 (ONEOP) | DSP_ONEOP (0x8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kabs8", "=rt,%ra",		ALU2_2 (ONEOP) | DSP_ONEOP (0xc), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"insb", "=rt,%ra,%ib2u",	ALU2_2 (ONEOP) | DSP_ONEOP (0x10), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smbb", "=rt,%ra,%rb",	ALU2_2 (SMBB), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smbt", "=rt,%ra,%rb",	ALU2_2 (SMBT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smtt", "=rt,%ra,%rb",	ALU2_2 (SMTT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmabb", "=rt,%ra,%rb",	ALU2_2 (KMABB), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmabt", "=rt,%ra,%rb",	ALU2_2 (KMABT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmatt", "=rt,%ra,%rb",	ALU2_2 (KMATT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmda", "=rt,%ra,%rb",	ALU2_2 (KMDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmxda", "=rt,%ra,%rb",	ALU2_2 (KMXDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmada", "=rt,%ra,%rb",	ALU2_2 (KMADA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmaxda", "=rt,%ra,%rb",	ALU2_2 (KMAXDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmsda", "=rt,%ra,%rb",	ALU2_2 (KMSDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmsxda", "=rt,%ra,%rb",	ALU2_2 (KMSXDA), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"radd16", "=rt,%ra,%rb",	ALU2_2 (RADD16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"rsub16", "=rt,%ra,%rb",	ALU2_2 (RSUB16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"rcras16", "=rt,%ra,%rb",	ALU2_2 (RCRAS16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"rcrsa16", "=rt,%ra,%rb",	ALU2_2 (RCRSA16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"radd8", "=rt,%ra,%rb",	ALU2_2 (RADD8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"rsub8", "=rt,%ra,%rb",	ALU2_2 (RSUB8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"raddw", "=rt,%ra,%rb",	ALU2_2 (RADDW), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"rsubw", "=rt,%ra,%rb",	ALU2_2 (RSUBW), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"uradd16", "=rt,%ra,%rb",	ALU2_2 (URADD16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ursub16", "=rt,%ra,%rb",	ALU2_2 (URSUB16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"urcras16", "=rt,%ra,%rb",	ALU2_2 (URCRAS16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"urcrsa16", "=rt,%ra,%rb",	ALU2_2 (URCRSA16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"uradd8", "=rt,%ra,%rb",	ALU2_2 (URADD8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ursub8", "=rt,%ra,%rb",	ALU2_2 (URSUB8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"uraddw", "=rt,%ra,%rb",	ALU2_2 (URADDW), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ursubw", "=rt,%ra,%rb",	ALU2_2 (URSUBW), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"add16", "=rt,%ra,%rb",	ALU2_2 (ADD16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sub16", "=rt,%ra,%rb",	ALU2_2 (SUB16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"cras16", "=rt,%ra,%rb",	ALU2_2 (CRAS16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"crsa16", "=rt,%ra,%rb",	ALU2_2 (CRSA16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"add8", "=rt,%ra,%rb",	ALU2_2 (ADD8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sub8", "=rt,%ra,%rb",	ALU2_2 (SUB8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"bitrev", "=rt,%ra,%rb",	ALU2_2 (BITREV), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"bitrevi", "=rt,%ra,%ib5u",	ALU2_2 (BITREVI), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smmul", "=rt,%ra,%rb",	ALU2_2 (SMMUL), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smmul.u", "=rt,%ra,%rb",	ALU2_2 (SMMULu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmmac", "=rt,%ra,%rb",	ALU2_2 (KMMAC), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmmac.u", "=rt,%ra,%rb",	ALU2_2 (KMMACu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmmsb", "=rt,%ra,%rb",	ALU2_2 (KMMSB), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmmsb.u", "=rt,%ra,%rb",	ALU2_2 (KMMSBu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kwmmul", "=rt,%ra,%rb",	ALU2_2 (KWMMUL), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kwmmul.u", "=rt,%ra,%rb",	ALU2_2 (KWMMULu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  /* ALU2 Bit 9-6 = 0010.  */
  {"smmwb", "=rt,%ra,%rb",	ALU2_3 (SMMWB), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smmwb.u", "=rt,%ra,%rb",	ALU2_3 (SMMWBu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smmwt", "=rt,%ra,%rb",	ALU2_3 (SMMWT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smmwt.u", "=rt,%ra,%rb",	ALU2_3 (SMMWTu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmmawb", "=rt,%ra,%rb",	ALU2_3 (KMMAWB), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmmawb.u", "=rt,%ra,%rb",	ALU2_3 (KMMAWBu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmmawt", "=rt,%ra,%rb",	ALU2_3 (KMMAWT), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kmmawt.u", "=rt,%ra,%rb",	ALU2_3 (KMMAWTu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"pktt16", "=rt,%ra,%rb",	ALU2_3 (PKTT16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"pktb16", "=rt,%ra,%rb",	ALU2_3 (PKTB16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"pkbt16", "=rt,%ra,%rb",	ALU2_3 (PKBT16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"pkbb16", "=rt,%ra,%rb",	ALU2_3 (PKBB16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sclip32", "=rt,%ra,%ib5u",	ALU2 (CLIPS), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"sclip16", "=rt,%ra,%ib4u",	ALU2_3 (SCLIP16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smax16", "=rt,%ra,%rb",	ALU2_3 (SMAX16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smax8", "=rt,%ra,%rb",	ALU2_3 (SMAX8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"uclip32", "=rt,%ra,%ib5u",	ALU2 (CLIP), 4, ATTR (PERF_EXT), 0, NULL, 0, NULL},
  {"uclip16", "=rt,%ra,%ib4u",	ALU2_3 (UCLIP16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"umax16", "=rt,%ra,%rb",	ALU2_3 (UMAX16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"umax8", "=rt,%ra,%rb",	ALU2_3 (UMAX8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sra16", "=rt,%ra,%rb",	ALU2_3 (SRA16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sra16.u", "=rt,%ra,%rb",	ALU2_3 (SRA16u), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"srl16", "=rt,%ra,%rb",	ALU2_3 (SRL16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"srl16.u", "=rt,%ra,%rb",	ALU2_3 (SRL16u), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sll16", "=rt,%ra,%rb",	ALU2_3 (SLL16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kslra16", "=rt,%ra,%rb",	ALU2_3 (KSLRA16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ksll16", "=rt,%ra,%rb",	ALU2_3 (KSLRA16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kslra16.u", "=rt,%ra,%rb",	ALU2_3 (KSLRA16u), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"sra.u", "=rt,%ra,%rb",	ALU2_3 (SRAu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"srai16", "=rt,%ra,%ib4u",	ALU2_3 (SRAI16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"srai16.u", "=rt,%ra,%ib4u",	ALU2_3 (SRAI16u), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"srli16", "=rt,%ra,%ib4u",	ALU2_3 (SRLI16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"srli16.u", "=rt,%ra,%ib4u",	ALU2_3 (SRLI16u), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"slli16", "=rt,%ra,%ib4u",	ALU2_3 (SLLI16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kslli16", "=rt,%ra,%ib4u",	ALU2_3 (KSLLI16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"kslli", "=rt,%ra,%ib5u",	ALU2_3 (KSLLI), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"srai.u", "=rt,%ra,%ib5u",	ALU2_3 (SRAIu), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"cmpeq16", "=rt,%ra,%rb",	ALU2_3 (CMPEQ16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"scmplt16", "=rt,%ra,%rb",	ALU2_3 (SCMPLT16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"scmple16", "=rt,%ra,%rb",	ALU2_3 (SCMPLE16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smin16", "=rt,%ra,%rb",	ALU2_3 (SMIN16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"cmpeq8", "=rt,%ra,%rb",	ALU2_3 (CMPEQ8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"scmplt8", "=rt,%ra,%rb",	ALU2_3 (SCMPLT8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"scmple8", "=rt,%ra,%rb",	ALU2_3 (SCMPLE8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"smin8", "=rt,%ra,%rb",	ALU2_3 (SMIN8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ucmplt16", "=rt,%ra,%rb",	ALU2_3 (UCMPLT16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ucmple16", "=rt,%ra,%rb",	ALU2_3 (UCMPLE16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"umin16", "=rt,%ra,%rb",	ALU2_3 (UMIN16), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ucmplt8", "=rt,%ra,%rb",	ALU2_3 (UCMPLT8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"ucmple8", "=rt,%ra,%rb",	ALU2_3 (UCMPLE8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"umin8", "=rt,%ra,%rb",	ALU2_3 (UMIN8), 4, ATTR (DSP_ISAEXT), 0, NULL, 0, NULL},
  {"mtlbi", "%i16s1",		BR2 (SOP0) | N32_BIT (20), 4, ATTR (ZOL) | ATTR (DSP_ISAEXT) | ATTR (PCREL), 0, NULL, 0, NULL},
  {"mtlei", "%i16s1",		BR2 (SOP0) | N32_BIT (21), 4, ATTR (ZOL) | ATTR (DSP_ISAEXT) | ATTR (PCREL), 0, NULL, 0, NULL},
  {NULL, NULL, 0, 0, 0, 0, NULL, 0, NULL},
};

const keyword_t nds32_keyword_gpr[] =
{
  /* Standard names.  */
  {"r0", 0, ATTR (RDREG)}, {"r1", 1, ATTR (RDREG)}, {"r2", 2, ATTR (RDREG)},
  {"r3", 3, ATTR (RDREG)}, {"r4", 4, ATTR (RDREG)}, {"r5", 5, ATTR (RDREG)},
  {"r6", 6, ATTR (RDREG)}, {"r7", 7, ATTR (RDREG)}, {"r8", 8, ATTR (RDREG)},
  {"r9", 9, ATTR (RDREG)}, {"r10", 10, ATTR (RDREG)}, {"r11", 11, 0},
  {"r12", 12, 0}, {"r13", 13, 0}, {"r14", 14, 0}, {"r15", 15, ATTR (RDREG)},
  {"r16", 16, 0}, {"r17", 17, 0}, {"r18", 18, 0}, {"r19", 19, 0},
  {"r20", 20, 0}, {"r21", 21, 0}, {"r22", 22, 0}, {"r23", 23, 0},
  {"r24", 24, 0}, {"r25", 25, 0},
  {"p0", 26, 0}, {"p1", 27, 0},
  {"fp", 28, ATTR (RDREG)}, {"gp", 29, ATTR (RDREG)},
  {"lp", 30, ATTR (RDREG)}, {"sp", 31, ATTR (RDREG)},
  {"r26", 26, 0}, {"r27", 27, 0},
  {"r28", 28, ATTR (RDREG)}, {"r29", 29, ATTR (RDREG)},
  {"r30", 30, ATTR (RDREG)}, {"r31", 31, ATTR (RDREG)},
  /* Names for parameter passing.  */
  {"a0", 0, ATTR (RDREG)}, {"a1", 1, ATTR (RDREG)},
  {"a2", 2, ATTR (RDREG)}, {"a3", 3, ATTR (RDREG)},
  {"a4", 4, ATTR (RDREG)}, {"a5", 5, ATTR (RDREG)},
  /* Names reserved for 5-bit addressing only.  */
  {"s0", 6, ATTR (RDREG)}, {"s1", 7, ATTR (RDREG)},
  {"s2", 8, ATTR (RDREG)}, {"s3", 9, ATTR (RDREG)},
  {"s4", 10, ATTR (RDREG)}, {"s5", 11, 0}, {"s6", 12, 0}, {"s7", 13, 0},
  {"s8", 14, 0}, {"s9", 28, ATTR (RDREG)},
  {"ta", 15, ATTR (RDREG)},
  {"t0", 16, 0}, {"t1", 17, 0}, {"t2", 18, 0}, {"t3", 19, 0},
  {"t4", 20, 0}, {"t5", 21, 0}, {"t6", 22, 0}, {"t7", 23, 0},
  {"t8", 24, 0}, {"t9", 25, 0},
  /* Names reserved for 4-bit addressing only.  */
  {"h0", 0, ATTR (RDREG)}, {"h1", 1, ATTR (RDREG)},
  {"h2", 2, ATTR (RDREG)}, {"h3", 3, ATTR (RDREG)},
  {"h4", 4, ATTR (RDREG)}, {"h5", 5, ATTR (RDREG)},
  {"h6", 6, ATTR (RDREG)}, {"h7", 7, ATTR (RDREG)},
  {"h8", 8, ATTR (RDREG)}, {"h9", 9, ATTR (RDREG)},
  {"h10", 10, ATTR (RDREG)}, {"h11", 11, 0},
  {"h12", 16, 0}, {"h13", 17, 0}, {"h14", 18, 0}, {"h15", 19, 0},
  /* Names reserved for 3-bit addressing only.  */
  {"o0", 0, ATTR (RDREG)}, {"o1", 1, ATTR (RDREG)},
  {"o2", 2, ATTR (RDREG)}, {"o3", 3, ATTR (RDREG)},
  {"o4", 4, ATTR (RDREG)}, {"o5", 5, ATTR (RDREG)},
  {"o6", 6, ATTR (RDREG)}, {"o7", 7, ATTR (RDREG)},
  {NULL, 0, 0}
};

static const keyword_t keyword_usr[] =
{
  {"d0.lo", USRIDX (0, 0), 0},
  {"d0.hi", USRIDX (0, 1), 0},
  {"d1.lo", USRIDX (0, 2), 0},
  {"d1.hi", USRIDX (0, 3), 0},
  {"lb", USRIDX (0, 25), 0},
  {"le", USRIDX (0, 26), 0},
  {"lc", USRIDX (0, 27), 0},
  {"itb", USRIDX (0, 28), 0},
  {"ifc_lp", USRIDX (0, 29), 0},
  {"pc", USRIDX (0, 31), 0},

  {"dma_cfg", USRIDX (1, 0), 0},
  {"dma_gcsw", USRIDX (1, 1), 0},
  {"dma_chnsel", USRIDX (1, 2), 0},
  {"dma_act", USRIDX (1, 3), 0},
  {"dma_setup", USRIDX (1, 4), 0},
  {"dma_isaddr", USRIDX (1, 5), 0},
  {"dma_esaddr", USRIDX (1, 6), 0},
  {"dma_tcnt", USRIDX (1, 7), 0},
  {"dma_status", USRIDX (1, 8), 0},
  {"dma_2dset", USRIDX (1, 9), 0},
  {"dma_rcnt", USRIDX (1, 23), 0},
  {"dma_hstatus", USRIDX (1, 24), 0},
  {"dma_2dsctl", USRIDX (1, 25), 0},

  {"pfmc0", USRIDX (2, 0), 0},
  {"pfmc1", USRIDX (2, 1), 0},
  {"pfmc2", USRIDX (2, 2), 0},
  {"pfm_ctl", USRIDX (2, 4), 0},

  {NULL, 0, 0}
};

static const keyword_t keyword_dxr[] =
{
  {"d0", 0, 0}, {"d1", 1, 0}, {NULL, 0, 0}
};

static const keyword_t keyword_sr[] =
{
  {"cpu_ver", SRIDX (0, 0, 0), 0},	{"cr0", SRIDX (0, 0, 0), 0},
  {"icm_cfg", SRIDX (0, 1, 0), 0},	{"cr1", SRIDX (0, 1, 0), 0},
  {"dcm_cfg", SRIDX (0, 2, 0), 0},	{"cr2", SRIDX (0, 2, 0), 0},
  {"mmu_cfg", SRIDX (0, 3, 0), 0},	{"cr3", SRIDX (0, 3, 0), 0},
  {"msc_cfg", SRIDX (0, 4, 0), 0},	{"cr4", SRIDX (0, 4, 0), 0},
  {"msc_cfg2", SRIDX (0, 4, 1), 0},	{"cr7", SRIDX (0, 4, 1), 0},
  {"core_id", SRIDX (0, 0, 1), 0},	{"cr5", SRIDX (0, 0, 1), 0},
  {"fucop_exist", SRIDX (0, 5, 0), 0},	{"cr6", SRIDX (0, 5, 0), 0},

  {"psw", SRIDX (1, 0, 0), 0},		{"ir0", SRIDX (1, 0, 0), 0},
  {"ipsw", SRIDX (1, 0, 1), 0},		{"ir1", SRIDX (1, 0, 1), 0},
  {"p_ipsw", SRIDX (1, 0, 2), 0},	{"ir2", SRIDX (1, 0, 2), 0},
  {"ivb", SRIDX (1, 1, 1), 0},		{"ir3", SRIDX (1, 1, 1), 0},
  {"eva", SRIDX (1, 2, 1), 0},		{"ir4", SRIDX (1, 2, 1), 0},
  {"p_eva", SRIDX (1, 2, 2), 0},	{"ir5", SRIDX (1, 2, 2), 0},
  {"itype", SRIDX (1, 3, 1), 0},	{"ir6", SRIDX (1, 3, 1), 0},
  {"p_itype", SRIDX (1, 3, 2), 0},	{"ir7", SRIDX (1, 3, 2), 0},
  {"merr", SRIDX (1, 4, 1), 0},		{"ir8", SRIDX (1, 4, 1), 0},
  {"ipc", SRIDX (1, 5, 1), 0},		{"ir9", SRIDX (1, 5, 1), 0},
  {"p_ipc", SRIDX (1, 5, 2), 0},	{"ir10", SRIDX (1, 5, 2), 0},
  {"oipc", SRIDX (1, 5, 3), 0},		{"ir11", SRIDX (1, 5, 3), 0},
  {"dipc", SRIDX (1, 5, 3), 0},
  {"p_p0", SRIDX (1, 6, 2), 0},		{"ir12", SRIDX (1, 6, 2), 0},
  {"p_p1", SRIDX (1, 7, 2), 0},		{"ir13", SRIDX (1, 7, 2), 0},
  {"int_mask", SRIDX (1, 8, 0), 0},	{"ir14", SRIDX (1, 8, 0), 0},
  {"int_pend", SRIDX (1, 9, 0), 0},	{"ir15", SRIDX (1, 9, 0), 0},
  {"sp_usr", SRIDX (1, 10, 0), 0},	{"ir16", SRIDX (1, 10, 0), 0},
  {"sp_priv", SRIDX (1, 10, 1), 0},	{"ir17", SRIDX (1, 10, 1), 0},
  {"int_pri", SRIDX (1, 11, 0), 0},	{"ir18", SRIDX (1, 11, 0), 0},
  {"int_ctrl", SRIDX (1, 1, 2), 0},	{"ir19", SRIDX (1, 1, 2), 0},
  {"sp_usr1", SRIDX (1, 10, 2), 0},	{"ir20", SRIDX (1, 10, 2), 0},
  {"sp_priv1", SRIDX (1, 10, 3), 0},	{"ir21", SRIDX (1, 10, 3), 0},
  {"sp_usr2", SRIDX (1, 10, 4), 0},	{"ir22", SRIDX (1, 10, 4), 0},
  {"sp_priv2", SRIDX (1, 10, 5), 0},	{"ir23", SRIDX (1, 10, 5), 0},
  {"sp_usr3", SRIDX (1, 10, 6), 0},	{"ir24", SRIDX (1, 10, 6), 0},
  {"sp_priv3", SRIDX (1, 10, 7), 0},	{"ir25", SRIDX (1, 10, 7), 0},
  {"int_mask2", SRIDX (1, 8, 1), 0},	{"ir26", SRIDX (1, 8, 1), 0},
  {"int_pend2", SRIDX (1, 9, 1), 0},	{"ir27", SRIDX (1, 9, 1), 0},
  {"int_pri2", SRIDX (1, 11, 1), 0},	{"ir28", SRIDX (1, 11, 1), 0},
  {"int_trigger", SRIDX (1, 9, 4), 0},	{"ir29", SRIDX (1, 9, 4), 0},
  {"int_gpr_push_dis", SRIDX(1, 1, 3), 0}, {"ir30", SRIDX (1, 1, 3), 0},
  {"int_mask3", SRIDX(1, 8, 2), 0},	{"ir31", SRIDX (1, 8, 2), 0},
  {"int_pend3", SRIDX(1, 9, 2), 0},	{"ir32", SRIDX (1, 9, 2), 0},
  {"int_pri3", SRIDX(1, 11, 2), 0},	{"ir33", SRIDX (1, 11, 2), 0},
  {"int_pri4", SRIDX(1, 11, 3), 0},	{"ir34", SRIDX (1, 11, 3), 0},
  {"int_trigger2", SRIDX(1, 9, 5), 0},	{"ir35", SRIDX (1, 9, 5), 0},

  {"mmu_ctl", SRIDX (2, 0, 0), 0},	{"mr0", SRIDX (2, 0, 0), 0},
  {"l1_pptb", SRIDX (2, 1, 0), 0},	{"mr1", SRIDX (2, 1, 0), 0},
  {"tlb_vpn", SRIDX (2, 2, 0), 0},	{"mr2", SRIDX (2, 2, 0), 0},
  {"tlb_data", SRIDX (2, 3, 0), 0},	{"mr3", SRIDX (2, 3, 0), 0},
  {"tlb_misc", SRIDX (2, 4, 0), 0},	{"mr4", SRIDX (2, 4, 0), 0},
  {"vlpt_idx", SRIDX (2, 5, 0), 0},	{"mr5", SRIDX (2, 5, 0), 0},
  {"ilmb", SRIDX (2, 6, 0), 0},		{"mr6", SRIDX (2, 6, 0), 0},
  {"dlmb", SRIDX (2, 7, 0), 0},		{"mr7", SRIDX (2, 7, 0), 0},
  {"cache_ctl", SRIDX (2, 8, 0), 0},	{"mr8", SRIDX (2, 8, 0), 0},
  {"hsmp_saddr", SRIDX (2, 9, 0), 0},	{"mr9", SRIDX (2, 9, 0), 0},
  {"hsmp_eaddr", SRIDX (2, 9, 1), 0},	{"mr10", SRIDX (2, 9, 1), 0},
  {"bg_region", SRIDX (2, 0, 1), 0},	{"mr11", SRIDX (2, 0, 1), 0},

  {"pfmc0", SRIDX (4, 0, 0), 0},	{"pfr0", SRIDX (4, 0, 0), 0},
  {"pfmc1", SRIDX (4, 0, 1), 0},	{"pfr1", SRIDX (4, 0, 1), 0},
  {"pfmc2", SRIDX (4, 0, 2), 0},	{"pfr2", SRIDX (4, 0, 2), 0},
  {"pfm_ctl", SRIDX (4, 1, 0), 0},	{"pfr3", SRIDX (4, 1, 0), 0},
  {"pft_ctl", SRIDX (4, 2, 0), 0},	{"pfr4", SRIDX (4, 2, 0), 0},
  {"hsp_ctl", SRIDX (4, 6, 0), 0},	{"hspr0", SRIDX (4, 6, 0), 0},
  {"sp_bound", SRIDX (4, 6, 1), 0},	{"hspr1", SRIDX (4, 6, 1), 0},
  {"sp_bound_priv", SRIDX (4, 6, 2), 0},{"hspr2", SRIDX (4, 6, 2), 0},
  {"sp_base", SRIDX (4, 6, 3), 0},	{"hspr3", SRIDX (4, 6, 3), 0},
  {"sp_base_priv", SRIDX (4, 6, 4), 0},	{"hspr4", SRIDX (4, 6, 4), 0},

  {"dma_cfg", SRIDX (5, 0, 0), 0},	{"dmar0", SRIDX (5, 0, 0), 0},
  {"dma_gcsw", SRIDX (5, 1, 0), 0},	{"dmar1", SRIDX (5, 1, 0), 0},
  {"dma_chnsel", SRIDX (5, 2, 0), 0},	{"dmar2", SRIDX (5, 2, 0), 0},
  {"dma_act", SRIDX (5, 3, 0), 0},	{"dmar3", SRIDX (5, 3, 0), 0},
  {"dma_setup", SRIDX (5, 4, 0), 0},	{"dmar4", SRIDX (5, 4, 0), 0},
  {"dma_isaddr", SRIDX (5, 5, 0), 0},	{"dmar5", SRIDX (5, 5, 0), 0},
  {"dma_esaddr", SRIDX (5, 6, 0), 0},	{"dmar6", SRIDX (5, 6, 0), 0},
  {"dma_tcnt", SRIDX (5, 7, 0), 0},	{"dmar7", SRIDX (5, 7, 0), 0},
  {"dma_status", SRIDX (5, 8, 0), 0},	{"dmar8", SRIDX (5, 8, 0), 0},
  {"dma_2dset", SRIDX (5, 9, 0), 0},	{"dmar9", SRIDX (5, 9, 0), 0},
  {"dma_2dsctl", SRIDX (5, 9, 1), 0},	{"dmar10", SRIDX (5, 9, 1), 0},
  {"dma_rcnt", SRIDX (5, 7, 1), 0},	{"dmar11", SRIDX (5, 7, 1), 0},
  {"dma_hstatus", SRIDX (5, 8, 1), 0},	{"dmar12", SRIDX (5, 8, 1), 0},

  {"sdz_ctl", SRIDX (2, 15, 0), 0},	{"idr0", SRIDX (2, 15, 0), 0},
  {"misc_ctl", SRIDX (2, 15, 1), 0},	{"n12misc_ctl", SRIDX (2, 15, 1), 0},
  {"idr1", SRIDX (2, 15, 1), 0},
  {"ecc_misc", SRIDX (2, 15, 2), 0},	{"idr2", SRIDX (2, 15, 2), 0},

  {"secur0", SRIDX (6, 0, 0), 0},	{"sfcr", SRIDX (6, 0, 0), 0},
  {"secur1", SRIDX (6, 1, 0), 0},	{"sign", SRIDX (6, 1, 0), 0},
  {"secur2", SRIDX (6, 1, 1), 0},	{"isign", SRIDX (6, 1, 1), 0},
  {"secur3", SRIDX (6, 1, 2), 0},	{"p_isign", SRIDX (6, 1, 2), 0},

  {"prusr_acc_ctl", SRIDX (4, 4, 0), 0},
  {"fucpr", SRIDX (4, 5, 0), 0},	{"fucop_ctl", SRIDX (4, 5, 0), 0},

  {"bpc0", SRIDX (3, 0, 0), 0},		{"dr0", SRIDX (3, 0, 0), 0},
  {"bpc1", SRIDX (3, 0, 1), 0},		{"dr5", SRIDX (3, 0, 1), 0},
  {"bpc2", SRIDX (3, 0, 2), 0},		{"dr10", SRIDX (3, 0, 2), 0},
  {"bpc3", SRIDX (3, 0, 3), 0},		{"dr15", SRIDX (3, 0, 3), 0},
  {"bpc4", SRIDX (3, 0, 4), 0},		{"dr20", SRIDX (3, 0, 4), 0},
  {"bpc5", SRIDX (3, 0, 5), 0},		{"dr25", SRIDX (3, 0, 5), 0},
  {"bpc6", SRIDX (3, 0, 6), 0},		{"dr30", SRIDX (3, 0, 6), 0},
  {"bpc7", SRIDX (3, 0, 7), 0},		{"dr35", SRIDX (3, 0, 7), 0},
  {"bpa0", SRIDX (3, 1, 0), 0},		{"dr1", SRIDX (3, 1, 0), 0},
  {"bpa1", SRIDX (3, 1, 1), 0},		{"dr6", SRIDX (3, 1, 1), 0},
  {"bpa2", SRIDX (3, 1, 2), 0},		{"dr11", SRIDX (3, 1, 2), 0},
  {"bpa3", SRIDX (3, 1, 3), 0},		{"dr16", SRIDX (3, 1, 3), 0},
  {"bpa4", SRIDX (3, 1, 4), 0},		{"dr21", SRIDX (3, 1, 4), 0},
  {"bpa5", SRIDX (3, 1, 5), 0},		{"dr26", SRIDX (3, 1, 5), 0},
  {"bpa6", SRIDX (3, 1, 6), 0},		{"dr31", SRIDX (3, 1, 6), 0},
  {"bpa7", SRIDX (3, 1, 7), 0},		{"dr36", SRIDX (3, 1, 7), 0},
  {"bpam0", SRIDX (3, 2, 0), 0},	{"dr2", SRIDX (3, 2, 0), 0},
  {"bpam1", SRIDX (3, 2, 1), 0},	{"dr7", SRIDX (3, 2, 1), 0},
  {"bpam2", SRIDX (3, 2, 2), 0},	{"dr12", SRIDX (3, 2, 2), 0},
  {"bpam3", SRIDX (3, 2, 3), 0},	{"dr17", SRIDX (3, 2, 3), 0},
  {"bpam4", SRIDX (3, 2, 4), 0},	{"dr22", SRIDX (3, 2, 4), 0},
  {"bpam5", SRIDX (3, 2, 5), 0},	{"dr27", SRIDX (3, 2, 5), 0},
  {"bpam6", SRIDX (3, 2, 6), 0},	{"dr32", SRIDX (3, 2, 6), 0},
  {"bpam7", SRIDX (3, 2, 7), 0},	{"dr37", SRIDX (3, 2, 7), 0},
  {"bpv0", SRIDX (3, 3, 0), 0},		{"dr3", SRIDX (3, 3, 0), 0},
  {"bpv1", SRIDX (3, 3, 1), 0},		{"dr8", SRIDX (3, 3, 1), 0},
  {"bpv2", SRIDX (3, 3, 2), 0},		{"dr13", SRIDX (3, 3, 2), 0},
  {"bpv3", SRIDX (3, 3, 3), 0},		{"dr18", SRIDX (3, 3, 3), 0},
  {"bpv4", SRIDX (3, 3, 4), 0},		{"dr23", SRIDX (3, 3, 4), 0},
  {"bpv5", SRIDX (3, 3, 5), 0},		{"dr28", SRIDX (3, 3, 5), 0},
  {"bpv6", SRIDX (3, 3, 6), 0},		{"dr33", SRIDX (3, 3, 6), 0},
  {"bpv7", SRIDX (3, 3, 7), 0},		{"dr38", SRIDX (3, 3, 7), 0},
  {"bpcid0", SRIDX (3, 4, 0), 0},	{"dr4", SRIDX (3, 4, 0), 0},
  {"bpcid1", SRIDX (3, 4, 1), 0},	{"dr9", SRIDX (3, 4, 1), 0},
  {"bpcid2", SRIDX (3, 4, 2), 0},	{"dr14", SRIDX (3, 4, 2), 0},
  {"bpcid3", SRIDX (3, 4, 3), 0},	{"dr19", SRIDX (3, 4, 3), 0},
  {"bpcid4", SRIDX (3, 4, 4), 0},	{"dr24", SRIDX (3, 4, 4), 0},
  {"bpcid5", SRIDX (3, 4, 5), 0},	{"dr29", SRIDX (3, 4, 5), 0},
  {"bpcid6", SRIDX (3, 4, 6), 0},	{"dr34", SRIDX (3, 4, 6), 0},
  {"bpcid7", SRIDX (3, 4, 7), 0},	{"dr39", SRIDX (3, 4, 7), 0},
  {"edm_cfg", SRIDX (3, 5, 0), 0},	{"dr40", SRIDX (3, 5, 0), 0},
  {"edmsw", SRIDX (3, 6, 0), 0},	{"dr41", SRIDX (3, 6, 0), 0},
  {"edm_ctl", SRIDX (3, 7, 0), 0},	{"dr42", SRIDX (3, 7, 0), 0},
  {"edm_dtr", SRIDX (3, 8, 0), 0},	{"dr43", SRIDX (3, 8, 0), 0},
  {"bpmtc", SRIDX (3, 9, 0), 0},	{"dr44", SRIDX (3, 9, 0), 0},
  {"dimbr", SRIDX (3, 10, 0), 0},	{"dr45", SRIDX (3, 10, 0), 0},
  {"tecr0", SRIDX (3, 14, 0), 0},	{"dr46", SRIDX (3, 14, 0), 0},
  {"tecr1", SRIDX (3, 14, 1), 0},	{"dr47", SRIDX (3, 14, 1), 0},
  {NULL,0 ,0}
};

static const keyword_t keyword_cp[] =
{
  {"cp0", 0, 0}, {"cp1", 1, 0}, {"cp2", 2, 0}, {"cp3", 3, 0}, {NULL, 0, 0}
};

static const keyword_t keyword_cpr[] =
{
  {"cpr0", 0, 0}, {"cpr1", 1, 0}, {"cpr2", 2, 0}, {"cpr3", 3, 0},
  {"cpr4", 4, 0}, {"cpr5", 5, 0}, {"cpr6", 6, 0}, {"cpr7", 7, 0},
  {"cpr8", 8, 0}, {"cpr9", 9, 0}, {"cpr10", 10, 0}, {"cpr11", 11, 0},
  {"cpr12", 12, 0}, {"cpr13", 13, 0}, {"cpr14", 14, 0}, {"cpr15", 15, 0},
  {"cpr16", 16, 0}, {"cpr17", 17, 0}, {"cpr18", 18, 0}, {"cpr19", 19, 0},
  {"cpr20", 20, 0}, {"cpr21", 21, 0}, {"cpr22", 22, 0}, {"cpr23", 23, 0},
  {"cpr24", 24, 0}, {"cpr25", 25, 0}, {"cpr26", 26, 0}, {"cpr27", 27, 0},
  {"cpr28", 28, 0}, {"cpr29", 29, 0}, {"cpr30", 30, 0}, {"cpr31", 31, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_fsr[] =
{
  {"fs0", 0, 0}, {"fs1", 1, 0}, {"fs2", 2, 0}, {"fs3", 3, 0}, {"fs4", 4, 0},
  {"fs5", 5, 0}, {"fs6", 6, 0}, {"fs7", 7, 0}, {"fs8", 8, 0}, {"fs9", 9, 0},
  {"fs10", 10, 0}, {"fs11", 11, 0}, {"fs12", 12, 0}, {"fs13", 13, 0},
  {"fs14", 14, 0}, {"fs15", 15, 0}, {"fs16", 16, 0}, {"fs17", 17, 0},
  {"fs18", 18, 0}, {"fs19", 19, 0}, {"fs20", 20, 0}, {"fs21", 21, 0},
  {"fs22", 22, 0}, {"fs23", 23, 0}, {"fs24", 24, 0}, {"fs25", 25, 0},
  {"fs26", 26, 0}, {"fs27", 27, 0}, {"fs28", 28, 0}, {"fs29", 29, 0},
  {"fs30", 30, 0}, {"fs31", 31, 0}, {NULL, 0 ,0}
};

static const keyword_t keyword_fdr[] =
{
  {"fd0", 0, 0}, {"fd1", 1, 0}, {"fd2", 2, 0}, {"fd3", 3, 0}, {"fd4", 4, 0},
  {"fd5", 5, 0}, {"fd6", 6, 0}, {"fd7", 7, 0}, {"fd8", 8, 0}, {"fd9", 9, 0},
  {"fd10", 10, 0}, {"fd11", 11, 0}, {"fd12", 12, 0}, {"fd13", 13, 0},
  {"fd14", 14, 0}, {"fd15", 15, 0}, {"fd16", 16, 0}, {"fd17", 17, 0},
  {"fd18", 18, 0}, {"fd19", 19, 0}, {"fd20", 20, 0}, {"fd21", 21, 0},
  {"fd22", 22, 0}, {"fd23", 23, 0}, {"fd24", 24, 0}, {"fd25", 25, 0},
  {"fd26", 26, 0}, {"fd27", 27, 0}, {"fd28", 28, 0}, {"fd29", 29, 0},
  {"fd30", 30, 0}, {"fd31", 31, 0}, {NULL, 0, 0}
};

static const keyword_t keyword_abdim[] =
{
  {"bi", 0, 0}, {"bim", 1, 0}, {"bd", 2, 0}, {"bdm", 3, 0},
  {"ai", 4, 0}, {"aim", 5, 0}, {"ad", 6, 0}, {"adm", 7, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_abm[] =
{
  {"b", 0, 0}, {"bm", 1, 0}, {"bx", 2, 0}, {"bmx", 3, 0},
  {"a", 4, 0}, {"am", 5, 0}, {"ax", 6, 0}, {"amx", 7, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_dtiton[] =
{
  {"iton", 1, 0}, {"ton", 3, 0}, {NULL, 0, 0}
};

static const keyword_t keyword_dtitoff[] =
{
  {"itoff", 1, 0}, {"toff", 3, 0}, {NULL, 0, 0}
};

static const keyword_t keyword_dpref_st[] =
{
  {"srd", 0, 0}, {"mrd", 1, 0}, {"swr", 2, 0}, {"mwr", 3, 0},
  {"pte", 4, 0}, {"clwr", 5, 0}, {NULL, 0, 0}
};

/* CCTL Ra, SubType.  */
static const keyword_t keyword_cctl_st0[] =
{
  {"l1d_ix_inval", 0X0, 0}, {"l1d_ix_wb", 0X1, 0}, {"l1d_ix_wbinval", 0X2, 0},
  {"l1d_va_fillck", 0XB, 0}, {"l1d_va_ulck", 0XC, 0}, {"l1i_ix_inval", 0X10, 0},
  {"l1i_va_fillck", 0X1B, 0}, {"l1i_va_ulck", 0X1C, 0},
  {NULL, 0, 0}
};

/* CCTL Ra, SubType, level.  */
static const keyword_t keyword_cctl_st1[] =
{
  {"l1d_va_inval", 0X8, 0}, {"l1d_va_wb", 0X9, 0},
  {"l1d_va_wbinval", 0XA, 0}, {"l1i_va_inval", 0X18, 0},
  {NULL, 0, 0}
};

/* CCTL Rt, Ra, SubType.  */
static const keyword_t keyword_cctl_st2[] =
{
  {"l1d_ix_rtag", 0X3, 0}, {"l1d_ix_rwd", 0X4, 0},
  {"l1i_ix_rtag", 0X13, 0}, {"l1i_ix_rwd", 0X14, 0},
  {NULL, 0, 0}
};

/* CCTL Rb, Ra, SubType.  */
static const keyword_t keyword_cctl_st3[] =
{
  {"l1d_ix_wtag", 0X5, 0}, {"l1d_ix_wwd", 0X6, 0},
  {"l1i_ix_wtag", 0X15, 0}, {"l1i_ix_wwd", 0X16, 0},
  {NULL, 0, 0}
};

/* CCTL L1D_INVALALL.  */
static const keyword_t keyword_cctl_st4[] =
{
  {"l1d_invalall", 0x7, 0}, {NULL, 0, 0}
};

/* CCTL L1D_WBALL, level.  */
static const keyword_t keyword_cctl_st5[] =
{
  {"l1d_wball", 0xf, 0}, {NULL, 0, 0}
};

static const keyword_t keyword_cctl_lv[] =
{
  {"1level", 0, 0}, {"alevel", 1, 0}, {"0", 0, 0}, {"1", 1, 0},
  {NULL, 0, 0},
};

static const keyword_t keyword_tlbop_st[] =
{
  {"targetread", 0, 0}, {"trd", 0, 0},
  {"targetwrite", 1, 0}, {"twr", 1, 0},
  {"rwrite", 2, 0}, {"rwr", 2, 0},
  {"rwritelock", 3, 0}, {"rwlk", 3, 0},
  {"unlock", 4, 0}, {"unlk", 4, 0},
  {"invalidate", 6, 0}, {"inv", 6, 0},
  {NULL, 0, 0},
};

static const keyword_t keyword_standby_st[] =
{
  {"no_wake_grant", 0, 0},
  {"wake_grant", 1, 0},
  {"wait_done", 2, 0},
  {"0", 0, 0},
  {"1", 1, 0},
  {"2", 2, 0},
  {"3", 3, 0},
  {NULL, 0, 0},
};

static const keyword_t keyword_msync_st[] =
{
  {"all", 0, 0}, {"store", 1, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_im5_i[] =
{
  {"i0", 0, 0}, {"i1", 1, 0}, {"i2", 2, 0}, {"i3", 3, 0},
  {"i4", 4, 0}, {"i5", 5, 0}, {"i6", 6, 0}, {"i7", 7, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_im5_m[] =
{
  {"m0", 0, 0}, {"m1", 1, 0}, {"m2", 2, 0}, {"m3", 3, 0},
  {"m4", 4, 0}, {"m5", 5, 0}, {"m6", 6, 0}, {"m7", 7, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_accumulator[] =
{
  {"d0.lo", 0, 0}, {"d0.hi", 1, 0}, {"d1.lo", 2, 0}, {"d1.hi", 3, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_aridx[] =
{
  {"i0", 0, 0}, {"i1", 1, 0}, {"i2", 2, 0}, {"i3", 3, 0},
  {"i4", 4, 0}, {"i5", 5, 0}, {"i6", 6, 0}, {"i7", 7, 0},
  {"mod", 8, 0}, {"m1", 9, 0}, {"m2", 10, 0}, {"m3",11, 0},
  {"m5",13, 0}, {"m6",14, 0}, {"m7",15, 0},
  {"d0.l24", 16, 0}, {"d1.l24", 17, 0},
  {"shft_ctl0", 18, 0}, {"shft_ctl1", 19, 0},
  {"lb", 24, 0}, {"le", 25, 0}, {"lc", 26, 0}, {"adm_vbase", 27, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_aridx2[] =
{
  {"cbb0", 0, 0}, {"cbb1", 1, 0}, {"cbb2", 2, 0}, {"cbb3", 3, 0},
  {"cbe0", 4, 0}, {"cbe1", 5, 0}, {"cbe2", 6, 0}, {"cbe3", 7, 0},
  {"cb_ctl", 31, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_aridxi[] =
{
  {"i0", 0, 0}, {"i1", 1, 0}, {"i2", 2, 0}, {"i3", 3, 0},
  {"i4", 4, 0}, {"i5", 5, 0}, {"i6", 6, 0}, {"i7", 7, 0},
  {"mod", 8, 0}, {"m1", 9, 0}, {"m2", 10, 0}, {"m3",11, 0},
  {"m5",13, 0}, {"m6",14, 0}, {"m7",15, 0},
  {NULL, 0, 0}
};

static const keyword_t keyword_aridxi_mx[] =
{
  {"m1", 9, 0}, {"m2", 10, 0}, {"m3",11, 0},
  {"m5",13, 0}, {"m6",14, 0}, {"m7",15, 0},
  {NULL, 0, 0}
};

const keyword_t *nds32_keywords[_HW_LAST] =
{
  nds32_keyword_gpr, keyword_usr, keyword_dxr, keyword_sr, keyword_fsr,
  keyword_fdr, keyword_cp, keyword_cpr, keyword_abdim, keyword_abm,
  keyword_dtiton, keyword_dtitoff, keyword_dpref_st,
  keyword_cctl_st0, keyword_cctl_st1, keyword_cctl_st2,
  keyword_cctl_st3, keyword_cctl_st4, keyword_cctl_st5,
  keyword_cctl_lv, keyword_tlbop_st, keyword_standby_st,
  keyword_msync_st,
  keyword_im5_i, keyword_im5_m,
  keyword_accumulator, keyword_aridx, keyword_aridx2,
  keyword_aridxi, keyword_aridxi_mx
};

const keyword_t **nds32_keyword_table[NDS32_CORE_COUNT];
static unsigned int nds32_keyword_count_table[NDS32_CORE_COUNT];
const field_t *nds32_field_table[NDS32_CORE_COUNT];
opcode_t *nds32_opcode_table[NDS32_CORE_COUNT];


/* Hash table for syntax lex.   */
static htab_t field_htab;
/* Hash table for opcodes.  */
static htab_t opcode_htab;
/* Hash table for hardware resources.  */
static htab_t *hw_ktabs;

static hashval_t
htab_hash_hash (const void *p)
{
  struct nds32_hash_entry *h = (struct nds32_hash_entry *) p;

  return htab_hash_string (h->name);
}

static int
htab_hash_eq (const void *p, const void *q)
{
  struct nds32_hash_entry *h = (struct nds32_hash_entry *) p;
  const char *name = (const char *) q;

  return strcmp (name, h->name) == 0;
}


static void
build_operand_hash_table (void)
{
  unsigned k;

  field_htab = htab_create_alloc (128, htab_hash_hash, htab_hash_eq,
				  NULL, xcalloc, free);

  for (k = 0; k < NDS32_CORE_COUNT; k++)
    {
      const field_t *fld;

      fld = nds32_field_table[k];
      if (fld == NULL)
	continue;

      /* Add op-codes.  */
      while (fld->name != NULL)
	{
	  hashval_t hash;
	  const field_t **slot;

	  hash = htab_hash_string (fld->name);
	  slot = (const field_t **)
	    htab_find_slot_with_hash (field_htab, fld->name, hash, INSERT);

	  assert (slot != NULL && *slot == NULL);
	  *slot = fld++;
	}
    }
}

static void
build_keyword_hash_table (void)
{
  unsigned int i, j, k, n;

  /* Count total keyword tables.  */
  for (n = 0, i = 0; i < NDS32_CORE_COUNT; i++)
    {
      n += nds32_keyword_count_table[i];
    }

  /* Allocate space.  */
  hw_ktabs = (htab_t *) malloc (n * sizeof (struct htab));
  for (i = 0; i < n; i++)
    {
      hw_ktabs[i] = htab_create_alloc (128, htab_hash_hash, htab_hash_eq,
				       NULL, xcalloc, free);
    }

  for (n = 0, k = 0; k < NDS32_CORE_COUNT; k++, n += j)
    {
      const keyword_t **kwd;

      if ((j = nds32_keyword_count_table[k]) == 0)
	continue;

      /* Add keywords.  */
      kwd = nds32_keyword_table[k];
      for (i = 0; i < j; i++)
	{
	  htab_t htab;
	  const keyword_t *kw;

	  kw = kwd[i];
	  htab = hw_ktabs[n + i];
	  while (kw->name != NULL)
	    {
	      hashval_t hash;
	      const keyword_t **slot;

	      hash = htab_hash_string (kw->name);
	      slot = (const keyword_t **)
		htab_find_slot_with_hash (htab, kw->name, hash, INSERT);

	      assert (slot != NULL && *slot == NULL);
	      *slot = kw++;
	    }
	}
    }
}

/* Build the syntax for a given opcode OPC.  It parses the string
   pointed by INSTRUCTION and store the result on SYNTAX, so
   when we assemble an instruction, we don't have to parse the syntax
   again.  */

static void
build_opcode_syntax (struct nds32_opcode *opc)
{
  char odstr[MAX_LEX_LEN];
  const char *str;
  const char *end;
  lex_t *plex;
  int len;
  hashval_t hash;
  field_t *fd;
  int opt = 0;

  /* Check whether it has been initialized.  */
  if (opc->syntax)
    return;

  opc->syntax = xmalloc (MAX_LEX_NUM * sizeof (lex_t));
  memset (opc->syntax, 0, MAX_LEX_NUM * sizeof (lex_t));

  str = opc->instruction;
  plex = opc->syntax;
  while (*str)
    {
      int fidx, i, k;

      switch (*str)
	{
	case '%':
	  *plex = SYN_INPUT;
	  break;
	case '=':
	  *plex = SYN_OUTPUT;
	  break;
	case '&':
	  *plex = SYN_INPUT | SYN_OUTPUT;
	  break;
	case '{':
	  *plex++ = SYN_LOPT;
	  opt++;
	  str++;
	  continue;
	case '}':
	  *plex++ = SYN_ROPT;
	  str++;
	  continue;
	default:
	  *plex++ = *str++;
	  continue;
	}
      str++;

      /* Extract operand.  */
      end = str;
      while (ISALNUM (*end) || *end == '_')
	end++;
      len = end - str;
      memcpy (odstr, str, len);
      odstr[len] = '\0';

      hash = htab_hash_string (odstr);
      fd = (field_t *) htab_find_with_hash (field_htab, odstr, hash);

      if (fd == NULL)
	{
	  /* xgettext: c-format */
	  opcodes_error_handler (_("internal error: unknown operand, %s"), str);
	  abort ();
	}

      /* We are not sure how these tables are organized.   */
      /* Thus, the minimal index should be the right one.  */
      for (fidx = 256, k = 0, i = 0; i < NDS32_CORE_COUNT; i++)
	{
	  int tmp;

	  tmp = fd - nds32_field_table[i];
	  if (tmp >= 0 && tmp < fidx)
	    {
	      fidx = tmp;
	      k = i;
	    }
	}
      assert (fidx >= 0 && fidx < (int) ARRAY_SIZE (nds32_operand_fields));
      *plex |= LEX_SET_FIELD (k, fidx);

      str += len;
      plex++;
    }

  *plex = 0;
  opc->variant = opt;
  return;
}

static void
build_opcode_hash_table (void)
{
  unsigned k;

  opcode_htab = htab_create_alloc (512, htab_hash_hash, htab_hash_eq,
				   NULL, xcalloc, free);

  for (k = 0; k < NDS32_CORE_COUNT; k++)
    {
      opcode_t *opc;

      opc = nds32_opcode_table[k];
      if (opc == NULL)
	continue;

      /* Add op-codes.  */
      while ((opc->opcode != NULL) && (opc->instruction != NULL))
	{
	  hashval_t hash;
	  opcode_t **slot;

	  hash = htab_hash_string (opc->opcode);
	  slot = (opcode_t **)
	    htab_find_slot_with_hash (opcode_htab, opc->opcode, hash,
				      INSERT);

#define NDS32_PREINIT_SYNTAX
#if defined (NDS32_PREINIT_SYNTAX)
	  /* Initial SYNTAX when build opcode table, so bug in syntax
	     can be found when initialized rather than used.  */
	  build_opcode_syntax (opc);
#endif

	  if (*slot == NULL)
	    {
	      /* This is the new one.  */
	      *slot = opc;
	    }
	  else
	    {
	      opcode_t *ptr;

	      /* Already exists.  Append to the list.  */
	      ptr = *slot;
	      while (ptr->next)
		ptr = ptr->next;
	      ptr->next = opc;
	      opc->next = NULL;
	    }
	  opc++;
	}
    }
}

/* Initialize the assembler.  It must be called before assembling.  */

void
nds32_asm_init (nds32_asm_desc_t *pdesc, int flags)
{
  pdesc->flags = flags;
  pdesc->mach = flags & NASM_OPEN_ARCH_MASK;

  /* Setup main core.  */
  nds32_keyword_table[NDS32_MAIN_CORE] = &nds32_keywords[0];
  nds32_keyword_count_table[NDS32_MAIN_CORE] = _HW_LAST;
  nds32_opcode_table[NDS32_MAIN_CORE] = &nds32_opcodes[0];
  nds32_field_table[NDS32_MAIN_CORE] = &nds32_operand_fields[0];

  /* Build operand hash table.  */
  build_operand_hash_table ();

  /* Build keyword hash tables.  */
  build_keyword_hash_table ();

  /* Build op-code hash table.  */
  build_opcode_hash_table ();
}

/* Parse the input and store operand keyword string in ODSTR.
   This function is only used for parsing keywords,
   HW_INT/HW_UINT are parsed parse_operand callback handler.  */

static char *
parse_to_delimiter (char *str, char odstr[MAX_KEYWORD_LEN])
{
  char *outp = odstr;

  while (ISALNUM (*str) || *str == '.' || *str == '_')
    *outp++ = TOLOWER (*str++);

  *outp = '\0';
  return str;
}

/* Parse the operand of lmw/smw/lmwa/smwa.  */

static int
parse_re (struct nds32_asm_desc *pdesc ATTRIBUTE_UNUSED,
	   struct nds32_asm_insn *pinsn, char **pstr, int64_t *value)
{
  char *end = *pstr;
  char odstr[MAX_KEYWORD_LEN];
  keyword_t *k;
  hashval_t hash;

  if (*end == '$')
    end++;
  end = parse_to_delimiter (end, odstr);

  hash = htab_hash_string (odstr);
  k = htab_find_with_hash (hw_ktabs[HW_GPR], odstr, hash);

  if (k == NULL)
    return NASM_ERR_OPERAND;

  if (__GF (pinsn->insn, 20, 5) > (unsigned int) k->value)
    return NASM_ERR_OPERAND;

  /* Register not allowed in reduced register.  */
  if ((pdesc->flags & NASM_OPEN_REDUCED_REG)
      && (k->attr & ATTR (RDREG)) == 0)
    return NASM_ERR_REG_REDUCED;

  *value = k->value;
  *pstr = end;
  return NASM_R_CONST;
}

/* Parse the operand of push25/pop25.  */

static int
parse_re2 (struct nds32_asm_desc *pdesc ATTRIBUTE_UNUSED,
	   struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	   char **pstr, int64_t *value)
{
  char *end = *pstr;
  char odstr[MAX_KEYWORD_LEN];
  keyword_t *k;
  hashval_t hash;

  if (*end == '$')
    end++;
  end = parse_to_delimiter (end, odstr);

  hash = htab_hash_string (odstr);
  k = htab_find_with_hash (hw_ktabs[HW_GPR], odstr, hash);

  if (k == NULL)
    return NASM_ERR_OPERAND;

  /* Register not allowed in reduced register.  */
  if ((pdesc->flags & NASM_OPEN_REDUCED_REG)
      && (k->attr & ATTR (RDREG)) == 0)
    return NASM_ERR_REG_REDUCED;

  if (k->value == 6)
    *value = 0;
  else if (k->value == 8)
    *value = 1;
  else if (k->value == 10)
    *value = 2;
  else if (k->value == 14)
    *value = 3;
  else
    return NASM_ERR_OPERAND;

  *pstr = end;
  return NASM_R_CONST;
}

/* Parse the operand of lwi45.fe.  */

static int
parse_fe5 (struct nds32_asm_desc *pdesc, struct nds32_asm_insn *pinsn,
	   char **pstr, int64_t *value)
{
  int r;

  r = pdesc->parse_operand (pdesc, pinsn, pstr, value);
  if (r != NASM_R_CONST)
    return NASM_ERR_OPERAND;

  /* 128 == 32 << 2.  Leave the shift to parse_opreand,
     so it can check whether it is a multiple of 4.  */
  *value = 128 + *value;
  return r;
}

/* Parse the operand of movpi45.  */

static int
parse_pi5 (struct nds32_asm_desc *pdesc, struct nds32_asm_insn *pinsn,
	   char **pstr, int64_t *value)
{
  int r;

  r = pdesc->parse_operand (pdesc, pinsn, pstr, value);
  if (r != NASM_R_CONST)
    return NASM_ERR_OPERAND;

  *value -= 16;
  return r;
}

static int aext_a30b20 = 0;
static int aext_rte = 0;
static int aext_im5_ip = 0;
static int aext_im6_ip = 0;
/* Parse the operand of audio ext.  */
static int
parse_aext_reg (struct nds32_asm_desc *pdesc, char **pstr,
		int *value, int hw_res)
{
  char *end = *pstr;
  char odstr[MAX_KEYWORD_LEN];
  keyword_t *k;
  hashval_t hash;

  if (*end == '$')
    end++;
  end = parse_to_delimiter (end, odstr);

  hash = htab_hash_string (odstr);
  k = htab_find_with_hash (hw_ktabs[hw_res], odstr, hash);

  if (k == NULL)
    return NASM_ERR_OPERAND;

  if (hw_res == HW_GPR
      && (pdesc->flags & NASM_OPEN_REDUCED_REG)
      && (k->attr & ATTR (RDREG)) == 0)
    return NASM_ERR_REG_REDUCED;

  *value = k->value;
  *pstr = end;
  return NASM_R_CONST;
}

static int
parse_a30b20 (struct nds32_asm_desc *pdesc,
	      struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	      char **pstr, int64_t *value)
{
  int rt_value, ret;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_GPR);
  if (ret == NASM_ERR_REG_REDUCED)
    return NASM_ERR_REG_REDUCED;
  if ((ret == NASM_ERR_OPERAND) || (rt_value > 15))
    return NASM_ERR_OPERAND;

  *value = rt_value;
  aext_a30b20 = rt_value;
  return NASM_R_CONST;
}

static int
parse_rt21 (struct nds32_asm_desc *pdesc,
	    struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	    char **pstr, int64_t *value)
{
  int rt_value, ret, tmp_value, tmp1, tmp2;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_GPR);
  if (ret == NASM_ERR_REG_REDUCED)
    return NASM_ERR_REG_REDUCED;
  if ((ret == NASM_ERR_OPERAND) || (rt_value > 15))
    return NASM_ERR_OPERAND;

  tmp1 = (aext_a30b20 & 0x08);
  tmp2 = (rt_value & 0x08);
  if (tmp1 != tmp2)
    return NASM_ERR_OPERAND;

  /* Rt=CONCAT(c, t21, t0), t21:bit11-10, t0:bit5.  */
  tmp_value = (rt_value & 0x06) << 4;
  tmp_value |= (rt_value & 0x01);
  *value = tmp_value;
  return NASM_R_CONST;
}

static int
parse_rte_start (struct nds32_asm_desc *pdesc,
		 struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
		 char **pstr, int64_t *value)
{
  int rt_value, ret, tmp1, tmp2;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_GPR);
  if (ret == NASM_ERR_REG_REDUCED)
    return NASM_ERR_REG_REDUCED;
  if ((ret == NASM_ERR_OPERAND) || (rt_value > 15)
      || (rt_value & 0x01))
    return NASM_ERR_OPERAND;

  tmp1 = (aext_a30b20 & 0x08);
  tmp2 = (rt_value & 0x08);
  if (tmp1 != tmp2)
    return NASM_ERR_OPERAND;

  aext_rte = rt_value;
  /* Rt=CONCAT(c, t21, 0), t21:bit11-10.  */
  rt_value = (rt_value & 0x06) << 4;
  *value = rt_value;
  return NASM_R_CONST;
}

static int
parse_rte_end (struct nds32_asm_desc *pdesc,
	       struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	       char **pstr, int64_t *value)
{
  int rt_value, ret, tmp1, tmp2;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_GPR);
  if (ret == NASM_ERR_REG_REDUCED)
    return NASM_ERR_REG_REDUCED;
  if ((ret == NASM_ERR_OPERAND) || (rt_value > 15)
      || ((rt_value & 0x01) == 0)
      || (rt_value != (aext_rte + 1)))
    return NASM_ERR_OPERAND;

  tmp1 = (aext_a30b20 & 0x08);
  tmp2 = (rt_value & 0x08);
  if (tmp1 != tmp2)
    return NASM_ERR_OPERAND;

  /* Rt=CONCAT(c, t21, 0), t21:bit11-10.  */
  rt_value = (rt_value & 0x06) << 4;
  *value = rt_value;
  return NASM_R_CONST;
}

static int
parse_rte69_start (struct nds32_asm_desc *pdesc,
		   struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
		   char **pstr, int64_t *value)
{
  int rt_value, ret;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_GPR);
  if (ret == NASM_ERR_REG_REDUCED)
    return NASM_ERR_REG_REDUCED;
  if ((ret == NASM_ERR_OPERAND)
      || (rt_value & 0x01))
    return NASM_ERR_OPERAND;

  aext_rte = rt_value;
  rt_value = (rt_value >> 1);
  *value = rt_value;
  return NASM_R_CONST;
}

static int
parse_rte69_end (struct nds32_asm_desc *pdesc,
		 struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
		 char **pstr, int64_t *value)
{
  int rt_value, ret;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_GPR);
  if (ret == NASM_ERR_REG_REDUCED)
    return NASM_ERR_REG_REDUCED;
  if ((ret == NASM_ERR_OPERAND)
      || ((rt_value & 0x01) == 0)
      || (rt_value != (aext_rte + 1)))
    return NASM_ERR_OPERAND;

  aext_rte = rt_value;
  rt_value = (rt_value >> 1);
  *value = rt_value;
  return NASM_R_CONST;
}

static int
parse_im5_ip (struct nds32_asm_desc *pdesc,
	      struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	      char **pstr, int64_t *value)
{
  int rt_value, ret, new_value;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_AEXT_IM_I);
  if (ret == NASM_ERR_OPERAND)
    return NASM_ERR_OPERAND;

  /* p = bit[4].bit[1:0], r = bit[4].bit[3:2].  */
  new_value = (rt_value & 0x04) << 2;
  new_value |= (rt_value & 0x03);
  *value = new_value;
  aext_im5_ip = new_value;
  return NASM_R_CONST;
}

static int
parse_im5_mr (struct nds32_asm_desc *pdesc,
	      struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	      char **pstr, int64_t *value)
{
  int rt_value, ret, new_value, tmp1, tmp2;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_AEXT_IM_M);
  if (ret == NASM_ERR_OPERAND)
    return NASM_ERR_OPERAND;

  /* p = bit[4].bit[1:0], r = bit[4].bit[3:2].  */
  new_value = (rt_value & 0x07) << 2;
  tmp1 = (aext_im5_ip & 0x10);
  tmp2 = (new_value & 0x10);
  if (tmp1 != tmp2)
    return NASM_ERR_OPERAND;

  *value = new_value;
  return NASM_R_CONST;
}

static int
parse_im6_ip (struct nds32_asm_desc *pdesc,
	      struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	      char **pstr, int64_t *value)
{
  int rt_value, ret;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_AEXT_IM_I);
  if ((ret == NASM_ERR_OPERAND) || (rt_value > 3))
    return NASM_ERR_OPERAND;

  /* p = 0.bit[1:0].  */
  aext_im6_ip = rt_value;
  *value = aext_im6_ip;
  return NASM_R_CONST;
}

static int
parse_im6_iq (struct nds32_asm_desc *pdesc,
	      struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	      char **pstr, int64_t *value)
{
  int rt_value, ret;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_AEXT_IM_I);
  if ((ret == NASM_ERR_OPERAND) || (rt_value < 4))
    return NASM_ERR_OPERAND;

  /* q = 1.bit[1:0].  */
  if ((rt_value & 0x03) != aext_im6_ip)
    return NASM_ERR_OPERAND;

  *value = aext_im6_ip;
  return NASM_R_CONST;
}

static int
parse_im6_mr (struct nds32_asm_desc *pdesc,
	      struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	      char **pstr, int64_t *value)
{
  int rt_value, ret;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_AEXT_IM_M);
  if ((ret == NASM_ERR_OPERAND) || (rt_value > 3))
    return NASM_ERR_OPERAND;

  /* r = 0.bit[3:2].  */
  *value = (rt_value & 0x03);
  return NASM_R_CONST;
}

static int
parse_im6_ms (struct nds32_asm_desc *pdesc,
	      struct nds32_asm_insn *pinsn ATTRIBUTE_UNUSED,
	      char **pstr, int64_t *value)
{
  int rt_value, ret;

  ret = parse_aext_reg (pdesc, pstr, &rt_value, HW_AEXT_IM_M);
  if ((ret == NASM_ERR_OPERAND) || (rt_value < 4))
    return NASM_ERR_OPERAND;

  /* s = 1.bit[5:4].  */
  *value = (rt_value & 0x03);
  return NASM_R_CONST;
}

/* Generic operand parse base on the information provided by the field.  */

static int
parse_operand (nds32_asm_desc_t *pdesc, nds32_asm_insn_t *pinsn,
	       char **str, int syn)
{
  char odstr[MAX_KEYWORD_LEN];
  char *end;
  hashval_t hash;
  const field_t *fld = &LEX_GET_FIELD (((syn >> 8) & 0xff) - 1, syn);
  keyword_t *k;
  int64_t value = 0;	/* 0x100000000; Big enough to overflow.  */
  int r;
  uint64_t modifier = 0;

  end = *str;

  if (fld->parse)
    {
      r = fld->parse (pdesc, pinsn, &end, &value);
      if (r == NASM_ERR_OPERAND || r == NASM_ERR_REG_REDUCED)
	{
	  pdesc->result = r;
	  return 0;
	}
      goto done;
    }

  /* Check valid keyword group.  */
  if (fld->hw_res < HW_INT)
    {
      int n = 0, i;

      /* Calculate index of keyword hash table.  */
      for (i = 0; i < (fld->hw_res >> 8); i++)
	n += nds32_keyword_count_table[i];

      /* Parse the operand in assembly code.  */
      if (*end == '$')
	end++;
      end = parse_to_delimiter (end, odstr);

      hash = htab_hash_string (odstr);
      k = htab_find_with_hash (hw_ktabs[n + (fld->hw_res & 0xff)], odstr,
			       hash);

      if (k == NULL)
	{
	  pdesc->result = NASM_ERR_OPERAND;
	  return 0;
	}

      if (fld->hw_res == HW_GPR && (pdesc->flags & NASM_OPEN_REDUCED_REG)
	  && (k->attr & ATTR (RDREG)) == 0)
	{
	  /* Register not allowed in reduced register.  */
	  pdesc->result = NASM_ERR_REG_REDUCED;
	  return 0;
	}

      if (fld->hw_res == HW_GPR)
	{
	  if (syn & SYN_INPUT)
	    pinsn->defuse |= USE_REG (k->value);
	  if (syn & SYN_OUTPUT)
	    pinsn->defuse |= DEF_REG (k->value);
	}

      value = k->value;
      if (fld->hw_res == HW_GPR && (fld->bitsize + fld->shift) == 4)
	value = nds32_r54map[value];
    }
  else if (fld->hw_res == HW_INT || fld->hw_res == HW_UINT)
    {
      if (*end == '#')
	end++;

      /* Handle modifiers.  Do we need to make a table for modifiers?
	 Do we need to check unknown modifier?  */
      if (strncasecmp (end, "hi20(", 5) == 0)
	{
	  modifier |= NASM_ATTR_HI20;
	  end += 5;
	}
      else if (strncasecmp (end, "lo12(", 5) == 0)
	{
	  modifier |= NASM_ATTR_LO12;
	  end += 5;
	}
      else if (strncasecmp (end, "lo20(", 5) == 0)
	{
	  /* e.g., movi.  */
	  modifier |= NASM_ATTR_LO20;
	  end += 5;
	}

      r = pdesc->parse_operand (pdesc, pinsn, &end, &value);
      if (modifier)
	{
	  /* Consume the ')' of modifier.  */
	  end++;
	  pinsn->attr |= modifier;
	}

      switch (r)
	{
	case NASM_R_ILLEGAL:
	  pdesc->result = NASM_ERR_OPERAND;
	  return 0;
	case NASM_R_SYMBOL:
	  /* This field needs special fix-up.  */
	  pinsn->field = fld;
	  break;
	case NASM_R_CONST:
	  if (modifier & NASM_ATTR_HI20)
	    value = (value >> 12) & 0xfffff;
	  else if (modifier & NASM_ATTR_LO12)
	    value = value & 0xfff;
	  else if (modifier & NASM_ATTR_LO20)
	    value = value & 0xfffff;
	  break;
	default:
	  /* xgettext: c-format */
	  opcodes_error_handler (_("internal error: don't know how to handle "
				   "parsing results"));
	  abort ();
	}
    }
  else
    {
      /* xgettext: c-format */
      opcodes_error_handler (_("internal error: unknown hardware resource"));
      abort ();
    }

 done:
  /* Don't silently discarding bits.  */
  if (value & __MASK (fld->shift))
    {
      pdesc->result = NASM_ERR_OUT_OF_RANGE;
      return 0;
    }

  /* Check the range of signed or unsigned result.  */
  if (fld->hw_res != HW_INT && ((int32_t) value >> (fld->bitsize + fld->shift)))
    {
      pdesc->result = NASM_ERR_OUT_OF_RANGE;
      return 0;
    }
  else if (fld->hw_res == HW_INT)
    {
      /* Sign-ext the value.  */
      if (((value >> 32) == 0) && (value & 0x80000000))
	value |= (int64_t) -1U << 31;


      /* Shift the value to positive domain.  */
      if ((value + (1 << (fld->bitsize + fld->shift - 1)))
	  >> (fld->bitsize + fld->shift))
	{
	  pdesc->result = NASM_ERR_OUT_OF_RANGE;
	  return 0;
	}
    }

  pinsn->insn |=
    (((value >> fld->shift) & __MASK (fld->bitsize)) << fld->bitpos);
  *str = end;
  return 1;
}

/* Try to parse an instruction string based on opcode syntax.  */

static int
parse_insn (nds32_asm_desc_t *pdesc, nds32_asm_insn_t *pinsn,
	    char *str, struct nds32_opcode *opc)
{
  int variant = 0;
  char *p = NULL;

  /* A syntax may has optional operands, so we have to try each possible
     combination to see if the input is accepted.  In order to do so,
     bit-N represent whether optional-operand-N is used in this combination.
     That is, if bit-N is set, optional-operand-N is not used.

     For example, there are 2 optional operands in this syntax,

     "a{,b}{,c}"

     we can try it 4 times (i.e., 1 << 2)

     0 (b00): "a,b,c"
     1 (b01): "a,c"
     2 (b10): "a,b"
     3 (b11): "a"
   */

  /* The outer do-while loop is used to try each possible optional
     operand combination, and VARIANT is the bit mask.  The inner loop
     iterates each lexeme in the syntax.  */

  do
    {
      /* OPT is the number of optional operands we've seen.  */
      int opt = 0;
      lex_t *plex;

      /* PLEX is the syntax iterator and P is the iterator for input
	 string.  */
      plex = opc->syntax;
      p = str;
      /* Initial the base value.  */
      pinsn->insn = opc->value;

      while (*plex)
	{
	  if (IS_LEX_CHAR (*plex))
	    {
	      /* If it's a plain char, just compare it.  */
	      if (LEX_CHAR (*plex) != TOLOWER (*p))
		{
		  if (LEX_CHAR (*plex) == '+' && TOLOWER (*p) == '-')
		    {
		      /* We don't define minus format for some signed
			 immediate case, so ignoring '+' here to parse
			 negative value eazily.  Besides, the minus format
			 can not support for instruction with relocation.
			 Ex: lwi $r0, [$r0 + imm]  */
		      plex++;
		      continue;
		    }
		  pdesc->result = NASM_ERR_SYNTAX;
		  goto reject;
		}
	      p++;
	    }
	  else if (*plex & SYN_LOPT)
	    {
	      /* If it's '{' and it's not used in this iteration,
		 just skip the whole optional operand.  */
	      if ((1 << (opt++)) & variant)
		{
		  while ((*plex & SYN_ROPT) == 0)
		    plex++;
		}
	    }
	  else if (*plex & SYN_ROPT)
	    {
	      /* ignore.  */
	    }
	  else
	    {
	      /* If it's a operand, parse the input operand from input.  */
	      if (!parse_operand (pdesc, pinsn, &p, *plex))
		goto reject;
	    }
	  plex++;
	}

      /* Check whether this syntax is accepted.  */
      if (*plex == 0 && (*p == '\0' || *p == '!' || *p == '#'))
	return 1;

    reject:
      /* If not accepted, try another combination.  */
      variant++;
    }
  while (variant < (1 << opc->variant));

  return 0;
}

void
nds32_assemble (nds32_asm_desc_t *pdesc, nds32_asm_insn_t *pinsn,
		char *str)
{
  struct nds32_opcode *opc;
  char *s;
  char *mnemoic;
  char *dot;
  hashval_t hash;

  /* Duplicate the string, so we can modify it for convenience.  */
  s = strdup (str);
  mnemoic = s;
  str = s;

  /* Find opcode mnemoic.  */
  while (*s != ' ' && *s != '\t' && *s != '\0')
    s++;
  if (*s != '\0')
    *s++ = '\0';
  dot = strchr (mnemoic, '.');

 retry_dot:
  /* Lookup the opcode syntax.  */
  hash = htab_hash_string (mnemoic);
  opc = (struct nds32_opcode *)
    htab_find_with_hash (opcode_htab, mnemoic, hash);

  /* If we cannot find a match syntax, try it again without `.'.
     For example, try "lmw.adm" first and then try "lmw" again.  */
  if (opc == NULL && dot != NULL)
    {
      *dot = '\0';
      s[-1] = ' ';
      s = dot + 1;
      dot = NULL;
      goto retry_dot;
    }
  else if (opc == NULL)
    {
      pdesc->result = NASM_ERR_UNKNOWN_OP;
      goto out;
    }

  /* There may be multiple syntaxes for a given opcode.
     Try each one until a match is found.  */
  for (; opc; opc = opc->next)
    {
      /* Build opcode syntax, if it's not been initialized yet.  */
      if (opc->syntax == NULL)
	build_opcode_syntax (opc);

      /* Reset status before assemble.  */
      pinsn->defuse = opc->defuse;
      pinsn->insn = 0;
      pinsn->field = NULL;
      /* Use opcode attributes to initial instruction attributes.  */
      pinsn->attr = opc->attr;
      if (parse_insn (pdesc, pinsn, s, opc))
	break;
    }

  pinsn->opcode = opc;
  if (opc == NULL)
    {
      if (pdesc->result == NASM_OK)
	pdesc->result = NASM_ERR_SYNTAX;
      goto out;
    }

  /* A matched opcode is found.  Write the result to instruction buffer.  */
  pdesc->result = NASM_OK;

 out:
  free (str);
}
