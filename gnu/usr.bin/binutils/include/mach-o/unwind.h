/* Mach-O compact unwind encoding.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _MACH_O_UNWIND_H
#define _MACH_O_UNWIND_H

/* Encodings bits for all cpus.  */
#define MACH_O_UNWIND_IS_NOT_FUNCTION_START 0x80000000
#define MACH_O_UNWIND_HAS_LSDA              0x40000000
#define MACH_O_UNWIND_PERSONALITY_MASK      0x30000000
#define MACH_O_UNWIND_PERSONALITY_SHIFT     28

/* Encodings for x86-64.  */

/* Kind of encoding (4 bits).  */
#define MACH_O_UNWIND_X86_64_MODE_MASK       0x0f000000

/* Frame is RBP based, using the standard sequence: push %rbp; mov %rsp, %rbp.
   Non-volatile registers must be saved in the stack starting at %rbp-8 to
   %rbp-2040 (offset is encoded in offset bits * 8).  Registers saved are
   encoded in registers bits, 3 bits per register.  */
#define MACH_O_UNWIND_X86_64_MODE_RBP_FRAME  0x01000000
#define  MACH_O_UNWIND_X86_64_RBP_FRAME_REGISTERS 0x00007FFF
#define  MACH_O_UNWIND_X86_64_RBP_FRAME_OFFSET    0x00FF0000

/* Frameless function, with a small stack size.  */
#define MACH_O_UNWIND_X86_64_MODE_STACK_IMMD 0x02000000
#define  MACH_O_UNWIND_X86_64_FRAMELESS_STACK_SIZE      0x00FF0000
#define  MACH_O_UNWIND_X86_64_FRAMELESS_REG_COUNT       0x00001C00
#define  MACH_O_UNWIND_X86_64_FRAMELESS_REG_PERMUTATION 0x000003FF

/* Frameless function, with a larger stack size. The stack size is the sum
   of the X in subq $X,%rsp (address of X is at function + stack size bits)
   and stack adjust.  */
#define MACH_O_UNWIND_X86_64_MODE_STACK_IND          0x03000000
#define  MACH_O_UNWIND_X86_64_FRAMELESS_STACK_ADJUST 0x0000E000

/* Use dwarf.  */
#define MACH_O_UNWIND_X86_64_MODE_DWARF      0x04000000
#define  MACH_O_UNWIND_X86_64_DWARF_SECTION_OFFSET 0x00ffffff

/* Registers.  */
#define MACH_O_UNWIND_X86_64_REG_NONE 0
#define MACH_O_UNWIND_X86_64_REG_RBX 1
#define MACH_O_UNWIND_X86_64_REG_R12 2
#define MACH_O_UNWIND_X86_64_REG_R13 3
#define MACH_O_UNWIND_X86_64_REG_R14 4
#define MACH_O_UNWIND_X86_64_REG_R15 5
#define MACH_O_UNWIND_X86_64_REG_RBP 6

/* Encodings for x86 (almot the same as x86-64).  */

/* Kind of encoding (4 bits).  */
#define MACH_O_UNWIND_X86_MODE_MASK       0x0f000000

/* Frame is EBP based, using the standard sequence: push %ebp; mov %esp, %ebp.
   Non-volatile registers must be saved in the stack starting at %ebp-4 to
   %ebp-240 (offset is encoded in offset bits * 4).  Registers saved are
   encoded in registers bits, 3 bits per register.  */
#define MACH_O_UNWIND_X86_MODE_EBP_FRAME  0x01000000
#define  MACH_O_UNWIND_X86_EBP_FRAME_REGISTERS 0x00007FFF
#define  MACH_O_UNWIND_X86_EBP_FRAME_OFFSET    0x00FF0000

/* Frameless function, with a small stack size.  */
#define MACH_O_UNWIND_X86_MODE_STACK_IMMD 0x02000000
#define  MACH_O_UNWIND_X86_FRAMELESS_STACK_SIZE      0x00FF0000
#define  MACH_O_UNWIND_X86_FRAMELESS_REG_COUNT       0x00001C00
#define  MACH_O_UNWIND_X86_FRAMELESS_REG_PERMUTATION 0x000003FF

/* Frameless function, with a larger stack size. The stack size is the sum
   of the X in subq $X,%esp (address of X is at function + stack size bits)
   and stack adjust.  */
#define MACH_O_UNWIND_X86_MODE_STACK_IND          0x03000000
#define  MACH_O_UNWIND_X86_FRAMELESS_STACK_ADJUST 0x0000E000

/* Use dwarf.  */
#define MACH_O_UNWIND_X86_MODE_DWARF      0x04000000
#define  MACH_O_UNWIND_X86_DWARF_SECTION_OFFSET 0x00ffffff

/* Registers.  */
#define MACH_O_UNWIND_X86_REG_NONE 0
#define MACH_O_UNWIND_X86_REG_EBX 1
#define MACH_O_UNWIND_X86_REG_ECX 2
#define MACH_O_UNWIND_X86_REG_EDX 3
#define MACH_O_UNWIND_X86_REG_EDI 4
#define MACH_O_UNWIND_X86_REG_ESI 5
#define MACH_O_UNWIND_X86_REG_EBP 6

/* Encodings for arm64.  */

#define MACH_O_UNWIND_ARM64_MODE_MASK		0x0f000000

/* Leaf function:  FP/LR are *not* saved, none or some non-volatile registers
   are saved, stack is allocated.  The size of the frame (register saved and
   memory) is encoded in STACK_SIZE in 16 byte units.  */
#define MACH_O_UNWIND_ARM64_MODE_FRAMELESS	0x02000000

#define MACH_O_UNWIND_ARM64_MODE_DWARF		0x03000000

/* Standard frame: FP/LR are pushed, SP is copied to FP, then non-volatile
   registers are saved.  */
#define MACH_O_UNWIND_ARM64_MODE_FRAME		0x04000000

/* Registers (for FRAME).  */
#define MACH_O_UNWIND_ARM64_FRAME_X19_X20_PAIR	0x00000001
#define MACH_O_UNWIND_ARM64_FRAME_X21_X22_PAIR	0x00000002
#define MACH_O_UNWIND_ARM64_FRAME_X23_X24_PAIR	0x00000004
#define MACH_O_UNWIND_ARM64_FRAME_X25_X26_PAIR	0x00000008
#define MACH_O_UNWIND_ARM64_FRAME_X27_X28_PAIR	0x00000010
#define MACH_O_UNWIND_ARM64_FRAME_D8_D9_PAIR	0x00000100
#define MACH_O_UNWIND_ARM64_FRAME_D10_D11_PAIR	0x00000200
#define MACH_O_UNWIND_ARM64_FRAME_D12_D13_PAIR	0x00000400
#define MACH_O_UNWIND_ARM64_FRAME_D14_D15_PAIR	0x00000800

#define MACH_O_UNWIND_ARM64_FRAMELESS_STACK_SIZE_MASK 0x00fff000
#define MACH_O_UNWIND_ARM64_DWARF_SECTION_OFFSET 0x00ffffff

/* Entry in object file (in __LD,__compact_unwind section).  */

struct mach_o_compact_unwind_32
{
  unsigned char start[4];
  unsigned char length[4];
  unsigned char encoding[4];
  unsigned char personality[4];
  unsigned char lsda[4];
};

struct mach_o_compact_unwind_64
{
  unsigned char start[8];
  unsigned char length[4];
  unsigned char encoding[4];
  unsigned char personality[8];
  unsigned char lsda[8];
};

/* Header in images (in __TEXT,__unwind_info).  */

#define MACH_O_UNWIND_SECTION_VERSION 1 /* Current verion in header.  */
struct mach_o_unwind_info_header
{
  unsigned char version[4];	/* Currently MACH_O_UNWIND_SECTION_VERSION. */
  unsigned char encodings_array_offset[4];
  unsigned char encodings_array_count[4];
  unsigned char personality_array_offset[4];
  unsigned char personality_array_count[4];
  unsigned char index_offset[4];
  unsigned char index_count[4];
  /* Followed by:
     - encodings array
       These are the encodings shared, for index < encoding_array_count
     - personality array
       count given by personality_array_count
     - index entries
       count given by index_count
     - lsda index entries
       last offset given by lsda offset of last index_entry.
  */
};

struct mach_o_unwind_index_entry
{
  unsigned char function_offset[4];
  unsigned char second_level_offset[4];
  unsigned char lsda_index_offset[4];
};

struct mach_o_unwind_lsda_index_entry
{
  unsigned char function_offset[4];
  unsigned char lsda_offset[4];
};

/* Second level index pages.  */

#define MACH_O_UNWIND_SECOND_LEVEL_REGULAR 2
struct mach_o_unwind_regular_second_level_page_header
{
  unsigned char kind[4];
  unsigned char entry_page_offset[2];
  unsigned char entry_count[2];
  /* Array of entries.  */
};

struct mach_o_unwind_regular_second_level_entry
{
  unsigned char function_offset[4];
  unsigned char encoding[4];
};

#define MACH_O_UNWIND_SECOND_LEVEL_COMPRESSED 3
struct mach_o_unwind_compressed_second_level_page_header
{
  unsigned char kind[4];
  unsigned char entry_page_offset[2];
  unsigned char entry_count[2];
  unsigned char encodings_offset[2];
  unsigned char encodings_count[2];
  /* Followed by entries array (one word, see below).  */
  /* Followed by (non-common) encodings array.  */
};

/* Compressed entries are one word, containing function offset and encoding
   index.  */
#define MACH_O_UNWIND_INFO_COMPRESSED_ENTRY_FUNC_OFFSET(en) \
   ((en) & 0x00FFFFFF)
#define MACH_O_UNWIND_INFO_COMPRESSED_ENTRY_ENCODING_INDEX(en) \
   (((en) >> 24) & 0xFF)

#endif /* _MACH_O_UNWIND_H */
