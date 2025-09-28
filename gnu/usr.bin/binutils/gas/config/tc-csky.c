/* tc-csky.c -- Assembler for C-SKY
   Copyright (C) 1989-2023 Free Software Foundation, Inc.
   Created by Lifang Xia (lifang_xia@c-sky.com)
   Contributed by C-SKY Microsystems and Mentor Graphics.

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

#include "as.h"
#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include "safe-ctype.h"
#include "subsegs.h"
#include "obstack.h"
#include "libiberty.h"

#ifdef OBJ_ELF
#include "elf/csky.h"
#include "dw2gencfi.h"
#endif
#include "tc-csky.h"
#include "dwarf2dbg.h"

#define BUILD_AS          1

#define OPCODE_MAX_LEN    20
#define HAS_SUB_OPERAND   0xfffffffful

/* This value is just for lrw to distinguish "[]" label.  */
#define NEED_OUTPUT_LITERAL           1

#define IS_EXTERNAL_SYM(sym, sec)     (S_GET_SEGMENT (sym) != sec)
#define IS_SUPPORT_OPCODE16(opcode)   (opcode->isa_flag16 | isa_flag)
#define IS_SUPPORT_OPCODE32(opcode)   (opcode->isa_flag32 | isa_flag)


#define KB                * 1024
#define MB                KB * 1024
#define GB                MB * 1024

/* Define DSP version flags. For different CPU, the version of DSP
   instructions may be different.  */
#define CSKY_DSP_FLAG_V1          (1 << 0) /* Normal DSP instructions.  */
#define CSKY_DSP_FLAG_V2          (1 << 1) /* CK803S enhanced DSP.  */

/* Literal pool related macros.  */
/* 1024 - 1 entry - 2 byte rounding.  */
#define v1_SPANPANIC      (998)
#define v1_SPANCLOSE      (900)
#define v1_SPANEXIT       (600)
#define v2_SPANPANIC      (1024 - 4)

/* 1024 is flrw offset.
   24 is the biggest size for single instruction.
   for lrw16 (3+7, 512 bytes).  */
#define v2_SPANCLOSE      (512 - 24)

/* For lrw16, 112 average size for a function.  */
#define v2_SPANEXIT       (512 - 112)

/* For lrw16 (3+7, 512 bytes).  */
#define v2_SPANCLOSE_ELRW (1016 - 24)

/* For lrw16, 112 average size for a function.  */
#define v2_SPANEXIT_ELRW  (1016 - 112)
#define MAX_POOL_SIZE     (1024 / 4)
#define POOL_END_LABEL    ".LE"
#define POOL_START_LABEL  ".LS"

/* Used in v1_relax_table.  */
/* These are the two types of relaxable instruction.  */
#define COND_JUMP         1
#define UNCD_JUMP         2
#define COND_JUMP_PIC     3
#define UNCD_JUMP_PIC     4

#define UNDEF_DISP        0
#define DISP12            1
#define DISP32            2
#define UNDEF_WORD_DISP   3

#define C12_LEN           2
/* Allow for align: bt/jmpi/.long + align.  */
#define C32_LEN           10
/* Allow for align: bt/subi/stw/bsr/lrw/add/ld/addi/jmp/.long + align.  */
#define C32_LEN_PIC       24
#define U12_LEN           2
/* Allow for align: jmpi/.long + align.  */
#define U32_LEN           8
/* Allow for align: subi/stw/bsr/lrw/add/ld/addi/jmp/.long + align.  */
#define U32_LEN_PIC       22

#define C(what,length)    (((what) << 2) + (length))
#define UNCD_JUMP_S       (do_pic ? UNCD_JUMP_PIC : UNCD_JUMP)
#define COND_JUMP_S       (do_pic ? COND_JUMP_PIC : COND_JUMP)
#define U32_LEN_S         (do_pic ? U32_LEN_PIC  : U32_LEN)
#define C32_LEN_S         (do_pic ? C32_LEN_PIC  : C32_LEN)

/* Used in v2_relax_table.  */
#define COND_DISP10_LEN   2   /* bt/bf_16.  */
#define COND_DISP16_LEN   4   /* bt/bf_32.  */

#define SCOND_DISP10_LEN   2   /* bt/bf_16, for CK801 only.  */
#define SCOND_DISP16_LEN   6   /* !(bt/bf_16) + br_32.  */

#define UNCD_DISP10_LEN   2   /* br_16.  */
#define UNCD_DISP16_LEN   4   /* br_32.  */
#define UNCD_DISP26_LEN   4   /* br32_old.  */

#define JCOND_DISP10_LEN  2   /* bt/bf_16.  */
#define JCOND_DISP16_LEN  4   /* bt/bf_32.  */
#define JCOND_DISP32_LEN  12  /* !(bt/bf_16)/jmpi 32/.align 2/literal 4.  */
#define JCOND_DISP26_LEN  8   /* bt/bf_32/br_32 old.  */

#define JUNCD_DISP26_LEN  4   /* bt/bf_32 old.  */
#define JUNCD_DISP10_LEN  2   /* br_16.  */
#define JUNCD_DISP16_LEN  4   /* bt/bf_32.  */
#define JUNCD_DISP32_LEN  10  /* jmpi_32/.align 2/literal 4/  CHANGED!.  */
#define JCOMP_DISP26_LEN  8   /* bne_32/br_32 old.  */

#define JCOMP_DISP16_LEN  4   /* bne_32 old.  */
#define JCOMPZ_DISP16_LEN 4   /* bhlz_32.  */
#define JCOMPZ_DISP32_LEN 14  /* bsz_32/jmpi 32/.align 2/literal 4.  */
#define JCOMPZ_DISP26_LEN 8   /* bsz_32/br_32  old.  */
#define JCOMP_DISP32_LEN  14  /* be_32/jmpi_32/.align 2/literal old.  */

#define BSR_DISP10_LEN    2   /* bsr_16.  */
#define BSR_DISP26_LEN    4   /* bsr_32.  */
#define LRW_DISP7_LEN     2   /* lrw16.  */
#define LRW_DISP16_LEN    4   /* lrw32.  */

/* Declare worker functions.  */
bool v1_work_lrw (void);
bool v1_work_jbsr (void);
bool v1_work_fpu_fo (void);
bool v1_work_fpu_fo_fc (void);
bool v1_work_fpu_write (void);
bool v1_work_fpu_read (void);
bool v1_work_fpu_writed (void);
bool v1_work_fpu_readd (void);
bool v2_work_istack (void);
bool v2_work_btsti (void);
bool v2_work_addi (void);
bool v2_work_subi (void);
bool v2_work_add_sub (void);
bool v2_work_rotlc (void);
bool v2_work_bgeni (void);
bool v2_work_not (void);
bool v2_work_jbtf (void);
bool v2_work_jbr (void);
bool v2_work_lrw (void);
bool v2_work_lrsrsw (void);
bool v2_work_jbsr (void);
bool v2_work_jsri (void);
bool v2_work_movih (void);
bool v2_work_ori (void);
bool float_work_fmovi (void);
bool dsp_work_bloop (void);
bool float_work_fpuv3_fmovi (void);
bool float_work_fpuv3_fstore (void);
bool v2_work_addc (void);

/* csky-opc.h must be included after workers are declared.  */
#include "opcodes/csky-opc.h"
#include "opcode/csky.h"

enum
{
  RELAX_NONE = 0,
  RELAX_OVERFLOW,

  COND_DISP10 = 20,    /* bt/bf_16.  */
  COND_DISP16,    /* bt/bf_32.  */

  SCOND_DISP10,	   /* br_16 */
  SCOND_DISP16,	   /* !(bt/bf_32) + br_32.  */

  UNCD_DISP10,    /* br_16.  */
  UNCD_DISP16,    /* br_32.  */

  JCOND_DISP10,   /* bt/bf_16.  */
  JCOND_DISP16,   /* bt/bf_32.  */
  JCOND_DISP32,   /* !(bt/bf_32)/jmpi + literal.  */

  JUNCD_DISP10,   /* br_16.  */
  JUNCD_DISP16,   /* br_32.  */
  JUNCD_DISP32,   /* jmpi + literal.  */

  JCOMPZ_DISP16,  /* bez/bnez/bhz/blsz/blz/bhsz.  */
  JCOMPZ_DISP32,  /* !(jbez/jbnez/jblsz/jblz/jbhsz) + jmpi + literal.  */

  BSR_DISP26,     /* bsr_32.  */

  LRW_DISP7,      /* lrw16.  */
  LRW2_DISP8,     /* lrw16, -mno-bsr16,8 bit offset.  */
  LRW_DISP16,     /* lrw32.  */
};

unsigned int mach_flag = 0;
unsigned int arch_flag = 0;
unsigned int other_flag = 0;
uint64_t isa_flag = 0;
unsigned int dsp_flag = 0;

typedef struct stack_size_entry
{
  struct stack_size_entry *next;
  symbolS *function;
  unsigned int stack_size;
} stack_size_entry;

struct csky_arch_info
{
  const char *name;
  unsigned int arch_flag;
  unsigned int bfd_mach_flag;
};

typedef enum
{
  INSN_OPCODE,
  INSN_OPCODE16F,
  INSN_OPCODE32F,
} inst_flag;

/* Macro information.  */
struct csky_macro_info
{
  const char *name;
  /* How many operands : if operands == 5, all of 1,2,3,4 are ok.  */
  long oprnd_num;
  uint64_t isa_flag;
  /* Do the work.  */
  void (*handle_func)(void);
};

struct csky_insn_info
{
  /* Name of the opcode.  */
  char *name;
  /* Output instruction.  */
  unsigned int inst;
  /* Pointer for frag.  */
  char *output;
  /* End of instruction.  */
  char *opcode_end;
  /* CPU infomations.  */
  const struct csky_cpu_info *cpu;
  /* Flag for INSN_OPCODE16F, INSN_OPCODE32F, INSN_OPCODE, INSN_MACRO.  */
  inst_flag flag_force;
  /* Operand number.  */
  int number;
  struct csky_opcode *opcode;
  struct csky_macro_info *macro;
  /* Insn size for check_literal.  */
  unsigned int isize;
  unsigned int last_isize;
  /* Max size of insn for relax frag_var.  */
  unsigned int max;
  /* Indicates which element is in csky_opcode_info op[] array.  */
  int opcode_idx;
  /* The value of each operand in instruction when layout.  */
  int idx;
  int val[MAX_OPRND_NUM];
  struct relax_info
    {
      int max;
      int var;
      int subtype;
    } relax;
  /* The following are used for constant expressions.  */
  expressionS e1;
  expressionS e2;
};

/* Literal pool data structures.  */
struct literal
{
  unsigned short  refcnt;
  unsigned int    offset;
  unsigned char   ispcrel;
  unsigned char   unused;
  bfd_reloc_code_real_type r_type;
  expressionS     e;
  struct tls_addend tls_addend;
  unsigned char   isdouble;
  uint64_t dbnum;
  LITTLENUM_TYPE bignum[SIZE_OF_LARGE_NUMBER + 6];
};

static void csky_idly (void);
static void csky_rolc (void);
static void csky_sxtrb (void);
static void csky_movtf (void);
static void csky_addc64 (void);
static void csky_subc64 (void);
static void csky_or64 (void);
static void csky_xor64 (void);
static void csky_neg (void);
static void csky_rsubi (void);
static void csky_arith (void);
static void csky_decne (void);
static void csky_lrw (void);

static enum bfd_reloc_code_real insn_reloc;

/* Assembler operand parse errors use these identifiers.  */

enum error_number
{
  /* The following are errors.  */
  ERROR_CREG_ILLEGAL = 0,
  ERROR_REG_OVER_RANGE,
  ERROR_FREG_OVER_RANGE,
  ERROR_VREG_OVER_RANGE,
  ERROR_GREG_ILLEGAL,
  ERROR_802J_REG_OVER_RANGE,
  ERROR_REG_FORMAT,
  ERROR_REG_LIST,
  ERROR_IMM_ILLEGAL,
  ERROR_IMM_OVERFLOW,             /* 5  */
  ERROR_IMM_POWER,
  ERROR_JMPIX_OVER_RANGE,
  ERROR_EXP_CREG,
  ERROR_EXP_GREG,
  ERROR_EXP_CONSTANT,
  ERROR_EXP_EVEN_FREG,
  ERROR_RELOC_ILLEGAL,
  ERROR_MISSING_OPERAND,          /* 10  */
  ERROR_MISSING_COMMA,
  ERROR_MISSING_LBRACKET,
  ERROR_MISSING_RBRACKET,
  ERROR_MISSING_LSQUARE_BRACKETS,
  ERROR_MISSING_RSQUARE_BRACKETS, /* 15  */
  ERROR_MISSING_LANGLE_BRACKETS,
  ERROR_MISSING_RANGLE_BRACKETS,
  ERROR_OFFSET_UNALIGNED,
  ERROR_BAD_END,
  ERROR_UNDEFINE,
  ERROR_CPREG_ILLEGAL,           /* 20  */
  ERROR_OPCODE_PSRBIT,
  ERROR_OPERANDS_ILLEGAL,
  ERROR_OPERANDS_NUMBER,
  ERROR_OPCODE_ILLEGAL,

  /* The following are warnings.  */
  WARNING_OPTIONS,
  WARNING_IDLY,

  /* Error and warning end.  */
  ERROR_NONE,
};

/* Global error state.  ARG1 and ARG2 are opaque data interpreted
   as appropriate for the error code.  */

struct csky_error_state
{
  enum error_number err_num;
  int opnum;
  int arg_int;
  const void *arg1;
  const void *arg2;
} error_state;

/* This macro is used to set error number and arg1 in the global state.  */

#define SET_ERROR_STRING(err, msg)                      \
  do {							\
    if (error_state.err_num > err)			\
      {							\
	error_state.err_num = err;			\
	error_state.arg1 = (void *)msg;			\
      }							\
  } while (0)

#define SET_ERROR_INTEGER(err, integer)			\
  do {							\
    if (error_state.err_num > err)			\
      {							\
	error_state.err_num = err;			\
	error_state.arg_int = integer;			\
      }							\
  } while (0)

/* Map error identifiers onto a format string, which will use
   arg1 and arg2 from the global error state.  */
struct csky_error_format_map
{
  enum error_number num;
  const char *fmt;
};

static const struct csky_error_format_map err_formats[] =
{
  {ERROR_CREG_ILLEGAL, "Operand %d error: control register is illegal."},
  {ERROR_REG_OVER_RANGE, "Operand %d error: r%d register is over range."},
  {ERROR_FREG_OVER_RANGE, "Operand %d error: vr%d register is over range."},
  {ERROR_VREG_OVER_RANGE, "Operand %d error: vr%d register is out of range."},
  {ERROR_GREG_ILLEGAL, "Operand %d error: general register is illegal."},
  {ERROR_802J_REG_OVER_RANGE, "Operand %d register %s out of range (802j only has registers:0-15,23,24,25,30)"},
  {ERROR_REG_FORMAT, "Operand %d error: %s."},
  {ERROR_REG_LIST, "Register list format is illegal."},
  {ERROR_IMM_ILLEGAL, "Operand %d is not an immediate."},
  {ERROR_IMM_OVERFLOW, "Operand %d immediate is overflow."},
  {ERROR_IMM_POWER, "immediate %d is not a power of two"},
  {ERROR_JMPIX_OVER_RANGE, "The second operand must be 16/24/32/40"},
  {ERROR_EXP_CREG, "Operand %d error: control register is expected."},
  {ERROR_EXP_GREG, "Operand %d error: general register is expected."},
  {ERROR_EXP_CONSTANT, "Operand %d error: constant is expected."},
  {ERROR_EXP_EVEN_FREG, "Operand %d error: even float register is expected."},
  {ERROR_RELOC_ILLEGAL, "@%s reloc is not supported"},
  {ERROR_MISSING_OPERAND, "Operand %d is missing."},
  {ERROR_MISSING_COMMA, "Missing ','"},
  {ERROR_MISSING_LBRACKET, "Missing '('"},
  {ERROR_MISSING_RBRACKET, "Missing ')'"},
  {ERROR_MISSING_LSQUARE_BRACKETS, "Missing '['"},
  {ERROR_MISSING_RSQUARE_BRACKETS, "Missing ']'"},
  {ERROR_MISSING_LANGLE_BRACKETS, "Missing '<'"},
  {ERROR_MISSING_RANGLE_BRACKETS, "Missing '>'"},
  {ERROR_OFFSET_UNALIGNED, "Operand %d is unaligned. It must be %d aligned!"},
  {ERROR_BAD_END, "Operands mismatch, it has a bad end: %s"},
  {ERROR_UNDEFINE, NULL},
  {ERROR_CPREG_ILLEGAL, "Operand %d illegal, expect a cpreg(cpr0-cpr63)."},
  {ERROR_OPCODE_PSRBIT, "The operands must be 'ie'/'ee'/'fe'."},
  {ERROR_OPERANDS_ILLEGAL, "Operands mismatch: %s."},
  {ERROR_OPERANDS_NUMBER, "Operands number mismatch, %d operands expected."},
  {ERROR_OPCODE_ILLEGAL, "The instruction is not recognized."},
  {WARNING_OPTIONS, "Option %s is not support in %s."},
  {WARNING_IDLY, "idly %d is encoded to: idly 4 "},
  {ERROR_NONE, "There is no error."},
};

static int do_pic = 0;            /* for jbr/jbf/jbt relax jmpi reloc.  */
static int do_pff = -1;           /* for insert two br ahead of literals.  */
static int do_force2bsr = -1;     /* for jbsr->bsr.  */
static int do_jsri2bsr = 1;       /* for jsri->bsr.  */
static int do_nolrw = 0;          /* lrw to movih & ori, only for V2.  */
static int do_long_jump = -1;      /* control if jbf,jbt,jbr relax to jmpi.  */
static int do_extend_lrw = -1;    /* delete bsr16 in both two options,
				     add btesti16, lrw offset +1 in -melrw.  */
static int do_func_dump = 0;      /* dump literals after every function.  */
static int do_br_dump = 1;        /* work for -mabr/-mno-abr, control the literals dump.  */
static int do_intr_stack = -1;    /* control interrupt stack module, 801&802&803
				     default on, 807&810, default off.  */
static int float_abi = 0;

#ifdef INCLUDE_BRANCH_STUB
static int do_use_branchstub = -1;
#else
static int do_use_branchstub = 0;
#endif

/* These are only used for options parsing.  Values are bitmasks and are
   OR'ed into the processor flag bits in md_begin.  */
static int do_opt_mmp = 0;
static int do_opt_mcp = 0;
static int do_opt_mcache = 0;
static int do_opt_msecurity = 0;
static int do_opt_mhard_float = 0;
static int do_opt_mtrust = 0;
static int do_opt_mdsp = 0;
static int do_opt_medsp = 0;
static int do_opt_mvdsp = 0;

const relax_typeS *md_relax_table = NULL;
struct literal *literal_insn_offset;
static struct literal litpool[MAX_POOL_SIZE];
static unsigned poolsize = 0;
static unsigned poolnumber = 0;
static unsigned long poolspan = 0;
static unsigned int SPANPANIC;
static unsigned int SPANCLOSE;
static unsigned int SPANEXIT;

static stack_size_entry *all_stack_size_data = NULL;
static stack_size_entry **last_stack_size_data = &all_stack_size_data;

/* Control by ".no_literal_dump N"
 * 1 : don't dump literal pool between insn1 and insnN+1
 * 0 : do nothing.  */
static int do_noliteraldump = 0;

/* Label for current pool.  */
static symbolS * poolsym;
static char poolname[8];

static bool mov_r1_before;
static bool mov_r1_after;

const relax_typeS csky_relax_table [] =
{
  /* C-SKY V1 relax table.  */
  {0, 0, 0, 0},                                   /* RELAX_NONE      */
  {0, 0, 0, 0},                                   /* RELAX_OVERFLOW  */
  {0, 0, 0, 0},
  {0, 0, 0, 0},

  /* COND_JUMP */
  {    0,     0, 0,       0 },                     /* UNDEF_DISP */
  { 2048, -2046, C12_LEN, C (COND_JUMP, DISP32) }, /* DISP12 */
  {    0,     0, C32_LEN, 0 },                     /* DISP32 */
  {    0,     0, C32_LEN, 0 },                     /* UNDEF_WORD_DISP */

  /* UNCD_JUMP */
  {    0,     0, 0,       0 },                     /* UNDEF_DISP */
  { 2048, -2046, U12_LEN, C (UNCD_JUMP, DISP32) }, /* DISP12 */
  {    0,     0, U32_LEN, 0 },                     /* DISP32 */
  {    0,     0, U32_LEN, 0 },                     /* UNDEF_WORD_DISP */

  /* COND_JUMP_PIC */
  {    0,     0, 0,           0 },                     /* UNDEF_DISP */
  { 2048, -2046, C12_LEN, C (COND_JUMP_PIC, DISP32) }, /* DISP12 */
  {    0,     0, C32_LEN_PIC, 0 },                     /* DISP32 */
  {    0,     0, C32_LEN_PIC, 0 },                     /* UNDEF_WORD_DISP */

  /* UNCD_JUMP_PIC */
  {    0,     0, 0,           0 },                     /* UNDEF_DISP */
  { 2048, -2046, U12_LEN, C (UNCD_JUMP_PIC, DISP32) }, /* DISP12 */
  {    0,     0, U32_LEN_PIC, 0 },                     /* DISP32 */
  {    0,     0, U32_LEN_PIC, 0 },                     /* UNDEF_WORD_DISP */

  /* C-SKY V2 relax table.  */
  /* forward  backward      length          more     */
  {  1 KB - 2,  -1 KB, COND_DISP10_LEN,   COND_DISP16    }, /* COND_DISP10 */
  { 64 KB - 2, -64 KB, COND_DISP16_LEN,   RELAX_OVERFLOW }, /* COND_DISP16 */

  {  1 KB - 2,  -1 KB, SCOND_DISP10_LEN,  SCOND_DISP16   }, /* SCOND_DISP10 */
  { 64 KB - 2, -64 KB, SCOND_DISP16_LEN,  RELAX_OVERFLOW }, /* SCOND_DISP16 */

  {  1 KB - 2,  -1 KB, UNCD_DISP10_LEN,   UNCD_DISP16    }, /* UNCD_DISP10 */
  { 64 KB - 2, -64 KB, UNCD_DISP16_LEN,   RELAX_OVERFLOW }, /* UNCD_DISP16 */

  {  1 KB - 2,  -1 KB, JCOND_DISP10_LEN,  JCOND_DISP16   }, /* JCOND_DISP10 */
  { 64 KB - 2, -64 KB, JCOND_DISP16_LEN,  JCOND_DISP32   }, /* JCOND_DISP16 */
  {         0,      0, JCOND_DISP32_LEN,  RELAX_NONE     }, /* JCOND_DISP32 */

  {  1 KB - 2,  -1 KB, JUNCD_DISP10_LEN,  JUNCD_DISP16   }, /* JUNCD_DISP10 */
  { 64 KB - 2, -64 KB, JUNCD_DISP16_LEN,  JUNCD_DISP32   }, /* JUNCD_DISP16 */
  {         0,      0, JUNCD_DISP32_LEN,  RELAX_NONE     }, /* JUNCD_DISP32 */

  { 64 KB - 2, -64 KB, JCOMPZ_DISP16_LEN, JCOMPZ_DISP32  }, /* JCOMPZ_DISP16 */
  {         0,      0, JCOMPZ_DISP32_LEN, RELAX_NONE     }, /* JCOMPZ_DISP32 */

  { 64 MB - 2, -64 MB, BSR_DISP26_LEN,    RELAX_OVERFLOW }, /* BSR_DISP26 */

  {       508,      0, LRW_DISP7_LEN,     LRW_DISP16     }, /* LRW_DISP7 */
  {      1016,      0, LRW_DISP7_LEN,     LRW_DISP16     }, /* LRW2_DISP8 */
  {     64 KB,      0, LRW_DISP16_LEN,    RELAX_OVERFLOW }, /* LRW_DISP16 */

};

static void csky_write_insn (char *ptr, valueT use, int nbytes);
void md_number_to_chars (char * buf, valueT val, int n);
long md_pcrel_from_section (fixS * fixP, segT seg);

/* C-SKY architecture table.  */
const struct csky_arch_info csky_archs[] =
{
  {"ck510",  CSKY_ARCH_510,  bfd_mach_ck510},
  {"ck610",  CSKY_ARCH_610,  bfd_mach_ck610},
  {"ck801",  CSKY_ARCH_801,  bfd_mach_ck801},
  {"ck802",  CSKY_ARCH_802,  bfd_mach_ck802},
  {"ck803",  CSKY_ARCH_803,  bfd_mach_ck803},
  {"ck807",  CSKY_ARCH_807,  bfd_mach_ck807},
  {"ck810",  CSKY_ARCH_810,  bfd_mach_ck810},
  {"ck860",  CSKY_ARCH_860,  bfd_mach_ck860},
  {NULL, 0, 0}
};

#define CSKY_ARCH_807_BASE    CSKY_ARCH_807 | CSKY_ARCH_DSP
#define CSKY_ARCH_810_BASE    CSKY_ARCH_810 | CSKY_ARCH_DSP

struct csky_cpu_feature
{
  const char unique;
  unsigned int arch_flag;
  uint64_t isa_flag;
};

struct csky_cpu_version
{
  int r;
  int p;
  uint64_t isa_flag;
};

#define CSKY_FEATURE_MAX  10
#define CSKY_CPU_REVERISON_MAX 10

struct csky_cpu_info
{
  const char *name;
  unsigned int arch_flag;
  uint64_t isa_flag;
  struct csky_cpu_feature features[CSKY_FEATURE_MAX];
  struct csky_cpu_version ver[CSKY_CPU_REVERISON_MAX];
};

#define FEATURE_DSP_EXT(isa)                \
   {'e', CSKY_ARCH_DSP, isa}
#define FEATURE_DSP(isa)                    \
   {'d', CSKY_ARCH_DSP, isa}
#define FEATURE_MMU()                       \
   {'m', 0, 0}
#define FEATURE_VDSP(isa)                   \
   {'v', CSKY_ARCH_DSP, isa}
#define FEATURE_FLOAT(isa)                  \
   {'f', CSKY_ARCH_FLOAT, isa}
#define FEATURE_TRUST(isa)                  \
   {'t', 0, isa}
#define FEATURE_JAVA(isa)                   \
   {'j', CSKY_ARCH_JAVA, isa}
#define FEATURE_SHIELD(isa)                 \
   {'h', 0, isa}


#define CSKY_FEATURES_DEF_NULL()            \
   {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_e(isa_e)          \
   {FEATURE_DSP_EXT(isa_e),                 \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_t(isa_t)          \
   {FEATURE_TRUST(isa_t),                   \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_f(isa_f)          \
   {FEATURE_FLOAT(isa_f),                   \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_v(isa_v)          \
   {FEATURE_VDSP(isa_v),                    \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_ef(isa_e, isa_f)  \
   {FEATURE_DSP_EXT(isa_e),                 \
    FEATURE_FLOAT(isa_f),                   \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_jt(isa_j, isa_t)  \
   {FEATURE_JAVA(isa_j),                    \
    FEATURE_TRUST(isa_t),                   \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_efht(isa_e, isa_f, isa_h, isa_t) \
   {FEATURE_DSP_EXT(isa_e),                 \
    FEATURE_FLOAT(isa_f),                   \
    FEATURE_SHIELD(isa_h),                  \
    FEATURE_TRUST(isa_t),                   \
    {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_efv(isa_e, isa_f, isa_v) \
   {FEATURE_DSP_EXT(isa_e),                 \
    FEATURE_FLOAT(isa_f),                   \
    FEATURE_VDSP(isa_v),                    \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_eft(isa_e, isa_f, isa_t) \
   {FEATURE_DSP_EXT(isa_e),                 \
    FEATURE_FLOAT(isa_f),                   \
    FEATURE_TRUST(isa_t),                   \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_d(isa_d) \
   {FEATURE_DSP(isa_d),             \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_df(isa_d, isa_f)  \
   {FEATURE_DSP(isa_d),             \
    FEATURE_FLOAT(isa_f),               \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_ft(isa_f, isa_t)  \
   {FEATURE_FLOAT(isa_f),                   \
    FEATURE_TRUST(isa_t),                   \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_tv(isa_t, isa_v)  \
   {FEATURE_TRUST(isa_t),                   \
    FEATURE_VDSP(isa_v),                    \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_fv(isa_f, isa_v)  \
   {FEATURE_FLOAT(isa_f),                   \
    FEATURE_VDSP(isa_v),                    \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}


#define CSKY_FEATURES_DEF_dft(isa_d, isa_f, isa_t) \
   {FEATURE_DSP(isa_d),                     \
    FEATURE_FLOAT(isa_f),                   \
    FEATURE_TRUST(isa_t),                   \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_dfv(isa_d, isa_f, isa_v) \
   {FEATURE_DSP(isa_d),                     \
    FEATURE_FLOAT(isa_f),                   \
    FEATURE_VDSP(isa_v),                    \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_ftv(isa_f, isa_t, isa_v) \
   {FEATURE_FLOAT(isa_f),                   \
    FEATURE_TRUST(isa_t),                   \
    FEATURE_VDSP(isa_v),                    \
    {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_FEATURES_DEF_eftv(isa_e, isa_f, isa_t, isa_v) \
   {FEATURE_DSP_EXT(isa_e),                 \
    FEATURE_FLOAT(isa_f),                   \
    FEATURE_TRUST(isa_t),                   \
    FEATURE_VDSP(isa_v),                    \
    {0}, {0}, {0}, {0}, {0}, {0}}


#define CSKY_CPU_REVERISON_r0p0(isa)        \
    {0, 0, 0}
#define CSKY_CPU_REVERISON_r1p0(isa)        \
    {1, 0, isa}
#define CSKY_CPU_REVERISON_r2p0(isa)        \
    {2, 0, isa}
#define CSKY_CPU_REVERISON_r3p0(isa)        \
    {3, 0, isa}

#define CSKY_CPU_REVERISON_RESERVED()  \
{{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}

#define CSKY_CPU_REVERISON_R3(isa1, isa2, isa3) \
  {CSKY_CPU_REVERISON_r1p0(isa1),           \
   CSKY_CPU_REVERISON_r2p0(isa2),           \
   CSKY_CPU_REVERISON_r3p0(isa3),           \
   {0}, {0}, {0}, {0}, {0}, {0}, {0}}

/* CSKY cpus table.  */
const struct csky_cpu_info csky_cpus[] =
{
#define CSKYV1_ISA_DSP   (CSKY_ISA_DSP | CSKY_ISA_MAC_DSP)
#define CSKY_ISA_510     (CSKYV1_ISA_E1)
#define CSKY_ISA_610     (CSKYV1_ISA_E1 | CSKY_ISA_CP)
  {"ck510",
    CSKY_ARCH_510,
    CSKY_ISA_510,
    CSKY_FEATURES_DEF_e(CSKYV1_ISA_DSP),
    CSKY_CPU_REVERISON_RESERVED()},
  {"ck520",
    CSKY_ARCH_510 | CSKY_ARCH_MAC,
    CSKY_ISA_510 | CSKY_ISA_MAC | CSKY_ISA_MAC_DSP,
    CSKY_FEATURES_DEF_NULL(),
    CSKY_CPU_REVERISON_RESERVED()},
  {"ck610", CSKY_ARCH_610, CSKY_ISA_610,
    CSKY_FEATURES_DEF_ef(CSKYV1_ISA_DSP, CSKY_ISA_FLOAT_E1),
    CSKY_CPU_REVERISON_RESERVED()},
  {"ck620",
    CSKY_ARCH_610 | CSKY_ARCH_MAC,
    CSKY_ISA_610 | CSKY_ISA_MAC | CSKY_ISA_MAC_DSP,
    CSKY_FEATURES_DEF_NULL(),
    CSKY_CPU_REVERISON_RESERVED()},

#define CSKY_ISA_801    (CSKYV2_ISA_E1 | CSKY_ISA_TRUST)
#define CSKYV2_ISA_DSP  (CSKY_ISA_DSP | CSKY_ISA_DSP_1E2 | CSKY_ISA_DSPE60)
  {"ck801",
    CSKY_ARCH_801,
    CSKY_ISA_801,
    CSKY_FEATURES_DEF_t(0),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKY_ISA_802    (CSKY_ISA_801 | CSKYV2_ISA_1E2 | CSKY_ISA_NVIC)
  {"ck802",
    CSKY_ARCH_802,
    CSKY_ISA_802,
    CSKY_FEATURES_DEF_jt(CSKY_ISA_JAVA, 0),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKY_ISA_803    (CSKY_ISA_802 | CSKYV2_ISA_2E3 | CSKY_ISA_MP)
#define CSKY_ISA_803R1  (CSKYV2_ISA_3E3R1)
#define CSKY_ISA_803R2  (CSKYV2_ISA_3E3R1 | CSKYV2_ISA_3E3R2)
#define CSKY_ISA_803R3  (CSKYV2_ISA_3E3R1 | CSKYV2_ISA_3E3R2 | CSKYV2_ISA_3E3R3)
#define CSKY_ISA_FLOAT_803 (CSKY_ISA_FLOAT_E1 | CSKY_ISA_FLOAT_1E3)
#define CSKY_ISA_EDSP   (CSKYV2_ISA_3E3R1 | CSKYV2_ISA_3E3R3 | CSKY_ISA_DSP_ENHANCE)
   {"ck803s",
    CSKY_ARCH_803,
    CSKY_ISA_803 | CSKY_ISA_803R1,
    CSKY_FEATURES_DEF_eft(CSKYV2_ISA_DSP, CSKY_ISA_FLOAT_803, 0),
    CSKY_CPU_REVERISON_RESERVED()},
   {"ck803",
    CSKY_ARCH_803,
    CSKY_ISA_803,
    CSKY_FEATURES_DEF_efht(CSKYV2_ISA_DSP, CSKY_ISA_FLOAT_803, 0, 0),
    CSKY_CPU_REVERISON_R3(CSKY_ISA_803R1, CSKY_ISA_803R2, CSKY_ISA_803R3)},
#define CSKY_ISA_804   (CSKY_ISA_803 | CSKY_ISA_803R3)
   {"ck804",
    CSKY_ARCH_804,
    CSKY_ISA_804,
    CSKY_FEATURES_DEF_efht(CSKY_ISA_EDSP, CSKY_ISA_FLOAT_803, 0, 0),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKY_ISA_805   (CSKY_ISA_804 | CSKY_ISA_VDSP_2)
#define CSKY_ARCH_805V  (CSKY_ARCH_805 | CSKY_ARCH_DSP)
#define CSKY_ISA_FLOAT_805 CSKY_ISA_FLOAT_803
   {"ck805",
    CSKY_ARCH_805,
    CSKY_ISA_805,
    CSKY_FEATURES_DEF_eft(CSKY_ISA_EDSP, CSKY_ISA_FLOAT_805, 0),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKY_ISA_807       (CSKY_ISA_803 | CSKYV2_ISA_3E7 | CSKY_ISA_MP_1E2 | CSKY_ISA_CACHE | CSKYV2_ISA_DSP)
#define CSKY_ISA_FLOAT_807 (CSKY_ISA_FLOAT_803 | CSKY_ISA_FLOAT_3E4 | CSKY_ISA_FLOAT_1E2)
   {"ck807",
    CSKY_ARCH_807,
    CSKY_ISA_807,
    CSKY_FEATURES_DEF_ef(CSKYV2_ISA_DSP, CSKY_ISA_FLOAT_807),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKY_ISA_810       (CSKY_ISA_807 | CSKYV2_ISA_7E10)
#define CSKY_ISA_FLOAT_810 (CSKY_ISA_FLOAT_E1 | CSKY_ISA_FLOAT_1E2)
   {"ck810v",
    CSKY_ARCH_810 | CSKY_ARCH_DSP,
    CSKY_ISA_810 | CSKY_ISA_VDSP,
    CSKY_FEATURES_DEF_NULL (),
    CSKY_CPU_REVERISON_RESERVED()},
   {"ck810",
    CSKY_ARCH_810,
    CSKY_ISA_810,
    CSKY_FEATURES_DEF_eftv(0, CSKY_ISA_FLOAT_810, 0, CSKY_ISA_VDSP),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKY_ISA_860       ((CSKY_ISA_810 & ~(CSKYV2_ISA_DSP)) | CSKYV2_ISA_10E60 | CSKY_ISA_803R3 | CSKY_ISA_DSPE60)
#define CSKY_ISA_860F      (CSKY_ISA_860 | CSKY_ISA_FLOAT_7E60)
#define CSKY_ISA_VDSP_860  (CSKY_ISA_VDSP_2)
   {"ck860v",
    CSKY_ARCH_860 | CSKY_ARCH_DSP,
    CSKY_ISA_860 | CSKY_ISA_VDSP_860,
    CSKY_FEATURES_DEF_f(CSKY_ISA_FLOAT_7E60),
    CSKY_CPU_REVERISON_RESERVED()},
   {"ck860",
    CSKY_ARCH_860,
    CSKY_ISA_860,
    CSKY_FEATURES_DEF_fv(CSKY_ISA_FLOAT_7E60, CSKY_ISA_VDSP_860),
    CSKY_CPU_REVERISON_RESERVED()},

   /* It is a special cpu, support all instructions.  */
#define CSKY_ISA_800       (CSKY_ISA_860 | CSKY_ISA_810 | CSKY_ISA_807 | CSKY_ISA_803)
   {"ck800",
    CSKY_ARCH_800,
    CSKY_ISA_800,
    CSKY_FEATURES_DEF_NULL(),
    CSKY_CPU_REVERISON_RESERVED()},


#define CSKY_ISA_E801      (CSKY_ISA_801)
#define CSKY_ISA_E802      (CSKY_ISA_E801 | CSKYV2_ISA_1E2 | CSKY_ISA_NVIC)
#define CSKY_ISA_E803      (CSKY_ISA_E802 | CSKYV2_ISA_2E3 | CSKY_ISA_MP | CSKYV2_ISA_3E3R1 | CSKYV2_ISA_3E3R2 | CSKYV2_ISA_3E3R3)
#define CSKY_ISA_E804      (CSKY_ISA_E803)
#define CSKY_ISA_FLOAT_V1  (CSKY_ISA_FLOAT_E1 | CSKY_ISA_FLOAT_1E3)
  {"e801",
    CSKY_ARCH_801,
    CSKY_ISA_E801,
    CSKY_FEATURES_DEF_NULL(),
    CSKY_CPU_REVERISON_RESERVED()},
  {"e802",
    CSKY_ARCH_802,
    CSKY_ISA_E802,
    CSKY_FEATURES_DEF_t(0),
    CSKY_CPU_REVERISON_RESERVED()},
  {"e803",
    CSKY_ARCH_803,
    CSKY_ISA_E803,
    CSKY_FEATURES_DEF_t(0),
    CSKY_CPU_REVERISON_RESERVED()},
  {"e804",
    CSKY_ARCH_804,
    CSKY_ISA_E804,
    CSKY_FEATURES_DEF_dft(CSKY_ISA_EDSP, CSKY_ISA_FLOAT_V1, 0),
    CSKY_CPU_REVERISON_RESERVED()},

#define CSKY_ISA_S802       (CSKY_ISA_E801 | CSKYV2_ISA_1E2 | CSKY_ISA_NVIC | CSKY_ISA_TRUST)
#define CSKY_ISA_S803       (CSKY_ISA_S802 | CSKYV2_ISA_2E3 | CSKY_ISA_MP | CSKYV2_ISA_3E3R1 | CSKYV2_ISA_3E3R2 | CSKYV2_ISA_3E3R3)
  {"s802",
    CSKY_ARCH_802,
    CSKY_ISA_S802,
    CSKY_FEATURES_DEF_t(0),
    CSKY_CPU_REVERISON_RESERVED()},
  {"s803",
    CSKY_ARCH_803,
    CSKY_ISA_S803,
    CSKY_FEATURES_DEF_t(0),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKY_ISA_I805       (CSKY_ISA_S803)
  {"i805",
    CSKY_ARCH_805 | CSKY_ARCH_DSP,
    CSKY_ISA_I805 | CSKY_ISA_VDSP_2,
    CSKY_FEATURES_DEF_ft(CSKY_ISA_FLOAT_V1, 0),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKYV2_ISA_DSP      (CSKY_ISA_DSP | CSKY_ISA_DSP_1E2 | CSKY_ISA_DSPE60)
#define CSKY_ISA_C807       (CSKY_ISA_E802 | CSKYV2_ISA_2E3 | CSKY_ISA_MP | CSKYV2_ISA_3E7 | CSKY_ISA_MP_1E2 | CSKY_ISA_CACHE | CSKYV2_ISA_DSP)
#define CSKY_ISA_FLOAT_C807 (CSKY_ISA_FLOAT_V1 | CSKY_ISA_FLOAT_3E4 | CSKY_ISA_FLOAT_1E2)
#define CSKY_ISA_FLOAT_C810 (CSKY_ISA_FLOAT_E1 | CSKY_ISA_FLOAT_1E2)
#define CSKY_ARCH_C810      (CSKY_ARCH_810 | CSKY_ARCH_FLOAT)
#define CSKY_ISA_C810       (CSKY_ISA_C807 | CSKYV2_ISA_7E10 | CSKY_ISA_FLOAT_C810)
#define CSKY_ARCH_C860      (CSKY_ARCH_860 | CSKY_ARCH_FLOAT)
#define CSKY_ISA_C860       (CSKY_ISA_860 | CSKY_ISA_FLOAT_7E60)
  {"c807",
    CSKY_ARCH_807,
    CSKY_ISA_C807,
    CSKY_FEATURES_DEF_fv(CSKY_ISA_FLOAT_C807, CSKY_ISA_VDSP),
    CSKY_CPU_REVERISON_RESERVED()},
  {"c810",
    CSKY_ARCH_C810,
    CSKY_ISA_C810,
    CSKY_FEATURES_DEF_tv(0, CSKY_ISA_VDSP),
    CSKY_CPU_REVERISON_RESERVED()},
  {"c860",
    CSKY_ARCH_C860,
    CSKY_ISA_C860,
    CSKY_FEATURES_DEF_v(CSKY_ISA_VDSP_2),
    CSKY_CPU_REVERISON_RESERVED()},
#define CSKY_ISA_R807       (CSKY_ISA_E802 | CSKYV2_ISA_2E3 | CSKY_ISA_MP | CSKYV2_ISA_3E7 | CSKY_ISA_MP_1E2 | CSKY_ISA_CACHE | CSKYV2_ISA_DSP)
#define CSKY_ISA_FLOAT_R807 (CSKY_ISA_FLOAT_V1 | CSKY_ISA_FLOAT_3E4 | CSKY_ISA_FLOAT_1E2)
  {"r807",
    CSKY_ARCH_807,
    CSKY_ISA_R807,
    CSKY_FEATURES_DEF_f(CSKY_ISA_FLOAT_R807),
    CSKY_CPU_REVERISON_RESERVED()},

/* Start of private CPUs.  */
/* End of private CPUs.  */

  {NULL},
};

int md_short_jump_size = 2;
int md_long_jump_size = 4;

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = "#";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.  */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
/* Also note that comments like this one will always work.  */
const char line_comment_chars[] = "#";

const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant
   from exp in floating point numbers.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.
   As in 0f12.456
   or   0d1.2345e12  */

const char FLT_CHARS[] = "rRsSfFdDxXeEpP";

const char *md_shortopts = "";

struct option md_longopts[] = {
#define OPTION_MARCH (OPTION_MD_BASE + 0)
  {"march", required_argument, NULL, OPTION_MARCH},
#define OPTION_MCPU (OPTION_MD_BASE + 1)
  {"mcpu", required_argument, NULL, OPTION_MCPU},
#define OPTION_FLOAT_ABI (OPTION_MD_BASE + 2)
  {"mfloat-abi", required_argument, NULL, OPTION_FLOAT_ABI},

  /* Remaining options just set boolean flags.  */
  {"EL", no_argument, &target_big_endian, 0},
  {"mlittle-endian", no_argument, &target_big_endian, 0},
  {"EB", no_argument, &target_big_endian, 1},
  {"mbig-endian", no_argument, &target_big_endian, 1},
  {"fpic", no_argument, &do_pic, 1},
  {"pic", no_argument, &do_pic, 1},
  {"mljump", no_argument, &do_long_jump, 1},
  {"mno-ljump", no_argument, &do_long_jump, 0},
  {"force2bsr", no_argument, &do_force2bsr, 1},
  {"mforce2bsr", no_argument, &do_force2bsr, 1},
  {"no-force2bsr", no_argument, &do_force2bsr, 0},
  {"mno-force2bsr", no_argument, &do_force2bsr, 0},
  {"jsri2bsr", no_argument, &do_jsri2bsr, 1},
  {"mjsri2bsr", no_argument, &do_jsri2bsr, 1},
  {"no-jsri2bsr", no_argument, &do_jsri2bsr, 0},
  {"mno-jsri2bsr", no_argument, &do_jsri2bsr, 0},
  {"mnolrw", no_argument, &do_nolrw, 1},
  {"mno-lrw", no_argument, &do_nolrw, 1},
  {"melrw", no_argument, &do_extend_lrw, 1},
  {"mno-elrw", no_argument, &do_extend_lrw, 0},
  {"mlaf", no_argument, &do_func_dump, 1},
  {"mliterals-after-func", no_argument, &do_func_dump, 1},
  {"mno-laf", no_argument, &do_func_dump, 0},
  {"mno-literals-after-func", no_argument, &do_func_dump, 0},
  {"mlabr", no_argument, &do_br_dump, 1},
  {"mliterals-after-br", no_argument, &do_br_dump, 1},
  {"mno-labr", no_argument, &do_br_dump, 0},
  {"mnoliterals-after-br", no_argument, &do_br_dump, 0},
  {"mistack", no_argument, &do_intr_stack, 1},
  {"mno-istack", no_argument, &do_intr_stack, 0},
#ifdef INCLUDE_BRANCH_STUB
  {"mbranch-stub", no_argument, &do_use_branchstub, 1},
  {"mno-branch-stub", no_argument, &do_use_branchstub, 0},
#endif
  {"mhard-float", no_argument, &do_opt_mhard_float, CSKY_ARCH_FLOAT},
  {"mmp", no_argument, &do_opt_mmp, CSKY_ARCH_MP},
  {"mcp", no_argument, &do_opt_mcp, CSKY_ARCH_CP},
  {"mcache", no_argument, &do_opt_mcache, CSKY_ARCH_CACHE},
  {"msecurity", no_argument, &do_opt_msecurity, CSKY_ARCH_MAC},
  {"mtrust", no_argument, &do_opt_mtrust, CSKY_ISA_TRUST},
  {"mdsp", no_argument, &do_opt_mdsp, CSKY_DSP_FLAG_V1},
  {"medsp", no_argument, &do_opt_medsp, CSKY_DSP_FLAG_V2},
  {"mvdsp", no_argument, &do_opt_mvdsp, CSKY_ISA_VDSP},
};

size_t md_longopts_size = sizeof (md_longopts);

static struct csky_insn_info csky_insn;

static htab_t csky_opcodes_hash;
static htab_t csky_macros_hash;

static struct csky_macro_info v1_macros_table[] =
{
  {"idly",   1, CSKYV1_ISA_E1, csky_idly},
  {"rolc",   2, CSKYV1_ISA_E1, csky_rolc},
  {"rotlc",  2, CSKYV1_ISA_E1, csky_rolc},
  {"sxtrb0", 2, CSKYV1_ISA_E1, csky_sxtrb},
  {"sxtrb1", 2, CSKYV1_ISA_E1, csky_sxtrb},
  {"sxtrb2", 2, CSKYV1_ISA_E1, csky_sxtrb},
  {"movtf",  3, CSKYV1_ISA_E1, csky_movtf},
  {"addc64", 3, CSKYV1_ISA_E1, csky_addc64},
  {"subc64", 3, CSKYV1_ISA_E1, csky_subc64},
  {"or64",   3, CSKYV1_ISA_E1, csky_or64},
  {"xor64",  3, CSKYV1_ISA_E1, csky_xor64},
  {NULL,0,0,0}
};

static struct csky_macro_info v2_macros_table[] =
{
  {"neg",   1, CSKYV2_ISA_E1,  csky_neg},
  {"rsubi", 2, CSKYV2_ISA_1E2, csky_rsubi},
  {"incf",  1, CSKYV2_ISA_1E2, csky_arith},
  {"inct",  1, CSKYV2_ISA_1E2, csky_arith},
  {"decf",  1, CSKYV2_ISA_2E3, csky_arith},
  {"decgt", 1, CSKYV2_ISA_2E3, csky_arith},
  {"declt", 1, CSKYV2_ISA_2E3, csky_arith},
  {"decne", 1, CSKYV2_ISA_1E2, csky_decne},
  {"dect",  1, CSKYV2_ISA_1E2, csky_arith},
  {"lslc",  1, CSKYV2_ISA_1E2, csky_arith},
  {"lsrc",  1, CSKYV2_ISA_1E2, csky_arith},
  {"xsr",   1, CSKYV2_ISA_1E2, csky_arith},
  {NULL,0,0,0}
};

/* For option -mnolrw, replace lrw by movih & ori.  */
static struct csky_macro_info v2_lrw_macro_opcode =
  {"lrw", 2, CSKYV2_ISA_1E2, csky_lrw};

/* This function is used to show errors or warnings.  */

static void
csky_show_error (enum error_number err, int idx, void *arg1, void *arg2)
{
  if (err == ERROR_NONE)
    return;

  switch (err)
    {
    case ERROR_REG_LIST:
    case ERROR_OPCODE_PSRBIT:
    case ERROR_OPCODE_ILLEGAL:
    case ERROR_JMPIX_OVER_RANGE:
    case ERROR_MISSING_COMMA:
    case ERROR_MISSING_LBRACKET:
    case ERROR_MISSING_RBRACKET:
    case ERROR_MISSING_LSQUARE_BRACKETS:
    case ERROR_MISSING_RSQUARE_BRACKETS:
    case ERROR_MISSING_LANGLE_BRACKETS:
    case ERROR_MISSING_RANGLE_BRACKETS:
      /* Add NULL to fix warnings.  */
      as_bad (_(err_formats[err].fmt), NULL);
      break;
    case ERROR_CREG_ILLEGAL:
    case ERROR_GREG_ILLEGAL:
    case ERROR_IMM_ILLEGAL:
    case ERROR_IMM_OVERFLOW:
    case ERROR_EXP_CREG:
    case ERROR_EXP_GREG:
    case ERROR_EXP_CONSTANT:
    case ERROR_EXP_EVEN_FREG:
    case ERROR_MISSING_OPERAND:
    case ERROR_CPREG_ILLEGAL:
      as_bad (_(err_formats[err].fmt), idx);
      break;
    case ERROR_OPERANDS_NUMBER:
    case ERROR_IMM_POWER:
      as_bad (_(err_formats[err].fmt), error_state.arg_int);
      break;

    case ERROR_OFFSET_UNALIGNED:
      as_bad (_(err_formats[err].fmt), idx, error_state.arg_int);
      break;
    case ERROR_RELOC_ILLEGAL:
    case ERROR_BAD_END:
    case ERROR_OPERANDS_ILLEGAL:
      as_bad (_(err_formats[err].fmt), (char *)arg1);
      break;
    case ERROR_REG_OVER_RANGE:
    case ERROR_FREG_OVER_RANGE:
    case ERROR_VREG_OVER_RANGE:
      as_bad (_(err_formats[err].fmt), idx, error_state.arg_int);
      break;
    case ERROR_802J_REG_OVER_RANGE:
    case ERROR_REG_FORMAT:
      as_bad (_(err_formats[err].fmt), idx, (char *)arg1);
      break;
    case ERROR_UNDEFINE:
      /* Add NULL to fix warnings.  */
      as_bad ((char *)arg1, NULL);
      break;
    case WARNING_IDLY:
      as_warn (_(err_formats[err].fmt), (long)arg1);
      break;
    case WARNING_OPTIONS:
      as_warn (_(err_formats[err].fmt), (char *)arg1, (char *)arg2);
      break;
    default:
      break;
    }
}

/* Handle errors in branch relaxation.  */

static void
csky_branch_report_error (const char* file, unsigned int line,
			  symbolS* sym, offsetT val)
{
  as_bad_where (file ? file : _("unknown"),
		line,
		_("pcrel offset for branch to %s too far (0x%lx)"),
		sym ? S_GET_NAME (sym) : _("<unknown>"),
		(long) val);
}

/* Set appropriate flags for the cpu matching STR.  */

static void
parse_cpu (const char *str)
{
  int i = 0;

  for (; csky_cpus[i].name != NULL; i++)
    if (strncasecmp (str, csky_cpus[i].name, strlen (csky_cpus[i].name)) == 0)
      {
	csky_insn.cpu = &csky_cpus[i];
	mach_flag |= csky_cpus[i].arch_flag;
	isa_flag = csky_cpus[i].isa_flag;
	const char *s = str + strlen (csky_cpus[i].name);
	while (*s)
	  {
	    const struct csky_cpu_feature *feature = csky_cpus[i].features;
	    const struct csky_cpu_version *version = csky_cpus[i].ver;
	    char *next;

	    if (*s == 'r')
	      {
		s++;
		while (version->r)
		  {
		    if (version->r == strtol (s, &next, 10))
		      break;
		    version++;
		  }
		if (version->r)
		  {
		    isa_flag |= version->isa_flag;
		    s = next;
		  }
		else
		  goto unknown_cpu;
		isa_flag = isa_flag & ~CSKYV2_ISA_DSP;
		isa_flag |= CSKY_ISA_EDSP;
		continue;
	      }

	    /* Parse csky features.  */
	    while (feature->unique)
	      {
		if (feature->unique == *s)
		  break;
		feature++;
	      }
	    if (feature->unique)
	      {
		isa_flag |= feature->isa_flag;
		mach_flag |= feature->arch_flag;
	      }
	    else
	      goto unknown_cpu;

	    s++;
	  }
	return;
      }

unknown_cpu:
  as_bad (_("unknown cpu `%s'"), str);
}

/* Set appropriate flags for the arch matching STR.  */

static void
parse_arch (const char *str)
{
  int i = 0;
  for (; csky_cpus[i].name != NULL; i++)
    if (strcasecmp (str, csky_cpus[i].name) == 0)
      {
	csky_insn.cpu = &csky_cpus[i];
	arch_flag |= csky_cpus[i].arch_flag;
	isa_flag |= csky_cpus[i].isa_flag;
	return;
      }
  as_bad (_("unknown architecture `%s'"), str);
}

struct csky_option_value_table
{
  const char *name;
  long value;
};

static const struct csky_option_value_table csky_float_abis[] =
{
  {"hard",	VAL_CSKY_FPU_ABI_HARD},
  {"softfp",	VAL_CSKY_FPU_ABI_SOFTFP},
  {"soft",	VAL_CSKY_FPU_ABI_SOFT},
  {NULL,	0}
};

static bool
parse_float_abi (const char *str)
{
  const struct csky_option_value_table * opt;

  for (opt = csky_float_abis; opt->name != NULL; opt++)
    if (strcasecmp (opt->name, str) == 0)
      {
	float_abi = opt->value;
	return true;
      }

  as_bad (_("unknown floating point abi `%s'\n"), str);
  return false;
}

#ifdef OBJ_ELF
/* Implement the TARGET_FORMAT macro.  */

const char *
elf32_csky_target_format (void)
{
  return (target_big_endian
	  ? "elf32-csky-big"
	  : "elf32-csky-little");
}
#endif

/* Turn an integer of n bytes (in val) into a stream of bytes appropriate
   for use in the a.out file, and stores them in the array pointed to by buf.
   This knows about the endian-ness of the target machine and does
   THE RIGHT THING, whatever it is.  Possible values for n are 1 (byte)
   2 (short) and 4 (long)  Floating numbers are put out as a series of
   LITTLENUMS (shorts, here at least).  */

void
md_number_to_chars (char * buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

/* Get a log2(val).  */

static int
csky_log_2 (unsigned int val)
{
    int log = -1;
    if ((val & (val - 1)) == 0)
      for (; val; val >>= 1)
	log ++;
    else
      csky_show_error (ERROR_IMM_POWER, 0, (void *)(long)val, NULL);
    return log;
}

/* Output one instruction to the buffer at PTR.  */

static void
csky_write_insn (char *ptr, valueT use, int nbytes)
{
  if (nbytes == 2)
    md_number_to_chars (ptr, use, nbytes);
  else  /* 32-bit instruction.  */
    {
      /* Significant figures are in low bits.  */
      md_number_to_chars (ptr, use >> 16, 2);
      md_number_to_chars (ptr + 2, use & 0xFFFF, 2);
    }
}

/* Read an NBYTES instruction from the buffer at PTR.  NBYTES should
   be either 2 or 4.  This function is used in branch relaxation.  */

static valueT
csky_read_insn (char *ptr, int nbytes)
{
  unsigned char *uptr = (unsigned char *)ptr;
  valueT v = 0;
  int lo, hi;   /* hi/lo byte index in binary stream.  */

  if (target_big_endian)
    {
      hi = 0;
      lo = 1;
    }
  else
    {
      hi = 1;
      lo = 0;
    }
  v = uptr[lo] | (uptr[hi] << 8);
  if (nbytes == 4)
    {
      v <<= 16;
      v |=  uptr[lo + 2] | (uptr[hi + 2] << 8);
    }
  return v;
}

/* Construct a label name into S from the 3-character prefix P and
   number N formatted as a 4-digit hex number.  */

static void
make_internal_label (char *s, const char *p, int n)
{
  static const char hex[] = "0123456789ABCDEF";

  s[0] = p[0];
  s[1] = p[1];
  s[2] = p[2];
  s[3] = hex[(n >> 12) & 0xF];
  s[4] = hex[(n >>  8) & 0xF];
  s[5] = hex[(n >>  4) & 0xF];
  s[6] = hex[(n)       & 0xF];
  s[7] = 0;
}

/* md_operand is a no-op on C-SKY; we do everything elsewhere.  */

void
md_operand (expressionS *expressionP ATTRIBUTE_UNUSED)
{
  return;
}

/* Under ELF we need to default _GLOBAL_OFFSET_TABLE.
   Otherwise we have no need to default values of symbols.  */

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
#ifdef OBJ_ELF
  /* TODO:  */
#endif
  return NULL;
}

/* Use IEEE format for floating-point constants.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, target_big_endian);
}

/* Print option help to FP.  */

void
md_show_usage (FILE *fp)
{
  int i, n;
  const int margin = 48;

  fprintf (fp, _("C-SKY assembler options:\n"));

  fprintf (fp, _("\
  -march=ARCH			select architecture ARCH:"));
  for (i = 0, n = margin; csky_archs[i].name != NULL; i++)
    {
      int l = strlen (csky_archs[i].name);
      if (n + l >= margin)
	{
	  fprintf (fp, "\n\t\t\t\t");
	  n = l;
	}
      else
	{
	  fprintf (fp, " ");
	  n += l + 1;
	}
      fprintf (fp, "%s", csky_archs[i].name);
    }
  fprintf (fp, "\n");

  fprintf (fp, _("\
  -mcpu=CPU			select processor CPU:"));
  const struct csky_cpu_feature *feature = NULL;
  const struct csky_cpu_version *version = NULL;
  for (i = 0; csky_cpus[i].name != NULL; i++)
    {
	fprintf (fp, "\t\t\t\t%s", csky_cpus[i].name);
	feature = csky_cpus[i].features;
	version = csky_cpus[i].ver;
	while (feature->unique)
	  {
	    if ((feature + 1)->unique)
	      fprintf (fp, "[%c]", feature->unique);
	    feature++;
	  }
	while (version->r)
	  {
	    if (csky_cpus[i].name[0] == 'c'
		&& csky_cpus[i].name[1] == 'k')
	      fprintf (fp, "[r%d]", version->r);
	    else
	      fprintf (fp, "[-r%dp%d]", version->r, version->p);
	    version++;
	  }
    }
  fprintf (fp, "\n");

  fprintf (fp, _("\
  -mfloat-abi=ABI		select float ABI:"));
  for (i = 0, n = margin; csky_float_abis[i].name != NULL; i++)
    {
      int l = strlen (csky_float_abis[i].name);
      if (n + l >= margin)
	{
	  fprintf (fp, "\n\t\t\t\t");
	  n = l;
	}
      else
	{
	  fprintf (fp, " ");
	  n += l + 1;
	}
      fprintf (fp, "%s", csky_float_abis[i].name);
    }
  fprintf (fp, "\n");

  fprintf (fp, _("\
  -EL  -mlittle-endian		generate little-endian output\n"));
  fprintf (fp, _("\
  -EB  -mbig-endian		generate big-endian output\n"));
  fprintf (fp, _("\
  -fpic  -pic			generate position-independent code\n"));

  fprintf (fp, _("\
  -mljump			transform jbf, jbt, jbr to jmpi (CK800 only)\n"));
  fprintf (fp, _("\
  -mno-ljump\n"));

#ifdef INCLUDE_BRANCH_STUB
  fprintf (fp, _("\
  -mbranch-stub			enable branch stubs for PC-relative calls\n"));
  fprintf (fp, _("\
  -mno-branch-stub\n"));
#endif

  fprintf (fp, _("\
  -force2bsr  -mforce2bsr	transform jbsr to bsr\n"));
  fprintf (fp, _("\
  -no-force2bsr  -mno-force2bsr\n"));
  fprintf (fp, _("\
  -jsri2bsr  -mjsri2bsr		transform jsri to bsr\n"));
  fprintf (fp, _("\
  -no-jsri2bsr  -mno-jsri2bsr\n"));

  fprintf (fp, _("\
  -mnolrw  -mno-lrw		implement lrw as movih + ori\n"));
  fprintf (fp, _("\
  -melrw			enable extended lrw (CK800 only)\n"));
  fprintf (fp, _("\
  -mno-elrw\n"));

  fprintf (fp, _("\
  -mlaf  -mliterals-after-func	emit literals after each function\n"));
  fprintf (fp, _("\
  -mno-laf  -mno-literals-after-func\n"));
  fprintf (fp, _("\
  -mlabr  -mliterals-after-br	emit literals after branch instructions\n"));
  fprintf (fp, _("\
  -mno-labr  -mnoliterals-after-br\n"));

  fprintf (fp, _("\
  -mistack			enable interrupt stack instructions\n"));
  fprintf (fp, _("\
  -mno-istack\n"));

  fprintf (fp, _("\
  -mhard-float			enable hard float instructions\n"));
  fprintf (fp, _("\
  -mmp				enable multiprocessor instructions\n"));
  fprintf (fp, _("\
  -mcp				enable coprocessor instructions\n"));
  fprintf (fp, _("\
  -mcache			enable cache prefetch instruction\n"));
  fprintf (fp, _("\
  -msecurity			enable security instructions\n"));
  fprintf (fp, _("\
  -mtrust			enable trust instructions\n"));
  fprintf (fp, _("\
  -mdsp				enable DSP instructions\n"));
  fprintf (fp, _("\
  -medsp			enable enhanced DSP instructions\n"));
  fprintf (fp, _("\
  -mvdsp			enable vector DSP instructions\n"));
}

static void set_csky_attribute (void)
{
  if (mach_flag & CSKY_ARCH_DSP)
    {
      if (dsp_flag & CSKY_DSP_FLAG_V2)
	{
	  /* Set DSPV2.  */
	  bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
				    Tag_CSKY_DSP_VERSION,
				    VAL_CSKY_DSP_VERSION_2);
	}
      else if (isa_flag & CSKY_ISA_DSP)
	{
	  /* Set DSP extension.  */
	  bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
				    Tag_CSKY_DSP_VERSION,
				    VAL_CSKY_DSP_VERSION_EXTENSION);
	}
      /* Set VDSP attribute.  */
      if (isa_flag & CSKY_ISA_VDSP)
	bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
				  Tag_CSKY_VDSP_VERSION,
				  VAL_CSKY_VDSP_VERSION_1);

      else if (isa_flag & CSKY_ISA_VDSP_2)
	bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
				  Tag_CSKY_VDSP_VERSION,
				  VAL_CSKY_VDSP_VERSION_2);

    }

  if (mach_flag & CSKY_ARCH_FLOAT)
    {
      unsigned int val = VAL_CSKY_FPU_HARDFP_SINGLE;
      if (IS_CSKY_ARCH_V1 (mach_flag)) {
	bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
				  Tag_CSKY_FPU_VERSION,
				  VAL_CSKY_FPU_VERSION_1);
      }
      else
	{
	  if (isa_flag & CSKY_ISA_FLOAT_3E4)
	    {
	      bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
					Tag_CSKY_FPU_VERSION,
					VAL_CSKY_FPU_VERSION_2);
	      val |= VAL_CSKY_FPU_HARDFP_DOUBLE;
	    }
	  else
	    {
	      bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
					Tag_CSKY_FPU_VERSION,
					VAL_CSKY_FPU_VERSION_2);
	    }

	  bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
				    Tag_CSKY_FPU_HARDFP,
				    val);
	  bfd_elf_add_obj_attr_string (stdoutput, OBJ_ATTR_PROC,
				    Tag_CSKY_FPU_NUMBER_MODULE,
				    "IEEE 754");
	  bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
				    Tag_CSKY_FPU_ABI,
				    float_abi);
	}
    }


  bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
			    Tag_CSKY_ISA_FLAGS, isa_flag);

  bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_PROC,
			    Tag_CSKY_ISA_EXT_FLAGS, (isa_flag >> 32));
}

/* Target-specific initialization and option handling.  */

void
md_begin (void)
{
  unsigned int bfd_mach_flag = 0;
  struct csky_opcode const *opcode;
  struct csky_macro_info const *macro;
  struct csky_arch_info const *p_arch;
  struct csky_cpu_info const *p_cpu;
  other_flag = (do_opt_mmp | do_opt_mcp | do_opt_mcache
		| do_opt_msecurity | do_opt_mhard_float);
  dsp_flag |= do_opt_mdsp | do_opt_medsp;
  isa_flag |= do_opt_mtrust | do_opt_mvdsp;

  if (dsp_flag)
    other_flag |= CSKY_ARCH_DSP;

  if (mach_flag != 0)
    {
      if (((mach_flag & CSKY_ARCH_MASK)
	    != (arch_flag & CSKY_ARCH_MASK))
	   && arch_flag != 0)
	as_warn ("-mcpu conflict with -march option, actually use -mcpu");
    }
  else if (arch_flag != 0)
    mach_flag |= arch_flag | other_flag;
  else
    {
#ifdef TARGET_WITH_CPU
      parse_cpu (TARGET_WITH_CPU);
#else
#if _CSKY_ABI==1
      parse_cpu ("ck610");
#else
      parse_cpu ("ck810");
#endif
      mach_flag |= other_flag;
#endif
    }

  if (IS_CSKY_ARCH_610 (mach_flag) || IS_CSKY_ARCH_510 (mach_flag))
    {
      if ((mach_flag & CSKY_ARCH_MP) && (mach_flag & CSKY_ARCH_MAC))
	as_fatal ("520/620 conflicts with -mmp option");
      else if ((mach_flag & CSKY_ARCH_MP) && (mach_flag & CSKY_ARCH_DSP))
	as_fatal ("510e/610e conflicts with -mmp option");
      else if ((mach_flag & CSKY_ARCH_DSP) && (mach_flag & CSKY_ARCH_MAC))
	as_fatal ("520/620 conflicts with 510e/610e or -mdsp option");
    }
  if (IS_CSKY_ARCH_510 (mach_flag) && (mach_flag & CSKY_ARCH_FLOAT))
    {
      mach_flag = (mach_flag & (~CSKY_ARCH_MASK));
      mach_flag |= CSKY_ARCH_610;
    }

  /* Find bfd_mach_flag, it will set to bfd backend data.  */
  for (p_arch = csky_archs; p_arch->arch_flag != 0; p_arch++)
    if ((mach_flag & CSKY_ARCH_MASK) == (p_arch->arch_flag & CSKY_ARCH_MASK))
      {
	bfd_elf_add_obj_attr_string (stdoutput, OBJ_ATTR_PROC,
				     Tag_CSKY_ARCH_NAME, p_arch->name);
	bfd_mach_flag =  p_arch->bfd_mach_flag;
	break;
      }

  /* Find isa_flag.  */
  for (p_cpu = csky_cpus; p_cpu->arch_flag != 0; p_cpu++)
    if ((mach_flag & CPU_ARCH_MASK) == p_cpu->arch_flag)
      {
	bfd_elf_add_obj_attr_string (stdoutput, OBJ_ATTR_PROC,
				     Tag_CSKY_CPU_NAME, p_cpu->name);
	isa_flag |= p_cpu->isa_flag;
	break;
      }

  /* Check if -mdsp and -medsp conflict. If cpu is ck803, we will
     use enhanced dsp instruction. Otherwise, we will use normal dsp.  */
  if (dsp_flag)
    {
      if (IS_CSKY_ARCH_803 (mach_flag))
	{
	  if ((dsp_flag & CSKY_DSP_FLAG_V1))
	    {
	      if (isa_flag & CSKY_ISA_DSP_ENHANCE)
		{
		  /* Option -mdsp conflicts with -mcpu=ck803ern,
		     CPU already indicates the dsp version.  */
		  as_warn ("Option -mdsp conflicts with -mcpu=ck803ern which "
		           "has indicated DSP version, ignoring -mdsp.");
		  isa_flag &= ~(CSKY_ISA_MAC_DSP | CSKY_ISA_DSP);
		  isa_flag |= CSKY_ISA_DSP_ENHANCE;
		}
	      else
		{
		  isa_flag |= (CSKY_ISA_MAC_DSP | CSKY_ISA_DSP);
		  isa_flag &= ~CSKY_ISA_DSP_ENHANCE;
		}
	    }

	  if ((dsp_flag & CSKY_DSP_FLAG_V2))
	    {
	      isa_flag &= ~(CSKY_ISA_MAC_DSP | CSKY_ISA_DSP);
	      isa_flag |= CSKY_ISA_DSP_ENHANCE;
	    }

	  if ((dsp_flag & CSKY_DSP_FLAG_V1)
	      && (dsp_flag & CSKY_DSP_FLAG_V2))
	    {
	      /* In 803, dspv1 is conflict with dspv2. We keep dspv2.  */
	      as_warn ("option -mdsp conflicts with -medsp, only enabling -medsp");
        dsp_flag &= ~CSKY_DSP_FLAG_V1;
	      isa_flag &= ~(CSKY_ISA_MAC_DSP | CSKY_ISA_DSP);
	      isa_flag |= CSKY_ISA_DSP_ENHANCE;
	    }
	}
      else
	{
	  if (dsp_flag & CSKY_DSP_FLAG_V2)
	    {
	      dsp_flag &= ~CSKY_DSP_FLAG_V2;
	      isa_flag &= ~CSKY_ISA_DSP_ENHANCE;
	      as_warn ("-medsp option is only supported by ck803s, ignoring -medsp");
	    }
	}
      ;
    }

  if (do_use_branchstub == -1)
    do_use_branchstub = !IS_CSKY_ARCH_V1 (mach_flag);
  else if (do_use_branchstub == 1)
    {
      if (IS_CSKY_ARCH_V1 (mach_flag))
	{
	  as_warn (_("C-SKY ABI v1 (ck510/ck610) does not support -mbranch-stub"));
	  do_use_branchstub = 0;
	}
      else if (do_force2bsr == 0)
	{
	  as_warn (_("-mno-force2bsr is ignored with -mbranch-stub"));
	  do_force2bsr = 1;
	}
    }

  if (IS_CSKY_ARCH_801 (mach_flag) || IS_CSKY_ARCH_802 (mach_flag))
    {
      if (!do_force2bsr)
	as_warn (_("-mno-force2bsr is ignored for ck801/ck802"));
      do_force2bsr = 1;
    }
  else if (do_force2bsr == -1)
    do_force2bsr = do_use_branchstub;

  if (do_pff == -1)
    {
      if (IS_CSKY_ARCH_V1 (mach_flag))
	do_pff = 1;
      else
	do_pff = 0;
    }

  if (do_extend_lrw == -1)
    {
      if ((mach_flag & CSKY_ARCH_MASK) == CSKY_ARCH_801
	  || (mach_flag & CSKY_ARCH_MASK) == CSKY_ARCH_802
	  || (mach_flag & CSKY_ARCH_MASK) == CSKY_ARCH_803
	  || (mach_flag & CSKY_ARCH_MASK) == CSKY_ARCH_860)
	do_extend_lrw = 1;
      else
	do_extend_lrw = 0;
    }
  if (IS_CSKY_ARCH_801 (mach_flag) || IS_CSKY_ARCH_802 (mach_flag))
    {
      if (do_long_jump > 0)
	as_warn (_("-mljump is ignored for ck801/ck802"));
      do_long_jump = 0;
    }
  else if (do_long_jump == -1)
    do_long_jump = 1;
  if (do_intr_stack == -1)
    {
      /* control interrupt stack module, 801&802&803 default on
	 807&810, default off.  */
      if (IS_CSKY_ARCH_807 (mach_flag) || IS_CSKY_ARCH_810 (mach_flag))
	do_intr_stack = 0;
      else
	do_intr_stack = 1;
    }
  /* Add isa_flag(SIMP/CACHE/APS).  */
  isa_flag |= (mach_flag & CSKY_ARCH_MAC) ? CSKY_ISA_MAC : 0;
  isa_flag |= (mach_flag & CSKY_ARCH_MP) ? CSKY_ISA_MP : 0;
  isa_flag |= (mach_flag & CSKY_ARCH_CP) ? CSKY_ISA_CP : 0;

  /* Set abi flag and get table address.  */
  if (IS_CSKY_ARCH_V1 (mach_flag))
    {
      mach_flag = mach_flag | CSKY_ABI_V1;
      opcode = csky_v1_opcodes;
      macro = v1_macros_table;
      SPANPANIC = v1_SPANPANIC;
      SPANCLOSE = v1_SPANCLOSE;
      SPANEXIT  = v1_SPANEXIT;
      md_relax_table = csky_relax_table;
    }
  else
    {
      mach_flag = mach_flag | CSKY_ABI_V2;
      opcode = csky_v2_opcodes;
      macro = v2_macros_table;
      SPANPANIC = v2_SPANPANIC;
      if (do_extend_lrw)
	{
	  SPANCLOSE = v2_SPANCLOSE_ELRW;
	  SPANEXIT  = v2_SPANEXIT_ELRW;
	}
      else
	{
	  SPANCLOSE = v2_SPANCLOSE;
	  SPANEXIT  = v2_SPANEXIT;
	}
      md_relax_table = csky_relax_table;
    }

  /* Establish hash table for opcodes and macros.  */
  csky_macros_hash = str_htab_create ();
  csky_opcodes_hash = str_htab_create ();
  for ( ; opcode->mnemonic != NULL; opcode++)
    if ((isa_flag & (opcode->isa_flag16 | opcode->isa_flag32)) != 0)
      str_hash_insert (csky_opcodes_hash, opcode->mnemonic, opcode, 0);
  for ( ; macro->name != NULL; macro++)
    if ((isa_flag & macro->isa_flag) != 0)
      str_hash_insert (csky_macros_hash, macro->name, macro, 0);
  if (do_nolrw && (isa_flag & CSKYV2_ISA_1E2) != 0)
    str_hash_insert (csky_macros_hash,
		     v2_lrw_macro_opcode.name, &v2_lrw_macro_opcode, 0);
  /* Set e_flag to ELF Head.  */
  bfd_set_private_flags (stdoutput, mach_flag | CSKY_VERSION_V1);
  /* Set bfd_mach to bfd backend data.  */
  bfd_set_arch_mach (stdoutput, bfd_arch_csky, bfd_mach_flag);

  set_csky_attribute ();
}

/* The C-SKY assembler emits mapping symbols $t and $d to mark the
   beginning of a sequence of instructions and data (such as a constant pool),
   respectively.  This is similar to what ARM does.  */

static void
make_mapping_symbol (map_state state, valueT value, fragS *frag)
{
  symbolS * symbolP;
  const char * symname;
  int type;
  switch (state)
    {
    case MAP_DATA:
      symname = "$d";
      type = BSF_NO_FLAGS;
      break;
    case MAP_TEXT:
      symname = "$t";
      type = BSF_NO_FLAGS;
      break;
    default:
      abort ();
    }

  symbolP = symbol_new (symname, now_seg, frag, value);
  symbol_get_bfdsym (symbolP)->flags |= type | BSF_LOCAL;
}

/* We need to keep track of whether we are emitting code or data; this
   function switches state and emits a mapping symbol if necessary.  */

static void
mapping_state (map_state state)
{
  map_state current_state
    = seg_info (now_seg)->tc_segment_info_data.current_state;

  if (current_state == state)
    return;
  else if (current_state == MAP_UNDEFINED && state == MAP_DATA)
    return;
  else if (current_state == MAP_UNDEFINED && state == MAP_TEXT)
   {
     struct frag * const frag_first = seg_info (now_seg)->frchainP->frch_root;
     if (frag_now != frag_first || frag_now_fix () > 0)
       make_mapping_symbol (MAP_DATA, (valueT) 0, frag_first);
   }

  seg_info (now_seg)->tc_segment_info_data.current_state = state;
  make_mapping_symbol (state, (valueT) frag_now_fix (), frag_now);
}

/* Dump the literal pool.  */

static void
dump_literals (int isforce)
{
#define CSKYV1_BR_INSN  0xF000
#define CSKYV2_BR_INSN  0x0400
  unsigned int i;
  struct literal * p;
  symbolS * brarsym = NULL;

  /* V1 nop encoding:  0x1200 : mov r0, r0.  */
  static char v1_nop_insn_big[2] = {0x12, 0x00};
  static char v1_nop_insn_little[2] = {0x00, 0x12};

  if (poolsize == 0)
    return;

  /* Must we branch around the literal table?  */
  if (isforce)
    {
      char brarname[8];
      make_internal_label (brarname, POOL_END_LABEL, poolnumber);
      brarsym = symbol_make (brarname);
      symbol_table_insert (brarsym);
      mapping_state (MAP_TEXT);
      if (IS_CSKY_ARCH_V1 (mach_flag))
	{
	  csky_insn.output
	    = frag_var (rs_machine_dependent,
			csky_relax_table[C (UNCD_JUMP_S, DISP32)].rlx_length,
			csky_relax_table[C (UNCD_JUMP_S, DISP12)].rlx_length,
			C (UNCD_JUMP_S, 0), brarsym, 0, 0);
	  md_number_to_chars (csky_insn.output, CSKYV1_BR_INSN, 2);
	}
      else
	{
	  csky_insn.output
	    = frag_var (rs_machine_dependent,
			UNCD_DISP16_LEN,
			UNCD_DISP10_LEN,
			UNCD_DISP10,
			brarsym, 0, 0);
	  md_number_to_chars (csky_insn.output, CSKYV2_BR_INSN, 2);
	}
    }
  /* Make sure that the section is sufficiently aligned and that
     the literal table is aligned within it.  */
  if (do_pff)
    {
      valueT br_self;
      csky_insn.output = frag_more (2);
      /* .Lxx: br .Lxx  */
      if (IS_CSKY_V1 (mach_flag))
	br_self = CSKYV1_BR_INSN | 0x7ff;
      else
	br_self = CSKYV2_BR_INSN;
      md_number_to_chars (csky_insn.output, br_self, 2);
      if (!isforce)
	{
	  csky_insn.output = frag_more (2);
	  /* .Lxx: br .Lxx  */
	  md_number_to_chars (csky_insn.output, br_self, 2);
	}
    }
  mapping_state (MAP_DATA);

  record_alignment (now_seg, 2);
  if (IS_CSKY_ARCH_V1 (mach_flag))
    frag_align_pattern (2,
			(target_big_endian
			 ? v1_nop_insn_big : v1_nop_insn_little),
			2, 0);
  else
    frag_align (2, 0, 3);

  colon (S_GET_NAME (poolsym));

  for (i = 0, p = litpool; i < poolsize; p++)
    {
      insn_reloc = p->r_type;
      if (insn_reloc == BFD_RELOC_CKCORE_TLS_IE32
	  || insn_reloc == BFD_RELOC_CKCORE_TLS_LDM32
	  || insn_reloc == BFD_RELOC_CKCORE_TLS_GD32)
	literal_insn_offset = p;
      if (p->isdouble)
	{
	  if (target_big_endian)
	    {
	      p->e.X_add_number = p->dbnum >> 32;
	      emit_expr (& p->e, 4);
	      p->e.X_add_number = p->dbnum & 0xffffffff;
	      emit_expr (& p->e, 4);
	    }
	  else
	    {
	      p->e.X_add_number = p->dbnum & 0xffffffff;
	      emit_expr (& p->e, 4);
	      p->e.X_add_number = p->dbnum >> 32;
	      emit_expr (& p->e, 4);
	    }
	}
      else if (p->e.X_op == O_big)
	{
	  memcpy (generic_bignum, p->bignum, sizeof (p->bignum));
	  emit_expr (& p->e, p->e.X_add_number * CHARS_PER_LITTLENUM);
	}
      else
	emit_expr (& p->e, 4);

      if (p->e.X_op == O_big)
	i += (p->e.X_add_number & 1) +
	  ((p->e.X_add_number  * CHARS_PER_LITTLENUM) >> 2);
      else
	i += (p->isdouble ? 2 : 1);
    }

  if (isforce && IS_CSKY_ARCH_V2 (mach_flag))
    {
      /* Add one nop insn at end of literal for disassembler.  */
      mapping_state (MAP_TEXT);
      csky_insn.output = frag_more (2);
      md_number_to_chars (csky_insn.output, CSKYV2_INST_NOP, 2);
    }

  insn_reloc = BFD_RELOC_NONE;

  if (brarsym != NULL)
    colon (S_GET_NAME (brarsym));
  poolsize = 0;
}

static struct literal *
enter_literal (expressionS *e,
	       int ispcrel,
	       unsigned char isdouble,
	       uint64_t dbnum)
{
  unsigned int i;
  struct literal * p;
  if (poolsize >= MAX_POOL_SIZE - 2)
    {
      /* The literal pool is as full as we can handle. We have
	 to be 2 entries shy of the 1024/4=256 entries because we
	 have to allow for the branch (2 bytes) and the alignment
	 (2 bytes before the first insn referencing the pool and
	 2 bytes before the pool itself) == 6 bytes, rounds up
	 to 2 entries.  */

      /* Save the parsed symbol's reloc.  */
      enum bfd_reloc_code_real last_reloc_before_dump = insn_reloc;
      dump_literals (1);
      insn_reloc = last_reloc_before_dump;
    }

  if (poolsize == 0)
    {
      /* Create new literal pool.  */
      if (++ poolnumber > 0xFFFF)
	as_fatal (_("more than 65K literal pools"));

      make_internal_label (poolname, POOL_START_LABEL, poolnumber);
      poolsym = symbol_make (poolname);
      symbol_table_insert (poolsym);
      poolspan = 0;
    }

  /* Search pool for value so we don't have duplicates.  */
  for (p = litpool,i = 0; i < poolsize; p++)
    {
      if (e->X_op == p->e.X_op
	  && e->X_add_symbol == p->e.X_add_symbol
	  && e->X_add_number == p->e.X_add_number
	  && ispcrel == p->ispcrel
	  && insn_reloc == p->r_type
	  && isdouble == p->isdouble
	  && insn_reloc != BFD_RELOC_CKCORE_TLS_GD32
	  && insn_reloc != BFD_RELOC_CKCORE_TLS_LDM32
	  && insn_reloc != BFD_RELOC_CKCORE_TLS_LDO32
	  && insn_reloc != BFD_RELOC_CKCORE_TLS_IE32
	  && insn_reloc != BFD_RELOC_CKCORE_TLS_LE32
	  && (e->X_op != O_big
	      || (memcmp (generic_bignum, p->bignum,
			  p->e.X_add_number * sizeof (LITTLENUM_TYPE)) == 0)))
	{
	  p->refcnt ++;
	  return p;
	}
      if (p->e.X_op == O_big)
	{
	  i += (p->e.X_add_number >> 1);
	  i += (p->e.X_add_number & 0x1);
    }
      else
	i += (p->isdouble ? 2 : 1);
    }
  p->refcnt = 1;
  p->ispcrel = ispcrel;
  p->e = *e;
  p->r_type = insn_reloc;
  p->isdouble = isdouble;
  p->offset = i;
  if (isdouble)
    p->dbnum = dbnum;
  if (e->X_op == O_big)
    memcpy (p->bignum, generic_bignum, sizeof (p->bignum));

  if (insn_reloc == BFD_RELOC_CKCORE_TLS_GD32
      || insn_reloc == BFD_RELOC_CKCORE_TLS_LDM32
      || insn_reloc == BFD_RELOC_CKCORE_TLS_IE32)
    {
      p->tls_addend.frag  = frag_now;
      p->tls_addend.offset = csky_insn.output - frag_now->fr_literal;
      literal_insn_offset = p;
    }
  if (p->e.X_op == O_big) {
    poolsize += (p->e.X_add_number >> 1);
    poolsize += (p->e.X_add_number & 0x1);
  } else
  poolsize += (p->isdouble ? 2 : 1);

  return p;
}

/* Check whether we must dump the literal pool here.
   kind == 0 is any old instruction.
   kind  > 0 means we just had a control transfer instruction.
   kind == 1 means within a function.
   kind == 2 means we just left a function.

   OFFSET is the length of the insn being processed.

   SPANCLOSE and SPANEXIT are smaller numbers than SPANPANIC.
   SPANPANIC means that we must dump now.
   The dump_literals (1) call inserts a branch around the table, so
   we first look to see if its a situation where we won't have to
   insert a branch (e.g., the previous instruction was an unconditional
   branch).

   SPANPANIC is the point where we must dump a single-entry pool.
   it accounts for alignments and an inserted branch.
   the 'poolsize*2' accounts for the scenario where we do:
   lrw r1,lit1; lrw r2,lit2; lrw r3,lit3
   Note that the 'lit2' reference is 2 bytes further along
   but the literal it references will be 4 bytes further along,
   so we must consider the poolsize into this equation.
   This is slightly over-cautious, but guarantees that we won't
   panic because a relocation is too distant.  */

static void
check_literals (int kind, int offset)
{
  poolspan += offset;

  if ((poolspan > SPANEXIT || do_func_dump)
      && kind > 1
      && (do_br_dump || do_func_dump))
    dump_literals (0);
  else if (poolspan > SPANCLOSE && (kind > 0) && do_br_dump)
    dump_literals (0);
  else if (poolspan
	   >= (SPANPANIC - (IS_CSKY_ARCH_V1 (mach_flag) ?  poolsize * 2 : 0)))
    dump_literals (1);
  /* We have not dumped literal pool before insn1,
     and will not dump literal pool between insn1 and insnN+1,
     so reset poolspan to original length.  */
  else if (do_noliteraldump == 1)
    poolspan -= offset;

  if (do_noliteraldump == 1)
    do_noliteraldump = 0;
}

/* The next group of functions are helpers for parsing various kinds
   of instruction operand syntax.  */

/* Parse operands of the form
   <symbol>@GOTOFF+<nnn>
   and similar .plt or .got references.

   If we find one, set up the correct relocation in RELOC and copy the
   input string, minus the `@GOTOFF' into a malloc'd buffer for
   parsing by the calling routine.  Return this buffer, and if ADJUST
   is non-null set it to the length of the string we removed from the
   input line.  Otherwise return NULL.  */

static char *
lex_got (enum bfd_reloc_code_real *reloc,
	 int *adjust)
{
  struct _gotrel
  {
    const char *str;
    const enum bfd_reloc_code_real rel;
  };
  static const struct _gotrel gotrel[] =
    {
      { "GOTOFF",     BFD_RELOC_CKCORE_GOTOFF      },
      { "GOTPC",      BFD_RELOC_CKCORE_GOTPC       },
      { "GOTTPOFF",   BFD_RELOC_CKCORE_TLS_IE32    },
      { "GOT",        BFD_RELOC_CKCORE_GOT32       },
      { "PLT",        BFD_RELOC_CKCORE_PLT32       },
      { "BTEXT",      BFD_RELOC_CKCORE_TOFFSET_LO16},
      { "BDATA",      BFD_RELOC_CKCORE_DOFFSET_LO16},
      { "TLSGD32",    BFD_RELOC_CKCORE_TLS_GD32    },
      { "TLSLDM32",   BFD_RELOC_CKCORE_TLS_LDM32   },
      { "TLSLDO32",   BFD_RELOC_CKCORE_TLS_LDO32   },
      { "TPOFF",      BFD_RELOC_CKCORE_TLS_LE32    }
    };

  char *cp;
  unsigned int j;

  for (cp = input_line_pointer; *cp != '@'; cp++)
    if (is_end_of_line[(unsigned char) *cp])
      return NULL;

  for (j = 0; j < sizeof (gotrel) / sizeof (gotrel[0]); j++)
    {
      int len = strlen (gotrel[j].str);

      if (strncasecmp (cp + 1, gotrel[j].str, len) == 0)
	{
	  if (gotrel[j].rel != 0)
	    {
	      *reloc = gotrel[j].rel;
	      if (adjust)
		*adjust = len;

	      /* input_line_pointer is the str pointer after relocation
		 token like @GOTOFF.  */
	      input_line_pointer += len + 1;
	      return input_line_pointer;
	    }

	  csky_show_error (ERROR_RELOC_ILLEGAL, 0,
			   (void *)gotrel[j].str, NULL);
	  return NULL;
	}
    }

  /* Might be a symbol version string.  Don't as_bad here.  */
  return NULL;
}

/* Parse an expression, returning it in E.  */

static char *
parse_exp (char *s, expressionS *e)
{
  char *save;
  char *new;

  /* Skip whitespace.  */
  while (ISSPACE (*s))
    ++s;

  save = input_line_pointer;
  input_line_pointer = s;

  insn_reloc = BFD_RELOC_NONE;
  expression (e);
  lex_got (&insn_reloc, NULL);

  if (e->X_op == O_absent)
    SET_ERROR_STRING (ERROR_MISSING_OPERAND, NULL);

  new = input_line_pointer;
  input_line_pointer = save;

  return new;
}

/* Parse a floating-point number from S into its target representation.
   If ISDOUBLE is true, return the result in *DBNUM; otherwise
   it's returned in E->X_add_number.  Returns the result of advancing
   S past the constant.  */

static char *
parse_fexp (char *s, expressionS *e, unsigned char isdouble, uint64_t *dbnum)
{
  int length;                       /* Number of chars in an object.  */
  const char *err = NULL;           /* Error from scanning float literal.  */
  unsigned char temp[8];

  /* input_line_pointer->1st char of a flonum (we hope!).  */
  input_line_pointer = s;

  if (input_line_pointer[0] == '0'
      && ISALPHA (input_line_pointer[1]))
    input_line_pointer += 2;

  if (isdouble)
    err = md_atof ('d', (char *) temp, &length);
  else
    err = md_atof ('f', (char *) temp, &length);
  know (length <= 8);
  know (err != NULL || length > 0);

  if (!is_end_of_line[(unsigned char) *input_line_pointer])
    as_bad (_("immediate operand required"));
  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    input_line_pointer++;

  if (err)
    {
      as_bad (_("bad floating literal: %s"), err);
      while (!is_end_of_line[(unsigned char) *input_line_pointer])
	input_line_pointer++;
      know (is_end_of_line[(unsigned char) input_line_pointer[-1]]);
      return input_line_pointer;
    }

  e->X_add_symbol = 0x0;
  e->X_op_symbol = 0x0;
  e->X_op = O_constant;
  e->X_unsigned = 1;
  e->X_md = 0x0;

  if (!isdouble)
    {
      uint32_t fnum;
      if (target_big_endian)
	fnum = (((uint32_t) temp[0] << 24)
		| (temp[1] << 16)
		| (temp[2] << 8)
		| temp[3]);
      else
	fnum = (((uint32_t) temp[3] << 24)
		| (temp[2] << 16)
		| (temp[1] << 8)
		| temp[0]);
      e->X_add_number = fnum;
    }
  else
    {
      if (target_big_endian)
	{
	  *dbnum = (((uint32_t) temp[0] << 24)
		    | (temp[1] << 16)
		    | (temp[2] << 8)
		    | temp[3]);
	  *dbnum <<= 32;
	  *dbnum |= (((uint32_t) temp[4] << 24)
		     | (temp[5] << 16)
		     | (temp[6] << 8)
		     | temp[7]);
	}
      else
	{
	  *dbnum = (((uint32_t) temp[7] << 24)
		    | (temp[6] << 16)
		    | (temp[5] << 8)
		    | temp[4]);
	  *dbnum <<= 32;
	  *dbnum |= (((uint32_t) temp[3] << 24)
		     | (temp[2] << 16)
		     | (temp[1] << 8)
		     | temp[0]);
      }
    }
  return input_line_pointer;
}

static char *
parse_rt (char *s,
	  int ispcrel,
	  expressionS *ep,
	  long reg ATTRIBUTE_UNUSED)
{
  expressionS e;

  if (ep)
    /* Indicate nothing there.  */
    ep->X_op = O_absent;

  if (*s == '[')
    {
      s = parse_exp (s + 1, &e);

      if (*s == ']')
	s++;
      else
	SET_ERROR_STRING (ERROR_MISSING_RSQUARE_BRACKETS, NULL);

      if (ep)
       *ep = e;
    }
  else
    {
      s = parse_exp (s, &e);
      if (BFD_RELOC_CKCORE_DOFFSET_LO16 == insn_reloc
	   || BFD_RELOC_CKCORE_TOFFSET_LO16 == insn_reloc)
	{
	  if (ep)
	    *ep = e;
	  return s;
	}
      if (ep)
	*ep = e;
      /* If the instruction has work, literal handling is in the work.  */
      if (!csky_insn.opcode->work)
	{
	  struct literal *p = enter_literal (&e, ispcrel, 0, 0);
	  if (ep)
	   *ep = e;

	  /* Create a reference to pool entry.  */
	  ep->X_op = O_symbol;
	  ep->X_add_symbol = poolsym;
	  ep->X_add_number = p->offset << 2;
	}
    }
  return s;
}

static int float_to_half (void *f, void *h)
{
  int imm_e;
  int imm_f;
  unsigned int value_f = *(unsigned int *)f;
  unsigned short value_h;

  imm_e = ((value_f >> 23) & 0xff);
  imm_f = ((value_f  & 0x7fffff));

  imm_e = ((imm_e - 127 + 15) << 10);
  imm_f = ((imm_f & 0x7fe000) >> 13);

  value_h = (value_f & 0x80000000 ? 0x8000 : 0x0) | imm_e | imm_f;

  if (h)
    *(unsigned short *)h = value_h;

  return value_h;
}

static char *
parse_rtf (char *s, int ispcrel, expressionS *ep)
{
  expressionS e;
  struct literal *p = NULL;

  if (ep)
    /* Indicate nothing there.  */
    ep->X_op = O_absent;

  if (*s == '[')
    {
      s = parse_exp (s + 1, & e);

      if (*s == ']')
	s++;
      else
	as_bad (_("missing ']'"));

      if (ep)
	*ep = e;
    }
  else
    {
      uint64_t dbnum;
      if (strstr(csky_insn.opcode->mnemonic, "flrws")
	  || strstr(csky_insn.opcode->mnemonic, "flrw.32"))
	{
	  s = parse_fexp (s, &e, 0, &dbnum);
	  p = enter_literal (& e, ispcrel, 0, dbnum);
	}
      else if (strstr(csky_insn.opcode->mnemonic, "flrwd")
	       || strstr(csky_insn.opcode->mnemonic, "flrw.64"))
	{
	  s = parse_fexp (s, &e, 1, &dbnum);
	  p = enter_literal (& e, ispcrel, 1, dbnum);
	}
      else if (strstr(csky_insn.opcode->mnemonic, "flrwh")
	       || strstr(csky_insn.opcode->mnemonic, "flrw.16"))
	{
	  s = parse_fexp (s, &e, 0, NULL);
	  e.X_add_number = float_to_half (&e.X_add_number, &e.X_add_number);
	  p = enter_literal (& e, ispcrel, 0, 0);
	}
      else
	as_bad (_("unrecognized opcode"));

      if (ep)
	*ep = e;

      /* Create a reference to pool entry.  */
      ep->X_op         = O_symbol;
      ep->X_add_symbol = poolsym;
      ep->X_add_number = p->offset << 2;
    }
  return s;
}

static bool
parse_type_ctrlreg (char** oper)
{
  int i = -1;
  int group = 0;
  int crx;
  int sel;
  char *s = *oper;
  expressionS e;

  if (TOLOWER (*(*oper + 0)) == 'c'
      && TOLOWER (*(*oper + 1)) == 'r'
      && ISDIGIT (*(*oper + 2)))
    {
      /* The control registers are named crxx.  */
      s = *oper+2;
      s = parse_exp (s, &e);
      if (e.X_op == O_constant)
        {
	  i = e.X_add_number;
	  *oper = s;
	}
    }

  if (IS_CSKY_V2 (mach_flag))
    {

      s = *oper;
      if (i != -1)
	{
	  crx = i;
	  sel = group;
	}
      else if (TOLOWER (*(*oper + 0)) == 'c'
	       && TOLOWER (*(*oper + 1)) == 'r')
	{
	  s += 2;
	  if (*s != '<')
	    {
	      SET_ERROR_STRING (ERROR_CREG_ILLEGAL, s);
	      return false;
	    }
	  s++;
	  crx = strtol(s, &s, 10);
	  if (crx < 0 || crx > 31 || *s != ',')
	    {
	      SET_ERROR_STRING (ERROR_CREG_ILLEGAL, s);
	      return false;
	    }
	  s++;
	  sel = strtol(s, &s, 10);
	  if (sel < 0 || sel > 31 || *s != '>')
	    {
	      SET_ERROR_STRING (ERROR_CREG_ILLEGAL, s);
	      return false;
	    }
	  s++;
	}
      else
	{
	  crx = csky_get_control_regno (mach_flag, s, &s, &sel);
	  if (crx < 0)
	    {
	      SET_ERROR_STRING (ERROR_CREG_ILLEGAL, s);
	      return false;
	    }
	}
	i = (sel << 5) | crx;
    }
  else if (i == -1)
    {
      i = csky_get_control_regno (mach_flag, s, &s, &sel);
      if (i < 0)
	{
	  SET_ERROR_STRING (ERROR_CREG_ILLEGAL, s);
	  return false;
	}
    }
  *oper = s;
  csky_insn.val[csky_insn.idx++] = i;
  return true;
}

static int
csky_get_reg_val (char *str, int *len)
{
  int regno = 0;
  char *s = str;
  regno = csky_get_general_regno (mach_flag, str, &s);
  *len = (s - str);
  return regno;
}

static bool
is_reg_sp_with_bracket (char **oper)
{
  int reg;
  int sp_idx;
  int len;

  if (IS_CSKY_V1 (mach_flag))
    sp_idx = 0;
  else
    sp_idx = 14;

  if (**oper != '(')
      return false;
  *oper += 1;
  reg = csky_get_reg_val (*oper, &len);
  *oper += len;
  if (reg == sp_idx)
    {
      if (**oper != ')')
        {
          SET_ERROR_STRING (ERROR_UNDEFINE,
			    "Operand format is error. '(sp)' expected");
          return false;
        }
      *oper += 1;
      csky_insn.val[csky_insn.idx++] = sp_idx;
      return true;
    }

  SET_ERROR_STRING (ERROR_UNDEFINE,
		    "Operand format is error. '(sp)' expected");
  return false;
}

static bool
is_reg_sp (char **oper)
{
  char sp_name[16];
  int sp_idx;
  int len;
  if (IS_CSKY_V1 (mach_flag))
    sp_idx = 0;
  else
    sp_idx = 14;

  /* ABI names: "sp". */
  if (memcmp (*oper, "sp", 2) == 0)
    {
      *oper += 2;
      csky_insn.val[csky_insn.idx++] = sp_idx;
      return true;
    }

  len = sprintf (sp_name, "r%d", sp_idx);
  if (memcmp (*oper, sp_name, len) == 0)
    {
      *oper += len;
      csky_insn.val[csky_insn.idx++] = sp_idx;
      return true;
    }

  return false;
}

static int
csky_get_freg_val (char *str, int *len)
{
  int reg = 0;
  char *s = NULL;
  if ((TOLOWER(str[0]) == 'v' || TOLOWER(str[0]) == 'f')
      && (TOLOWER(str[1]) == 'r'))
    {
      /* It is fpu register.  */
      s = &str[2];
      while (ISDIGIT (*s))
	{
	  reg = reg * 10 + (*s) - '0';
	  s++;
	}
      if (reg > 31)
	return -1;
    }
  else
    return -1;
  *len = s - str;
  return reg;
}

static bool
is_reglist_legal (char **oper)
{
  int reg1 = -1;
  int reg2 = -1;
  int len = 0;
  reg1 = csky_get_reg_val  (*oper, &len);
  *oper += len;

  if (reg1 == -1 || (IS_CSKY_V1 (mach_flag) && (reg1 == 0 || reg1 == 15)))
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT,
			"The first reg must not be r0/r15");
      return false;
    }

  if (**oper != '-')
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT,
			"The operand format must be rx-ry");
      return false;
    }
  *oper += 1;

  reg2 = csky_get_reg_val  (*oper, &len);
  *oper += len;

  if (reg2 == -1 || (IS_CSKY_V1 (mach_flag) && reg1 == 15))
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT,
			"The operand format must be r15 in C-SKY V1");
      return false;
    }
  if (IS_CSKY_V2 (mach_flag))
    {
      if (reg2 < reg1)
	{
	  SET_ERROR_STRING (ERROR_REG_FORMAT,
			    "The operand format must be rx-ry (rx < ry)");
	  return false;
	}
      reg2 = reg2 - reg1;
      reg1 <<= 5;
      reg1 |= reg2;
    }
  csky_insn.val[csky_insn.idx++] = reg1;
  return true;
}

static bool
is_freglist_legal (char **oper)
{
  int reg1 = -1;
  int reg2 = -1;
  int len = 0;
  int shift = 0;
  reg1 = csky_get_freg_val  (*oper, &len);
  *oper += len;

  if (reg1 == -1)
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT,
			"The fpu register format is not recognized.");
      return false;
    }

  if (**oper != '-')
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT,
			"The operand format must be vrx-vry/frx-fry.");
      return false;
    }
  *oper += 1;

  reg2 = csky_get_freg_val  (*oper, &len);
  *oper += len;

  if (reg2 == -1)
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT,
			"The fpu register format is not recognized.");
      return false;
    }
  if (reg2 < reg1)
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT,
			"The operand format must be rx-ry(rx < ry)");
      return false;
    }

  reg2 = reg2 - reg1;
  /* The fldm/fstm in CSKY_ISA_FLOAT_7E60 has 5 bits frz(reg1).  */
  shift = 4;
  if (startswith (csky_insn.opcode->mnemonic, "fstm")
      || startswith (csky_insn.opcode->mnemonic, "fldm"))
    {
      if ((!(isa_flag & CSKY_ISA_FLOAT_7E60)
	   && (reg2 > (int)15 || reg1 > 15))
	  || ((isa_flag & CSKY_ISA_FLOAT_7E60)
	      && (reg2 > (int)31 || reg1 > (int)31)))
	{
	  /* ISA_FLOAT_E1 fstm/fldm fry-frx is within 15.
	     ISA_FLOAT_7E60 fstm(u)/fldm(u) frx-fry is within 31.  */
	  SET_ERROR_STRING(ERROR_REG_FORMAT, (void *)"frx-fry is over range");
	  return false;
	}
      if ((mach_flag & CSKY_ARCH_MASK) == CSKY_ARCH_860)
	{
	  shift = 5;
	}
    }
  else
    {
      if (reg2 > (int)0x3) {
        SET_ERROR_STRING(ERROR_REG_FORMAT, (void *)"vry-vrx is over range");
        return false;
      }
    }
  reg2 <<= shift;
  reg1 |= reg2;
  csky_insn.val[csky_insn.idx++] = reg1;
  return true;
}

static bool
is_reglist_dash_comma_legal (char **oper, struct operand *oprnd)
{
  int reg1 = -1;
  int reg2 = -1;
  int len = 0;
  int list = 0;
  int flag = 0;
  int temp = 0;
  while (**oper != '\n' && **oper != '\0')
    {
      reg1 = csky_get_reg_val  (*oper, &len);
      if (reg1 == -1)
	{
	  SET_ERROR_STRING (ERROR_REG_LIST, NULL);
	  return false;
	}
      flag |= (1 << reg1);
      *oper += len;
      if (**oper == '-')
	{
	  *oper += 1;
	  reg2 = csky_get_reg_val  (*oper, &len);
	  if (reg2 == -1)
	    {
	      SET_ERROR_STRING (ERROR_REG_LIST, NULL);
	      return false;
	    }
	  *oper += len;
	  if (reg1 > reg2)
	    {
	      SET_ERROR_STRING (ERROR_REG_LIST, NULL);
	      return false;
	    }
	  while (reg2 >= reg1)
	    {
	      flag |= (1 << reg2);
	      reg2--;
	    }
	}
      if (**oper == ',')
	*oper += 1;
    }
  /* The reglist: r4-r11, r15, r16-r17, r28.  */
#define REGLIST_BITS         0x10038ff0
  if (flag & ~(REGLIST_BITS))
    {
      SET_ERROR_STRING (ERROR_REG_LIST, NULL);
      return false;
    }
  /* Check r4-r11.  */
  int i = 4;
  while (i <= 11)
    {
      if (flag & (1 << i))
	temp = i - 4 + 1;
      i++;
    }
  list |= temp;

  /* Check r15.  */
  if (flag & (1 << 15))
    list |= (1 << 4);

  /* Check r16-r17.  */
  i = 16;
  temp = 0;
  while (i <= 17)
    {
      if (flag & (1 << i))
	temp = i - 16 + 1;
      i++;
    }
  list |= (temp << 5);

  /* Check r28.  */
  if (flag & (1 << 28))
    list |= (1 << 8);
  if (oprnd->mask == OPRND_MASK_0_4 && (list & ~OPRND_MASK_0_4))
    {
      SET_ERROR_STRING (ERROR_REG_LIST, NULL);
      return false;
    }
  csky_insn.val[csky_insn.idx++] = list;
  return true;
}

static bool
is_reg_lshift_illegal (char **oper, int is_float)
{
  int value;
  int len;
  int reg;
  reg = csky_get_reg_val  (*oper, &len);
  if (reg == -1)
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT, "The register must be r0-r31.");
      return false;
    }

  *oper += len;
  if ((*oper)[0] != '<' || (*oper)[1] != '<')
    {
      SET_ERROR_STRING (ERROR_UNDEFINE,
			"Operand format error; should be (rx, ry << n)");
      return false;
    }
  *oper += 2;

  expressionS e;
  char *new_oper = parse_exp (*oper, &e);
  if (e.X_op == O_constant)
    {
      *oper = new_oper;
      /* The immediate must be in [0, 3].  */
      if (e.X_add_number < 0 || e.X_add_number > 3)
	{
	  SET_ERROR_STRING (ERROR_IMM_OVERFLOW, NULL);
	  return false;
	}
    }
  else
    {
      SET_ERROR_STRING (ERROR_EXP_CONSTANT, NULL);
      return false;
    }
  if (is_float)
    value = (reg << 2) | e.X_add_number;
  else
    value = (reg << 5) | (1 << e.X_add_number);
  csky_insn.val[csky_insn.idx++] = value;

  return true;
}

static bool
is_imm_within_range (char **oper, int min, int max)
{
  expressionS e;
  bool ret = false;
  char *new_oper = parse_exp (*oper, &e);
  if (e.X_op == O_constant)
    {
      ret = true;
      *oper = new_oper;
      if (e.X_add_number < min || e.X_add_number > max)
	{
	  ret = false;
	  SET_ERROR_STRING (ERROR_IMM_OVERFLOW, NULL);
	}
      if (!e.X_unsigned)
	e.X_add_number |= 0x80000000;
      csky_insn.val[csky_insn.idx++] = e.X_add_number;
    }
  else
    SET_ERROR_STRING(ERROR_IMM_ILLEGAL, NULL);

  return ret;
}

static bool
is_imm_within_range_ext (char **oper, int min, int max, int ext)
{
  expressionS e;
  bool ret = false;
  char *new_oper = parse_exp (*oper, &e);
  if (e.X_op == O_constant)
    {
      ret = true;
      *oper = new_oper;
      if ((int)e.X_add_number != ext
	  && (e.X_add_number < min || e.X_add_number > max))
	{
	  ret = false;
	  SET_ERROR_STRING (ERROR_IMM_OVERFLOW, NULL);
	}
      csky_insn.val[csky_insn.idx++] = e.X_add_number;
    }

  else
    SET_ERROR_STRING(ERROR_IMM_ILLEGAL, NULL);

  return ret;
}

static bool
is_oimm_within_range (char **oper, int min, int max)
{
  expressionS e;
  bool ret = false;
  char *new_oper = parse_exp (*oper, &e);
  if (e.X_op == O_constant)
    {
      ret = true;
      *oper = new_oper;
      if (e.X_add_number < min || e.X_add_number > max)
	{
	  ret = false;
	  SET_ERROR_STRING (ERROR_IMM_OVERFLOW, NULL);
	}
      csky_insn.val[csky_insn.idx++] = e.X_add_number - 1;
    }
  else
    SET_ERROR_STRING (ERROR_IMM_ILLEGAL, NULL);

  return ret;
}

static bool
is_psr_bit (char **oper)
{
  const struct psrbit *bits;
  int i = 0;

  if (IS_CSKY_V1 (mach_flag))
    bits = cskyv1_psr_bits;
  else
    bits = cskyv2_psr_bits;

  while (bits[i].name != NULL)
    {
      if (bits[i].isa && !(bits[i].isa & isa_flag))
	{
	  i++;
	  continue;
	}
      if (strncasecmp (*oper, bits[i].name, strlen (bits[i].name)) == 0)
	{
	  *oper += strlen (bits[i].name);
	  csky_insn.val[csky_insn.idx] |= bits[i].value;
	  return true;
	}
      i++;
    }
  SET_ERROR_STRING (ERROR_OPCODE_PSRBIT, NULL);
  return false;
}

static bool
parse_type_cpidx (char** oper)
{
  char *s = *oper;
  int idx;
  if (s[0] == 'c' && s[1] == 'p')
    {
      if (ISDIGIT (s[2]) && ISDIGIT (s[3]) && ! ISDIGIT (s[4]))
	{
	  idx = (s[2] - '0') * 10 + s[3] - '0';
	  *oper += 4;
	}
      else if (ISDIGIT (s[2]) && !ISDIGIT (s[3]))
	{
	  idx = s[2] - '0';
	  *oper += 3;
	}
      else
	return false;
    }
  else
    {
      expressionS e;
      *oper = parse_exp (*oper, &e);
      if (e.X_op != O_constant)
	{
	  /* Can not recognize the operand.  */
	  return false;
	}
      idx = e.X_add_number;
    }

  csky_insn.val[csky_insn.idx++] = idx;

  return true;
}

static bool
parse_type_cpreg (char** oper)
{
  expressionS e;

  if (strncasecmp (*oper, "cpr", 3) != 0)
    {
      SET_ERROR_STRING(ERROR_CPREG_ILLEGAL, *oper);
      return false;
    }

  *oper += 3;

  *oper = parse_exp (*oper, &e);
  if (e.X_op != O_constant)
    {
      SET_ERROR_STRING(ERROR_CPREG_ILLEGAL, *oper);
      return false;
    }

  csky_insn.val[csky_insn.idx++] = e.X_add_number;

  return true;
}

static bool
parse_type_cpcreg (char** oper)
{
  expressionS e;

  if (strncasecmp (*oper, "cpcr", 4) != 0)
    {
      SET_ERROR_STRING(ERROR_CPREG_ILLEGAL, *oper);
      return false;
    }

  *oper += 4;

  *oper = parse_exp (*oper, &e);
  if (e.X_op != O_constant)
    {
      SET_ERROR_STRING(ERROR_CPREG_ILLEGAL, *oper);
      return false;
    }

  csky_insn.val[csky_insn.idx++] = e.X_add_number;

  return true;
}

static bool
parse_type_areg (char** oper)
{
  int i = 0;
  int len = 0;
  i = csky_get_reg_val (*oper, &len);
  if (i == -1)
    {
      SET_ERROR_STRING (ERROR_GREG_ILLEGAL, NULL);
      return false;
    }
  *oper += len;
  csky_insn.val[csky_insn.idx++] = i;

  return true;
}

static bool
parse_type_freg (char** oper, int even)
{
  int reg;
  int len;
  reg = csky_get_freg_val (*oper, &len);
  if (reg == -1)
    {
      SET_ERROR_STRING (ERROR_REG_FORMAT,
			(void *)"The fpu register format is not recognized.");
      return false;
    }
  *oper += len;
  csky_insn.opcode_end = *oper;
  if (even && reg & 0x1)
    {
      SET_ERROR_STRING (ERROR_EXP_EVEN_FREG, NULL);
      return false;
    }

  if (IS_CSKY_V2 (mach_flag)
      && ((csky_insn.opcode->isa_flag32 & CSKY_ISA_VDSP_2)
	  || !(csky_insn.opcode->isa_flag32 & CSKY_ISA_FLOAT_7E60))
      && reg > 15)
    {
      if ((csky_insn.opcode->isa_flag32 & CSKY_ISA_VDSP_2))
	{
	  SET_ERROR_INTEGER (ERROR_VREG_OVER_RANGE, reg);
	}
      else
	{
	  SET_ERROR_INTEGER (ERROR_FREG_OVER_RANGE, reg);
	}
      return false;
    }
  /* TODO: recognize vreg or freg.  */
  if (reg > 31)
    {
      SET_ERROR_INTEGER (ERROR_VREG_OVER_RANGE, reg);
    }
  csky_insn.val[csky_insn.idx++] = reg;
  return true;
}

static bool
parse_ldst_imm (char **oper, struct csky_opcode_info *op ATTRIBUTE_UNUSED,
		struct operand *oprnd)
{
  unsigned int mask = oprnd->mask;
  int max = 1;
  int shift = 0;

  shift = oprnd->shift;

  while (mask)
    {
      if (mask & 1)
	max <<= 1;
      mask >>= 1;
    }
  max = max << shift;

  if (**oper == '\0' || **oper == ')')
    {
      csky_insn.val[csky_insn.idx++] = 0;
      return true;
    }

  expressionS e;
  *oper = parse_exp (*oper, &e);
  if (e.X_op != O_constant)
    {
    /* Not a constant.  */
      SET_ERROR_STRING(ERROR_UNDEFINE, (void *)"Operand format is error. eg. \"ld rz, (rx, n)\"");
    return false;
    }
  else if (e.X_add_number < 0 || e.X_add_number >= max)
    {
      /* Out of range.  */
      SET_ERROR_STRING(ERROR_IMM_OVERFLOW, NULL);
      return false;
    }
  if ((e.X_add_number % (1 << shift)) != 0)
    {
      /* Not aligned.  */
      SET_ERROR_INTEGER (ERROR_OFFSET_UNALIGNED, ((unsigned long)1 << shift));
      return false;
    }

  csky_insn.val[csky_insn.idx++] = e.X_add_number >> shift;

  return true;

}

static unsigned int
csky_count_operands (char *str)
{
  char *oper_end = str;
  unsigned int oprnd_num;
  int bracket_cnt = 0;

  if (is_end_of_line[(unsigned char) *oper_end])
    oprnd_num = 0;
  else
    oprnd_num = 1;

  /* Count how many operands.  */
  if (oprnd_num)
    while (!is_end_of_line[(unsigned char) *oper_end])
      {
	if (*oper_end == '(' || *oper_end == '<')
	  {
	    bracket_cnt++;
	    oper_end++;
	    continue;
	  }
	if (*oper_end == ')' || *oper_end == '>')
	  {
	    bracket_cnt--;
	    oper_end++;
	    continue;
	  }
	if (!bracket_cnt && *oper_end == ',')
	  oprnd_num++;
	oper_end++;
      }
  return oprnd_num;
}

/* End of the operand parsing helper functions.  */

/* Parse the opcode part of an instruction.  Fill in the csky_insn
   state and return true on success, false otherwise.  */

static bool
parse_opcode (char *str)
{
#define IS_OPCODE32F(a) (*(a - 2) == '3' && *(a - 1) == '2')
#define IS_OPCODE16F(a) (*(a - 2) == '1' && *(a - 1) == '6')

  /* TRUE if this opcode has a suffix, like 'lrw.h'.  */
  unsigned int has_suffix = false;
  unsigned int nlen = 0;
  char *opcode_end;
  char name[OPCODE_MAX_LEN + 1];
  char macro_name[OPCODE_MAX_LEN + 1];

  /* Remove space ahead of string.  */
  while (ISSPACE (*str))
    str++;
  opcode_end = str;

  /* Find the opcode end.  */
  while (nlen < OPCODE_MAX_LEN
	 && !is_end_of_line [(unsigned char) *opcode_end]
	 && *opcode_end != ' ')
    {
      /* Is csky force 32 or 16 instruction?  */
      if (IS_CSKY_V2 (mach_flag)
	  && *opcode_end == '.' && !has_suffix)
	{
	  has_suffix = true;
	  if (IS_OPCODE32F (opcode_end))
	    {
	      csky_insn.flag_force = INSN_OPCODE32F;
	      nlen -= 2;
	    }
	  else if (IS_OPCODE16F (opcode_end))
	    {
	      csky_insn.flag_force = INSN_OPCODE16F;
	      nlen -= 2;
	    }
	}
      name[nlen] = *opcode_end;
      nlen++;
      opcode_end++;
    }

  /* Is csky force 32 or 16 instruction?  */
  if (!has_suffix)
    {
      if (IS_CSKY_V2 (mach_flag) && IS_OPCODE32F (opcode_end))
	{
	  csky_insn.flag_force = INSN_OPCODE32F;
	  nlen -= 2;
	}
      else if (IS_OPCODE16F (opcode_end))
	{
	  csky_insn.flag_force = INSN_OPCODE16F;
	  nlen -= 2;
	}
    }
  name[nlen] = '\0';

  /* Generate macro_name for finding hash in macro hash_table.  */
  if (has_suffix)
    nlen += 2;
  strncpy (macro_name, str, nlen);
  macro_name[nlen] = '\0';

  /* Get csky_insn.opcode_end.  */
  while (ISSPACE (*opcode_end))
    opcode_end++;
  csky_insn.opcode_end = opcode_end;

  /* Count the operands.  */
  csky_insn.number = csky_count_operands (opcode_end);

  /* Find hash by name in csky_macros_hash and csky_opcodes_hash.  */
  csky_insn.macro = (struct csky_macro_info *) str_hash_find (csky_macros_hash,
							      macro_name);
  csky_insn.opcode = (struct csky_opcode *) str_hash_find (csky_opcodes_hash,
							   name);

  if (csky_insn.macro == NULL && csky_insn.opcode == NULL)
    return false;
  return true;
}

/* Main dispatch routine to parse operand OPRND for opcode OP from string
   *OPER.  */

static bool
get_operand_value (struct csky_opcode_info *op,
		   char **oper, struct operand *oprnd)
{
  struct soperand *soprnd = NULL;
  if (oprnd->mask == HAS_SUB_OPERAND)
    {
      /* It has sub operand, it must be like:
	 (oprnd1, oprnd2)
	 or
	 <oprnd1, oprnd2>
	 We will check the format here.  */
      soprnd = (struct soperand *) oprnd;
      char lc = 0;
      char rc = 0;
      char *s = *oper;
      int  bracket_cnt = 0;
      if (oprnd->type == OPRND_TYPE_BRACKET)
	{
	  lc = '(';
	  rc = ')';
	}
      else if (oprnd->type == OPRND_TYPE_ABRACKET)
	{
	  lc = '<';
	  rc = '>';
	}

      if (**oper == lc)
	{
	  *oper += 1;
	  s += 1;
	}
      else
	{
	  SET_ERROR_STRING ((oprnd->type == OPRND_TYPE_BRACKET
			     ? ERROR_MISSING_LBRACKET
			     : ERROR_MISSING_LANGLE_BRACKETS), NULL);
	  return false;
	}

      /* If the oprnd2 is an immediate, it can not be parsed
	 that end with ')'/'>'.  Modify ')'/'>' to '\0'.  */
      while ((*s != rc || bracket_cnt != 0) && (*s != '\n' && *s != '\0'))
	{
	  if (*s == lc)
	    bracket_cnt++;
	  else if (*s == rc)
	    bracket_cnt--;
	  s++;
	}

      if (*s == rc)
	*s = '\0';
      else
	{
	  SET_ERROR_STRING ((oprnd->type == OPRND_TYPE_BRACKET
			     ? ERROR_MISSING_RBRACKET
			     : ERROR_MISSING_RANGLE_BRACKETS), NULL);
	  return false;
	}

      if (!get_operand_value (op, oper, &soprnd->subs[0]))
	{
	  *s = rc;
	  return false;
	}
      if (**oper == ',')
	*oper += 1;
      else if (**oper != '\0')
	{
	  SET_ERROR_STRING (ERROR_MISSING_COMMA, NULL);
	  return false;
	}

      if (!get_operand_value (op, oper, &soprnd->subs[1]))
	{
	  *s = rc;
	  return false;
	}

      *s = rc;
      *oper += 1;
      return true;
    }

  switch (oprnd->type)
    {
      /* TODO: add opcode type here, log errors in the function.
	 If REGLIST, then j = csky_insn.number - 1.
	 If there is needed to parse expressions, it will be
	 handled here.  */
    case OPRND_TYPE_CTRLREG:
      /* some parse.  */
      return parse_type_ctrlreg (oper);
    case OPRND_TYPE_AREG:
      return parse_type_areg (oper);
    case OPRND_TYPE_FREG:
    case OPRND_TYPE_VREG:
      return parse_type_freg (oper, 0);
    case OPRND_TYPE_FEREG:
      return parse_type_freg (oper, 1);
    case OPRND_TYPE_CPCREG:
      return parse_type_cpcreg (oper);
    case OPRND_TYPE_CPREG:
      return parse_type_cpreg (oper);
    case OPRND_TYPE_CPIDX:
      return parse_type_cpidx (oper);
    case OPRND_TYPE_GREG0_7:
    case OPRND_TYPE_GREG0_15:
      {
	int len;
	long reg;
	reg = csky_get_reg_val (*oper, &len);

	if (reg == -1)
	  {
	    SET_ERROR_STRING (ERROR_GREG_ILLEGAL, NULL);
	    return false;
	  }
	else if ((oprnd->type == OPRND_TYPE_GREG0_7 && reg > 7)
		 || (oprnd->type == OPRND_TYPE_GREG0_15 && reg > 15))
	  {
	    SET_ERROR_INTEGER (ERROR_REG_OVER_RANGE, reg);
	    return false;
	  }
	*oper += len;
	csky_insn.val[csky_insn.idx++] = reg;
	return true;
      }
    case OPRND_TYPE_REGnsplr:
      {
	int len;
	long reg;
	reg = csky_get_reg_val (*oper, &len);

	if (reg == -1
	    || (IS_CSKY_V1 (mach_flag)
		&& (reg == V1_REG_SP || reg == V1_REG_LR)))
	  {
	    SET_ERROR_STRING (ERROR_REG_OVER_RANGE, reg);
	    return false;
	  }
	csky_insn.val[csky_insn.idx++] = reg;
	*oper += len;
	return true;;
      }
    case OPRND_TYPE_REGnr4_r7:
      {
	int len;
	int reg;
	if (**oper == '(')
	  *oper += 1;
	reg = csky_get_reg_val (*oper, &len);
	if (reg == -1 || (reg <= 7 && reg >= 4))
	  return false;

	csky_insn.val[csky_insn.idx++] = reg;
	*oper += len;

	if (**oper == ')')
	  *oper += 1;
	return true;;
      }
    case OPRND_TYPE_REGr4_r7:
      if (memcmp (*oper, "r4-r7", sizeof ("r4-r7") - 1) == 0)
	{
	  *oper += sizeof ("r4-r7") - 1;
	  csky_insn.val[csky_insn.idx++] = 0;
	  return true;
	}
      SET_ERROR_STRING (ERROR_OPCODE_ILLEGAL, NULL);
      return false;
    case OPRND_TYPE_IMM_LDST:
      return parse_ldst_imm (oper, op, oprnd);
    case OPRND_TYPE_IMM_FLDST:
      return parse_ldst_imm (oper, op, oprnd);
    case OPRND_TYPE_IMM1b:
      return is_imm_within_range (oper, 0, 1);
    case OPRND_TYPE_IMM2b:
      return is_imm_within_range (oper, 0, 3);
    case OPRND_TYPE_IMM2b_JMPIX:
      /* ck802j support jmpix16, but not support jmpix32.  */
      if (IS_CSKY_ARCH_802 (mach_flag)
	  && (op->opcode & 0xffff0000) != 0)
	{
	  SET_ERROR_STRING (ERROR_OPCODE_ILLEGAL, NULL);
	  return false;
	}
      *oper = parse_exp (*oper, &csky_insn.e1);
      if (csky_insn.e1.X_op == O_constant)
	{
	  csky_insn.opcode_end = *oper;
	  if (csky_insn.e1.X_add_number & 0x7)
	    {
	      SET_ERROR_STRING (ERROR_JMPIX_OVER_RANGE, NULL);
	      return false;
	    }
	  csky_insn.val[csky_insn.idx++]
	    = (csky_insn.e1.X_add_number >> 3) - 2;
	}
      return true;
    case OPRND_TYPE_IMM4b:
      return is_imm_within_range (oper, 0, 15);
    case OPRND_TYPE_IMM5b:
      return is_imm_within_range (oper, 0, 31);
      /* This type for "bgeni" in csky v1 ISA.  */
    case OPRND_TYPE_IMM5b_7_31:
      if (is_imm_within_range (oper, 0, 31))
	{
	  int val = csky_insn.val[csky_insn.idx - 1];
	  /* immediate values of 0 -> 6 translate to movi.  */
	  if (val <= 6)
	    {
	      const char *name = "movi";
	      csky_insn.opcode = (struct csky_opcode *)
		str_hash_find (csky_opcodes_hash, name);
	      csky_insn.val[csky_insn.idx - 1] = 1 << val;
	    }
	  return true;
	}
      else
	return false;

    case OPRND_TYPE_IMM5b_1_31:
      return is_imm_within_range (oper, 1, 31);
    case OPRND_TYPE_IMM5b_POWER:
      if (is_imm_within_range_ext (oper, 1, (1u << 31) - 1, 1u << 31))
	{
	  int log;
	  int val = csky_insn.val[csky_insn.idx - 1];
	  log = csky_log_2 (val);
	  csky_insn.val[csky_insn.idx - 1] = log;
	  return log != -1;
	}
      else
	return false;

      /* This type for "mgeni" in csky v1 ISA.  */
      case OPRND_TYPE_IMM5b_7_31_POWER:
	if (is_imm_within_range_ext (oper, 1, (1u << 31) - 1, 1u << 31))
	  {
	    int log;
	    int val = csky_insn.val[csky_insn.idx - 1];
	    log = csky_log_2 (val);
	    /* Immediate values of 0 -> 6 translate to movi.  */
	    if (log <= 6)
	      {
		const char *name = "movi";
		csky_insn.opcode = (struct csky_opcode *)
		  str_hash_find (csky_opcodes_hash, name);
		as_warn (_("translating mgeni to movi"));
	      }
	    else
	      csky_insn.val[csky_insn.idx - 1] = log;
	    return log != -1;
	  }
	else
	  return false;

    case OPRND_TYPE_IMM5b_LS:
      return is_imm_within_range (oper,
				  0,
				  csky_insn.val[csky_insn.idx - 1]);
    case OPRND_TYPE_IMM5b_RORI:
      {
	unsigned max_shift = IS_CSKY_V1 (mach_flag) ? 31 : 32;

	if (is_imm_within_range (oper, 1, max_shift))
	  {
	    int i = csky_insn.idx - 1;
	    csky_insn.val[i] = 32 - csky_insn.val[i];
	    return true;
	  }
	else
	  return false;
      }

    case OPRND_TYPE_IMM5b_BMASKI:
      /* For csky v1 bmask inst.  */

      if (!is_imm_within_range_ext (oper, 8, 31, 0))
	{
	  unsigned int mask_val = csky_insn.val[csky_insn.idx - 1];
	  if (mask_val > 0 && mask_val < 8)
	    {
	      const char *op_movi = "movi";
	      csky_insn.opcode = (struct csky_opcode *)
		str_hash_find (csky_opcodes_hash, op_movi);
	      if (csky_insn.opcode == NULL)
		return false;
	      csky_insn.val[csky_insn.idx - 1] = (1 << mask_val) - 1;
	      return true;
	    }
	}
      return true;

    case OPRND_TYPE_IMM5b_VSH:
    /* For vshri.T and vshli.T.  */
      if (is_imm_within_range (oper, 0, 31))
	{
	  int val = csky_insn.val[csky_insn.idx - 1];
	  val =  (val << 1) | (val >> 4);
	  val &= 0x1f;
	  csky_insn.val[csky_insn.idx - 1] = val;
	  return true;
	}
      return false;
      case OPRND_TYPE_IMM8b_BMASKI:
      /* For csky v2 bmask, which will transfer to 16bits movi.  */
	if (is_imm_within_range (oper, 1, 8))
	  {
	    unsigned int mask_val = csky_insn.val[csky_insn.idx - 1];
	    csky_insn.val[csky_insn.idx - 1] = (1 << mask_val) - 1;
	    return true;
	  }
	return false;
    case OPRND_TYPE_OIMM4b:
      return is_oimm_within_range (oper, 1, 16);
    case OPRND_TYPE_OIMM5b:
      return is_oimm_within_range (oper, 1, 32);
    case OPRND_TYPE_OIMM5b_IDLY:
      if (is_imm_within_range (oper, 0, 32))
	{
	  /* imm5b for idly n: 0<=n<4, imm5b=3; 4<=n<=32, imm5b=n-1.  */
	  unsigned long imm = csky_insn.val[csky_insn.idx - 1];
	  if (imm < 4)
	    {
	      csky_show_error (WARNING_IDLY, 0, (void *)imm, NULL);
	      imm = 3;
	    }
	  else imm--;
	  csky_insn.val[csky_insn.idx - 1] = imm;
	  return true;
	}
      else
	return false;

      /* For csky v2 bmask inst.  */
    case OPRND_TYPE_OIMM5b_BMASKI:
      if (!is_oimm_within_range (oper, 17, 32))
	{
	  int mask_val = csky_insn.val[csky_insn.idx - 1];
	  if (mask_val + 1 == 0)
	    return true;
	  if (mask_val > 0 && mask_val < 16)
	    {
	      const char *op_movi = "movi";
	      csky_insn.opcode = (struct csky_opcode *)
		str_hash_find (csky_opcodes_hash, op_movi);
	      if (csky_insn.opcode == NULL)
		return false;
	      csky_insn.val[csky_insn.idx - 1] = (1 << (mask_val + 1)) - 1;
	      return true;
	    }
	}
      return true;
    case OPRND_TYPE_IMM7b:
      return is_imm_within_range (oper, 0, 127);
    case OPRND_TYPE_IMM8b:
      return is_imm_within_range (oper, 0, 255);
    case OPRND_TYPE_IMM9b:
      return is_imm_within_range (oper, -256, 255);
    case OPRND_TYPE_IMM12b:
      return is_imm_within_range (oper, 0, 4095);
    case OPRND_TYPE_IMM15b:
      return is_imm_within_range (oper, 0, 0xfffff);
    case OPRND_TYPE_IMM16b:
      return is_imm_within_range (oper, 0, 65535);
    case OPRND_TYPE_OIMM16b:
      return is_oimm_within_range (oper, 1, 65536);
    case OPRND_TYPE_IMM32b:
      {
	expressionS e;
	char *new_oper = parse_exp (*oper, &e);
	if (e.X_op == O_constant)
	  {
	    *oper = new_oper;
	    csky_insn.val[csky_insn.idx++] = e.X_add_number;
	    return true;
	  }
	return false;
      }
    case OPRND_TYPE_IMM16b_MOVIH:
    case OPRND_TYPE_IMM16b_ORI:
      {
	bfd_reloc_code_real_type r = BFD_RELOC_NONE;
	int len;
	char *curr = *oper;
	char * save = input_line_pointer;
	/* get the reloc type, and set "@GOTxxx" as ' '  */
	while (**oper != '@' && **oper != '\0')
	  *oper += 1;
	if (**oper != '\0')
	  {
	    input_line_pointer = *oper;
	    lex_got (&r, &len);
	    while (*(*oper + len + 1) != '\0')
	      {
		**oper = *(*oper + len + 1);
		*(*oper + len + 1) = '\0';
		*oper += 1;
	      }
	    **oper = '\0';
	  }
	input_line_pointer = save;
	*oper = parse_exp (curr, &csky_insn.e1);
	return true;
      }
    case OPRND_TYPE_PSR_BITS_LIST:
      {
	int ret = true;
	if (csky_insn.number == 0)
	  ret = false;
	else
	  {
	    csky_insn.val[csky_insn.idx] = 0;
	    if (is_psr_bit (oper))
	      while (**oper == ',')
		{
		  *oper += 1;
		  if (!is_psr_bit (oper))
		    {
		      ret = false;
		      break;
		    }
		}
	    else
	      ret = false;
	    if (ret && IS_CSKY_V1 (mach_flag)
		&& csky_insn.val[csky_insn.idx] > 8)
	      ret = false;
	  }
	if (!ret)
	  SET_ERROR_STRING (ERROR_OPERANDS_ILLEGAL, csky_insn.opcode_end);
	return ret;
      }
    case OPRND_TYPE_RM:
      {
	/* FPU round mode.  */
	static const char *round_mode[] =
	  {
	    "rm_nearest",
	    "rm_zero",
	    "rm_posinf",
	    "rm_neginf",
	    NULL
	  };
	int i;
	for (i = 0; round_mode[i]; i++)
	  if (strncasecmp (*oper, round_mode[i], strlen (round_mode[i])) == 0)
	    {
	      *oper += strlen (round_mode[i]);
	      csky_insn.val[csky_insn.idx++] = i;
	      return true;
	    }
	return false;
      }

    case OPRND_TYPE_REGLIST_COMMA:
    case OPRND_TYPE_BRACKET:
      /* TODO: using sub operand union.  */
    case OPRND_TYPE_ABRACKET:
      /* TODO: using sub operand union.  */
    case OPRND_TYPE_REGLIST_DASH:
      return is_reglist_legal (oper);
    case OPRND_TYPE_FREGLIST_DASH:
      return is_freglist_legal (oper);
    case OPRND_TYPE_AREG_WITH_BRACKET:
      {
	int len;
	int reg;
	if (**oper != '(')
	  {
	    SET_ERROR_STRING (ERROR_MISSING_LBRACKET, NULL);
	    return false;
	  }
	*oper += 1;
	reg = csky_get_reg_val (*oper, &len);
	if (reg == -1)
	  {
	    SET_ERROR_STRING (ERROR_EXP_GREG, NULL);
	    return false;
	  }
	*oper += len;
	if (**oper != ')')
	  {
	    SET_ERROR_STRING (ERROR_MISSING_RBRACKET, NULL);
	    return false;
	  }
	*oper += 1;
	csky_insn.val[csky_insn.idx++] = reg;
	return true;
      }
    case OPRND_TYPE_REGsp:
      return is_reg_sp (oper);
    case OPRND_TYPE_REGbsp:
      return is_reg_sp_with_bracket (oper);
      /* For jmpi.  */
    case OPRND_TYPE_OFF8b:
    case OPRND_TYPE_OFF16b:
      *oper = parse_rt (*oper, 1, &csky_insn.e1, -1);
      csky_insn.val[csky_insn.idx++] = 0;
      return true;
    case OPRND_TYPE_LABEL_WITH_BRACKET:
    case OPRND_TYPE_CONSTANT:
    case OPRND_TYPE_ELRW_CONSTANT:
      if (**oper == '[')
	csky_insn.val[csky_insn.idx++] = 0;
      else
	csky_insn.val[csky_insn.idx++] = NEED_OUTPUT_LITERAL;
      *oper = parse_rt (*oper, 0, &csky_insn.e1, -1);
      return true;
    case OPRND_TYPE_FCONSTANT:
      *oper = parse_rtf (*oper, 0, &csky_insn.e1);
      return true;

    case OPRND_TYPE_SFLOAT:
    case OPRND_TYPE_DFLOAT:
      /* For fmovis and fmovid, which accept a constant float with
	 a limited range.  */
      {
	uint64_t dbnum;
	int imm4, imm8;

	*oper = parse_fexp (*oper, &csky_insn.e1, 1, &dbnum);
	if (csky_insn.e1.X_op == O_absent)
	  return false;

	/* Convert the representation from IEEE double to the 13-bit
	   encoding used internally for fmovis and fmovid.  */
	imm4 = 11 - (((dbnum & 0x7ff0000000000000ULL) >> 52) - 1023);
	/* Check float range.  */
	if ((dbnum & 0x00000fffffffffffULL) || imm4 < 0 || imm4 > 15)
	  {
	    csky_show_error (ERROR_IMM_OVERFLOW, 2, NULL, NULL);
	    return false;
	  }
	imm8 = (dbnum & 0x000ff00000000000ULL) >> 44;
	csky_insn.e1.X_add_number
	  = (((imm8 & 0xf) << 4)
	     | ((imm8 & 0xf0) << 17)
	     | ((imm4 & 0xf) << 16)
	     | ((dbnum & 0x8000000000000000ULL) >> 43));
	return true;
      }
    case OPRND_TYPE_HFLOAT_FMOVI:
    case OPRND_TYPE_SFLOAT_FMOVI:
    case OPRND_TYPE_DFLOAT_FMOVI:
      /* For fpuv3 fmovis and fmovid, which accept a constant
	 float with a limited range.  */
      {
	uint64_t dbnum;
	int imm4, imm8, sign;

	*oper = parse_fexp (*oper, &csky_insn.e1, 1, &dbnum);
	if (csky_insn.e1.X_op == O_absent)
	  return false;

	/* Convert the representation from IEEE double to the 13-bit
	   encoding used internally for fmovis and fmovid.  */
	imm4 = 11 - (((dbnum & 0x7ff0000000000000ULL) >> 52) - 1023);
	/* Check float range.  */
	if ((dbnum & 0x00000fffffffffffULL) || imm4 < 0 || imm4 > 15)
	  {
	    csky_show_error (ERROR_IMM_OVERFLOW, 2, NULL, NULL);
	    return true;
	  }
	imm8 = (dbnum & 0x000ff00000000000ULL) >> 44;
	sign = (dbnum & 0x8000000000000000ULL) >> 58;
	csky_insn.e1.X_add_number
	  = (((imm8 & 0x3) << 8)
	     | ((imm8 & 0xfc) << 18)
	     | ((imm4 & 0xf) << 16)
	     | sign);
	return true;
      }
      /* For grs v2.  */
    case OPRND_TYPE_IMM_OFF18b:
      *oper = parse_exp (*oper, &csky_insn.e1);
      return true;

    case OPRND_TYPE_BLOOP_OFF4b:
      *oper = parse_exp (*oper, &csky_insn.e2);
      if (csky_insn.e2.X_op == O_symbol)
	{
	  csky_insn.opcode_end = *oper;
	  return true;
	}
      else
	return false;

    case OPRND_TYPE_BLOOP_OFF12b:
    case OPRND_TYPE_OFF10b:
    case OPRND_TYPE_OFF11b:
    case OPRND_TYPE_OFF16b_LSL1:
    case OPRND_TYPE_OFF26b:
      *oper = parse_exp (*oper, &csky_insn.e1);
      if (csky_insn.e1.X_op == O_symbol)
	{
	  csky_insn.opcode_end = *oper;
	  return true;
	}
      else
	return false;
      /* For xtrb0(1)(2)(3) and div in csky v1 ISA.  */
    case OPRND_TYPE_REG_r1a:
      {
	int reg = 0;
	int len = 0;
	reg = csky_get_reg_val (*oper, &len);
	if (reg == -1)
	  {
	    SET_ERROR_STRING (ERROR_REG_FORMAT,
			      "The first operand must be register r1.");
	    return false;
	  }
	if (reg != 1)
	  mov_r1_after = true;
	*oper += len;
	csky_insn.opcode_end = *oper;
	csky_insn.val[csky_insn.idx++] = reg;
	return true;
      }
    case OPRND_TYPE_REG_r1b:
      {
	int reg = 0;
	int len = 0;
	reg = csky_get_reg_val (*oper, &len);
	if (reg == -1)
	  {
	    SET_ERROR_STRING (ERROR_REG_FORMAT,
			      "The second operand must be register r1.");
	    return false;
	  }
	if (reg != 1)
	  {
	    unsigned int mov_insn = CSKYV1_INST_MOV_R1_RX;
	    mov_insn |= reg << 4;
	    mov_r1_before = true;
	    csky_insn.output = frag_more (2);
	    dwarf2_emit_insn (0);
	    md_number_to_chars (csky_insn.output, mov_insn, 2);
	  }
	*oper += len;
	csky_insn.opcode_end = *oper;
	csky_insn.val[csky_insn.idx++] = reg;
	return true;
      }
    case OPRND_TYPE_DUMMY_REG:
      {
	int reg = 0;
	int len = 0;
	reg = csky_get_reg_val (*oper, &len);
	if (reg == -1)
	  {
	    SET_ERROR_STRING (ERROR_GREG_ILLEGAL, NULL);
	    return false;
	  }
	if (reg != csky_insn.val[0])
	  {
	    SET_ERROR_STRING (ERROR_REG_FORMAT,
			      "The second register must be the same as the first.");
	    return false;
	  }
	*oper += len;
	csky_insn.opcode_end = *oper;
	csky_insn.val[csky_insn.idx++] = reg;
	return true;
      }
    case OPRND_TYPE_2IN1_DUMMY:
      {
	int reg = 0;
	int len = 0;
	int max = 0;
	int min = 0;
	reg = csky_get_reg_val (*oper, &len);
	if (reg == -1)
	  {
	    SET_ERROR_STRING (ERROR_GREG_ILLEGAL, NULL);
	    return false;
	  }
	/* dummy reg's real type should be same with first operand.  */
	if (op->oprnd.oprnds[0].type == OPRND_TYPE_GREG0_15)
	  max = 15;
	else if (op->oprnd.oprnds[0].type == OPRND_TYPE_GREG0_7)
	  max = 7;
	else
	  return false;
	if (reg < min || reg > max)
	  return false;
	csky_insn.val[csky_insn.idx++] = reg;
	/* if it is the last operands.  */
	if (csky_insn.idx > 2)
	  {
	    /* For "insn rz, rx, ry", if rx or ry is equal to rz,
	       we can output the insn like "insn rz, rx".  */
	    if (csky_insn.val[0] ==  csky_insn.val[1])
	      csky_insn.val[1] = 0;
	    else if (csky_insn.val[0] ==  csky_insn.val[2])
	      csky_insn.val[2] = 0;
	    else
	      return false;
	  }
	*oper += len;
	csky_insn.opcode_end = *oper;
	return true;
      }
    case OPRND_TYPE_DUP_GREG0_7:
    case OPRND_TYPE_DUP_GREG0_15:
    case OPRND_TYPE_DUP_AREG:
      {
	long reg = 0;
	int len = 0;
	long max_reg;
	unsigned int shift_num;
	if (oprnd->type == OPRND_TYPE_DUP_GREG0_7)
	  {
	    max_reg = 7;
	    shift_num = 3;
	  }
	else if (oprnd->type == OPRND_TYPE_DUP_GREG0_15)
	  {
	    max_reg = 15;
	    shift_num = 4;
	  }
	else
	  {
	    max_reg = 31;
	    shift_num = 5;
	  }
	reg = csky_get_reg_val (*oper, &len);
	if (reg == -1)
	  {
	    if (max_reg == 31)
	      SET_ERROR_STRING (ERROR_REG_FORMAT,
				"The register must be r0-r31");
	    else
	      SET_ERROR_STRING (ERROR_REG_FORMAT,
				"The register must be r0-r15");
	    return false;
	  }
	if (reg > max_reg)
	  {
	    SET_ERROR_STRING (ERROR_REG_OVER_RANGE, reg);
	    return false;
	  }
	reg |= reg << shift_num;
	*oper += len;
	csky_insn.opcode_end = *oper;
	csky_insn.val[csky_insn.idx++] = reg;
	return true;
      }
    case OPRND_TYPE_CONST1:
      *oper = parse_exp (*oper, &csky_insn.e1);
      if (csky_insn.e1.X_op == O_constant)
	{
	  csky_insn.opcode_end = *oper;
	  if (csky_insn.e1.X_add_number != 1)
	    return false;
	  csky_insn.val[csky_insn.idx++] = 1;
	  return true;
	}
      return false;
    case OPRND_TYPE_UNCOND10b:
    case OPRND_TYPE_UNCOND16b:
      *oper = parse_exp (*oper, &csky_insn.e1);
      if (csky_insn.e1.X_op == O_constant)
	return false;
      input_line_pointer = *oper;
      csky_insn.opcode_end = *oper;
      csky_insn.relax.max = UNCD_DISP16_LEN;
      csky_insn.relax.var = UNCD_DISP10_LEN;
      csky_insn.relax.subtype = UNCD_DISP10;
      csky_insn.val[csky_insn.idx++] = 0;
      return true;
    case OPRND_TYPE_COND10b:
    case OPRND_TYPE_COND16b:
      *oper = parse_exp (*oper, &csky_insn.e1);
      if (csky_insn.e1.X_op == O_constant)
	return false;
      input_line_pointer = *oper;
      csky_insn.opcode_end = *oper;
      /* CK801 doesn't have 32-bit bt/bf insns; relax to a short
	 jump around a 32-bit unconditional branch instead.  */
      if (IS_CSKY_ARCH_801 (mach_flag))
	{
	  csky_insn.relax.max = SCOND_DISP16_LEN;
	  csky_insn.relax.var = SCOND_DISP10_LEN;
	  csky_insn.relax.subtype = SCOND_DISP10;
	}
      else
	{
	  csky_insn.relax.max = COND_DISP16_LEN;
	  csky_insn.relax.var = COND_DISP10_LEN;
	  csky_insn.relax.subtype = COND_DISP10;
	}
      csky_insn.val[csky_insn.idx++] = 0;
      return true;
    case OPRND_TYPE_JCOMPZ:
      *oper = parse_exp (*oper, &csky_insn.e1);
      if (csky_insn.e1.X_op == O_constant)
	return false;
      input_line_pointer = *oper;
      csky_insn.opcode_end = *oper;
      csky_insn.relax.max = JCOMPZ_DISP32_LEN;
      csky_insn.relax.var = JCOMPZ_DISP16_LEN;
      csky_insn.relax.subtype = JCOMPZ_DISP16;
      csky_insn.max = JCOMPZ_DISP32_LEN;
      csky_insn.val[csky_insn.idx++] = 0;
      return true;
    case OPRND_TYPE_JBTF:
      *oper = parse_exp (*oper, &csky_insn.e1);
      input_line_pointer = *oper;
      csky_insn.opcode_end = *oper;
      csky_insn.relax.max = csky_relax_table[C (COND_JUMP_S, DISP32)].rlx_length;
      csky_insn.relax.var = csky_relax_table[C (COND_JUMP_S, DISP12)].rlx_length;
      csky_insn.relax.subtype = C (COND_JUMP_S, 0);
      csky_insn.val[csky_insn.idx++] = 0;
      csky_insn.max = C32_LEN_S + 2;
      return true;
    case OPRND_TYPE_JBR:
      *oper = parse_exp (*oper, &csky_insn.e1);
      input_line_pointer = *oper;
      csky_insn.opcode_end = *oper;
      csky_insn.relax.max = csky_relax_table[C (UNCD_JUMP_S, DISP32)].rlx_length;
      csky_insn.relax.var = csky_relax_table[C (UNCD_JUMP_S, DISP12)].rlx_length;
      csky_insn.relax.subtype = C (UNCD_JUMP_S, 0);
      csky_insn.val[csky_insn.idx++] = 0;
      csky_insn.max = U32_LEN_S + 2;
      return true;
    case OPRND_TYPE_JBSR:
      if (do_force2bsr)
	*oper = parse_exp (*oper, &csky_insn.e1);
      else
	*oper = parse_rt (*oper, 1, &csky_insn.e1, -1);
      input_line_pointer = *oper;
      csky_insn.opcode_end = *oper;
      csky_insn.val[csky_insn.idx++] = 0;
      return true;
    case OPRND_TYPE_REGLIST_DASH_COMMA:
      return is_reglist_dash_comma_legal (oper, oprnd);

    case OPRND_TYPE_MSB2SIZE:
    case OPRND_TYPE_LSB2SIZE:
      {
	expressionS e;
	char *new_oper = parse_exp (*oper, &e);
	if (e.X_op == O_constant)
	  {
	    *oper = new_oper;
	    if (e.X_add_number > 31)
	      {
		SET_ERROR_STRING (ERROR_IMM_OVERFLOW, NULL);
		return false;
	      }
	    csky_insn.val[csky_insn.idx++] = e.X_add_number;
	    if (oprnd->type == OPRND_TYPE_LSB2SIZE)
	      {
		if (csky_insn.val[csky_insn.idx - 1] > csky_insn.val[csky_insn.idx - 2])
		  {
		    SET_ERROR_STRING (ERROR_IMM_OVERFLOW, NULL);
		    return false;
		  }
		csky_insn.val[csky_insn.idx - 2] -= e.X_add_number;
	      }
	    return true;
	  }
	return false;
      }
    case OPRND_TYPE_AREG_WITH_LSHIFT:
      return is_reg_lshift_illegal (oper, 0);
    case OPRND_TYPE_AREG_WITH_LSHIFT_FPU:
      return is_reg_lshift_illegal (oper, 1);
    case OPRND_TYPE_FREG_WITH_INDEX:
    case OPRND_TYPE_VREG_WITH_INDEX:
      if (parse_type_freg (oper, 0))
	{
	  if (**oper == '[')
	    {
	      (*oper)++;
	      if (is_imm_within_range (oper, 0, 0xf))
		{
		  if (**oper == ']')
		    {
		      unsigned int idx = --csky_insn.idx;
		      unsigned int val = csky_insn.val[idx];
		      (*oper)++;
		      csky_insn.val[idx - 1] |= val << 4;
		      return true;
		    }
		  else
		    SET_ERROR_STRING (ERROR_MISSING_RSQUARE_BRACKETS, NULL);
		}
	    }
	  else
	    SET_ERROR_STRING (ERROR_MISSING_LSQUARE_BRACKETS, NULL);
	}
      return false;

    default:
      break;
      /* error code.  */
    }
  return false;
}

/* Subroutine of parse_operands.  */

static bool
parse_operands_op (char *str, struct csky_opcode_info *op)
{
  int i;
  int j;
  char *oper = str;
  int flag_pass;

  for (i = 0; i < OP_TABLE_NUM && op[i].operand_num != -2; i++)
    {
      flag_pass = true;
      csky_insn.idx = 0;
      oper = str;
      /* if operand_num = -1, it is a insn with a REGLIST type operand.i.  */
      if (!(op[i].operand_num == csky_insn.number
	    || (op[i].operand_num == -1 && csky_insn.number != 0)))
	{
	  /* The smaller err_num is more serious.  */
	  SET_ERROR_INTEGER (ERROR_OPERANDS_NUMBER, op[i].operand_num);
	  flag_pass = false;
	  continue;
	}

      for (j = 0; j < csky_insn.number; j++)
	{
	  while (ISSPACE (*oper))
	    oper++;
	  flag_pass = get_operand_value (&op[i], &oper,
					 &op[i].oprnd.oprnds[j]);
	  if (!flag_pass)
	    break;
	  while (ISSPACE (*oper))
	    oper++;
	  /* Skip the ','.  */
	  if (j < csky_insn.number - 1 && op[i].operand_num != -1)
	    {
	      if (*oper == ',')
		oper++;
	      else
		{
		  SET_ERROR_STRING (ERROR_MISSING_COMMA, NULL);
		  flag_pass = false;
		  break;
		}
	    }
	  else if (!is_end_of_line[(unsigned char) *oper])
	    {
	      SET_ERROR_STRING (ERROR_BAD_END, NULL);
	      flag_pass = false;
	      break;
	    }
	  else
	    break;
	}
      /* Parse operands in one table end.  */

      if (flag_pass)
	{
	  /* Parse operands success, set opcode_idx.  */
	  csky_insn.opcode_idx = i;
	  return true;
	}
      else
	error_state.opnum = j + 1;
    }
  /* Parse operands in ALL tables end.  */
  return false;
}

/* Parse the operands according to operand type.  */

static bool
parse_operands (char *str)
{
  char *oper = str;

  /* Parse operands according to flag_force.  */
  if (csky_insn.flag_force == INSN_OPCODE16F
      && (csky_insn.opcode->isa_flag16 & isa_flag) != 0)
    {
      if (parse_operands_op (oper, csky_insn.opcode->op16))
	{
	  csky_insn.isize = 2;
	  return true;
	}
      return false;
    }
  else if (csky_insn.flag_force == INSN_OPCODE32F
	   && (csky_insn.opcode->isa_flag32 & isa_flag) != 0)
    {
      if (parse_operands_op (oper, csky_insn.opcode->op32))
	{
	  csky_insn.isize = 4;
	  return true;
	}
      return false;
    }
  else
    {
      if ((csky_insn.opcode->isa_flag16 & isa_flag) != 0
	  && parse_operands_op (oper, csky_insn.opcode->op16))
	{
	  csky_insn.isize = 2;
	  return true;
	}
      if ((csky_insn.opcode->isa_flag32 & isa_flag) != 0
	  && parse_operands_op (oper, csky_insn.opcode->op32))
	{
	  csky_insn.isize = 4;
	  return true;
	}
      return false;
    }
}

static bool
csky_generate_frags (void)
{
  /* frag more relax reloc.  */
  if (csky_insn.flag_force == INSN_OPCODE16F
      || !IS_SUPPORT_OPCODE32 (csky_insn.opcode))
    {
      csky_insn.output = frag_more (csky_insn.isize);
      if (csky_insn.opcode->reloc16)
	{
	  /* 16 bits opcode force, should generate fixup.  */
	  reloc_howto_type *howto;
	  howto = bfd_reloc_type_lookup (stdoutput,
					 csky_insn.opcode->reloc16);
	  fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		       2, &csky_insn.e1, howto->pc_relative,
		       csky_insn.opcode->reloc16);
	}
    }
  else if (csky_insn.flag_force == INSN_OPCODE32F)
    {
      csky_insn.output = frag_more (csky_insn.isize);
      if (csky_insn.opcode->reloc32)
	{
	  reloc_howto_type *howto;
	  howto = bfd_reloc_type_lookup (stdoutput,
					 csky_insn.opcode->reloc32);
	  fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		       4, &csky_insn.e1, howto->pc_relative,
		       csky_insn.opcode->reloc32);
	}
    }
  else if (csky_insn.opcode->relax)
    /* Generate the relax information.  */
    csky_insn.output = frag_var (rs_machine_dependent,
				 csky_insn.relax.max,
				 csky_insn.relax.var,
				 csky_insn.relax.subtype,
				 csky_insn.e1.X_add_symbol,
				 csky_insn.e1.X_add_number, 0);
  else
    {
      csky_insn.output = frag_more (csky_insn.isize);
      if (csky_insn.opcode->reloc16 && csky_insn.isize == 2)
	{
	  reloc_howto_type *howto;
	  howto = bfd_reloc_type_lookup (stdoutput,
					 csky_insn.opcode->reloc16);
	  fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		       2, &csky_insn.e1, howto->pc_relative,
		       csky_insn.opcode->reloc16);
	}
      else if (csky_insn.opcode->reloc32 && csky_insn.isize == 4)
	{
	  reloc_howto_type *howto;
	  howto = bfd_reloc_type_lookup (stdoutput,
					 csky_insn.opcode->reloc32);
	  fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		       4, &csky_insn.e1, howto->pc_relative,
		       csky_insn.opcode->reloc32);
	}
    }
  return true;
}

/* Return the bits of VAL shifted according to MASK.  The bits of MASK
   need not be contiguous.  */

static int
generate_masked_value (int mask, int val)
{
  int ret = 0;
  int bit;

  for (bit = 1; mask; bit = bit << 1)
    if (mask & bit)
      {
	if (val & 0x1)
	  ret |= bit;
	val = val >> 1;
	mask &= ~bit;
      }
  return ret;
}

/* Return the result of masking operand number OPRND_IDX into the
   instruction word according to the information in OPRND.  */

static int
generate_masked_operand (struct operand *oprnd, int *oprnd_idx)
{
  struct soperand *soprnd = NULL;
  int mask;
  int val;
  if ((unsigned int)oprnd->mask == HAS_SUB_OPERAND)
    {
      soprnd = (struct soperand *) oprnd;
      generate_masked_operand (&soprnd->subs[0], oprnd_idx);
      generate_masked_operand (&soprnd->subs[1], oprnd_idx);
      return 0;
    }
  mask = oprnd->mask;
  val = csky_insn.val[*oprnd_idx];
  (*oprnd_idx)++;
  val = generate_masked_value (mask, val);
  csky_insn.inst |= val;

  return 0;
}

static bool
csky_generate_insn (void)
{
  int i = 0;
  struct csky_opcode_info *opinfo = NULL;

  if (csky_insn.isize == 4)
    opinfo = &csky_insn.opcode->op32[csky_insn.opcode_idx];
  else if (csky_insn.isize == 2)
    opinfo = &csky_insn.opcode->op16[csky_insn.opcode_idx];

  int sidx = 0;
  csky_insn.inst = opinfo->opcode;
  if (opinfo->operand_num == -1)
    {
      generate_masked_operand (&opinfo->oprnd.oprnds[i], &sidx);
      return 0;
    }
  else
    for (i = 0; i < opinfo->operand_num; i++)
      generate_masked_operand (&opinfo->oprnd.oprnds[i], &sidx);
  return 0;
}

/* Main entry point for assembling a single instruction.  */

void
md_assemble (char *str)
{
  bool must_check_literals = true;
  csky_insn.isize = 0;
  csky_insn.idx = 0;
  csky_insn.max = 0;
  csky_insn.flag_force = INSN_OPCODE;
  csky_insn.macro = NULL;
  csky_insn.opcode = NULL;
  memset (csky_insn.val, 0, sizeof (int) * MAX_OPRND_NUM);
  /* Initialize err_num.  */
  error_state.err_num = ERROR_NONE;
  mov_r1_before = false;
  mov_r1_after = false;

  mapping_state (MAP_TEXT);
  /* Tie dwarf2 debug info to every insn if set option --gdwarf2.  */
  dwarf2_emit_insn (0);
  while (ISSPACE (* str))
    str++;
  /* Get opcode from str.  */
  if (!parse_opcode (str))
    {
      csky_show_error (ERROR_OPCODE_ILLEGAL, 0, NULL, NULL);
      return;
    }

  /* If it is a macro instruction, handle it.  */
  if (csky_insn.macro != NULL)
    {
      if (csky_insn.number == csky_insn.macro->oprnd_num)
	{
	  csky_insn.macro->handle_func ();
	  return;
	}
      else if (error_state.err_num > ERROR_OPERANDS_NUMBER)
	SET_ERROR_STRING (ERROR_OPERANDS_NUMBER, csky_insn.macro->oprnd_num);
    }

  if (csky_insn.opcode == NULL)
    {
      SET_ERROR_STRING (ERROR_OPCODE_ILLEGAL, NULL);
      csky_show_error (error_state.err_num, error_state.opnum,
		       (void *)error_state.arg1, (void *)error_state.arg1);
      return;
    }

  /* Parse the operands according to operand type.  */
  if (!parse_operands (csky_insn.opcode_end))
    {
      csky_show_error (error_state.err_num, error_state.opnum,
		       (void *)error_state.arg1, (void *)error_state.arg1);
      return;
    }
  error_state.err_num = ERROR_NONE;

  /* if this insn has work in opcode table, then do it.  */
  if (csky_insn.opcode->work != NULL)
      must_check_literals = csky_insn.opcode->work ();
  else
    {
      /* Generate relax or reloc if necessary.  */
      csky_generate_frags ();
      /* Generate the insn by mask.  */
      csky_generate_insn ();
      /* Write inst to frag.  */
      csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
    }

  /* Adjust for xtrb0/xtrb1/xtrb2/xtrb3/divs/divu in csky v1 ISA.  */
  if (mov_r1_after)
    {
      unsigned int mov_insn = CSKYV1_INST_MOV_RX_R1;
      mov_insn |= csky_insn.val[0];
      mov_r1_before = true;
      csky_insn.output = frag_more (2);
      dwarf2_emit_insn (0);
      md_number_to_chars (csky_insn.output, mov_insn, 2);
      csky_insn.isize += 2;
    }
  if (mov_r1_before)
    csky_insn.isize += 2;

  /* Check literal.  */
  if (must_check_literals)
    {
      if (csky_insn.max == 0)
	check_literals (csky_insn.opcode->transfer, csky_insn.isize);
      else
	check_literals (csky_insn.opcode->transfer, csky_insn.max);
    }

  csky_insn.last_isize = csky_insn.isize;
  insn_reloc = BFD_RELOC_NONE;
}

/* Attempt to handle option with value C, returning non-zero on success.  */

int
md_parse_option (int c, const char *arg)
{
  switch (c)
    {
    case 0:
      break;
    case OPTION_MARCH:
      parse_arch (arg);
      break;
    case OPTION_MCPU:
      parse_cpu (arg);
      break;
    case OPTION_FLOAT_ABI:
      parse_float_abi (arg);
      break;
    default:
      return 0;
    }
  return 1;
}

/* Convert a machine dependent frag.  */
#define PAD_LITERAL_LENGTH                        6
#define opposite_of_stored_comp(insn)             (insn ^ 0x04000000)
#define opposite_of_stored_compz(insn)            (insn ^ 0x00200000)
#define make_insn(total_length, opcode, operand, operand_length)	\
  do {									\
    if (total_length > 0)						\
      {									\
	csky_write_insn (buf,						\
			 opcode | (operand & ((1 << operand_length) - 1)), \
			 total_length);					\
	buf += total_length;						\
	fragp->fr_fix += total_length;					\
      }									\
  } while (0)

#define make_literal(fragp, literal_offset)				\
  do {									\
    make_insn (literal_offset, PAD_FILL_CONTENT, 0, 0);			\
    fix_new (fragp, fragp->fr_fix, 4, fragp->fr_symbol,			\
	     fragp->fr_offset, 0, BFD_RELOC_CKCORE_ADDR32);		\
    make_insn (4, 0, 0, 0);						\
    make_insn (2 - literal_offset, PAD_FILL_CONTENT, 0, 0);		\
  } while (0)

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, segT asec,  fragS *fragp)
{
  offsetT disp;
  char *buf = fragp->fr_fix + &fragp->fr_literal[0];

  gas_assert (fragp->fr_symbol);
  if (IS_EXTERNAL_SYM (fragp->fr_symbol, asec))
    disp = 0;
  else
    disp = (S_GET_VALUE (fragp->fr_symbol)
	    + fragp->fr_offset
	    - fragp->fr_address
	    - fragp->fr_fix);

  switch (fragp->fr_subtype)
    {
      /* generate new insn.  */
    case C (COND_JUMP, DISP12):
    case C (UNCD_JUMP, DISP12):
    case C (COND_JUMP_PIC, DISP12):
    case C (UNCD_JUMP_PIC, DISP12):
      {
#define CSKY_V1_B_MASK   0xf8
	unsigned char t0;
	disp -= 2;
	if (disp & 1)
	  {
	    /* Error. odd displacement at %x, next_inst-2.  */
	    ;
	  }
	disp >>= 1;

	if (!target_big_endian)
	  {
	    t0 = buf[1] & CSKY_V1_B_MASK;
	    md_number_to_chars (buf, disp, 2);
	    buf[1] = (buf[1] & ~CSKY_V1_B_MASK) | t0;
	  }
	else
	  {
	    t0 = buf[0] & CSKY_V1_B_MASK;
	    md_number_to_chars (buf, disp, 2);
	    buf[0] = (buf[0] & ~CSKY_V1_B_MASK) | t0;
	  }
	fragp->fr_fix += 2;
	break;
      }
    case C (COND_JUMP, DISP32):
    case C (COND_JUMP, UNDEF_WORD_DISP):
      {
	/* A conditional branch wont fit into 12 bits:
	   b!cond 1f
	   jmpi 0f
	   .align 2
	   0: .long disp
	   1:
	*/
	int first_inst = fragp->fr_fix + fragp->fr_address;
	int is_unaligned = (first_inst & 3);

	if (!target_big_endian)
	  {
	    /* b!cond instruction.  */
	    buf[1] ^= 0x08;
	    /* jmpi instruction.  */
	    buf[2] = CSKYV1_INST_JMPI & 0xff;
	    buf[3] = CSKYV1_INST_JMPI >> 8;
	  }
	else
	  {
	    /* b!cond instruction.  */
	    buf[0] ^= 0x08;
	    /* jmpi instruction.  */
	    buf[2] = CSKYV1_INST_JMPI >> 8;
	    buf[3] = CSKYV1_INST_JMPI & 0xff;
	  }

	if (is_unaligned)
	  {
	    if (!target_big_endian)
	      {
		/* bt/bf: jump to pc + 2 + (4 << 1).  */
		buf[0] = 4;
		/* jmpi: jump to MEM (pc + 2 + (1 << 2)).  */
		buf[2] = 1;
	      }
	    else
	      {
		/* bt/bf: jump to pc + 2 + (4 << 1).  */
		buf[1] = 4;
		/* jmpi: jump to MEM (pc + 2 + (1 << 2)).  */
		buf[3] = 1;
	      }
	    /* Aligned 4 bytes.  */
	    buf[4] = 0;
	    buf[5] = 0;
	    /* .long  */
	    buf[6] = 0;
	    buf[7] = 0;
	    buf[8] = 0;
	    buf[9] = 0;

	    /* Make reloc for the long disp.  */
	    fix_new (fragp, fragp->fr_fix + 6, 4,
		     fragp->fr_symbol, fragp->fr_offset, 0, BFD_RELOC_32);
	    fragp->fr_fix += C32_LEN;
	  }
	else
	  {
	    if (!target_big_endian)
	      {
		/* bt/bf: jump to pc + 2 + (3 << 1).  */
		buf[0] = 3;
		/* jmpi: jump to MEM (pc + 2 + (0 << 2)).  */
		buf[2] = 0;
	      }
	    else
	      {
		/* bt/bf: jump to pc + 2 + (3 << 1).  */
		buf[1] = 3;
		/* jmpi: jump to MEM (pc + 2 + (0 << 2)).  */
		buf[3] = 0;
	      }
	    /* .long  */
	    buf[4] = 0;
	    buf[5] = 0;
	    buf[6] = 0;
	    buf[7] = 0;

	    /* Make reloc for the long disp.  */
	    fix_new (fragp, fragp->fr_fix + 4, 4,
		     fragp->fr_symbol, fragp->fr_offset, 0, BFD_RELOC_32);
	    fragp->fr_fix += C32_LEN;

	    /* Frag is actually shorter (see the other side of this ifdef)
	       but gas isn't prepared for that.  We have to re-adjust
	       the branch displacement so that it goes beyond the
	       full length of the fragment, not just what we actually
	       filled in.  */
	    if (!target_big_endian)
	      buf[0] = 4;
	    else
	      buf[1] = 4;
	  }
      }
      break;

    case C (COND_JUMP_PIC, DISP32):
    case C (COND_JUMP_PIC, UNDEF_WORD_DISP):
      {
#define BYTE_1(a) (target_big_endian ? ((a) & 0xff) : ((a) >> 8))
#define BYTE_0(a) (target_big_endian ? ((a) >> 8) : ((a) & 0xff))
	/* b!cond 1f
	   subi sp, 8
	   stw  r15, (sp, 0)
	   bsr  .L0
	   .L0:
	   lrw r1, 0f
	   add r1, r15
	   addi sp, 8
	   jmp r1
	   .align 2
	   0: .long (tar_addr - pc)
	   1:
	*/
	int first_inst = fragp->fr_fix + fragp->fr_address;
	int is_unaligned = (first_inst & 3);
	disp -= 8;
	/* Toggle T/F bit.  */
	if (! target_big_endian)
	  buf[1] ^= 0x08;
	else
	  buf[0] ^= 0x08;
	buf[2] = BYTE_0 (CSKYV1_INST_SUBI | (7 << 4));     /* subi r0, 8.  */
	buf[3] = BYTE_1 (CSKYV1_INST_SUBI | (7 << 4));
	buf[4] = BYTE_0 (CSKYV1_INST_STW  | (15 << 8));    /* stw r15, r0.  */
	buf[5] = BYTE_1 (CSKYV1_INST_STW  | (15 << 8));
	buf[6] = BYTE_0 (CSKYV1_INST_BSR);                 /* bsr pc + 2.  */
	buf[7] = BYTE_1 (CSKYV1_INST_BSR);
	buf[8] = BYTE_0 (CSKYV1_INST_LRW | (1 << 8));      /* lrw r1, (tar_addr - pc).  */
	buf[9] = BYTE_1 (CSKYV1_INST_LRW | (1 << 8));
	buf[10] = BYTE_0 (CSKYV1_INST_ADDU | (15 << 4) | 1);  /* add r1, r15.  */
	buf[11] = BYTE_1 (CSKYV1_INST_ADDU | (15 << 4) | 1);
	buf[12] = BYTE_0 (CSKYV1_INST_LDW | (15 << 8));     /* ldw r15, r0.  */
	buf[13] = BYTE_1 (CSKYV1_INST_LDW | (15 << 8));
	buf[14] = BYTE_0 (CSKYV1_INST_ADDI | (7 << 4));     /* addi r0, 8.  */
	buf[15] = BYTE_1 (CSKYV1_INST_ADDI | (7 << 4));
	buf[16] = BYTE_0 (CSKYV1_INST_JMP | 1);             /* jmp r1.  */
	buf[17] = BYTE_1 (CSKYV1_INST_JMP | 1);

	if (!is_unaligned)
	  {
	    if (!target_big_endian)
	      {
		buf[0] = 11;
		buf[8] = 3;
		buf[20] = disp & 0xff;
		buf[21] = (disp >> 8) & 0xff;
		buf[22] = (disp >> 16) & 0xff;
		buf[23] = (disp >> 24) & 0xff;
	      }
	    else /* if !target_big_endian.  */
	      {
		buf[1] = 11;
		buf[9] = 3;
		buf[20] = (disp >> 24) & 0xff;
		buf[21] = (disp >> 16) & 0xff;
		buf[22] = (disp >> 8) & 0xff;
		buf[23] = disp & 0xff;
	      }
	    buf[18] = 0;  /* alignment.  */
	    buf[19] = 0;
	    fragp->fr_fix += C32_LEN_PIC;
	  }
	else  /* if !is_unaligned.  */
	  {
	    if (!target_big_endian)
	      {
		buf[0] = 11;
		buf[8] = 2;
		buf[18] = disp & 0xff;
		buf[19] = (disp >> 8) & 0xff;
		buf[20] = (disp >> 16) & 0xff;
		buf[21] = (disp >> 24) & 0xff;
	      }
	    else /* if !target_big_endian.  */
	      {
		buf[1] = 11;
		buf[9] = 2;
		buf[18] = (disp >> 24) & 0xff;
		buf[19] = (disp >> 16) & 0xff;
		buf[20] = (disp >> 8) & 0xff;
		buf[21] = disp & 0xff;
	      }
	    buf[22] = 0;  /* initialise.  */
	    buf[23] = 0;
	    fragp->fr_fix += C32_LEN_PIC;

	  } /* end if is_unaligned.  */
      } /* end case C (COND_JUMP_PIC, DISP32)/C (COND_JUMP_PIC, UNDEF_WORD_DISP).  */
      break;
    case C (UNCD_JUMP, DISP32):
    case C (UNCD_JUMP, UNDEF_WORD_DISP):
      {
	/* jmpi 0f
	   .align 2
	   0: .long disp.  */
	int first_inst = fragp->fr_fix + fragp->fr_address;
	int is_unaligned = (first_inst & 3);
	/* Build jmpi.  */
	buf[0] = BYTE_0 (CSKYV1_INST_JMPI);
	buf[1] = BYTE_1 (CSKYV1_INST_JMPI);
	if (!is_unaligned)
	  {
	    if (!target_big_endian)
	      buf[0] = 1;
	    else
	      buf[1] = 1;
	    /* Alignment.  */
	    buf[2] = 0;
	    buf[3] = 0;
	    /* .long  */
	    buf[4] = 0;
	    buf[5] = 0;
	    buf[6] = 0;
	    buf[7] = 0;
	    fix_new (fragp, fragp->fr_fix + 4, 4,
		     fragp->fr_symbol, fragp->fr_offset, 0, BFD_RELOC_32);
	    fragp->fr_fix += U32_LEN;
	  }
	else /* if is_unaligned.  */
	  {
	    if (!target_big_endian)
	      buf[0] = 0;
	    else
	      buf[1] = 0;
	    /* .long  */
	    buf[2] = 0;
	    buf[3] = 0;
	    buf[4] = 0;
	    buf[5] = 0;
	    fix_new (fragp, fragp->fr_fix + 2, 4,
		     fragp->fr_symbol, fragp->fr_offset, 0, BFD_RELOC_32);
	    fragp->fr_fix += U32_LEN;

	  }
      }
      break;
    case C (UNCD_JUMP_PIC, DISP32):
    case C (UNCD_JUMP_PIC, UNDEF_WORD_DISP):
      {
	/*    subi sp, 8
	      stw  r15, (sp)
	      bsr  .L0
	      .L0:
	      lrw  r1, 0f
	      add  r1, r15
	      ldw  r15, (sp)
	      addi sp, 8
	      jmp r1
	      .align 2
	      0: .long (tar_add - pc)
	      1:
	*/
	/* If the b!cond is 4 byte aligned, the literal which would
	   go at x+4 will also be aligned.  */
	int first_inst = fragp->fr_fix + fragp->fr_address;
	int is_unaligned = (first_inst & 3);
	disp -= 6;

	buf[0] = BYTE_0 (CSKYV1_INST_SUBI | (7 << 4));     /* subi r0, 8.  */
	buf[1] = BYTE_1 (CSKYV1_INST_SUBI | (7 << 4));
	buf[2] = BYTE_0 (CSKYV1_INST_STW  | (15 << 8));    /* stw r15, r0.  */
	buf[3] = BYTE_1 (CSKYV1_INST_STW  | (15 << 8));
	buf[4] = BYTE_0 (CSKYV1_INST_BSR);                 /* bsr pc + 2.  */
	buf[5] = BYTE_1 (CSKYV1_INST_BSR);
	buf[6] = BYTE_0 (CSKYV1_INST_LRW | (1 << 8));      /* lrw r1, (tar_addr - pc).  */
	buf[7] = BYTE_1 (CSKYV1_INST_LRW | (1 << 8));
	buf[8] = BYTE_0 (CSKYV1_INST_ADDU | (15 << 4) | 1);  /* add r1, r15.  */
	buf[9] = BYTE_1 (CSKYV1_INST_ADDU | (15 << 4) | 1);
	buf[10] = BYTE_0 (CSKYV1_INST_LDW | (15 << 8));     /* ldw r15, r0.  */
	buf[11] = BYTE_1 (CSKYV1_INST_LDW | (15 << 8));
	buf[12] = BYTE_0 (CSKYV1_INST_ADDI | (7 << 4));     /* addi r0, 8.  */
	buf[13] = BYTE_1 (CSKYV1_INST_ADDI | (7 << 4));
	buf[14] = BYTE_0 (CSKYV1_INST_JMP | 1);             /* jmp r1.  */
	buf[15] = BYTE_1 (CSKYV1_INST_JMP | 1);

	if (is_unaligned)
	  {
	    if (!target_big_endian)
	      {
		buf[6] = 3;
		buf[18] = disp & 0xff;
		buf[19] = (disp >> 8) & 0xff;
		buf[20] = (disp >> 16) & 0xff;
		buf[21] = (disp >> 24) & 0xff;
	      }
	    else
	      {
		buf[7] = 3;
		buf[18] = (disp >> 24) & 0xff;
		buf[19] = (disp >> 16) & 0xff;
		buf[20] = (disp >> 8) & 0xff;
		buf[21] = disp & 0xff;
	      }
	    buf[16] = 0;
	    buf[17] = 0;
	    fragp->fr_fix += U32_LEN_PIC;
	  }
	else
	  {
	    if (!target_big_endian)
	      {
		buf[6] = 2;
		buf[16] = disp & 0xff;
		buf[17] = (disp >> 8) & 0xff;
		buf[18] = (disp >> 16) & 0xff;
		buf[19] = (disp >> 24) & 0xff;
	      }
	    else
	      {
		buf[7] = 2;
		buf[16] = (disp >> 24) & 0xff;
		buf[17] = (disp >> 16) & 0xff;
		buf[18] = (disp >> 8) & 0xff;
		buf[19] = disp & 0xff;
	      }
	    fragp->fr_fix += U32_LEN_PIC;
	  }
      }
      break;
    case COND_DISP10:
    case SCOND_DISP10:
    case UNCD_DISP10:
    case JCOND_DISP10:
    case JUNCD_DISP10:
      {
	unsigned int inst = csky_read_insn (buf, 2);
	inst |= (disp >> 1) & ((1 << 10) - 1);
	csky_write_insn (buf, inst, 2);
	fragp->fr_fix += 2;
	break;
      }
    case SCOND_DISP16:
      {
	unsigned int inst = csky_read_insn (buf, 2);

	if (inst == CSKYV2_INST_BT16)
	  inst = CSKYV2_INST_BF16;
	else
	  inst = CSKYV2_INST_BT16;
	make_insn (2, inst, (2 + 4) >> 1, 10);
	if (IS_EXTERNAL_SYM (fragp->fr_symbol, asec))
	  fix_new (fragp, fragp->fr_fix, 4,
		   fragp->fr_symbol, fragp->fr_offset, 1,
		   BFD_RELOC_CKCORE_PCREL_IMM16BY2);
	disp -= 2;
	inst = CSKYV2_INST_BR32 | ((disp >> 1) & ((1 << 16) - 1));
	csky_write_insn (buf, inst, 4);
	fragp->fr_fix += 4;
	break;
      }
    case COND_DISP16:
    case JCOND_DISP16:
      {
	unsigned int inst = csky_read_insn (buf, 2);

	if (inst == CSKYV2_INST_BT16)
	  inst = CSKYV2_INST_BT32;
	else
	  inst = CSKYV2_INST_BF32;
	if (IS_EXTERNAL_SYM (fragp->fr_symbol, asec))
	  fix_new (fragp, fragp->fr_fix, 4,
		   fragp->fr_symbol, fragp->fr_offset, 1,
		   BFD_RELOC_CKCORE_PCREL_IMM16BY2);
	inst |= (disp >> 1) & ((1 << 16) - 1);
	csky_write_insn (buf, inst, 4);
	fragp->fr_fix += 4;
	break;
      }
    case LRW_DISP7:
      {
	unsigned int inst = csky_read_insn (buf, 2);
	int imm;
	imm = (disp + 2) >> 2;
	inst |= (imm >> 5) << 8;
	make_insn (2, inst, (imm & 0x1f), 5);
	break;
      }
    case LRW2_DISP8:
      {
	unsigned int inst = csky_read_insn (buf, 2);
	int imm = (disp + 2) >> 2;
	if (imm >= 0x80)
	  {
	    inst &= 0xe0;
	    inst |= (~((imm >> 5) << 8)) & 0x300;
	    make_insn (2, inst, (~imm & 0x1f), 5);
	  }
	else
	  {
	    inst |= (imm >> 5) << 8;
	    make_insn (2, inst, (imm & 0x1f), 5);
	  }
	break;
      }
    case LRW_DISP16:
      {
	unsigned int inst = csky_read_insn (buf, 2);
	inst = CSKYV2_INST_LRW32 | (((inst & 0xe0) >> 5) << 16);
	if (IS_EXTERNAL_SYM (fragp->fr_symbol, asec))
	  fix_new (fragp, fragp->fr_fix, 4,
		   fragp->fr_symbol, fragp->fr_offset, 1,
		   BFD_RELOC_CKCORE_PCREL_IMM16BY4);
	make_insn (4, inst, ((disp + 2) >> 2), 16);
	break;
      }
    case JCOMPZ_DISP16:
      {
	unsigned int inst = csky_read_insn (buf, 4);
	make_insn (4, inst, disp >> 1, 16);
      }
      break;
    case JCOMPZ_DISP32:
      {
	unsigned int inst = csky_read_insn (buf, 4);
	int literal_offset;
	make_insn (4, opposite_of_stored_compz (inst),
		   (4 + 4 + PAD_LITERAL_LENGTH) >> 1, 16);
	literal_offset = ((fragp->fr_address + fragp->fr_fix) % 4 == 0
			  ? 0 : 2);
	make_insn (4, CSKYV2_INST_JMPI32, (4 + literal_offset + 2) >> 2, 10);
	make_literal (fragp, literal_offset);
      }
      break;
    case JUNCD_DISP16:
    case UNCD_DISP16:
      {
	if (IS_EXTERNAL_SYM (fragp->fr_symbol, asec))
	  fix_new (fragp, fragp->fr_fix, 4,
		   fragp->fr_symbol, fragp->fr_offset, 1,
		   BFD_RELOC_CKCORE_PCREL_IMM16BY2);
	make_insn (4, CSKYV2_INST_BR32, disp >> 1, 16);
      }
      break;
    case JCOND_DISP32:
      {
	/* 'jbt'/'jbf'-> <bf16/bt16>; jmpi32; [pad16]+literal32  */
	unsigned int inst = csky_read_insn (buf, 2);
	int literal_offset;

	if (inst == CSKYV2_INST_BT16)
	  inst = CSKYV2_INST_BF16;
	else
	  inst = CSKYV2_INST_BT16;
	make_insn (2, inst, (2 + 4 + PAD_LITERAL_LENGTH) >> 1, 10);
	literal_offset = ((fragp->fr_address + fragp->fr_fix) % 4 == 0
			  ? 0 : 2);
	make_insn (4, CSKYV2_INST_JMPI32, (4 + literal_offset + 2) >> 2, 10);
	make_literal (fragp, literal_offset);
	break;
      }
    case JUNCD_DISP32:
      {
	int literal_offset;
	literal_offset = ((fragp->fr_address + fragp->fr_fix) % 4 == 0
			  ? 0 : 2);
	make_insn (4, CSKYV2_INST_JMPI32, (4 + literal_offset + 2) >> 2, 10);
	make_literal (fragp, literal_offset);
      }
      break;
    case RELAX_OVERFLOW:
      csky_branch_report_error (fragp->fr_file, fragp->fr_line,
				fragp->fr_symbol, disp);
      break;
    default:
      abort ();
      break;
    }
}

/* Round up a section size to the appropriate boundary.  */

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED,
		  valueT size)
{
  return size;
}

/* MD interface: Symbol and relocation handling.  */

void csky_md_finish (void)
{
  dump_literals (0);
}

/* Return the address within the segment that a PC-relative fixup is
   relative to.  */

long
md_pcrel_from_section (fixS * fixP, segT seg)
{
  /* If the symbol is undefined or defined in another section
     we leave the add number alone for the linker to fix it later.  */
  if (fixP->fx_addsy != (symbolS *) NULL
      && (! S_IS_DEFINED (fixP->fx_addsy)
	  || S_GET_SEGMENT (fixP->fx_addsy) != seg))
    return fixP->fx_size;

  /* The case where we are going to resolve things.  */
  return  fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address;
}

/* csky_cons_fix_new is called via the expression parsing code when a
   reloc is needed.  We use this hook to get the correct .got reloc.  */

void
csky_cons_fix_new (fragS *frag,
		   unsigned int off,
		   unsigned int len,
		   expressionS *exp,
		   bfd_reloc_code_real_type reloc)
{
  fixS *fixP;

  if (BFD_RELOC_CKCORE_GOTOFF == insn_reloc
      || BFD_RELOC_CKCORE_GOTPC == insn_reloc
      || BFD_RELOC_CKCORE_GOT32 == insn_reloc
      || BFD_RELOC_CKCORE_PLT32 == insn_reloc
      || BFD_RELOC_CKCORE_TLS_LE32 == insn_reloc
      || BFD_RELOC_CKCORE_TLS_GD32 == insn_reloc
      || BFD_RELOC_CKCORE_TLS_LDM32 == insn_reloc
      || BFD_RELOC_CKCORE_TLS_LDO32 == insn_reloc
      || BFD_RELOC_CKCORE_TLS_IE32 == insn_reloc)
    reloc = insn_reloc;
  else
    switch (len)
      {
      case 1:
	reloc = BFD_RELOC_8;
	break;
      case 2:
	reloc = BFD_RELOC_16;
	break;
      case 4:
	reloc = BFD_RELOC_32;
	break;
      case 8:
	reloc = BFD_RELOC_64;
	break;
      default:
	as_bad (_("unsupported BFD relocation size %d"), len);
	reloc = BFD_RELOC_32;
	break;
      }
  fixP = fix_new_exp (frag, off, (int) len, exp, 0, reloc);
  if (BFD_RELOC_CKCORE_TLS_IE32 == insn_reloc
      || BFD_RELOC_CKCORE_TLS_GD32 == insn_reloc
      || BFD_RELOC_CKCORE_TLS_LDM32 == insn_reloc)
    {
      fixP->tc_fix_data.frag = literal_insn_offset->tls_addend.frag;
      fixP->tc_fix_data.offset = literal_insn_offset->tls_addend.offset;
    }
}

/* See whether we need to force a relocation into the output file.
   This is used to force out switch and PC relative relocations when
   relaxing.  */

int
csky_force_relocation (fixS * fix)
{
  if (fix->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fix->fx_r_type == BFD_RELOC_VTABLE_ENTRY
      || fix->fx_r_type == BFD_RELOC_RVA
      || fix->fx_r_type == BFD_RELOC_CKCORE_ADDR_HI16
      || fix->fx_r_type == BFD_RELOC_CKCORE_ADDR_LO16
      || fix->fx_r_type == BFD_RELOC_CKCORE_TOFFSET_LO16
      || fix->fx_r_type == BFD_RELOC_CKCORE_DOFFSET_LO16)
    return 1;

  if (fix->fx_addsy == NULL)
    return 0;

  if (do_use_branchstub
      && fix->fx_r_type == BFD_RELOC_CKCORE_PCREL_IMM26BY2
      && (symbol_get_bfdsym (fix->fx_addsy)->flags & BSF_FUNCTION))
    return 1;
  return S_FORCE_RELOC (fix->fx_addsy, fix->fx_subsy == NULL);
}

/* Return true if the fix can be handled by GAS, false if it must
   be passed through to the linker.  */

bool
csky_fix_adjustable (fixS * fixP)
{
  if (fixP->fx_addsy == NULL)
    return 1;

  /* We need the symbol name for the VTABLE entries.  */
  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY
      || fixP->fx_r_type == BFD_RELOC_CKCORE_PLT32
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOT32
      || fixP->fx_r_type == BFD_RELOC_CKCORE_PLT12
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOT12
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOT_HI16
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOT_LO16
      || fixP->fx_r_type == BFD_RELOC_CKCORE_PLT_HI16
      || fixP->fx_r_type == BFD_RELOC_CKCORE_PLT_LO16
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOTOFF
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOTOFF_HI16
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOTOFF_LO16
      || fixP->fx_r_type == BFD_RELOC_CKCORE_ADDR_HI16
      || fixP->fx_r_type == BFD_RELOC_CKCORE_ADDR_LO16
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOT_IMM18BY4
      || fixP->fx_r_type == BFD_RELOC_CKCORE_PLT_IMM18BY4
      || fixP->fx_r_type == BFD_RELOC_CKCORE_GOTOFF_IMM18
      || fixP->fx_r_type == BFD_RELOC_CKCORE_TLS_LE32
      || fixP->fx_r_type == BFD_RELOC_CKCORE_TLS_IE32
      || fixP->fx_r_type == BFD_RELOC_CKCORE_TLS_GD32
      || fixP->fx_r_type == BFD_RELOC_CKCORE_TLS_LDM32
      || fixP->fx_r_type == BFD_RELOC_CKCORE_TLS_LDO32)
    return 0;

  if (do_use_branchstub
      && fixP->fx_r_type == BFD_RELOC_CKCORE_PCREL_IMM26BY2
      && (symbol_get_bfdsym (fixP->fx_addsy)->flags & BSF_FUNCTION))
    return 0;

  return 1;
}

void
md_apply_fix (fixS   *fixP,
	      valueT *valP,
	      segT   seg)
{
  reloc_howto_type *howto;
  /* Note: use offsetT because it is signed, valueT is unsigned.  */
  offsetT val = *valP;
  char *buf = fixP->fx_frag->fr_literal + fixP->fx_where;

  /* if fx_done = 0, fixup will also be processed in
   * tc_gen_reloc() after md_apply_fix().  */
  fixP->fx_done = 0;

  /* If the fix is relative to a symbol which is not defined, or not
     in the same segment as the fix, we cannot resolve it here.  */
  if (IS_CSKY_V1 (mach_flag) && fixP->fx_addsy != NULL
      && (! S_IS_DEFINED (fixP->fx_addsy)
	  || S_GET_SEGMENT (fixP->fx_addsy) != seg))
    {
      switch (fixP->fx_r_type)
       {
	 /* Data fx_addnumber is greater than 16 bits,
	    so fx_addnumber is assigned zero.  */
       case BFD_RELOC_CKCORE_PCREL_JSR_IMM11BY2:
	 *valP = 0;
	 break;
       case BFD_RELOC_CKCORE_TLS_IE32:
       case BFD_RELOC_CKCORE_TLS_LDM32:
       case BFD_RELOC_CKCORE_TLS_GD32:
	 {
	   struct tls_addend *ta = &(fixP->tc_fix_data);
	   fixP->fx_offset = (fixP->fx_frag->fr_address + fixP->fx_where
			      - (ta->frag->fr_address + ta->offset));
	   *valP = fixP->fx_offset;
	 }
	 /* Fall through.  */
       case BFD_RELOC_CKCORE_TLS_LE32:
       case BFD_RELOC_CKCORE_TLS_LDO32:
	 S_SET_THREAD_LOCAL (fixP->fx_addsy);
	 break;
       default:
	 break;
       }
#ifdef OBJ_ELF
      /* For ELF we can just return and let the reloc that will be generated
	 take care of everything.  For COFF we still have to insert 'val'
	 into the insn since the addend field will be ignored.  */
      return;
#endif
    }

  /* We can handle these relocs.  */
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_32_PCREL:
    case BFD_RELOC_CKCORE_PCREL32:
      fixP->fx_r_type = BFD_RELOC_CKCORE_PCREL32;
      break;
    case BFD_RELOC_VTABLE_INHERIT:
      fixP->fx_r_type = BFD_RELOC_CKCORE_GNU_VTINHERIT;
      if (fixP->fx_addsy && !S_IS_DEFINED (fixP->fx_addsy)
	  && !S_IS_WEAK (fixP->fx_addsy))
	S_SET_WEAK (fixP->fx_addsy);
      break;
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_r_type = BFD_RELOC_CKCORE_GNU_VTENTRY;
      break;
    case BFD_RELOC_CKCORE_GOT12:
    case BFD_RELOC_CKCORE_PLT12:
    case BFD_RELOC_CKCORE_ADDR_HI16:
    case BFD_RELOC_CKCORE_ADDR_LO16:
    case BFD_RELOC_CKCORE_TOFFSET_LO16:
    case BFD_RELOC_CKCORE_DOFFSET_LO16:
    case BFD_RELOC_CKCORE_GOT_HI16:
    case BFD_RELOC_CKCORE_GOT_LO16:
    case BFD_RELOC_CKCORE_PLT_HI16:
    case BFD_RELOC_CKCORE_PLT_LO16:
    case BFD_RELOC_CKCORE_GOTPC_HI16:
    case BFD_RELOC_CKCORE_GOTPC_LO16:
    case BFD_RELOC_CKCORE_GOTOFF_HI16:
    case BFD_RELOC_CKCORE_GOTOFF_LO16:
    case BFD_RELOC_CKCORE_DOFFSET_IMM18:
    case BFD_RELOC_CKCORE_DOFFSET_IMM18BY2:
    case BFD_RELOC_CKCORE_DOFFSET_IMM18BY4:
    case BFD_RELOC_CKCORE_GOTOFF_IMM18:
    case BFD_RELOC_CKCORE_GOT_IMM18BY4:
    case BFD_RELOC_CKCORE_PLT_IMM18BY4:
      break;
    case BFD_RELOC_CKCORE_TLS_IE32:
    case BFD_RELOC_CKCORE_TLS_LDM32:
    case BFD_RELOC_CKCORE_TLS_GD32:
      {
	struct tls_addend *ta = &(fixP->tc_fix_data);
	fixP->fx_offset = (fixP->fx_frag->fr_address + fixP->fx_where
			   - (ta->frag->fr_address + ta->offset));
	*valP = fixP->fx_offset;
      }
      /* Fall through.  */
    case BFD_RELOC_CKCORE_TLS_LE32:
    case BFD_RELOC_CKCORE_TLS_LDO32:
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      break;
    case BFD_RELOC_32:
      fixP->fx_r_type = BFD_RELOC_CKCORE_ADDR32;
      /* Fall through.  */
    case BFD_RELOC_16:
    case BFD_RELOC_8:
      if (fixP->fx_addsy == NULL)
	{
	  if (fixP->fx_size == 4)
	    ;
	  else if (fixP->fx_size == 2 && val >= -32768 && val <= 32767)
	    ;
	  else if (fixP->fx_size == 1 && val >= -256 && val <= 255)
	    ;
	  else
	    break;

	  md_number_to_chars (buf, val, fixP->fx_size);
	  fixP->fx_done = 1;
	}
      break;
    case BFD_RELOC_CKCORE_PCREL_JSR_IMM11BY2:
      if (fixP->fx_addsy == 0 && val > -2 KB && val < 2 KB)
	{
	  long nval = (val >> 1) & 0x7ff;
	  nval |= CSKYV1_INST_BSR;
	  csky_write_insn (buf, nval, 2);
	  fixP->fx_done = 1;
	}
      else
	*valP = 0;
      break;
    case BFD_RELOC_CKCORE_PCREL_JSR_IMM26BY2:
      if (fixP->fx_addsy == 0)
	{
	  if (val >= -(1 << 26) && val < (1 << 26))
	    {
	      unsigned int nval = ((val + fixP->fx_size) >> 1) & 0x3ffffff;
	      nval |= CSKYV2_INST_BSR32;

	      csky_write_insn (buf, nval, 4);
	    }
	  /* If bsr32 cannot reach,
	     generate 'lrw r25,label;jsr r25' instead of 'jsri label'.  */
	  else if (IS_CSKY_ARCH_810 (mach_flag))
	    {
	      howto = bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);
	      valueT opcode = csky_read_insn (buf, 4);
	      opcode = (opcode & howto->dst_mask) | CSKYV2_INST_JSRI_TO_LRW;
	      csky_write_insn (buf, opcode, 4);
	      opcode = CSKYV2_INST_JSR_R26;
	      csky_write_insn (buf + 4, opcode, 4);
	    }
	  fixP->fx_done = 1;
	}
      break;

    default:
      {
	valueT opcode;
	offsetT min, max;
	unsigned int issigned = 0;

	if (fixP->fx_addsy)
	  break;

	howto = bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);
	if (howto == NULL)
	  {
	    if (fixP->fx_size == 4
		|| (fixP->fx_size == 2 && val >= -32768 && val <= 32767)
		|| (fixP->fx_size == 1 && val >= -256 && val <= 255))
	      {
		md_number_to_chars (buf, val, fixP->fx_size);
		fixP->fx_done = 1;
		break;
	      }
	    else
	      abort ();
	  }

	if (IS_CSKY_V2 (mach_flag))
	  val += fixP->fx_size;

	if (howto->rightshift == 2)
	  val += 2;

	val >>= howto->rightshift;

	switch (fixP->fx_r_type)
	  {
	    /* Offset is unsigned.  */
	  case BFD_RELOC_CKCORE_PCREL_IMM8BY4:
	  case BFD_RELOC_CKCORE_PCREL_IMM10BY4:
	  case BFD_RELOC_CKCORE_PCREL_IMM16BY4:
	    max = (offsetT) howto->dst_mask;
	    min = 0;
	    break;
	    /* lrw16.  */
	  case BFD_RELOC_CKCORE_PCREL_IMM7BY4:
	    if (do_extend_lrw)
	      max = (offsetT)((1 << (howto->bitsize + 1)) - 2);
	    else
	      max = (offsetT)((1 << howto->bitsize) - 1);
	    min = 0;
	    break;
	    /* flrws, flrwd: the offset bits are divided in two parts.  */
	  case BFD_RELOC_CKCORE_PCREL_FLRW_IMM8BY4:
	    max = (offsetT)((1 << howto->bitsize) - 1);
	    min = 0;
	    break;
	    /* Offset is signed.  */
	  default:
	    max = (offsetT)(howto->dst_mask >> 1);
	    min = - max - 1;
	    issigned = 1;
	  }
	if (val < min || val > max)
	  {
	    csky_branch_report_error (fixP->fx_file, fixP->fx_line,
				      fixP->fx_addsy, val);
	    return;
	  }
	opcode = csky_read_insn (buf, fixP->fx_size);
	/* Clear redundant bits brought from the last
	   operation if there is any.  */
	if (do_extend_lrw && (opcode & 0xfc00) == CSKYV2_INST_LRW16)
	  val &= 0xff;
	else
	  val &= issigned ? (offsetT)(howto->dst_mask) : max;

	if (fixP->fx_r_type == BFD_RELOC_CKCORE_PCREL_BLOOP_IMM4BY4)
	  val = (val & 0xf) << 12;

	if (fixP->fx_size == 2  && (opcode & 0xfc00) == CSKYV2_INST_LRW16)
	  {
	    /* 8 bit offset lrw16.  */
	    if (val >= 0x80)
	      csky_write_insn (buf,
			       ((~val & 0x1f)
				| ((~val & 0x60) << 3) | (opcode & 0xe0)),
			       fixP->fx_size);
	    /* 7 bit offset lrw16.  */
	    else
	      csky_write_insn (buf,
			       (val & 0x1f) | ((val & 0x60) << 3) | opcode,
			       fixP->fx_size);
	  }
	else if (fixP->fx_size == 4
		 && (opcode & 0xfe1ffe00) == CSKYV2_INST_FLRW)
	  csky_write_insn (buf,
			   ((val & 0xf) << 4) | ((val & 0xf0) << 17) | opcode,
			   fixP->fx_size);
	else
	  csky_write_insn (buf, val | opcode, fixP->fx_size);
	fixP->fx_done = 1;
	break;
      }
    }
  fixP->fx_addnumber = val;
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixP)
{
  arelent *rel;

  if (fixP->fx_pcrel
      && fixP->fx_r_type == BFD_RELOC_CKCORE_ADDR32)
      fixP->fx_r_type = BFD_RELOC_CKCORE_PCREL32;

  rel = xmalloc (sizeof (arelent));
  rel->sym_ptr_ptr = xmalloc (sizeof (asymbol *));
  *rel->sym_ptr_ptr = symbol_get_bfdsym (fixP->fx_addsy);
  rel->howto = bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);
  rel->addend = fixP->fx_offset;
  if (rel->howto == NULL)
    {
      as_bad_where (fixP->fx_file, fixP->fx_line,
		    _("cannot represent `%s' relocation in object file"),
		    bfd_get_reloc_code_name (fixP->fx_r_type));

      /* Set howto to a garbage value so that we can keep going.  */
      rel->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_32);
    }
  gas_assert (rel->howto != NULL);
  rel->address = fixP->fx_frag->fr_address + fixP->fx_where;
  return rel;
}

/* Relax a fragment by scanning TC_GENERIC_RELAX_TABLE.  */

long
csky_relax_frag (segT segment, fragS *fragP, long stretch)
{
  const relax_typeS *this_type;
  const relax_typeS *start_type;
  relax_substateT next_state;
  relax_substateT this_state;
  offsetT growth;
  offsetT aim;
  addressT target;
  addressT address;
  symbolS *symbolP;
  const relax_typeS *table;

  target = fragP->fr_offset;
  address = fragP->fr_address;
  table = TC_GENERIC_RELAX_TABLE;
  this_state = fragP->fr_subtype;
  start_type = this_type = table + this_state;
  symbolP = fragP->fr_symbol;

  if (symbolP)
    {
      fragS *sym_frag;

      sym_frag = symbol_get_frag (symbolP);

#ifndef DIFF_EXPR_OK
      know (sym_frag != NULL);
#endif
      know (S_GET_SEGMENT (symbolP) != absolute_section
	    || sym_frag == &zero_address_frag);
      target += S_GET_VALUE (symbolP);

      /* If SYM_FRAG has yet to be reached on this pass, assume it
	 will move by STRETCH just as we did, unless there is an
	 alignment frag between here and SYM_FRAG.  An alignment may
	 well absorb any STRETCH, and we don't want to choose a larger
	 branch insn by overestimating the needed reach of this
	 branch.  It isn't critical to calculate TARGET exactly;  We
	 know we'll be doing another pass if STRETCH is non-zero.  */

      if (stretch != 0
	  && sym_frag->relax_marker != fragP->relax_marker
	  && S_GET_SEGMENT (symbolP) == segment)
	{
	  fragS *f;

	  /* Adjust stretch for any alignment frag.  Note that if have
	     been expanding the earlier code, the symbol may be
	     defined in what appears to be an earlier frag.  FIXME:
	     This doesn't handle the fr_subtype field, which specifies
	     a maximum number of bytes to skip when doing an
	     alignment.  */
	  for (f = fragP; f != NULL && f != sym_frag; f = f->fr_next)
	    {
	      if (f->fr_type == rs_align || f->fr_type == rs_align_code)
		{
		  if (stretch < 0)
		    stretch = -((-stretch)
				& ~((1 << (int) f->fr_offset) - 1));
		  else
		    stretch &= ~((1 << (int) f->fr_offset) - 1);
		}
	      if (stretch == 0)
		break;
	    }
	  if (f != 0)
	    target += stretch;
	}
    }

  aim = target - address - fragP->fr_fix;

  /* If the fragP->fr_symbol is extern symbol, aim should be 0.  */
  if (fragP->fr_symbol && S_GET_SEGMENT (symbolP) != segment)
    aim = 0;

  if (aim < 0)
    {
      /* Look backwards.  */
      for (next_state = this_type->rlx_more; next_state;)
	if (aim >= this_type->rlx_backward)
	  next_state = 0;
	else
	  {
	    /* Grow to next state.  */
	    this_state = next_state;
	    this_type = table + this_state;
	    next_state = this_type->rlx_more;
	  }
    }
  else
    {
      /* Look forwards.  */
      for (next_state = this_type->rlx_more; next_state;)
	if (aim <= this_type->rlx_forward)
	  next_state = 0;
	else
	  {
	    /* Grow to next state.  */
	    this_state = next_state;
	    this_type = table + this_state;
	    next_state = this_type->rlx_more;
	  }
    }

  growth = this_type->rlx_length - start_type->rlx_length;
  if (growth != 0)
    fragP->fr_subtype = this_state;
  return growth;
}

int
md_estimate_size_before_relax (fragS * fragp,
			       segT  segtype)
{
  switch (fragp->fr_subtype)
    {
    case COND_DISP10:
    case COND_DISP16:
    case SCOND_DISP10:
    case SCOND_DISP16:
    case UNCD_DISP10:
    case UNCD_DISP16:
    case JCOND_DISP10:
    case JCOND_DISP16:
    case JCOND_DISP32:
    case JUNCD_DISP10:
    case JUNCD_DISP16:
    case JUNCD_DISP32:
    case JCOMPZ_DISP16:
    case JCOMPZ_DISP32:
    case BSR_DISP26:
    case LRW_DISP7:
    case LRW2_DISP8:
    case LRW_DISP16:
      gas_assert (fragp->fr_symbol);
      if (IS_EXTERNAL_SYM (fragp->fr_symbol, segtype))
	while (csky_relax_table[fragp->fr_subtype].rlx_more > RELAX_OVERFLOW)
	  fragp->fr_subtype = csky_relax_table[fragp->fr_subtype].rlx_more;
      return csky_relax_table[fragp->fr_subtype].rlx_length;

      /* C-SKY V1 relaxes.  */
    case C (UNCD_JUMP, UNDEF_DISP):
    case C (UNCD_JUMP_PIC, UNDEF_DISP):
      if (!fragp->fr_symbol)
	fragp->fr_subtype = C (UNCD_JUMP_S, DISP12);
      else if (S_GET_SEGMENT (fragp->fr_symbol) == segtype)
	fragp->fr_subtype = C (UNCD_JUMP_S, DISP12);
      else
	fragp->fr_subtype = C (UNCD_JUMP_S, UNDEF_WORD_DISP);
      break;

    case C (COND_JUMP, UNDEF_DISP):
    case C (COND_JUMP_PIC, UNDEF_DISP):
      if (fragp->fr_symbol
	  && S_GET_SEGMENT (fragp->fr_symbol) == segtype)
	/* Got a symbol and it's defined in this segment, become byte
	   sized. Maybe it will fix up.  */
	fragp->fr_subtype = C (COND_JUMP_S, DISP12);
      else if (fragp->fr_symbol)
	/* It's got a segment, but it's not ours, so it will always be
	   long.  */
	fragp->fr_subtype = C (COND_JUMP_S, UNDEF_WORD_DISP);
      else
	/* We know the abs value.  */
	fragp->fr_subtype = C (COND_JUMP_S, DISP12);
      break;

    case C (UNCD_JUMP, DISP12):
    case C (UNCD_JUMP, DISP32):
    case C (UNCD_JUMP, UNDEF_WORD_DISP):
    case C (COND_JUMP, DISP12):
    case C (COND_JUMP, DISP32):
    case C (COND_JUMP, UNDEF_WORD_DISP):
    case C (UNCD_JUMP_PIC, DISP12):
    case C (UNCD_JUMP_PIC, DISP32):
    case C (UNCD_JUMP_PIC, UNDEF_WORD_DISP):
    case C (COND_JUMP_PIC, DISP12):
    case C (COND_JUMP_PIC, DISP32):
    case C (COND_JUMP_PIC, UNDEF_WORD_DISP):
    case RELAX_OVERFLOW:
      break;

    default:
      abort ();
    }
  return csky_relax_table[fragp->fr_subtype].rlx_length;
}

/* Parse opcode like: "op oprnd1, oprnd2, oprnd3".  */

static void
csky_macro_md_assemble (const char *op,
			const char *oprnd1,
			const char *oprnd2,
			const char *oprnd3)
{
  char str[80];
  str[0] = '\0';
  strcat (str, op);
  if (oprnd1 != NULL)
    {
      strcat (str, " ");
      strcat (str, oprnd1);
      if (oprnd2 != NULL)
	{
	  strcat (str, ",");
	  strcat (str, oprnd2);
	  if (oprnd3 != NULL)
	    {
	      strcat (str, ",");
	      strcat (str, oprnd3);
	    }
	}
    }
  md_assemble (str);
  return;
}

/* Get the string of operand.  */

static int
csky_get_macro_operand (char *src_s, char *dst_s, char end_sym)
{
  int nlen = 0;
  while (ISSPACE (*src_s))
    ++src_s;
  while (*src_s != end_sym)
    dst_s[nlen++] = *(src_s++);
  dst_s[nlen] = '\0';
  return nlen;
}

/* idly 4 -> idly4.  */

static void
csky_idly (void)
{
  char *s = csky_insn.opcode_end;
  if (!is_imm_within_range (&s, 4, 4))
    {
      as_bad (_("second operand must be 4"));
      return;
    }
  csky_macro_md_assemble ("idly4", NULL, NULL, NULL);
  return;
}

/* rolc rd, 1 or roltc rd, 1 -> addc rd, rd.  */

static void
csky_rolc (void)
{
  char reg[10];
  char *s = csky_insn.opcode_end;

  s += csky_get_macro_operand (s, reg, ',');
  ++s;

  if (is_imm_within_range (&s, 1, 1))
    {
      csky_macro_md_assemble ("addc", reg, reg, NULL);
      return;
    }
  else
    as_bad (_("second operand must be 1"));
}

/* sxtrb0(1)(2) r1, rx -> xtbr0(1)(2) r1,rx; sextb r1.  */

static void
csky_sxtrb (void)
{
  char reg1[10];
  char reg2[10];

  char *s = csky_insn.opcode_end;
  s += csky_get_macro_operand (s, reg1, ',');
  ++s;
  csky_get_macro_operand (s, reg2, '\0');

  csky_macro_md_assemble (csky_insn.macro->name + 1, reg1, reg2, NULL);
  csky_macro_md_assemble ("sextb", reg1, NULL, NULL);
  return;
}

static void
csky_movtf (void)
{
  char reg1[10];
  char reg2[10];
  char reg3[10];

  char *s = csky_insn.opcode_end;
  s += csky_get_macro_operand (s, reg1, ',');
  ++s;

  s += csky_get_macro_operand (s, reg2, ',');
  ++s;

  s += csky_get_macro_operand (s, reg3, '\0');
  ++s;
  csky_macro_md_assemble ("movt", reg1, reg2, NULL);
  csky_macro_md_assemble ("movf", reg1, reg3, NULL);
  return;
}

static bool
get_macro_reg_vals (int *reg1, int *reg2, int *reg3)
{
  int nlen;
  char *s = csky_insn.opcode_end;

  *reg1 = csky_get_reg_val (s, &nlen);
  s += nlen;
  if (*s != ',')
    {
      csky_show_error (ERROR_MISSING_COMMA, 0, NULL, NULL);
      return false;
    }
  s++;
  *reg2 = csky_get_reg_val (s, &nlen);
  s += nlen;
  if (*s != ',')
    {
      csky_show_error (ERROR_MISSING_COMMA, 0, NULL, NULL);
      return false;
    }
  s++;
  *reg3 = csky_get_reg_val (s, &nlen);
  s += nlen;
  if (*s != '\0')
    {
      csky_show_error (ERROR_BAD_END, 0, s, NULL);
      return false;
    }
  if (*reg1 == -1 || *reg2 == -1 || *reg3 == -1)
    {
      as_bad (_("register number out of range"));
      return false;
    }
  if (*reg1 != *reg2)
    {
      as_bad (_("dest and source1 must be the same register"));
      return false;
    }
  if (*reg1 >= 15 || *reg3 >= 15)
    {
      as_bad (_("64-bit operator src/dst register must be less than 15"));
      return false;
    }
  return true;
}

/* addc64 rx, rx, ry -> cmplt rx, rx, addc  rx, ry, addc  rx+1, ry+1.  */

static void
csky_addc64 (void)
{
  int reg1;
  int reg2;
  int reg3;
  char reg1_name[16] = {0};
  char reg3_name[16] = {0};

  if (!get_macro_reg_vals (&reg1, &reg2, &reg3))
    return;

  sprintf (reg1_name, "r%d", reg1);
  csky_macro_md_assemble ("cmplt", reg1_name, reg1_name, NULL);
  if (error_state.err_num != ERROR_NONE)
    return;

  sprintf (reg1_name, "r%d", reg1 + (target_big_endian ? 1 : 0));
  sprintf (reg3_name, "r%d", reg3 + (target_big_endian ? 1 : 0));
  csky_macro_md_assemble ("addc", reg1_name, reg3_name, NULL);
  if (error_state.err_num != ERROR_NONE)
    return;

  sprintf (reg1_name, "r%d", reg1 + (target_big_endian ? 0 : 1));
  sprintf (reg3_name, "r%d", reg3 + (target_big_endian ? 0 : 1));
  csky_macro_md_assemble ("addc", reg1_name, reg3_name, NULL);
  return;
}

/* subc64 rx, rx, ry -> cmphs rx, rx, subc  rx, ry, subc  rx+1, ry+1.  */

static void
csky_subc64 (void)
{
  int reg1;
  int reg2;
  int reg3;
  char reg1_name[16] = {0};
  char reg3_name[16] = {0};

  if (!get_macro_reg_vals (&reg1, &reg2, &reg3))
    return;

  sprintf (reg1_name, "r%d", reg1);
  csky_macro_md_assemble ("cmphs", reg1_name, reg1_name, NULL);
  if (error_state.err_num != ERROR_NONE)
    return;

  sprintf (reg1_name, "r%d", reg1 + (target_big_endian ? 1 : 0));
  sprintf (reg3_name, "r%d", reg3 + (target_big_endian ? 1 : 0));
  csky_macro_md_assemble ("subc", reg1_name, reg3_name, NULL);
  if (error_state.err_num != ERROR_NONE)
    return;

  sprintf (reg1_name, "r%d", reg1 + (target_big_endian ? 0 : 1));
  sprintf (reg3_name, "r%d", reg3 + (target_big_endian ? 0 : 1));
  csky_macro_md_assemble ("subc", reg1_name, reg3_name, NULL);
  return;
}

/* or64 rx, rx, ry -> or rx, ry, or rx+1, ry+1.  */

static void
csky_or64 (void)
{
  int reg1;
  int reg2;
  int reg3;
  char reg1_name[16] = {0};
  char reg3_name[16] = {0};

  if (!get_macro_reg_vals (&reg1, &reg2, &reg3))
    return;
  sprintf (reg1_name, "r%d", reg1 + (target_big_endian ? 1 : 0));
  sprintf (reg3_name, "r%d", reg3 + (target_big_endian ? 1 : 0));
  csky_macro_md_assemble ("or", reg1_name, reg3_name, NULL);

  if (error_state.err_num != ERROR_NONE)
    return;
  sprintf (reg1_name, "r%d", reg1 + (target_big_endian ? 0 : 1));
  sprintf (reg3_name, "r%d", reg3 + (target_big_endian ? 0 : 1));
  csky_macro_md_assemble ("or", reg1_name, reg3_name, NULL);
  return;
}

/* xor64 rx, rx, ry -> xor rx, ry, xor rx+1, ry+1.  */

static void
csky_xor64 (void)
{
  int reg1;
  int reg2;
  int reg3;
  char reg1_name[16] = {0};
  char reg3_name[16] = {0};

  if (!get_macro_reg_vals (&reg1, &reg2, &reg3))
    return;

  sprintf (reg1_name, "r%d", reg1 + (target_big_endian ? 1 : 0));
  sprintf (reg3_name, "r%d", reg3 + (target_big_endian ? 1 : 0));
  csky_macro_md_assemble ("xor", reg1_name, reg3_name, NULL);
  if (error_state.err_num != ERROR_NONE)
    return;

  sprintf (reg1_name, "r%d", reg1 + (target_big_endian ? 0 : 1));
  sprintf (reg3_name, "r%d", reg3 + (target_big_endian ? 0 : 1));
  csky_macro_md_assemble ("xor", reg1_name, reg3_name, NULL);
  return;
}

/* The following are V2 macro instructions.  */

/* neg rd -> not rd, rd; addi rd, 1.  */

static void
csky_neg (void)
{
  char reg1[10];

  char *s = csky_insn.opcode_end;
  s += csky_get_macro_operand (s, reg1, '\0');
  ++s;

  csky_macro_md_assemble ("not", reg1, reg1, NULL);
  csky_macro_md_assemble ("addi", reg1, "1", NULL);
  return;
}

/* rsubi rd, imm16 -> not rd; addi rd, imm16 + 1  */

static void
csky_rsubi (void)
{
  char reg1[10];
  char str_imm16[20];
  unsigned int imm16 = 0;
  expressionS e;
  char *s = csky_insn.opcode_end;
  s += csky_get_macro_operand (s, reg1, ',');
  ++s;

  s = parse_exp (s, &e);
  if (e.X_op == O_constant)
    imm16 = e.X_add_number;
  else
    csky_show_error (ERROR_IMM_ILLEGAL, 2, NULL, NULL);

  sprintf (str_imm16, "%d", imm16 + 1);

  csky_macro_md_assemble ("not", reg1, reg1, NULL);
  csky_macro_md_assemble ("addi", reg1, str_imm16, NULL);
  return;
}

/* Such as: asrc rd -> asrc rd, rd, 1.  */

static void
csky_arith (void)
{
  char reg1[10];
  char *s = csky_insn.opcode_end;
  s += csky_get_macro_operand (s, reg1, '\0');
  ++s;
  csky_macro_md_assemble (csky_insn.macro->name, reg1, reg1, "1");
  return;
}

/* decne rd ->  if ck802: subi rd, 1; cmpnei rd, 0.
   else: decne rd, rd, 1  */

static void
csky_decne (void)
{
  char reg1[10];
  char *s = csky_insn.opcode_end;
  s += csky_get_macro_operand (s, reg1, '\0');
  ++s;
  if (IS_CSKY_ARCH_802 (mach_flag))
    {
      csky_macro_md_assemble ("subi", reg1, "1", NULL);
      csky_macro_md_assemble ("cmpnei", reg1, "0", NULL);
    }
  else
    csky_macro_md_assemble ("decne", reg1, reg1, "1");
  return;
}

/* If -mnolrw, lrw rd, imm -> movih rd, imm_hi16; ori rd, imm_lo16.  */

static void
csky_lrw (void)
{
  char reg1[10];
  char imm[40];
  char imm_hi16[40];
  char imm_lo16[40];

  char *s = csky_insn.opcode_end;
  s += csky_get_macro_operand (s, reg1, ',');
  ++s;
  s += csky_get_macro_operand (s, imm, '\0');
  ++s;

  imm_hi16[0] = '\0';
  strcat (imm_hi16, "(");
  strcat (imm_hi16, imm);
  strcat (imm_hi16, ") >> 16");
  imm_lo16[0] = '\0';
  strcat (imm_lo16, "(");
  strcat (imm_lo16, imm);
  strcat (imm_lo16, ") & 0xffff");

  csky_macro_md_assemble ("movih", reg1, imm_hi16,  NULL);
  csky_macro_md_assemble ("ori", reg1, reg1, imm_lo16);

  return;
}

/* The following are worker functions for C-SKY v1.  */

bool
v1_work_lrw (void)
{
  int reg;
  int output_literal = csky_insn.val[1];

  reg = csky_insn.val[0];
  csky_insn.isize = 2;
  csky_insn.output = frag_more (2);
  if (csky_insn.e1.X_op == O_constant
      && csky_insn.e1.X_add_number <= 0x7f
      && csky_insn.e1.X_add_number >= 0)
    /* lrw to movi.  */
    csky_insn.inst = 0x6000 | reg | (csky_insn.e1.X_add_number << 4);
  else
    {
      csky_insn.inst = csky_insn.opcode->op16[0].opcode;
      csky_insn.inst |= reg << 8;
      if (output_literal)
	{
	  struct literal *p = enter_literal (&csky_insn.e1, 0, 0, 0);

	  /* Create a reference to pool entry.  */
	  csky_insn.e1.X_op = O_symbol;
	  csky_insn.e1.X_add_symbol = poolsym;
	  csky_insn.e1.X_add_number = p->offset << 2;
	}

      if (insn_reloc == BFD_RELOC_CKCORE_TLS_GD32
	  || insn_reloc == BFD_RELOC_CKCORE_TLS_LDM32
	  || insn_reloc == BFD_RELOC_CKCORE_TLS_IE32)
	{
	  literal_insn_offset->tls_addend.frag  = frag_now;
	  literal_insn_offset->tls_addend.offset
	    = (csky_insn.output
	       - literal_insn_offset->tls_addend.frag->fr_literal);
	}
      fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal, 2,
		   &csky_insn.e1, 1, BFD_RELOC_CKCORE_PCREL_IMM8BY4);
    }
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);

  return true;
}

bool
v1_work_fpu_fo (void)
{
  int i = 0;
  int inst;
  int greg = -1;
  char buff[50];
  struct csky_opcode_info *opinfo = NULL;

  if (csky_insn.isize == 4)
    opinfo = &csky_insn.opcode->op32[csky_insn.opcode_idx];
  else if (csky_insn.isize == 2)
    opinfo = &csky_insn.opcode->op16[csky_insn.opcode_idx];

  /* Firstly, get general reg.  */
  for (i = 0;i < opinfo->operand_num; i++)
    if (opinfo->oprnd.oprnds[i].type == OPRND_TYPE_GREG0_15)
      greg = csky_insn.val[i];
  gas_assert (greg != -1);

  /* Secondly, get float inst.  */
  csky_generate_insn ();
  inst = csky_insn.inst;

  /* Now get greg and inst, we can write instruction to floating unit.  */
  sprintf (buff, "lrw r%d,0x%x", greg, inst);
  md_assemble (buff);
  sprintf (buff, "cpwir r%d", greg);
  md_assemble (buff);
  return false;
}

bool
v1_work_fpu_fo_fc (void)
{
  int i = 0;
  int inst;
  int greg = -1;
  char buff[50];
  struct csky_opcode_info *opinfo = NULL;

  if (csky_insn.isize == 4)
    opinfo = &csky_insn.opcode->op32[csky_insn.opcode_idx];
  else if (csky_insn.isize == 2)
    opinfo = &csky_insn.opcode->op16[csky_insn.opcode_idx];

  /* Firstly, get general reg.  */
  for (i = 0;i < opinfo->operand_num; i++)
    if (opinfo->oprnd.oprnds[i].type == OPRND_TYPE_GREG0_15)
      greg = csky_insn.val[i];
  gas_assert (greg != -1);

  /* Secondly, get float inst.  */
  csky_generate_insn ();
  inst = csky_insn.inst;

  /* Now get greg and inst, we can write instruction to floating unit.  */
  sprintf (buff, "lrw r%d,0x%x", greg, inst);
  md_assemble (buff);
  sprintf (buff, "cpwir r%d", greg);
  md_assemble (buff);
  sprintf (buff, "cprc");
  md_assemble (buff);

  return false;
}

bool
v1_work_fpu_write (void)
{
  int greg;
  int freg;
  char buff[50];

  greg = csky_insn.val[0];
  freg = csky_insn.val[1];

  /* Now get greg and freg, we can write instruction to floating unit.  */
  sprintf (buff, "cpwgr r%d,cpr%d", greg, freg);
  md_assemble (buff);

  return false;
}

bool
v1_work_fpu_read (void)
{
  int greg;
  int freg;
  char buff[50];

  greg = csky_insn.val[0];
  freg = csky_insn.val[1];
  /* Now get greg and freg, we can write instruction to floating unit.  */
  sprintf (buff, "cprgr r%d,cpr%d", greg, freg);
  md_assemble (buff);

  return false;
}

bool
v1_work_fpu_writed (void)
{
  int greg;
  int freg;
  char buff[50];

  greg = csky_insn.val[0];
  freg = csky_insn.val[1];

  if (greg & 0x1)
    {
      as_bad (_("even register number required"));
      return false;
    }
  /* Now get greg and freg, we can write instruction to floating unit.  */
  if (target_big_endian)
    sprintf (buff, "cpwgr r%d,cpr%d", greg + 1, freg);
  else
    sprintf (buff, "cpwgr r%d,cpr%d", greg, freg);
  md_assemble (buff);
  if (target_big_endian)
    sprintf (buff, "cpwgr r%d,cpr%d", greg, freg + 1);
  else
    sprintf (buff, "cpwgr r%d,cpr%d", greg+1, freg + 1);
  md_assemble (buff);
  return false;
}

bool
v1_work_fpu_readd (void)
{
  int greg;
  int freg;
  char buff[50];

  greg = csky_insn.val[0];
  freg = csky_insn.val[1];

  if (greg & 0x1)
    {
      as_bad (_("even register number required"));
      return false;
    }
  /* Now get greg and freg, we can write instruction to floating unit.  */
  if (target_big_endian)
    sprintf (buff, "cprgr r%d,cpr%d", greg+1, freg);
  else
    sprintf (buff, "cprgr r%d,cpr%d", greg, freg);
  md_assemble (buff);
  if (target_big_endian)
    sprintf (buff, "cprgr r%d,cpr%d", greg, freg + 1);
  else
    sprintf (buff, "cprgr r%d,cpr%d", greg+1, freg + 1);
  md_assemble (buff);

  return false;
}

/* The following are for csky pseudo handling.  */

bool
v1_work_jbsr (void)
{
  csky_insn.output = frag_more (2);
  if (do_force2bsr)
    /* Generate fixup BFD_RELOC_CKCORE_PCREL_IMM11BY2.  */
    fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		 2, & csky_insn.e1, 1, BFD_RELOC_CKCORE_PCREL_IMM11BY2);
  else
    {
      /* Using jsri instruction.  */
      const char *name = "jsri";
      csky_insn.opcode = (struct csky_opcode *)
	str_hash_find (csky_opcodes_hash, name);
      csky_insn.opcode_idx = 0;
      csky_insn.isize = 2;

      struct literal *p = enter_literal (&csky_insn.e1, 1, 0, 0);

      /* Create a reference to pool entry.  */
      csky_insn.e1.X_op = O_symbol;
      csky_insn.e1.X_add_symbol = poolsym;
      csky_insn.e1.X_add_number = p->offset << 2;

      /* Generate fixup BFD_RELOC_CKCORE_PCREL_IMM8BY4.  */
      fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		   2, & csky_insn.e1, 1, BFD_RELOC_CKCORE_PCREL_IMM8BY4);

      if (csky_insn.e1.X_op != O_absent && do_jsri2bsr)
	/* Generate fixup BFD_RELOC_CKCORE_PCREL_JSR_IMM11BY2.  */
	fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		     2, &p->e,
		     1, BFD_RELOC_CKCORE_PCREL_JSR_IMM11BY2);
    }
  csky_generate_insn ();

  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);

  return true;
}

/* The following are worker functions for csky v2 instruction handling.  */

/* For nie/nir/ipush/ipop.  */

bool
v2_work_istack (void)
{
  if (!do_intr_stack)
    {
      csky_show_error (ERROR_OPCODE_ILLEGAL, 0, NULL, NULL);
      return false;
    }
  csky_insn.output = frag_more (csky_insn.isize);
  csky_insn.inst = csky_insn.opcode->op16[0].opcode;
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
v2_work_btsti (void)
{
  if (!do_extend_lrw
      && (csky_insn.flag_force == INSN_OPCODE16F
	  || IS_CSKY_ARCH_801 (mach_flag)))
    {
      csky_show_error (ERROR_OPCODE_ILLEGAL, 0, NULL, NULL);
      return false;
    }
  if (!do_extend_lrw && csky_insn.isize == 2)
    csky_insn.isize = 4;
  /* Generate relax or reloc if necessary.  */
  csky_generate_frags ();
  /* Generate the insn by mask.  */
  csky_generate_insn ();
  /* Write inst to frag.  */
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
v2_work_addi (void)
{
  csky_insn.isize = 2;
  if (csky_insn.number == 2)
    {
      if (csky_insn.val[0] == 14
	  && csky_insn.val[1] >= 0 && csky_insn.val[1] <= 0x1fc
	  && (csky_insn.val[1] & 0x3) == 0
	  && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  /* addi sp, sp, imm.  */
	  csky_insn.inst = 0x1400 | ((csky_insn.val[1] >> 2) & 0x1f);
	  csky_insn.inst |= (csky_insn.val[1] << 1) & 0x300;
	  csky_insn.output = frag_more (2);
	}
      else if (csky_insn.val[0] < 8
	       && csky_insn.val[1] >= 1 && csky_insn.val[1] <= 0x100
	       && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x2000 | (csky_insn.val[0] << 8);
	  csky_insn.inst |=  (csky_insn.val[1] - 1);
	  csky_insn.output = frag_more (2);
	}
      else if (csky_insn.val[1] >= 1 && csky_insn.val[1] <= 0x10000
	       && csky_insn.flag_force != INSN_OPCODE16F
	       && !IS_CSKY_ARCH_801 (mach_flag))
	{
	  csky_insn.inst = 0xe4000000 | (csky_insn.val[0] << 21);
	  csky_insn.inst |= csky_insn.val[0] << 16;
	  csky_insn.inst |= (csky_insn.val[1] - 1);
	  csky_insn.isize = 4;
	  csky_insn.output = frag_more (4);
	}
      else
	{
	  csky_show_error (ERROR_OPERANDS_ILLEGAL, 0,
			   csky_insn.opcode_end, NULL);
	  return false;
	}
    }
  else if (csky_insn.number == 3)
    {
      if (csky_insn.val[0] == 14
	  && csky_insn.val[1] == 14
	  && csky_insn.val[2] >= 0 && csky_insn.val[2] <= 0x1fc
	  && (csky_insn.val[2] & 0x3) == 0
	  && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x1400 | ((csky_insn.val[2] >> 2) & 0x1f);
	  csky_insn.inst |= (csky_insn.val[2] << 1) & 0x300;
	  csky_insn.output = frag_more (2);
	}
      else if (csky_insn.val[0] < 8
	       && csky_insn.val[1] == 14
	       && csky_insn.val[2] >= 0 && csky_insn.val[2] <= 0x3fc
	       && (csky_insn.val[2] & 0x3) == 0
	       && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x1800 | (csky_insn.val[0] << 8);
	  csky_insn.inst |= csky_insn.val[2] >> 2;
	  csky_insn.output = frag_more (2);
	}
      else if (csky_insn.val[0] < 8
	       && csky_insn.val[0] == csky_insn.val[1]
	       && csky_insn.val[2] >= 1 && csky_insn.val[2] <= 0x100
	       && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x2000 | (csky_insn.val[0] << 8);
	  csky_insn.inst |=  (csky_insn.val[2] - 1);
	  csky_insn.output = frag_more (2);
	}
      else if (csky_insn.val[0] < 8
	       && csky_insn.val[1] < 8
	       && csky_insn.val[2] >= 1 && csky_insn.val[2] <= 0x8
	       && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x5802 | (csky_insn.val[0] << 5);
	  csky_insn.inst |= csky_insn.val[1] << 8;
	  csky_insn.inst |= (csky_insn.val[2] - 1) << 2;
	  csky_insn.output = frag_more (2);
	}
      else if (csky_insn.val[1] == 28
	       && csky_insn.val[2] >= 1 && csky_insn.val[2] <= 0x40000
	       && csky_insn.flag_force != INSN_OPCODE16F
	       && !IS_CSKY_ARCH_801 (mach_flag))
	{
	  csky_insn.inst = 0xcc1c0000 | (csky_insn.val[0] << 21);
	  csky_insn.isize = 4;
	  csky_insn.output = frag_more (4);
	  if (insn_reloc == BFD_RELOC_CKCORE_GOTOFF)
	    {
	      fix_new_exp (frag_now, csky_insn.output-frag_now->fr_literal,
			   4, &csky_insn.e1, 0, BFD_RELOC_CKCORE_GOTOFF_IMM18);
	    }
	  else
	    csky_insn.inst |= (csky_insn.val[2] - 1);
	}
      else if (csky_insn.val[2] >= 1 && csky_insn.val[2] <= 0x1000
	       && csky_insn.flag_force != INSN_OPCODE16F
	       && !IS_CSKY_ARCH_801 (mach_flag))
	{
	  csky_insn.inst = 0xe4000000 | (csky_insn.val[0] << 21);
	  csky_insn.inst |= csky_insn.val[1] << 16;
	  csky_insn.inst |= (csky_insn.val[2] - 1);
	  csky_insn.isize = 4;
	  csky_insn.output = frag_more (4);
	}
      else
	{
	  csky_show_error (ERROR_OPERANDS_ILLEGAL, 0,
			   (char *)csky_insn.opcode_end, NULL);
	  return false;
	}
    }
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);

  return true;
}

bool
v2_work_subi (void)
{
  csky_insn.isize = 2;
  if (csky_insn.number == 2)
    {
      if (csky_insn.val[0] == 14
	  && csky_insn.val[1] >= 0 && csky_insn.val[2] <= 0x1fc
	  && (csky_insn.val[1] & 0x3) == 0
	  && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x1420 | ((csky_insn.val[1] >> 2) & 0x1f);
	  csky_insn.inst |= (csky_insn.val[1] << 1) & 0x300;
	}
      else if (csky_insn.val[0] < 8
	       && csky_insn.val[1] >= 1 && csky_insn.val[1] <= 0x100
	       && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x2800 | (csky_insn.val[0] << 8);
	  csky_insn.inst |=  (csky_insn.val[1] - 1);
	}
      else if (csky_insn.val[1] >= 1 && csky_insn.val[1] <= 0x10000
	       && csky_insn.flag_force != INSN_OPCODE16F
	       && !IS_CSKY_ARCH_801 (mach_flag))
	{
	  csky_insn.inst = 0xe4001000 | (csky_insn.val[0] << 21);
	  csky_insn.inst |= csky_insn.val[0] << 16;
	  csky_insn.inst |= (csky_insn.val[1] - 1);
	  csky_insn.isize = 4;
	}
      else
	{
	  csky_show_error (ERROR_OPERANDS_ILLEGAL, 0,
			   (char *)csky_insn.opcode_end, NULL);
	  return false;
	}
    }
  else if (csky_insn.number == 3)
    {
      if (csky_insn.val[0] == 14
	  && csky_insn.val[1] == 14
	  && csky_insn.val[2] >= 0 && csky_insn.val[2] <= 0x1fc
	  && (csky_insn.val[2] & 0x3) == 0
	  && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x1420 | ((csky_insn.val[2] >> 2) & 0x1f);
	  csky_insn.inst |= (csky_insn.val[2] << 1) & 0x300;
	}

      else if (csky_insn.val[0] < 8
	       && csky_insn.val[0] == csky_insn.val[1]
	       && csky_insn.val[2] >= 1 && csky_insn.val[2] <= 0x100
	       && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x2800 | (csky_insn.val[0] << 8);
	  csky_insn.inst |=  (csky_insn.val[2] - 1);
	}
      else if (csky_insn.val[0] < 8
	       && csky_insn.val[1] < 8
	       && csky_insn.val[2] >= 1 && csky_insn.val[2] <= 0x8
	       && csky_insn.flag_force != INSN_OPCODE32F)
	{
	  csky_insn.inst = 0x5803 | (csky_insn.val[0] << 5);
	  csky_insn.inst |= csky_insn.val[1] << 8;
	  csky_insn.inst |= (csky_insn.val[2] - 1) << 2;
	}
      else if (csky_insn.val[2] >= 1 && csky_insn.val[2] <= 0x1000
	       && csky_insn.flag_force != INSN_OPCODE16F
	       && !IS_CSKY_ARCH_801 (mach_flag))
	{
	  csky_insn.inst = 0xe4001000 | (csky_insn.val[0] << 21);
	  csky_insn.inst |= csky_insn.val[1] << 16;
	  csky_insn.inst |= (csky_insn.val[2] - 1);
	  csky_insn.isize = 4;
	}
      else
	{
	  csky_show_error (ERROR_OPERANDS_ILLEGAL, 0,
			   (char *)csky_insn.opcode_end, NULL);
	  return false;
	}
    }
  csky_insn.output = frag_more (csky_insn.isize);
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);

  return true;
}

bool
v2_work_add_sub (void)
{
  if (csky_insn.number == 3
	   && (csky_insn.val[0] == csky_insn.val[1]
	      || csky_insn.val[0] == csky_insn.val[2])
	   && csky_insn.val[0] <= 15
	   && csky_insn.val[1] <= 15
	   && csky_insn.val[2] <= 15)
    {
      if (!strstr (csky_insn.opcode->mnemonic, "sub")
	  || csky_insn.val[0] == csky_insn.val[1])
	{
	  csky_insn.opcode_idx = 0;
	  csky_insn.isize = 2;
	  if (csky_insn.val[0] == csky_insn.val[1])
	    csky_insn.val[1] = csky_insn.val[2];

	  csky_insn.number = 2;

	}
    }
  if (csky_insn.isize == 4
      && IS_CSKY_ARCH_801 (mach_flag))
    {
      if (csky_insn.number == 3)
	{
	  if (csky_insn.val[0] > 7)
	    {
	      SET_ERROR_INTEGER (ERROR_REG_OVER_RANGE, csky_insn.val[0]);
	      csky_show_error (ERROR_REG_OVER_RANGE, 1, NULL, NULL);
	    }
	  if (csky_insn.val[1] > 7)
	    {
	      SET_ERROR_INTEGER (ERROR_REG_OVER_RANGE, csky_insn.val[1]);
	      csky_show_error (ERROR_REG_OVER_RANGE, 2, NULL, NULL);
	    }
	  if (csky_insn.val[2] > 7)
	    {
	      SET_ERROR_INTEGER (ERROR_REG_OVER_RANGE, csky_insn.val[2]);
	      csky_show_error (ERROR_REG_OVER_RANGE, 3, NULL, NULL);
	    }
	}
      else
	{
	  if (csky_insn.val[0] > 15)
	    {
	      SET_ERROR_INTEGER (ERROR_REG_OVER_RANGE, csky_insn.val[0]);
	      csky_show_error (ERROR_REG_OVER_RANGE, 1, NULL, NULL);
	    }
	  if (csky_insn.val[1] > 15)
	    {
	      SET_ERROR_INTEGER (ERROR_REG_OVER_RANGE, csky_insn.val[1]);
	      csky_show_error (ERROR_REG_OVER_RANGE, 2, NULL, NULL);
	    }
	}
      return false;
    }
  /* sub rz, rx.  */
  /* Generate relax or reloc if necessary.  */
  csky_generate_frags ();
  /* Generate the insn by mask.  */
  csky_generate_insn ();
  /* Write inst to frag.  */
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
v2_work_rotlc (void)
{
  const char *name = "addc";
  csky_insn.opcode
    = (struct csky_opcode *) str_hash_find (csky_opcodes_hash, name);
  csky_insn.opcode_idx = 0;
  if (csky_insn.isize == 2)
    {
      /* addc rz, rx.  */
      csky_insn.number = 2;
      csky_insn.val[1] = csky_insn.val[0];
    }
  else
    {
      csky_insn.number = 3;
      /* addc rz, rx, ry.  */
      csky_insn.val[1] = csky_insn.val[0];
      csky_insn.val[2] = csky_insn.val[0];
    }
  /* Generate relax or reloc if necessary.  */
  csky_generate_frags ();
  /* Generate the insn by mask.  */
  csky_generate_insn ();
  /* Write inst to frag.  */
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
v2_work_bgeni (void)
{
  const char *name = NULL;
  int imm = csky_insn.val[1];
  int val = 1 << imm;
  if (imm < 16)
      name = "movi";
  else
    {
      name = "movih";
      val >>= 16;
    }
  csky_insn.opcode
    = (struct csky_opcode *) str_hash_find (csky_opcodes_hash, name);
  csky_insn.opcode_idx = 0;
  csky_insn.val[1] = val;

  /* Generate relax or reloc if necessary.  */
  csky_generate_frags ();
  /* Generate the insn by mask.  */
  csky_generate_insn ();
  /* Write inst to frag.  */
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
v2_work_not (void)
{
  const char *name = "nor";
  csky_insn.opcode
    = (struct csky_opcode *) str_hash_find (csky_opcodes_hash, name);
  csky_insn.opcode_idx = 0;
  if (csky_insn.number == 1)
    {
      csky_insn.val[1] = csky_insn.val[0];
      if (csky_insn.val[0] < 16)
	{
	  /* 16 bits nor rz, rz.  */
	  csky_insn.number = 2;
	  csky_insn.isize = 2;
	}
      else
	{
	  csky_insn.val[2] = csky_insn.val[0];
	  csky_insn.number = 3;
	  csky_insn.isize = 4;
	}
    }
  if (csky_insn.number == 2)
    {
      if (csky_insn.val[0] == csky_insn.val[1]
	  && csky_insn.val[0] < 16)
	{
	  /* 16 bits nor rz, rz.  */
	  csky_insn.number = 2;
	  csky_insn.isize = 2;
	}
      else
	{
	  csky_insn.val[2] = csky_insn.val[1];
	  csky_insn.number = 3;
	  csky_insn.isize = 4;
	}
    }

  /* Generate relax or reloc if necessary.  */
  csky_generate_frags ();
  /* Generate the insn by mask.  */
  csky_generate_insn ();
  /* Write inst to frag.  */
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
v2_work_jbtf (void)
{
  if (csky_insn.e1.X_add_symbol == NULL || csky_insn.e1.X_op == O_constant)
    {
      csky_show_error (ERROR_UNDEFINE, 0, (void *)"operand is invalid", NULL);
      return false;
    }

  if (IS_CSKY_ARCH_801 (mach_flag))
    {
      /* CK801 doesn't have 32-bit bt/bf insns or a jump insn with a
	 range larger than SCOND_DISP16.  Relax to a short jump around
	 an unconditional branch, and give up if that overflows too.  */
      csky_insn.output = frag_var (rs_machine_dependent,
				   SCOND_DISP16_LEN,
				   SCOND_DISP10_LEN,
				   SCOND_DISP10,
				   csky_insn.e1.X_add_symbol,
				   csky_insn.e1.X_add_number,
				   0);
      csky_insn.isize = 2;
      csky_insn.max = SCOND_DISP16_LEN;
      csky_insn.inst = csky_insn.opcode->op16[0].opcode;
    }
  else if (do_long_jump && !IS_CSKY_ARCH_802 (mach_flag))
    {
      /* Generate relax with jcondition.
	 Note that CK802 doesn't support the JMPI instruction so
	 we cannot relax to a jump with a 32-bit offset.  */
      csky_insn.output = frag_var (rs_machine_dependent,
				   JCOND_DISP32_LEN,
				   JCOND_DISP10_LEN,
				   JCOND_DISP10,
				   csky_insn.e1.X_add_symbol,
				   csky_insn.e1.X_add_number,
				   0);
      csky_insn.isize = 2;
      csky_insn.max = JCOND_DISP32_LEN;
      csky_insn.inst = csky_insn.opcode->op16[0].opcode;
    }
  else
    {
      /* Generate relax with condition.  */
      csky_insn.output = frag_var (rs_machine_dependent,
				   COND_DISP16_LEN,
				   COND_DISP10_LEN,
				   COND_DISP10,
				   csky_insn.e1.X_add_symbol,
				   csky_insn.e1.X_add_number,
				   0);
      csky_insn.isize = 2;
      csky_insn.max = COND_DISP16_LEN;
      csky_insn.inst = csky_insn.opcode->op16[0].opcode;
    }
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);

  return true;
}

bool
v2_work_jbr (void)
{
  if (csky_insn.e1.X_add_symbol == NULL || csky_insn.e1.X_op == O_constant)
    {
      csky_show_error (ERROR_UNDEFINE, 0, (void *)"operand is invalid", NULL);
      return false;
    }

  if (do_long_jump
      && !IS_CSKY_ARCH_801 (mach_flag)
      && !IS_CSKY_ARCH_802 (mach_flag))
    {
      csky_insn.output = frag_var (rs_machine_dependent,
				   JUNCD_DISP32_LEN,
				   JUNCD_DISP10_LEN,
				   JUNCD_DISP10,
				   csky_insn.e1.X_add_symbol,
				   csky_insn.e1.X_add_number,
				   0);

      csky_insn.inst = csky_insn.opcode->op16[0].opcode;
      csky_insn.max = JUNCD_DISP32_LEN;
      csky_insn.isize = 2;
    }
  else
    {
      /* Generate relax with condition.  */
      csky_insn.output = frag_var (rs_machine_dependent,
				   UNCD_DISP16_LEN,
				   UNCD_DISP10_LEN,
				   UNCD_DISP10,
				   csky_insn.e1.X_add_symbol,
				   csky_insn.e1.X_add_number,
				   0);
      csky_insn.isize = 2;
      csky_insn.max = UNCD_DISP16_LEN;
      csky_insn.inst = csky_insn.opcode->op16[0].opcode;

    }
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

#define SIZE_V2_MOVI16(x)         ((addressT)x <= 0xff)
#define SIZE_V2_MOVI32(x)         ((addressT)x <= 0xffff)
#define SIZE_V2_MOVIH(x)          ((addressT)x <= 0xffffffff && (((addressT)x & 0xffff) == 0))

bool
v2_work_lrw (void)
{
  int reg = csky_insn.val[0];
  int output_literal = csky_insn.val[1];
  int is_done = 0;

  /* If the second operand is O_constant, We can use movi/movih
     instead of lrw.  */
  if (csky_insn.e1.X_op == O_constant)
    {
      /* 801 only has movi16.  */
      if (SIZE_V2_MOVI16 (csky_insn.e1.X_add_number) && reg < 8)
	{
	  /* movi16 instead.  */
	  csky_insn.output = frag_more (2);
	  csky_insn.inst = (CSKYV2_INST_MOVI16 | (reg << 8)
			    | (csky_insn.e1.X_add_number));
	  csky_insn.isize = 2;
	  is_done = 1;
	}
      else if (SIZE_V2_MOVI32 (csky_insn.e1.X_add_number)
	       && !IS_CSKY_ARCH_801 (mach_flag))
	{
	  /* movi32 instead.  */
	  csky_insn.output = frag_more (4);
	  csky_insn.inst = (CSKYV2_INST_MOVI32 | (reg << 16)
			    | (csky_insn.e1.X_add_number));
	  csky_insn.isize = 4;
	  is_done = 1;
	}
      else if (SIZE_V2_MOVIH (csky_insn.e1.X_add_number)
	       && !IS_CSKY_ARCH_801 (mach_flag))
	{
	  /* movih instead.  */
	  csky_insn.output = frag_more (4);
	  csky_insn.inst = (CSKYV2_INST_MOVIH | (reg << 16)
			    | ((csky_insn.e1.X_add_number >> 16) & 0xffff));
	  csky_insn.isize = 4;
	  is_done = 1;
	}
    }

  if (is_done)
    {
      csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
      return true;
    }

  if (output_literal)
    {
      struct literal *p = enter_literal (&csky_insn.e1, 0, 0, 0);
      /* Create a reference to pool entry.  */
      csky_insn.e1.X_op = O_symbol;
      csky_insn.e1.X_add_symbol = poolsym;
      csky_insn.e1.X_add_number = p->offset << 2;
    }
  /* If 16bit force.  */
  if (csky_insn.flag_force == INSN_OPCODE16F)
    {
      /* Generate fixup.  */
      if (reg > 7)
	{
	  csky_show_error (ERROR_UNDEFINE, 0,
			   (void *)"The register is out of range.", NULL);
	  return false;
	}
      csky_insn.isize = 2;
      csky_insn.output = frag_more (2);

      if (insn_reloc == BFD_RELOC_CKCORE_TLS_GD32
	  || insn_reloc == BFD_RELOC_CKCORE_TLS_LDM32
	  || insn_reloc == BFD_RELOC_CKCORE_TLS_IE32)
	{
	  literal_insn_offset->tls_addend.frag = frag_now;
	  literal_insn_offset->tls_addend.offset
	    = csky_insn.output - frag_now->fr_literal;
	}
      csky_insn.inst = csky_insn.opcode->op16[0].opcode | (reg << 5);
      csky_insn.max = 4;
      fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		   2, &csky_insn.e1, 1, BFD_RELOC_CKCORE_PCREL_IMM7BY4);
    }
  else if (csky_insn.flag_force == INSN_OPCODE32F)
    {
      csky_insn.isize = 4;
      csky_insn.output = frag_more (4);
      if (insn_reloc == BFD_RELOC_CKCORE_TLS_GD32
	  || insn_reloc == BFD_RELOC_CKCORE_TLS_LDM32
	  || insn_reloc == BFD_RELOC_CKCORE_TLS_IE32)
       {
	  literal_insn_offset->tls_addend.frag = frag_now;
	  literal_insn_offset->tls_addend.offset
	    = csky_insn.output - frag_now->fr_literal;
       }
      csky_insn.inst = csky_insn.opcode->op32[0].opcode | (reg << 16);
      fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		   4, &csky_insn.e1, 1, BFD_RELOC_CKCORE_PCREL_IMM16BY4);
    }
  else if (!is_done)
    {
      if (reg < 8)
	{
	  csky_insn.isize = 2;

	  if (insn_reloc == BFD_RELOC_CKCORE_TLS_GD32
	      || insn_reloc == BFD_RELOC_CKCORE_TLS_LDM32
	      || insn_reloc == BFD_RELOC_CKCORE_TLS_IE32)
	    literal_insn_offset->tls_addend.frag = frag_now;

	  csky_insn.output = frag_var (rs_machine_dependent,
				       LRW_DISP16_LEN,
				       LRW_DISP7_LEN,
				       (do_extend_lrw
					? LRW2_DISP8 : LRW_DISP7),
				       csky_insn.e1.X_add_symbol,
				       csky_insn.e1.X_add_number, 0);
	  if (insn_reloc == BFD_RELOC_CKCORE_TLS_GD32
	      || insn_reloc == BFD_RELOC_CKCORE_TLS_LDM32
	      || insn_reloc == BFD_RELOC_CKCORE_TLS_IE32)
	    {
	      if (literal_insn_offset->tls_addend.frag->fr_next != frag_now)
		literal_insn_offset->tls_addend.frag
		  = literal_insn_offset->tls_addend.frag->fr_next;
	      literal_insn_offset->tls_addend.offset
		= (csky_insn.output
		   - literal_insn_offset->tls_addend.frag->fr_literal);
	    }
	  csky_insn.inst = csky_insn.opcode->op16[0].opcode | (reg << 5);
	  csky_insn.max = LRW_DISP16_LEN;
	  csky_insn.isize = 2;
	}
      else
	{
	  csky_insn.isize = 4;
	  csky_insn.output = frag_more (4);
	  if (insn_reloc == BFD_RELOC_CKCORE_TLS_GD32
	      || insn_reloc == BFD_RELOC_CKCORE_TLS_LDM32
	      || insn_reloc == BFD_RELOC_CKCORE_TLS_IE32)
	   {
	      literal_insn_offset->tls_addend.frag = frag_now;
	      literal_insn_offset->tls_addend.offset
		= csky_insn.output - frag_now->fr_literal;
	   }
	  csky_insn.inst = csky_insn.opcode->op32[0].opcode | (reg << 16);
	  fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		       4, &csky_insn.e1, 1, BFD_RELOC_CKCORE_PCREL_IMM16BY4);
       }
    }

  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
v2_work_lrsrsw (void)
{
  int reg = csky_insn.val[0];
  csky_insn.output = frag_more (4);
  csky_insn.inst = csky_insn.opcode->op32[0].opcode | (reg << 21);
  csky_insn.isize = 4;

  switch (insn_reloc)
    {
      case BFD_RELOC_CKCORE_GOT32:
	fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		     4, &csky_insn.e1, 0, BFD_RELOC_CKCORE_GOT_IMM18BY4);
	break;
      case BFD_RELOC_CKCORE_PLT32:
	fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		     4, &csky_insn.e1, 0, BFD_RELOC_CKCORE_PLT_IMM18BY4);
	break;
      default:
	fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		     4, &csky_insn.e1, 1, BFD_RELOC_CKCORE_DOFFSET_IMM18BY4);
	break;
    }
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
v2_work_jbsr (void)
{
  if (do_force2bsr
      || IS_CSKY_ARCH_801 (mach_flag)
      || IS_CSKY_ARCH_802 (mach_flag))
    {
      csky_insn.output = frag_more (4);
      fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		   4, &csky_insn.e1, 1, BFD_RELOC_CKCORE_PCREL_IMM26BY2);
      csky_insn.isize = 4;
      csky_insn.inst = CSKYV2_INST_BSR32;
    }
  else
    {
      struct literal *p = enter_literal (&csky_insn.e1, 0, 0, 0);
      csky_insn.output = frag_more (4);
      csky_insn.e1.X_op = O_symbol;
      csky_insn.e1.X_add_symbol = poolsym;
      csky_insn.e1.X_add_number = p->offset << 2;
      fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		 4, &csky_insn.e1, 1, BFD_RELOC_CKCORE_PCREL_IMM16BY4);
      if (do_jsri2bsr || IS_CSKY_ARCH_810 (mach_flag))
	fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		     4,
		     &(litpool + (csky_insn.e1.X_add_number >> 2))->e,
		     1,
		     BFD_RELOC_CKCORE_PCREL_JSR_IMM26BY2);
      csky_insn.inst = CSKYV2_INST_JSRI32;
      csky_insn.isize = 4;
      if (IS_CSKY_ARCH_810 (mach_flag))
	{
	  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
	  csky_insn.output = frag_more (4);
	  dwarf2_emit_insn (0);
	  /* Insert "mov r0, r0".  */
	  csky_insn.inst = CSKYV2_INST_MOV_R0_R0;
	  csky_insn.max = 8;
	}
    }
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);

  return true;
}

bool
v2_work_jsri (void)
{
  /* dump literal.  */
  struct literal *p = enter_literal (&csky_insn.e1, 1, 0, 0);
  csky_insn.e1.X_op = O_symbol;
  csky_insn.e1.X_add_symbol = poolsym;
  csky_insn.e1.X_add_number = p->offset << 2;

  /* Generate relax or reloc if necessary.  */
  csky_generate_frags ();
  /* Generate the insn by mask.  */
  csky_generate_insn ();
  /* Write inst to frag.  */
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  /* Control 810 not to generate jsri.  */
  if (IS_CSKY_ARCH_810 (mach_flag))
    {
      /* Look at adding the R_PCREL_JSRIMM26BY2.
	 For 'jbsr .L1', this reloc type's symbol
	 is bound to '.L1', isn't bound to literal pool.  */
      fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		   4, &p->e, 1,
		   BFD_RELOC_CKCORE_PCREL_JSR_IMM26BY2);
      csky_insn.output = frag_more (4);
      dwarf2_emit_insn (0);
      /* The opcode of "mov32 r0,r0".  */
      csky_insn.inst = CSKYV2_INST_MOV_R0_R0;
      /* The effect of this value is to check literal.  */
      csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
      csky_insn.max = 8;
    }
  return true;
}

bool
v2_work_movih (void)
{
  int rz = csky_insn.val[0];
  csky_insn.output = frag_more (4);
  csky_insn.inst = csky_insn.opcode->op32[0].opcode | (rz << 16);
  if (csky_insn.e1.X_op == O_constant)
    {
      if (csky_insn.e1.X_unsigned == 1 && csky_insn.e1.X_add_number > 0xffff)
	{
	  csky_show_error (ERROR_IMM_OVERFLOW, 2, NULL, NULL);
	  return false;
	}
      else if (csky_insn.e1.X_unsigned == 0 && csky_insn.e1.X_add_number < 0)
	{
	  csky_show_error (ERROR_IMM_OVERFLOW, 2, NULL, NULL);
	  return false;
	}
      else
	csky_insn.inst |= (csky_insn.e1.X_add_number & 0xffff);
    }
  else if (csky_insn.e1.X_op == O_right_shift
	   || (csky_insn.e1.X_op == O_symbol && insn_reloc != BFD_RELOC_NONE))
    {
      if (csky_insn.e1.X_op_symbol != 0
	  && symbol_constant_p (csky_insn.e1.X_op_symbol)
	  && S_GET_SEGMENT (csky_insn.e1.X_op_symbol) == absolute_section
	  && 16 == S_GET_VALUE (csky_insn.e1.X_op_symbol))
	{
	  csky_insn.e1.X_op = O_symbol;
	  if (insn_reloc == BFD_RELOC_CKCORE_GOT32)
	    insn_reloc = BFD_RELOC_CKCORE_GOT_HI16;
	  else if (insn_reloc == BFD_RELOC_CKCORE_PLT32)
	    insn_reloc = BFD_RELOC_CKCORE_PLT_HI16;
	  else if (insn_reloc == BFD_RELOC_CKCORE_GOTPC)
	    insn_reloc = BFD_RELOC_CKCORE_GOTPC_HI16;
	  else if (insn_reloc == BFD_RELOC_CKCORE_GOTOFF)
	    insn_reloc = BFD_RELOC_CKCORE_GOTOFF_HI16;
	  else
	    insn_reloc = BFD_RELOC_CKCORE_ADDR_HI16;
	  fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		       4, &csky_insn.e1, 0, insn_reloc);
	}
      else
	{
	  void *arg = (void *)"the second operand must be \"SYMBOL >> 16\"";
	  csky_show_error (ERROR_UNDEFINE, 0, arg, NULL);
	  return false;
	}
    }
  csky_insn.isize = 4;
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);

  return true;
}

bool
v2_work_ori (void)
{
  int rz = csky_insn.val[0];
  int rx = csky_insn.val[1];
  csky_insn.output = frag_more (4);
  csky_insn.inst = csky_insn.opcode->op32[0].opcode | (rz << 21) | (rx << 16);
  if (csky_insn.e1.X_op == O_constant)
    {
      if (csky_insn.e1.X_add_number <= 0xffff
	  && csky_insn.e1.X_add_number >= 0)
	csky_insn.inst |= csky_insn.e1.X_add_number;
      else
	{
	  csky_show_error (ERROR_IMM_OVERFLOW, 3, NULL, NULL);
	  return false;
	}
    }
  else if (csky_insn.e1.X_op == O_bit_and)
    {
      if (symbol_constant_p (csky_insn.e1.X_op_symbol)
	  && S_GET_SEGMENT (csky_insn.e1.X_op_symbol) == absolute_section
	  && 0xffff == S_GET_VALUE (csky_insn.e1.X_op_symbol))
	{
	  csky_insn.e1.X_op = O_symbol;
	  if (insn_reloc == BFD_RELOC_CKCORE_GOT32)
	    insn_reloc = BFD_RELOC_CKCORE_GOT_LO16;
	  else if (insn_reloc == BFD_RELOC_CKCORE_PLT32)
	    insn_reloc = BFD_RELOC_CKCORE_PLT_LO16;
	  else if (insn_reloc == BFD_RELOC_CKCORE_GOTPC)
	    insn_reloc = BFD_RELOC_CKCORE_GOTPC_LO16;
	  else if (insn_reloc == BFD_RELOC_CKCORE_GOTOFF)
	    insn_reloc = BFD_RELOC_CKCORE_GOTOFF_LO16;
	  else
	    insn_reloc = BFD_RELOC_CKCORE_ADDR_LO16;
	  fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		       4, &csky_insn.e1, 0, insn_reloc);
	}
      else
	{
	  void *arg = (void *)"the third operand must be \"SYMBOL & 0xffff\"";
	  csky_show_error (ERROR_UNDEFINE, 0, arg, NULL);
	  return false;
	}
    }
  csky_insn.isize = 4;
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

/* Helper function to encode a single/double floating point constant
   into the instruction word for fmovis and fmovid instructions.
   The constant is in its IEEE single/double precision representation
   and is repacked into the internal 13-bit representation for these
   instructions with a diagnostic for overflow.  Note that there is no
   rounding when converting to the smaller format, just an error if there
   is excess precision or the number is too small/large to be represented.  */

bool
float_work_fmovi (void)
{
  int rx = csky_insn.val[0];

  /* We already converted the float constant to the internal 13-bit
     representation so we just need to OR it in here.  */
  csky_insn.inst = csky_insn.opcode->op32[0].opcode | rx;
  csky_insn.inst |= (uint32_t) csky_insn.e1.X_add_number;

  csky_insn.output = frag_more (4);
  csky_insn.isize = 4;
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

/* Like float_work_fmovi, but for FPUV3 fmovi.16, fmovi.32 and fmovi.64
   instructions.  */

bool
float_work_fpuv3_fmovi (void)
{
  int rx = csky_insn.val[0];
  int idx = csky_insn.opcode_idx;
  int imm4 = 0;
  int imm8 = 0;
  int sign = 0;

  csky_insn.inst = csky_insn.opcode->op32[idx].opcode | rx;

  if (csky_insn.opcode->op32[idx].operand_num == 3)
    {
      /* fmovi.xx frz, imm9, imm4.  */
      imm8 = csky_insn.val[1];
      imm4 = csky_insn.val[2];
      if (imm8 < 0 || (imm8 & 0x80000000))
	{
	  sign = (1 << 5);
	  imm8 = 0 - imm8;
	}

      if (imm8 > 255)
	{
	  csky_show_error (ERROR_IMM_OVERFLOW, 2, NULL, NULL);
	  return false;
	}

      /* imm8 store at bit [25:20] and [9:8].  */
      /* imm4 store at bit [19:16].  */
      /* sign store at bit [5].  */
      csky_insn.inst = csky_insn.inst
	| ((imm8 & 0x3) << 8)
	| ((imm8 & 0xfc) << 18)
	| ((imm4 & 0xf) << 16)
	| sign;
    }
  else
    {
       csky_insn.inst |= (uint32_t) csky_insn.e1.X_add_number;
    }

  csky_insn.output = frag_more(4);
  csky_insn.isize = 4;
  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
dsp_work_bloop (void)
{
  int reg = csky_insn.val[0];
  csky_insn.output = frag_more (4);
  csky_insn.inst = csky_insn.opcode->op32[0].opcode | (reg << 16);
  csky_insn.isize = 4;

  if (csky_insn.number == 3
      && csky_insn.e1.X_op == O_symbol
      && csky_insn.e2.X_op == O_symbol)
    {
	fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		     4, &csky_insn.e1, 1,
		     BFD_RELOC_CKCORE_PCREL_BLOOP_IMM12BY4);
	fix_new_exp (frag_now, csky_insn.output - frag_now->fr_literal,
		     4, &csky_insn.e2, 1,
		     BFD_RELOC_CKCORE_PCREL_BLOOP_IMM4BY4);
    }
  else if (csky_insn.number == 2
	   && csky_insn.e1.X_op == O_symbol)
    {
      fix_new_exp (frag_now, csky_insn.output-frag_now->fr_literal,
		   4, &csky_insn.e1, 1,
		   BFD_RELOC_CKCORE_PCREL_BLOOP_IMM12BY4);
      if (csky_insn.last_isize == 2)
	csky_insn.inst |= (0xf << 12);
      else if (csky_insn.last_isize != 0)
	csky_insn.inst |= (0xe << 12);
      else
	{
	  void *arg = (void *)"bloop can not be the first instruction"\
		      "when the end label is not specified.\n";
	  csky_show_error (ERROR_UNDEFINE, 0, arg, NULL);
	}
    }

  csky_write_insn (csky_insn.output, csky_insn.inst, csky_insn.isize);
  return true;
}

bool
float_work_fpuv3_fstore(void)
{
  /* Generate relax or reloc if necessary.  */
  csky_generate_frags ();
  /* Generate the insn by mask.  */
  csky_generate_insn ();
  /* Write inst to frag.  */
  csky_write_insn (csky_insn.output,
                   csky_insn.inst,
                   csky_insn.isize);


  return true;
}

bool
v2_work_addc (void)
{
  int reg1;
  int reg2;
  int reg3 = 0;
  int is_16_bit = 0;

  reg1 = csky_insn.val[0];
  reg2 = csky_insn.val[1];
  if (csky_insn.number == 2)
    {
      if (reg1 > 15 || reg2 > 15)
	{
	  is_16_bit = 0;
	  reg3 = reg1;
	}
      else
	is_16_bit = 1;
    }
  else
    {
      reg3 = csky_insn.val[2];
      if (reg1 > 15 || reg2 > 15 || reg3 > 15)
	is_16_bit = 0;
      else if (reg1 == reg2 || reg1 == reg3)
	{
	  is_16_bit = 1;
	  reg2 = (reg1 == reg2) ? reg3 : reg2;
	}
      else
	is_16_bit = 0;
    }

  if (is_16_bit
      && csky_insn.flag_force != INSN_OPCODE32F)
    {
      csky_insn.isize = 2;
      csky_insn.inst = csky_insn.opcode->op16[0].opcode
	| (reg1 << 6) | (reg2 << 2);
    }
  else if (csky_insn.flag_force != INSN_OPCODE16F)
    {
      csky_insn.isize = 4;
      csky_insn.inst = csky_insn.opcode->op32[0].opcode
	| (reg1 << 0) | (reg2 << 16) | (reg3 << 21);
    }
  else
    {
      SET_ERROR_INTEGER (ERROR_REG_OVER_RANGE, reg1 > 15 ? reg1 : reg2);
      csky_show_error (ERROR_REG_OVER_RANGE, 0, 0, NULL);
    }

  /* Generate relax or reloc if necessary.  */
  csky_generate_frags ();
  /* Write inst to frag.  */
  csky_write_insn (csky_insn.output,
		   csky_insn.inst,
		   csky_insn.isize);

  return true;
}

/* The following are for assembler directive handling.  */

/* Helper function to adjust constant pool counts when we emit a
   data directive in the text section.  FUNC is one of the standard
   gas functions to handle these directives, like "stringer" for the
   .string directive, and ARG is the argument to FUNC.  csky_pool_count
   essentially wraps the call with the constant pool magic.  */

static void
csky_pool_count (void (*func) (int), int arg)
{
  const fragS *curr_frag = frag_now;
  offsetT added = -frag_now_fix_octets ();

  (*func) (arg);

  while (curr_frag != frag_now)
    {
      added += curr_frag->fr_fix;
      curr_frag = curr_frag->fr_next;
    }

  added += frag_now_fix_octets ();
  poolspan += added;
}

/* Support the .literals directive.  */
static void
csky_s_literals (int ignore ATTRIBUTE_UNUSED)
{
  dump_literals (0);
  demand_empty_rest_of_line ();
}

/* Support the .string, etc directives.  */
static void
csky_stringer (int append_zero)
{
  if (now_seg == text_section)
    csky_pool_count (stringer, append_zero);
  else
    stringer (append_zero);

  /* We call check_literals here in case a large number of strings are
     being placed into the text section with a sequence of stringer
     directives.  In theory we could be upsetting something if these
     strings are actually in an indexed table instead of referenced by
     individual labels.  Let us hope that that never happens.  */
  check_literals (2, 0);
}

/* Support integer-mode constructors like .word, .byte, etc.  */

static void
csky_cons (int nbytes)
{
  mapping_state (MAP_DATA);
  if (nbytes == 4)  /* @GOT.  */
    {
      do
	{
	  bfd_reloc_code_real_type reloc;
	  expressionS exp;

	  reloc = BFD_RELOC_NONE;
	  expression (&exp);
	  lex_got (&reloc, NULL);

	  if (exp.X_op == O_symbol && reloc != BFD_RELOC_NONE)
	    {
	      reloc_howto_type *howto
		= bfd_reloc_type_lookup (stdoutput, reloc);
	      int size = bfd_get_reloc_size (howto);

	      if (size > nbytes)
		as_bad (ngettext ("%s relocations do not fit in %d byte",
				  "%s relocations do not fit in %d bytes",
				  nbytes),
			howto->name, nbytes);
	      else
		{
		  register char *p = frag_more ((int) nbytes);
		  int offset = nbytes - size;

		  fix_new_exp (frag_now,
			       p - frag_now->fr_literal + offset,
			       size, &exp, 0, reloc);
		}
	    }
	  else
	    emit_expr (&exp, (unsigned int) nbytes);
	  if (now_seg == text_section)
	    poolspan += nbytes;
	}
      while (*input_line_pointer++ == ',');

      /* Put terminator back into stream.  */
      input_line_pointer --;
      demand_empty_rest_of_line ();

      return;
    }

  if (now_seg == text_section)
    csky_pool_count (cons, nbytes);
  else
    cons (nbytes);

  /* In theory we ought to call check_literals (2,0) here in case
     we need to dump the literal table.  We cannot do this however,
     as the directives that we are intercepting may be being used
     to build a switch table, and we must not interfere with its
     contents.  Instead we cross our fingers and pray...  */
}

/* Support floating-mode constant directives like .float and .double.  */

static void
csky_float_cons (int float_type)
{
  mapping_state (MAP_DATA);
  if (now_seg == text_section)
    csky_pool_count (float_cons, float_type);
  else
    float_cons (float_type);

  /* See the comment in csky_cons () about calling check_literals.
     It is unlikely that a switch table will be constructed using
     floating point values, but it is still likely that an indexed
     table of floating point constants is being created by these
     directives, so again we must not interfere with their placement.  */
}

/* Support the .fill directive.  */

static void
csky_fill (int ignore)
{
  if (now_seg == text_section)
    csky_pool_count (s_fill, ignore);
  else
    s_fill (ignore);

  check_literals (2, 0);
}

/* Handle the section changing pseudo-ops.  These call through to the
   normal implementations, but they dump the literal pool first.  */

static void
csky_s_text (int ignore)
{
  dump_literals (0);

#ifdef OBJ_ELF
  obj_elf_text (ignore);
#else
  s_text (ignore);
#endif
}

static void
csky_s_data (int ignore)
{
  dump_literals (0);

#ifdef OBJ_ELF
  obj_elf_data (ignore);
#else
  s_data (ignore);
#endif
}

static void
csky_s_section (int ignore)
{
  /* Scan forwards to find the name of the section.  If the section
     being switched to is ".line" then this is a DWARF1 debug section
     which is arbitrarily placed inside generated code.  In this case
     do not dump the literal pool because it is a) inefficient and
     b) would require the generation of extra code to jump around the
     pool.  */
  char * ilp = input_line_pointer;

  while (*ilp != 0 && ISSPACE (*ilp))
    ++ ilp;

  if (startswith (ilp, ".line")
      && (ISSPACE (ilp[5]) || *ilp == '\n' || *ilp == '\r'))
    ;
  else
    dump_literals (0);

#ifdef OBJ_ELF
  obj_elf_section (ignore);
#endif
#ifdef OBJ_COFF
  obj_coff_section (ignore);
#endif
}

static void
csky_s_bss (int needs_align)
{
  dump_literals (0);
  s_lcomm_bytes (needs_align);
}

#ifdef OBJ_ELF
static void
csky_s_comm (int needs_align)
{
  dump_literals (0);
  obj_elf_common (needs_align);
}
#endif

/* Handle the .no_literal_dump directive.  */

static void
csky_noliteraldump (int ignore ATTRIBUTE_UNUSED)
{
  do_noliteraldump = 1;
  int insn_num = get_absolute_expression ();
  /* The insn after '.no_literal_dump insn_num' is insn1,
     Don't dump literal pool between insn1 and insn(insn_num+1)
     The insn cannot be the insn generate literal, like lrw & jsri.  */
  check_literals (0, insn_num * 2);
}

/* Handle the .align directive.
   We must check literals before doing alignment.  For example, if
   '.align n', add (2^n-1) to poolspan and check literals.  */

static void
csky_s_align_ptwo (int arg)
{
  /* Get the .align's first absolute number.  */
  char * temp_pointer = input_line_pointer;
  int align = get_absolute_expression ();
  check_literals (0, (1 << align) - 1);
  input_line_pointer = temp_pointer;

  /* Do alignment.  */
  s_align_ptwo (arg);
}

/* Handle the .stack_size directive.  */

static void
csky_stack_size (int arg ATTRIBUTE_UNUSED)
{
  expressionS exp;
  stack_size_entry *sse
    = (stack_size_entry *) xcalloc (1, sizeof (stack_size_entry));

  expression (&exp);
  if (exp.X_op == O_symbol)
    sse->function = exp.X_add_symbol;
  else
    {
      as_bad (_("the first operand must be a symbol"));
      ignore_rest_of_line ();
      free (sse);
      return;
    }

  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad (_("missing stack size"));
      ignore_rest_of_line ();
      free (sse);
      return;
    }

  ++input_line_pointer;
  expression (&exp);
  if (exp.X_op == O_constant)
    {
      if (exp.X_add_number < 0 || exp.X_add_number > (offsetT)0xffffffff)
	{

	  as_bad (_("value not in range [0, 0xffffffff]"));
	  ignore_rest_of_line ();
	  free (sse);
	  return;
	}
      else
	sse->stack_size = exp.X_add_number;
    }
  else
    {
      as_bad (_("operand must be a constant"));
      ignore_rest_of_line ();
      free (sse);
      return;
    }

  if (*last_stack_size_data != NULL)
    last_stack_size_data = &((*last_stack_size_data)->next);

  *last_stack_size_data = sse;
}

/* This table describes all the machine specific pseudo-ops the assembler
   has to support.  The fields are:
     pseudo-op name without dot
     function to call to execute this pseudo-op
     Integer arg to pass to the function.  */

const pseudo_typeS md_pseudo_table[] =
{
  { "export",   s_globl,          0 },
  { "import",   s_ignore,         0 },
  { "literals", csky_s_literals, 0 },
  { "page",     listing_eject,    0 },

  /* The following are to intercept the placement of data into the text
     section (eg addresses for a switch table), so that the space they
     occupy can be taken into account when deciding whether or not to
     dump the current literal pool.
     XXX - currently we do not cope with the .space and .dcb.d directives.  */
  { "ascii",    csky_stringer,       8 + 0 },
  { "asciz",    csky_stringer,       8 + 1 },
  { "byte",     csky_cons,           1 },
  { "dc",       csky_cons,           2 },
  { "dc.b",     csky_cons,           1 },
  { "dc.d",     csky_float_cons,    'd'},
  { "dc.l",     csky_cons,           4 },
  { "dc.s",     csky_float_cons,    'f'},
  { "dc.w",     csky_cons,           2 },
  { "dc.x",     csky_float_cons,    'x'},
  { "double",   csky_float_cons,    'd'},
  { "float",    csky_float_cons,    'f'},
  { "hword",    csky_cons,           2 },
  { "int",      csky_cons,           4 },
  { "long",     csky_cons,           4 },
  { "octa",     csky_cons,          16 },
  { "quad",     csky_cons,           8 },
  { "short",    csky_cons,           2 },
  { "single",   csky_float_cons,    'f'},
  { "string",   csky_stringer,       8 + 1 },
  { "word",     csky_cons,           4 },
  { "fill",     csky_fill,           0 },

  /* Allow for the effect of section changes.  */
  { "text",      csky_s_text,    0 },
  { "data",      csky_s_data,    0 },
  { "bss",       csky_s_bss,     1 },
#ifdef OBJ_ELF
  { "comm",      csky_s_comm,    0 },
#endif
  { "section",   csky_s_section, 0 },
  { "section.s", csky_s_section, 0 },
  { "sect",      csky_s_section, 0 },
  { "sect.s",    csky_s_section, 0 },
  /* When ".no_literal_dump N" is in front of insn1,
     and instruction sequence is:
       insn1
       insn2
       ......
       insnN+1
     it means literals will not dump between insn1 and insnN+1
     The insn cannot itself generate literal, like lrw & jsri.  */
  { "no_literal_dump",	csky_noliteraldump,	0 },
  { "align",		csky_s_align_ptwo,	 0 },
  { "stack_size",	csky_stack_size,	 0 },
  {0,       0,            0}
};

/* Implement tc_cfi_frame_initial_instructions.  */

void
csky_cfi_frame_initial_instructions (void)
{
  int sp_reg = IS_CSKY_V1 (mach_flag) ? 0 : 14;
  cfi_add_CFA_def_cfa_register (sp_reg);
}

/* Implement tc_regname_to_dw2regnum.  */

int
tc_csky_regname_to_dw2regnum (char *regname)
{
  int reg_num = -1;
  int len;

  /* FIXME the reg should be parsed according to
     the abi version.  */
  reg_num = csky_get_reg_val (regname, &len);
  return reg_num;
}
