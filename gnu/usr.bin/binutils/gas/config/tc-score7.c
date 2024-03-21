/* tc-score7.c -- Assembler for Score7
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by:
   Brain.lin (brain.lin@sunplusct.com)
   Mei Ligang (ligang@sunnorth.com.cn)
   Pei-Lin Tsai (pltsai@sunplus.com)

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
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "as.h"
#include "config.h"
#include "subsegs.h"
#include "safe-ctype.h"
#include "opcode/score-inst.h"
#include "libiberty.h"

#ifdef OBJ_ELF
#include "elf/score.h"
#include "dwarf2dbg.h"
#endif

static void s7_do_ldst_insn (char *);
static void s7_do_crdcrscrsimm5 (char *);
static void s7_do_ldst_unalign (char *);
static void s7_do_ldst_atomic (char *);
static void s7_do_ldst_cop (char *);
static void s7_do_macro_li_rdi32 (char *);
static void s7_do_macro_la_rdi32 (char *);
static void s7_do_macro_rdi32hi (char *);
static void s7_do_macro_rdi32lo (char *);
static void s7_do_macro_mul_rdrsrs (char *);
static void s7_do_macro_ldst_label (char *);
static void s7_do_branch (char *);
static void s7_do_jump (char *);
static void s7_do_empty (char *);
static void s7_do_rdrsrs (char *);
static void s7_do_rdsi16 (char *);
static void s7_do_rdrssi14 (char *);
static void s7_do_sub_rdsi16 (char *);
static void s7_do_sub_rdrssi14 (char *);
static void s7_do_rdrsi5 (char *);
static void s7_do_rdrsi14 (char *);
static void s7_do_rdi16 (char *);
static void s7_do_xrsi5 (char *);
static void s7_do_rdrs (char *);
static void s7_do_rdxrs (char *);
static void s7_do_rsrs (char *);
static void s7_do_rdcrs (char *);
static void s7_do_rdsrs (char *);
static void s7_do_rd (char *);
static void s7_do_rs (char *);
static void s7_do_i15 (char *);
static void s7_do_xi5x (char *);
static void s7_do_ceinst (char *);
static void s7_do_cache (char *);
static void s7_do16_rdrs (char *);
static void s7_do16_rs (char *);
static void s7_do16_xrs (char *);
static void s7_do16_mv_rdrs (char *);
static void s7_do16_hrdrs (char *);
static void s7_do16_rdhrs (char *);
static void s7_do16_rdi4 (char *);
static void s7_do16_rdi5 (char *);
static void s7_do16_xi5 (char *);
static void s7_do16_ldst_insn (char *);
static void s7_do16_ldst_imm_insn (char *);
static void s7_do16_push_pop (char *);
static void s7_do16_branch (char *);
static void s7_do16_jump (char *);
static void s7_do_rdi16_pic (char *);
static void s7_do_addi_s_pic (char *);
static void s7_do_addi_u_pic (char *);
static void s7_do_lw_pic (char *);

#define s7_GP                     28
#define s7_PIC_CALL_REG           29
#define s7_MAX_LITERAL_POOL_SIZE  1024
#define s7_FAIL	               0x80000000
#define s7_SUCCESS         0
#define s7_INSN_SIZE       4
#define s7_INSN16_SIZE     2
#define s7_RELAX_INST_NUM  3

/* For score5u : div/mul will pop warning message, mmu/alw/asw will pop error message.  */
#define s7_BAD_ARGS 	             _("bad arguments to instruction")
#define s7_ERR_FOR_SCORE5U_MUL_DIV   _("div / mul are reserved instructions")
#define s7_ERR_FOR_SCORE5U_MMU       _("This architecture doesn't support mmu")
#define s7_ERR_FOR_SCORE5U_ATOMIC    _("This architecture doesn't support atomic instruction")
#define s7_BAD_SKIP_COMMA            s7_BAD_ARGS
#define s7_BAD_GARBAGE               _("garbage following instruction");

#define s7_skip_whitespace(str)  while (*(str) == ' ') ++(str)

/* The name of the readonly data section.  */
#define s7_RDATA_SECTION_NAME (OUTPUT_FLAVOR == bfd_target_aout_flavour \
			    ? ".data" \
			    : OUTPUT_FLAVOR == bfd_target_ecoff_flavour \
			    ? ".rdata" \
			    : OUTPUT_FLAVOR == bfd_target_coff_flavour \
			    ? ".rdata" \
			    : OUTPUT_FLAVOR == bfd_target_elf_flavour \
			    ? ".rodata" \
			    : (abort (), ""))

#define s7_RELAX_ENCODE(old, new, type, reloc1, reloc2, opt) \
  ((relax_substateT) \
   (((old) << 23) \
    | ((new) << 16) \
    | ((type) << 9) \
    | ((reloc1) << 5) \
    | ((reloc2) << 1) \
    | ((opt) ? 1 : 0)))

#define s7_RELAX_OLD(i)       (((i) >> 23) & 0x7f)
#define s7_RELAX_NEW(i)       (((i) >> 16) & 0x7f)
#define s7_RELAX_TYPE(i)      (((i) >> 9) & 0x7f)
#define s7_RELAX_RELOC1(i)    ((valueT) ((i) >> 5) & 0xf)
#define s7_RELAX_RELOC2(i)    ((valueT) ((i) >> 1) & 0xf)
#define s7_RELAX_OPT(i)       ((i) & 1)
#define s7_RELAX_OPT_CLEAR(i) ((i) & ~1)

#define s7_SET_INSN_ERROR(s) (s7_inst.error = (s))
#define s7_INSN_IS_PCE_P(s)  (strstr (str, "||") != NULL)

#define s7_GET_INSN_CLASS(type) (s7_get_insn_class_from_type (type))

#define s7_GET_INSN_SIZE(type) ((s7_GET_INSN_CLASS (type) == INSN_CLASS_16) \
                             ? s7_INSN16_SIZE : s7_INSN_SIZE)

#define s7_INSN_NAME_LEN 16

/* Relax will need some padding for alignment.  */
#define s7_RELAX_PAD_BYTE 3

#define s7_USE_GLOBAL_POINTER_OPT 1



/* Enumeration matching entries in table above.  */
enum s7_score_reg_type
{
  s7_REG_TYPE_SCORE = 0,
#define REG_TYPE_FIRST s7_REG_TYPE_SCORE
  s7_REG_TYPE_SCORE_SR = 1,
  s7_REG_TYPE_SCORE_CR = 2,
  s7_REG_TYPE_MAX = 3
};

enum s7_score_pic_level
{
  s7_NO_PIC,
  s7_PIC
};
static enum s7_score_pic_level s7_score_pic = s7_NO_PIC;

enum s7_insn_type_for_dependency
{
  s7_D_pce,
  s7_D_cond_br,
  s7_D_cond_mv,
  s7_D_cached,
  s7_D_cachei,
  s7_D_ldst,
  s7_D_ldcombine,
  s7_D_mtcr,
  s7_D_mfcr,
  s7_D_mfsr,
  s7_D_mftlb,
  s7_D_mtptlb,
  s7_D_mtrtlb,
  s7_D_stlb,
  s7_D_all_insn
};

struct s7_insn_to_dependency
{
  const char *insn_name;
  enum s7_insn_type_for_dependency type;
};

struct s7_data_dependency
{
  enum s7_insn_type_for_dependency pre_insn_type;
  char pre_reg[6];
  enum s7_insn_type_for_dependency cur_insn_type;
  char cur_reg[6];
  int bubblenum_7;
  int bubblenum_5;
  int warn_or_error;           /* warning - 0; error - 1  */
};

static const struct s7_insn_to_dependency s7_insn_to_dependency_table[] =
{
  /* pce instruction.  */
  {"pce",       s7_D_pce},
  /* conditional branch instruction.  */
  {"bcs",       s7_D_cond_br},
  {"bcc",       s7_D_cond_br},
  {"bgtu",      s7_D_cond_br},
  {"bleu",      s7_D_cond_br},
  {"beq",       s7_D_cond_br},
  {"bne",       s7_D_cond_br},
  {"bgt",       s7_D_cond_br},
  {"ble",       s7_D_cond_br},
  {"bge",       s7_D_cond_br},
  {"blt",       s7_D_cond_br},
  {"bmi",       s7_D_cond_br},
  {"bpl",       s7_D_cond_br},
  {"bvs",       s7_D_cond_br},
  {"bvc",       s7_D_cond_br},
  {"bcsl",      s7_D_cond_br},
  {"bccl",      s7_D_cond_br},
  {"bgtul",     s7_D_cond_br},
  {"bleul",     s7_D_cond_br},
  {"beql",      s7_D_cond_br},
  {"bnel",      s7_D_cond_br},
  {"bgtl",      s7_D_cond_br},
  {"blel",      s7_D_cond_br},
  {"bgel",      s7_D_cond_br},
  {"bltl",      s7_D_cond_br},
  {"bmil",      s7_D_cond_br},
  {"bpll",      s7_D_cond_br},
  {"bvsl",      s7_D_cond_br},
  {"bvcl",      s7_D_cond_br},
  {"bcs!",      s7_D_cond_br},
  {"bcc!",      s7_D_cond_br},
  {"bgtu!",     s7_D_cond_br},
  {"bleu!",     s7_D_cond_br},
  {"beq!",      s7_D_cond_br},
  {"bne!",      s7_D_cond_br},
  {"bgt!",      s7_D_cond_br},
  {"ble!",      s7_D_cond_br},
  {"bge!",      s7_D_cond_br},
  {"blt!",      s7_D_cond_br},
  {"bmi!",      s7_D_cond_br},
  {"bpl!",      s7_D_cond_br},
  {"bvs!",      s7_D_cond_br},
  {"bvc!",      s7_D_cond_br},
  {"brcs",      s7_D_cond_br},
  {"brcc",      s7_D_cond_br},
  {"brgtu",     s7_D_cond_br},
  {"brleu",     s7_D_cond_br},
  {"breq",      s7_D_cond_br},
  {"brne",      s7_D_cond_br},
  {"brgt",      s7_D_cond_br},
  {"brle",      s7_D_cond_br},
  {"brge",      s7_D_cond_br},
  {"brlt",      s7_D_cond_br},
  {"brmi",      s7_D_cond_br},
  {"brpl",      s7_D_cond_br},
  {"brvs",      s7_D_cond_br},
  {"brvc",      s7_D_cond_br},
  {"brcsl",     s7_D_cond_br},
  {"brccl",     s7_D_cond_br},
  {"brgtul",    s7_D_cond_br},
  {"brleul",    s7_D_cond_br},
  {"breql",     s7_D_cond_br},
  {"brnel",     s7_D_cond_br},
  {"brgtl",     s7_D_cond_br},
  {"brlel",     s7_D_cond_br},
  {"brgel",     s7_D_cond_br},
  {"brltl",     s7_D_cond_br},
  {"brmil",     s7_D_cond_br},
  {"brpll",     s7_D_cond_br},
  {"brvsl",     s7_D_cond_br},
  {"brvcl",     s7_D_cond_br},
  {"brcs!",     s7_D_cond_br},
  {"brcc!",     s7_D_cond_br},
  {"brgtu!",    s7_D_cond_br},
  {"brleu!",    s7_D_cond_br},
  {"breq!",     s7_D_cond_br},
  {"brne!",     s7_D_cond_br},
  {"brgt!",     s7_D_cond_br},
  {"brle!",     s7_D_cond_br},
  {"brge!",     s7_D_cond_br},
  {"brlt!",     s7_D_cond_br},
  {"brmi!",     s7_D_cond_br},
  {"brpl!",     s7_D_cond_br},
  {"brvs!",     s7_D_cond_br},
  {"brvc!",     s7_D_cond_br},
  {"brcsl!",    s7_D_cond_br},
  {"brccl!",    s7_D_cond_br},
  {"brgtul!",   s7_D_cond_br},
  {"brleul!",   s7_D_cond_br},
  {"breql!",    s7_D_cond_br},
  {"brnel!",    s7_D_cond_br},
  {"brgtl!",    s7_D_cond_br},
  {"brlel!",    s7_D_cond_br},
  {"brgel!",    s7_D_cond_br},
  {"brltl!",    s7_D_cond_br},
  {"brmil!",    s7_D_cond_br},
  {"brpll!",    s7_D_cond_br},
  {"brvsl!",    s7_D_cond_br},
  {"brvcl!",    s7_D_cond_br},
  /* conditional move instruction.  */
  {"mvcs",      s7_D_cond_mv},
  {"mvcc",      s7_D_cond_mv},
  {"mvgtu",     s7_D_cond_mv},
  {"mvleu",     s7_D_cond_mv},
  {"mveq",      s7_D_cond_mv},
  {"mvne",      s7_D_cond_mv},
  {"mvgt",      s7_D_cond_mv},
  {"mvle",      s7_D_cond_mv},
  {"mvge",      s7_D_cond_mv},
  {"mvlt",      s7_D_cond_mv},
  {"mvmi",      s7_D_cond_mv},
  {"mvpl",      s7_D_cond_mv},
  {"mvvs",      s7_D_cond_mv},
  {"mvvc",      s7_D_cond_mv},
  /* move special instruction.  */
  {"mtcr",      s7_D_mtcr},
  {"mftlb",     s7_D_mftlb},
  {"mtptlb",    s7_D_mtptlb},
  {"mtrtlb",    s7_D_mtrtlb},
  {"stlb",      s7_D_stlb},
  {"mfcr",      s7_D_mfcr},
  {"mfsr",      s7_D_mfsr},
  /* cache instruction.  */
  {"cache 8",   s7_D_cached},
  {"cache 9",   s7_D_cached},
  {"cache 10",  s7_D_cached},
  {"cache 11",  s7_D_cached},
  {"cache 12",  s7_D_cached},
  {"cache 13",  s7_D_cached},
  {"cache 14",  s7_D_cached},
  {"cache 24",  s7_D_cached},
  {"cache 26",  s7_D_cached},
  {"cache 27",  s7_D_cached},
  {"cache 29",  s7_D_cached},
  {"cache 30",  s7_D_cached},
  {"cache 31",  s7_D_cached},
  {"cache 0",   s7_D_cachei},
  {"cache 1",   s7_D_cachei},
  {"cache 2",   s7_D_cachei},
  {"cache 3",   s7_D_cachei},
  {"cache 4",   s7_D_cachei},
  {"cache 16",  s7_D_cachei},
  {"cache 17",  s7_D_cachei},
  /* load/store instruction.  */
  {"lb",        s7_D_ldst},
  {"lbu",       s7_D_ldst},
  {"lbu!",      s7_D_ldst},
  {"lbup!",     s7_D_ldst},
  {"lh",        s7_D_ldst},
  {"lhu",       s7_D_ldst},
  {"lh!",       s7_D_ldst},
  {"lhp!",      s7_D_ldst},
  {"lw",        s7_D_ldst},
  {"lw!",       s7_D_ldst},
  {"lwp!",      s7_D_ldst},
  {"sb",        s7_D_ldst},
  {"sb!",       s7_D_ldst},
  {"sbp!",      s7_D_ldst},
  {"sh",        s7_D_ldst},
  {"sh!",       s7_D_ldst},
  {"shp!",      s7_D_ldst},
  {"sw",        s7_D_ldst},
  {"sw!",       s7_D_ldst},
  {"swp!",      s7_D_ldst},
  {"alw",       s7_D_ldst},
  {"asw",       s7_D_ldst},
  {"push!",     s7_D_ldst},
  {"pushhi!",   s7_D_ldst},
  {"pop!",      s7_D_ldst},
  {"pophi!",    s7_D_ldst},
  {"ldc1",      s7_D_ldst},
  {"ldc2",      s7_D_ldst},
  {"ldc3",      s7_D_ldst},
  {"stc1",      s7_D_ldst},
  {"stc2",      s7_D_ldst},
  {"stc3",      s7_D_ldst},
  {"scb",       s7_D_ldst},
  {"scw",       s7_D_ldst},
  {"sce",       s7_D_ldst},
  /* load combine instruction.  */
  {"lcb",       s7_D_ldcombine},
  {"lcw",       s7_D_ldcombine},
  {"lce",       s7_D_ldcombine},
};

static const struct s7_data_dependency s7_data_dependency_table[] =
{
  /* Condition register.  */
  {s7_D_mtcr, "cr1", s7_D_pce, "", 2, 1, 0},
  {s7_D_mtcr, "cr1", s7_D_cond_br, "", 1, 0, 1},
  {s7_D_mtcr, "cr1", s7_D_cond_mv, "", 1, 0, 1},
  /* Status register.  */
  {s7_D_mtcr, "cr0", s7_D_all_insn, "", 5, 4, 0},
  /* CCR register.  */
  {s7_D_mtcr, "cr4", s7_D_all_insn, "", 6, 5, 0},
  /* EntryHi/EntryLo register.  */
  {s7_D_mftlb, "", s7_D_mtptlb, "", 1, 1, 1},
  {s7_D_mftlb, "", s7_D_mtrtlb, "", 1, 1, 1},
  {s7_D_mftlb, "", s7_D_stlb, "", 1, 1,1},
  {s7_D_mftlb, "", s7_D_mfcr, "cr11", 1, 1, 1},
  {s7_D_mftlb, "", s7_D_mfcr, "cr12", 1, 1, 1},
  /* Index register.  */
  {s7_D_stlb, "", s7_D_mtptlb, "", 1, 1, 1},
  {s7_D_stlb, "", s7_D_mftlb, "", 1, 1, 1},
  {s7_D_stlb, "", s7_D_mfcr, "cr8", 2, 2, 1},
  /* Cache.  */
  {s7_D_cached, "", s7_D_ldst, "", 1, 1, 0},
  {s7_D_cached, "", s7_D_ldcombine, "", 1, 1, 0},
  {s7_D_cachei, "", s7_D_all_insn, "", 5, 4, 0},
  /* Load combine.  */
  {s7_D_ldcombine, "", s7_D_mfsr, "sr1", 3, 3, 1},
};



/* Used to contain constructed error messages.  */
static char s7_err_msg[255];
static int s7_fix_data_dependency = 0;
static int s7_warn_fix_data_dependency = 1;

static int s7_in_my_get_expression = 0;

/* Default, pop warning message when using r1.  */
static int s7_nor1 = 1;

/* Default will do instruction relax, -O0 will set s7_g_opt = 0.  */
static unsigned int s7_g_opt = 1;

/* The size of the small data section.  */
static unsigned int s7_g_switch_value = 8;

static segT s7_pdr_seg;

struct s7_score_it
{
  char name[s7_INSN_NAME_LEN];
  unsigned long instruction;
  unsigned long relax_inst;
  int size;
  int relax_size;
  enum score_insn_type type;
  char str[s7_MAX_LITERAL_POOL_SIZE];
  const char *error;
  int bwarn;
  char reg[s7_INSN_NAME_LEN];
  struct
  {
    bfd_reloc_code_real_type type;
    expressionS exp;
    int pc_rel;
  }reloc;
};
static struct s7_score_it s7_inst;

typedef struct proc
{
  symbolS *isym;
  unsigned long reg_mask;
  unsigned long reg_offset;
  unsigned long fpreg_mask;
  unsigned long leaf;
  unsigned long frame_offset;
  unsigned long frame_reg;
  unsigned long pc_reg;
} s7_procS;
static s7_procS s7_cur_proc;
static s7_procS *s7_cur_proc_ptr;
static int s7_numprocs;

/* Structure for a hash table entry for a register.  */
struct s7_reg_entry
{
  const char *name;
  int number;
};

static const struct s7_reg_entry s7_score_rn_table[] =
{
  {"r0", 0}, {"r1", 1}, {"r2", 2}, {"r3", 3},
  {"r4", 4}, {"r5", 5}, {"r6", 6}, {"r7", 7},
  {"r8", 8}, {"r9", 9}, {"r10", 10}, {"r11", 11},
  {"r12", 12}, {"r13", 13}, {"r14", 14}, {"r15", 15},
  {"r16", 16}, {"r17", 17}, {"r18", 18}, {"r19", 19},
  {"r20", 20}, {"r21", 21}, {"r22", 22}, {"r23", 23},
  {"r24", 24}, {"r25", 25}, {"r26", 26}, {"r27", 27},
  {"r28", 28}, {"r29", 29}, {"r30", 30}, {"r31", 31},
  {NULL, 0}
};

static const struct s7_reg_entry s7_score_srn_table[] =
{
  {"sr0", 0}, {"sr1", 1}, {"sr2", 2},
  {NULL, 0}
};

static const struct s7_reg_entry s7_score_crn_table[] =
{
  {"cr0", 0}, {"cr1", 1}, {"cr2", 2}, {"cr3", 3},
  {"cr4", 4}, {"cr5", 5}, {"cr6", 6}, {"cr7", 7},
  {"cr8", 8}, {"cr9", 9}, {"cr10", 10}, {"cr11", 11},
  {"cr12", 12}, {"cr13", 13}, {"cr14", 14}, {"cr15", 15},
  {"cr16", 16}, {"cr17", 17}, {"cr18", 18}, {"cr19", 19},
  {"cr20", 20}, {"cr21", 21}, {"cr22", 22}, {"cr23", 23},
  {"cr24", 24}, {"cr25", 25}, {"cr26", 26}, {"cr27", 27},
  {"cr28", 28}, {"cr29", 29}, {"cr30", 30}, {"cr31", 31},
  {NULL, 0}
};

struct s7_reg_map
{
  const struct s7_reg_entry *names;
  int max_regno;
  htab_t htab;
  const char *expected;
};

static struct s7_reg_map s7_all_reg_maps[] =
{
  {s7_score_rn_table, 31, NULL, N_("S+core register expected")},
  {s7_score_srn_table, 2, NULL, N_("S+core special-register expected")},
  {s7_score_crn_table, 31, NULL, N_("S+core co-processor register expected")},
};

static htab_t s7_score_ops_hsh = NULL;
static htab_t s7_dependency_insn_hsh = NULL;


struct s7_datafield_range
{
  int data_type;
  int bits;
  int range[2];
};

static struct s7_datafield_range s7_score_df_range[] =
{
  {_IMM4,             4,  {0, (1 << 4) - 1}},	        /* (     0 ~ 15   ) */
  {_IMM5,             5,  {0, (1 << 5) - 1}},	        /* (     0 ~ 31   ) */
  {_IMM8,             8,  {0, (1 << 8) - 1}},	        /* (     0 ~ 255  ) */
  {_IMM14,            14, {0, (1 << 14) - 1}},	        /* (     0 ~ 16383) */
  {_IMM15,            15, {0, (1 << 15) - 1}},	        /* (     0 ~ 32767) */
  {_IMM16,            16, {0, (1 << 16) - 1}},	        /* (     0 ~ 65535) */
  {_SIMM10,           10, {-(1 << 9), (1 << 9) - 1}},	/* (  -512 ~ 511  ) */
  {_SIMM12,           12, {-(1 << 11), (1 << 11) - 1}},	/* ( -2048 ~ 2047 ) */
  {_SIMM14,           14, {-(1 << 13), (1 << 13) - 1}},	/* ( -8192 ~ 8191 ) */
  {_SIMM15,           15, {-(1 << 14), (1 << 14) - 1}},	/* (-16384 ~ 16383) */
  {_SIMM16,           16, {-(1 << 15), (1 << 15) - 1}},	/* (-32768 ~ 32767) */
  {_SIMM14_NEG,       14, {-(1 << 13), (1 << 13) - 1}},	/* ( -8191 ~ 8192 ) */
  {_IMM16_NEG,        16, {0, (1 << 16) - 1}},	        /* (-65535 ~ 0    ) */
  {_SIMM16_NEG,       16, {-(1 << 15), (1 << 15) - 1}},	/* (-32768 ~ 32767) */
  {_IMM20,            20, {0, (1 << 20) - 1}},
  {_IMM25,            25, {0, (1 << 25) - 1}},
  {_DISP8div2,        8,  {-(1 << 8), (1 << 8) - 1}},	/* (  -256 ~ 255  ) */
  {_DISP11div2,       11, {0, 0}},
  {_DISP19div2,       19, {-(1 << 19), (1 << 19) - 1}},	/* (-524288 ~ 524287) */
  {_DISP24div2,       24, {0, 0}},
  {_VALUE,            32, {0, ((unsigned int)1 << 31) - 1}},
  {_VALUE_HI16,       16, {0, (1 << 16) - 1}},
  {_VALUE_LO16,       16, {0, (1 << 16) - 1}},
  {_VALUE_LDST_LO16,  16, {0, (1 << 16) - 1}},
  {_SIMM16_LA,        16, {-(1 << 15), (1 << 15) - 1}},	/* (-32768 ~ 32767) */
  {_IMM5_RSHIFT_1,    5,  {0, (1 << 6) - 1}},	        /* (     0 ~ 63   ) */
  {_IMM5_RSHIFT_2,    5,  {0, (1 << 7) - 1}},	        /* (     0 ~ 127  ) */
  {_SIMM16_LA_POS,    16, {0, (1 << 15) - 1}},	        /* (     0 ~ 32767) */
  {_IMM5_RANGE_8_31,  5,  {8, 31}},	                /* But for cop0 the valid data : (8 ~ 31). */
  {_IMM10_RSHIFT_2,   10, {-(1 << 11), (1 << 11) - 1}},	/* For ldc#, stc#. */
  {_SIMM10,           10, {0, (1 << 10) - 1}},	        /* ( -1024 ~ 1023 ) */
  {_SIMM12,           12, {0, (1 << 12) - 1}},	        /* ( -2048 ~ 2047 ) */
  {_SIMM14,           14, {0, (1 << 14) - 1}},          /* ( -8192 ~ 8191 ) */
  {_SIMM15,           15, {0, (1 << 15) - 1}},	        /* (-16384 ~ 16383) */
  {_SIMM16,           16, {0, (1 << 16) - 1}},	        /* (-65536 ~ 65536) */
  {_SIMM14_NEG,       14, {0, (1 << 16) - 1}},          /* ( -8191 ~ 8192 ) */
  {_IMM16_NEG,        16, {0, (1 << 16) - 1}},	        /* ( 65535 ~ 0    ) */
  {_SIMM16_NEG,       16, {0, (1 << 16) - 1}},	        /* ( 65535 ~ 0    ) */
  {_IMM20,            20, {0, (1 << 20) - 1}},	        /* (-32768 ~ 32767) */
  {_IMM25,            25, {0, (1 << 25) - 1}},	        /* (-32768 ~ 32767) */
  {_GP_IMM15,         15, {0, (1 << 15) - 1}},	        /* (     0 ~ 65535) */
  {_GP_IMM14,         14, {0, (1 << 14) - 1}},	        /* (     0 ~ 65535) */
  {_SIMM16_pic,       16, {-(1 << 15), (1 << 15) - 1}},	/* (-32768 ~ 32767) */
  {_IMM16_LO16_pic,   16, {0, (1 << 16) - 1}},	        /* ( 65535 ~ 0    ) */
  {_IMM16_pic,        16, {0, (1 << 16) - 1}},	        /* (     0 ~ 65535) */
};


struct s7_asm_opcode
{
  /* Instruction name.  */
  const char *template_name;

  /* Instruction Opcode.  */
  bfd_vma value;

  /* Instruction bit mask.  */
  bfd_vma bitmask;

  /* Relax instruction opcode.  0x8000 imply no relaxation.  */
  bfd_vma relax_value;

  /* Instruction type.  */
  enum score_insn_type type;

  /* Function to call to parse args.  */
  void (*parms) (char *);
};

static const struct s7_asm_opcode s7_score_ldst_insns[] =
{
  {"lw",        0x20000000, 0x3e000000, 0x2008,     Rd_rvalueRs_SI15,     s7_do_ldst_insn},
  {"lw",        0x06000000, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s7_do_ldst_insn},
  {"lw",        0x0e000000, 0x3e000007, 0x200a,     Rd_rvalueRs_postSI12, s7_do_ldst_insn},
  {"lh",        0x22000000, 0x3e000000, 0x2009,     Rd_rvalueRs_SI15,     s7_do_ldst_insn},
  {"lh",        0x06000001, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s7_do_ldst_insn},
  {"lh",        0x0e000001, 0x3e000007, 0x8000,     Rd_rvalueRs_postSI12, s7_do_ldst_insn},
  {"lhu",       0x24000000, 0x3e000000, 0x8000,     Rd_rvalueRs_SI15,     s7_do_ldst_insn},
  {"lhu",       0x06000002, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s7_do_ldst_insn},
  {"lhu",       0x0e000002, 0x3e000007, 0x8000,     Rd_rvalueRs_postSI12, s7_do_ldst_insn},
  {"lb",        0x26000000, 0x3e000000, 0x8000,     Rd_rvalueRs_SI15,     s7_do_ldst_insn},
  {"lb",        0x06000003, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s7_do_ldst_insn},
  {"lb",        0x0e000003, 0x3e000007, 0x8000,     Rd_rvalueRs_postSI12, s7_do_ldst_insn},
  {"sw",        0x28000000, 0x3e000000, 0x200c,     Rd_lvalueRs_SI15,     s7_do_ldst_insn},
  {"sw",        0x06000004, 0x3e000007, 0x200e,     Rd_lvalueRs_preSI12,  s7_do_ldst_insn},
  {"sw",        0x0e000004, 0x3e000007, 0x8000,     Rd_lvalueRs_postSI12, s7_do_ldst_insn},
  {"sh",        0x2a000000, 0x3e000000, 0x200d,     Rd_lvalueRs_SI15,     s7_do_ldst_insn},
  {"sh",        0x06000005, 0x3e000007, 0x8000,     Rd_lvalueRs_preSI12,  s7_do_ldst_insn},
  {"sh",        0x0e000005, 0x3e000007, 0x8000,     Rd_lvalueRs_postSI12, s7_do_ldst_insn},
  {"lbu",       0x2c000000, 0x3e000000, 0x200b,     Rd_rvalueRs_SI15,     s7_do_ldst_insn},
  {"lbu",       0x06000006, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s7_do_ldst_insn},
  {"lbu",       0x0e000006, 0x3e000007, 0x8000,     Rd_rvalueRs_postSI12, s7_do_ldst_insn},
  {"sb",        0x2e000000, 0x3e000000, 0x200f,     Rd_lvalueRs_SI15,     s7_do_ldst_insn},
  {"sb",        0x06000007, 0x3e000007, 0x8000,     Rd_lvalueRs_preSI12,  s7_do_ldst_insn},
  {"sb",        0x0e000007, 0x3e000007, 0x8000,     Rd_lvalueRs_postSI12, s7_do_ldst_insn},
};

static const struct s7_asm_opcode s7_score_insns[] =
{
  {"abs",       0x3800000a, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"abs.s",     0x3800004b, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"add",       0x00000010, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"add.c",     0x00000011, 0x3e0003ff, 0x2000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"add.s",     0x38000048, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"addc",      0x00000012, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"addc.c",    0x00000013, 0x3e0003ff, 0x0009,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"addi",      0x02000000, 0x3e0e0001, 0x8000,     Rd_SI16,              s7_do_rdsi16},
  {"addi.c",    0x02000001, 0x3e0e0001, 0x8000,     Rd_SI16,              s7_do_rdsi16},
  {"addis",     0x0a000000, 0x3e0e0001, 0x8000,     Rd_SI16,              s7_do_rdi16},
  {"addis.c",   0x0a000001, 0x3e0e0001, 0x8000,     Rd_SI16,              s7_do_rdi16},
  {"addri",     0x10000000, 0x3e000001, 0x8000,     Rd_Rs_SI14,           s7_do_rdrssi14},
  {"addri.c",   0x10000001, 0x3e000001, 0x8000,     Rd_Rs_SI14,           s7_do_rdrssi14},
  {"addc!",     0x0009,     0x700f,     0x00000013, Rd_Rs,                s7_do16_rdrs},
  {"add!",      0x2000,     0x700f,     0x00000011, Rd_Rs,                s7_do16_rdrs},
  {"addei!",    0x6000    , 0x7087,     0x02000001, Rd_I4,                s7_do16_rdi4},
  {"subi",      0x02000000, 0x3e0e0001, 0x8000,     Rd_SI16,              s7_do_sub_rdsi16},
  {"subi.c",    0x02000001, 0x3e0e0001, 0x8000,     Rd_SI16,              s7_do_sub_rdsi16},
  {"subri",     0x10000000, 0x3e000001, 0x8000,     Rd_Rs_SI14,           s7_do_sub_rdrssi14},
  {"subri.c",   0x10000001, 0x3e000001, 0x8000,     Rd_Rs_SI14,           s7_do_sub_rdrssi14},
  {"and",       0x00000020, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"and.c",     0x00000021, 0x3e0003ff, 0x2004,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"andi",      0x02080000, 0x3e0e0001, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"andi.c",    0x02080001, 0x3e0e0001, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"andis",     0x0a080000, 0x3e0e0001, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"andis.c",   0x0a080001, 0x3e0e0001, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"andri",     0x18000000, 0x3e000001, 0x8000,     Rd_Rs_I14,            s7_do_rdrsi14},
  {"andri.c",   0x18000001, 0x3e000001, 0x8000,     Rd_Rs_I14,            s7_do_rdrsi14},
  {"and!",      0x2004,     0x700f,     0x00000021, Rd_Rs,                s7_do16_rdrs},
  {"bcs",       0x08000000, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bcc",       0x08000400, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bcnz",      0x08003800, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bcsl",      0x08000001, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bccl",      0x08000401, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bcnzl",     0x08003801, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bcs!",      0x4000,     0x7f00,     0x08000000, PC_DISP8div2,         s7_do16_branch},
  {"bcc!",      0x4100,     0x7f00,     0x08000400, PC_DISP8div2,         s7_do16_branch},
  {"bcnz!",     0x4e00,     0x7f00,     0x08003800, PC_DISP8div2,         s7_do16_branch},
  {"beq",       0x08001000, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"beql",      0x08001001, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"beq!",      0x4400,     0x7f00,     0x08001000, PC_DISP8div2,         s7_do16_branch},
  {"bgtu",      0x08000800, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bgt",       0x08001800, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bge",       0x08002000, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bgtul",     0x08000801, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bgtl",      0x08001801, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bgel",      0x08002001, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bgtu!",     0x4200,     0x7f00,     0x08000800, PC_DISP8div2,         s7_do16_branch},
  {"bgt!",      0x4600,     0x7f00,     0x08001800, PC_DISP8div2,         s7_do16_branch},
  {"bge!",      0x4800,     0x7f00,     0x08002000, PC_DISP8div2,         s7_do16_branch},
  {"bitclr.c",  0x00000029, 0x3e0003ff, 0x6004,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"bitrev",    0x3800000c, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"bitset.c",  0x0000002b, 0x3e0003ff, 0x6005,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"bittst.c",  0x0000002d, 0x3e0003ff, 0x6006,     x_Rs_I5,              s7_do_xrsi5},
  {"bittgl.c",  0x0000002f, 0x3e0003ff, 0x6007,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"bitclr!",   0x6004,     0x7007,     0x00000029, Rd_I5,                s7_do16_rdi5},
  {"bitset!",   0x6005,     0x7007,     0x0000002b, Rd_I5,                s7_do16_rdi5},
  {"bittst!",   0x6006,     0x7007,     0x0000002d, Rd_I5,                s7_do16_rdi5},
  {"bittgl!",   0x6007,     0x7007,     0x0000002f, Rd_I5,                s7_do16_rdi5},
  {"bleu",      0x08000c00, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"ble",       0x08001c00, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"blt",       0x08002400, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bleul",     0x08000c01, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"blel",      0x08001c01, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bltl",      0x08002401, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bl",        0x08003c01, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bleu!",     0x4300,     0x7f00,     0x08000c00, PC_DISP8div2,         s7_do16_branch},
  {"ble!",      0x4700,     0x7f00,     0x08001c00, PC_DISP8div2,         s7_do16_branch},
  {"blt!",      0x4900,     0x7f00,     0x08002400, PC_DISP8div2,         s7_do16_branch},
  {"bmi",       0x08002800, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bmil",      0x08002801, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bmi!",      0x00004a00, 0x00007f00, 0x08002800, PC_DISP8div2,         s7_do16_branch},
  {"bne",       0x08001400, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bnel",      0x08001401, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bne!",      0x4500,     0x7f00,     0x08001400, PC_DISP8div2,         s7_do16_branch},
  {"bpl",       0x08002c00, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bpll",      0x08002c01, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bpl!",      0x4b00,     0x7f00,     0x08002c00, PC_DISP8div2,         s7_do16_branch},
  {"brcs",      0x00000008, 0x3e007fff, 0x0004,     x_Rs_x,               s7_do_rs},
  {"brcc",      0x00000408, 0x3e007fff, 0x0104,     x_Rs_x,               s7_do_rs},
  {"brgtu",     0x00000808, 0x3e007fff, 0x0204,     x_Rs_x,               s7_do_rs},
  {"brleu",     0x00000c08, 0x3e007fff, 0x0304,     x_Rs_x,               s7_do_rs},
  {"breq",      0x00001008, 0x3e007fff, 0x0404,     x_Rs_x,               s7_do_rs},
  {"brne",      0x00001408, 0x3e007fff, 0x0504,     x_Rs_x,               s7_do_rs},
  {"brgt",      0x00001808, 0x3e007fff, 0x0604,     x_Rs_x,               s7_do_rs},
  {"brle",      0x00001c08, 0x3e007fff, 0x0704,     x_Rs_x,               s7_do_rs},
  {"brge",      0x00002008, 0x3e007fff, 0x0804,     x_Rs_x,               s7_do_rs},
  {"brlt",      0x00002408, 0x3e007fff, 0x0904,     x_Rs_x,               s7_do_rs},
  {"brmi",      0x00002808, 0x3e007fff, 0x0a04,     x_Rs_x,               s7_do_rs},
  {"brpl",      0x00002c08, 0x3e007fff, 0x0b04,     x_Rs_x,               s7_do_rs},
  {"brvs",      0x00003008, 0x3e007fff, 0x0c04,     x_Rs_x,               s7_do_rs},
  {"brvc",      0x00003408, 0x3e007fff, 0x0d04,     x_Rs_x,               s7_do_rs},
  {"brcnz",     0x00003808, 0x3e007fff, 0x0e04,     x_Rs_x,               s7_do_rs},
  {"br",        0x00003c08, 0x3e007fff, 0x0f04,     x_Rs_x,               s7_do_rs},
  {"brcsl",     0x00000009, 0x3e007fff, 0x000c,     x_Rs_x,               s7_do_rs},
  {"brccl",     0x00000409, 0x3e007fff, 0x010c,     x_Rs_x,               s7_do_rs},
  {"brgtul",    0x00000809, 0x3e007fff, 0x020c,     x_Rs_x,               s7_do_rs},
  {"brleul",    0x00000c09, 0x3e007fff, 0x030c,     x_Rs_x,               s7_do_rs},
  {"breql",     0x00001009, 0x3e007fff, 0x040c,     x_Rs_x,               s7_do_rs},
  {"brnel",     0x00001409, 0x3e007fff, 0x050c,     x_Rs_x,               s7_do_rs},
  {"brgtl",     0x00001809, 0x3e007fff, 0x060c,     x_Rs_x,               s7_do_rs},
  {"brlel",     0x00001c09, 0x3e007fff, 0x070c,     x_Rs_x,               s7_do_rs},
  {"brgel",     0x00002009, 0x3e007fff, 0x080c,     x_Rs_x,               s7_do_rs},
  {"brltl",     0x00002409, 0x3e007fff, 0x090c,     x_Rs_x,               s7_do_rs},
  {"brmil",     0x00002809, 0x3e007fff, 0x0a0c,     x_Rs_x,               s7_do_rs},
  {"brpll",     0x00002c09, 0x3e007fff, 0x0b0c,     x_Rs_x,               s7_do_rs},
  {"brvsl",     0x00003009, 0x3e007fff, 0x0c0c,     x_Rs_x,               s7_do_rs},
  {"brvcl",     0x00003409, 0x3e007fff, 0x0d0c,     x_Rs_x,               s7_do_rs},
  {"brcnzl",    0x00003809, 0x3e007fff, 0x0e0c,     x_Rs_x,               s7_do_rs},
  {"brl",       0x00003c09, 0x3e007fff, 0x0f0c,     x_Rs_x,               s7_do_rs},
  {"brcs!",     0x0004,     0x7f0f,     0x00000008, x_Rs,                 s7_do16_xrs},
  {"brcc!",     0x0104,     0x7f0f,     0x00000408, x_Rs,                 s7_do16_xrs},
  {"brgtu!",    0x0204,     0x7f0f,     0x00000808, x_Rs,                 s7_do16_xrs},
  {"brleu!",    0x0304,     0x7f0f,     0x00000c08, x_Rs,                 s7_do16_xrs},
  {"breq!",     0x0404,     0x7f0f,     0x00001008, x_Rs,                 s7_do16_xrs},
  {"brne!",     0x0504,     0x7f0f,     0x00001408, x_Rs,                 s7_do16_xrs},
  {"brgt!",     0x0604,     0x7f0f,     0x00001808, x_Rs,                 s7_do16_xrs},
  {"brle!",     0x0704,     0x7f0f,     0x00001c08, x_Rs,                 s7_do16_xrs},
  {"brge!",     0x0804,     0x7f0f,     0x00002008, x_Rs,                 s7_do16_xrs},
  {"brlt!",     0x0904,     0x7f0f,     0x00002408, x_Rs,                 s7_do16_xrs},
  {"brmi!",     0x0a04,     0x7f0f,     0x00002808, x_Rs,                 s7_do16_xrs},
  {"brpl!",     0x0b04,     0x7f0f,     0x00002c08, x_Rs,                 s7_do16_xrs},
  {"brvs!",     0x0c04,     0x7f0f,     0x00003008, x_Rs,                 s7_do16_xrs},
  {"brvc!",     0x0d04,     0x7f0f,     0x00003408, x_Rs,                 s7_do16_xrs},
  {"brcnz!",    0x0e04,     0x7f0f,     0x00003808, x_Rs,                 s7_do16_xrs},
  {"br!",       0x0f04,     0x7f0f,     0x00003c08, x_Rs,                 s7_do16_xrs},
  {"brcsl!",    0x000c,     0x7f0f,     0x00000009, x_Rs,                 s7_do16_xrs},
  {"brccl!",    0x010c,     0x7f0f,     0x00000409, x_Rs,                 s7_do16_xrs},
  {"brgtul!",   0x020c,     0x7f0f,     0x00000809, x_Rs,                 s7_do16_xrs},
  {"brleul!",   0x030c,     0x7f0f,     0x00000c09, x_Rs,                 s7_do16_xrs},
  {"breql!",    0x040c,     0x7f0f,     0x00001009, x_Rs,                 s7_do16_xrs},
  {"brnel!",    0x050c,     0x7f0f,     0x00001409, x_Rs,                 s7_do16_xrs},
  {"brgtl!",    0x060c,     0x7f0f,     0x00001809, x_Rs,                 s7_do16_xrs},
  {"brlel!",    0x070c,     0x7f0f,     0x00001c09, x_Rs,                 s7_do16_xrs},
  {"brgel!",    0x080c,     0x7f0f,     0x00002009, x_Rs,                 s7_do16_xrs},
  {"brltl!",    0x090c,     0x7f0f,     0x00002409, x_Rs,                 s7_do16_xrs},
  {"brmil!",    0x0a0c,     0x7f0f,     0x00002809, x_Rs,                 s7_do16_xrs},
  {"brpll!",    0x0b0c,     0x7f0f,     0x00002c09, x_Rs,                 s7_do16_xrs},
  {"brvsl!",    0x0c0c,     0x7f0f,     0x00003009, x_Rs,                 s7_do16_xrs},
  {"brvcl!",    0x0d0c,     0x7f0f,     0x00003409, x_Rs,                 s7_do16_xrs},
  {"brcnzl!",   0x0e0c,     0x7f0f,     0x00003809, x_Rs,                 s7_do16_xrs},
  {"brl!",      0x0f0c,     0x7f0f,     0x00003c09, x_Rs,                 s7_do16_xrs},
  {"bvs",       0x08003000, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bvc",       0x08003400, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"bvsl",      0x08003001, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bvcl",      0x08003401, 0x3e007c01, 0x8000,     PC_DISP19div2,        s7_do_branch},
  {"bvs!",      0x4c00,     0x7f00,     0x08003000, PC_DISP8div2,         s7_do16_branch},
  {"bvc!",      0x4d00,     0x7f00,     0x08003400, PC_DISP8div2,         s7_do16_branch},
  {"b!",        0x4f00,     0x7f00,     0x08003c00, PC_DISP8div2,         s7_do16_branch},
  {"b",         0x08003c00, 0x3e007c01, 0x4000,     PC_DISP19div2,        s7_do_branch},
  {"cache",     0x30000000, 0x3ff00000, 0x8000,     OP5_rvalueRs_SI15,    s7_do_cache},
  {"ceinst",    0x38000000, 0x3e000000, 0x8000,     I5_Rs_Rs_I5_OP5,      s7_do_ceinst},
  {"clz",       0x3800000d, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"cmpteq.c",  0x00000019, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"cmptmi.c",  0x00100019, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"cmp.c",     0x00300019, 0x3ff003ff, 0x2003,     x_Rs_Rs,              s7_do_rsrs},
  {"cmpzteq.c", 0x0000001b, 0x3ff07fff, 0x8000,     x_Rs_x,               s7_do_rs},
  {"cmpztmi.c", 0x0010001b, 0x3ff07fff, 0x8000,     x_Rs_x,               s7_do_rs},
  {"cmpz.c",    0x0030001b, 0x3ff07fff, 0x8000,     x_Rs_x,               s7_do_rs},
  {"cmpi.c",    0x02040001, 0x3e0e0001, 0x8000,     Rd_SI16,              s7_do_rdsi16},
  {"cmp!",      0x2003,     0x700f,     0x00300019, Rd_Rs,                s7_do16_rdrs},
  {"cop1",      0x0c00000c, 0x3e00001f, 0x8000,     Rd_Rs_Rs_imm,         s7_do_crdcrscrsimm5},
  {"cop2",      0x0c000014, 0x3e00001f, 0x8000,     Rd_Rs_Rs_imm,         s7_do_crdcrscrsimm5},
  {"cop3",      0x0c00001c, 0x3e00001f, 0x8000,     Rd_Rs_Rs_imm,         s7_do_crdcrscrsimm5},
  {"drte",      0x0c0000a4, 0x3e0003ff, 0x8000,     NO_OPD,               s7_do_empty},
  {"extsb",     0x00000058, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"extsb.c",   0x00000059, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"extsh",     0x0000005a, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"extsh.c",   0x0000005b, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"extzb",     0x0000005c, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"extzb.c",   0x0000005d, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"extzh",     0x0000005e, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"extzh.c",   0x0000005f, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"jl",        0x04000001, 0x3e000001, 0x8000,     PC_DISP24div2,        s7_do_jump},
  {"jl!",       0x3001,     0x7001,     0x04000001, PC_DISP11div2,        s7_do16_jump},
  {"j!",        0x3000,     0x7001,     0x04000000, PC_DISP11div2,        s7_do16_jump},
  {"j",         0x04000000, 0x3e000001, 0x8000,     PC_DISP24div2,        s7_do_jump},
  {"lbu!",      0x200b,     0x0000700f, 0x2c000000, Rd_rvalueRs,          s7_do16_ldst_insn},
  {"lbup!",     0x7003,     0x7007,     0x2c000000, Rd_rvalueBP_I5,       s7_do16_ldst_imm_insn},
  {"alw",       0x0000000c, 0x3e0003ff, 0x8000,     Rd_rvalue32Rs,        s7_do_ldst_atomic},
  {"lcb",       0x00000060, 0x3e0003ff, 0x8000,     x_rvalueRs_post4,     s7_do_ldst_unalign},
  {"lcw",       0x00000062, 0x3e0003ff, 0x8000,     Rd_rvalueRs_post4,    s7_do_ldst_unalign},
  {"lce",       0x00000066, 0x3e0003ff, 0x8000,     Rd_rvalueRs_post4,    s7_do_ldst_unalign},
  {"ldc1",      0x0c00000a, 0x3e00001f, 0x8000,     Rd_rvalueRs_SI10,     s7_do_ldst_cop},
  {"ldc2",      0x0c000012, 0x3e00001f, 0x8000,     Rd_rvalueRs_SI10,     s7_do_ldst_cop},
  {"ldc3",      0x0c00001a, 0x3e00001f, 0x8000,     Rd_rvalueRs_SI10,     s7_do_ldst_cop},
  {"lh!",       0x2009,     0x700f,     0x22000000, Rd_rvalueRs,          s7_do16_ldst_insn},
  {"lhp!",      0x7001,     0x7007,     0x22000000, Rd_rvalueBP_I5,       s7_do16_ldst_imm_insn},
  {"ldi",       0x020c0000, 0x3e0e0000, 0x5000,     Rd_SI16,              s7_do_rdsi16},
  {"ldis",      0x0a0c0000, 0x3e0e0000, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"ldiu!",     0x5000,     0x7000,     0x020c0000, Rd_I8,                s7_do16_ldst_imm_insn},
  {"lw!",       0x2008,     0x700f,     0x20000000, Rd_rvalueRs,          s7_do16_ldst_insn},
  {"lwp!",      0x7000,     0x7007,     0x20000000, Rd_rvalueBP_I5,       s7_do16_ldst_imm_insn},
  {"mfcel",     0x00000448, 0x3e007fff, 0x8000,     Rd_x_x,               s7_do_rd},
  {"mfcel!",    0x1001,     0x7f0f,     0x00000448, x_Rs,                 s7_do16_rs},
  {"mad",       0x38000000, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mad.f!",    0x1004,     0x700f,     0x38000080, Rd_Rs,                s7_do16_rdrs},
  {"madh",      0x38000203, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"madh.fs",   0x380002c3, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"madh.fs!",  0x100b,     0x700f,     0x380002c3, Rd_Rs,                s7_do16_rdrs},
  {"madl",      0x38000002, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"madl.fs",   0x380000c2, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"madl.fs!",  0x100a,     0x700f,     0x380000c2, Rd_Rs,                s7_do16_rdrs},
  {"madu",      0x38000020, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"madu!",     0x1005,     0x700f,     0x38000020, Rd_Rs,                s7_do16_rdrs},
  {"mad.f",     0x38000080, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"max",       0x38000007, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"mazh",      0x38000303, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mazh.f",    0x38000383, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mazh.f!",   0x1009,     0x700f,     0x38000383, Rd_Rs,                s7_do16_rdrs},
  {"mazl",      0x38000102, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mazl.f",    0x38000182, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mazl.f!",   0x1008,     0x700f,     0x38000182, Rd_Rs,                s7_do16_rdrs},
  {"mfceh",     0x00000848, 0x3e007fff, 0x8000,     Rd_x_x,               s7_do_rd},
  {"mfceh!",    0x1101,     0x7f0f,     0x00000848, x_Rs,                 s7_do16_rs},
  {"mfcehl",    0x00000c48, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mfsr",      0x00000050, 0x3e0003ff, 0x8000,     Rd_x_I5,              s7_do_rdsrs},
  {"mfcr",      0x0c000001, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mfc1",      0x0c000009, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mfc2",      0x0c000011, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mfc3",      0x0c000019, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mfcc1",     0x0c00000f, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mfcc2",     0x0c000017, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mfcc3",     0x0c00001f, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mhfl!",     0x0002,     0x700f,     0x00003c56, Rd_LowRs,             s7_do16_hrdrs},
  {"min",       0x38000006, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"mlfh!",     0x0001,     0x700f,     0x00003c56, Rd_HighRs,            s7_do16_rdhrs},
  {"msb",       0x38000001, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"msb.f!",    0x1006,     0x700f,     0x38000081, Rd_Rs,                s7_do16_rdrs},
  {"msbh",      0x38000205, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"msbh.fs",   0x380002c5, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"msbh.fs!",  0x100f,     0x700f,     0x380002c5, Rd_Rs,                s7_do16_rdrs},
  {"msbl",      0x38000004, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"msbl.fs",   0x380000c4, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"msbl.fs!",  0x100e,     0x700f,     0x380000c4, Rd_Rs,                s7_do16_rdrs},
  {"msbu",      0x38000021, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"msbu!",     0x1007,     0x700f,     0x38000021, Rd_Rs,                s7_do16_rdrs},
  {"msb.f",     0x38000081, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mszh",      0x38000305, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mszh.f",    0x38000385, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mszh.f!",   0x100d,     0x700f,     0x38000385, Rd_Rs,                s7_do16_rdrs},
  {"mszl",      0x38000104, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mszl.f",    0x38000184, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s7_do_rsrs},
  {"mszl.f!",   0x100c,     0x700f,     0x38000184, Rd_Rs,                s7_do16_rdrs},
  {"mtcel!",    0x1000,     0x7f0f,     0x0000044a, x_Rs,                 s7_do16_rs},
  {"mtcel",     0x0000044a, 0x3e007fff, 0x8000,     Rd_x_x,               s7_do_rd},
  {"mtceh",     0x0000084a, 0x3e007fff, 0x8000,     Rd_x_x,               s7_do_rd},
  {"mtceh!",    0x1100,     0x7f0f,     0x0000084a, x_Rs,                 s7_do16_rs},
  {"mtcehl",    0x00000c4a, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mtsr",      0x00000052, 0x3e0003ff, 0x8000,     x_Rs_I5,              s7_do_rdsrs},
  {"mtcr",      0x0c000000, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mtc1",      0x0c000008, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mtc2",      0x0c000010, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mtc3",      0x0c000018, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mtcc1",     0x0c00000e, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mtcc2",     0x0c000016, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mtcc3",     0x0c00001e, 0x3e00001f, 0x8000,     Rd_Rs_x,              s7_do_rdcrs},
  {"mul.f!",    0x1002,     0x700f,     0x00000041, Rd_Rs,                s7_do16_rdrs},
  {"mulu!",     0x1003,     0x700f,     0x00000042, Rd_Rs,                s7_do16_rdrs},
  {"mvcs",      0x00000056, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvcc",      0x00000456, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvgtu",     0x00000856, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvleu",     0x00000c56, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mveq",      0x00001056, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvne",      0x00001456, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvgt",      0x00001856, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvle",      0x00001c56, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvge",      0x00002056, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvlt",      0x00002456, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvmi",      0x00002856, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvpl",      0x00002c56, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvvs",      0x00003056, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mvvc",      0x00003456, 0x3e007fff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"mv",        0x00003c56, 0x3e007fff, 0x0003,     Rd_Rs_x,              s7_do_rdrs},
  {"mv!",       0x0003,     0x700f,     0x00003c56, Rd_Rs,                s7_do16_mv_rdrs},
  {"neg",       0x0000001e, 0x3e0003ff, 0x8000,     Rd_x_Rs,              s7_do_rdxrs},
  {"neg.c",     0x0000001f, 0x3e0003ff, 0x2002,     Rd_x_Rs,              s7_do_rdxrs},
  {"neg!",      0x2002,     0x700f,     0x0000001f, Rd_Rs,                s7_do16_rdrs},
  {"nop",       0x00000000, 0x3e0003ff, 0x0000,     NO_OPD,               s7_do_empty},
  {"not",       0x00000024, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s7_do_rdrs},
  {"not.c",     0x00000025, 0x3e0003ff, 0x2006,     Rd_Rs_x,              s7_do_rdrs},
  {"nop!",      0x0000,     0x700f,     0x00000000, NO16_OPD,               s7_do_empty},
  {"not!",      0x2006,     0x700f,     0x00000025, Rd_Rs,                s7_do16_rdrs},
  {"or",        0x00000022, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"or.c",      0x00000023, 0x3e0003ff, 0x2005,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"ori",       0x020a0000, 0x3e0e0001, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"ori.c",     0x020a0001, 0x3e0e0001, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"oris",      0x0a0a0000, 0x3e0e0001, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"oris.c",    0x0a0a0001, 0x3e0e0001, 0x8000,     Rd_I16,               s7_do_rdi16},
  {"orri",      0x1a000000, 0x3e000001, 0x8000,     Rd_Rs_I14,            s7_do_rdrsi14},
  {"orri.c",    0x1a000001, 0x3e000001, 0x8000,     Rd_Rs_I14,            s7_do_rdrsi14},
  {"or!",       0x2005,     0x700f,     0x00000023, Rd_Rs,                s7_do16_rdrs},
  {"pflush",    0x0000000a, 0x3e0003ff, 0x8000,     NO_OPD,               s7_do_empty},
  {"pop!",      0x200a,     0x700f,     0x0e000000, Rd_rvalueRs,          s7_do16_push_pop},
  {"push!",     0x200e,     0x700f,     0x06000004, Rd_lvalueRs,          s7_do16_push_pop},
  {"ror",       0x00000038, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"ror.c",     0x00000039, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"rorc.c",    0x0000003b, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"rol",       0x0000003c, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"rol.c",     0x0000003d, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"rolc.c",    0x0000003f, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"rori",      0x00000078, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"rori.c",    0x00000079, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"roric.c",   0x0000007b, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"roli",      0x0000007c, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"roli.c",    0x0000007d, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"rolic.c",   0x0000007f, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"rte",       0x0c000084, 0x3e0003ff, 0x8000,     NO_OPD,               s7_do_empty},
  {"sb!",       0x200f,     0x700f,     0x2e000000, Rd_lvalueRs,          s7_do16_ldst_insn},
  {"sbp!",      0x7007,     0x7007,     0x2e000000, Rd_lvalueBP_I5,       s7_do16_ldst_imm_insn},
  {"asw",       0x0000000e, 0x3e0003ff, 0x8000,     Rd_lvalue32Rs,        s7_do_ldst_atomic},
  {"scb",       0x00000068, 0x3e0003ff, 0x8000,     Rd_lvalueRs_post4,    s7_do_ldst_unalign},
  {"scw",       0x0000006a, 0x3e0003ff, 0x8000,     Rd_lvalueRs_post4,    s7_do_ldst_unalign},
  {"sce",       0x0000006e, 0x3e0003ff, 0x8000,     x_lvalueRs_post4,     s7_do_ldst_unalign},
  {"sdbbp",     0x00000006, 0x3e0003ff, 0x6002,     x_I5_x,               s7_do_xi5x},
  {"sdbbp!",    0x6002,     0x7007,     0x00000006, Rd_I5,                s7_do16_xi5},
  {"sh!",       0x200d,     0x700f,     0x2a000000, Rd_lvalueRs,          s7_do16_ldst_insn},
  {"shp!",      0x7005,     0x7007,     0x2a000000, Rd_lvalueBP_I5,       s7_do16_ldst_imm_insn},
  {"sleep",     0x0c0000c4, 0x3e0003ff, 0x8000,     NO_OPD,               s7_do_empty},
  {"sll",       0x00000030, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"sll.c",     0x00000031, 0x3e0003ff, 0x0008,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"sll.s",     0x3800004e, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"slli",      0x00000070, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"slli.c",    0x00000071, 0x3e0003ff, 0x6001,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"sll!",      0x0008,     0x700f,     0x00000031, Rd_Rs,                s7_do16_rdrs},
  {"slli!",     0x6001,     0x7007,     0x00000071, Rd_I5,                s7_do16_rdi5},
  {"srl",       0x00000034, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"srl.c",     0x00000035, 0x3e0003ff, 0x000a,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"sra",       0x00000036, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"sra.c",     0x00000037, 0x3e0003ff, 0x000b,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"srli",      0x00000074, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"srli.c",    0x00000075, 0x3e0003ff, 0x6003,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"srai",      0x00000076, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"srai.c",    0x00000077, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s7_do_rdrsi5},
  {"srl!",      0x000a,     0x700f,     0x00000035, Rd_Rs,                s7_do16_rdrs},
  {"sra!",      0x000b,     0x700f,     0x00000037, Rd_Rs,                s7_do16_rdrs},
  {"srli!",     0x6003,     0x7007,     0x00000075, Rd_Rs,                s7_do16_rdi5},
  {"stc1",      0x0c00000b, 0x3e00001f, 0x8000,     Rd_lvalueRs_SI10,     s7_do_ldst_cop},
  {"stc2",      0x0c000013, 0x3e00001f, 0x8000,     Rd_lvalueRs_SI10,     s7_do_ldst_cop},
  {"stc3",      0x0c00001b, 0x3e00001f, 0x8000,     Rd_lvalueRs_SI10,     s7_do_ldst_cop},
  {"sub",       0x00000014, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"sub.c",     0x00000015, 0x3e0003ff, 0x2001,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"sub.s",     0x38000049, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"subc",      0x00000016, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"subc.c",    0x00000017, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"sub!",      0x2001,     0x700f,     0x00000015, Rd_Rs,                s7_do16_rdrs},
  {"subei!",    0x6080,     0x7087,     0x02000001, Rd_I4,                s7_do16_rdi4},
  {"sw!",       0x200c,     0x700f,     0x28000000, Rd_lvalueRs,          s7_do16_ldst_insn},
  {"swp!",      0x7004,     0x7007,     0x28000000, Rd_lvalueBP_I5,       s7_do16_ldst_imm_insn},
  {"syscall",   0x00000002, 0x3e0003ff, 0x8000,     I15,                  s7_do_i15},
  {"tcs",       0x00000054, 0x3e007fff, 0x0005,     NO_OPD,               s7_do_empty},
  {"tcc",       0x00000454, 0x3e007fff, 0x0105,     NO_OPD,               s7_do_empty},
  {"tcnz",      0x00003854, 0x3e007fff, 0x0e05,     NO_OPD,               s7_do_empty},
  {"tcs!",      0x0005,     0x7f0f,     0x00000054, NO16_OPD,             s7_do_empty},
  {"tcc!",      0x0105,     0x7f0f,     0x00000454, NO16_OPD,             s7_do_empty},
  {"tcnz!",     0x0e05,     0x7f0f,     0x00003854, NO16_OPD,             s7_do_empty},
  {"teq",       0x00001054, 0x3e007fff, 0x0405,     NO_OPD,               s7_do_empty},
  {"teq!",      0x0405,     0x7f0f,     0x00001054, NO16_OPD,             s7_do_empty},
  {"tgtu",      0x00000854, 0x3e007fff, 0x0205,     NO_OPD,               s7_do_empty},
  {"tgt",       0x00001854, 0x3e007fff, 0x0605,     NO_OPD,               s7_do_empty},
  {"tge",       0x00002054, 0x3e007fff, 0x0805,     NO_OPD,               s7_do_empty},
  {"tgtu!",     0x0205,     0x7f0f,     0x00000854, NO16_OPD,             s7_do_empty},
  {"tgt!",      0x0605,     0x7f0f,     0x00001854, NO16_OPD,             s7_do_empty},
  {"tge!",      0x0805,     0x7f0f,     0x00002054, NO16_OPD,             s7_do_empty},
  {"tleu",      0x00000c54, 0x3e007fff, 0x0305,     NO_OPD,               s7_do_empty},
  {"tle",       0x00001c54, 0x3e007fff, 0x0705,     NO_OPD,               s7_do_empty},
  {"tlt",       0x00002454, 0x3e007fff, 0x0905,     NO_OPD,               s7_do_empty},
  {"stlb",      0x0c000004, 0x3e0003ff, 0x8000,     NO_OPD,               s7_do_empty},
  {"mftlb",     0x0c000024, 0x3e0003ff, 0x8000,     NO_OPD,               s7_do_empty},
  {"mtptlb",    0x0c000044, 0x3e0003ff, 0x8000,     NO_OPD,               s7_do_empty},
  {"mtrtlb",    0x0c000064, 0x3e0003ff, 0x8000,     NO_OPD,               s7_do_empty},
  {"tleu!",     0x0305,     0x7f0f,     0x00000c54, NO16_OPD,             s7_do_empty},
  {"tle!",      0x0705,     0x7f0f,     0x00001c54, NO16_OPD,             s7_do_empty},
  {"tlt!",      0x0905,     0x7f0f,     0x00002454, NO16_OPD,             s7_do_empty},
  {"tmi",       0x00002854, 0x3e007fff, 0x0a05,     NO_OPD,               s7_do_empty},
  {"tmi!",      0x0a05,     0x7f0f,     0x00002854, NO16_OPD,             s7_do_empty},
  {"tne",       0x00001454, 0x3e007fff, 0x0505,     NO_OPD,               s7_do_empty},
  {"tne!",      0x0505,     0x7f0f,     0x00001454, NO16_OPD,             s7_do_empty},
  {"tpl",       0x00002c54, 0x3e007fff, 0x0b05,     NO_OPD,               s7_do_empty},
  {"tpl!",      0x0b05,     0x7f0f,     0x00002c54, NO16_OPD,             s7_do_empty},
  {"trapcs",    0x00000004, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapcc",    0x00000404, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapgtu",   0x00000804, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapleu",   0x00000c04, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapeq",    0x00001004, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapne",    0x00001404, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapgt",    0x00001804, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"traple",    0x00001c04, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapge",    0x00002004, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"traplt",    0x00002404, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapmi",    0x00002804, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trappl",    0x00002c04, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapvs",    0x00003004, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trapvc",    0x00003404, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"trap",      0x00003c04, 0x3e007fff, 0x8000,     x_I5_x,               s7_do_xi5x},
  {"tset",      0x00003c54, 0x3e007fff, 0x0f05,     NO_OPD,               s7_do_empty},
  {"tset!",     0x0f05,     0x00007f0f, 0x00003c54, NO16_OPD,             s7_do_empty},
  {"tvs",       0x00003054, 0x3e007fff, 0x0c05,     NO_OPD,               s7_do_empty},
  {"tvc",       0x00003454, 0x3e007fff, 0x0d05,     NO_OPD,               s7_do_empty},
  {"tvs!",      0x0c05,     0x7f0f,     0x00003054, NO16_OPD,             s7_do_empty},
  {"tvc!",      0x0d05,     0x7f0f,     0x00003454, NO16_OPD,             s7_do_empty},
  {"xor",       0x00000026, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"xor.c",     0x00000027, 0x3e0003ff, 0x2007,     Rd_Rs_Rs,             s7_do_rdrsrs},
  {"xor!",      0x2007,     0x700f,     0x00000027, Rd_Rs,                s7_do16_rdrs},
  /* Macro instruction.  */
  {"li",        0x020c0000, 0x3e0e0000, 0x8000,     Insn_Type_SYN,        s7_do_macro_li_rdi32},
  /* la reg, imm32        -->(1)  ldi  reg, simm16
                             (2)  ldis reg, %HI(imm32)
                                  ori  reg, %LO(imm32)

     la reg, symbol       -->(1)  lis  reg, %HI(imm32)
                                  ori  reg, %LO(imm32)  */
  {"la",        0x020c0000, 0x3e0e0000, 0x8000,     Insn_Type_SYN,        s7_do_macro_la_rdi32},
  {"div",       0x00000044, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"divu",      0x00000046, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"rem",       0x00000044, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"remu",      0x00000046, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"mul",       0x00000040, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"mulu",      0x00000042, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"maz",       0x00000040, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"mazu",      0x00000042, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"mul.f",     0x00000041, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"maz.f",     0x00000041, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s7_do_macro_mul_rdrsrs},
  {"lb",        INSN_LB,    0x00000000, 0x8000,     Insn_Type_SYN,        s7_do_macro_ldst_label},
  {"lbu",       INSN_LBU,   0x00000000, 0x200b,     Insn_Type_SYN,        s7_do_macro_ldst_label},
  {"lh",        INSN_LH,    0x00000000, 0x2009,     Insn_Type_SYN,        s7_do_macro_ldst_label},
  {"lhu",       INSN_LHU,   0x00000000, 0x8000,     Insn_Type_SYN,        s7_do_macro_ldst_label},
  {"lw",        INSN_LW,    0x00000000, 0x2008,     Insn_Type_SYN,        s7_do_macro_ldst_label},
  {"sb",        INSN_SB,    0x00000000, 0x200f,     Insn_Type_SYN,        s7_do_macro_ldst_label},
  {"sh",        INSN_SH,    0x00000000, 0x200d,     Insn_Type_SYN,        s7_do_macro_ldst_label},
  {"sw",        INSN_SW,    0x00000000, 0x200c,     Insn_Type_SYN,        s7_do_macro_ldst_label},
  /* Assembler use internal.  */
  {"ld_i32hi",  0x0a0c0000, 0x3e0e0000, 0x8000,     Insn_internal, s7_do_macro_rdi32hi},
  {"ld_i32lo",  0x020a0000, 0x3e0e0001, 0x8000,     Insn_internal, s7_do_macro_rdi32lo},
  {"ldis_pic",  0x0a0c0000, 0x3e0e0000, 0x5000,     Insn_internal, s7_do_rdi16_pic},
  {"addi_s_pic",0x02000000, 0x3e0e0001, 0x8000,     Insn_internal, s7_do_addi_s_pic},
  {"addi_u_pic",0x02000000, 0x3e0e0001, 0x8000,     Insn_internal, s7_do_addi_u_pic},
  {"lw_pic",    0x20000000, 0x3e000000, 0x8000,     Insn_internal, s7_do_lw_pic},
};

#define s7_SCORE5_PIPELINE 5
#define s7_SCORE7_PIPELINE 7

static int s7_university_version = 0;
static int s7_vector_size = s7_SCORE7_PIPELINE;
static struct s7_score_it s7_dependency_vector[s7_SCORE7_PIPELINE];

static int s7_score7d = 1;



static int
s7_end_of_line (char *str)
{
  int retval = s7_SUCCESS;

  s7_skip_whitespace (str);
  if (*str != '\0')
    {
      retval = (int) s7_FAIL;

      if (!s7_inst.error)
        s7_inst.error = s7_BAD_GARBAGE;
    }

  return retval;
}

static int
s7_score_reg_parse (char **ccp, htab_t htab)
{
  char *start = *ccp;
  char c;
  char *p;
  struct s7_reg_entry *reg;

  p = start;
  if (!ISALPHA (*p) || !is_name_beginner (*p))
    return (int) s7_FAIL;

  c = *p++;

  while (ISALPHA (c) || ISDIGIT (c) || c == '_')
    c = *p++;

  *--p = 0;
  reg = (struct s7_reg_entry *) str_hash_find (htab, start);
  *p = c;

  if (reg)
    {
      *ccp = p;
      return reg->number;
    }
  return (int) s7_FAIL;
}

/* If shift <= 0, only return reg.  */
static int
s7_reg_required_here (char **str, int shift, enum s7_score_reg_type reg_type)
{
  static char buff[s7_MAX_LITERAL_POOL_SIZE];
  int reg = (int) s7_FAIL;
  char *start = *str;

  if ((reg = s7_score_reg_parse (str, s7_all_reg_maps[reg_type].htab)) != (int) s7_FAIL)
    {
      if (reg_type == s7_REG_TYPE_SCORE)
        {
          if ((reg == 1) && (s7_nor1 == 1) && (s7_inst.bwarn == 0))
            {
              as_warn (_("Using temp register(r1)"));
              s7_inst.bwarn = 1;
            }
        }
      if (shift >= 0)
	{
          if (reg_type == s7_REG_TYPE_SCORE_CR)
	    strcpy (s7_inst.reg, s7_score_crn_table[reg].name);
          else if (reg_type == s7_REG_TYPE_SCORE_SR)
	    strcpy (s7_inst.reg, s7_score_srn_table[reg].name);
          else
	    strcpy (s7_inst.reg, "");

          s7_inst.instruction |= reg << shift;
	}
    }
  else
    {
      *str = start;
      sprintf (buff, _("register expected, not '%.100s'"), start);
      s7_inst.error = buff;
    }

  return reg;
}

static int
s7_skip_past_comma (char **str)
{
  char *p = *str;
  char c;
  int comma = 0;

  while ((c = *p) == ' ' || c == ',')
    {
      p++;
      if (c == ',' && comma++)
        {
          s7_inst.error = s7_BAD_SKIP_COMMA;
          return (int) s7_FAIL;
        }
    }

  if ((c == '\0') || (comma == 0))
    {
      s7_inst.error = s7_BAD_SKIP_COMMA;
      return (int) s7_FAIL;
    }

  *str = p;
  return comma ? s7_SUCCESS : (int) s7_FAIL;
}

static void
s7_do_rdrsrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 10, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      if ((((s7_inst.instruction >> 15) & 0x10) == 0)
          && (((s7_inst.instruction >> 10) & 0x10) == 0)
          && (((s7_inst.instruction >> 20) & 0x10) == 0)
          && (s7_inst.relax_inst != 0x8000)
          && (((s7_inst.instruction >> 20) & 0xf) == ((s7_inst.instruction >> 15) & 0xf)))
        {
          s7_inst.relax_inst |= (((s7_inst.instruction >> 10) & 0xf) << 4)
            | (((s7_inst.instruction >> 15) & 0xf) << 8);
          s7_inst.relax_size = 2;
        }
      else
        {
          s7_inst.relax_inst = 0x8000;
        }
    }
}

static int
s7_walk_no_bignums (symbolS * sp)
{
  if (symbol_get_value_expression (sp)->X_op == O_big)
    return 1;

  if (symbol_get_value_expression (sp)->X_add_symbol)
    return (s7_walk_no_bignums (symbol_get_value_expression (sp)->X_add_symbol)
	    || (symbol_get_value_expression (sp)->X_op_symbol
		&& s7_walk_no_bignums (symbol_get_value_expression (sp)->X_op_symbol)));

  return 0;
}

static int
s7_my_get_expression (expressionS * ep, char **str)
{
  char *save_in;

  save_in = input_line_pointer;
  input_line_pointer = *str;
  s7_in_my_get_expression = 1;

  (void) expression (ep);
  s7_in_my_get_expression = 0;

  if (ep->X_op == O_illegal)
    {
      *str = input_line_pointer;
      input_line_pointer = save_in;
      s7_inst.error = _("illegal expression");
      return (int) s7_FAIL;
    }
  /* Get rid of any bignums now, so that we don't generate an error for which
     we can't establish a line number later on.  Big numbers are never valid
     in instructions, which is where this routine is always called.  */
  if (ep->X_op == O_big
      || (ep->X_add_symbol
          && (s7_walk_no_bignums (ep->X_add_symbol)
              || (ep->X_op_symbol && s7_walk_no_bignums (ep->X_op_symbol)))))
    {
      s7_inst.error = _("invalid constant");
      *str = input_line_pointer;
      input_line_pointer = save_in;
      return (int) s7_FAIL;
    }

  if ((ep->X_add_symbol != NULL)
      && (s7_inst.type != PC_DISP19div2)
      && (s7_inst.type != PC_DISP8div2)
      && (s7_inst.type != PC_DISP24div2)
      && (s7_inst.type != PC_DISP11div2)
      && (s7_inst.type != Insn_Type_SYN)
      && (s7_inst.type != Rd_rvalueRs_SI15)
      && (s7_inst.type != Rd_lvalueRs_SI15)
      && (s7_inst.type != Insn_internal))
    {
      s7_inst.error = s7_BAD_ARGS;
      *str = input_line_pointer;
      input_line_pointer = save_in;
      return (int) s7_FAIL;
    }

  *str = input_line_pointer;
  input_line_pointer = save_in;
  return s7_SUCCESS;
}

/* Check if an immediate is valid.  If so, convert it to the right format.  */

static bfd_signed_vma
s7_validate_immediate (bfd_signed_vma val, unsigned int data_type, int hex_p)
{
  switch (data_type)
    {
    case _VALUE_HI16:
      {
        int val_hi = ((val & 0xffff0000) >> 16);

        if (s7_score_df_range[data_type].range[0] <= val_hi
            && val_hi <= s7_score_df_range[data_type].range[1])
	  return val_hi;
      }
      break;

    case _VALUE_LO16:
      {
        int val_lo = (val & 0xffff);

        if (s7_score_df_range[data_type].range[0] <= val_lo
            && val_lo <= s7_score_df_range[data_type].range[1])
	  return val_lo;
      }
      break;

    case _SIMM12:
      if (hex_p == 1)
        {
          if (!(val >= -0x800 && val <= 0xfff))
            {
              return (int) s7_FAIL;
            }
        }
      else
        {
          if (!(val >= -2048 && val <= 2047))
            {
              return (int) s7_FAIL;
            }
        }

      return val;
      break;

    case _SIMM14:
      if (hex_p == 1)
        {
          if (!(val >= -0x2000 && val <= 0x3fff))
            {
              return (int) s7_FAIL;
            }
        }
      else
        {
          if (!(val >= -8192 && val <= 8191))
            {
              return (int) s7_FAIL;
            }
        }

      return val;
      break;

    case _SIMM15:
      if (hex_p == 1)
        {
          if (!(val >= -0x4000 && val <= 0x7fff))
            {
              return (int) s7_FAIL;
            }
        }
      else
        {
          if (!(val >= -16384 && val <= 16383))
            {
              return (int) s7_FAIL;
            }
        }

      return val;
      break;

    case _SIMM16:
      if (hex_p == 1)
        {
          if (!(val >= -0x8000 && val <= 0xffff))
            {
              return (int) s7_FAIL;
            }
        }
      else
        {
          if (!(val >= -32768 && val <= 32767))
            {
              return (int) s7_FAIL;
            }
        }

      return val;
      break;

    case _SIMM16_NEG:
      if (hex_p == 1)
        {
          if (!(val >= -0x7fff && val <= 0xffff && val != 0x8000))
            {
              return (int) s7_FAIL;
            }
        }
      else
        {
          if (!(val >= -32767 && val <= 32768))
            {
              return (int) s7_FAIL;
            }
        }

      val = -val;
      return val;
      break;

    case _IMM32:
      if (val >= 0 && val <= 0xffffffff)
        {
          return val;
        }
      else
        {
          return (int) s7_FAIL;
        }

    default:
      if (data_type == _SIMM14_NEG || data_type == _IMM16_NEG)
	val = -val;

      if (s7_score_df_range[data_type].range[0] <= val
          && val <= s7_score_df_range[data_type].range[1])
	return val;

      break;
    }

  return (int) s7_FAIL;
}

static int
s7_data_op2 (char **str, int shift, enum score_data_type data_type)
{
  int value;
  char data_exp[s7_MAX_LITERAL_POOL_SIZE];
  char *dataptr;
  int cnt = 0;
  char *pp = NULL;

  s7_skip_whitespace (*str);
  s7_inst.error = NULL;
  dataptr = * str;

  /* Set hex_p to zero.  */
  int hex_p = 0;

  while ((*dataptr != '\0') && (*dataptr != '|') && (cnt <= s7_MAX_LITERAL_POOL_SIZE))     /* 0x7c = ='|' */
    {
      data_exp[cnt] = *dataptr;
      dataptr++;
      cnt++;
    }

  data_exp[cnt] = '\0';
  pp = (char *)&data_exp;

  if (*dataptr == '|')          /* process PCE */
    {
      if (s7_my_get_expression (&s7_inst.reloc.exp, &pp) == (int) s7_FAIL)
        return (int) s7_FAIL;
      s7_end_of_line (pp);
      if (s7_inst.error != 0)
        return (int) s7_FAIL;       /* to ouptut_inst to printf out the error */
      *str = dataptr;
    }
  else                          /* process  16 bit */
    {
      if (s7_my_get_expression (&s7_inst.reloc.exp, str) == (int) s7_FAIL)
        {
          return (int) s7_FAIL;
        }

      dataptr = (char *) data_exp;
      for (; *dataptr != '\0'; dataptr++)
        {
          *dataptr = TOLOWER (*dataptr);
          if (*dataptr == '!' || *dataptr == ' ')
            break;
        }
      dataptr = (char *) data_exp;

      if ((dataptr != NULL)
          && (((strstr (dataptr, "0x")) != NULL)
              || ((strstr (dataptr, "0X")) != NULL)))
        {
          hex_p = 1;
          if ((data_type != _SIMM16_LA)
              && (data_type != _VALUE_HI16)
              && (data_type != _VALUE_LO16)
              && (data_type != _IMM16)
              && (data_type != _IMM15)
              && (data_type != _IMM14)
              && (data_type != _IMM4)
              && (data_type != _IMM5)
              && (data_type != _IMM8)
              && (data_type != _IMM5_RSHIFT_1)
              && (data_type != _IMM5_RSHIFT_2)
              && (data_type != _SIMM14)
              && (data_type != _SIMM16)
              && (data_type != _SIMM14_NEG)
              && (data_type != _SIMM16_NEG)
              && (data_type != _IMM10_RSHIFT_2)
              && (data_type != _GP_IMM15))
            {
              data_type += 24;
            }
        }

      if ((s7_inst.reloc.exp.X_add_number == 0)
          /* for "addi r0,-((((((32*4)+4)+4)+4)+4)&0xf)".  */
          && (s7_inst.type != Rd_SI16)
          && (s7_inst.type != Insn_Type_SYN)
          && (s7_inst.type != Rd_rvalueRs_SI15)
          && (s7_inst.type != Rd_lvalueRs_SI15)
          && (s7_inst.type != Insn_internal)
          && (((*dataptr >= 'a') && (*dataptr <= 'z'))
             || ((*dataptr == '0') && (*(dataptr + 1) == 'x') && (*(dataptr + 2) != '0'))
             || ((*dataptr == '+') && (*(dataptr + 1) != '0'))
             || ((*dataptr == '-') && (*(dataptr + 1) != '0'))))
        {
          s7_inst.error = s7_BAD_ARGS;
          return (int) s7_FAIL;
        }
    }

  if ((s7_inst.reloc.exp.X_add_symbol)
      && ((data_type == _SIMM16)
          || (data_type == _SIMM16_NEG)
          || (data_type == _IMM16_NEG)
          || (data_type == _SIMM14)
          || (data_type == _SIMM14_NEG)
          || (data_type == _IMM5)
          || (data_type == _IMM14)
          || (data_type == _IMM20)
          || (data_type == _IMM16)
          || (data_type == _IMM15)
          || (data_type == _IMM4)))
    {
      s7_inst.error = s7_BAD_ARGS;
      return (int) s7_FAIL;
    }

  if (s7_inst.reloc.exp.X_add_symbol)
    {
      switch (data_type)
        {
        case _SIMM16_LA:
          return (int) s7_FAIL;
        case _VALUE_HI16:
          s7_inst.reloc.type = BFD_RELOC_HI16_S;
          s7_inst.reloc.pc_rel = 0;
          break;
        case _VALUE_LO16:
          s7_inst.reloc.type = BFD_RELOC_LO16;
          s7_inst.reloc.pc_rel = 0;
          break;
        case _GP_IMM15:
          s7_inst.reloc.type = BFD_RELOC_SCORE_GPREL15;
          s7_inst.reloc.pc_rel = 0;
          break;
        case _SIMM16_pic:
        case _IMM16_LO16_pic:
          s7_inst.reloc.type = BFD_RELOC_SCORE_GOT_LO16;
          s7_inst.reloc.pc_rel = 0;
          break;
        default:
          s7_inst.reloc.type = BFD_RELOC_32;
          s7_inst.reloc.pc_rel = 0;
          break;
        }
    }
  else
    {
      if (data_type == _IMM16_pic)
	{
          s7_inst.reloc.type = BFD_RELOC_SCORE_DUMMY_HI16;
          s7_inst.reloc.pc_rel = 0;
	}

      if (data_type == _SIMM16_LA && s7_inst.reloc.exp.X_unsigned == 1)
        {
          value = s7_validate_immediate (s7_inst.reloc.exp.X_add_number, _SIMM16_LA_POS, hex_p);
          if (value == (int) s7_FAIL)       /* for advance to check if this is ldis */
            if ((s7_inst.reloc.exp.X_add_number & 0xffff) == 0)
              {
                s7_inst.instruction |= 0x8000000;
                s7_inst.instruction |= ((s7_inst.reloc.exp.X_add_number >> 16) << 1) & 0x1fffe;
                return s7_SUCCESS;
              }
        }
      else
        {
          value = s7_validate_immediate (s7_inst.reloc.exp.X_add_number, data_type, hex_p);
        }

      if (value == (int) s7_FAIL)
        {
          if ((data_type != _SIMM14_NEG) && (data_type != _SIMM16_NEG) && (data_type != _IMM16_NEG))
            {
              sprintf (s7_err_msg,
                       _("invalid constant: %d bit expression not in range %d..%d"),
                       s7_score_df_range[data_type].bits,
                       s7_score_df_range[data_type].range[0], s7_score_df_range[data_type].range[1]);
            }
          else
            {
              sprintf (s7_err_msg,
                       _("invalid constant: %d bit expression not in range %d..%d"),
                       s7_score_df_range[data_type].bits,
                       -s7_score_df_range[data_type].range[1], -s7_score_df_range[data_type].range[0]);
            }

          s7_inst.error = s7_err_msg;
          return (int) s7_FAIL;
        }

      if ((s7_score_df_range[data_type].range[0] != 0) || (data_type == _IMM5_RANGE_8_31))
        {
          value &= (1 << s7_score_df_range[data_type].bits) - 1;
        }

      s7_inst.instruction |= value << shift;
    }

  if ((s7_inst.instruction & 0x3e000000) == 0x30000000)
    {
      if ((((s7_inst.instruction >> 20) & 0x1F) != 0)
          && (((s7_inst.instruction >> 20) & 0x1F) != 1)
          && (((s7_inst.instruction >> 20) & 0x1F) != 2)
          && (((s7_inst.instruction >> 20) & 0x1F) != 3)
          && (((s7_inst.instruction >> 20) & 0x1F) != 4)
          && (((s7_inst.instruction >> 20) & 0x1F) != 8)
          && (((s7_inst.instruction >> 20) & 0x1F) != 9)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0xa)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0xb)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0xc)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0xd)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0xe)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0x10)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0x11)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0x18)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0x1A)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0x1B)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0x1d)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0x1e)
          && (((s7_inst.instruction >> 20) & 0x1F) != 0x1f))
        {
          s7_inst.error = _("invalid constant: bit expression not defined");
          return (int) s7_FAIL;
        }
    }

  return s7_SUCCESS;
}

/* Handle addi/addi.c/addis.c/cmpi.c/addis.c/ldi.  */

static void
s7_do_rdsi16 (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_data_op2 (&str, 1, _SIMM16) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  /* ldi.  */
  if ((s7_inst.instruction & 0x20c0000) == 0x20c0000)
    {
      if ((((s7_inst.instruction >> 20) & 0x10) == 0x10) || ((s7_inst.instruction & 0x1fe00) != 0))
        {
          s7_inst.relax_inst = 0x8000;
        }
      else
        {
          s7_inst.relax_inst |= (s7_inst.instruction >> 1) & 0xff;
          s7_inst.relax_inst |= (((s7_inst.instruction >> 20) & 0xf) << 8);
          s7_inst.relax_size = 2;
        }
    }
  else if (((s7_inst.instruction >> 20) & 0x10) == 0x10)
    {
      s7_inst.relax_inst = 0x8000;
    }
}

/* Handle subi/subi.c.  */

static void
s7_do_sub_rdsi16 (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_data_op2 (&str, 1, _SIMM16_NEG) != (int) s7_FAIL)
    s7_end_of_line (str);
}


/* Handle addri/addri.c.  */

static void
s7_do_rdrssi14 (char *str)         /* -(2^13)~((2^13)-1) */
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL)
    s7_data_op2 (&str, 1, _SIMM14);
}

/* Handle subri.c/subri.  */

static void
s7_do_sub_rdrssi14 (char *str)     /* -(2^13)~((2^13)-1) */
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_data_op2 (&str, 1, _SIMM14_NEG) != (int) s7_FAIL)
    s7_end_of_line (str);
}

/* Handle bitclr.c/bitset.c/bittgl.c/slli.c/srai.c/srli.c/roli.c/rori.c/rolic.c.  */

static void
s7_do_rdrsi5 (char *str)           /* 0~((2^14)-1) */
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_data_op2 (&str, 10, _IMM5) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if ((((s7_inst.instruction >> 20) & 0x1f) == ((s7_inst.instruction >> 15) & 0x1f))
      && (s7_inst.relax_inst != 0x8000) && (((s7_inst.instruction >> 15) & 0x10) == 0))
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 10) & 0x1f) << 3) | (((s7_inst.instruction >> 15) & 0xf) << 8);
      s7_inst.relax_size = 2;
    }
  else
    s7_inst.relax_inst = 0x8000;
}

/* Handle andri/orri/andri.c/orri.c.  */

static void
s7_do_rdrsi14 (char *str)          /* 0 ~ ((2^14)-1)  */
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_data_op2 (&str, 1, _IMM14) != (int) s7_FAIL)
    s7_end_of_line (str);
}

/* Handle bittst.c.  */

static void
s7_do_xrsi5 (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_data_op2 (&str, 10, _IMM5) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if ((s7_inst.relax_inst != 0x8000) && (((s7_inst.instruction >> 15) & 0x10) == 0))
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 10) & 0x1f) << 3) | (((s7_inst.instruction >> 15) & 0xf) << 8);
      s7_inst.relax_size = 2;
    }
  else
    s7_inst.relax_inst = 0x8000;
}

/* Handle addis/andi/ori/andis/oris/ldis.  */

static void
s7_do_rdi16 (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_data_op2 (&str, 1, _IMM16) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;
}

static void
s7_do_macro_rdi32hi (char *str)
{
  s7_skip_whitespace (str);

  /* Do not handle s7_end_of_line().  */
  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL)
    s7_data_op2 (&str, 1, _VALUE_HI16);
}

static void
s7_do_macro_rdi32lo (char *str)
{
  s7_skip_whitespace (str);

  /* Do not handle s7_end_of_line().  */
  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL)
    s7_data_op2 (&str, 1, _VALUE_LO16);
}

/* Handle ldis_pic.  */

static void
s7_do_rdi16_pic (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_data_op2 (&str, 1, _IMM16_pic) != (int) s7_FAIL)
    s7_end_of_line (str);
}

/* Handle addi_s_pic to generate R_SCORE_GOT_LO16 .  */

static void
s7_do_addi_s_pic (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_data_op2 (&str, 1, _SIMM16_pic) != (int) s7_FAIL)
    s7_end_of_line (str);
}

/* Handle addi_u_pic to generate R_SCORE_GOT_LO16 .  */

static void
s7_do_addi_u_pic (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_data_op2 (&str, 1, _IMM16_LO16_pic) != (int) s7_FAIL)
    s7_end_of_line (str);
}

/* Handle mfceh/mfcel/mtceh/mtchl.  */

static void
s7_do_rd (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL)
    s7_end_of_line (str);
}

static void
s7_do_rs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if ((s7_inst.relax_inst != 0x8000) && (((s7_inst.instruction >> 15) & 0x10) == 0))
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 10) & 0xf) << 8) | (((s7_inst.instruction >> 15) & 0xf) << 4);
      s7_inst.relax_size = 2;
    }
  else
    s7_inst.relax_inst = 0x8000;
}

static void
s7_do_i15 (char *str)
{
  s7_skip_whitespace (str);

  if (s7_data_op2 (&str, 10, _IMM15) != (int) s7_FAIL)
    s7_end_of_line (str);
}

static void
s7_do_xi5x (char *str)
{
  s7_skip_whitespace (str);

  if (s7_data_op2 (&str, 15, _IMM5) == (int) s7_FAIL || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if (s7_inst.relax_inst != 0x8000)
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0x1f) << 3);
      s7_inst.relax_size = 2;
    }
}

static void
s7_do_rdrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if (s7_inst.relax_inst != 0x8000)
    {
      if (((s7_inst.instruction & 0x7f) == 0x56))  /* adjust mv -> mv! / mlfh! / mhfl! */
        {
          /* mlfh */
          if ((((s7_inst.instruction >> 15) & 0x10) != 0x0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
            {
              s7_inst.relax_inst = 0x00000001 | (((s7_inst.instruction >> 15) & 0xf) << 4)
                | (((s7_inst.instruction >> 20) & 0xf) << 8);
              s7_inst.relax_size = 2;
            }
          /* mhfl */
          else if ((((s7_inst.instruction >> 15) & 0x10) == 0x0) && ((s7_inst.instruction >> 20) & 0x10) != 0)
            {
              s7_inst.relax_inst = 0x00000002 | (((s7_inst.instruction >> 15) & 0xf) << 4)
                | (((s7_inst.instruction >> 20) & 0xf) << 8);
              s7_inst.relax_size = 2;
            }
          else if ((((s7_inst.instruction >> 15) & 0x10) == 0x0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
            {
              s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0xf) << 4)
                | (((s7_inst.instruction >> 20) & 0xf) << 8);
              s7_inst.relax_size = 2;
            }
          else
            {
              s7_inst.relax_inst = 0x8000;
            }
        }
      else if ((((s7_inst.instruction >> 15) & 0x10) == 0x0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
        {
          s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0xf) << 4)
            | (((s7_inst.instruction >> 20) & 0xf) << 8);
          s7_inst.relax_size = 2;
        }
      else
        {
          s7_inst.relax_inst = 0x8000;
        }
    }
}

/* Handle mfcr/mtcr.  */
static void
s7_do_rdcrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE_CR) != (int) s7_FAIL)
    s7_end_of_line (str);
}

/* Handle mfsr/mtsr.  */

static void
s7_do_rdsrs (char *str)
{
  s7_skip_whitespace (str);

  /* mfsr */
  if ((s7_inst.instruction & 0xff) == 0x50)
    {
      if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) != (int) s7_FAIL
          && s7_skip_past_comma (&str) != (int) s7_FAIL
          && s7_reg_required_here (&str, 10, s7_REG_TYPE_SCORE_SR) != (int) s7_FAIL)
	s7_end_of_line (str);
    }
  else
    {
      if (s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) != (int) s7_FAIL
          && s7_skip_past_comma (&str) != (int) s7_FAIL)
	s7_reg_required_here (&str, 10, s7_REG_TYPE_SCORE_SR);
    }
}

/* Handle neg.  */

static void
s7_do_rdxrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 10, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if ((s7_inst.relax_inst != 0x8000) && (((s7_inst.instruction >> 10) & 0x10) == 0)
      && (((s7_inst.instruction >> 20) & 0x10) == 0))
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 10) & 0xf) << 4) | (((s7_inst.instruction >> 20) & 0xf) << 8);
      s7_inst.relax_size = 2;
    }
  else
    s7_inst.relax_inst = 0x8000;
}

/* Handle cmp.c/cmp<cond>.  */
static void
s7_do_rsrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 10, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if ((s7_inst.relax_inst != 0x8000) && (((s7_inst.instruction >> 20) & 0x1f) == 3)
      && (((s7_inst.instruction >> 10) & 0x10) == 0) && (((s7_inst.instruction >> 15) & 0x10) == 0))
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 10) & 0xf) << 4) | (((s7_inst.instruction >> 15) & 0xf) << 8);
      s7_inst.relax_size = 2;
    }
  else
    s7_inst.relax_inst = 0x8000;
}

static void
s7_do_ceinst (char *str)
{
  char *strbak;

  strbak = str;
  s7_skip_whitespace (str);

  if (s7_data_op2 (&str, 20, _IMM5) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 10, s7_REG_TYPE_SCORE) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_data_op2 (&str, 5, _IMM5) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_data_op2 (&str, 0, _IMM5) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      str = strbak;
      if (s7_data_op2 (&str, 0, _IMM25) == (int) s7_FAIL)
	return;
    }
}

static int
s7_reglow_required_here (char **str, int shift)
{
  static char buff[s7_MAX_LITERAL_POOL_SIZE];
  int reg;
  char *start = *str;

  if ((reg = s7_score_reg_parse (str, s7_all_reg_maps[s7_REG_TYPE_SCORE].htab)) != (int) s7_FAIL)
    {
      if ((reg == 1) && (s7_nor1 == 1) && (s7_inst.bwarn == 0))
        {
          as_warn (_("Using temp register(r1)"));
          s7_inst.bwarn = 1;
        }
      if (reg < 16)
        {
          if (shift >= 0)
            s7_inst.instruction |= reg << shift;

          return reg;
        }
    }

  /* Restore the start point, we may have got a reg of the wrong class.  */
  *str = start;
  sprintf (buff, _("low register(r0-r15)expected, not '%.100s'"), start);
  s7_inst.error = buff;
  return (int) s7_FAIL;
}

/* Handle addc!/add!/and!/cmp!/neg!/not!/or!/sll!/srl!/sra!/xor!/sub!.  */

static void
s7_do16_rdrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reglow_required_here (&str, 8) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reglow_required_here (&str, 4) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      if ((s7_inst.instruction & 0x700f) == 0x2003)        /* cmp!  */
        {
          s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 15)
            | (((s7_inst.instruction >> 4) & 0xf) << 10);
        }
      else if ((s7_inst.instruction & 0x700f) == 0x2006)   /* not!  */
	{
	  s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
	    | (((s7_inst.instruction >> 4) & 0xf) << 15);
	}
      else if ((s7_inst.instruction & 0x700f) == 0x1009)   /* mazh.f!  */
	{
          s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 15)
                             | (((s7_inst.instruction >> 4) & 0xf) << 10);
	}
      else
        {
          s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
            | (((s7_inst.instruction >> 8) & 0xf) << 15) | (((s7_inst.instruction >> 4) & 0xf) << 10);
        }
      s7_inst.relax_size = 4;
    }
}

static void
s7_do16_rs (char *str)
{
  int rd = 0;

  s7_skip_whitespace (str);

  if ((rd = s7_reglow_required_here (&str, 4)) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      s7_inst.relax_inst |= rd << 20;
      s7_inst.relax_size = 4;
    }
}

/* Handle br!/brl!.  */

static void
s7_do16_xrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reglow_required_here (&str, 4) == (int) s7_FAIL || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 10)
                      | (((s7_inst.instruction >> 4) & 0xf) << 15);
      s7_inst.relax_size = 4;
    }
}

static int
s7_reghigh_required_here (char **str, int shift)
{
  static char buff[s7_MAX_LITERAL_POOL_SIZE];
  int reg;
  char *start = *str;

  if ((reg = s7_score_reg_parse (str, s7_all_reg_maps[s7_REG_TYPE_SCORE].htab)) != (int) s7_FAIL)
    {
      if (15 < reg && reg < 32)
        {
          if (shift >= 0)
            s7_inst.instruction |= (reg & 0xf) << shift;

          return reg;
        }
    }

  *str = start;
  sprintf (buff, _("high register(r16-r31)expected, not '%.100s'"), start);
  s7_inst.error = buff;
  return (int) s7_FAIL;
}

/* Handle mhfl!.  */

static void
s7_do16_hrdrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reghigh_required_here (&str, 8) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_reglow_required_here (&str, 4) != (int) s7_FAIL
      && s7_end_of_line (str) != (int) s7_FAIL)
    {
      s7_inst.relax_inst |= ((((s7_inst.instruction >> 8) & 0xf) | 0x10) << 20)
        | (((s7_inst.instruction >> 4) & 0xf) << 15) | (0xf << 10);
      s7_inst.relax_size = 4;
    }
}

/* Handle mlfh!.  */

static void
s7_do16_rdhrs (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reglow_required_here (&str, 8) != (int) s7_FAIL
      && s7_skip_past_comma (&str) != (int) s7_FAIL
      && s7_reghigh_required_here (&str, 4) != (int) s7_FAIL
      && s7_end_of_line (str) != (int) s7_FAIL)
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
        | ((((s7_inst.instruction >> 4) & 0xf) | 0x10) << 15) | (0xf << 10);
      s7_inst.relax_size = 4;
    }
}

/* We need to be able to fix up arbitrary expressions in some statements.
   This is so that we can handle symbols that are an arbitrary distance from
   the pc.  The most common cases are of the form ((+/-sym -/+ . - 8) & mask),
   which returns part of an address in a form which will be valid for
   a data instruction.  We do this by pushing the expression into a symbol
   in the expr_section, and creating a fix for that.  */

static fixS *
s7_fix_new_score (fragS * frag, int where, short int size, expressionS * exp, int pc_rel, int reloc)
{
  fixS *new_fix;

  switch (exp->X_op)
    {
    case O_constant:
    case O_symbol:
    case O_add:
    case O_subtract:
      new_fix = fix_new_exp (frag, where, size, exp, pc_rel, reloc);
      break;
    default:
      new_fix = fix_new (frag, where, size, make_expr_symbol (exp), 0, pc_rel, reloc);
      break;
    }
  return new_fix;
}

static void
s7_init_dependency_vector (void)
{
  int i;

  for (i = 0; i < s7_vector_size; i++)
    memset (&s7_dependency_vector[i], '\0', sizeof (s7_dependency_vector[i]));

  return;
}

static enum s7_insn_type_for_dependency
s7_dependency_type_from_insn (char *insn_name)
{
  char name[s7_INSN_NAME_LEN];
  const struct s7_insn_to_dependency *tmp;

  strcpy (name, insn_name);
  tmp = (const struct s7_insn_to_dependency *)
    str_hash_find (s7_dependency_insn_hsh, name);

  if (tmp)
    return tmp->type;

  return s7_D_all_insn;
}

static int
s7_check_dependency (char *pre_insn, char *pre_reg,
		     char *cur_insn, char *cur_reg, int *warn_or_error)
{
  int bubbles = 0;
  unsigned int i;
  enum s7_insn_type_for_dependency pre_insn_type;
  enum s7_insn_type_for_dependency cur_insn_type;

  pre_insn_type = s7_dependency_type_from_insn (pre_insn);
  cur_insn_type = s7_dependency_type_from_insn (cur_insn);

  for (i = 0; i < sizeof (s7_data_dependency_table) / sizeof (s7_data_dependency_table[0]); i++)
    {
      if ((pre_insn_type == s7_data_dependency_table[i].pre_insn_type)
          && (s7_D_all_insn == s7_data_dependency_table[i].cur_insn_type
              || cur_insn_type == s7_data_dependency_table[i].cur_insn_type)
          && (strcmp (s7_data_dependency_table[i].pre_reg, "") == 0
              || strcmp (s7_data_dependency_table[i].pre_reg, pre_reg) == 0)
          && (strcmp (s7_data_dependency_table[i].cur_reg, "") == 0
              || strcmp (s7_data_dependency_table[i].cur_reg, cur_reg) == 0))
        {
          if (s7_vector_size == s7_SCORE5_PIPELINE)
            bubbles = s7_data_dependency_table[i].bubblenum_5;
          else
            bubbles = s7_data_dependency_table[i].bubblenum_7;
          *warn_or_error = s7_data_dependency_table[i].warn_or_error;
          break;
        }
    }

  return bubbles;
}

/* Turn an integer of n bytes (in val) into a stream of bytes appropriate
   for use in the a.out file, and stores them in the array pointed to by buf.
   This knows about the endian-ness of the target machine and does
   THE RIGHT THING, whatever it is.  Possible values for n are 1 (byte)
   2 (short) and 4 (long)  Floating numbers are put out as a series of
   LITTLENUMS (shorts, here at least).  */

static void
s7_number_to_chars (char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

static void
s7_build_one_frag (struct s7_score_it one_inst)
{
  char *p;
  int relaxable_p = s7_g_opt;
  int relax_size = 0;

  /* Start a new frag if frag_now is not empty.  */
  if (frag_now_fix () != 0)
    {
      if (!frag_now->tc_frag_data.is_insn)
	frag_wane (frag_now);

      frag_new (0);
    }
  frag_grow (20);

  p = frag_more (one_inst.size);
  s7_number_to_chars (p, one_inst.instruction, one_inst.size);

#ifdef OBJ_ELF
  dwarf2_emit_insn (one_inst.size);
#endif

  relaxable_p &= (one_inst.relax_size != 0);
  relax_size = relaxable_p ? one_inst.relax_size : 0;

  p = frag_var (rs_machine_dependent, relax_size + s7_RELAX_PAD_BYTE, 0,
                s7_RELAX_ENCODE (one_inst.size, one_inst.relax_size,
                              one_inst.type, 0, 0, relaxable_p),
                NULL, 0, NULL);

  if (relaxable_p)
    s7_number_to_chars (p, one_inst.relax_inst, relax_size);
}

static void
s7_handle_dependency (struct s7_score_it *theinst)
{
  int i;
  int warn_or_error = 0;   /* warn - 0; error - 1  */
  int bubbles = 0;
  int remainder_bubbles = 0;
  char cur_insn[s7_INSN_NAME_LEN];
  char pre_insn[s7_INSN_NAME_LEN];
  struct s7_score_it nop_inst;
  struct s7_score_it pflush_inst;

  nop_inst.instruction = 0x0000;
  nop_inst.size = 2;
  nop_inst.relax_inst = 0x80008000;
  nop_inst.relax_size = 4;
  nop_inst.type = NO16_OPD;

  pflush_inst.instruction = 0x8000800a;
  pflush_inst.size = 4;
  pflush_inst.relax_inst = 0x8000;
  pflush_inst.relax_size = 0;
  pflush_inst.type = NO_OPD;

  /* pflush will clear all data dependency.  */
  if (strcmp (theinst->name, "pflush") == 0)
    {
      s7_init_dependency_vector ();
      return;
    }

  /* Push current instruction to s7_dependency_vector[0].  */
  for (i = s7_vector_size - 1; i > 0; i--)
    memcpy (&s7_dependency_vector[i], &s7_dependency_vector[i - 1], sizeof (s7_dependency_vector[i]));

  memcpy (&s7_dependency_vector[0], theinst, sizeof (s7_dependency_vector[i]));

  /* There is no dependency between nop and any instruction.  */
  if (strcmp (s7_dependency_vector[0].name, "nop") == 0
      || strcmp (s7_dependency_vector[0].name, "nop!") == 0)
    return;

  /* "pce" is defined in s7_insn_to_dependency_table.  */
#define PCE_NAME "pce"

  if (s7_dependency_vector[0].type == Insn_Type_PCE)
    strcpy (cur_insn, PCE_NAME);
  else
    strcpy (cur_insn, s7_dependency_vector[0].name);

  for (i = 1; i < s7_vector_size; i++)
    {
      /* The element of s7_dependency_vector is NULL.  */
      if (s7_dependency_vector[i].name[0] == '\0')
	continue;

      if (s7_dependency_vector[i].type == Insn_Type_PCE)
	strcpy (pre_insn, PCE_NAME);
      else
	strcpy (pre_insn, s7_dependency_vector[i].name);

      bubbles = s7_check_dependency (pre_insn, s7_dependency_vector[i].reg,
                                  cur_insn, s7_dependency_vector[0].reg, &warn_or_error);
      remainder_bubbles = bubbles - i + 1;

      if (remainder_bubbles > 0)
        {
          int j;

          if (s7_fix_data_dependency == 1)
            {
	      if (remainder_bubbles <= 2)
		{
		  if (s7_warn_fix_data_dependency)
		    as_warn (_("Fix data dependency: %s %s -- %s %s (insert %d nop!/%d)"),
			     s7_dependency_vector[i].name, s7_dependency_vector[i].reg,
			     s7_dependency_vector[0].name, s7_dependency_vector[0].reg,
			     remainder_bubbles, bubbles);

                  for (j = (s7_vector_size - 1); (j - remainder_bubbles) > 0; j--)
		    memcpy (&s7_dependency_vector[j], &s7_dependency_vector[j - remainder_bubbles],
			    sizeof (s7_dependency_vector[j]));

                  for (j = 1; j <= remainder_bubbles; j++)
                    {
                      memset (&s7_dependency_vector[j], '\0', sizeof (s7_dependency_vector[j]));
		      /* Insert nop!.  */
    		      s7_build_one_frag (nop_inst);
                    }
		}
	      else
		{
		  if (s7_warn_fix_data_dependency)
		    as_warn (_("Fix data dependency: %s %s -- %s %s (insert 1 pflush/%d)"),
			     s7_dependency_vector[i].name, s7_dependency_vector[i].reg,
			     s7_dependency_vector[0].name, s7_dependency_vector[0].reg,
			     bubbles);

                  for (j = 1; j < s7_vector_size; j++)
		    memset (&s7_dependency_vector[j], '\0', sizeof (s7_dependency_vector[j]));

                  /* Insert pflush.  */
                  s7_build_one_frag (pflush_inst);
		}
            }
          else
            {
	      if (warn_or_error)
		{
                  as_bad (_("data dependency: %s %s -- %s %s (%d/%d bubble)"),
                           s7_dependency_vector[i].name, s7_dependency_vector[i].reg,
                           s7_dependency_vector[0].name, s7_dependency_vector[0].reg,
                           remainder_bubbles, bubbles);
		}
	      else
		{
                  as_warn (_("data dependency: %s %s -- %s %s (%d/%d bubble)"),
                           s7_dependency_vector[i].name, s7_dependency_vector[i].reg,
                           s7_dependency_vector[0].name, s7_dependency_vector[0].reg,
                           remainder_bubbles, bubbles);
		}
            }
        }
    }
}

static enum insn_class
s7_get_insn_class_from_type (enum score_insn_type type)
{
  enum insn_class retval = (int) s7_FAIL;

  switch (type)
    {
    case Rd_I4:
    case Rd_I5:
    case Rd_rvalueBP_I5:
    case Rd_lvalueBP_I5:
    case Rd_I8:
    case PC_DISP8div2:
    case PC_DISP11div2:
    case Rd_Rs:
    case Rd_HighRs:
    case Rd_lvalueRs:
    case Rd_rvalueRs:
    case x_Rs:
    case Rd_LowRs:
    case NO16_OPD:
      retval = INSN_CLASS_16;
      break;
    case Rd_Rs_I5:
    case x_Rs_I5:
    case x_I5_x:
    case Rd_Rs_I14:
    case I15:
    case Rd_I16:
    case Rd_SI16:
    case Rd_rvalueRs_SI10:
    case Rd_lvalueRs_SI10:
    case Rd_rvalueRs_preSI12:
    case Rd_rvalueRs_postSI12:
    case Rd_lvalueRs_preSI12:
    case Rd_lvalueRs_postSI12:
    case Rd_Rs_SI14:
    case Rd_rvalueRs_SI15:
    case Rd_lvalueRs_SI15:
    case PC_DISP19div2:
    case PC_DISP24div2:
    case Rd_Rs_Rs:
    case x_Rs_x:
    case x_Rs_Rs:
    case Rd_Rs_x:
    case Rd_x_Rs:
    case Rd_x_x:
    case OP5_rvalueRs_SI15:
    case I5_Rs_Rs_I5_OP5:
    case x_rvalueRs_post4:
    case Rd_rvalueRs_post4:
    case Rd_x_I5:
    case Rd_lvalueRs_post4:
    case x_lvalueRs_post4:
    case Rd_Rs_Rs_imm:
    case NO_OPD:
    case Rd_lvalue32Rs:
    case Rd_rvalue32Rs:
    case Insn_GP:
    case Insn_PIC:
    case Insn_internal:
      retval = INSN_CLASS_32;
      break;
    case Insn_Type_PCE:
      retval = INSN_CLASS_PCE;
      break;
    case Insn_Type_SYN:
      retval = INSN_CLASS_SYN;
      break;
    default:
      abort ();
      break;
    }
  return retval;
}

static unsigned long
s7_adjust_paritybit (unsigned long m_code, enum insn_class i_class)
{
  unsigned long result = 0;
  unsigned long m_code_high = 0;
  unsigned long m_code_low = 0;
  unsigned long pb_high = 0;
  unsigned long pb_low = 0;

  if (i_class == INSN_CLASS_32)
    {
      pb_high = 0x80000000;
      pb_low = 0x00008000;
    }
  else if (i_class == INSN_CLASS_16)
    {
      pb_high = 0;
      pb_low = 0;
    }
  else if (i_class == INSN_CLASS_PCE)
    {
      pb_high = 0;
      pb_low = 0x00008000;
    }
  else if (i_class == INSN_CLASS_SYN)
    {
      /* FIXME.  at this time, INSN_CLASS_SYN must be 32 bit, but, instruction type should
         be changed if macro instruction has been expanded.  */
      pb_high = 0x80000000;
      pb_low = 0x00008000;
    }
  else
    {
      abort ();
    }

  m_code_high = m_code & 0x3fff8000;
  m_code_low = m_code & 0x00007fff;
  result = pb_high | (m_code_high << 1) | pb_low | m_code_low;
  return result;

}

static void
s7_gen_insn_frag (struct s7_score_it *part_1, struct s7_score_it *part_2)
{
  char *p;
  bool pce_p = false;
  int relaxable_p = s7_g_opt;
  int relax_size = 0;
  struct s7_score_it *inst1 = part_1;
  struct s7_score_it *inst2 = part_2;
  struct s7_score_it backup_inst1;

  pce_p = inst2 != NULL;
  memcpy (&backup_inst1, inst1, sizeof (struct s7_score_it));

  /* Adjust instruction opcode and to be relaxed instruction opcode.  */
  if (pce_p)
    {
      backup_inst1.instruction = ((backup_inst1.instruction & 0x7FFF) << 15)
                                  | (inst2->instruction & 0x7FFF);
      backup_inst1.instruction = s7_adjust_paritybit (backup_inst1.instruction, INSN_CLASS_PCE);
      if (!target_big_endian)
        {
          unsigned long tmp = backup_inst1.instruction;
          backup_inst1.instruction = ((tmp & 0xffff) << 16)
                                     | (tmp >> 16);
        }
      backup_inst1.relax_inst = 0x8000;
      backup_inst1.size = s7_INSN_SIZE;
      backup_inst1.relax_size = 0;
      backup_inst1.type = Insn_Type_PCE;
    }
  else
    {
      backup_inst1.instruction = s7_adjust_paritybit (backup_inst1.instruction,
						   s7_GET_INSN_CLASS (backup_inst1.type));
    }

  if (backup_inst1.relax_size != 0)
    {
      enum insn_class tmp;

      tmp = (backup_inst1.size == s7_INSN_SIZE) ? INSN_CLASS_16 : INSN_CLASS_32;
      backup_inst1.relax_inst = s7_adjust_paritybit (backup_inst1.relax_inst, tmp);
    }

  /* Check data dependency.  */
  s7_handle_dependency (&backup_inst1);

  /* Start a new frag if frag_now is not empty and is not instruction frag, maybe it contains
     data produced by .ascii etc.  Doing this is to make one instruction per frag.  */
  if (frag_now_fix () != 0)
    {
      if (!frag_now->tc_frag_data.is_insn)
	frag_wane (frag_now);

      frag_new (0);
    }

  /* Here, we must call frag_grow in order to keep the instruction frag type is
     rs_machine_dependent.
     For, frag_var may change frag_now->fr_type to rs_fill by calling frag_grow which
     actually will call frag_wane.
     Calling frag_grow first will create a new frag_now which free size is 20 that is enough
     for frag_var.  */
  frag_grow (20);

  p = frag_more (backup_inst1.size);
  s7_number_to_chars (p, backup_inst1.instruction, backup_inst1.size);

#ifdef OBJ_ELF
  dwarf2_emit_insn (backup_inst1.size);
#endif

  /* Generate fixup structure.  */
  if (pce_p)
    {
      if (inst1->reloc.type != BFD_RELOC_NONE)
	s7_fix_new_score (frag_now, p - frag_now->fr_literal,
		       inst1->size, &inst1->reloc.exp,
		       inst1->reloc.pc_rel, inst1->reloc.type);

      if (inst2->reloc.type != BFD_RELOC_NONE)
	s7_fix_new_score (frag_now, p - frag_now->fr_literal + 2,
		       inst2->size, &inst2->reloc.exp, inst2->reloc.pc_rel, inst2->reloc.type);
    }
  else
    {
      if (backup_inst1.reloc.type != BFD_RELOC_NONE)
	s7_fix_new_score (frag_now, p - frag_now->fr_literal,
		       backup_inst1.size, &backup_inst1.reloc.exp,
		       backup_inst1.reloc.pc_rel, backup_inst1.reloc.type);
    }

  /* relax_size may be 2, 4, 12 or 0, 0 indicates no relaxation.  */
  relaxable_p &= (backup_inst1.relax_size != 0);
  relax_size = relaxable_p ? backup_inst1.relax_size : 0;

  p = frag_var (rs_machine_dependent, relax_size + s7_RELAX_PAD_BYTE, 0,
                s7_RELAX_ENCODE (backup_inst1.size, backup_inst1.relax_size,
                              backup_inst1.type, 0, 0, relaxable_p),
                backup_inst1.reloc.exp.X_add_symbol, 0, NULL);

  if (relaxable_p)
    s7_number_to_chars (p, backup_inst1.relax_inst, relax_size);

  memcpy (inst1, &backup_inst1, sizeof (struct s7_score_it));
}

static void
s7_parse_16_32_inst (char *insnstr, bool gen_frag_p)
{
  char c;
  char *p;
  char *operator = insnstr;
  const struct s7_asm_opcode *opcode;

  /* Parse operator and operands.  */
  s7_skip_whitespace (operator);

  for (p = operator; *p != '\0'; p++)
    if ((*p == ' ') || (*p == '!'))
      break;

  if (*p == '!')
    p++;

  c = *p;
  *p = '\0';

  opcode = (const struct s7_asm_opcode *) str_hash_find (s7_score_ops_hsh,
							 operator);
  *p = c;

  memset (&s7_inst, '\0', sizeof (s7_inst));
  strcpy (s7_inst.str, insnstr);
  if (opcode)
    {
      s7_inst.instruction = opcode->value;
      s7_inst.relax_inst = opcode->relax_value;
      s7_inst.type = opcode->type;
      s7_inst.size = s7_GET_INSN_SIZE (s7_inst.type);
      s7_inst.relax_size = 0;
      s7_inst.bwarn = 0;
      strcpy (s7_inst.name, opcode->template_name);
      strcpy (s7_inst.reg, "");
      s7_inst.error = NULL;
      s7_inst.reloc.type = BFD_RELOC_NONE;

      (*opcode->parms) (p);

      /* It indicates current instruction is a macro instruction if s7_inst.bwarn equals -1.  */
      if ((s7_inst.bwarn != -1) && (!s7_inst.error) && (gen_frag_p))
	s7_gen_insn_frag (&s7_inst, NULL);
    }
  else
    s7_inst.error = _("unrecognized opcode");
}

static int
s7_append_insn (char *str, bool gen_frag_p)
{
  int retval = s7_SUCCESS;

  s7_parse_16_32_inst (str, gen_frag_p);

  if (s7_inst.error)
    {
      retval = (int) s7_FAIL;
      as_bad (_("%s -- `%s'"), s7_inst.error, s7_inst.str);
      s7_inst.error = NULL;
    }

  return retval;
}

/* Handle mv! reg_high, reg_low;
          mv! reg_low, reg_high;
          mv! reg_low, reg_low;  */
static void
s7_do16_mv_rdrs (char *str)
{
  int reg_rd;
  int reg_rs;
  char *backupstr = NULL;

  backupstr = str;
  s7_skip_whitespace (str);

  if ((reg_rd = s7_reg_required_here (&str, 8, s7_REG_TYPE_SCORE)) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || (reg_rs = s7_reg_required_here (&str, 4, s7_REG_TYPE_SCORE)) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      /* Case 1 : mv! or mlfh!.  */
      if (reg_rd < 16)
        {
          if (reg_rs < 16)
            {
              s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
                | (((s7_inst.instruction >> 4) & 0xf) << 15) | (0xf << 10);
              s7_inst.relax_size = 4;
            }
          else
            {
              char append_str[s7_MAX_LITERAL_POOL_SIZE];

              sprintf (append_str, "mlfh! %s", backupstr);
              if (s7_append_insn (append_str, true) == (int) s7_FAIL)
		return;
              /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
              s7_inst.bwarn = -1;
            }
        }
      /* Case 2 : mhfl!.  */
      else
        {
          if (reg_rs > 16)
            {
              s7_SET_INSN_ERROR (s7_BAD_ARGS);
              return;
            }
          else
            {
              char append_str[s7_MAX_LITERAL_POOL_SIZE];

              sprintf (append_str, "mhfl! %s", backupstr);
              if (s7_append_insn (append_str, true) == (int) s7_FAIL)
		return;

              /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
              s7_inst.bwarn = -1;
            }
        }
    }
}

static void
s7_do16_rdi4 (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reglow_required_here (&str, 8) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_data_op2 (&str, 3, _IMM4) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      if (((s7_inst.instruction >> 3) & 0x10) == 0)        /* for judge is addei or subei : bit 5 =0 : addei */
        {
          if (((s7_inst.instruction >> 3) & 0xf) != 0xf)
            {
              s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
                | ((1 << ((s7_inst.instruction >> 3) & 0xf)) << 1);
              s7_inst.relax_size = 4;
            }
          else
            {
              s7_inst.relax_inst = 0x8000;
            }
        }
      else
        {
          if (((s7_inst.instruction >> 3) & 0xf) != 0xf)
            {
              s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
                | (((-(1 << ((s7_inst.instruction >> 3) & 0xf))) & 0xffff) << 1);
              s7_inst.relax_size = 4;
            }
          else
            {
              s7_inst.relax_inst = 0x8000;
            }
        }
    }
}

static void
s7_do16_rdi5 (char *str)
{
  s7_skip_whitespace (str);

  if (s7_reglow_required_here (&str, 8) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_data_op2 (&str, 3, _IMM5) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;
  else
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
        | (((s7_inst.instruction >> 8) & 0xf) << 15) | (((s7_inst.instruction >> 3) & 0x1f) << 10);
      s7_inst.relax_size = 4;
    }
}

/* Handle sdbbp.  */

static void
s7_do16_xi5 (char *str)
{
  s7_skip_whitespace (str);

  if (s7_data_op2 (&str, 3, _IMM5) == (int) s7_FAIL || s7_end_of_line (str) == (int) s7_FAIL)
    return;
  else
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 3) & 0x1f) << 15);
      s7_inst.relax_size = 4;
    }
}

/* Check that an immediate is word alignment or half word alignment.
   If so, convert it to the right format.  */

static int
s7_validate_immediate_align (int val, unsigned int data_type)
{
  if (data_type == _IMM5_RSHIFT_1)
    {
      if (val % 2)
        {
          s7_inst.error = _("address offset must be half word alignment");
          return (int) s7_FAIL;
        }
    }
  else if ((data_type == _IMM5_RSHIFT_2) || (data_type == _IMM10_RSHIFT_2))
    {
      if (val % 4)
        {
          s7_inst.error = _("address offset must be word alignment");
          return (int) s7_FAIL;
        }
    }

  return s7_SUCCESS;
}

static int
s7_exp_ldst_offset (char **str, int shift, unsigned int data_type)
{
  char *dataptr;
  int hex_p = 0;

  dataptr = * str;

  if ((dataptr != NULL)
      && (((strstr (dataptr, "0x")) != NULL)
          || ((strstr (dataptr, "0X")) != NULL)))
    {
      hex_p = 1;
      if ((data_type != _SIMM16_LA)
        && (data_type != _VALUE_HI16)
        && (data_type != _VALUE_LO16)
        && (data_type != _IMM16)
        && (data_type != _IMM15)
        && (data_type != _IMM14)
        && (data_type != _IMM4)
        && (data_type != _IMM5)
        && (data_type != _IMM8)
        && (data_type != _IMM5_RSHIFT_1)
        && (data_type != _IMM5_RSHIFT_2)
        && (data_type != _SIMM12)
        && (data_type != _SIMM15)
        && (data_type != _SIMM14_NEG)
        && (data_type != _IMM10_RSHIFT_2))
        {
          data_type += 24;
        }
    }

  if (s7_my_get_expression (&s7_inst.reloc.exp, str) == (int) s7_FAIL)
    return (int) s7_FAIL;

  if (s7_inst.reloc.exp.X_op == O_constant)
    {
      /* Need to check the immediate align.  */
      int value = s7_validate_immediate_align (s7_inst.reloc.exp.X_add_number, data_type);

      if (value == (int) s7_FAIL)
	return (int) s7_FAIL;

      value = s7_validate_immediate (s7_inst.reloc.exp.X_add_number, data_type, hex_p);
      if (value == (int) s7_FAIL)
        {
          if (data_type < 30)
            sprintf (s7_err_msg,
                     _("invalid constant: %d bit expression not in range %d..%d"),
                     s7_score_df_range[data_type].bits,
                     s7_score_df_range[data_type].range[0], s7_score_df_range[data_type].range[1]);
          else
            sprintf (s7_err_msg,
                     _("invalid constant: %d bit expression not in range %d..%d"),
                     s7_score_df_range[data_type - 24].bits,
                     s7_score_df_range[data_type - 24].range[0], s7_score_df_range[data_type - 24].range[1]);
          s7_inst.error = s7_err_msg;
          return (int) s7_FAIL;
        }

      if (data_type == _IMM5_RSHIFT_1)
        {
          value >>= 1;
        }
      else if ((data_type == _IMM5_RSHIFT_2) || (data_type == _IMM10_RSHIFT_2))
        {
          value >>= 2;
        }

      if (s7_score_df_range[data_type].range[0] != 0)
        {
          value &= (1 << s7_score_df_range[data_type].bits) - 1;
        }

      s7_inst.instruction |= value << shift;
    }
  else
    {
      s7_inst.reloc.pc_rel = 0;
    }

  return s7_SUCCESS;
}

static void
s7_do_ldst_insn (char *str)
{
  int pre_inc = 0;
  int conflict_reg;
  int value;
  char * temp;
  char *dataptr;
  int reg;
  int ldst_idx = 0;

  int hex_p = 0;

  s7_skip_whitespace (str);

  if (((conflict_reg = s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
      || (s7_skip_past_comma (&str) == (int) s7_FAIL))
    return;

  /* ld/sw rD, [rA, simm15]    ld/sw rD, [rA]+, simm12     ld/sw rD, [rA, simm12]+.  */
  if (*str == '[')
    {
      str++;
      s7_skip_whitespace (str);

      if ((reg = s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
	return;

      /* Conflicts can occur on stores as well as loads.  */
      conflict_reg = (conflict_reg == reg);
      s7_skip_whitespace (str);
      temp = str + 1;    /* The latter will process decimal/hex expression.  */

      /* ld/sw rD, [rA]+, simm12    ld/sw rD, [rA]+.  */
      if (*str == ']')
        {
          str++;
          if (*str == '+')
            {
              str++;
              /* ld/sw rD, [rA]+, simm12.  */
              if (s7_skip_past_comma (&str) == s7_SUCCESS)
                {
                  if ((s7_exp_ldst_offset (&str, 3, _SIMM12) == (int) s7_FAIL)
                      || (s7_end_of_line (str) == (int) s7_FAIL))
		    return;

                  if (conflict_reg)
                    {
                      unsigned int ldst_func = s7_inst.instruction & OPC_PSEUDOLDST_MASK;

                      if ((ldst_func == INSN_LH)
                          || (ldst_func == INSN_LHU)
                          || (ldst_func == INSN_LW)
                          || (ldst_func == INSN_LB)
                          || (ldst_func == INSN_LBU))
                        {
                          s7_inst.error = _("register same as write-back base");
                          return;
                        }
                    }

                  ldst_idx = s7_inst.instruction & OPC_PSEUDOLDST_MASK;
                  s7_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
                  s7_inst.instruction |= s7_score_ldst_insns[ldst_idx * 3 + LDST_POST].value;

                  /* lw rD, [rA]+, 4 convert to pop rD, [rA].  */
                  if ((s7_inst.instruction & 0x3e000007) == 0x0e000000)
                    {
                      /* rs =  r0-r7, offset = 4 */
                      if ((((s7_inst.instruction >> 15) & 0x18) == 0)
                          && (((s7_inst.instruction >> 3) & 0xfff) == 4))
                        {
                          /* Relax to pophi.  */
                          if ((((s7_inst.instruction >> 20) & 0x10) == 0x10))
                            {
                              s7_inst.relax_inst = 0x0000200a | (((s7_inst.instruction >> 20) & 0xf)
                                                              << 8) | 1 << 7 |
                                (((s7_inst.instruction >> 15) & 0x7) << 4);
                            }
                          /* Relax to pop.  */
                          else
                            {
                              s7_inst.relax_inst = 0x0000200a | (((s7_inst.instruction >> 20) & 0xf)
                                                              << 8) | 0 << 7 |
                                (((s7_inst.instruction >> 15) & 0x7) << 4);
                            }
                          s7_inst.relax_size = 2;
                        }
                    }
                  return;
                }
              /* ld/sw rD, [rA]+ convert to ld/sw rD, [rA, 0]+.  */
              else
                {
                  s7_SET_INSN_ERROR (NULL);
                  if (s7_end_of_line (str) == (int) s7_FAIL)
                    {
                      return;
                    }

                  pre_inc = 1;
                  value = s7_validate_immediate (s7_inst.reloc.exp.X_add_number, _SIMM12, 0);
                  value &= (1 << s7_score_df_range[_SIMM12].bits) - 1;
                  ldst_idx = s7_inst.instruction & OPC_PSEUDOLDST_MASK;
                  s7_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
                  s7_inst.instruction |= s7_score_ldst_insns[ldst_idx * 3 + pre_inc].value;
                  s7_inst.instruction |= value << 3;
                  s7_inst.relax_inst = 0x8000;
                  return;
                }
            }
          /* ld/sw rD, [rA] convert to ld/sw rD, [rA, simm15].  */
          else
            {
              if (s7_end_of_line (str) == (int) s7_FAIL)
		return;

              ldst_idx = s7_inst.instruction & OPC_PSEUDOLDST_MASK;
              s7_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
              s7_inst.instruction |= s7_score_ldst_insns[ldst_idx * 3 + LDST_NOUPDATE].value;

              /* lbu rd, [rs] -> lbu! rd, [rs]  */
              if (ldst_idx == INSN_LBU)
                {
                  s7_inst.relax_inst = INSN16_LBU;
                }
              else if (ldst_idx == INSN_LH)
                {
                  s7_inst.relax_inst = INSN16_LH;
                }
              else if (ldst_idx == INSN_LW)
                {
                  s7_inst.relax_inst = INSN16_LW;
                }
              else if (ldst_idx == INSN_SB)
                {
                  s7_inst.relax_inst = INSN16_SB;
                }
              else if (ldst_idx == INSN_SH)
                {
                  s7_inst.relax_inst = INSN16_SH;
                }
              else if (ldst_idx == INSN_SW)
                {
                  s7_inst.relax_inst = INSN16_SW;
                }
              else
                {
                  s7_inst.relax_inst = 0x8000;
                }

              /* lw/lh/lbu/sw/sh/sb, offset = 0, relax to 16 bit instruction.  */
              if ((ldst_idx == INSN_LBU)
                  || (ldst_idx == INSN_LH)
                  || (ldst_idx == INSN_LW)
                  || (ldst_idx == INSN_SB) || (ldst_idx == INSN_SH) || (ldst_idx == INSN_SW))
                {
                  if ((((s7_inst.instruction >> 15) & 0x10) == 0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
                    {
                      s7_inst.relax_inst |= (2 << 12) | (((s7_inst.instruction >> 20) & 0xf) << 8) |
                        (((s7_inst.instruction >> 15) & 0xf) << 4);
                      s7_inst.relax_size = 2;
                    }
                }

              return;
            }
        }
      /* ld/sw rD, [rA, simm15]    ld/sw rD, [rA, simm12]+.  */
      else
        {
          if (s7_skip_past_comma (&str) == (int) s7_FAIL)
            {
              s7_inst.error = _("pre-indexed expression expected");
              return;
            }

          if (s7_my_get_expression (&s7_inst.reloc.exp, &str) == (int) s7_FAIL)
	    return;

          s7_skip_whitespace (str);
          if (*str++ != ']')
            {
              s7_inst.error = _("missing ]");
              return;
            }

          s7_skip_whitespace (str);
          /* ld/sw rD, [rA, simm12]+.  */
          if (*str == '+')
            {
              str++;
              pre_inc = 1;
              if (conflict_reg)
                {
                  unsigned int ldst_func = s7_inst.instruction & OPC_PSEUDOLDST_MASK;

                  if ((ldst_func == INSN_LH)
                      || (ldst_func == INSN_LHU)
                      || (ldst_func == INSN_LW)
                      || (ldst_func == INSN_LB)
                      || (ldst_func == INSN_LBU))
                    {
                      s7_inst.error = _("register same as write-back base");
                      return;
                    }
                }
            }

          if (s7_end_of_line (str) == (int) s7_FAIL)
	    return;

          if (s7_inst.reloc.exp.X_op == O_constant)
            {
              unsigned int data_type;

              if (pre_inc == 1)
                data_type = _SIMM12;
              else
                data_type = _SIMM15;
              dataptr = temp;

            if ((dataptr != NULL)
              && (((strstr (dataptr, "0x")) != NULL)
                  || ((strstr (dataptr, "0X")) != NULL)))
              {
                hex_p = 1;
                if ((data_type != _SIMM16_LA)
                    && (data_type != _VALUE_HI16)
                    && (data_type != _VALUE_LO16)
                    && (data_type != _IMM16)
                    && (data_type != _IMM15)
                    && (data_type != _IMM14)
                    && (data_type != _IMM4)
                    && (data_type != _IMM5)
                    && (data_type != _IMM8)
                    && (data_type != _SIMM12)
                    && (data_type != _SIMM15)
                    && (data_type != _IMM5_RSHIFT_1)
                    && (data_type != _IMM5_RSHIFT_2)
                    && (data_type != _SIMM14_NEG)
                    && (data_type != _IMM10_RSHIFT_2))
                  {
                    data_type += 24;
                  }
              }

              value = s7_validate_immediate (s7_inst.reloc.exp.X_add_number, data_type, hex_p);
              if (value == (int) s7_FAIL)
                {
                  if (data_type < 30)
                    sprintf (s7_err_msg,
                             _("invalid constant: %d bit expression not in range %d..%d"),
                             s7_score_df_range[data_type].bits,
                             s7_score_df_range[data_type].range[0], s7_score_df_range[data_type].range[1]);
                  else
                    sprintf (s7_err_msg,
                             _("invalid constant: %d bit expression not in range %d..%d"),
                             s7_score_df_range[data_type - 24].bits,
                             s7_score_df_range[data_type - 24].range[0],
                             s7_score_df_range[data_type - 24].range[1]);
                  s7_inst.error = s7_err_msg;
                  return;
                }

              value &= (1 << s7_score_df_range[data_type].bits) - 1;
              ldst_idx = s7_inst.instruction & OPC_PSEUDOLDST_MASK;
              s7_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
              s7_inst.instruction |= s7_score_ldst_insns[ldst_idx * 3 + pre_inc].value;
              if (pre_inc == 1)
                s7_inst.instruction |= value << 3;
              else
                s7_inst.instruction |= value;

              /* lw rD, [rA, simm15]  */
              if ((s7_inst.instruction & 0x3e000000) == 0x20000000)
                {
                  /* Both rD and rA are in [r0 - r15].  */
                  if ((((s7_inst.instruction >> 15) & 0x10) == 0)
                      && (((s7_inst.instruction >> 20) & 0x10) == 0))
                    {
                      /* simm15 = 0, lw -> lw!.  */
                      if ((s7_inst.instruction & 0x7fff) == 0)
                        {
                          s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0xf) << 4)
                            | (((s7_inst.instruction >> 20) & 0xf) << 8);
                          s7_inst.relax_size = 2;
                        }
                      /* rA = r2, lw -> lwp!.  */
                      else if ((((s7_inst.instruction >> 15) & 0xf) == 2)
                               && ((s7_inst.instruction & 0x3) == 0)
                               && ((s7_inst.instruction & 0x7fff) < 128))
                        {
                          s7_inst.relax_inst = 0x7000 | (((s7_inst.instruction >> 20) & 0xf) << 8)
                            | (((s7_inst.instruction & 0x7fff) >> 2) << 3);
                          s7_inst.relax_size = 2;
                        }
                      else
                        {
                          s7_inst.relax_inst = 0x8000;
                        }
                    }
                  else
                    {
                      s7_inst.relax_inst = 0x8000;
                    }
                }
              /* sw rD, [rA, simm15]  */
              else if ((s7_inst.instruction & 0x3e000000) == 0x28000000)
                {
                  /* Both rD and rA are in [r0 - r15].  */
                  if ((((s7_inst.instruction >> 15) & 0x10) == 0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
                    {
                      /* simm15 = 0, sw -> sw!.  */
                      if ((s7_inst.instruction & 0x7fff) == 0)
                        {
                          s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0xf) << 4)
                            | (((s7_inst.instruction >> 20) & 0xf) << 8);
                          s7_inst.relax_size = 2;
                        }
                      /* rA = r2, sw -> swp!.  */
                      else if ((((s7_inst.instruction >> 15) & 0xf) == 2)
                               && ((s7_inst.instruction & 0x3) == 0)
                               && ((s7_inst.instruction & 0x7fff) < 128))
                        {
                          s7_inst.relax_inst = 0x7004 | (((s7_inst.instruction >> 20) & 0xf) << 8)
                            | (((s7_inst.instruction & 0x7fff) >> 2) << 3);
                          s7_inst.relax_size = 2;
                        }
                      else
                        {
                          s7_inst.relax_inst = 0x8000;
                        }
                    }
                  else
                    {
                      s7_inst.relax_inst = 0x8000;
                    }
                }
              /* sw rD, [rA, simm15]+    sw pre.  */
              else if ((s7_inst.instruction & 0x3e000007) == 0x06000004)
                {
                  /* rA is in [r0 - r7], and simm15 = -4.  */
                  if ((((s7_inst.instruction >> 15) & 0x18) == 0)
                      && (((s7_inst.instruction >> 3) & 0xfff) == 0xffc))
                    {
                      /* sw -> pushhi!.  */
                      if ((((s7_inst.instruction >> 20) & 0x10) == 0x10))
                        {
                          s7_inst.relax_inst = 0x0000200e | (((s7_inst.instruction >> 20) & 0xf) << 8)
                            | 1 << 7 | (((s7_inst.instruction >> 15) & 0x7) << 4);
                          s7_inst.relax_size = 2;
                        }
                      /* sw -> push!.  */
                      else
                        {
                          s7_inst.relax_inst = 0x0000200e | (((s7_inst.instruction >> 20) & 0xf) << 8)
                            | 0 << 7 | (((s7_inst.instruction >> 15) & 0x7) << 4);
                          s7_inst.relax_size = 2;
                        }
                    }
                  else
                    {
                      s7_inst.relax_inst = 0x8000;
                    }
                }
              /* lh rD, [rA, simm15]  */
              else if ((s7_inst.instruction & 0x3e000000) == 0x22000000)
                {
                  /* Both rD and rA are in [r0 - r15].  */
                  if ((((s7_inst.instruction >> 15) & 0x10) == 0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
                    {
                      /* simm15 = 0, lh -> lh!.  */
                      if ((s7_inst.instruction & 0x7fff) == 0)
                        {
                          s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0xf) << 4)
                            | (((s7_inst.instruction >> 20) & 0xf) << 8);
                          s7_inst.relax_size = 2;
                        }
                      /* rA = r2, lh -> lhp!.  */
                      else if ((((s7_inst.instruction >> 15) & 0xf) == 2)
                               && ((s7_inst.instruction & 0x1) == 0)
                               && ((s7_inst.instruction & 0x7fff) < 64))
                        {
                          s7_inst.relax_inst = 0x7001 | (((s7_inst.instruction >> 20) & 0xf) << 8)
                            | (((s7_inst.instruction & 0x7fff) >> 1) << 3);
                          s7_inst.relax_size = 2;
                        }
                      else
                        {
                          s7_inst.relax_inst = 0x8000;
                        }
                    }
                  else
                    {
                      s7_inst.relax_inst = 0x8000;
                    }
                }
              /* sh rD, [rA, simm15]  */
              else if ((s7_inst.instruction & 0x3e000000) == 0x2a000000)
                {
                  /* Both rD and rA are in [r0 - r15].  */
                  if ((((s7_inst.instruction >> 15) & 0x10) == 0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
                    {
                      /* simm15 = 0, sh -> sh!.  */
                      if ((s7_inst.instruction & 0x7fff) == 0)
                        {
                          s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0xf) << 4)
                            | (((s7_inst.instruction >> 20) & 0xf) << 8);
                          s7_inst.relax_size = 2;
                        }
                      /* rA = r2, sh -> shp!.  */
                      else if ((((s7_inst.instruction >> 15) & 0xf) == 2)
                               && ((s7_inst.instruction & 0x1) == 0)
                               && ((s7_inst.instruction & 0x7fff) < 64))
                        {
                          s7_inst.relax_inst = 0x7005 | (((s7_inst.instruction >> 20) & 0xf) << 8)
                            | (((s7_inst.instruction & 0x7fff) >> 1) << 3);
                          s7_inst.relax_size = 2;
                        }
                      else
                        {
                          s7_inst.relax_inst = 0x8000;
                        }
                    }
                  else
                    {
                      s7_inst.relax_inst = 0x8000;
                    }
                }
              /* lbu rD, [rA, simm15]  */
              else if ((s7_inst.instruction & 0x3e000000) == 0x2c000000)
                {
                  /* Both rD and rA are in [r0 - r15].  */
                  if ((((s7_inst.instruction >> 15) & 0x10) == 0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
                    {
                      /* simm15 = 0, lbu -> lbu!.  */
                      if ((s7_inst.instruction & 0x7fff) == 0)
                        {
                          s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0xf) << 4)
                            | (((s7_inst.instruction >> 20) & 0xf) << 8);
                          s7_inst.relax_size = 2;
                        }
                      /* rA = r2, lbu -> lbup!.  */
                      else if ((((s7_inst.instruction >> 15) & 0xf) == 2)
                               && ((s7_inst.instruction & 0x7fff) < 32))
                        {
                          s7_inst.relax_inst = 0x7003 | (((s7_inst.instruction >> 20) & 0xf) << 8)
                            | ((s7_inst.instruction & 0x7fff) << 3);
                          s7_inst.relax_size = 2;
                        }
                      else
                        {
                          s7_inst.relax_inst = 0x8000;
                        }
                    }
                  else
                    {
                      s7_inst.relax_inst = 0x8000;
                    }
                }
              /* sb rD, [rA, simm15]  */
              else if ((s7_inst.instruction & 0x3e000000) == 0x2e000000)
                {
                  /* Both rD and rA are in [r0 - r15].  */
                  if ((((s7_inst.instruction >> 15) & 0x10) == 0) && (((s7_inst.instruction >> 20) & 0x10) == 0))
                    {
                      /* simm15 = 0, sb -> sb!.  */
                      if ((s7_inst.instruction & 0x7fff) == 0)
                        {
                          s7_inst.relax_inst |= (((s7_inst.instruction >> 15) & 0xf) << 4)
                            | (((s7_inst.instruction >> 20) & 0xf) << 8);
                          s7_inst.relax_size = 2;
                        }
                      /* rA = r2, sb -> sb!.  */
                      else if ((((s7_inst.instruction >> 15) & 0xf) == 2)
                               && ((s7_inst.instruction & 0x7fff) < 32))
                        {
                          s7_inst.relax_inst = 0x7007 | (((s7_inst.instruction >> 20) & 0xf) << 8)
                            | ((s7_inst.instruction & 0x7fff) << 3);
                          s7_inst.relax_size = 2;
                        }
                      else
                        {
                          s7_inst.relax_inst = 0x8000;
                        }
                    }
                  else
                    {
                      s7_inst.relax_inst = 0x8000;
                    }
                }
              else
                {
                  s7_inst.relax_inst = 0x8000;
                }

              return;
            }
          else
            {
              /* FIXME: may set error, for there is no ld/sw rD, [rA, label] */
              s7_inst.reloc.pc_rel = 0;
            }
        }
    }
  else
    {
      s7_inst.error = s7_BAD_ARGS;
    }
}

/* Handle cache.  */
static void
s7_do_cache (char *str)
{
  s7_skip_whitespace (str);

  if ((s7_data_op2 (&str, 20, _IMM5) == (int) s7_FAIL) || (s7_skip_past_comma (&str) == (int) s7_FAIL))
    {
      return;
    }
  else
    {
      int cache_op;

      cache_op = (s7_inst.instruction >> 20) & 0x1F;
      sprintf (s7_inst.name, "cache %d", cache_op);
    }

  if (*str == '[')
    {
      str++;
      s7_skip_whitespace (str);

      if (s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL)
	return;

      s7_skip_whitespace (str);

      /* cache op, [rA]  */
      if (s7_skip_past_comma (&str) == (int) s7_FAIL)
        {
          s7_SET_INSN_ERROR (NULL);
          if (*str != ']')
            {
              s7_inst.error = _("missing ]");
              return;
            }
          str++;
        }
      /* cache op, [rA, simm15]  */
      else
        {
          if (s7_exp_ldst_offset (&str, 0, _SIMM15) == (int) s7_FAIL)
            {
              return;
            }

          s7_skip_whitespace (str);
          if (*str++ != ']')
            {
              s7_inst.error = _("missing ]");
              return;
            }
        }

      if (s7_end_of_line (str) == (int) s7_FAIL)
	return;
    }
  else
    {
      s7_inst.error = s7_BAD_ARGS;
    }
}

static void
s7_do_crdcrscrsimm5 (char *str)
{
  char *strbak;

  strbak = str;
  s7_skip_whitespace (str);

  if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE_CR) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE_CR) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL
      || s7_reg_required_here (&str, 10, s7_REG_TYPE_SCORE_CR) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL)
    {
      str = strbak;
      /* cop1 cop_code20.  */
      if (s7_data_op2 (&str, 5, _IMM20) == (int) s7_FAIL)
	return;
    }
  else
    {
      if (s7_data_op2 (&str, 5, _IMM5) == (int) s7_FAIL)
	return;
    }

  s7_end_of_line (str);
}

/* Handle ldc/stc.  */
static void
s7_do_ldst_cop (char *str)
{
  s7_skip_whitespace (str);

  if ((s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE_CR) == (int) s7_FAIL)
      || (s7_skip_past_comma (&str) == (int) s7_FAIL))
    return;

  if (*str == '[')
    {
      str++;
      s7_skip_whitespace (str);

      if (s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) == (int) s7_FAIL)
	return;

      s7_skip_whitespace (str);

      if (*str++ != ']')
        {
          if (s7_exp_ldst_offset (&str, 5, _IMM10_RSHIFT_2) == (int) s7_FAIL)
	    return;

          s7_skip_whitespace (str);
          if (*str++ != ']')
            {
              s7_inst.error = _("missing ]");
              return;
            }
        }

      s7_end_of_line (str);
    }
  else
    s7_inst.error = s7_BAD_ARGS;
}

static void
s7_do16_ldst_insn (char *str)
{
  s7_skip_whitespace (str);

  if ((s7_reglow_required_here (&str, 8) == (int) s7_FAIL) || (s7_skip_past_comma (&str) == (int) s7_FAIL))
    return;

  if (*str == '[')
    {
      int reg;

      str++;
      s7_skip_whitespace (str);

      if ((reg = s7_reglow_required_here (&str, 4)) == (int) s7_FAIL)
	return;

      s7_skip_whitespace (str);
      if (*str++ == ']')
        {
          if (s7_end_of_line (str) == (int) s7_FAIL)
	    return;
          else
            {
              s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
                              | (((s7_inst.instruction >> 4) & 0xf) << 15);
	      s7_inst.relax_size = 4;
            }
        }
      else
        {
          s7_inst.error = _("missing ]");
        }
    }
  else
    {
      s7_inst.error = s7_BAD_ARGS;
    }
}

/* Handle lbup!/lhp!/ldiu!/lwp!/sbp!/shp!/swp!.  */

static void
s7_do16_ldst_imm_insn (char *str)
{
  char data_exp[s7_MAX_LITERAL_POOL_SIZE];
  int reg_rd;
  char *dataptr = NULL, *pp = NULL;
  int cnt = 0;
  int assign_data = (int) s7_FAIL;
  unsigned int ldst_func;

  s7_skip_whitespace (str);

  if (((reg_rd = s7_reglow_required_here (&str, 8)) == (int) s7_FAIL)
      || (s7_skip_past_comma (&str) == (int) s7_FAIL))
    return;

  s7_skip_whitespace (str);
  dataptr = str;

  while ((*dataptr != '\0') && (*dataptr != '|') && (cnt <= s7_MAX_LITERAL_POOL_SIZE))
    {
      data_exp[cnt] = *dataptr;
      dataptr++;
      cnt++;
    }

  data_exp[cnt] = '\0';
  pp = &data_exp[0];

  str = dataptr;

  ldst_func = s7_inst.instruction & LDST16_RI_MASK;
  if (ldst_func == N16_LIU)
    assign_data = s7_exp_ldst_offset (&pp, 0, _IMM8);
  else if (ldst_func == N16_LHP || ldst_func == N16_SHP)
    assign_data = s7_exp_ldst_offset (&pp, 3, _IMM5_RSHIFT_1);
  else if (ldst_func == N16_LWP || ldst_func == N16_SWP)
    assign_data = s7_exp_ldst_offset (&pp, 3, _IMM5_RSHIFT_2);
  else
    assign_data = s7_exp_ldst_offset (&pp, 3, _IMM5);

  if ((assign_data == (int) s7_FAIL) || (s7_end_of_line (pp) == (int) s7_FAIL))
    return;
  else
    {
      if ((s7_inst.instruction & 0x7000) == N16_LIU)
        {
          s7_inst.relax_inst |= ((s7_inst.instruction >> 8) & 0xf) << 20
                          | ((s7_inst.instruction & 0xff) << 1);
        }
      else if (((s7_inst.instruction & 0x7007) == N16_LHP)
               || ((s7_inst.instruction & 0x7007) == N16_SHP))
        {
          s7_inst.relax_inst |= ((s7_inst.instruction >> 8) & 0xf) << 20 | 2 << 15
                          | (((s7_inst.instruction >> 3) & 0x1f) << 1);
        }
      else if (((s7_inst.instruction & 0x7007) == N16_LWP)
               || ((s7_inst.instruction & 0x7007) == N16_SWP))
        {
          s7_inst.relax_inst |= ((s7_inst.instruction >> 8) & 0xf) << 20 | 2 << 15
                          | (((s7_inst.instruction >> 3) & 0x1f) << 2);
        }
      else if (((s7_inst.instruction & 0x7007) == N16_LBUP)
               || ((s7_inst.instruction & 0x7007) == N16_SBP))
        {
          s7_inst.relax_inst |= ((s7_inst.instruction >> 8) & 0xf) << 20 | 2 << 15
                          | (((s7_inst.instruction >> 3) & 0x1f));
        }

      s7_inst.relax_size = 4;
    }
}

static void
s7_do16_push_pop (char *str)
{
  int reg_rd;
  int H_bit_mask = 0;

  s7_skip_whitespace (str);
  if (((reg_rd = s7_reg_required_here (&str, 8, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
      || (s7_skip_past_comma (&str) == (int) s7_FAIL))
    return;

  if (reg_rd >= 16)
    H_bit_mask = 1;

  /* s7_reg_required_here will change bit 12 of opcode, so we must restore bit 12.  */
  s7_inst.instruction &= ~(1 << 12);

  s7_inst.instruction |= H_bit_mask << 7;

  if (*str == '[')
    {
      int reg;

      str++;
      s7_skip_whitespace (str);
      if ((reg = s7_reg_required_here (&str, 4, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
	return;
      else if (reg > 7)
        {
          if (!s7_inst.error)
	    s7_inst.error = _("base register nums are over 3 bit");

          return;
        }

      s7_skip_whitespace (str);
      if ((*str++ != ']') || (s7_end_of_line (str) == (int) s7_FAIL))
        {
          if (!s7_inst.error)
	    s7_inst.error = _("missing ]");

          return;
        }

      /* pop! */
      if ((s7_inst.instruction & 0xf) == 0xa)
        {
          if (H_bit_mask)
            {
              s7_inst.relax_inst |= ((((s7_inst.instruction >> 8) & 0xf) | 0x10) << 20)
                                  | (((s7_inst.instruction >> 4) & 0x7) << 15) | (4 << 3);
            }
          else
            {
              s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
                                  | (((s7_inst.instruction >> 4) & 0x7) << 15) | (4 << 3);
            }
        }
      /* push! */
      else
        {
          if (H_bit_mask)
            {
              s7_inst.relax_inst |= ((((s7_inst.instruction >> 8) & 0xf) | 0x10) << 20)
                                  | (((s7_inst.instruction >> 4) & 0x7) << 15) | (((-4) & 0xfff) << 3);
            }
          else
            {
              s7_inst.relax_inst |= (((s7_inst.instruction >> 8) & 0xf) << 20)
                                  | (((s7_inst.instruction >> 4) & 0x7) << 15) | (((-4) & 0xfff) << 3);
            }
        }
      s7_inst.relax_size = 4;
    }
  else
    {
      s7_inst.error = s7_BAD_ARGS;
    }
}

/* Handle lcb/lcw/lce/scb/scw/sce.  */
static void
s7_do_ldst_unalign (char *str)
{
  int conflict_reg;

  if (s7_university_version == 1)
    {
      s7_inst.error = s7_ERR_FOR_SCORE5U_ATOMIC;
      return;
    }

  s7_skip_whitespace (str);

  /* lcb/scb [rA]+.  */
  if (*str == '[')
    {
      str++;
      s7_skip_whitespace (str);

      if (s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE) == (int) s7_FAIL)
	return;

      if (*str++ == ']')
        {
          if (*str++ != '+')
            {
              s7_inst.error = _("missing +");
              return;
            }
        }
      else
        {
          s7_inst.error = _("missing ]");
          return;
        }

      if (s7_end_of_line (str) == (int) s7_FAIL)
	return;
    }
  /* lcw/lce/scb/sce rD, [rA]+.  */
  else
    {
      if (((conflict_reg = s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
          || (s7_skip_past_comma (&str) == (int) s7_FAIL))
        {
          return;
        }

      s7_skip_whitespace (str);
      if (*str++ == '[')
        {
          int reg;

          s7_skip_whitespace (str);
          if ((reg = s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
            {
              return;
            }

          /* Conflicts can occur on stores as well as loads.  */
          conflict_reg = (conflict_reg == reg);
          s7_skip_whitespace (str);
          if (*str++ == ']')
            {
              unsigned int ldst_func = s7_inst.instruction & LDST_UNALIGN_MASK;

              if (*str++ == '+')
                {
                  if (conflict_reg)
                    {
                      as_warn (_("%s register same as write-back base"),
                               ((ldst_func & UA_LCE) || (ldst_func & UA_LCW)
                                ? _("destination") : _("source")));
                    }
                }
              else
                {
                  s7_inst.error = _("missing +");
                  return;
                }

              if (s7_end_of_line (str) == (int) s7_FAIL)
		return;
            }
          else
            {
              s7_inst.error = _("missing ]");
              return;
            }
        }
      else
        {
          s7_inst.error = s7_BAD_ARGS;
          return;
        }
    }
}

/* Handle alw/asw.  */

static void
s7_do_ldst_atomic (char *str)
{
  if (s7_university_version == 1)
    {
      s7_inst.error = s7_ERR_FOR_SCORE5U_ATOMIC;
      return;
    }

  s7_skip_whitespace (str);

  if ((s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE) == (int) s7_FAIL)
      || (s7_skip_past_comma (&str) == (int) s7_FAIL))
    {
      return;
    }
  else
    {

      s7_skip_whitespace (str);
      if (*str++ == '[')
        {
          int reg;

          s7_skip_whitespace (str);
          if ((reg = s7_reg_required_here (&str, 15, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
            {
              return;
            }

          s7_skip_whitespace (str);
          if (*str++ != ']')
            {
              s7_inst.error = _("missing ]");
              return;
            }

          s7_end_of_line (str);
        }
      else
	s7_inst.error = s7_BAD_ARGS;
    }
}

static void
s7_build_relax_frag (struct s7_score_it fix_insts[s7_RELAX_INST_NUM],
		     int fix_num ATTRIBUTE_UNUSED,
		     struct s7_score_it var_insts[s7_RELAX_INST_NUM], int var_num,
		     symbolS *add_symbol)
{
  int i;
  char *p;
  fixS *fixp = NULL;
  fixS *cur_fixp = NULL;
  long where;
  struct s7_score_it inst_main;

  memcpy (&inst_main, &fix_insts[0], sizeof (struct s7_score_it));

  /* Adjust instruction opcode and to be relaxed instruction opcode.  */
  inst_main.instruction = s7_adjust_paritybit (inst_main.instruction, s7_GET_INSN_CLASS (inst_main.type));
  inst_main.type = Insn_PIC;

  for (i = 0; i < var_num; i++)
    {
      inst_main.relax_size += var_insts[i].size;
      var_insts[i].instruction = s7_adjust_paritybit (var_insts[i].instruction,
                                                   s7_GET_INSN_CLASS (var_insts[i].type));
    }

  /* Check data dependency.  */
  s7_handle_dependency (&inst_main);

  /* Start a new frag if frag_now is not empty.  */
  if (frag_now_fix () != 0)
    {
      if (!frag_now->tc_frag_data.is_insn)
	{
          frag_wane (frag_now);
	}
      frag_new (0);
    }
  frag_grow (20);

  /* Write fr_fix part.  */
  p = frag_more (inst_main.size);
  s7_number_to_chars (p, inst_main.instruction, inst_main.size);

  if (inst_main.reloc.type != BFD_RELOC_NONE)
    fixp = s7_fix_new_score (frag_now, p - frag_now->fr_literal, inst_main.size,
			  &inst_main.reloc.exp, inst_main.reloc.pc_rel, inst_main.reloc.type);

  frag_now->tc_frag_data.fixp = fixp;
  cur_fixp = frag_now->tc_frag_data.fixp;

#ifdef OBJ_ELF
  dwarf2_emit_insn (inst_main.size);
#endif

  where = p - frag_now->fr_literal + inst_main.size;
  for (i = 0; i < var_num; i++)
    {
      if (i > 0)
        where += var_insts[i - 1].size;

      if (var_insts[i].reloc.type != BFD_RELOC_NONE)
        {
          fixp = s7_fix_new_score (frag_now, where, var_insts[i].size,
                                &var_insts[i].reloc.exp, var_insts[i].reloc.pc_rel,
                                var_insts[i].reloc.type);
          if (fixp)
            {
              if (cur_fixp)
                {
                  cur_fixp->fx_next = fixp;
                  cur_fixp = cur_fixp->fx_next;
                }
              else
                {
                  frag_now->tc_frag_data.fixp = fixp;
                  cur_fixp = frag_now->tc_frag_data.fixp;
                }
	    }
        }
    }

  p = frag_var (rs_machine_dependent, inst_main.relax_size + s7_RELAX_PAD_BYTE, 0,
                s7_RELAX_ENCODE (inst_main.size, inst_main.relax_size, inst_main.type,
                0, inst_main.size, 0), add_symbol, 0, NULL);

  /* Write fr_var part.
     no calling s7_gen_insn_frag, no fixS will be generated.  */
  for (i = 0; i < var_num; i++)
    {
      s7_number_to_chars (p, var_insts[i].instruction, var_insts[i].size);
      p += var_insts[i].size;
    }
  /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
  s7_inst.bwarn = -1;
}

/* Build a relax frag for la instruction when generating s7_PIC,
   external symbol first and local symbol second.  */

static void
s7_build_la_pic (int reg_rd, expressionS exp)
{
  symbolS *add_symbol = exp.X_add_symbol;
  offsetT add_number = exp.X_add_number;
  struct s7_score_it fix_insts[s7_RELAX_INST_NUM];
  struct s7_score_it var_insts[s7_RELAX_INST_NUM];
  int fix_num = 0;
  int var_num = 0;
  char tmp[s7_MAX_LITERAL_POOL_SIZE];
  int r1_bak;

  r1_bak = s7_nor1;
  s7_nor1 = 0;

  if (add_number == 0)
    {
      fix_num = 1;
      var_num = 2;

      /* For an external symbol, only one insn is generated;
         For a local symbol, two insns are generated.  */
      /* Fix part
         For an external symbol: lw rD, <sym>($gp)
                                 (BFD_RELOC_SCORE_GOT15 or BFD_RELOC_SCORE_CALL15)  */
      sprintf (tmp, "lw_pic r%d, %s", reg_rd, S_GET_NAME (add_symbol));
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
	return;

      if (reg_rd == s7_PIC_CALL_REG)
        s7_inst.reloc.type = BFD_RELOC_SCORE_CALL15;
      memcpy (&fix_insts[0], &s7_inst, sizeof (struct s7_score_it));

      /* Var part
	 For a local symbol :
         lw rD, <sym>($gp)    (BFD_RELOC_SCORE_GOT15)
	 addi rD, <sym>       (BFD_RELOC_GOT_LO16) */
      s7_inst.reloc.type = BFD_RELOC_SCORE_GOT15;
      memcpy (&var_insts[0], &s7_inst, sizeof (struct s7_score_it));
      sprintf (tmp, "addi_s_pic r%d, %s", reg_rd, S_GET_NAME (add_symbol));
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
	return;

      memcpy (&var_insts[1], &s7_inst, sizeof (struct s7_score_it));
      s7_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);
    }
  else if (add_number >= -0x8000 && add_number <= 0x7fff)
    {
      /* Insn 1: lw rD, <sym>($gp)    (BFD_RELOC_SCORE_GOT15)  */
      sprintf (tmp, "lw_pic r%d, %s", reg_rd, S_GET_NAME (add_symbol));
      if (s7_append_insn (tmp, true) == (int) s7_FAIL)
	return;

      /* Insn 2  */
      fix_num = 1;
      var_num = 1;
      /* Fix part
         For an external symbol: addi rD, <constant> */
      sprintf (tmp, "addi r%d, %d", reg_rd, (int) add_number);
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
	return;

      memcpy (&fix_insts[0], &s7_inst, sizeof (struct s7_score_it));

      /* Var part
 	 For a local symbol: addi rD, <sym>+<constant>    (BFD_RELOC_GOT_LO16)  */
      sprintf (tmp, "addi_s_pic r%d, %s + %d", reg_rd,
	       S_GET_NAME (add_symbol), (int) add_number);
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
	return;

      memcpy (&var_insts[0], &s7_inst, sizeof (struct s7_score_it));
      s7_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);
    }
  else
    {
      int hi = (add_number >> 16) & 0x0000FFFF;
      int lo = add_number & 0x0000FFFF;

      /* Insn 1: lw rD, <sym>($gp)    (BFD_RELOC_SCORE_GOT15)  */
      sprintf (tmp, "lw_pic r%d, %s", reg_rd, S_GET_NAME (add_symbol));
      if (s7_append_insn (tmp, true) == (int) s7_FAIL)
	return;

      /* Insn 2  */
      fix_num = 1;
      var_num = 1;
      /* Fix part
	 For an external symbol: ldis r1, HI%<constant>  */
      sprintf (tmp, "ldis r1, %d", hi);
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
	return;

      memcpy (&fix_insts[0], &s7_inst, sizeof (struct s7_score_it));

      /* Var part
	 For a local symbol: ldis r1, HI%<constant>
         but, if lo is out of 16 bit, make hi plus 1  */
      if ((lo < -0x8000) || (lo > 0x7fff))
	{
	  hi += 1;
	}
      sprintf (tmp, "ldis_pic r1, %d", hi);
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
	return;

      memcpy (&var_insts[0], &s7_inst, sizeof (struct s7_score_it));
      s7_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);

      /* Insn 3  */
      fix_num = 1;
      var_num = 1;
      /* Fix part
	 For an external symbol: ori r1, LO%<constant>  */
      sprintf (tmp, "ori r1, %d", lo);
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
	return;

      memcpy (&fix_insts[0], &s7_inst, sizeof (struct s7_score_it));

      /* Var part
  	 For a local symbol: addi r1, <sym>+LO%<constant>    (BFD_RELOC_GOT_LO16)  */
      sprintf (tmp, "addi_u_pic r1, %s + %d", S_GET_NAME (add_symbol), lo);
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
	return;

      memcpy (&var_insts[0], &s7_inst, sizeof (struct s7_score_it));
      s7_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);

      /* Insn 4: add rD, rD, r1  */
      sprintf (tmp, "add r%d, r%d, r1", reg_rd, reg_rd);
      if (s7_append_insn (tmp, true) == (int) s7_FAIL)
	return;

     /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
     s7_inst.bwarn = -1;
    }

  s7_nor1 = r1_bak;
}

/* Handle la.  */

static void
s7_do_macro_la_rdi32 (char *str)
{
  int reg_rd;

  s7_skip_whitespace (str);
  if ((reg_rd = s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE)) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      char append_str[s7_MAX_LITERAL_POOL_SIZE];
      char *keep_data = str;

      /* Check immediate value.  */
      if (s7_my_get_expression (&s7_inst.reloc.exp, &str) == (int) s7_FAIL)
        {
          s7_inst.error = _("expression error");
          return;
        }
      else if ((s7_inst.reloc.exp.X_add_symbol == NULL)
               && (s7_validate_immediate (s7_inst.reloc.exp.X_add_number, _IMM32, 0) == (int) s7_FAIL))
        {
          s7_inst.error = _("value not in range [0, 0xffffffff]");
          return;
        }

      /* Reset str.  */
      str = keep_data;

      /* la rd, simm16.  */
      if (s7_data_op2 (&str, 1, _SIMM16_LA) != (int) s7_FAIL)
        {
          s7_end_of_line (str);
          return;
        }
      /* la rd, imm32 or la rd, label.  */
      else
        {
          s7_SET_INSN_ERROR (NULL);
          str = keep_data;
          if ((s7_data_op2 (&str, 1, _VALUE_HI16) == (int) s7_FAIL)
              || (s7_end_of_line (str) == (int) s7_FAIL))
            {
              return;
            }
          else
            {
              if ((s7_score_pic == s7_NO_PIC) || (!s7_inst.reloc.exp.X_add_symbol))
                {
                  sprintf (append_str, "ld_i32hi r%d, %s", reg_rd, keep_data);
                  if (s7_append_insn (append_str, true) == (int) s7_FAIL)
		    return;

                  sprintf (append_str, "ld_i32lo r%d, %s", reg_rd, keep_data);
                  if (s7_append_insn (append_str, true) == (int) s7_FAIL)
		    return;
		}
	      else
		{
		  gas_assert (s7_inst.reloc.exp.X_add_symbol);
		  s7_build_la_pic (reg_rd, s7_inst.reloc.exp);
		}

              /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
              s7_inst.bwarn = -1;
            }
        }
    }
}

/* Handle li.  */

static void
s7_do_macro_li_rdi32 (char *str)
{
  int reg_rd;

  s7_skip_whitespace (str);
  if ((reg_rd = s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE)) == (int) s7_FAIL
      || s7_skip_past_comma (&str) == (int) s7_FAIL)
    {
      return;
    }
  else
    {
      char *keep_data = str;

      /* Check immediate value.  */
      if (s7_my_get_expression (&s7_inst.reloc.exp, &str) == (int) s7_FAIL)
        {
          s7_inst.error = _("expression error");
          return;
        }
      else if (!(s7_inst.reloc.exp.X_add_number >= -0xffffffffLL
                 && s7_inst.reloc.exp.X_add_number <= 0xffffffffLL))
        {
          s7_inst.error = _("value not in range [-0xffffffff, 0xffffffff]");
          return;
        }

      /* Reset str.  */
      str = keep_data;

      /* li rd, simm16.  */
      if (s7_data_op2 (&str, 1, _SIMM16_LA) != (int) s7_FAIL)
        {
          s7_end_of_line (str);
          return;
        }
      /* li rd, imm32.  */
      else
        {
          char append_str[s7_MAX_LITERAL_POOL_SIZE];

          str = keep_data;

          if ((s7_data_op2 (&str, 1, _VALUE_HI16) == (int) s7_FAIL)
              || (s7_end_of_line (str) == (int) s7_FAIL))
            {
              return;
            }
          else if (s7_inst.reloc.exp.X_add_symbol)
            {
              s7_inst.error = _("li rd label isn't correct instruction form");
              return;
            }
          else
            {
              sprintf (append_str, "ld_i32hi r%d, %s", reg_rd, keep_data);

              if (s7_append_insn (append_str, true) == (int) s7_FAIL)
		return;
              else
                {
                  sprintf (append_str, "ld_i32lo r%d, %s", reg_rd, keep_data);
                  if (s7_append_insn (append_str, true) == (int) s7_FAIL)
		    return;

                  /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
                  s7_inst.bwarn = -1;
                }
            }
        }
    }
}

/* Handle mul/mulu/div/divu/rem/remu.  */

static void
s7_do_macro_mul_rdrsrs (char *str)
{
  int reg_rd;
  int reg_rs1;
  int reg_rs2;
  char *backupstr;
  char append_str[s7_MAX_LITERAL_POOL_SIZE];

  if (s7_university_version == 1)
    as_warn ("%s", s7_ERR_FOR_SCORE5U_MUL_DIV);

  strcpy (append_str, str);
  backupstr = append_str;
  s7_skip_whitespace (backupstr);
  if (((reg_rd = s7_reg_required_here (&backupstr, -1, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
      || (s7_skip_past_comma (&backupstr) == (int) s7_FAIL)
      || ((reg_rs1 = s7_reg_required_here (&backupstr, -1, s7_REG_TYPE_SCORE)) == (int) s7_FAIL))
    {
      s7_inst.error = s7_BAD_ARGS;
      return;
    }

  if (s7_skip_past_comma (&backupstr) == (int) s7_FAIL)
    {
      /* rem/remu rA, rB is error format.  */
      if (strcmp (s7_inst.name, "rem") == 0 || strcmp (s7_inst.name, "remu") == 0)
        {
          s7_SET_INSN_ERROR (s7_BAD_ARGS);
        }
      else
        {
          s7_SET_INSN_ERROR (NULL);
          s7_do_rsrs (str);
        }
      return;
    }
  else
    {
      s7_SET_INSN_ERROR (NULL);
      if (((reg_rs2 = s7_reg_required_here (&backupstr, -1, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
          || (s7_end_of_line (backupstr) == (int) s7_FAIL))
        {
          return;
        }
      else
        {
          char append_str1[s7_MAX_LITERAL_POOL_SIZE];

          if (strcmp (s7_inst.name, "rem") == 0)
            {
              sprintf (append_str, "mul r%d, r%d", reg_rs1, reg_rs2);
              sprintf (append_str1, "mfceh  r%d", reg_rd);
            }
          else if (strcmp (s7_inst.name, "remu") == 0)
            {
              sprintf (append_str, "mulu r%d, r%d", reg_rs1, reg_rs2);
              sprintf (append_str1, "mfceh  r%d", reg_rd);
            }
          else
            {
              sprintf (append_str, "%s r%d, r%d", s7_inst.name, reg_rs1, reg_rs2);
              sprintf (append_str1, "mfcel  r%d", reg_rd);
            }

          /* Output mul/mulu or div/divu or rem/remu.  */
          if (s7_append_insn (append_str, true) == (int) s7_FAIL)
	    return;

          /* Output mfcel or mfceh.  */
          if (s7_append_insn (append_str1, true) == (int) s7_FAIL)
	    return;

          /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
          s7_inst.bwarn = -1;
        }
    }
}

static void
s7_exp_macro_ldst_abs (char *str)
{
  int reg_rd;
  char *backupstr, *tmp;
  char append_str[s7_MAX_LITERAL_POOL_SIZE];
  char verifystr[s7_MAX_LITERAL_POOL_SIZE];
  struct s7_score_it inst_backup;
  int r1_bak = 0;

  r1_bak = s7_nor1;
  s7_nor1 = 0;
  memcpy (&inst_backup, &s7_inst, sizeof (struct s7_score_it));

  strcpy (verifystr, str);
  backupstr = verifystr;
  s7_skip_whitespace (backupstr);
  if ((reg_rd = s7_reg_required_here (&backupstr, -1, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
    return;

  tmp = backupstr;
  if (s7_skip_past_comma (&backupstr) == (int) s7_FAIL)
    return;

  backupstr = tmp;
  sprintf (append_str, "li r1  %s", backupstr);
  s7_append_insn (append_str, true);

  memcpy (&s7_inst, &inst_backup, sizeof (struct s7_score_it));
  sprintf (append_str, " r%d, [r1,0]", reg_rd);
  s7_do_ldst_insn (append_str);

  s7_nor1 = r1_bak;
}

static int
s7_nopic_need_relax (symbolS * sym, int before_relaxing)
{
  if (sym == NULL)
    return 0;
  else if (s7_USE_GLOBAL_POINTER_OPT && s7_g_switch_value > 0)
    {
      const char *symname;
      const char *segname;

      /* Find out whether this symbol can be referenced off the $gp
         register.  It can be if it is smaller than the -G size or if
         it is in the .sdata or .sbss section.  Certain symbols can
         not be referenced off the $gp, although it appears as though
         they can.  */
      symname = S_GET_NAME (sym);
      if (symname != NULL
          && (strcmp (symname, "eprol") == 0
              || strcmp (symname, "etext") == 0
              || strcmp (symname, "_gp") == 0
              || strcmp (symname, "edata") == 0
              || strcmp (symname, "_fbss") == 0
              || strcmp (symname, "_fdata") == 0
              || strcmp (symname, "_ftext") == 0
              || strcmp (symname, "end") == 0
              || strcmp (symname, GP_DISP_LABEL) == 0))
        {
          return 1;
        }
      else if ((!S_IS_DEFINED (sym) || S_IS_COMMON (sym)) && (0
      /* We must defer this decision until after the whole file has been read,
         since there might be a .extern after the first use of this symbol.  */
               || (before_relaxing
                   && S_GET_VALUE (sym) == 0)
               || (S_GET_VALUE (sym) != 0
                   && S_GET_VALUE (sym) <= s7_g_switch_value)))
        {
          return 0;
        }

      segname = segment_name (S_GET_SEGMENT (sym));
      return (strcmp (segname, ".sdata") != 0
	      && strcmp (segname, ".sbss") != 0
	      && !startswith (segname, ".sdata.")
	      && !startswith (segname, ".gnu.linkonce.s."));
    }
  /* We are not optimizing for the $gp register.  */
  else
    return 1;
}

/* Build a relax frag for lw/st instruction when generating s7_PIC,
   external symbol first and local symbol second.  */

static void
s7_build_lwst_pic (int reg_rd, expressionS exp, const char *insn_name)
{
  symbolS *add_symbol = exp.X_add_symbol;
  int add_number = exp.X_add_number;
  struct s7_score_it fix_insts[s7_RELAX_INST_NUM];
  struct s7_score_it var_insts[s7_RELAX_INST_NUM];
  int fix_num = 0;
  int var_num = 0;
  char tmp[s7_MAX_LITERAL_POOL_SIZE];
  int r1_bak;

  r1_bak = s7_nor1;
  s7_nor1 = 0;

  if ((add_number == 0) || (add_number >= -0x8000 && add_number <= 0x7fff))
    {
      fix_num = 1;
      var_num = 2;

      /* For an external symbol, two insns are generated;
         For a local symbol, three insns are generated.  */
      /* Fix part
         For an external symbol: lw rD, <sym>($gp)
                                 (BFD_RELOC_SCORE_GOT15)  */
      sprintf (tmp, "lw_pic r1, %s", S_GET_NAME (add_symbol));
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
        return;

      memcpy (&fix_insts[0], &s7_inst, sizeof (struct s7_score_it));

      /* Var part
	 For a local symbol :
         lw rD, <sym>($gp)    (BFD_RELOC_SCORE_GOT15)
	 addi rD, <sym>       (BFD_RELOC_GOT_LO16) */
      s7_inst.reloc.type = BFD_RELOC_SCORE_GOT15;
      memcpy (&var_insts[0], &s7_inst, sizeof (struct s7_score_it));
      sprintf (tmp, "addi_s_pic r1, %s", S_GET_NAME (add_symbol));
      if (s7_append_insn (tmp, false) == (int) s7_FAIL)
        return;

      memcpy (&var_insts[1], &s7_inst, sizeof (struct s7_score_it));
      s7_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);

      /* Insn 2 or Insn 3: lw/st rD, [r1, constant]  */
      sprintf (tmp, "%s r%d, [r1, %d]", insn_name, reg_rd, add_number);
      if (s7_append_insn (tmp, true) == (int) s7_FAIL)
        return;

      /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
      s7_inst.bwarn = -1;
    }
  else
    {
      s7_inst.error = _("PIC code offset overflow (max 16 signed bits)");
      return;
    }

  s7_nor1 = r1_bak;
}

static void
s7_do_macro_ldst_label (char *str)
{
  int i;
  int ldst_gp_p = 0;
  int reg_rd;
  int r1_bak;
  char *backup_str;
  char *label_str;
  char *absolute_value;
  char append_str[3][s7_MAX_LITERAL_POOL_SIZE];
  char verifystr[s7_MAX_LITERAL_POOL_SIZE];
  struct s7_score_it inst_backup;
  struct s7_score_it inst_expand[3];
  struct s7_score_it inst_main;

  memcpy (&inst_backup, &s7_inst, sizeof (struct s7_score_it));
  strcpy (verifystr, str);
  backup_str = verifystr;

  s7_skip_whitespace (backup_str);
  if ((reg_rd = s7_reg_required_here (&backup_str, -1, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
    return;

  if (s7_skip_past_comma (&backup_str) == (int) s7_FAIL)
    return;

  label_str = backup_str;

  /* Ld/st rD, [rA, imm]      ld/st rD, [rA]+, imm      ld/st rD, [rA, imm]+.  */
  if (*backup_str == '[')
    {
      s7_inst.type = Rd_rvalueRs_preSI12;
      s7_do_ldst_insn (str);
      return;
    }

  /* Ld/st rD, imm.  */
  absolute_value = backup_str;
  s7_inst.type = Rd_rvalueRs_SI15;

  if (s7_my_get_expression (&s7_inst.reloc.exp, &backup_str) == (int) s7_FAIL)
    {
      s7_inst.error = _("expression error");
      return;
    }
  else if ((s7_inst.reloc.exp.X_add_symbol == NULL)
           && (s7_validate_immediate (s7_inst.reloc.exp.X_add_number, _VALUE, 0) == (int) s7_FAIL))
    {
      s7_inst.error = _("value not in range [0, 0x7fffffff]");
      return;
    }
  else if (s7_end_of_line (backup_str) == (int) s7_FAIL)
    {
      s7_inst.error = _("end on line error");
      return;
    }
  else
    {
      if (s7_inst.reloc.exp.X_add_symbol == 0)
        {
          memcpy (&s7_inst, &inst_backup, sizeof (struct s7_score_it));
          s7_exp_macro_ldst_abs (str);
          return;
        }
    }

  /* Ld/st rD, label.  */
  s7_inst.type = Rd_rvalueRs_SI15;
  backup_str = absolute_value;
  if ((s7_data_op2 (&backup_str, 1, _GP_IMM15) == (int) s7_FAIL)
      || (s7_end_of_line (backup_str) == (int) s7_FAIL))
    {
      return;
    }
  else
    {
      if (s7_inst.reloc.exp.X_add_symbol == 0)
        {
          if (!s7_inst.error)
	    s7_inst.error = s7_BAD_ARGS;

          return;
        }

      if (s7_score_pic == s7_PIC)
        {
          int ldst_idx = 0;
          ldst_idx = s7_inst.instruction & OPC_PSEUDOLDST_MASK;
          s7_build_lwst_pic (reg_rd, s7_inst.reloc.exp,
                             s7_score_ldst_insns[ldst_idx * 3 + 0].template_name);
          return;
        }
      else
	{
          if ((s7_inst.reloc.exp.X_add_number <= 0x3fff)
               && (s7_inst.reloc.exp.X_add_number >= -0x4000)
               && (!s7_nopic_need_relax (s7_inst.reloc.exp.X_add_symbol, 1)))
	    {
              int ldst_idx = 0;

              /* Assign the real opcode.  */
              ldst_idx = s7_inst.instruction & OPC_PSEUDOLDST_MASK;
              s7_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
              s7_inst.instruction |= s7_score_ldst_insns[ldst_idx * 3 + 0].value;
              s7_inst.instruction |= reg_rd << 20;
              s7_inst.instruction |= s7_GP << 15;
              s7_inst.relax_inst = 0x8000;
              s7_inst.relax_size = 0;
              ldst_gp_p = 1;
	    }
	}
    }

  /* Backup s7_inst.  */
  memcpy (&inst_main, &s7_inst, sizeof (struct s7_score_it));
  r1_bak = s7_nor1;
  s7_nor1 = 0;

  /* Determine which instructions should be output.  */
  sprintf (append_str[0], "ld_i32hi r1, %s", label_str);
  sprintf (append_str[1], "ld_i32lo r1, %s", label_str);
  sprintf (append_str[2], "%s r%d, [r1, 0]", inst_backup.name, reg_rd);

  /* Generate three instructions.
     la r1, label
     ld/st rd, [r1, 0]  */
  for (i = 0; i < 3; i++)
    {
      if (s7_append_insn (append_str[i], false) == (int) s7_FAIL)
	return;

      memcpy (&inst_expand[i], &s7_inst, sizeof (struct s7_score_it));
    }

  if (ldst_gp_p)
    {
      char *p;

      /* Adjust instruction opcode and to be relaxed instruction opcode.  */
      inst_main.instruction = s7_adjust_paritybit (inst_main.instruction, s7_GET_INSN_CLASS (inst_main.type));
      inst_main.relax_size = inst_expand[0].size + inst_expand[1].size + inst_expand[2].size;
      inst_main.type = Insn_GP;

      for (i = 0; i < 3; i++)
	inst_expand[i].instruction = s7_adjust_paritybit (inst_expand[i].instruction
						       , s7_GET_INSN_CLASS (inst_expand[i].type));

      /* Check data dependency.  */
      s7_handle_dependency (&inst_main);

      /* Start a new frag if frag_now is not empty.  */
      if (frag_now_fix () != 0)
        {
          if (!frag_now->tc_frag_data.is_insn)
	    frag_wane (frag_now);

          frag_new (0);
        }
      frag_grow (20);

      /* Write fr_fix part.  */
      p = frag_more (inst_main.size);
      s7_number_to_chars (p, inst_main.instruction, inst_main.size);

      if (inst_main.reloc.type != BFD_RELOC_NONE)
        {
          s7_fix_new_score (frag_now, p - frag_now->fr_literal, inst_main.size,
                         &inst_main.reloc.exp, inst_main.reloc.pc_rel, inst_main.reloc.type);
        }

#ifdef OBJ_ELF
      dwarf2_emit_insn (inst_main.size);
#endif

      /* s7_GP instruction can not do optimization, only can do relax between
         1 instruction and 3 instructions.  */
      p = frag_var (rs_machine_dependent, inst_main.relax_size + s7_RELAX_PAD_BYTE, 0,
                    s7_RELAX_ENCODE (inst_main.size, inst_main.relax_size, inst_main.type, 0, 4, 0),
                    inst_main.reloc.exp.X_add_symbol, 0, NULL);

      /* Write fr_var part.
         no calling s7_gen_insn_frag, no fixS will be generated.  */
      s7_number_to_chars (p, inst_expand[0].instruction, inst_expand[0].size);
      p += inst_expand[0].size;
      s7_number_to_chars (p, inst_expand[1].instruction, inst_expand[1].size);
      p += inst_expand[1].size;
      s7_number_to_chars (p, inst_expand[2].instruction, inst_expand[2].size);
    }
  else
    {
      s7_gen_insn_frag (&inst_expand[0], NULL);
      s7_gen_insn_frag (&inst_expand[1], NULL);
      s7_gen_insn_frag (&inst_expand[2], NULL);
    }
  s7_nor1 = r1_bak;

  /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
  s7_inst.bwarn = -1;
}

static void
s7_do_lw_pic (char *str)
{
  int reg_rd;

  s7_skip_whitespace (str);
  if (((reg_rd = s7_reg_required_here (&str, 20, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
      || (s7_skip_past_comma (&str) == (int) s7_FAIL)
      || (s7_my_get_expression (&s7_inst.reloc.exp, &str) == (int) s7_FAIL)
      || (s7_end_of_line (str) == (int) s7_FAIL))
    {
      return;
    }
  else
    {
      if (s7_inst.reloc.exp.X_add_symbol == 0)
        {
          if (!s7_inst.error)
	    s7_inst.error = s7_BAD_ARGS;

          return;
        }

      s7_inst.instruction |= s7_GP << 15;
      s7_inst.reloc.type = BFD_RELOC_SCORE_GOT15;
    }
}

static void
s7_do_empty (char *str)
{
  str = str;
  if (s7_university_version == 1)
    {
      if (((s7_inst.instruction & 0x3e0003ff) == 0x0c000004)
          || ((s7_inst.instruction & 0x3e0003ff) == 0x0c000024)
          || ((s7_inst.instruction & 0x3e0003ff) == 0x0c000044)
          || ((s7_inst.instruction & 0x3e0003ff) == 0x0c000064))
        {
          s7_inst.error = s7_ERR_FOR_SCORE5U_MMU;
          return;
        }
    }
  if (s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if (s7_inst.relax_inst != 0x8000)
    {
      if (s7_inst.type == NO_OPD)
        {
          s7_inst.relax_size = 2;
        }
      else
        {
          s7_inst.relax_size = 4;
        }
    }
}

static void
s7_do_jump (char *str)
{
  char *save_in;

  s7_skip_whitespace (str);
  if (s7_my_get_expression (&s7_inst.reloc.exp, &str) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    return;

  if (s7_inst.reloc.exp.X_add_symbol == 0)
    {
      s7_inst.error = _("lacking label  ");
      return;
    }

  if (!(s7_inst.reloc.exp.X_add_number >= -16777216
      && s7_inst.reloc.exp.X_add_number <= 16777215))
    {
      s7_inst.error = _("invalid constant: 25 bit expression not in range [-16777216, 16777215]");
      return;
    }

  save_in = input_line_pointer;
  input_line_pointer = str;
  s7_inst.reloc.type = BFD_RELOC_SCORE_JMP;
  s7_inst.reloc.pc_rel = 1;
  input_line_pointer = save_in;
}

static void
s7_do16_jump (char *str)
{
  s7_skip_whitespace (str);
  if (s7_my_get_expression (&s7_inst.reloc.exp, &str) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else if (s7_inst.reloc.exp.X_add_symbol == 0)
    {
      s7_inst.error = _("lacking label  ");
      return;
    }
  else if (!(s7_inst.reloc.exp.X_add_number >= 0
           && s7_inst.reloc.exp.X_add_number <= 4095))
    {
      s7_inst.error = _("invalid constant: 12 bit expression not in range [0, 4095]");
      return;
    }

  s7_inst.reloc.type = BFD_RELOC_SCORE16_JMP;
  s7_inst.reloc.pc_rel = 1;
}

static void
s7_do_branch (char *str)
{
  unsigned long abs_value = 0;

  if (s7_my_get_expression (&s7_inst.reloc.exp, &str) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL)
    {
      return;
    }
  else if (s7_inst.reloc.exp.X_add_symbol == 0)
    {
      s7_inst.error = _("lacking label  ");
      return;
    }
  else if (!(s7_inst.reloc.exp.X_add_number >= -524288
           && s7_inst.reloc.exp.X_add_number <= 524287))
    {
      s7_inst.error = _("invalid constant: 20 bit expression not in range -2^19..2^19");
      return;
    }

  s7_inst.reloc.type = BFD_RELOC_SCORE_BRANCH;
  s7_inst.reloc.pc_rel = 1;

  /* Branch 32  offset field : 20 bit, 16 bit branch offset field : 8 bit.  */
  s7_inst.instruction |= (s7_inst.reloc.exp.X_add_number & 0x3fe) | ((s7_inst.reloc.exp.X_add_number & 0xffc00) << 5);

  /* Compute 16 bit branch instruction.  */
  if ((s7_inst.relax_inst != 0x8000) && (abs_value & 0xfffffe00) == 0)
    {
      s7_inst.relax_inst |= (((s7_inst.instruction >> 10) & 0xf) << 8);
      s7_inst.relax_inst |= ((s7_inst.reloc.exp.X_add_number >> 1) & 0xff);
      s7_inst.relax_size = 2;
    }
  else
    {
      s7_inst.relax_inst = 0x8000;
    }
}

static void
s7_do16_branch (char *str)
{
  if ((s7_my_get_expression (&s7_inst.reloc.exp, &str) == (int) s7_FAIL
      || s7_end_of_line (str) == (int) s7_FAIL))
    {
      ;
    }
  else if (s7_inst.reloc.exp.X_add_symbol == 0)
    {
      s7_inst.error = _("lacking label");
    }
  else if (!(s7_inst.reloc.exp.X_add_number >= -512
           && s7_inst.reloc.exp.X_add_number <= 511))
    {
      s7_inst.error = _("invalid constant: 10 bit expression not in range [-2^9, 2^9-1]");
    }
  else
    {
      s7_inst.reloc.type = BFD_RELOC_SCORE16_BRANCH;
      s7_inst.reloc.pc_rel = 1;
      s7_inst.instruction |= ((s7_inst.reloc.exp.X_add_number >> 1) & 0xff);
    }
}

/* Iterate over the base tables to create the instruction patterns.  */

static void
s7_build_score_ops_hsh (void)
{
  unsigned int i;

  for (i = 0; i < sizeof (s7_score_insns) / sizeof (struct s7_asm_opcode); i++)
    {
      const struct s7_asm_opcode *insn = s7_score_insns + i;
      size_t len = strlen (insn->template_name) + 1;
      struct s7_asm_opcode *new_opcode;
      char *template_name;

      new_opcode = notes_alloc (sizeof (*new_opcode));
      template_name = notes_memdup (insn->template_name, len, len);

      new_opcode->template_name = template_name;
      new_opcode->parms = insn->parms;
      new_opcode->value = insn->value;
      new_opcode->relax_value = insn->relax_value;
      new_opcode->type = insn->type;
      new_opcode->bitmask = insn->bitmask;
      str_hash_insert (s7_score_ops_hsh, new_opcode->template_name,
		       new_opcode, 0);
    }
}

static void
s7_build_dependency_insn_hsh (void)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (s7_insn_to_dependency_table); i++)
    {
      const struct s7_insn_to_dependency *tmp = s7_insn_to_dependency_table + i;
      size_t len = strlen (tmp->insn_name) + 1;
      struct s7_insn_to_dependency *new_i2d;
      char *insn_name;

      new_i2d = notes_alloc (sizeof (*new_i2d));
      insn_name = notes_memdup (tmp->insn_name, len, len);

      new_i2d->insn_name = insn_name;
      new_i2d->type = tmp->type;
      str_hash_insert (s7_dependency_insn_hsh, new_i2d->insn_name, new_i2d, 0);
    }
}

static valueT
s7_md_chars_to_number (char *buf, int n)
{
  valueT result = 0;
  unsigned char *where = (unsigned char *) buf;

  if (target_big_endian)
    {
      while (n--)
        {
          result <<= 8;
          result |= (*where++ & 255);
        }
    }
  else
    {
      while (n--)
        {
          result <<= 8;
          result |= (where[n] & 255);
        }
    }

  return result;
}

/* Return true if the given symbol should be considered local for s7_PIC.  */

static bool
s7_pic_need_relax (symbolS *sym, asection *segtype)
{
  asection *symsec;
  bool linkonce;

  /* Handle the case of a symbol equated to another symbol.  */
  while (symbol_equated_reloc_p (sym))
    {
      symbolS *n;

      /* It's possible to get a loop here in a badly written
	 program.  */
      n = symbol_get_value_expression (sym)->X_add_symbol;
      if (n == sym)
	break;
      sym = n;
    }

  symsec = S_GET_SEGMENT (sym);

  /* Duplicate the test for LINK_ONCE sections as in adjust_reloc_syms */
  linkonce = false;
  if (symsec != segtype && ! S_IS_LOCAL (sym))
    {
      if ((bfd_section_flags (symsec) & SEC_LINK_ONCE) != 0)
	linkonce = true;

      /* The GNU toolchain uses an extension for ELF: a section
	  beginning with the magic string .gnu.linkonce is a linkonce
	  section.  */
      if (startswith (segment_name (symsec), ".gnu.linkonce"))
	linkonce = true;
    }

  /* This must duplicate the test in adjust_reloc_syms.  */
  return (!bfd_is_und_section (symsec)
	  && !bfd_is_abs_section (symsec)
	  && !bfd_is_com_section (symsec)
	  && !linkonce
#ifdef OBJ_ELF
	  /* A global or weak symbol is treated as external.  */
	  && (OUTPUT_FLAVOR != bfd_target_elf_flavour
	      || (! S_IS_WEAK (sym) && ! S_IS_EXTERNAL (sym)))
#endif
	  );
}

static int
s7_judge_size_before_relax (fragS * fragp, asection *sec)
{
  int change = 0;

  if (s7_score_pic == s7_NO_PIC)
    change = s7_nopic_need_relax (fragp->fr_symbol, 0);
  else
    change = s7_pic_need_relax (fragp->fr_symbol, sec);

  if (change == 1)
    {
      /* Only at the first time determining whether s7_GP instruction relax should be done,
         return the difference between instruction size and instruction relax size.  */
      if (fragp->fr_opcode == NULL)
	{
	  fragp->fr_fix = s7_RELAX_NEW (fragp->fr_subtype);
	  fragp->fr_opcode = fragp->fr_literal + s7_RELAX_RELOC1 (fragp->fr_subtype);
          return s7_RELAX_NEW (fragp->fr_subtype) - s7_RELAX_OLD (fragp->fr_subtype);
	}
    }

  return 0;
}

static int
s7_b32_relax_to_b16 (fragS * fragp)
{
  int grows = 0;
  int relaxable_p = 0;
  int frag_addr = fragp->fr_address + fragp->insn_addr;

  addressT symbol_address = 0;
  symbolS *s;
  offsetT offset;
  unsigned long value;
  unsigned long abs_value;

  /* FIXME : here may be able to modify better .
     I don't know how to get the fragp's section ,
     so in relax stage , it may be wrong to calculate the symbol's offset when the frag's section
     is different from the symbol's.  */

  relaxable_p = s7_RELAX_OPT (fragp->fr_subtype);

  s = fragp->fr_symbol;
  /* b/bl immediate  */
  if (s == NULL)
    frag_addr = 0;
  else
    symbol_address = (addressT) symbol_get_frag (s)->fr_address;

  value = s7_md_chars_to_number (fragp->fr_literal, s7_INSN_SIZE);

  /* b 32's offset : 20 bit, b 16's tolerate field : 0xff.  */
  offset = ((value & 0x3ff0000) >> 6) | (value & 0x3fe);
  if ((offset & 0x80000) == 0x80000)
    offset |= 0xfff00000;

  abs_value = offset + symbol_address - frag_addr;
  if ((abs_value & 0x80000000) == 0x80000000)
    abs_value = 0xffffffff - abs_value + 1;

  /* Relax branch 32 to branch 16.  */
  if (relaxable_p && ((abs_value & 0xffffff00) == 0)
      && (S_IS_DEFINED (s) && !S_IS_COMMON (s) && !S_IS_EXTERNAL (s)))
    {
      /* do nothing.  */
    }
  else
    {
      /* Branch 32 can not be relaxed to b 16, so clear OPT bit.  */
      fragp->fr_opcode = NULL;
      fragp->fr_subtype = s7_RELAX_OPT_CLEAR (fragp->fr_subtype);
    }

  return grows;
}

static void
s7_parse_pce_inst (char *insnstr)
{
  char c;
  char *p;
  char *q;
  char first[s7_MAX_LITERAL_POOL_SIZE];
  char second[s7_MAX_LITERAL_POOL_SIZE];
  struct s7_score_it pec_part_1;

  /* Get first part string of PCE.  */
  p = strstr (insnstr, "||");
  c = *p;
  *p = '\0';
  strcpy (first, insnstr);

  /* Get second part string of PCE.  */
  *p = c;
  p += 2;
  strcpy (second, p);

  s7_parse_16_32_inst (first, false);
  if (s7_inst.error)
    return;

  memcpy (&pec_part_1, &s7_inst, sizeof (s7_inst));

  q = second;
  while (q && *q)
    {
      *q = TOLOWER (*q);
      q++;
    }

  s7_parse_16_32_inst (second, false);
  if (s7_inst.error)
    return;

  if (   ((pec_part_1.size == s7_INSN_SIZE) && (s7_inst.size == s7_INSN_SIZE))
      || ((pec_part_1.size == s7_INSN_SIZE) && (s7_inst.size == s7_INSN16_SIZE))
      || ((pec_part_1.size == s7_INSN16_SIZE) && (s7_inst.size == s7_INSN_SIZE)))
    {
      s7_inst.error = _("pce instruction error (16 bit || 16 bit)'");
      strcpy (s7_inst.str, insnstr);
      return;
    }

  if (!s7_inst.error)
    s7_gen_insn_frag (&pec_part_1, &s7_inst);
}



static void
s7_insert_reg (const struct s7_reg_entry *r, htab_t htab)
{
  int i = 0;
  int len = strlen (r->name) + 2;
  char *buf = XNEWVEC (char, len);
  char *buf2 = XNEWVEC (char, len);

  strcpy (buf + i, r->name);
  for (i = 0; buf[i]; i++)
    {
      buf2[i] = TOUPPER (buf[i]);
    }
  buf2[i] = '\0';

  str_hash_insert (htab, buf, r, 0);
  str_hash_insert (htab, buf2, r, 0);
}

static void
s7_build_reg_hsh (struct s7_reg_map *map)
{
  const struct s7_reg_entry *r;

  map->htab = str_htab_create ();
  for (r = map->names; r->name != NULL; r++)
    s7_insert_reg (r, map->htab);
}



/* If we change section we must dump the literal pool first.  */
static void
s7_s_score_bss (int ignore ATTRIBUTE_UNUSED)
{
  subseg_set (bss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

static void
s7_s_score_text (int ignore)
{
  obj_elf_text (ignore);
  record_alignment (now_seg, 2);
}

static void
s7_s_section (int ignore)
{
  obj_elf_section (ignore);
  if ((bfd_section_flags (now_seg) & SEC_CODE) != 0)
    record_alignment (now_seg, 2);

}

static void
s7_s_change_sec (int sec)
{
  segT seg;

#ifdef OBJ_ELF
  /* The ELF backend needs to know that we are changing sections, so
     that .previous works correctly.  We could do something like check
     for an obj_section_change_hook macro, but that might be confusing
     as it would not be appropriate to use it in the section changing
     functions in read.c, since obj-elf.c intercepts those.  FIXME:
     This should be cleaner, somehow.  */
  obj_elf_section_change_hook ();
#endif
  switch (sec)
    {
    case 'r':
      seg = subseg_new (s7_RDATA_SECTION_NAME, (subsegT) get_absolute_expression ());
      bfd_set_section_flags (seg, (SEC_ALLOC | SEC_LOAD | SEC_READONLY
				   | SEC_RELOC | SEC_DATA));
      if (strcmp (TARGET_OS, "elf") != 0)
        record_alignment (seg, 4);
      demand_empty_rest_of_line ();
      break;
    case 's':
      seg = subseg_new (".sdata", (subsegT) get_absolute_expression ());
      bfd_set_section_flags (seg, (SEC_ALLOC | SEC_LOAD | SEC_RELOC
				   | SEC_DATA | SEC_SMALL_DATA));
      if (strcmp (TARGET_OS, "elf") != 0)
        record_alignment (seg, 4);
      demand_empty_rest_of_line ();
      break;
    }
}

static void
s7_s_score_mask (int reg_type  ATTRIBUTE_UNUSED)
{
  long mask, off;

  if (s7_cur_proc_ptr == NULL)
    {
      as_warn (_(".mask outside of .ent"));
      demand_empty_rest_of_line ();
      return;
    }
  if (get_absolute_expression_and_terminator (&mask) != ',')
    {
      as_warn (_("Bad .mask directive"));
      --input_line_pointer;
      demand_empty_rest_of_line ();
      return;
    }
  off = get_absolute_expression ();
  s7_cur_proc_ptr->reg_mask = mask;
  s7_cur_proc_ptr->reg_offset = off;
  demand_empty_rest_of_line ();
}

static symbolS *
s7_get_symbol (void)
{
  int c;
  char *name;
  symbolS *p;

  c = get_symbol_name (&name);
  p = (symbolS *) symbol_find_or_make (name);
  (void) restore_line_pointer (c);
  return p;
}

static long
s7_get_number (void)
{
  int negative = 0;
  long val = 0;

  if (*input_line_pointer == '-')
    {
      ++input_line_pointer;
      negative = 1;
    }
  if (!ISDIGIT (*input_line_pointer))
    as_bad (_("expected simple number"));
  if (input_line_pointer[0] == '0')
    {
      if (input_line_pointer[1] == 'x')
        {
          input_line_pointer += 2;
          while (ISXDIGIT (*input_line_pointer))
            {
              val <<= 4;
              val |= hex_value (*input_line_pointer++);
            }
          return negative ? -val : val;
        }
      else
        {
          ++input_line_pointer;
          while (ISDIGIT (*input_line_pointer))
            {
              val <<= 3;
              val |= *input_line_pointer++ - '0';
            }
          return negative ? -val : val;
        }
    }
  if (!ISDIGIT (*input_line_pointer))
    {
      printf (_(" *input_line_pointer == '%c' 0x%02x\n"), *input_line_pointer, *input_line_pointer);
      as_warn (_("invalid number"));
      return -1;
    }
  while (ISDIGIT (*input_line_pointer))
    {
      val *= 10;
      val += *input_line_pointer++ - '0';
    }
  return negative ? -val : val;
}

/* The .aent and .ent directives.  */

static void
s7_s_score_ent (int aent)
{
  symbolS *symbolP;
  int maybe_text;

  symbolP = s7_get_symbol ();
  if (*input_line_pointer == ',')
    ++input_line_pointer;
  SKIP_WHITESPACE ();
  if (ISDIGIT (*input_line_pointer) || *input_line_pointer == '-')
    s7_get_number ();

  if ((bfd_section_flags (now_seg) & SEC_CODE) != 0)
    maybe_text = 1;
  else
    maybe_text = 0;
  if (!maybe_text)
    as_warn (_(".ent or .aent not in text section."));
  if (!aent && s7_cur_proc_ptr)
    as_warn (_("missing .end"));
  if (!aent)
    {
      s7_cur_proc_ptr = &s7_cur_proc;
      s7_cur_proc_ptr->reg_mask = 0xdeadbeaf;
      s7_cur_proc_ptr->reg_offset = 0xdeadbeaf;
      s7_cur_proc_ptr->fpreg_mask = 0xdeafbeaf;
      s7_cur_proc_ptr->leaf = 0xdeafbeaf;
      s7_cur_proc_ptr->frame_offset = 0xdeafbeaf;
      s7_cur_proc_ptr->frame_reg = 0xdeafbeaf;
      s7_cur_proc_ptr->pc_reg = 0xdeafbeaf;
      s7_cur_proc_ptr->isym = symbolP;
      symbol_get_bfdsym (symbolP)->flags |= BSF_FUNCTION;
      ++s7_numprocs;
      if (debug_type == DEBUG_STABS)
        stabs_generate_asm_func (S_GET_NAME (symbolP), S_GET_NAME (symbolP));
    }
  demand_empty_rest_of_line ();
}

static void
s7_s_score_frame (int ignore ATTRIBUTE_UNUSED)
{
  char *backupstr;
  char str[30];
  long val;
  int i = 0;

  backupstr = input_line_pointer;

#ifdef OBJ_ELF
  if (s7_cur_proc_ptr == NULL)
    {
      as_warn (_(".frame outside of .ent"));
      demand_empty_rest_of_line ();
      return;
    }
  s7_cur_proc_ptr->frame_reg = s7_reg_required_here ((&backupstr), 0, s7_REG_TYPE_SCORE);
  SKIP_WHITESPACE ();
  s7_skip_past_comma (&backupstr);
  while (*backupstr != ',')
    {
      str[i] = *backupstr;
      i++;
      backupstr++;
    }
  str[i] = '\0';
  val = atoi (str);

  SKIP_WHITESPACE ();
  s7_skip_past_comma (&backupstr);
  s7_cur_proc_ptr->frame_offset = val;
  s7_cur_proc_ptr->pc_reg = s7_reg_required_here ((&backupstr), 0, s7_REG_TYPE_SCORE);

  SKIP_WHITESPACE ();
  s7_skip_past_comma (&backupstr);
  i = 0;
  while (*backupstr != '\n')
    {
      str[i] = *backupstr;
      i++;
      backupstr++;
    }
  str[i] = '\0';
  val = atoi (str);
  s7_cur_proc_ptr->leaf = val;
  SKIP_WHITESPACE ();
  s7_skip_past_comma (&backupstr);

#endif /* OBJ_ELF */
  while (input_line_pointer != backupstr)
    input_line_pointer++;
}

/* The .end directive.  */

static void
s7_s_score_end (int x ATTRIBUTE_UNUSED)
{
  symbolS *p;
  int maybe_text;

  /* Generate a .pdr section.  */
  segT saved_seg = now_seg;
  subsegT saved_subseg = now_subseg;
  expressionS exp;
  char *fragp;

  if (!is_end_of_line[(unsigned char)*input_line_pointer])
    {
      p = s7_get_symbol ();
      demand_empty_rest_of_line ();
    }
  else
    p = NULL;

  if ((bfd_section_flags (now_seg) & SEC_CODE) != 0)
    maybe_text = 1;
  else
    maybe_text = 0;

  if (!maybe_text)
    as_warn (_(".end not in text section"));
  if (!s7_cur_proc_ptr)
    {
      as_warn (_(".end directive without a preceding .ent directive."));
      demand_empty_rest_of_line ();
      return;
    }
  if (p != NULL)
    {
      gas_assert (S_GET_NAME (p));
      if (strcmp (S_GET_NAME (p), S_GET_NAME (s7_cur_proc_ptr->isym)))
        as_warn (_(".end symbol does not match .ent symbol."));
      if (debug_type == DEBUG_STABS)
        stabs_generate_asm_endfunc (S_GET_NAME (p), S_GET_NAME (p));
    }
  else
    as_warn (_(".end directive missing or unknown symbol"));

  if ((s7_cur_proc_ptr->reg_mask == 0xdeadbeaf) ||
      (s7_cur_proc_ptr->reg_offset == 0xdeadbeaf) ||
      (s7_cur_proc_ptr->leaf == 0xdeafbeaf) ||
      (s7_cur_proc_ptr->frame_offset == 0xdeafbeaf) ||
      (s7_cur_proc_ptr->frame_reg == 0xdeafbeaf) || (s7_cur_proc_ptr->pc_reg == 0xdeafbeaf));

  else
    {
      (void) frag_now_fix ();
      gas_assert (s7_pdr_seg);
      subseg_set (s7_pdr_seg, 0);
      /* Write the symbol.  */
      exp.X_op = O_symbol;
      exp.X_add_symbol = p;
      exp.X_add_number = 0;
      emit_expr (&exp, 4);
      fragp = frag_more (7 * 4);
      s7_number_to_chars (fragp, (valueT) s7_cur_proc_ptr->reg_mask, 4);
      s7_number_to_chars (fragp + 4, (valueT) s7_cur_proc_ptr->reg_offset, 4);
      s7_number_to_chars (fragp + 8, (valueT) s7_cur_proc_ptr->fpreg_mask, 4);
      s7_number_to_chars (fragp + 12, (valueT) s7_cur_proc_ptr->leaf, 4);
      s7_number_to_chars (fragp + 16, (valueT) s7_cur_proc_ptr->frame_offset, 4);
      s7_number_to_chars (fragp + 20, (valueT) s7_cur_proc_ptr->frame_reg, 4);
      s7_number_to_chars (fragp + 24, (valueT) s7_cur_proc_ptr->pc_reg, 4);
      subseg_set (saved_seg, saved_subseg);

    }
  s7_cur_proc_ptr = NULL;
}

/* Handle the .set pseudo-op.  */

static void
s7_s_score_set (int x ATTRIBUTE_UNUSED)
{
  int i = 0;
  char name[s7_MAX_LITERAL_POOL_SIZE];
  char * orig_ilp = input_line_pointer;

  while (!is_end_of_line[(unsigned char)*input_line_pointer])
    {
      name[i] = (char) * input_line_pointer;
      i++;
      ++input_line_pointer;
    }

  name[i] = '\0';

  if (strcmp (name, "nwarn") == 0)
    {
      s7_warn_fix_data_dependency = 0;
    }
  else if (strcmp (name, "fixdd") == 0)
    {
      s7_fix_data_dependency = 1;
    }
  else if (strcmp (name, "nofixdd") == 0)
    {
      s7_fix_data_dependency = 0;
    }
  else if (strcmp (name, "r1") == 0)
    {
      s7_nor1 = 0;
    }
  else if (strcmp (name, "nor1") == 0)
    {
      s7_nor1 = 1;
    }
  else if (strcmp (name, "optimize") == 0)
    {
      s7_g_opt = 1;
    }
  else if (strcmp (name, "volatile") == 0)
    {
      s7_g_opt = 0;
    }
  else if (strcmp (name, "pic") == 0)
    {
      s7_score_pic = s7_PIC;
    }
  else
    {
      input_line_pointer = orig_ilp;
      s_set (0);
    }
}

/* Handle the .cpload pseudo-op.  This is used when generating s7_PIC code.  It sets the
   $gp register for the function based on the function address, which is in the register
   named in the argument. This uses a relocation against GP_DISP_LABEL, which is handled
   specially by the linker.  The result is:
   ldis gp, %hi(GP_DISP_LABEL)
   ori  gp, %low(GP_DISP_LABEL)
   add  gp, gp, .cpload argument
   The .cpload argument is normally r29.  */

static void
s7_s_score_cpload (int ignore ATTRIBUTE_UNUSED)
{
  int reg;
  char insn_str[s7_MAX_LITERAL_POOL_SIZE];

  /* If we are not generating s7_PIC code, .cpload is ignored.  */
  if (s7_score_pic == s7_NO_PIC)
    {
      s_ignore (0);
      return;
    }

  if ((reg = s7_reg_required_here (&input_line_pointer, -1, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
    return;

  demand_empty_rest_of_line ();

  sprintf (insn_str, "ld_i32hi r%d, %s", s7_GP, GP_DISP_LABEL);
  if (s7_append_insn (insn_str, true) == (int) s7_FAIL)
    return;

  sprintf (insn_str, "ld_i32lo r%d, %s", s7_GP, GP_DISP_LABEL);
  if (s7_append_insn (insn_str, true) == (int) s7_FAIL)
    return;

  sprintf (insn_str, "add r%d, r%d, r%d", s7_GP, s7_GP, reg);
  if (s7_append_insn (insn_str, true) == (int) s7_FAIL)
    return;
}

/* Handle the .cprestore pseudo-op.  This stores $gp into a given
   offset from $sp.  The offset is remembered, and after making a s7_PIC
   call $gp is restored from that location.  */

static void
s7_s_score_cprestore (int ignore ATTRIBUTE_UNUSED)
{
  int reg;
  int cprestore_offset;
  char insn_str[s7_MAX_LITERAL_POOL_SIZE];

  /* If we are not generating s7_PIC code, .cprestore is ignored.  */
  if (s7_score_pic == s7_NO_PIC)
    {
      s_ignore (0);
      return;
    }

  if ((reg = s7_reg_required_here (&input_line_pointer, -1, s7_REG_TYPE_SCORE)) == (int) s7_FAIL
      || s7_skip_past_comma (&input_line_pointer) == (int) s7_FAIL)
    {
      return;
    }

  cprestore_offset = get_absolute_expression ();

  if (cprestore_offset <= 0x3fff)
    {
      sprintf (insn_str, "sw r%d, [r%d, %d]", s7_GP, reg, cprestore_offset);
      if (s7_append_insn (insn_str, true) == (int) s7_FAIL)
        return;
    }
  else
    {
      int r1_bak;

      r1_bak = s7_nor1;
      s7_nor1 = 0;

      sprintf (insn_str, "li r1, %d", cprestore_offset);
      if (s7_append_insn (insn_str, true) == (int) s7_FAIL)
        return;

      sprintf (insn_str, "add r1, r1, r%d", reg);
      if (s7_append_insn (insn_str, true) == (int) s7_FAIL)
        return;

      sprintf (insn_str, "sw r%d, [r1]", s7_GP);
      if (s7_append_insn (insn_str, true) == (int) s7_FAIL)
        return;

      s7_nor1 = r1_bak;
    }

  demand_empty_rest_of_line ();
}

/* Handle the .gpword pseudo-op.  This is used when generating s7_PIC
   code.  It generates a 32 bit s7_GP relative reloc.  */

static void
s7_s_score_gpword (int ignore ATTRIBUTE_UNUSED)
{
  expressionS ex;
  char *p;

  /* When not generating s7_PIC code, this is treated as .word.  */
  if (s7_score_pic == s7_NO_PIC)
    {
      cons (4);
      return;
    }
  expression (&ex);
  if (ex.X_op != O_symbol || ex.X_add_number != 0)
    {
      as_bad (_("Unsupported use of .gpword"));
      ignore_rest_of_line ();
    }
  p = frag_more (4);
  s7_number_to_chars (p, (valueT) 0, 4);
  fix_new_exp (frag_now, p - frag_now->fr_literal, 4, &ex, false, BFD_RELOC_GPREL32);
  demand_empty_rest_of_line ();
}

/* Handle the .cpadd pseudo-op.  This is used when dealing with switch
   tables in s7_PIC code.  */

static void
s7_s_score_cpadd (int ignore ATTRIBUTE_UNUSED)
{
  int reg;
  char insn_str[s7_MAX_LITERAL_POOL_SIZE];

  /* If we are not generating s7_PIC code, .cpload is ignored.  */
  if (s7_score_pic == s7_NO_PIC)
    {
      s_ignore (0);
      return;
    }

  if ((reg = s7_reg_required_here (&input_line_pointer, -1, s7_REG_TYPE_SCORE)) == (int) s7_FAIL)
    {
      return;
    }
  demand_empty_rest_of_line ();

  /* Add $gp to the register named as an argument.  */
  sprintf (insn_str, "add r%d, r%d, r%d", reg, reg, s7_GP);
  if (s7_append_insn (insn_str, true) == (int) s7_FAIL)
    return;
}

#ifndef TC_IMPLICIT_LCOMM_ALIGNMENT
#define TC_IMPLICIT_LCOMM_ALIGNMENT(SIZE, P2VAR)        	\
    do								\
    {                                                   	\
    if ((SIZE) >= 8)                                      	\
    (P2VAR) = 3;                                        	\
    else if ((SIZE) >= 4)                                 	\
    (P2VAR) = 2;                                        	\
    else if ((SIZE) >= 2)                                 	\
    (P2VAR) = 1;                                        	\
    else                                                  	\
    (P2VAR) = 0;                                        	\
    }								\
  while (0)
#endif

static void
s7_s_score_lcomm (int bytes_p)
{
  char *name;
  char c;
  char *p;
  int temp;
  symbolS *symbolP;
  segT current_seg = now_seg;
  subsegT current_subseg = now_subseg;
  const int max_alignment = 15;
  int align = 0;
  segT bss_seg = bss_section;
  int needs_align = 0;

  c = get_symbol_name (&name);
  p = input_line_pointer;
  *p = c;

  if (name == p)
    {
      as_bad (_("expected symbol name"));
      discard_rest_of_line ();
      return;
    }

  SKIP_WHITESPACE_AFTER_NAME ();

  /* Accept an optional comma after the name.  The comma used to be
     required, but Irix 5 cc does not generate it.  */
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      SKIP_WHITESPACE ();
    }

  if (is_end_of_line[(unsigned char)*input_line_pointer])
    {
      as_bad (_("missing size expression"));
      return;
    }

  if ((temp = get_absolute_expression ()) < 0)
    {
      as_warn (_("BSS length (%d) < 0 ignored"), temp);
      ignore_rest_of_line ();
      return;
    }

#if defined (TC_SCORE)
  if (OUTPUT_FLAVOR == bfd_target_ecoff_flavour || OUTPUT_FLAVOR == bfd_target_elf_flavour)
    {
      /* For Score and Alpha ECOFF or ELF, small objects are put in .sbss.  */
      if ((unsigned) temp <= bfd_get_gp_size (stdoutput))
	{
	  bss_seg = subseg_new (".sbss", 1);
	  seg_info (bss_seg)->bss = 1;
	  if (!bfd_set_section_flags (bss_seg, SEC_ALLOC | SEC_SMALL_DATA))
	    as_warn (_("error setting flags for \".sbss\": %s"),
		     bfd_errmsg (bfd_get_error ()));
	}
    }
#endif

  SKIP_WHITESPACE ();
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      SKIP_WHITESPACE ();

      if (is_end_of_line[(unsigned char)*input_line_pointer])
        {
          as_bad (_("missing alignment"));
          return;
        }
      else
        {
          align = get_absolute_expression ();
          needs_align = 1;
        }
    }

  if (!needs_align)
    {
      TC_IMPLICIT_LCOMM_ALIGNMENT (temp, align);

      /* Still zero unless TC_IMPLICIT_LCOMM_ALIGNMENT set it.  */
      if (align)
        record_alignment (bss_seg, align);
    }

  if (needs_align)
    {
      if (bytes_p)
        {
          /* Convert to a power of 2.  */
          if (align != 0)
            {
              unsigned int i;

              for (i = 0; align != 0; align >>= 1, ++i)
                ;
              align = i - 1;
            }
        }

      if (align > max_alignment)
        {
          align = max_alignment;
          as_warn (_("alignment too large; %d assumed"), align);
        }
      else if (align < 0)
        {
          align = 0;
          as_warn (_("alignment negative; 0 assumed"));
        }

      record_alignment (bss_seg, align);
    }
  else
    {
      /* Assume some objects may require alignment on some systems.  */
#if defined (TC_ALPHA) && ! defined (VMS)
      if (temp > 1)
        {
          align = ffs (temp) - 1;
          if (temp % (1 << align))
            abort ();
        }
#endif
    }

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (
#if (defined (OBJ_AOUT) || defined (OBJ_MAYBE_AOUT))
       (OUTPUT_FLAVOR != bfd_target_aout_flavour
        || (S_GET_OTHER (symbolP) == 0 && S_GET_DESC (symbolP) == 0)) &&
#endif
       (S_GET_SEGMENT (symbolP) == bss_seg || (!S_IS_DEFINED (symbolP) && S_GET_VALUE (symbolP) == 0)))
    {
      char *pfrag;

      subseg_set (bss_seg, 1);

      if (align)
        frag_align (align, 0, 0);

      /* Detach from old frag.  */
      if (S_GET_SEGMENT (symbolP) == bss_seg)
        symbol_get_frag (symbolP)->fr_symbol = NULL;

      symbol_set_frag (symbolP, frag_now);
      pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP, (offsetT) temp, NULL);
      *pfrag = 0;


      S_SET_SEGMENT (symbolP, bss_seg);

#ifdef OBJ_COFF
      /* The symbol may already have been created with a preceding
         ".globl" directive -- be careful not to step on storage class
         in that case.  Otherwise, set it to static.  */
      if (S_GET_STORAGE_CLASS (symbolP) != C_EXT)
        {
          S_SET_STORAGE_CLASS (symbolP, C_STAT);
        }
#endif /* OBJ_COFF */

#ifdef S_SET_SIZE
      S_SET_SIZE (symbolP, temp);
#endif
    }
  else
    as_bad (_("symbol `%s' is already defined"), S_GET_NAME (symbolP));

  subseg_set (current_seg, current_subseg);

  demand_empty_rest_of_line ();
}



static void
s7_begin (void)
{
  unsigned int i;
  segT seg;
  subsegT subseg;

  s7_score_ops_hsh = str_htab_create ();

  s7_build_score_ops_hsh ();

  s7_dependency_insn_hsh = str_htab_create ();

  s7_build_dependency_insn_hsh ();

  for (i = (int) REG_TYPE_FIRST; i < (int) s7_REG_TYPE_MAX; i++)
    s7_build_reg_hsh (s7_all_reg_maps + i);

  /* Initialize dependency vector.  */
  s7_init_dependency_vector ();

  bfd_set_arch_mach (stdoutput, TARGET_ARCH, 0);
  seg = now_seg;
  subseg = now_subseg;
  s7_pdr_seg = subseg_new (".pdr", (subsegT) 0);
  bfd_set_section_flags (s7_pdr_seg, SEC_READONLY | SEC_RELOC | SEC_DEBUGGING);
  bfd_set_section_alignment (s7_pdr_seg, 2);
  subseg_set (seg, subseg);

  if (s7_USE_GLOBAL_POINTER_OPT)
    bfd_set_gp_size (stdoutput, s7_g_switch_value);
}

static void
s7_assemble (char *str)
{
  know (str);
  know (strlen (str) < s7_MAX_LITERAL_POOL_SIZE);

  memset (&s7_inst, '\0', sizeof (s7_inst));
  if (s7_INSN_IS_PCE_P (str))
    s7_parse_pce_inst (str);
  else
    s7_parse_16_32_inst (str, true);

  if (s7_inst.error)
    as_bad (_("%s -- `%s'"), s7_inst.error, s7_inst.str);
}

/* We handle all bad expressions here, so that we can report the faulty
   instruction in the error message.  */

static void
s7_operand (expressionS * exp)
{
  if (s7_in_my_get_expression)
    {
      exp->X_op = O_illegal;
      if (s7_inst.error == NULL)
        {
          s7_inst.error = _("bad expression");
        }
    }
}

/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.

   Note that fp constants aren't represent in the normal way on the ARM.
   In big endian mode, things are as expected.  However, in little endian
   mode fp constants are big-endian word-wise, and little-endian byte-wise
   within the words.  For example, (double) 1.1 in big endian mode is
   the byte sequence 3f f1 99 99 99 99 99 9a, and in little endian mode is
   the byte sequence 99 99 f1 3f 9a 99 99 99.  */

static const char *
s7_atof (int type, char *litP, int *sizeP)
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  char *t;
  int i;

  switch (type)
    {
    case 'f':
    case 'F':
    case 's':
    case 'S':
      prec = 2;
      break;
    case 'd':
    case 'D':
    case 'r':
    case 'R':
      prec = 4;
      break;
    case 'x':
    case 'X':
    case 'p':
    case 'P':
      prec = 6;
      break;
    default:
      *sizeP = 0;
      return _("bad call to MD_ATOF()");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  *sizeP = prec * 2;

  if (target_big_endian)
    {
      for (i = 0; i < prec; i++)
        {
          s7_number_to_chars (litP, (valueT) words[i], 2);
          litP += 2;
        }
    }
  else
    {
      for (i = 0; i < prec; i += 2)
        {
          s7_number_to_chars (litP, (valueT) words[i + 1], 2);
          s7_number_to_chars (litP + 2, (valueT) words[i], 2);
          litP += 4;
        }
    }

  return 0;
}

/* Implementation of md_frag_check.
   Called after md_convert_frag().  */

static void
s7_frag_check (fragS * fragp ATTRIBUTE_UNUSED)
{
  know (fragp->insn_addr <= s7_RELAX_PAD_BYTE);
}

/* Implementation of TC_VALIDATE_FIX.
   Called before md_apply_fix() and after md_convert_frag().  */

static void
s7_validate_fix (fixS *fixP)
{
  fixP->fx_where += fixP->fx_frag->insn_addr;
}

static int
s7_force_relocation (struct fix *fixp)
{
  int retval = 0;

  if (fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY
      || fixp->fx_r_type == BFD_RELOC_SCORE_JMP
      || fixp->fx_r_type == BFD_RELOC_SCORE_BRANCH
      || fixp->fx_r_type == BFD_RELOC_SCORE16_JMP
      || fixp->fx_r_type == BFD_RELOC_SCORE16_BRANCH)
    {
      retval = 1;
    }

  return retval;
}

static bool
s7_fix_adjustable (fixS * fixP)
{
  if (fixP->fx_addsy == NULL)
    {
      return 1;
    }
  else if (OUTPUT_FLAVOR == bfd_target_elf_flavour
      && (S_IS_EXTERNAL (fixP->fx_addsy) || S_IS_WEAK (fixP->fx_addsy)))
    {
      return 0;
    }
  else if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
           || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY
           || fixP->fx_r_type == BFD_RELOC_SCORE_JMP
           || fixP->fx_r_type == BFD_RELOC_SCORE16_JMP)
    {
      return 0;
    }

  return 1;
}

static void
s7_elf_final_processing (void)
{
  unsigned long val = E_SCORE_MACH_SCORE7;

  elf_elfheader (stdoutput)->e_machine = EM_SCORE;
  elf_elfheader (stdoutput)->e_flags &= ~EF_SCORE_MACH;
  elf_elfheader (stdoutput)->e_flags |= val;

  if (s7_fix_data_dependency == 1)
    {
      elf_elfheader (stdoutput)->e_flags |= EF_SCORE_FIXDEP;
    }
  if (s7_score_pic == s7_PIC)
    {
      elf_elfheader (stdoutput)->e_flags |= EF_SCORE_PIC;
    }
}

/* In this function, we determine whether s7_GP instruction should do relaxation,
   for the label being against was known now.
   Doing this here but not in md_relax_frag() can induce iteration times
   in stage of doing relax.  */

static int
s7_estimate_size_before_relax (fragS * fragp, asection * sec)
{
  if ((s7_RELAX_TYPE (fragp->fr_subtype) == Insn_GP)
      || (s7_RELAX_TYPE (fragp->fr_subtype) == Insn_PIC))
    return s7_judge_size_before_relax (fragp, sec);

  return 0;
}

static int
s7_relax_frag (asection * sec ATTRIBUTE_UNUSED,
	       fragS * fragp,
	       long stretch ATTRIBUTE_UNUSED)
{
  int grows = 0;
  int insn_size;
  int do_relax_p = 0;           /* Indicate doing relaxation for this frag.  */
  int relaxable_p = 0;
  bool word_align_p = false;
  fragS *next_fragp;

  /* If the instruction address is odd, make it half word align first.  */
  if ((fragp->fr_address) % 2 != 0)
    {
      if ((fragp->fr_address + fragp->insn_addr) % 2 != 0)
	{
          fragp->insn_addr = 1;
          grows += 1;
	}
    }

  word_align_p = (fragp->fr_address + fragp->insn_addr) % 4 == 0;

  /* Get instruction size and relax size after the last relaxation.  */
  if (fragp->fr_opcode)
    insn_size = s7_RELAX_NEW (fragp->fr_subtype);
  else
    insn_size = s7_RELAX_OLD (fragp->fr_subtype);

  /* Handle specially for s7_GP instruction.  for, s7_judge_size_before_relax() has already determine
     whether the s7_GP instruction should do relax.  */
  if ((s7_RELAX_TYPE (fragp->fr_subtype) == Insn_GP)
      || (s7_RELAX_TYPE (fragp->fr_subtype) == Insn_PIC))
    {
      if (!word_align_p)
        {
          if (fragp->insn_addr < 2)
            {
              fragp->insn_addr += 2;
              grows += 2;
            }
          else
            {
              fragp->insn_addr -= 2;
              grows -= 2;
            }
        }

      if (fragp->fr_opcode)
	fragp->fr_fix = s7_RELAX_NEW (fragp->fr_subtype) + fragp->insn_addr;
      else
	fragp->fr_fix = s7_RELAX_OLD (fragp->fr_subtype) + fragp->insn_addr;
    }
  else
    {
      if (s7_RELAX_TYPE (fragp->fr_subtype) == PC_DISP19div2)
	s7_b32_relax_to_b16 (fragp);

      relaxable_p = s7_RELAX_OPT (fragp->fr_subtype);
      next_fragp = fragp->fr_next;
      while ((next_fragp) && (next_fragp->fr_type != rs_machine_dependent))
	{
          next_fragp = next_fragp->fr_next;
	}

      if (next_fragp)
        {
          int n_insn_size;
          int n_relaxable_p = 0;

          if (next_fragp->fr_opcode)
            {
              n_insn_size = s7_RELAX_NEW (next_fragp->fr_subtype);
            }
          else
            {
              n_insn_size = s7_RELAX_OLD (next_fragp->fr_subtype);
            }

          if (s7_RELAX_TYPE (next_fragp->fr_subtype) == PC_DISP19div2)
            s7_b32_relax_to_b16 (next_fragp);
          n_relaxable_p = s7_RELAX_OPT (next_fragp->fr_subtype);

          if (word_align_p)
            {
              if (insn_size == 4)
                {
                  /* 32 -> 16.  */
                  if (relaxable_p && ((n_insn_size == 2) || n_relaxable_p))
                    {
                      grows -= 2;
                      do_relax_p = 1;
                    }
                }
              else if (insn_size == 2)
                {
                  /* 16 -> 32.  */
                  if (relaxable_p && (((n_insn_size == 4) && !n_relaxable_p) || (n_insn_size > 4)))
                    {
                      grows += 2;
                      do_relax_p = 1;
                    }
                }
              else
                {
		  abort ();
                }
            }
          else
            {
              if (insn_size == 4)
                {
                  /* 32 -> 16.  */
                  if (relaxable_p)
                    {
                      grows -= 2;
                      do_relax_p = 1;
                    }
                  /* Make the 32 bit instruction word align.  */
                  else
                    {
                      fragp->insn_addr += 2;
                      grows += 2;
		    }
                }
              else if (insn_size == 2)
                {
                  /* Do nothing.  */
                }
              else
                {
		  abort ();
                }
            }
        }
      else
        {
	  /* Here, try best to do relax regardless fragp->fr_next->fr_type.  */
          if (!word_align_p)
            {
              if (insn_size % 4 == 0)
                {
                  /* 32 -> 16.  */
                  if (relaxable_p)
                    {
                      grows -= 2;
                      do_relax_p = 1;
                    }
                  else
                    {
                      fragp->insn_addr += 2;
                      grows += 2;
                    }
                }
            }
          else
            {
	      /* Do nothing.  */
            }
        }

      /* fragp->fr_opcode indicates whether this frag should be relaxed.  */
      if (do_relax_p)
        {
          if (fragp->fr_opcode)
            {
              fragp->fr_opcode = NULL;
	      /* Guarantee estimate stage is correct.  */
              fragp->fr_fix = s7_RELAX_OLD (fragp->fr_subtype);
              fragp->fr_fix += fragp->insn_addr;
            }
          else
            {
              fragp->fr_opcode = fragp->fr_literal + s7_RELAX_RELOC1 (fragp->fr_subtype);
	      /* Guarantee estimate stage is correct.  */
              fragp->fr_fix = s7_RELAX_NEW (fragp->fr_subtype);
              fragp->fr_fix += fragp->insn_addr;
            }
        }
      else
	{
          if (fragp->fr_opcode)
            {
	      /* Guarantee estimate stage is correct.  */
              fragp->fr_fix = s7_RELAX_NEW (fragp->fr_subtype);
              fragp->fr_fix += fragp->insn_addr;
            }
          else
            {
	      /* Guarantee estimate stage is correct.  */
              fragp->fr_fix = s7_RELAX_OLD (fragp->fr_subtype);
              fragp->fr_fix += fragp->insn_addr;
            }
	}
    }

  return grows;
}

static void
s7_convert_frag (bfd * abfd ATTRIBUTE_UNUSED,
		 segT sec ATTRIBUTE_UNUSED,
		 fragS * fragp)
{
  unsigned int r_old;
  unsigned int r_new;
  char backup[20];
  fixS *fixp;

  r_old = s7_RELAX_OLD (fragp->fr_subtype);
  r_new = s7_RELAX_NEW (fragp->fr_subtype);

  /* fragp->fr_opcode indicates whether this frag should be relaxed.  */
  if (fragp->fr_opcode == NULL)
    {
      memcpy (backup, fragp->fr_literal, r_old);
      fragp->fr_fix = r_old;
    }
  else
    {
      memcpy (backup, fragp->fr_literal + r_old, r_new);
      fragp->fr_fix = r_new;
    }

  fixp = fragp->tc_frag_data.fixp;
  while (fixp && fixp->fx_frag == fragp && fixp->fx_where < r_old)
    {
      if (fragp->fr_opcode)
	fixp->fx_done = 1;
      fixp = fixp->fx_next;
    }
  while (fixp && fixp->fx_frag == fragp)
    {
      if (fragp->fr_opcode)
	fixp->fx_where -= r_old + fragp->insn_addr;
      else
	fixp->fx_done = 1;
      fixp = fixp->fx_next;
    }

  if (fragp->insn_addr)
    {
      s7_number_to_chars (fragp->fr_literal, 0x0, fragp->insn_addr);
    }
  memcpy (fragp->fr_literal + fragp->insn_addr, backup, fragp->fr_fix);
  fragp->fr_fix += fragp->insn_addr;
}

static long
s7_pcrel_from (fixS * fixP)
{
  long retval = 0;

  if (fixP->fx_addsy
      && (S_GET_SEGMENT (fixP->fx_addsy) == undefined_section)
      && (fixP->fx_subsy == NULL))
    {
      retval = 0;
    }
  else
    {
      retval = fixP->fx_where + fixP->fx_frag->fr_address;
    }

  return retval;
}

/* Round up a section size to the appropriate boundary.  */
static valueT
s7_section_align (segT segment, valueT size)
{
  int align = bfd_section_alignment (segment);

  return ((size + (1 << align) - 1) & -(1 << align));
}

static void
s7_apply_fix (fixS *fixP, valueT *valP, segT seg)
{
  valueT value = *valP;
  valueT abs_value = 0;
  valueT newval;
  valueT content;
  valueT HI, LO;

  char *buf = fixP->fx_frag->fr_literal + fixP->fx_where;

  gas_assert (fixP->fx_r_type < BFD_RELOC_UNUSED);
  if (fixP->fx_addsy == 0 && !fixP->fx_pcrel)
    {
      if (fixP->fx_r_type != BFD_RELOC_SCORE_DUMMY_HI16)
        fixP->fx_done = 1;
    }

  /* If this symbol is in a different section then we need to leave it for
     the linker to deal with.  Unfortunately, md_pcrel_from can't tell,
     so we have to undo it's effects here.  */
  if (fixP->fx_pcrel)
    {
      if (fixP->fx_addsy != NULL
	  && S_IS_DEFINED (fixP->fx_addsy)
	  && S_GET_SEGMENT (fixP->fx_addsy) != seg)
	value += md_pcrel_from (fixP);
    }

  /* Remember value for emit_reloc.  */
  fixP->fx_addnumber = value;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_HI16_S:
      if (fixP->fx_done)
        {                       /* For la rd, imm32.  */
          newval = s7_md_chars_to_number (buf, s7_INSN_SIZE);
          HI = value >> 16;   /* mul to 2, then take the hi 16 bit.  */
          newval |= (HI & 0x3fff) << 1;
          newval |= ((HI >> 14) & 0x3) << 16;
          s7_number_to_chars (buf, newval, s7_INSN_SIZE);
        }
      break;
    case BFD_RELOC_LO16:
      if (fixP->fx_done)        /* For la rd, imm32.  */
        {
          newval = s7_md_chars_to_number (buf, s7_INSN_SIZE);
          LO = value & 0xffff;
          newval |= (LO & 0x3fff) << 1; /* 16 bit: imm -> 14 bit in lo, 2 bit in hi.  */
          newval |= ((LO >> 14) & 0x3) << 16;
          s7_number_to_chars (buf, newval, s7_INSN_SIZE);
        }
      break;
    case BFD_RELOC_SCORE_JMP:
      {
        content = s7_md_chars_to_number (buf, s7_INSN_SIZE);
        value = fixP->fx_offset;
        if (value > 0x1ffffff)
          {
            as_bad_where (fixP->fx_file, fixP->fx_line,
                          _("j or jl truncate (0x%x)  [0 ~ 2^25-1]"), (unsigned int) value);
            return;
          }
        content = (content & ~0x3ff7ffe) | ((value << 1) & 0x3ff0000) | (value & 0x7fff);
        s7_number_to_chars (buf, content, s7_INSN_SIZE);
      }
      break;
    case BFD_RELOC_SCORE_BRANCH:
      if ((S_GET_SEGMENT (fixP->fx_addsy) != seg) || (fixP->fx_addsy != NULL && S_IS_EXTERNAL (fixP->fx_addsy)))
        value = fixP->fx_offset;
      else
        fixP->fx_done = 1;

      content = s7_md_chars_to_number (buf, s7_INSN_SIZE);
      if ((fixP->fx_frag->fr_opcode != 0) && ((content & 0x80008000) != 0x80008000))
        {
          if ((value & 0x80000000) == 0x80000000)
            abs_value = 0xffffffff - value + 1;
          if ((abs_value & 0xffffff00) != 0)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x) [-2^8 ~ 2^8]"), (unsigned int) value);
              return;
            }
          content = s7_md_chars_to_number (buf, s7_INSN16_SIZE);
          content &= 0xff00;
          content = (content & 0xff00) | ((value >> 1) & 0xff);
          s7_number_to_chars (buf, content, s7_INSN16_SIZE);
          fixP->fx_r_type = BFD_RELOC_SCORE16_BRANCH;
          fixP->fx_size = 2;
        }
      else
        {
          if ((value & 0x80000000) == 0x80000000)
            abs_value = 0xffffffff - value + 1;
          if ((abs_value & 0xfff80000) != 0)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x) [-2^19 ~ 2^19]"),
			    (unsigned int) value);
              return;
            }
          content = s7_md_chars_to_number (buf, s7_INSN_SIZE);
          content &= 0xfc00fc01;
          content = (content & 0xfc00fc01) | (value & 0x3fe) | ((value << 6) & 0x3ff0000);
          s7_number_to_chars (buf, content, s7_INSN_SIZE);
        }
      break;
    case BFD_RELOC_SCORE16_JMP:
      content = s7_md_chars_to_number (buf, s7_INSN16_SIZE);
      content &= 0xf001;
      value = fixP->fx_offset;
      if (value > 0xfff)
        {
          as_bad_where (fixP->fx_file, fixP->fx_line,
                        _("j! or jl! truncate (0x%x)  [0 ~ 2^12-1]"), (unsigned int) value);
          return;
        }
      value = fixP->fx_offset & 0xfff;
      content = (content & 0xfc01) | (value & 0xffe);
      s7_number_to_chars (buf, content, s7_INSN16_SIZE);
      break;
    case BFD_RELOC_SCORE16_BRANCH:
      content = s7_md_chars_to_number (buf, s7_INSN_SIZE);
      if ((fixP->fx_frag->fr_opcode != 0) && ((content & 0x80008000) == 0x80008000))
        {
          if ((S_GET_SEGMENT (fixP->fx_addsy) != seg) ||
              (fixP->fx_addsy != NULL && S_IS_EXTERNAL (fixP->fx_addsy)))
            value = fixP->fx_offset;
          else
            fixP->fx_done = 1;

          if ((value & 0xfff80000) != 0 && (value & 0xfff80000) != 0xfff80000)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x) [-2^19 ~ 2^19]"),
			    (unsigned int) value);
              return;
            }
          content = s7_md_chars_to_number (buf, s7_INSN_SIZE);
          content = (content & 0xfc00fc01) | (value & 0x3fe) | ((value << 6) & 0x3ff0000);
          s7_number_to_chars (buf, content, s7_INSN_SIZE);
          fixP->fx_r_type = BFD_RELOC_SCORE_BRANCH;
          fixP->fx_size = 4;
          break;
        }
      else
        {
          /* In different section.  */
          if ((S_GET_SEGMENT (fixP->fx_addsy) != seg) ||
              (fixP->fx_addsy != NULL && S_IS_EXTERNAL (fixP->fx_addsy)))
            value = fixP->fx_offset;
          else
            fixP->fx_done = 1;

          if ((value & 0xffffff00) != 0 && (value & 0xffffff00) != 0xffffff00)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x)  [-2^8 ~ 2^8]"),
			    (unsigned int) value);
              return;
            }
          content = s7_md_chars_to_number (buf, s7_INSN16_SIZE);
          content = (content & 0xff00) | ((value >> 1) & 0xff);
          s7_number_to_chars (buf, content, s7_INSN16_SIZE);
          break;
        }
    case BFD_RELOC_8:
      if (fixP->fx_done || fixP->fx_pcrel)
	s7_number_to_chars (buf, value, 1);
#ifdef OBJ_ELF
      else
        {
          value = fixP->fx_offset;
          s7_number_to_chars (buf, value, 1);
        }
#endif
      break;

    case BFD_RELOC_16:
      if (fixP->fx_done || fixP->fx_pcrel)
        s7_number_to_chars (buf, value, 2);
#ifdef OBJ_ELF
      else
        {
          value = fixP->fx_offset;
          s7_number_to_chars (buf, value, 2);
        }
#endif
      break;
    case BFD_RELOC_RVA:
    case BFD_RELOC_32:
      if (fixP->fx_done || fixP->fx_pcrel)
        s7_number_to_chars (buf, value, 4);
#ifdef OBJ_ELF
      else
        {
          value = fixP->fx_offset;
          s7_number_to_chars (buf, value, 4);
        }
#endif
      break;
    case BFD_RELOC_VTABLE_INHERIT:
      fixP->fx_done = 0;
      if (fixP->fx_addsy && !S_IS_DEFINED (fixP->fx_addsy) && !S_IS_WEAK (fixP->fx_addsy))
        S_SET_WEAK (fixP->fx_addsy);
      break;
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = 0;
      break;
    case BFD_RELOC_SCORE_GPREL15:
      content = s7_md_chars_to_number (buf, s7_INSN_SIZE);
      if ((fixP->fx_frag->fr_opcode != 0) && ((content & 0xfc1c8000) != 0x94188000))
        fixP->fx_r_type = BFD_RELOC_NONE;
      fixP->fx_done = 0;
      break;
    case BFD_RELOC_SCORE_GOT15:
    case BFD_RELOC_SCORE_DUMMY_HI16:
    case BFD_RELOC_SCORE_GOT_LO16:
    case BFD_RELOC_SCORE_CALL15:
    case BFD_RELOC_GPREL32:
      break;
    case BFD_RELOC_NONE:
    default:
      as_bad_where (fixP->fx_file, fixP->fx_line, _("bad relocation fixup type (%d)"), fixP->fx_r_type);
    }
}

/* Translate internal representation of relocation info to BFD target format.  */

static arelent **
s7_gen_reloc (asection * section ATTRIBUTE_UNUSED, fixS * fixp)
{
  static arelent *retval[MAX_RELOC_EXPANSION + 1];  /* MAX_RELOC_EXPANSION equals 2.  */
  arelent *reloc;
  bfd_reloc_code_real_type code;
  const char *type;

  reloc = retval[0] = XNEW (arelent);
  retval[1] = NULL;

  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->addend = fixp->fx_offset;

  /* If this is a variant frag, we may need to adjust the existing
     reloc and generate a new one.  */
  if (fixp->fx_frag->fr_opcode != NULL && (fixp->fx_r_type == BFD_RELOC_SCORE_GPREL15))
    {
      /* Update instruction imm bit.  */
      offsetT newval;
      unsigned short off;
      char *buf;

      buf = fixp->fx_frag->fr_literal + fixp->fx_frag->insn_addr;
      newval = s7_md_chars_to_number (buf, s7_INSN_SIZE);
      off = fixp->fx_offset >> 16;
      newval |= (off & 0x3fff) << 1;
      newval |= ((off >> 14) & 0x3) << 16;
      s7_number_to_chars (buf, newval, s7_INSN_SIZE);

      buf += s7_INSN_SIZE;
      newval = s7_md_chars_to_number (buf, s7_INSN_SIZE);
      off = fixp->fx_offset & 0xffff;
      newval |= ((off & 0x3fff) << 1);
      newval |= (((off >> 14) & 0x3) << 16);
      s7_number_to_chars (buf, newval, s7_INSN_SIZE);

      retval[1] = XNEW (arelent);
      retval[2] = NULL;
      retval[1]->sym_ptr_ptr = XNEW (asymbol *);
      *retval[1]->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
      retval[1]->address = (reloc->address + s7_RELAX_RELOC2 (fixp->fx_frag->fr_subtype));

      retval[1]->addend = 0;
      retval[1]->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_LO16);
      gas_assert (retval[1]->howto != NULL);

      fixp->fx_r_type = BFD_RELOC_HI16_S;
    }

  code = fixp->fx_r_type;
  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_32:
      if (fixp->fx_pcrel)
        {
          code = BFD_RELOC_32_PCREL;
          break;
        }
      /* Fall through.  */
    case BFD_RELOC_HI16_S:
    case BFD_RELOC_LO16:
    case BFD_RELOC_SCORE_JMP:
    case BFD_RELOC_SCORE_BRANCH:
    case BFD_RELOC_SCORE16_JMP:
    case BFD_RELOC_SCORE16_BRANCH:
    case BFD_RELOC_VTABLE_ENTRY:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_SCORE_GPREL15:
    case BFD_RELOC_SCORE_GOT15:
    case BFD_RELOC_SCORE_DUMMY_HI16:
    case BFD_RELOC_SCORE_GOT_LO16:
    case BFD_RELOC_SCORE_CALL15:
    case BFD_RELOC_GPREL32:
    case BFD_RELOC_NONE:
      code = fixp->fx_r_type;
      break;
    default:
      type = _("<unknown>");
      as_bad_where (fixp->fx_file, fixp->fx_line,
                    _("cannot represent %s relocation in this object file format"), type);
      return NULL;
    }

  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
                    _("cannot represent %s relocation in this object file format1"),
                    bfd_get_reloc_code_name (code));
      return NULL;
    }
  /* HACK: Since arm ELF uses Rel instead of Rela, encode the
     vtable entry to be used in the relocation's section offset.  */
  if (fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    reloc->address = fixp->fx_offset;

  return retval;
}
