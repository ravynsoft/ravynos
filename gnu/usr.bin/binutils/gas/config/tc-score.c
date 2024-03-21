/* tc-score.c -- Assembler for Score
   Copyright (C) 2006-2023 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "tc-score7.c"

static void s3_s_score_bss (int ignore ATTRIBUTE_UNUSED);
static void s3_s_score_text (int ignore);
static void s3_score_s_section (int ignore);
static void s3_s_change_sec (int sec);
static void s3_s_score_mask (int reg_type ATTRIBUTE_UNUSED);
static void s3_s_score_ent (int aent);
static void s3_s_score_frame (int ignore ATTRIBUTE_UNUSED);
static void s3_s_score_end (int x ATTRIBUTE_UNUSED);
static void s3_s_score_set (int x ATTRIBUTE_UNUSED);
static void s3_s_score_cpload (int ignore ATTRIBUTE_UNUSED);
static void s3_s_score_cprestore (int ignore ATTRIBUTE_UNUSED);
static void s3_s_score_gpword (int ignore ATTRIBUTE_UNUSED);
static void s3_s_score_cpadd (int ignore ATTRIBUTE_UNUSED);
static void s3_s_score_lcomm (int bytes_p);

static void s_score_bss (int ignore ATTRIBUTE_UNUSED);
static void s_score_text (int ignore);
static void s_section (int ignore);
static void s_change_sec (int sec);
static void s_score_mask (int reg_type ATTRIBUTE_UNUSED);
static void s_score_ent (int aent);
static void s_score_frame (int ignore ATTRIBUTE_UNUSED);
static void s_score_end (int x ATTRIBUTE_UNUSED);
static void s_score_set (int x ATTRIBUTE_UNUSED);
static void s_score_cpload (int ignore ATTRIBUTE_UNUSED);
static void s_score_cprestore (int ignore ATTRIBUTE_UNUSED);
static void s_score_gpword (int ignore ATTRIBUTE_UNUSED);
static void s_score_cpadd (int ignore ATTRIBUTE_UNUSED);
static void s_score_lcomm (int bytes_p);

/* s3: hooks.  */
static void s3_md_number_to_chars (char *buf, valueT val, int n);
static valueT s3_md_chars_to_number (char *buf, int n);
static void s3_assemble (char *str);
static void s3_operand (expressionS *);
static void s3_begin (void);
static void s3_number_to_chars (char *buf, valueT val, int n);
static const char *s3_atof (int type, char *litP, int *sizeP);
static void s3_frag_check (fragS * fragp ATTRIBUTE_UNUSED);
static void s3_validate_fix (fixS *fixP);
static int s3_force_relocation (struct fix *fixp);
static bool s3_fix_adjustable (fixS * fixP);
static void s3_elf_final_processing (void);
static int s3_estimate_size_before_relax (fragS * fragp, asection * sec ATTRIBUTE_UNUSED);
static int s3_relax_frag (asection * sec ATTRIBUTE_UNUSED, fragS * fragp, long stretch ATTRIBUTE_UNUSED);
static void s3_convert_frag (bfd * abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED, fragS * fragp);
static long s3_pcrel_from (fixS * fixP);
static valueT s3_section_align (segT segment ATTRIBUTE_UNUSED, valueT size);
static void s3_apply_fix (fixS *fixP, valueT *valP, segT seg);
static arelent **s3_gen_reloc (asection * section ATTRIBUTE_UNUSED, fixS * fixp);

/* s3: utils.  */
static void s3_do_ldst_insn (char *);
static void s3_do_crdcrscrsimm5 (char *);
static void s3_do_ldst_unalign (char *);
static void s3_do_ldst_atomic (char *);
static void s3_do_ldst_cop (char *);
static void s3_do_macro_li_rdi32 (char *);
static void s3_do_macro_la_rdi32 (char *);
static void s3_do_macro_rdi32hi (char *);
static void s3_do_macro_rdi32lo (char *);
static void s3_do_macro_mul_rdrsrs (char *);
static void s3_do_macro_bcmp (char *);
static void s3_do_macro_bcmpz (char *);
static void s3_do_macro_ldst_label (char *);
static void s3_do_branch (char *);
static void s3_do_jump (char *);
static void s3_do_empty (char *);
static void s3_do16_int (char *);
static void s3_do_rdrsrs (char *);
static void s3_do_rdsi16 (char *);
static void s3_do_rdrssi14 (char *);
static void s3_do_sub_rdsi16 (char *);
static void s3_do_sub_rdi16 (char *);
static void s3_do_sub_rdrssi14 (char *);
static void s3_do_rdrsi5 (char *);
static void s3_do_rdrsi14 (char *);
static void s3_do_rdi16 (char *);
static void s3_do_ldis (char *);
static void s3_do_xrsi5 (char *);
static void s3_do_rdrs (char *);
static void s3_do_rdxrs (char *);
static void s3_do_rsrs (char *);
static void s3_do_rdcrs (char *);
static void s3_do_rdsrs (char *);
static void s3_do_rd (char *);
static void s3_do16_dsp (char *);
static void s3_do16_dsp2 (char *);
static void s3_do_dsp (char *);
static void s3_do_dsp2 (char *);
static void s3_do_dsp3 (char *);
static void s3_do_rs (char *);
static void s3_do_i15 (char *);
static void s3_do_xi5x (char *);
static void s3_do_ceinst (char *);
static void s3_do_cache (char *);
static void s3_do16_rdrs2 (char *);
static void s3_do16_br (char *);
static void s3_do16_brr (char *);
static void s3_do_ltb (char *);
static void s3_do16_mv_cmp (char *);
static void s3_do16_addi (char *);
static void s3_do16_cmpi (char *);
static void s3_do16_rdi5 (char *);
static void s3_do16_xi5 (char *);
static void s3_do16_ldst_insn (char *);
static void s3_do16_slli_srli (char *);
static void s3_do16_ldiu (char *);
static void s3_do16_push_pop (char *);
static void s3_do16_rpush (char *);
static void s3_do16_rpop (char *);
static void s3_do16_branch (char *);
static void s3_do_lw48 (char *);
static void s3_do_sw48 (char *);
static void s3_do_ldi48 (char *);
static void s3_do_sdbbp48 (char *);
static void s3_do_and48 (char *);
static void s3_do_or48 (char *);
static void s3_do_mbitclr (char *);
static void s3_do_mbitset (char *);
static void s3_do_rdi16_pic (char *);
static void s3_do_addi_s_pic (char *);
static void s3_do_addi_u_pic (char *);
static void s3_do_lw_pic (char *);

#define MARCH_SCORE3   "score3"
#define MARCH_SCORE3D  "score3d"
#define MARCH_SCORE7   "score7"
#define MARCH_SCORE7D  "score7d"
#define MARCH_SCORE5   "score5"
#define MARCH_SCORE5U  "score5u"

#define SCORE_BI_ENDIAN

#ifdef SCORE_BI_ENDIAN
#define OPTION_EB             (OPTION_MD_BASE + 0)
#define OPTION_EL             (OPTION_MD_BASE + 1)
#else
#if TARGET_BYTES_BIG_ENDIAN
#define OPTION_EB             (OPTION_MD_BASE + 0)
#else
#define OPTION_EL             (OPTION_MD_BASE + 1)
#endif
#endif
#define OPTION_FIXDD          (OPTION_MD_BASE + 2)
#define OPTION_NWARN          (OPTION_MD_BASE + 3)
#define OPTION_SCORE5         (OPTION_MD_BASE + 4)
#define OPTION_SCORE5U        (OPTION_MD_BASE + 5)
#define OPTION_SCORE7         (OPTION_MD_BASE + 6)
#define OPTION_R1             (OPTION_MD_BASE + 7)
#define OPTION_O0             (OPTION_MD_BASE + 8)
#define OPTION_SCORE_VERSION  (OPTION_MD_BASE + 9)
#define OPTION_PIC            (OPTION_MD_BASE + 10)
#define OPTION_MARCH          (OPTION_MD_BASE + 11)
#define OPTION_SCORE3         (OPTION_MD_BASE + 12)

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = "#";
const char line_comment_chars[] = "#";
const char line_separator_chars[] = ";";
/* Chars that can be used to separate mant from exp in floating point numbers.  */
const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "rRsSfFdDxXeEpP";

#ifdef OBJ_ELF
/* Pre-defined "_GLOBAL_OFFSET_TABLE_"  */
symbolS *GOT_symbol;
#endif

const pseudo_typeS md_pseudo_table[] =
{
  {"bss", s_score_bss, 0},
  {"text", s_score_text, 0},
  {"word", cons, 4},
  {"long", cons, 4},
  {"extend", float_cons, 'x'},
  {"ldouble", float_cons, 'x'},
  {"packed", float_cons, 'p'},
  {"end", s_score_end, 0},
  {"ent", s_score_ent, 0},
  {"frame", s_score_frame, 0},
  {"rdata", s_change_sec, 'r'},
  {"sdata", s_change_sec, 's'},
  {"set", s_score_set, 0},
  {"mask", s_score_mask, 'R'},
  {"dword", cons, 8},
  {"lcomm", s_score_lcomm, 1},
  {"section", s_section, 0},
  {"cpload", s_score_cpload, 0},
  {"cprestore", s_score_cprestore, 0},
  {"gpword", s_score_gpword, 0},
  {"cpadd", s_score_cpadd, 0},
  {0, 0, 0}
};

const char *md_shortopts = "nO::g::G:";
struct option md_longopts[] =
{
#ifdef OPTION_EB
  {"EB"     , no_argument, NULL, OPTION_EB},
#endif
#ifdef OPTION_EL
  {"EL"     , no_argument, NULL, OPTION_EL},
#endif
  {"FIXDD"  , no_argument, NULL, OPTION_FIXDD},
  {"NWARN"  , no_argument, NULL, OPTION_NWARN},
  {"SCORE5" , no_argument, NULL, OPTION_SCORE5},
  {"SCORE5U", no_argument, NULL, OPTION_SCORE5U},
  {"SCORE7" , no_argument, NULL, OPTION_SCORE7},
  {"USE_R1" , no_argument, NULL, OPTION_R1},
  {"O0"     , no_argument, NULL, OPTION_O0},
  {"V"      , no_argument, NULL, OPTION_SCORE_VERSION},
  {"KPIC"   , no_argument, NULL, OPTION_PIC},
  {"march=" , required_argument, NULL, OPTION_MARCH},
  {"SCORE3" , no_argument, NULL, OPTION_SCORE3},
  {NULL     , no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

#define s3_GP                     28
#define s3_PIC_CALL_REG           29
#define s3_MAX_LITERAL_POOL_SIZE  1024
#define s3_FAIL	                  0x80000000
#define s3_SUCCESS                0
#define s3_INSN48_SIZE            6
#define s3_INSN_SIZE              4
#define s3_INSN16_SIZE            2
#define s3_RELAX_INST_NUM         3

/* For score5u : div/mul will pop warning message, mmu/alw/asw will pop error message.  */
#define s3_BAD_ARGS 	          _("bad arguments to instruction")
#define s3_ERR_FOR_SCORE5U_MUL_DIV   _("div / mul are reserved instructions")
#define s3_ERR_FOR_SCORE5U_MMU       _("This architecture doesn't support mmu")
#define s3_ERR_FOR_SCORE5U_ATOMIC    _("This architecture doesn't support atomic instruction")
#define s3_BAD_SKIP_COMMA            s3_BAD_ARGS
#define s3_BAD_GARBAGE               _("garbage following instruction");

#define s3_skip_whitespace(str)  while (*(str) == ' ') ++(str)

/* The name of the readonly data section.  */
#define s3_RDATA_SECTION_NAME (OUTPUT_FLAVOR == bfd_target_aout_flavour \
			    ? ".data" \
			    : OUTPUT_FLAVOR == bfd_target_ecoff_flavour \
			    ? ".rdata" \
			    : OUTPUT_FLAVOR == bfd_target_coff_flavour \
			    ? ".rdata" \
			    : OUTPUT_FLAVOR == bfd_target_elf_flavour \
			    ? ".rodata" \
			    : (abort (), ""))

#define s3_RELAX_ENCODE(old, new, type, reloc1, reloc2, opt) \
  ((relax_substateT) \
   (((old) << 23) \
    | ((new) << 16) \
    | ((type) << 9) \
    | ((reloc1) << 5) \
    | ((reloc2) << 1) \
    | ((opt) ? 1 : 0)))

#define s3_RELAX_OLD(i)       (((i) >> 23) & 0x7f)
#define s3_RELAX_NEW(i)       (((i) >> 16) & 0x7f)
#define s3_RELAX_TYPE(i)      (((i) >> 9) & 0x7f)
#define s3_RELAX_RELOC1(i)    ((valueT) ((i) >> 5) & 0xf)
#define s3_RELAX_RELOC2(i)    ((valueT) ((i) >> 1) & 0xf)
#define s3_RELAX_OPT(i)       ((i) & 1)

#define s3_SET_INSN_ERROR(s) (s3_inst.error = (s))
#define s3_INSN_IS_PCE_P(s)  (strstr (str, "||") != NULL)
#define s3_INSN_IS_48_P(s)  (strstr (str, "48") != NULL)
#define s3_GET_INSN_CLASS(type) (s3_get_insn_class_from_type (type))
#define s3_GET_INSN_SIZE(type) ((s3_GET_INSN_CLASS (type) == INSN_CLASS_16) \
                             ? s3_INSN16_SIZE : (s3_GET_INSN_CLASS (type) == INSN_CLASS_48) \
                                             ? s3_INSN48_SIZE : s3_INSN_SIZE)

#define s3_INSN_NAME_LEN 16

/* Relax will need some padding for alignment.  */
#define s3_RELAX_PAD_BYTE 3


#define s3_USE_GLOBAL_POINTER_OPT 1

/* Enumeration matching entries in table above.  */
enum s3_score_reg_type
{
  s3_REG_TYPE_SCORE = 0,
#define s3_REG_TYPE_FIRST s3_REG_TYPE_SCORE
  s3_REG_TYPE_SCORE_SR = 1,
  s3_REG_TYPE_SCORE_CR = 2,
  s3_REG_TYPE_MAX = 3
};

enum s3_score_pic_level
{
  s3_NO_PIC,
  s3_PIC
};
static enum s3_score_pic_level s3_score_pic = s3_NO_PIC;

enum s3_insn_type_for_dependency
{
  s3_D_mtcr,
  s3_D_all_insn
};

struct s3_insn_to_dependency
{
  const char *insn_name;
  enum s3_insn_type_for_dependency type;
};

struct s3_data_dependency
{
  enum s3_insn_type_for_dependency pre_insn_type;
  char pre_reg[6];
  enum s3_insn_type_for_dependency cur_insn_type;
  char cur_reg[6];
  int bubblenum_7;
  int bubblenum_3;
  int warn_or_error;           /* warning - 0; error - 1  */
};

static const struct s3_insn_to_dependency s3_insn_to_dependency_table[] =
{
  /* move special instruction.  */
  {"mtcr",      s3_D_mtcr},
};

static const struct s3_data_dependency s3_data_dependency_table[] =
{
  /* Status register.  */
  {s3_D_mtcr, "cr0", s3_D_all_insn, "", 5, 1, 0},
};

/* Used to contain constructed error messages.  */
static char s3_err_msg[255];

static int s3_fix_data_dependency = 0;
static int s3_warn_fix_data_dependency = 1;

static int s3_in_my_get_expression = 0;

/* Default, pop warning message when using r1.  */
static int s3_nor1 = 1;

/* Default will do instruction relax, -O0 will set s3_g_opt = 0.  */
static unsigned int s3_g_opt = 1;

/* The size of the small data section.  */
static unsigned int s3_g_switch_value = 8;

static segT s3_pdr_seg;

struct s3_score_it
{
  char name[s3_INSN_NAME_LEN];
  bfd_vma instruction;
  bfd_vma relax_inst;
  int size;
  int relax_size;
  enum score_insn_type type;
  char str[s3_MAX_LITERAL_POOL_SIZE];
  const char *error;
  int bwarn;
  char reg[s3_INSN_NAME_LEN];
  struct
  {
    bfd_reloc_code_real_type type;
    expressionS exp;
    int pc_rel;
  }reloc;
};
static struct s3_score_it s3_inst;

typedef struct s3_proc
{
  symbolS *isym;
  unsigned long reg_mask;
  unsigned long reg_offset;
  unsigned long fpreg_mask;
  unsigned long leaf;
  unsigned long frame_offset;
  unsigned long frame_reg;
  unsigned long pc_reg;
} s3_procS;
static s3_procS s3_cur_proc;
static s3_procS *s3_cur_proc_ptr;
static int s3_numprocs;


/* Structure for a hash table entry for a register.  */
struct s3_reg_entry
{
  const char *name;
  int number;
};

static const struct s3_reg_entry s3_score_rn_table[] =
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

static const struct s3_reg_entry s3_score_srn_table[] =
{
  {"sr0", 0}, {"sr1", 1}, {"sr2", 2},
  {NULL, 0}
};

static const struct s3_reg_entry s3_score_crn_table[] =
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

struct s3_reg_map
{
  const struct s3_reg_entry *names;
  int max_regno;
  htab_t htab;
  const char *expected;
};

static struct s3_reg_map s3_all_reg_maps[] =
{
  {s3_score_rn_table, 31, NULL, N_("S+core register expected")},
  {s3_score_srn_table, 2, NULL, N_("S+core special-register expected")},
  {s3_score_crn_table, 31, NULL, N_("S+core co-processor register expected")},
};

static htab_t s3_score_ops_hsh = NULL;
static htab_t s3_dependency_insn_hsh = NULL;


struct s3_datafield_range
{
  int data_type;
  int bits;
  int range[2];
};

static struct s3_datafield_range s3_score_df_range[] =
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
  {_SIMM5,            5,  {-(1 << 4), (1 << 4) - 1}},	/* (   -16 ~ 15   ) */
  {_SIMM6,            6,  {-(1 << 5), (1 << 5) - 1}},	/* (   -32 ~ 31   ) */
  {_IMM32,            32, {0, 0xfffffff}},
  {_SIMM32,           32, {-0x80000000, 0x7fffffff}},
  {_IMM11,            11, {0, (1 << 11) - 1}},
};

struct s3_asm_opcode
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

static const struct s3_asm_opcode s3_score_ldst_insns[] =
{
  {"lw",        0x20000000, 0x3e000000, 0x1000,     Rd_rvalueRs_SI15,     s3_do_ldst_insn},
  {"lw",        0x06000000, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s3_do_ldst_insn},
  {"lw",        0x0e000000, 0x3e000007, 0x0040,     Rd_rvalueRs_postSI12, s3_do_ldst_insn},
  {"lh",        0x22000000, 0x3e000000, 0x8000,     Rd_rvalueRs_SI15,     s3_do_ldst_insn},
  {"lh",        0x06000001, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s3_do_ldst_insn},
  {"lh",        0x0e000001, 0x3e000007, 0x8000,     Rd_rvalueRs_postSI12, s3_do_ldst_insn},
  {"lhu",       0x24000000, 0x3e000000, 0x8000,     Rd_rvalueRs_SI15,     s3_do_ldst_insn},
  {"lhu",       0x06000002, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s3_do_ldst_insn},
  {"lhu",       0x0e000002, 0x3e000007, 0x8000,     Rd_rvalueRs_postSI12, s3_do_ldst_insn},
  {"lb",        0x26000000, 0x3e000000, 0x8000,     Rd_rvalueRs_SI15,     s3_do_ldst_insn},
  {"lb",        0x06000003, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s3_do_ldst_insn},
  {"lb",        0x0e000003, 0x3e000007, 0x8000,     Rd_rvalueRs_postSI12, s3_do_ldst_insn},
  {"sw",        0x28000000, 0x3e000000, 0x2000,     Rd_lvalueRs_SI15,     s3_do_ldst_insn},
  {"sw",        0x06000004, 0x3e000007, 0x0060,     Rd_lvalueRs_preSI12,  s3_do_ldst_insn},
  {"sw",        0x0e000004, 0x3e000007, 0x8000,     Rd_lvalueRs_postSI12, s3_do_ldst_insn},
  {"sh",        0x2a000000, 0x3e000000, 0x8000,     Rd_lvalueRs_SI15,     s3_do_ldst_insn},
  {"sh",        0x06000005, 0x3e000007, 0x8000,     Rd_lvalueRs_preSI12,  s3_do_ldst_insn},
  {"sh",        0x0e000005, 0x3e000007, 0x8000,     Rd_lvalueRs_postSI12, s3_do_ldst_insn},
  {"lbu",       0x2c000000, 0x3e000000, 0x8000,     Rd_rvalueRs_SI15,     s3_do_ldst_insn},
  {"lbu",       0x06000006, 0x3e000007, 0x8000,     Rd_rvalueRs_preSI12,  s3_do_ldst_insn},
  {"lbu",       0x0e000006, 0x3e000007, 0x8000,     Rd_rvalueRs_postSI12, s3_do_ldst_insn},
  {"sb",        0x2e000000, 0x3e000000, 0x8000,     Rd_lvalueRs_SI15,     s3_do_ldst_insn},
  {"sb",        0x06000007, 0x3e000007, 0x8000,     Rd_lvalueRs_preSI12,  s3_do_ldst_insn},
  {"sb",        0x0e000007, 0x3e000007, 0x8000,     Rd_lvalueRs_postSI12, s3_do_ldst_insn},
};

static const struct s3_asm_opcode s3_score_insns[] =
{
  {"abs",       0x3800000a, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_dsp3},
  {"abs.s",     0x3800004b, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_dsp3},
  {"add",       0x00000010, 0x3e0003ff, 0x4800,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"add.c",     0x00000011, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"add.s",     0x38000048, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_dsp2},
  {"addc",      0x00000012, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"addc.c",    0x00000013, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"addi",      0x02000000, 0x3e0e0001, 0x5c00,     Rd_SI16,              s3_do_rdsi16},
  {"addi.c",    0x02000001, 0x3e0e0001, 0x8000,     Rd_SI16,              s3_do_rdsi16},
  {"addis",     0x0a000000, 0x3e0e0001, 0x8000,     Rd_SI16,              s3_do_rdi16},
  {"addis.c",   0x0a000001, 0x3e0e0001, 0x8000,     Rd_SI16,              s3_do_rdi16},
  {"addi!",     0x5c00,     0x7c00,     0x8000,     Rd_SI6,               s3_do16_addi},
  {"addri",     0x10000000, 0x3e000001, 0x8000,     Rd_Rs_SI14,           s3_do_rdrssi14},
  {"addri.c",   0x10000001, 0x3e000001, 0x8000,     Rd_Rs_SI14,           s3_do_rdrssi14},

  /* add.c <-> add!.  */
  {"add!",      0x4800,     0x7f00,     0x8000,     Rd_Rs,                s3_do16_rdrs2},
  {"subi",      0x02000000, 0x3e0e0001, 0x8000,     Rd_SI16,              s3_do_sub_rdsi16},
  {"subi.c",    0x02000001, 0x3e0e0001, 0x8000,     Rd_SI16,              s3_do_sub_rdsi16},
  {"subis",     0x0a000000, 0x3e0e0001, 0x8000,     Rd_SI16,              s3_do_sub_rdi16},
  {"subis.c",   0x0a000001, 0x3e0e0001, 0x8000,     Rd_SI16,              s3_do_sub_rdi16},
  {"subri",     0x10000000, 0x3e000001, 0x8000,     Rd_Rs_SI14,           s3_do_sub_rdrssi14},
  {"subri.c",   0x10000001, 0x3e000001, 0x8000,     Rd_Rs_SI14,           s3_do_sub_rdrssi14},
  {"and",       0x00000020, 0x3e0003ff, 0x4b00,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"and.c",     0x00000021, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"andi",      0x02080000, 0x3e0e0001, 0x8000,     Rd_I16,               s3_do_rdi16},
  {"andi.c",    0x02080001, 0x3e0e0001, 0x8000,     Rd_I16,               s3_do_rdi16},
  {"andis",     0x0a080000, 0x3e0e0001, 0x8000,     Rd_I16,               s3_do_rdi16},
  {"andis.c",   0x0a080001, 0x3e0e0001, 0x8000,     Rd_I16,               s3_do_rdi16},
  {"andri",     0x18000000, 0x3e000001, 0x8000,     Rd_Rs_I14,            s3_do_rdrsi14},
  {"andri.c",   0x18000001, 0x3e000001, 0x8000,     Rd_Rs_I14,            s3_do_rdrsi14},

  /* and.c <-> and!.  */
  {"and!",      0x4b00,     0x7f00,     0x8000,     Rd_Rs,                s3_do16_rdrs2},
  {"bcs",       0x08000000, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bcc",       0x08000400, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bcnz",      0x08003800, 0x3e007c01, 0x3200,     PC_DISP19div2,        s3_do_branch},
  {"bcsl",      0x08000001, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bccl",      0x08000401, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bcnzl",     0x08003801, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bcnz!",     0x3200,     0x7f00,     0x08003800, PC_DISP8div2,         s3_do16_branch},
  {"beq",       0x08001000, 0x3e007c01, 0x3800,     PC_DISP19div2,        s3_do_branch},
  {"beql",      0x08001001, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"beq!",      0x3800,     0x7e00,     0x08001000, PC_DISP8div2,         s3_do16_branch},
  {"bgtu",      0x08000800, 0x3e007c01, 0x3400,     PC_DISP19div2,        s3_do_branch},
  {"bgt",       0x08001800, 0x3e007c01, 0x3c00,     PC_DISP19div2,        s3_do_branch},
  {"bge",       0x08002000, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bgtul",     0x08000801, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bgtl",      0x08001801, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bgel",      0x08002001, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bgtu!",     0x3400,     0x7e00,     0x08000800, PC_DISP8div2,         s3_do16_branch},
  {"bgt!",      0x3c00,     0x7e00,     0x08001800, PC_DISP8div2,         s3_do16_branch},
  {"bitclr",    0x00000028, 0x3e0003ff, 0x5000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"bitclr.c",  0x00000029, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},

  {"mbitclr",   0x00000064, 0x3e00007e, 0x8000,     Ra_I9_I5,             s3_do_mbitclr},
  {"mbitset",   0x0000006c, 0x3e00007e, 0x8000,     Ra_I9_I5,             s3_do_mbitset},

  {"bitrev",    0x3800000c, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_dsp2},
  {"bitset",    0x0000002a, 0x3e0003ff, 0x5200,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"bitset.c",  0x0000002b, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"bittst.c",  0x0000002d, 0x3e0003ff, 0x5400,     x_Rs_I5,              s3_do_xrsi5},
  {"bittgl",    0x0000002e, 0x3e0003ff, 0x5600,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"bittgl.c",  0x0000002f, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"bitclr!",   0x5000,     0x7e00,     0x8000,     Rd_I5,                s3_do16_rdi5},
  {"bitset!",   0x5200,     0x7e00,     0x8000,     Rd_I5,                s3_do16_rdi5},
  {"bittst!",   0x5400,     0x7e00,     0x8000,     Rd_I5,                s3_do16_rdi5},
  {"bittgl!",   0x5600,     0x7e00,     0x8000,     Rd_I5,                s3_do16_rdi5},
  {"bleu",      0x08000c00, 0x3e007c01, 0x3600,     PC_DISP19div2,        s3_do_branch},
  {"ble",       0x08001c00, 0x3e007c01, 0x3e00,     PC_DISP19div2,        s3_do_branch},
  {"blt",       0x08002400, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bleul",     0x08000c01, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"blel",      0x08001c01, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bltl",      0x08002401, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bl",        0x08003c01, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bleu!",     0x3600,     0x7e00,     0x08000c00, PC_DISP8div2,         s3_do16_branch},
  {"ble!",      0x3e00,     0x7e00,     0x08001c00, PC_DISP8div2,         s3_do16_branch},
  {"bmi",       0x08002800, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bmil",      0x08002801, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bne",       0x08001400, 0x3e007c01, 0x3a00,     PC_DISP19div2,        s3_do_branch},
  {"bnel",      0x08001401, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bne!",      0x3a00,     0x7e00,     0x08001400, PC_DISP8div2,         s3_do16_branch},
  {"bpl",       0x08002c00, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bpll",      0x08002c01, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"brcs",      0x00000008, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brcc",      0x00000408, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brgtu",     0x00000808, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brleu",     0x00000c08, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"breq",      0x00001008, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brne",      0x00001408, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brgt",      0x00001808, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brle",      0x00001c08, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brge",      0x00002008, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brlt",      0x00002408, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brmi",      0x00002808, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brpl",      0x00002c08, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brvs",      0x00003008, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brvc",      0x00003408, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brcnz",     0x00003808, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"br",        0x00003c08, 0x3e007fff, 0x0080,     x_Rs_x,               s3_do_rs},
  {"brcsl",     0x00000009, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brccl",     0x00000409, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brgtul",    0x00000809, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brleul",    0x00000c09, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"breql",     0x00001009, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brnel",     0x00001409, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brgtl",     0x00001809, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brlel",     0x00001c09, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brgel",     0x00002009, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brltl",     0x00002409, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brmil",     0x00002809, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brpll",     0x00002c09, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brvsl",     0x00003009, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brvcl",     0x00003409, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brcnzl",    0x00003809, 0x3e007fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"brl",       0x00003c09, 0x3e007fff, 0x00a0,     x_Rs_x,               s3_do_rs},
  {"br!",       0x0080,     0x7fe0,     0x8000,     x_Rs,                 s3_do16_br},
  {"brl!",      0x00a0,     0x7fe0,     0x8000,     x_Rs,                 s3_do16_br},
  {"brr!",      0x00c0,     0x7fe0,     0x8000,     x_Rs,                 s3_do16_brr},
  {"bvs",       0x08003000, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bvc",       0x08003400, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bvsl",      0x08003001, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"bvcl",      0x08003401, 0x3e007c01, 0x8000,     PC_DISP19div2,        s3_do_branch},
  {"b!",        0x3000,     0x7e00,     0x08003c00, PC_DISP8div2,         s3_do16_branch},
  {"b",         0x08003c00, 0x3e007c01, 0x3000,     PC_DISP19div2,        s3_do_branch},
  {"cache",     0x30000000, 0x3ff00000, 0x8000,     OP5_rvalueRs_SI15,    s3_do_cache},
  {"ceinst",    0x38000000, 0x3e000000, 0x8000,     I5_Rs_Rs_I5_OP5,      s3_do_ceinst},
  {"clz",       0x0000001c, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"cmp.c",     0x00300019, 0x3ff003ff, 0x4400,     x_Rs_Rs,              s3_do_rsrs},
  {"cmpz.c",    0x0030001b, 0x3ff07fff, 0x8000,     x_Rs_x,               s3_do_rs},
  {"cmpi.c",    0x02040001, 0x3e0e0001, 0x6000,     Rd_SI16,              s3_do_rdsi16},

  /* cmp.c <-> cmp!.  */
  {"cmp!",      0x4400,     0x7c00,     0x8000,     Rd_Rs,                s3_do16_mv_cmp},
  {"cmpi!",     0x6000,     0x7c00,     0x8000,     Rd_SI5,               s3_do16_cmpi},
  {"cop1",      0x0c00000c, 0x3e00001f, 0x8000,     Rd_Rs_Rs_imm,         s3_do_crdcrscrsimm5},
  {"cop2",      0x0c000014, 0x3e00001f, 0x8000,     Rd_Rs_Rs_imm,         s3_do_crdcrscrsimm5},
  {"cop3",      0x0c00001c, 0x3e00001f, 0x8000,     Rd_Rs_Rs_imm,         s3_do_crdcrscrsimm5},
  {"drte",      0x0c0000a4, 0x3e0003ff, 0x8000,     NO_OPD,               s3_do_empty},
  {"disint!",    0x00e0,     0xffe1,     0x8000,     NO16_OPD,               s3_do16_int},
  {"enint!",     0x00e1,     0xffe1,     0x8000,     NO16_OPD,               s3_do16_int},
  {"extsb",     0x00000058, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"extsb.c",   0x00000059, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"extsh",     0x0000005a, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"extsh.c",   0x0000005b, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"extzb",     0x0000005c, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"extzb.c",   0x0000005d, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"extzh",     0x0000005e, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"extzh.c",   0x0000005f, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"jl",        0x04000001, 0x3e000001, 0x8000,     PC_DISP24div2,        s3_do_jump},
  {"j",         0x04000000, 0x3e000001, 0x8000,     PC_DISP24div2,        s3_do_jump},
  {"alw",       0x0000000c, 0x3e0003ff, 0x8000,     Rd_rvalue32Rs,        s3_do_ldst_atomic},
  {"lcb",       0x00000060, 0x3e0003ff, 0x8000,     x_rvalueRs_post4,     s3_do_ldst_unalign},
  {"lcw",       0x00000062, 0x3e0003ff, 0x8000,     Rd_rvalueRs_post4,    s3_do_ldst_unalign},
  {"lce",       0x00000066, 0x3e0003ff, 0x8000,     Rd_rvalueRs_post4,    s3_do_ldst_unalign},
  {"ldc1",      0x0c00000a, 0x3e00001f, 0x8000,     Rd_rvalueRs_SI10,     s3_do_ldst_cop},
  {"ldc2",      0x0c000012, 0x3e00001f, 0x8000,     Rd_rvalueRs_SI10,     s3_do_ldst_cop},
  {"ldc3",      0x0c00001a, 0x3e00001f, 0x8000,     Rd_rvalueRs_SI10,     s3_do_ldst_cop},

  /* s3_inst.relax */
  {"ldi",       0x020c0000, 0x3e0e0000, 0x6400,     Rd_SI16,              s3_do_rdsi16},
  {"ldis",      0x0a0c0000, 0x3e0e0000, 0x8000,     Rd_I16,               s3_do_ldis},

  /* ldi <-> ldiu!.  */
  {"ldiu!",     0x6400,     0x7c00,     0x8000,     Rd_I5,                s3_do16_ldiu},

  /*ltbb! , ltbh! ltbw! */
  {"ltbw",      0x00000032, 0x03ff,     0x8000,     Rd_Rs_Rs,             s3_do_ltb},
  {"ltbh",      0x00000132, 0x03ff,     0x8000,     Rd_Rs_Rs,             s3_do_ltb},
  {"ltbb",      0x00000332, 0x03ff,     0x8000,     Rd_Rs_Rs,             s3_do_ltb},
  {"lw!",       0x1000,     0x7000,     0x8000,     Rd_rvalueRs,          s3_do16_ldst_insn},
  {"mfcel",     0x00000448, 0x3e007fff, 0x8000,     Rd_x_x,               s3_do_rd},
  {"mfcel!",    0x7100,     0x7ff0,     0x00000448, x_Rs,                 s3_do16_dsp},
  {"mad",       0x38000000, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mad.f!",    0x7400,     0x7f00,     0x38000080, Rd_Rs,                s3_do16_dsp2},
  {"madh",      0x38000203, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"madh.fs",   0x380002c3, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"madh.fs!",  0x7b00,     0x7f00,     0x380002c3, Rd_Rs,                s3_do16_dsp2},
  {"madl",      0x38000002, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"madl.fs",   0x380000c2, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"madl.fs!",  0x7a00,     0x7f00,     0x380000c2, Rd_Rs,                s3_do16_dsp2},
  {"madu",      0x38000020, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"madu!",     0x7500,     0x7f00,     0x38000020, Rd_Rs,                s3_do16_dsp2},
  {"mad.f",     0x38000080, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"max",       0x38000007, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_dsp2},
  {"mazh",      0x38000303, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mazh.f",    0x38000383, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mazh.f!",   0x7900,     0x7f00,     0x3800038c, Rd_Rs,                s3_do16_dsp2},
  {"mazl",      0x38000102, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mazl.f",    0x38000182, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mazl.f!",   0x7800,     0x7f00,     0x38000182, Rd_Rs,                s3_do16_dsp2},
  {"mfceh",     0x00000848, 0x3e007fff, 0x8000,     Rd_x_x,               s3_do_rd},
  {"mfceh!",    0x7110,     0x7ff0,     0x00000848, x_Rs,                 s3_do16_dsp},
  {"mfcehl",    0x00000c48, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mfsr",      0x00000050, 0x3e0003ff, 0x8000,     Rd_x_I5,              s3_do_rdsrs},
  {"mfcr",      0x0c000001, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mfc1",      0x0c000009, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mfc2",      0x0c000011, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mfc3",      0x0c000019, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mfcc1",     0x0c00000f, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mfcc2",     0x0c000017, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mfcc3",     0x0c00001f, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"min",       0x38000006, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_dsp2},
  {"msb",       0x38000001, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"msb.f!",    0x7600,     0x7f00,     0x38000081, Rd_Rs,                s3_do16_dsp2},
  {"msbh",      0x38000205, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"msbh.fs",   0x380002c5, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"msbh.fs!",  0x7f00,     0x7f00,     0x380002c5, Rd_Rs,                s3_do16_dsp2},
  {"msbl",      0x38000004, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"msbl.fs",   0x380000c4, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"msbl.fs!",  0x7e00,     0x7f00,     0x380000c4, Rd_Rs,                s3_do16_dsp2},
  {"msbu",      0x38000021, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"msbu!",     0x7700,     0x7f00,     0x38000021, Rd_Rs,                s3_do16_dsp2},
  {"msb.f",     0x38000081, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mszh",      0x38000305, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mszh.f",    0x38000385, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mszh.f!",   0x7d00,     0x7f00,     0x38000385, Rd_Rs,                s3_do16_dsp2},
  {"mszl",      0x38000104, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mszl.f",    0x38000184, 0x3ff003ff, 0x8000,     x_Rs_Rs,              s3_do_dsp},
  {"mszl.f!",   0x7c00,     0x7f00,     0x38000184, Rd_Rs,                s3_do16_dsp2},
  {"mtcel!",    0x7000,     0x7ff0,     0x0000044a, x_Rs,                 s3_do16_dsp},
  {"mtcel",     0x0000044a, 0x3e007fff, 0x8000,     Rd_x_x,               s3_do_rd},
  {"mtceh",     0x0000084a, 0x3e007fff, 0x8000,     Rd_x_x,               s3_do_rd},
  {"mtceh!",    0x7010,     0x7ff0,     0x0000084a, x_Rs,                 s3_do16_dsp},
  {"mtcehl",    0x00000c4a, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mtsr",      0x00000052, 0x3e0003ff, 0x8000,     x_Rs_I5,              s3_do_rdsrs},
  {"mtcr",      0x0c000000, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mtc1",      0x0c000008, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mtc2",      0x0c000010, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mtc3",      0x0c000018, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mtcc1",     0x0c00000e, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mtcc2",     0x0c000016, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mtcc3",     0x0c00001e, 0x3e00001f, 0x8000,     Rd_Rs_x,              s3_do_rdcrs},
  {"mul.f!",    0x7200,     0x7f00,     0x00000041, Rd_Rs,                s3_do16_dsp2},
  {"mulu!",     0x7300,     0x7f00,     0x00000042, Rd_Rs,                s3_do16_dsp2},
  {"mulr.l",    0x00000140, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mulr.h",    0x00000240, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mulr",      0x00000340, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mulr.lf",   0x00000141, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mulr.hf",   0x00000241, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mulr.f",    0x00000341, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mulur.l",   0x00000142, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mulur.h",   0x00000242, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mulur",     0x00000342, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"divr.q",    0x00000144, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"divr.r",    0x00000244, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"divr",      0x00000344, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"divur.q",   0x00000146, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"divur.r",   0x00000246, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"divur",     0x00000346, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_rdrsrs},
  {"mvcs",      0x00000056, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvcc",      0x00000456, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvgtu",     0x00000856, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvleu",     0x00000c56, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mveq",      0x00001056, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvne",      0x00001456, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvgt",      0x00001856, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvle",      0x00001c56, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvge",      0x00002056, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvlt",      0x00002456, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvmi",      0x00002856, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvpl",      0x00002c56, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvvs",      0x00003056, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"mvvc",      0x00003456, 0x3e007fff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},

  /* mv <-> mv!.  */
  {"mv",        0x00003c56, 0x3e007fff, 0x4000,     Rd_Rs_x,              s3_do_rdrs},
  {"mv!",       0x4000,     0x7c00,     0x8000,     Rd_Rs,                s3_do16_mv_cmp},
  {"neg",       0x0000001e, 0x3e0003ff, 0x8000,     Rd_x_Rs,              s3_do_rdxrs},
  {"neg.c",     0x0000001f, 0x3e0003ff, 0x8000,     Rd_x_Rs,              s3_do_rdxrs},
  {"nop",       0x00000000, 0x3e0003ff, 0x0000,     NO_OPD,               s3_do_empty},
  {"not",       0x00000024, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"not.c",     0x00000025, 0x3e0003ff, 0x8000,     Rd_Rs_x,              s3_do_rdrs},
  {"nop!",      0x0000,     0x7fff,     0x8000,     NO16_OPD,             s3_do_empty},
  {"or",        0x00000022, 0x3e0003ff, 0x4a00,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"or.c",      0x00000023, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"ori",       0x020a0000, 0x3e0e0001, 0x8000,     Rd_I16,               s3_do_rdi16},
  {"ori.c",     0x020a0001, 0x3e0e0001, 0x8000,     Rd_I16,               s3_do_rdi16},
  {"oris",      0x0a0a0000, 0x3e0e0001, 0x8000,     Rd_I16,               s3_do_rdi16},
  {"oris.c",    0x0a0a0001, 0x3e0e0001, 0x8000,     Rd_I16,               s3_do_rdi16},
  {"orri",      0x1a000000, 0x3e000001, 0x8000,     Rd_Rs_I14,            s3_do_rdrsi14},
  {"orri.c",    0x1a000001, 0x3e000001, 0x8000,     Rd_Rs_I14,            s3_do_rdrsi14},

  /* or.c <-> or!.  */
  {"or!",       0x4a00,     0x7f00,     0x8000,     Rd_Rs,                s3_do16_rdrs2},
  {"pflush",    0x0000000a, 0x3e0003ff, 0x8000,     NO_OPD,               s3_do_empty},
  {"pop!",      0x0040,     0x7fe0,     0x8000,     Rd_rvalueRs,          s3_do16_push_pop},
  {"push!",     0x0060,     0x7fe0,     0x8000,     Rd_lvalueRs,          s3_do16_push_pop},

  {"rpop!",     0x6800,     0x7c00,     0x8000,     Rd_I5,                s3_do16_rpop},
  {"rpush!",    0x6c00,     0x7c00,     0x8000,     Rd_I5,                s3_do16_rpush},

  {"ror",       0x00000038, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"ror.c",     0x00000039, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"rorc.c",    0x0000003b, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"rol",       0x0000003c, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"rol.c",     0x0000003d, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"rolc.c",    0x0000003f, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"rori",      0x00000078, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"rori.c",    0x00000079, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"roric.c",   0x0000007b, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"roli",      0x0000007c, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"roli.c",    0x0000007d, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"rolic.c",   0x0000007f, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"rte",       0x0c000084, 0x3e0003ff, 0x8000,     NO_OPD,               s3_do_empty},
  {"asw",       0x0000000e, 0x3e0003ff, 0x8000,     Rd_lvalue32Rs,        s3_do_ldst_atomic},
  {"scb",       0x00000068, 0x3e0003ff, 0x8000,     Rd_lvalueRs_post4,    s3_do_ldst_unalign},
  {"scw",       0x0000006a, 0x3e0003ff, 0x8000,     Rd_lvalueRs_post4,    s3_do_ldst_unalign},
  {"sce",       0x0000006e, 0x3e0003ff, 0x8000,     x_lvalueRs_post4,     s3_do_ldst_unalign},
  {"sdbbp",     0x00000006, 0x3e0003ff, 0x0020,     x_I5_x,               s3_do_xi5x},
  {"sdbbp!",    0x0020,     0x7fe0,     0x8000,     Rd_I5,                s3_do16_xi5},
  {"sleep",     0x0c0000c4, 0x3e0003ff, 0x8000,     NO_OPD,               s3_do_empty},
  {"rti",       0x0c0000e4, 0x3e0003ff, 0x8000,     NO_OPD,               s3_do_empty},
  {"sll",       0x00000030, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"sll.c",     0x00000031, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"sll.s",     0x3800004e, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_dsp2},
  {"slli",      0x00000070, 0x3e0003ff, 0x5800,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"slli.c",    0x00000071, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},

  /* slli.c <-> slli!.  */
  {"slli!",     0x5800,     0x7e00,     0x8000,     Rd_I5,                s3_do16_slli_srli},
  {"srl",       0x00000034, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"srl.c",     0x00000035, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"sra",       0x00000036, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"sra.c",     0x00000037, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"srli",      0x00000074, 0x3e0003ff, 0x5a00,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"srli.c",    0x00000075, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"srai",      0x00000076, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},
  {"srai.c",    0x00000077, 0x3e0003ff, 0x8000,     Rd_Rs_I5,             s3_do_rdrsi5},

  /* srli.c <-> srli!.  */
  {"srli!",     0x5a00,     0x7e00,     0x8000,     Rd_Rs,                s3_do16_slli_srli},
  {"stc1",      0x0c00000b, 0x3e00001f, 0x8000,     Rd_lvalueRs_SI10,     s3_do_ldst_cop},
  {"stc2",      0x0c000013, 0x3e00001f, 0x8000,     Rd_lvalueRs_SI10,     s3_do_ldst_cop},
  {"stc3",      0x0c00001b, 0x3e00001f, 0x8000,     Rd_lvalueRs_SI10,     s3_do_ldst_cop},
  {"sub",       0x00000014, 0x3e0003ff, 0x4900,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"sub.c",     0x00000015, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"sub.s",     0x38000049, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_dsp2},
  {"subc",      0x00000016, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"subc.c",    0x00000017, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},

  /* sub.c <-> sub!.  */
  {"sub!",      0x4900,     0x7f00,     0x8000,     Rd_Rs,                s3_do16_rdrs2},
  {"sw!",       0x2000,     0x7000,     0x8000,     Rd_lvalueRs,          s3_do16_ldst_insn},
  {"syscall",   0x00000002, 0x3e0003ff, 0x8000,     I15,                  s3_do_i15},
  {"trapcs",    0x00000004, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapcc",    0x00000404, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapgtu",   0x00000804, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapleu",   0x00000c04, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapeq",    0x00001004, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapne",    0x00001404, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapgt",    0x00001804, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"traple",    0x00001c04, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapge",    0x00002004, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"traplt",    0x00002404, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapmi",    0x00002804, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trappl",    0x00002c04, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapvs",    0x00003004, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trapvc",    0x00003404, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"trap",      0x00003c04, 0x3e007fff, 0x8000,     x_I5_x,               s3_do_xi5x},
  {"xor",       0x00000026, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},
  {"xor.c",     0x00000027, 0x3e0003ff, 0x8000,     Rd_Rs_Rs,             s3_do_rdrsrs},

  /* Macro instruction.  */
  {"li",        0x020c0000, 0x3e0e0000, 0x8000,     Insn_Type_SYN,        s3_do_macro_li_rdi32},

  /* la reg, imm32        -->(1)  ldi  reg, simm16
                             (2)  ldis reg, %HI(imm32)
                                  ori  reg, %LO(imm32)

     la reg, symbol       -->(1)  lis  reg, %HI(imm32)
                                  ori  reg, %LO(imm32)  */
  {"la",        0x020c0000, 0x3e0e0000, 0x8000,     Insn_Type_SYN,        s3_do_macro_la_rdi32},
  {"bcmpeqz",       0x0000004c, 0x3e00007e, 0x8000,     Insn_BCMP,        s3_do_macro_bcmpz},
  {"bcmpeq",       0x0000004c, 0x3e00007e, 0x8000,     Insn_BCMP,        s3_do_macro_bcmp},
  {"bcmpnez",       0x0000004e, 0x3e00007e, 0x8000,     Insn_BCMP,        s3_do_macro_bcmpz},
  {"bcmpne",       0x0000004e, 0x3e00007e, 0x8000,     Insn_BCMP,        s3_do_macro_bcmp},
  {"div",       0x00000044, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"divu",      0x00000046, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"rem",       0x00000044, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"remu",      0x00000046, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"mul",       0x00000040, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"mulu",      0x00000042, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"maz",       0x00000040, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"mazu",      0x00000042, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"mul.f",     0x00000041, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"maz.f",     0x00000041, 0x3e0003ff, 0x8000,     Insn_Type_SYN,        s3_do_macro_mul_rdrsrs},
  {"lb",        INSN_LB,    0x00000000, 0x8000,     Insn_Type_SYN,        s3_do_macro_ldst_label},
  {"lbu",       INSN_LBU,   0x00000000, 0x8000,     Insn_Type_SYN,        s3_do_macro_ldst_label},
  {"lh",        INSN_LH,    0x00000000, 0x8000,     Insn_Type_SYN,        s3_do_macro_ldst_label},
  {"lhu",       INSN_LHU,   0x00000000, 0x8000,     Insn_Type_SYN,        s3_do_macro_ldst_label},
  {"lw",        INSN_LW,    0x00000000, 0x1000,     Insn_Type_SYN,        s3_do_macro_ldst_label},
  {"sb",        INSN_SB,    0x00000000, 0x8000,     Insn_Type_SYN,        s3_do_macro_ldst_label},
  {"sh",        INSN_SH,    0x00000000, 0x8000,     Insn_Type_SYN,        s3_do_macro_ldst_label},
  {"sw",        INSN_SW,    0x00000000, 0x2000,     Insn_Type_SYN,        s3_do_macro_ldst_label},

  /* Assembler use internal.  */
  {"ld_i32hi",  0x0a0c0000, 0x3e0e0000, 0x8000,     Insn_internal, s3_do_macro_rdi32hi},
  {"ld_i32lo",  0x020a0000, 0x3e0e0001, 0x8000,     Insn_internal, s3_do_macro_rdi32lo},
  {"ldis_pic",  0x0a0c0000, 0x3e0e0000, 0x8000,     Insn_internal, s3_do_rdi16_pic},
  {"addi_s_pic",0x02000000, 0x3e0e0001, 0x8000,     Insn_internal, s3_do_addi_s_pic},
  {"addi_u_pic",0x02000000, 0x3e0e0001, 0x8000,     Insn_internal, s3_do_addi_u_pic},
  {"lw_pic",    0x20000000, 0x3e000000, 0x8000,     Insn_internal, s3_do_lw_pic},

  /* 48-bit instructions.  */
  {"sdbbp48",   0x000000000000LL,   0x1c000000001fLL,   0x8000,     Rd_I32,    s3_do_sdbbp48},
  {"ldi48",     0x000000000001LL,   0x1c000000001fLL,   0x8000,     Rd_I32,    s3_do_ldi48},
  {"lw48",      0x000000000002LL,   0x1c000000001fLL,   0x8000,     Rd_I30,    s3_do_lw48},
  {"sw48",      0x000000000003LL,   0x1c000000001fLL,   0x8000,     Rd_I30,    s3_do_sw48},
  {"andri48",   0x040000000000LL,   0x1c0000000003LL,   0x8000,     Rd_I32,    s3_do_and48},
  {"andri48.c", 0x040000000001LL,   0x1c0000000003LL,   0x8000,     Rd_I32,    s3_do_and48},
  {"orri48",    0x040000000002LL,   0x1c0000000003LL,   0x8000,     Rd_I32,    s3_do_or48},
  {"orri48.c",  0x040000000003LL,   0x1c0000000003LL,   0x8000,     Rd_I32,    s3_do_or48},
};

#define s3_SCORE3_PIPELINE 3

static int s3_university_version = 0;
static int s3_vector_size = s3_SCORE3_PIPELINE;
static struct s3_score_it s3_dependency_vector[s3_SCORE3_PIPELINE];

static int s3_score3d = 1;

static int
s3_end_of_line (char *str)
{
  int retval = s3_SUCCESS;

  s3_skip_whitespace (str);
  if (*str != '\0')
    {
      retval = (int) s3_FAIL;

      if (!s3_inst.error)
        s3_inst.error = s3_BAD_GARBAGE;
    }

  return retval;
}

static int
s3_score_reg_parse (char **ccp, htab_t htab)
{
  char *start = *ccp;
  char c;
  char *p;
  struct s3_reg_entry *reg;

  p = start;
  if (!ISALPHA (*p) || !is_name_beginner (*p))
    return (int) s3_FAIL;

  c = *p++;

  while (ISALPHA (c) || ISDIGIT (c) || c == '_')
    c = *p++;

  *--p = 0;
  reg = (struct s3_reg_entry *) str_hash_find (htab, start);
  *p = c;

  if (reg)
    {
      *ccp = p;
      return reg->number;
    }
  return (int) s3_FAIL;
}

/* If shift <= 0, only return reg.  */

static int
s3_reg_required_here (char **str, int shift, enum s3_score_reg_type reg_type)
{
  static char buff[s3_MAX_LITERAL_POOL_SIZE];
  int reg = (int) s3_FAIL;
  char *start = *str;

  if ((reg = s3_score_reg_parse (str, s3_all_reg_maps[reg_type].htab)) != (int) s3_FAIL)
    {
      if (reg_type == s3_REG_TYPE_SCORE)
        {
          if ((reg == 1) && (s3_nor1 == 1) && (s3_inst.bwarn == 0))
            {
              as_warn (_("Using temp register (r1)"));
              s3_inst.bwarn = 1;
            }
        }
      if (shift >= 0)
	{
          if (reg_type == s3_REG_TYPE_SCORE_CR)
	    strcpy (s3_inst.reg, s3_score_crn_table[reg].name);
          else if (reg_type == s3_REG_TYPE_SCORE_SR)
	    strcpy (s3_inst.reg, s3_score_srn_table[reg].name);
          else
	    strcpy (s3_inst.reg, "");

          s3_inst.instruction |= (bfd_vma) reg << shift;
	}
    }
  else
    {
      *str = start;
      sprintf (buff, _("register expected, not '%.100s'"), start);
      s3_inst.error = buff;
    }

  return reg;
}

static int
s3_skip_past_comma (char **str)
{
  char *p = *str;
  char c;
  int comma = 0;

  while ((c = *p) == ' ' || c == ',')
    {
      p++;
      if (c == ',' && comma++)
        {
          s3_inst.error = s3_BAD_SKIP_COMMA;
          return (int) s3_FAIL;
        }
    }

  if ((c == '\0') || (comma == 0))
    {
      s3_inst.error = s3_BAD_SKIP_COMMA;
      return (int) s3_FAIL;
    }

  *str = p;
  return comma ? s3_SUCCESS : (int) s3_FAIL;
}

static void
s3_do_rdrsrs (char *str)
{
  int reg;
  s3_skip_whitespace (str);

  if ((reg = s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
  else
    {
      /* Check mulr, mulur rd is even number.  */
      if (((s3_inst.instruction & 0x3e0003ff) == 0x00000340
	   || (s3_inst.instruction & 0x3e0003ff) == 0x00000342)
          && (reg % 2))
        {
          s3_inst.error = _("rd must be even number.");
          return;
        }

      if ((((s3_inst.instruction >> 15) & 0x10) == 0)
          && (((s3_inst.instruction >> 10) & 0x10) == 0)
          && (((s3_inst.instruction >> 20) & 0x10) == 0)
          && (s3_inst.relax_inst != 0x8000)
          && (((s3_inst.instruction >> 20) & 0xf) == ((s3_inst.instruction >> 15) & 0xf)))
        {
          s3_inst.relax_inst |= (((s3_inst.instruction >> 10) & 0xf) )
            | (((s3_inst.instruction >> 15) & 0xf) << 4);
          s3_inst.relax_size = 2;
        }
      else
        {
          s3_inst.relax_inst = 0x8000;
        }
    }
}

static int
s3_walk_no_bignums (symbolS * sp)
{
  if (symbol_get_value_expression (sp)->X_op == O_big)
    return 1;

  if (symbol_get_value_expression (sp)->X_add_symbol)
    return (s3_walk_no_bignums (symbol_get_value_expression (sp)->X_add_symbol)
	    || (symbol_get_value_expression (sp)->X_op_symbol
		&& s3_walk_no_bignums (symbol_get_value_expression (sp)->X_op_symbol)));

  return 0;
}

static int
s3_my_get_expression (expressionS * ep, char **str)
{
  char *save_in;

  save_in = input_line_pointer;
  input_line_pointer = *str;
  s3_in_my_get_expression = 1;
  (void) expression (ep);
  s3_in_my_get_expression = 0;

  if (ep->X_op == O_illegal)
    {
      *str = input_line_pointer;
      input_line_pointer = save_in;
      s3_inst.error = _("illegal expression");
      return (int) s3_FAIL;
    }
  /* Get rid of any bignums now, so that we don't generate an error for which
     we can't establish a line number later on.  Big numbers are never valid
     in instructions, which is where this routine is always called.  */
  if (ep->X_op == O_big
      || (ep->X_add_symbol
          && (s3_walk_no_bignums (ep->X_add_symbol)
              || (ep->X_op_symbol && s3_walk_no_bignums (ep->X_op_symbol)))))
    {
      s3_inst.error = _("invalid constant");
      *str = input_line_pointer;
      input_line_pointer = save_in;
      return (int) s3_FAIL;
    }

  if ((ep->X_add_symbol != NULL)
      && (s3_inst.type != PC_DISP19div2)
      && (s3_inst.type != PC_DISP8div2)
      && (s3_inst.type != PC_DISP24div2)
      && (s3_inst.type != PC_DISP11div2)
      && (s3_inst.type != Insn_Type_SYN)
      && (s3_inst.type != Rd_rvalueRs_SI15)
      && (s3_inst.type != Rd_lvalueRs_SI15)
      && (s3_inst.type != Insn_internal)
      && (s3_inst.type != Rd_I30)
      && (s3_inst.type != Rd_I32)
      && (s3_inst.type != Insn_BCMP))
    {
      s3_inst.error = s3_BAD_ARGS;
      *str = input_line_pointer;
      input_line_pointer = save_in;
      return (int) s3_FAIL;
    }

  *str = input_line_pointer;
  input_line_pointer = save_in;
  return s3_SUCCESS;
}

/* Check if an immediate is valid.  If so, convert it to the right format.  */
static bfd_signed_vma
s3_validate_immediate (bfd_signed_vma val, unsigned int data_type, int hex_p)
{
  switch (data_type)
    {
    case _VALUE_HI16:
      {
        bfd_signed_vma val_hi = ((val & 0xffff0000) >> 16);

        if (s3_score_df_range[data_type].range[0] <= val_hi
            && val_hi <= s3_score_df_range[data_type].range[1])
	  return val_hi;
      }
      break;

    case _VALUE_LO16:
      {
        bfd_signed_vma val_lo = (val & 0xffff);

        if (s3_score_df_range[data_type].range[0] <= val_lo
            && val_lo <= s3_score_df_range[data_type].range[1])
	  return val_lo;
      }
      break;

    case _SIMM14:
      if (hex_p == 1)
        {
          if (!(val >= -0x2000 && val <= 0x3fff))
            {
              return (int) s3_FAIL;
            }
        }
      else
        {
          if (!(val >= -8192 && val <= 8191))
            {
              return (int) s3_FAIL;
            }
        }

      return val;
      break;

    case _SIMM16_NEG:
      if (hex_p == 1)
        {
          if (!(val >= -0x7fff && val <= 0xffff && val != 0x8000))
            {
              return (int) s3_FAIL;
            }
        }
      else
        {
          if (!(val >= -32767 && val <= 32768))
            {
              return (int) s3_FAIL;
            }
        }

      val = -val;
      return val;
      break;

    case _IMM5_MULTI_LOAD:
      if (val >= 2 && val <= 32)
        {
          if (val == 32)
	    val = 0;
          return val;
        }
      return (int) s3_FAIL;

    case _IMM32:
      if (val >= 0 && val <= 0xffffffff)
        {
          return val;
        }
      else
        {
          return (int) s3_FAIL;
        }

    default:
      if (data_type == _SIMM14_NEG || data_type == _IMM16_NEG)
	val = -val;

      if (s3_score_df_range[data_type].range[0] <= val
          && val <= s3_score_df_range[data_type].range[1])
	return val;

      break;
    }

  return (int) s3_FAIL;
}

static int
s3_data_op2 (char **str, int shift, enum score_data_type data_type)
{
  bfd_signed_vma value;
  char data_exp[s3_MAX_LITERAL_POOL_SIZE];
  char *dataptr;
  int cnt = 0;
  char *pp = NULL;

  s3_skip_whitespace (*str);
  s3_inst.error = NULL;
  dataptr = * str;

  /* Set hex_p to zero.  */
  int hex_p = 0;

  while ((*dataptr != '\0') && (*dataptr != '|') && (cnt <= s3_MAX_LITERAL_POOL_SIZE))     /* 0x7c = ='|' */
    {
      data_exp[cnt] = *dataptr;
      dataptr++;
      cnt++;
    }

  data_exp[cnt] = '\0';
  pp = (char *)&data_exp;

  if (*dataptr == '|')          /* process PCE */
    {
      if (s3_my_get_expression (&s3_inst.reloc.exp, &pp) == (int) s3_FAIL)
        return (int) s3_FAIL;
      s3_end_of_line (pp);
      if (s3_inst.error != 0)
        return (int) s3_FAIL;       /* to ouptut_inst to printf out the error */
      *str = dataptr;
    }
  else                          /* process  16 bit */
    {
      if (s3_my_get_expression (&s3_inst.reloc.exp, str) == (int) s3_FAIL)
        {
          return (int) s3_FAIL;
        }

      dataptr = (char *)data_exp;
      for (; *dataptr != '\0'; dataptr++)
        {
          *dataptr = TOLOWER (*dataptr);
          if (*dataptr == '!' || *dataptr == ' ')
            break;
        }
      dataptr = (char *)data_exp;

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
              && (data_type != _IMM5_MULTI_LOAD)
              && (data_type != _IMM11)
              && (data_type != _IMM8)
              && (data_type != _IMM5_RSHIFT_1)
              && (data_type != _IMM5_RSHIFT_2)
              && (data_type != _SIMM14)
              && (data_type != _SIMM14_NEG)
              && (data_type != _SIMM16_NEG)
              && (data_type != _IMM10_RSHIFT_2)
              && (data_type != _GP_IMM15)
              && (data_type != _SIMM5)
              && (data_type != _SIMM6)
              && (data_type != _IMM32)
              && (data_type != _SIMM32))
            {
              data_type += 24;
            }
        }

      if ((s3_inst.reloc.exp.X_add_number == 0)
          && (s3_inst.type != Insn_Type_SYN)
          && (s3_inst.type != Rd_rvalueRs_SI15)
          && (s3_inst.type != Rd_lvalueRs_SI15)
          && (s3_inst.type != Insn_internal)
          && (((*dataptr >= 'a') && (*dataptr <= 'z'))
	      || ((*dataptr == '0') && (*(dataptr + 1) == 'x') && (*(dataptr + 2) != '0'))
	      || ((*dataptr == '+') && (*(dataptr + 1) != '0'))
	      || ((*dataptr == '-') && (*(dataptr + 1) != '0'))))
        {
          s3_inst.error = s3_BAD_ARGS;
          return (int) s3_FAIL;
        }
    }

  if ((s3_inst.reloc.exp.X_add_symbol)
      && ((data_type == _SIMM16)
          || (data_type == _SIMM16_NEG)
          || (data_type == _IMM16_NEG)
          || (data_type == _SIMM14)
          || (data_type == _SIMM14_NEG)
          || (data_type == _IMM5)
          || (data_type == _IMM5_MULTI_LOAD)
          || (data_type == _IMM11)
          || (data_type == _IMM14)
          || (data_type == _IMM20)
          || (data_type == _IMM16)
          || (data_type == _IMM15)
          || (data_type == _IMM4)))
    {
      s3_inst.error = s3_BAD_ARGS;
      return (int) s3_FAIL;
    }

  if (s3_inst.reloc.exp.X_add_symbol)
    {
      switch (data_type)
        {
        case _SIMM16_LA:
          return (int) s3_FAIL;
        case _VALUE_HI16:
          s3_inst.reloc.type = BFD_RELOC_HI16_S;
          s3_inst.reloc.pc_rel = 0;
          break;
        case _VALUE_LO16:
          s3_inst.reloc.type = BFD_RELOC_LO16;
          s3_inst.reloc.pc_rel = 0;
          break;
        case _GP_IMM15:
          s3_inst.reloc.type = BFD_RELOC_SCORE_GPREL15;
          s3_inst.reloc.pc_rel = 0;
          break;
        case _SIMM16_pic:
        case _IMM16_LO16_pic:
          s3_inst.reloc.type = BFD_RELOC_SCORE_GOT_LO16;
          s3_inst.reloc.pc_rel = 0;
          break;
        default:
          s3_inst.reloc.type = BFD_RELOC_32;
          s3_inst.reloc.pc_rel = 0;
          break;
        }
    }
  else
    {
      if (data_type == _IMM16_pic)
	{
          s3_inst.reloc.type = BFD_RELOC_SCORE_DUMMY_HI16;
          s3_inst.reloc.pc_rel = 0;
	}

      if (data_type == _SIMM16_LA && s3_inst.reloc.exp.X_unsigned == 1)
        {
          value = s3_validate_immediate (s3_inst.reloc.exp.X_add_number, _SIMM16_LA_POS, hex_p);
          if (value == (int) s3_FAIL)       /* for advance to check if this is ldis */
            if ((s3_inst.reloc.exp.X_add_number & 0xffff) == 0)
              {
                s3_inst.instruction |= 0x8000000;
                s3_inst.instruction |= ((s3_inst.reloc.exp.X_add_number >> 16) << 1) & 0x1fffe;
                return s3_SUCCESS;
              }
        }
      else
        {
          value = s3_validate_immediate (s3_inst.reloc.exp.X_add_number, data_type, hex_p);
        }

      if (value == (int) s3_FAIL)
        {
          if (data_type == _IMM32)
            {
              sprintf (s3_err_msg,
                       _("invalid constant: %d bit expression not in range %u..%u"),
                       s3_score_df_range[data_type].bits,
                       0, (unsigned)0xffffffff);
            }
          else if (data_type == _IMM5_MULTI_LOAD)
            {
              sprintf (s3_err_msg,
                       _("invalid constant: %d bit expression not in range %u..%u"),
                       5, 2, 32);
            }
          else if ((data_type != _SIMM14_NEG) && (data_type != _SIMM16_NEG) && (data_type != _IMM16_NEG))
            {
              sprintf (s3_err_msg,
                       _("invalid constant: %d bit expression not in range %d..%d"),
                       s3_score_df_range[data_type].bits,
                       s3_score_df_range[data_type].range[0], s3_score_df_range[data_type].range[1]);
            }
          else
            {
              sprintf (s3_err_msg,
                       _("invalid constant: %d bit expression not in range %d..%d"),
                       s3_score_df_range[data_type].bits,
                       -s3_score_df_range[data_type].range[1], -s3_score_df_range[data_type].range[0]);
            }

          s3_inst.error = s3_err_msg;
          return (int) s3_FAIL;
        }

      if (((s3_score_df_range[data_type].range[0] != 0) || (data_type == _IMM5_RANGE_8_31))
          && data_type != _IMM5_MULTI_LOAD)
        {
          value &= (1 << s3_score_df_range[data_type].bits) - 1;
        }

      s3_inst.instruction |= value << shift;
    }

  if ((s3_inst.instruction & 0x3e000000) == 0x30000000)
    {
      if ((((s3_inst.instruction >> 20) & 0x1F) != 0)
          && (((s3_inst.instruction >> 20) & 0x1F) != 1)
          && (((s3_inst.instruction >> 20) & 0x1F) != 2)
          && (((s3_inst.instruction >> 20) & 0x1F) != 0x10))
        {
          s3_inst.error = _("invalid constant: bit expression not defined");
          return (int) s3_FAIL;
        }
    }

  return s3_SUCCESS;
}

/* Handle addi/addi.c/addis.c/cmpi.c/addis.c/ldi.  */
static void
s3_do_rdsi16 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 1, _SIMM16) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  /* ldi.->ldiu! only for imm5  */
  if ((s3_inst.instruction & 0x20c0000) == 0x20c0000)
    {
      if  ((s3_inst.instruction & 0x1ffc0) != 0)
        {
          s3_inst.relax_inst = 0x8000;
        }
      else
        {
          s3_inst.relax_inst |= (s3_inst.instruction >> 1) & 0x1f;
          s3_inst.relax_inst |= (((s3_inst.instruction >> 20)& 0x1f)  <<5);
          s3_inst.relax_size = 2;
        }
    }
  /*cmpi.c */
  else  if ((s3_inst.instruction & 0x02040001) == 0x02040001)
    {
      /*  imm <=0x3f  (5 bit<<1)*/
      if (((s3_inst.instruction & 0x1ffe0) == 0)
	  || (((s3_inst.instruction & 0x1ffe0) == 0x1ffe0)
	      && (s3_inst.instruction & 0x003e) != 0))
        {
          s3_inst.relax_inst |= (s3_inst.instruction >> 1) & 0x1f;
          s3_inst.relax_inst |= (((s3_inst.instruction >> 20) & 0x1f) << 5);
          s3_inst.relax_size = 2;
        }
      else
        {
          s3_inst.relax_inst =0x8000;

        }
    }
  /* addi */
  else  if (((s3_inst.instruction & 0x2000000) == 0x02000000) && (s3_inst.relax_inst!=0x8000))
    {
      /* rd : 0-16 ; imm <=0x7f  (6 bit<<1)*/
      if ((((s3_inst.instruction >> 20) & 0x10) != 0x10)
	  && (((s3_inst.instruction & 0x1ffc0) == 0)
	      || (((s3_inst.instruction & 0x1ffc0) == 0x1ffc0)
		  && (s3_inst.instruction & 0x007e) != 0)))
        {
          s3_inst.relax_inst |= (s3_inst.instruction >> 1) & 0x3f;
          s3_inst.relax_inst |= (((s3_inst.instruction >> 20) & 0xf) << 6);
          s3_inst.relax_size = 2;
        }
      else
        {
          s3_inst.relax_inst =0x8000;
        }
    }

  else if (((s3_inst.instruction >> 20) & 0x10) == 0x10)
    {
      s3_inst.relax_inst = 0x8000;
    }
}

static void
s3_do_ldis (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 1, _IMM16) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;
}

/* Handle subi/subi.c.  */
static void
s3_do_sub_rdsi16 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_data_op2 (&str, 1, _SIMM16_NEG) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle subis/subis.c.  */
static void
s3_do_sub_rdi16 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_data_op2 (&str, 1, _IMM16_NEG) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle addri/addri.c.  */
static void
s3_do_rdrssi14 (char *str)         /* -(2^13)~((2^13)-1) */
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL)
    s3_data_op2 (&str, 1, _SIMM14);
}

/* Handle subri.c/subri.  */
static void
s3_do_sub_rdrssi14 (char *str)     /* -(2^13)~((2^13)-1) */
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_data_op2 (&str, 1, _SIMM14_NEG) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle bitclr.c/bitset.c/bittgl.c/slli.c/srai.c/srli.c/roli.c/rori.c/rolic.c.
   0~((2^14)-1) */
static void
s3_do_rdrsi5 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 10, _IMM5) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if ((((s3_inst.instruction >> 20) & 0x1f) == ((s3_inst.instruction >> 15) & 0x1f))
      && (s3_inst.relax_inst != 0x8000) && (((s3_inst.instruction >> 15) & 0x10) == 0))
    {
      s3_inst.relax_inst |= (((s3_inst.instruction >> 10) & 0x1f) ) | (((s3_inst.instruction >> 15) & 0xf) << 5);
      s3_inst.relax_size = 2;
    }
  else
    s3_inst.relax_inst = 0x8000;
}

/* Handle andri/orri/andri.c/orri.c.
   0 ~ ((2^14)-1)  */
static void
s3_do_rdrsi14 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_data_op2 (&str, 1, _IMM14) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle bittst.c.  */
static void
s3_do_xrsi5 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 10, _IMM5) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if ((s3_inst.relax_inst != 0x8000) && (((s3_inst.instruction >> 15) & 0x10) == 0))
    {
      s3_inst.relax_inst |= ((s3_inst.instruction >> 10) & 0x1f)  | (((s3_inst.instruction >> 15) & 0xf) << 5);
      s3_inst.relax_size = 2;
    }
  else
    s3_inst.relax_inst = 0x8000;
}

/* Handle addis/andi/ori/andis/oris/ldis.  */
static void
s3_do_rdi16 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 1, _IMM16) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  /* ldis */
  if ((s3_inst.instruction & 0x3e0e0000) == 0x0a0c0000)
    {
      /* rd : 0-16 ;imm =0 -> can transform to addi!*/
      if ((((s3_inst.instruction >> 20) & 0x10) != 0x10) && ((s3_inst.instruction & 0x1ffff)==0))
        {
          s3_inst.relax_inst =0x5400; /* ldiu! */
          s3_inst.relax_inst |= (s3_inst.instruction >> 1) & 0x1f;
          s3_inst.relax_inst |= (((s3_inst.instruction >> 20) & 0xf) << 5);
          s3_inst.relax_size = 2;
        }
      else
        {
          s3_inst.relax_inst =0x8000;

        }
    }

  /* addis */
  else if ((s3_inst.instruction & 0x3e0e0001) == 0x0a000000)
    {
      /* rd : 0-16 ;imm =0 -> can transform to addi!*/
      if ((((s3_inst.instruction >> 20) & 0x10) != 0x10) && ((s3_inst.instruction & 0x1ffff)==0))
        {
	  s3_inst.relax_inst =0x5c00; /* addi! */
          s3_inst.relax_inst |= (s3_inst.instruction >> 1) & 0x3f;
          s3_inst.relax_inst |= (((s3_inst.instruction >> 20) & 0xf) << 6);
          s3_inst.relax_size = 2;
        }
      else
        {
          s3_inst.relax_inst =0x8000;

        }
    }
}

static void
s3_do_macro_rdi32hi (char *str)
{
  s3_skip_whitespace (str);

  /* Do not handle s3_end_of_line().  */
  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL)
    s3_data_op2 (&str, 1, _VALUE_HI16);
}

static void
s3_do_macro_rdi32lo (char *str)
{
  s3_skip_whitespace (str);

  /* Do not handle s3_end_of_line().  */
  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL)
    s3_data_op2 (&str, 1, _VALUE_LO16);
}

/* Handle ldis_pic.  */
static void
s3_do_rdi16_pic (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_data_op2 (&str, 1, _IMM16_pic) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle addi_s_pic to generate R_SCORE_GOT_LO16 .  */
static void
s3_do_addi_s_pic (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_data_op2 (&str, 1, _SIMM16_pic) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle addi_u_pic to generate R_SCORE_GOT_LO16 .  */
static void
s3_do_addi_u_pic (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_data_op2 (&str, 1, _IMM16_LO16_pic) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle mfceh/mfcel/mtceh/mtchl.  */
static void
s3_do_rd (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle br{cond},cmpzteq.c ,cmpztmi.c ,cmpz.c */
static void
s3_do_rs (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if ((s3_inst.relax_inst != 0x8000) )
    {
      s3_inst.relax_inst |=  ((s3_inst.instruction >> 15) &0x1f);
      s3_inst.relax_size = 2;
    }
  else
    s3_inst.relax_inst = 0x8000;
}

static void
s3_do_i15 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_data_op2 (&str, 10, _IMM15) != (int) s3_FAIL)
    s3_end_of_line (str);
}

static void
s3_do_xi5x (char *str)
{
  s3_skip_whitespace (str);

  if (s3_data_op2 (&str, 15, _IMM5) == (int) s3_FAIL || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if (s3_inst.relax_inst != 0x8000)
    {
      s3_inst.relax_inst |= ((s3_inst.instruction >> 15) & 0x1f);
      s3_inst.relax_size = 2;
    }
}

static void
s3_do_rdrs (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if (s3_inst.relax_inst != 0x8000)
    {
      if (((s3_inst.instruction & 0x7f) == 0x56))  /* adjust mv -> mv!*/
        {
          /* mv! rd : 5bit , ra : 5bit */
          s3_inst.relax_inst |= ((s3_inst.instruction >> 15) & 0x1f)  | (((s3_inst.instruction >> 20) & 0x1f) << 5);
          s3_inst.relax_size = 2;
        }
      else if ((((s3_inst.instruction >> 15) & 0x10) == 0x0) && (((s3_inst.instruction >> 20) & 0x10) == 0))
        {
          s3_inst.relax_inst |= (((s3_inst.instruction >> 15) & 0xf) << 4)
            | (((s3_inst.instruction >> 20) & 0xf) << 8);
          s3_inst.relax_size = 2;
        }
      else
        {
          s3_inst.relax_inst = 0x8000;
        }
    }
}

/* Handle mfcr/mtcr.  */
static void
s3_do_rdcrs (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
      && s3_skip_past_comma (&str) != (int) s3_FAIL
      && s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE_CR) != (int) s3_FAIL)
    s3_end_of_line (str);
}

/* Handle mfsr/mtsr.  */
static void
s3_do_rdsrs (char *str)
{
  s3_skip_whitespace (str);

  /* mfsr */
  if ((s3_inst.instruction & 0xff) == 0x50)
    {
      if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) != (int) s3_FAIL
          && s3_skip_past_comma (&str) != (int) s3_FAIL
          && s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE_SR) != (int) s3_FAIL)
	s3_end_of_line (str);
    }
  else
    {
      if (s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) != (int) s3_FAIL
          && s3_skip_past_comma (&str) != (int) s3_FAIL)
	s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE_SR);
    }
}

/* Handle neg.  */
static void
s3_do_rdxrs (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if ((s3_inst.relax_inst != 0x8000) && (((s3_inst.instruction >> 10) & 0x10) == 0)
      && (((s3_inst.instruction >> 20) & 0x10) == 0))
    {
      s3_inst.relax_inst |= (((s3_inst.instruction >> 10) & 0xf) << 4) | (((s3_inst.instruction >> 20) & 0xf) << 8);
      s3_inst.relax_size = 2;
    }
  else
    s3_inst.relax_inst = 0x8000;
}

/* Handle cmp.c/cmp<cond>.  */
static void
s3_do_rsrs (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if ((s3_inst.relax_inst != 0x8000) && (((s3_inst.instruction >> 20) & 0x1f) == 3) )
    {
      s3_inst.relax_inst |= (((s3_inst.instruction >> 10) & 0x1f)) | (((s3_inst.instruction >> 15) & 0x1f) << 5);
      s3_inst.relax_size = 2;
    }
  else
    s3_inst.relax_inst = 0x8000;
}

static void
s3_do_ceinst (char *str)
{
  char *strbak;

  strbak = str;
  s3_skip_whitespace (str);

  if (s3_data_op2 (&str, 20, _IMM5) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 5, _IMM5) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 0, _IMM5) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
  else
    {
      str = strbak;
      if (s3_data_op2 (&str, 0, _IMM25) == (int) s3_FAIL)
	return;
    }
}

static int
s3_reglow_required_here (char **str, int shift)
{
  static char buff[s3_MAX_LITERAL_POOL_SIZE];
  int reg;
  char *start = *str;

  if ((reg = s3_score_reg_parse (str, s3_all_reg_maps[s3_REG_TYPE_SCORE].htab)) != (int) s3_FAIL)
    {
      if ((reg == 1) && (s3_nor1 == 1) && (s3_inst.bwarn == 0))
        {
          as_warn (_("Using temp register(r1)"));
          s3_inst.bwarn = 1;
        }
      if (reg < 16)
        {
          if (shift >= 0)
            s3_inst.instruction |= (bfd_vma) reg << shift;

          return reg;
        }
    }

  /* Restore the start point, we may have got a reg of the wrong class.  */
  *str = start;
  sprintf (buff, _("low register (r0-r15) expected, not '%.100s'"), start);
  s3_inst.error = buff;
  return (int) s3_FAIL;
}

/* Handle add!/and!/or!/sub!.  */
static void
s3_do16_rdrs2 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reglow_required_here (&str, 4) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reglow_required_here (&str, 0) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
}

/* Handle br!/brl!.  */
static void
s3_do16_br (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 0, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
}

/* Handle brr!.  */
static void
s3_do16_brr (char *str)
{
  int rd = 0;

  s3_skip_whitespace (str);

  if ((rd = s3_reg_required_here (&str, 0,s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
}

/*Handle ltbw / ltbh / ltbb */
static void
s3_do_ltb (char *str)
{
  s3_skip_whitespace (str);
  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL)
    {
      return;
    }

  s3_skip_whitespace (str);
  if (*str++ != '[')
    {
      s3_inst.error = _("missing [");
      return;
    }

  if (s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
    {
      return;
    }

  s3_skip_whitespace (str);
  if (*str++ != ']')
    {
      s3_inst.error = _("missing ]");
      return;
    }
}

/* We need to be able to fix up arbitrary expressions in some statements.
   This is so that we can handle symbols that are an arbitrary distance from
   the pc.  The most common cases are of the form ((+/-sym -/+ . - 8) & mask),
   which returns part of an address in a form which will be valid for
   a data instruction.  We do this by pushing the expression into a symbol
   in the expr_section, and creating a fix for that.  */
static fixS *
s3_fix_new_score (fragS * frag, int where, short int size, expressionS * exp, int pc_rel, int reloc)
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
s3_init_dependency_vector (void)
{
  int i;

  for (i = 0; i < s3_vector_size; i++)
    memset (&s3_dependency_vector[i], '\0', sizeof (s3_dependency_vector[i]));

  return;
}

static enum s3_insn_type_for_dependency
s3_dependency_type_from_insn (char *insn_name)
{
  char name[s3_INSN_NAME_LEN];
  const struct s3_insn_to_dependency *tmp;

  strcpy (name, insn_name);
  tmp = (const struct s3_insn_to_dependency *)
    str_hash_find (s3_dependency_insn_hsh, name);

  if (tmp)
    return tmp->type;

  return s3_D_all_insn;
}

static int
s3_check_dependency (char *pre_insn, char *pre_reg,
		     char *cur_insn, char *cur_reg, int *warn_or_error)
{
  int bubbles = 0;
  unsigned int i;
  enum s3_insn_type_for_dependency pre_insn_type;
  enum s3_insn_type_for_dependency cur_insn_type;

  pre_insn_type = s3_dependency_type_from_insn (pre_insn);
  cur_insn_type = s3_dependency_type_from_insn (cur_insn);

  for (i = 0; i < sizeof (s3_data_dependency_table) / sizeof (s3_data_dependency_table[0]); i++)
    {
      if ((pre_insn_type == s3_data_dependency_table[i].pre_insn_type)
          && (s3_D_all_insn == s3_data_dependency_table[i].cur_insn_type
              || cur_insn_type == s3_data_dependency_table[i].cur_insn_type)
          && (strcmp (s3_data_dependency_table[i].pre_reg, "") == 0
              || strcmp (s3_data_dependency_table[i].pre_reg, pre_reg) == 0)
          && (strcmp (s3_data_dependency_table[i].cur_reg, "") == 0
              || strcmp (s3_data_dependency_table[i].cur_reg, cur_reg) == 0))
        {
          bubbles = s3_data_dependency_table[i].bubblenum_3;
          *warn_or_error = s3_data_dependency_table[i].warn_or_error;
          break;
        }
    }

  return bubbles;
}

static void
s3_build_one_frag (struct s3_score_it one_inst)
{
  char *p;
  int relaxable_p = s3_g_opt;
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
  s3_md_number_to_chars (p, one_inst.instruction, one_inst.size);

#ifdef OBJ_ELF
  dwarf2_emit_insn (one_inst.size);
#endif

  relaxable_p &= (one_inst.relax_size != 0);
  relax_size = relaxable_p ? one_inst.relax_size : 0;

  p = frag_var (rs_machine_dependent, relax_size + s3_RELAX_PAD_BYTE, 0,
                s3_RELAX_ENCODE (one_inst.size, one_inst.relax_size,
				 one_inst.type, 0, 0, relaxable_p),
                NULL, 0, NULL);

  if (relaxable_p)
    s3_md_number_to_chars (p, one_inst.relax_inst, relax_size);
}

static void
s3_handle_dependency (struct s3_score_it *theinst)
{
  int i;
  int warn_or_error = 0;   /* warn - 0; error - 1  */
  int bubbles = 0;
  int remainder_bubbles = 0;
  char cur_insn[s3_INSN_NAME_LEN];
  char pre_insn[s3_INSN_NAME_LEN];
  struct s3_score_it nop_inst;
  struct s3_score_it pflush_inst;

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
      s3_init_dependency_vector ();
      return;
    }

  /* Push current instruction to s3_dependency_vector[0].  */
  for (i = s3_vector_size - 1; i > 0; i--)
    memcpy (&s3_dependency_vector[i], &s3_dependency_vector[i - 1], sizeof (s3_dependency_vector[i]));

  memcpy (&s3_dependency_vector[0], theinst, sizeof (s3_dependency_vector[i]));

  /* There is no dependency between nop and any instruction.  */
  if (strcmp (s3_dependency_vector[0].name, "nop") == 0
      || strcmp (s3_dependency_vector[0].name, "nop!") == 0)
    return;

  strcpy (cur_insn, s3_dependency_vector[0].name);

  for (i = 1; i < s3_vector_size; i++)
    {
      /* The element of s3_dependency_vector is NULL.  */
      if (s3_dependency_vector[i].name[0] == '\0')
	continue;

      strcpy (pre_insn, s3_dependency_vector[i].name);

      bubbles = s3_check_dependency (pre_insn, s3_dependency_vector[i].reg,
				     cur_insn, s3_dependency_vector[0].reg, &warn_or_error);
      remainder_bubbles = bubbles - i + 1;

      if (remainder_bubbles > 0)
        {
          int j;

          if (s3_fix_data_dependency == 1)
            {
	      if (remainder_bubbles <= 2)
		{
		  if (s3_warn_fix_data_dependency)
		    as_warn (_("Fix data dependency: %s %s -- %s %s (insert %d nop!/%d)"),
			     s3_dependency_vector[i].name, s3_dependency_vector[i].reg,
			     s3_dependency_vector[0].name, s3_dependency_vector[0].reg,
			     remainder_bubbles, bubbles);

                  for (j = (s3_vector_size - 1); (j - remainder_bubbles) > 0; j--)
		    memcpy (&s3_dependency_vector[j], &s3_dependency_vector[j - remainder_bubbles],
			    sizeof (s3_dependency_vector[j]));

                  for (j = 1; j <= remainder_bubbles; j++)
                    {
                      memset (&s3_dependency_vector[j], '\0', sizeof (s3_dependency_vector[j]));
		      /* Insert nop!.  */
    		      s3_build_one_frag (nop_inst);
                    }
		}
	      else
		{
		  if (s3_warn_fix_data_dependency)
		    as_warn (_("Fix data dependency: %s %s -- %s %s (insert 1 pflush/%d)"),
			     s3_dependency_vector[i].name, s3_dependency_vector[i].reg,
			     s3_dependency_vector[0].name, s3_dependency_vector[0].reg,
			     bubbles);

                  for (j = 1; j < s3_vector_size; j++)
		    memset (&s3_dependency_vector[j], '\0', sizeof (s3_dependency_vector[j]));

                  /* Insert pflush.  */
                  s3_build_one_frag (pflush_inst);
		}
            }
          else
            {
	      if (warn_or_error)
		{
                  as_bad (_("data dependency: %s %s -- %s %s (%d/%d bubble)"),
			  s3_dependency_vector[i].name, s3_dependency_vector[i].reg,
			  s3_dependency_vector[0].name, s3_dependency_vector[0].reg,
			  remainder_bubbles, bubbles);
		}
	      else
		{
                  as_warn (_("data dependency: %s %s -- %s %s (%d/%d bubble)"),
                           s3_dependency_vector[i].name, s3_dependency_vector[i].reg,
                           s3_dependency_vector[0].name, s3_dependency_vector[0].reg,
                           remainder_bubbles, bubbles);
		}
            }
        }
    }
}

static enum insn_class
s3_get_insn_class_from_type (enum score_insn_type type)
{
  enum insn_class retval = (int) s3_FAIL;

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
    case Rd_SI5:
    case Rd_SI6:
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
    case Insn_BCMP:
    case Ra_I9_I5:
      retval = INSN_CLASS_32;
      break;
    case Insn_Type_PCE:
      retval = INSN_CLASS_PCE;
      break;
    case Insn_Type_SYN:
      retval = INSN_CLASS_SYN;
      break;
    case Rd_I30:
    case Rd_I32:
      retval = INSN_CLASS_48;
      break;
    default:
      abort ();
      break;
    }
  return retval;
}

/* Type of p-bits:
   48-bit instruction: 1, 1, 0.
   32-bit instruction: 1, 0.
   16-bit instruction: 0.  */
static bfd_vma
s3_adjust_paritybit (bfd_vma m_code, enum insn_class i_class)
{
  bfd_vma result = 0;
  bfd_vma m_code_high = 0;
  unsigned long m_code_middle = 0;
  unsigned long m_code_low = 0;
  bfd_vma pb_high = 0;
  unsigned long pb_middle = 0;
  unsigned long pb_low = 0;

  if (i_class == INSN_CLASS_48)
    {
      pb_high = 0x800000000000LL;
      pb_middle = 0x80000000;
      pb_low = 0x00000000;
      m_code_high = m_code & 0x1fffc0000000LL;
      m_code_middle = m_code & 0x3fff8000;
      m_code_low = m_code & 0x00007fff;
      result = pb_high | (m_code_high << 2) |
	pb_middle | (m_code_middle << 1) |
	pb_low | m_code_low;
    }
  else if (i_class == INSN_CLASS_32 || i_class == INSN_CLASS_SYN)
    {
      pb_high = 0x80000000;
      pb_low = 0x00000000;
      m_code_high = m_code & 0x3fff8000;
      m_code_low = m_code & 0x00007fff;
      result = pb_high | (m_code_high << 1) | pb_low | m_code_low;
    }
  else if (i_class == INSN_CLASS_16)
    {
      pb_high = 0;
      pb_low = 0;
      m_code_high = m_code & 0x3fff8000;
      m_code_low = m_code & 0x00007fff;
      result = pb_high | (m_code_high << 1) | pb_low | m_code_low;
    }
  else if (i_class == INSN_CLASS_PCE)
    {
      /* Keep original.  */
      pb_high = 0;
      pb_low = 0x00008000;
      m_code_high = m_code & 0x3fff8000;
      m_code_low = m_code & 0x00007fff;
      result = pb_high | (m_code_high << 1) | pb_low | m_code_low;
    }
  else
    {
      abort ();
    }

  return result;
}

static void
s3_gen_insn_frag (struct s3_score_it *part_1, struct s3_score_it *part_2)
{
  char *p;
  bool pce_p = false;
  int relaxable_p = s3_g_opt;
  int relax_size = 0;
  struct s3_score_it *inst1 = part_1;
  struct s3_score_it *inst2 = part_2;
  struct s3_score_it backup_inst1;

  pce_p = inst2 != NULL;
  memcpy (&backup_inst1, inst1, sizeof (struct s3_score_it));

  /* Adjust instruction opcode and to be relaxed instruction opcode.  */
  if (pce_p)
    {
      backup_inst1.instruction = ((backup_inst1.instruction & 0x7FFF) << 15)
	| (inst2->instruction & 0x7FFF);
      backup_inst1.instruction = s3_adjust_paritybit (backup_inst1.instruction, INSN_CLASS_PCE);
      backup_inst1.relax_inst = 0x8000;
      backup_inst1.size = s3_INSN_SIZE;
      backup_inst1.relax_size = 0;
      backup_inst1.type = Insn_Type_PCE;
    }
  else
    {
      backup_inst1.instruction = s3_adjust_paritybit (backup_inst1.instruction,
						      s3_GET_INSN_CLASS (backup_inst1.type));
    }

  if (backup_inst1.relax_size != 0)
    {
      enum insn_class tmp;

      tmp = (backup_inst1.size == s3_INSN_SIZE) ? INSN_CLASS_16 : INSN_CLASS_32;
      backup_inst1.relax_inst = s3_adjust_paritybit (backup_inst1.relax_inst, tmp);
    }

  /* Check data dependency.  */
  s3_handle_dependency (&backup_inst1);

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
  s3_md_number_to_chars (p, backup_inst1.instruction, backup_inst1.size);

#ifdef OBJ_ELF
  dwarf2_emit_insn (backup_inst1.size);
#endif

  /* Generate fixup structure.  */
  if (pce_p)
    {
      if (inst1->reloc.type != BFD_RELOC_NONE)
	s3_fix_new_score (frag_now, p - frag_now->fr_literal,
			  inst1->size, &inst1->reloc.exp,
			  inst1->reloc.pc_rel, inst1->reloc.type);

      if (inst2->reloc.type != BFD_RELOC_NONE)
	s3_fix_new_score (frag_now, p - frag_now->fr_literal + 2,
			  inst2->size, &inst2->reloc.exp, inst2->reloc.pc_rel, inst2->reloc.type);
    }
  else
    {
      if (backup_inst1.reloc.type != BFD_RELOC_NONE)
	s3_fix_new_score (frag_now, p - frag_now->fr_literal,
			  backup_inst1.size, &backup_inst1.reloc.exp,
			  backup_inst1.reloc.pc_rel, backup_inst1.reloc.type);
    }

  /* relax_size may be 2, 4, 12 or 0, 0 indicates no relaxation.  */
  relaxable_p &= (backup_inst1.relax_size != 0);
  relax_size = relaxable_p ? backup_inst1.relax_size : 0;

  p = frag_var (rs_machine_dependent, relax_size + s3_RELAX_PAD_BYTE, 0,
                s3_RELAX_ENCODE (backup_inst1.size, backup_inst1.relax_size,
				 backup_inst1.type, 0, 0, relaxable_p),
                backup_inst1.reloc.exp.X_add_symbol, 0, NULL);

  if (relaxable_p)
    s3_md_number_to_chars (p, backup_inst1.relax_inst, relax_size);

  memcpy (inst1, &backup_inst1, sizeof (struct s3_score_it));
}

static void
s3_parse_16_32_inst (char *insnstr, bool gen_frag_p)
{
  char c;
  char *p;
  char *operator = insnstr;
  const struct s3_asm_opcode *opcode;

  /* Parse operator and operands.  */
  s3_skip_whitespace (operator);

  for (p = operator; *p != '\0'; p++)
    if ((*p == ' ') || (*p == '!'))
      break;

  if (*p == '!')
    p++;

  c = *p;
  *p = '\0';

  opcode = (const struct s3_asm_opcode *) str_hash_find (s3_score_ops_hsh,
							 operator);
  *p = c;

  memset (&s3_inst, '\0', sizeof (s3_inst));
  sprintf (s3_inst.str, "%s", insnstr);
  if (opcode)
    {
      s3_inst.instruction = opcode->value;
      s3_inst.relax_inst = opcode->relax_value;
      s3_inst.type = opcode->type;
      s3_inst.size = s3_GET_INSN_SIZE (s3_inst.type);
      s3_inst.relax_size = 0;
      s3_inst.bwarn = 0;
      sprintf (s3_inst.name, "%s", opcode->template_name);
      strcpy (s3_inst.reg, "");
      s3_inst.error = NULL;
      s3_inst.reloc.type = BFD_RELOC_NONE;

      (*opcode->parms) (p);

      /* It indicates current instruction is a macro instruction if s3_inst.bwarn equals -1.  */
      if ((s3_inst.bwarn != -1) && (!s3_inst.error) && (gen_frag_p))
	s3_gen_insn_frag (&s3_inst, NULL);
    }
  else
    s3_inst.error = _("unrecognized opcode");
}

static void
s3_parse_48_inst (char *insnstr, bool gen_frag_p)
{
  char c;
  char *p;
  char *operator = insnstr;
  const struct s3_asm_opcode *opcode;

  /* Parse operator and operands.  */
  s3_skip_whitespace (operator);

  for (p = operator; *p != '\0'; p++)
    if (*p == ' ')
      break;

  c = *p;
  *p = '\0';

  opcode = (const struct s3_asm_opcode *) str_hash_find (s3_score_ops_hsh,
							 operator);
  *p = c;

  memset (&s3_inst, '\0', sizeof (s3_inst));
  sprintf (s3_inst.str, "%s", insnstr);
  if (opcode)
    {
      s3_inst.instruction = opcode->value;
      s3_inst.relax_inst = opcode->relax_value;
      s3_inst.type = opcode->type;
      s3_inst.size = s3_GET_INSN_SIZE (s3_inst.type);
      s3_inst.relax_size = 0;
      s3_inst.bwarn = 0;
      sprintf (s3_inst.name, "%s", opcode->template_name);
      strcpy (s3_inst.reg, "");
      s3_inst.error = NULL;
      s3_inst.reloc.type = BFD_RELOC_NONE;

      (*opcode->parms) (p);

      /* It indicates current instruction is a macro instruction if s3_inst.bwarn equals -1.  */
      if ((s3_inst.bwarn != -1) && (!s3_inst.error) && (gen_frag_p))
	s3_gen_insn_frag (&s3_inst, NULL);
    }
  else
    s3_inst.error = _("unrecognized opcode");
}

static int
s3_append_insn (char *str, bool gen_frag_p)
{
  int retval = s3_SUCCESS;

  s3_parse_16_32_inst (str, gen_frag_p);

  if (s3_inst.error)
    {
      retval = (int) s3_FAIL;
      as_bad (_("%s -- `%s'"), s3_inst.error, s3_inst.str);
      s3_inst.error = NULL;
    }

  return retval;
}

static void
s3_do16_mv_cmp (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 5, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 0, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
}

static void
s3_do16_cmpi (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 5, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 0, _SIMM5) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
}

static void
s3_do16_addi (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reglow_required_here (&str, 6) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 0, _SIMM6) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
}

/* Handle bitclr! / bitset! / bittst! / bittgl! */
static void
s3_do16_rdi5 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reglow_required_here (&str, 5) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 0, _IMM5) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;
  else
    {
      s3_inst.relax_inst |= (((s3_inst.instruction >>5) & 0xf) << 20)
        | (((s3_inst.instruction >> 5) & 0xf) << 15) | (((s3_inst.instruction ) & 0x1f) << 10);
      s3_inst.relax_size = 4;
    }
}


/* Handle sdbbp!.  */
static void
s3_do16_xi5 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_data_op2 (&str, 0, _IMM5) == (int) s3_FAIL || s3_end_of_line (str) == (int) s3_FAIL)
    return;
}

/* Check that an immediate is word alignment or half word alignment.
   If so, convert it to the right format.  */
static int
s3_validate_immediate_align (int val, unsigned int data_type)
{
  if (data_type == _IMM5_RSHIFT_1)
    {
      if (val % 2)
        {
          s3_inst.error = _("address offset must be half word alignment");
          return (int) s3_FAIL;
        }
    }
  else if ((data_type == _IMM5_RSHIFT_2) || (data_type == _IMM10_RSHIFT_2))
    {
      if (val % 4)
        {
          s3_inst.error = _("address offset must be word alignment");
          return (int) s3_FAIL;
        }
    }

  return s3_SUCCESS;
}

static int
s3_exp_ldst_offset (char **str, int shift, unsigned int data_type)
{
  char *dataptr;

  dataptr = * str;

  if ((*dataptr == '0') && (*(dataptr + 1) == 'x')
      && (data_type != _SIMM16_LA)
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
      && (data_type != _SIMM14_NEG)
      && (data_type != _IMM10_RSHIFT_2))
    {
      data_type += 24;
    }

  if (s3_my_get_expression (&s3_inst.reloc.exp, str) == (int) s3_FAIL)
    return (int) s3_FAIL;

  if (s3_inst.reloc.exp.X_op == O_constant)
    {
      /* Need to check the immediate align.  */
      int value = s3_validate_immediate_align (s3_inst.reloc.exp.X_add_number, data_type);

      if (value == (int) s3_FAIL)
	return (int) s3_FAIL;

      value = s3_validate_immediate (s3_inst.reloc.exp.X_add_number, data_type, 0);
      if (value == (int) s3_FAIL)
        {
          if (data_type < 30)
            sprintf (s3_err_msg,
                     _("invalid constant: %d bit expression not in range %d..%d"),
                     s3_score_df_range[data_type].bits,
                     s3_score_df_range[data_type].range[0], s3_score_df_range[data_type].range[1]);
          else
            sprintf (s3_err_msg,
                     _("invalid constant: %d bit expression not in range %d..%d"),
                     s3_score_df_range[data_type - 24].bits,
                     s3_score_df_range[data_type - 24].range[0], s3_score_df_range[data_type - 24].range[1]);
          s3_inst.error = s3_err_msg;
          return (int) s3_FAIL;
        }

      if (data_type == _IMM5_RSHIFT_1)
        {
          value >>= 1;
        }
      else if ((data_type == _IMM5_RSHIFT_2) || (data_type == _IMM10_RSHIFT_2))
        {
          value >>= 2;
        }

      if (s3_score_df_range[data_type].range[0] != 0)
        {
          value &= (1 << s3_score_df_range[data_type].bits) - 1;
        }

      s3_inst.instruction |= value << shift;
    }
  else
    {
      s3_inst.reloc.pc_rel = 0;
    }

  return s3_SUCCESS;
}

static void
s3_do_ldst_insn (char *str)
{
  int pre_inc = 0;
  int conflict_reg;
  int value;
  char * temp;
  char *dataptr;
  int reg;
  int ldst_idx = 0;

  s3_skip_whitespace (str);

  if (((conflict_reg = s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL))
    return;

  /* ld/sw rD, [rA, simm15]    ld/sw rD, [rA]+, simm12     ld/sw rD, [rA, simm12]+.  */
  if (*str == '[')
    {
      str++;
      s3_skip_whitespace (str);

      if ((reg = s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
	return;

      /* Conflicts can occur on stores as well as loads.  */
      conflict_reg = (conflict_reg == reg);
      s3_skip_whitespace (str);
      temp = str + 1;    /* The latter will process decimal/hex expression.  */

      /* ld/sw rD, [rA]+, simm12    ld/sw rD, [rA]+.  */
      if (*str == ']')
        {
          str++;
          if (*str == '+')
            {
              str++;
              /* ld/sw rD, [rA]+, simm12.  */
              if (s3_skip_past_comma (&str) == s3_SUCCESS)
                {
                  if ((s3_exp_ldst_offset (&str, 3, _SIMM12) == (int) s3_FAIL)
                      || (s3_end_of_line (str) == (int) s3_FAIL))
		    return;

                  if (conflict_reg)
                    {
                      unsigned int ldst_func = s3_inst.instruction & OPC_PSEUDOLDST_MASK;

                      if ((ldst_func == INSN_LH)
                          || (ldst_func == INSN_LHU)
                          || (ldst_func == INSN_LW)
                          || (ldst_func == INSN_LB)
                          || (ldst_func == INSN_LBU))
                        {
                          s3_inst.error = _("register same as write-back base");
                          return;
                        }
                    }

                  ldst_idx = s3_inst.instruction & OPC_PSEUDOLDST_MASK;
                  s3_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
                  s3_inst.instruction |= s3_score_ldst_insns[ldst_idx * 3 + LDST_POST].value;

                  /* lw rD, [rA]+, 4 convert to pop rD, [rA].  */
                  if ((s3_inst.instruction & 0x3e000007) == 0x0e000000)
                    {
                      /* rs =  r0, offset = 4 */
                      if ((((s3_inst.instruction >> 15) & 0x1f) == 0)
                          && (((s3_inst.instruction >> 3) & 0xfff) == 4))
                        {
                          /* Relax to pop!.  */
                          s3_inst.relax_inst = 0x0040 | ((s3_inst.instruction >> 20) & 0x1f);
                          s3_inst.relax_size = 2;
                        }
                    }
                  return;
                }
              /* ld/sw rD, [rA]+ convert to ld/sw rD, [rA, 0]+.  */
              else
                {
                  s3_SET_INSN_ERROR (NULL);
                  if (s3_end_of_line (str) == (int) s3_FAIL)
                    {
                      return;
                    }

                  pre_inc = 1;
                  value = s3_validate_immediate (s3_inst.reloc.exp.X_add_number, _SIMM12, 0);
                  value &= (1 << s3_score_df_range[_SIMM12].bits) - 1;
                  ldst_idx = s3_inst.instruction & OPC_PSEUDOLDST_MASK;
                  s3_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
                  s3_inst.instruction |= s3_score_ldst_insns[ldst_idx * 3 + pre_inc].value;
                  s3_inst.instruction |= value << 3;
                  s3_inst.relax_inst = 0x8000;
                  return;
                }
            }
          /* ld/sw rD, [rA] convert to ld/sw rD, [rA, simm15].  */
          else
            {
              if (s3_end_of_line (str) == (int) s3_FAIL)
		return;

              ldst_idx = s3_inst.instruction & OPC_PSEUDOLDST_MASK;
              s3_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
              s3_inst.instruction |= s3_score_ldst_insns[ldst_idx * 3 + LDST_NOUPDATE].value;

              /* lbu rd, [rs] -> lbu! rd, [rs]  */
              if (ldst_idx == INSN_LBU)
                {
                  s3_inst.relax_inst = INSN16_LBU;
                }
              else if (ldst_idx == INSN_LH)
                {
                  s3_inst.relax_inst = INSN16_LH;
                }
              else if (ldst_idx == INSN_LW)
                {
                  s3_inst.relax_inst = INSN16_LW;
                }
              else if (ldst_idx == INSN_SB)
                {
                  s3_inst.relax_inst = INSN16_SB;
                }
              else if (ldst_idx == INSN_SH)
                {
                  s3_inst.relax_inst = INSN16_SH;
                }
              else if (ldst_idx == INSN_SW)
                {
                  s3_inst.relax_inst = INSN16_SW;
                }
              else
                {
                  s3_inst.relax_inst = 0x8000;
                }

              /* lw/lh/lbu/sw/sh/sb, offset = 0, relax to 16 bit instruction.  */
              /* if ((ldst_idx == INSN_LBU)
		 || (ldst_idx == INSN_LH)
		 || (ldst_idx == INSN_LW)
		 || (ldst_idx == INSN_SB) || (ldst_idx == INSN_SH) || (ldst_idx == INSN_SW))*/
              if ( (ldst_idx == INSN_LW)|| (ldst_idx == INSN_SW))
                {
                  /* ra only 3 bit , rd only 4 bit for lw! and sw! */
                  if ((((s3_inst.instruction >> 15) & 0x18) == 0) && (((s3_inst.instruction >> 20) & 0x10) == 0))
                    {
                      s3_inst.relax_inst |= (((s3_inst.instruction >> 20) & 0xf) << 8) |
                        (((s3_inst.instruction >> 15) & 0x7) << 5);
                      s3_inst.relax_size = 2;
                    }
                }

              return;
            }
        }
      /* ld/sw rD, [rA, simm15]    ld/sw rD, [rA, simm12]+.  */
      else
        {
          if (s3_skip_past_comma (&str) == (int) s3_FAIL)
            {
              s3_inst.error = _("pre-indexed expression expected");
              return;
            }

          if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL)
	    return;

          s3_skip_whitespace (str);
          if (*str++ != ']')
            {
              s3_inst.error = _("missing ]");
              return;
            }

          s3_skip_whitespace (str);
          /* ld/sw rD, [rA, simm12]+.  */
          if (*str == '+')
            {
              str++;
              pre_inc = 1;
              if (conflict_reg)
                {
                  unsigned int ldst_func = s3_inst.instruction & OPC_PSEUDOLDST_MASK;

                  if ((ldst_func == INSN_LH)
                      || (ldst_func == INSN_LHU)
                      || (ldst_func == INSN_LW)
                      || (ldst_func == INSN_LB)
                      || (ldst_func == INSN_LBU))
                    {
                      s3_inst.error = _("register same as write-back base");
                      return;
                    }
                }
            }

          if (s3_end_of_line (str) == (int) s3_FAIL)
	    return;

          if (s3_inst.reloc.exp.X_op == O_constant)
            {
              unsigned int data_type;

              if (pre_inc == 1)
                data_type = _SIMM12;
              else
                data_type = _SIMM15;
              dataptr = temp;

              if ((*dataptr == '0') && (*(dataptr + 1) == 'x')
                  && (data_type != _SIMM16_LA)
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
                  && (data_type != _SIMM14_NEG)
                  && (data_type != _IMM10_RSHIFT_2))
                {
                  data_type += 24;
                }

              value = s3_validate_immediate (s3_inst.reloc.exp.X_add_number, data_type, 0);
              if (value == (int) s3_FAIL)
                {
                  if (data_type < 30)
                    sprintf (s3_err_msg,
                             _("invalid constant: %d bit expression not in range %d..%d"),
                             s3_score_df_range[data_type].bits,
                             s3_score_df_range[data_type].range[0], s3_score_df_range[data_type].range[1]);
                  else
                    sprintf (s3_err_msg,
                             _("invalid constant: %d bit expression not in range %d..%d"),
                             s3_score_df_range[data_type - 24].bits,
                             s3_score_df_range[data_type - 24].range[0],
                             s3_score_df_range[data_type - 24].range[1]);
                  s3_inst.error = s3_err_msg;
                  return;
                }

              value &= (1 << s3_score_df_range[data_type].bits) - 1;
              ldst_idx = s3_inst.instruction & OPC_PSEUDOLDST_MASK;
              s3_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
              s3_inst.instruction |= s3_score_ldst_insns[ldst_idx * 3 + pre_inc].value;
              if (pre_inc == 1)
                s3_inst.instruction |= value << 3;
              else
                s3_inst.instruction |= value;

              /* lw rD, [rA, simm15]  */
              if ((s3_inst.instruction & 0x3e000000) == 0x20000000)
                {
                  /*  rD  in [r0 - r15]. , ra in [r0-r7] */
                  if ((((s3_inst.instruction >> 15) & 0x18) == 0)
                      && (((s3_inst.instruction >> 20) & 0x10) == 0))
                    {
                      /* simm = [bit 7], lw -> lw!.  */
                      if (((s3_inst.instruction & 0x7f80) == 0)&&((s3_inst.instruction &0x3)==0))
                        {
                          s3_inst.relax_inst |= (((s3_inst.instruction >> 15) & 0x7) << 5)
                            | (((s3_inst.instruction >> 20) & 0xf) << 8)|(value>>2);
                          s3_inst.relax_size = 2;
                        }
                      else
                        {
                          s3_inst.relax_inst = 0x8000;
                        }
                    }
                  else
                    {
                      s3_inst.relax_inst = 0x8000;
                    }
                }
              /* sw rD, [rA, simm15]  */
              else if ((s3_inst.instruction & 0x3e000000) == 0x28000000)
                {
                  /* rD is  in [r0 - r15] and ra in [r0-r7] */
                  if ((((s3_inst.instruction >> 15) & 0x18) == 0) && (((s3_inst.instruction >> 20) & 0x10) == 0))
                    {
                      /* simm15 =7 bit  , sw -> sw!.  */
                      if (((s3_inst.instruction & 0x7f80) == 0)&&((s3_inst.instruction &0x3)==0))
                        {
                          s3_inst.relax_inst |= (((s3_inst.instruction >> 15) & 0xf) << 5)
                            | (((s3_inst.instruction >> 20) & 0xf) << 8)|(value>>2);
                          s3_inst.relax_size = 2;
                        }
                      /* rA = r2, sw -> swp!.  */
                      else
                        {
                          s3_inst.relax_inst = 0x8000;
                        }
                    }
                  else
                    {
                      s3_inst.relax_inst = 0x8000;
                    }
                }
              /* sw rD, [rA, simm15]+    sw pre.  */
              else if ((s3_inst.instruction & 0x3e000007) == 0x06000004)
                {
                  /* simm15 = -4. and ra==r0 */
                  if ((((s3_inst.instruction >> 15) & 0x1f) == 0)
                      && (((s3_inst.instruction >> 3) & 0xfff) == 0xffc))
                    {
                      /* sw -> push!.  */
                      s3_inst.relax_inst = 0x0060 | ((s3_inst.instruction >> 20) & 0x1f);
                      s3_inst.relax_size = 2;
                    }
                  else
                    {
                      s3_inst.relax_inst = 0x8000;
                    }
                }
              else
                {
                  s3_inst.relax_inst = 0x8000;
                }

              return;
            }
          else
            {
              /* FIXME: may set error, for there is no ld/sw rD, [rA, label] */
              s3_inst.reloc.pc_rel = 0;
            }
        }
    }
  else
    {
      s3_inst.error = s3_BAD_ARGS;
    }
}

/* Handle cache.  */
static void
s3_do_cache (char *str)
{
  s3_skip_whitespace (str);

  if ((s3_data_op2 (&str, 20, _IMM5) == (int) s3_FAIL) || (s3_skip_past_comma (&str) == (int) s3_FAIL))
    {
      return;
    }
  else
    {
      int cache_op;

      cache_op = (s3_inst.instruction >> 20) & 0x1F;
      sprintf (s3_inst.name, "cache %d", cache_op);
    }

  if (*str == '[')
    {
      str++;
      s3_skip_whitespace (str);

      if (s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
	return;

      s3_skip_whitespace (str);

      /* cache op, [rA]  */
      if (s3_skip_past_comma (&str) == (int) s3_FAIL)
        {
          s3_SET_INSN_ERROR (NULL);
          if (*str != ']')
            {
              s3_inst.error = _("missing ]");
              return;
            }
          str++;
        }
      /* cache op, [rA, simm15]  */
      else
        {
          if (s3_exp_ldst_offset (&str, 0, _SIMM15) == (int) s3_FAIL)
            {
              return;
            }

          s3_skip_whitespace (str);
          if (*str++ != ']')
            {
              s3_inst.error = _("missing ]");
              return;
            }
        }

      if (s3_end_of_line (str) == (int) s3_FAIL)
	return;
    }
  else
    {
      s3_inst.error = s3_BAD_ARGS;
    }
}

static void
s3_do_crdcrscrsimm5 (char *str)
{
  char *strbak;

  strbak = str;
  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE_CR) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE_CR) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE_CR) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL)
    {
      str = strbak;
      /* cop1 cop_code20.  */
      if (s3_data_op2 (&str, 5, _IMM20) == (int) s3_FAIL)
	return;
    }
  else
    {
      if (s3_data_op2 (&str, 5, _IMM5) == (int) s3_FAIL)
	return;
    }

  s3_end_of_line (str);
}

/* Handle ldc/stc.  */
static void
s3_do_ldst_cop (char *str)
{
  s3_skip_whitespace (str);

  if ((s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE_CR) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL))
    return;

  if (*str == '[')
    {
      str++;
      s3_skip_whitespace (str);

      if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
	return;

      s3_skip_whitespace (str);

      if (*str++ != ']')
        {
          if (s3_exp_ldst_offset (&str, 5, _IMM10_RSHIFT_2) == (int) s3_FAIL)
	    return;

          s3_skip_whitespace (str);
          if (*str++ != ']')
            {
              s3_inst.error = _("missing ]");
              return;
            }
        }

      s3_end_of_line (str);
    }
  else
    s3_inst.error = s3_BAD_ARGS;
}

static void
s3_do16_ldst_insn (char *str)
{
  int conflict_reg = 0;
  s3_skip_whitespace (str);

  if ((s3_reglow_required_here (&str, 8) == (int) s3_FAIL) || (s3_skip_past_comma (&str) == (int) s3_FAIL))
    return;

  if (*str == '[')
    {

      str++;
      s3_skip_whitespace (str);

      if ((conflict_reg = s3_reglow_required_here (&str, 5)) == (int) s3_FAIL)
	return;
      if (conflict_reg&0x8)
        {
          sprintf (s3_err_msg,  _("invalid register number: %d is not in [r0--r7]"),conflict_reg);
          s3_inst.error = s3_err_msg;
          return;
        }

      s3_skip_whitespace (str);

      if (*str == ']')
        {
          str++;
          if (s3_end_of_line (str) == (int) s3_FAIL)
	    return;
        }
      else
        {
          if (s3_skip_past_comma (&str) == (int) s3_FAIL)
	    {
	      s3_inst.error = _("comma is  expected");
              return;
	    }
          if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL)
	    return;
          s3_skip_whitespace (str);
	  if (*str++ != ']')
	    {
	      s3_inst.error = _("missing ]");
	      return;
	    }
          if (s3_end_of_line (str) == (int) s3_FAIL)
	    return;
          if (s3_inst.reloc.exp.X_op == O_constant)
            {
              int value;
	      unsigned int data_type;
              data_type = _IMM5_RSHIFT_2;
              value = s3_validate_immediate (s3_inst.reloc.exp.X_add_number, data_type, 0);
              if (value == (int) s3_FAIL)
		{
		  if (data_type < 30)
		    sprintf (s3_err_msg,
			     _("invalid constant: %d bit expression not in range %d..%d"),
			     s3_score_df_range[data_type].bits,
			     s3_score_df_range[data_type].range[0], s3_score_df_range[data_type].range[1]);
                  s3_inst.error = s3_err_msg;
	          return;
	        }
              if (value & 0x3)
                {
                  sprintf (s3_err_msg,  _("invalid constant: %d is not word align integer"),value);
                  s3_inst.error = s3_err_msg;
                  return;
                }

              value >>= 2;
              s3_inst.instruction |= value;
            }
        }
    }
  else
    {
      sprintf (s3_err_msg,  _("missing ["));
      s3_inst.error = s3_err_msg;
      return;
    }
}

static void
s3_do_lw48 (char *str)
{
  bfd_signed_vma val = 0;

  s3_skip_whitespace (str);

  if ((s3_reg_required_here (&str, 37, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL))
    return;

  if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }

  /* Check word align for lw48 rd, value.  */
  if ((s3_inst.reloc.exp.X_add_symbol == NULL)
      && ((s3_inst.reloc.exp.X_add_number & 0x3) != 0))
    {
      s3_inst.error = _("invalid constant: 32 bit expression not word align");
      return;
    }

  /* Check and set offset.  */
  val = s3_inst.reloc.exp.X_add_number;
  if ((s3_inst.reloc.exp.X_add_symbol == NULL)
      && (!(val >= 0 && val <= 0xffffffffLL)))
    {
      s3_inst.error = _("invalid constant: 32 bit expression not in range [0, 0xffffffff]");
      return;
    }

  val &= 0xffffffff;
  val >>= 2;
  s3_inst.instruction |= (val << 7);

  /* Set reloc type.  */
  s3_inst.reloc.type = BFD_RELOC_SCORE_IMM30;

}

static void
s3_do_sw48 (char *str)
{
  bfd_signed_vma val = 0;

  s3_skip_whitespace (str);

  if ((s3_reg_required_here (&str, 37, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL))
    return;

  if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }

  /* Check word align for lw48 rd, value.  */
  if ((s3_inst.reloc.exp.X_add_symbol == NULL)
      && ((s3_inst.reloc.exp.X_add_number & 0x3) != 0))
    {
      s3_inst.error = _("invalid constant: 32 bit expression not word align");
      return;
    }

  /* Check and set offset.  */
  val = s3_inst.reloc.exp.X_add_number;
  if ((s3_inst.reloc.exp.X_add_symbol == NULL)
      && (!(val >= 0 && val <= 0xffffffffLL)))
    {
      s3_inst.error = _("invalid constant: 32 bit expression not in range [0, 0xffffffff]");
      return;
    }

  val &= 0xffffffff;
  val >>= 2;
  s3_inst.instruction |= (val << 7);

  /* Set reloc type.  */
  s3_inst.reloc.type = BFD_RELOC_SCORE_IMM30;
}

static void
s3_do_ldi48 (char *str)
{
  bfd_signed_vma val;

  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 37, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL)
    return;

  if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }

  /* Check and set offset.  */
  val = s3_inst.reloc.exp.X_add_number;
  if (!(val >= -0xffffffffLL && val <= 0xffffffffLL))
    {
      s3_inst.error = _("invalid constant: 32 bit expression not in range [-0x80000000, 0x7fffffff]");
      return;
    }

  val &= 0xffffffff;
  s3_inst.instruction |= (val << 5);

  /* Set reloc type.  */
  s3_inst.reloc.type = BFD_RELOC_SCORE_IMM32;
}

static void
s3_do_sdbbp48 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_data_op2 (&str, 5, _IMM5) == (int) s3_FAIL || s3_end_of_line (str) == (int) s3_FAIL)
    return;
}

static void
s3_do_and48 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reglow_required_here (&str, 38) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reglow_required_here (&str, 34) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 2, _IMM32) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;
}

static void
s3_do_or48 (char *str)
{
  s3_skip_whitespace (str);

  if (s3_reglow_required_here (&str, 38) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reglow_required_here (&str, 34) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 2, _IMM32) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;
}

static void
s3_do_mbitclr (char *str)
{
  int val;
  s3_skip_whitespace (str);

  if (*str != '[')
    {
      sprintf (s3_err_msg,  _("missing ["));
      s3_inst.error = s3_err_msg;
      return;
    }
  str++;

  s3_inst.instruction &= 0x0;

  if ((s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL)
      || (s3_data_op2 (&str, 0, _IMM11) == (int) s3_FAIL))
    return;

  /* Get imm11 and refill opcode.  */
  val = s3_inst.instruction & 0x7ff;
  val >>= 2;
  s3_inst.instruction &= 0x000f8000;
  s3_inst.instruction |= 0x00000064;

  if (*str != ']')
    {
      sprintf (s3_err_msg,  _("missing ]"));
      s3_inst.error = s3_err_msg;
      return;
    }
  str++;

  if ((s3_skip_past_comma (&str) == (int) s3_FAIL)
      || (s3_data_op2 (&str, 10, _IMM5) == (int) s3_FAIL))
    return;

  /* Set imm11 to opcode.  */
  s3_inst.instruction |= (val & 0x1)
    | (((val >> 1 ) & 0x7) << 7)
    | (((val >> 4 ) & 0x1f) << 20);
}

static void
s3_do_mbitset (char *str)
{
  int val;
  s3_skip_whitespace (str);

  if (*str != '[')
    {
      sprintf (s3_err_msg,  _("missing ["));
      s3_inst.error = s3_err_msg;
      return;
    }
  str++;

  s3_inst.instruction &= 0x0;

  if ((s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL)
      || (s3_data_op2 (&str, 0, _IMM11) == (int) s3_FAIL))
    return;

  /* Get imm11 and refill opcode.  */
  val = s3_inst.instruction & 0x7ff;
  val >>= 2;
  s3_inst.instruction &= 0x000f8000;
  s3_inst.instruction |= 0x0000006c;

  if (*str != ']')
    {
      sprintf (s3_err_msg,  _("missing ]"));
      s3_inst.error = s3_err_msg;
      return;
    }
  str++;

  if ((s3_skip_past_comma (&str) == (int) s3_FAIL)
      || (s3_data_op2 (&str, 10, _IMM5) == (int) s3_FAIL))
    return;

  /* Set imm11 to opcode.  */
  s3_inst.instruction |= (val & 0x1)
    | (((val >> 1 ) & 0x7) << 7)
    | (((val >> 4 ) & 0x1f) << 20);
}

static void
s3_do16_slli_srli (char *str)
{
  s3_skip_whitespace (str);

  if ((s3_reglow_required_here (&str, 5) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL)
      || s3_data_op2 (&str, 0, _IMM5) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;
}

static void
s3_do16_ldiu (char *str)
{
  s3_skip_whitespace (str);

  if ((s3_reg_required_here (&str, 5,s3_REG_TYPE_SCORE) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL)
      || s3_data_op2 (&str, 0, _IMM5) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;
}

static void
s3_do16_push_pop (char *str)
{
  s3_skip_whitespace (str);
  if ((s3_reg_required_here (&str, 0, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;
}

static void
s3_do16_rpush (char *str)
{
  int reg;
  int val;
  s3_skip_whitespace (str);
  if ((reg = (s3_reg_required_here (&str, 5, s3_REG_TYPE_SCORE))) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 0, _IMM5_MULTI_LOAD) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  /* 0: indicate 32.
     1: invalid value.
     2: to 31: normal value.  */
  val = s3_inst.instruction & 0x1f;
  if (val == 1)
    {
      s3_inst.error = _("imm5 should >= 2");
      return;
    }
  if (reg >= 32)
    {
      s3_inst.error = _("reg should <= 31");
      return;
    }
}

static void
s3_do16_rpop (char *str)
{
  int reg;
  int val;
  s3_skip_whitespace (str);
  if ((reg = (s3_reg_required_here (&str, 5, s3_REG_TYPE_SCORE))) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_data_op2 (&str, 0, _IMM5_MULTI_LOAD) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  /* 0: indicate 32.
     1: invalid value.
     2: to 31: normal value.  */
  val = s3_inst.instruction & 0x1f;
  if (val == 1)
    {
      s3_inst.error = _("imm5 should >= 2");
      return;
    }

  if (reg >= 32)
    {
      s3_inst.error = _("reg should <= 31");
      return;
    }
  else
    {
      if ((reg + val) <= 32)
        reg = reg + val - 1;
      else
        reg = reg + val - 33;
      s3_inst.instruction &= 0x7c1f;
      s3_inst.instruction |= (reg << 5);
      return;
    }
}

/* Handle lcb/lcw/lce/scb/scw/sce.  */
static void
s3_do_ldst_unalign (char *str)
{
  int conflict_reg;

  if (s3_university_version == 1)
    {
      s3_inst.error = s3_ERR_FOR_SCORE5U_ATOMIC;
      return;
    }

  s3_skip_whitespace (str);

  /* lcb/scb [rA]+.  */
  if (*str == '[')
    {
      str++;
      s3_skip_whitespace (str);

      if (s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
	return;

      if (*str++ == ']')
        {
          if (*str++ != '+')
            {
              s3_inst.error = _("missing +");
              return;
            }
        }
      else
        {
          s3_inst.error = _("missing ]");
          return;
        }

      if (s3_end_of_line (str) == (int) s3_FAIL)
	return;
    }
  /* lcw/lce/scb/sce rD, [rA]+.  */
  else
    {
      if (((conflict_reg = s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
          || (s3_skip_past_comma (&str) == (int) s3_FAIL))
        {
          return;
        }

      s3_skip_whitespace (str);
      if (*str++ == '[')
        {
          int reg;

          s3_skip_whitespace (str);
          if ((reg = s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
            {
              return;
            }

          /* Conflicts can occur on stores as well as loads.  */
          conflict_reg = (conflict_reg == reg);
          s3_skip_whitespace (str);
          if (*str++ == ']')
            {
              unsigned int ldst_func = s3_inst.instruction & LDST_UNALIGN_MASK;

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
                  s3_inst.error = _("missing +");
                  return;
                }

              if (s3_end_of_line (str) == (int) s3_FAIL)
		return;
            }
          else
            {
              s3_inst.error = _("missing ]");
              return;
            }
        }
      else
        {
          s3_inst.error = s3_BAD_ARGS;
          return;
        }
    }
}

/* Handle alw/asw.  */
static void
s3_do_ldst_atomic (char *str)
{
  if (s3_university_version == 1)
    {
      s3_inst.error = s3_ERR_FOR_SCORE5U_ATOMIC;
      return;
    }

  s3_skip_whitespace (str);

  if ((s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL))
    {
      return;
    }
  else
    {

      s3_skip_whitespace (str);
      if (*str++ == '[')
        {
          int reg;

          s3_skip_whitespace (str);
          if ((reg = s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
            {
              return;
            }

          s3_skip_whitespace (str);
          if (*str++ != ']')
            {
              s3_inst.error = _("missing ]");
              return;
            }

          s3_end_of_line (str);
        }
      else
	s3_inst.error = s3_BAD_ARGS;
    }
}

static void
s3_build_relax_frag (struct s3_score_it fix_insts[s3_RELAX_INST_NUM], int fix_num ATTRIBUTE_UNUSED,
		     struct s3_score_it var_insts[s3_RELAX_INST_NUM], int var_num,
		     symbolS *add_symbol)
{
  int i;
  char *p;
  fixS *fixp = NULL;
  fixS *cur_fixp = NULL;
  long where;
  struct s3_score_it inst_main;

  memcpy (&inst_main, &fix_insts[0], sizeof (struct s3_score_it));

  /* Adjust instruction opcode and to be relaxed instruction opcode.  */
  inst_main.instruction = s3_adjust_paritybit (inst_main.instruction, s3_GET_INSN_CLASS (inst_main.type));
  inst_main.type = Insn_PIC;

  for (i = 0; i < var_num; i++)
    {
      inst_main.relax_size += var_insts[i].size;
      var_insts[i].instruction = s3_adjust_paritybit (var_insts[i].instruction,
						      s3_GET_INSN_CLASS (var_insts[i].type));
    }

  /* Check data dependency.  */
  s3_handle_dependency (&inst_main);

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
  s3_md_number_to_chars (p, inst_main.instruction, inst_main.size);

  if (inst_main.reloc.type != BFD_RELOC_NONE)
    fixp = s3_fix_new_score (frag_now, p - frag_now->fr_literal, inst_main.size,
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
          fixp = s3_fix_new_score (frag_now, where, var_insts[i].size,
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

  p = frag_var (rs_machine_dependent, inst_main.relax_size + s3_RELAX_PAD_BYTE, 0,
                s3_RELAX_ENCODE (inst_main.size, inst_main.relax_size, inst_main.type,
				 0, inst_main.size, 0), add_symbol, 0, NULL);

  /* Write fr_var part.
     no calling s3_gen_insn_frag, no fixS will be generated.  */
  for (i = 0; i < var_num; i++)
    {
      s3_md_number_to_chars (p, var_insts[i].instruction, var_insts[i].size);
      p += var_insts[i].size;
    }
  /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
  s3_inst.bwarn = -1;
}

/* Build a relax frag for la instruction when generating s3_PIC,
   external symbol first and local symbol second.  */
static void
s3_build_la_pic (int reg_rd, expressionS exp)
{
  symbolS *add_symbol = exp.X_add_symbol;
  offsetT add_number = exp.X_add_number;
  struct s3_score_it fix_insts[s3_RELAX_INST_NUM];
  struct s3_score_it var_insts[s3_RELAX_INST_NUM];
  int fix_num = 0;
  int var_num = 0;
  char tmp[s3_MAX_LITERAL_POOL_SIZE];
  int r1_bak;

  r1_bak = s3_nor1;
  s3_nor1 = 0;

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
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
	return;

      if (reg_rd == s3_PIC_CALL_REG)
        s3_inst.reloc.type = BFD_RELOC_SCORE_CALL15;
      memcpy (&fix_insts[0], &s3_inst, sizeof (struct s3_score_it));

      /* Var part
	 For a local symbol :
         lw rD, <sym>($gp)    (BFD_RELOC_SCORE_GOT15)
	 addi rD, <sym>       (BFD_RELOC_GOT_LO16) */
      s3_inst.reloc.type = BFD_RELOC_SCORE_GOT15;
      memcpy (&var_insts[0], &s3_inst, sizeof (struct s3_score_it));
      sprintf (tmp, "addi_s_pic r%d, %s", reg_rd, S_GET_NAME (add_symbol));
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
	return;

      memcpy (&var_insts[1], &s3_inst, sizeof (struct s3_score_it));
      s3_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);
    }
  else if (add_number >= -0x8000 && add_number <= 0x7fff)
    {
      /* Insn 1: lw rD, <sym>($gp)    (BFD_RELOC_SCORE_GOT15)  */
      sprintf (tmp, "lw_pic r%d, %s", reg_rd, S_GET_NAME (add_symbol));
      if (s3_append_insn (tmp, true) == (int) s3_FAIL)
	return;

      /* Insn 2  */
      fix_num = 1;
      var_num = 1;
      /* Fix part
         For an external symbol: addi rD, <constant> */
      sprintf (tmp, "addi r%d, %d", reg_rd, (int)add_number);
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
	return;

      memcpy (&fix_insts[0], &s3_inst, sizeof (struct s3_score_it));

      /* Var part
 	 For a local symbol: addi rD, <sym>+<constant>    (BFD_RELOC_GOT_LO16)  */
      sprintf (tmp, "addi_s_pic r%d, %s + %d", reg_rd,
	       S_GET_NAME (add_symbol), (int) add_number);
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
	return;

      memcpy (&var_insts[0], &s3_inst, sizeof (struct s3_score_it));
      s3_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);
    }
  else
    {
      int hi = (add_number >> 16) & 0x0000FFFF;
      int lo = add_number & 0x0000FFFF;

      /* Insn 1: lw rD, <sym>($gp)    (BFD_RELOC_SCORE_GOT15)  */
      sprintf (tmp, "lw_pic r%d, %s", reg_rd, S_GET_NAME (add_symbol));
      if (s3_append_insn (tmp, true) == (int) s3_FAIL)
	return;

      /* Insn 2  */
      fix_num = 1;
      var_num = 1;
      /* Fix part
	 For an external symbol: ldis r1, HI%<constant>  */
      sprintf (tmp, "ldis r1, %d", hi);
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
	return;

      memcpy (&fix_insts[0], &s3_inst, sizeof (struct s3_score_it));

      /* Var part
	 For a local symbol: ldis r1, HI%<constant>
         but, if lo is out of 16 bit, make hi plus 1  */
      if ((lo < -0x8000) || (lo > 0x7fff))
	{
	  hi += 1;
	}
      sprintf (tmp, "ldis_pic r1, %d", hi);
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
	return;

      memcpy (&var_insts[0], &s3_inst, sizeof (struct s3_score_it));
      s3_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);

      /* Insn 3  */
      fix_num = 1;
      var_num = 1;
      /* Fix part
	 For an external symbol: ori r1, LO%<constant>  */
      sprintf (tmp, "ori r1, %d", lo);
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
	return;

      memcpy (&fix_insts[0], &s3_inst, sizeof (struct s3_score_it));

      /* Var part
  	 For a local symbol: addi r1, <sym>+LO%<constant>    (BFD_RELOC_GOT_LO16)  */
      sprintf (tmp, "addi_u_pic r1, %s + %d", S_GET_NAME (add_symbol), lo);
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
	return;

      memcpy (&var_insts[0], &s3_inst, sizeof (struct s3_score_it));
      s3_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);

      /* Insn 4: add rD, rD, r1  */
      sprintf (tmp, "add r%d, r%d, r1", reg_rd, reg_rd);
      if (s3_append_insn (tmp, true) == (int) s3_FAIL)
	return;

      /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
      s3_inst.bwarn = -1;
    }

  s3_nor1 = r1_bak;
}

/* Handle la.  */
static void
s3_do_macro_la_rdi32 (char *str)
{
  int reg_rd;

  s3_skip_whitespace (str);
  if ((reg_rd = s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL)
    {
      return;
    }
  else
    {
      /* Save str.  */
      char *keep_data = str;
      char append_str[s3_MAX_LITERAL_POOL_SIZE];

      /* Check immediate value.  */
      if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL)
        {
          s3_inst.error = _("expression error");
          return;
        }
      else if ((s3_inst.reloc.exp.X_add_symbol == NULL)
               && (s3_validate_immediate (s3_inst.reloc.exp.X_add_number, _IMM32, 0) == (int) s3_FAIL))
        {
          s3_inst.error = _("value not in range [0, 0xffffffff]");
          return;
        }

      /* Reset str.  */
      str = keep_data;

      /* la rd, simm16.  */
      if (s3_data_op2 (&str, 1, _SIMM16_LA) != (int) s3_FAIL)
        {
          s3_end_of_line (str);
          return;
        }
      /* la rd, imm32 or la rd, label.  */
      else
        {
          s3_SET_INSN_ERROR (NULL);
          /* Reset str.  */
          str = keep_data;
          if ((s3_data_op2 (&str, 1, _VALUE_HI16) == (int) s3_FAIL)
              || (s3_end_of_line (str) == (int) s3_FAIL))
            {
              return;
            }
          else
            {
              if ((s3_score_pic == s3_NO_PIC) || (!s3_inst.reloc.exp.X_add_symbol))
                {
                  sprintf (append_str, "ld_i32hi r%d, %s", reg_rd, keep_data);
                  if (s3_append_insn (append_str, true) == (int) s3_FAIL)
		    return;

                  sprintf (append_str, "ld_i32lo r%d, %s", reg_rd, keep_data);
                  if (s3_append_insn (append_str, true) == (int) s3_FAIL)
		    return;
		}
	      else
		{
		  gas_assert (s3_inst.reloc.exp.X_add_symbol);
		  s3_build_la_pic (reg_rd, s3_inst.reloc.exp);
		}

              /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
              s3_inst.bwarn = -1;
            }
        }
    }
}

/* Handle li.  */
static void
s3_do_macro_li_rdi32 (char *str)
{

  int reg_rd;

  s3_skip_whitespace (str);
  if ((reg_rd = s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL)
    {
      return;
    }
  else
    {
      /* Save str.  */
      char *keep_data = str;

      /* Check immediate value.  */
      if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL)
        {
          s3_inst.error = _("expression error");
          return;
        }
      else if (!(s3_inst.reloc.exp.X_add_number >= -0xffffffffLL
                 && s3_inst.reloc.exp.X_add_number <= 0xffffffffLL))
        {
          s3_inst.error = _("value not in range [-0xffffffff, 0xffffffff]");
          return;
        }

      /* Reset str.  */
      str = keep_data;

      /* li rd, simm16.  */
      if (s3_data_op2 (&str, 1, _SIMM16_LA) != (int) s3_FAIL)
        {
          s3_end_of_line (str);
          return;
        }
      /* li rd, imm32.  */
      else
        {
          char append_str[s3_MAX_LITERAL_POOL_SIZE];

          /* Reset str.  */
          str = keep_data;

          if ((s3_data_op2 (&str, 1, _VALUE_HI16) == (int) s3_FAIL)
              || (s3_end_of_line (str) == (int) s3_FAIL))
            {
              return;
            }
          else if (s3_inst.reloc.exp.X_add_symbol)
            {
              s3_inst.error = _("li rd label isn't correct instruction form");
              return;
            }
          else
            {
              sprintf (append_str, "ld_i32hi r%d, %s", reg_rd, keep_data);

              if (s3_append_insn (append_str, true) == (int) s3_FAIL)
		return;
              else
                {
                  sprintf (append_str, "ld_i32lo r%d, %s", reg_rd, keep_data);
                  if (s3_append_insn (append_str, true) == (int) s3_FAIL)
		    return;

                  /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
                  s3_inst.bwarn = -1;
                }
            }
        }
    }
}

/* Handle mul/mulu/div/divu/rem/remu.  */
static void
s3_do_macro_mul_rdrsrs (char *str)
{
  int reg_rd;
  int reg_rs1;
  int reg_rs2;
  char *backupstr;
  char append_str[s3_MAX_LITERAL_POOL_SIZE];

  if (s3_university_version == 1)
    as_warn ("%s", s3_ERR_FOR_SCORE5U_MUL_DIV);

  strcpy (append_str, str);
  backupstr = append_str;
  s3_skip_whitespace (backupstr);
  if (((reg_rd = s3_reg_required_here (&backupstr, -1, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
      || (s3_skip_past_comma (&backupstr) == (int) s3_FAIL)
      || ((reg_rs1 = s3_reg_required_here (&backupstr, -1, s3_REG_TYPE_SCORE)) == (int) s3_FAIL))
    {
      s3_inst.error = s3_BAD_ARGS;
      return;
    }

  if (s3_skip_past_comma (&backupstr) == (int) s3_FAIL)
    {
      /* rem/remu rA, rB is error format.  */
      if (strcmp (s3_inst.name, "rem") == 0 || strcmp (s3_inst.name, "remu") == 0)
        {
          s3_SET_INSN_ERROR (s3_BAD_ARGS);
        }
      else
        {
          s3_SET_INSN_ERROR (NULL);
          s3_do_rsrs (str);
        }
      return;
    }
  else
    {
      s3_SET_INSN_ERROR (NULL);
      if (((reg_rs2 = s3_reg_required_here (&backupstr, -1, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
          || (s3_end_of_line (backupstr) == (int) s3_FAIL))
        {
          return;
        }
      else
        {
          char append_str1[s3_MAX_LITERAL_POOL_SIZE];

          if (strcmp (s3_inst.name, "rem") == 0)
            {
              sprintf (append_str, "mul r%d, r%d", reg_rs1, reg_rs2);
              sprintf (append_str1, "mfceh  r%d", reg_rd);
            }
          else if (strcmp (s3_inst.name, "remu") == 0)
            {
              sprintf (append_str, "mulu r%d, r%d", reg_rs1, reg_rs2);
              sprintf (append_str1, "mfceh  r%d", reg_rd);
            }
          else
            {
              sprintf (append_str, "%s r%d, r%d", s3_inst.name, reg_rs1, reg_rs2);
              sprintf (append_str1, "mfcel  r%d", reg_rd);
            }

          /* Output mul/mulu or div/divu or rem/remu.  */
          if (s3_append_insn (append_str, true) == (int) s3_FAIL)
	    return;

          /* Output mfcel or mfceh.  */
          if (s3_append_insn (append_str1, true) == (int) s3_FAIL)
	    return;

          /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
          s3_inst.bwarn = -1;
        }
    }
}

static void
s3_exp_macro_ldst_abs (char *str)
{
  int reg_rd;
  char *backupstr, *tmp;
  char append_str[s3_MAX_LITERAL_POOL_SIZE];
  char verifystr[s3_MAX_LITERAL_POOL_SIZE];
  struct s3_score_it inst_backup;
  int r1_bak = 0;

  r1_bak = s3_nor1;
  s3_nor1 = 0;
  memcpy (&inst_backup, &s3_inst, sizeof (struct s3_score_it));

  strcpy (verifystr, str);
  backupstr = verifystr;
  s3_skip_whitespace (backupstr);
  if ((reg_rd = s3_reg_required_here (&backupstr, -1, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
    return;

  tmp = backupstr;
  if (s3_skip_past_comma (&backupstr) == (int) s3_FAIL)
    return;

  backupstr = tmp;
  sprintf (append_str, "li r1  %s", backupstr);
  s3_append_insn (append_str, true);

  memcpy (&s3_inst, &inst_backup, sizeof (struct s3_score_it));
  sprintf (append_str, " r%d, [r1,0]", reg_rd);
  s3_do_ldst_insn (append_str);

  s3_nor1 = r1_bak;
}

/* Handle bcmpeq / bcmpne  */
static void
s3_do_macro_bcmp (char *str)
{
  int reg_a , reg_b;
  char *keep_data;
  size_t keep_data_size;
  int i;
  struct s3_score_it inst_expand[2];
  struct s3_score_it inst_main;

  memset (inst_expand, 0, sizeof inst_expand);
  s3_skip_whitespace (str);
  if (( reg_a = s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      ||(reg_b = s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL)
    return;

  keep_data_size = strlen (str) + 1;
  keep_data = xmalloc (keep_data_size * 2 + 14);
  memcpy (keep_data, str, keep_data_size);

  if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL
      ||reg_b == 0
      || s3_end_of_line (str) == (int) s3_FAIL)
    goto out;
  else if (s3_inst.reloc.exp.X_add_symbol == 0)
    {
      s3_inst.error = _("lacking label  ");
      goto out;
    }
  else
    {
      char *append_str = keep_data + keep_data_size;
      s3_SET_INSN_ERROR (NULL);

      s3_inst.reloc.type = BFD_RELOC_SCORE_BCMP;
      s3_inst.reloc.pc_rel = 1;
      bfd_signed_vma val = s3_inst.reloc.exp.X_add_number;

      /* Branch 32  offset field : 20 bit, 16 bit branch offset field : 8 bit.  */
      s3_inst.instruction |= ((s3_inst.reloc.exp.X_add_number >> 1) & 0x1)
	| ((s3_inst.reloc.exp.X_add_number >> 2) & 0x7) << 7
	| ((s3_inst.reloc.exp.X_add_number >> 5) & 0x1f) << 20;

      /* Check and set offset.  */
      if (((val & 0xfffffe00) != 0)
	  && ((val & 0xfffffe00) != 0xfffffe00))
        {
          /* support bcmp --> cmp!+beq (bne) */
          if (s3_score_pic == s3_NO_PIC)
            {
	      sprintf (append_str, "cmp! r%d, r%d", reg_a, reg_b);
	      if (s3_append_insn (append_str, true) == (int) s3_FAIL)
		goto out;
	      if ((inst_main.instruction & 0x3e00007e) == 0x0000004c)
		memcpy (append_str, "beq ", 4);
	      else
		memcpy (append_str, "bne ", 4);
	      memmove (append_str + 4, keep_data, strlen (keep_data) + 1);
	      if (s3_append_insn (append_str, true) == (int) s3_FAIL)
		goto out;
	    }
	  else
	    {
	      gas_assert (s3_inst.reloc.exp.X_add_symbol);
	    }
	  /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
	  s3_inst.bwarn = -1;
	  goto out;
        }
      else
        {
          val >>= 1;
          s3_inst.instruction |= (val & 0x1)
	    | (((val >> 1) & 0x7) << 7)
	    | (((val >> 4) & 0x1f) << 20);
        }

      /* Backup s3_inst.  */
      memcpy (&inst_main, &s3_inst, sizeof (struct s3_score_it));

      if (s3_score_pic == s3_NO_PIC)
        {
	  sprintf (append_str, "cmp! r%d, r%d", reg_a, reg_b);
	  if (s3_append_insn (append_str, false) == (int) s3_FAIL)
	    goto out;
	  memcpy (&inst_expand[0], &s3_inst, sizeof (struct s3_score_it));

	  if ((inst_main.instruction & 0x3e00007e) == 0x0000004c)
	    memcpy (append_str, "beq ", 4);
	  else
	    memcpy (append_str, "bne ", 4);
	  memmove (append_str + 4, keep_data, strlen (keep_data) + 1);
	  if (s3_append_insn (append_str, false) == (int) s3_FAIL)
	    goto out;
	  memcpy (&inst_expand[1], &s3_inst, sizeof (struct s3_score_it));
        }
      else
        {
          gas_assert (s3_inst.reloc.exp.X_add_symbol);
        }
      inst_main.relax_size = inst_expand[0].size + inst_expand[1].size;
      inst_main.type = Insn_BCMP;

      /* Adjust instruction opcode and to be relaxed instruction opcode.  */
      inst_main.instruction = s3_adjust_paritybit (inst_main.instruction, s3_GET_INSN_CLASS (inst_main.type));

      for (i = 0; i < 2; i++)
        inst_expand[i].instruction = s3_adjust_paritybit (inst_expand[i].instruction,
                                                          s3_GET_INSN_CLASS (inst_expand[i].type));
      /* Check data dependency.  */
      s3_handle_dependency (&inst_main);
      /* Start a new frag if frag_now is not empty.  */
      if (frag_now_fix () != 0)
	{
	  if (!frag_now->tc_frag_data.is_insn)
	    frag_wane (frag_now);
	  frag_new (0);
	}
      frag_grow (20);

      /* Write fr_fix part.  */
      char *p;
      p = frag_more (inst_main.size);
      s3_md_number_to_chars (p, inst_main.instruction, inst_main.size);

      if (inst_main.reloc.type != BFD_RELOC_NONE)
	{
	  s3_fix_new_score (frag_now, p - frag_now->fr_literal, inst_main.size,
			    &inst_main.reloc.exp, inst_main.reloc.pc_rel, inst_main.reloc.type);
	}
#ifdef OBJ_ELF
      dwarf2_emit_insn (inst_main.size);
#endif

      /* s3_GP instruction can not do optimization, only can do relax between
         1 instruction and 3 instructions.  */
      p = frag_var (rs_machine_dependent, inst_main.relax_size + s3_RELAX_PAD_BYTE, 0,
                    s3_RELAX_ENCODE (inst_main.size, inst_main.relax_size, inst_main.type, 0, 4, 1),
                    inst_main.reloc.exp.X_add_symbol, 0, NULL);

      /* Write fr_var part.
         no calling s3_gen_insn_frag, no fixS will be generated.  */
      s3_md_number_to_chars (p, inst_expand[0].instruction, inst_expand[0].size);
      p += inst_expand[0].size;
      s3_md_number_to_chars (p, inst_expand[1].instruction, inst_expand[1].size);
      p += inst_expand[1].size;

      /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
      s3_inst.bwarn = -1;
    }
 out:
  free (keep_data);
}

/* Handle bcmpeqz / bcmpnez  */
static void
s3_do_macro_bcmpz (char *str)
{
  int reg_a;
  char *keep_data;
  size_t keep_data_size;
  int i;
  struct s3_score_it inst_expand[2];
  struct s3_score_it inst_main;

  memset (inst_expand, 0, sizeof inst_expand);
  s3_skip_whitespace (str);
  if (( reg_a = s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL)
    return;

  keep_data_size = strlen (str) + 1;
  keep_data = xmalloc (keep_data_size * 2 + 13);
  memcpy (keep_data, str, keep_data_size);

  if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    goto out;
  else if (s3_inst.reloc.exp.X_add_symbol == 0)
    {
      s3_inst.error = _("lacking label  ");
      goto out;
    }
  else
    {
      char *append_str = keep_data + keep_data_size;
      s3_SET_INSN_ERROR (NULL);
      s3_inst.reloc.type = BFD_RELOC_SCORE_BCMP;
      s3_inst.reloc.pc_rel = 1;
      bfd_signed_vma val = s3_inst.reloc.exp.X_add_number;

      /* Branch 32  offset field : 20 bit, 16 bit branch offset field : 8 bit.  */
      s3_inst.instruction |= ((s3_inst.reloc.exp.X_add_number>>1) & 0x1) | ((s3_inst.reloc.exp.X_add_number>>2) & 0x7)<<7 |((s3_inst.reloc.exp.X_add_number>>5) & 0x1f)<<20;

      /* Check and set offset.  */
      if (((val & 0xfffffe00) != 0)
	  && ((val & 0xfffffe00) != 0xfffffe00))
        {
          if (s3_score_pic == s3_NO_PIC)
            {
	      sprintf (append_str, "cmpi! r%d, 0", reg_a);
	      if (s3_append_insn (append_str, true) == (int) s3_FAIL)
		goto out;
	      if ((inst_main.instruction & 0x3e00007e) == 0x0000004c)
		memcpy (append_str, "beq ", 4);
	      else
		memcpy (append_str, "bne ", 4);
	      memmove (append_str + 4, keep_data, strlen (keep_data) + 1);
	      if (s3_append_insn (append_str, true) == (int) s3_FAIL)
		goto out;
            }
          else
            {
              gas_assert (s3_inst.reloc.exp.X_add_symbol);
            }
          /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
          s3_inst.bwarn = -1;
	  goto out;
        }
      else
        {
          val >>= 1;
          s3_inst.instruction |= (val & 0x1)
	    | (((val >> 1) & 0x7) << 7)
	    | (((val >> 4) & 0x1f) << 20);
        }

      /* Backup s3_inst.  */
      memcpy (&inst_main, &s3_inst, sizeof (struct s3_score_it));

      if (s3_score_pic == s3_NO_PIC)
        {
	  sprintf (append_str, "cmpi! r%d, 0", reg_a);
	  if (s3_append_insn (append_str, false) == (int) s3_FAIL)
	    goto out;
	  memcpy (&inst_expand[0], &s3_inst, sizeof (struct s3_score_it));
	  if ((inst_main.instruction & 0x3e00007e) == 0x0000004c)
	    memcpy (append_str, "beq ", 4);
	  else
	    memcpy (append_str, "bne ", 4);
	  memmove (append_str + 4, keep_data, strlen (keep_data) + 1);
	  if (s3_append_insn (append_str, false) == (int) s3_FAIL)
	    goto out;
	  memcpy (&inst_expand[1], &s3_inst, sizeof (struct s3_score_it));
        }
      else
        {
          gas_assert (s3_inst.reloc.exp.X_add_symbol);
        }
      inst_main.relax_size = inst_expand[0].size + inst_expand[1].size;
      inst_main.type = Insn_BCMP;

      /* Adjust instruction opcode and to be relaxed instruction opcode.  */
      inst_main.instruction = s3_adjust_paritybit (inst_main.instruction, s3_GET_INSN_CLASS (inst_main.type));

      for (i = 0; i < 2; i++)
        inst_expand[i].instruction = s3_adjust_paritybit (inst_expand[i].instruction ,
							  s3_GET_INSN_CLASS (inst_expand[i].type));
      /* Check data dependency.  */
      s3_handle_dependency (&inst_main);
      /* Start a new frag if frag_now is not empty.  */
      if (frag_now_fix () != 0)
	{
	  if (!frag_now->tc_frag_data.is_insn)
	    frag_wane (frag_now);
	  frag_new (0);
	}
      frag_grow (20);

      /* Write fr_fix part.  */
      char *p;
      p = frag_more (inst_main.size);
      s3_md_number_to_chars (p, inst_main.instruction, inst_main.size);

      if (inst_main.reloc.type != BFD_RELOC_NONE)
	{
	  s3_fix_new_score (frag_now, p - frag_now->fr_literal, inst_main.size,
			    &inst_main.reloc.exp, inst_main.reloc.pc_rel, inst_main.reloc.type);
	}
#ifdef OBJ_ELF
      dwarf2_emit_insn (inst_main.size);
#endif

      /* s3_GP instruction can not do optimization, only can do relax between
         1 instruction and 3 instructions.  */
      p = frag_var (rs_machine_dependent, inst_main.relax_size + s3_RELAX_PAD_BYTE, 0,
                    s3_RELAX_ENCODE (inst_main.size, inst_main.relax_size, inst_main.type, 0, 4, 1),
                    inst_main.reloc.exp.X_add_symbol, 0, NULL);

      /* Write fr_var part.
         no calling s3_gen_insn_frag, no fixS will be generated.  */
      s3_md_number_to_chars (p, inst_expand[0].instruction, inst_expand[0].size);
      p += inst_expand[0].size;
      s3_md_number_to_chars (p, inst_expand[1].instruction, inst_expand[1].size);
      p += inst_expand[1].size;

      /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
      s3_inst.bwarn = -1;
    }
 out:
  free (keep_data);
}

static int
s3_nopic_need_relax (symbolS * sym, int before_relaxing)
{
  if (sym == NULL)
    return 0;
  else if (s3_USE_GLOBAL_POINTER_OPT && s3_g_switch_value > 0)
    {
      const char *symname;
      const char *segname;

      /* Find out whether this symbol can be referenced off the $gp
         register.  It can be if it is smaller than the -G size or if
         it is in the .sdata or .sbss section.  Certain symbols can
         not be referenced off the $gp, although it appears as though
         they can.  */
      symname = S_GET_NAME (sym);
      if (symname != (const char *)NULL
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
								  && S_GET_VALUE (sym) <= s3_g_switch_value)))
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

/* Build a relax frag for lw/st instruction when generating s3_PIC,
   external symbol first and local symbol second.  */
static void
s3_build_lwst_pic (int reg_rd, expressionS exp, const char *insn_name)
{
  symbolS *add_symbol = exp.X_add_symbol;
  int add_number = exp.X_add_number;
  struct s3_score_it fix_insts[s3_RELAX_INST_NUM];
  struct s3_score_it var_insts[s3_RELAX_INST_NUM];
  int fix_num = 0;
  int var_num = 0;
  char tmp[s3_MAX_LITERAL_POOL_SIZE];
  int r1_bak;

  r1_bak = s3_nor1;
  s3_nor1 = 0;

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
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
        return;

      memcpy (&fix_insts[0], &s3_inst, sizeof (struct s3_score_it));

      /* Var part
	 For a local symbol :
         lw rD, <sym>($gp)    (BFD_RELOC_SCORE_GOT15)
	 addi rD, <sym>       (BFD_RELOC_GOT_LO16) */
      s3_inst.reloc.type = BFD_RELOC_SCORE_GOT15;
      memcpy (&var_insts[0], &s3_inst, sizeof (struct s3_score_it));
      sprintf (tmp, "addi_s_pic r1, %s", S_GET_NAME (add_symbol));
      if (s3_append_insn (tmp, false) == (int) s3_FAIL)
        return;

      memcpy (&var_insts[1], &s3_inst, sizeof (struct s3_score_it));
      s3_build_relax_frag (fix_insts, fix_num, var_insts, var_num, add_symbol);

      /* Insn 2 or Insn 3: lw/st rD, [r1, constant]  */
      sprintf (tmp, "%s r%d, [r1, %d]", insn_name, reg_rd, add_number);
      if (s3_append_insn (tmp, true) == (int) s3_FAIL)
        return;

      /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
      s3_inst.bwarn = -1;
    }
  else
    {
      s3_inst.error = _("s3_PIC code offset overflow (max 16 signed bits)");
      return;
    }

  s3_nor1 = r1_bak;
}

static void
s3_do_macro_ldst_label (char *str)
{
  int i;
  int ldst_gp_p = 0;
  int reg_rd;
  int r1_bak;
  char *backup_str;
  char *label_str;
  char *absolute_value;
  char append_str[3][s3_MAX_LITERAL_POOL_SIZE];
  char verifystr[s3_MAX_LITERAL_POOL_SIZE];
  struct s3_score_it inst_backup;
  struct s3_score_it inst_expand[3];
  struct s3_score_it inst_main;

  memcpy (&inst_backup, &s3_inst, sizeof (struct s3_score_it));
  strcpy (verifystr, str);
  backup_str = verifystr;

  s3_skip_whitespace (backup_str);
  if ((reg_rd = s3_reg_required_here (&backup_str, -1, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
    return;

  if (s3_skip_past_comma (&backup_str) == (int) s3_FAIL)
    return;

  label_str = backup_str;

  /* Ld/st rD, [rA, imm]      ld/st rD, [rA]+, imm      ld/st rD, [rA, imm]+.  */
  if (*backup_str == '[')
    {
      s3_inst.type = Rd_rvalueRs_preSI12;
      s3_do_ldst_insn (str);
      return;
    }

  /* Ld/st rD, imm.  */
  absolute_value = backup_str;
  s3_inst.type = Rd_rvalueRs_SI15;

  if (s3_my_get_expression (&s3_inst.reloc.exp, &backup_str) == (int) s3_FAIL)
    {
      s3_inst.error = _("expression error");
      return;
    }
  else if ((s3_inst.reloc.exp.X_add_symbol == NULL)
           && (s3_validate_immediate (s3_inst.reloc.exp.X_add_number, _VALUE, 0) == (int) s3_FAIL))
    {
      s3_inst.error = _("value not in range [0, 0x7fffffff]");
      return;
    }
  else if (s3_end_of_line (backup_str) == (int) s3_FAIL)
    {
      s3_inst.error = _("end on line error");
      return;
    }
  else
    {
      if (s3_inst.reloc.exp.X_add_symbol == 0)
        {
          memcpy (&s3_inst, &inst_backup, sizeof (struct s3_score_it));
          s3_exp_macro_ldst_abs (str);
          return;
        }
    }

  /* Ld/st rD, label.  */
  s3_inst.type = Rd_rvalueRs_SI15;
  backup_str = absolute_value;
  if ((s3_data_op2 (&backup_str, 1, _GP_IMM15) == (int) s3_FAIL)
      || (s3_end_of_line (backup_str) == (int) s3_FAIL))
    {
      return;
    }
  else
    {
      if (s3_inst.reloc.exp.X_add_symbol == 0)
        {
          if (!s3_inst.error)
	    s3_inst.error = s3_BAD_ARGS;

          return;
        }

      if (s3_score_pic == s3_PIC)
        {
          int ldst_idx = 0;
          ldst_idx = s3_inst.instruction & OPC_PSEUDOLDST_MASK;
          s3_build_lwst_pic (reg_rd, s3_inst.reloc.exp,
                             s3_score_ldst_insns[ldst_idx * 3 + 0].template_name);
          return;
        }
      else
	{
          if ((s3_inst.reloc.exp.X_add_number <= 0x3fff)
	      && (s3_inst.reloc.exp.X_add_number >= -0x4000)
	      && (!s3_nopic_need_relax (s3_inst.reloc.exp.X_add_symbol, 1)))
	    {
              int ldst_idx = 0;

              /* Assign the real opcode.  */
              ldst_idx = s3_inst.instruction & OPC_PSEUDOLDST_MASK;
              s3_inst.instruction &= ~OPC_PSEUDOLDST_MASK;
              s3_inst.instruction |= s3_score_ldst_insns[ldst_idx * 3 + 0].value;
              s3_inst.instruction |= reg_rd << 20;
              s3_inst.instruction |= s3_GP << 15;
              s3_inst.relax_inst = 0x8000;
              s3_inst.relax_size = 0;
              ldst_gp_p = 1;
	    }
	}
    }

  /* Backup s3_inst.  */
  memcpy (&inst_main, &s3_inst, sizeof (struct s3_score_it));
  r1_bak = s3_nor1;
  s3_nor1 = 0;

  /* Determine which instructions should be output.  */
  sprintf (append_str[0], "ld_i32hi r1, %s", label_str);
  sprintf (append_str[1], "ld_i32lo r1, %s", label_str);
  sprintf (append_str[2], "%s r%d, [r1, 0]", inst_backup.name, reg_rd);

  /* Generate three instructions.
     la r1, label
     ld/st rd, [r1, 0]  */
  for (i = 0; i < 3; i++)
    {
      if (s3_append_insn (append_str[i], false) == (int) s3_FAIL)
	return;

      memcpy (&inst_expand[i], &s3_inst, sizeof (struct s3_score_it));
    }

  if (ldst_gp_p)
    {
      char *p;

      /* Adjust instruction opcode and to be relaxed instruction opcode.  */
      inst_main.instruction = s3_adjust_paritybit (inst_main.instruction, s3_GET_INSN_CLASS (inst_main.type));

      /* relax lw rd, label -> ldis rs, imm16
	 ori  rd, imm16
	 lw rd, [rs, imm15] or lw! rd, [rs, imm5].  */
      if (inst_expand[2].relax_size == 0)
        inst_main.relax_size = inst_expand[0].size + inst_expand[1].size + inst_expand[2].size;
      else
        inst_main.relax_size = inst_expand[0].size + inst_expand[1].size + inst_expand[2].relax_size;

      inst_main.type = Insn_GP;

      for (i = 0; i < 3; i++)
	inst_expand[i].instruction = s3_adjust_paritybit (inst_expand[i].instruction,
                                                          s3_GET_INSN_CLASS (inst_expand[i].type));

      /* Check data dependency.  */
      s3_handle_dependency (&inst_main);

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
      s3_md_number_to_chars (p, inst_main.instruction, inst_main.size);

      if (inst_main.reloc.type != BFD_RELOC_NONE)
        {
          s3_fix_new_score (frag_now, p - frag_now->fr_literal, inst_main.size,
			    &inst_main.reloc.exp, inst_main.reloc.pc_rel, inst_main.reloc.type);
        }

#ifdef OBJ_ELF
      dwarf2_emit_insn (inst_main.size);
#endif

      /* s3_GP instruction can not do optimization, only can do relax between
         1 instruction and 3 instructions.  */
      p = frag_var (rs_machine_dependent, inst_main.relax_size + s3_RELAX_PAD_BYTE, 0,
                    s3_RELAX_ENCODE (inst_main.size, inst_main.relax_size, inst_main.type, 0, 4, 0),
                    inst_main.reloc.exp.X_add_symbol, 0, NULL);

      /* Write fr_var part.
         no calling s3_gen_insn_frag, no fixS will be generated.  */
      s3_md_number_to_chars (p, inst_expand[0].instruction, inst_expand[0].size);
      p += inst_expand[0].size;
      s3_md_number_to_chars (p, inst_expand[1].instruction, inst_expand[1].size);
      p += inst_expand[1].size;

      /* relax lw rd, label -> ldis rs, imm16
	 ori  rd, imm16
	 lw rd, [rs, imm15] or lw! rd, [rs, imm5].  */
      if (inst_expand[2].relax_size == 0)
        s3_md_number_to_chars (p, inst_expand[2].instruction, inst_expand[2].size);
      else
        s3_md_number_to_chars (p, inst_expand[2].relax_inst, inst_expand[2].relax_size);
    }
  else
    {
      s3_gen_insn_frag (&inst_expand[0], NULL);
      s3_gen_insn_frag (&inst_expand[1], NULL);
      s3_gen_insn_frag (&inst_expand[2], NULL);
    }
  s3_nor1 = r1_bak;

  /* Set bwarn as -1, so macro instruction itself will not be generated frag.  */
  s3_inst.bwarn = -1;
}

static void
s3_do_lw_pic (char *str)
{
  int reg_rd;

  s3_skip_whitespace (str);
  if (((reg_rd = s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
      || (s3_skip_past_comma (&str) == (int) s3_FAIL)
      || (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL)
      || (s3_end_of_line (str) == (int) s3_FAIL))
    {
      return;
    }
  else
    {
      if (s3_inst.reloc.exp.X_add_symbol == 0)
        {
          if (!s3_inst.error)
	    s3_inst.error = s3_BAD_ARGS;

          return;
        }

      s3_inst.instruction |= s3_GP << 15;
      s3_inst.reloc.type = BFD_RELOC_SCORE_GOT15;
    }
}

static void
s3_do_empty (char *str)
{
  str = str;
  if (s3_university_version == 1)
    {
      if (((s3_inst.instruction & 0x3e0003ff) == 0x0c000004)
          || ((s3_inst.instruction & 0x3e0003ff) == 0x0c000024)
          || ((s3_inst.instruction & 0x3e0003ff) == 0x0c000044)
          || ((s3_inst.instruction & 0x3e0003ff) == 0x0c000064))
        {
          s3_inst.error = s3_ERR_FOR_SCORE5U_MMU;
          return;
        }
    }
  if (s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if (s3_inst.relax_inst != 0x8000)
    {
      if (s3_inst.type == NO_OPD)
        {
          s3_inst.relax_size = 2;
        }
      else
        {
          s3_inst.relax_size = 4;
        }
    }
}

static void
s3_do16_int (char *str)
{
  s3_skip_whitespace (str);
  return;
}

static void
s3_do_jump (char *str)
{
  char *save_in;

  s3_skip_whitespace (str);
  if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if (s3_inst.reloc.exp.X_add_symbol == 0)
    {
      s3_inst.error = _("lacking label  ");
      return;
    }

  if (!(s3_inst.reloc.exp.X_add_number >= -16777216
	&& s3_inst.reloc.exp.X_add_number <= 16777215))
    {
      s3_inst.error = _("invalid constant: 25 bit expression not in range [-16777216, 16777215]");
      return;
    }

  save_in = input_line_pointer;
  input_line_pointer = str;
  s3_inst.reloc.type = BFD_RELOC_SCORE_JMP;
  s3_inst.reloc.pc_rel = 1;
  input_line_pointer = save_in;
}

static void
s3_do_branch (char *str)
{
  if (s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
  else if (s3_inst.reloc.exp.X_add_symbol == 0)
    {
      s3_inst.error = _("lacking label  ");
      return;
    }
  else if (!(s3_inst.reloc.exp.X_add_number >= -524288
	     && s3_inst.reloc.exp.X_add_number <= 524287))
    {
      s3_inst.error = _("invalid constant: 20 bit expression not in range -2^19..2^19-1");
      return;
    }

  s3_inst.reloc.type = BFD_RELOC_SCORE_BRANCH;
  s3_inst.reloc.pc_rel = 1;

  /* Branch 32  offset field : 20 bit, 16 bit branch offset field : 8 bit.  */
  s3_inst.instruction |= (s3_inst.reloc.exp.X_add_number & 0x3fe) | ((s3_inst.reloc.exp.X_add_number & 0xffc00) << 5);

  /* Compute 16 bit branch instruction.  */
  if ((s3_inst.relax_inst != 0x8000)
      && (s3_inst.reloc.exp.X_add_number >= -512 && s3_inst.reloc.exp.X_add_number <= 511))
    {
      s3_inst.relax_inst |= ((s3_inst.reloc.exp.X_add_number >> 1) & 0x1ff);/*b! :disp 9 bit */
      s3_inst.relax_size = 2;
    }
  else
    {
      s3_inst.relax_inst = 0x8000;
    }
}

static void
s3_do16_branch (char *str)
{
  if ((s3_my_get_expression (&s3_inst.reloc.exp, &str) == (int) s3_FAIL
       || s3_end_of_line (str) == (int) s3_FAIL))
    {
      ;
    }
  else if (s3_inst.reloc.exp.X_add_symbol == 0)
    {
      s3_inst.error = _("lacking label");
    }
  else if (!(s3_inst.reloc.exp.X_add_number >= -512
	     && s3_inst.reloc.exp.X_add_number <= 511))
    {
      s3_inst.error = _("invalid constant: 10 bit expression not in range [-2^9, 2^9-1]");
    }
  else
    {
      s3_inst.reloc.type = BFD_RELOC_SCORE16_BRANCH;
      s3_inst.reloc.pc_rel = 1;
      s3_inst.instruction |= ((s3_inst.reloc.exp.X_add_number >> 1) & 0x1ff);
      s3_inst.relax_inst |= ((s3_inst.reloc.exp.X_add_number ) & 0x1ff);
      s3_inst.relax_size = 4;
    }
}

/* Return true if the given symbol should be considered local for s3_PIC.  */
static bool
s3_pic_need_relax (symbolS *sym, asection *segtype)
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

  /* duplicate the test for LINK_ONCE sections as in adjust_reloc_syms */
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

static void
s3_parse_pce_inst (char *insnstr)
{
  char c;
  char *p;
  char first[s3_MAX_LITERAL_POOL_SIZE];
  char second[s3_MAX_LITERAL_POOL_SIZE];
  struct s3_score_it pec_part_1;

  /* Get first part string of PCE.  */
  p = strstr (insnstr, "||");
  c = *p;
  *p = '\0';
  sprintf (first, "%s", insnstr);

  /* Get second part string of PCE.  */
  *p = c;
  p += 2;
  sprintf (second, "%s", p);

  s3_parse_16_32_inst (first, false);
  if (s3_inst.error)
    return;

  memcpy (&pec_part_1, &s3_inst, sizeof (s3_inst));

  s3_parse_16_32_inst (second, false);
  if (s3_inst.error)
    return;

  if (   ((pec_part_1.size == s3_INSN_SIZE) && (s3_inst.size == s3_INSN_SIZE))
	 || ((pec_part_1.size == s3_INSN_SIZE) && (s3_inst.size == s3_INSN16_SIZE))
	 || ((pec_part_1.size == s3_INSN16_SIZE) && (s3_inst.size == s3_INSN_SIZE)))
    {
      s3_inst.error = _("pce instruction error (16 bit || 16 bit).");
      sprintf (s3_inst.str, "%s", insnstr);
      return;
    }

  if (!s3_inst.error)
    s3_gen_insn_frag (&pec_part_1, &s3_inst);
}

/* s3: dsp.  */
static void
s3_do16_dsp (char *str)
{
  int rd = 0;

  /* Check 3d.  */
  if (s3_score3d == 0)
    {
      s3_inst.error = _("score3d instruction.");
      return;
    }

  s3_skip_whitespace (str);

  if ((rd = s3_reglow_required_here (&str, 0)) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
  else
    {
      s3_inst.relax_inst |= rd << 20;
      s3_inst.relax_size = 4;
    }
}

static void
s3_do16_dsp2 (char *str)
{
  /* Check 3d.  */
  if (s3_score3d == 0)
    {
      s3_inst.error = _("score3d instruction.");
      return;
    }

  s3_skip_whitespace (str);

  if (s3_reglow_required_here (&str, 4) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reglow_required_here (&str, 0) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
  else
    {
      s3_inst.relax_inst |= (((s3_inst.instruction >> 8) & 0xf) << 20)
        | (((s3_inst.instruction >> 8) & 0xf) << 15) | (((s3_inst.instruction >> 4) & 0xf) << 10);
      s3_inst.relax_size = 4;
    }
}

static void
s3_do_dsp (char *str)
{
  /* Check 3d.  */
  if (s3_score3d == 0)
    {
      s3_inst.error = _("score3d instruction.");
      return;
    }

  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if ((s3_inst.relax_inst != 0x8000) && (((s3_inst.instruction >> 20) & 0x1f) == 3) )
    {
      s3_inst.relax_inst |= (((s3_inst.instruction >> 10) & 0x1f)) | (((s3_inst.instruction >> 15) & 0x1f) << 5);
      s3_inst.relax_size = 2;
    }
  else
    s3_inst.relax_inst = 0x8000;
}

static void
s3_do_dsp2 (char *str)
{
  int reg;

  /* Check 3d.  */
  if (s3_score3d == 0)
    {
      s3_inst.error = _("score3d instruction.");
      return;
    }

  s3_skip_whitespace (str);

  if ((reg = s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 10, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    {
      return;
    }
  else
    {
      /* Check mulr, mulur rd is even number.  */
      if (((s3_inst.instruction & 0x3e0003ff) == 0x00000340
	   || (s3_inst.instruction & 0x3e0003ff) == 0x00000342)
          && (reg % 2))
        {
          s3_inst.error = _("rd must be even number.");
          return;
        }

      if ((((s3_inst.instruction >> 15) & 0x10) == 0)
          && (((s3_inst.instruction >> 10) & 0x10) == 0)
          && (((s3_inst.instruction >> 20) & 0x10) == 0)
          && (s3_inst.relax_inst != 0x8000)
          && (((s3_inst.instruction >> 20) & 0xf) == ((s3_inst.instruction >> 15) & 0xf)))
        {
          s3_inst.relax_inst |= (((s3_inst.instruction >> 10) & 0xf) )
            | (((s3_inst.instruction >> 15) & 0xf) << 4);
          s3_inst.relax_size = 2;
        }
      else
        {
          s3_inst.relax_inst = 0x8000;
        }
    }
}

static void
s3_do_dsp3 (char *str)
{
  /* Check 3d.  */
  if (s3_score3d == 0)
    {
      s3_inst.error = _("score3d instruction.");
      return;
    }

  s3_skip_whitespace (str);

  if (s3_reg_required_here (&str, 20, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_skip_past_comma (&str) == (int) s3_FAIL
      || s3_reg_required_here (&str, 15, s3_REG_TYPE_SCORE) == (int) s3_FAIL
      || s3_end_of_line (str) == (int) s3_FAIL)
    return;

  if ((s3_inst.relax_inst != 0x8000) && (((s3_inst.instruction >> 20) & 0x1f) == 3) )
    {
      s3_inst.relax_inst |= (((s3_inst.instruction >> 10) & 0x1f)) | (((s3_inst.instruction >> 15) & 0x1f) << 5);
      s3_inst.relax_size = 2;
    }
  else
    s3_inst.relax_inst = 0x8000;
}


/* If we change section we must dump the literal pool first.  */
static void
s3_s_score_bss (int ignore ATTRIBUTE_UNUSED)
{
  subseg_set (bss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

static void
s3_s_score_text (int ignore)
{
  obj_elf_text (ignore);
  record_alignment (now_seg, 2);
}

static void
s3_score_s_section (int ignore)
{
  obj_elf_section (ignore);
  if ((bfd_section_flags (now_seg) & SEC_CODE) != 0)
    record_alignment (now_seg, 2);

}

static void
s3_s_change_sec (int sec)
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
      seg = subseg_new (s3_RDATA_SECTION_NAME, (subsegT) get_absolute_expression ());
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
s3_s_score_mask (int reg_type ATTRIBUTE_UNUSED)
{
  long mask, off;

  if (s3_cur_proc_ptr == (s3_procS *) NULL)
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
  s3_cur_proc_ptr->reg_mask = mask;
  s3_cur_proc_ptr->reg_offset = off;
  demand_empty_rest_of_line ();
}

static symbolS *
s3_get_symbol (void)
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
s3_get_number (void)
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
s3_s_score_ent (int aent)
{
  symbolS *symbolP;
  int maybe_text;

  symbolP = s3_get_symbol ();
  if (*input_line_pointer == ',')
    ++input_line_pointer;
  SKIP_WHITESPACE ();
  if (ISDIGIT (*input_line_pointer) || *input_line_pointer == '-')
    s3_get_number ();

  if ((bfd_section_flags (now_seg) & SEC_CODE) != 0)
    maybe_text = 1;
  else
    maybe_text = 0;
  if (!maybe_text)
    as_warn (_(".ent or .aent not in text section."));
  if (!aent && s3_cur_proc_ptr)
    as_warn (_("missing .end"));
  if (!aent)
    {
      s3_cur_proc_ptr = &s3_cur_proc;
      s3_cur_proc_ptr->reg_mask = 0xdeadbeaf;
      s3_cur_proc_ptr->reg_offset = 0xdeadbeaf;
      s3_cur_proc_ptr->fpreg_mask = 0xdeafbeaf;
      s3_cur_proc_ptr->leaf = 0xdeafbeaf;
      s3_cur_proc_ptr->frame_offset = 0xdeafbeaf;
      s3_cur_proc_ptr->frame_reg = 0xdeafbeaf;
      s3_cur_proc_ptr->pc_reg = 0xdeafbeaf;
      s3_cur_proc_ptr->isym = symbolP;
      symbol_get_bfdsym (symbolP)->flags |= BSF_FUNCTION;
      ++s3_numprocs;
      if (debug_type == DEBUG_STABS)
        stabs_generate_asm_func (S_GET_NAME (symbolP), S_GET_NAME (symbolP));
    }
  demand_empty_rest_of_line ();
}

static void
s3_s_score_frame (int ignore ATTRIBUTE_UNUSED)
{
  char *backupstr;
  char str[30];
  long val;
  int i = 0;

  backupstr = input_line_pointer;

#ifdef OBJ_ELF
  if (s3_cur_proc_ptr == (s3_procS *) NULL)
    {
      as_warn (_(".frame outside of .ent"));
      demand_empty_rest_of_line ();
      return;
    }
  s3_cur_proc_ptr->frame_reg = s3_reg_required_here ((&backupstr), 0, s3_REG_TYPE_SCORE);
  SKIP_WHITESPACE ();
  s3_skip_past_comma (&backupstr);
  while (*backupstr != ',')
    {
      str[i] = *backupstr;
      i++;
      backupstr++;
    }
  str[i] = '\0';
  val = atoi (str);

  SKIP_WHITESPACE ();
  s3_skip_past_comma (&backupstr);
  s3_cur_proc_ptr->frame_offset = val;
  s3_cur_proc_ptr->pc_reg = s3_reg_required_here ((&backupstr), 0, s3_REG_TYPE_SCORE);

  SKIP_WHITESPACE ();
  s3_skip_past_comma (&backupstr);
  i = 0;
  while (*backupstr != '\n')
    {
      str[i] = *backupstr;
      i++;
      backupstr++;
    }
  str[i] = '\0';
  val = atoi (str);
  s3_cur_proc_ptr->leaf = val;
  SKIP_WHITESPACE ();
  s3_skip_past_comma (&backupstr);

#endif /* OBJ_ELF */
  while (input_line_pointer != backupstr)
    input_line_pointer++;
}

/* The .end directive.  */
static void
s3_s_score_end (int x ATTRIBUTE_UNUSED)
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
      p = s3_get_symbol ();
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
  if (!s3_cur_proc_ptr)
    {
      as_warn (_(".end directive without a preceding .ent directive."));
      demand_empty_rest_of_line ();
      return;
    }
  if (p != NULL)
    {
      gas_assert (S_GET_NAME (p));
      if (strcmp (S_GET_NAME (p), S_GET_NAME (s3_cur_proc_ptr->isym)))
        as_warn (_(".end symbol does not match .ent symbol."));
      if (debug_type == DEBUG_STABS)
        stabs_generate_asm_endfunc (S_GET_NAME (p), S_GET_NAME (p));
    }
  else
    as_warn (_(".end directive missing or unknown symbol"));

  if ((s3_cur_proc_ptr->reg_mask == 0xdeadbeaf) ||
      (s3_cur_proc_ptr->reg_offset == 0xdeadbeaf) ||
      (s3_cur_proc_ptr->leaf == 0xdeafbeaf) ||
      (s3_cur_proc_ptr->frame_offset == 0xdeafbeaf) ||
      (s3_cur_proc_ptr->frame_reg == 0xdeafbeaf) || (s3_cur_proc_ptr->pc_reg == 0xdeafbeaf));

  else
    {
      (void) frag_now_fix ();
      gas_assert (s3_pdr_seg);
      subseg_set (s3_pdr_seg, 0);
      /* Write the symbol.  */
      exp.X_op = O_symbol;
      exp.X_add_symbol = p;
      exp.X_add_number = 0;
      emit_expr (&exp, 4);
      fragp = frag_more (7 * 4);
      md_number_to_chars (fragp, (valueT) s3_cur_proc_ptr->reg_mask, 4);
      md_number_to_chars (fragp + 4, (valueT) s3_cur_proc_ptr->reg_offset, 4);
      md_number_to_chars (fragp + 8, (valueT) s3_cur_proc_ptr->fpreg_mask, 4);
      md_number_to_chars (fragp + 12, (valueT) s3_cur_proc_ptr->leaf, 4);
      md_number_to_chars (fragp + 16, (valueT) s3_cur_proc_ptr->frame_offset, 4);
      md_number_to_chars (fragp + 20, (valueT) s3_cur_proc_ptr->frame_reg, 4);
      md_number_to_chars (fragp + 24, (valueT) s3_cur_proc_ptr->pc_reg, 4);
      subseg_set (saved_seg, saved_subseg);

    }
  s3_cur_proc_ptr = NULL;
}

/* Handle the .set pseudo-op.  */
static void
s3_s_score_set (int x ATTRIBUTE_UNUSED)
{
  int i = 0;
  char name[s3_MAX_LITERAL_POOL_SIZE];
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
      s3_warn_fix_data_dependency = 0;
    }
  else if (strcmp (name, "fixdd") == 0)
    {
      s3_fix_data_dependency = 1;
    }
  else if (strcmp (name, "nofixdd") == 0)
    {
      s3_fix_data_dependency = 0;
    }
  else if (strcmp (name, "r1") == 0)
    {
      s3_nor1 = 0;
    }
  else if (strcmp (name, "nor1") == 0)
    {
      s3_nor1 = 1;
    }
  else if (strcmp (name, "optimize") == 0)
    {
      s3_g_opt = 1;
    }
  else if (strcmp (name, "volatile") == 0)
    {
      s3_g_opt = 0;
    }
  else if (strcmp (name, "pic") == 0)
    {
      s3_score_pic = s3_PIC;
    }
  else
    {
      input_line_pointer = orig_ilp;
      s_set (0);
    }
}

/* Handle the .cpload pseudo-op.  This is used when generating s3_PIC code.  It sets the
   $gp register for the function based on the function address, which is in the register
   named in the argument. This uses a relocation against GP_DISP_LABEL, which is handled
   specially by the linker.  The result is:
   ldis gp, %hi(GP_DISP_LABEL)
   ori  gp, %low(GP_DISP_LABEL)
   add  gp, gp, .cpload argument
   The .cpload argument is normally r29.  */
static void
s3_s_score_cpload (int ignore ATTRIBUTE_UNUSED)
{
  int reg;
  char insn_str[s3_MAX_LITERAL_POOL_SIZE];

  /* If we are not generating s3_PIC code, .cpload is ignored.  */
  if (s3_score_pic == s3_NO_PIC)
    {
      s_ignore (0);
      return;
    }

  if ((reg = s3_reg_required_here (&input_line_pointer, -1, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
    return;

  demand_empty_rest_of_line ();

  sprintf (insn_str, "ld_i32hi r%d, %s", s3_GP, GP_DISP_LABEL);
  if (s3_append_insn (insn_str, true) == (int) s3_FAIL)
    return;

  sprintf (insn_str, "ld_i32lo r%d, %s", s3_GP, GP_DISP_LABEL);
  if (s3_append_insn (insn_str, true) == (int) s3_FAIL)
    return;

  sprintf (insn_str, "add r%d, r%d, r%d", s3_GP, s3_GP, reg);
  if (s3_append_insn (insn_str, true) == (int) s3_FAIL)
    return;
}

/* Handle the .cprestore pseudo-op.  This stores $gp into a given
   offset from $sp.  The offset is remembered, and after making a s3_PIC
   call $gp is restored from that location.  */
static void
s3_s_score_cprestore (int ignore ATTRIBUTE_UNUSED)
{
  int reg;
  int cprestore_offset;
  char insn_str[s3_MAX_LITERAL_POOL_SIZE];

  /* If we are not generating s3_PIC code, .cprestore is ignored.  */
  if (s3_score_pic == s3_NO_PIC)
    {
      s_ignore (0);
      return;
    }

  if ((reg = s3_reg_required_here (&input_line_pointer, -1, s3_REG_TYPE_SCORE)) == (int) s3_FAIL
      || s3_skip_past_comma (&input_line_pointer) == (int) s3_FAIL)
    {
      return;
    }

  cprestore_offset = get_absolute_expression ();

  if (cprestore_offset <= 0x3fff)
    {
      sprintf (insn_str, "sw r%d, [r%d, %d]", s3_GP, reg, cprestore_offset);
      if (s3_append_insn (insn_str, true) == (int) s3_FAIL)
        return;
    }
  else
    {
      int r1_bak;

      r1_bak = s3_nor1;
      s3_nor1 = 0;

      sprintf (insn_str, "li r1, %d", cprestore_offset);
      if (s3_append_insn (insn_str, true) == (int) s3_FAIL)
        return;

      sprintf (insn_str, "add r1, r1, r%d", reg);
      if (s3_append_insn (insn_str, true) == (int) s3_FAIL)
        return;

      sprintf (insn_str, "sw r%d, [r1]", s3_GP);
      if (s3_append_insn (insn_str, true) == (int) s3_FAIL)
        return;

      s3_nor1 = r1_bak;
    }

  demand_empty_rest_of_line ();
}

/* Handle the .gpword pseudo-op.  This is used when generating s3_PIC
   code.  It generates a 32 bit s3_GP relative reloc.  */
static void
s3_s_score_gpword (int ignore ATTRIBUTE_UNUSED)
{
  expressionS ex;
  char *p;

  /* When not generating s3_PIC code, this is treated as .word.  */
  if (s3_score_pic == s3_NO_PIC)
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
  s3_md_number_to_chars (p, (valueT) 0, 4);
  fix_new_exp (frag_now, p - frag_now->fr_literal, 4, &ex, false, BFD_RELOC_GPREL32);
  demand_empty_rest_of_line ();
}

/* Handle the .cpadd pseudo-op.  This is used when dealing with switch
   tables in s3_PIC code.  */
static void
s3_s_score_cpadd (int ignore ATTRIBUTE_UNUSED)
{
  int reg;
  char insn_str[s3_MAX_LITERAL_POOL_SIZE];

  /* If we are not generating s3_PIC code, .cpload is ignored.  */
  if (s3_score_pic == s3_NO_PIC)
    {
      s_ignore (0);
      return;
    }

  if ((reg = s3_reg_required_here (&input_line_pointer, -1, s3_REG_TYPE_SCORE)) == (int) s3_FAIL)
    {
      return;
    }
  demand_empty_rest_of_line ();

  /* Add $gp to the register named as an argument.  */
  sprintf (insn_str, "add r%d, r%d, r%d", reg, reg, s3_GP);
  if (s3_append_insn (insn_str, true) == (int) s3_FAIL)
    return;
}

#ifndef TC_IMPLICIT_LCOMM_ALIGNMENT
#define TC_IMPLICIT_LCOMM_ALIGNMENT(SIZE, P2VAR)	\
  do							\
    {							\
      if ((SIZE) >= 8)					\
	(P2VAR) = 3;					\
      else if ((SIZE) >= 4)				\
	(P2VAR) = 2;					\
      else if ((SIZE) >= 2)				\
	(P2VAR) = 1;					\
      else						\
	(P2VAR) = 0;					\
    }							\
  while (0)
#endif

static void
s3_s_score_lcomm (int bytes_p)
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
  (void) restore_line_pointer (c);

  if (name == p)
    {
      as_bad (_("expected symbol name"));
      discard_rest_of_line ();
      return;
    }

  SKIP_WHITESPACE ();

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
s3_insert_reg (const struct s3_reg_entry *r, htab_t htab)
{
  char *buf = notes_strdup (r->name);
  char *p;

  for (p = buf; *p; p++)
    *p = TOUPPER (*p);

  str_hash_insert (htab, r->name, r, 0);
  str_hash_insert (htab, buf, r, 0);
}

static void
s3_build_reg_hsh (struct s3_reg_map *map)
{
  const struct s3_reg_entry *r;

  map->htab = str_htab_create ();
  for (r = map->names; r->name != NULL; r++)
    s3_insert_reg (r, map->htab);
}

/* Iterate over the base tables to create the instruction patterns.  */
static void
s3_build_score_ops_hsh (void)
{
  unsigned int i;

  for (i = 0; i < sizeof (s3_score_insns) / sizeof (struct s3_asm_opcode); i++)
    {
      const struct s3_asm_opcode *insn = s3_score_insns + i;
      size_t len = strlen (insn->template_name) + 1;
      struct s3_asm_opcode *new_opcode;
      char *template_name;

      new_opcode = notes_alloc (sizeof (*new_opcode));
      template_name = notes_memdup (insn->template_name, len, len);

      new_opcode->template_name = template_name;
      new_opcode->parms = insn->parms;
      new_opcode->value = insn->value;
      new_opcode->relax_value = insn->relax_value;
      new_opcode->type = insn->type;
      new_opcode->bitmask = insn->bitmask;
      str_hash_insert (s3_score_ops_hsh, new_opcode->template_name,
		       new_opcode, 0);
    }
}

static void
s3_build_dependency_insn_hsh (void)
{
  unsigned int i;

  for (i = 0; i < sizeof (s3_insn_to_dependency_table) / sizeof (s3_insn_to_dependency_table[0]); i++)
    {
      const struct s3_insn_to_dependency *tmp = s3_insn_to_dependency_table + i;
      size_t len = strlen (tmp->insn_name) + 1;
      struct s3_insn_to_dependency *new_i2n;
      char *buf;

      new_i2n = notes_alloc (sizeof (*new_i2n));
      buf = notes_memdup (tmp->insn_name, len, len);

      new_i2n->insn_name = buf;
      new_i2n->type = tmp->type;
      str_hash_insert (s3_dependency_insn_hsh, new_i2n->insn_name, new_i2n, 0);
    }
}

static void
s_score_bss (int ignore ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_bss (ignore);
  else
    return s7_s_score_bss (ignore);
}

static void
s_score_text (int ignore)
{
  if (score3)
    return s3_s_score_text (ignore);
  else
    return s7_s_score_text (ignore);
}

static void
s_section (int ignore)
{
  if (score3)
    return s3_score_s_section (ignore);
  else
    return s7_s_section (ignore);
}

static void
s_change_sec (int sec)
{
  if (score3)
    return s3_s_change_sec (sec);
  else
    return s7_s_change_sec (sec);
}

static void
s_score_mask (int reg_type ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_mask (reg_type);
  else
    return s7_s_score_mask (reg_type);
}

static void
s_score_ent (int aent)
{
  if (score3)
    return s3_s_score_ent (aent);
  else
    return s7_s_score_ent (aent);
}

static void
s_score_frame (int ignore ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_frame (ignore);
  else
    return s7_s_score_frame (ignore);
}

static void
s_score_end (int x ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_end (x);
  else
    return s7_s_score_end (x);
}

static void
s_score_set (int x ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_set (x);
  else
    return s7_s_score_set (x);
}

static void
s_score_cpload (int ignore ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_cpload (ignore);
  else
    return s7_s_score_cpload (ignore);
}

static void
s_score_cprestore (int ignore ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_cprestore (ignore);
  else
    return s7_s_score_cprestore (ignore);
}

static void
s_score_gpword (int ignore ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_gpword (ignore);
  else
    return s7_s_score_gpword (ignore);
}

static void
s_score_cpadd (int ignore ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_s_score_cpadd (ignore);
  else
    return s7_s_score_cpadd (ignore);
}

static void
s_score_lcomm (int bytes_p)
{
  if (score3)
    return s3_s_score_lcomm (bytes_p);
  else
    return s7_s_score_lcomm (bytes_p);
}

static void
s3_assemble (char *str)
{
  know (str);
  know (strlen (str) < s3_MAX_LITERAL_POOL_SIZE);

  memset (&s3_inst, '\0', sizeof (s3_inst));
  if (s3_INSN_IS_PCE_P (str))
    s3_parse_pce_inst (str);
  else if (s3_INSN_IS_48_P (str))
    s3_parse_48_inst (str, true);
  else
    s3_parse_16_32_inst (str, true);

  if (s3_inst.error)
    as_bad (_("%s -- `%s'"), s3_inst.error, s3_inst.str);
}

static void
s3_operand (expressionS * exp)
{
  if (s3_in_my_get_expression)
    {
      exp->X_op = O_illegal;
      if (s3_inst.error == NULL)
        {
          s3_inst.error = _("bad expression");
        }
    }
}

static void
s3_begin (void)
{
  unsigned int i;
  segT seg;
  subsegT subseg;

  s3_score_ops_hsh = str_htab_create ();

  s3_build_score_ops_hsh ();

  s3_dependency_insn_hsh = str_htab_create ();

  s3_build_dependency_insn_hsh ();

  for (i = (int)s3_REG_TYPE_FIRST; i < (int)s3_REG_TYPE_MAX; i++)
    s3_build_reg_hsh (s3_all_reg_maps + i);

  /* Initialize dependency vector.  */
  s3_init_dependency_vector ();

  bfd_set_arch_mach (stdoutput, TARGET_ARCH, 0);
  seg = now_seg;
  subseg = now_subseg;
  s3_pdr_seg = subseg_new (".pdr", (subsegT) 0);
  bfd_set_section_flags (s3_pdr_seg, SEC_READONLY | SEC_RELOC | SEC_DEBUGGING);
  bfd_set_section_alignment (s3_pdr_seg, 2);
  subseg_set (seg, subseg);

  if (s3_USE_GLOBAL_POINTER_OPT)
    bfd_set_gp_size (stdoutput, s3_g_switch_value);
}

static void
s3_number_to_chars (char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

static valueT
s3_normal_chars_to_number (char *buf, int n)
{
  valueT result = 0;
  unsigned char *where = (unsigned char *)buf;

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

static void
s3_number_to_chars_littleendian (void *p, valueT data, int n)
{
  char *buf = (char *) p;

  switch (n)
    {
    case 4:
      md_number_to_chars (buf, data >> 16, 2);
      md_number_to_chars (buf + 2, data, 2);
      break;
    case 6:
      md_number_to_chars (buf, data >> 32, 2);
      md_number_to_chars (buf + 2, data >> 16, 2);
      md_number_to_chars (buf + 4, data, 2);
      break;
    default:
      /* Error routine.  */
      as_bad_where (__FILE__, __LINE__, _("size is not 4 or 6"));
      break;
    }
}

static valueT
s3_chars_to_number_littleendian (const void *p, int n)
{
  char *buf = (char *) p;
  valueT result = 0;

  switch (n)
    {
    case 4:
      result =  s3_normal_chars_to_number (buf, 2) << 16;
      result |= s3_normal_chars_to_number (buf + 2, 2);
      break;
    case 6:
      result =  s3_normal_chars_to_number (buf, 2) << 32;
      result |= s3_normal_chars_to_number (buf + 2, 2) << 16;
      result |= s3_normal_chars_to_number (buf + 4, 2);
      break;
    default:
      /* Error routine.  */
      as_bad_where (__FILE__, __LINE__, _("size is not 4 or 6"));
      break;
    }

  return result;
}

static void
s3_md_number_to_chars (char *buf, valueT val, int n)
{
  if (!target_big_endian && n >= 4)
    s3_number_to_chars_littleendian (buf, val, n);
  else
    md_number_to_chars (buf, val, n);
}

static valueT
s3_md_chars_to_number (char *buf, int n)
{
  valueT result = 0;

  if (!target_big_endian && n >= 4)
    result = s3_chars_to_number_littleendian (buf, n);
  else
    result = s3_normal_chars_to_number (buf, n);

  return result;
}

static const char *
s3_atof (int type, char *litP, int *sizeP)
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
          s3_md_number_to_chars (litP, (valueT) words[i], 2);
          litP += 2;
        }
    }
  else
    {
      for (i = 0; i < prec; i += 2)
        {
          s3_md_number_to_chars (litP, (valueT) words[i + 1], 2);
          s3_md_number_to_chars (litP + 2, (valueT) words[i], 2);
          litP += 4;
        }
    }

  return 0;
}

static void
s3_frag_check (fragS * fragp ATTRIBUTE_UNUSED)
{
  know (fragp->insn_addr <= s3_RELAX_PAD_BYTE);
}

static void
s3_validate_fix (fixS *fixP)
{
  fixP->fx_where += fixP->fx_frag->insn_addr;
}

static int
s3_force_relocation (struct fix *fixp)
{
  int retval = 0;

  if (fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY
      || fixp->fx_r_type == BFD_RELOC_SCORE_JMP
      || fixp->fx_r_type == BFD_RELOC_SCORE_BRANCH
      || fixp->fx_r_type == BFD_RELOC_SCORE16_JMP
      || fixp->fx_r_type == BFD_RELOC_SCORE16_BRANCH
      || fixp->fx_r_type == BFD_RELOC_SCORE_BCMP)
    {
      retval = 1;
    }
  return retval;
}

static bool
s3_fix_adjustable (fixS * fixP)
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
s3_elf_final_processing (void)
{
  unsigned long val = 0;

  if (score3)
    val = E_SCORE_MACH_SCORE3;
  else if (score7)
    val = E_SCORE_MACH_SCORE7;

  elf_elfheader (stdoutput)->e_machine = EM_SCORE;
  elf_elfheader (stdoutput)->e_flags &= ~EF_SCORE_MACH;
  elf_elfheader (stdoutput)->e_flags |= val;

  if (s3_fix_data_dependency == 1)
    {
      elf_elfheader (stdoutput)->e_flags |= EF_SCORE_FIXDEP;
    }
  if (s3_score_pic == s3_PIC)
    {
      elf_elfheader (stdoutput)->e_flags |= EF_SCORE_PIC;
    }
}

static int
s3_judge_size_before_relax (fragS * fragp, asection *sec)
{
  int change = 0;

  if (s3_score_pic == s3_NO_PIC)
    change = s3_nopic_need_relax (fragp->fr_symbol, 0);
  else
    change = s3_pic_need_relax (fragp->fr_symbol, sec);

  if (change == 1)
    {
      /* Only at the first time determining whether s3_GP instruction relax should be done,
         return the difference between instruction size and instruction relax size.  */
      if (fragp->fr_opcode == NULL)
	{
	  fragp->fr_fix = s3_RELAX_NEW (fragp->fr_subtype);
	  fragp->fr_opcode = fragp->fr_literal + s3_RELAX_RELOC1 (fragp->fr_subtype);
          return s3_RELAX_NEW (fragp->fr_subtype) - s3_RELAX_OLD (fragp->fr_subtype);
	}
    }

  return 0;
}

static int
s3_estimate_size_before_relax (fragS * fragp, asection * sec ATTRIBUTE_UNUSED)
{
  if ((s3_RELAX_TYPE (fragp->fr_subtype) == Insn_GP)
      || (s3_RELAX_TYPE (fragp->fr_subtype) == Insn_PIC))
    return s3_judge_size_before_relax (fragp, sec);

  return 0;
}

static int
s3_relax_branch_inst32 (fragS * fragp)
{
  fragp->fr_opcode = NULL;
  return 0;
}

static int
s3_relax_branch_inst16 (fragS * fragp)
{
  int relaxable_p = 0;
  int frag_addr = fragp->fr_address + fragp->insn_addr;
  addressT symbol_address = 0;
  symbolS *s;
  offsetT offset;
  long value;
  unsigned long inst_value;

  relaxable_p = s3_RELAX_OPT (fragp->fr_subtype);

  s = fragp->fr_symbol;
  if (s == NULL)
    frag_addr = 0;
  else
    symbol_address = (addressT) symbol_get_frag (s)->fr_address;

  inst_value = s3_md_chars_to_number (fragp->fr_literal, s3_INSN16_SIZE);
  offset = (inst_value & 0x1ff) << 1;
  if ((offset & 0x200) == 0x200)
    offset |= 0xfffffc00;

  value = offset + symbol_address - frag_addr;

  if (relaxable_p
      && (!((value & 0xfffffe00) == 0 || (value & 0xfffffe00) == 0xfffffe00))
      && fragp->fr_fix == 2
      && (S_IS_DEFINED (s)
          && !S_IS_COMMON (s)
          && !S_IS_EXTERNAL (s)))
    {
      /* Relax branch 32 to branch 16.  */
      fragp->fr_opcode = fragp->fr_literal + s3_RELAX_RELOC1 (fragp->fr_subtype);
      fragp->fr_fix = 4;
      return 2;
    }
  else
    return 0;
}

static int
s3_relax_cmpbranch_inst32 (fragS * fragp)
{
  int relaxable_p = 0;
  symbolS *s;
  /* For sign bit.  */
  long offset;
  long frag_addr = fragp->fr_address + fragp->insn_addr;
  long symbol_address = 0;
  long value;
  unsigned long inst_value;

  relaxable_p = s3_RELAX_OPT (fragp->fr_subtype);

  s = fragp->fr_symbol;
  if (s == NULL)
    frag_addr = 0;
  else
    symbol_address = (addressT) symbol_get_frag (s)->fr_address;

  inst_value = s3_md_chars_to_number (fragp->fr_literal, s3_INSN_SIZE);
  offset = (inst_value & 0x1)
    | (((inst_value >> 7) & 0x7) << 1)
    | (((inst_value >> 21) & 0x1f) << 4);
  offset <<= 1;
  if ((offset & 0x200) == 0x200)
    offset |= 0xfffffe00;

  value = offset + symbol_address - frag_addr;
  /* change the order of judging rule is because
     1.not defined symbol or common symbol or external symbol will change
     bcmp to cmp!+beq/bne ,here need to record fragp->fr_opcode
     2.if the flow is as before : it will results to recursive loop
  */
  if (fragp->fr_fix == 6)
    {
      /* Have already relaxed!  Just return 0 to terminate the loop.  */
      return 0;
    }
  /* need to translate when extern or not defined or common symbol */
  else if ((relaxable_p
	    && (!((value & 0xfffffe00) == 0 || (value & 0xfffffe00) == 0xfffffe00))
	    && fragp->fr_fix == 4)
	   || !S_IS_DEFINED (s)
	   ||S_IS_COMMON (s)
	   ||S_IS_EXTERNAL (s))
    {
      fragp->fr_opcode = fragp->fr_literal + s3_RELAX_RELOC1 (fragp->fr_subtype);
      fragp->fr_fix = 6;
      return 2;
    }
  else
    {
      /* Never relax.  Modify fr_opcode to NULL to verify it's value in
         md_apply_fix.  */
      fragp->fr_opcode = NULL;
      return 0;
    }
}


static int
s3_relax_other_inst32 (fragS * fragp)
{
  int relaxable_p = s3_RELAX_OPT (fragp->fr_subtype);

  if (relaxable_p
      && fragp->fr_fix == 4)
    {
      fragp->fr_opcode = fragp->fr_literal + s3_RELAX_RELOC1 (fragp->fr_subtype);
      fragp->fr_fix = 2;
      return -2;
    }
  else
    return 0;
}

static int
s3_relax_gp_and_pic_inst32 (void)
{
  /* md_estimate_size_before_relax has already relaxed s3_GP and s3_PIC
     instructions.  We don't change relax size here.  */
  return 0;
}

static int
s3_relax_frag (asection * sec ATTRIBUTE_UNUSED, fragS * fragp, long stretch ATTRIBUTE_UNUSED)
{
  int grows = 0;
  int adjust_align_p = 0;

  /* If the instruction address is odd, make it half word align first.  */
  if ((fragp->fr_address) % 2 != 0)
    {
      if ((fragp->fr_address + fragp->insn_addr) % 2 != 0)
	{
          fragp->insn_addr = 1;
          grows += 1;
          adjust_align_p = 1;
	}
    }

  switch (s3_RELAX_TYPE (fragp->fr_subtype))
    {
    case PC_DISP19div2:
      grows += s3_relax_branch_inst32 (fragp);
      break;

    case PC_DISP8div2:
      grows += s3_relax_branch_inst16 (fragp);
      break;

    case Insn_BCMP :
      grows += s3_relax_cmpbranch_inst32 (fragp);
      break;

    case Insn_GP:
    case Insn_PIC:
      grows += s3_relax_gp_and_pic_inst32 ();
      break;

    default:
      grows += s3_relax_other_inst32 (fragp);
      break;
    }

  /* newly added */
  if (adjust_align_p && fragp->insn_addr)
    {
      fragp->fr_fix += fragp->insn_addr;
    }

  return grows;
}

static void
s3_convert_frag (bfd * abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED, fragS * fragp)
{
  unsigned int r_old;
  unsigned int r_new;
  char backup[20];
  fixS *fixp;

  r_old = s3_RELAX_OLD (fragp->fr_subtype);
  r_new = s3_RELAX_NEW (fragp->fr_subtype);

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
      s3_md_number_to_chars (fragp->fr_literal, 0x0, fragp->insn_addr);
    }
  memcpy (fragp->fr_literal + fragp->insn_addr, backup, fragp->fr_fix);
  fragp->fr_fix += fragp->insn_addr;
}

static long
s3_pcrel_from (fixS * fixP)
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

static valueT
s3_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
  int align = bfd_section_alignment (segment);
  return ((size + (1 << align) - 1) & -(1 << align));
}

static void
s3_apply_fix (fixS *fixP, valueT *valP, segT seg)
{
  valueT value = *valP;
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
      if (fixP->fx_done)        /* For la rd, imm32.  */
        {
          newval = s3_md_chars_to_number (buf, s3_INSN_SIZE);
          HI = value >> 16;   /* mul to 2, then take the hi 16 bit.  */
          newval |= (HI & 0x3fff) << 1;
          newval |= ((HI >> 14) & 0x3) << 16;
          s3_md_number_to_chars (buf, newval, s3_INSN_SIZE);
        }
      break;
    case BFD_RELOC_LO16:
      if (fixP->fx_done)        /* For la rd, imm32.  */
        {
          newval = s3_md_chars_to_number (buf, s3_INSN_SIZE);
          LO = value & 0xffff;
          newval |= (LO & 0x3fff) << 1; /* 16 bit: imm -> 14 bit in lo, 2 bit in hi.  */
          newval |= ((LO >> 14) & 0x3) << 16;
          s3_md_number_to_chars (buf, newval, s3_INSN_SIZE);
        }
      break;
    case BFD_RELOC_SCORE_JMP:
      {
        content = s3_md_chars_to_number (buf, s3_INSN_SIZE);
        value = fixP->fx_offset;
        content = (content & ~0x3ff7ffe) | ((value << 1) & 0x3ff0000) | (value & 0x7fff);
        s3_md_number_to_chars (buf, content, s3_INSN_SIZE);
      }
      break;

    case BFD_RELOC_SCORE_IMM30:
      {
        content = s3_md_chars_to_number (buf, s3_INSN48_SIZE);
        value = fixP->fx_offset;
        value >>= 2;
        content = (content & ~0x7f7fff7f80LL)
	  | (((value & 0xff) >> 0) << 7)
	  | (((value & 0x7fff00) >> 8) << 16)
	  | (((value & 0x3f800000) >> 23) << 32);
        s3_md_number_to_chars (buf, content, s3_INSN48_SIZE);
        break;
      }

    case BFD_RELOC_SCORE_IMM32:
      {
        content = s3_md_chars_to_number (buf, s3_INSN48_SIZE);
        value = fixP->fx_offset;
        content = (content & ~0x7f7fff7fe0LL)
	  | ((value & 0x3ff) << 5)
	  | (((value >> 10) & 0x7fff) << 16)
	  | (((value >> 25) & 0x7f) << 32);
        s3_md_number_to_chars (buf, content, s3_INSN48_SIZE);
        break;
      }

    case BFD_RELOC_SCORE_BRANCH:
      if ((S_GET_SEGMENT (fixP->fx_addsy) != seg) || (fixP->fx_addsy != NULL && S_IS_EXTERNAL (fixP->fx_addsy)))
        value = fixP->fx_offset;
      else
        fixP->fx_done = 1;

      content = s3_md_chars_to_number (buf, s3_INSN_SIZE);

      /* Don't check c-bit.  */
      if (fixP->fx_frag->fr_opcode != 0)
        {
          if ((value & 0xfffffe00) != 0 && (value & 0xfffffe00) != 0xfffffe00)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x) [-2^9 ~ 2^9-1]"), (unsigned int) value);
              return;
            }
          content = s3_md_chars_to_number (buf, s3_INSN16_SIZE);
          content &= 0xfe00;
          content = (content & 0xfe00) | ((value >> 1) & 0x1ff);
          s3_md_number_to_chars (buf, content, s3_INSN16_SIZE);
          fixP->fx_r_type = BFD_RELOC_SCORE16_BRANCH;
          fixP->fx_size = 2;
        }
      else
        {
          if ((value & 0xfff80000) != 0 && (value & 0xfff80000) != 0xfff80000)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x) [-2^19 ~ 2^19-1]"), (unsigned int) value);
              return;
            }
          content = s3_md_chars_to_number (buf, s3_INSN_SIZE);
          content &= 0xfc00fc01;
          content = (content & 0xfc00fc01) | (value & 0x3fe) | ((value << 6) & 0x3ff0000);
          s3_md_number_to_chars (buf, content, s3_INSN_SIZE);
        }
      break;
    case BFD_RELOC_SCORE16_JMP:
      content = s3_md_chars_to_number (buf, s3_INSN16_SIZE);
      content &= 0xf001;
      value = fixP->fx_offset & 0xfff;
      content = (content & 0xfc01) | (value & 0xffe);
      s3_md_number_to_chars (buf, content, s3_INSN16_SIZE);
      break;
    case BFD_RELOC_SCORE16_BRANCH:
      content = s3_md_chars_to_number (buf, s3_INSN_SIZE);
      /* Don't check c-bit.  */
      if (fixP->fx_frag->fr_opcode != 0)
        {
          if ((S_GET_SEGMENT (fixP->fx_addsy) != seg) ||
              (fixP->fx_addsy != NULL && S_IS_EXTERNAL (fixP->fx_addsy)))
            value = fixP->fx_offset;
          else
            fixP->fx_done = 1;
          if ((value & 0xfff80000) != 0 && (value & 0xfff80000) != 0xfff80000)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x) [-2^19 ~ 2^19-1]"), (unsigned int) value);
              return;
            }
          content = s3_md_chars_to_number (buf, s3_INSN_SIZE);
          content = (content & 0xfc00fc01) | (value & 0x3fe) | ((value << 6) & 0x3ff0000);
          s3_md_number_to_chars (buf, content, s3_INSN_SIZE);
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

          if ((value & 0xfffffe00) != 0 && (value & 0xfffffe00) != 0xfffffe00)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x) [-2^9 ~ 2^9-1]"), (unsigned int) value);
              return;
            }

          content = s3_md_chars_to_number (buf, s3_INSN16_SIZE);
          content = (content & 0xfe00) | ((value >> 1) & 0x1ff);
          s3_md_number_to_chars (buf, content, s3_INSN16_SIZE);
          break;
        }

      break;

    case BFD_RELOC_SCORE_BCMP:
      if (fixP->fx_frag->fr_opcode != 0)
        {
          char *buf_ptr = buf;
          buf_ptr += 2;

          if ((S_GET_SEGMENT (fixP->fx_addsy) != seg) || (fixP->fx_addsy != NULL && S_IS_EXTERNAL (fixP->fx_addsy)))
            value = fixP->fx_offset;
          else
            fixP->fx_done = 1;

          /* NOTE!!!
             bcmp -> cmp! and branch, so value -= 2.  */
          value -= 2;

          if ((value & 0xfff80000) != 0 && (value & 0xfff80000) != 0xfff80000)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            _(" branch relocation truncate (0x%x) [-2^19 ~ 2^19-1]"), (unsigned int) value);
              return;
            }

          content = s3_md_chars_to_number (buf_ptr, s3_INSN_SIZE);
          content &= 0xfc00fc01;
          content = (content & 0xfc00fc01) | (value & 0x3fe) | ((value << 6) & 0x3ff0000);
          s3_md_number_to_chars (buf_ptr, content, s3_INSN_SIZE);
          /* change relocation type to BFD_RELOC_SCORE_BRANCH */
          fixP->fx_r_type = BFD_RELOC_SCORE_BRANCH;
          fixP->fx_where+=2; /* first insn is cmp! , the second insn is beq/bne */
          break;
        }
      else
        {
          if ((S_GET_SEGMENT (fixP->fx_addsy) != seg) || (fixP->fx_addsy != NULL && S_IS_EXTERNAL (fixP->fx_addsy)))
            value = fixP->fx_offset;
          else
            fixP->fx_done = 1;

          content = s3_md_chars_to_number (buf, s3_INSN_SIZE);

          if ((value & 0xfffffe00) != 0 && (value & 0xfffffe00) != 0xfffffe00)
            {
              as_bad_where (fixP->fx_file, fixP->fx_line,
			    _(" branch relocation truncate (0x%x)  [-2^9 ~ 2^9-1]"), (unsigned int) value);
              return;
            }

          value >>= 1;
          content &= ~0x03e00381;
          content = content
	    | (value & 0x1)
	    | (((value & 0xe) >> 1) << 7)
	    | (((value & 0x1f0) >> 4) << 21);

          s3_md_number_to_chars (buf, content, s3_INSN_SIZE);
          break;
        }

    case BFD_RELOC_8:
      if (fixP->fx_done || fixP->fx_pcrel)
	s3_md_number_to_chars (buf, value, 1);
#ifdef OBJ_ELF
      else
        {
          value = fixP->fx_offset;
          s3_md_number_to_chars (buf, value, 1);
        }
#endif
      break;

    case BFD_RELOC_16:
      if (fixP->fx_done || fixP->fx_pcrel)
        s3_md_number_to_chars (buf, value, 2);
#ifdef OBJ_ELF
      else
        {
          value = fixP->fx_offset;
          s3_md_number_to_chars (buf, value, 2);
        }
#endif
      break;
    case BFD_RELOC_RVA:
    case BFD_RELOC_32:
      if (fixP->fx_done || fixP->fx_pcrel)
        md_number_to_chars (buf, value, 4);
#ifdef OBJ_ELF
      else
        {
          value = fixP->fx_offset;
          md_number_to_chars (buf, value, 4);
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
      content = s3_md_chars_to_number (buf, s3_INSN_SIZE);
      /* c-bit.  */
      if ((fixP->fx_frag->fr_opcode != 0) && ((content & 0xfc1c8000) != 0x94180000))
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

static arelent **
s3_gen_reloc (asection * section ATTRIBUTE_UNUSED, fixS * fixp)
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
      newval = s3_md_chars_to_number (buf, s3_INSN_SIZE);
      off = fixp->fx_offset >> 16;
      newval |= (off & 0x3fff) << 1;
      newval |= ((off >> 14) & 0x3) << 16;
      s3_md_number_to_chars (buf, newval, s3_INSN_SIZE);

      buf += s3_INSN_SIZE;
      newval = s3_md_chars_to_number (buf, s3_INSN_SIZE);
      off = fixp->fx_offset & 0xffff;
      newval |= ((off & 0x3fff) << 1);
      newval |= (((off >> 14) & 0x3) << 16);
      s3_md_number_to_chars (buf, newval, s3_INSN_SIZE);

      retval[1] = XNEW (arelent);
      retval[2] = NULL;
      retval[1]->sym_ptr_ptr = XNEW (asymbol *);
      *retval[1]->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
      retval[1]->address = (reloc->address + s3_RELAX_RELOC2 (fixp->fx_frag->fr_subtype));

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
    case BFD_RELOC_SCORE_BCMP:
    case BFD_RELOC_VTABLE_ENTRY:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_SCORE_GPREL15:
    case BFD_RELOC_SCORE_GOT15:
    case BFD_RELOC_SCORE_DUMMY_HI16:
    case BFD_RELOC_SCORE_GOT_LO16:
    case BFD_RELOC_SCORE_CALL15:
    case BFD_RELOC_GPREL32:
    case BFD_RELOC_NONE:
    case BFD_RELOC_SCORE_IMM30:
    case BFD_RELOC_SCORE_IMM32:
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

void
md_assemble (char *str)
{
  if (score3)
    s3_assemble (str);
  else
    s7_assemble (str);
}

/* We handle all bad expressions here, so that we can report the faulty
   instruction in the error message.  */
void
md_operand (expressionS * exp)
{
  if (score3)
    s3_operand (exp);
  else
    s7_operand (exp);
}

/* Turn an integer of n bytes (in val) into a stream of bytes appropriate
   for use in the a.out file, and stores them in the array pointed to by buf.
   This knows about the endian-ness of the target machine and does
   THE RIGHT THING, whatever it is.  Possible values for n are 1 (byte)
   2 (short) and 4 (long)  Floating numbers are put out as a series of
   LITTLENUMS (shorts, here at least).  */
void
md_number_to_chars (char *buf, valueT val, int n)
{
  if (score3)
    s3_number_to_chars (buf, val, n);
  else
    s7_number_to_chars (buf, val, n);
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
const char *
md_atof (int type, char *litP, int *sizeP)
{
  if (score3)
    return s3_atof (type, litP, sizeP);
  else
    return s7_atof (type, litP, sizeP);
}

void
score_frag_check (fragS * fragp ATTRIBUTE_UNUSED)
{
  if (score3)
    s3_frag_check (fragp);
  else
    s7_frag_check (fragp);
}

/* Implementation of TC_VALIDATE_FIX.
   Called before md_apply_fix() and after md_convert_frag().  */
void
score_validate_fix (fixS *fixP)
{
  if (score3)
    s3_validate_fix (fixP);
  else
    s7_validate_fix (fixP);
}

int
score_force_relocation (struct fix *fixp)
{
  if (score3)
    return s3_force_relocation (fixp);
  else
    return s7_force_relocation (fixp);
}

/* Implementation of md_frag_check.
   Called after md_convert_frag().  */
bool
score_fix_adjustable (fixS * fixP)
{
  if (score3)
    return s3_fix_adjustable (fixP);
  else
    return s7_fix_adjustable (fixP);
}

void
score_elf_final_processing (void)
{
  if (score3)
    s3_elf_final_processing ();
  else
    s7_elf_final_processing ();
}

/* In this function, we determine whether s3_GP instruction should do relaxation,
   for the label being against was known now.
   Doing this here but not in md_relax_frag() can induce iteration times
   in stage of doing relax.  */
int
md_estimate_size_before_relax (fragS * fragp, asection * sec ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_estimate_size_before_relax (fragp, sec);
  else
    return s7_estimate_size_before_relax (fragp, sec);
}

int
score_relax_frag (asection * sec ATTRIBUTE_UNUSED, fragS * fragp, long stretch ATTRIBUTE_UNUSED)
{
  if (score3)
    return s3_relax_frag (sec, fragp, stretch);
  else
    return s7_relax_frag (sec, fragp, stretch);
}

void
md_convert_frag (bfd * abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED, fragS * fragp)
{
  if (score3)
    return s3_convert_frag (abfd, sec, fragp);
  else
    return s7_convert_frag (abfd, sec, fragp);
}

long
md_pcrel_from (fixS * fixP)
{
  if (score3)
    return s3_pcrel_from (fixP);
  else
    return s7_pcrel_from (fixP);
}

/* Round up a section size to the appropriate boundary.  */
valueT
md_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
  if (score3)
    return s3_section_align (segment, size);
  else
    return s7_section_align (segment, size);
}

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg)
{
  if (score3)
    return s3_apply_fix (fixP, valP, seg);
  else
    return s7_apply_fix (fixP, valP, seg);
}

/* Translate internal representation of relocation info to BFD target format.  */
arelent **
tc_gen_reloc (asection * section ATTRIBUTE_UNUSED, fixS * fixp)
{
  if (score3)
    return s3_gen_reloc (section, fixp);
  else
    return s7_gen_reloc (section, fixp);
}

void
md_begin (void)
{
  s3_begin ();
  s7_begin ();
}

static void
score_set_mach (const char *arg)
{
  if (strcmp (arg, MARCH_SCORE3) == 0)
    {
      score3 = 1;
      score7 = 0;
      s3_score3d = 1;
    }
  else if (strcmp (arg, MARCH_SCORE7) == 0)
    {
      score3 = 0;
      score7 = 1;
      s7_score7d = 1;
      s7_university_version = 0;
      s7_vector_size = s7_SCORE7_PIPELINE;
    }
  else if (strcmp (arg, MARCH_SCORE5) == 0)
    {
      score3 = 0;
      score7 = 1;
      s7_score7d = 1;
      s7_university_version = 0;
      s7_vector_size = s7_SCORE5_PIPELINE;
    }
  else if (strcmp (arg, MARCH_SCORE5U) == 0)
    {
      score3 = 0;
      score7 = 1;
      s7_score7d = 1;
      s7_university_version = 1;
      s7_vector_size = s7_SCORE5_PIPELINE;
    }
  else
    {
      as_bad (_("unknown architecture `%s'\n"), arg);
    }
}

int
md_parse_option (int c, const char *arg)
{
  switch (c)
    {
#ifdef OPTION_EB
    case OPTION_EB:
      target_big_endian = 1;
      break;
#endif
#ifdef OPTION_EL
    case OPTION_EL:
      target_big_endian = 0;
      break;
#endif
    case OPTION_FIXDD:
      s3_fix_data_dependency = 1;
      s7_fix_data_dependency = 1;
      break;
    case OPTION_NWARN:
      s3_warn_fix_data_dependency = 0;
      s7_warn_fix_data_dependency = 0;
      break;
    case OPTION_SCORE5:
      score3 = 0;
      score7 = 1;
      s7_university_version = 0;
      s7_vector_size = s7_SCORE5_PIPELINE;
      break;
    case OPTION_SCORE5U:
      score3 = 0;
      score7 = 1;
      s7_university_version = 1;
      s7_vector_size = s7_SCORE5_PIPELINE;
      break;
    case OPTION_SCORE7:
      score3 = 0;
      score7 = 1;
      s7_score7d = 1;
      s7_university_version = 0;
      s7_vector_size = s7_SCORE7_PIPELINE;
      break;
    case OPTION_SCORE3:
      score3 = 1;
      score7 = 0;
      s3_score3d = 1;
      break;
    case OPTION_R1:
      s3_nor1 = 0;
      s7_nor1 = 0;
      break;
    case 'G':
      s3_g_switch_value = atoi (arg);
      s7_g_switch_value = atoi (arg);
      break;
    case OPTION_O0:
      s3_g_opt = 0;
      s7_g_opt = 0;
      break;
    case OPTION_SCORE_VERSION:
      printf (_("Sunplus-v2-0-0-20060510\n"));
      break;
    case OPTION_PIC:
      s3_score_pic = s3_NO_PIC; /* Score3 doesn't support PIC now.  */
      s7_score_pic = s7_PIC;
      s3_g_switch_value = 0;    /* Must set -G num as 0 to generate s3_PIC code.  */
      s7_g_switch_value = 0;    /* Must set -G num as 0 to generate s7_PIC code.  */
      break;
    case OPTION_MARCH:
      score_set_mach (arg);
      break;
    default:
      return 0;
    }
  return 1;
}

void
md_show_usage (FILE * fp)
{
  fprintf (fp, _(" Score-specific assembler options:\n"));
#ifdef OPTION_EB
  fprintf (fp, _("\
        -EB\t\tassemble code for a big-endian cpu\n"));
#endif

#ifdef OPTION_EL
  fprintf (fp, _("\
        -EL\t\tassemble code for a little-endian cpu\n"));
#endif

  fprintf (fp, _("\
        -FIXDD\t\tfix data dependencies\n"));
  fprintf (fp, _("\
        -NWARN\t\tdo not print warning message when fixing data dependencies\n"));
  fprintf (fp, _("\
        -SCORE5\t\tassemble code for target SCORE5\n"));
  fprintf (fp, _("\
        -SCORE5U\tassemble code for target SCORE5U\n"));
  fprintf (fp, _("\
        -SCORE7\t\tassemble code for target SCORE7 [default]\n"));
  fprintf (fp, _("\
        -SCORE3\t\tassemble code for target SCORE3\n"));
  fprintf (fp, _("\
        -march=score7\tassemble code for target SCORE7 [default]\n"));
  fprintf (fp, _("\
        -march=score3\tassemble code for target SCORE3\n"));
  fprintf (fp, _("\
        -USE_R1\t\tassemble code for no warning message when using temp register r1\n"));
  fprintf (fp, _("\
        -KPIC\t\tgenerate PIC\n"));
  fprintf (fp, _("\
        -O0\t\tdo not perform any optimizations\n"));
  fprintf (fp, _("\
        -G gpnum\tassemble code for setting gpsize, default is 8 bytes\n"));
  fprintf (fp, _("\
        -V \t\tSunplus release version\n"));
}
