/* dwarf2dbg.h - DWARF2 debug support
   Copyright 1999, 2000, 2002, 2003 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef AS_DWARF2DBG_H
#define AS_DWARF2DBG_H

/* HACKS for bfd_* and BFD_RELOC_* These would come from bfd/reloc.c */
typedef int bfd_boolean;

#include "as.h"

#define DWARF2_FLAG_IS_STMT		(1 << 0)
#define DWARF2_FLAG_BASIC_BLOCK		(1 << 1)
#define DWARF2_FLAG_PROLOGUE_END	(1 << 2)
#define DWARF2_FLAG_EPILOGUE_BEGIN	(1 << 3)

struct dwarf2_line_info {
  unsigned int filenum;
  unsigned int line;
  unsigned int column;
  unsigned int isa;
  unsigned int flags;
};

/* When creating dwarf2 debugging information for assembly files, the variable
   dwarf2_file_number is used to generate a .file for each assembly source file
   in read_a_source_file() in read.c .  */
extern uint32_t dwarf2_file_number;

/* Info that is needed to be gathered for each symbol that will have a 
   dwarf2_subprogram when generating debug info directly for assembly files.
   This is done in make_subprogram_for_symbol() in symbols.c . */
struct dwarf2_subprogram_info {
    char *name; /* without a leading underbar, if any */
    uint32_t file_number; /* the dwarf file number this symbol is in */
    uint32_t line_number; /* the line number this symbol is at */
    /*
     * The low_pc for the dwarf2_subprogram is taken from the symbol's
     * n_value.  The high_pc is taken from the next symbol's value or
     * the end of the section for the last symbol.
     */
    symbolS *symbol;
    struct dwarf2_subprogram_info *next;
};
extern struct dwarf2_subprogram_info *dwarf2_subprograms_info;

/* Implements the .file FILENO "FILENAME" directive.  FILENO can be 0
   to indicate that no file number has been assigned.  All real file
   number must be >0.  */
extern char *dwarf2_directive_file (uintptr_t dummy);

/*
 * dwarf2_file() is what is called when generating
 * debug info directly for assembly files with --gdwarf2.
 */
extern void dwarf2_file (char *filename, offsetT num);

/* Implements the .loc FILENO LINENO [COLUMN] directive.  FILENO is
   the file number, LINENO the line number and the (optional) COLUMN
   the column of the source code that the following instruction
   corresponds to.  FILENO can be 0 to indicate that the filename
   specified by the textually most recent .file directive should be
   used.  */
extern void dwarf2_directive_loc (uintptr_t dummy);

/*
 * dwarf2_loc() is what is called when generating
 * debug info directly for assembly files with --gdwarf2.
 */
extern void dwarf2_loc (offsetT filenum, offsetT line);

/* Implements the .loc_mark_labels {0,1} directive.  */
extern void dwarf2_directive_loc_mark_labels (uintptr_t dummy);

/* Returns the current source information.  If .file directives have
   been encountered, the info for the corresponding source file is
   returned.  Otherwise, the info for the assembly source file is
   returned.  */
extern void dwarf2_where (struct dwarf2_line_info *l);

/* A hook to allow the target backend to inform the line number state 
   machine of isa changes when assembler debug info is enabled.  */
extern void dwarf2_set_isa (unsigned int isa);

/* This function generates .debug_line info based on the address and
   source information passed in the arguments.  ADDR should be the
   frag-relative offset of the instruction the information is for and
   L is the source information that should be associated with that
   address.  */
extern void dwarf2_gen_line_info (addressT addr, struct dwarf2_line_info *l);

/* Must be called for each generated instruction.  */
extern void dwarf2_emit_insn (int);

/* Should be called for each code label.  */
extern void dwarf2_emit_label (symbolS *);

/* True when we're supposed to set the basic block mark whenever a label
   is seen.  Unless the target is doing Something Weird, just call 
   dwarf2_emit_label.  */
extern bfd_boolean dwarf2_loc_mark_labels;

extern void dwarf2_finish (void);

extern int dwarf2dbg_estimate_size_before_relax (fragS *);
extern int dwarf2dbg_relax_frag (fragS *);
extern void dwarf2dbg_convert_frag (fragS *);

/* An enumeration which describes the sizes of offsets (to DWARF sections)
   and the mechanism by which the size is indicated.  */
enum dwarf2_format {
  /* 32-bit format: the initial length field is 4 bytes long.  */
  dwarf2_format_32bit,
  /* DWARF3 64-bit format: the representation of the initial length
     (of a DWARF section) is 0xffffffff (4 bytes) followed by eight
     bytes indicating the actual length.  */
  dwarf2_format_64bit,
  /* SGI extension to DWARF2: The initial length is eight bytes.  */
  dwarf2_format_64bit_irix
};

#endif /* AS_DWARF2DBG_H */
