/* dw2gencfi.h - Support for generating Dwarf2 CFI information.
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

#ifndef DW2GENCFI_H
#define DW2GENCFI_H

#include "dwarf2.h"

struct symbol;

extern const pseudo_typeS cfi_pseudo_table[];

/* cfi_finish() is called at the end of file. It will complain if
   the last CFI wasn't properly closed by .cfi_endproc.  */
extern void cfi_finish (void);

/* Entry points for backends to add unwind information.  */
extern void cfi_new_fde (struct symbol *);
extern void cfi_end_fde (struct symbol *);
extern void cfi_set_return_column (unsigned);
extern void cfi_set_sections (void);
extern void cfi_add_advance_loc (struct symbol *);
extern void cfi_add_label (const char *);

extern void cfi_add_CFA_offset (unsigned, offsetT);
extern void cfi_add_CFA_val_offset (unsigned, offsetT);
extern void cfi_add_CFA_def_cfa (unsigned, offsetT);
extern void cfi_add_CFA_register (unsigned, unsigned);
extern void cfi_add_CFA_def_cfa_register (unsigned);
extern void cfi_add_CFA_def_cfa_offset (offsetT);
extern void cfi_add_CFA_restore (unsigned);
extern void cfi_add_CFA_undefined (unsigned);
extern void cfi_add_CFA_same_value (unsigned);
extern void cfi_add_CFA_remember_state (void);
extern void cfi_add_CFA_restore_state (void);

/* Structures for md_cfi_end.  */

#if defined (TE_PE) || defined (TE_PEP)
#define SUPPORT_FRAME_LINKONCE 1
#else
#define SUPPORT_FRAME_LINKONCE 0
#endif

#ifdef tc_cfi_reloc_for_encoding
#define SUPPORT_COMPACT_EH 1
#else
#define SUPPORT_COMPACT_EH 0
#endif

#ifndef TARGET_MULTIPLE_EH_FRAME_SECTIONS
#define TARGET_MULTIPLE_EH_FRAME_SECTIONS 0
#endif

#define MULTIPLE_FRAME_SECTIONS (SUPPORT_FRAME_LINKONCE || SUPPORT_COMPACT_EH \
				 || TARGET_MULTIPLE_EH_FRAME_SECTIONS)

struct cfi_insn_data
{
  struct cfi_insn_data *next;
#if MULTIPLE_FRAME_SECTIONS
  segT cur_seg;
#endif
  int insn;
  union
  {
    struct
    {
      unsigned reg;
      offsetT offset;
    } ri;

    struct
    {
      unsigned reg1;
      unsigned reg2;
    } rr;

    unsigned r;
    offsetT i;

    struct
    {
      symbolS *lab1;
      symbolS *lab2;
    } ll;

    struct cfi_escape_data *esc;

    struct
    {
      unsigned reg, encoding;
      expressionS exp;
    } ea;

    const char *sym_name;
  } u;
};

/* An enumeration describing the Compact EH header format.  The least
   significant bit is used to distinguish the entries.

   Inline Compact:			Function offset [0]
					Four chars of unwind data.
   Out-of-line Compact:			Function offset [1]
					Compact unwind data offset [0]
   Legacy:				Function offset [1]
					Unwind data offset [1]

   The header type is initialized to EH_COMPACT_UNKNOWN until the
   format is discovered by encountering a .fde_data entry.
   Failure to find a .fde_data entry will cause an EH_COMPACT_LEGACY
   header to be generated.  */

enum {
  EH_COMPACT_UNKNOWN,
  EH_COMPACT_LEGACY,
  EH_COMPACT_INLINE,
  EH_COMPACT_OUTLINE,
  EH_COMPACT_OUTLINE_DONE,
  /* Outline if .cfi_inline_lsda used, otherwise legacy FDE.  */
  EH_COMPACT_HAS_LSDA
};

/* Stack of old CFI data, for save/restore.  */
struct cfa_save_data
{
  struct cfa_save_data *next;
  offsetT cfa_offset;
};

/* Current open FDE entry.  */
struct frch_cfi_data
{
  struct fde_entry *cur_fde_data;
  symbolS *last_address;
  offsetT cur_cfa_offset;
  struct cfa_save_data *cfa_save_stack;
};

struct fde_entry
{
  struct fde_entry *next;
#if MULTIPLE_FRAME_SECTIONS
  segT cur_seg;
#endif
  symbolS *start_address;
  symbolS *end_address;
  struct cfi_insn_data *data;
  struct cfi_insn_data **last;
  unsigned char per_encoding;
  unsigned char lsda_encoding;
  int personality_id;
  expressionS personality;
  expressionS lsda;
  unsigned int return_column;
  unsigned int signal_frame;
#if MULTIPLE_FRAME_SECTIONS
  int handled;
#endif
  int eh_header_type;
  /* Compact unwinding opcodes, not including the PR byte or LSDA.  */
  int eh_data_size;
  bfd_byte *eh_data;
  /* For out of line tables and FDEs.  */
  symbolS *eh_loc;
  int sections;
#ifdef tc_fde_entry_extras
  tc_fde_entry_extras
#endif
};

/* The list of all FDEs that have been collected.  */
extern struct fde_entry *all_fde_data;

/* Fake CFI type; outside the byte range of any real CFI insn.  */
#define CFI_adjust_cfa_offset	0x100
#define CFI_return_column	0x101
#define CFI_rel_offset		0x102
#define CFI_escape		0x103
#define CFI_signal_frame	0x104
#define CFI_val_encoded_addr	0x105
#define CFI_label		0x106

/* By default emit .eh_frame only, not .debug_frame.  */
#define CFI_EMIT_eh_frame               (1 << 0)
#define CFI_EMIT_debug_frame            (1 << 1)
#define CFI_EMIT_target                 (1 << 2)
#define CFI_EMIT_eh_frame_compact       (1 << 3)
#define CFI_EMIT_sframe                 (1 << 4)

#endif /* DW2GENCFI_H */
