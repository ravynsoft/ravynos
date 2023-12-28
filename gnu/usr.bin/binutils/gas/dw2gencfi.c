/* dw2gencfi.c - Support for generating Dwarf2 CFI information.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.
   Contributed by Michal Ludvig <mludvig@suse.cz>

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
#include "dw2gencfi.h"
#include "subsegs.h"
#include "dwarf2dbg.h"
#include "gen-sframe.h"

#ifdef TARGET_USE_CFIPOP

/* By default, use difference expressions if DIFF_EXPR_OK is defined.  */
#ifndef CFI_DIFF_EXPR_OK
# ifdef DIFF_EXPR_OK
#  define CFI_DIFF_EXPR_OK 1
# else
#  define CFI_DIFF_EXPR_OK 0
# endif
#endif

#ifndef CFI_DIFF_LSDA_OK
#define CFI_DIFF_LSDA_OK CFI_DIFF_EXPR_OK
#endif

#if CFI_DIFF_EXPR_OK == 1 && CFI_DIFF_LSDA_OK == 0
# error "CFI_DIFF_EXPR_OK should imply CFI_DIFF_LSDA_OK"
#endif

/* We re-use DWARF2_LINE_MIN_INSN_LENGTH for the code alignment field
   of the CIE.  Default to 1 if not otherwise specified.  */
#ifndef DWARF2_LINE_MIN_INSN_LENGTH
#define DWARF2_LINE_MIN_INSN_LENGTH 1
#endif

/* By default, use 32-bit relocations from .eh_frame into .text.  */
#ifndef DWARF2_FDE_RELOC_SIZE
#define DWARF2_FDE_RELOC_SIZE 4
#endif

/* By default, use a read-only .eh_frame section.  */
#ifndef DWARF2_EH_FRAME_READ_ONLY
#define DWARF2_EH_FRAME_READ_ONLY SEC_READONLY
#endif

#ifndef EH_FRAME_ALIGNMENT
#define EH_FRAME_ALIGNMENT (bfd_get_arch_size (stdoutput) == 64 ? 3 : 2)
#endif

#ifndef tc_cfi_frame_initial_instructions
#define tc_cfi_frame_initial_instructions() ((void)0)
#endif

#ifndef tc_cfi_startproc
# define tc_cfi_startproc() ((void)0)
#endif

#ifndef tc_cfi_endproc
# define tc_cfi_endproc(fde) ((void) (fde))
#endif

#define EH_FRAME_LINKONCE (SUPPORT_FRAME_LINKONCE || compact_eh \
			   || TARGET_MULTIPLE_EH_FRAME_SECTIONS)

#ifndef DWARF2_FORMAT
#define DWARF2_FORMAT(SEC) dwarf2_format_32bit
#endif

#ifndef DWARF2_ADDR_SIZE
#define DWARF2_ADDR_SIZE(bfd) (bfd_arch_bits_per_address (bfd) / 8)
#endif

#if MULTIPLE_FRAME_SECTIONS
#define CUR_SEG(structp) structp->cur_seg
#define SET_CUR_SEG(structp, seg) structp->cur_seg = seg
#define HANDLED(structp) structp->handled
#define SET_HANDLED(structp, val) structp->handled = val
#else
#define CUR_SEG(structp) NULL
#define SET_CUR_SEG(structp, seg) (void) (0 && seg)
#define HANDLED(structp) 0
#define SET_HANDLED(structp, val) (void) (0 && val)
#endif

#ifndef tc_cfi_reloc_for_encoding
#define tc_cfi_reloc_for_encoding(e) BFD_RELOC_NONE
#endif

/* Targets which support SFrame format will define this and return true.  */
#ifndef support_sframe_p
# define support_sframe_p() false
#endif

/* Private segment collection list.  */
struct dwcfi_seg_list
{
  segT   seg;
  int    subseg;
  char * seg_name;
};

#ifdef SUPPORT_COMPACT_EH
static bool compact_eh;
#else
#define compact_eh 0
#endif

static htab_t dwcfi_hash;

/* Emit a single byte into the current segment.  */

static inline void
out_one (int byte)
{
  FRAG_APPEND_1_CHAR (byte);
}

/* Emit a two-byte word into the current segment.  */

static inline void
out_two (int data)
{
  md_number_to_chars (frag_more (2), data, 2);
}

/* Emit a four byte word into the current segment.  */

static inline void
out_four (int data)
{
  md_number_to_chars (frag_more (4), data, 4);
}

/* Emit an unsigned "little-endian base 128" number.  */

static void
out_uleb128 (addressT value)
{
  output_leb128 (frag_more (sizeof_leb128 (value, 0)), value, 0);
}

/* Emit an unsigned "little-endian base 128" number.  */

static void
out_sleb128 (offsetT value)
{
  output_leb128 (frag_more (sizeof_leb128 (value, 1)), value, 1);
}

static unsigned int
encoding_size (unsigned char encoding)
{
  if (encoding == DW_EH_PE_omit)
    return 0;
  switch (encoding & 0x7)
    {
    case 0:
      return bfd_get_arch_size (stdoutput) == 64 ? 8 : 4;
    case DW_EH_PE_udata2:
      return 2;
    case DW_EH_PE_udata4:
      return 4;
    case DW_EH_PE_udata8:
      return 8;
    default:
      abort ();
    }
}

/* Emit expression EXP in ENCODING.  If EMIT_ENCODING is true, first
   emit a byte containing ENCODING.  */

static void
emit_expr_encoded (expressionS *exp, int encoding, bool emit_encoding)
{
  unsigned int size = encoding_size (encoding);
  bfd_reloc_code_real_type code;

  if (encoding == DW_EH_PE_omit)
    return;

  if (emit_encoding)
    out_one (encoding);

  code = tc_cfi_reloc_for_encoding (encoding);
  if (code != BFD_RELOC_NONE)
    {
      reloc_howto_type *howto = bfd_reloc_type_lookup (stdoutput, code);
      char *p = frag_more (size);
      gas_assert (size == (unsigned) howto->bitsize / 8);
      md_number_to_chars (p, 0, size);
      fix_new (frag_now, p - frag_now->fr_literal, size, exp->X_add_symbol,
	       exp->X_add_number, howto->pc_relative, code);
    }
  else if ((encoding & 0x70) == DW_EH_PE_pcrel)
    {
#if CFI_DIFF_EXPR_OK
      expressionS tmp = *exp;
      tmp.X_op = O_subtract;
      tmp.X_op_symbol = symbol_temp_new_now ();
      emit_expr (&tmp, size);
#elif defined (tc_cfi_emit_pcrel_expr)
      tc_cfi_emit_pcrel_expr (exp, size);
#else
      abort ();
#endif
    }
  else
    emit_expr (exp, size);
}

/* Build based on segment the derived .debug_...
   segment name containing origin segment's postfix name part.  */

static char *
get_debugseg_name (segT seg, const char *base_name)
{
  const char * name;
  const char * dollar;
  const char * dot;

  if (!seg
      || (name = bfd_section_name (seg)) == NULL
      || *name == 0)
    return notes_strdup (base_name);
	
  dollar = strchr (name, '$');
  dot = strchr (name + 1, '.');

  if (!dollar && !dot)
    {
      if (!strcmp (base_name, ".eh_frame_entry")
	  && strcmp (name, ".text") != 0)
	return notes_concat (base_name, ".", name, NULL);

      name = "";
    }
  else if (!dollar)
    name = dot;
  else if (!dot)
    name = dollar;
  else if (dot < dollar)
    name = dot;
  else
    name = dollar;

  return notes_concat (base_name, name, NULL);
}

/* Allocate a dwcfi_seg_list structure.  */

static struct dwcfi_seg_list *
alloc_debugseg_item (segT seg, int subseg, char *name)
{
  struct dwcfi_seg_list *r;

  r = notes_alloc (sizeof (*r) + strlen (name));
  r->seg = seg;
  r->subseg = subseg;
  r->seg_name = name;
  return r;
}

static segT
is_now_linkonce_segment (void)
{
  if (compact_eh)
    return now_seg;

  if (TARGET_MULTIPLE_EH_FRAME_SECTIONS)
    return now_seg;

  if ((bfd_section_flags (now_seg)
       & (SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD
	  | SEC_LINK_DUPLICATES_ONE_ONLY | SEC_LINK_DUPLICATES_SAME_SIZE
	  | SEC_LINK_DUPLICATES_SAME_CONTENTS)) != 0)
    return now_seg;
  return NULL;
}

/* Generate debug... segment with same linkonce properties
   of based segment.  */

static segT
make_debug_seg (segT cseg, char *name, int sflags)
{
  segT save_seg = now_seg;
  int save_subseg = now_subseg;
  segT r;
  flagword flags;

  r = subseg_new (name, 0);

  /* Check if code segment is marked as linked once.  */
  if (!cseg)
    flags = 0;
  else
    flags = (bfd_section_flags (cseg)
	     & (SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD
		| SEC_LINK_DUPLICATES_ONE_ONLY | SEC_LINK_DUPLICATES_SAME_SIZE
		| SEC_LINK_DUPLICATES_SAME_CONTENTS));

  /* Add standard section flags.  */
  flags |= sflags;

  /* Apply possibly linked once flags to new generated segment, too.  */
  if (!bfd_set_section_flags (r, flags))
    as_bad (_("bfd_set_section_flags: %s"),
	    bfd_errmsg (bfd_get_error ()));

  /* Restore to previous segment.  */
  if (save_seg != NULL)
    subseg_set (save_seg, save_subseg);
  return r;
}

static struct dwcfi_seg_list *
dwcfi_hash_find (char *name)
{
  return (struct dwcfi_seg_list *) str_hash_find (dwcfi_hash, name);
}

static struct dwcfi_seg_list *
dwcfi_hash_find_or_make (segT cseg, const char *base_name, int flags)
{
  struct dwcfi_seg_list *item;
  char *name;

  /* Initialize dwcfi_hash once.  */
  if (!dwcfi_hash)
    dwcfi_hash = str_htab_create ();

  name = get_debugseg_name (cseg, base_name);

  item = dwcfi_hash_find (name);
  if (!item)
    {
      item = alloc_debugseg_item (make_debug_seg (cseg, name, flags), 0, name);

      str_hash_insert (dwcfi_hash, item->seg_name, item, 0);
    }
  else
    notes_free (name);

  return item;
}

/* ??? Share this with dwarf2cfg.c.  */
#ifndef TC_DWARF2_EMIT_OFFSET
#define TC_DWARF2_EMIT_OFFSET  generic_dwarf2_emit_offset

/* Create an offset to .dwarf2_*.  */

static void
generic_dwarf2_emit_offset (symbolS *symbol, unsigned int size)
{
  expressionS exp;

  exp.X_op = O_symbol;
  exp.X_add_symbol = symbol;
  exp.X_add_number = 0;
  emit_expr (&exp, size);
}
#endif

struct cfi_escape_data
{
  struct cfi_escape_data *next;
  expressionS exp;
};

struct cie_entry
{
  struct cie_entry *next;
#if MULTIPLE_FRAME_SECTIONS
  segT cur_seg;
#endif
  symbolS *start_address;
  unsigned int return_column;
  unsigned int signal_frame;
  unsigned char fde_encoding;
  unsigned char per_encoding;
  unsigned char lsda_encoding;
  expressionS personality;
#ifdef tc_cie_entry_extras
  tc_cie_entry_extras
#endif
  struct cfi_insn_data *first, *last;
};

/* List of FDE entries.  */

struct fde_entry *all_fde_data;
static struct fde_entry **last_fde_data = &all_fde_data;

/* List of CIEs so that they could be reused.  */
static struct cie_entry *cie_root;

/* Construct a new FDE structure and add it to the end of the fde list.  */

static struct fde_entry *
alloc_fde_entry (void)
{
  struct fde_entry *fde = XCNEW (struct fde_entry);

  frchain_now->frch_cfi_data = XCNEW (struct frch_cfi_data);
  frchain_now->frch_cfi_data->cur_fde_data = fde;
  *last_fde_data = fde;
  last_fde_data = &fde->next;
  SET_CUR_SEG (fde, is_now_linkonce_segment ());
  SET_HANDLED (fde, 0);
  fde->last = &fde->data;
  fde->return_column = DWARF2_DEFAULT_RETURN_COLUMN;
  fde->per_encoding = DW_EH_PE_omit;
  fde->lsda_encoding = DW_EH_PE_omit;
  fde->eh_header_type = EH_COMPACT_UNKNOWN;
#ifdef tc_fde_entry_init_extra
  tc_fde_entry_init_extra (fde)
#endif

  return fde;
}

/* The following functions are available for a backend to construct its
   own unwind information, usually from legacy unwind directives.  */

/* Construct a new INSN structure and add it to the end of the insn list
   for the currently active FDE.  */

static bool cfi_sections_set = false;
static int cfi_sections = CFI_EMIT_eh_frame;
int all_cfi_sections = 0;
static struct fde_entry *last_fde;

static struct cfi_insn_data *
alloc_cfi_insn_data (void)
{
  struct cfi_insn_data *insn = XCNEW (struct cfi_insn_data);
  struct fde_entry *cur_fde_data = frchain_now->frch_cfi_data->cur_fde_data;

  *cur_fde_data->last = insn;
  cur_fde_data->last = &insn->next;
  SET_CUR_SEG (insn, is_now_linkonce_segment ());
  return insn;
}

/* Construct a new FDE structure that begins at LABEL.  */

void
cfi_new_fde (symbolS *label)
{
  struct fde_entry *fde = alloc_fde_entry ();
  fde->start_address = label;
  frchain_now->frch_cfi_data->last_address = label;
}

/* End the currently open FDE.  */

void
cfi_end_fde (symbolS *label)
{
  frchain_now->frch_cfi_data->cur_fde_data->end_address = label;
  free (frchain_now->frch_cfi_data);
  frchain_now->frch_cfi_data = NULL;
}

/* Set the return column for the current FDE.  */

void
cfi_set_return_column (unsigned regno)
{
  frchain_now->frch_cfi_data->cur_fde_data->return_column = regno;
}

void
cfi_set_sections (void)
{
  frchain_now->frch_cfi_data->cur_fde_data->sections = all_cfi_sections;
  cfi_sections_set = true;
}

/* Universal functions to store new instructions.  */

static void
cfi_add_CFA_insn (int insn)
{
  struct cfi_insn_data *insn_ptr = alloc_cfi_insn_data ();

  insn_ptr->insn = insn;
}

static void
cfi_add_CFA_insn_reg (int insn, unsigned regno)
{
  struct cfi_insn_data *insn_ptr = alloc_cfi_insn_data ();

  insn_ptr->insn = insn;
  insn_ptr->u.r = regno;
}

static void
cfi_add_CFA_insn_offset (int insn, offsetT offset)
{
  struct cfi_insn_data *insn_ptr = alloc_cfi_insn_data ();

  insn_ptr->insn = insn;
  insn_ptr->u.i = offset;
}

static void
cfi_add_CFA_insn_reg_reg (int insn, unsigned reg1, unsigned reg2)
{
  struct cfi_insn_data *insn_ptr = alloc_cfi_insn_data ();

  insn_ptr->insn = insn;
  insn_ptr->u.rr.reg1 = reg1;
  insn_ptr->u.rr.reg2 = reg2;
}

static void
cfi_add_CFA_insn_reg_offset (int insn, unsigned regno, offsetT offset)
{
  struct cfi_insn_data *insn_ptr = alloc_cfi_insn_data ();

  insn_ptr->insn = insn;
  insn_ptr->u.ri.reg = regno;
  insn_ptr->u.ri.offset = offset;
}

/* Add a CFI insn to advance the PC from the last address to LABEL.  */

void
cfi_add_advance_loc (symbolS *label)
{
  struct cfi_insn_data *insn = alloc_cfi_insn_data ();

  insn->insn = DW_CFA_advance_loc;
  insn->u.ll.lab1 = frchain_now->frch_cfi_data->last_address;
  insn->u.ll.lab2 = label;

  frchain_now->frch_cfi_data->last_address = label;
}

/* Add a CFI insn to label the current position in the CFI segment.  */

void
cfi_add_label (const char *name)
{
  unsigned int len = strlen (name) + 1;
  struct cfi_insn_data *insn = alloc_cfi_insn_data ();

  insn->insn = CFI_label;
  obstack_grow (&notes, name, len);
  insn->u.sym_name = (char *) obstack_finish (&notes);
}

/* Add a DW_CFA_offset record to the CFI data.  */

void
cfi_add_CFA_offset (unsigned regno, offsetT offset)
{
  unsigned int abs_data_align;

  gas_assert (DWARF2_CIE_DATA_ALIGNMENT != 0);
  cfi_add_CFA_insn_reg_offset (DW_CFA_offset, regno, offset);

  abs_data_align = (DWARF2_CIE_DATA_ALIGNMENT < 0
		    ? -DWARF2_CIE_DATA_ALIGNMENT : DWARF2_CIE_DATA_ALIGNMENT);
  if (offset % abs_data_align)
    as_bad (_("register save offset not a multiple of %u"), abs_data_align);
}

/* Add a DW_CFA_val_offset record to the CFI data.  */

void
cfi_add_CFA_val_offset (unsigned regno, offsetT offset)
{
  unsigned int abs_data_align;

  gas_assert (DWARF2_CIE_DATA_ALIGNMENT != 0);
  cfi_add_CFA_insn_reg_offset (DW_CFA_val_offset, regno, offset);

  abs_data_align = (DWARF2_CIE_DATA_ALIGNMENT < 0
		    ? -DWARF2_CIE_DATA_ALIGNMENT : DWARF2_CIE_DATA_ALIGNMENT);
  if (offset % abs_data_align)
    as_bad (_("register save offset not a multiple of %u"), abs_data_align);
}

/* Add a DW_CFA_def_cfa record to the CFI data.  */

void
cfi_add_CFA_def_cfa (unsigned regno, offsetT offset)
{
  cfi_add_CFA_insn_reg_offset (DW_CFA_def_cfa, regno, offset);
  frchain_now->frch_cfi_data->cur_cfa_offset = offset;
}

/* Add a DW_CFA_register record to the CFI data.  */

void
cfi_add_CFA_register (unsigned reg1, unsigned reg2)
{
  cfi_add_CFA_insn_reg_reg (DW_CFA_register, reg1, reg2);
}

/* Add a DW_CFA_def_cfa_register record to the CFI data.  */

void
cfi_add_CFA_def_cfa_register (unsigned regno)
{
  cfi_add_CFA_insn_reg (DW_CFA_def_cfa_register, regno);
}

/* Add a DW_CFA_def_cfa_offset record to the CFI data.  */

void
cfi_add_CFA_def_cfa_offset (offsetT offset)
{
  cfi_add_CFA_insn_offset (DW_CFA_def_cfa_offset, offset);
  frchain_now->frch_cfi_data->cur_cfa_offset = offset;
}

void
cfi_add_CFA_restore (unsigned regno)
{
  cfi_add_CFA_insn_reg (DW_CFA_restore, regno);
}

void
cfi_add_CFA_undefined (unsigned regno)
{
  cfi_add_CFA_insn_reg (DW_CFA_undefined, regno);
}

void
cfi_add_CFA_same_value (unsigned regno)
{
  cfi_add_CFA_insn_reg (DW_CFA_same_value, regno);
}

void
cfi_add_CFA_remember_state (void)
{
  struct cfa_save_data *p;

  cfi_add_CFA_insn (DW_CFA_remember_state);

  p = XNEW (struct cfa_save_data);
  p->cfa_offset = frchain_now->frch_cfi_data->cur_cfa_offset;
  p->next = frchain_now->frch_cfi_data->cfa_save_stack;
  frchain_now->frch_cfi_data->cfa_save_stack = p;
}

void
cfi_add_CFA_restore_state (void)
{
  struct cfa_save_data *p;

  cfi_add_CFA_insn (DW_CFA_restore_state);

  p = frchain_now->frch_cfi_data->cfa_save_stack;
  if (p)
    {
      frchain_now->frch_cfi_data->cur_cfa_offset = p->cfa_offset;
      frchain_now->frch_cfi_data->cfa_save_stack = p->next;
      free (p);
    }
  else
    as_bad (_("CFI state restore without previous remember"));
}


/* Parse CFI assembler directives.  */

static void dot_cfi (int);
static void dot_cfi_escape (int);
static void dot_cfi_sections (int);
static void dot_cfi_startproc (int);
static void dot_cfi_endproc (int);
static void dot_cfi_fde_data (int);
static void dot_cfi_personality (int);
static void dot_cfi_personality_id (int);
static void dot_cfi_lsda (int);
static void dot_cfi_val_encoded_addr (int);
static void dot_cfi_inline_lsda (int);
static void dot_cfi_label (int);

const pseudo_typeS cfi_pseudo_table[] =
  {
    { "cfi_sections", dot_cfi_sections, 0 },
    { "cfi_startproc", dot_cfi_startproc, 0 },
    { "cfi_endproc", dot_cfi_endproc, 0 },
    { "cfi_fde_data", dot_cfi_fde_data, 0 },
    { "cfi_def_cfa", dot_cfi, DW_CFA_def_cfa },
    { "cfi_def_cfa_register", dot_cfi, DW_CFA_def_cfa_register },
    { "cfi_def_cfa_offset", dot_cfi, DW_CFA_def_cfa_offset },
    { "cfi_adjust_cfa_offset", dot_cfi, CFI_adjust_cfa_offset },
    { "cfi_offset", dot_cfi, DW_CFA_offset },
    { "cfi_rel_offset", dot_cfi, CFI_rel_offset },
    { "cfi_register", dot_cfi, DW_CFA_register },
    { "cfi_return_column", dot_cfi, CFI_return_column },
    { "cfi_restore", dot_cfi, DW_CFA_restore },
    { "cfi_undefined", dot_cfi, DW_CFA_undefined },
    { "cfi_same_value", dot_cfi, DW_CFA_same_value },
    { "cfi_remember_state", dot_cfi, DW_CFA_remember_state },
    { "cfi_restore_state", dot_cfi, DW_CFA_restore_state },
    { "cfi_window_save", dot_cfi, DW_CFA_GNU_window_save },
    { "cfi_negate_ra_state", dot_cfi, DW_CFA_AARCH64_negate_ra_state },
    { "cfi_escape", dot_cfi_escape, 0 },
    { "cfi_signal_frame", dot_cfi, CFI_signal_frame },
    { "cfi_personality", dot_cfi_personality, 0 },
    { "cfi_personality_id", dot_cfi_personality_id, 0 },
    { "cfi_lsda", dot_cfi_lsda, 0 },
    { "cfi_val_encoded_addr", dot_cfi_val_encoded_addr, 0 },
    { "cfi_inline_lsda", dot_cfi_inline_lsda, 0 },
    { "cfi_label", dot_cfi_label, 0 },
    { "cfi_val_offset", dot_cfi, DW_CFA_val_offset },
    { NULL, NULL, 0 }
  };

static void
cfi_parse_separator (void)
{
  SKIP_WHITESPACE ();
  if (*input_line_pointer == ',')
    input_line_pointer++;
  else
    as_bad (_("missing separator"));
}

#ifndef tc_parse_to_dw2regnum
static void
tc_parse_to_dw2regnum (expressionS *exp)
{
# ifdef tc_regname_to_dw2regnum
  SKIP_WHITESPACE ();
  if (is_name_beginner (*input_line_pointer)
      || (*input_line_pointer == '%'
	  && is_name_beginner (*++input_line_pointer)))
    {
      char *name, c;

      c = get_symbol_name (& name);

      exp->X_op = O_constant;
      exp->X_add_number = tc_regname_to_dw2regnum (name);

      restore_line_pointer (c);
    }
  else
# endif
    expression_and_evaluate (exp);
}
#endif

static unsigned
cfi_parse_reg (void)
{
  int regno;
  expressionS exp;

  tc_parse_to_dw2regnum (&exp);
  switch (exp.X_op)
    {
    case O_register:
    case O_constant:
      regno = exp.X_add_number;
      break;

    default:
      regno = -1;
      break;
    }

  if (regno < 0)
    {
      as_bad (_("bad register expression"));
      regno = 0;
    }

  return regno;
}

static offsetT
cfi_parse_const (void)
{
  return get_absolute_expression ();
}

static void
dot_cfi (int arg)
{
  offsetT offset;
  unsigned reg1, reg2;

  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_("CFI instruction used without previous .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  /* If the last address was not at the current PC, advance to current.  */
  if (symbol_get_frag (frchain_now->frch_cfi_data->last_address) != frag_now
      || (S_GET_VALUE (frchain_now->frch_cfi_data->last_address)
	  != frag_now_fix ()))
    cfi_add_advance_loc (symbol_temp_new_now ());

  switch (arg)
    {
    case DW_CFA_offset:
      reg1 = cfi_parse_reg ();
      cfi_parse_separator ();
      offset = cfi_parse_const ();
      cfi_add_CFA_offset (reg1, offset);
      break;

    case DW_CFA_val_offset:
      reg1 = cfi_parse_reg ();
      cfi_parse_separator ();
      offset = cfi_parse_const ();
      cfi_add_CFA_val_offset (reg1, offset);
      break;

    case CFI_rel_offset:
      reg1 = cfi_parse_reg ();
      cfi_parse_separator ();
      offset = cfi_parse_const ();
      cfi_add_CFA_offset (reg1,
			  offset - frchain_now->frch_cfi_data->cur_cfa_offset);
      break;

    case DW_CFA_def_cfa:
      reg1 = cfi_parse_reg ();
      cfi_parse_separator ();
      offset = cfi_parse_const ();
      cfi_add_CFA_def_cfa (reg1, offset);
      break;

    case DW_CFA_register:
      reg1 = cfi_parse_reg ();
      cfi_parse_separator ();
      reg2 = cfi_parse_reg ();
      cfi_add_CFA_register (reg1, reg2);
      break;

    case DW_CFA_def_cfa_register:
      reg1 = cfi_parse_reg ();
      cfi_add_CFA_def_cfa_register (reg1);
      break;

    case DW_CFA_def_cfa_offset:
      offset = cfi_parse_const ();
      cfi_add_CFA_def_cfa_offset (offset);
      break;

    case CFI_adjust_cfa_offset:
      offset = cfi_parse_const ();
      cfi_add_CFA_def_cfa_offset (frchain_now->frch_cfi_data->cur_cfa_offset
				  + offset);
      break;

    case DW_CFA_restore:
      for (;;)
	{
	  reg1 = cfi_parse_reg ();
	  cfi_add_CFA_restore (reg1);
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer != ',')
	    break;
	  ++input_line_pointer;
	}
      break;

    case DW_CFA_undefined:
      for (;;)
	{
	  reg1 = cfi_parse_reg ();
	  cfi_add_CFA_undefined (reg1);
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer != ',')
	    break;
	  ++input_line_pointer;
	}
      break;

    case DW_CFA_same_value:
      reg1 = cfi_parse_reg ();
      cfi_add_CFA_same_value (reg1);
      break;

    case CFI_return_column:
      reg1 = cfi_parse_reg ();
      cfi_set_return_column (reg1);
      break;

    case DW_CFA_remember_state:
      cfi_add_CFA_remember_state ();
      break;

    case DW_CFA_restore_state:
      cfi_add_CFA_restore_state ();
      break;

    case DW_CFA_GNU_window_save:
      cfi_add_CFA_insn (DW_CFA_GNU_window_save);
      break;

    case CFI_signal_frame:
      frchain_now->frch_cfi_data->cur_fde_data->signal_frame = 1;
      break;

    default:
      abort ();
    }

  demand_empty_rest_of_line ();
}

static void
dot_cfi_escape (int ignored ATTRIBUTE_UNUSED)
{
  struct cfi_escape_data *head, **tail, *e;
  struct cfi_insn_data *insn;

  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_("CFI instruction used without previous .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  /* If the last address was not at the current PC, advance to current.  */
  if (symbol_get_frag (frchain_now->frch_cfi_data->last_address) != frag_now
      || (S_GET_VALUE (frchain_now->frch_cfi_data->last_address)
	  != frag_now_fix ()))
    cfi_add_advance_loc (symbol_temp_new_now ());

  tail = &head;
  do
    {
      e = XNEW (struct cfi_escape_data);
      do_parse_cons_expression (&e->exp, 1);
      *tail = e;
      tail = &e->next;
    }
  while (*input_line_pointer++ == ',');
  *tail = NULL;

  insn = alloc_cfi_insn_data ();
  insn->insn = CFI_escape;
  insn->u.esc = head;

  --input_line_pointer;
  demand_empty_rest_of_line ();
}

static void
dot_cfi_personality (int ignored ATTRIBUTE_UNUSED)
{
  struct fde_entry *fde;
  offsetT encoding;

  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_("CFI instruction used without previous .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  fde = frchain_now->frch_cfi_data->cur_fde_data;
  encoding = cfi_parse_const ();
  if (encoding == DW_EH_PE_omit)
    {
      demand_empty_rest_of_line ();
      fde->per_encoding = encoding;
      return;
    }

  if ((encoding & 0xff) != encoding
      || ((((encoding & 0x70) != 0
#if CFI_DIFF_EXPR_OK || defined tc_cfi_emit_pcrel_expr
	    && (encoding & 0x70) != DW_EH_PE_pcrel
#endif
	    )
	   /* leb128 can be handled, but does something actually need it?  */
	   || (encoding & 7) == DW_EH_PE_uleb128
	   || (encoding & 7) > DW_EH_PE_udata8)
	  && tc_cfi_reloc_for_encoding (encoding) == BFD_RELOC_NONE))
    {
      as_bad (_("invalid or unsupported encoding in .cfi_personality"));
      ignore_rest_of_line ();
      return;
    }

  if (*input_line_pointer++ != ',')
    {
      as_bad (_(".cfi_personality requires encoding and symbol arguments"));
      ignore_rest_of_line ();
      return;
    }

  expression_and_evaluate (&fde->personality);
  switch (fde->personality.X_op)
    {
    case O_symbol:
      break;
    case O_constant:
      if ((encoding & 0x70) == DW_EH_PE_pcrel)
	encoding = DW_EH_PE_omit;
      break;
    default:
      encoding = DW_EH_PE_omit;
      break;
    }

  fde->per_encoding = encoding;

  if (encoding == DW_EH_PE_omit)
    {
      as_bad (_("wrong second argument to .cfi_personality"));
      ignore_rest_of_line ();
      return;
    }

  demand_empty_rest_of_line ();
}

static void
dot_cfi_lsda (int ignored ATTRIBUTE_UNUSED)
{
  struct fde_entry *fde;
  offsetT encoding;

  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_("CFI instruction used without previous .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  fde = frchain_now->frch_cfi_data->cur_fde_data;
  encoding = cfi_parse_const ();
  if (encoding == DW_EH_PE_omit)
    {
      demand_empty_rest_of_line ();
      fde->lsda_encoding = encoding;
      return;
    }

  if ((encoding & 0xff) != encoding
      || ((((encoding & 0x70) != 0
#if CFI_DIFF_LSDA_OK || defined tc_cfi_emit_pcrel_expr
	    && (encoding & 0x70) != DW_EH_PE_pcrel
#endif
	    )
	   /* leb128 can be handled, but does something actually need it?  */
	   || (encoding & 7) == DW_EH_PE_uleb128
	   || (encoding & 7) > DW_EH_PE_udata8)
	  && tc_cfi_reloc_for_encoding (encoding) == BFD_RELOC_NONE))
    {
      as_bad (_("invalid or unsupported encoding in .cfi_lsda"));
      ignore_rest_of_line ();
      return;
    }

  if (*input_line_pointer++ != ',')
    {
      as_bad (_(".cfi_lsda requires encoding and symbol arguments"));
      ignore_rest_of_line ();
      return;
    }

  fde->lsda_encoding = encoding;

  expression_and_evaluate (&fde->lsda);
  switch (fde->lsda.X_op)
    {
    case O_symbol:
      break;
    case O_constant:
      if ((encoding & 0x70) == DW_EH_PE_pcrel)
	encoding = DW_EH_PE_omit;
      break;
    default:
      encoding = DW_EH_PE_omit;
      break;
    }

  fde->lsda_encoding = encoding;

  if (encoding == DW_EH_PE_omit)
    {
      as_bad (_("wrong second argument to .cfi_lsda"));
      ignore_rest_of_line ();
      return;
    }

  demand_empty_rest_of_line ();
}

static void
dot_cfi_val_encoded_addr (int ignored ATTRIBUTE_UNUSED)
{
  struct cfi_insn_data *insn_ptr;
  offsetT encoding;

  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_("CFI instruction used without previous .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  /* If the last address was not at the current PC, advance to current.  */
  if (symbol_get_frag (frchain_now->frch_cfi_data->last_address) != frag_now
      || (S_GET_VALUE (frchain_now->frch_cfi_data->last_address)
	  != frag_now_fix ()))
    cfi_add_advance_loc (symbol_temp_new_now ());

  insn_ptr = alloc_cfi_insn_data ();
  insn_ptr->insn = CFI_val_encoded_addr;

  insn_ptr->u.ea.reg = cfi_parse_reg ();

  cfi_parse_separator ();
  encoding = cfi_parse_const ();
  if ((encoding & 0xff) != encoding
      || ((encoding & 0x70) != 0
#if CFI_DIFF_EXPR_OK || defined tc_cfi_emit_pcrel_expr
	  && (encoding & 0x70) != DW_EH_PE_pcrel
#endif
	  )
      /* leb128 can be handled, but does something actually need it?  */
      || (encoding & 7) == DW_EH_PE_uleb128
      || (encoding & 7) > DW_EH_PE_udata8)
    {
      as_bad (_("invalid or unsupported encoding in .cfi_lsda"));
      encoding = DW_EH_PE_omit;
    }

  cfi_parse_separator ();
  expression_and_evaluate (&insn_ptr->u.ea.exp);
  switch (insn_ptr->u.ea.exp.X_op)
    {
    case O_symbol:
      break;
    case O_constant:
      if ((encoding & 0x70) != DW_EH_PE_pcrel)
	break;
      /* Fall through.  */
    default:
      encoding = DW_EH_PE_omit;
      break;
    }

  insn_ptr->u.ea.encoding = encoding;
  if (encoding == DW_EH_PE_omit)
    {
      as_bad (_("wrong third argument to .cfi_val_encoded_addr"));
      ignore_rest_of_line ();
      return;
    }

  demand_empty_rest_of_line ();
}

static void
dot_cfi_label (int ignored ATTRIBUTE_UNUSED)
{
  char *name;

  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_("CFI instruction used without previous .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  name = read_symbol_name ();
  if (name == NULL)
    return;

  /* If the last address was not at the current PC, advance to current.  */
  if (symbol_get_frag (frchain_now->frch_cfi_data->last_address) != frag_now
      || (S_GET_VALUE (frchain_now->frch_cfi_data->last_address)
	  != frag_now_fix ()))
    cfi_add_advance_loc (symbol_temp_new_now ());

  cfi_add_label (name);
  free (name);

  demand_empty_rest_of_line ();
}

static void
dot_cfi_sections (int ignored ATTRIBUTE_UNUSED)
{
  int sections = 0;

  SKIP_WHITESPACE ();
  if (is_name_beginner (*input_line_pointer) || *input_line_pointer == '"')
    while (1)
      {
	char * saved_ilp;
	char *name, c;

	saved_ilp = input_line_pointer;
	c = get_symbol_name (& name);

	if (startswith (name, ".eh_frame")
	    && name[9] != '_')
	  sections |= CFI_EMIT_eh_frame;
	else if (startswith (name, ".debug_frame"))
	  sections |= CFI_EMIT_debug_frame;
#if SUPPORT_COMPACT_EH
	else if (startswith (name, ".eh_frame_entry"))
	  {
	    compact_eh = true;
	    sections |= CFI_EMIT_eh_frame_compact;
	  }
#endif
#ifdef tc_cfi_section_name
	else if (strcmp (name, tc_cfi_section_name) == 0)
	  sections |= CFI_EMIT_target;
#endif
	else if (startswith (name, ".sframe"))
	    sections |= CFI_EMIT_sframe;
	else
	  {
	    *input_line_pointer = c;
	    input_line_pointer = saved_ilp;
	    break;
	  }

	*input_line_pointer = c;
	SKIP_WHITESPACE_AFTER_NAME ();
	if (*input_line_pointer == ',')
	  {
	    name = input_line_pointer++;
	    SKIP_WHITESPACE ();
	    if (!is_name_beginner (*input_line_pointer)
		&& *input_line_pointer != '"')
	      {
		input_line_pointer = name;
		break;
	      }
	  }
	else if (is_name_beginner (*input_line_pointer)
		 || *input_line_pointer == '"')
	  break;
      }

  demand_empty_rest_of_line ();
  if (cfi_sections_set
      && (sections & (CFI_EMIT_eh_frame | CFI_EMIT_eh_frame_compact))
      && ((cfi_sections & (CFI_EMIT_eh_frame | CFI_EMIT_eh_frame_compact))
	  != (sections & (CFI_EMIT_eh_frame | CFI_EMIT_eh_frame_compact))))
    as_bad (_("inconsistent uses of .cfi_sections"));
  cfi_sections = sections;
}

static void
dot_cfi_startproc (int ignored ATTRIBUTE_UNUSED)
{
  int simple = 0;

  if (frchain_now->frch_cfi_data != NULL)
    {
      as_bad (_("previous CFI entry not closed (missing .cfi_endproc)"));
      ignore_rest_of_line ();
      return;
    }

  cfi_new_fde (symbol_temp_new_now ());

  SKIP_WHITESPACE ();
  if (is_name_beginner (*input_line_pointer) || *input_line_pointer == '"')
    {
      char * saved_ilp = input_line_pointer;
      char *name, c;

      c = get_symbol_name (& name);

      if (strcmp (name, "simple") == 0)
	{
	  simple = 1;
	  restore_line_pointer (c);
	}
      else
	input_line_pointer = saved_ilp;
    }
  demand_empty_rest_of_line ();

  cfi_sections_set = true;
  all_cfi_sections |= cfi_sections;
  cfi_set_sections ();
  frchain_now->frch_cfi_data->cur_cfa_offset = 0;
  if (!simple)
    tc_cfi_frame_initial_instructions ();

  if ((cfi_sections & CFI_EMIT_target) != 0)
    tc_cfi_startproc ();
}

static void
dot_cfi_endproc (int ignored ATTRIBUTE_UNUSED)
{
  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_(".cfi_endproc without corresponding .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  last_fde = frchain_now->frch_cfi_data->cur_fde_data;

  cfi_end_fde (symbol_temp_new_now ());

  demand_empty_rest_of_line ();

  cfi_sections_set = true;
  if ((cfi_sections & CFI_EMIT_target) != 0)
    tc_cfi_endproc (last_fde);
}

static segT
get_cfi_seg (segT cseg, const char *base, flagword flags, int align)
{
  /* Exclude .debug_frame sections for Compact EH.  */
  if (SUPPORT_FRAME_LINKONCE || ((flags & SEC_DEBUGGING) == 0 && compact_eh)
      || ((flags & SEC_DEBUGGING) == 0 && TARGET_MULTIPLE_EH_FRAME_SECTIONS))
    {
      segT iseg = cseg;
      struct dwcfi_seg_list *l;

      l = dwcfi_hash_find_or_make (cseg, base, flags);

      cseg = l->seg;
      subseg_set (cseg, l->subseg);

      if (TARGET_MULTIPLE_EH_FRAME_SECTIONS
	  && (flags & DWARF2_EH_FRAME_READ_ONLY))
	{
	  const frchainS *ifrch = seg_info (iseg)->frchainP;
	  const frchainS *frch = seg_info (cseg)->frchainP;
	  expressionS exp;

	  exp.X_op = O_symbol;
	  exp.X_add_symbol = (symbolS *) local_symbol_make (cseg->name, cseg, frch->frch_root, 0);
	  exp.X_add_number = 0;
	  subseg_set (iseg, ifrch->frch_subseg);
	  fix_new_exp (ifrch->frch_root, 0, 0, &exp, 0, BFD_RELOC_NONE);

	  /* Restore the original segment info.  */
	  subseg_set (cseg, l->subseg);
	}
    }
  else
    {
      cseg = subseg_new (base, 0);
      bfd_set_section_flags (cseg, flags);
    }
  record_alignment (cseg, align);
  return cseg;
}

#if SUPPORT_COMPACT_EH
static void
dot_cfi_personality_id (int ignored ATTRIBUTE_UNUSED)
{
  struct fde_entry *fde;

  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_("CFI instruction used without previous .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  fde = frchain_now->frch_cfi_data->cur_fde_data;
  fde->personality_id = cfi_parse_const ();
  demand_empty_rest_of_line ();

  if (fde->personality_id == 0 || fde->personality_id > 3)
    {
      as_bad (_("wrong argument to .cfi_personality_id"));
      return;
    }
}

static void
dot_cfi_fde_data (int ignored ATTRIBUTE_UNUSED)
{
  if (frchain_now->frch_cfi_data == NULL)
    {
      as_bad (_(".cfi_fde_data without corresponding .cfi_startproc"));
      ignore_rest_of_line ();
      return;
    }

  last_fde = frchain_now->frch_cfi_data->cur_fde_data;

  cfi_sections_set = true;
  if ((cfi_sections & CFI_EMIT_target) != 0
      || (cfi_sections & CFI_EMIT_eh_frame_compact) != 0)
    {
      struct cfi_escape_data *head, **tail, *e;
      int num_ops = 0;

      tail = &head;
      if (!is_it_end_of_statement ())
	{
	  num_ops = 0;
	  do
	    {
	      e = XNEW (struct cfi_escape_data);
	      do_parse_cons_expression (&e->exp, 1);
	      *tail = e;
	      tail = &e->next;
	      num_ops++;
	    }
	  while (*input_line_pointer++ == ',');
	  --input_line_pointer;
	}
      *tail = NULL;

      if (last_fde->lsda_encoding != DW_EH_PE_omit)
	last_fde->eh_header_type = EH_COMPACT_HAS_LSDA;
      else if (num_ops <= 3 && last_fde->per_encoding == DW_EH_PE_omit)
	last_fde->eh_header_type = EH_COMPACT_INLINE;
      else
	last_fde->eh_header_type = EH_COMPACT_OUTLINE;

      if (last_fde->eh_header_type == EH_COMPACT_INLINE)
	num_ops = 3;

      last_fde->eh_data_size = num_ops;
      last_fde->eh_data =  XNEWVEC (bfd_byte, num_ops);
      num_ops = 0;
      while (head)
	{
	  e = head;
	  head = e->next;
	  last_fde->eh_data[num_ops++] = e->exp.X_add_number;
	  free (e);
	}
      if (last_fde->eh_header_type == EH_COMPACT_INLINE)
	while (num_ops < 3)
	  last_fde->eh_data[num_ops++] = tc_compact_eh_opcode_stop;
    }

  demand_empty_rest_of_line ();
}

/* Function to emit the compact unwinding opcodes stored in the
   fde's eh_data field.  The end of the opcode data will be
   padded to the value in align.  */

static void
output_compact_unwind_data (struct fde_entry *fde, int align)
{
  int data_size = fde->eh_data_size + 2;
  int align_padding;
  int amask;
  char *p;

  fde->eh_loc = symbol_temp_new_now ();

  p = frag_more (1);
  if (fde->personality_id != 0)
    *p = fde->personality_id;
  else if (fde->per_encoding != DW_EH_PE_omit)
    {
      *p = 0;
      emit_expr_encoded (&fde->personality, fde->per_encoding, false);
      data_size += encoding_size (fde->per_encoding);
    }
  else
    *p = 1;

  amask = (1 << align) - 1;
  align_padding = ((data_size + amask) & ~amask) - data_size;

  p = frag_more (fde->eh_data_size + 1 + align_padding);
  memcpy (p, fde->eh_data, fde->eh_data_size);
  p += fde->eh_data_size;

  while (align_padding-- > 0)
    *(p++) = tc_compact_eh_opcode_pad;

  *(p++) = tc_compact_eh_opcode_stop;
  fde->eh_header_type = EH_COMPACT_OUTLINE_DONE;
}

/* Handle the .cfi_inline_lsda directive.  */
static void
dot_cfi_inline_lsda (int ignored ATTRIBUTE_UNUSED)
{
  segT ccseg;
  int align;
  long max_alignment = 28;

  if (!last_fde)
    {
      as_bad (_("unexpected .cfi_inline_lsda"));
      ignore_rest_of_line ();
      return;
    }

  if ((last_fde->sections & CFI_EMIT_eh_frame_compact) == 0)
    {
      as_bad (_(".cfi_inline_lsda not valid for this frame"));
      ignore_rest_of_line ();
      return;
    }

  if (last_fde->eh_header_type != EH_COMPACT_UNKNOWN
      && last_fde->eh_header_type != EH_COMPACT_HAS_LSDA)
    {
      as_bad (_(".cfi_inline_lsda seen for frame without .cfi_lsda"));
      ignore_rest_of_line ();
      return;
    }

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  align = get_absolute_expression ();
  if (align > max_alignment)
    {
      align = max_alignment;
      as_bad (_("Alignment too large: %d. assumed."), align);
    }
  else if (align < 0)
    {
      as_warn (_("Alignment negative: 0 assumed."));
      align = 0;
    }

  demand_empty_rest_of_line ();
  ccseg = CUR_SEG (last_fde);

  /* Open .gnu_extab section.  */
  get_cfi_seg (ccseg, ".gnu_extab",
	       (SEC_ALLOC | SEC_LOAD | SEC_DATA
		| DWARF2_EH_FRAME_READ_ONLY),
	       1);

  frag_align (align, 0, 0);
  record_alignment (now_seg, align);
  if (last_fde->eh_header_type == EH_COMPACT_HAS_LSDA)
    output_compact_unwind_data (last_fde, align);

  last_fde = NULL;

  return;
}
#else /* !SUPPORT_COMPACT_EH */
static void
dot_cfi_inline_lsda (int ignored ATTRIBUTE_UNUSED)
{
  as_bad (_(".cfi_inline_lsda is not supported for this target"));
  ignore_rest_of_line ();
}

static void
dot_cfi_fde_data (int ignored ATTRIBUTE_UNUSED)
{
  as_bad (_(".cfi_fde_data is not supported for this target"));
  ignore_rest_of_line ();
}

static void
dot_cfi_personality_id (int ignored ATTRIBUTE_UNUSED)
{
  as_bad (_(".cfi_personality_id is not supported for this target"));
  ignore_rest_of_line ();
}
#endif

static void
output_cfi_insn (struct cfi_insn_data *insn)
{
  offsetT offset;
  unsigned int regno;

  switch (insn->insn)
    {
    case DW_CFA_advance_loc:
      {
	symbolS *from = insn->u.ll.lab1;
	symbolS *to = insn->u.ll.lab2;

	if (symbol_get_frag (to) == symbol_get_frag (from))
	  {
	    addressT delta = S_GET_VALUE (to) - S_GET_VALUE (from);
	    addressT scaled = delta / DWARF2_LINE_MIN_INSN_LENGTH;

	    if (scaled == 0)
	      ;
	    else if (scaled <= 0x3F)
	      out_one (DW_CFA_advance_loc + scaled);
	    else if (scaled <= 0xFF)
	      {
		out_one (DW_CFA_advance_loc1);
		out_one (scaled);
	      }
	    else if (scaled <= 0xFFFF)
	      {
		out_one (DW_CFA_advance_loc2);
		out_two (scaled);
	      }
	    else
	      {
		out_one (DW_CFA_advance_loc4);
		out_four (scaled);
	      }
	  }
	else
	  {
	    expressionS exp;

	    exp.X_op = O_subtract;
	    exp.X_add_symbol = to;
	    exp.X_op_symbol = from;
	    exp.X_add_number = 0;

	    /* The code in ehopt.c expects that one byte of the encoding
	       is already allocated to the frag.  This comes from the way
	       that it scans the .eh_frame section looking first for the
	       .byte DW_CFA_advance_loc4.  Call frag_grow with the sum of
	       room needed by frag_more and frag_var to preallocate space
	       ensuring that the DW_CFA_advance_loc4 is in the fixed part
	       of the rs_cfa frag, so that the relax machinery can remove
	       the advance_loc should it advance by zero.  */
	    frag_grow (5);
	    *frag_more (1) = DW_CFA_advance_loc4;

	    frag_var (rs_cfa, 4, 0, DWARF2_LINE_MIN_INSN_LENGTH << 3,
		      make_expr_symbol (&exp), frag_now_fix () - 1,
		      (char *) frag_now);
	  }
      }
      break;

    case DW_CFA_def_cfa:
      offset = insn->u.ri.offset;
      if (offset < 0)
	{
	  out_one (DW_CFA_def_cfa_sf);
	  out_uleb128 (insn->u.ri.reg);
	  out_sleb128 (offset / DWARF2_CIE_DATA_ALIGNMENT);
	}
      else
	{
	  out_one (DW_CFA_def_cfa);
	  out_uleb128 (insn->u.ri.reg);
	  out_uleb128 (offset);
	}
      break;

    case DW_CFA_def_cfa_register:
    case DW_CFA_undefined:
    case DW_CFA_same_value:
      out_one (insn->insn);
      out_uleb128 (insn->u.r);
      break;

    case DW_CFA_def_cfa_offset:
      offset = insn->u.i;
      if (offset < 0)
	{
	  out_one (DW_CFA_def_cfa_offset_sf);
	  out_sleb128 (offset / DWARF2_CIE_DATA_ALIGNMENT);
	}
      else
	{
	  out_one (DW_CFA_def_cfa_offset);
	  out_uleb128 (offset);
	}
      break;

    case DW_CFA_restore:
      regno = insn->u.r;
      if (regno <= 0x3F)
	{
	  out_one (DW_CFA_restore + regno);
	}
      else
	{
	  out_one (DW_CFA_restore_extended);
	  out_uleb128 (regno);
	}
      break;

    case DW_CFA_offset:
      regno = insn->u.ri.reg;
      offset = insn->u.ri.offset / DWARF2_CIE_DATA_ALIGNMENT;
      if (offset < 0)
	{
	  out_one (DW_CFA_offset_extended_sf);
	  out_uleb128 (regno);
	  out_sleb128 (offset);
	}
      else if (regno <= 0x3F)
	{
	  out_one (DW_CFA_offset + regno);
	  out_uleb128 (offset);
	}
      else
	{
	  out_one (DW_CFA_offset_extended);
	  out_uleb128 (regno);
	  out_uleb128 (offset);
	}
      break;

    case DW_CFA_val_offset:
      regno = insn->u.ri.reg;
      offset = insn->u.ri.offset / DWARF2_CIE_DATA_ALIGNMENT;
      if (offset < 0)
	{
	  out_one (DW_CFA_val_offset_sf);
	  out_uleb128 (regno);
	  out_sleb128 (offset);
	}
      else
	{
	  out_one (DW_CFA_val_offset);
	  out_uleb128 (regno);
	  out_uleb128 (offset);
	}
      break;

    case DW_CFA_register:
      out_one (DW_CFA_register);
      out_uleb128 (insn->u.rr.reg1);
      out_uleb128 (insn->u.rr.reg2);
      break;

    case DW_CFA_remember_state:
    case DW_CFA_restore_state:
      out_one (insn->insn);
      break;

    case DW_CFA_GNU_window_save:
      out_one (DW_CFA_GNU_window_save);
      break;

    case CFI_escape:
      {
	struct cfi_escape_data *e;
	for (e = insn->u.esc; e ; e = e->next)
	  emit_expr (&e->exp, 1);
	break;
      }

    case CFI_val_encoded_addr:
      {
	unsigned encoding = insn->u.ea.encoding;
	offsetT enc_size;

	if (encoding == DW_EH_PE_omit)
	  break;
	out_one (DW_CFA_val_expression);
	out_uleb128 (insn->u.ea.reg);

	switch (encoding & 0x7)
	  {
	  case DW_EH_PE_absptr:
	    enc_size = DWARF2_ADDR_SIZE (stdoutput);
	    break;
	  case DW_EH_PE_udata2:
	    enc_size = 2;
	    break;
	  case DW_EH_PE_udata4:
	    enc_size = 4;
	    break;
	  case DW_EH_PE_udata8:
	    enc_size = 8;
	    break;
	  default:
	    abort ();
	  }

	/* If the user has requested absolute encoding,
	   then use the smaller DW_OP_addr encoding.  */
	if (insn->u.ea.encoding == DW_EH_PE_absptr)
	  {
	    out_uleb128 (1 + enc_size);
	    out_one (DW_OP_addr);
	  }
	else
	  {
	    out_uleb128 (1 + 1 + enc_size);
	    out_one (DW_OP_GNU_encoded_addr);
	    out_one (encoding);

	    if ((encoding & 0x70) == DW_EH_PE_pcrel)
	      {
#if CFI_DIFF_EXPR_OK
		insn->u.ea.exp.X_op = O_subtract;
		insn->u.ea.exp.X_op_symbol = symbol_temp_new_now ();
#elif defined (tc_cfi_emit_pcrel_expr)
		tc_cfi_emit_pcrel_expr (&insn->u.ea.exp, enc_size);
		break;
#else
		abort ();
#endif
	      }
	  }
	emit_expr (&insn->u.ea.exp, enc_size);
      }
      break;

    case CFI_label:
      colon (insn->u.sym_name);
      break;

    default:
      abort ();
    }
}

static void
output_cie (struct cie_entry *cie, bool eh_frame, int align)
{
  symbolS *after_size_address, *end_address;
  expressionS exp;
  struct cfi_insn_data *i;
  offsetT augmentation_size;
  int enc;
  enum dwarf2_format fmt = DWARF2_FORMAT (now_seg);

  cie->start_address = symbol_temp_new_now ();
  after_size_address = symbol_temp_make ();
  end_address = symbol_temp_make ();

  exp.X_op = O_subtract;
  exp.X_add_symbol = end_address;
  exp.X_op_symbol = after_size_address;
  exp.X_add_number = 0;

  if (eh_frame || fmt == dwarf2_format_32bit)
    emit_expr (&exp, 4);			/* Length.  */
  else
    {
      if (fmt == dwarf2_format_64bit)
	out_four (-1);
      emit_expr (&exp, 8);			/* Length.  */
    }
  symbol_set_value_now (after_size_address);
  if (eh_frame)
    out_four (0);				/* CIE id.  */
  else
    {
      out_four (-1);				/* CIE id.  */
      if (fmt != dwarf2_format_32bit)
	out_four (-1);
    }
  out_one (flag_dwarf_cie_version);		/* Version.  */
  if (eh_frame)
    {
      out_one ('z');				/* Augmentation.  */
      if (cie->per_encoding != DW_EH_PE_omit)
	out_one ('P');
      if (cie->lsda_encoding != DW_EH_PE_omit)
	out_one ('L');
      out_one ('R');
#ifdef tc_output_cie_extra
      tc_output_cie_extra (cie);
#endif
    }
  if (cie->signal_frame)
    out_one ('S');
  out_one (0);
  if (flag_dwarf_cie_version >= 4)
    {
      /* For now we are assuming a flat address space with 4 or 8 byte
         addresses.  */
      int address_size = dwarf2_format_32bit ? 4 : 8;
      out_one (address_size);			/* Address size.  */
      out_one (0);				/* Segment size.  */
    }
  out_uleb128 (DWARF2_LINE_MIN_INSN_LENGTH);	/* Code alignment.  */
  out_sleb128 (DWARF2_CIE_DATA_ALIGNMENT);	/* Data alignment.  */
  if (flag_dwarf_cie_version == 1)		/* Return column.  */
    {
      if ((cie->return_column & 0xff) != cie->return_column)
	as_bad (_("return column number %d overflows in CIE version 1"),
		cie->return_column);
      out_one (cie->return_column);
    }
  else
    out_uleb128 (cie->return_column);
  if (eh_frame)
    {
      augmentation_size = 1 + (cie->lsda_encoding != DW_EH_PE_omit);
      if (cie->per_encoding != DW_EH_PE_omit)
	augmentation_size += 1 + encoding_size (cie->per_encoding);
      out_uleb128 (augmentation_size);		/* Augmentation size.  */

      emit_expr_encoded (&cie->personality, cie->per_encoding, true);

      if (cie->lsda_encoding != DW_EH_PE_omit)
	out_one (cie->lsda_encoding);
    }

  switch (DWARF2_FDE_RELOC_SIZE)
    {
    case 2:
      enc = DW_EH_PE_sdata2;
      break;
    case 4:
      enc = DW_EH_PE_sdata4;
      break;
    case 8:
      enc = DW_EH_PE_sdata8;
      break;
    default:
      abort ();
    }
#if CFI_DIFF_EXPR_OK || defined tc_cfi_emit_pcrel_expr
  enc |= DW_EH_PE_pcrel;
#endif
#ifdef DWARF2_FDE_RELOC_ENCODING
  /* Allow target to override encoding.  */
  enc = DWARF2_FDE_RELOC_ENCODING (enc);
#endif
  cie->fde_encoding = enc;
  if (eh_frame)
    out_one (enc);

  if (cie->first)
    {
      for (i = cie->first; i != cie->last; i = i->next)
	{
	  if (CUR_SEG (i) != CUR_SEG (cie))
	    continue;
	  output_cfi_insn (i);
	}
    }

  frag_align (align, DW_CFA_nop, 0);
  symbol_set_value_now (end_address);
}

static void
output_fde (struct fde_entry *fde, struct cie_entry *cie,
	    bool eh_frame, struct cfi_insn_data *first,
	    int align)
{
  symbolS *after_size_address, *end_address;
  expressionS exp;
  offsetT augmentation_size;
  enum dwarf2_format fmt = DWARF2_FORMAT (now_seg);
  unsigned int offset_size;
  unsigned int addr_size;

  after_size_address = symbol_temp_make ();
  end_address = symbol_temp_make ();

  exp.X_op = O_subtract;
  exp.X_add_symbol = end_address;
  exp.X_op_symbol = after_size_address;
  exp.X_add_number = 0;
  if (eh_frame || fmt == dwarf2_format_32bit)
    offset_size = 4;
  else
    {
      if (fmt == dwarf2_format_64bit)
	out_four (-1);
      offset_size = 8;
    }
  emit_expr (&exp, offset_size);		/* Length.  */
  symbol_set_value_now (after_size_address);

  if (eh_frame)
    {
      exp.X_op = O_subtract;
      exp.X_add_symbol = after_size_address;
      exp.X_op_symbol = cie->start_address;
      exp.X_add_number = 0;
      emit_expr (&exp, offset_size);		/* CIE offset.  */
    }
  else
    {
      TC_DWARF2_EMIT_OFFSET (cie->start_address, offset_size);
    }

  exp.X_op = O_symbol;
  if (eh_frame)
    {
      bfd_reloc_code_real_type code
	= tc_cfi_reloc_for_encoding (cie->fde_encoding);
      addr_size = DWARF2_FDE_RELOC_SIZE;
      if (code != BFD_RELOC_NONE)
	{
	  reloc_howto_type *howto = bfd_reloc_type_lookup (stdoutput, code);
	  char *p = frag_more (addr_size);
	  gas_assert (addr_size == (unsigned) howto->bitsize / 8);
	  md_number_to_chars (p, 0, addr_size);
	  fix_new (frag_now, p - frag_now->fr_literal, addr_size,
		   fde->start_address, 0, howto->pc_relative, code);
	}
      else
	{
	  exp.X_op = O_subtract;
	  exp.X_add_number = 0;
#if CFI_DIFF_EXPR_OK
	  exp.X_add_symbol = fde->start_address;
	  exp.X_op_symbol = symbol_temp_new_now ();
	  emit_expr (&exp, addr_size);	/* Code offset.  */
#else
	  exp.X_op = O_symbol;
	  exp.X_add_symbol = fde->start_address;

#if defined(tc_cfi_emit_pcrel_expr)
	  tc_cfi_emit_pcrel_expr (&exp, addr_size);	 /* Code offset.  */
#else
	  emit_expr (&exp, addr_size);	/* Code offset.  */
#endif
#endif
	}
    }
  else
    {
      exp.X_add_number = 0;
      exp.X_add_symbol = fde->start_address;
      addr_size = DWARF2_ADDR_SIZE (stdoutput);
      emit_expr (&exp, addr_size);
    }

  exp.X_op = O_subtract;
  exp.X_add_symbol = fde->end_address;
  exp.X_op_symbol = fde->start_address;		/* Code length.  */
  exp.X_add_number = 0;
  emit_expr (&exp, addr_size);

  augmentation_size = encoding_size (fde->lsda_encoding);
  if (eh_frame)
    out_uleb128 (augmentation_size);		/* Augmentation size.  */

  emit_expr_encoded (&fde->lsda, cie->lsda_encoding, false);

  for (; first; first = first->next)
    if (CUR_SEG (first) == CUR_SEG (fde))
      output_cfi_insn (first);

  frag_align (align, DW_CFA_nop, 0);
  symbol_set_value_now (end_address);
}

/* Allow these insns to be put in the initial sequence of a CIE.
   If J is non-NULL, then compare I and J insns for a match.  */

static inline bool
initial_cie_insn (const struct cfi_insn_data *i, const struct cfi_insn_data *j)
{
  if (j && i->insn != j->insn)
    return false;
  switch (i->insn)
    {
    case DW_CFA_offset:
    case DW_CFA_def_cfa:
    case DW_CFA_val_offset:
      if (j)
	{
	  if (i->u.ri.reg != j->u.ri.reg)
	    return false;
	  if (i->u.ri.offset != j->u.ri.offset)
	    return false;
	}
      break;

    case DW_CFA_register:
      if (j)
	{
	  if (i->u.rr.reg1 != j->u.rr.reg1)
	    return false;
	  if (i->u.rr.reg2 != j->u.rr.reg2)
	    return false;
	}
      break;

    case DW_CFA_def_cfa_register:
    case DW_CFA_restore:
    case DW_CFA_undefined:
    case DW_CFA_same_value:
      if (j)
	{
	  if (i->u.r != j->u.r)
	    return false;
	}
      break;

    case DW_CFA_def_cfa_offset:
      if (j)
	{
	  if (i->u.i != j->u.i)
	    return false;
	}
      break;

    default:
      return false;
    }
  return true;
}

static struct cie_entry *
select_cie_for_fde (struct fde_entry *fde, bool eh_frame,
		    struct cfi_insn_data **pfirst, int align)
{
  struct cfi_insn_data *i, *j;
  struct cie_entry *cie;

  for (cie = cie_root; cie; cie = cie->next)
    {
      if (CUR_SEG (cie) != CUR_SEG (fde))
	continue;
#ifdef tc_cie_fde_equivalent_extra
      if (!tc_cie_fde_equivalent_extra (cie, fde))
	continue;
#endif
      if (cie->return_column != fde->return_column
	  || cie->signal_frame != fde->signal_frame
	  || cie->per_encoding != fde->per_encoding
	  || cie->lsda_encoding != fde->lsda_encoding)
	continue;
      if (cie->per_encoding != DW_EH_PE_omit)
	{
	  if (cie->personality.X_op != fde->personality.X_op
	      || (cie->personality.X_add_number
		  != fde->personality.X_add_number))
	    continue;
	  switch (cie->personality.X_op)
	    {
	    case O_constant:
	      if (cie->personality.X_unsigned != fde->personality.X_unsigned)
		continue;
	      break;
	    case O_symbol:
	      if (cie->personality.X_add_symbol
		  != fde->personality.X_add_symbol)
		continue;
	      break;
	    default:
	      abort ();
	    }
	}
      for (i = cie->first, j = fde->data;
	   i != cie->last && j != NULL;
	   i = i->next, j = j->next)
	{
	  if (!initial_cie_insn (i, j))
	    break;
	}

      if (i == cie->last)
	{
	  *pfirst = j;
	  return cie;
	}
    }

  cie = XNEW (struct cie_entry);
  cie->next = cie_root;
  cie_root = cie;
  SET_CUR_SEG (cie, CUR_SEG (fde));
  cie->return_column = fde->return_column;
  cie->signal_frame = fde->signal_frame;
  cie->per_encoding = fde->per_encoding;
  cie->lsda_encoding = fde->lsda_encoding;
  cie->personality = fde->personality;
  cie->first = fde->data;
#ifdef tc_cie_entry_init_extra
  tc_cie_entry_init_extra (cie, fde)
#endif

  for (i = cie->first; i ; i = i->next)
    if (!initial_cie_insn (i, NULL))
      break;

  cie->last = i;
  *pfirst = i;

  output_cie (cie, eh_frame, align);

  return cie;
}

#ifdef md_reg_eh_frame_to_debug_frame
static void
cfi_change_reg_numbers (struct cfi_insn_data *insn, segT ccseg)
{
  for (; insn; insn = insn->next)
    {
      if (CUR_SEG (insn) != ccseg)
	continue;
      switch (insn->insn)
	{
	case DW_CFA_advance_loc:
	case DW_CFA_def_cfa_offset:
	case DW_CFA_remember_state:
	case DW_CFA_restore_state:
	case DW_CFA_GNU_window_save:
	case CFI_escape:
	case CFI_label:
	  break;

	case DW_CFA_def_cfa:
	case DW_CFA_offset:
	  insn->u.ri.reg = md_reg_eh_frame_to_debug_frame (insn->u.ri.reg);
	  break;

	case DW_CFA_def_cfa_register:
	case DW_CFA_undefined:
	case DW_CFA_same_value:
	case DW_CFA_restore:
	  insn->u.r = md_reg_eh_frame_to_debug_frame (insn->u.r);
	  break;

	case DW_CFA_register:
	  insn->u.rr.reg1 = md_reg_eh_frame_to_debug_frame (insn->u.rr.reg1);
	  insn->u.rr.reg2 = md_reg_eh_frame_to_debug_frame (insn->u.rr.reg2);
	  break;

	case CFI_val_encoded_addr:
	  insn->u.ea.reg = md_reg_eh_frame_to_debug_frame (insn->u.ea.reg);
	  break;

	default:
	  abort ();
	}
    }
}
#else
#define cfi_change_reg_numbers(insn, cseg) do { } while (0)
#endif

#if SUPPORT_COMPACT_EH
static void
cfi_emit_eh_header (symbolS *sym, bfd_vma addend)
{
  expressionS exp;

  exp.X_add_number = addend;
  exp.X_add_symbol = sym;
  emit_expr_encoded (&exp, DW_EH_PE_sdata4 | DW_EH_PE_pcrel, false);
}

static void
output_eh_header (struct fde_entry *fde)
{
  char *p;
  bfd_vma addend;

  if (fde->eh_header_type == EH_COMPACT_INLINE)
    addend = 0;
  else
    addend = 1;

  cfi_emit_eh_header (fde->start_address, addend);

  if (fde->eh_header_type == EH_COMPACT_INLINE)
    {
      p = frag_more (4);
      /* Inline entries always use PR1.  */
      *(p++) = 1;
      memcpy(p, fde->eh_data, 3);
    }
  else
    {
      if (fde->eh_header_type == EH_COMPACT_LEGACY)
	addend = 1;
      else if (fde->eh_header_type == EH_COMPACT_OUTLINE
	       || fde->eh_header_type == EH_COMPACT_OUTLINE_DONE)
	addend = 0;
      else
	abort ();
      cfi_emit_eh_header (fde->eh_loc, addend);
    }
}
#endif

void
cfi_finish (void)
{
  struct cie_entry *cie, *cie_next;
  segT cfi_seg, ccseg;
  struct fde_entry *fde;
  struct cfi_insn_data *first;
  int save_flag_traditional_format, seek_next_seg;

  if (all_fde_data == 0)
    return;

  cfi_sections_set = true;
  if ((all_cfi_sections & CFI_EMIT_eh_frame) != 0
      || (all_cfi_sections & CFI_EMIT_eh_frame_compact) != 0)
    {
      /* Make sure check_eh_frame doesn't do anything with our output.  */
      save_flag_traditional_format = flag_traditional_format;
      flag_traditional_format = 1;

      if (!EH_FRAME_LINKONCE)
	{
	  /* Open .eh_frame section.  */
	  cfi_seg = get_cfi_seg (NULL, ".eh_frame",
				 (SEC_ALLOC | SEC_LOAD | SEC_DATA
				  | DWARF2_EH_FRAME_READ_ONLY),
				 EH_FRAME_ALIGNMENT);
#ifdef md_fix_up_eh_frame
	  md_fix_up_eh_frame (cfi_seg);
#else
	  (void) cfi_seg;
#endif
	}

      do
	{
	  ccseg = NULL;
	  seek_next_seg = 0;

	  for (cie = cie_root; cie; cie = cie_next)
	    {
	      cie_next = cie->next;
	      free ((void *) cie);
	    }
	  cie_root = NULL;

	  for (fde = all_fde_data; fde ; fde = fde->next)
	    {
	      if ((fde->sections & CFI_EMIT_eh_frame) == 0
		  && (fde->sections & CFI_EMIT_eh_frame_compact) == 0)
		continue;

#if SUPPORT_COMPACT_EH
	      /* Emit a LEGACY format header if we have processed all
		 of the .cfi directives without encountering either inline or
		 out-of-line compact unwinding opcodes.  */
	      if (fde->eh_header_type == EH_COMPACT_HAS_LSDA
		  || fde->eh_header_type == EH_COMPACT_UNKNOWN)
		fde->eh_header_type = EH_COMPACT_LEGACY;

	      if (fde->eh_header_type != EH_COMPACT_LEGACY)
		continue;
#endif
	      if (EH_FRAME_LINKONCE)
		{
		  if (HANDLED (fde))
		    continue;
		  if (seek_next_seg && CUR_SEG (fde) != ccseg)
		    {
		      seek_next_seg = 2;
		      continue;
		    }
		  if (!seek_next_seg)
		    {
		      ccseg = CUR_SEG (fde);
		      /* Open .eh_frame section.  */
		      cfi_seg = get_cfi_seg (ccseg, ".eh_frame",
					     (SEC_ALLOC | SEC_LOAD | SEC_DATA
					      | DWARF2_EH_FRAME_READ_ONLY),
					     EH_FRAME_ALIGNMENT);
#ifdef md_fix_up_eh_frame
		      md_fix_up_eh_frame (cfi_seg);
#else
		      (void) cfi_seg;
#endif
		      seek_next_seg = 1;
		    }
		  SET_HANDLED (fde, 1);
		}

	      if (fde->end_address == NULL)
		{
		  as_bad (_("open CFI at the end of file; "
			    "missing .cfi_endproc directive"));
		  fde->end_address = fde->start_address;
		}

	      cie = select_cie_for_fde (fde, true, &first, 2);
	      fde->eh_loc = symbol_temp_new_now ();
	      output_fde (fde, cie, true, first,
			  fde->next == NULL ? EH_FRAME_ALIGNMENT : 2);
	    }
	}
      while (EH_FRAME_LINKONCE && seek_next_seg == 2);

      if (EH_FRAME_LINKONCE)
	for (fde = all_fde_data; fde ; fde = fde->next)
	  SET_HANDLED (fde, 0);

#if SUPPORT_COMPACT_EH
      if (compact_eh)
	{
	  /* Create remaining out of line table entries.  */
	  do
	    {
	      ccseg = NULL;
	      seek_next_seg = 0;

	      for (fde = all_fde_data; fde ; fde = fde->next)
		{
		  if ((fde->sections & CFI_EMIT_eh_frame) == 0
		      && (fde->sections & CFI_EMIT_eh_frame_compact) == 0)
		    continue;

		  if (fde->eh_header_type != EH_COMPACT_OUTLINE)
		    continue;
		  if (HANDLED (fde))
		    continue;
		  if (seek_next_seg && CUR_SEG (fde) != ccseg)
		    {
		      seek_next_seg = 2;
		      continue;
		    }
		  if (!seek_next_seg)
		    {
		      ccseg = CUR_SEG (fde);
		      /* Open .gnu_extab section.  */
		      get_cfi_seg (ccseg, ".gnu_extab",
				   (SEC_ALLOC | SEC_LOAD | SEC_DATA
				    | DWARF2_EH_FRAME_READ_ONLY),
				   1);
		      seek_next_seg = 1;
		    }
		  SET_HANDLED (fde, 1);

		  frag_align (1, 0, 0);
		  record_alignment (now_seg, 1);
		  output_compact_unwind_data (fde, 1);
		}
	    }
	  while (EH_FRAME_LINKONCE && seek_next_seg == 2);

	  for (fde = all_fde_data; fde ; fde = fde->next)
	    SET_HANDLED (fde, 0);

	  /* Create index table fragments.  */
	  do
	    {
	      ccseg = NULL;
	      seek_next_seg = 0;

	      for (fde = all_fde_data; fde ; fde = fde->next)
		{
		  if ((fde->sections & CFI_EMIT_eh_frame) == 0
		      && (fde->sections & CFI_EMIT_eh_frame_compact) == 0)
		    continue;

		  if (HANDLED (fde))
		    continue;
		  if (seek_next_seg && CUR_SEG (fde) != ccseg)
		    {
		      seek_next_seg = 2;
		      continue;
		    }
		  if (!seek_next_seg)
		    {
		      ccseg = CUR_SEG (fde);
		      /* Open .eh_frame_entry section.  */
		      cfi_seg = get_cfi_seg (ccseg, ".eh_frame_entry",
					     (SEC_ALLOC | SEC_LOAD | SEC_DATA
					      | DWARF2_EH_FRAME_READ_ONLY),
					     2);
		      seek_next_seg = 1;
		    }
		  SET_HANDLED (fde, 1);

		  output_eh_header (fde);
		}
	    }
	  while (seek_next_seg == 2);

	  for (fde = all_fde_data; fde ; fde = fde->next)
	    SET_HANDLED (fde, 0);
	}
#endif /* SUPPORT_COMPACT_EH */

      flag_traditional_format = save_flag_traditional_format;
    }

  cfi_sections_set = true;
  /* Generate SFrame section if the user specifies:
	- the command line option to gas, or
	- .sframe in the .cfi_sections directive.  */
  if (flag_gen_sframe || (all_cfi_sections & CFI_EMIT_sframe) != 0)
    {
      if (support_sframe_p ())
	{
	  segT sframe_seg;
	  int alignment = ffs (DWARF2_ADDR_SIZE (stdoutput)) - 1;

	  if (!SUPPORT_FRAME_LINKONCE)
	    sframe_seg = get_cfi_seg (NULL, ".sframe",
					 (SEC_ALLOC | SEC_LOAD | SEC_DATA
					  | DWARF2_EH_FRAME_READ_ONLY),
					 alignment);
	  output_sframe (sframe_seg);
	}
      else
	as_bad (_(".sframe not supported for target"));
    }

  cfi_sections_set = true;
  if ((all_cfi_sections & CFI_EMIT_debug_frame) != 0)
    {
      int alignment = ffs (DWARF2_ADDR_SIZE (stdoutput)) - 1;

      if (!SUPPORT_FRAME_LINKONCE)
	get_cfi_seg (NULL, ".debug_frame",
		     SEC_READONLY | SEC_DEBUGGING,
		     alignment);

      do
	{
	  ccseg = NULL;
	  seek_next_seg = 0;

	  for (cie = cie_root; cie; cie = cie_next)
	    {
	      cie_next = cie->next;
	      free ((void *) cie);
	    }
	  cie_root = NULL;

	  for (fde = all_fde_data; fde ; fde = fde->next)
	    {
	      if ((fde->sections & CFI_EMIT_debug_frame) == 0)
		continue;

	      if (SUPPORT_FRAME_LINKONCE)
		{
		  if (HANDLED (fde))
		    continue;
		  if (seek_next_seg && CUR_SEG (fde) != ccseg)
		    {
		      seek_next_seg = 2;
		      continue;
		    }
		  if (!seek_next_seg)
		    {
		      ccseg = CUR_SEG (fde);
		      /* Open .debug_frame section.  */
		      get_cfi_seg (ccseg, ".debug_frame",
				   SEC_READONLY | SEC_DEBUGGING,
				   alignment);
		      seek_next_seg = 1;
		    }
		  SET_HANDLED (fde, 1);
		}
	      if (fde->end_address == NULL)
		{
		  as_bad (_("open CFI at the end of file; "
			    "missing .cfi_endproc directive"));
		  fde->end_address = fde->start_address;
		}

	      fde->per_encoding = DW_EH_PE_omit;
	      fde->lsda_encoding = DW_EH_PE_omit;
	      cfi_change_reg_numbers (fde->data, ccseg);
	      cie = select_cie_for_fde (fde, false, &first, alignment);
	      output_fde (fde, cie, false, first, alignment);
	    }
	}
      while (SUPPORT_FRAME_LINKONCE && seek_next_seg == 2);

      if (SUPPORT_FRAME_LINKONCE)
	for (fde = all_fde_data; fde ; fde = fde->next)
	  SET_HANDLED (fde, 0);
    }
  if (dwcfi_hash)
    htab_delete (dwcfi_hash);
}

#else /* TARGET_USE_CFIPOP */

/* Emit an intelligible error message for missing support.  */

static void
dot_cfi_dummy (int ignored ATTRIBUTE_UNUSED)
{
  as_bad (_("CFI is not supported for this target"));
  ignore_rest_of_line ();
}

const pseudo_typeS cfi_pseudo_table[] =
  {
    { "cfi_sections", dot_cfi_dummy, 0 },
    { "cfi_startproc", dot_cfi_dummy, 0 },
    { "cfi_endproc", dot_cfi_dummy, 0 },
    { "cfi_fde_data", dot_cfi_dummy, 0 },
    { "cfi_def_cfa", dot_cfi_dummy, 0 },
    { "cfi_def_cfa_register", dot_cfi_dummy, 0 },
    { "cfi_def_cfa_offset", dot_cfi_dummy, 0 },
    { "cfi_adjust_cfa_offset", dot_cfi_dummy, 0 },
    { "cfi_offset", dot_cfi_dummy, 0 },
    { "cfi_rel_offset", dot_cfi_dummy, 0 },
    { "cfi_register", dot_cfi_dummy, 0 },
    { "cfi_return_column", dot_cfi_dummy, 0 },
    { "cfi_restore", dot_cfi_dummy, 0 },
    { "cfi_undefined", dot_cfi_dummy, 0 },
    { "cfi_same_value", dot_cfi_dummy, 0 },
    { "cfi_remember_state", dot_cfi_dummy, 0 },
    { "cfi_restore_state", dot_cfi_dummy, 0 },
    { "cfi_window_save", dot_cfi_dummy, 0 },
    { "cfi_escape", dot_cfi_dummy, 0 },
    { "cfi_signal_frame", dot_cfi_dummy, 0 },
    { "cfi_personality", dot_cfi_dummy, 0 },
    { "cfi_personality_id", dot_cfi_dummy, 0 },
    { "cfi_lsda", dot_cfi_dummy, 0 },
    { "cfi_val_encoded_addr", dot_cfi_dummy, 0 },
    { "cfi_label", dot_cfi_dummy, 0 },
    { "cfi_inline_lsda", dot_cfi_dummy, 0 },
    { "cfi_val_offset", dot_cfi_dummy, 0 },
    { NULL, NULL, 0 }
  };

void
cfi_finish (void)
{
}
#endif /* TARGET_USE_CFIPOP */
