/* Definitions for TI C6X assembler.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

#define TC_TIC6X 1
#define TARGET_BYTES_BIG_ENDIAN 0
#define WORKING_DOT_WORD
#define DOUBLEBAR_PARALLEL
#define DWARF2_LINE_MIN_INSN_LENGTH 2
#define MD_APPLY_SYM_VALUE(FIX) 0
#define TC_PREDICATE_START_CHAR '['
#define TC_PREDICATE_END_CHAR ']'
/* For TI C6X, we keep spaces in what the preprocessor considers
   operands as they may separate functional unit specifiers from
   operands.  */
#define TC_KEEP_OPERAND_SPACES 1

#define TARGET_ARCH 	bfd_arch_tic6x
#define TARGET_FORMAT	(target_big_endian	\
			 ? "elf32-tic6x-be"	\
			 : "elf32-tic6x-le")

typedef struct tic6x_label_list
{
  struct tic6x_label_list *next;
  symbolS *label;
} tic6x_label_list;

/* Must be consistent with the enum in tc-tic6x.c.  */
#define TIC6X_NUM_UNWIND_REGS 13

/* Unwinding information state.  */
typedef struct tic6x_unwind_info {
    int personality_index;
    symbolS *personality_routine;
    symbolS *function_start;
    segT saved_seg;
    subsegT saved_subseg;
    /* NULL if table entry is inline.  */
    symbolS *table_entry;
    char *frag_start;
    valueT data;
    /* 0 before .cfi_startproc
      -1 between .cfi_startproc and .handlerdata
      >0 between .handlerdata and .endp */
    int data_bytes;

    offsetT reg_offset[TIC6X_NUM_UNWIND_REGS];
    bool reg_saved[TIC6X_NUM_UNWIND_REGS];
    int cfa_reg;
    int return_reg;
    unsigned safe_mask;
    unsigned compact_mask;
    unsigned reg_saved_mask;
    offsetT cfa_offset;
    bool pop_rts;
    /* Only valid for UNWIND_OP_POP_REG */
    int saved_reg_count;
} tic6x_unwind_info;

typedef struct
{
  /* Any labels seen since the last instruction or data.  If not NULL,
     a following instruction may not have parallel bars, but must
     start a new execute packet.  */
  tic6x_label_list *label_list;

  /* Whether compact instructions are forbidden here.  */
  bool nocmp;

  /* If there is a current execute packet, the frag being used for
     that execute packet.  */
  fragS *execute_packet_frag;

  /* If there is a current execute packet, a pointer to the
     least-significant byte of the last instruction in it (for setting
     the p-bit).  */
  char *last_insn_lsb;

  /* If there has been an SPMASK instruction in the current execute
     packet, a pointer to the first byte in it (for processing
     ||^); otherwise NULL.  */
  char *spmask_addr;

  /* The functional units used in the current execute packet, recorded
     by setting the same bits as would be set in the 32-bit SPMASK
     instruction.  */
  unsigned int func_units_used;

  /* If an SPLOOP-family instruction has been seen, and a following
     SPKERNEL-family instruction has not yet been seen, the ii value
     from the SPLOOP instruction (in the range 1 to 14); otherwise
     0.  */
  int sploop_ii;

  /* Bit N indicates that an R_C6000_NONE relocation has been output for
     __c6xabi_unwind_cpp_prN already if set. This enables dependencies to be
     emitted only once per section, to save unnecessary bloat.  */
  unsigned int marked_pr_dependency;

  tic6x_unwind_info *unwind;
  tic6x_unwind_info *text_unwind;
} tic6x_segment_info_type;
#define TC_SEGMENT_INFO_TYPE tic6x_segment_info_type

typedef struct
{
  /* Whether this machine-dependent frag is used for instructions (as
     opposed to code alignment).  */
  bool is_insns;

  /* For a frag used for instructions, whether it is may cross a fetch
     packet boundary (subject to alignment requirements).  */
  bool can_cross_fp_boundary;
} tic6x_frag_info;
#define TC_FRAG_TYPE tic6x_frag_info
#define TC_FRAG_INIT(fragP, max_bytes) tic6x_frag_init (fragP)
extern void tic6x_frag_init (fragS *fragp);

typedef struct
{
  /* Whether this fix was for an ADDA instruction.  If so, a constant
     resulting from resolving the fix should be implicitly shifted
     left (it represents a value to be encoded literally in the
     instruction, whereas a non-constant represents a DP-relative
     value counting in the appropriate units).  */
  bool fix_adda;
  /* The symbol to be subtracted in case of a PCR_H16 or PCR_L16
     reloc.  */
  symbolS *fix_subsy;
} tic6x_fix_info;
#define TC_FIX_TYPE tic6x_fix_info
#define TC_INIT_FIX_DATA(fixP) tic6x_init_fix_data (fixP)
struct fix;
extern void tic6x_init_fix_data (struct fix *fixP);

#define md_after_parse_args() tic6x_after_parse_args ()
extern void tic6x_after_parse_args (void);

#define md_cleanup() tic6x_cleanup ()
extern void tic6x_cleanup (void);

#define md_cons_align(n) tic6x_cons_align (n)
extern void tic6x_cons_align (int n);

#define md_do_align(n, fill, len, max, label)	\
  do {						\
    if (tic6x_do_align (n, fill, len, max))	\
      goto label;				\
  } while (0)
extern bool tic6x_do_align (int n, char *fill, int len, int max);

#define CONVERT_SYMBOLIC_ATTRIBUTE(name)	\
  tic6x_convert_symbolic_attribute (name)
extern int tic6x_convert_symbolic_attribute (const char *);

#define md_finish() tic6x_md_finish ();
extern void tic6x_md_finish (void);

#define md_parse_name(name, exprP, mode, nextcharP)	\
  tic6x_parse_name (name, exprP, mode, nextcharP)
extern int tic6x_parse_name (const char *name, expressionS *exprP,
			     enum expr_mode mode, char *nextchar);

#define MD_PCREL_FROM_SECTION(FIX, SEC) tic6x_pcrel_from_section (FIX, SEC)
extern long tic6x_pcrel_from_section (struct fix *fixp, segT sec);

#define md_start_line_hook() tic6x_start_line_hook ()
extern void tic6x_start_line_hook (void);

#define TC_CONS_FIX_NEW(frag, where, size, exp, reloc)	\
  tic6x_cons_fix_new (frag, where, size, exp, reloc)
extern void tic6x_cons_fix_new (fragS *, int, int, expressionS *,
				bfd_reloc_code_real_type);

#define tc_fix_adjustable(FIX) tic6x_fix_adjustable (FIX)
extern bool tic6x_fix_adjustable (struct fix *);

#define tc_frob_label(sym) tic6x_frob_label (sym)
extern void tic6x_frob_label (symbolS *sym);

#define tc_init_after_args() tic6x_init_after_args ()
extern void tic6x_init_after_args (void);

#define tc_unrecognized_line(c) tic6x_unrecognized_line (c)
extern int tic6x_unrecognized_line (int c);

/* We want .cfi_* pseudo-ops for generating unwind info.  */
#define TARGET_USE_CFIPOP              1

/* CFI hooks.  */
#define tc_regname_to_dw2regnum            tic6x_regname_to_dw2regnum
int tic6x_regname_to_dw2regnum (char *regname);

#define tc_cfi_frame_initial_instructions  tic6x_frame_initial_instructions
void tic6x_frame_initial_instructions (void);

/* The return register is B3.  */
#define DWARF2_DEFAULT_RETURN_COLUMN  (16 + 3)

/* Registers are generally saved at negative offsets to the CFA.  */
#define DWARF2_CIE_DATA_ALIGNMENT     (-4)

#define tc_cfi_startproc tic6x_cfi_startproc
void tic6x_cfi_startproc (void);

#define tc_cfi_endproc tic6x_cfi_endproc
struct fde_entry;
void tic6x_cfi_endproc (struct fde_entry *fde);

#define tc_cfi_section_name ".c6xabi.exidx"
