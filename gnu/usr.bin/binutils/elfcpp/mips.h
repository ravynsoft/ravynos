// mips.h -- ELF definitions specific to EM_MIPS  -*- C++ -*-

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
// Written by Sasa Stankovic <sasa.stankovic@imgtec.com>
//        and Aleksandar Simeonov <aleksandar.simeonov@rt-rk.com>.

// This file is part of elfcpp.

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// In addition to the permissions in the GNU Library General Public
// License, the Free Software Foundation gives you unlimited
// permission to link the compiled version of this file into
// combinations with other programs, and to distribute those
// combinations without any restriction coming from the use of this
// file.  (The Library Public License restrictions do apply in other
// respects; for example, they cover modification of the file, and
/// distribution when not linked into a combined executable.)

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef ELFCPP_MIPS_H
#define ELFCPP_MIPS_H

// Documentation for the MIPS relocs is taken from
//   http://math-atlas.sourceforge.net/devel/assembly/mipsabi32.pdf

namespace elfcpp
{

//
// MIPS Relocation Codes
//

enum
{
  R_MIPS_NONE = 0,
  R_MIPS_16 = 1,
  R_MIPS_32 = 2,                   // In Elf 64: alias R_MIPS_ADD
  R_MIPS_REL32 = 3,                // In Elf 64: alias R_MIPS_REL
  R_MIPS_26 = 4,
  R_MIPS_HI16 = 5,
  R_MIPS_LO16 = 6,
  R_MIPS_GPREL16 = 7,              // In Elf 64: alias R_MIPS_GPREL
  R_MIPS_LITERAL = 8,
  R_MIPS_GOT16 = 9,                // In Elf 64: alias R_MIPS_GOT
  R_MIPS_PC16 = 10,
  R_MIPS_CALL16 = 11,              // In Elf 64: alias R_MIPS_CALL
  R_MIPS_GPREL32 = 12,
  R_MIPS_UNUSED1 = 13,
  R_MIPS_UNUSED2 = 14,
  R_MIPS_UNUSED3 = 15,
  R_MIPS_SHIFT5 = 16,
  R_MIPS_SHIFT6 = 17,
  R_MIPS_64 = 18,
  R_MIPS_GOT_DISP = 19,
  R_MIPS_GOT_PAGE = 20,
  R_MIPS_GOT_OFST = 21,
  R_MIPS_GOT_HI16 = 22,
  R_MIPS_GOT_LO16 = 23,
  R_MIPS_SUB = 24,
  R_MIPS_INSERT_A = 25,
  R_MIPS_INSERT_B = 26,
  R_MIPS_DELETE = 27,
  R_MIPS_HIGHER = 28,
  R_MIPS_HIGHEST = 29,
  R_MIPS_CALL_HI16 = 30,
  R_MIPS_CALL_LO16 = 31,
  R_MIPS_SCN_DISP = 32,
  R_MIPS_REL16 = 33,
  R_MIPS_ADD_IMMEDIATE = 34,
  R_MIPS_PJUMP = 35,
  R_MIPS_RELGOT = 36,
  R_MIPS_JALR = 37,
  // TLS relocations.
  R_MIPS_TLS_DTPMOD32 = 38,
  R_MIPS_TLS_DTPREL32 = 39,
  R_MIPS_TLS_DTPMOD64 = 40,
  R_MIPS_TLS_DTPREL64 = 41,
  R_MIPS_TLS_GD = 42,
  R_MIPS_TLS_LDM = 43,
  R_MIPS_TLS_DTPREL_HI16 = 44,
  R_MIPS_TLS_DTPREL_LO16 = 45,
  R_MIPS_TLS_GOTTPREL = 46,
  R_MIPS_TLS_TPREL32 = 47,
  R_MIPS_TLS_TPREL64 = 48,
  R_MIPS_TLS_TPREL_HI16 = 49,
  R_MIPS_TLS_TPREL_LO16 = 50,
  R_MIPS_GLOB_DAT = 51,
  R_MIPS_PC21_S2 = 60,
  R_MIPS_PC26_S2 = 61,
  R_MIPS_PC18_S3 = 62,
  R_MIPS_PC19_S2 = 63,
  R_MIPS_PCHI16 = 64,
  R_MIPS_PCLO16 = 65,
  // These relocs are used for the mips16.
  R_MIPS16_26 = 100,
  R_MIPS16_GPREL = 101,
  R_MIPS16_GOT16 = 102,
  R_MIPS16_CALL16 = 103,
  R_MIPS16_HI16 = 104,
  R_MIPS16_LO16 = 105,
  R_MIPS16_TLS_GD = 106,
  R_MIPS16_TLS_LDM = 107,
  R_MIPS16_TLS_DTPREL_HI16 = 108,
  R_MIPS16_TLS_DTPREL_LO16 = 109,
  R_MIPS16_TLS_GOTTPREL = 110,
  R_MIPS16_TLS_TPREL_HI16 = 111,
  R_MIPS16_TLS_TPREL_LO16 = 112,

  R_MIPS_COPY = 126,
  R_MIPS_JUMP_SLOT = 127,

  // These relocations are specific to microMIPS.
  R_MICROMIPS_26_S1 = 133,
  R_MICROMIPS_HI16 = 134,
  R_MICROMIPS_LO16 = 135,
  R_MICROMIPS_GPREL16 = 136,       // In Elf 64: alias R_MICROMIPS_GPREL
  R_MICROMIPS_LITERAL = 137,
  R_MICROMIPS_GOT16 = 138,         // In Elf 64: alias R_MICROMIPS_GOT
  R_MICROMIPS_PC7_S1 = 139,
  R_MICROMIPS_PC10_S1 = 140,
  R_MICROMIPS_PC16_S1 = 141,
  R_MICROMIPS_CALL16 = 142,        // In Elf 64: alias R_MICROMIPS_CALL
  R_MICROMIPS_GOT_DISP = 145,
  R_MICROMIPS_GOT_PAGE = 146,
  R_MICROMIPS_GOT_OFST = 147,
  R_MICROMIPS_GOT_HI16 = 148,
  R_MICROMIPS_GOT_LO16 = 149,
  R_MICROMIPS_SUB = 150,
  R_MICROMIPS_HIGHER = 151,
  R_MICROMIPS_HIGHEST = 152,
  R_MICROMIPS_CALL_HI16 = 153,
  R_MICROMIPS_CALL_LO16 = 154,
  R_MICROMIPS_SCN_DISP = 155,
  R_MICROMIPS_JALR = 156,
  R_MICROMIPS_HI0_LO16 = 157,
  // TLS relocations.
  R_MICROMIPS_TLS_GD = 162,
  R_MICROMIPS_TLS_LDM = 163,
  R_MICROMIPS_TLS_DTPREL_HI16 = 164,
  R_MICROMIPS_TLS_DTPREL_LO16 = 165,
  R_MICROMIPS_TLS_GOTTPREL = 166,
  R_MICROMIPS_TLS_TPREL_HI16 = 169,
  R_MICROMIPS_TLS_TPREL_LO16 = 170,
  // microMIPS GP- and PC-relative relocations.
  R_MICROMIPS_GPREL7_S2 = 172,
  R_MICROMIPS_PC23_S2 = 173,

  // This was a GNU extension used by embedded-PIC.  It was co-opted by
  // mips-linux for exception-handling data.  GCC stopped using it in
  // May, 2004, then started using it again for compact unwind tables.
  R_MIPS_PC32 = 248,
  R_MIPS_EH = 249,
  // This relocation is used internally by gas.
  R_MIPS_GNU_REL16_S2 = 250,
  // These are GNU extensions to enable C++ vtable garbage collection.
  R_MIPS_GNU_VTINHERIT = 253,
  R_MIPS_GNU_VTENTRY = 254
};

// Processor specific flags for the ELF header e_flags field.
enum
{
  // At least one .noreorder directive appears in the source.
  EF_MIPS_NOREORDER = 0x00000001,
  // File contains position independent code.
  EF_MIPS_PIC = 0x00000002,
  // Code in file uses the standard calling sequence for calling
  // position independent code.
  EF_MIPS_CPIC = 0x00000004,
  // ???  Unknown flag, set in IRIX 6's BSDdup2.o in libbsd.a.
  EF_MIPS_XGOT = 0x00000008,
  // Code in file uses UCODE (obsolete)
  EF_MIPS_UCODE = 0x00000010,
  // Code in file uses new ABI (-n32 on Irix 6).
  EF_MIPS_ABI2 = 0x00000020,
  // Process the .MIPS.options section first by ld
  EF_MIPS_OPTIONS_FIRST = 0x00000080,
  // Architectural Extensions used by this file
  EF_MIPS_ARCH_ASE = 0x0f000000,
  // Use MDMX multimedia extensions
  EF_MIPS_ARCH_ASE_MDMX = 0x08000000,
  // Use MIPS-16 ISA extensions
  EF_MIPS_ARCH_ASE_M16 = 0x04000000,
  // Use MICROMIPS ISA extensions.
  EF_MIPS_ARCH_ASE_MICROMIPS = 0x02000000,
  // Indicates code compiled for a 64-bit machine in 32-bit mode.
  // (regs are 32-bits wide.)
  EF_MIPS_32BITMODE = 0x00000100,
  // 32-bit machine but FP registers are 64 bit (-mfp64).
  EF_MIPS_FP64 = 0x00000200,
  /// Code in file uses the IEEE 754-2008 NaN encoding convention.
  EF_MIPS_NAN2008 = 0x00000400,
  // MIPS dynamic
  EF_MIPS_DYNAMIC = 0x40
};

// Machine variant if we know it.  This field was invented at Cygnus,
// but it is hoped that other vendors will adopt it.  If some standard
// is developed, this code should be changed to follow it.
enum
{
  EF_MIPS_MACH = 0x00FF0000,

// Cygnus is choosing values between 80 and 9F;
// 00 - 7F should be left for a future standard;
// the rest are open.

  E_MIPS_MACH_3900 = 0x00810000,
  E_MIPS_MACH_4010 = 0x00820000,
  E_MIPS_MACH_4100 = 0x00830000,
  E_MIPS_MACH_4650 = 0x00850000,
  E_MIPS_MACH_4120 = 0x00870000,
  E_MIPS_MACH_4111 = 0x00880000,
  E_MIPS_MACH_SB1 = 0x008a0000,
  E_MIPS_MACH_OCTEON = 0x008b0000,
  E_MIPS_MACH_XLR = 0x008c0000,
  E_MIPS_MACH_OCTEON2 = 0x008d0000,
  E_MIPS_MACH_OCTEON3 = 0x008e0000,
  E_MIPS_MACH_5400 = 0x00910000,
  E_MIPS_MACH_5900 = 0x00920000,
  E_MIPS_MACH_5500 = 0x00980000,
  E_MIPS_MACH_9000 = 0x00990000,
  E_MIPS_MACH_LS2E = 0x00A00000,
  E_MIPS_MACH_LS2F = 0x00A10000,
  E_MIPS_MACH_GS464 = 0x00A20000,
  E_MIPS_MACH_GS464E = 0x00A30000,
  E_MIPS_MACH_GS264E = 0x00A40000,
};

// MIPS architecture
enum
{
  // Four bit MIPS architecture field.
  EF_MIPS_ARCH = 0xf0000000,
  // -mips1 code.
  E_MIPS_ARCH_1 = 0x00000000,
  // -mips2 code.
  E_MIPS_ARCH_2 = 0x10000000,
  // -mips3 code.
  E_MIPS_ARCH_3 = 0x20000000,
  // -mips4 code.
  E_MIPS_ARCH_4 = 0x30000000,
  // -mips5 code.
  E_MIPS_ARCH_5 = 0x40000000,
  // -mips32 code.
  E_MIPS_ARCH_32 = 0x50000000,
  // -mips64 code.
  E_MIPS_ARCH_64 = 0x60000000,
  // -mips32r2 code.
  E_MIPS_ARCH_32R2 = 0x70000000,
  // -mips64r2 code.
  E_MIPS_ARCH_64R2 = 0x80000000,
  // -mips32r6 code.
  E_MIPS_ARCH_32R6 = 0x90000000,
  // -mips64r6 code.
  E_MIPS_ARCH_64R6 = 0xa0000000,
};

// Values for the xxx_size bytes of an ABI flags structure.
enum
{
  // No registers.
  AFL_REG_NONE = 0x00,
  // 32-bit registers.
  AFL_REG_32 = 0x01,
  // 64-bit registers.
  AFL_REG_64 = 0x02,
  // 128-bit registers.
  AFL_REG_128 = 0x03
};

// Masks for the ases word of an ABI flags structure.
enum
{
  // DSP ASE.
  AFL_ASE_DSP = 0x00000001,
  // DSP R2 ASE.
  AFL_ASE_DSPR2 = 0x00000002,
  // Enhanced VA Scheme.
  AFL_ASE_EVA = 0x00000004,
  // MCU (MicroController) ASE.
  AFL_ASE_MCU = 0x00000008,
  // MDMX ASE.
  AFL_ASE_MDMX = 0x00000010,
  // MIPS-3D ASE.
  AFL_ASE_MIPS3D = 0x00000020,
  // MT ASE.
  AFL_ASE_MT  = 0x00000040,
  // SmartMIPS ASE.
  AFL_ASE_SMARTMIPS = 0x00000080,
  // VZ ASE.
  AFL_ASE_VIRT = 0x00000100,
  // MSA ASE.
  AFL_ASE_MSA = 0x00000200,
  // MIPS16 ASE.
  AFL_ASE_MIPS16 = 0x00000400,
  // MICROMIPS ASE.
  AFL_ASE_MICROMIPS = 0x00000800,
  // XPA ASE.
  AFL_ASE_XPA = 0x00001000,
  // Loongson EXT ASE.
  AFL_ASE_LOONGSON_EXT = 0x00002000
};

// Values for the isa_ext word of an ABI flags structure.
enum
{
  // RMI Xlr instruction.
  AFL_EXT_XLR = 1,
  // Cavium Networks Octeon2.
  AFL_EXT_OCTEON2 = 2,
  // Cavium Networks OcteonP.
  AFL_EXT_OCTEONP = 3,
  // Loongson 3A.
  AFL_EXT_LOONGSON_3A = 4,
  // Cavium Networks Octeon.
  AFL_EXT_OCTEON = 5,
  // MIPS R5900 instruction.
  AFL_EXT_5900 = 6,
  // MIPS R4650 instruction.
  AFL_EXT_4650 = 7,
  // LSI R4010 instruction.
  AFL_EXT_4010 = 8,
  // NEC VR4100 instruction.
  AFL_EXT_4100 = 9,
  // Toshiba R3900 instruction.
  AFL_EXT_3900 = 10,
  // MIPS R10000 instruction.
  AFL_EXT_10000 = 11,
  // Broadcom SB-1 instruction.
  AFL_EXT_SB1 = 12,
  // NEC VR4111/VR4181 instruction.
  AFL_EXT_4111 = 13,
  // NEC VR4120 instruction.
  AFL_EXT_4120 = 14,
  // NEC VR5400 instruction.
  AFL_EXT_5400 = 15,
  // NEC VR5500 instruction.
  AFL_EXT_5500 = 16,
  // ST Microelectronics Loongson 2E.
  AFL_EXT_LOONGSON_2E = 17,
  // ST Microelectronics Loongson 2F.
  AFL_EXT_LOONGSON_2F = 18,
  // Cavium Networks Octeon3.
  AFL_EXT_OCTEON3 = 19
};

// Masks for the flags1 word of an ABI flags structure.
enum
{
  // Uses odd single-precision registers.
  AFL_FLAGS1_ODDSPREG = 1
};

// Object attribute tags.
enum
{
  // 0-3 are generic.
  // Floating-point ABI used by this object file.
  Tag_GNU_MIPS_ABI_FP = 4,
  // MSA ABI used by this object file.
  Tag_GNU_MIPS_ABI_MSA = 8
};

// Object attribute values.
enum
{
  // Values defined for Tag_GNU_MIPS_ABI_FP.
  // Not tagged or not using any ABIs affected by the differences.
  Val_GNU_MIPS_ABI_FP_ANY = 0,
  // Using hard-float -mdouble-float.
  Val_GNU_MIPS_ABI_FP_DOUBLE = 1,
  // Using hard-float -msingle-float.
  Val_GNU_MIPS_ABI_FP_SINGLE = 2,
  // Using soft-float.
  Val_GNU_MIPS_ABI_FP_SOFT = 3,
  // Using -mips32r2 -mfp64.
  Val_GNU_MIPS_ABI_FP_OLD_64 = 4,
  // Using -mfpxx
  Val_GNU_MIPS_ABI_FP_XX = 5,
  // Using -mips32r2 -mfp64.
  Val_GNU_MIPS_ABI_FP_64 = 6,
  // Using -mips32r2 -mfp64 -mno-odd-spreg.
  Val_GNU_MIPS_ABI_FP_64A = 7,
  // This is reserved for backward-compatibility with an earlier
  // implementation of the MIPS NaN2008 functionality.
  Val_GNU_MIPS_ABI_FP_NAN2008 = 8,

  // Values defined for Tag_GNU_MIPS_ABI_MSA.
  // Not tagged or not using any ABIs affected by the differences.
  Val_GNU_MIPS_ABI_MSA_ANY = 0,
  // Using 128-bit MSA.
  Val_GNU_MIPS_ABI_MSA_128 = 1
};

enum
{
  // Mask to extract ABI version, not really a flag value.
  EF_MIPS_ABI = 0x0000F000,

  // The original o32 abi.
  E_MIPS_ABI_O32 = 0x00001000,
  // O32 extended to work on 64 bit architectures
  E_MIPS_ABI_O64 = 0x00002000,
  // EABI in 32 bit mode
  E_MIPS_ABI_EABI32 = 0x00003000,
  // EABI in 64 bit mode
  E_MIPS_ABI_EABI64 = 0x00004000,
};

// Dynamic section MIPS flags
enum
{
  // None
  RHF_NONE = 0x00000000,
  // Use shortcut pointers
  RHF_QUICKSTART = 0x00000001,
  // Hash size not power of two
  RHF_NOTPOT = 0x00000002,
  // Ignore LD_LIBRARY_PATH
  RHF_NO_LIBRARY_REPLACEMENT = 0x00000004
};

// Special values for the st_other field in the symbol table.
enum
{
  // Two topmost bits denote the MIPS ISA for .text symbols:
  // + 00 -- standard MIPS code,
  // + 10 -- microMIPS code,
  // + 11 -- MIPS16 code; requires the following two bits to be set too.
  // Note that one of the MIPS16 bits overlaps with STO_MIPS_PIC.
  STO_MIPS_ISA = 0xc0,

  // The mask spanning the rest of MIPS psABI flags.  At most one is expected
  // to be set except for STO_MIPS16.
  STO_MIPS_FLAGS = ~(STO_MIPS_ISA | 0x3),

  // The MIPS psABI was updated in 2008 with support for PLTs and copy
  // relocs.  There are therefore two types of nonzero SHN_UNDEF functions:
  // PLT entries and traditional MIPS lazy binding stubs.  We mark the former
  // with STO_MIPS_PLT to distinguish them from the latter.
  STO_MIPS_PLT = 0x8,

  // This value is used to mark PIC functions in an object that mixes
  // PIC and non-PIC.  Note that this bit overlaps with STO_MIPS16,
  // although MIPS16 symbols are never considered to be MIPS_PIC.
  STO_MIPS_PIC = 0x20,

  // This value is used for a mips16 .text symbol.
  STO_MIPS16 = 0xf0,

  // This value is used for a microMIPS .text symbol.  To distinguish from
  // STO_MIPS16, we set top two bits to be 10 to denote STO_MICROMIPS.  The
  // mask is STO_MIPS_ISA.
  STO_MICROMIPS  = 0x80
};

// Values for base offsets for thread-local storage
enum
{
  TP_OFFSET = 0x7000,
  DTP_OFFSET = 0x8000
};


bool
elf_st_is_mips16(unsigned char st_other)
{ return (st_other & elfcpp::STO_MIPS16) == elfcpp::STO_MIPS16; }

bool
elf_st_is_micromips(unsigned char st_other)
{ return (st_other & elfcpp::STO_MIPS_ISA) == elfcpp::STO_MICROMIPS; }

// Whether the ABI is N32.
bool
abi_n32(elfcpp::Elf_Word e_flags)
{ return (e_flags & elfcpp::EF_MIPS_ABI2) != 0; }

// Whether the ISA is R6.
bool
r6_isa(elfcpp::Elf_Word e_flags)
{
  return ((e_flags & elfcpp::EF_MIPS_ARCH) == elfcpp::E_MIPS_ARCH_32R6)
           || ((e_flags & elfcpp::EF_MIPS_ARCH) == elfcpp::E_MIPS_ARCH_64R6);
}

// Whether the file has microMIPS code.
bool
is_micromips(elfcpp::Elf_Word e_flags)
{ return (e_flags & elfcpp::EF_MIPS_ARCH_ASE_MICROMIPS) != 0; }

// Values which may appear in the kind field of an Elf_Options structure.
enum
{
  // Undefined.
  ODK_NULL = 0,
  // Register usage and GP value.
  ODK_REGINFO = 1,
  // Exception processing information.
  ODK_EXCEPTIONS = 2,
  // Section padding information.
  ODK_PAD = 3,
  // Hardware workarounds performed.
  ODK_HWPATCH = 4,
  // Fill value used by the linker.
  ODK_FILL = 5,
  // Reserved space for desktop tools.
  ODK_TAGS = 6,
  // Hardware workarounds, AND bits when merging.
  ODK_HWAND = 7,
  // Hardware workarounds, OR bits when merging.
  ODK_HWOR = 8,
  // GP group to use for text/data sections.
  ODK_GP_GROUP = 9,
  // ID information.
  ODK_IDENT = 10
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_MIPS_H)
