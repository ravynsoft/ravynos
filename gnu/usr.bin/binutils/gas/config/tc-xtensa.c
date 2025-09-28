/* tc-xtensa.c -- Assemble Xtensa instructions.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.

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
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "as.h"
#include <limits.h>
#include "sb.h"
#include "safe-ctype.h"
#include "tc-xtensa.h"
#include "subsegs.h"
#include "xtensa-relax.h"
#include "dwarf2dbg.h"
#include "xtensa-istack.h"
#include "xtensa-dynconfig.h"
#include "elf/xtensa.h"

#ifndef uint32
#define uint32 unsigned int
#endif
#ifndef int32
#define int32 signed int
#endif

/* Notes:

   Naming conventions (used somewhat inconsistently):
      The xtensa_ functions are exported
      The xg_ functions are internal

   We also have a couple of different extensibility mechanisms.
   1) The idiom replacement:
      This is used when a line is first parsed to
      replace an instruction pattern with another instruction
      It is currently limited to replacements of instructions
      with constant operands.
   2) The xtensa-relax.c mechanism that has stronger instruction
      replacement patterns.  When an instruction's immediate field
      does not fit the next instruction sequence is attempted.
      In addition, "narrow" opcodes are supported this way.  */


/* Define characters with special meanings to GAS.  */
const char comment_chars[] = "#";
const char line_comment_chars[] = "#";
const char line_separator_chars[] = ";";
const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "rRsSfFdDxXpP";


/* Flags to indicate whether the hardware supports the density and
   absolute literals options.  */

bool density_supported;
bool absolute_literals_supported;

static unsigned microarch_earliest;

static vliw_insn cur_vinsn;

unsigned xtensa_num_pipe_stages;
unsigned xtensa_fetch_width;

static enum debug_info_type xt_saved_debug_type = DEBUG_NONE;

/* Some functions are only valid in the front end.  This variable
   allows us to assert that we haven't crossed over into the
   back end.  */
static bool past_xtensa_md_finish = false;

/* Flags for properties of the last instruction in a segment.  */
#define FLAG_IS_A0_WRITER	0x1
#define FLAG_IS_BAD_LOOPEND	0x2


/* We define a special segment names ".literal" to place literals
   into.  The .fini and .init sections are special because they
   contain code that is moved together by the linker.  We give them
   their own special .fini.literal and .init.literal sections.  */

#define LITERAL_SECTION_NAME		xtensa_section_rename (".literal")
#define LIT4_SECTION_NAME		xtensa_section_rename (".lit4")
#define INIT_SECTION_NAME		xtensa_section_rename (".init")
#define FINI_SECTION_NAME		xtensa_section_rename (".fini")


/* This type is used for the directive_stack to keep track of the
   state of the literal collection pools.  If lit_prefix is set, it is
   used to determine the literal section names; otherwise, the literal
   sections are determined based on the current text section.  The
   lit_seg and lit4_seg fields cache these literal sections, with the
   current_text_seg field used a tag to indicate whether the cached
   values are valid.  */

typedef struct lit_state_struct
{
  char *lit_prefix;
  segT current_text_seg;
  segT lit_seg;
  segT lit4_seg;
} lit_state;

static lit_state default_lit_sections;


/* We keep a list of literal segments.  The seg_list type is the node
   for this list.  The literal_head pointer is the head of the list,
   with the literal_head_h dummy node at the start.  */

typedef struct seg_list_struct
{
  struct seg_list_struct *next;
  segT seg;
} seg_list;

static seg_list literal_head_h;
static seg_list *literal_head = &literal_head_h;


/* Lists of symbols.  We keep a list of symbols that label the current
   instruction, so that we can adjust the symbols when inserting alignment
   for various instructions.  We also keep a list of all the symbols on
   literals, so that we can fix up those symbols when the literals are
   later moved into the text sections.  */

typedef struct sym_list_struct
{
  struct sym_list_struct *next;
  symbolS *sym;
} sym_list;

static sym_list *insn_labels = NULL;
static sym_list *free_insn_labels = NULL;
static sym_list *saved_insn_labels = NULL;

static sym_list *literal_syms;


/* Flags to determine whether to prefer const16 or l32r
   if both options are available.  */
int prefer_const16 = 0;
int prefer_l32r = 0;

/* Global flag to indicate when we are emitting literals.  */
int generating_literals = 0;

/* The following PROPERTY table definitions are copied from
   <elf/xtensa.h> and must be kept in sync with the code there.  */

/* Flags in the property tables to specify whether blocks of memory
   are literals, instructions, data, or unreachable.  For
   instructions, blocks that begin loop targets and branch targets are
   designated.  Blocks that do not allow density, instruction
   reordering or transformation are also specified.  Finally, for
   branch targets, branch target alignment priority is included.
   Alignment of the next block is specified in the current block
   and the size of the current block does not include any fill required
   to align to the next block.  */

#define XTENSA_PROP_LITERAL		0x00000001
#define XTENSA_PROP_INSN		0x00000002
#define XTENSA_PROP_DATA		0x00000004
#define XTENSA_PROP_UNREACHABLE		0x00000008
/* Instruction only properties at beginning of code.  */
#define XTENSA_PROP_INSN_LOOP_TARGET	0x00000010
#define XTENSA_PROP_INSN_BRANCH_TARGET	0x00000020
/* Instruction only properties about code.  */
#define XTENSA_PROP_INSN_NO_DENSITY	0x00000040
#define XTENSA_PROP_INSN_NO_REORDER	0x00000080
/* Historically, NO_TRANSFORM was a property of instructions,
   but it should apply to literals under certain circumstances.  */
#define XTENSA_PROP_NO_TRANSFORM	0x00000100

/*  Branch target alignment information.  This transmits information
    to the linker optimization about the priority of aligning a
    particular block for branch target alignment: None, low priority,
    high priority, or required.  These only need to be checked in
    instruction blocks marked as XTENSA_PROP_INSN_BRANCH_TARGET.
    Common usage is

    switch (GET_XTENSA_PROP_BT_ALIGN (flags))
    case XTENSA_PROP_BT_ALIGN_NONE:
    case XTENSA_PROP_BT_ALIGN_LOW:
    case XTENSA_PROP_BT_ALIGN_HIGH:
    case XTENSA_PROP_BT_ALIGN_REQUIRE:
*/
#define XTENSA_PROP_BT_ALIGN_MASK       0x00000600

/* No branch target alignment.  */
#define XTENSA_PROP_BT_ALIGN_NONE       0x0
/* Low priority branch target alignment.  */
#define XTENSA_PROP_BT_ALIGN_LOW        0x1
/* High priority branch target alignment.  */
#define XTENSA_PROP_BT_ALIGN_HIGH       0x2
/* Required branch target alignment.  */
#define XTENSA_PROP_BT_ALIGN_REQUIRE    0x3

#define SET_XTENSA_PROP_BT_ALIGN(flag, align) \
  (((flag) & (~XTENSA_PROP_BT_ALIGN_MASK)) | \
    (((align) << 9) & XTENSA_PROP_BT_ALIGN_MASK))


/* Alignment is specified in the block BEFORE the one that needs
   alignment.  Up to 5 bits.  Use GET_XTENSA_PROP_ALIGNMENT(flags) to
   get the required alignment specified as a power of 2.  Use
   SET_XTENSA_PROP_ALIGNMENT(flags, pow2) to set the required
   alignment.  Be careful of side effects since the SET will evaluate
   flags twice.  Also, note that the SIZE of a block in the property
   table does not include the alignment size, so the alignment fill
   must be calculated to determine if two blocks are contiguous.
   TEXT_ALIGN is not currently implemented but is a placeholder for a
   possible future implementation.  */

#define XTENSA_PROP_ALIGN		0x00000800

#define XTENSA_PROP_ALIGNMENT_MASK      0x0001f000

#define SET_XTENSA_PROP_ALIGNMENT(flag, align) \
  (((flag) & (~XTENSA_PROP_ALIGNMENT_MASK)) | \
    (((align) << 12) & XTENSA_PROP_ALIGNMENT_MASK))

#define XTENSA_PROP_INSN_ABSLIT 0x00020000


/* Structure for saving instruction and alignment per-fragment data
   that will be written to the object file.  This structure is
   equivalent to the actual data that will be written out to the file
   but is easier to use.   We provide a conversion to file flags
   in frag_flags_to_number.  */

typedef struct frag_flags_struct frag_flags;

struct frag_flags_struct
{
  /* is_literal should only be used after xtensa_move_literals.
     If you need to check if you are generating a literal fragment,
     then use the generating_literals global.  */

  unsigned is_literal : 1;
  unsigned is_insn : 1;
  unsigned is_data : 1;
  unsigned is_unreachable : 1;

  /* is_specific_opcode implies no_transform.  */
  unsigned is_no_transform : 1;

  struct
  {
    unsigned is_loop_target : 1;
    unsigned is_branch_target : 1; /* Branch targets have a priority.  */
    unsigned bt_align_priority : 2;

    unsigned is_no_density : 1;
    /* no_longcalls flag does not need to be placed in the object file.  */

    unsigned is_no_reorder : 1;

    /* Uses absolute literal addressing for l32r.  */
    unsigned is_abslit : 1;
  } insn;
  unsigned is_align : 1;
  unsigned alignment : 5;
};


/* Structure for saving information about a block of property data
   for frags that have the same flags.  */
struct xtensa_block_info_struct
{
  segT sec;
  bfd_vma offset;
  size_t size;
  frag_flags flags;
  struct xtensa_block_info_struct *next;
};


/* Structure for saving the current state before emitting literals.  */
typedef struct emit_state_struct
{
  const char *name;
  segT now_seg;
  subsegT now_subseg;
  int generating_literals;
} emit_state;


/* Opcode placement information */

typedef unsigned long long bitfield;
#define bit_is_set(bit, bf)	((bf) & (0x01ll << (bit)))
#define set_bit(bit, bf)	((bf) |= (0x01ll << (bit)))
#define clear_bit(bit, bf)	((bf) &= ~(0x01ll << (bit)))

#define MAX_FORMATS 32

typedef struct op_placement_info_struct
{
  int num_formats;
  /* A number describing how restrictive the issue is for this
     opcode.  For example, an opcode that fits lots of different
     formats has a high freedom, as does an opcode that fits
     only one format but many slots in that format.  The most
     restrictive is the opcode that fits only one slot in one
     format.  */
  int issuef;
  xtensa_format narrowest;
  char narrowest_size;
  char narrowest_slot;

  /* formats is a bitfield with the Nth bit set
     if the opcode fits in the Nth xtensa_format.  */
  bitfield formats;

  /* slots[N]'s Mth bit is set if the op fits in the
     Mth slot of the Nth xtensa_format.  */
  bitfield slots[MAX_FORMATS];

  /* A count of the number of slots in a given format
     an op can fit (i.e., the bitcount of the slot field above).  */
  char slots_in_format[MAX_FORMATS];

} op_placement_info, *op_placement_info_table;

op_placement_info_table op_placement_table;


/* Extra expression types.  */

#define O_pltrel	O_md1	/* like O_symbol but use a PLT reloc */
#define O_hi16		O_md2	/* use high 16 bits of symbolic value */
#define O_lo16		O_md3	/* use low 16 bits of symbolic value */
#define O_pcrel		O_md4	/* value is a PC-relative offset */
#define O_tlsfunc	O_md5	/* TLS_FUNC/TLSDESC_FN relocation */
#define O_tlsarg	O_md6	/* TLS_ARG/TLSDESC_ARG relocation */
#define O_tlscall	O_md7	/* TLS_CALL relocation */
#define O_tpoff		O_md8	/* TPOFF relocation */
#define O_dtpoff	O_md9	/* DTPOFF relocation */

struct suffix_reloc_map
{
  const char *suffix;
  int length;
  bfd_reloc_code_real_type reloc;
  operatorT operator;
};

#define SUFFIX_MAP(str, reloc, op) { str, sizeof (str) - 1, reloc, op }

static struct suffix_reloc_map suffix_relocs[] =
{
  SUFFIX_MAP ("l",	BFD_RELOC_LO16,			O_lo16),
  SUFFIX_MAP ("h",	BFD_RELOC_HI16,			O_hi16),
  SUFFIX_MAP ("plt",	BFD_RELOC_XTENSA_PLT,		O_pltrel),
  SUFFIX_MAP ("pcrel",	BFD_RELOC_32_PCREL,		O_pcrel),
  SUFFIX_MAP ("tlsfunc", BFD_RELOC_XTENSA_TLS_FUNC,	O_tlsfunc),
  SUFFIX_MAP ("tlsarg",	BFD_RELOC_XTENSA_TLS_ARG,	O_tlsarg),
  SUFFIX_MAP ("tlscall", BFD_RELOC_XTENSA_TLS_CALL,	O_tlscall),
  SUFFIX_MAP ("tpoff",	BFD_RELOC_XTENSA_TLS_TPOFF,	O_tpoff),
  SUFFIX_MAP ("dtpoff",	BFD_RELOC_XTENSA_TLS_DTPOFF,	O_dtpoff),
};


/* Directives.  */

typedef enum
{
  directive_none = 0,
  directive_literal,
  directive_density,
  directive_transform,
  directive_freeregs,
  directive_longcalls,
  directive_literal_prefix,
  directive_schedule,
  directive_absolute_literals,
  directive_last_directive
} directiveE;

typedef struct
{
  const char *name;
  bool can_be_negated;
} directive_infoS;

const directive_infoS directive_info[] =
{
  { "none",		false },
  { "literal",		false },
  { "density",		true },
  { "transform",	true },
  { "freeregs",		false },
  { "longcalls",	true },
  { "literal_prefix",	false },
  { "schedule",		true },
  { "absolute-literals", true }
};

bool directive_state[] =
{
  false,			/* none */
  false,			/* literal */
  false,			/* density */
  true,				/* transform */
  false,			/* freeregs */
  false,			/* longcalls */
  false,			/* literal_prefix */
  false,			/* schedule */
  false				/* absolute_literals */
};

/* A circular list of all potential and actual literal pool locations
   in a segment.  */
struct litpool_frag
{
  struct litpool_frag *next;
  struct litpool_frag *prev;
  fragS *fragP;
  addressT addr;
  short priority; /* 1, 2, or 3 -- 1 is highest  */
  short original_priority;
  int literal_count;
};

/* Map a segment to its litpool_frag list.  */
struct litpool_seg
{
  struct litpool_seg *next;
  asection *seg;
  struct litpool_frag frag_list;
  int frag_count; /* since last litpool location  */
};

static struct litpool_seg litpool_seg_list;

/* Limit maximal size of auto litpool by half of the j range.  */
#define MAX_AUTO_POOL_LITERALS 16384

/* Limit maximal size of explicit literal pool by l32r range.  */
#define MAX_EXPLICIT_POOL_LITERALS 65536

#define MAX_POOL_LITERALS \
  (auto_litpools ? MAX_AUTO_POOL_LITERALS : MAX_EXPLICIT_POOL_LITERALS)

/* Directive functions.  */

static void xtensa_begin_directive (int);
static void xtensa_end_directive (int);
static void xtensa_literal_prefix (void);
static void xtensa_literal_position (int);
static void xtensa_literal_pseudo (int);
static void xtensa_frequency_pseudo (int);
static void xtensa_elf_cons (int);
static void xtensa_leb128 (int);

/* Parsing and Idiom Translation.  */

static bfd_reloc_code_real_type xtensa_elf_suffix (char **, expressionS *);

/* Various Other Internal Functions.  */

extern bool xg_is_single_relaxable_insn (TInsn *, TInsn *, bool);
static bool xg_build_to_insn (TInsn *, TInsn *, BuildInstr *);
static void xtensa_mark_literal_pool_location (void);
static addressT get_expanded_loop_offset (xtensa_opcode);
static fragS *get_literal_pool_location (segT);
static void set_literal_pool_location (segT, fragS *);
static void xtensa_set_frag_assembly_state (fragS *);
static void finish_vinsn (vliw_insn *);
static bool emit_single_op (TInsn *);
static int total_frag_text_expansion (fragS *);
static bool use_trampolines = true;
static void xtensa_check_frag_count (void);
static void xtensa_create_trampoline_frag (bool);
static void xtensa_maybe_create_trampoline_frag (void);
struct trampoline_frag;
static int init_trampoline_frag (fragS *);
static fixS *xg_append_jump (fragS *fragP, symbolS *sym, offsetT offset);
static void xtensa_maybe_create_literal_pool_frag (bool, bool);
static bool auto_litpools = false;
static int auto_litpool_limit = 0;
static bool xtensa_is_init_fini (segT seg);

/* Alignment Functions.  */

static int get_text_align_power (unsigned);
static int get_text_align_max_fill_size (int, bool, bool);
static int branch_align_power (segT);

/* Helpers for xtensa_relax_frag().  */

static long relax_frag_add_nop (fragS *);

/* Accessors for additional per-subsegment information.  */

static unsigned get_last_insn_flags (segT, subsegT);
static void set_last_insn_flags (segT, subsegT, unsigned, bool);
static float get_subseg_total_freq (segT, subsegT);
static float get_subseg_target_freq (segT, subsegT);
static void set_subseg_freq (segT, subsegT, float, float);

/* Segment list functions.  */

static void xtensa_move_literals (void);
static void xtensa_reorder_segments (void);
static void xtensa_switch_to_literal_fragment (emit_state *);
static void xtensa_switch_to_non_abs_literal_fragment (emit_state *);
static void xtensa_switch_section_emit_state (emit_state *, segT, subsegT);
static void xtensa_restore_emit_state (emit_state *);
static segT cache_literal_section (bool);

/* op_placement_info functions.  */

static void init_op_placement_info_table (void);
extern bool opcode_fits_format_slot (xtensa_opcode, xtensa_format, int);
static int xg_get_single_size (xtensa_opcode);
static xtensa_format xg_get_single_format (xtensa_opcode);
static int xg_get_single_slot (xtensa_opcode);

/* TInsn and IStack functions.  */

static bool tinsn_has_symbolic_operands (const TInsn *);
static bool tinsn_has_invalid_symbolic_operands (const TInsn *);
static bool tinsn_has_complex_operands (const TInsn *);
static bool tinsn_to_insnbuf (TInsn *, xtensa_insnbuf);
static bool tinsn_check_arguments (const TInsn *);
static void tinsn_from_chars (TInsn *, char *, int);
static void tinsn_immed_from_frag (TInsn *, fragS *, int);
static int get_num_stack_text_bytes (IStack *);
static int get_num_stack_literal_bytes (IStack *);
static bool tinsn_to_slotbuf (xtensa_format, int, TInsn *, xtensa_insnbuf);

/* vliw_insn functions.  */

static void xg_init_vinsn (vliw_insn *);
static void xg_copy_vinsn (vliw_insn *, vliw_insn *);
static void xg_clear_vinsn (vliw_insn *);
static bool vinsn_has_specific_opcodes (vliw_insn *);
static void xg_free_vinsn (vliw_insn *);
static bool vinsn_to_insnbuf
  (vliw_insn *, char *, fragS *, bool);
static void vinsn_from_chars (vliw_insn *, char *);

/* Expression Utilities.  */

bool expr_is_const (const expressionS *);
offsetT get_expr_const (const expressionS *);
void set_expr_const (expressionS *, offsetT);
bool expr_is_register (const expressionS *);
offsetT get_expr_register (const expressionS *);
void set_expr_symbol_offset (expressionS *, symbolS *, offsetT);
bool expr_is_equal (expressionS *, expressionS *);
static void copy_expr (expressionS *, const expressionS *);

/* Section renaming.  */

static void build_section_rename (const char *);


/* ISA imported from bfd.  */
extern xtensa_isa xtensa_default_isa;

extern int target_big_endian;

static xtensa_opcode xtensa_addi_opcode;
static xtensa_opcode xtensa_addmi_opcode;
static xtensa_opcode xtensa_call0_opcode;
static xtensa_opcode xtensa_call4_opcode;
static xtensa_opcode xtensa_call8_opcode;
static xtensa_opcode xtensa_call12_opcode;
static xtensa_opcode xtensa_callx0_opcode;
static xtensa_opcode xtensa_callx4_opcode;
static xtensa_opcode xtensa_callx8_opcode;
static xtensa_opcode xtensa_callx12_opcode;
static xtensa_opcode xtensa_const16_opcode;
static xtensa_opcode xtensa_entry_opcode;
static xtensa_opcode xtensa_extui_opcode;
static xtensa_opcode xtensa_movi_opcode;
static xtensa_opcode xtensa_movi_n_opcode;
static xtensa_opcode xtensa_isync_opcode;
static xtensa_opcode xtensa_j_opcode;
static xtensa_opcode xtensa_jx_opcode;
static xtensa_opcode xtensa_l32r_opcode;
static xtensa_opcode xtensa_loop_opcode;
static xtensa_opcode xtensa_loopnez_opcode;
static xtensa_opcode xtensa_loopgtz_opcode;
static xtensa_opcode xtensa_nop_opcode;
static xtensa_opcode xtensa_nop_n_opcode;
static xtensa_opcode xtensa_or_opcode;
static xtensa_opcode xtensa_ret_opcode;
static xtensa_opcode xtensa_ret_n_opcode;
static xtensa_opcode xtensa_retw_opcode;
static xtensa_opcode xtensa_retw_n_opcode;
static xtensa_opcode xtensa_rsr_lcount_opcode;
static xtensa_opcode xtensa_waiti_opcode;
static int config_max_slots = 0;


/* Command-line Options.  */

bool use_literal_section = true;
enum flix_level produce_flix = FLIX_ALL;
static bool align_targets = true;
static bool warn_unaligned_branch_targets = false;
static bool has_a0_b_retw = false;
static bool workaround_a0_b_retw = false;
static bool workaround_b_j_loop_end = false;
static bool workaround_short_loop = false;
static bool maybe_has_short_loop = false;
static bool workaround_close_loop_end = false;
static bool maybe_has_close_loop_end = false;
static bool enforce_three_byte_loop_align = false;
static bool opt_linkrelax = true;

/* When workaround_short_loops is TRUE, all loops with early exits must
   have at least 3 instructions.  workaround_all_short_loops is a modifier
   to the workaround_short_loop flag.  In addition to the
   workaround_short_loop actions, all straightline loopgtz and loopnez
   must have at least 3 instructions.  */

static bool workaround_all_short_loops = false;

/* Generate individual property section for every section.
   This option is defined in BDF library.  */
extern bool elf32xtensa_separate_props;

/* Xtensa ABI.
   This option is defined in BDF library.  */
extern int elf32xtensa_abi;

static void
xtensa_setup_hw_workarounds (int earliest, int latest)
{
  if (earliest > latest)
    as_fatal (_("illegal range of target hardware versions"));

  /* Enable all workarounds for pre-T1050.0 hardware.  */
  if (earliest < 105000 || latest < 105000)
    {
      workaround_a0_b_retw |= true;
      workaround_b_j_loop_end |= true;
      workaround_short_loop |= true;
      workaround_close_loop_end |= true;
      workaround_all_short_loops |= true;
      enforce_three_byte_loop_align = true;
    }
}


enum
{
  option_density = OPTION_MD_BASE,
  option_no_density,

  option_flix,
  option_no_generate_flix,
  option_no_flix,

  option_relax,
  option_no_relax,

  option_link_relax,
  option_no_link_relax,

  option_generics,
  option_no_generics,

  option_transform,
  option_no_transform,

  option_text_section_literals,
  option_no_text_section_literals,

  option_absolute_literals,
  option_no_absolute_literals,

  option_align_targets,
  option_no_align_targets,

  option_warn_unaligned_targets,

  option_longcalls,
  option_no_longcalls,

  option_workaround_a0_b_retw,
  option_no_workaround_a0_b_retw,

  option_workaround_b_j_loop_end,
  option_no_workaround_b_j_loop_end,

  option_workaround_short_loop,
  option_no_workaround_short_loop,

  option_workaround_all_short_loops,
  option_no_workaround_all_short_loops,

  option_workaround_close_loop_end,
  option_no_workaround_close_loop_end,

  option_no_workarounds,

  option_rename_section_name,

  option_prefer_l32r,
  option_prefer_const16,

  option_target_hardware,

  option_trampolines,
  option_no_trampolines,

  option_auto_litpools,
  option_no_auto_litpools,
  option_auto_litpool_limit,

  option_separate_props,
  option_no_separate_props,

  option_abi_windowed,
  option_abi_call0,
};

const char *md_shortopts = "";

struct option md_longopts[] =
{
  { "density", no_argument, NULL, option_density },
  { "no-density", no_argument, NULL, option_no_density },

  { "flix", no_argument, NULL, option_flix },
  { "no-generate-flix", no_argument, NULL, option_no_generate_flix },
  { "no-allow-flix", no_argument, NULL, option_no_flix },

  /* Both "relax" and "generics" are deprecated and treated as equivalent
     to the "transform" option.  */
  { "relax", no_argument, NULL, option_relax },
  { "no-relax", no_argument, NULL, option_no_relax },
  { "generics", no_argument, NULL, option_generics },
  { "no-generics", no_argument, NULL, option_no_generics },

  { "transform", no_argument, NULL, option_transform },
  { "no-transform", no_argument, NULL, option_no_transform },
  { "text-section-literals", no_argument, NULL, option_text_section_literals },
  { "no-text-section-literals", no_argument, NULL,
    option_no_text_section_literals },
  { "absolute-literals", no_argument, NULL, option_absolute_literals },
  { "no-absolute-literals", no_argument, NULL, option_no_absolute_literals },
  /* This option was changed from -align-target to -target-align
     because it conflicted with the "-al" option.  */
  { "target-align", no_argument, NULL, option_align_targets },
  { "no-target-align", no_argument, NULL, option_no_align_targets },
  { "warn-unaligned-targets", no_argument, NULL,
    option_warn_unaligned_targets },
  { "longcalls", no_argument, NULL, option_longcalls },
  { "no-longcalls", no_argument, NULL, option_no_longcalls },

  { "no-workaround-a0-b-retw", no_argument, NULL,
    option_no_workaround_a0_b_retw },
  { "workaround-a0-b-retw", no_argument, NULL, option_workaround_a0_b_retw },

  { "no-workaround-b-j-loop-end", no_argument, NULL,
    option_no_workaround_b_j_loop_end },
  { "workaround-b-j-loop-end", no_argument, NULL,
    option_workaround_b_j_loop_end },

  { "no-workaround-short-loops", no_argument, NULL,
    option_no_workaround_short_loop },
  { "workaround-short-loops", no_argument, NULL,
    option_workaround_short_loop },

  { "no-workaround-all-short-loops", no_argument, NULL,
    option_no_workaround_all_short_loops },
  { "workaround-all-short-loop", no_argument, NULL,
    option_workaround_all_short_loops },

  { "prefer-l32r", no_argument, NULL, option_prefer_l32r },
  { "prefer-const16", no_argument, NULL, option_prefer_const16 },

  { "no-workarounds", no_argument, NULL, option_no_workarounds },

  { "no-workaround-close-loop-end", no_argument, NULL,
    option_no_workaround_close_loop_end },
  { "workaround-close-loop-end", no_argument, NULL,
    option_workaround_close_loop_end },

  { "rename-section", required_argument, NULL, option_rename_section_name },

  { "link-relax", no_argument, NULL, option_link_relax },
  { "no-link-relax", no_argument, NULL, option_no_link_relax },

  { "target-hardware", required_argument, NULL, option_target_hardware },

  { "trampolines", no_argument, NULL, option_trampolines },
  { "no-trampolines", no_argument, NULL, option_no_trampolines },

  { "auto-litpools", no_argument, NULL, option_auto_litpools },
  { "no-auto-litpools", no_argument, NULL, option_no_auto_litpools },
  { "auto-litpool-limit", required_argument, NULL, option_auto_litpool_limit },

  { "separate-prop-tables", no_argument, NULL, option_separate_props },

  { "abi-windowed", no_argument, NULL, option_abi_windowed },
  { "abi-call0", no_argument, NULL, option_abi_call0 },

  { NULL, no_argument, NULL, 0 }
};

size_t md_longopts_size = sizeof md_longopts;


int
md_parse_option (int c, const char *arg)
{
  switch (c)
    {
    case option_density:
      as_warn (_("--density option is ignored"));
      return 1;
    case option_no_density:
      as_warn (_("--no-density option is ignored"));
      return 1;
    case option_link_relax:
      opt_linkrelax = true;
      return 1;
    case option_no_link_relax:
      opt_linkrelax = false;
      return 1;
    case option_flix:
      produce_flix = FLIX_ALL;
      return 1;
    case option_no_generate_flix:
      produce_flix = FLIX_NO_GENERATE;
      return 1;
    case option_no_flix:
      produce_flix = FLIX_NONE;
      return 1;
    case option_generics:
      as_warn (_("--generics is deprecated; use --transform instead"));
      return md_parse_option (option_transform, arg);
    case option_no_generics:
      as_warn (_("--no-generics is deprecated; use --no-transform instead"));
      return md_parse_option (option_no_transform, arg);
    case option_relax:
      as_warn (_("--relax is deprecated; use --transform instead"));
      return md_parse_option (option_transform, arg);
    case option_no_relax:
      as_warn (_("--no-relax is deprecated; use --no-transform instead"));
      return md_parse_option (option_no_transform, arg);
    case option_longcalls:
      directive_state[directive_longcalls] = true;
      return 1;
    case option_no_longcalls:
      directive_state[directive_longcalls] = false;
      return 1;
    case option_text_section_literals:
      use_literal_section = false;
      return 1;
    case option_no_text_section_literals:
      use_literal_section = true;
      return 1;
    case option_absolute_literals:
      if (!absolute_literals_supported)
	{
	  as_fatal (_("--absolute-literals option not supported in this Xtensa configuration"));
	  return 0;
	}
      directive_state[directive_absolute_literals] = true;
      return 1;
    case option_no_absolute_literals:
      directive_state[directive_absolute_literals] = false;
      return 1;

    case option_workaround_a0_b_retw:
      workaround_a0_b_retw = true;
      return 1;
    case option_no_workaround_a0_b_retw:
      workaround_a0_b_retw = false;
      return 1;
    case option_workaround_b_j_loop_end:
      workaround_b_j_loop_end = true;
      return 1;
    case option_no_workaround_b_j_loop_end:
      workaround_b_j_loop_end = false;
      return 1;

    case option_workaround_short_loop:
      workaround_short_loop = true;
      return 1;
    case option_no_workaround_short_loop:
      workaround_short_loop = false;
      return 1;

    case option_workaround_all_short_loops:
      workaround_all_short_loops = true;
      return 1;
    case option_no_workaround_all_short_loops:
      workaround_all_short_loops = false;
      return 1;

    case option_workaround_close_loop_end:
      workaround_close_loop_end = true;
      return 1;
    case option_no_workaround_close_loop_end:
      workaround_close_loop_end = false;
      return 1;

    case option_no_workarounds:
      workaround_a0_b_retw = false;
      workaround_b_j_loop_end = false;
      workaround_short_loop = false;
      workaround_all_short_loops = false;
      workaround_close_loop_end = false;
      return 1;

    case option_align_targets:
      align_targets = true;
      return 1;
    case option_no_align_targets:
      align_targets = false;
      return 1;

    case option_warn_unaligned_targets:
      warn_unaligned_branch_targets = true;
      return 1;

    case option_rename_section_name:
      build_section_rename (arg);
      return 1;

    case 'Q':
      /* -Qy, -Qn: SVR4 arguments controlling whether a .comment section
         should be emitted or not.  FIXME: Not implemented.  */
      return 1;

    case option_prefer_l32r:
      if (prefer_const16)
	as_fatal (_("prefer-l32r conflicts with prefer-const16"));
      prefer_l32r = 1;
      return 1;

    case option_prefer_const16:
      if (prefer_l32r)
	as_fatal (_("prefer-const16 conflicts with prefer-l32r"));
      prefer_const16 = 1;
      return 1;

    case option_target_hardware:
      {
	int earliest, latest = 0;
	char *end;
	if (*arg == 0 || *arg == '-')
	  as_fatal (_("invalid target hardware version"));

	earliest = strtol (arg, &end, 0);

	if (*end == 0)
	  latest = earliest;
	else if (*end == '-')
	  {
	    if (*++end == 0)
	      as_fatal (_("invalid target hardware version"));
	    latest = strtol (end, &end, 0);
	  }
	if (*end != 0)
	  as_fatal (_("invalid target hardware version"));

	xtensa_setup_hw_workarounds (earliest, latest);
	return 1;
      }

    case option_transform:
      /* This option has no affect other than to use the defaults,
	 which are already set.  */
      return 1;

    case option_no_transform:
      /* This option turns off all transformations of any kind.
	 However, because we want to preserve the state of other
	 directives, we only change its own field.  Thus, before
	 you perform any transformation, always check if transform
	 is available.  If you use the functions we provide for this
	 purpose, you will be ok.  */
      directive_state[directive_transform] = false;
      return 1;

    case option_trampolines:
      use_trampolines = true;
      return 1;

    case option_no_trampolines:
      use_trampolines = false;
      return 1;

    case option_auto_litpools:
      auto_litpools = true;
      use_literal_section = false;
      if (auto_litpool_limit <= 0)
	auto_litpool_limit = MAX_AUTO_POOL_LITERALS / 2;
      return 1;

    case option_no_auto_litpools:
      auto_litpools = false;
      auto_litpool_limit = -1;
      return 1;

    case option_auto_litpool_limit:
      {
	int value = 0;
	char *end;
	if (auto_litpool_limit < 0)
	  as_fatal (_("no-auto-litpools is incompatible with auto-litpool-limit"));
	if (*arg == 0 || *arg == '-')
	  as_fatal (_("invalid auto-litpool-limit argument"));
	value = strtol (arg, &end, 10);
	if (*end != 0)
	  as_fatal (_("invalid auto-litpool-limit argument"));
	if (value < 100 || value > 10000)
	  as_fatal (_("invalid auto-litpool-limit argument (range is 100-10000)"));
	auto_litpool_limit = value;
	auto_litpools = true;
	use_literal_section = false;
	return 1;
      }

    case option_separate_props:
      elf32xtensa_separate_props = true;
      return 1;

    case option_no_separate_props:
      elf32xtensa_separate_props = false;
      return 1;

    case option_abi_windowed:
      elf32xtensa_abi = XTHAL_ABI_WINDOWED;
      return 1;

    case option_abi_call0:
      elf32xtensa_abi = XTHAL_ABI_CALL0;
      return 1;

    default:
      return 0;
    }
}


void
md_show_usage (FILE *stream)
{
  fputs ("\n\
Xtensa options:\n\
  --[no-]text-section-literals\n\
                          [Do not] put literals in the text section\n\
  --[no-]absolute-literals\n\
                          [Do not] default to use non-PC-relative literals\n\
  --[no-]target-align     [Do not] try to align branch targets\n\
  --[no-]longcalls        [Do not] emit 32-bit call sequences\n\
  --[no-]transform        [Do not] transform instructions\n\
  --flix                  both allow hand-written and generate flix bundles\n\
  --no-generate-flix      allow hand-written but do not generate\n\
                          flix bundles\n\
  --no-allow-flix         neither allow hand-written nor generate\n\
                          flix bundles\n\
  --rename-section old=new Rename section 'old' to 'new'\n\
  --[no-]trampolines      [Do not] generate trampolines (jumps to jumps)\n\
                          when jumps do not reach their targets\n\
  --[no-]auto-litpools    [Do not] automatically create literal pools\n\
  --auto-litpool-limit=<value>\n\
                          (range 100-10000) Maximum number of blocks of\n\
                          instructions to emit between literal pool\n\
                          locations; implies --auto-litpools flag\n\
  --[no-]separate-prop-tables\n\
                          [Do not] place Xtensa property records into\n\
                          individual property sections for each section.\n\
                          Default is to generate single property section.\n", stream);
}


/* Functions related to the list of current label symbols.  */

static void
xtensa_add_insn_label (symbolS *sym)
{
  sym_list *l;

  if (!free_insn_labels)
    l = XNEW (sym_list);
  else
    {
      l = free_insn_labels;
      free_insn_labels = l->next;
    }

  l->sym = sym;
  l->next = insn_labels;
  insn_labels = l;
}


static void
xtensa_clear_insn_labels (void)
{
  sym_list **pl;

  for (pl = &free_insn_labels; *pl != NULL; pl = &(*pl)->next)
    ;
  *pl = insn_labels;
  insn_labels = NULL;
}


static void
xtensa_move_labels (fragS *new_frag, valueT new_offset)
{
  sym_list *lit;

  for (lit = insn_labels; lit; lit = lit->next)
    {
      symbolS *lit_sym = lit->sym;
      S_SET_VALUE (lit_sym, new_offset);
      symbol_set_frag (lit_sym, new_frag);
    }
}


/* Directive data and functions.  */

typedef struct state_stackS_struct
{
  directiveE directive;
  bool negated;
  bool old_state;
  const char *file;
  unsigned int line;
  const void *datum;
  struct state_stackS_struct *prev;
} state_stackS;

state_stackS *directive_state_stack;

const pseudo_typeS md_pseudo_table[] =
{
  { "align", s_align_bytes, 0 }, /* Defaulting is invalid (0).  */
  { "literal_position", xtensa_literal_position, 0 },
  { "frame", s_ignore, 0 },	/* Formerly used for STABS debugging.  */
  { "long", xtensa_elf_cons, 4 },
  { "word", xtensa_elf_cons, 4 },
  { "4byte", xtensa_elf_cons, 4 },
  { "short", xtensa_elf_cons, 2 },
  { "2byte", xtensa_elf_cons, 2 },
  { "sleb128", xtensa_leb128, 1},
  { "uleb128", xtensa_leb128, 0},
  { "begin", xtensa_begin_directive, 0 },
  { "end", xtensa_end_directive, 0 },
  { "literal", xtensa_literal_pseudo, 0 },
  { "frequency", xtensa_frequency_pseudo, 0 },
  { NULL, 0, 0 },
};


static bool
use_transform (void)
{
  /* After md_finish, you should be checking frag by frag, rather
     than state directives.  */
  gas_assert (!past_xtensa_md_finish);
  return directive_state[directive_transform];
}


static bool
do_align_targets (void)
{
  /* Do not use this function after md_finish; just look at align_targets
     instead.  There is no target-align directive, so alignment is either
     enabled for all frags or not done at all.  */
  gas_assert (!past_xtensa_md_finish);
  return align_targets && use_transform ();
}


static void
directive_push (directiveE directive, bool negated, const void *datum)
{
  const char *file;
  unsigned int line;
  state_stackS *stack = XNEW (state_stackS);

  file = as_where (&line);

  stack->directive = directive;
  stack->negated = negated;
  stack->old_state = directive_state[directive];
  stack->file = file;
  stack->line = line;
  stack->datum = datum;
  stack->prev = directive_state_stack;
  directive_state_stack = stack;

  directive_state[directive] = !negated;
}


static void
directive_pop (directiveE *directive,
	       bool *negated,
	       const char **file,
	       unsigned int *line,
	       const void **datum)
{
  state_stackS *top = directive_state_stack;

  if (!directive_state_stack)
    {
      as_bad (_("unmatched .end directive"));
      *directive = directive_none;
      return;
    }

  directive_state[directive_state_stack->directive] = top->old_state;
  *directive = top->directive;
  *negated = top->negated;
  *file = top->file;
  *line = top->line;
  *datum = top->datum;
  directive_state_stack = top->prev;
  free (top);
}


static void
directive_balance (void)
{
  while (directive_state_stack)
    {
      directiveE directive;
      bool negated;
      const char *file;
      unsigned int line;
      const void *datum;

      directive_pop (&directive, &negated, &file, &line, &datum);
      as_warn_where ((char *) file, line,
		     _(".begin directive with no matching .end directive"));
    }
}


static bool
inside_directive (directiveE dir)
{
  state_stackS *top = directive_state_stack;

  while (top && top->directive != dir)
    top = top->prev;

  return (top != NULL);
}


static void
get_directive (directiveE *directive, bool *negated)
{
  int len;
  unsigned i;
  const char *directive_string;

  if (!startswith (input_line_pointer, "no-"))
    *negated = false;
  else
    {
      *negated = true;
      input_line_pointer += 3;
    }

  len = strspn (input_line_pointer,
		"abcdefghijklmnopqrstuvwxyz_-/0123456789.");

  /* This code is a hack to make .begin [no-][generics|relax] exactly
     equivalent to .begin [no-]transform.  We should remove it when
     we stop accepting those options.  */

  if (startswith (input_line_pointer, "generics"))
    {
      as_warn (_("[no-]generics is deprecated; use [no-]transform instead"));
      directive_string = "transform";
    }
  else if (startswith (input_line_pointer, "relax"))
    {
      as_warn (_("[no-]relax is deprecated; use [no-]transform instead"));
      directive_string = "transform";
    }
  else
    directive_string = input_line_pointer;

  for (i = 0; i < sizeof (directive_info) / sizeof (*directive_info); ++i)
    {
      if (strncmp (directive_string, directive_info[i].name, len) == 0)
	{
	  input_line_pointer += len;
	  *directive = (directiveE) i;
	  if (*negated && !directive_info[i].can_be_negated)
	    as_bad (_("directive %s cannot be negated"),
		    directive_info[i].name);
	  return;
	}
    }

  as_bad (_("unknown directive"));
  *directive = (directiveE) XTENSA_UNDEFINED;
}


static void
xtensa_begin_directive (int ignore ATTRIBUTE_UNUSED)
{
  directiveE directive;
  bool negated;
  emit_state *state;
  lit_state *ls;

  get_directive (&directive, &negated);
  if (directive == (directiveE) XTENSA_UNDEFINED)
    {
      discard_rest_of_line ();
      return;
    }

  if (cur_vinsn.inside_bundle)
    as_bad (_("directives are not valid inside bundles"));

  switch (directive)
    {
    case directive_literal:
      if (!inside_directive (directive_literal))
	{
	  /* Previous labels go with whatever follows this directive, not with
	     the literal, so save them now.  */
	  saved_insn_labels = insn_labels;
	  insn_labels = NULL;
	}
      as_warn (_(".begin literal is deprecated; use .literal instead"));
      state = XNEW (emit_state);
      xtensa_switch_to_literal_fragment (state);
      directive_push (directive_literal, negated, state);
      break;

    case directive_literal_prefix:
      /* Have to flush pending output because a movi relaxed to an l32r
	 might produce a literal.  */
      md_flush_pending_output ();
      /* Check to see if the current fragment is a literal
	 fragment.  If it is, then this operation is not allowed.  */
      if (generating_literals)
	{
	  as_bad (_("cannot set literal_prefix inside literal fragment"));
	  return;
	}

      /* Allocate the literal state for this section and push
	 onto the directive stack.  */
      ls = XNEW (lit_state);
      gas_assert (ls);

      *ls = default_lit_sections;
      directive_push (directive_literal_prefix, negated, ls);

      /* Process the new prefix.  */
      xtensa_literal_prefix ();
      break;

    case directive_freeregs:
      /* This information is currently unused, but we'll accept the statement
         and just discard the rest of the line.  This won't check the syntax,
         but it will accept every correct freeregs directive.  */
      input_line_pointer += strcspn (input_line_pointer, "\n");
      directive_push (directive_freeregs, negated, 0);
      break;

    case directive_schedule:
      md_flush_pending_output ();
      frag_var (rs_fill, 0, 0, frag_now->fr_subtype,
		frag_now->fr_symbol, frag_now->fr_offset, NULL);
      directive_push (directive_schedule, negated, 0);
      xtensa_set_frag_assembly_state (frag_now);
      break;

    case directive_density:
      as_warn (_(".begin [no-]density is ignored"));
      break;

    case directive_absolute_literals:
      md_flush_pending_output ();
      if (!absolute_literals_supported && !negated)
	{
	  as_warn (_("Xtensa absolute literals option not supported; ignored"));
	  break;
	}
      xtensa_set_frag_assembly_state (frag_now);
      directive_push (directive, negated, 0);
      break;

    default:
      md_flush_pending_output ();
      xtensa_set_frag_assembly_state (frag_now);
      directive_push (directive, negated, 0);
      break;
    }

  demand_empty_rest_of_line ();
}


static void
xtensa_end_directive (int ignore ATTRIBUTE_UNUSED)
{
  directiveE begin_directive, end_directive;
  bool begin_negated, end_negated;
  const char *file;
  unsigned int line;
  emit_state *state;
  emit_state **state_ptr;
  lit_state *s;

  if (cur_vinsn.inside_bundle)
    as_bad (_("directives are not valid inside bundles"));

  get_directive (&end_directive, &end_negated);

  md_flush_pending_output ();

  switch ((int) end_directive)
    {
    case XTENSA_UNDEFINED:
      discard_rest_of_line ();
      return;

    case (int) directive_density:
      as_warn (_(".end [no-]density is ignored"));
      demand_empty_rest_of_line ();
      break;

    case (int) directive_absolute_literals:
      if (!absolute_literals_supported && !end_negated)
	{
	  as_warn (_("Xtensa absolute literals option not supported; ignored"));
	  demand_empty_rest_of_line ();
	  return;
	}
      break;

    default:
      break;
    }

  state_ptr = &state; /* use state_ptr to avoid type-punning warning */
  directive_pop (&begin_directive, &begin_negated, &file, &line,
		 (const void **) state_ptr);

  if (begin_directive != directive_none)
    {
      if (begin_directive != end_directive || begin_negated != end_negated)
	{
	  as_bad (_("does not match begin %s%s at %s:%d"),
		  begin_negated ? "no-" : "",
		  directive_info[begin_directive].name, file, line);
	}
      else
	{
	  switch (end_directive)
	    {
	    case directive_literal:
	      frag_var (rs_fill, 0, 0, 0, NULL, 0, NULL);
	      xtensa_restore_emit_state (state);
	      xtensa_set_frag_assembly_state (frag_now);
	      free (state);
	      if (!inside_directive (directive_literal))
		{
		  /* Restore the list of current labels.  */
		  xtensa_clear_insn_labels ();
		  insn_labels = saved_insn_labels;
		}
	      break;

	    case directive_literal_prefix:
	      /* Restore the default collection sections from saved state.  */
	      s = (lit_state *) state;
	      gas_assert (s);
	      default_lit_sections = *s;

	      /* Free the state storage.  */
	      free (s->lit_prefix);
	      free (s);
	      break;

	    case directive_schedule:
	    case directive_freeregs:
	      break;

	    default:
	      xtensa_set_frag_assembly_state (frag_now);
	      break;
	    }
	}
    }

  demand_empty_rest_of_line ();
}


/* Place an aligned literal fragment at the current location.  */

static void
xtensa_literal_position (int ignore ATTRIBUTE_UNUSED)
{
  md_flush_pending_output ();

  if (inside_directive (directive_literal))
    as_warn (_(".literal_position inside literal directive; ignoring"));
  xtensa_mark_literal_pool_location ();

  demand_empty_rest_of_line ();
  xtensa_clear_insn_labels ();
}


/* Support .literal label, expr, ...  */

static void
xtensa_literal_pseudo (int ignored ATTRIBUTE_UNUSED)
{
  emit_state state;
  char *p, *base_name;
  char c;

  if (inside_directive (directive_literal))
    {
      as_bad (_(".literal not allowed inside .begin literal region"));
      ignore_rest_of_line ();
      return;
    }

  md_flush_pending_output ();

  /* Previous labels go with whatever follows this directive, not with
     the literal, so save them now.  */
  saved_insn_labels = insn_labels;
  insn_labels = NULL;

  base_name = input_line_pointer;

  xtensa_switch_to_literal_fragment (&state);

  /* All literals are aligned to four-byte boundaries.  */
  frag_align (2, 0, 0);
  record_alignment (now_seg, 2);

  c = get_symbol_name (&base_name);
  /* Just after name is now '\0'.  */
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE_AFTER_NAME ();

  if (*input_line_pointer != ',' && *input_line_pointer != ':')
    {
      as_bad (_("expected comma or colon after symbol name; "
		"rest of line ignored"));
      ignore_rest_of_line ();
      xtensa_restore_emit_state (&state);
      return;
    }

  *p = 0;
  colon (base_name);
  *p = c;

  input_line_pointer++;		/* skip ',' or ':' */

  xtensa_elf_cons (4);

  xtensa_restore_emit_state (&state);

  /* Restore the list of current labels.  */
  xtensa_clear_insn_labels ();
  insn_labels = saved_insn_labels;
}


static void
xtensa_literal_prefix (void)
{
  char *name;
  int len;

  /* Parse the new prefix from the input_line_pointer.  */
  SKIP_WHITESPACE ();
  len = strspn (input_line_pointer,
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz_/0123456789.$");

  /* Get a null-terminated copy of the name.  */
  name = xmemdup0 (input_line_pointer, len);

  /* Skip the name in the input line.  */
  input_line_pointer += len;

  default_lit_sections.lit_prefix = name;

  /* Clear cached literal sections, since the prefix has changed.  */
  default_lit_sections.lit_seg = NULL;
  default_lit_sections.lit4_seg = NULL;
}


/* Support ".frequency branch_target_frequency fall_through_frequency".  */

static void
xtensa_frequency_pseudo (int ignored ATTRIBUTE_UNUSED)
{
  float fall_through_f, target_f;

  fall_through_f = (float) strtod (input_line_pointer, &input_line_pointer);
  if (fall_through_f < 0)
    {
      as_bad (_("fall through frequency must be greater than 0"));
      ignore_rest_of_line ();
      return;
    }

  target_f = (float) strtod (input_line_pointer, &input_line_pointer);
  if (target_f < 0)
    {
      as_bad (_("branch target frequency must be greater than 0"));
      ignore_rest_of_line ();
      return;
    }

  set_subseg_freq (now_seg, now_subseg, target_f + fall_through_f, target_f);

  demand_empty_rest_of_line ();
}


/* Like normal .long/.short/.word, except support @plt, etc.
   Clobbers input_line_pointer, checks end-of-line.  */

static void
xtensa_elf_cons (int nbytes)
{
  expressionS exp;
  bfd_reloc_code_real_type reloc;

  md_flush_pending_output ();

  if (cur_vinsn.inside_bundle)
    as_bad (_("directives are not valid inside bundles"));

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  do
    {
      expression (&exp);
      if (exp.X_op == O_symbol
	  && *input_line_pointer == '@'
	  && ((reloc = xtensa_elf_suffix (&input_line_pointer, &exp))
	      != BFD_RELOC_NONE))
	{
	  reloc_howto_type *reloc_howto =
	    bfd_reloc_type_lookup (stdoutput, reloc);

	  if (reloc == BFD_RELOC_UNUSED || !reloc_howto)
	    as_bad (_("unsupported relocation"));
	  else if ((reloc >= BFD_RELOC_XTENSA_SLOT0_OP
		    && reloc <= BFD_RELOC_XTENSA_SLOT14_OP)
		   || (reloc >= BFD_RELOC_XTENSA_SLOT0_ALT
		       && reloc <= BFD_RELOC_XTENSA_SLOT14_ALT))
	    as_bad (_("opcode-specific %s relocation used outside "
		      "an instruction"), reloc_howto->name);
	  else if (nbytes != (int) bfd_get_reloc_size (reloc_howto))
	    as_bad (ngettext ("%s relocations do not fit in %d byte",
			      "%s relocations do not fit in %d bytes",
			      nbytes),
		    reloc_howto->name, nbytes);
	  else if (reloc == BFD_RELOC_XTENSA_TLS_FUNC
		   || reloc == BFD_RELOC_XTENSA_TLS_ARG
		   || reloc == BFD_RELOC_XTENSA_TLS_CALL)
	    as_bad (_("invalid use of %s relocation"), reloc_howto->name);
	  else
	    {
	      char *p = frag_more ((int) nbytes);
	      xtensa_set_frag_assembly_state (frag_now);
	      fix_new_exp (frag_now, p - frag_now->fr_literal,
			   nbytes, &exp, reloc_howto->pc_relative, reloc);
	    }
	}
      else
	{
	  xtensa_set_frag_assembly_state (frag_now);
	  emit_expr (&exp, (unsigned int) nbytes);
	}
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;		/* Put terminator back into stream.  */
  demand_empty_rest_of_line ();
}

static bool is_leb128_expr;

static void
xtensa_leb128 (int sign)
{
  is_leb128_expr = true;
  s_leb128 (sign);
  is_leb128_expr = false;
}


/* Parsing and Idiom Translation.  */

/* Parse @plt, etc. and return the desired relocation.  */
static bfd_reloc_code_real_type
xtensa_elf_suffix (char **str_p, expressionS *exp_p)
{
  char ident[20];
  char *str = *str_p;
  char *str2;
  int ch;
  int len;
  unsigned int i;

  if (*str++ != '@')
    return BFD_RELOC_NONE;

  for (ch = *str, str2 = ident;
       (str2 < ident + sizeof (ident) - 1
	&& (ISALNUM (ch) || ch == '@'));
       ch = *++str)
    {
      *str2++ = (ISLOWER (ch)) ? ch : TOLOWER (ch);
    }

  *str2 = '\0';
  len = str2 - ident;

  ch = ident[0];
  for (i = 0; i < ARRAY_SIZE (suffix_relocs); i++)
    if (ch == suffix_relocs[i].suffix[0]
	&& len == suffix_relocs[i].length
	&& memcmp (ident, suffix_relocs[i].suffix, suffix_relocs[i].length) == 0)
      {
	/* Now check for "identifier@suffix+constant".  */
	if (*str == '-' || *str == '+')
	  {
	    char *orig_line = input_line_pointer;
	    expressionS new_exp;

	    input_line_pointer = str;
	    expression (&new_exp);
	    if (new_exp.X_op == O_constant)
	      {
		exp_p->X_add_number += new_exp.X_add_number;
		str = input_line_pointer;
	      }

	    if (&input_line_pointer != str_p)
	      input_line_pointer = orig_line;
	  }

	*str_p = str;
	return suffix_relocs[i].reloc;
      }

  return BFD_RELOC_UNUSED;
}


/* Find the matching operator type.  */
static operatorT
map_suffix_reloc_to_operator (bfd_reloc_code_real_type reloc)
{
  operatorT operator = O_illegal;
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (suffix_relocs); i++)
    {
      if (suffix_relocs[i].reloc == reloc)
	{
	  operator = suffix_relocs[i].operator;
	  break;
	}
    }
  gas_assert (operator != O_illegal);
  return operator;
}


/* Find the matching reloc type.  */
static bfd_reloc_code_real_type
map_operator_to_reloc (unsigned char operator, bool is_literal)
{
  unsigned int i;
  bfd_reloc_code_real_type reloc = BFD_RELOC_UNUSED;

  for (i = 0; i < ARRAY_SIZE (suffix_relocs); i++)
    {
      if (suffix_relocs[i].operator == operator)
	{
	  reloc = suffix_relocs[i].reloc;
	  break;
	}
    }

  if (is_literal)
    {
      if (reloc == BFD_RELOC_XTENSA_TLS_FUNC)
	return BFD_RELOC_XTENSA_TLSDESC_FN;
      else if (reloc == BFD_RELOC_XTENSA_TLS_ARG)
	return BFD_RELOC_XTENSA_TLSDESC_ARG;
    }

  if (reloc == BFD_RELOC_UNUSED)
    return BFD_RELOC_32;

  return reloc;
}


static const char *
expression_end (const char *name)
{
  while (1)
    {
      switch (*name)
	{
	case '}':
	case ';':
	case '\0':
	case ',':
	case ':':
	  return name;
	case ' ':
	case '\t':
	  ++name;
	  continue;
	default:
	  return 0;
	}
    }
}


#define ERROR_REG_NUM ((unsigned) -1)

static unsigned
tc_get_register (const char *prefix)
{
  unsigned reg;
  const char *next_expr;
  const char *old_line_pointer;

  SKIP_WHITESPACE ();
  old_line_pointer = input_line_pointer;

  if (*input_line_pointer == '$')
    ++input_line_pointer;

  /* Accept "sp" as a synonym for "a1".  */
  if (input_line_pointer[0] == 's' && input_line_pointer[1] == 'p'
      && expression_end (input_line_pointer + 2))
    {
      input_line_pointer += 2;
      return 1;  /* AR[1] */
    }

  while (*input_line_pointer++ == *prefix++)
    ;
  --input_line_pointer;
  --prefix;

  if (*prefix)
    {
      as_bad (_("bad register name: %s"), old_line_pointer);
      return ERROR_REG_NUM;
    }

  if (!ISDIGIT ((unsigned char) *input_line_pointer))
    {
      as_bad (_("bad register number: %s"), input_line_pointer);
      return ERROR_REG_NUM;
    }

  reg = 0;

  while (ISDIGIT ((int) *input_line_pointer))
    reg = reg * 10 + *input_line_pointer++ - '0';

  if (!(next_expr = expression_end (input_line_pointer)))
    {
      as_bad (_("bad register name: %s"), old_line_pointer);
      return ERROR_REG_NUM;
    }

  input_line_pointer = (char *) next_expr;

  return reg;
}


static void
expression_maybe_register (xtensa_opcode opc, int opnd, expressionS *tok)
{
  xtensa_isa isa = xtensa_default_isa;

  /* Check if this is an immediate operand.  */
  if (xtensa_operand_is_register (isa, opc, opnd) == 0)
    {
      bfd_reloc_code_real_type reloc;
      segT t = expression (tok);

      if (t == absolute_section
	  && xtensa_operand_is_PCrelative (isa, opc, opnd) == 1)
	{
	  gas_assert (tok->X_op == O_constant);
	  tok->X_op = O_symbol;
	  tok->X_add_symbol = &abs_symbol;
	}

      if ((tok->X_op == O_constant || tok->X_op == O_symbol)
	  && ((reloc = xtensa_elf_suffix (&input_line_pointer, tok))
	      != BFD_RELOC_NONE))
	{
	  switch (reloc)
	    {
	    case BFD_RELOC_LO16:
	      if (tok->X_op == O_constant)
		{
		  tok->X_add_number &= 0xffff;
		  return;
		}
	      break;
	    case BFD_RELOC_HI16:
	      if (tok->X_op == O_constant)
		{
		  tok->X_add_number = ((unsigned) tok->X_add_number) >> 16;
		  return;
		}
	      break;
	    case BFD_RELOC_UNUSED:
	      as_bad (_("unsupported relocation"));
	      return;
	    case BFD_RELOC_32_PCREL:
	      as_bad (_("pcrel relocation not allowed in an instruction"));
	      return;
	    default:
	      break;
	    }
	  tok->X_op = map_suffix_reloc_to_operator (reloc);
	}
    }
  else
    {
      xtensa_regfile opnd_rf = xtensa_operand_regfile (isa, opc, opnd);
      unsigned reg = tc_get_register (xtensa_regfile_shortname (isa, opnd_rf));

      if (reg != ERROR_REG_NUM)	/* Already errored */
	{
	  uint32 buf = reg;
	  if (xtensa_operand_encode (isa, opc, opnd, &buf))
	    as_bad (_("register number out of range"));
	}

      tok->X_op = O_register;
      tok->X_add_symbol = 0;
      tok->X_add_number = reg;
    }
}


/* Split up the arguments for an opcode or pseudo-op.  */

static int
tokenize_arguments (char **args, char *str)
{
  char *old_input_line_pointer;
  bool saw_comma = false;
  bool saw_arg = false;
  bool saw_colon = false;
  int num_args = 0;
  char *arg_end, *arg;
  int arg_len;

  /* Save and restore input_line_pointer around this function.  */
  old_input_line_pointer = input_line_pointer;
  input_line_pointer = str;

  while (*input_line_pointer)
    {
      SKIP_WHITESPACE ();
      switch (*input_line_pointer)
	{
	case '\0':
	case '}':
	  goto fini;

	case ':':
	  input_line_pointer++;
	  if (saw_comma || saw_colon || !saw_arg)
	    goto err;
	  saw_colon = true;
	  break;

	case ',':
	  input_line_pointer++;
	  if (saw_comma || saw_colon || !saw_arg)
	    goto err;
	  saw_comma = true;
	  break;

	default:
	  if (!saw_comma && !saw_colon && saw_arg)
	    goto err;

	  arg_end = input_line_pointer + 1;
	  while (!expression_end (arg_end))
	    arg_end += 1;

	  arg_len = arg_end - input_line_pointer;
	  arg = XNEWVEC (char, (saw_colon ? 1 : 0) + arg_len + 1);
	  args[num_args] = arg;

	  if (saw_colon)
	    *arg++ = ':';
	  strncpy (arg, input_line_pointer, arg_len);
	  arg[arg_len] = '\0';

	  input_line_pointer = arg_end;
	  num_args += 1;
	  saw_comma = false;
	  saw_colon = false;
	  saw_arg = true;
	  break;
	}
    }

 fini:
  if (saw_comma || saw_colon)
    goto err;
  input_line_pointer = old_input_line_pointer;
  return num_args;

 err:
  if (saw_comma)
    as_bad (_("extra comma"));
  else if (saw_colon)
    as_bad (_("extra colon"));
  else if (!saw_arg)
    as_bad (_("missing argument"));
  else
    as_bad (_("missing comma or colon"));
  input_line_pointer = old_input_line_pointer;
  return -1;
}


/* Parse the arguments to an opcode.  Return TRUE on error.  */

static bool
parse_arguments (TInsn *insn, int num_args, char **arg_strings)
{
  expressionS *tok, *last_tok;
  xtensa_opcode opcode = insn->opcode;
  bool had_error = true;
  xtensa_isa isa = xtensa_default_isa;
  int n, num_regs = 0;
  int opcode_operand_count;
  int opnd_cnt, last_opnd_cnt;
  unsigned int next_reg = 0;
  char *old_input_line_pointer;

  if (insn->insn_type == ITYPE_LITERAL)
    opcode_operand_count = 1;
  else
    opcode_operand_count = xtensa_opcode_num_operands (isa, opcode);

  tok = insn->tok;
  memset (tok, 0, sizeof (*tok) * MAX_INSN_ARGS);

  /* Save and restore input_line_pointer around this function.  */
  old_input_line_pointer = input_line_pointer;

  last_tok = 0;
  last_opnd_cnt = -1;
  opnd_cnt = 0;

  /* Skip invisible operands.  */
  while (xtensa_operand_is_visible (isa, opcode, opnd_cnt) == 0)
    {
      opnd_cnt += 1;
      tok++;
    }

  for (n = 0; n < num_args; n++)
    {
      input_line_pointer = arg_strings[n];
      if (*input_line_pointer == ':')
	{
	  xtensa_regfile opnd_rf;
	  input_line_pointer++;
	  if (num_regs == 0)
	    goto err;
	  gas_assert (opnd_cnt > 0);
	  num_regs--;
	  opnd_rf = xtensa_operand_regfile (isa, opcode, last_opnd_cnt);
	  if (next_reg
	      != tc_get_register (xtensa_regfile_shortname (isa, opnd_rf)))
	    as_warn (_("incorrect register number, ignoring"));
	  next_reg++;
	}
      else
	{
	  if (opnd_cnt >= opcode_operand_count)
	    {
	      as_warn (_("too many arguments"));
	      goto err;
	    }
	  gas_assert (opnd_cnt < MAX_INSN_ARGS);

	  expression_maybe_register (opcode, opnd_cnt, tok);
	  next_reg = tok->X_add_number + 1;

	  if (tok->X_op == O_illegal || tok->X_op == O_absent)
	    goto err;
	  if (xtensa_operand_is_register (isa, opcode, opnd_cnt) == 1)
	    {
	      num_regs = xtensa_operand_num_regs (isa, opcode, opnd_cnt) - 1;
	      /* minus 1 because we are seeing one right now */
	    }
	  else
	    num_regs = 0;

	  last_tok = tok;
	  last_opnd_cnt = opnd_cnt;
	  demand_empty_rest_of_line ();

	  do
	    {
	      opnd_cnt += 1;
	      tok++;
	    }
	  while (xtensa_operand_is_visible (isa, opcode, opnd_cnt) == 0);
	}
    }

  if (num_regs > 0 && ((int) next_reg != last_tok->X_add_number + 1))
    goto err;

  insn->ntok = tok - insn->tok;
  had_error = false;

 err:
  input_line_pointer = old_input_line_pointer;
  return had_error;
}


static int
get_invisible_operands (TInsn *insn)
{
  xtensa_isa isa = xtensa_default_isa;
  static xtensa_insnbuf slotbuf = NULL;
  xtensa_format fmt;
  xtensa_opcode opc = insn->opcode;
  int slot, opnd, fmt_found;
  unsigned val;

  if (!slotbuf)
    slotbuf = xtensa_insnbuf_alloc (isa);

  /* Find format/slot where this can be encoded.  */
  fmt_found = 0;
  slot = 0;
  for (fmt = 0; fmt < xtensa_isa_num_formats (isa); fmt++)
    {
      for (slot = 0; slot < xtensa_format_num_slots (isa, fmt); slot++)
	{
	  if (xtensa_opcode_encode (isa, fmt, slot, slotbuf, opc) == 0)
	    {
	      fmt_found = 1;
	      break;
	    }
	}
      if (fmt_found) break;
    }

  if (!fmt_found)
    {
      as_bad (_("cannot encode opcode \"%s\""), xtensa_opcode_name (isa, opc));
      return -1;
    }

  /* First encode all the visible operands
     (to deal with shared field operands).  */
  for (opnd = 0; opnd < insn->ntok; opnd++)
    {
      if (xtensa_operand_is_visible (isa, opc, opnd) == 1
	  && (insn->tok[opnd].X_op == O_register
	      || insn->tok[opnd].X_op == O_constant))
	{
	  val = insn->tok[opnd].X_add_number;
	  xtensa_operand_encode (isa, opc, opnd, &val);
	  xtensa_operand_set_field (isa, opc, opnd, fmt, slot, slotbuf, val);
	}
    }

  /* Then pull out the values for the invisible ones.  */
  for (opnd = 0; opnd < insn->ntok; opnd++)
    {
      if (xtensa_operand_is_visible (isa, opc, opnd) == 0)
	{
	  xtensa_operand_get_field (isa, opc, opnd, fmt, slot, slotbuf, &val);
	  xtensa_operand_decode (isa, opc, opnd, &val);
	  insn->tok[opnd].X_add_number = val;
	  if (xtensa_operand_is_register (isa, opc, opnd) == 1)
	    insn->tok[opnd].X_op = O_register;
	  else
	    insn->tok[opnd].X_op = O_constant;
	}
    }

  return 0;
}


static void
xg_reverse_shift_count (char **cnt_argp)
{
  char *cnt_arg, *new_arg;
  cnt_arg = *cnt_argp;

  /* replace the argument with "31-(argument)" */
  new_arg = concat ("31-(", cnt_arg, ")", (char *) NULL);

  free (cnt_arg);
  *cnt_argp = new_arg;
}


/* If "arg" is a constant expression, return non-zero with the value
   in *valp.  */

static int
xg_arg_is_constant (char *arg, offsetT *valp)
{
  expressionS exp;
  char *save_ptr = input_line_pointer;

  input_line_pointer = arg;
  expression (&exp);
  input_line_pointer = save_ptr;

  if (exp.X_op == O_constant)
    {
      *valp = exp.X_add_number;
      return 1;
    }

  return 0;
}


static void
xg_replace_opname (char **popname, const char *newop)
{
  free (*popname);
  *popname = xstrdup (newop);
}


static int
xg_check_num_args (int *pnum_args,
		   int expected_num,
		   char *opname,
		   char **arg_strings)
{
  int num_args = *pnum_args;

  if (num_args < expected_num)
    {
      as_bad (_("not enough operands (%d) for '%s'; expected %d"),
	      num_args, opname, expected_num);
      return -1;
    }

  if (num_args > expected_num)
    {
      as_warn (_("too many operands (%d) for '%s'; expected %d"),
	       num_args, opname, expected_num);
      while (num_args-- > expected_num)
	{
	  free (arg_strings[num_args]);
	  arg_strings[num_args] = 0;
	}
      *pnum_args = expected_num;
      return -1;
    }

  return 0;
}


/* If the register is not specified as part of the opcode,
   then get it from the operand and move it to the opcode.  */

static int
xg_translate_sysreg_op (char **popname, int *pnum_args, char **arg_strings)
{
  xtensa_isa isa = xtensa_default_isa;
  xtensa_sysreg sr;
  char *opname, *new_opname;
  const char *sr_name;
  int is_user, is_write;

  opname = *popname;
  if (*opname == '_')
    opname += 1;
  is_user = (opname[1] == 'u');
  is_write = (opname[0] == 'w');

  /* Opname == [rw]ur or [rwx]sr... */

  if (xg_check_num_args (pnum_args, 2, opname, arg_strings))
    return -1;

  /* Check if the argument is a symbolic register name.  */
  sr = xtensa_sysreg_lookup_name (isa, arg_strings[1]);
  /* Handle WSR to "INTSET" as a special case.  */
  if (sr == XTENSA_UNDEFINED && is_write && !is_user
      && !strcasecmp (arg_strings[1], "intset"))
    sr = xtensa_sysreg_lookup_name (isa, "interrupt");
  if (sr == XTENSA_UNDEFINED
      || (xtensa_sysreg_is_user (isa, sr) == 1) != is_user)
    {
      /* Maybe it's a register number.... */
      offsetT val;
      if (!xg_arg_is_constant (arg_strings[1], &val))
	{
	  as_bad (_("invalid register '%s' for '%s' instruction"),
		  arg_strings[1], opname);
	  return -1;
	}
      sr = xtensa_sysreg_lookup (isa, val, is_user);
      if (sr == XTENSA_UNDEFINED)
	{
	  as_bad (_("invalid register number (%ld) for '%s' instruction"),
		  (long) val, opname);
	  return -1;
	}
    }

  /* Remove the last argument, which is now part of the opcode.  */
  free (arg_strings[1]);
  arg_strings[1] = 0;
  *pnum_args = 1;

  /* Translate the opcode.  */
  sr_name = xtensa_sysreg_name (isa, sr);
  /* Another special case for "WSR.INTSET"....  */
  if (is_write && !is_user && !strcasecmp ("interrupt", sr_name))
    sr_name = "intset";
  new_opname = concat (*popname, ".", sr_name, (char *) NULL);
  free (*popname);
  *popname = new_opname;

  return 0;
}


static int
xtensa_translate_old_userreg_ops (char **popname)
{
  xtensa_isa isa = xtensa_default_isa;
  xtensa_sysreg sr;
  char *opname, *new_opname;
  const char *sr_name;
  bool has_underbar = false;

  opname = *popname;
  if (opname[0] == '_')
    {
      has_underbar = true;
      opname += 1;
    }

  sr = xtensa_sysreg_lookup_name (isa, opname + 1);
  if (sr != XTENSA_UNDEFINED)
    {
      /* The new default name ("nnn") is different from the old default
	 name ("URnnn").  The old default is handled below, and we don't
	 want to recognize [RW]nnn, so do nothing if the name is the (new)
	 default.  */
      static char namebuf[10];
      sprintf (namebuf, "%d", xtensa_sysreg_number (isa, sr));
      if (strcmp (namebuf, opname + 1) == 0)
	return 0;
    }
  else
    {
      offsetT val;
      char *end;

      /* Only continue if the reg name is "URnnn".  */
      if (opname[1] != 'u' || opname[2] != 'r')
	return 0;
      val = strtoul (opname + 3, &end, 10);
      if (*end != '\0')
	return 0;

      sr = xtensa_sysreg_lookup (isa, val, 1);
      if (sr == XTENSA_UNDEFINED)
	{
	  as_bad (_("invalid register number (%ld) for '%s'"),
		  (long) val, opname);
	  return -1;
	}
    }

  /* Translate the opcode.  */
  sr_name = xtensa_sysreg_name (isa, sr);
  new_opname = XNEWVEC (char, strlen (sr_name) + 6);
  sprintf (new_opname, "%s%cur.%s", (has_underbar ? "_" : ""),
	   opname[0], sr_name);
  free (*popname);
  *popname = new_opname;

  return 0;
}


static int
xtensa_translate_zero_immed (const char *old_op,
			     const char *new_op,
			     char **popname,
			     int *pnum_args,
			     char **arg_strings)
{
  char *opname;
  offsetT val;

  opname = *popname;
  gas_assert (opname[0] != '_');

  if (strcmp (opname, old_op) != 0)
    return 0;

  if (xg_check_num_args (pnum_args, 3, opname, arg_strings))
    return -1;
  if (xg_arg_is_constant (arg_strings[1], &val) && val == 0)
    {
      xg_replace_opname (popname, new_op);
      free (arg_strings[1]);
      arg_strings[1] = arg_strings[2];
      arg_strings[2] = 0;
      *pnum_args = 2;
    }

  return 0;
}


/* If the instruction is an idiom (i.e., a built-in macro), translate it.
   Returns non-zero if an error was found.  */

static int
xg_translate_idioms (char **popname, int *pnum_args, char **arg_strings)
{
  char *opname = *popname;
  bool has_underbar = false;

  if (*opname == '_')
    {
      has_underbar = true;
      opname += 1;
    }

  if (strcmp (opname, "mov") == 0)
    {
      if (use_transform () && !has_underbar && density_supported)
	xg_replace_opname (popname, "mov.n");
      else
	{
	  if (xg_check_num_args (pnum_args, 2, opname, arg_strings))
	    return -1;
	  xg_replace_opname (popname, (has_underbar ? "_or" : "or"));
	  arg_strings[2] = xstrdup (arg_strings[1]);
	  *pnum_args = 3;
	}
      return 0;
    }

  /* Without an operand, this is given a default immediate operand of 0.  */
  if ((strcmp (opname, "simcall") == 0 && microarch_earliest >= 280000))
    {
      if (*pnum_args == 0)
	{
	  arg_strings[0] = (char *) xmalloc (2);
	  strcpy (arg_strings[0], "0");
	  *pnum_args = 1;
	}
      return 0;
    }

  if (strcmp (opname, "bbsi.l") == 0)
    {
      if (xg_check_num_args (pnum_args, 3, opname, arg_strings))
	return -1;
      xg_replace_opname (popname, (has_underbar ? "_bbsi" : "bbsi"));
      if (target_big_endian)
	xg_reverse_shift_count (&arg_strings[1]);
      return 0;
    }

  if (strcmp (opname, "bbci.l") == 0)
    {
      if (xg_check_num_args (pnum_args, 3, opname, arg_strings))
	return -1;
      xg_replace_opname (popname, (has_underbar ? "_bbci" : "bbci"));
      if (target_big_endian)
	xg_reverse_shift_count (&arg_strings[1]);
      return 0;
    }

  /* Don't do anything special with NOPs inside FLIX instructions.  They
     are handled elsewhere.  Real NOP instructions are always available
     in configurations with FLIX, so this should never be an issue but
     check for it anyway.  */
  if (!cur_vinsn.inside_bundle && xtensa_nop_opcode == XTENSA_UNDEFINED
      && strcmp (opname, "nop") == 0)
    {
      if (use_transform () && !has_underbar && density_supported)
	xg_replace_opname (popname, "nop.n");
      else
	{
	  if (xg_check_num_args (pnum_args, 0, opname, arg_strings))
	    return -1;
	  xg_replace_opname (popname, (has_underbar ? "_or" : "or"));
	  arg_strings[0] = xstrdup ("a1");
	  arg_strings[1] = xstrdup ("a1");
	  arg_strings[2] = xstrdup ("a1");
	  *pnum_args = 3;
	}
      return 0;
    }

  /* Recognize [RW]UR and [RWX]SR.  */
  if ((((opname[0] == 'r' || opname[0] == 'w')
	&& (opname[1] == 'u' || opname[1] == 's'))
       || (opname[0] == 'x' && opname[1] == 's'))
      && opname[2] == 'r'
      && opname[3] == '\0')
    return xg_translate_sysreg_op (popname, pnum_args, arg_strings);

  /* Backward compatibility for RUR and WUR: Recognize [RW]UR<nnn> and
     [RW]<name> if <name> is the non-default name of a user register.  */
  if ((opname[0] == 'r' || opname[0] == 'w')
      && xtensa_opcode_lookup (xtensa_default_isa, opname) == XTENSA_UNDEFINED)
    return xtensa_translate_old_userreg_ops (popname);

  /* Relax branches that don't allow comparisons against an immediate value
     of zero to the corresponding branches with implicit zero immediates.  */
  if (!has_underbar && use_transform ())
    {
      if (xtensa_translate_zero_immed ("bnei", "bnez", popname,
				       pnum_args, arg_strings))
	return -1;

      if (xtensa_translate_zero_immed ("beqi", "beqz", popname,
				       pnum_args, arg_strings))
	return -1;

      if (xtensa_translate_zero_immed ("bgei", "bgez", popname,
				       pnum_args, arg_strings))
	return -1;

      if (xtensa_translate_zero_immed ("blti", "bltz", popname,
				       pnum_args, arg_strings))
	return -1;
    }

  return 0;
}


/* Functions for dealing with the Xtensa ISA.  */

/* Currently the assembler only allows us to use a single target per
   fragment.  Because of this, only one operand for a given
   instruction may be symbolic.  If there is a PC-relative operand,
   the last one is chosen.  Otherwise, the result is the number of the
   last immediate operand, and if there are none of those, we fail and
   return -1.  */

static int
get_relaxable_immed (xtensa_opcode opcode)
{
  int last_immed = -1;
  int noperands, opi;

  if (opcode == XTENSA_UNDEFINED)
    return -1;

  noperands = xtensa_opcode_num_operands (xtensa_default_isa, opcode);
  for (opi = noperands - 1; opi >= 0; opi--)
    {
      if (xtensa_operand_is_visible (xtensa_default_isa, opcode, opi) == 0)
	continue;
      if (xtensa_operand_is_PCrelative (xtensa_default_isa, opcode, opi) == 1)
	return opi;
      if (last_immed == -1
	  && xtensa_operand_is_register (xtensa_default_isa, opcode, opi) == 0)
	last_immed = opi;
    }
  return last_immed;
}


static xtensa_opcode
get_opcode_from_buf (const char *buf, int slot)
{
  static xtensa_insnbuf insnbuf = NULL;
  static xtensa_insnbuf slotbuf = NULL;
  xtensa_isa isa = xtensa_default_isa;
  xtensa_format fmt;

  if (!insnbuf)
    {
      insnbuf = xtensa_insnbuf_alloc (isa);
      slotbuf = xtensa_insnbuf_alloc (isa);
    }

  xtensa_insnbuf_from_chars (isa, insnbuf, (const unsigned char *) buf, 0);
  fmt = xtensa_format_decode (isa, insnbuf);
  if (fmt == XTENSA_UNDEFINED)
    return XTENSA_UNDEFINED;

  if (slot >= xtensa_format_num_slots (isa, fmt))
    return XTENSA_UNDEFINED;

  xtensa_format_get_slot (isa, fmt, slot, insnbuf, slotbuf);
  return xtensa_opcode_decode (isa, fmt, slot, slotbuf);
}


#ifdef TENSILICA_DEBUG

/* For debugging, print out the mapping of opcode numbers to opcodes.  */

static void
xtensa_print_insn_table (void)
{
  int num_opcodes, num_operands;
  xtensa_opcode opcode;
  xtensa_isa isa = xtensa_default_isa;

  num_opcodes = xtensa_isa_num_opcodes (xtensa_default_isa);
  for (opcode = 0; opcode < num_opcodes; opcode++)
    {
      int opn;
      fprintf (stderr, "%d: %s: ", opcode, xtensa_opcode_name (isa, opcode));
      num_operands = xtensa_opcode_num_operands (isa, opcode);
      for (opn = 0; opn < num_operands; opn++)
	{
	  if (xtensa_operand_is_visible (isa, opcode, opn) == 0)
	    continue;
	  if (xtensa_operand_is_register (isa, opcode, opn) == 1)
	    {
	      xtensa_regfile opnd_rf =
		xtensa_operand_regfile (isa, opcode, opn);
	      fprintf (stderr, "%s ", xtensa_regfile_shortname (isa, opnd_rf));
	    }
	  else if (xtensa_operand_is_PCrelative (isa, opcode, opn) == 1)
	    fputs ("[lLr] ", stderr);
	  else
	    fputs ("i ", stderr);
	}
      fprintf (stderr, "\n");
    }
}


static void
print_vliw_insn (xtensa_insnbuf vbuf)
{
  xtensa_isa isa = xtensa_default_isa;
  xtensa_format f = xtensa_format_decode (isa, vbuf);
  xtensa_insnbuf sbuf = xtensa_insnbuf_alloc (isa);
  int op;

  fprintf (stderr, "format = %d\n", f);

  for (op = 0; op < xtensa_format_num_slots (isa, f); op++)
    {
      xtensa_opcode opcode;
      const char *opname;
      int operands;

      xtensa_format_get_slot (isa, f, op, vbuf, sbuf);
      opcode = xtensa_opcode_decode (isa, f, op, sbuf);
      opname = xtensa_opcode_name (isa, opcode);

      fprintf (stderr, "op in slot %i is %s;\n", op, opname);
      fprintf (stderr, "   operands = ");
      for (operands = 0;
	   operands < xtensa_opcode_num_operands (isa, opcode);
	   operands++)
	{
	  unsigned int val;
	  if (xtensa_operand_is_visible (isa, opcode, operands) == 0)
	    continue;
	  xtensa_operand_get_field (isa, opcode, operands, f, op, sbuf, &val);
	  xtensa_operand_decode (isa, opcode, operands, &val);
	  fprintf (stderr, "%d ", val);
	}
      fprintf (stderr, "\n");
    }
  xtensa_insnbuf_free (isa, sbuf);
}

#endif /* TENSILICA_DEBUG */


static bool
is_direct_call_opcode (xtensa_opcode opcode)
{
  xtensa_isa isa = xtensa_default_isa;
  int n, num_operands;

  if (xtensa_opcode_is_call (isa, opcode) != 1)
    return false;

  num_operands = xtensa_opcode_num_operands (isa, opcode);
  for (n = 0; n < num_operands; n++)
    {
      if (xtensa_operand_is_register (isa, opcode, n) == 0
	  && xtensa_operand_is_PCrelative (isa, opcode, n) == 1)
	return true;
    }
  return false;
}


/* Convert from BFD relocation type code to slot and operand number.
   Returns non-zero on failure.  */

static int
decode_reloc (bfd_reloc_code_real_type reloc, int *slot, bool *is_alt)
{
  if (reloc >= BFD_RELOC_XTENSA_SLOT0_OP
      && reloc <= BFD_RELOC_XTENSA_SLOT14_OP)
    {
      *slot = reloc - BFD_RELOC_XTENSA_SLOT0_OP;
      *is_alt = false;
    }
  else if (reloc >= BFD_RELOC_XTENSA_SLOT0_ALT
      && reloc <= BFD_RELOC_XTENSA_SLOT14_ALT)
    {
      *slot = reloc - BFD_RELOC_XTENSA_SLOT0_ALT;
      *is_alt = true;
    }
  else
    return -1;

  return 0;
}


/* Convert from slot number to BFD relocation type code for the
   standard PC-relative relocations.  Return BFD_RELOC_NONE on
   failure.  */

static bfd_reloc_code_real_type
encode_reloc (int slot)
{
  if (slot < 0 || slot > 14)
    return BFD_RELOC_NONE;

  return BFD_RELOC_XTENSA_SLOT0_OP + slot;
}


/* Convert from slot numbers to BFD relocation type code for the
   "alternate" relocations.  Return BFD_RELOC_NONE on failure.  */

static bfd_reloc_code_real_type
encode_alt_reloc (int slot)
{
  if (slot < 0 || slot > 14)
    return BFD_RELOC_NONE;

  return BFD_RELOC_XTENSA_SLOT0_ALT + slot;
}


static void
xtensa_insnbuf_set_operand (xtensa_insnbuf slotbuf,
			    xtensa_format fmt,
			    int slot,
			    xtensa_opcode opcode,
			    int operand,
			    uint32 value,
			    const char *file,
			    unsigned int line)
{
  uint32 valbuf = value;

  if (xtensa_operand_encode (xtensa_default_isa, opcode, operand, &valbuf))
    {
      if (xtensa_operand_is_PCrelative (xtensa_default_isa, opcode, operand)
	  == 1)
	as_bad_where ((char *) file, line,
		      _("operand %d of '%s' has out of range value '%u'"),
		      operand + 1,
		      xtensa_opcode_name (xtensa_default_isa, opcode),
		      value);
      else
	as_bad_where ((char *) file, line,
		      _("operand %d of '%s' has invalid value '%u'"),
		      operand + 1,
		      xtensa_opcode_name (xtensa_default_isa, opcode),
		      value);
      return;
    }

  xtensa_operand_set_field (xtensa_default_isa, opcode, operand, fmt, slot,
			    slotbuf, valbuf);
}


static uint32
xtensa_insnbuf_get_operand (xtensa_insnbuf slotbuf,
			    xtensa_format fmt,
			    int slot,
			    xtensa_opcode opcode,
			    int opnum)
{
  uint32 val = 0;
  (void) xtensa_operand_get_field (xtensa_default_isa, opcode, opnum,
				   fmt, slot, slotbuf, &val);
  (void) xtensa_operand_decode (xtensa_default_isa, opcode, opnum, &val);
  return val;
}


/* Checks for rules from xtensa-relax tables.  */

/* The routine xg_instruction_matches_option_term must return TRUE
   when a given option term is true.  The meaning of all of the option
   terms is given interpretation by this function.  */

static bool
xg_instruction_matches_option_term (TInsn *insn, const ReqOrOption *option)
{
  if (strcmp (option->option_name, "realnop") == 0
      || startswith (option->option_name, "IsaUse"))
    {
      /* These conditions were evaluated statically when building the
	 relaxation table.  There's no need to reevaluate them now.  */
      return true;
    }
  else if (strcmp (option->option_name, "FREEREG") == 0)
    return insn->extra_arg.X_op == O_register;
  else
    {
      as_fatal (_("internal error: unknown option name '%s'"),
		option->option_name);
    }
}


static bool
xg_instruction_matches_or_options (TInsn *insn,
				   const ReqOrOptionList *or_option)
{
  const ReqOrOption *option;
  /* Must match each of the AND terms.  */
  for (option = or_option; option != NULL; option = option->next)
    {
      if (xg_instruction_matches_option_term (insn, option))
	return true;
    }
  return false;
}


static bool
xg_instruction_matches_options (TInsn *insn, const ReqOptionList *options)
{
  const ReqOption *req_options;
  /* Must match each of the AND terms.  */
  for (req_options = options;
       req_options != NULL;
       req_options = req_options->next)
    {
      /* Must match one of the OR clauses.  */
      if (!xg_instruction_matches_or_options (insn,
					      req_options->or_option_terms))
	return false;
    }
  return true;
}


/* Return the transition rule that matches or NULL if none matches.  */

static bool
xg_instruction_matches_rule (TInsn *insn, TransitionRule *rule)
{
  PreconditionList *condition_l;

  if (rule->opcode != insn->opcode)
    return false;

  for (condition_l = rule->conditions;
       condition_l != NULL;
       condition_l = condition_l->next)
    {
      expressionS *exp1;
      expressionS *exp2;
      Precondition *cond = condition_l->precond;

      switch (cond->typ)
	{
	case OP_CONSTANT:
	  /* The expression must be the constant.  */
	  gas_assert (cond->op_num < insn->ntok);
	  exp1 = &insn->tok[cond->op_num];
	  if (expr_is_const (exp1))
	    {
	      switch (cond->cmp)
		{
		case OP_EQUAL:
		  if (get_expr_const (exp1) != cond->op_data)
		    return false;
		  break;
		case OP_NOTEQUAL:
		  if (get_expr_const (exp1) == cond->op_data)
		    return false;
		  break;
		default:
		  return false;
		}
	    }
	  else if (expr_is_register (exp1))
	    {
	      switch (cond->cmp)
		{
		case OP_EQUAL:
		  if (get_expr_register (exp1) != cond->op_data)
		    return false;
		  break;
		case OP_NOTEQUAL:
		  if (get_expr_register (exp1) == cond->op_data)
		    return false;
		  break;
		default:
		  return false;
		}
	    }
	  else
	    return false;
	  break;

	case OP_OPERAND:
	  gas_assert (cond->op_num < insn->ntok);
	  gas_assert (cond->op_data < insn->ntok);
	  exp1 = &insn->tok[cond->op_num];
	  exp2 = &insn->tok[cond->op_data];

	  switch (cond->cmp)
	    {
	    case OP_EQUAL:
	      if (!expr_is_equal (exp1, exp2))
		return false;
	      break;
	    case OP_NOTEQUAL:
	      if (expr_is_equal (exp1, exp2))
		return false;
	      break;
	    }
	  break;

	case OP_LITERAL:
	case OP_LABEL:
	default:
	  return false;
	}
    }
  if (!xg_instruction_matches_options (insn, rule->options))
    return false;

  return true;
}


static int
transition_rule_cmp (const TransitionRule *a, const TransitionRule *b)
{
  bool a_greater = false;
  bool b_greater = false;

  ReqOptionList *l_a = a->options;
  ReqOptionList *l_b = b->options;

  /* We only care if they both are the same except for
     a const16 vs. an l32r.  */

  while (l_a && l_b && ((l_a->next == NULL) == (l_b->next == NULL)))
    {
      ReqOrOptionList *l_or_a = l_a->or_option_terms;
      ReqOrOptionList *l_or_b = l_b->or_option_terms;
      while (l_or_a && l_or_b && ((l_a->next == NULL) == (l_b->next == NULL)))
	{
	  if (l_or_a->is_true != l_or_b->is_true)
	    return 0;
	  if (strcmp (l_or_a->option_name, l_or_b->option_name) != 0)
	    {
	      /* This is the case we care about.  */
	      if (strcmp (l_or_a->option_name, "IsaUseConst16") == 0
		  && strcmp (l_or_b->option_name, "IsaUseL32R") == 0)
		{
		  if (prefer_const16)
		    a_greater = true;
		  else
		    b_greater = true;
		}
	      else if (strcmp (l_or_a->option_name, "IsaUseL32R") == 0
		       && strcmp (l_or_b->option_name, "IsaUseConst16") == 0)
		{
		  if (prefer_const16)
		    b_greater = true;
		  else
		    a_greater = true;
		}
	      else
		return 0;
	    }
	  l_or_a = l_or_a->next;
	  l_or_b = l_or_b->next;
	}
      if (l_or_a || l_or_b)
	return 0;

      l_a = l_a->next;
      l_b = l_b->next;
    }
  if (l_a || l_b)
    return 0;

  /* Incomparable if the substitution was used differently in two cases.  */
  if (a_greater && b_greater)
    return 0;

  if (b_greater)
    return 1;
  if (a_greater)
    return -1;

  return 0;
}


static TransitionRule *
xg_instruction_match (TInsn *insn)
{
  TransitionTable *table = xg_build_simplify_table (&transition_rule_cmp);
  TransitionList *l;
  gas_assert (insn->opcode < table->num_opcodes);

  /* Walk through all of the possible transitions.  */
  for (l = table->table[insn->opcode]; l != NULL; l = l->next)
    {
      TransitionRule *rule = l->rule;
      if (xg_instruction_matches_rule (insn, rule))
	return rule;
    }
  return NULL;
}


/* Various Other Internal Functions.  */

static bool
is_unique_insn_expansion (TransitionRule *r)
{
  if (!r->to_instr || r->to_instr->next != NULL)
    return false;
  if (r->to_instr->typ != INSTR_INSTR)
    return false;
  return true;
}


/* Check if there is exactly one relaxation for INSN that converts it to
   another instruction of equal or larger size.  If so, and if TARG is
   non-null, go ahead and generate the relaxed instruction into TARG.  If
   NARROW_ONLY is true, then only consider relaxations that widen a narrow
   instruction, i.e., ignore relaxations that convert to an instruction of
   equal size.  In some contexts where this function is used, only
   a single widening is allowed and the NARROW_ONLY argument is used to
   exclude cases like ADDI being "widened" to an ADDMI, which may
   later be relaxed to an ADDMI/ADDI pair.  */

bool
xg_is_single_relaxable_insn (TInsn *insn, TInsn *targ, bool narrow_only)
{
  TransitionTable *table = xg_build_widen_table (&transition_rule_cmp);
  TransitionList *l;
  TransitionRule *match = 0;

  gas_assert (insn->insn_type == ITYPE_INSN);
  gas_assert (insn->opcode < table->num_opcodes);

  for (l = table->table[insn->opcode]; l != NULL; l = l->next)
    {
      TransitionRule *rule = l->rule;

      if (xg_instruction_matches_rule (insn, rule)
	  && is_unique_insn_expansion (rule)
	  && (xg_get_single_size (insn->opcode) + (narrow_only ? 1 : 0)
	      <= xg_get_single_size (rule->to_instr->opcode)))
	{
	  if (match)
	    return false;
	  match = rule;
	}
    }
  if (!match)
    return false;

  if (targ)
    xg_build_to_insn (targ, insn, match->to_instr);
  return true;
}


/* Return the maximum number of bytes this opcode can expand to.  */

static int
xg_get_max_insn_widen_size (xtensa_opcode opcode)
{
  TransitionTable *table = xg_build_widen_table (&transition_rule_cmp);
  TransitionList *l;
  int max_size = xg_get_single_size (opcode);

  gas_assert (opcode < table->num_opcodes);

  for (l = table->table[opcode]; l != NULL; l = l->next)
    {
      TransitionRule *rule = l->rule;
      BuildInstr *build_list;
      int this_size = 0;

      if (!rule)
	continue;
      build_list = rule->to_instr;
      if (is_unique_insn_expansion (rule))
	{
	  gas_assert (build_list->typ == INSTR_INSTR);
	  this_size = xg_get_max_insn_widen_size (build_list->opcode);
	}
      else
	for (; build_list != NULL; build_list = build_list->next)
	  {
	    switch (build_list->typ)
	      {
	      case INSTR_INSTR:
		this_size += xg_get_single_size (build_list->opcode);
		break;
	      case INSTR_LITERAL_DEF:
	      case INSTR_LABEL_DEF:
	      default:
		break;
	      }
	  }
      if (this_size > max_size)
	max_size = this_size;
    }
  return max_size;
}


/* Return the maximum number of literal bytes this opcode can generate.  */

static int
xg_get_max_insn_widen_literal_size (xtensa_opcode opcode)
{
  TransitionTable *table = xg_build_widen_table (&transition_rule_cmp);
  TransitionList *l;
  int max_size = 0;

  gas_assert (opcode < table->num_opcodes);

  for (l = table->table[opcode]; l != NULL; l = l->next)
    {
      TransitionRule *rule = l->rule;
      BuildInstr *build_list;
      int this_size = 0;

      if (!rule)
	continue;
      build_list = rule->to_instr;
      if (is_unique_insn_expansion (rule))
	{
	  gas_assert (build_list->typ == INSTR_INSTR);
	  this_size = xg_get_max_insn_widen_literal_size (build_list->opcode);
	}
      else
	for (; build_list != NULL; build_list = build_list->next)
	  {
	    switch (build_list->typ)
	      {
	      case INSTR_LITERAL_DEF:
		/* Hard-coded 4-byte literal.  */
		this_size += 4;
		break;
	      case INSTR_INSTR:
	      case INSTR_LABEL_DEF:
	      default:
		break;
	      }
	  }
      if (this_size > max_size)
	max_size = this_size;
    }
  return max_size;
}


static bool
xg_is_relaxable_insn (TInsn *insn, int lateral_steps)
{
  int steps_taken = 0;
  TransitionTable *table = xg_build_widen_table (&transition_rule_cmp);
  TransitionList *l;

  gas_assert (insn->insn_type == ITYPE_INSN);
  gas_assert (insn->opcode < table->num_opcodes);

  for (l = table->table[insn->opcode]; l != NULL; l = l->next)
    {
      TransitionRule *rule = l->rule;

      if (xg_instruction_matches_rule (insn, rule))
	{
	  if (steps_taken == lateral_steps)
	    return true;
	  steps_taken++;
	}
    }
  return false;
}


static symbolS *
get_special_literal_symbol (void)
{
  static symbolS *sym = NULL;

  if (sym == NULL)
    sym = symbol_find_or_make ("SPECIAL_LITERAL0\001");
  return sym;
}


static symbolS *
get_special_label_symbol (void)
{
  static symbolS *sym = NULL;

  if (sym == NULL)
    sym = symbol_find_or_make ("SPECIAL_LABEL0\001");
  return sym;
}


static bool
xg_valid_literal_expression (const expressionS *exp)
{
  switch (exp->X_op)
    {
    case O_constant:
    case O_symbol:
    case O_big:
    case O_uminus:
    case O_subtract:
    case O_pltrel:
    case O_pcrel:
    case O_tlsfunc:
    case O_tlsarg:
    case O_tpoff:
    case O_dtpoff:
      return true;
    default:
      return false;
    }
}


/* This will check to see if the value can be converted into the
   operand type.  It will return TRUE if it does not fit.  */

static bool
xg_check_operand (int32 value, xtensa_opcode opcode, int operand)
{
  uint32 valbuf = value;
  if (xtensa_operand_encode (xtensa_default_isa, opcode, operand, &valbuf))
    return true;
  return false;
}


/* Assumes: All immeds are constants.  Check that all constants fit
   into their immeds; return FALSE if not.  */

static bool
xg_immeds_fit (const TInsn *insn)
{
  xtensa_isa isa = xtensa_default_isa;
  int i;

  int n = insn->ntok;
  gas_assert (insn->insn_type == ITYPE_INSN);
  for (i = 0; i < n; ++i)
    {
      const expressionS *exp = &insn->tok[i];

      if (xtensa_operand_is_register (isa, insn->opcode, i) == 1)
	continue;

      switch (exp->X_op)
	{
	case O_register:
	case O_constant:
	  if (xg_check_operand (exp->X_add_number, insn->opcode, i))
	    return false;
	  break;

	default:
	  /* The symbol should have a fixup associated with it.  */
	  gas_assert (false);
	  break;
	}
    }
  return true;
}


/* This should only be called after we have an initial
   estimate of the addresses.  */

static bool
xg_symbolic_immeds_fit (const TInsn *insn,
			segT pc_seg,
			fragS *pc_frag,
			offsetT pc_offset,
			long stretch)
{
  xtensa_isa isa = xtensa_default_isa;
  symbolS *symbolP;
  fragS *sym_frag;
  offsetT target, pc;
  uint32 new_offset;
  int i;
  int n = insn->ntok;

  gas_assert (insn->insn_type == ITYPE_INSN);

  for (i = 0; i < n; ++i)
    {
      const expressionS *exp = &insn->tok[i];

      if (xtensa_operand_is_register (isa, insn->opcode, i) == 1)
	continue;

      switch (exp->X_op)
	{
	case O_register:
	case O_constant:
	  if (xg_check_operand (exp->X_add_number, insn->opcode, i))
	    return false;
	  break;

	case O_lo16:
	case O_hi16:
	  /* Check for the worst case.  */
	  if (xg_check_operand (0xffff, insn->opcode, i))
	    return false;
	  break;

	case O_symbol:
	  /* We only allow symbols for PC-relative references.
	     If pc_frag == 0, then we don't have frag locations yet.  */
	  if (pc_frag == 0
	      || xtensa_operand_is_PCrelative (isa, insn->opcode, i) == 0)
	    return false;

	  /* If it is a weak symbol or a symbol in a different section,
	     it cannot be known to fit at assembly time.  */
	  if (S_IS_WEAK (exp->X_add_symbol)
	      || S_GET_SEGMENT (exp->X_add_symbol) != pc_seg)
	    {
	      /* For a direct call with --no-longcalls, be optimistic and
		 assume it will be in range.  If the symbol is weak and
		 undefined, it may remain undefined at link-time, in which
		 case it will have a zero value and almost certainly be out
		 of range for a direct call; thus, relax for undefined weak
		 symbols even if longcalls is not enabled.  */
	      if (is_direct_call_opcode (insn->opcode)
		  && ! pc_frag->tc_frag_data.use_longcalls
		  && (! S_IS_WEAK (exp->X_add_symbol)
		      || S_IS_DEFINED (exp->X_add_symbol)))
		return true;

	      return false;
	    }

	  symbolP = exp->X_add_symbol;
	  sym_frag = symbol_get_frag (symbolP);
	  target = S_GET_VALUE (symbolP) + exp->X_add_number;
	  pc = pc_frag->fr_address + pc_offset;

	  /* If frag has yet to be reached on this pass, assume it
	     will move by STRETCH just as we did.  If this is not so,
	     it will be because some frag between grows, and that will
	     force another pass.  Beware zero-length frags.  There
	     should be a faster way to do this.  */

	  if (stretch != 0
	      && sym_frag->relax_marker != pc_frag->relax_marker
	      && S_GET_SEGMENT (symbolP) == pc_seg)
	    {
	      target += stretch;
	    }

	  new_offset = target;
	  xtensa_operand_do_reloc (isa, insn->opcode, i, &new_offset, pc);
	  if (xg_check_operand (new_offset, insn->opcode, i))
	    return false;
	  break;

	default:
	  /* The symbol should have a fixup associated with it.  */
	  return false;
	}
    }

  return true;
}


/* Return TRUE on success.  */

static bool
xg_build_to_insn (TInsn *targ, TInsn *insn, BuildInstr *bi)
{
  BuildOp *op;
  symbolS *sym;

  tinsn_init (targ);
  targ->debug_line = insn->debug_line;
  targ->loc_directive_seen = insn->loc_directive_seen;
  switch (bi->typ)
    {
    case INSTR_INSTR:
      op = bi->ops;
      targ->opcode = bi->opcode;
      targ->insn_type = ITYPE_INSN;
      targ->is_specific_opcode = false;

      for (; op != NULL; op = op->next)
	{
	  int op_num = op->op_num;
	  int op_data = op->op_data;

	  gas_assert (op->op_num < MAX_INSN_ARGS);

	  if (targ->ntok <= op_num)
	    targ->ntok = op_num + 1;

	  switch (op->typ)
	    {
	    case OP_CONSTANT:
	      set_expr_const (&targ->tok[op_num], op_data);
	      break;
	    case OP_OPERAND:
	      gas_assert (op_data < insn->ntok);
	      copy_expr (&targ->tok[op_num], &insn->tok[op_data]);
	      break;
	    case OP_FREEREG:
	      if (insn->extra_arg.X_op != O_register)
		return false;
	      copy_expr (&targ->tok[op_num], &insn->extra_arg);
	      break;
	    case OP_LITERAL:
	      sym = get_special_literal_symbol ();
	      set_expr_symbol_offset (&targ->tok[op_num], sym, 0);
	      if (insn->tok[op_data].X_op == O_tlsfunc
		  || insn->tok[op_data].X_op == O_tlsarg)
		copy_expr (&targ->extra_arg, &insn->tok[op_data]);
	      break;
	    case OP_LABEL:
	      sym = get_special_label_symbol ();
	      set_expr_symbol_offset (&targ->tok[op_num], sym, 0);
	      break;
	    case OP_OPERAND_HI16U:
	    case OP_OPERAND_LOW16U:
	      gas_assert (op_data < insn->ntok);
	      if (expr_is_const (&insn->tok[op_data]))
		{
		  long val;
		  copy_expr (&targ->tok[op_num], &insn->tok[op_data]);
		  val = xg_apply_userdef_op_fn (op->typ,
						targ->tok[op_num].
						X_add_number);
		  targ->tok[op_num].X_add_number = val;
		}
	      else
		{
		  /* For const16 we can create relocations for these.  */
		  if (targ->opcode == XTENSA_UNDEFINED
		      || (targ->opcode != xtensa_const16_opcode))
		    return false;
		  gas_assert (op_data < insn->ntok);
		  /* Need to build a O_lo16 or O_hi16.  */
		  copy_expr (&targ->tok[op_num], &insn->tok[op_data]);
		  if (targ->tok[op_num].X_op == O_symbol)
		    {
		      if (op->typ == OP_OPERAND_HI16U)
			targ->tok[op_num].X_op = O_hi16;
		      else if (op->typ == OP_OPERAND_LOW16U)
			targ->tok[op_num].X_op = O_lo16;
		      else
			return false;
		    }
		}
	      break;
	    default:
	      /* currently handles:
		 OP_OPERAND_LOW8
		 OP_OPERAND_HI24S
		 OP_OPERAND_F32MINUS */
	      if (xg_has_userdef_op_fn (op->typ))
		{
		  gas_assert (op_data < insn->ntok);
		  if (expr_is_const (&insn->tok[op_data]))
		    {
		      long val;
		      copy_expr (&targ->tok[op_num], &insn->tok[op_data]);
		      val = xg_apply_userdef_op_fn (op->typ,
						    targ->tok[op_num].
						    X_add_number);
		      targ->tok[op_num].X_add_number = val;
		    }
		  else
		    return false; /* We cannot use a relocation for this.  */
		  break;
		}
	      gas_assert (0);
	      break;
	    }
	}
      break;

    case INSTR_LITERAL_DEF:
      op = bi->ops;
      targ->opcode = XTENSA_UNDEFINED;
      targ->insn_type = ITYPE_LITERAL;
      targ->is_specific_opcode = false;
      for (; op != NULL; op = op->next)
	{
	  int op_num = op->op_num;
	  int op_data = op->op_data;
	  gas_assert (op->op_num < MAX_INSN_ARGS);

	  if (targ->ntok <= op_num)
	    targ->ntok = op_num + 1;

	  switch (op->typ)
	    {
	    case OP_OPERAND:
	      gas_assert (op_data < insn->ntok);
	      /* We can only pass resolvable literals through.  */
	      if (!xg_valid_literal_expression (&insn->tok[op_data]))
		return false;
	      copy_expr (&targ->tok[op_num], &insn->tok[op_data]);
	      break;
	    case OP_LITERAL:
	    case OP_CONSTANT:
	    case OP_LABEL:
	    default:
	      gas_assert (0);
	      break;
	    }
	}
      break;

    case INSTR_LABEL_DEF:
      op = bi->ops;
      targ->opcode = XTENSA_UNDEFINED;
      targ->insn_type = ITYPE_LABEL;
      targ->is_specific_opcode = false;
      /* Literal with no ops is a label?  */
      gas_assert (op == NULL);
      break;

    default:
      gas_assert (0);
    }

  return true;
}


/* Return TRUE on success.  */

static bool
xg_build_to_stack (IStack *istack, TInsn *insn, BuildInstr *bi)
{
  for (; bi != NULL; bi = bi->next)
    {
      TInsn *next_insn = istack_push_space (istack);

      if (!xg_build_to_insn (next_insn, insn, bi))
	return false;
    }
  return true;
}


/* Return TRUE on valid expansion.  */

static bool
xg_expand_to_stack (IStack *istack, TInsn *insn, int lateral_steps)
{
  int stack_size = istack->ninsn;
  int steps_taken = 0;
  TransitionTable *table = xg_build_widen_table (&transition_rule_cmp);
  TransitionList *l;

  gas_assert (insn->insn_type == ITYPE_INSN);
  gas_assert (insn->opcode < table->num_opcodes);

  for (l = table->table[insn->opcode]; l != NULL; l = l->next)
    {
      TransitionRule *rule = l->rule;

      if (xg_instruction_matches_rule (insn, rule))
	{
	  if (lateral_steps == steps_taken)
	    {
	      int i;

	      /* This is it.  Expand the rule to the stack.  */
	      if (!xg_build_to_stack (istack, insn, rule->to_instr))
		return false;

	      /* Check to see if it fits.  */
	      for (i = stack_size; i < istack->ninsn; i++)
		{
		  TInsn *tinsn = &istack->insn[i];

		  if (tinsn->insn_type == ITYPE_INSN
		      && !tinsn_has_symbolic_operands (tinsn)
		      && !xg_immeds_fit (tinsn))
		    {
		      istack->ninsn = stack_size;
		      return false;
		    }
		}
	      return true;
	    }
	  steps_taken++;
	}
    }
  return false;
}


/* Relax the assembly instruction at least "min_steps".
   Return the number of steps taken.

   For relaxation to correctly terminate, every relaxation chain must
   terminate in one of two ways:

   1.  If the chain from one instruction to the next consists entirely of
       single instructions, then the chain *must* handle all possible
       immediates without failing.  It must not ever fail because an
       immediate is out of range.  The MOVI.N -> MOVI -> L32R relaxation
       chain is one example.  L32R loads 32 bits, and there cannot be an
       immediate larger than 32 bits, so it satisfies this condition.
       Single instruction relaxation chains are as defined by
       xg_is_single_relaxable_instruction.

   2.  Otherwise, the chain must end in a multi-instruction expansion: e.g.,
       BNEZ.N -> BNEZ -> BNEZ.W15 -> BENZ.N/J

   Strictly speaking, in most cases you can violate condition 1 and be OK
   -- in particular when the last two instructions have the same single
   size.  But nevertheless, you should guarantee the above two conditions.

   We could fix this so that single-instruction expansions correctly
   terminate when they can't handle the range, but the error messages are
   worse, and it actually turns out that in every case but one (18-bit wide
   branches), you need a multi-instruction expansion to get the full range
   anyway.  And because 18-bit branches are handled identically to 15-bit
   branches, there isn't any point in changing it.  */

static int
xg_assembly_relax (IStack *istack,
		   TInsn *insn,
		   segT pc_seg,
		   fragS *pc_frag,	/* if pc_frag == 0, not pc-relative */
		   offsetT pc_offset,	/* offset in fragment */
		   int min_steps,	/* minimum conversion steps */
		   long stretch)	/* number of bytes stretched so far */
{
  int steps_taken = 0;

  /* Some of its immeds don't fit.  Try to build a relaxed version.
     This may go through a couple of stages of single instruction
     transformations before we get there.  */

  TInsn single_target;
  TInsn current_insn;
  int lateral_steps = 0;
  int istack_size = istack->ninsn;

  if (xg_symbolic_immeds_fit (insn, pc_seg, pc_frag, pc_offset, stretch)
      && steps_taken >= min_steps)
    {
      istack_push (istack, insn);
      return steps_taken;
    }
  current_insn = *insn;

  /* Walk through all of the single instruction expansions.  */
  while (xg_is_single_relaxable_insn (&current_insn, &single_target, false))
    {
      steps_taken++;
      if (xg_symbolic_immeds_fit (&single_target, pc_seg, pc_frag, pc_offset,
				  stretch))
	{
	  if (steps_taken >= min_steps)
	    {
	      istack_push (istack, &single_target);
	      return steps_taken;
	    }
	}
      current_insn = single_target;
    }

  /* Now check for a multi-instruction expansion.  */
  while (xg_is_relaxable_insn (&current_insn, lateral_steps))
    {
      if (xg_symbolic_immeds_fit (&current_insn, pc_seg, pc_frag, pc_offset,
				  stretch))
	{
	  if (steps_taken >= min_steps)
	    {
	      istack_push (istack, &current_insn);
	      return steps_taken;
	    }
	}
      steps_taken++;
      if (xg_expand_to_stack (istack, &current_insn, lateral_steps))
	{
	  if (steps_taken >= min_steps)
	    return steps_taken;
	}
      lateral_steps++;
      istack->ninsn = istack_size;
    }

  /* It's not going to work -- use the original.  */
  istack_push (istack, insn);
  return steps_taken;
}


static void
xg_finish_frag (char *last_insn,
		enum xtensa_relax_statesE frag_state,
		enum xtensa_relax_statesE slot0_state,
		int max_growth,
		bool is_insn)
{
  /* Finish off this fragment so that it has at LEAST the desired
     max_growth.  If it doesn't fit in this fragment, close this one
     and start a new one.  In either case, return a pointer to the
     beginning of the growth area.  */

  fragS *old_frag;

  frag_grow (max_growth);
  old_frag = frag_now;

  frag_now->fr_opcode = last_insn;
  if (is_insn)
    frag_now->tc_frag_data.is_insn = true;

  frag_var (rs_machine_dependent, max_growth, max_growth,
	    frag_state, frag_now->fr_symbol, frag_now->fr_offset, last_insn);

  old_frag->tc_frag_data.slot_subtypes[0] = slot0_state;
  xtensa_set_frag_assembly_state (frag_now);

  /* Just to make sure that we did not split it up.  */
  gas_assert (old_frag->fr_next == frag_now);
}


/* Return TRUE if the target frag is one of the next non-empty frags.  */

static bool
is_next_frag_target (const fragS *fragP, const fragS *target)
{
  if (fragP == NULL)
    return false;

  for (; fragP; fragP = fragP->fr_next)
    {
      if (fragP == target)
	return true;
      if (fragP->fr_fix != 0)
	return false;
      if (fragP->fr_type == rs_fill && fragP->fr_offset != 0)
	return false;
      if ((fragP->fr_type == rs_align || fragP->fr_type == rs_align_code)
	  && ((fragP->fr_address % (1 << fragP->fr_offset)) != 0))
	return false;
      if (fragP->fr_type == rs_space)
	return false;
    }
  return false;
}


static bool
is_branch_jmp_to_next (TInsn *insn, fragS *fragP)
{
  xtensa_isa isa = xtensa_default_isa;
  int i;
  int num_ops = xtensa_opcode_num_operands (isa, insn->opcode);
  int target_op = -1;
  symbolS *sym;
  fragS *target_frag;

  if (xtensa_opcode_is_branch (isa, insn->opcode) != 1
      && xtensa_opcode_is_jump (isa, insn->opcode) != 1)
    return false;

  for (i = 0; i < num_ops; i++)
    {
      if (xtensa_operand_is_PCrelative (isa, insn->opcode, i) == 1)
	{
	  target_op = i;
	  break;
	}
    }
  if (target_op == -1)
    return false;

  if (insn->ntok <= target_op)
    return false;

  if (insn->tok[target_op].X_op != O_symbol)
    return false;

  sym = insn->tok[target_op].X_add_symbol;
  if (sym == NULL)
    return false;

  if (insn->tok[target_op].X_add_number != 0)
    return false;

  target_frag = symbol_get_frag (sym);
  if (target_frag == NULL)
    return false;

  if (is_next_frag_target (fragP->fr_next, target_frag)
      && S_GET_VALUE (sym) == target_frag->fr_address)
    return true;

  return false;
}


static void
xg_add_branch_and_loop_targets (TInsn *insn)
{
  xtensa_isa isa = xtensa_default_isa;
  int num_ops = xtensa_opcode_num_operands (isa, insn->opcode);

  if (xtensa_opcode_is_loop (isa, insn->opcode) == 1)
    {
      int i = 1;
      if (xtensa_operand_is_PCrelative (isa, insn->opcode, i) == 1
	  && insn->tok[i].X_op == O_symbol)
	symbol_get_tc (insn->tok[i].X_add_symbol)->is_loop_target = true;
      return;
    }

  if (xtensa_opcode_is_branch (isa, insn->opcode) == 1
      || xtensa_opcode_is_loop (isa, insn->opcode) == 1)
    {
      int i;

      for (i = 0; i < insn->ntok && i < num_ops; i++)
	{
	  if (xtensa_operand_is_PCrelative (isa, insn->opcode, i) == 1
	      && insn->tok[i].X_op == O_symbol)
	    {
	      symbolS *sym = insn->tok[i].X_add_symbol;
	      symbol_get_tc (sym)->is_branch_target = true;
	      if (S_IS_DEFINED (sym))
		symbol_get_frag (sym)->tc_frag_data.is_branch_target = true;
	    }
	}
    }
}


/* Return FALSE if no error.  */

static bool
xg_build_token_insn (BuildInstr *instr_spec, TInsn *old_insn, TInsn *new_insn)
{
  int num_ops = 0;
  BuildOp *b_op;

  switch (instr_spec->typ)
    {
    case INSTR_INSTR:
      new_insn->insn_type = ITYPE_INSN;
      new_insn->opcode = instr_spec->opcode;
      break;
    case INSTR_LITERAL_DEF:
      new_insn->insn_type = ITYPE_LITERAL;
      new_insn->opcode = XTENSA_UNDEFINED;
      break;
    case INSTR_LABEL_DEF:
      abort ();
    }
  new_insn->is_specific_opcode = false;
  new_insn->debug_line = old_insn->debug_line;
  new_insn->loc_directive_seen = old_insn->loc_directive_seen;

  for (b_op = instr_spec->ops; b_op != NULL; b_op = b_op->next)
    {
      expressionS *exp;
      const expressionS *src_exp;

      num_ops++;
      switch (b_op->typ)
	{
	case OP_CONSTANT:
	  /* The expression must be the constant.  */
	  gas_assert (b_op->op_num < MAX_INSN_ARGS);
	  exp = &new_insn->tok[b_op->op_num];
	  set_expr_const (exp, b_op->op_data);
	  break;

	case OP_OPERAND:
	  gas_assert (b_op->op_num < MAX_INSN_ARGS);
	  gas_assert (b_op->op_data < (unsigned) old_insn->ntok);
	  src_exp = &old_insn->tok[b_op->op_data];
	  exp = &new_insn->tok[b_op->op_num];
	  copy_expr (exp, src_exp);
	  break;

	case OP_LITERAL:
	case OP_LABEL:
	  as_bad (_("can't handle generation of literal/labels yet"));
	  gas_assert (0);

	default:
	  as_bad (_("can't handle undefined OP TYPE"));
	  gas_assert (0);
	}
    }

  new_insn->ntok = num_ops;
  return false;
}


/* Return TRUE if it was simplified.  */

static bool
xg_simplify_insn (TInsn *old_insn, TInsn *new_insn)
{
  TransitionRule *rule;
  BuildInstr *insn_spec;

  if (old_insn->is_specific_opcode || !density_supported)
    return false;

  rule = xg_instruction_match (old_insn);
  if (rule == NULL)
    return false;

  insn_spec = rule->to_instr;
  /* There should only be one.  */
  gas_assert (insn_spec != NULL);
  gas_assert (insn_spec->next == NULL);
  if (insn_spec->next != NULL)
    return false;

  xg_build_token_insn (insn_spec, old_insn, new_insn);

  return true;
}


/* xg_expand_assembly_insn: (1) Simplify the instruction, i.e., l32i ->
   l32i.n. (2) Check the number of operands.  (3) Place the instruction
   tokens into the stack or relax it and place multiple
   instructions/literals onto the stack.  Return FALSE if no error.  */

static bool
xg_expand_assembly_insn (IStack *istack, TInsn *orig_insn)
{
  int noperands;
  TInsn new_insn;
  bool do_expand;

  tinsn_init (&new_insn);

  /* Narrow it if we can.  xg_simplify_insn now does all the
     appropriate checking (e.g., for the density option).  */
  if (xg_simplify_insn (orig_insn, &new_insn))
    orig_insn = &new_insn;

  noperands = xtensa_opcode_num_operands (xtensa_default_isa,
					  orig_insn->opcode);
  if (orig_insn->ntok < noperands)
    {
      as_bad (ngettext ("found %d operand for '%s':  Expected %d",
			"found %d operands for '%s':  Expected %d",
			orig_insn->ntok),
	      orig_insn->ntok,
	      xtensa_opcode_name (xtensa_default_isa, orig_insn->opcode),
	      noperands);
      return true;
    }
  if (orig_insn->ntok > noperands)
    as_warn (ngettext ("found %d operand for '%s':  Expected %d",
		       "found %d operands for '%s':  Expected %d",
		       orig_insn->ntok),
	     orig_insn->ntok,
	     xtensa_opcode_name (xtensa_default_isa, orig_insn->opcode),
	     noperands);

  /* If there are not enough operands, we will assert above.  If there
     are too many, just cut out the extras here.  */
  orig_insn->ntok = noperands;

  if (tinsn_has_invalid_symbolic_operands (orig_insn))
    return true;

  /* Special case for extui opcode which has constraints not handled
     by the ordinary operand encoding checks.  The number of operands
     and related syntax issues have already been checked.  */
  if (orig_insn->opcode == xtensa_extui_opcode)
    {
      int shiftimm = orig_insn->tok[2].X_add_number;
      int maskimm = orig_insn->tok[3].X_add_number;
      if (shiftimm + maskimm > 32)
	{
	  as_bad (_("immediate operands sum to greater than 32"));
	  return true;
	}
    }

  /* If the instruction will definitely need to be relaxed, it is better
     to expand it now for better scheduling.  Decide whether to expand
     now....  */
  do_expand = (!orig_insn->is_specific_opcode && use_transform ());

  /* Calls should be expanded to longcalls only in the backend relaxation
     so that the assembly scheduler will keep the L32R/CALLX instructions
     adjacent.  */
  if (is_direct_call_opcode (orig_insn->opcode))
    do_expand = false;

  if (tinsn_has_symbolic_operands (orig_insn))
    {
      /* The values of symbolic operands are not known yet, so only expand
	 now if an operand is "complex" (e.g., difference of symbols) and
	 will have to be stored as a literal regardless of the value.  */
      if (!tinsn_has_complex_operands (orig_insn))
	do_expand = false;
    }
  else if (xg_immeds_fit (orig_insn))
    do_expand = false;

  if (do_expand)
    xg_assembly_relax (istack, orig_insn, 0, 0, 0, 0, 0);
  else
    istack_push (istack, orig_insn);

  return false;
}


/* Return TRUE if the section flags are marked linkonce
   or the name is .gnu.linkonce.*.  */

static int linkonce_len = sizeof (".gnu.linkonce.") - 1;

static bool
get_is_linkonce_section (bfd *abfd ATTRIBUTE_UNUSED, segT sec)
{
  flagword flags, link_once_flags;

  flags = bfd_section_flags (sec);
  link_once_flags = (flags & SEC_LINK_ONCE);

  /* Flags might not be set yet.  */
  if (!link_once_flags
      && strncmp (segment_name (sec), ".gnu.linkonce.", linkonce_len) == 0)
    link_once_flags = SEC_LINK_ONCE;

  return (link_once_flags != 0);
}


static void
xtensa_add_literal_sym (symbolS *sym)
{
  sym_list *l;

  l = XNEW (sym_list);
  l->sym = sym;
  l->next = literal_syms;
  literal_syms = l;
}


static symbolS *
xtensa_create_literal_symbol (segT sec, fragS *frag)
{
  static int lit_num = 0;
  static char name[256];
  symbolS *symbolP;

  sprintf (name, ".L_lit_sym%d", lit_num);

  /* Create a local symbol.  If it is in a linkonce section, we have to
     be careful to make sure that if it is used in a relocation that the
     symbol will be in the output file.  */
  if (get_is_linkonce_section (stdoutput, sec))
    {
      symbolP = symbol_new (name, sec, frag, 0);
      S_CLEAR_EXTERNAL (symbolP);
      /* symbolP->local = 1; */
    }
  else
    symbolP = symbol_new (name, sec, frag, 0);

  xtensa_add_literal_sym (symbolP);

  lit_num++;
  return symbolP;
}


/* Currently all literals that are generated here are 32-bit L32R targets.  */

static symbolS *
xg_assemble_literal (/* const */ TInsn *insn)
{
  emit_state state;
  symbolS *lit_sym = NULL;
  bfd_reloc_code_real_type reloc;
  bool pcrel = false;
  char *p;

  /* size = 4 for L32R.  It could easily be larger when we move to
     larger constants.  Add a parameter later.  */
  offsetT litsize = 4;
  offsetT litalign = 2;		/* 2^2 = 4 */
  expressionS saved_loc;
  expressionS * emit_val;

  set_expr_symbol_offset (&saved_loc, frag_now->fr_symbol, frag_now_fix ());

  gas_assert (insn->insn_type == ITYPE_LITERAL);
  gas_assert (insn->ntok == 1);	/* must be only one token here */

  xtensa_switch_to_literal_fragment (&state);

  emit_val = &insn->tok[0];
  if (emit_val->X_op == O_big)
    {
      int size = emit_val->X_add_number * CHARS_PER_LITTLENUM;
      if (size > litsize)
	{
	  /* This happens when someone writes a "movi a2, big_number".  */
	  as_bad_where (frag_now->fr_file, frag_now->fr_line,
			_("invalid immediate"));
	  xtensa_restore_emit_state (&state);
	  return NULL;
	}
    }

  /* Force a 4-byte align here.  Note that this opens a new frag, so all
     literals done with this function have a frag to themselves.  That's
     important for the way text section literals work.  */
  frag_align (litalign, 0, 0);
  record_alignment (now_seg, litalign);

  switch (emit_val->X_op)
    {
    case O_pcrel:
      pcrel = true;
      /* fall through */
    case O_pltrel:
    case O_tlsfunc:
    case O_tlsarg:
    case O_tpoff:
    case O_dtpoff:
      p = frag_more (litsize);
      xtensa_set_frag_assembly_state (frag_now);
      reloc = map_operator_to_reloc (emit_val->X_op, true);
      if (emit_val->X_add_symbol)
	emit_val->X_op = O_symbol;
      else
	emit_val->X_op = O_constant;
      fix_new_exp (frag_now, p - frag_now->fr_literal,
		   litsize, emit_val, pcrel, reloc);
      break;

    default:
      emit_expr (emit_val, litsize);
      break;
    }

  gas_assert (frag_now->tc_frag_data.literal_frag == NULL);
  frag_now->tc_frag_data.literal_frag = get_literal_pool_location (now_seg);
  frag_now->fr_symbol = xtensa_create_literal_symbol (now_seg, frag_now);
  lit_sym = frag_now->fr_symbol;

  /* Go back.  */
  xtensa_restore_emit_state (&state);
  return lit_sym;
}


static void
xg_assemble_literal_space (/* const */ int size, int slot)
{
  emit_state state;
  /* We might have to do something about this alignment.  It only
     takes effect if something is placed here.  */
  offsetT litalign = 2;		/* 2^2 = 4 */
  fragS *lit_saved_frag;

  gas_assert (size % 4 == 0);

  xtensa_switch_to_literal_fragment (&state);

  /* Force a 4-byte align here.  */
  frag_align (litalign, 0, 0);
  record_alignment (now_seg, litalign);

  frag_grow (size);

  lit_saved_frag = frag_now;
  frag_now->tc_frag_data.literal_frag = get_literal_pool_location (now_seg);
  frag_now->fr_symbol = xtensa_create_literal_symbol (now_seg, frag_now);
  xg_finish_frag (0, RELAX_LITERAL, 0, size, false);

  /* Go back.  */
  xtensa_restore_emit_state (&state);
  frag_now->tc_frag_data.literal_frags[slot] = lit_saved_frag;
}


/* Put in a fixup record based on the opcode.
   Return TRUE on success.  */

static bool
xg_add_opcode_fix (TInsn *tinsn,
		   int opnum,
		   xtensa_format fmt,
		   int slot,
		   expressionS *exp,
		   fragS *fragP,
		   offsetT offset)
{
  xtensa_opcode opcode = tinsn->opcode;
  bfd_reloc_code_real_type reloc;
  reloc_howto_type *howto;
  int fmt_length;
  fixS *the_fix;

  reloc = BFD_RELOC_NONE;

  /* First try the special cases for "alternate" relocs.  */
  if (opcode == xtensa_l32r_opcode)
    {
      if (fragP->tc_frag_data.use_absolute_literals)
	reloc = encode_alt_reloc (slot);
    }
  else if (opcode == xtensa_const16_opcode)
    {
      if (exp->X_op == O_lo16)
	{
	  reloc = encode_reloc (slot);
	  exp->X_op = O_symbol;
	}
      else if (exp->X_op == O_hi16)
	{
	  reloc = encode_alt_reloc (slot);
	  exp->X_op = O_symbol;
	}
    }

  if (opnum != get_relaxable_immed (opcode))
    {
      as_bad (_("invalid relocation for operand %i of '%s'"),
	      opnum + 1, xtensa_opcode_name (xtensa_default_isa, opcode));
      return false;
    }

  /* Handle erroneous "@h" and "@l" expressions here before they propagate
     into the symbol table where the generic portions of the assembler
     won't know what to do with them.  */
  if (exp->X_op == O_lo16 || exp->X_op == O_hi16)
    {
      as_bad (_("invalid expression for operand %i of '%s'"),
	      opnum + 1, xtensa_opcode_name (xtensa_default_isa, opcode));
      return false;
    }

  /* Next try the generic relocs.  */
  if (reloc == BFD_RELOC_NONE)
    reloc = encode_reloc (slot);
  if (reloc == BFD_RELOC_NONE)
    {
      as_bad (_("invalid relocation in instruction slot %i"), slot);
      return false;
    }

  howto = bfd_reloc_type_lookup (stdoutput, reloc);
  if (!howto)
    {
      as_bad (_("undefined symbol for opcode \"%s\""),
	      xtensa_opcode_name (xtensa_default_isa, opcode));
      return false;
    }

  fmt_length = xtensa_format_length (xtensa_default_isa, fmt);
  the_fix = fix_new_exp (fragP, offset, fmt_length, exp,
			 howto->pc_relative, reloc);
  the_fix->fx_no_overflow = 1;
  the_fix->tc_fix_data.X_add_symbol = exp->X_add_symbol;
  the_fix->tc_fix_data.X_add_number = exp->X_add_number;
  the_fix->tc_fix_data.slot = slot;

  return true;
}


static bool
xg_emit_insn_to_buf (TInsn *tinsn,
		     char *buf,
		     fragS *fragP,
		     offsetT offset,
		     bool build_fix)
{
  static xtensa_insnbuf insnbuf = NULL;
  bool has_symbolic_immed = false;
  bool ok = true;

  if (!insnbuf)
    insnbuf = xtensa_insnbuf_alloc (xtensa_default_isa);

  has_symbolic_immed = tinsn_to_insnbuf (tinsn, insnbuf);
  if (has_symbolic_immed && build_fix)
    {
      /* Add a fixup.  */
      xtensa_format fmt = xg_get_single_format (tinsn->opcode);
      int slot = xg_get_single_slot (tinsn->opcode);
      int opnum = get_relaxable_immed (tinsn->opcode);
      expressionS *exp = &tinsn->tok[opnum];

      if (!xg_add_opcode_fix (tinsn, opnum, fmt, slot, exp, fragP, offset))
	ok = false;
    }
  fragP->tc_frag_data.is_insn = true;
  xtensa_insnbuf_to_chars (xtensa_default_isa, insnbuf,
			   (unsigned char *) buf, 0);
  return ok;
}


static void
xg_resolve_literals (TInsn *insn, symbolS *lit_sym)
{
  symbolS *sym = get_special_literal_symbol ();
  int i;
  if (lit_sym == 0)
    return;
  gas_assert (insn->insn_type == ITYPE_INSN);
  for (i = 0; i < insn->ntok; i++)
    if (insn->tok[i].X_add_symbol == sym)
      insn->tok[i].X_add_symbol = lit_sym;

}


static void
xg_resolve_labels (TInsn *insn, symbolS *label_sym)
{
  symbolS *sym = get_special_label_symbol ();
  int i;
  for (i = 0; i < insn->ntok; i++)
    if (insn->tok[i].X_add_symbol == sym)
      insn->tok[i].X_add_symbol = label_sym;

}


/* Return TRUE if the instruction can write to the specified
   integer register.  */

static bool
is_register_writer (const TInsn *insn, const char *regset, int regnum)
{
  int i;
  int num_ops;
  xtensa_isa isa = xtensa_default_isa;

  num_ops = xtensa_opcode_num_operands (isa, insn->opcode);

  for (i = 0; i < num_ops; i++)
    {
      char inout;
      inout = xtensa_operand_inout (isa, insn->opcode, i);
      if ((inout == 'o' || inout == 'm')
	  && xtensa_operand_is_register (isa, insn->opcode, i) == 1)
	{
	  xtensa_regfile opnd_rf =
	    xtensa_operand_regfile (isa, insn->opcode, i);
	  if (!strcmp (xtensa_regfile_shortname (isa, opnd_rf), regset))
	    {
	      if ((insn->tok[i].X_op == O_register)
		  && (insn->tok[i].X_add_number == regnum))
		return true;
	    }
	}
    }
  return false;
}


static bool
is_bad_loopend_opcode (const TInsn *tinsn)
{
  xtensa_opcode opcode = tinsn->opcode;

  if (opcode == XTENSA_UNDEFINED)
    return false;

  if (opcode == xtensa_call0_opcode
      || opcode == xtensa_callx0_opcode
      || opcode == xtensa_call4_opcode
      || opcode == xtensa_callx4_opcode
      || opcode == xtensa_call8_opcode
      || opcode == xtensa_callx8_opcode
      || opcode == xtensa_call12_opcode
      || opcode == xtensa_callx12_opcode
      || opcode == xtensa_isync_opcode
      || opcode == xtensa_ret_opcode
      || opcode == xtensa_ret_n_opcode
      || opcode == xtensa_retw_opcode
      || opcode == xtensa_retw_n_opcode
      || opcode == xtensa_waiti_opcode
      || opcode == xtensa_rsr_lcount_opcode)
    return true;

  return false;
}


/* Labels that begin with ".Ln" or ".LM"  are unaligned.
   This allows the debugger to add unaligned labels.
   Also, the assembler generates stabs labels that need
   not be aligned:  FAKE_LABEL_NAME . {"F", "L", "endfunc"}.  */

static bool
is_unaligned_label (symbolS *sym)
{
  const char *name = S_GET_NAME (sym);
  static size_t fake_size = 0;

  if (name
      && name[0] == '.'
      && name[1] == 'L' && (name[2] == 'n' || name[2] == 'M'))
    return true;

  /* FAKE_LABEL_NAME followed by "F", "L" or "endfunc" */
  if (fake_size == 0)
    fake_size = strlen (FAKE_LABEL_NAME);

  if (name
      && strncmp (FAKE_LABEL_NAME, name, fake_size) == 0
      && (name[fake_size] == 'F'
	  || name[fake_size] == 'L'
	  || (name[fake_size] == 'e'
	      && startswith (name + fake_size, "endfunc"))))
    return true;

  return false;
}


static fragS *
next_non_empty_frag (const fragS *fragP)
{
  fragS *next_fragP = fragP->fr_next;

  /* Sometimes an empty will end up here due storage allocation issues.
     So we have to skip until we find something legit.  */
  while (next_fragP && next_fragP->fr_fix == 0)
    next_fragP = next_fragP->fr_next;

  if (next_fragP == NULL || next_fragP->fr_fix == 0)
    return NULL;

  return next_fragP;
}


static bool
next_frag_opcode_is_loop (const fragS *fragP, xtensa_opcode *opcode)
{
  xtensa_opcode out_opcode;
  const fragS *next_fragP = next_non_empty_frag (fragP);

  if (next_fragP == NULL)
    return false;

  out_opcode = get_opcode_from_buf (next_fragP->fr_literal, 0);
  if (xtensa_opcode_is_loop (xtensa_default_isa, out_opcode) == 1)
    {
      *opcode = out_opcode;
      return true;
    }
  return false;
}


static int
frag_format_size (const fragS *fragP)
{
  static xtensa_insnbuf insnbuf = NULL;
  xtensa_isa isa = xtensa_default_isa;
  xtensa_format fmt;
  int fmt_size;

  if (!insnbuf)
    insnbuf = xtensa_insnbuf_alloc (isa);

  if (fragP == NULL)
    return XTENSA_UNDEFINED;

  xtensa_insnbuf_from_chars (isa, insnbuf,
			     (unsigned char *) fragP->fr_literal, 0);

  fmt = xtensa_format_decode (isa, insnbuf);
  if (fmt == XTENSA_UNDEFINED)
    return XTENSA_UNDEFINED;
  fmt_size = xtensa_format_length (isa, fmt);

  /* If the next format won't be changing due to relaxation, just
     return the length of the first format.  */
  if (fragP->fr_opcode != fragP->fr_literal)
    return fmt_size;

  /* If during relaxation we have to pull an instruction out of a
     multi-slot instruction, we will return the more conservative
     number.  This works because alignment on bigger instructions
     is more restrictive than alignment on smaller instructions.
     This is more conservative than we would like, but it happens
     infrequently.  */

  if (xtensa_format_num_slots (xtensa_default_isa, fmt) > 1)
    return fmt_size;

  /* If we aren't doing one of our own relaxations or it isn't
     slot-based, then the insn size won't change.  */
  if (fragP->fr_type != rs_machine_dependent)
    return fmt_size;
  if (fragP->fr_subtype != RELAX_SLOTS)
    return fmt_size;

  /* If an instruction is about to grow, return the longer size.  */
  if (fragP->tc_frag_data.slot_subtypes[0] == RELAX_IMMED_STEP1
      || fragP->tc_frag_data.slot_subtypes[0] == RELAX_IMMED_STEP2
      || fragP->tc_frag_data.slot_subtypes[0] == RELAX_IMMED_STEP3)
    {
      /* For most frags at RELAX_IMMED_STEPX, with X > 0, the first
	 instruction in the relaxed version is of length 3.  (The case
	 where we have to pull the instruction out of a FLIX bundle
	 is handled conservatively above.)  However, frags with opcodes
	 that are expanding to wide branches end up having formats that
	 are not determinable by the RELAX_IMMED_STEPX enumeration, and
	 we can't tell directly what format the relaxer picked.  This
	 is a wart in the design of the relaxer that should someday be
	 fixed, but would require major changes, or at least should
	 be accompanied by major changes to make use of that data.

	 In any event, we can tell that we are expanding from a single-slot
	 format to a wider one with the logic below.  */

      int i;
      int relaxed_size = fmt_size + fragP->tc_frag_data.text_expansion[0];

      for (i = 0; i < xtensa_isa_num_formats (isa); i++)
	{
	  if (relaxed_size == xtensa_format_length (isa, i))
	    return relaxed_size;
	}

      return 3;
    }

  if (fragP->tc_frag_data.slot_subtypes[0] == RELAX_NARROW)
    return 2 + fragP->tc_frag_data.text_expansion[0];

  return fmt_size;
}


static int
next_frag_format_size (const fragS *fragP)
{
  const fragS *next_fragP = next_non_empty_frag (fragP);
  return frag_format_size (next_fragP);
}


/* In early Xtensa Processors, for reasons that are unclear, the ISA
   required two-byte instructions to be treated as three-byte instructions
   for loop instruction alignment.  This restriction was removed beginning
   with Xtensa LX.  Now the only requirement on loop instruction alignment
   is that the first instruction of the loop must appear at an address that
   does not cross a fetch boundary.  */

static int
get_loop_align_size (int insn_size)
{
  if (insn_size == XTENSA_UNDEFINED)
    return xtensa_fetch_width;

  if (enforce_three_byte_loop_align && insn_size == 2)
    return 3;

  return insn_size;
}


/* If the next legit fragment is an end-of-loop marker,
   switch its state so it will instantiate a NOP.  */

static void
update_next_frag_state (fragS *fragP)
{
  fragS *next_fragP = fragP->fr_next;
  fragS *new_target = NULL;

  if (align_targets)
    {
      /* We are guaranteed there will be one of these...   */
      while (!(next_fragP->fr_type == rs_machine_dependent
	       && (next_fragP->fr_subtype == RELAX_MAYBE_UNREACHABLE
		   || next_fragP->fr_subtype == RELAX_UNREACHABLE)))
	next_fragP = next_fragP->fr_next;

      gas_assert (next_fragP->fr_type == rs_machine_dependent
	      && (next_fragP->fr_subtype == RELAX_MAYBE_UNREACHABLE
		  || next_fragP->fr_subtype == RELAX_UNREACHABLE));

      /* ...and one of these.  */
      new_target = next_fragP->fr_next;
      while (!(new_target->fr_type == rs_machine_dependent
	       && (new_target->fr_subtype == RELAX_MAYBE_DESIRE_ALIGN
		   || new_target->fr_subtype == RELAX_DESIRE_ALIGN)))
	new_target = new_target->fr_next;

      gas_assert (new_target->fr_type == rs_machine_dependent
	      && (new_target->fr_subtype == RELAX_MAYBE_DESIRE_ALIGN
		  || new_target->fr_subtype == RELAX_DESIRE_ALIGN));
    }

  while (next_fragP && next_fragP->fr_fix == 0)
    {
      if (next_fragP->fr_type == rs_machine_dependent
	  && next_fragP->fr_subtype == RELAX_LOOP_END)
	{
	  next_fragP->fr_subtype = RELAX_LOOP_END_ADD_NOP;
	  return;
	}

      next_fragP = next_fragP->fr_next;
    }
}


static bool
next_frag_is_branch_target (const fragS *fragP)
{
  /* Sometimes an empty will end up here due to storage allocation issues,
     so we have to skip until we find something legit.  */
  for (fragP = fragP->fr_next; fragP; fragP = fragP->fr_next)
    {
      if (fragP->tc_frag_data.is_branch_target)
	return true;
      if (fragP->fr_fix != 0)
	break;
    }
  return false;
}


static bool
next_frag_is_loop_target (const fragS *fragP)
{
  /* Sometimes an empty will end up here due storage allocation issues.
     So we have to skip until we find something legit. */
  for (fragP = fragP->fr_next; fragP; fragP = fragP->fr_next)
    {
      if (fragP->tc_frag_data.is_loop_target)
	return true;
      if (fragP->fr_fix != 0)
	break;
    }
  return false;
}


/* As specified in the relaxation table, when a loop instruction is
   relaxed, there are 24 bytes between the loop instruction itself and
   the first instruction in the loop.  */

#define RELAXED_LOOP_INSN_BYTES 24

static addressT
next_frag_pre_opcode_bytes (const fragS *fragp)
{
  const fragS *next_fragp = fragp->fr_next;
  xtensa_opcode next_opcode;

  if (!next_frag_opcode_is_loop (fragp, &next_opcode))
    return 0;

  /* Sometimes an empty will end up here due to storage allocation issues,
     so we have to skip until we find something legit.  */
  while (next_fragp->fr_fix == 0)
    next_fragp = next_fragp->fr_next;

  if (next_fragp->fr_type != rs_machine_dependent)
    return 0;

  /* There is some implicit knowledge encoded in here.
     The LOOP instructions that are NOT RELAX_IMMED have
     been relaxed.  Note that we can assume that the LOOP
     instruction is in slot 0 because loops aren't bundleable.  */
  if (next_fragp->tc_frag_data.slot_subtypes[0] > RELAX_IMMED)
      return get_expanded_loop_offset (next_opcode) + RELAXED_LOOP_INSN_BYTES;

  return 0;
}


/* Mark a location where we can later insert literal frags.  Update
   the section's literal_pool_loc, so subsequent literals can be
   placed nearest to their use.  */

static void
xtensa_mark_literal_pool_location (void)
{
  /* Any labels pointing to the current location need
     to be adjusted to after the literal pool.  */
  fragS *pool_location;

  if (use_literal_section)
    return;

  /* We stash info in these frags so we can later move the literal's
     fixes into this frchain's fix list.  */
  pool_location = frag_now;
  frag_now->tc_frag_data.lit_frchain = frchain_now;
  frag_now->tc_frag_data.literal_frag = frag_now;
  /* Just record this frag.  */
  xtensa_maybe_create_literal_pool_frag (false, false);
  frag_variant (rs_machine_dependent, 0, 0,
		RELAX_LITERAL_POOL_BEGIN, NULL, 0, NULL);
  xtensa_set_frag_assembly_state (frag_now);
  frag_now->tc_frag_data.lit_seg = now_seg;
  frag_variant (rs_machine_dependent, 0, 0,
		RELAX_LITERAL_POOL_END, NULL, 0, NULL);
  xtensa_set_frag_assembly_state (frag_now);

  set_literal_pool_location (now_seg, pool_location);
}


/* Build a nop of the correct size into tinsn.  */

static void
build_nop (TInsn *tinsn, int size)
{
  tinsn_init (tinsn);
  switch (size)
    {
    case 2:
      tinsn->opcode = xtensa_nop_n_opcode;
      tinsn->ntok = 0;
      if (tinsn->opcode == XTENSA_UNDEFINED)
	as_fatal (_("opcode 'NOP.N' unavailable in this configuration"));
      break;

    case 3:
      if (xtensa_nop_opcode == XTENSA_UNDEFINED)
	{
	  tinsn->opcode = xtensa_or_opcode;
	  set_expr_const (&tinsn->tok[0], 1);
	  set_expr_const (&tinsn->tok[1], 1);
	  set_expr_const (&tinsn->tok[2], 1);
	  tinsn->ntok = 3;
	}
      else
	tinsn->opcode = xtensa_nop_opcode;

      gas_assert (tinsn->opcode != XTENSA_UNDEFINED);
    }
}


/* Assemble a NOP of the requested size in the buffer.  User must have
   allocated "buf" with at least "size" bytes.  */

static void
assemble_nop (int size, char *buf)
{
  static xtensa_insnbuf insnbuf = NULL;
  TInsn tinsn;

  build_nop (&tinsn, size);

  if (!insnbuf)
    insnbuf = xtensa_insnbuf_alloc (xtensa_default_isa);

  tinsn_to_insnbuf (&tinsn, insnbuf);
  xtensa_insnbuf_to_chars (xtensa_default_isa, insnbuf,
			   (unsigned char *) buf, 0);
}


/* Return the number of bytes for the offset of the expanded loop
   instruction.  This should be incorporated into the relaxation
   specification but is hard-coded here.  This is used to auto-align
   the loop instruction.  It is invalid to call this function if the
   configuration does not have loops or if the opcode is not a loop
   opcode.  */

static addressT
get_expanded_loop_offset (xtensa_opcode opcode)
{
  /* This is the OFFSET of the loop instruction in the expanded loop.
     This MUST correspond directly to the specification of the loop
     expansion.  It will be validated on fragment conversion.  */
  gas_assert (opcode != XTENSA_UNDEFINED);
  if (opcode == xtensa_loop_opcode)
    return 0;
  if (opcode == xtensa_loopnez_opcode)
    return 3;
  if (opcode == xtensa_loopgtz_opcode)
    return 6;
  as_fatal (_("get_expanded_loop_offset: invalid opcode"));
  return 0;
}


static fragS *
get_literal_pool_location (segT seg)
{
  if (auto_litpools)
    {
      struct litpool_seg *lps = litpool_seg_list.next;
      struct litpool_frag *lpf;
      for ( ; lps && lps->seg->id != seg->id; lps = lps->next)
	;
      if (lps)
	{
	  for (lpf = lps->frag_list.prev; lpf->fragP; lpf = lpf->prev)
	    { /* Skip "candidates" for now.  */
	      if (lpf->fragP->fr_subtype == RELAX_LITERAL_POOL_BEGIN &&
		  lpf->priority == 1)
		return lpf->fragP;
	    }
	  /* Must convert a lower-priority pool.  */
	  for (lpf = lps->frag_list.prev; lpf->fragP; lpf = lpf->prev)
	    {
	      if (lpf->fragP->fr_subtype == RELAX_LITERAL_POOL_BEGIN)
		return lpf->fragP;
	    }
	  /* Still no match -- try for a low priority pool.  */
	  for (lpf = lps->frag_list.prev; lpf->fragP; lpf = lpf->prev)
	    {
	      if (lpf->fragP->fr_subtype == RELAX_LITERAL_POOL_CANDIDATE_BEGIN)
		return lpf->fragP;
	    }
	}
    }
  return seg_info (seg)->tc_segment_info_data.literal_pool_loc;
}


static void
set_literal_pool_location (segT seg, fragS *literal_pool_loc)
{
  seg_info (seg)->tc_segment_info_data.literal_pool_loc = literal_pool_loc;
}


/* Set frag assembly state should be called when a new frag is
   opened and after a frag has been closed.  */

static void
xtensa_set_frag_assembly_state (fragS *fragP)
{
  if (!density_supported)
    fragP->tc_frag_data.is_no_density = true;

  /* This function is called from subsegs_finish, which is called
     after xtensa_md_finish, so we can't use "use_transform" or
     "use_schedule" here.  */
  if (!directive_state[directive_transform])
    fragP->tc_frag_data.is_no_transform = true;
  if (directive_state[directive_longcalls])
    fragP->tc_frag_data.use_longcalls = true;
  fragP->tc_frag_data.use_absolute_literals =
    directive_state[directive_absolute_literals];
  fragP->tc_frag_data.is_assembly_state_set = true;
}


static bool
relaxable_section (asection *sec)
{
  return ((sec->flags & SEC_DEBUGGING) == 0
	  && strcmp (sec->name, ".eh_frame") != 0);
}


static void
xtensa_mark_frags_for_org (void)
{
  segT *seclist;

  /* Walk over each fragment of all of the current segments.  If we find
     a .org frag in any of the segments, mark all frags prior to it as
     "no transform", which will prevent linker optimizations from messing
     up the .org distance.  This should be done after
     xtensa_find_unmarked_state_frags, because we don't want to worry here
     about that function trashing the data we save here.  */

  for (seclist = &stdoutput->sections;
       seclist && *seclist;
       seclist = &(*seclist)->next)
    {
      segT sec = *seclist;
      segment_info_type *seginfo;
      fragS *fragP;
      flagword flags;
      flags = bfd_section_flags (sec);
      if (flags & SEC_DEBUGGING)
	continue;
      if (!(flags & SEC_ALLOC))
	continue;

      seginfo = seg_info (sec);
      if (seginfo && seginfo->frchainP)
	{
	  fragS *last_fragP = seginfo->frchainP->frch_root;
	  for (fragP = seginfo->frchainP->frch_root; fragP;
	       fragP = fragP->fr_next)
	    {
	      /* cvt_frag_to_fill has changed the fr_type of org frags to
		 rs_fill, so use the value as cached in rs_subtype here.  */
	      if (fragP->fr_subtype == RELAX_ORG)
		{
		  while (last_fragP != fragP->fr_next)
		    {
		      last_fragP->tc_frag_data.is_no_transform = true;
		      last_fragP = last_fragP->fr_next;
		    }
		}
	    }
	}
    }
}


static void
xtensa_find_unmarked_state_frags (void)
{
  segT *seclist;

  /* Walk over each fragment of all of the current segments.  For each
     unmarked fragment, mark it with the same info as the previous
     fragment.  */
  for (seclist = &stdoutput->sections;
       seclist && *seclist;
       seclist = &(*seclist)->next)
    {
      segT sec = *seclist;
      segment_info_type *seginfo;
      fragS *fragP;
      flagword flags;
      flags = bfd_section_flags (sec);
      if (flags & SEC_DEBUGGING)
	continue;
      if (!(flags & SEC_ALLOC))
	continue;

      seginfo = seg_info (sec);
      if (seginfo && seginfo->frchainP)
	{
	  fragS *last_fragP = 0;
	  for (fragP = seginfo->frchainP->frch_root; fragP;
	       fragP = fragP->fr_next)
	    {
	      if (fragP->fr_fix != 0
		  && !fragP->tc_frag_data.is_assembly_state_set)
		{
		  if (last_fragP == 0)
		    {
		      as_warn_where (fragP->fr_file, fragP->fr_line,
				     _("assembly state not set for first frag in section %s"),
				     sec->name);
		    }
		  else
		    {
		      fragP->tc_frag_data.is_assembly_state_set = true;
		      fragP->tc_frag_data.is_no_density =
			last_fragP->tc_frag_data.is_no_density;
		      fragP->tc_frag_data.is_no_transform =
			last_fragP->tc_frag_data.is_no_transform;
		      fragP->tc_frag_data.use_longcalls =
			last_fragP->tc_frag_data.use_longcalls;
		      fragP->tc_frag_data.use_absolute_literals =
			last_fragP->tc_frag_data.use_absolute_literals;
		    }
		}
	      if (fragP->tc_frag_data.is_assembly_state_set)
		last_fragP = fragP;
	    }
	}
    }
}


static void
xtensa_find_unaligned_branch_targets (bfd *abfd ATTRIBUTE_UNUSED,
				      asection *sec,
				      void *unused ATTRIBUTE_UNUSED)
{
  flagword flags = bfd_section_flags (sec);
  segment_info_type *seginfo = seg_info (sec);
  fragS *frag = seginfo->frchainP->frch_root;

  if (flags & SEC_CODE)
    {
      xtensa_isa isa = xtensa_default_isa;
      xtensa_insnbuf insnbuf = xtensa_insnbuf_alloc (isa);
      while (frag != NULL)
	{
	  if (frag->tc_frag_data.is_branch_target)
	    {
	      int op_size;
	      addressT branch_align, frag_addr;
	      xtensa_format fmt;

	      xtensa_insnbuf_from_chars
		(isa, insnbuf, (unsigned char *) frag->fr_literal, 0);
	      fmt = xtensa_format_decode (isa, insnbuf);
	      op_size = xtensa_format_length (isa, fmt);
	      branch_align = 1 << branch_align_power (sec);
	      frag_addr = frag->fr_address % branch_align;
	      if (frag_addr + op_size > branch_align)
		as_warn_where (frag->fr_file, frag->fr_line,
			       _("unaligned branch target: %d bytes at 0x%lx"),
			       op_size, (long) frag->fr_address);
	    }
	  frag = frag->fr_next;
	}
      xtensa_insnbuf_free (isa, insnbuf);
    }
}


static void
xtensa_find_unaligned_loops (bfd *abfd ATTRIBUTE_UNUSED,
			     asection *sec,
			     void *unused ATTRIBUTE_UNUSED)
{
  flagword flags = bfd_section_flags (sec);
  segment_info_type *seginfo = seg_info (sec);
  fragS *frag = seginfo->frchainP->frch_root;
  xtensa_isa isa = xtensa_default_isa;

  if (flags & SEC_CODE)
    {
      xtensa_insnbuf insnbuf = xtensa_insnbuf_alloc (isa);
      while (frag != NULL)
	{
	  if (frag->tc_frag_data.is_first_loop_insn)
	    {
	      int op_size;
	      addressT frag_addr;
	      xtensa_format fmt;

	      if (frag->fr_fix == 0)
		frag = next_non_empty_frag (frag);

	      if (frag)
		{
		  xtensa_insnbuf_from_chars
		    (isa, insnbuf, (unsigned char *) frag->fr_literal, 0);
		  fmt = xtensa_format_decode (isa, insnbuf);
		  op_size = xtensa_format_length (isa, fmt);
		  frag_addr = frag->fr_address % xtensa_fetch_width;

		  if (frag_addr + op_size > xtensa_fetch_width)
		    as_warn_where (frag->fr_file, frag->fr_line,
				   _("unaligned loop: %d bytes at 0x%lx"),
				   op_size, (long) frag->fr_address);
		}
	    }
	  frag = frag->fr_next;
	}
      xtensa_insnbuf_free (isa, insnbuf);
    }
}


static int
xg_apply_fix_value (fixS *fixP, valueT val)
{
  xtensa_isa isa = xtensa_default_isa;
  static xtensa_insnbuf insnbuf = NULL;
  static xtensa_insnbuf slotbuf = NULL;
  xtensa_format fmt;
  int slot;
  bool alt_reloc;
  xtensa_opcode opcode;
  char *const fixpos = fixP->fx_frag->fr_literal + fixP->fx_where;

  if (decode_reloc (fixP->fx_r_type, &slot, &alt_reloc)
      || alt_reloc)
    as_fatal (_("unexpected fix"));

  if (!insnbuf)
    {
      insnbuf = xtensa_insnbuf_alloc (isa);
      slotbuf = xtensa_insnbuf_alloc (isa);
    }

  xtensa_insnbuf_from_chars (isa, insnbuf, (unsigned char *) fixpos, 0);
  fmt = xtensa_format_decode (isa, insnbuf);
  if (fmt == XTENSA_UNDEFINED)
    as_fatal (_("undecodable fix"));
  xtensa_format_get_slot (isa, fmt, slot, insnbuf, slotbuf);
  opcode = xtensa_opcode_decode (isa, fmt, slot, slotbuf);
  if (opcode == XTENSA_UNDEFINED)
    as_fatal (_("undecodable fix"));

  /* CONST16 immediates are not PC-relative, despite the fact that we
     reuse the normal PC-relative operand relocations for the low part
     of a CONST16 operand.  */
  if (opcode == xtensa_const16_opcode)
    return 0;

  xtensa_insnbuf_set_operand (slotbuf, fmt, slot, opcode,
			      get_relaxable_immed (opcode), val,
			      fixP->fx_file, fixP->fx_line);

  xtensa_format_set_slot (isa, fmt, slot, insnbuf, slotbuf);
  xtensa_insnbuf_to_chars (isa, insnbuf, (unsigned char *) fixpos, 0);

  return 1;
}


/* External Functions and Other GAS Hooks.  */

const char *
xtensa_target_format (void)
{
  return (target_big_endian ? "elf32-xtensa-be" : "elf32-xtensa-le");
}


void
xtensa_file_arch_init (bfd *abfd)
{
  bfd_set_private_flags (abfd, 0x100 | 0x200);
}


void
md_number_to_chars (char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

static void
xg_init_global_config (void)
{
  target_big_endian = XCHAL_HAVE_BE;

  density_supported = XCHAL_HAVE_DENSITY;
  absolute_literals_supported = XSHAL_USE_ABSOLUTE_LITERALS;
  xtensa_fetch_width = XCHAL_INST_FETCH_WIDTH;

  directive_state[directive_density] = XCHAL_HAVE_DENSITY;
  directive_state[directive_absolute_literals] = XSHAL_USE_ABSOLUTE_LITERALS;

  microarch_earliest = XTENSA_MARCH_EARLIEST;
}

void
xtensa_init (int argc ATTRIBUTE_UNUSED, char **argv ATTRIBUTE_UNUSED)
{
  xg_init_global_config ();
}

/* This function is called once, at assembler startup time.  It should
   set up all the tables, etc. that the MD part of the assembler will
   need.  */

void
md_begin (void)
{
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  xtensa_isa isa;
  int i;

  xtensa_default_isa = xtensa_isa_init (0, 0);
  isa = xtensa_default_isa;

  linkrelax = opt_linkrelax;

  /* Set up the literal sections.  */
  memset (&default_lit_sections, 0, sizeof (default_lit_sections));

  subseg_set (current_section, current_subsec);

  xtensa_addi_opcode = xtensa_opcode_lookup (isa, "addi");
  xtensa_addmi_opcode = xtensa_opcode_lookup (isa, "addmi");
  xtensa_call0_opcode = xtensa_opcode_lookup (isa, "call0");
  xtensa_call4_opcode = xtensa_opcode_lookup (isa, "call4");
  xtensa_call8_opcode = xtensa_opcode_lookup (isa, "call8");
  xtensa_call12_opcode = xtensa_opcode_lookup (isa, "call12");
  xtensa_callx0_opcode = xtensa_opcode_lookup (isa, "callx0");
  xtensa_callx4_opcode = xtensa_opcode_lookup (isa, "callx4");
  xtensa_callx8_opcode = xtensa_opcode_lookup (isa, "callx8");
  xtensa_callx12_opcode = xtensa_opcode_lookup (isa, "callx12");
  xtensa_const16_opcode = xtensa_opcode_lookup (isa, "const16");
  xtensa_entry_opcode = xtensa_opcode_lookup (isa, "entry");
  xtensa_extui_opcode = xtensa_opcode_lookup (isa, "extui");
  xtensa_movi_opcode = xtensa_opcode_lookup (isa, "movi");
  xtensa_movi_n_opcode = xtensa_opcode_lookup (isa, "movi.n");
  xtensa_isync_opcode = xtensa_opcode_lookup (isa, "isync");
  xtensa_j_opcode = xtensa_opcode_lookup (isa, "j");
  xtensa_jx_opcode = xtensa_opcode_lookup (isa, "jx");
  xtensa_l32r_opcode = xtensa_opcode_lookup (isa, "l32r");
  xtensa_loop_opcode = xtensa_opcode_lookup (isa, "loop");
  xtensa_loopnez_opcode = xtensa_opcode_lookup (isa, "loopnez");
  xtensa_loopgtz_opcode = xtensa_opcode_lookup (isa, "loopgtz");
  xtensa_nop_opcode = xtensa_opcode_lookup (isa, "nop");
  xtensa_nop_n_opcode = xtensa_opcode_lookup (isa, "nop.n");
  xtensa_or_opcode = xtensa_opcode_lookup (isa, "or");
  xtensa_ret_opcode = xtensa_opcode_lookup (isa, "ret");
  xtensa_ret_n_opcode = xtensa_opcode_lookup (isa, "ret.n");
  xtensa_retw_opcode = xtensa_opcode_lookup (isa, "retw");
  xtensa_retw_n_opcode = xtensa_opcode_lookup (isa, "retw.n");
  xtensa_rsr_lcount_opcode = xtensa_opcode_lookup (isa, "rsr.lcount");
  xtensa_waiti_opcode = xtensa_opcode_lookup (isa, "waiti");

  for (i = 0; i < xtensa_isa_num_formats (isa); i++)
    {
      int format_slots = xtensa_format_num_slots (isa, i);
      if (format_slots > config_max_slots)
	config_max_slots = format_slots;
    }

  xg_init_vinsn (&cur_vinsn);

  xtensa_num_pipe_stages = xtensa_isa_num_pipe_stages (isa);

  init_op_placement_info_table ();

  /* Set up the assembly state.  */
  if (!frag_now->tc_frag_data.is_assembly_state_set)
    xtensa_set_frag_assembly_state (frag_now);

  if (!use_literal_section)
    xtensa_mark_literal_pool_location ();
}


/* TC_INIT_FIX_DATA hook */

void
xtensa_init_fix_data (fixS *x)
{
  x->tc_fix_data.slot = 0;
  x->tc_fix_data.X_add_symbol = NULL;
  x->tc_fix_data.X_add_number = 0;
}


/* tc_frob_label hook */

void
xtensa_frob_label (symbolS *sym)
{
  float freq;

  if (cur_vinsn.inside_bundle)
    {
      as_bad (_("labels are not valid inside bundles"));
      return;
    }

  freq = get_subseg_target_freq (now_seg, now_subseg);

  /* Since the label was already attached to a frag associated with the
     previous basic block, it now needs to be reset to the current frag.  */
  symbol_set_frag (sym, frag_now);
  S_SET_VALUE (sym, (valueT) frag_now_fix ());

  if (generating_literals)
    xtensa_add_literal_sym (sym);
  else
    xtensa_add_insn_label (sym);

  if (symbol_get_tc (sym)->is_loop_target)
    {
      if ((get_last_insn_flags (now_seg, now_subseg)
	  & FLAG_IS_BAD_LOOPEND) != 0)
	as_bad (_("invalid last instruction for a zero-overhead loop"));

      xtensa_set_frag_assembly_state (frag_now);
      frag_var (rs_machine_dependent, 4, 4, RELAX_LOOP_END,
		frag_now->fr_symbol, frag_now->fr_offset, NULL);

      xtensa_set_frag_assembly_state (frag_now);
      xtensa_move_labels (frag_now, 0);
    }

  /* No target aligning in the absolute section.  */
  if (now_seg != absolute_section
      && !is_unaligned_label (sym)
      && !generating_literals)
    {
      xtensa_set_frag_assembly_state (frag_now);

      if (do_align_targets ())
	frag_var (rs_machine_dependent, 0, (int) freq,
		  RELAX_DESIRE_ALIGN_IF_TARGET, frag_now->fr_symbol,
		  frag_now->fr_offset, NULL);
      else
	frag_var (rs_fill, 0, 0, frag_now->fr_subtype,
		  frag_now->fr_symbol, frag_now->fr_offset, NULL);
      xtensa_set_frag_assembly_state (frag_now);
      xtensa_move_labels (frag_now, 0);
    }

  /* We need to mark the following properties even if we aren't aligning.  */

  /* If the label is already known to be a branch target, i.e., a
     forward branch, mark the frag accordingly.  Backward branches
     are handled by xg_add_branch_and_loop_targets.  */
  if (symbol_get_tc (sym)->is_branch_target)
    symbol_get_frag (sym)->tc_frag_data.is_branch_target = true;

  /* Loops only go forward, so they can be identified here.  */
  if (symbol_get_tc (sym)->is_loop_target)
    symbol_get_frag (sym)->tc_frag_data.is_loop_target = true;

  dwarf2_emit_label (sym);
}


/* tc_unrecognized_line hook */

int
xtensa_unrecognized_line (int ch)
{
  switch (ch)
    {
    case '{' :
      if (cur_vinsn.inside_bundle == 0)
	{
	  /* PR8110: Cannot emit line number info inside a FLIX bundle
	     when using --gstabs.  Temporarily disable debug info.  */
	  generate_lineno_debug ();
	  if (debug_type == DEBUG_STABS)
	    {
	      xt_saved_debug_type = debug_type;
	      debug_type = DEBUG_NONE;
	    }

	  cur_vinsn.inside_bundle = 1;
	}
      else
	{
	  as_bad (_("extra opening brace"));
	  return 0;
	}
      break;

    case '}' :
      if (cur_vinsn.inside_bundle)
	finish_vinsn (&cur_vinsn);
      else
	{
	  as_bad (_("extra closing brace"));
	  return 0;
	}
      break;
    default:
      as_bad (_("syntax error"));
      return 0;
    }
  return 1;
}


/* md_flush_pending_output hook */

void
xtensa_flush_pending_output (void)
{
  /* This line fixes a bug where automatically generated gstabs info
     separates a function label from its entry instruction, ending up
     with the literal position between the function label and the entry
     instruction and crashing code.  It only happens with --gstabs and
     --text-section-literals, and when several other obscure relaxation
     conditions are met.  */
  if (outputting_stabs_line_debug)
    return;

  if (cur_vinsn.inside_bundle)
    as_bad (_("missing closing brace"));

  /* If there is a non-zero instruction fragment, close it.  */
  if (frag_now_fix () != 0 && frag_now->tc_frag_data.is_insn)
    {
      frag_wane (frag_now);
      frag_new (0);
      xtensa_set_frag_assembly_state (frag_now);
    }
  frag_now->tc_frag_data.is_insn = false;

  xtensa_clear_insn_labels ();
}


/* We had an error while parsing an instruction.  The string might look
   like this: "insn arg1, arg2 }".  If so, we need to see the closing
   brace and reset some fields.  Otherwise, the vinsn never gets closed
   and the num_slots field will grow past the end of the array of slots,
   and bad things happen.  */

static void
error_reset_cur_vinsn (void)
{
  if (cur_vinsn.inside_bundle)
    {
      if (*input_line_pointer == '}'
	  || *(input_line_pointer - 1) == '}'
	  || *(input_line_pointer - 2) == '}')
	xg_clear_vinsn (&cur_vinsn);
    }
}


void
md_assemble (char *str)
{
  xtensa_isa isa = xtensa_default_isa;
  char *opname;
  unsigned opnamelen;
  bool has_underbar = false;
  char *arg_strings[MAX_INSN_ARGS];
  int num_args;
  TInsn orig_insn;		/* Original instruction from the input.  */

  tinsn_init (&orig_insn);

  /* Split off the opcode.  */
  opnamelen = strspn (str, "abcdefghijklmnopqrstuvwxyz_/0123456789.");
  opname = xstrndup (str, opnamelen);

  num_args = tokenize_arguments (arg_strings, str + opnamelen);
  if (num_args == -1)
    {
      as_bad (_("syntax error"));
      return;
    }

  if (xg_translate_idioms (&opname, &num_args, arg_strings))
    return;

  /* Check for an underbar prefix.  */
  if (*opname == '_')
    {
      has_underbar = true;
      opname += 1;
    }

  orig_insn.insn_type = ITYPE_INSN;
  orig_insn.ntok = 0;
  orig_insn.is_specific_opcode = (has_underbar || !use_transform ());
  orig_insn.opcode = xtensa_opcode_lookup (isa, opname);

  /* Special case: Check for "CALLXn.TLS" pseudo op.  If found, grab its
     extra argument and set the opcode to "CALLXn".  */
  if (orig_insn.opcode == XTENSA_UNDEFINED
      && strncasecmp (opname, "callx", 5) == 0)
    {
      unsigned long window_size;
      char *suffix;

      window_size = strtoul (opname + 5, &suffix, 10);
      if (suffix != opname + 5
	  && (window_size == 0
	      || window_size == 4
	      || window_size == 8
	      || window_size == 12)
	  && strcasecmp (suffix, ".tls") == 0)
	{
	  switch (window_size)
	    {
	    case 0: orig_insn.opcode = xtensa_callx0_opcode; break;
	    case 4: orig_insn.opcode = xtensa_callx4_opcode; break;
	    case 8: orig_insn.opcode = xtensa_callx8_opcode; break;
	    case 12: orig_insn.opcode = xtensa_callx12_opcode; break;
	    }

	  if (num_args != 2)
	    as_bad (_("wrong number of operands for '%s'"), opname);
	  else
	    {
	      bfd_reloc_code_real_type reloc;
	      char *old_input_line_pointer;
	      expressionS *tok = &orig_insn.extra_arg;

	      old_input_line_pointer = input_line_pointer;
	      input_line_pointer = arg_strings[num_args - 1];

	      expression (tok);
	      if (tok->X_op == O_symbol
		  && ((reloc = xtensa_elf_suffix (&input_line_pointer, tok))
		      == BFD_RELOC_XTENSA_TLS_CALL))
		tok->X_op = map_suffix_reloc_to_operator (reloc);
	      else
		as_bad (_("bad relocation expression for '%s'"), opname);

	      input_line_pointer = old_input_line_pointer;
	      num_args -= 1;
	    }
	}
    }

  /* Special case: Check for "j.l" pseudo op.  */
  if (orig_insn.opcode == XTENSA_UNDEFINED
      && strncasecmp (opname, "j.l", 3) == 0)
    {
      if (num_args != 2)
	as_bad (_("wrong number of operands for '%s'"), opname);
      else
	{
	  char *old_input_line_pointer;
	  expressionS *tok = &orig_insn.extra_arg;

	  old_input_line_pointer = input_line_pointer;
	  input_line_pointer = arg_strings[num_args - 1];

	  expression_maybe_register (xtensa_jx_opcode, 0, tok);
	  input_line_pointer = old_input_line_pointer;

	  num_args -= 1;
	  orig_insn.opcode = xtensa_j_opcode;
	}
    }

  if (orig_insn.opcode == XTENSA_UNDEFINED)
    {
      xtensa_format fmt = xtensa_format_lookup (isa, opname);
      if (fmt == XTENSA_UNDEFINED)
	{
	  as_bad (_("unknown opcode or format name '%s'"), opname);
	  error_reset_cur_vinsn ();
	  return;
	}
      if (!cur_vinsn.inside_bundle)
	{
	  as_bad (_("format names only valid inside bundles"));
	  error_reset_cur_vinsn ();
	  return;
	}
      if (cur_vinsn.format != XTENSA_UNDEFINED)
	as_warn (_("multiple formats specified for one bundle; using '%s'"),
		 opname);
      cur_vinsn.format = fmt;
      free (has_underbar ? opname - 1 : opname);
      error_reset_cur_vinsn ();
      return;
    }

  /* Parse the arguments.  */
  if (parse_arguments (&orig_insn, num_args, arg_strings))
    {
      as_bad (_("syntax error"));
      error_reset_cur_vinsn ();
      return;
    }

  /* Free the opcode and argument strings, now that they've been parsed.  */
  free (has_underbar ? opname - 1 : opname);
  opname = 0;
  while (num_args-- > 0)
    free (arg_strings[num_args]);

  /* Get expressions for invisible operands.  */
  if (get_invisible_operands (&orig_insn))
    {
      error_reset_cur_vinsn ();
      return;
    }

  /* Check for the right number and type of arguments.  */
  if (tinsn_check_arguments (&orig_insn))
    {
      error_reset_cur_vinsn ();
      return;
    }

  /* Record the line number for each TInsn, because a FLIX bundle may be
     spread across multiple input lines and individual instructions may be
     moved around in some cases.  */
  orig_insn.loc_directive_seen = dwarf2_loc_directive_seen;
  dwarf2_where (&orig_insn.debug_line);
  dwarf2_consume_line_info ();

  xg_add_branch_and_loop_targets (&orig_insn);

  /* Check that immediate value for ENTRY is >= 16.  */
  if (orig_insn.opcode == xtensa_entry_opcode && orig_insn.ntok >= 3)
    {
      expressionS *exp = &orig_insn.tok[2];
      if (exp->X_op == O_constant && exp->X_add_number < 16)
	as_warn (_("entry instruction with stack decrement < 16"));
    }

  /* Finish it off:
     assemble_tokens (opcode, tok, ntok);
     expand the tokens from the orig_insn into the
     stack of instructions that will not expand
     unless required at relaxation time.  */

  if (!cur_vinsn.inside_bundle)
    emit_single_op (&orig_insn);
  else /* We are inside a bundle.  */
    {
      cur_vinsn.slots[cur_vinsn.num_slots] = orig_insn;
      cur_vinsn.num_slots++;
      if (*input_line_pointer == '}'
	  || *(input_line_pointer - 1) == '}'
	  || *(input_line_pointer - 2) == '}')
	finish_vinsn (&cur_vinsn);
    }

  /* We've just emitted a new instruction so clear the list of labels.  */
  xtensa_clear_insn_labels ();

  xtensa_check_frag_count ();
}


/* HANDLE_ALIGN hook */

/* For a .align directive, we mark the previous block with the alignment
   information.  This will be placed in the object file in the
   property section corresponding to this section.  */

void
xtensa_handle_align (fragS *fragP)
{
  if (linkrelax
      && ! fragP->tc_frag_data.is_literal
      && (fragP->fr_type == rs_align
	  || fragP->fr_type == rs_align_code)
      && fragP->fr_offset > 0
      && now_seg != bss_section)
    {
      fragP->tc_frag_data.is_align = true;
      fragP->tc_frag_data.alignment = fragP->fr_offset;
    }

  if (fragP->fr_type == rs_align_test)
    {
      int count;
      count = fragP->fr_next->fr_address - fragP->fr_address - fragP->fr_fix;
      if (count != 0)
	as_bad_where (fragP->fr_file, fragP->fr_line,
		      _("unaligned entry instruction"));
    }

  if (linkrelax && fragP->fr_type == rs_org)
    fragP->fr_subtype = RELAX_ORG;
}


/* TC_FRAG_INIT hook */

void
xtensa_frag_init (fragS *frag)
{
  xtensa_set_frag_assembly_state (frag);
}


symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}


/* Round up a section size to the appropriate boundary.  */

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
  return size;			/* Byte alignment is fine.  */
}


long
md_pcrel_from (fixS *fixP)
{
  char *insn_p;
  static xtensa_insnbuf insnbuf = NULL;
  static xtensa_insnbuf slotbuf = NULL;
  int opnum;
  uint32 opnd_value;
  xtensa_opcode opcode;
  xtensa_format fmt;
  int slot;
  xtensa_isa isa = xtensa_default_isa;
  valueT addr = fixP->fx_where + fixP->fx_frag->fr_address;
  bool alt_reloc;

  if (fixP->fx_r_type == BFD_RELOC_XTENSA_ASM_EXPAND)
    return 0;

  if (fixP->fx_r_type == BFD_RELOC_32_PCREL)
    return addr;

  if (!insnbuf)
    {
      insnbuf = xtensa_insnbuf_alloc (isa);
      slotbuf = xtensa_insnbuf_alloc (isa);
    }

  insn_p = &fixP->fx_frag->fr_literal[fixP->fx_where];
  xtensa_insnbuf_from_chars (isa, insnbuf, (unsigned char *) insn_p, 0);
  fmt = xtensa_format_decode (isa, insnbuf);

  if (fmt == XTENSA_UNDEFINED)
    as_fatal (_("bad instruction format"));

  if (decode_reloc (fixP->fx_r_type, &slot, &alt_reloc) != 0)
    as_fatal (_("invalid relocation"));

  xtensa_format_get_slot (isa, fmt, slot, insnbuf, slotbuf);
  opcode = xtensa_opcode_decode (isa, fmt, slot, slotbuf);

  /* Check for "alternate" relocations (operand not specified).  None
     of the current uses for these are really PC-relative.  */
  if (alt_reloc || opcode == xtensa_const16_opcode)
    {
      if (opcode != xtensa_l32r_opcode
	  && opcode != xtensa_const16_opcode)
	as_fatal (_("invalid relocation for '%s' instruction"),
		  xtensa_opcode_name (isa, opcode));
      return 0;
    }

  opnum = get_relaxable_immed (opcode);
  opnd_value = 0;
  if (xtensa_operand_is_PCrelative (isa, opcode, opnum) != 1
      || xtensa_operand_do_reloc (isa, opcode, opnum, &opnd_value, addr))
    {
      as_bad_where (fixP->fx_file,
		    fixP->fx_line,
		    _("invalid relocation for operand %d of '%s'"),
		    opnum, xtensa_opcode_name (isa, opcode));
      return 0;
    }
  return 0 - opnd_value;
}


/* TC_FORCE_RELOCATION hook */

int
xtensa_force_relocation (fixS *fix)
{
  switch (fix->fx_r_type)
    {
    case BFD_RELOC_XTENSA_ASM_EXPAND:
    case BFD_RELOC_XTENSA_SLOT0_ALT:
    case BFD_RELOC_XTENSA_SLOT1_ALT:
    case BFD_RELOC_XTENSA_SLOT2_ALT:
    case BFD_RELOC_XTENSA_SLOT3_ALT:
    case BFD_RELOC_XTENSA_SLOT4_ALT:
    case BFD_RELOC_XTENSA_SLOT5_ALT:
    case BFD_RELOC_XTENSA_SLOT6_ALT:
    case BFD_RELOC_XTENSA_SLOT7_ALT:
    case BFD_RELOC_XTENSA_SLOT8_ALT:
    case BFD_RELOC_XTENSA_SLOT9_ALT:
    case BFD_RELOC_XTENSA_SLOT10_ALT:
    case BFD_RELOC_XTENSA_SLOT11_ALT:
    case BFD_RELOC_XTENSA_SLOT12_ALT:
    case BFD_RELOC_XTENSA_SLOT13_ALT:
    case BFD_RELOC_XTENSA_SLOT14_ALT:
      return 1;
    default:
      break;
    }

  if (linkrelax && fix->fx_addsy
      && relaxable_section (S_GET_SEGMENT (fix->fx_addsy)))
    return 1;

  return generic_force_reloc (fix);
}


/* TC_VALIDATE_FIX_SUB hook */

int
xtensa_validate_fix_sub (fixS *fix)
{
  segT add_symbol_segment, sub_symbol_segment;

  /* The difference of two symbols should be resolved by the assembler when
     linkrelax is not set.  If the linker may relax the section containing
     the symbols, then an Xtensa DIFF relocation must be generated so that
     the linker knows to adjust the difference value.  */
  if (!linkrelax || fix->fx_addsy == NULL)
    return 0;

  /* Make sure both symbols are in the same segment, and that segment is
     "normal" and relaxable.  If the segment is not "normal", then the
     fix is not valid.  If the segment is not "relaxable", then the fix
     should have been handled earlier.  */
  add_symbol_segment = S_GET_SEGMENT (fix->fx_addsy);
  if (! SEG_NORMAL (add_symbol_segment) ||
      ! relaxable_section (add_symbol_segment))
    return 0;
  sub_symbol_segment = S_GET_SEGMENT (fix->fx_subsy);
  return (sub_symbol_segment == add_symbol_segment);
}


/* NO_PSEUDO_DOT hook */

/* This function has nothing to do with pseudo dots, but this is the
   nearest macro to where the check needs to take place.  FIXME: This
   seems wrong.  */

bool
xtensa_check_inside_bundle (void)
{
  if (cur_vinsn.inside_bundle && input_line_pointer[-1] == '.')
    as_bad (_("directives are not valid inside bundles"));

  /* This function must always return FALSE because it is called via a
     macro that has nothing to do with bundling.  */
  return false;
}


/* md_elf_section_change_hook */

void
xtensa_elf_section_change_hook (void)
{
  /* Set up the assembly state.  */
  if (!frag_now->tc_frag_data.is_assembly_state_set)
    xtensa_set_frag_assembly_state (frag_now);

  if (!use_literal_section
      && seg_info (now_seg)->tc_segment_info_data.literal_pool_loc == NULL
      && !xtensa_is_init_fini (now_seg))
    xtensa_mark_literal_pool_location ();
}


/* tc_fix_adjustable hook */

bool
xtensa_fix_adjustable (fixS *fixP)
{
  /* We need the symbol name for the VTABLE entries.  */
  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return 0;

  return 1;
}


/* tc_symbol_new_hook */

symbolS *expr_symbols = NULL;

void
xtensa_symbol_new_hook (symbolS *sym)
{
  if (is_leb128_expr && S_GET_SEGMENT (sym) == expr_section)
    {
      symbol_get_tc (sym)->next_expr_symbol = expr_symbols;
      expr_symbols = sym;
    }
}


void
md_apply_fix (fixS *fixP, valueT *valP, segT seg)
{
  char *const fixpos = fixP->fx_frag->fr_literal + fixP->fx_where;
  valueT val = 0;

  /* Subtracted symbols are only allowed for a few relocation types, and
     unless linkrelax is enabled, they should not make it to this point.  */
  if (fixP->fx_subsy && !(linkrelax && (fixP->fx_r_type == BFD_RELOC_32
					|| fixP->fx_r_type == BFD_RELOC_16
					|| fixP->fx_r_type == BFD_RELOC_8)))
    as_bad_subtract (fixP);

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_32_PCREL:
    case BFD_RELOC_32:
    case BFD_RELOC_16:
    case BFD_RELOC_8:
      if (fixP->fx_subsy)
	{
	  bool neg = (S_GET_VALUE (fixP->fx_addsy) + fixP->fx_offset
		      < S_GET_VALUE (fixP->fx_subsy));

	  switch (fixP->fx_r_type)
	    {
	    case BFD_RELOC_8:
	      fixP->fx_r_type = neg
		? BFD_RELOC_XTENSA_NDIFF8 : BFD_RELOC_XTENSA_PDIFF8;
	      fixP->fx_signed = 0;
	      break;
	    case BFD_RELOC_16:
	      fixP->fx_r_type = neg
		? BFD_RELOC_XTENSA_NDIFF16 : BFD_RELOC_XTENSA_PDIFF16;
	      fixP->fx_signed = 0;
	      break;
	    case BFD_RELOC_32:
	      fixP->fx_r_type = neg
		? BFD_RELOC_XTENSA_NDIFF32 : BFD_RELOC_XTENSA_PDIFF32;
	      fixP->fx_signed = 0;
	      break;
	    default:
	      break;
	    }

	  val = (S_GET_VALUE (fixP->fx_addsy) + fixP->fx_offset
		 - S_GET_VALUE (fixP->fx_subsy));

	  /* The difference value gets written out, and the DIFF reloc
	     identifies the address of the subtracted symbol (i.e., the one
	     with the lowest address).  */
	  *valP = val;
	  fixP->fx_offset -= val;
	  fixP->fx_subsy = NULL;
	}
      else if (! fixP->fx_addsy)
	{
	  val = *valP;
	  fixP->fx_done = 1;
	}
      else if (S_GET_SEGMENT (fixP->fx_addsy) == absolute_section)
	{
	  val = S_GET_VALUE (fixP->fx_addsy) + fixP->fx_offset;
	  *valP = val;
	  fixP->fx_done = 1;
	}
      /* fall through */

    case BFD_RELOC_XTENSA_PLT:
      md_number_to_chars (fixpos, val, fixP->fx_size);
      fixP->fx_no_overflow = 0; /* Use the standard overflow check.  */
      break;

    case BFD_RELOC_XTENSA_TLSDESC_FN:
    case BFD_RELOC_XTENSA_TLSDESC_ARG:
    case BFD_RELOC_XTENSA_TLS_TPOFF:
    case BFD_RELOC_XTENSA_TLS_DTPOFF:
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      md_number_to_chars (fixpos, 0, fixP->fx_size);
      fixP->fx_no_overflow = 0; /* Use the standard overflow check.  */
      break;

    case BFD_RELOC_XTENSA_SLOT0_OP:
    case BFD_RELOC_XTENSA_SLOT1_OP:
    case BFD_RELOC_XTENSA_SLOT2_OP:
    case BFD_RELOC_XTENSA_SLOT3_OP:
    case BFD_RELOC_XTENSA_SLOT4_OP:
    case BFD_RELOC_XTENSA_SLOT5_OP:
    case BFD_RELOC_XTENSA_SLOT6_OP:
    case BFD_RELOC_XTENSA_SLOT7_OP:
    case BFD_RELOC_XTENSA_SLOT8_OP:
    case BFD_RELOC_XTENSA_SLOT9_OP:
    case BFD_RELOC_XTENSA_SLOT10_OP:
    case BFD_RELOC_XTENSA_SLOT11_OP:
    case BFD_RELOC_XTENSA_SLOT12_OP:
    case BFD_RELOC_XTENSA_SLOT13_OP:
    case BFD_RELOC_XTENSA_SLOT14_OP:
      if (linkrelax)
	{
	  /* Write the tentative value of a PC-relative relocation to a
	     local symbol into the instruction.  The value will be ignored
	     by the linker, and it makes the object file disassembly
	     readable when all branch targets are encoded in relocations.  */

	  gas_assert (fixP->fx_addsy);
	  if (S_GET_SEGMENT (fixP->fx_addsy) == seg
	      && !S_FORCE_RELOC (fixP->fx_addsy, 1))
	    {
	      val = (S_GET_VALUE (fixP->fx_addsy) + fixP->fx_offset
		     - md_pcrel_from (fixP));
	      (void) xg_apply_fix_value (fixP, val);
	    }
	}
      else if (! fixP->fx_addsy)
	{
	  val = *valP;
	  if (xg_apply_fix_value (fixP, val))
	    fixP->fx_done = 1;
	}
      break;

    case BFD_RELOC_XTENSA_ASM_EXPAND:
    case BFD_RELOC_XTENSA_TLS_FUNC:
    case BFD_RELOC_XTENSA_TLS_ARG:
    case BFD_RELOC_XTENSA_TLS_CALL:
    case BFD_RELOC_XTENSA_SLOT0_ALT:
    case BFD_RELOC_XTENSA_SLOT1_ALT:
    case BFD_RELOC_XTENSA_SLOT2_ALT:
    case BFD_RELOC_XTENSA_SLOT3_ALT:
    case BFD_RELOC_XTENSA_SLOT4_ALT:
    case BFD_RELOC_XTENSA_SLOT5_ALT:
    case BFD_RELOC_XTENSA_SLOT6_ALT:
    case BFD_RELOC_XTENSA_SLOT7_ALT:
    case BFD_RELOC_XTENSA_SLOT8_ALT:
    case BFD_RELOC_XTENSA_SLOT9_ALT:
    case BFD_RELOC_XTENSA_SLOT10_ALT:
    case BFD_RELOC_XTENSA_SLOT11_ALT:
    case BFD_RELOC_XTENSA_SLOT12_ALT:
    case BFD_RELOC_XTENSA_SLOT13_ALT:
    case BFD_RELOC_XTENSA_SLOT14_ALT:
      /* These all need to be resolved at link-time.  Do nothing now.  */
      break;

    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = 0;
      break;

    default:
      as_bad (_("unhandled local relocation fix %s"),
	      bfd_get_reloc_code_name (fixP->fx_r_type));
    }
}


const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, target_big_endian);
}


int
md_estimate_size_before_relax (fragS *fragP, segT seg ATTRIBUTE_UNUSED)
{
  return total_frag_text_expansion (fragP);
}


/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc;

  reloc = XNEW (arelent);
  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  /* Make sure none of our internal relocations make it this far.
     They'd better have been fully resolved by this point.  */
  gas_assert ((int) fixp->fx_r_type > 0);

  reloc->addend = fixp->fx_offset;

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("cannot represent `%s' relocation in object file"),
		    bfd_get_reloc_code_name (fixp->fx_r_type));
      free (reloc->sym_ptr_ptr);
      free (reloc);
      return NULL;
    }

  if (!fixp->fx_pcrel != !reloc->howto->pc_relative)
    as_fatal (_("internal error; cannot generate `%s' relocation"),
	      bfd_get_reloc_code_name (fixp->fx_r_type));

  return reloc;
}


/* Checks for resource conflicts between instructions.  */

/* The func unit stuff could be implemented as bit-vectors rather
   than the iterative approach here.  If it ends up being too
   slow, we will switch it.  */

resource_table *
new_resource_table (void *data,
		    int cycles,
		    int nu,
		    unit_num_copies_func uncf,
		    opcode_num_units_func onuf,
		    opcode_funcUnit_use_unit_func ouuf,
		    opcode_funcUnit_use_stage_func ousf)
{
  int i;
  resource_table *rt = XNEW (resource_table);
  rt->data = data;
  rt->cycles = cycles;
  rt->allocated_cycles = cycles;
  rt->num_units = nu;
  rt->unit_num_copies = uncf;
  rt->opcode_num_units = onuf;
  rt->opcode_unit_use = ouuf;
  rt->opcode_unit_stage = ousf;

  rt->units = XCNEWVEC (unsigned char *, cycles);
  for (i = 0; i < cycles; i++)
    rt->units[i] = XCNEWVEC (unsigned char, nu);

  return rt;
}


void
clear_resource_table (resource_table *rt)
{
  int i, j;
  for (i = 0; i < rt->allocated_cycles; i++)
    for (j = 0; j < rt->num_units; j++)
      rt->units[i][j] = 0;
}


/* We never shrink it, just fake it into thinking so.  */

void
resize_resource_table (resource_table *rt, int cycles)
{
  int i, old_cycles;

  rt->cycles = cycles;
  if (cycles <= rt->allocated_cycles)
    return;

  old_cycles = rt->allocated_cycles;
  rt->allocated_cycles = cycles;

  rt->units = XRESIZEVEC (unsigned char *, rt->units, rt->allocated_cycles);
  for (i = 0; i < old_cycles; i++)
    rt->units[i] = XRESIZEVEC (unsigned char, rt->units[i], rt->num_units);
  for (i = old_cycles; i < cycles; i++)
    rt->units[i] = XCNEWVEC (unsigned char, rt->num_units);
}


bool
resources_available (resource_table *rt, xtensa_opcode opcode, int cycle)
{
  int i;
  int uses = (rt->opcode_num_units) (rt->data, opcode);

  for (i = 0; i < uses; i++)
    {
      xtensa_funcUnit unit = (rt->opcode_unit_use) (rt->data, opcode, i);
      int stage = (rt->opcode_unit_stage) (rt->data, opcode, i);
      int copies_in_use = rt->units[stage + cycle][unit];
      int copies = (rt->unit_num_copies) (rt->data, unit);
      if (copies_in_use >= copies)
	return false;
    }
  return true;
}


void
reserve_resources (resource_table *rt, xtensa_opcode opcode, int cycle)
{
  int i;
  int uses = (rt->opcode_num_units) (rt->data, opcode);

  for (i = 0; i < uses; i++)
    {
      xtensa_funcUnit unit = (rt->opcode_unit_use) (rt->data, opcode, i);
      int stage = (rt->opcode_unit_stage) (rt->data, opcode, i);
      /* Note that this allows resources to be oversubscribed.  That's
	 essential to the way the optional scheduler works.
	 resources_available reports when a resource is over-subscribed,
	 so it's easy to tell.  */
      rt->units[stage + cycle][unit]++;
    }
}


void
release_resources (resource_table *rt, xtensa_opcode opcode, int cycle)
{
  int i;
  int uses = (rt->opcode_num_units) (rt->data, opcode);

  for (i = 0; i < uses; i++)
    {
      xtensa_funcUnit unit = (rt->opcode_unit_use) (rt->data, opcode, i);
      int stage = (rt->opcode_unit_stage) (rt->data, opcode, i);
      gas_assert (rt->units[stage + cycle][unit] > 0);
      rt->units[stage + cycle][unit]--;
    }
}


/* Wrapper functions make parameterized resource reservation
   more convenient.  */

int
opcode_funcUnit_use_unit (void *data, xtensa_opcode opcode, int idx)
{
  xtensa_funcUnit_use *use = xtensa_opcode_funcUnit_use (data, opcode, idx);
  return use->unit;
}


int
opcode_funcUnit_use_stage (void *data, xtensa_opcode opcode, int idx)
{
  xtensa_funcUnit_use *use = xtensa_opcode_funcUnit_use (data, opcode, idx);
  return use->stage;
}


/* Note that this function does not check issue constraints, but
   solely whether the hardware is available to execute the given
   instructions together.  It also doesn't check if the tinsns
   write the same state, or access the same tieports.  That is
   checked by check_t1_t2_reads_and_writes.  */

static bool
resources_conflict (vliw_insn *vinsn)
{
  int i;
  static resource_table *rt = NULL;

  /* This is the most common case by far.  Optimize it.  */
  if (vinsn->num_slots == 1)
    return false;

  if (rt == NULL)
    {
      xtensa_isa isa = xtensa_default_isa;
      rt = new_resource_table
	(isa, xtensa_num_pipe_stages,
	 xtensa_isa_num_funcUnits (isa),
	 (unit_num_copies_func) xtensa_funcUnit_num_copies,
	 (opcode_num_units_func) xtensa_opcode_num_funcUnit_uses,
	 opcode_funcUnit_use_unit,
	 opcode_funcUnit_use_stage);
    }

  clear_resource_table (rt);

  for (i = 0; i < vinsn->num_slots; i++)
    {
      if (!resources_available (rt, vinsn->slots[i].opcode, 0))
	return true;
      reserve_resources (rt, vinsn->slots[i].opcode, 0);
    }

  return false;
}


/* finish_vinsn, emit_single_op and helper functions.  */

static bool find_vinsn_conflicts (vliw_insn *);
static xtensa_format xg_find_narrowest_format (vliw_insn *);
static void xg_assemble_vliw_tokens (vliw_insn *);


/* We have reached the end of a bundle; emit into the frag.  */

static void
finish_vinsn (vliw_insn *vinsn)
{
  IStack slotstack;
  int i;
  int slots;

  if (find_vinsn_conflicts (vinsn))
    {
      xg_clear_vinsn (vinsn);
      return;
    }

  /* First, find a format that works.  */
  if (vinsn->format == XTENSA_UNDEFINED)
    vinsn->format = xg_find_narrowest_format (vinsn);

  slots = xtensa_format_num_slots (xtensa_default_isa, vinsn->format);
  if (slots > 1
      && produce_flix == FLIX_NONE)
    {
      as_bad (_("The option \"--no-allow-flix\" prohibits multi-slot flix."));
      xg_clear_vinsn (vinsn);
      return;
    }

  if (vinsn->format == XTENSA_UNDEFINED)
    {
      as_bad (_("couldn't find a valid instruction format"));
      fprintf (stderr, _("    ops were: "));
      for (i = 0; i < vinsn->num_slots; i++)
	fprintf (stderr, _(" %s;"),
		 xtensa_opcode_name (xtensa_default_isa,
				     vinsn->slots[i].opcode));
      fprintf (stderr, _("\n"));
      xg_clear_vinsn (vinsn);
      return;
    }

  if (vinsn->num_slots != slots)
    {
      as_bad (_("mismatch for format '%s': #slots = %d, #opcodes = %d"),
	      xtensa_format_name (xtensa_default_isa, vinsn->format),
	      slots, vinsn->num_slots);
      xg_clear_vinsn (vinsn);
      return;
    }

  if (resources_conflict (vinsn))
    {
      as_bad (_("illegal resource usage in bundle"));
      fprintf (stderr, "    ops were: ");
      for (i = 0; i < vinsn->num_slots; i++)
	fprintf (stderr, " %s;",
		 xtensa_opcode_name (xtensa_default_isa,
				     vinsn->slots[i].opcode));
      fprintf (stderr, "\n");
      xg_clear_vinsn (vinsn);
      return;
    }

  for (i = 0; i < vinsn->num_slots; i++)
    {
      if (vinsn->slots[i].opcode != XTENSA_UNDEFINED)
	{
	  symbolS *lit_sym = NULL;
	  int j;
	  bool e = false;
	  bool saved_density = density_supported;

	  /* We don't want to narrow ops inside multi-slot bundles.  */
	  if (vinsn->num_slots > 1)
	    density_supported = false;

	  istack_init (&slotstack);
	  if (vinsn->slots[i].opcode == xtensa_nop_opcode)
	    {
	      vinsn->slots[i].opcode =
		xtensa_format_slot_nop_opcode (xtensa_default_isa,
					       vinsn->format, i);
	      vinsn->slots[i].ntok = 0;
	    }

	  if (xg_expand_assembly_insn (&slotstack, &vinsn->slots[i]))
	    {
	      e = true;
	      continue;
	    }

	  density_supported = saved_density;

	  if (e)
	    {
	      xg_clear_vinsn (vinsn);
	      return;
	    }

	  for (j = 0; j < slotstack.ninsn; j++)
	    {
	      TInsn *insn = &slotstack.insn[j];
	      if (insn->insn_type == ITYPE_LITERAL)
		{
		  gas_assert (lit_sym == NULL);
		  lit_sym = xg_assemble_literal (insn);
		}
	      else
		{
		  gas_assert (insn->insn_type == ITYPE_INSN);
		  if (lit_sym)
		    xg_resolve_literals (insn, lit_sym);
		  if (j != slotstack.ninsn - 1)
		    emit_single_op (insn);
		}
	    }

	  if (vinsn->num_slots > 1)
	    {
	      if (opcode_fits_format_slot
		  (slotstack.insn[slotstack.ninsn - 1].opcode,
		   vinsn->format, i))
		{
		  vinsn->slots[i] = slotstack.insn[slotstack.ninsn - 1];
		}
	      else
		{
		  emit_single_op (&slotstack.insn[slotstack.ninsn - 1]);
		  if (vinsn->format == XTENSA_UNDEFINED)
		    vinsn->slots[i].opcode = xtensa_nop_opcode;
		  else
		    vinsn->slots[i].opcode
		      = xtensa_format_slot_nop_opcode (xtensa_default_isa,
						       vinsn->format, i);

		  vinsn->slots[i].ntok = 0;
		}
	    }
	  else
	    {
	      vinsn->slots[0] = slotstack.insn[slotstack.ninsn - 1];
	      vinsn->format = XTENSA_UNDEFINED;
	    }
	}
    }

  /* Now check resource conflicts on the modified bundle.  */
  if (resources_conflict (vinsn))
    {
      as_bad (_("illegal resource usage in bundle"));
      fprintf (stderr, "    ops were: ");
      for (i = 0; i < vinsn->num_slots; i++)
	fprintf (stderr, " %s;",
		 xtensa_opcode_name (xtensa_default_isa,
				     vinsn->slots[i].opcode));
      fprintf (stderr, "\n");
      xg_clear_vinsn (vinsn);
      return;
    }

  /* First, find a format that works.  */
  if (vinsn->format == XTENSA_UNDEFINED)
      vinsn->format = xg_find_narrowest_format (vinsn);

  xg_assemble_vliw_tokens (vinsn);

  xg_clear_vinsn (vinsn);

  xtensa_check_frag_count ();
}


/* Given an vliw instruction, what conflicts are there in register
   usage and in writes to states and queues?

   This function does two things:
   1. Reports an error when a vinsn contains illegal combinations
      of writes to registers states or queues.
   2. Marks individual tinsns as not relaxable if the combination
      contains antidependencies.

   Job 2 handles things like swap semantics in instructions that need
   to be relaxed.  For example,

	addi a0, a1, 100000

   normally would be relaxed to

	l32r a0, some_label
	add a0, a1, a0

   _but_, if the above instruction is bundled with an a0 reader, e.g.,

	{ addi a0, a1, 10000 ; add a2, a0, a4 ; }

   then we can't relax it into

	l32r a0, some_label
	{ add a0, a1, a0 ; add a2, a0, a4 ; }

   because the value of a0 is trashed before the second add can read it.  */

static char check_t1_t2_reads_and_writes (TInsn *, TInsn *);

static bool
find_vinsn_conflicts (vliw_insn *vinsn)
{
  int i, j;
  int branches = 0;
  xtensa_isa isa = xtensa_default_isa;

  gas_assert (!past_xtensa_md_finish);

  for (i = 0 ; i < vinsn->num_slots; i++)
    {
      TInsn *op1 = &vinsn->slots[i];
      if (op1->is_specific_opcode)
	op1->keep_wide = true;
      else
	op1->keep_wide = false;
    }

  for (i = 0 ; i < vinsn->num_slots; i++)
    {
      TInsn *op1 = &vinsn->slots[i];

      if (xtensa_opcode_is_branch (isa, op1->opcode) == 1)
	branches++;

      for (j = 0; j < vinsn->num_slots; j++)
	{
	  if (i != j)
	    {
	      TInsn *op2 = &vinsn->slots[j];
	      char conflict_type = check_t1_t2_reads_and_writes (op1, op2);
	      switch (conflict_type)
		{
		case 'c':
		  as_bad (_("opcodes '%s' (slot %d) and '%s' (slot %d) write the same register"),
			  xtensa_opcode_name (isa, op1->opcode), i,
			  xtensa_opcode_name (isa, op2->opcode), j);
		  return true;
		case 'd':
		  as_bad (_("opcodes '%s' (slot %d) and '%s' (slot %d) write the same state"),
			  xtensa_opcode_name (isa, op1->opcode), i,
			  xtensa_opcode_name (isa, op2->opcode), j);
		  return true;
		case 'e':
		  as_bad (_("opcodes '%s' (slot %d) and '%s' (slot %d) write the same port"),
			  xtensa_opcode_name (isa, op1->opcode), i,
			  xtensa_opcode_name (isa, op2->opcode), j);
		  return true;
		case 'f':
		  as_bad (_("opcodes '%s' (slot %d) and '%s' (slot %d) both have volatile port accesses"),
			  xtensa_opcode_name (isa, op1->opcode), i,
			  xtensa_opcode_name (isa, op2->opcode), j);
		  return true;
		default:
		  /* Everything is OK.  */
		  break;
		}
	      op2->is_specific_opcode = (op2->is_specific_opcode
					 || conflict_type == 'a');
	    }
	}
    }

  if (branches > 1)
    {
      as_bad (_("multiple branches or jumps in the same bundle"));
      return true;
    }

  return false;
}


/* Check how the state used by t1 and t2 relate.
   Cases found are:

   case A: t1 reads a register t2 writes (an antidependency within a bundle)
   case B: no relationship between what is read and written (both could
           read the same reg though)
   case C: t1 writes a register t2 writes (a register conflict within a
           bundle)
   case D: t1 writes a state that t2 also writes
   case E: t1 writes a tie queue that t2 also writes
   case F: two volatile queue accesses
*/

static char
check_t1_t2_reads_and_writes (TInsn *t1, TInsn *t2)
{
  xtensa_isa isa = xtensa_default_isa;
  xtensa_regfile t1_regfile, t2_regfile;
  int t1_reg, t2_reg;
  int t1_base_reg, t1_last_reg;
  int t2_base_reg, t2_last_reg;
  char t1_inout, t2_inout;
  int i, j;
  char conflict = 'b';
  int t1_states;
  int t2_states;
  int t1_interfaces;
  int t2_interfaces;
  bool t1_volatile = false;
  bool t2_volatile = false;

  /* Check registers.  */
  for (j = 0; j < t2->ntok; j++)
    {
      if (xtensa_operand_is_register (isa, t2->opcode, j) != 1)
	continue;

      t2_regfile = xtensa_operand_regfile (isa, t2->opcode, j);
      t2_base_reg = t2->tok[j].X_add_number;
      t2_last_reg = t2_base_reg + xtensa_operand_num_regs (isa, t2->opcode, j);

      for (i = 0; i < t1->ntok; i++)
	{
	  if (xtensa_operand_is_register (isa, t1->opcode, i) != 1)
	    continue;

	  t1_regfile = xtensa_operand_regfile (isa, t1->opcode, i);

	  if (t1_regfile != t2_regfile)
	    continue;

	  t1_inout = xtensa_operand_inout (isa, t1->opcode, i);
	  t2_inout = xtensa_operand_inout (isa, t2->opcode, j);

	  if (xtensa_operand_is_known_reg (isa, t1->opcode, i) == 0
	      || xtensa_operand_is_known_reg (isa, t2->opcode, j) == 0)
	    {
	      if (t1_inout == 'm' || t1_inout == 'o'
		  || t2_inout == 'm' || t2_inout == 'o')
		{
		  conflict = 'a';
		  continue;
		}
	    }

	  t1_base_reg = t1->tok[i].X_add_number;
	  t1_last_reg = (t1_base_reg
			 + xtensa_operand_num_regs (isa, t1->opcode, i));

	  for (t1_reg = t1_base_reg; t1_reg < t1_last_reg; t1_reg++)
	    {
	      for (t2_reg = t2_base_reg; t2_reg < t2_last_reg; t2_reg++)
		{
		  if (t1_reg != t2_reg)
		    continue;

		  if (t2_inout == 'i' && (t1_inout == 'm' || t1_inout == 'o'))
		    {
		      conflict = 'a';
		      continue;
		    }

		  if (t1_inout == 'i' && (t2_inout == 'm' || t2_inout == 'o'))
		    {
		      conflict = 'a';
		      continue;
		    }

		  if (t1_inout != 'i' && t2_inout != 'i')
		    return 'c';
		}
	    }
	}
    }

  /* Check states.  */
  t1_states = xtensa_opcode_num_stateOperands (isa, t1->opcode);
  t2_states = xtensa_opcode_num_stateOperands (isa, t2->opcode);
  for (j = 0; j < t2_states; j++)
    {
      xtensa_state t2_so = xtensa_stateOperand_state (isa, t2->opcode, j);
      t2_inout = xtensa_stateOperand_inout (isa, t2->opcode, j);
      for (i = 0; i < t1_states; i++)
	{
	  xtensa_state t1_so = xtensa_stateOperand_state (isa, t1->opcode, i);
	  t1_inout = xtensa_stateOperand_inout (isa, t1->opcode, i);
	  if (t1_so != t2_so || xtensa_state_is_shared_or (isa, t1_so) == 1)
	    continue;

	  if (t2_inout == 'i' && (t1_inout == 'm' || t1_inout == 'o'))
	    {
	      conflict = 'a';
	      continue;
	    }

	  if (t1_inout == 'i' && (t2_inout == 'm' || t2_inout == 'o'))
	    {
	      conflict = 'a';
	      continue;
	    }

	  if (t1_inout != 'i' && t2_inout != 'i')
	    return 'd';
	}
    }

  /* Check tieports.  */
  t1_interfaces = xtensa_opcode_num_interfaceOperands (isa, t1->opcode);
  t2_interfaces = xtensa_opcode_num_interfaceOperands (isa, t2->opcode);
  for (j = 0; j < t2_interfaces; j++)
    {
      xtensa_interface t2_int
	= xtensa_interfaceOperand_interface (isa, t2->opcode, j);
      int t2_class = xtensa_interface_class_id (isa, t2_int);

      t2_inout = xtensa_interface_inout (isa, t2_int);
      if (xtensa_interface_has_side_effect (isa, t2_int) == 1)
	t2_volatile = true;

      for (i = 0; i < t1_interfaces; i++)
	{
	  xtensa_interface t1_int
	    = xtensa_interfaceOperand_interface (isa, t1->opcode, j);
	  int t1_class = xtensa_interface_class_id (isa, t1_int);

	  t1_inout = xtensa_interface_inout (isa, t1_int);
	  if (xtensa_interface_has_side_effect (isa, t1_int) == 1)
	    t1_volatile = true;

	  if (t1_volatile && t2_volatile && (t1_class == t2_class))
	    return 'f';

	  if (t1_int != t2_int)
	    continue;

	  if (t2_inout == 'i' && t1_inout == 'o')
	    {
	      conflict = 'a';
	      continue;
	    }

	  if (t1_inout == 'i' && t2_inout == 'o')
	    {
	      conflict = 'a';
	      continue;
	    }

	  if (t1_inout != 'i' && t2_inout != 'i')
	    return 'e';
	}
    }

  return conflict;
}


static xtensa_format
xg_find_narrowest_format (vliw_insn *vinsn)
{
  /* Right now we assume that the ops within the vinsn are properly
     ordered for the slots that the programmer wanted them in.  In
     other words, we don't rearrange the ops in hopes of finding a
     better format.  The scheduler handles that.  */

  xtensa_isa isa = xtensa_default_isa;
  xtensa_format format;
  xtensa_opcode nop_opcode = xtensa_nop_opcode;

  if (vinsn->num_slots == 1)
    return xg_get_single_format (vinsn->slots[0].opcode);

  for (format = 0; format < xtensa_isa_num_formats (isa); format++)
    {
      vliw_insn v_copy;
      xg_copy_vinsn (&v_copy, vinsn);
      if (xtensa_format_num_slots (isa, format) == v_copy.num_slots)
	{
	  int slot;
	  int fit = 0;
	  for (slot = 0; slot < v_copy.num_slots; slot++)
	    {
	      if (v_copy.slots[slot].opcode == nop_opcode)
		{
		  v_copy.slots[slot].opcode =
		    xtensa_format_slot_nop_opcode (isa, format, slot);
		  v_copy.slots[slot].ntok = 0;
		}

	      if (opcode_fits_format_slot (v_copy.slots[slot].opcode,
					   format, slot))
		fit++;
	      else if (v_copy.num_slots > 1)
		{
		  TInsn widened;
		  /* Try the widened version.  */
		  if (!v_copy.slots[slot].keep_wide
		      && !v_copy.slots[slot].is_specific_opcode
		      && xg_is_single_relaxable_insn (&v_copy.slots[slot],
						      &widened, true)
		      && opcode_fits_format_slot (widened.opcode,
						  format, slot))
		    {
		      v_copy.slots[slot] = widened;
		      fit++;
		    }
		}
	    }
	  if (fit == v_copy.num_slots)
	    {
	      xg_copy_vinsn (vinsn, &v_copy);
	      xtensa_format_encode (isa, format, vinsn->insnbuf);
	      vinsn->format = format;
	      break;
	    }
	}
    }

  if (format == xtensa_isa_num_formats (isa))
    return XTENSA_UNDEFINED;

  return format;
}


/* Return the additional space needed in a frag
   for possible relaxations of any ops in a VLIW insn.
   Also fill out the relaxations that might be required of
   each tinsn in the vinsn.  */

static int
relaxation_requirements (vliw_insn *vinsn, bool *pfinish_frag)
{
  bool finish_frag = false;
  int extra_space = 0;
  int slot;

  for (slot = 0; slot < vinsn->num_slots; slot++)
    {
      TInsn *tinsn = &vinsn->slots[slot];
      if (!tinsn_has_symbolic_operands (tinsn))
	{
	  /* A narrow instruction could be widened later to help
	     alignment issues.  */
	  if (xg_is_single_relaxable_insn (tinsn, 0, true)
	      && !tinsn->is_specific_opcode
	      && vinsn->num_slots == 1)
	    {
	      /* Difference in bytes between narrow and wide insns...  */
	      extra_space += 1;
	      tinsn->subtype = RELAX_NARROW;
	    }
	}
      else
	{
	  if (workaround_b_j_loop_end
	      && tinsn->opcode == xtensa_jx_opcode
	      && use_transform ())
	    {
	      /* Add 2 of these.  */
	      extra_space += 3; /* for the nop size */
	      tinsn->subtype = RELAX_ADD_NOP_IF_PRE_LOOP_END;
	    }

	  /* Need to assemble it with space for the relocation.  */
	  if (xg_is_relaxable_insn (tinsn, 0)
	      && !tinsn->is_specific_opcode)
	    {
	      int max_size = xg_get_max_insn_widen_size (tinsn->opcode);
	      int max_literal_size =
		xg_get_max_insn_widen_literal_size (tinsn->opcode);

	      tinsn->literal_space = max_literal_size;

	      tinsn->subtype = RELAX_IMMED;
	      extra_space += max_size;
	    }
	  else
	    {
	      /* A fix record will be added for this instruction prior
		 to relaxation, so make it end the frag.  */
	      finish_frag = true;
	    }
	}
    }
  *pfinish_frag = finish_frag;
  return extra_space;
}


static void
bundle_tinsn (TInsn *tinsn, vliw_insn *vinsn)
{
  xtensa_isa isa = xtensa_default_isa;
  int slot, chosen_slot;

  vinsn->format = xg_get_single_format (tinsn->opcode);
  gas_assert (vinsn->format != XTENSA_UNDEFINED);
  vinsn->num_slots = xtensa_format_num_slots (isa, vinsn->format);

  chosen_slot = xg_get_single_slot (tinsn->opcode);
  for (slot = 0; slot < vinsn->num_slots; slot++)
    {
      if (slot == chosen_slot)
	vinsn->slots[slot] = *tinsn;
      else
	{
	  vinsn->slots[slot].opcode =
	    xtensa_format_slot_nop_opcode (isa, vinsn->format, slot);
	  vinsn->slots[slot].ntok = 0;
	  vinsn->slots[slot].insn_type = ITYPE_INSN;
	}
    }
}


static bool
emit_single_op (TInsn *orig_insn)
{
  int i;
  IStack istack;		/* put instructions into here */
  symbolS *lit_sym = NULL;
  symbolS *label_sym = NULL;

  istack_init (&istack);

  /* Special-case for "movi aX, foo" which is guaranteed to need relaxing.
     Because the scheduling and bundling characteristics of movi and
     l32r or const16 are so different, we can do much better if we relax
     it prior to scheduling and bundling, rather than after.  */
  if ((orig_insn->opcode == xtensa_movi_opcode
       || orig_insn->opcode == xtensa_movi_n_opcode)
      && !cur_vinsn.inside_bundle
      && (orig_insn->tok[1].X_op == O_symbol
	  || orig_insn->tok[1].X_op == O_pltrel
	  || orig_insn->tok[1].X_op == O_tlsfunc
	  || orig_insn->tok[1].X_op == O_tlsarg
	  || orig_insn->tok[1].X_op == O_tpoff
	  || orig_insn->tok[1].X_op == O_dtpoff)
      && !orig_insn->is_specific_opcode && use_transform ())
    xg_assembly_relax (&istack, orig_insn, now_seg, frag_now, 0, 1, 0);
  else
    if (xg_expand_assembly_insn (&istack, orig_insn))
      return true;

  for (i = 0; i < istack.ninsn; i++)
    {
      TInsn *insn = &istack.insn[i];
      switch (insn->insn_type)
	{
	case ITYPE_LITERAL:
	  gas_assert (lit_sym == NULL);
	  lit_sym = xg_assemble_literal (insn);
	  break;
	case ITYPE_LABEL:
	  {
	    static int relaxed_sym_idx = 0;
	    char *label = XNEWVEC (char, strlen (FAKE_LABEL_NAME) + 12);
	    sprintf (label, "%s_rl_%x", FAKE_LABEL_NAME, relaxed_sym_idx++);
	    colon (label);
	    gas_assert (label_sym == NULL);
	    label_sym = symbol_find_or_make (label);
	    gas_assert (label_sym);
	    free (label);
	  }
	  break;
	case ITYPE_INSN:
	  {
	    vliw_insn v;
	    if (lit_sym)
	      xg_resolve_literals (insn, lit_sym);
	    if (label_sym)
	      xg_resolve_labels (insn, label_sym);
	    xg_init_vinsn (&v);
	    bundle_tinsn (insn, &v);
	    finish_vinsn (&v);
	    xg_free_vinsn (&v);
	  }
	  break;
	default:
	  gas_assert (0);
	  break;
	}
    }
  return false;
}


static int
total_frag_text_expansion (fragS *fragP)
{
  int slot;
  int total_expansion = 0;

  for (slot = 0; slot < config_max_slots; slot++)
    total_expansion += fragP->tc_frag_data.text_expansion[slot];

  return total_expansion;
}


/* Emit a vliw instruction to the current fragment.  */

static void
xg_assemble_vliw_tokens (vliw_insn *vinsn)
{
  bool finish_frag;
  bool is_jump = false;
  bool is_branch = false;
  xtensa_isa isa = xtensa_default_isa;
  int insn_size;
  int extra_space;
  char *f = NULL;
  int slot;
  struct dwarf2_line_info debug_line;
  bool loc_directive_seen = false;
  TInsn *tinsn;

  memset (&debug_line, 0, sizeof (struct dwarf2_line_info));

  if (generating_literals)
    {
      static int reported = 0;
      if (reported < 4)
	as_bad_where (frag_now->fr_file, frag_now->fr_line,
		      _("cannot assemble into a literal fragment"));
      if (reported == 3)
	as_bad (_("..."));
      reported++;
      return;
    }

  if (frag_now_fix () != 0
      && (! frag_now->tc_frag_data.is_insn
 	  || (vinsn_has_specific_opcodes (vinsn) && use_transform ())
 	  || (!use_transform ()) != frag_now->tc_frag_data.is_no_transform
 	  || (directive_state[directive_longcalls]
	      != frag_now->tc_frag_data.use_longcalls)
 	  || (directive_state[directive_absolute_literals]
	      != frag_now->tc_frag_data.use_absolute_literals)))
    {
      frag_wane (frag_now);
      frag_new (0);
      xtensa_set_frag_assembly_state (frag_now);
    }

  if (workaround_a0_b_retw
      && vinsn->num_slots == 1
      && (get_last_insn_flags (now_seg, now_subseg) & FLAG_IS_A0_WRITER) != 0
      && xtensa_opcode_is_branch (isa, vinsn->slots[0].opcode) == 1
      && use_transform ())
    {
      has_a0_b_retw = true;

      /* Mark this fragment with the special RELAX_ADD_NOP_IF_A0_B_RETW.
	 After the first assembly pass we will check all of them and
	 add a nop if needed.  */
      frag_now->tc_frag_data.is_insn = true;
      frag_var (rs_machine_dependent, 4, 4,
		RELAX_ADD_NOP_IF_A0_B_RETW,
		frag_now->fr_symbol,
		frag_now->fr_offset,
		NULL);
      xtensa_set_frag_assembly_state (frag_now);
      frag_now->tc_frag_data.is_insn = true;
      frag_var (rs_machine_dependent, 4, 4,
		RELAX_ADD_NOP_IF_A0_B_RETW,
		frag_now->fr_symbol,
		frag_now->fr_offset,
		NULL);
      xtensa_set_frag_assembly_state (frag_now);
    }

  for (slot = 0; slot < vinsn->num_slots; slot++)
    {
      tinsn = &vinsn->slots[slot];

      /* See if the instruction implies an aligned section.  */
      if (xtensa_opcode_is_loop (isa, tinsn->opcode) == 1)
	record_alignment (now_seg, 2);

      /* Determine the best line number for debug info.  */
      if ((tinsn->loc_directive_seen || !loc_directive_seen)
	  && (tinsn->debug_line.filenum != debug_line.filenum
	      || tinsn->debug_line.line < debug_line.line
	      || tinsn->debug_line.column < debug_line.column))
	debug_line = tinsn->debug_line;
      if (tinsn->loc_directive_seen)
	loc_directive_seen = true;
    }

  /* Special cases for instructions that force an alignment... */
  /* None of these opcodes are bundle-able.  */
  if (xtensa_opcode_is_loop (isa, vinsn->slots[0].opcode) == 1)
    {
      int max_fill;

      /* Remember the symbol that marks the end of the loop in the frag
	 that marks the start of the loop.  This way we can easily find
	 the end of the loop at the beginning, without adding special code
	 to mark the loop instructions themselves.  */
      symbolS *target_sym = NULL;
      if (vinsn->slots[0].tok[1].X_op == O_symbol)
	target_sym = vinsn->slots[0].tok[1].X_add_symbol;

      xtensa_set_frag_assembly_state (frag_now);
      frag_now->tc_frag_data.is_insn = true;

      max_fill = get_text_align_max_fill_size
	(get_text_align_power (xtensa_fetch_width),
	 true, frag_now->tc_frag_data.is_no_density);

      if (use_transform ())
	frag_var (rs_machine_dependent, max_fill, max_fill,
		  RELAX_ALIGN_NEXT_OPCODE, target_sym, 0, NULL);
      else
	frag_var (rs_machine_dependent, 0, 0,
		  RELAX_CHECK_ALIGN_NEXT_OPCODE, target_sym, 0, NULL);
      xtensa_set_frag_assembly_state (frag_now);
    }

  if (vinsn->slots[0].opcode == xtensa_entry_opcode
      && !vinsn->slots[0].is_specific_opcode)
    {
      xtensa_mark_literal_pool_location ();
      xtensa_move_labels (frag_now, 0);
      frag_var (rs_align_test, 1, 1, 0, NULL, 2, NULL);
    }

  if (vinsn->num_slots == 1)
    {
      if (workaround_a0_b_retw && use_transform ())
	set_last_insn_flags (now_seg, now_subseg, FLAG_IS_A0_WRITER,
			     is_register_writer (&vinsn->slots[0], "a", 0));

      set_last_insn_flags (now_seg, now_subseg, FLAG_IS_BAD_LOOPEND,
			   is_bad_loopend_opcode (&vinsn->slots[0]));
    }
  else
    set_last_insn_flags (now_seg, now_subseg, FLAG_IS_BAD_LOOPEND, false);

  insn_size = xtensa_format_length (isa, vinsn->format);

  extra_space = relaxation_requirements (vinsn, &finish_frag);

  /* vinsn_to_insnbuf will produce the error.  */
  if (vinsn->format != XTENSA_UNDEFINED)
    {
      f = frag_more (insn_size + extra_space);
      xtensa_set_frag_assembly_state (frag_now);
      frag_now->tc_frag_data.is_insn = true;
    }

  vinsn_to_insnbuf (vinsn, f, frag_now, false);
  if (vinsn->format == XTENSA_UNDEFINED)
    return;

  xtensa_insnbuf_to_chars (isa, vinsn->insnbuf, (unsigned char *) f, 0);

  if (debug_type == DEBUG_DWARF2 || loc_directive_seen)
    dwarf2_gen_line_info (frag_now_fix () - (insn_size + extra_space),
			  &debug_line);

  for (slot = 0; slot < vinsn->num_slots; slot++)
    {
      tinsn = &vinsn->slots[slot];
      frag_now->tc_frag_data.slot_subtypes[slot] = tinsn->subtype;
      frag_now->tc_frag_data.slot_symbols[slot] = tinsn->symbol;
      frag_now->tc_frag_data.slot_offsets[slot] = tinsn->offset;
      frag_now->tc_frag_data.literal_frags[slot] = tinsn->literal_frag;
      if (tinsn->opcode == xtensa_l32r_opcode)
	frag_now->tc_frag_data.literal_frags[slot]
	  = symbol_get_frag (tinsn->tok[1].X_add_symbol);
      if (tinsn->literal_space != 0)
	xg_assemble_literal_space (tinsn->literal_space, slot);
      frag_now->tc_frag_data.free_reg[slot] = tinsn->extra_arg;

      if (tinsn->subtype == RELAX_NARROW)
	gas_assert (vinsn->num_slots == 1);
      if (xtensa_opcode_is_jump (isa, tinsn->opcode) == 1)
	is_jump = true;
      if (xtensa_opcode_is_branch (isa, tinsn->opcode) == 1)
	is_branch = true;

      if (tinsn->subtype || tinsn->symbol || tinsn->offset
	  || tinsn->literal_frag || is_jump || is_branch)
	finish_frag = true;
    }

  if (vinsn_has_specific_opcodes (vinsn) && use_transform ())
    frag_now->tc_frag_data.is_specific_opcode = true;

  if (finish_frag)
    {
      frag_variant (rs_machine_dependent,
		    extra_space, extra_space, RELAX_SLOTS,
		    frag_now->fr_symbol, frag_now->fr_offset, f);
      xtensa_set_frag_assembly_state (frag_now);
    }

  /* Special cases for loops:
     close_loop_end should be inserted AFTER short_loop.
     Make sure that CLOSE loops are processed BEFORE short_loops
     when converting them.  */

  /* "short_loop": Add a NOP if the loop is < 4 bytes.  */
  if (xtensa_opcode_is_loop (isa, vinsn->slots[0].opcode) == 1
      && !vinsn->slots[0].is_specific_opcode)
    {
      if (workaround_short_loop && use_transform ())
	{
	  maybe_has_short_loop = true;
	  frag_now->tc_frag_data.is_insn = true;
	  frag_var (rs_machine_dependent, 4, 4,
		    RELAX_ADD_NOP_IF_SHORT_LOOP,
		    frag_now->fr_symbol, frag_now->fr_offset, NULL);
	  frag_now->tc_frag_data.is_insn = true;
	  frag_var (rs_machine_dependent, 4, 4,
		    RELAX_ADD_NOP_IF_SHORT_LOOP,
		    frag_now->fr_symbol, frag_now->fr_offset, NULL);
	}

      /* "close_loop_end": Add up to 12 bytes of NOPs to keep a
	 loop at least 12 bytes away from another loop's end.  */
      if (workaround_close_loop_end && use_transform ())
	{
	  maybe_has_close_loop_end = true;
	  frag_now->tc_frag_data.is_insn = true;
	  frag_var (rs_machine_dependent, 12, 12,
		    RELAX_ADD_NOP_IF_CLOSE_LOOP_END,
		    frag_now->fr_symbol, frag_now->fr_offset, NULL);
	}
    }

  if (use_transform ())
    {
      if (is_jump)
	{
	  gas_assert (finish_frag);
	  frag_var (rs_machine_dependent,
		    xtensa_fetch_width, xtensa_fetch_width,
		    RELAX_UNREACHABLE,
		    frag_now->fr_symbol, frag_now->fr_offset, NULL);
	  xtensa_set_frag_assembly_state (frag_now);
	  xtensa_maybe_create_trampoline_frag ();
	  /* Always create one here.  */
	  xtensa_maybe_create_literal_pool_frag (true, false);
	}
      else if (is_branch && do_align_targets ())
	{
	  gas_assert (finish_frag);
	  frag_var (rs_machine_dependent,
		    xtensa_fetch_width, xtensa_fetch_width,
		    RELAX_MAYBE_UNREACHABLE,
		    frag_now->fr_symbol, frag_now->fr_offset, NULL);
	  xtensa_set_frag_assembly_state (frag_now);
	  frag_var (rs_machine_dependent,
		    0, 0,
		    RELAX_MAYBE_DESIRE_ALIGN,
		    frag_now->fr_symbol, frag_now->fr_offset, NULL);
	  xtensa_set_frag_assembly_state (frag_now);
	}
    }

  /* Now, if the original opcode was a call...  */
  if (do_align_targets ()
      && xtensa_opcode_is_call (isa, vinsn->slots[0].opcode) == 1)
    {
      float freq = get_subseg_total_freq (now_seg, now_subseg);
      frag_now->tc_frag_data.is_insn = true;
      frag_var (rs_machine_dependent, 4, (int) freq, RELAX_DESIRE_ALIGN,
		frag_now->fr_symbol, frag_now->fr_offset, NULL);
      xtensa_set_frag_assembly_state (frag_now);
    }

  if (vinsn_has_specific_opcodes (vinsn) && use_transform ())
    {
      frag_wane (frag_now);
      frag_new (0);
      xtensa_set_frag_assembly_state (frag_now);
    }
}


/* xtensa_md_finish and helper functions.  */

static void xtensa_cleanup_align_frags (void);
static void xtensa_fix_target_frags (void);
static void xtensa_mark_narrow_branches (void);
static void xtensa_mark_zcl_first_insns (void);
static void xtensa_mark_difference_of_two_symbols (void);
static void xtensa_fix_a0_b_retw_frags (void);
static void xtensa_fix_b_j_loop_end_frags (void);
static void xtensa_fix_close_loop_end_frags (void);
static void xtensa_fix_short_loop_frags (void);
static void xtensa_sanity_check (void);
static void xtensa_add_config_info (void);

void
xtensa_md_finish (void)
{
  directive_balance ();
  xtensa_flush_pending_output ();

  past_xtensa_md_finish = true;

  xtensa_move_literals ();

  xtensa_reorder_segments ();
  xtensa_cleanup_align_frags ();
  xtensa_fix_target_frags ();
  if (workaround_a0_b_retw && has_a0_b_retw)
    xtensa_fix_a0_b_retw_frags ();
  if (workaround_b_j_loop_end)
    xtensa_fix_b_j_loop_end_frags ();

  /* "close_loop_end" should be processed BEFORE "short_loop".  */
  if (workaround_close_loop_end && maybe_has_close_loop_end)
    xtensa_fix_close_loop_end_frags ();

  if (workaround_short_loop && maybe_has_short_loop)
    xtensa_fix_short_loop_frags ();
  if (align_targets)
    xtensa_mark_narrow_branches ();
  xtensa_mark_zcl_first_insns ();

  xtensa_sanity_check ();

  xtensa_add_config_info ();

  xtensa_check_frag_count ();
}

struct trampoline_chain_entry
{
  symbolS *sym;
  addressT offset;
};

/* Trampoline chain for a given (sym, offset) pair is a sorted array
   of locations of trampoline jumps leading there.  Jumps are represented
   as pairs (sym, offset): trampoline frag symbol and offset of the jump
   inside the frag.  */
struct trampoline_chain
{
  struct trampoline_chain_entry target;
  struct trampoline_chain_entry *entry;
  size_t n_entries;
  size_t n_max;
  bool needs_sorting;
};

struct trampoline_chain_index
{
  struct trampoline_chain *entry;
  size_t n_entries;
  size_t n_max;
  bool needs_sorting;
};

struct trampoline_index
{
  fragS **entry;
  size_t n_entries;
  size_t n_max;
};

struct trampoline_seg
{
  struct trampoline_seg *next;
  asection *seg;
  /* Trampolines ordered by their frag fr_address */
  struct trampoline_index index;
  /* Known trampoline chains ordered by (sym, offset) pair */
  struct trampoline_chain_index chain_index;
};

static struct trampoline_seg trampoline_seg_list;
#define J_RANGE (128 * 1024)
#define J_MARGIN 4096

static int unreachable_count = 0;


static void
xtensa_maybe_create_trampoline_frag (void)
{
  if (!use_trampolines)
    return;

  /* We create an area for possible trampolines every 10 unreachable frags.
     These are preferred over the ones not preceded by an unreachable frag,
     because we don't have to jump around them. This function is called after
     each RELAX_UNREACHABLE frag is created.  */

  if (++unreachable_count > 10)
    {
      xtensa_create_trampoline_frag (false);
      clear_frag_count ();
      unreachable_count = 0;
    }
}

static void
xtensa_check_frag_count (void)
{
  if (!use_trampolines || frag_now->tc_frag_data.is_no_transform)
    return;

  /* We create an area for possible trampolines every 8000 frags or so. This
     is an estimate based on the max range of a "j" insn (+/-128K) divided
     by a typical frag byte count (16), minus a few for safety. This function
     is called after each source line is processed.  */

  if (get_frag_count () > 8000)
    {
      xtensa_create_trampoline_frag (true);
      clear_frag_count ();
      unreachable_count = 0;
    }

  /* We create an area for a possible literal pool every N (default 5000)
     frags or so.  */
  xtensa_maybe_create_literal_pool_frag (true, true);
}

static xtensa_insnbuf trampoline_buf = NULL;
static xtensa_insnbuf trampoline_slotbuf = NULL;

static xtensa_insnbuf litpool_buf = NULL;
static xtensa_insnbuf litpool_slotbuf = NULL;

#define TRAMPOLINE_FRAG_SIZE 3000

static struct trampoline_seg *
find_trampoline_seg (asection *seg)
{
  struct trampoline_seg *ts = trampoline_seg_list.next;
  static struct trampoline_seg *mr;

  if (mr && mr->seg == seg)
    return mr;

  for ( ; ts; ts = ts->next)
    {
      if (ts->seg == seg)
	{
	  mr = ts;
	  return ts;
	}
    }

  return NULL;
}

static size_t xg_find_trampoline (const struct trampoline_index *idx,
				  addressT addr)
{
  size_t a = 0;
  size_t b = idx->n_entries;

  while (b - a > 1)
    {
      size_t c = (a + b) / 2;

      if (idx->entry[c]->fr_address <= addr)
	a = c;
      else
	b = c;
    }
  return a;
}

static void xg_add_trampoline_to_index (struct trampoline_index *idx,
					fragS *fragP)
{
  if (idx->n_entries == idx->n_max)
    {
      idx->n_max = (idx->n_entries + 1) * 2;
      idx->entry = xrealloc (idx->entry,
			     sizeof (*idx->entry) * idx->n_max);
    }
  idx->entry[idx->n_entries] = fragP;
  ++idx->n_entries;
}

static void xg_remove_trampoline_from_index (struct trampoline_index *idx,
					     size_t i)
{
  gas_assert (i < idx->n_entries);
  memmove (idx->entry + i, idx->entry + i + 1,
	   (idx->n_entries - i - 1) * sizeof (*idx->entry));
  --idx->n_entries;
}

static void xg_add_trampoline_to_seg (struct trampoline_seg *ts,
				      fragS *fragP)
{
  xg_add_trampoline_to_index (&ts->index, fragP);
}

static void
xtensa_create_trampoline_frag (bool needs_jump_around)
{
  /* Emit a frag where we can place intermediate jump instructions,
     in case we need to jump farther than 128K bytes.
     Each jump instruction takes three bytes.
     We allocate enough for 1000 trampolines in each frag.
     If that's not enough, oh well.  */

  struct trampoline_seg *ts = find_trampoline_seg (now_seg);
  char *varP;
  fragS *fragP;
  int size = TRAMPOLINE_FRAG_SIZE;

  if (ts == NULL)
    {
      ts = XCNEW(struct trampoline_seg);
      ts->next = trampoline_seg_list.next;
      trampoline_seg_list.next = ts;
      ts->seg = now_seg;
    }

  frag_wane (frag_now);
  frag_new (0);
  xtensa_set_frag_assembly_state (frag_now);
  varP = frag_var (rs_machine_dependent, size, size, RELAX_TRAMPOLINE, NULL, 0, NULL);
  fragP = (fragS *)(varP - SIZEOF_STRUCT_FRAG);
  if (trampoline_buf == NULL)
    {
      trampoline_buf = xtensa_insnbuf_alloc (xtensa_default_isa);
      trampoline_slotbuf = xtensa_insnbuf_alloc (xtensa_default_isa);
    }
  fragP->tc_frag_data.needs_jump_around = needs_jump_around;
  xg_add_trampoline_to_seg (ts, fragP);
}

static bool xg_is_trampoline_frag_full (const fragS *fragP)
{
  return fragP->fr_var < 3;
}

static int xg_order_trampoline_chain_entry (const void *a, const void *b)
{
  const struct trampoline_chain_entry *pa = a;
  const struct trampoline_chain_entry *pb = b;

  if (pa->sym != pb->sym)
    {
      valueT aval = S_GET_VALUE (pa->sym);
      valueT bval = S_GET_VALUE (pb->sym);

      if (aval != bval)
	return aval < bval ? -1 : 1;
    }
  if (pa->offset != pb->offset)
    return pa->offset < pb->offset ? -1 : 1;
  return 0;
}

static void xg_sort_trampoline_chain (struct trampoline_chain *tc)
{
  qsort (tc->entry, tc->n_entries, sizeof (*tc->entry),
	 xg_order_trampoline_chain_entry);
  tc->needs_sorting = false;
}

/* Find entry index in the given chain with maximal address <= source.  */
static size_t xg_find_chain_entry (struct trampoline_chain *tc,
				   addressT source)
{
  size_t a = 0;
  size_t b = tc->n_entries;

  if (tc->needs_sorting)
    xg_sort_trampoline_chain (tc);

  while (b - a > 1)
    {
      size_t c = (a + b) / 2;
      struct trampoline_chain_entry *e = tc->entry + c;

      if (S_GET_VALUE(e->sym) + e->offset <= source)
	a = c;
      else
	b = c;
    }
  return a;
}

/* Find the best jump target for the source in the given trampoline chain.
   The best jump target is the one that results in the shortest path to the
   final target, it's the location of the jump closest to the final target,
   but within the J_RANGE - J_MARGIN from the source.  */
static struct trampoline_chain_entry *
xg_get_best_chain_entry (struct trampoline_chain *tc, addressT source)
{
  addressT target = S_GET_VALUE(tc->target.sym) + tc->target.offset;
  size_t i = xg_find_chain_entry (tc, source);
  struct trampoline_chain_entry *e = tc->entry + i;
  int step = target < source ? -1 : 1;
  addressT chained_target;
  offsetT off;

  if (target > source &&
      S_GET_VALUE(e->sym) + e->offset <= source &&
      i + 1 < tc->n_entries)
    ++i;

  while (i + step < tc->n_entries)
    {
      struct trampoline_chain_entry *next = tc->entry + i + step;

      chained_target = S_GET_VALUE(next->sym) + next->offset;
      off = source - chained_target;

      if (labs (off) >= J_RANGE - J_MARGIN)
	break;

      i += step;
    }

  e = tc->entry + i;
  chained_target = S_GET_VALUE(e->sym) + e->offset;
  off = source - chained_target;

  if (labs (off) < J_MARGIN ||
      labs (off) >= J_RANGE - J_MARGIN)
    return &tc->target;
  return tc->entry + i;
}

static int xg_order_trampoline_chain (const void *a, const void *b)
{
  const struct trampoline_chain *_pa = a;
  const struct trampoline_chain *_pb = b;
  const struct trampoline_chain_entry *pa = &_pa->target;
  const struct trampoline_chain_entry *pb = &_pb->target;
  symbolS *s1 = pa->sym;
  symbolS *s2 = pb->sym;

  if (s1 != s2)
    {
      symbolS *tmp = symbol_symbolS (s1);
      if (tmp)
	s1 = tmp;

      tmp = symbol_symbolS (s2);
      if (tmp)
	s2 = tmp;

      if (s1 != s2)
	return s1 < s2 ? -1 : 1;
    }

  if (pa->offset != pb->offset)
    return pa->offset < pb->offset ? -1 : 1;
  return 0;
}

static struct trampoline_chain *
xg_get_trampoline_chain (struct trampoline_seg *ts,
			 symbolS *sym,
			 addressT offset)
{
  struct trampoline_chain_index *idx = &ts->chain_index;
  struct trampoline_chain c;

  if (idx->n_entries == 0)
    return NULL;

  if (idx->needs_sorting)
    {
      qsort (idx->entry, idx->n_entries, sizeof (*idx->entry),
	     xg_order_trampoline_chain);
      idx->needs_sorting = false;
    }
  c.target.sym = sym;
  c.target.offset = offset;
  return bsearch (&c, idx->entry, idx->n_entries,
		  sizeof (struct trampoline_chain),
		  xg_order_trampoline_chain);
}

/* Find trampoline chain in the given trampoline segment that is going
   to the *sym + *offset.  If found, replace *sym and *offset with the
   best jump target in that chain.  */
static struct trampoline_chain *
xg_find_best_eq_target (struct trampoline_seg *ts,
			addressT source, symbolS **sym,
			addressT *offset)
{
  struct trampoline_chain *tc = xg_get_trampoline_chain (ts, *sym, *offset);

  if (tc)
    {
      struct trampoline_chain_entry *e = xg_get_best_chain_entry (tc, source);

      *sym = e->sym;
      *offset = e->offset;
    }
  return tc;
}

static void xg_add_location_to_chain (struct trampoline_chain *tc,
				      symbolS *sym, addressT offset)
{
  struct trampoline_chain_entry *e;

  if (tc->n_entries == tc->n_max)
    {
      tc->n_max = (tc->n_max + 1) * 2;
      tc->entry = xrealloc (tc->entry, sizeof (*tc->entry) * tc->n_max);
    }
  e = tc->entry + tc->n_entries;
  e->sym = sym;
  e->offset = offset;
  ++tc->n_entries;
  tc->needs_sorting = true;
}

static struct trampoline_chain *
xg_create_trampoline_chain (struct trampoline_seg *ts,
			    symbolS *sym, addressT offset)
{
  struct trampoline_chain_index *idx = &ts->chain_index;
  struct trampoline_chain *tc;

  if (idx->n_entries == idx->n_max)
    {
      idx->n_max = (idx->n_max + 1) * 2;
      idx->entry = xrealloc (idx->entry,
			     sizeof (*idx->entry) * idx->n_max);
    }

  tc = idx->entry + idx->n_entries;
  tc->target.sym = sym;
  tc->target.offset = offset;
  tc->entry = NULL;
  tc->n_entries = 0;
  tc->n_max = 0;
  xg_add_location_to_chain (tc, sym, offset);

  ++idx->n_entries;
  idx->needs_sorting = true;

  return tc;
}

void dump_trampolines (void);

void
dump_trampolines (void)
{
  struct trampoline_seg *ts = trampoline_seg_list.next;

  for ( ; ts; ts = ts->next)
    {
      size_t i;
      asection *seg = ts->seg;

      if (seg == NULL)
	continue;
      fprintf(stderr, "SECTION %s\n", seg->name);

      for (i = 0; i < ts->index.n_entries; ++i)
	{
	  fragS *tf = ts->index.entry[i];

	  fprintf(stderr, "   0x%08x: fix=%d, jump_around=%s\n",
		  (int)tf->fr_address, (int)tf->fr_fix,
		  tf->tc_frag_data.needs_jump_around ? "T" : "F");
	}
    }
}

static void dump_litpools (void) __attribute__ ((unused));

static void
dump_litpools (void)
{
  struct litpool_seg *lps = litpool_seg_list.next;
  struct litpool_frag *lpf;

  for ( ; lps ; lps = lps->next )
    {
      printf("litpool seg %s\n", lps->seg->name);
      for ( lpf = lps->frag_list.next; lpf->fragP; lpf = lpf->next )
	{
	  fragS *litfrag = lpf->fragP->fr_next;
	  int count = 0;
	  while (litfrag && litfrag->fr_subtype != RELAX_LITERAL_POOL_END)
	    {
	      if (litfrag->fr_fix == 4)
		count++;
	      litfrag = litfrag->fr_next;
	    }
	  printf("   %ld <%d:%d> (%d) [%d]: ",
		 lpf->addr, lpf->priority, lpf->original_priority,
		 lpf->fragP->fr_line, count);
	  /* dump_frag(lpf->fragP);  */
	}
    }
}

static void
xtensa_maybe_create_literal_pool_frag (bool create, bool only_if_needed)
{
  struct litpool_seg *lps = litpool_seg_list.next;
  fragS *fragP;
  struct litpool_frag *lpf;
  bool needed = false;

  if (use_literal_section || !auto_litpools)
    return;

  for ( ; lps ; lps = lps->next )
    {
      if (lps->seg == now_seg)
	break;
    }

  if (lps == NULL)
    {
      lps = XCNEW (struct litpool_seg);
      lps->next = litpool_seg_list.next;
      litpool_seg_list.next = lps;
      lps->seg = now_seg;
      lps->frag_list.next = &lps->frag_list;
      lps->frag_list.prev = &lps->frag_list;
      /* Put candidate literal pool at the beginning of every section,
         so that even when section starts with literal load there's a
	 literal pool available.  */
      lps->frag_count = auto_litpool_limit;
    }

  lps->frag_count++;

  if (create)
    {
      if (only_if_needed)
	{
	  if (past_xtensa_md_finish || !use_transform() ||
	      frag_now->tc_frag_data.is_no_transform)
	    {
	      return;
	    }
	  if (auto_litpool_limit <= 0)
	    {
	      /* Don't create a litpool based only on frag count.  */
	      return;
	    }
	  else if (lps->frag_count > auto_litpool_limit)
	    {
	      needed = true;
	    }
	  else
	    {
	      return;
	    }
	}
      else
	{
	  needed = true;
	}
    }

  if (needed)
    {
      int size = (only_if_needed) ? 3 : 0; /* Space for a "j" insn.  */
      /* Create a potential site for a literal pool.  */
      frag_wane (frag_now);
      frag_new (0);
      xtensa_set_frag_assembly_state (frag_now);
      fragP = frag_now;
      fragP->tc_frag_data.lit_frchain = frchain_now;
      fragP->tc_frag_data.literal_frag = fragP;
      frag_var (rs_machine_dependent, size, size,
		    (only_if_needed) ?
		        RELAX_LITERAL_POOL_CANDIDATE_BEGIN :
		        RELAX_LITERAL_POOL_BEGIN,
		    NULL, 0, NULL);
      frag_now->tc_frag_data.lit_seg = now_seg;
      frag_variant (rs_machine_dependent, 0, 0,
		    RELAX_LITERAL_POOL_END, NULL, 0, NULL);
      xtensa_set_frag_assembly_state (frag_now);
    }
  else
    {
      /* RELAX_LITERAL_POOL_BEGIN frag is being created;
	 just record it here.  */
      fragP = frag_now;
    }

  lpf = XNEW (struct litpool_frag);
  /* Insert at tail of circular list.  */
  lpf->addr = 0;
  lps->frag_list.prev->next = lpf;
  lpf->next = &lps->frag_list;
  lpf->prev = lps->frag_list.prev;
  lps->frag_list.prev = lpf;
  lpf->fragP = fragP;
  lpf->priority = (needed) ? (only_if_needed) ? 3 : 2 : 1;
  lpf->original_priority = lpf->priority;
  lpf->literal_count = 0;

  lps->frag_count = 0;
}

static void
xtensa_cleanup_align_frags (void)
{
  frchainS *frchP;
  asection *s;

  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;
	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if ((fragP->fr_type == rs_align
		 || fragP->fr_type == rs_align_code
		 || (fragP->fr_type == rs_machine_dependent
		     && (fragP->fr_subtype == RELAX_DESIRE_ALIGN
			 || fragP->fr_subtype == RELAX_DESIRE_ALIGN_IF_TARGET)))
		&& fragP->fr_fix == 0)
	      {
		fragS *next = fragP->fr_next;

		while (next
		       && next->fr_fix == 0
		       && next->fr_type == rs_machine_dependent
		       && next->fr_subtype == RELAX_DESIRE_ALIGN_IF_TARGET)
		  {
		    frag_wane (next);
		    next = next->fr_next;
		  }
	      }
	    /* If we don't widen branch targets, then they
	       will be easier to align.  */
	    if (fragP->tc_frag_data.is_branch_target
		&& fragP->fr_opcode == fragP->fr_literal
		&& fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_SLOTS
		&& fragP->tc_frag_data.slot_subtypes[0] == RELAX_NARROW)
	      frag_wane (fragP);
	    if (fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_UNREACHABLE)
	      fragP->tc_frag_data.is_unreachable = true;
	  }
      }
}


/* Re-process all of the fragments looking to convert all of the
   RELAX_DESIRE_ALIGN_IF_TARGET fragments.  If there is a branch
   target in the next fragment, convert this to RELAX_DESIRE_ALIGN.
   Otherwise, convert to a .fill 0.  */

static void
xtensa_fix_target_frags (void)
{
  frchainS *frchP;
  asection *s;

  /* When this routine is called, all of the subsections are still intact
     so we walk over subsections instead of sections.  */
  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;

	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if (fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_DESIRE_ALIGN_IF_TARGET)
	      {
		if (next_frag_is_branch_target (fragP))
		  fragP->fr_subtype = RELAX_DESIRE_ALIGN;
		else
		  frag_wane (fragP);
	      }
	  }
      }
}


static bool is_narrow_branch_guaranteed_in_range (fragS *, TInsn *);

static void
xtensa_mark_narrow_branches (void)
{
  frchainS *frchP;
  asection *s;

  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;
	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if (fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_SLOTS
		&& fragP->tc_frag_data.slot_subtypes[0] == RELAX_IMMED)
	      {
		vliw_insn vinsn;

		vinsn_from_chars (&vinsn, fragP->fr_opcode);
		tinsn_immed_from_frag (&vinsn.slots[0], fragP, 0);

		if (vinsn.num_slots == 1
		    && xtensa_opcode_is_branch (xtensa_default_isa,
						vinsn.slots[0].opcode) == 1
		    && xg_get_single_size (vinsn.slots[0].opcode) == 2
		    && is_narrow_branch_guaranteed_in_range (fragP,
							     &vinsn.slots[0]))
		  {
		    fragP->fr_subtype = RELAX_SLOTS;
		    fragP->tc_frag_data.slot_subtypes[0] = RELAX_NARROW;
		    fragP->tc_frag_data.is_aligning_branch = 1;
		  }
	      }
	  }
      }
}


/* A branch is typically widened only when its target is out of
   range.  However, we would like to widen them to align a subsequent
   branch target when possible.

   Because the branch relaxation code is so convoluted, the optimal solution
   (combining the two cases) is difficult to get right in all circumstances.
   We therefore go with an "almost as good" solution, where we only
   use for alignment narrow branches that definitely will not expand to a
   jump and a branch.  These functions find and mark these cases.  */

/* The range in bytes of BNEZ.N and BEQZ.N.  The target operand is encoded
   as PC + 4 + imm6, where imm6 is a 6-bit immediate ranging from 0 to 63.
   We start counting beginning with the frag after the 2-byte branch, so the
   maximum offset is (4 - 2) + 63 = 65.  */
#define MAX_IMMED6 65

static offsetT unrelaxed_frag_max_size (fragS *);

static bool
is_narrow_branch_guaranteed_in_range (fragS *fragP, TInsn *tinsn)
{
  const expressionS *exp = &tinsn->tok[1];
  symbolS *symbolP = exp->X_add_symbol;
  offsetT max_distance = exp->X_add_number;
  fragS *target_frag;

  if (exp->X_op != O_symbol)
    return false;

  target_frag = symbol_get_frag (symbolP);

  max_distance += (S_GET_VALUE (symbolP) - target_frag->fr_address);
  if (is_branch_jmp_to_next (tinsn, fragP))
    return false;

  /* The branch doesn't branch over it's own frag,
     but over the subsequent ones.  */
  fragP = fragP->fr_next;
  while (fragP != NULL && fragP != target_frag && max_distance <= MAX_IMMED6)
    {
      max_distance += unrelaxed_frag_max_size (fragP);
      fragP = fragP->fr_next;
    }
  if (max_distance <= MAX_IMMED6 && fragP == target_frag)
    return true;
  return false;
}


static void
xtensa_mark_zcl_first_insns (void)
{
  frchainS *frchP;
  asection *s;

  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;
	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if (fragP->fr_type == rs_machine_dependent
		&& (fragP->fr_subtype == RELAX_ALIGN_NEXT_OPCODE
		    || fragP->fr_subtype == RELAX_CHECK_ALIGN_NEXT_OPCODE))
	      {
		/* Find the loop frag.  */
		fragS *loop_frag = next_non_empty_frag (fragP);
		/* Find the first insn frag.  */
		fragS *targ_frag = next_non_empty_frag (loop_frag);

	      /* Handle a corner case that comes up in hardware
		 diagnostics.  The original assembly looks like this:

		 loop aX, LabelA
		 <empty_frag>--not found by next_non_empty_frag
		 loop aY, LabelB

		 Depending on the start address, the assembler may or
		 may not change it to look something like this:

		 loop aX, LabelA
		 nop--frag isn't empty anymore
		 loop aY, LabelB

		 So set up to check the alignment of the nop if it
		 exists  */
		while (loop_frag != targ_frag)
		  {
		    if (loop_frag->fr_type == rs_machine_dependent
			&& (loop_frag->fr_subtype == RELAX_ALIGN_NEXT_OPCODE
			    || loop_frag->fr_subtype
			    == RELAX_CHECK_ALIGN_NEXT_OPCODE))
		      targ_frag = loop_frag;
		    else
		      loop_frag = loop_frag->fr_next;
		  }

		/* Of course, sometimes (mostly for toy test cases) a
		   zero-cost loop instruction is the last in a section.  */
		if (targ_frag)
		  {
		    targ_frag->tc_frag_data.is_first_loop_insn = true;
		    /* Do not widen a frag that is the first instruction of a
		       zero-cost loop.  It makes that loop harder to align.  */
		    if (targ_frag->fr_type == rs_machine_dependent
			&& targ_frag->fr_subtype == RELAX_SLOTS
			&& (targ_frag->tc_frag_data.slot_subtypes[0]
			    == RELAX_NARROW))
		      {
			if (targ_frag->tc_frag_data.is_aligning_branch)
			  targ_frag->tc_frag_data.slot_subtypes[0] = RELAX_IMMED;
			else
			  {
			    frag_wane (targ_frag);
			    targ_frag->tc_frag_data.slot_subtypes[0] = 0;
			  }
		      }
		  }
		if (fragP->fr_subtype == RELAX_CHECK_ALIGN_NEXT_OPCODE)
		  frag_wane (fragP);
	      }
	  }
      }
}


/* When a difference-of-symbols expression is encoded as a uleb128 or
   sleb128 value, the linker is unable to adjust that value to account for
   link-time relaxation.  Mark all the code between such symbols so that
   its size cannot be changed by linker relaxation.  */

static void
xtensa_mark_difference_of_two_symbols (void)
{
  symbolS *expr_sym;

  for (expr_sym = expr_symbols; expr_sym;
       expr_sym = symbol_get_tc (expr_sym)->next_expr_symbol)
    {
      expressionS *exp = symbol_get_value_expression (expr_sym);

      if (exp->X_op == O_subtract)
	{
	  symbolS *left = exp->X_add_symbol;
	  symbolS *right = exp->X_op_symbol;

	  /* Difference of two symbols not in the same section
	     are handled with relocations in the linker.  */
	  if (S_GET_SEGMENT (left) == S_GET_SEGMENT (right))
	    {
	      fragS *start;
	      fragS *end;
	      fragS *walk;

	      if (symbol_get_frag (left)->fr_address
		  <= symbol_get_frag (right)->fr_address)
		{
		  start = symbol_get_frag (left);
		  end = symbol_get_frag (right);
		}
	      else
		{
		  start = symbol_get_frag (right);
		  end = symbol_get_frag (left);
		}

	      if (start->tc_frag_data.no_transform_end != NULL)
		walk = start->tc_frag_data.no_transform_end;
	      else
		walk = start;
	      do
		{
		  walk->tc_frag_data.is_no_transform = 1;
		  walk = walk->fr_next;
		}
	      while (walk && walk->fr_address < end->fr_address);

	      start->tc_frag_data.no_transform_end = walk;
	    }
	}
    }
}


/* Re-process all of the fragments looking to convert all of the
   RELAX_ADD_NOP_IF_A0_B_RETW.  If the next instruction is a
   conditional branch or a retw/retw.n, convert this frag to one that
   will generate a NOP.  In any case close it off with a .fill 0.  */

static bool next_instrs_are_b_retw (fragS *);

static void
xtensa_fix_a0_b_retw_frags (void)
{
  frchainS *frchP;
  asection *s;

  /* When this routine is called, all of the subsections are still intact
     so we walk over subsections instead of sections.  */
  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;

	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if (fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_ADD_NOP_IF_A0_B_RETW)
	      {
		if (next_instrs_are_b_retw (fragP))
		  {
		    if (fragP->tc_frag_data.is_no_transform)
		      as_bad (_("instruction sequence (write a0, branch, retw) may trigger hardware errata"));
		    else
		      relax_frag_add_nop (fragP);
		  }
		frag_wane (fragP);
	      }
	  }
      }
}


static bool
next_instrs_are_b_retw (fragS *fragP)
{
  xtensa_opcode opcode;
  xtensa_format fmt;
  const fragS *next_fragP = next_non_empty_frag (fragP);
  static xtensa_insnbuf insnbuf = NULL;
  static xtensa_insnbuf slotbuf = NULL;
  xtensa_isa isa = xtensa_default_isa;
  unsigned int offset = 0;
  int slot;
  bool branch_seen = false;

  if (!insnbuf)
    {
      insnbuf = xtensa_insnbuf_alloc (isa);
      slotbuf = xtensa_insnbuf_alloc (isa);
    }

  if (next_fragP == NULL)
    return false;

  /* Check for the conditional branch.  */
  xtensa_insnbuf_from_chars
    (isa, insnbuf, (unsigned char *) &next_fragP->fr_literal[offset], 0);
  fmt = xtensa_format_decode (isa, insnbuf);
  if (fmt == XTENSA_UNDEFINED)
    return false;

  for (slot = 0; slot < xtensa_format_num_slots (isa, fmt); slot++)
    {
      xtensa_format_get_slot (isa, fmt, slot, insnbuf, slotbuf);
      opcode = xtensa_opcode_decode (isa, fmt, slot, slotbuf);

      branch_seen = (branch_seen
		     || xtensa_opcode_is_branch (isa, opcode) == 1);
    }

  if (!branch_seen)
    return false;

  offset += xtensa_format_length (isa, fmt);
  if (offset == next_fragP->fr_fix)
    {
      next_fragP = next_non_empty_frag (next_fragP);
      offset = 0;
    }

  if (next_fragP == NULL)
    return false;

  /* Check for the retw/retw.n.  */
  xtensa_insnbuf_from_chars
    (isa, insnbuf, (unsigned char *) &next_fragP->fr_literal[offset], 0);
  fmt = xtensa_format_decode (isa, insnbuf);

  /* Because RETW[.N] is not bundleable, a VLIW bundle here means that we
     have no problems.  */
  if (fmt == XTENSA_UNDEFINED
      || xtensa_format_num_slots (isa, fmt) != 1)
    return false;

  xtensa_format_get_slot (isa, fmt, 0, insnbuf, slotbuf);
  opcode = xtensa_opcode_decode (isa, fmt, 0, slotbuf);

  if (opcode == xtensa_retw_opcode || opcode == xtensa_retw_n_opcode)
    return true;

  return false;
}


/* Re-process all of the fragments looking to convert all of the
   RELAX_ADD_NOP_IF_PRE_LOOP_END.  If there is one instruction and a
   loop end label, convert this frag to one that will generate a NOP.
   In any case close it off with a .fill 0.  */

static bool next_instr_is_loop_end (fragS *);

static void
xtensa_fix_b_j_loop_end_frags (void)
{
  frchainS *frchP;
  asection *s;

  /* When this routine is called, all of the subsections are still intact
     so we walk over subsections instead of sections.  */
  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;

	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if (fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_ADD_NOP_IF_PRE_LOOP_END)
	      {
		if (next_instr_is_loop_end (fragP))
		  {
		    if (fragP->tc_frag_data.is_no_transform)
		      as_bad (_("branching or jumping to a loop end may trigger hardware errata"));
		    else
		      relax_frag_add_nop (fragP);
		  }
		frag_wane (fragP);
	      }
	  }
      }
}


static bool
next_instr_is_loop_end (fragS *fragP)
{
  const fragS *next_fragP;

  if (next_frag_is_loop_target (fragP))
    return false;

  next_fragP = next_non_empty_frag (fragP);
  if (next_fragP == NULL)
    return false;

  if (!next_frag_is_loop_target (next_fragP))
    return false;

  /* If the size is >= 3 then there is more than one instruction here.
     The hardware bug will not fire.  */
  if (next_fragP->fr_fix > 3)
    return false;

  return true;
}


/* Re-process all of the fragments looking to convert all of the
   RELAX_ADD_NOP_IF_CLOSE_LOOP_END.  If there is an loop end that is
   not MY loop's loop end within 12 bytes, add enough nops here to
   make it at least 12 bytes away.  In any case close it off with a
   .fill 0.  */

static offsetT min_bytes_to_other_loop_end
  (fragS *, fragS *, offsetT);

static void
xtensa_fix_close_loop_end_frags (void)
{
  frchainS *frchP;
  asection *s;

  /* When this routine is called, all of the subsections are still intact
     so we walk over subsections instead of sections.  */
  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;

	fragS *current_target = NULL;

	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if (fragP->fr_type == rs_machine_dependent
		&& ((fragP->fr_subtype == RELAX_ALIGN_NEXT_OPCODE)
		    || (fragP->fr_subtype == RELAX_CHECK_ALIGN_NEXT_OPCODE)))
	      current_target = symbol_get_frag (fragP->fr_symbol);

	    if (current_target
		&& fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_ADD_NOP_IF_CLOSE_LOOP_END)
	      {
		offsetT min_bytes;
		int bytes_added = 0;

#define REQUIRED_LOOP_DIVIDING_BYTES 12
		/* Max out at 12.  */
		min_bytes = min_bytes_to_other_loop_end
		  (fragP->fr_next, current_target, REQUIRED_LOOP_DIVIDING_BYTES);

		if (min_bytes < REQUIRED_LOOP_DIVIDING_BYTES)
		  {
		    if (fragP->tc_frag_data.is_no_transform)
		      as_bad (_("loop end too close to another loop end may trigger hardware errata"));
		    else
		      {
			while (min_bytes + bytes_added
			       < REQUIRED_LOOP_DIVIDING_BYTES)
			  {
			    int length = 3;

			    if (fragP->fr_var < length)
			      as_fatal (_("fr_var %lu < length %d"),
					(long) fragP->fr_var, length);
			    else
			      {
				assemble_nop (length,
					      fragP->fr_literal + fragP->fr_fix);
				fragP->fr_fix += length;
				fragP->fr_var -= length;
			      }
			    bytes_added += length;
			  }
		      }
		  }
		frag_wane (fragP);
	      }
	    gas_assert (fragP->fr_type != rs_machine_dependent
		    || fragP->fr_subtype != RELAX_ADD_NOP_IF_CLOSE_LOOP_END);
	  }
      }
}


static offsetT unrelaxed_frag_min_size (fragS *);

static offsetT
min_bytes_to_other_loop_end (fragS *fragP,
			     fragS *current_target,
			     offsetT max_size)
{
  offsetT offset = 0;
  fragS *current_fragP;

  for (current_fragP = fragP;
       current_fragP;
       current_fragP = current_fragP->fr_next)
    {
      if (current_fragP->tc_frag_data.is_loop_target
	  && current_fragP != current_target)
	return offset;

      offset += unrelaxed_frag_min_size (current_fragP);

      if (offset >= max_size)
	return max_size;
    }
  return max_size;
}


static offsetT
unrelaxed_frag_min_size (fragS *fragP)
{
  offsetT size = fragP->fr_fix;

  /* Add fill size.  */
  if (fragP->fr_type == rs_fill)
    size += fragP->fr_offset;

  return size;
}


static offsetT
unrelaxed_frag_max_size (fragS *fragP)
{
  offsetT size = fragP->fr_fix;
  switch (fragP->fr_type)
    {
    case 0:
      /* Empty frags created by the obstack allocation scheme
	 end up with type 0.  */
      break;
    case rs_fill:
    case rs_org:
    case rs_space:
      size += fragP->fr_offset;
      break;
    case rs_align:
    case rs_align_code:
    case rs_align_test:
    case rs_leb128:
    case rs_cfa:
    case rs_dwarf2dbg:
    case rs_sframe:
      /* No further adjustments needed.  */
      break;
    case rs_machine_dependent:
      if (fragP->fr_subtype != RELAX_DESIRE_ALIGN)
	size += fragP->fr_var;
      break;
    default:
      /* We had darn well better know how big it is.  */
      gas_assert (0);
      break;
    }

  return size;
}


/* Re-process all of the fragments looking to convert all
   of the RELAX_ADD_NOP_IF_SHORT_LOOP.  If:

   A)
     1) the instruction size count to the loop end label
        is too short (<= 2 instructions),
     2) loop has a jump or branch in it

   or B)
     1) workaround_all_short_loops is TRUE
     2) The generating loop was a  'loopgtz' or 'loopnez'
     3) the instruction size count to the loop end label is too short
        (<= 2 instructions)
   then convert this frag (and maybe the next one) to generate a NOP.
   In any case close it off with a .fill 0.  */

static int count_insns_to_loop_end (fragS *, bool, int);
static bool branch_before_loop_end (fragS *);

static void
xtensa_fix_short_loop_frags (void)
{
  frchainS *frchP;
  asection *s;

  /* When this routine is called, all of the subsections are still intact
     so we walk over subsections instead of sections.  */
  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;
	xtensa_opcode current_opcode = XTENSA_UNDEFINED;

	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if (fragP->fr_type == rs_machine_dependent
		&& ((fragP->fr_subtype == RELAX_ALIGN_NEXT_OPCODE)
		    || (fragP->fr_subtype == RELAX_CHECK_ALIGN_NEXT_OPCODE)))
	      {
		TInsn t_insn;
		fragS *loop_frag = next_non_empty_frag (fragP);
		tinsn_from_chars (&t_insn, loop_frag->fr_opcode, 0);
		current_opcode = t_insn.opcode;
		gas_assert (xtensa_opcode_is_loop (xtensa_default_isa,
					       current_opcode) == 1);
	      }

	    if (fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_ADD_NOP_IF_SHORT_LOOP)
	      {
		if (count_insns_to_loop_end (fragP->fr_next, true, 3) < 3
		    && (branch_before_loop_end (fragP->fr_next)
			|| (workaround_all_short_loops
			    && current_opcode != XTENSA_UNDEFINED
			    && current_opcode != xtensa_loop_opcode)))
		  {
		    if (fragP->tc_frag_data.is_no_transform)
		      as_bad (_("loop containing less than three instructions may trigger hardware errata"));
		    else
		      relax_frag_add_nop (fragP);
		  }
		frag_wane (fragP);
	      }
	  }
      }
}


static int unrelaxed_frag_min_insn_count (fragS *);

static int
count_insns_to_loop_end (fragS *base_fragP,
			 bool count_relax_add,
			 int max_count)
{
  fragS *fragP = NULL;
  int insn_count = 0;

  fragP = base_fragP;

  for (; fragP && !fragP->tc_frag_data.is_loop_target; fragP = fragP->fr_next)
    {
      insn_count += unrelaxed_frag_min_insn_count (fragP);
      if (insn_count >= max_count)
	return max_count;

      if (count_relax_add)
	{
	  if (fragP->fr_type == rs_machine_dependent
	      && fragP->fr_subtype == RELAX_ADD_NOP_IF_SHORT_LOOP)
	    {
	      /* In order to add the appropriate number of
	         NOPs, we count an instruction for downstream
	         occurrences.  */
	      insn_count++;
	      if (insn_count >= max_count)
		return max_count;
	    }
	}
    }
  return insn_count;
}


static int
unrelaxed_frag_min_insn_count (fragS *fragP)
{
  xtensa_isa isa = xtensa_default_isa;
  static xtensa_insnbuf insnbuf = NULL;
  int insn_count = 0;
  unsigned int offset = 0;

  if (!fragP->tc_frag_data.is_insn)
    return insn_count;

  if (!insnbuf)
    insnbuf = xtensa_insnbuf_alloc (isa);

  /* Decode the fixed instructions.  */
  while (offset < fragP->fr_fix)
    {
      xtensa_format fmt;

      xtensa_insnbuf_from_chars
	(isa, insnbuf, (unsigned char *) fragP->fr_literal + offset, 0);
      fmt = xtensa_format_decode (isa, insnbuf);

      if (fmt == XTENSA_UNDEFINED)
	{
	  as_fatal (_("undecodable instruction in instruction frag"));
	  return insn_count;
	}
      offset += xtensa_format_length (isa, fmt);
      insn_count++;
    }

  return insn_count;
}


static bool unrelaxed_frag_has_b_j (fragS *);

static bool
branch_before_loop_end (fragS *base_fragP)
{
  fragS *fragP;

  for (fragP = base_fragP;
       fragP && !fragP->tc_frag_data.is_loop_target;
       fragP = fragP->fr_next)
    {
      if (unrelaxed_frag_has_b_j (fragP))
	return true;
    }
  return false;
}


static bool
unrelaxed_frag_has_b_j (fragS *fragP)
{
  static xtensa_insnbuf insnbuf = NULL;
  xtensa_isa isa = xtensa_default_isa;
  unsigned int offset = 0;

  if (!fragP->tc_frag_data.is_insn)
    return false;

  if (!insnbuf)
    insnbuf = xtensa_insnbuf_alloc (isa);

  /* Decode the fixed instructions.  */
  while (offset < fragP->fr_fix)
    {
      xtensa_format fmt;
      int slot;

      xtensa_insnbuf_from_chars
	(isa, insnbuf, (unsigned char *) fragP->fr_literal + offset, 0);
      fmt = xtensa_format_decode (isa, insnbuf);
      if (fmt == XTENSA_UNDEFINED)
	return false;

      for (slot = 0; slot < xtensa_format_num_slots (isa, fmt); slot++)
	{
	  xtensa_opcode opcode =
	    get_opcode_from_buf (fragP->fr_literal + offset, slot);
	  if (xtensa_opcode_is_branch (isa, opcode) == 1
	      || xtensa_opcode_is_jump (isa, opcode) == 1)
	    return true;
	}
      offset += xtensa_format_length (isa, fmt);
    }
  return false;
}


/* Checks to be made after initial assembly but before relaxation.  */

static bool is_empty_loop (const TInsn *, fragS *);
static bool is_local_forward_loop (const TInsn *, fragS *);

static void
xtensa_sanity_check (void)
{
  const char *file_name;
  unsigned line;
  frchainS *frchP;
  asection *s;

  file_name = as_where (&line);
  for (s = stdoutput->sections; s; s = s->next)
    for (frchP = seg_info (s)->frchainP; frchP; frchP = frchP->frch_next)
      {
	fragS *fragP;

	/* Walk over all of the fragments in a subsection.  */
	for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	  {
	    if (fragP->fr_type == rs_machine_dependent
		&& fragP->fr_subtype == RELAX_SLOTS
		&& fragP->tc_frag_data.slot_subtypes[0] == RELAX_IMMED)
	      {
		static xtensa_insnbuf insnbuf = NULL;
		TInsn t_insn;

		if (fragP->fr_opcode != NULL)
		  {
		    if (!insnbuf)
		      insnbuf = xtensa_insnbuf_alloc (xtensa_default_isa);
		    tinsn_from_chars (&t_insn, fragP->fr_opcode, 0);
		    tinsn_immed_from_frag (&t_insn, fragP, 0);

		    if (xtensa_opcode_is_loop (xtensa_default_isa,
					       t_insn.opcode) == 1)
		      {
			if (is_empty_loop (&t_insn, fragP))
			  {
			    new_logical_line (fragP->fr_file, fragP->fr_line);
			    as_bad (_("invalid empty loop"));
			  }
			if (!is_local_forward_loop (&t_insn, fragP))
			  {
			    new_logical_line (fragP->fr_file, fragP->fr_line);
			    as_bad (_("loop target does not follow "
				      "loop instruction in section"));
			  }
		      }
		  }
	      }
	  }
      }
  new_logical_line (file_name, line);
}


#define LOOP_IMMED_OPN 1

/* Return TRUE if the loop target is the next non-zero fragment.  */

static bool
is_empty_loop (const TInsn *insn, fragS *fragP)
{
  const expressionS *exp;
  symbolS *symbolP;
  fragS *next_fragP;

  if (insn->insn_type != ITYPE_INSN)
    return false;

  if (xtensa_opcode_is_loop (xtensa_default_isa, insn->opcode) != 1)
    return false;

  if (insn->ntok <= LOOP_IMMED_OPN)
    return false;

  exp = &insn->tok[LOOP_IMMED_OPN];

  if (exp->X_op != O_symbol)
    return false;

  symbolP = exp->X_add_symbol;
  if (!symbolP)
    return false;

  if (symbol_get_frag (symbolP) == NULL)
    return false;

  if (S_GET_VALUE (symbolP) != 0)
    return false;

  /* Walk through the zero-size fragments from this one.  If we find
     the target fragment, then this is a zero-size loop.  */

  for (next_fragP = fragP->fr_next;
       next_fragP != NULL;
       next_fragP = next_fragP->fr_next)
    {
      if (next_fragP == symbol_get_frag (symbolP))
	return true;
      if (next_fragP->fr_fix != 0)
	return false;
    }
  return false;
}


static bool
is_local_forward_loop (const TInsn *insn, fragS *fragP)
{
  const expressionS *exp;
  symbolS *symbolP;
  fragS *next_fragP;

  if (insn->insn_type != ITYPE_INSN)
    return false;

  if (xtensa_opcode_is_loop (xtensa_default_isa, insn->opcode) != 1)
    return false;

  if (insn->ntok <= LOOP_IMMED_OPN)
    return false;

  exp = &insn->tok[LOOP_IMMED_OPN];

  if (exp->X_op != O_symbol)
    return false;

  symbolP = exp->X_add_symbol;
  if (!symbolP)
    return false;

  if (symbol_get_frag (symbolP) == NULL)
    return false;

  /* Walk through fragments until we find the target.
     If we do not find the target, then this is an invalid loop.  */

  for (next_fragP = fragP->fr_next;
       next_fragP != NULL;
       next_fragP = next_fragP->fr_next)
    {
      if (next_fragP == symbol_get_frag (symbolP))
	return true;
    }

  return false;
}

#define XTINFO_NAME "Xtensa_Info"
#define XTINFO_NAMESZ 12
#define XTINFO_TYPE 1

static void
xtensa_add_config_info (void)
{
  asection *info_sec;
  char *data, *p;
  int sz;

  info_sec = subseg_new (".xtensa.info", 0);
  bfd_set_section_flags (info_sec, SEC_HAS_CONTENTS | SEC_READONLY);

  data = XNEWVEC (char, 100);
  sprintf (data, "USE_ABSOLUTE_LITERALS=%d\nABI=%d\n",
	   XSHAL_USE_ABSOLUTE_LITERALS, xtensa_abi_choice ());
  sz = strlen (data) + 1;

  /* Add enough null terminators to pad to a word boundary.  */
  do
    data[sz++] = 0;
  while ((sz & 3) != 0);

  /* Follow the standard note section layout:
     First write the length of the name string.  */
  p = frag_more (4);
  md_number_to_chars (p, (valueT) XTINFO_NAMESZ, 4);

  /* Next comes the length of the "descriptor", i.e., the actual data.  */
  p = frag_more (4);
  md_number_to_chars (p, (valueT) sz, 4);

  /* Write the note type.  */
  p = frag_more (4);
  md_number_to_chars (p, (valueT) XTINFO_TYPE, 4);

  /* Write the name field.  */
  p = frag_more (XTINFO_NAMESZ);
  memcpy (p, XTINFO_NAME, XTINFO_NAMESZ);

  /* Finally, write the descriptor.  */
  p = frag_more (sz);
  memcpy (p, data, sz);

  free (data);
}


/* Alignment Functions.  */

static int
get_text_align_power (unsigned target_size)
{
  if (target_size <= 4)
    return 2;

  if (target_size <= 8)
    return 3;

  if (target_size <= 16)
    return 4;

  if (target_size <= 32)
    return 5;

  if (target_size <= 64)
    return 6;

  if (target_size <= 128)
    return 7;

  if (target_size <= 256)
    return 8;

  if (target_size <= 512)
    return 9;

  if (target_size <= 1024)
    return 10;

  gas_assert (0);
  return 0;
}


static int
get_text_align_max_fill_size (int align_pow,
			      bool use_nops,
			      bool use_no_density)
{
  if (!use_nops)
    return (1 << align_pow);
  if (use_no_density)
    return 3 * (1 << align_pow);

  return 1 + (1 << align_pow);
}


/* Calculate the minimum bytes of fill needed at "address" to align a
   target instruction of size "target_size" so that it does not cross a
   power-of-two boundary specified by "align_pow".  If "use_nops" is FALSE,
   the fill can be an arbitrary number of bytes.  Otherwise, the space must
   be filled by NOP instructions.  */

static int
get_text_align_fill_size (addressT address,
			  int align_pow,
			  int target_size,
			  bool use_nops,
			  bool use_no_density)
{
  addressT alignment, fill, fill_limit, fill_step;
  bool skip_one = false;

  alignment = (1 << align_pow);
  gas_assert (target_size > 0 && alignment >= (addressT) target_size);

  if (!use_nops)
    {
      fill_limit = alignment;
      fill_step = 1;
    }
  else if (!use_no_density)
    {
      /* Combine 2- and 3-byte NOPs to fill anything larger than one.  */
      fill_limit = alignment * 2;
      fill_step = 1;
      skip_one = true;
    }
  else
    {
      /* Fill with 3-byte NOPs -- can only fill multiples of 3.  */
      fill_limit = alignment * 3;
      fill_step = 3;
    }

  /* Try all fill sizes until finding one that works.  */
  for (fill = 0; fill < fill_limit; fill += fill_step)
    {
      if (skip_one && fill == 1)
	continue;
      if ((address + fill) >> align_pow
	  == (address + fill + target_size - 1) >> align_pow)
	return fill;
    }
  gas_assert (0);
  return 0;
}


static int
branch_align_power (segT sec)
{
  /* If the Xtensa processor has a fetch width of X, and
     the section is aligned to at least that boundary, then a branch
     target need only fit within that aligned block of memory to avoid
     a stall.  Otherwise, try to fit branch targets within 4-byte
     aligned blocks (which may be insufficient, e.g., if the section
     has no alignment, but it's good enough).  */
  int fetch_align = get_text_align_power(xtensa_fetch_width);
  int sec_align = get_recorded_alignment (sec);

  if (sec_align >= fetch_align)
    return fetch_align;

  return 2;
}


/* This will assert if it is not possible.  */

static int
get_text_align_nop_count (offsetT fill_size, bool use_no_density)
{
  int count = 0;

  if (use_no_density)
    {
      gas_assert (fill_size % 3 == 0);
      return (fill_size / 3);
    }

  gas_assert (fill_size != 1);	/* Bad argument.  */

  while (fill_size > 1)
    {
      int insn_size = 3;
      if (fill_size == 2 || fill_size == 4)
	insn_size = 2;
      fill_size -= insn_size;
      count++;
    }
  gas_assert (fill_size != 1);	/* Bad algorithm.  */
  return count;
}


static int
get_text_align_nth_nop_size (offsetT fill_size,
			     int n,
			     bool use_no_density)
{
  int count = 0;

  if (use_no_density)
    return 3;

  gas_assert (fill_size != 1);	/* Bad argument.  */

  while (fill_size > 1)
    {
      int insn_size = 3;
      if (fill_size == 2 || fill_size == 4)
	insn_size = 2;
      fill_size -= insn_size;
      count++;
      if (n + 1 == count)
	return insn_size;
    }
  gas_assert (0);
  return 0;
}


/* For the given fragment, find the appropriate address
   for it to begin at if we are using NOPs to align it.  */

static addressT
get_noop_aligned_address (fragS *fragP, addressT address)
{
  /* The rule is: get next fragment's FIRST instruction.  Find
     the smallest number of bytes that need to be added to
     ensure that the next fragment's FIRST instruction will fit
     in a single word.

     E.G.,   2 bytes : 0, 1, 2 mod 4
	     3 bytes: 0, 1 mod 4

     If the FIRST instruction MIGHT be relaxed,
     assume that it will become a 3-byte instruction.

     Note again here that LOOP instructions are not bundleable,
     and this relaxation only applies to LOOP opcodes.  */

  int fill_size = 0;
  int first_insn_size;
  int loop_insn_size;
  addressT pre_opcode_bytes;
  int align_power;
  fragS *first_insn;
  xtensa_opcode opcode;
  bool is_loop;

  gas_assert (fragP->fr_type == rs_machine_dependent);
  gas_assert (fragP->fr_subtype == RELAX_ALIGN_NEXT_OPCODE);

  /* Find the loop frag.  */
  first_insn = next_non_empty_frag (fragP);
  /* Now find the first insn frag.  */
  first_insn = next_non_empty_frag (first_insn);

  is_loop = next_frag_opcode_is_loop (fragP, &opcode);
  gas_assert (is_loop);
  loop_insn_size = xg_get_single_size (opcode);

  pre_opcode_bytes = next_frag_pre_opcode_bytes (fragP);
  pre_opcode_bytes += loop_insn_size;

  /* For loops, the alignment depends on the size of the
     instruction following the loop, not the LOOP instruction.  */

  if (first_insn == NULL)
    first_insn_size = xtensa_fetch_width;
  else
    first_insn_size = get_loop_align_size (frag_format_size (first_insn));

  /* If it was 8, then we'll need a larger alignment for the section.  */
  align_power = get_text_align_power (first_insn_size);
  record_alignment (now_seg, align_power);

  fill_size = get_text_align_fill_size
    (address + pre_opcode_bytes, align_power, first_insn_size, true,
     fragP->tc_frag_data.is_no_density);

  return address + fill_size;
}


/* 3 mechanisms for relaxing an alignment:

   Align to a power of 2.
   Align so the next fragment's instruction does not cross a word boundary.
   Align the current instruction so that if the next instruction
       were 3 bytes, it would not cross a word boundary.

   We can align with:

   zeros    - This is easy; always insert zeros.
   nops     - 3-byte and 2-byte instructions
              2 - 2-byte nop
              3 - 3-byte nop
              4 - 2 2-byte nops
              >=5 : 3-byte instruction + fn (n-3)
   widening - widen previous instructions.  */

static offsetT
get_aligned_diff (fragS *fragP, addressT address, offsetT *max_diff)
{
  addressT target_address, loop_insn_offset;
  int target_size;
  xtensa_opcode loop_opcode;
  bool is_loop;
  int align_power;
  offsetT opt_diff;
  offsetT branch_align;
  fragS *loop_frag;

  gas_assert (fragP->fr_type == rs_machine_dependent);
  switch (fragP->fr_subtype)
    {
    case RELAX_DESIRE_ALIGN:
      target_size = next_frag_format_size (fragP);
      if (target_size == XTENSA_UNDEFINED)
	target_size = 3;
      align_power = branch_align_power (now_seg);
      branch_align = 1 << align_power;
      /* Don't count on the section alignment being as large as the target.  */
      if (target_size > branch_align)
	target_size = branch_align;
      opt_diff = get_text_align_fill_size (address, align_power,
					   target_size, false, false);

      *max_diff = (opt_diff + branch_align
		   - (target_size + ((address + opt_diff) % branch_align)));
      gas_assert (*max_diff >= opt_diff);
      return opt_diff;

    case RELAX_ALIGN_NEXT_OPCODE:
      /* The next non-empty frag after this one holds the LOOP instruction
	 that needs to be aligned.  The required alignment depends on the
	 size of the next non-empty frag after the loop frag, i.e., the
	 first instruction in the loop.  */
      loop_frag = next_non_empty_frag (fragP);
      target_size = get_loop_align_size (next_frag_format_size (loop_frag));
      loop_insn_offset = 0;
      is_loop = next_frag_opcode_is_loop (fragP, &loop_opcode);
      gas_assert (is_loop);

      /* If the loop has been expanded then the LOOP instruction
	 could be at an offset from this fragment.  */
      if (loop_frag->tc_frag_data.slot_subtypes[0] != RELAX_IMMED)
	loop_insn_offset = get_expanded_loop_offset (loop_opcode);

      /* In an ideal world, which is what we are shooting for here,
	 we wouldn't need to use any NOPs immediately prior to the
	 LOOP instruction.  If this approach fails, relax_frag_loop_align
	 will call get_noop_aligned_address.  */
      target_address =
	address + loop_insn_offset + xg_get_single_size (loop_opcode);
      align_power = get_text_align_power (target_size);
      opt_diff = get_text_align_fill_size (target_address, align_power,
					   target_size, false, false);

      *max_diff = xtensa_fetch_width
	- ((target_address + opt_diff) % xtensa_fetch_width)
	- target_size + opt_diff;
      gas_assert (*max_diff >= opt_diff);
      return opt_diff;

    default:
      break;
    }
  gas_assert (0);
  return 0;
}


/* md_relax_frag Hook and Helper Functions.  */

static long relax_frag_loop_align (fragS *, long);
static long relax_frag_for_align (fragS *, long);
static long relax_frag_immed
  (segT, fragS *, long, int, xtensa_format, int, int *, bool);

/* Get projected address for the first fulcrum on a path from source to
   target.  */
static addressT xg_get_fulcrum (addressT source, addressT target)
{
  offsetT delta = target - source;
  int n;

  n = (labs (delta) + J_RANGE - J_MARGIN - 1) / (J_RANGE - J_MARGIN);
  return source + delta / n;
}

/* Given trampoline index, source and target of a jump find the best
   candidate trampoline for the first fulcrum.  The best trampoline is
   the one in the reach of "j' instruction from the source, closest to
   the projected fulcrum address, and preferrably w/o a jump around or
   with already initialized jump around.  */
static size_t xg_find_best_trampoline (struct trampoline_index *idx,
				       addressT source, addressT target)
{
  addressT fulcrum = xg_get_fulcrum (source, target);
  size_t dist = 0;
  size_t best = -1;
  size_t base_tr = xg_find_trampoline (idx, fulcrum);
  int checked = 1;

  /* Check trampoline frags around the base_tr to find the best.  */
  for (dist = 0; checked; ++dist)
    {
      int i;
      size_t tr = base_tr - dist;

      checked = 0;

      /* Trampolines are checked in the following order:
         base_tr, base_tr + 1, base_tr - 1, base_tr + 2, base_tr - 2  */
      for (i = 0; i < 2; ++i, tr = base_tr + dist + 1)
	if (tr < idx->n_entries)
	  {
	    fragS *trampoline_frag = idx->entry[tr];
	    offsetT off;

	    /* Don't check trampolines outside source - target interval.  */
	    if ((trampoline_frag->fr_address < source &&
		 trampoline_frag->fr_address < target) ||
		(trampoline_frag->fr_address > source &&
		 trampoline_frag->fr_address > target))
	      continue;

	    /* Don't choose trampoline that contains the source.  */
	    if (source >= trampoline_frag->fr_address
		&& source <= trampoline_frag->fr_address +
		trampoline_frag->fr_fix)
	      continue;

	    off = trampoline_frag->fr_address - fulcrum;
	    /* Stop if some trampoline is found and the search is more than
	       J_RANGE / 4 from the projected fulcrum.  A trampoline w/o jump
	       around is nice, but it shouldn't have much overhead.  */
	    if (best < idx->n_entries && labs (off) > J_RANGE / 4)
	      return best;

	    off = trampoline_frag->fr_address - source;
	    if (labs (off) < J_RANGE - J_MARGIN)
	      {
		++checked;
		/* Stop if a trampoline w/o jump around is found or initialized
		   trampoline with jump around is found.  */
		if (!trampoline_frag->tc_frag_data.needs_jump_around ||
		    trampoline_frag->fr_fix)
		  return tr;
		else if (best >= idx->n_entries)
		  best = tr;
	      }
	  }
    }

  if (best < idx->n_entries)
    return best;
  else
    as_fatal (_("cannot find suitable trampoline"));
}

static fixS *xg_relax_fixup (struct trampoline_index *idx, fixS *fixP)
{
  symbolS *s = fixP->fx_addsy;
  addressT source = fixP->fx_frag->fr_address;
  addressT target = S_GET_VALUE (s) + fixP->fx_offset;
  size_t tr = xg_find_best_trampoline (idx, source, target);
  fragS *trampoline_frag = idx->entry[tr];
  fixS *newfixP;

  init_trampoline_frag (trampoline_frag);
  newfixP = xg_append_jump (trampoline_frag,
			    fixP->fx_addsy, fixP->fx_offset);

  /* Adjust the fixup for the original "j" instruction to
     point to the newly added jump.  */
  fixP->fx_addsy = trampoline_frag->fr_symbol;
  fixP->fx_offset = trampoline_frag->fr_fix - 3;
  fixP->tc_fix_data.X_add_symbol = trampoline_frag->fr_symbol;
  fixP->tc_fix_data.X_add_number = trampoline_frag->fr_fix - 3;

  trampoline_frag->tc_frag_data.relax_seen = false;

  if (xg_is_trampoline_frag_full (trampoline_frag))
    xg_remove_trampoline_from_index (idx, tr);

  return newfixP;
}

static bool xg_is_relaxable_fixup (fixS *fixP)
{
  xtensa_isa isa = xtensa_default_isa;
  addressT addr = fixP->fx_frag->fr_address;
  addressT target;
  offsetT delta;
  symbolS *s = fixP->fx_addsy;
  int slot;
  xtensa_format fmt;
  xtensa_opcode opcode;

  if (fixP->fx_r_type < BFD_RELOC_XTENSA_SLOT0_OP ||
      fixP->fx_r_type > BFD_RELOC_XTENSA_SLOT14_OP)
    return false;

  target = S_GET_VALUE (s) + fixP->fx_offset;
  delta = target - addr;

  if (labs (delta) < J_RANGE - J_MARGIN)
    return false;

  xtensa_insnbuf_from_chars (isa, trampoline_buf,
			     (unsigned char *) fixP->fx_frag->fr_literal +
			     fixP->fx_where, 0);
  fmt = xtensa_format_decode (isa, trampoline_buf);
  gas_assert (fmt != XTENSA_UNDEFINED);
  slot = fixP->tc_fix_data.slot;
  xtensa_format_get_slot (isa, fmt, slot, trampoline_buf, trampoline_slotbuf);
  opcode = xtensa_opcode_decode (isa, fmt, slot, trampoline_slotbuf);
  return opcode == xtensa_j_opcode;
}

static void xg_relax_fixups (struct trampoline_seg *ts)
{
  struct trampoline_index *idx = &ts->index;
  segment_info_type *seginfo = seg_info (now_seg);
  fixS *fx;

  for (fx = seginfo->fix_root; fx; fx = fx->fx_next)
    {
      fixS *fixP = fx;
      struct trampoline_chain *tc = NULL;

      if (xg_is_relaxable_fixup (fixP))
	{
	  tc = xg_find_best_eq_target (ts, fixP->fx_frag->fr_address,
				       &fixP->fx_addsy, &fixP->fx_offset);
	  if (!tc)
	    tc = xg_create_trampoline_chain (ts, fixP->fx_addsy,
					     fixP->fx_offset);
	  gas_assert (tc);
	}

      while (xg_is_relaxable_fixup (fixP))
	{
	  fixP = xg_relax_fixup (idx, fixP);
	  xg_add_location_to_chain (tc, fixP->fx_frag->fr_symbol,
				    fixP->fx_where);
	}
    }
}

/* Given a trampoline frag relax all jumps that might want to use this
   trampoline.  Only do real work once per relaxation cycle, when
   xg_relax_trampoline is called for the first trampoline in the now_seg.
   Don't use stretch, don't update new_stretch: place fulcrums with a
   slack to tolerate code movement.  In the worst case if a jump between
   two trampolines wouldn't reach the next relaxation pass will fix it.  */
static void xg_relax_trampoline (fragS *fragP, long stretch ATTRIBUTE_UNUSED,
				 long *new_stretch ATTRIBUTE_UNUSED)
{
  struct trampoline_seg *ts = find_trampoline_seg (now_seg);

  if (ts->index.n_entries && ts->index.entry[0] == fragP)
    xg_relax_fixups (ts);
}

/* Return the number of bytes added to this fragment, given that the
   input has been stretched already by "stretch".  */

long
xtensa_relax_frag (fragS *fragP, long stretch, int *stretched_p)
{
  xtensa_isa isa = xtensa_default_isa;
  int unreported = fragP->tc_frag_data.unreported_expansion;
  long new_stretch = 0;
  const char *file_name;
  unsigned line;
  int lit_size;
  static xtensa_insnbuf vbuf = NULL;
  int slot, num_slots;
  xtensa_format fmt;

  file_name = as_where (&line);
  new_logical_line (fragP->fr_file, fragP->fr_line);

  fragP->tc_frag_data.unreported_expansion = 0;

  switch (fragP->fr_subtype)
    {
    case RELAX_ALIGN_NEXT_OPCODE:
      /* Always convert.  */
      if (fragP->tc_frag_data.relax_seen)
	new_stretch = relax_frag_loop_align (fragP, stretch);
      break;

    case RELAX_LOOP_END:
      /* Do nothing.  */
      break;

    case RELAX_LOOP_END_ADD_NOP:
      /* Add a NOP and switch to .fill 0.  */
      new_stretch = relax_frag_add_nop (fragP);
      frag_wane (fragP);
      break;

    case RELAX_DESIRE_ALIGN:
      /* Do nothing. The narrowing before this frag will either align
         it or not.  */
      break;

    case RELAX_LITERAL:
    case RELAX_LITERAL_FINAL:
      return 0;

    case RELAX_LITERAL_NR:
      lit_size = 4;
      fragP->fr_subtype = RELAX_LITERAL_FINAL;
      gas_assert (unreported == lit_size);
      memset (&fragP->fr_literal[fragP->fr_fix], 0, 4);
      fragP->fr_var -= lit_size;
      fragP->fr_fix += lit_size;
      new_stretch = 4;
      break;

    case RELAX_SLOTS:
      if (vbuf == NULL)
	vbuf = xtensa_insnbuf_alloc (isa);

      xtensa_insnbuf_from_chars
	(isa, vbuf, (unsigned char *) fragP->fr_opcode, 0);
      fmt = xtensa_format_decode (isa, vbuf);
      num_slots = xtensa_format_num_slots (isa, fmt);

      for (slot = 0; slot < num_slots; slot++)
	{
	  switch (fragP->tc_frag_data.slot_subtypes[slot])
	    {
	    case RELAX_NARROW:
	      if (fragP->tc_frag_data.relax_seen)
		new_stretch += relax_frag_for_align (fragP, stretch);
	      break;

	    case RELAX_IMMED:
	    case RELAX_IMMED_STEP1:
	    case RELAX_IMMED_STEP2:
	    case RELAX_IMMED_STEP3:
	      /* Place the immediate.  */
	      new_stretch += relax_frag_immed
		(now_seg, fragP, stretch,
		 fragP->tc_frag_data.slot_subtypes[slot] - RELAX_IMMED,
		 fmt, slot, stretched_p, false);
	      break;

	    default:
	      /* This is OK; see the note in xg_assemble_vliw_tokens.  */
	      break;
	    }
	}
      break;

    case RELAX_LITERAL_POOL_BEGIN:
      if (fragP->fr_var != 0)
	{
	  /* We have a converted "candidate" literal pool;
	     assemble a jump around it.  */
	  TInsn insn;
	  if (!litpool_slotbuf)
	    {
	      litpool_buf = xtensa_insnbuf_alloc (isa);
	      litpool_slotbuf = xtensa_insnbuf_alloc (isa);
	    }
	  new_stretch += 3;
	  fragP->tc_frag_data.relax_seen = false; /* Need another pass.  */
	  fragP->tc_frag_data.is_insn = true;
	  tinsn_init (&insn);
	  insn.insn_type = ITYPE_INSN;
	  insn.opcode = xtensa_j_opcode;
	  insn.ntok = 1;
	  set_expr_symbol_offset (&insn.tok[0], fragP->fr_symbol,
				  fragP->fr_fix);
	  fmt = xg_get_single_format (xtensa_j_opcode);
	  tinsn_to_slotbuf (fmt, 0, &insn, litpool_slotbuf);
	  xtensa_format_set_slot (isa, fmt, 0, litpool_buf, litpool_slotbuf);
	  xtensa_insnbuf_to_chars (isa, litpool_buf,
				   (unsigned char *)fragP->fr_literal +
				   fragP->fr_fix, 3);
	  fragP->fr_fix += 3;
	  fragP->fr_var -= 3;
	  /* Add a fix-up.  */
	  fix_new (fragP, 0, 3, fragP->fr_symbol, 0, true,
		   BFD_RELOC_XTENSA_SLOT0_OP);
	}
      break;

    case RELAX_LITERAL_POOL_END:
    case RELAX_LITERAL_POOL_CANDIDATE_BEGIN:
    case RELAX_MAYBE_UNREACHABLE:
    case RELAX_MAYBE_DESIRE_ALIGN:
      /* No relaxation required.  */
      break;

    case RELAX_FILL_NOP:
    case RELAX_UNREACHABLE:
      if (fragP->tc_frag_data.relax_seen)
	new_stretch += relax_frag_for_align (fragP, stretch);
      break;

    case RELAX_TRAMPOLINE:
      if (fragP->tc_frag_data.relax_seen)
	xg_relax_trampoline (fragP, stretch, &new_stretch);
      break;

    default:
      as_bad (_("bad relaxation state"));
    }

  /* Tell gas we need another relaxation pass.  */
  if (! fragP->tc_frag_data.relax_seen)
    {
      fragP->tc_frag_data.relax_seen = true;
      *stretched_p = 1;
    }

  new_logical_line (file_name, line);
  return new_stretch;
}


static long
relax_frag_loop_align (fragS *fragP, long stretch)
{
  addressT old_address, old_next_address, old_size;
  addressT new_address, new_next_address, new_size;
  addressT growth;

  /* All the frags with relax_frag_for_alignment prior to this one in the
     section have been done, hopefully eliminating the need for a NOP here.
     But, this will put it in if necessary.  */

  /* Calculate the old address of this fragment and the next fragment.  */
  old_address = fragP->fr_address - stretch;
  old_next_address = (fragP->fr_address - stretch + fragP->fr_fix +
		      fragP->tc_frag_data.text_expansion[0]);
  old_size = old_next_address - old_address;

  /* Calculate the new address of this fragment and the next fragment.  */
  new_address = fragP->fr_address;
  new_next_address =
    get_noop_aligned_address (fragP, fragP->fr_address + fragP->fr_fix);
  new_size = new_next_address - new_address;

  growth = new_size - old_size;

  /* Fix up the text_expansion field and return the new growth.  */
  fragP->tc_frag_data.text_expansion[0] += growth;
  return growth;
}


/* Add a NOP instruction.  */

static long
relax_frag_add_nop (fragS *fragP)
{
  char *nop_buf = fragP->fr_literal + fragP->fr_fix;
  int length = fragP->tc_frag_data.is_no_density ? 3 : 2;
  assemble_nop (length, nop_buf);
  fragP->tc_frag_data.is_insn = true;

  if (fragP->fr_var < length)
    {
      as_fatal (_("fr_var (%ld) < length (%d)"), (long) fragP->fr_var, length);
      return 0;
    }

  fragP->fr_fix += length;
  fragP->fr_var -= length;
  return length;
}


static long future_alignment_required (fragS *, long);

static long
relax_frag_for_align (fragS *fragP, long stretch)
{
  /* Overview of the relaxation procedure for alignment:
     We can widen with NOPs or by widening instructions or by filling
     bytes after jump instructions.  Find the opportune places and widen
     them if necessary.  */

  long stretch_me;
  long diff;

  gas_assert (fragP->fr_subtype == RELAX_FILL_NOP
	  || fragP->fr_subtype == RELAX_UNREACHABLE
	  || (fragP->fr_subtype == RELAX_SLOTS
	      && fragP->tc_frag_data.slot_subtypes[0] == RELAX_NARROW));

  stretch_me = future_alignment_required (fragP, stretch);
  diff = stretch_me - fragP->tc_frag_data.text_expansion[0];
  if (diff == 0)
    return 0;

  if (diff < 0)
    {
      /* We expanded on a previous pass.  Can we shrink now?  */
      long shrink = fragP->tc_frag_data.text_expansion[0] - stretch_me;
      if (shrink <= stretch && stretch > 0)
	{
	  fragP->tc_frag_data.text_expansion[0] = stretch_me;
	  return -shrink;
	}
      return 0;
    }

  /* Below here, diff > 0.  */
  fragP->tc_frag_data.text_expansion[0] = stretch_me;

  return diff;
}


/* Return the address of the next frag that should be aligned.

   By "address" we mean the address it _would_ be at if there
   is no action taken to align it between here and the target frag.
   In other words, if no narrows and no fill nops are used between
   here and the frag to align, _even_if_ some of the frags we use
   to align targets have already expanded on a previous relaxation
   pass.

   Also, count each frag that may be used to help align the target.

   Return 0 if there are no frags left in the chain that need to be
   aligned.  */

static addressT
find_address_of_next_align_frag (fragS **fragPP,
				 int *wide_nops,
				 int *narrow_nops,
				 int *widens,
				 bool *paddable)
{
  fragS *fragP = *fragPP;
  addressT address = fragP->fr_address;

  /* Do not reset the counts to 0.  */

  while (fragP)
    {
      /* Limit this to a small search.  */
      if (*widens >= (int) xtensa_fetch_width)
	{
	  *fragPP = fragP;
	  return 0;
	}
      address += fragP->fr_fix;

      if (fragP->fr_type == rs_fill)
	address += fragP->fr_offset * fragP->fr_var;
      else if (fragP->fr_type == rs_machine_dependent)
	{
	  switch (fragP->fr_subtype)
	    {
	    case RELAX_UNREACHABLE:
	      *paddable = true;
	      break;

	    case RELAX_FILL_NOP:
	      (*wide_nops)++;
	      if (!fragP->tc_frag_data.is_no_density)
		(*narrow_nops)++;
	      break;

	    case RELAX_SLOTS:
	      if (fragP->tc_frag_data.slot_subtypes[0] == RELAX_NARROW)
		{
		  (*widens)++;
		  break;
		}
	      address += total_frag_text_expansion (fragP);
	      break;

	    case RELAX_IMMED:
	      address += fragP->tc_frag_data.text_expansion[0];
	      break;

	    case RELAX_ALIGN_NEXT_OPCODE:
	    case RELAX_DESIRE_ALIGN:
	      *fragPP = fragP;
	      return address;

	    case RELAX_MAYBE_UNREACHABLE:
	    case RELAX_MAYBE_DESIRE_ALIGN:
	      /* Do nothing.  */
	      break;

	    default:
	      /* Just punt if we don't know the type.  */
	      *fragPP = fragP;
	      return 0;
	    }
	}
      else
	{
	  /* Just punt if we don't know the type.  */
	  *fragPP = fragP;
	  return 0;
	}
      fragP = fragP->fr_next;
    }

  *fragPP = fragP;
  return 0;
}


static long bytes_to_stretch (fragS *, int, int, int, int);

static long
future_alignment_required (fragS *fragP, long stretch ATTRIBUTE_UNUSED)
{
  fragS *this_frag = fragP;
  long address;
  int num_widens = 0;
  int wide_nops = 0;
  int narrow_nops = 0;
  bool paddable = false;
  offsetT local_opt_diff;
  offsetT opt_diff;
  offsetT max_diff;
  int stretch_amount = 0;
  int local_stretch_amount;
  int global_stretch_amount;

  address = find_address_of_next_align_frag
    (&fragP, &wide_nops, &narrow_nops, &num_widens, &paddable);

  if (!address)
    {
      if (this_frag->tc_frag_data.is_aligning_branch)
	this_frag->tc_frag_data.slot_subtypes[0] = RELAX_IMMED;
      else
	frag_wane (this_frag);
    }
  else
    {
      local_opt_diff = get_aligned_diff (fragP, address, &max_diff);
      opt_diff = local_opt_diff;
      gas_assert (opt_diff >= 0);
      gas_assert (max_diff >= opt_diff);
      if (max_diff == 0)
	return 0;

      if (fragP)
	fragP = fragP->fr_next;

      while (fragP && opt_diff < max_diff && address)
	{
	  /* We only use these to determine if we can exit early
	     because there will be plenty of ways to align future
	     align frags.  */
	  int glob_widens = 0;
	  int dnn = 0;
	  int dw = 0;
	  bool glob_pad = 0;
	  address = find_address_of_next_align_frag
	    (&fragP, &glob_widens, &dnn, &dw, &glob_pad);
	  /* If there is a padable portion, then skip.  */
	  if (glob_pad || glob_widens >= (1 << branch_align_power (now_seg)))
	    address = 0;

	  if (address)
	    {
	      offsetT next_m_diff;
	      offsetT next_o_diff;

	      /* Downrange frags haven't had stretch added to them yet.  */
	      address += stretch;

	      /* The address also includes any text expansion from this
		 frag in a previous pass, but we don't want that.  */
	      address -= this_frag->tc_frag_data.text_expansion[0];

	      /* Assume we are going to move at least opt_diff.  In
		 reality, we might not be able to, but assuming that
		 we will helps catch cases where moving opt_diff pushes
		 the next target from aligned to unaligned.  */
	      address += opt_diff;

	      next_o_diff = get_aligned_diff (fragP, address, &next_m_diff);

	      /* Now cleanup for the adjustments to address.  */
	      next_o_diff += opt_diff;
	      next_m_diff += opt_diff;
	      if (next_o_diff <= max_diff && next_o_diff > opt_diff)
		opt_diff = next_o_diff;
	      if (next_m_diff < max_diff)
		max_diff = next_m_diff;
	      fragP = fragP->fr_next;
	    }
	}

      /* If there are enough wideners in between, do it.  */
      if (paddable)
	{
	  if (this_frag->fr_subtype == RELAX_UNREACHABLE)
	    {
	      gas_assert (opt_diff <= (signed) xtensa_fetch_width);
	      return opt_diff;
	    }
	  return 0;
	}
      local_stretch_amount
	= bytes_to_stretch (this_frag, wide_nops, narrow_nops,
			    num_widens, local_opt_diff);
      global_stretch_amount
	= bytes_to_stretch (this_frag, wide_nops, narrow_nops,
			    num_widens, opt_diff);
      /* If the condition below is true, then the frag couldn't
	 stretch the correct amount for the global case, so we just
	 optimize locally.  We'll rely on the subsequent frags to get
	 the correct alignment in the global case.  */
      if (global_stretch_amount < local_stretch_amount)
	stretch_amount = local_stretch_amount;
      else
	stretch_amount = global_stretch_amount;

      if (this_frag->fr_subtype == RELAX_SLOTS
	  && this_frag->tc_frag_data.slot_subtypes[0] == RELAX_NARROW)
	gas_assert (stretch_amount <= 1);
      else if (this_frag->fr_subtype == RELAX_FILL_NOP)
	{
	  if (this_frag->tc_frag_data.is_no_density)
	    gas_assert (stretch_amount == 3 || stretch_amount == 0);
	  else
	    gas_assert (stretch_amount <= 3);
	}
    }
  return stretch_amount;
}


/* The idea: widen everything you can to get a target or loop aligned,
   then start using NOPs.

   wide_nops   = the number of wide NOPs available for aligning
   narrow_nops = the number of narrow NOPs available for aligning
		 (a subset of wide_nops)
   widens      = the number of narrow instructions that should be widened

*/

static long
bytes_to_stretch (fragS *this_frag,
		  int wide_nops,
		  int narrow_nops,
		  int num_widens,
		  int desired_diff)
{
  int nops_needed;
  int nop_bytes;
  int extra_bytes;
  int bytes_short = desired_diff - num_widens;

  gas_assert (desired_diff >= 0
	      && desired_diff < (signed) xtensa_fetch_width);
  if (desired_diff == 0)
    return 0;

  gas_assert (wide_nops > 0 || num_widens > 0);

  /* Always prefer widening to NOP-filling.  */
  if (bytes_short < 0)
    {
      /* There are enough RELAX_NARROW frags after this one
	 to align the target without widening this frag in any way.  */
      return 0;
    }

  if (bytes_short == 0)
    {
      /* Widen every narrow between here and the align target
	 and the align target will be properly aligned.  */
      if (this_frag->fr_subtype == RELAX_FILL_NOP)
	return 0;
      else
	return 1;
    }

  /* From here we will need at least one NOP to get an alignment.
     However, we may not be able to align at all, in which case,
     don't widen.  */
  nops_needed = desired_diff / 3;

  /* If there aren't enough nops, don't widen.  */
  if (nops_needed > wide_nops)
    return 0;

  /* First try it with all wide nops.  */
  nop_bytes = nops_needed * 3;
  extra_bytes = desired_diff - nop_bytes;

  if (nop_bytes + num_widens >= desired_diff)
    {
      if (this_frag->fr_subtype == RELAX_FILL_NOP)
	return 3;
      else if (num_widens == extra_bytes)
	return 1;
      return 0;
    }

  /* Add a narrow nop.  */
  nops_needed++;
  nop_bytes += 2;
  extra_bytes -= 2;
  if (narrow_nops == 0 || nops_needed > wide_nops)
    return 0;

  if (nop_bytes + num_widens >= desired_diff && extra_bytes >= 0)
    {
      if (this_frag->fr_subtype == RELAX_FILL_NOP)
	return !this_frag->tc_frag_data.is_no_density ? 2 : 3;
      else if (num_widens == extra_bytes)
	return 1;
      return 0;
    }

  /* Replace a wide nop with a narrow nop--we can get here if
     extra_bytes was negative in the previous conditional.  */
  if (narrow_nops == 1)
    return 0;
  nop_bytes--;
  extra_bytes++;
  if (nop_bytes + num_widens >= desired_diff)
    {
      if (this_frag->fr_subtype == RELAX_FILL_NOP)
	return !this_frag->tc_frag_data.is_no_density ? 2 : 3;
      else if (num_widens == extra_bytes)
	return 1;
      return 0;
    }

  /* If we can't satisfy any of the above cases, then we can't align
     using padding or fill nops.  */
  return 0;
}


static fragS *
xg_find_best_trampoline_for_tinsn (TInsn *tinsn, fragS *fragP)
{
  symbolS *sym = tinsn->tok[0].X_add_symbol;
  addressT source = fragP->fr_address;
  addressT target = S_GET_VALUE (sym) + tinsn->tok[0].X_add_number;
  struct trampoline_seg *ts = find_trampoline_seg (now_seg);
  size_t i;

  if (!ts || !ts->index.n_entries)
    return NULL;

  i = xg_find_best_trampoline (&ts->index, source, target);

  return ts->index.entry[i];
}


/* Append jump to sym + offset to the end of the trampoline frag fragP.
   Adjust fragP's jump around if it's present.  Adjust fragP's fr_fix/fr_var
   and finish the frag if it's full (but don't remove it from the trampoline
   frag index).  Return fixup for the newly created jump.  */
static fixS *xg_append_jump (fragS *fragP, symbolS *sym, offsetT offset)
{
  fixS *fixP;
  TInsn insn;
  xtensa_format fmt;
  xtensa_isa isa = xtensa_default_isa;

  gas_assert (fragP->fr_var >= 3);
  tinsn_init (&insn);
  insn.insn_type = ITYPE_INSN;
  insn.opcode = xtensa_j_opcode;
  insn.ntok = 1;
  set_expr_symbol_offset (&insn.tok[0], sym, offset);
  fmt = xg_get_single_format (xtensa_j_opcode);
  tinsn_to_slotbuf (fmt, 0, &insn, trampoline_slotbuf);
  xtensa_format_set_slot (isa, fmt, 0, trampoline_buf, trampoline_slotbuf);
  xtensa_insnbuf_to_chars (isa, trampoline_buf,
			   (unsigned char *)fragP->fr_literal + fragP->fr_fix, 3);
  fixP = fix_new (fragP, fragP->fr_fix, 3, sym, offset, true,
		  BFD_RELOC_XTENSA_SLOT0_OP);
  fixP->tc_fix_data.slot = 0;

  fragP->fr_fix += 3;
  fragP->fr_var -= 3;

  /* Adjust the jump around this trampoline (if present).  */
  if (fragP->tc_frag_data.jump_around_fix)
    fragP->tc_frag_data.jump_around_fix->fx_offset += 3;

  /* Do we have room for more? */
  if (xg_is_trampoline_frag_full (fragP))
    {
      frag_wane (fragP);
      fragP->fr_subtype = 0;
    }

  return fixP;
}


static int
init_trampoline_frag (fragS *fp)
{
  int growth = 0;

  if (fp->fr_fix == 0)
    {
      symbolS *lsym;
      char label[10 + 2 * sizeof(fp)];

      sprintf (label, ".L0_TR_%p", fp);
      lsym = (symbolS *) local_symbol_make (label, now_seg, fp, 0);
      fp->fr_symbol = lsym;
      if (fp->tc_frag_data.needs_jump_around)
        {
	  fp->tc_frag_data.jump_around_fix = xg_append_jump (fp, lsym, 3);
	  growth = 3;
        }
    }
  return growth;
}

static int
xg_get_single_symbol_slot (fragS *fragP)
{
  int i;
  int slot = -1;

  for (i = 0; i < MAX_SLOTS; ++i)
    if (fragP->tc_frag_data.slot_symbols[i])
      {
	gas_assert (slot == -1);
	slot = i;
      }

  gas_assert (slot >= 0 && slot < MAX_SLOTS);

  return slot;
}

static fixS *
add_jump_to_trampoline (fragS *tramp, fragS *origfrag)
{
  int slot = xg_get_single_symbol_slot (origfrag);
  fixS *fixP;

  /* Assemble a jump to the target label in the trampoline frag.  */
  fixP = xg_append_jump (tramp,
			 origfrag->tc_frag_data.slot_symbols[slot],
			 origfrag->tc_frag_data.slot_offsets[slot]);

  /* Modify the original j to point here.  */
  origfrag->tc_frag_data.slot_symbols[slot] = tramp->fr_symbol;
  origfrag->tc_frag_data.slot_offsets[slot] = tramp->fr_fix - 3;

  /* If trampoline is full, remove it from the list.  */
  if (xg_is_trampoline_frag_full (tramp))
    {
      struct trampoline_seg *ts = find_trampoline_seg (now_seg);
      size_t tr = xg_find_trampoline (&ts->index, tramp->fr_address);

      gas_assert (ts->index.entry[tr] == tramp);
      xg_remove_trampoline_from_index (&ts->index, tr);
    }

  return fixP;
}


static long
relax_frag_immed (segT segP,
		  fragS *fragP,
		  long stretch,
		  int min_steps,
		  xtensa_format fmt,
		  int slot,
		  int *stretched_p,
		  bool estimate_only)
{
  TInsn tinsn;
  int old_size;
  bool negatable_branch = false;
  bool branch_jmp_to_next = false;
  bool from_wide_insn = false;
  xtensa_isa isa = xtensa_default_isa;
  IStack istack;
  offsetT frag_offset;
  int num_steps;
  int num_text_bytes, num_literal_bytes;
  int literal_diff, total_text_diff, this_text_diff;

  gas_assert (fragP->fr_opcode != NULL);

  xg_clear_vinsn (&cur_vinsn);
  vinsn_from_chars (&cur_vinsn, fragP->fr_opcode);
  if (cur_vinsn.num_slots > 1)
    from_wide_insn = true;

  tinsn = cur_vinsn.slots[slot];
  tinsn_immed_from_frag (&tinsn, fragP, slot);

  if (estimate_only && xtensa_opcode_is_loop (isa, tinsn.opcode) == 1)
    return 0;

  if (workaround_b_j_loop_end && ! fragP->tc_frag_data.is_no_transform)
    branch_jmp_to_next = is_branch_jmp_to_next (&tinsn, fragP);

  negatable_branch = (xtensa_opcode_is_branch (isa, tinsn.opcode) == 1);

  old_size = xtensa_format_length (isa, fmt);

  /* Special case: replace a branch to the next instruction with a NOP.
     This is required to work around a hardware bug in T1040.0 and also
     serves as an optimization.  */

  if (branch_jmp_to_next
      && ((old_size == 2) || (old_size == 3))
      && !next_frag_is_loop_target (fragP))
    return 0;

  /* Here is the fun stuff: Get the immediate field from this
     instruction.  If it fits, we are done.  If not, find the next
     instruction sequence that fits.  */

  frag_offset = fragP->fr_opcode - fragP->fr_literal;
  istack_init (&istack);
  num_steps = xg_assembly_relax (&istack, &tinsn, segP, fragP, frag_offset,
				 min_steps, stretch);
  gas_assert (num_steps >= min_steps && num_steps <= RELAX_IMMED_MAXSTEPS);

  fragP->tc_frag_data.slot_subtypes[slot] = (int) RELAX_IMMED + num_steps;

  /* Figure out the number of bytes needed.  */
  num_literal_bytes = get_num_stack_literal_bytes (&istack);
  literal_diff
    = num_literal_bytes - fragP->tc_frag_data.literal_expansion[slot];
  num_text_bytes = get_num_stack_text_bytes (&istack);

  if (from_wide_insn)
    {
      int first = 0;
      while (istack.insn[first].opcode == XTENSA_UNDEFINED)
	first++;

      num_text_bytes += old_size;
      if (opcode_fits_format_slot (istack.insn[first].opcode, fmt, slot))
	num_text_bytes -= xg_get_single_size (istack.insn[first].opcode);
      else
	{
	  /* The first instruction in the relaxed sequence will go after
	     the current wide instruction, and thus its symbolic immediates
	     might not fit.  */

	  istack_init (&istack);
	  num_steps = xg_assembly_relax (&istack, &tinsn, segP, fragP,
					 frag_offset + old_size,
					 min_steps, stretch + old_size);
	  gas_assert (num_steps >= min_steps && num_steps <= RELAX_IMMED_MAXSTEPS);

	  fragP->tc_frag_data.slot_subtypes[slot]
	    = (int) RELAX_IMMED + num_steps;

	  num_literal_bytes = get_num_stack_literal_bytes (&istack);
	  literal_diff
	    = num_literal_bytes - fragP->tc_frag_data.literal_expansion[slot];

	  num_text_bytes = get_num_stack_text_bytes (&istack) + old_size;
	}
    }

  total_text_diff = num_text_bytes - old_size;
  this_text_diff = total_text_diff - fragP->tc_frag_data.text_expansion[slot];

  /* It MUST get larger.  If not, we could get an infinite loop.  */
  gas_assert (num_text_bytes >= 0);
  gas_assert (literal_diff >= 0);
  gas_assert (total_text_diff >= 0);

  fragP->tc_frag_data.text_expansion[slot] = total_text_diff;
  fragP->tc_frag_data.literal_expansion[slot] = num_literal_bytes;
  gas_assert (fragP->tc_frag_data.text_expansion[slot] >= 0);
  gas_assert (fragP->tc_frag_data.literal_expansion[slot] >= 0);

  /* Find the associated expandable literal for this.  */
  if (literal_diff != 0)
    {
      fragS *lit_fragP = fragP->tc_frag_data.literal_frags[slot];
      if (lit_fragP)
	{
	  gas_assert (literal_diff == 4);
	  lit_fragP->tc_frag_data.unreported_expansion += literal_diff;

	  /* We expect that the literal section state has NOT been
	     modified yet.  */
	  gas_assert (lit_fragP->fr_type == rs_machine_dependent
		  && lit_fragP->fr_subtype == RELAX_LITERAL);
	  lit_fragP->fr_subtype = RELAX_LITERAL_NR;

	  /* We need to mark this section for another iteration
	     of relaxation.  */
	  (*stretched_p)++;
	}
    }

  if (negatable_branch && istack.ninsn > 1)
    update_next_frag_state (fragP);

  /* If last insn is a jump, and it cannot reach its target, try to find a trampoline.  */
  if (istack.ninsn > 2 &&
      istack.insn[istack.ninsn - 1].insn_type == ITYPE_LABEL &&
      istack.insn[istack.ninsn - 2].insn_type == ITYPE_INSN &&
      istack.insn[istack.ninsn - 2].opcode == xtensa_j_opcode)
    {
      TInsn *jinsn = &istack.insn[istack.ninsn - 2];
      struct trampoline_seg *ts = find_trampoline_seg (segP);
      struct trampoline_chain *tc = NULL;

      if (ts &&
	  !xg_symbolic_immeds_fit (jinsn, segP, fragP, fragP->fr_offset,
				   total_text_diff))
	{
	  int s = xg_get_single_symbol_slot (fragP);
	  addressT offset = fragP->tc_frag_data.slot_offsets[s];

	  tc = xg_find_best_eq_target (ts, fragP->fr_address,
				       &fragP->tc_frag_data.slot_symbols[s],
				       &offset);

	  if (!tc)
	    tc = xg_create_trampoline_chain (ts,
					     fragP->tc_frag_data.slot_symbols[s],
					     offset);
	  fragP->tc_frag_data.slot_offsets[s] = offset;
	  tinsn_immed_from_frag (jinsn, fragP, s);
	}

      if (!xg_symbolic_immeds_fit (jinsn, segP, fragP, fragP->fr_offset,
				   total_text_diff))
	{
	  fragS *tf = xg_find_best_trampoline_for_tinsn (jinsn, fragP);

	  if (tf)
	    {
	      fixS *fixP;

	      this_text_diff += init_trampoline_frag (tf) + 3;
	      fixP = add_jump_to_trampoline (tf, fragP);
	      xg_add_location_to_chain (tc, fixP->fx_frag->fr_symbol,
					fixP->fx_where);
	      fragP->tc_frag_data.relax_seen = false;
	    }
	  else
	    {
	      /* If target symbol is undefined, assume it will reach once linked.  */
	      expressionS *exp = &istack.insn[istack.ninsn - 2].tok[0];

	      if (exp->X_op == O_symbol && S_IS_DEFINED (exp->X_add_symbol))
		{
		  as_bad_where (fragP->fr_file, fragP->fr_line,
		    _("jump target out of range; no usable trampoline found"));
		}
	    }
	}
    }

  return this_text_diff;
}


/* md_convert_frag Hook and Helper Functions.  */

static void convert_frag_align_next_opcode (fragS *);
static void convert_frag_narrow (segT, fragS *, xtensa_format, int);
static void convert_frag_fill_nop (fragS *);
static void convert_frag_immed (segT, fragS *, int, xtensa_format, int);

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, segT sec, fragS *fragp)
{
  static xtensa_insnbuf vbuf = NULL;
  xtensa_isa isa = xtensa_default_isa;
  int slot;
  int num_slots;
  xtensa_format fmt;
  const char *file_name;
  unsigned line;

  file_name = as_where (&line);
  new_logical_line (fragp->fr_file, fragp->fr_line);

  switch (fragp->fr_subtype)
    {
    case RELAX_ALIGN_NEXT_OPCODE:
      /* Always convert.  */
      convert_frag_align_next_opcode (fragp);
      break;

    case RELAX_DESIRE_ALIGN:
      /* Do nothing.  If not aligned already, too bad.  */
      break;

    case RELAX_LITERAL:
    case RELAX_LITERAL_FINAL:
      break;

    case RELAX_SLOTS:
      if (vbuf == NULL)
	vbuf = xtensa_insnbuf_alloc (isa);

      xtensa_insnbuf_from_chars
	(isa, vbuf, (unsigned char *) fragp->fr_opcode, 0);
      fmt = xtensa_format_decode (isa, vbuf);
      num_slots = xtensa_format_num_slots (isa, fmt);

      for (slot = 0; slot < num_slots; slot++)
	{
	  switch (fragp->tc_frag_data.slot_subtypes[slot])
	    {
	    case RELAX_NARROW:
	      convert_frag_narrow (sec, fragp, fmt, slot);
	      break;

	    case RELAX_IMMED:
	    case RELAX_IMMED_STEP1:
	    case RELAX_IMMED_STEP2:
	    case RELAX_IMMED_STEP3:
	      /* Place the immediate.  */
	      convert_frag_immed
		(sec, fragp,
		 fragp->tc_frag_data.slot_subtypes[slot] - RELAX_IMMED,
		 fmt, slot);
	      break;

	    default:
	      /* This is OK because some slots could have
		 relaxations and others have none.  */
	      break;
	    }
	}
      break;

    case RELAX_UNREACHABLE:
      memset (&fragp->fr_literal[fragp->fr_fix], 0, fragp->fr_var);
      fragp->fr_fix += fragp->tc_frag_data.text_expansion[0];
      fragp->fr_var -= fragp->tc_frag_data.text_expansion[0];
      frag_wane (fragp);
      break;

    case RELAX_MAYBE_UNREACHABLE:
    case RELAX_MAYBE_DESIRE_ALIGN:
      frag_wane (fragp);
      break;

    case RELAX_FILL_NOP:
      convert_frag_fill_nop (fragp);
      break;

    case RELAX_LITERAL_NR:
      if (use_literal_section)
	{
	  /* This should have been handled during relaxation.  When
	     relaxing a code segment, literals sometimes need to be
	     added to the corresponding literal segment.  If that
	     literal segment has already been relaxed, then we end up
	     in this situation.  Marking the literal segments as data
	     would make this happen less often (since GAS always relaxes
	     code before data), but we could still get into trouble if
	     there are instructions in a segment that is not marked as
	     containing code.  Until we can implement a better solution,
	     cheat and adjust the addresses of all the following frags.
	     This could break subsequent alignments, but the linker's
	     literal coalescing will do that anyway.  */

	  fragS *f;
	  fragp->fr_subtype = RELAX_LITERAL_FINAL;
	  gas_assert (fragp->tc_frag_data.unreported_expansion == 4);
	  memset (&fragp->fr_literal[fragp->fr_fix], 0, 4);
	  fragp->fr_var -= 4;
	  fragp->fr_fix += 4;
	  for (f = fragp->fr_next; f; f = f->fr_next)
	    f->fr_address += 4;
	}
      else
	as_bad (_("invalid relaxation fragment result"));
      break;

    case RELAX_TRAMPOLINE:
      break;
    }

  fragp->fr_var = 0;
  new_logical_line (file_name, line);
}


static void
convert_frag_align_next_opcode (fragS *fragp)
{
  char *nop_buf;		/* Location for Writing.  */
  bool use_no_density = fragp->tc_frag_data.is_no_density;
  addressT aligned_address;
  offsetT fill_size;
  int nop, nop_count;

  aligned_address = get_noop_aligned_address (fragp, fragp->fr_address +
					      fragp->fr_fix);
  fill_size = aligned_address - (fragp->fr_address + fragp->fr_fix);
  nop_count = get_text_align_nop_count (fill_size, use_no_density);
  nop_buf = fragp->fr_literal + fragp->fr_fix;

  for (nop = 0; nop < nop_count; nop++)
    {
      int nop_size;
      nop_size = get_text_align_nth_nop_size (fill_size, nop, use_no_density);

      assemble_nop (nop_size, nop_buf);
      nop_buf += nop_size;
    }

  fragp->fr_fix += fill_size;
  fragp->fr_var -= fill_size;
}


static void
convert_frag_narrow (segT segP, fragS *fragP, xtensa_format fmt, int slot)
{
  TInsn tinsn, single_target;
  int size, old_size, diff;
  offsetT frag_offset;

  gas_assert (slot == 0);
  tinsn_from_chars (&tinsn, fragP->fr_opcode, 0);

  if (fragP->tc_frag_data.is_aligning_branch == 1)
    {
      gas_assert (fragP->tc_frag_data.text_expansion[0] == 1
	      || fragP->tc_frag_data.text_expansion[0] == 0);
      convert_frag_immed (segP, fragP, fragP->tc_frag_data.text_expansion[0],
			  fmt, slot);
      return;
    }

  if (fragP->tc_frag_data.text_expansion[0] == 0)
    {
      /* No conversion.  */
      fragP->fr_var = 0;
      return;
    }

  gas_assert (fragP->fr_opcode != NULL);

  /* Frags in this relaxation state should only contain
     single instruction bundles.  */
  tinsn_immed_from_frag (&tinsn, fragP, 0);

  /* Just convert it to a wide form....  */
  size = 0;
  old_size = xg_get_single_size (tinsn.opcode);

  tinsn_init (&single_target);
  frag_offset = fragP->fr_opcode - fragP->fr_literal;

  if (! xg_is_single_relaxable_insn (&tinsn, &single_target, false))
    {
      as_bad (_("unable to widen instruction"));
      return;
    }

  size = xg_get_single_size (single_target.opcode);
  xg_emit_insn_to_buf (&single_target, fragP->fr_opcode, fragP,
		       frag_offset, true);

  diff = size - old_size;
  gas_assert (diff >= 0);
  gas_assert (diff <= fragP->fr_var);
  fragP->fr_var -= diff;
  fragP->fr_fix += diff;

  /* clean it up */
  fragP->fr_var = 0;
}


static void
convert_frag_fill_nop (fragS *fragP)
{
  char *loc = &fragP->fr_literal[fragP->fr_fix];
  int size = fragP->tc_frag_data.text_expansion[0];
  gas_assert ((unsigned) size == (fragP->fr_next->fr_address
			      - fragP->fr_address - fragP->fr_fix));
  if (size == 0)
    {
      /* No conversion.  */
      fragP->fr_var = 0;
      return;
    }
  assemble_nop (size, loc);
  fragP->tc_frag_data.is_insn = true;
  fragP->fr_var -= size;
  fragP->fr_fix += size;
  frag_wane (fragP);
}


static fixS *fix_new_exp_in_seg
  (segT, subsegT, fragS *, int, int, expressionS *, int,
   bfd_reloc_code_real_type);

static void
convert_frag_immed (segT segP,
		    fragS *fragP,
		    int min_steps,
		    xtensa_format fmt,
		    int slot)
{
  char *immed_instr = fragP->fr_opcode;
  TInsn orig_tinsn;
  bool expanded = false;
  bool branch_jmp_to_next = false;
  char *fr_opcode = fragP->fr_opcode;
  xtensa_isa isa = xtensa_default_isa;
  bool from_wide_insn = false;
  int bytes;
  bool is_loop;

  gas_assert (fr_opcode != NULL);

  xg_clear_vinsn (&cur_vinsn);

  vinsn_from_chars (&cur_vinsn, fr_opcode);
  if (cur_vinsn.num_slots > 1)
    from_wide_insn = true;

  orig_tinsn = cur_vinsn.slots[slot];
  tinsn_immed_from_frag (&orig_tinsn, fragP, slot);

  is_loop = xtensa_opcode_is_loop (xtensa_default_isa, orig_tinsn.opcode) == 1;

  if (workaround_b_j_loop_end && ! fragP->tc_frag_data.is_no_transform)
    branch_jmp_to_next = is_branch_jmp_to_next (&orig_tinsn, fragP);

  if (branch_jmp_to_next && !next_frag_is_loop_target (fragP))
    {
      /* Conversion just inserts a NOP and marks the fix as completed.  */
      bytes = xtensa_format_length (isa, fmt);
      if (bytes >= 4)
	{
	  cur_vinsn.slots[slot].opcode =
	    xtensa_format_slot_nop_opcode (isa, cur_vinsn.format, slot);
	  cur_vinsn.slots[slot].ntok = 0;
	}
      else
	{
	  bytes += fragP->tc_frag_data.text_expansion[0];
	  gas_assert (bytes == 2 || bytes == 3);
	  build_nop (&cur_vinsn.slots[0], bytes);
	  fragP->fr_fix += fragP->tc_frag_data.text_expansion[0];
	}
      vinsn_to_insnbuf (&cur_vinsn, fr_opcode, frag_now, true);
      xtensa_insnbuf_to_chars
	(isa, cur_vinsn.insnbuf, (unsigned char *) fr_opcode, 0);
      fragP->fr_var = 0;
    }
  else
    {
      /* Here is the fun stuff:  Get the immediate field from this
	 instruction.  If it fits, we're done.  If not, find the next
	 instruction sequence that fits.  */

      IStack istack;
      int i;
      symbolS *lit_sym = NULL;
      int total_size = 0;
      int target_offset = 0;
      int old_size;
      int diff;
      symbolS *gen_label = NULL;
      offsetT frag_offset;
      bool first = true;

      /* It does not fit.  Find something that does and
         convert immediately.  */
      frag_offset = fr_opcode - fragP->fr_literal;
      istack_init (&istack);
      xg_assembly_relax (&istack, &orig_tinsn,
			 segP, fragP, frag_offset, min_steps, 0);

      old_size = xtensa_format_length (isa, fmt);

      /* Assemble this right inline.  */

      /* First, create the mapping from a label name to the REAL label.  */
      target_offset = 0;
      for (i = 0; i < istack.ninsn; i++)
	{
	  TInsn *tinsn = &istack.insn[i];
	  fragS *lit_frag;

	  switch (tinsn->insn_type)
	    {
	    case ITYPE_LITERAL:
	      if (lit_sym != NULL)
		as_bad (_("multiple literals in expansion"));
	      /* First find the appropriate space in the literal pool.  */
	      lit_frag = fragP->tc_frag_data.literal_frags[slot];
	      if (lit_frag == NULL)
		as_bad (_("no registered fragment for literal"));
	      if (tinsn->ntok != 1)
		as_bad (_("number of literal tokens != 1"));

	      /* Set the literal symbol and add a fixup.  */
	      lit_sym = lit_frag->fr_symbol;
	      break;

	    case ITYPE_LABEL:
	      if (align_targets && !is_loop)
		{
		  fragS *unreach = fragP->fr_next;
		  while (!(unreach->fr_type == rs_machine_dependent
			   && (unreach->fr_subtype == RELAX_MAYBE_UNREACHABLE
			       || unreach->fr_subtype == RELAX_UNREACHABLE)))
		    {
		      unreach = unreach->fr_next;
		    }

		  gas_assert (unreach->fr_type == rs_machine_dependent
			  && (unreach->fr_subtype == RELAX_MAYBE_UNREACHABLE
			      || unreach->fr_subtype == RELAX_UNREACHABLE));

		  target_offset += unreach->tc_frag_data.text_expansion[0];
		}
	      gas_assert (gen_label == NULL);
	      gen_label = symbol_new (FAKE_LABEL_NAME, now_seg, fragP,
				      fr_opcode - fragP->fr_literal
				      + target_offset);
	      break;

	    case ITYPE_INSN:
	      if (first && from_wide_insn)
		{
		  target_offset += xtensa_format_length (isa, fmt);
		  first = false;
		  if (!opcode_fits_format_slot (tinsn->opcode, fmt, slot))
		    target_offset += xg_get_single_size (tinsn->opcode);
		}
	      else
		target_offset += xg_get_single_size (tinsn->opcode);
	      break;
	    }
	}

      total_size = 0;
      first = true;
      for (i = 0; i < istack.ninsn; i++)
	{
	  TInsn *tinsn = &istack.insn[i];
	  fragS *lit_frag;
	  int size;
	  segT target_seg;
	  bfd_reloc_code_real_type reloc_type;

	  switch (tinsn->insn_type)
	    {
	    case ITYPE_LITERAL:
	      lit_frag = fragP->tc_frag_data.literal_frags[slot];
	      /* Already checked.  */
	      gas_assert (lit_frag != NULL);
	      gas_assert (lit_sym != NULL);
	      gas_assert (tinsn->ntok == 1);
	      /* Add a fixup.  */
	      target_seg = S_GET_SEGMENT (lit_sym);
	      gas_assert (target_seg);
	      reloc_type = map_operator_to_reloc (tinsn->tok[0].X_op, true);
	      fix_new_exp_in_seg (target_seg, 0, lit_frag, 0, 4,
				  &tinsn->tok[0], false, reloc_type);
	      break;

	    case ITYPE_LABEL:
	      break;

	    case ITYPE_INSN:
	      xg_resolve_labels (tinsn, gen_label);
	      xg_resolve_literals (tinsn, lit_sym);
	      if (from_wide_insn && first)
		{
		  first = false;
		  if (opcode_fits_format_slot (tinsn->opcode, fmt, slot))
		    {
		      cur_vinsn.slots[slot] = *tinsn;
		    }
		  else
		    {
		      cur_vinsn.slots[slot].opcode =
			xtensa_format_slot_nop_opcode (isa, fmt, slot);
		      cur_vinsn.slots[slot].ntok = 0;
		    }
		  vinsn_to_insnbuf (&cur_vinsn, immed_instr, fragP, true);
		  xtensa_insnbuf_to_chars (isa, cur_vinsn.insnbuf,
					   (unsigned char *) immed_instr, 0);
		  fragP->tc_frag_data.is_insn = true;
		  size = xtensa_format_length (isa, fmt);
		  if (!opcode_fits_format_slot (tinsn->opcode, fmt, slot))
		    {
		      xg_emit_insn_to_buf
			(tinsn, immed_instr + size, fragP,
			 immed_instr - fragP->fr_literal + size, true);
		      size += xg_get_single_size (tinsn->opcode);
		    }
		}
	      else
		{
		  size = xg_get_single_size (tinsn->opcode);
		  xg_emit_insn_to_buf (tinsn, immed_instr, fragP,
				       immed_instr - fragP->fr_literal, true);
		}
	      immed_instr += size;
	      total_size += size;
	      break;
	    }
	}

      diff = total_size - old_size;
      gas_assert (diff >= 0);
      if (diff != 0)
	expanded = true;
      gas_assert (diff <= fragP->fr_var);
      fragP->fr_var -= diff;
      fragP->fr_fix += diff;
    }

  /* Check for undefined immediates in LOOP instructions.  */
  if (is_loop)
    {
      symbolS *sym;
      sym = orig_tinsn.tok[1].X_add_symbol;
      if (sym != NULL && !S_IS_DEFINED (sym))
	{
	  as_bad (_("unresolved loop target symbol: %s"), S_GET_NAME (sym));
	  return;
	}
      sym = orig_tinsn.tok[1].X_op_symbol;
      if (sym != NULL && !S_IS_DEFINED (sym))
	{
	  as_bad (_("unresolved loop target symbol: %s"), S_GET_NAME (sym));
	  return;
	}
    }

  if (expanded && is_direct_call_opcode (orig_tinsn.opcode))
    {
      /* Add an expansion note on the expanded instruction.  */
      fix_new_exp_in_seg (now_seg, 0, fragP, fr_opcode - fragP->fr_literal, 4,
			  &orig_tinsn.tok[0], true,
			  BFD_RELOC_XTENSA_ASM_EXPAND);
    }
}


/* Add a new fix expression into the desired segment.  We have to
   switch to that segment to do this.  */

static fixS *
fix_new_exp_in_seg (segT new_seg,
		    subsegT new_subseg,
		    fragS *frag,
		    int where,
		    int size,
		    expressionS *exp,
		    int pcrel,
		    bfd_reloc_code_real_type r_type)
{
  fixS *new_fix;
  segT seg = now_seg;
  subsegT subseg = now_subseg;

  gas_assert (new_seg != 0);
  subseg_set (new_seg, new_subseg);

  new_fix = fix_new_exp (frag, where, size, exp, pcrel, r_type);
  subseg_set (seg, subseg);
  return new_fix;
}



/* A map that keeps information on a per-subsegment basis.  This is
   maintained during initial assembly, but is invalid once the
   subsegments are smashed together.  I.E., it cannot be used during
   the relaxation.  */

typedef struct subseg_map_struct
{
  /* the key */
  segT seg;
  subsegT subseg;

  /* the data */
  unsigned flags;
  float total_freq;	/* fall-through + branch target frequency */
  float target_freq;	/* branch target frequency alone */

  struct subseg_map_struct *next;
} subseg_map;


static subseg_map *sseg_map = NULL;

static subseg_map *
get_subseg_info (segT seg, subsegT subseg)
{
  subseg_map *subseg_e;

  for (subseg_e = sseg_map; subseg_e; subseg_e = subseg_e->next)
    {
      if (seg == subseg_e->seg && subseg == subseg_e->subseg)
	break;
    }
  return subseg_e;
}


static subseg_map *
add_subseg_info (segT seg, subsegT subseg)
{
  subseg_map *subseg_e = XNEW (subseg_map);
  memset (subseg_e, 0, sizeof (subseg_map));
  subseg_e->seg = seg;
  subseg_e->subseg = subseg;
  subseg_e->flags = 0;
  /* Start off considering every branch target very important.  */
  subseg_e->target_freq = 1.0;
  subseg_e->total_freq = 1.0;
  subseg_e->next = sseg_map;
  sseg_map = subseg_e;
  return subseg_e;
}


static unsigned
get_last_insn_flags (segT seg, subsegT subseg)
{
  subseg_map *subseg_e = get_subseg_info (seg, subseg);
  if (subseg_e)
    return subseg_e->flags;
  return 0;
}


static void
set_last_insn_flags (segT seg,
		     subsegT subseg,
		     unsigned fl,
		     bool val)
{
  subseg_map *subseg_e = get_subseg_info (seg, subseg);
  if (! subseg_e)
    subseg_e = add_subseg_info (seg, subseg);
  if (val)
    subseg_e->flags |= fl;
  else
    subseg_e->flags &= ~fl;
}


static float
get_subseg_total_freq (segT seg, subsegT subseg)
{
  subseg_map *subseg_e = get_subseg_info (seg, subseg);
  if (subseg_e)
    return subseg_e->total_freq;
  return 1.0;
}


static float
get_subseg_target_freq (segT seg, subsegT subseg)
{
  subseg_map *subseg_e = get_subseg_info (seg, subseg);
  if (subseg_e)
    return subseg_e->target_freq;
  return 1.0;
}


static void
set_subseg_freq (segT seg, subsegT subseg, float total_f, float target_f)
{
  subseg_map *subseg_e = get_subseg_info (seg, subseg);
  if (! subseg_e)
    subseg_e = add_subseg_info (seg, subseg);
  subseg_e->total_freq = total_f;
  subseg_e->target_freq = target_f;
}


/* Segment Lists and emit_state Stuff.  */

static void
xtensa_move_seg_list_to_beginning (seg_list *head)
{
  head = head->next;
  while (head)
    {
      segT literal_section = head->seg;

      /* Move the literal section to the front of the section list.  */
      gas_assert (literal_section);
      if (literal_section != stdoutput->sections)
	{
	  bfd_section_list_remove (stdoutput, literal_section);
	  bfd_section_list_prepend (stdoutput, literal_section);
	}
      head = head->next;
    }
}


static void mark_literal_frags (seg_list *);

static void
xg_promote_candidate_litpool (struct litpool_seg *lps,
			      struct litpool_frag *lp)
{
  fragS *poolbeg;
  fragS *poolend;
  symbolS *lsym;
  char label[10 + 2 * sizeof (fragS *)];

  poolbeg = lp->fragP;
  lp->priority = 1;
  poolbeg->fr_subtype = RELAX_LITERAL_POOL_BEGIN;
  poolend = poolbeg->fr_next;
  gas_assert (poolend->fr_type == rs_machine_dependent &&
	      poolend->fr_subtype == RELAX_LITERAL_POOL_END);
  /* Create a local symbol pointing to the
     end of the pool.  */
  sprintf (label, ".L0_LT_%p", poolbeg);
  lsym = (symbolS *) local_symbol_make (label, lps->seg, poolend, 0);
  poolbeg->fr_symbol = lsym;
  /* Rest is done in xtensa_relax_frag.  */
}

static struct litpool_frag *xg_find_litpool (struct litpool_seg *lps,
					     struct litpool_frag *lpf,
					     addressT addr)
{
  struct litpool_frag *lp = lpf->prev;

  gas_assert (lp->fragP);

  while (lp->fragP->fr_subtype == RELAX_LITERAL_POOL_CANDIDATE_BEGIN)
    {
      lp = lp->prev;
      if (lp->fragP == NULL)
	{
	  /* End of list; have to bite the bullet.
	     Take the nearest.  */
	  lp = lpf->prev;
	  break;
	}
      /* Does it (conservatively) reach?  */
      if (addr - lp->addr <= 128 * 1024)
	{
	  if (lp->fragP->fr_subtype == RELAX_LITERAL_POOL_BEGIN &&
	      lp->literal_count < MAX_POOL_LITERALS)
	    {
	      /* Found a good one.  */
	      break;
	    }
	  else if (lp->prev->fragP &&
		   addr - lp->prev->addr > 128 * 1024 &&
		   lp->prev->literal_count < MAX_POOL_LITERALS)
	    {
	      /* This is still a "candidate" but the next one
		 will be too far away, so revert to the nearest
		 one, convert it and add the jump around.  */
	      lp = lpf->prev;
	      break;
	    }
	}
    }

  if (lp->literal_count >= MAX_POOL_LITERALS)
    {
      lp = lpf->prev;
      while (lp && lp->fragP && lp->literal_count >= MAX_POOL_LITERALS)
	{
	  lp = lp->prev;
	}
      gas_assert (lp);
    }

  gas_assert (lp && lp->fragP && lp->literal_count < MAX_POOL_LITERALS);
  ++lp->literal_count;

  /* Convert candidate and add the jump around.  */
  if (lp->fragP->fr_subtype == RELAX_LITERAL_POOL_CANDIDATE_BEGIN)
    xg_promote_candidate_litpool (lps, lp);

  return lp;
}

static bool xtensa_is_init_fini (segT seg)
{
  if (!seg)
    return 0;
  return strcmp (segment_name (seg), INIT_SECTION_NAME) == 0
    || strcmp (segment_name (seg), FINI_SECTION_NAME) == 0;
}

static void
xtensa_assign_litpool_addresses (void)
{
  struct litpool_seg *lps;

  for (lps = litpool_seg_list.next; lps; lps = lps->next)
    {
      frchainS *frchP = seg_info (lps->seg)->frchainP;
      struct litpool_frag *lpf = lps->frag_list.next;
      addressT addr = 0;

      if (xtensa_is_init_fini (lps->seg))
	continue;

      for ( ; frchP; frchP = frchP->frch_next)
	{
	  fragS *fragP;
	  for (fragP = frchP->frch_root; fragP; fragP = fragP->fr_next)
	    {
	      if (lpf && fragP == lpf->fragP)
		{
		  gas_assert(fragP->fr_type == rs_machine_dependent &&
			     (fragP->fr_subtype == RELAX_LITERAL_POOL_BEGIN ||
			      fragP->fr_subtype == RELAX_LITERAL_POOL_CANDIDATE_BEGIN));
		  /* Found a litpool location.  */
		  lpf->addr = addr;
		  lpf = lpf->next;
		}
	      if (fragP->fr_type == rs_machine_dependent &&
		  fragP->fr_subtype == RELAX_SLOTS)
		{
		  int slot;
		  for (slot = 0; slot < MAX_SLOTS; slot++)
		    {
		      fragS *litfrag = fragP->tc_frag_data.literal_frags[slot];

		      if (litfrag
			  && litfrag->tc_frag_data.is_literal
			  && !litfrag->tc_frag_data.literal_frag)
			{
			  /* L32R referring .literal or generated as a result
			     of relaxation.  Point its literal to the nearest
			     litpool preferring non-"candidate" positions to
			     avoid the jump-around.  */

			  struct litpool_frag *lp;

			  lp = xg_find_litpool (lps, lpf, addr);
			  /* Take earliest use of this literal to avoid
			     forward refs.  */
			  litfrag->tc_frag_data.literal_frag = lp->fragP;
			}
		    }
		}
	      addr += fragP->fr_fix;
	      if (fragP->fr_type == rs_fill)
		addr += fragP->fr_offset;
	    }
	}
    }
}

static void
xtensa_move_literals (void)
{
  seg_list *segment;
  frchainS *frchain_from, *frchain_to;
  fragS *search_frag, *next_frag, *literal_pool, *insert_after;
  fragS **frag_splice;
  emit_state state;
  segT dest_seg;
  fixS *fix, *next_fix, **fix_splice;
  sym_list *lit;
  const char *init_name = INIT_SECTION_NAME;
  const char *fini_name = FINI_SECTION_NAME;
  int init_name_len = strlen(init_name);
  int fini_name_len = strlen(fini_name);

  mark_literal_frags (literal_head->next);

  if (use_literal_section)
    return;

  /* Assign addresses (rough estimates) to the potential literal pool locations
     and create new ones if the gaps are too large.  */

  xtensa_assign_litpool_addresses ();

  /* Walk through the literal segments.  */
  for (segment = literal_head->next; segment; segment = segment->next)
    {
      const char *seg_name = segment_name (segment->seg);

      /* Keep the literals for .init and .fini in separate sections.  */
      if ((!memcmp (seg_name, init_name, init_name_len) &&
	   !strcmp (seg_name + init_name_len, ".literal")) ||
	  (!memcmp (seg_name, fini_name, fini_name_len) &&
	   !strcmp (seg_name + fini_name_len, ".literal")))
	continue;

      frchain_from = seg_info (segment->seg)->frchainP;
      search_frag = frchain_from->frch_root;
      literal_pool = NULL;
      frchain_to = NULL;
      frag_splice = &(frchain_from->frch_root);

      while (search_frag && !search_frag->tc_frag_data.literal_frag)
	{
	  gas_assert (search_frag->fr_fix == 0
		  || search_frag->fr_type == rs_align);
	  search_frag = search_frag->fr_next;
	}

      if (!search_frag)
	continue;

      gas_assert (search_frag->tc_frag_data.literal_frag->fr_subtype
	      == RELAX_LITERAL_POOL_BEGIN);
      xtensa_switch_section_emit_state (&state, segment->seg, 0);

      /* Make sure that all the frags in this series are closed, and
	 that there is at least one left over of zero-size.  This
	 prevents us from making a segment with an frchain without any
	 frags in it.  */
      frag_variant (rs_fill, 0, 0, 0, NULL, 0, NULL);
      xtensa_set_frag_assembly_state (frag_now);
      frag_variant (rs_fill, 0, 0, 0, NULL, 0, NULL);
      xtensa_set_frag_assembly_state (frag_now);

      while (search_frag != frag_now)
	{
	  next_frag = search_frag->fr_next;
	  if (search_frag->tc_frag_data.literal_frag)
	    {
	      literal_pool = search_frag->tc_frag_data.literal_frag;
	      gas_assert (literal_pool->fr_subtype == RELAX_LITERAL_POOL_BEGIN);
	      frchain_to = literal_pool->tc_frag_data.lit_frchain;
	      gas_assert (frchain_to);
	    }

	  if (search_frag->fr_type == rs_fill && search_frag->fr_fix == 0)
	    {
	      /* Skip empty fill frags.  */
	      *frag_splice = next_frag;
	      search_frag = next_frag;
	      continue;
	    }

	  if (search_frag->fr_type == rs_align)
	    {
	      /* Skip alignment frags, because the pool as a whole will be
	         aligned if used, and we don't want to force alignment if the
		 pool is unused.  */
	      *frag_splice = next_frag;
	      search_frag = next_frag;
	      continue;
	    }

	  /* First, move the frag out of the literal section and
	     to the appropriate place.  */

	  /* Insert an alignment frag at start of pool.  */
	  if (literal_pool->fr_next->fr_type == rs_machine_dependent &&
	      literal_pool->fr_next->fr_subtype == RELAX_LITERAL_POOL_END)
	    {
	      segT pool_seg = literal_pool->fr_next->tc_frag_data.lit_seg;
	      emit_state prev_state;
	      fragS *prev_frag;
	      fragS *align_frag;
	      xtensa_switch_section_emit_state (&prev_state, pool_seg, 0);
	      prev_frag = frag_now;
	      frag_variant (rs_fill, 0, 0, 0, NULL, 0, NULL);
	      align_frag = frag_now;
	      frag_align (2, 0, 0);
	      /* Splice it into the right place.  */
	      prev_frag->fr_next = align_frag->fr_next;
	      align_frag->fr_next = literal_pool->fr_next;
	      literal_pool->fr_next = align_frag;
	      /* Insert after this one.  */
	      literal_pool->tc_frag_data.literal_frag = align_frag;
	      xtensa_restore_emit_state (&prev_state);
	    }
	  insert_after = literal_pool->tc_frag_data.literal_frag;
	  dest_seg = insert_after->fr_next->tc_frag_data.lit_seg;
	  /* Skip align frag.  */
	  if (insert_after->fr_next->fr_type == rs_align)
	    {
	      insert_after = insert_after->fr_next;
	    }

	  *frag_splice = next_frag;
	  search_frag->fr_next = insert_after->fr_next;
	  insert_after->fr_next = search_frag;
	  search_frag->tc_frag_data.lit_seg = dest_seg;
	  literal_pool->tc_frag_data.literal_frag = search_frag;

	  /* Now move any fixups associated with this frag to the
	     right section.  */
	  fix = frchain_from->fix_root;
	  fix_splice = &(frchain_from->fix_root);
	  while (fix)
	    {
	      next_fix = fix->fx_next;
	      if (fix->fx_frag == search_frag)
		{
		  *fix_splice = next_fix;
		  fix->fx_next = frchain_to->fix_root;
		  frchain_to->fix_root = fix;
		  if (frchain_to->fix_tail == NULL)
		    frchain_to->fix_tail = fix;
		}
	      else
		fix_splice = &(fix->fx_next);
	      fix = next_fix;
	    }
	  search_frag = next_frag;
	}

      if (frchain_from->fix_root != NULL)
	{
	  frchain_from = seg_info (segment->seg)->frchainP;
	  as_warn (_("fixes not all moved from %s"), segment->seg->name);

	  gas_assert (frchain_from->fix_root == NULL);
	}
      frchain_from->fix_tail = NULL;
      xtensa_restore_emit_state (&state);
    }

  /* Now fix up the SEGMENT value for all the literal symbols.  */
  for (lit = literal_syms; lit; lit = lit->next)
    {
      symbolS *lit_sym = lit->sym;
      segT dseg = symbol_get_frag (lit_sym)->tc_frag_data.lit_seg;
      if (dseg)
	S_SET_SEGMENT (lit_sym, dseg);
    }
}


/* Walk over all the frags for segments in a list and mark them as
   containing literals.  As clunky as this is, we can't rely on frag_var
   and frag_variant to get called in all situations.  */

static void
mark_literal_frags (seg_list *segment)
{
  frchainS *frchain_from;
  fragS *search_frag;

  while (segment)
    {
      frchain_from = seg_info (segment->seg)->frchainP;
      search_frag = frchain_from->frch_root;
      while (search_frag)
	{
	  search_frag->tc_frag_data.is_literal = true;
	  search_frag = search_frag->fr_next;
	}
      segment = segment->next;
    }
}


static void
xtensa_reorder_seg_list (seg_list *head, segT after)
{
  /* Move all of the sections in the section list to come
     after "after" in the gnu segment list.  */

  head = head->next;
  while (head)
    {
      segT literal_section = head->seg;

      /* Move the literal section after "after".  */
      gas_assert (literal_section);
      if (literal_section != after)
	{
	  bfd_section_list_remove (stdoutput, literal_section);
	  bfd_section_list_insert_after (stdoutput, after, literal_section);
	}

      head = head->next;
    }
}


/* Push all the literal segments to the end of the gnu list.  */

static void
xtensa_reorder_segments (void)
{
  segT sec;
  segT last_sec = 0;
  int old_count = 0;
  int new_count = 0;

  for (sec = stdoutput->sections; sec != NULL; sec = sec->next)
    {
      last_sec = sec;
      old_count++;
    }

  /* Now that we have the last section, push all the literal
     sections to the end.  */
  xtensa_reorder_seg_list (literal_head, last_sec);

  /* Now perform the final error check.  */
  for (sec = stdoutput->sections; sec != NULL; sec = sec->next)
    new_count++;
  gas_assert (new_count == old_count);
}


/* Change the emit state (seg, subseg, and frag related stuff) to the
   correct location.  Return a emit_state which can be passed to
   xtensa_restore_emit_state to return to current fragment.  */

static void
xtensa_switch_to_literal_fragment (emit_state *result)
{
  if (directive_state[directive_absolute_literals])
    {
      segT lit4_seg = cache_literal_section (true);
      xtensa_switch_section_emit_state (result, lit4_seg, 0);
    }
  else
    xtensa_switch_to_non_abs_literal_fragment (result);

  /* Do a 4-byte align here.  */
  frag_align (2, 0, 0);
  record_alignment (now_seg, 2);
}


static void
xtensa_switch_to_non_abs_literal_fragment (emit_state *result)
{
  fragS *pool_location = get_literal_pool_location (now_seg);
  segT lit_seg;
  bool is_init_fini = xtensa_is_init_fini (now_seg);

  if (pool_location == NULL
      && !use_literal_section
      && !is_init_fini)
    {
      if (!auto_litpools)
	{
	  as_bad (_("literal pool location required for text-section-literals; specify with .literal_position"));
	}
      xtensa_maybe_create_literal_pool_frag (true, true);
      pool_location = get_literal_pool_location (now_seg);
    }

  lit_seg = cache_literal_section (false);
  xtensa_switch_section_emit_state (result, lit_seg, 0);

  if (!use_literal_section
      && !is_init_fini
      && get_literal_pool_location (now_seg) != pool_location)
    {
      /* Close whatever frag is there.  */
      frag_variant (rs_fill, 0, 0, 0, NULL, 0, NULL);
      xtensa_set_frag_assembly_state (frag_now);
      frag_now->tc_frag_data.literal_frag = pool_location;
      frag_variant (rs_fill, 0, 0, 0, NULL, 0, NULL);
      xtensa_set_frag_assembly_state (frag_now);
    }
}


/* Call this function before emitting data into the literal section.
   This is a helper function for xtensa_switch_to_literal_fragment.
   This is similar to a .section new_now_seg subseg. */

static void
xtensa_switch_section_emit_state (emit_state *state,
				  segT new_now_seg,
				  subsegT new_now_subseg)
{
  state->name = now_seg->name;
  state->now_seg = now_seg;
  state->now_subseg = now_subseg;
  state->generating_literals = generating_literals;
  generating_literals++;
  subseg_set (new_now_seg, new_now_subseg);
}


/* Use to restore the emitting into the normal place.  */

static void
xtensa_restore_emit_state (emit_state *state)
{
  generating_literals = state->generating_literals;
  subseg_set (state->now_seg, state->now_subseg);
}


/* Predicate function used to look up a section in a particular group.  */

static bool
match_section_group (bfd *abfd ATTRIBUTE_UNUSED, asection *sec, void *inf)
{
  const char *gname = inf;
  const char *group_name = elf_group_name (sec);

  return (group_name == gname
	  || (group_name != NULL
	      && gname != NULL
	      && strcmp (group_name, gname) == 0));
}


/* Get the literal section to be used for the current text section.
   The result may be cached in the default_lit_sections structure.  */

static segT
cache_literal_section (bool use_abs_literals)
{
  const char *text_name, *group_name = 0;
  const char *base_name, *suffix;
  char *name;
  segT *pcached;
  segT seg, current_section;
  int current_subsec;
  bool linkonce = false;

  /* Save the current section/subsection.  */
  current_section = now_seg;
  current_subsec = now_subseg;

  /* Clear the cached values if they are no longer valid.  */
  if (now_seg != default_lit_sections.current_text_seg)
    {
      default_lit_sections.current_text_seg = now_seg;
      default_lit_sections.lit_seg = NULL;
      default_lit_sections.lit4_seg = NULL;
    }

  /* Check if the literal section is already cached.  */
  if (use_abs_literals)
    pcached = &default_lit_sections.lit4_seg;
  else
    pcached = &default_lit_sections.lit_seg;

  if (*pcached)
    return *pcached;

  text_name = default_lit_sections.lit_prefix;
  if (! text_name || ! *text_name)
    {
      text_name = segment_name (current_section);
      group_name = elf_group_name (current_section);
      linkonce = (current_section->flags & SEC_LINK_ONCE) != 0;
    }

  base_name = use_abs_literals ? ".lit4" : ".literal";
  if (group_name)
    {
      name = concat (base_name, ".", group_name, (char *) NULL);
    }
  else if (strncmp (text_name, ".gnu.linkonce.", linkonce_len) == 0)
    {
      suffix = strchr (text_name + linkonce_len, '.');

      name = concat (".gnu.linkonce", base_name, suffix ? suffix : "",
		     (char *) NULL);
      linkonce = true;
    }
  else
    {
      /* If the section name begins or ends with ".text", then replace
	 that portion instead of appending an additional suffix.  */
      size_t len = strlen (text_name);
      if (len >= 5
	  && (strcmp (text_name + len - 5, ".text") == 0
	      || startswith (text_name, ".text")))
	len -= 5;

      name = XNEWVEC (char, len + strlen (base_name) + 1);
      if (startswith (text_name, ".text"))
	{
	  strcpy (name, base_name);
	  strcat (name, text_name + 5);
	}
      else
	{
	  strcpy (name, text_name);
	  strcpy (name + len, base_name);
	}
    }

  /* Canonicalize section names to allow renaming literal sections.
     The group name, if any, came from the current text section and
     has already been canonicalized.  */
  name = tc_canonicalize_symbol_name (name);

  seg = bfd_get_section_by_name_if (stdoutput, name, match_section_group,
				    (void *) group_name);
  if (! seg)
    {
      flagword flags;

      seg = subseg_force_new (name, 0);

      if (! use_abs_literals)
	{
	  /* Add the newly created literal segment to the list.  */
	  seg_list *n = XNEW (seg_list);
	  n->seg = seg;
	  n->next = literal_head->next;
	  literal_head->next = n;
	}

      flags = (SEC_HAS_CONTENTS | SEC_READONLY | SEC_ALLOC | SEC_LOAD
	       | (linkonce ? (SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD) : 0)
	       | (use_abs_literals ? SEC_DATA : SEC_CODE));

      elf_group_name (seg) = group_name;

      bfd_set_section_flags (seg, flags);
      bfd_set_section_alignment (seg, 2);
    }

  *pcached = seg;
  subseg_set (current_section, current_subsec);
  return seg;
}


/* Property Tables Stuff.  */

#define XTENSA_INSN_SEC_NAME ".xt.insn"
#define XTENSA_LIT_SEC_NAME ".xt.lit"
#define XTENSA_PROP_SEC_NAME ".xt.prop"

typedef bool (*frag_predicate) (const fragS *);
typedef void (*frag_flags_fn) (const fragS *, frag_flags *);

static bool get_frag_is_literal (const fragS *);
static void xtensa_create_property_segments
  (frag_predicate, frag_predicate, const char *, xt_section_type);
static void xtensa_create_xproperty_segments
  (frag_flags_fn, const char *, xt_section_type);
static bool exclude_section_from_property_tables (segT);
static bool section_has_property (segT, frag_predicate);
static bool section_has_xproperty (segT, frag_flags_fn);
static void add_xt_block_frags
  (segT, xtensa_block_info **, frag_predicate, frag_predicate);
static bool xtensa_frag_flags_is_empty (const frag_flags *);
static void xtensa_frag_flags_init (frag_flags *);
static void get_frag_property_flags (const fragS *, frag_flags *);
static flagword frag_flags_to_number (const frag_flags *);
static void add_xt_prop_frags (segT, xtensa_block_info **, frag_flags_fn);

/* Set up property tables after relaxation.  */

void
xtensa_post_relax_hook (void)
{
  xtensa_move_seg_list_to_beginning (literal_head);

  xtensa_find_unmarked_state_frags ();
  xtensa_mark_frags_for_org ();
  xtensa_mark_difference_of_two_symbols ();

  xtensa_create_property_segments (get_frag_is_literal,
				   NULL,
				   XTENSA_LIT_SEC_NAME,
				   xt_literal_sec);
  xtensa_create_xproperty_segments (get_frag_property_flags,
				    XTENSA_PROP_SEC_NAME,
				    xt_prop_sec);

  if (warn_unaligned_branch_targets)
    bfd_map_over_sections (stdoutput, xtensa_find_unaligned_branch_targets, 0);
  bfd_map_over_sections (stdoutput, xtensa_find_unaligned_loops, 0);
}


/* This function is only meaningful after xtensa_move_literals.  */

static bool
get_frag_is_literal (const fragS *fragP)
{
  gas_assert (fragP != NULL);
  return fragP->tc_frag_data.is_literal;
}


static void
xtensa_create_property_segments (frag_predicate property_function,
				 frag_predicate end_property_function,
				 const char *section_name_base,
				 xt_section_type sec_type)
{
  segT *seclist;

  /* Walk over all of the current segments.
     Walk over each fragment
     For each non-empty fragment,
     Build a property record (append where possible).  */

  for (seclist = &stdoutput->sections;
       seclist && *seclist;
       seclist = &(*seclist)->next)
    {
      segT sec = *seclist;

      if (exclude_section_from_property_tables (sec))
	continue;

      if (section_has_property (sec, property_function))
	{
	  segment_info_type *xt_seg_info;
	  xtensa_block_info **xt_blocks;
	  segT prop_sec = xtensa_make_property_section (sec, section_name_base);

	  prop_sec->output_section = prop_sec;
	  subseg_set (prop_sec, 0);
	  xt_seg_info = seg_info (prop_sec);
	  xt_blocks = &xt_seg_info->tc_segment_info_data.blocks[sec_type];

	  /* Walk over all of the frchains here and add new sections.  */
	  add_xt_block_frags (sec, xt_blocks, property_function,
			      end_property_function);
	}
    }

  /* Now we fill them out....  */

  for (seclist = &stdoutput->sections;
       seclist && *seclist;
       seclist = &(*seclist)->next)
    {
      segment_info_type *seginfo;
      xtensa_block_info *block;
      segT sec = *seclist;

      seginfo = seg_info (sec);
      block = seginfo->tc_segment_info_data.blocks[sec_type];

      if (block)
	{
	  xtensa_block_info *cur_block;
	  int num_recs = 0;
	  bfd_size_type rec_size;

	  for (cur_block = block; cur_block; cur_block = cur_block->next)
	    num_recs++;

	  rec_size = num_recs * 8;
	  bfd_set_section_size (sec, rec_size);

	  if (num_recs)
	    {
	      char *frag_data;
	      int i;

	      subseg_set (sec, 0);
	      frag_data = frag_more (rec_size);
	      cur_block = block;
	      for (i = 0; i < num_recs; i++)
		{
		  fixS *fix;

		  /* Write the fixup.  */
		  gas_assert (cur_block);
		  fix = fix_new (frag_now, i * 8, 4,
				 section_symbol (cur_block->sec),
				 cur_block->offset,
				 false, BFD_RELOC_32);
		  fix->fx_file = "<internal>";
		  fix->fx_line = 0;

		  /* Write the length.  */
		  md_number_to_chars (&frag_data[4 + i * 8],
				      cur_block->size, 4);
		  cur_block = cur_block->next;
		}
	      frag_wane (frag_now);
	      frag_new (0);
	      frag_wane (frag_now);
	    }
	}
    }
}


static void
xtensa_create_xproperty_segments (frag_flags_fn flag_fn,
				  const char *section_name_base,
				  xt_section_type sec_type)
{
  segT *seclist;

  /* Walk over all of the current segments.
     Walk over each fragment.
     For each fragment that has instructions,
     build an instruction record (append where possible).  */

  for (seclist = &stdoutput->sections;
       seclist && *seclist;
       seclist = &(*seclist)->next)
    {
      segT sec = *seclist;

      if (exclude_section_from_property_tables (sec))
	continue;

      if (section_has_xproperty (sec, flag_fn))
	{
	  segment_info_type *xt_seg_info;
	  xtensa_block_info **xt_blocks;
	  segT prop_sec = xtensa_make_property_section (sec, section_name_base);

	  prop_sec->output_section = prop_sec;
	  subseg_set (prop_sec, 0);
	  xt_seg_info = seg_info (prop_sec);
	  xt_blocks = &xt_seg_info->tc_segment_info_data.blocks[sec_type];

	  /* Walk over all of the frchains here and add new sections.  */
	  add_xt_prop_frags (sec, xt_blocks, flag_fn);
	}
    }

  /* Now we fill them out....  */

  for (seclist = &stdoutput->sections;
       seclist && *seclist;
       seclist = &(*seclist)->next)
    {
      segment_info_type *seginfo;
      xtensa_block_info *block;
      segT sec = *seclist;

      seginfo = seg_info (sec);
      block = seginfo->tc_segment_info_data.blocks[sec_type];

      if (block)
	{
	  xtensa_block_info *cur_block;
	  int num_recs = 0;
	  bfd_size_type rec_size;

	  for (cur_block = block; cur_block; cur_block = cur_block->next)
	    num_recs++;

	  rec_size = num_recs * (8 + 4);
	  bfd_set_section_size (sec, rec_size);
	  /* elf_section_data (sec)->this_hdr.sh_entsize = 12; */

	  if (num_recs)
	    {
	      char *frag_data;
	      int i;

	      subseg_set (sec, 0);
	      frag_data = frag_more (rec_size);
	      cur_block = block;
	      for (i = 0; i < num_recs; i++)
		{
		  fixS *fix;

		  /* Write the fixup.  */
		  gas_assert (cur_block);
		  fix = fix_new (frag_now, i * 12, 4,
				 section_symbol (cur_block->sec),
				 cur_block->offset,
				 false, BFD_RELOC_32);
		  fix->fx_file = "<internal>";
		  fix->fx_line = 0;

		  /* Write the length.  */
		  md_number_to_chars (&frag_data[4 + i * 12],
				      cur_block->size, 4);
		  md_number_to_chars (&frag_data[8 + i * 12],
				      frag_flags_to_number (&cur_block->flags),
				      sizeof (flagword));
		  cur_block = cur_block->next;
		}
	      frag_wane (frag_now);
	      frag_new (0);
	      frag_wane (frag_now);
	    }
	}
    }
}


static bool
exclude_section_from_property_tables (segT sec)
{
  flagword flags = bfd_section_flags (sec);

  /* Sections that don't contribute to the memory footprint are excluded.  */
  if ((flags & SEC_DEBUGGING)
      || !(flags & SEC_ALLOC)
      || (flags & SEC_MERGE))
    return true;

  /* Linker cie and fde optimizations mess up property entries for
     eh_frame sections, but there is nothing inside them relevant to
     property tables anyway.  */
  if (strcmp (sec->name, ".eh_frame") == 0)
    return true;

  return false;
}


static bool
section_has_property (segT sec, frag_predicate property_function)
{
  segment_info_type *seginfo = seg_info (sec);
  fragS *fragP;

  if (seginfo && seginfo->frchainP)
    {
      for (fragP = seginfo->frchainP->frch_root; fragP; fragP = fragP->fr_next)
	{
	  if (property_function (fragP)
	      && (fragP->fr_type != rs_fill || fragP->fr_fix != 0))
	    return true;
	}
    }
  return false;
}


static bool
section_has_xproperty (segT sec, frag_flags_fn property_function)
{
  segment_info_type *seginfo = seg_info (sec);
  fragS *fragP;

  if (seginfo && seginfo->frchainP)
    {
      for (fragP = seginfo->frchainP->frch_root; fragP; fragP = fragP->fr_next)
	{
	  frag_flags prop_flags;
	  property_function (fragP, &prop_flags);
	  if (!xtensa_frag_flags_is_empty (&prop_flags))
	    return true;
	}
    }
  return false;
}


/* Two types of block sections exist right now: literal and insns.  */

static void
add_xt_block_frags (segT sec,
		    xtensa_block_info **xt_block,
		    frag_predicate property_function,
		    frag_predicate end_property_function)
{
  fragS *fragP;

  /* Build it if needed.  */
  while (*xt_block != NULL)
    xt_block = &(*xt_block)->next;
  /* We are either at NULL at the beginning or at the end.  */

  /* Walk through the frags.  */
  if (seg_info (sec)->frchainP)
    {
      for (fragP = seg_info (sec)->frchainP->frch_root;
	   fragP;
	   fragP = fragP->fr_next)
	{
	  if (property_function (fragP)
	      && (fragP->fr_type != rs_fill || fragP->fr_fix != 0))
	    {
	      if (*xt_block != NULL)
		{
		  if ((*xt_block)->offset + (*xt_block)->size
		      == fragP->fr_address)
		    (*xt_block)->size += fragP->fr_fix;
		  else
		    xt_block = &((*xt_block)->next);
		}
	      if (*xt_block == NULL)
		{
		  xtensa_block_info *new_block = XNEW (xtensa_block_info);
		  new_block->sec = sec;
		  new_block->offset = fragP->fr_address;
		  new_block->size = fragP->fr_fix;
		  new_block->next = NULL;
		  xtensa_frag_flags_init (&new_block->flags);
		  *xt_block = new_block;
		}
	      if (end_property_function
		  && end_property_function (fragP))
		{
		  xt_block = &((*xt_block)->next);
		}
	    }
	}
    }
}


/* Break the encapsulation of add_xt_prop_frags here.  */

static bool
xtensa_frag_flags_is_empty (const frag_flags *prop_flags)
{
  if (prop_flags->is_literal
      || prop_flags->is_insn
      || prop_flags->is_data
      || prop_flags->is_unreachable)
    return false;
  return true;
}


static void
xtensa_frag_flags_init (frag_flags *prop_flags)
{
  memset (prop_flags, 0, sizeof (frag_flags));
}


static void
get_frag_property_flags (const fragS *fragP, frag_flags *prop_flags)
{
  xtensa_frag_flags_init (prop_flags);
  if (fragP->tc_frag_data.is_literal)
    prop_flags->is_literal = true;
  if (fragP->tc_frag_data.is_specific_opcode
      || fragP->tc_frag_data.is_no_transform)
    {
      prop_flags->is_no_transform = true;
      if (xtensa_frag_flags_is_empty (prop_flags))
	prop_flags->is_data = true;
    }
  if (fragP->tc_frag_data.is_unreachable)
    prop_flags->is_unreachable = true;
  else if (fragP->tc_frag_data.is_insn)
    {
      prop_flags->is_insn = true;
      if (fragP->tc_frag_data.is_loop_target)
	prop_flags->insn.is_loop_target = true;
      if (fragP->tc_frag_data.is_branch_target)
	prop_flags->insn.is_branch_target = true;
      if (fragP->tc_frag_data.is_no_density)
	prop_flags->insn.is_no_density = true;
      if (fragP->tc_frag_data.use_absolute_literals)
	prop_flags->insn.is_abslit = true;
    }
  if (fragP->tc_frag_data.is_align)
    {
      prop_flags->is_align = true;
      prop_flags->alignment = fragP->tc_frag_data.alignment;
      if (xtensa_frag_flags_is_empty (prop_flags))
	prop_flags->is_data = true;
    }
}


static flagword
frag_flags_to_number (const frag_flags *prop_flags)
{
  flagword num = 0;
  if (prop_flags->is_literal)
    num |= XTENSA_PROP_LITERAL;
  if (prop_flags->is_insn)
    num |= XTENSA_PROP_INSN;
  if (prop_flags->is_data)
    num |= XTENSA_PROP_DATA;
  if (prop_flags->is_unreachable)
    num |= XTENSA_PROP_UNREACHABLE;
  if (prop_flags->insn.is_loop_target)
    num |= XTENSA_PROP_INSN_LOOP_TARGET;
  if (prop_flags->insn.is_branch_target)
    {
      num |= XTENSA_PROP_INSN_BRANCH_TARGET;
      num = SET_XTENSA_PROP_BT_ALIGN (num, prop_flags->insn.bt_align_priority);
    }

  if (prop_flags->insn.is_no_density)
    num |= XTENSA_PROP_INSN_NO_DENSITY;
  if (prop_flags->is_no_transform)
    num |= XTENSA_PROP_NO_TRANSFORM;
  if (prop_flags->insn.is_no_reorder)
    num |= XTENSA_PROP_INSN_NO_REORDER;
  if (prop_flags->insn.is_abslit)
    num |= XTENSA_PROP_INSN_ABSLIT;

  if (prop_flags->is_align)
    {
      num |= XTENSA_PROP_ALIGN;
      num = SET_XTENSA_PROP_ALIGNMENT (num, prop_flags->alignment);
    }

  return num;
}


static bool
xtensa_frag_flags_combinable (const frag_flags *prop_flags_1,
			      const frag_flags *prop_flags_2)
{
  /* Cannot combine with an end marker.  */

  if (prop_flags_1->is_literal != prop_flags_2->is_literal)
    return false;
  if (prop_flags_1->is_insn != prop_flags_2->is_insn)
    return false;
  if (prop_flags_1->is_data != prop_flags_2->is_data)
    return false;

  if (prop_flags_1->is_insn)
    {
      /* Properties of the beginning of the frag.  */
      if (prop_flags_2->insn.is_loop_target)
	return false;
      if (prop_flags_2->insn.is_branch_target)
	return false;
      if (prop_flags_1->insn.is_no_density !=
	  prop_flags_2->insn.is_no_density)
	return false;
      if (prop_flags_1->is_no_transform !=
	  prop_flags_2->is_no_transform)
	return false;
      if (prop_flags_1->insn.is_no_reorder !=
	  prop_flags_2->insn.is_no_reorder)
	return false;
      if (prop_flags_1->insn.is_abslit !=
	  prop_flags_2->insn.is_abslit)
	return false;
    }

  if (prop_flags_1->is_align)
    return false;

  return true;
}


static bfd_vma
xt_block_aligned_size (const xtensa_block_info *xt_block)
{
  bfd_vma end_addr;
  unsigned align_bits;

  if (!xt_block->flags.is_align)
    return xt_block->size;

  end_addr = xt_block->offset + xt_block->size;
  align_bits = xt_block->flags.alignment;
  end_addr = ((end_addr + ((1 << align_bits) -1)) >> align_bits) << align_bits;
  return end_addr - xt_block->offset;
}


static bool
xtensa_xt_block_combine (xtensa_block_info *xt_block,
			 const xtensa_block_info *xt_block_2)
{
  if (xt_block->sec != xt_block_2->sec)
    return false;
  if (xt_block->offset + xt_block_aligned_size (xt_block)
      != xt_block_2->offset)
    return false;

  if (xt_block_2->size == 0
      && (!xt_block_2->flags.is_unreachable
	  || xt_block->flags.is_unreachable))
    {
      if (xt_block_2->flags.is_align
	  && xt_block->flags.is_align)
	{
	  /* Nothing needed.  */
	  if (xt_block->flags.alignment >= xt_block_2->flags.alignment)
	    return true;
	}
      else
	{
	  if (xt_block_2->flags.is_align)
	    {
	      /* Push alignment to previous entry.  */
	      xt_block->flags.is_align = xt_block_2->flags.is_align;
	      xt_block->flags.alignment = xt_block_2->flags.alignment;
	    }
	  return true;
	}
    }
  if (!xtensa_frag_flags_combinable (&xt_block->flags,
				     &xt_block_2->flags))
    return false;

  xt_block->size += xt_block_2->size;

  if (xt_block_2->flags.is_align)
    {
      xt_block->flags.is_align = true;
      xt_block->flags.alignment = xt_block_2->flags.alignment;
    }

  return true;
}


static void
add_xt_prop_frags (segT sec,
		   xtensa_block_info **xt_block,
		   frag_flags_fn property_function)
{
  fragS *fragP;

  /* Build it if needed.  */
  while (*xt_block != NULL)
    {
      xt_block = &(*xt_block)->next;
    }
  /* We are either at NULL at the beginning or at the end.  */

  /* Walk through the frags.  */
  if (seg_info (sec)->frchainP)
    {
      for (fragP = seg_info (sec)->frchainP->frch_root; fragP;
	   fragP = fragP->fr_next)
	{
	  xtensa_block_info tmp_block;
	  tmp_block.sec = sec;
	  tmp_block.offset = fragP->fr_address;
	  tmp_block.size = fragP->fr_fix;
	  tmp_block.next = NULL;
	  property_function (fragP, &tmp_block.flags);

	  if (!xtensa_frag_flags_is_empty (&tmp_block.flags))
	    /* && fragP->fr_fix != 0) */
	    {
	      if ((*xt_block) == NULL
		  || !xtensa_xt_block_combine (*xt_block, &tmp_block))
		{
		  xtensa_block_info *new_block;
		  if ((*xt_block) != NULL)
		    xt_block = &(*xt_block)->next;
		  new_block = XNEW (xtensa_block_info);
		  *new_block = tmp_block;
		  *xt_block = new_block;
		}
	    }
	}
    }
}


/* op_placement_info_table */

/* op_placement_info makes it easier to determine which
   ops can go in which slots.  */

static void
init_op_placement_info_table (void)
{
  xtensa_isa isa = xtensa_default_isa;
  xtensa_insnbuf ibuf = xtensa_insnbuf_alloc (isa);
  xtensa_opcode opcode;
  xtensa_format fmt;
  int slot;
  int num_opcodes = xtensa_isa_num_opcodes (isa);

  op_placement_table = XNEWVEC (op_placement_info, num_opcodes);
  gas_assert (xtensa_isa_num_formats (isa) < MAX_FORMATS);

  for (opcode = 0; opcode < num_opcodes; opcode++)
    {
      op_placement_info *opi = &op_placement_table[opcode];
      /* FIXME: Make tinsn allocation dynamic.  */
      if (xtensa_opcode_num_operands (isa, opcode) > MAX_INSN_ARGS)
	as_fatal (_("too many operands in instruction"));
      opi->narrowest = XTENSA_UNDEFINED;
      opi->narrowest_size = 0x7F;
      opi->narrowest_slot = 0;
      opi->formats = 0;
      opi->num_formats = 0;
      opi->issuef = 0;
      for (fmt = 0; fmt < xtensa_isa_num_formats (isa); fmt++)
	{
	  opi->slots[fmt] = 0;
	  for (slot = 0; slot < xtensa_format_num_slots (isa, fmt); slot++)
	    {
	      if (xtensa_opcode_encode (isa, fmt, slot, ibuf, opcode) == 0)
		{
		  int fmt_length = xtensa_format_length (isa, fmt);
		  opi->issuef++;
		  set_bit (fmt, opi->formats);
		  set_bit (slot, opi->slots[fmt]);
		  if (fmt_length < opi->narrowest_size
		      || (fmt_length == opi->narrowest_size
			  && (xtensa_format_num_slots (isa, fmt)
			      < xtensa_format_num_slots (isa,
							 opi->narrowest))))
		    {
		      opi->narrowest = fmt;
		      opi->narrowest_size = fmt_length;
		      opi->narrowest_slot = slot;
		    }
		}
	    }
	  if (opi->formats)
	    opi->num_formats++;
	}
    }
  xtensa_insnbuf_free (isa, ibuf);
}


bool
opcode_fits_format_slot (xtensa_opcode opcode, xtensa_format fmt, int slot)
{
  return bit_is_set (slot, op_placement_table[opcode].slots[fmt]);
}


/* If the opcode is available in a single slot format, return its size.  */

static int
xg_get_single_size (xtensa_opcode opcode)
{
  return op_placement_table[opcode].narrowest_size;
}


static xtensa_format
xg_get_single_format (xtensa_opcode opcode)
{
  return op_placement_table[opcode].narrowest;
}


static int
xg_get_single_slot (xtensa_opcode opcode)
{
  return op_placement_table[opcode].narrowest_slot;
}


/* Instruction Stack Functions (from "xtensa-istack.h").  */

void
istack_init (IStack *stack)
{
  stack->ninsn = 0;
}


bool
istack_empty (IStack *stack)
{
  return (stack->ninsn == 0);
}


bool
istack_full (IStack *stack)
{
  return (stack->ninsn == MAX_ISTACK);
}


/* Return a pointer to the top IStack entry.
   It is an error to call this if istack_empty () is TRUE. */

TInsn *
istack_top (IStack *stack)
{
  int rec = stack->ninsn - 1;
  gas_assert (!istack_empty (stack));
  return &stack->insn[rec];
}


/* Add a new TInsn to an IStack.
   It is an error to call this if istack_full () is TRUE.  */

void
istack_push (IStack *stack, TInsn *insn)
{
  int rec = stack->ninsn;
  gas_assert (!istack_full (stack));
  stack->insn[rec] = *insn;
  stack->ninsn++;
}


/* Clear space for the next TInsn on the IStack and return a pointer
   to it.  It is an error to call this if istack_full () is TRUE.  */

TInsn *
istack_push_space (IStack *stack)
{
  int rec = stack->ninsn;
  TInsn *insn;
  gas_assert (!istack_full (stack));
  insn = &stack->insn[rec];
  tinsn_init (insn);
  stack->ninsn++;
  return insn;
}


/* Remove the last pushed instruction.  It is an error to call this if
   istack_empty () returns TRUE.  */

void
istack_pop (IStack *stack)
{
  int rec = stack->ninsn - 1;
  gas_assert (!istack_empty (stack));
  stack->ninsn--;
  tinsn_init (&stack->insn[rec]);
}


/* TInsn functions.  */

void
tinsn_init (TInsn *dst)
{
  memset (dst, 0, sizeof (TInsn));
}


/* Return TRUE if ANY of the operands in the insn are symbolic.  */

static bool
tinsn_has_symbolic_operands (const TInsn *insn)
{
  int i;
  int n = insn->ntok;

  gas_assert (insn->insn_type == ITYPE_INSN);

  for (i = 0; i < n; ++i)
    {
      switch (insn->tok[i].X_op)
	{
	case O_register:
	case O_constant:
	  break;
	default:
	  return true;
	}
    }
  return false;
}


bool
tinsn_has_invalid_symbolic_operands (const TInsn *insn)
{
  xtensa_isa isa = xtensa_default_isa;
  int i;
  int n = insn->ntok;

  gas_assert (insn->insn_type == ITYPE_INSN);

  for (i = 0; i < n; ++i)
    {
      switch (insn->tok[i].X_op)
	{
	case O_register:
	case O_constant:
	  break;
	case O_big:
	case O_illegal:
	case O_absent:
	  /* Errors for these types are caught later.  */
	  break;
	case O_hi16:
	case O_lo16:
	default:
	  /* Symbolic immediates are only allowed on the last immediate
	     operand.  At this time, CONST16 is the only opcode where we
	     support non-PC-relative relocations.  */
	  if (i != get_relaxable_immed (insn->opcode)
	      || (xtensa_operand_is_PCrelative (isa, insn->opcode, i) != 1
		  && insn->opcode != xtensa_const16_opcode))
	    {
	      as_bad (_("invalid symbolic operand"));
	      return true;
	    }
	}
    }
  return false;
}


/* For assembly code with complex expressions (e.g. subtraction),
   we have to build them in the literal pool so that
   their results are calculated correctly after relaxation.
   The relaxation only handles expressions that
   boil down to SYMBOL + OFFSET.  */

static bool
tinsn_has_complex_operands (const TInsn *insn)
{
  int i;
  int n = insn->ntok;
  gas_assert (insn->insn_type == ITYPE_INSN);
  for (i = 0; i < n; ++i)
    {
      switch (insn->tok[i].X_op)
	{
	case O_register:
	case O_constant:
	case O_symbol:
	case O_lo16:
	case O_hi16:
	  break;
	default:
	  return true;
	}
    }
  return false;
}


/* Encode a TInsn opcode and its constant operands into slotbuf.
   Return TRUE if there is a symbol in the immediate field.  This
   function assumes that:
   1) The number of operands are correct.
   2) The insn_type is ITYPE_INSN.
   3) The opcode can be encoded in the specified format and slot.
   4) Operands are either O_constant or O_symbol, and all constants fit.  */

static bool
tinsn_to_slotbuf (xtensa_format fmt,
		  int slot,
		  TInsn *tinsn,
		  xtensa_insnbuf slotbuf)
{
  xtensa_isa isa = xtensa_default_isa;
  xtensa_opcode opcode = tinsn->opcode;
  bool has_fixup = false;
  int noperands = xtensa_opcode_num_operands (isa, opcode);
  int i;

  gas_assert (tinsn->insn_type == ITYPE_INSN);
  if (noperands != tinsn->ntok)
    as_fatal (_("operand number mismatch"));

  if (xtensa_opcode_encode (isa, fmt, slot, slotbuf, opcode))
    {
      as_bad (_("cannot encode opcode \"%s\" in the given format \"%s\""),
	      xtensa_opcode_name (isa, opcode), xtensa_format_name (isa, fmt));
      return false;
    }

  for (i = 0; i < noperands; i++)
    {
      expressionS *exp = &tinsn->tok[i];
      int rc;
      unsigned line;
      const char *file_name;
      uint32 opnd_value;

      switch (exp->X_op)
	{
	case O_register:
	  if (xtensa_operand_is_visible (isa, opcode, i) == 0)
	    break;
	  /* The register number has already been checked in
	     expression_maybe_register, so we don't need to check here.  */
	  opnd_value = exp->X_add_number;
	  (void) xtensa_operand_encode (isa, opcode, i, &opnd_value);
	  rc = xtensa_operand_set_field (isa, opcode, i, fmt, slot, slotbuf,
					 opnd_value);
	  if (rc != 0)
	    as_warn (_("xtensa-isa failure: %s"), xtensa_isa_error_msg (isa));
	  break;

	case O_constant:
	  if (xtensa_operand_is_visible (isa, opcode, i) == 0)
	    break;
	  file_name = as_where (&line);
	  /* It is a constant and we called this function
	     then we have to try to fit it.  */
	  xtensa_insnbuf_set_operand (slotbuf, fmt, slot, opcode, i,
				      exp->X_add_number, file_name, line);
	  break;

	default:
	  has_fixup = true;
	  break;
	}
    }

  return has_fixup;
}


/* Encode a single TInsn into an insnbuf.  If the opcode can only be encoded
   into a multi-slot instruction, fill the other slots with NOPs.
   Return TRUE if there is a symbol in the immediate field.  See also the
   assumptions listed for tinsn_to_slotbuf.  */

static bool
tinsn_to_insnbuf (TInsn *tinsn, xtensa_insnbuf insnbuf)
{
  static xtensa_insnbuf slotbuf = 0;
  static vliw_insn vinsn;
  xtensa_isa isa = xtensa_default_isa;
  bool has_fixup = false;
  int i;

  if (!slotbuf)
    {
      slotbuf = xtensa_insnbuf_alloc (isa);
      xg_init_vinsn (&vinsn);
    }

  xg_clear_vinsn (&vinsn);

  bundle_tinsn (tinsn, &vinsn);

  xtensa_format_encode (isa, vinsn.format, insnbuf);

  for (i = 0; i < vinsn.num_slots; i++)
    {
      /* Only one slot may have a fix-up because the rest contains NOPs.  */
      has_fixup |=
	tinsn_to_slotbuf (vinsn.format, i, &vinsn.slots[i], vinsn.slotbuf[i]);
      xtensa_format_set_slot (isa, vinsn.format, i, insnbuf, vinsn.slotbuf[i]);
    }

  return has_fixup;
}


/* Check the instruction arguments.  Return TRUE on failure.  */

static bool
tinsn_check_arguments (const TInsn *insn)
{
  xtensa_isa isa = xtensa_default_isa;
  xtensa_opcode opcode = insn->opcode;
  xtensa_regfile t1_regfile, t2_regfile;
  int t1_reg, t2_reg;
  int t1_base_reg, t1_last_reg;
  int t2_base_reg, t2_last_reg;
  char t1_inout, t2_inout;
  int i, j;

  if (opcode == XTENSA_UNDEFINED)
    {
      as_bad (_("invalid opcode"));
      return true;
    }

  if (xtensa_opcode_num_operands (isa, opcode) > insn->ntok)
    {
      as_bad (_("too few operands"));
      return true;
    }

  if (xtensa_opcode_num_operands (isa, opcode) < insn->ntok)
    {
      as_bad (_("too many operands"));
      return true;
    }

  /* Check registers.  */
  for (j = 0; j < insn->ntok; j++)
    {
      if (xtensa_operand_is_register (isa, insn->opcode, j) != 1)
	continue;

      t2_regfile = xtensa_operand_regfile (isa, insn->opcode, j);
      t2_base_reg = insn->tok[j].X_add_number;
      t2_last_reg
	= t2_base_reg + xtensa_operand_num_regs (isa, insn->opcode, j);

      for (i = 0; i < insn->ntok; i++)
	{
	  if (i == j)
	    continue;

	  if (xtensa_operand_is_register (isa, insn->opcode, i) != 1)
	    continue;

	  t1_regfile = xtensa_operand_regfile (isa, insn->opcode, i);

	  if (t1_regfile != t2_regfile)
	    continue;

	  t1_inout = xtensa_operand_inout (isa, insn->opcode, i);
	  t2_inout = xtensa_operand_inout (isa, insn->opcode, j);

	  t1_base_reg = insn->tok[i].X_add_number;
	  t1_last_reg = (t1_base_reg
			 + xtensa_operand_num_regs (isa, insn->opcode, i));

	  for (t1_reg = t1_base_reg; t1_reg < t1_last_reg; t1_reg++)
	    {
	      for (t2_reg = t2_base_reg; t2_reg < t2_last_reg; t2_reg++)
		{
		  if (t1_reg != t2_reg)
		    continue;

		  if (t1_inout != 'i' && t2_inout != 'i')
		    {
		      as_bad (_("multiple writes to the same register"));
		      return true;
		    }
		}
	    }
	}
    }
  return false;
}


/* Load an instruction from its encoded form.  */

static void
tinsn_from_chars (TInsn *tinsn, char *f, int slot)
{
  vliw_insn vinsn;

  xg_init_vinsn (&vinsn);
  vinsn_from_chars (&vinsn, f);

  *tinsn = vinsn.slots[slot];
  xg_free_vinsn (&vinsn);
}


static void
tinsn_from_insnbuf (TInsn *tinsn,
		    xtensa_insnbuf slotbuf,
		    xtensa_format fmt,
		    int slot)
{
  int i;
  xtensa_isa isa = xtensa_default_isa;

  /* Find the immed.  */
  tinsn_init (tinsn);
  tinsn->insn_type = ITYPE_INSN;
  tinsn->is_specific_opcode = false;	/* must not be specific */
  tinsn->opcode = xtensa_opcode_decode (isa, fmt, slot, slotbuf);
  tinsn->ntok = xtensa_opcode_num_operands (isa, tinsn->opcode);
  for (i = 0; i < tinsn->ntok; i++)
    {
      set_expr_const (&tinsn->tok[i],
		      xtensa_insnbuf_get_operand (slotbuf, fmt, slot,
						  tinsn->opcode, i));
    }
}


/* Read the value of the relaxable immed from the fr_symbol and fr_offset.  */

static void
tinsn_immed_from_frag (TInsn *tinsn, fragS *fragP, int slot)
{
  xtensa_opcode opcode = tinsn->opcode;
  int opnum;

  if (fragP->tc_frag_data.slot_symbols[slot])
    {
      opnum = get_relaxable_immed (opcode);
      gas_assert (opnum >= 0);
      set_expr_symbol_offset (&tinsn->tok[opnum],
			      fragP->tc_frag_data.slot_symbols[slot],
			      fragP->tc_frag_data.slot_offsets[slot]);
    }
  tinsn->extra_arg = fragP->tc_frag_data.free_reg[slot];
}


static int
get_num_stack_text_bytes (IStack *istack)
{
  int i;
  int text_bytes = 0;

  for (i = 0; i < istack->ninsn; i++)
    {
      TInsn *tinsn = &istack->insn[i];
      if (tinsn->insn_type == ITYPE_INSN)
	text_bytes += xg_get_single_size (tinsn->opcode);
    }
  return text_bytes;
}


static int
get_num_stack_literal_bytes (IStack *istack)
{
  int i;
  int lit_bytes = 0;

  for (i = 0; i < istack->ninsn; i++)
    {
      TInsn *tinsn = &istack->insn[i];
      if (tinsn->insn_type == ITYPE_LITERAL && tinsn->ntok == 1)
	lit_bytes += 4;
    }
  return lit_bytes;
}


/* vliw_insn functions.  */

static void
xg_init_vinsn (vliw_insn *v)
{
  int i;
  xtensa_isa isa = xtensa_default_isa;

  xg_clear_vinsn (v);

  v->insnbuf = xtensa_insnbuf_alloc (isa);
  if (v->insnbuf == NULL)
    as_fatal (_("out of memory"));

  for (i = 0; i < config_max_slots; i++)
    {
      v->slotbuf[i] = xtensa_insnbuf_alloc (isa);
      if (v->slotbuf[i] == NULL)
	as_fatal (_("out of memory"));
    }
}


static void
xg_clear_vinsn (vliw_insn *v)
{
  int i;

  memset (v, 0, offsetof (vliw_insn, slots)
                + sizeof(TInsn) * config_max_slots);

  v->format = XTENSA_UNDEFINED;
  v->num_slots = 0;
  v->inside_bundle = false;

  if (xt_saved_debug_type != DEBUG_NONE)
    debug_type = xt_saved_debug_type;

  for (i = 0; i < config_max_slots; i++)
    v->slots[i].opcode = XTENSA_UNDEFINED;
}


static void
xg_copy_vinsn (vliw_insn *dst, vliw_insn *src)
{
  memcpy (dst, src,
	  offsetof(vliw_insn, slots) + src->num_slots * sizeof(TInsn));
  dst->insnbuf = src->insnbuf;
  memcpy (dst->slotbuf, src->slotbuf, src->num_slots * sizeof(xtensa_insnbuf));
}


static bool
vinsn_has_specific_opcodes (vliw_insn *v)
{
  int i;

  for (i = 0; i < v->num_slots; i++)
    {
      if (v->slots[i].is_specific_opcode)
	return true;
    }
  return false;
}


static void
xg_free_vinsn (vliw_insn *v)
{
  int i;
  xtensa_insnbuf_free (xtensa_default_isa, v->insnbuf);
  for (i = 0; i < config_max_slots; i++)
    xtensa_insnbuf_free (xtensa_default_isa, v->slotbuf[i]);
}


/* Encode a vliw_insn into an insnbuf.  Return TRUE if there are any symbolic
   operands.  See also the assumptions listed for tinsn_to_slotbuf.  */

static bool
vinsn_to_insnbuf (vliw_insn *vinsn,
		  char *frag_offset,
		  fragS *fragP,
		  bool record_fixup)
{
  xtensa_isa isa = xtensa_default_isa;
  xtensa_format fmt = vinsn->format;
  xtensa_insnbuf insnbuf = vinsn->insnbuf;
  int slot;
  bool has_fixup = false;

  xtensa_format_encode (isa, fmt, insnbuf);

  for (slot = 0; slot < vinsn->num_slots; slot++)
    {
      TInsn *tinsn = &vinsn->slots[slot];
      expressionS *extra_arg = &tinsn->extra_arg;
      bool tinsn_has_fixup =
	tinsn_to_slotbuf (vinsn->format, slot, tinsn,
			  vinsn->slotbuf[slot]);

      xtensa_format_set_slot (isa, fmt, slot,
			      insnbuf, vinsn->slotbuf[slot]);
      if (extra_arg->X_op != O_illegal && extra_arg->X_op != O_register)
	{
	  if (vinsn->num_slots != 1)
	    as_bad (_("TLS relocation not allowed in FLIX bundle"));
	  else if (record_fixup)
	    /* Instructions that generate TLS relocations should always be
	       relaxed in the front-end.  If "record_fixup" is set, then this
	       function is being called during back-end relaxation, so flag
	       the unexpected behavior as an error.  */
	    as_bad (_("unexpected TLS relocation"));
	  else
	    fix_new (fragP, frag_offset - fragP->fr_literal,
		     xtensa_format_length (isa, fmt),
		     extra_arg->X_add_symbol, extra_arg->X_add_number,
		     false, map_operator_to_reloc (extra_arg->X_op, false));
	}
      if (tinsn_has_fixup)
	{
	  int i;
	  xtensa_opcode opcode = tinsn->opcode;
	  int noperands = xtensa_opcode_num_operands (isa, opcode);
	  has_fixup = true;

	  for (i = 0; i < noperands; i++)
	    {
	      expressionS* exp = &tinsn->tok[i];
	      switch (exp->X_op)
		{
		case O_symbol:
		case O_lo16:
		case O_hi16:
		  if (get_relaxable_immed (opcode) == i)
		    {
		      /* Add a fix record for the instruction, except if this
			 function is being called prior to relaxation, i.e.,
			 if record_fixup is false, and the instruction might
			 be relaxed later.  */
		      if (record_fixup
			  || tinsn->is_specific_opcode
			  || !xg_is_relaxable_insn (tinsn, 0))
			{
			  xg_add_opcode_fix (tinsn, i, fmt, slot, exp, fragP,
					     frag_offset - fragP->fr_literal);
			}
		      else
			{
			  if (exp->X_op != O_symbol)
			    as_bad (_("invalid operand"));
			  tinsn->symbol = exp->X_add_symbol;
			  tinsn->offset = exp->X_add_number;
			}
		    }
		  else
		    as_bad (_("symbolic operand not allowed"));
		  break;

		case O_constant:
		case O_register:
		  break;

		default:
		  as_bad (_("expression too complex"));
		  break;
		}
	    }
	}
    }

  return has_fixup;
}


static void
vinsn_from_chars (vliw_insn *vinsn, char *f)
{
  static xtensa_insnbuf insnbuf = NULL;
  static xtensa_insnbuf slotbuf = NULL;
  int i;
  xtensa_format fmt;
  xtensa_isa isa = xtensa_default_isa;

  if (!insnbuf)
    {
      insnbuf = xtensa_insnbuf_alloc (isa);
      slotbuf = xtensa_insnbuf_alloc (isa);
    }

  xtensa_insnbuf_from_chars (isa, insnbuf, (unsigned char *) f, 0);
  fmt = xtensa_format_decode (isa, insnbuf);
  if (fmt == XTENSA_UNDEFINED)
    as_fatal (_("cannot decode instruction format"));
  vinsn->format = fmt;
  vinsn->num_slots = xtensa_format_num_slots (isa, fmt);

  for (i = 0; i < vinsn->num_slots; i++)
    {
      TInsn *tinsn = &vinsn->slots[i];
      xtensa_format_get_slot (isa, fmt, i, insnbuf, slotbuf);
      tinsn_from_insnbuf (tinsn, slotbuf, fmt, i);
    }
}


/* Expression utilities.  */

/* Return TRUE if the expression is an integer constant.  */

bool
expr_is_const (const expressionS *s)
{
  return (s->X_op == O_constant);
}


/* Get the expression constant.
   Calling this is illegal if expr_is_const () returns TRUE.  */

offsetT
get_expr_const (const expressionS *s)
{
  gas_assert (expr_is_const (s));
  return s->X_add_number;
}


/* Set the expression to a constant value.  */

void
set_expr_const (expressionS *s, offsetT val)
{
  s->X_op = O_constant;
  s->X_add_number = val;
  s->X_add_symbol = NULL;
  s->X_op_symbol = NULL;
}


bool
expr_is_register (const expressionS *s)
{
  return (s->X_op == O_register);
}


/* Get the expression constant.
   Calling this is illegal if expr_is_const () returns TRUE.  */

offsetT
get_expr_register (const expressionS *s)
{
  gas_assert (expr_is_register (s));
  return s->X_add_number;
}


/* Set the expression to a symbol + constant offset.  */

void
set_expr_symbol_offset (expressionS *s, symbolS *sym, offsetT offset)
{
  s->X_op = O_symbol;
  s->X_add_symbol = sym;
  s->X_op_symbol = NULL;	/* unused */
  s->X_add_number = offset;
}


/* Return TRUE if the two expressions are equal.  */

bool
expr_is_equal (expressionS *s1, expressionS *s2)
{
  if (s1->X_op != s2->X_op)
    return false;
  if (s1->X_add_symbol != s2->X_add_symbol)
    return false;
  if (s1->X_op_symbol != s2->X_op_symbol)
    return false;
  if (s1->X_add_number != s2->X_add_number)
    return false;
  return true;
}


static void
copy_expr (expressionS *dst, const expressionS *src)
{
  memcpy (dst, src, sizeof (expressionS));
}


/* Support for the "--rename-section" option.  */

struct rename_section_struct
{
  const char *old_name;
  char *new_name;
  struct rename_section_struct *next;
};

static struct rename_section_struct *section_rename;


/* Parse the string "oldname=new_name(:oldname2=new_name2)*" and add
   entries to the section_rename list.  Note: Specifying multiple
   renamings separated by colons is not documented and is retained only
   for backward compatibility.  */

static void
build_section_rename (const char *arg)
{
  struct rename_section_struct *r;
  char *this_arg = NULL;
  char *next_arg = NULL;

  for (this_arg = xstrdup (arg); this_arg != NULL; this_arg = next_arg)
    {
      char *old_name, *new_name;

      if (this_arg)
	{
	  next_arg = strchr (this_arg, ':');
	  if (next_arg)
	    {
	      *next_arg = '\0';
	      next_arg++;
	    }
	}

      old_name = this_arg;
      new_name = strchr (this_arg, '=');

      if (*old_name == '\0')
	{
	  as_warn (_("ignoring extra '-rename-section' delimiter ':'"));
	  continue;
	}
      if (!new_name || new_name[1] == '\0')
	{
	  as_warn (_("ignoring invalid '-rename-section' specification: '%s'"),
		   old_name);
	  continue;
	}
      *new_name = '\0';
      new_name++;

      /* Check for invalid section renaming.  */
      for (r = section_rename; r != NULL; r = r->next)
	{
	  if (strcmp (r->old_name, old_name) == 0)
	    as_bad (_("section %s renamed multiple times"), old_name);
	  if (strcmp (r->new_name, new_name) == 0)
	    as_bad (_("multiple sections remapped to output section %s"),
		    new_name);
	}

      /* Now add it.  */
      r = XNEW (struct rename_section_struct);
      r->old_name = xstrdup (old_name);
      r->new_name = xstrdup (new_name);
      r->next = section_rename;
      section_rename = r;
    }
}


char *
xtensa_section_rename (const char *name)
{
  struct rename_section_struct *r = section_rename;

  for (r = section_rename; r != NULL; r = r->next)
    {
      if (strcmp (r->old_name, name) == 0)
	return r->new_name;
    }

  return (char *) name;
}
