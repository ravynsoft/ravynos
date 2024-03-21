// elfcpp.h -- main header file for elfcpp    -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

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
// distribution when not linked into a combined executable.)

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
// 02110-1301, USA.

// This is the external interface for elfcpp.

#ifndef ELFCPP_H
#define ELFCPP_H

#include "elfcpp_swap.h"

#include <stdint.h>

namespace elfcpp
{

// Basic ELF types.

// These types are always the same size.

typedef uint16_t Elf_Half;
typedef uint32_t Elf_Word;
typedef int32_t Elf_Sword;
typedef uint64_t Elf_Xword;
typedef int64_t Elf_Sxword;

// These types vary in size depending on the ELF file class.  The
// template parameter should be 32 or 64.

template<int size>
struct Elf_types;

template<>
struct Elf_types<32>
{
  typedef uint32_t Elf_Addr;
  typedef uint32_t Elf_Off;
  typedef uint32_t Elf_WXword;
  typedef int32_t Elf_Swxword;
};

template<>
struct Elf_types<64>
{
  typedef uint64_t Elf_Addr;
  typedef uint64_t Elf_Off;
  typedef uint64_t Elf_WXword;
  typedef int64_t Elf_Swxword;
};

// Offsets within the Ehdr e_ident field.

const int EI_MAG0 = 0;
const int EI_MAG1 = 1;
const int EI_MAG2 = 2;
const int EI_MAG3 = 3;
const int EI_CLASS = 4;
const int EI_DATA = 5;
const int EI_VERSION = 6;
const int EI_OSABI = 7;
const int EI_ABIVERSION = 8;
const int EI_PAD = 9;
const int EI_NIDENT = 16;

// The valid values found in Ehdr e_ident[EI_MAG0 through EI_MAG3].

const int ELFMAG0 = 0x7f;
const int ELFMAG1 = 'E';
const int ELFMAG2 = 'L';
const int ELFMAG3 = 'F';

// The valid values found in Ehdr e_ident[EI_CLASS].

enum
{
  ELFCLASSNONE = 0,
  ELFCLASS32 = 1,
  ELFCLASS64 = 2
};

// The valid values found in Ehdr e_ident[EI_DATA].

enum
{
  ELFDATANONE = 0,
  ELFDATA2LSB = 1,
  ELFDATA2MSB = 2
};

// The valid values found in Ehdr e_ident[EI_VERSION] and e_version.

enum
{
  EV_NONE = 0,
  EV_CURRENT = 1
};

// The valid values found in Ehdr e_ident[EI_OSABI].

enum ELFOSABI
{
  ELFOSABI_NONE = 0,
  ELFOSABI_HPUX = 1,
  ELFOSABI_NETBSD = 2,
  ELFOSABI_GNU = 3,
  // ELFOSABI_LINUX is an alias for ELFOSABI_GNU.
  ELFOSABI_LINUX = 3,
  ELFOSABI_SOLARIS = 6,
  ELFOSABI_AIX = 7,
  ELFOSABI_IRIX = 8,
  ELFOSABI_FREEBSD = 9,
  ELFOSABI_TRU64 = 10,
  ELFOSABI_MODESTO = 11,
  ELFOSABI_OPENBSD = 12,
  ELFOSABI_OPENVMS = 13,
  ELFOSABI_NSK = 14,
  ELFOSABI_AROS = 15,
  // A GNU extension for the ARM.
  ELFOSABI_ARM = 97,
  // A GNU extension for the MSP.
  ELFOSABI_STANDALONE = 255
};

// The valid values found in the Ehdr e_type field.

enum ET
{
  ET_NONE = 0,
  ET_REL = 1,
  ET_EXEC = 2,
  ET_DYN = 3,
  ET_CORE = 4,
  ET_LOOS = 0xfe00,
  ET_HIOS = 0xfeff,
  ET_LOPROC = 0xff00,
  ET_HIPROC = 0xffff
};

// The valid values found in the Ehdr e_machine field.

enum EM
{
  EM_NONE = 0,
  EM_M32 = 1,
  EM_SPARC = 2,
  EM_386 = 3,
  EM_68K = 4,
  EM_88K = 5,
  EM_IAMCU = 6,
  EM_860 = 7,
  EM_MIPS = 8,
  EM_S370 = 9,
  EM_MIPS_RS3_LE = 10,
  // 11 was the old Sparc V9 ABI.
  // 12 through 14 are reserved.
  EM_PARISC = 15,
  // 16 is reserved.
  // Some old PowerPC object files use 17.
  EM_VPP500 = 17,
  EM_SPARC32PLUS = 18,
  EM_960 = 19,
  EM_PPC = 20,
  EM_PPC64 = 21,
  EM_S390 = 22,
  // 23 through 35 are served.
  EM_V800 = 36,
  EM_FR20 = 37,
  EM_RH32 = 38,
  EM_RCE = 39,
  EM_ARM = 40,
  EM_ALPHA = 41,
  EM_SH = 42,
  EM_SPARCV9 = 43,
  EM_TRICORE = 44,
  EM_ARC = 45,
  EM_H8_300 = 46,
  EM_H8_300H = 47,
  EM_H8S = 48,
  EM_H8_500 = 49,
  EM_IA_64 = 50,
  EM_MIPS_X = 51,
  EM_COLDFIRE = 52,
  EM_68HC12 = 53,
  EM_MMA = 54,
  EM_PCP = 55,
  EM_NCPU = 56,
  EM_NDR1 = 57,
  EM_STARCORE = 58,
  EM_ME16 = 59,
  EM_ST100 = 60,
  EM_TINYJ = 61,
  EM_X86_64 = 62,
  EM_PDSP = 63,
  EM_PDP10 = 64,
  EM_PDP11 = 65,
  EM_FX66 = 66,
  EM_ST9PLUS = 67,
  EM_ST7 = 68,
  EM_68HC16 = 69,
  EM_68HC11 = 70,
  EM_68HC08 = 71,
  EM_68HC05 = 72,
  EM_SVX = 73,
  EM_ST19 = 74,
  EM_VAX = 75,
  EM_CRIS = 76,
  EM_JAVELIN = 77,
  EM_FIREPATH = 78,
  EM_ZSP = 79,
  EM_MMIX = 80,
  EM_HUANY = 81,
  EM_PRISM = 82,
  EM_AVR = 83,
  EM_FR30 = 84,
  EM_D10V = 85,
  EM_D30V = 86,
  EM_V850 = 87,
  EM_M32R = 88,
  EM_MN10300 = 89,
  EM_MN10200 = 90,
  EM_PJ = 91,
  EM_OR1K = 92,
  EM_ARC_A5 = 93,
  EM_XTENSA = 94,
  EM_VIDEOCORE = 95,
  EM_TMM_GPP = 96,
  EM_NS32K = 97,
  EM_TPC = 98,
  // Some old picoJava object files use 99 (EM_PJ is correct).
  EM_SNP1K = 99,
  EM_ST200 = 100,
  EM_IP2K = 101,
  EM_MAX = 102,
  EM_CR = 103,
  EM_F2MC16 = 104,
  EM_MSP430 = 105,
  EM_BLACKFIN = 106,
  EM_SE_C33 = 107,
  EM_SEP = 108,
  EM_ARCA = 109,
  EM_UNICORE = 110,
  EM_ALTERA_NIOS2 = 113,
  EM_CRX = 114,
  EM_TI_PRU = 144,
  EM_AARCH64 = 183,
  EM_TILEGX = 191,
  // The Morph MT.
  EM_MT = 0x2530,
  // DLX.
  EM_DLX = 0x5aa5,
  // FRV.
  EM_FRV = 0x5441,
  // Infineon Technologies 16-bit microcontroller with C166-V2 core.
  EM_X16X = 0x4688,
  // Xstorym16
  EM_XSTORMY16 = 0xad45,
  // Renesas M32C
  EM_M32C = 0xfeb0,
  // Vitesse IQ2000
  EM_IQ2000 = 0xfeba,
  // NIOS
  EM_NIOS32 = 0xfebb
  // Old AVR objects used 0x1057 (EM_AVR is correct).
  // Old MSP430 objects used 0x1059 (EM_MSP430 is correct).
  // Old FR30 objects used 0x3330 (EM_FR30 is correct).
  // Old OpenRISC objects used 0x3426 and 0x8472 (EM_OR1K is correct).
  // Old D10V objects used 0x7650 (EM_D10V is correct).
  // Old D30V objects used 0x7676 (EM_D30V is correct).
  // Old IP2X objects used 0x8217 (EM_IP2K is correct).
  // Old PowerPC objects used 0x9025 (EM_PPC is correct).
  // Old Alpha objects used 0x9026 (EM_ALPHA is correct).
  // Old M32R objects used 0x9041 (EM_M32R is correct).
  // Old V850 objects used 0x9080 (EM_V850 is correct).
  // Old S/390 objects used 0xa390 (EM_S390 is correct).
  // Old Xtensa objects used 0xabc7 (EM_XTENSA is correct).
  // Old MN10300 objects used 0xbeef (EM_MN10300 is correct).
  // Old MN10200 objects used 0xdead (EM_MN10200 is correct).
};

// A special value found in the Ehdr e_phnum field.

enum
{
  // Number of program segments stored in sh_info field of first
  // section headre.
  PN_XNUM = 0xffff
};

// Special section indices.

enum
{
  SHN_UNDEF = 0,
  SHN_LORESERVE = 0xff00,
  SHN_LOPROC = 0xff00,
  SHN_HIPROC = 0xff1f,
  SHN_LOOS = 0xff20,
  SHN_HIOS = 0xff3f,
  SHN_ABS = 0xfff1,
  SHN_COMMON = 0xfff2,
  SHN_XINDEX = 0xffff,
  SHN_HIRESERVE = 0xffff,

  // Provide for initial and final section ordering in conjunction
  // with the SHF_LINK_ORDER and SHF_ORDERED section flags.
  SHN_BEFORE = 0xff00,
  SHN_AFTER = 0xff01,

  // x86_64 specific large common symbol.
  SHN_X86_64_LCOMMON = 0xff02
};

// The valid values found in the Shdr sh_type field.

enum SHT
{
  SHT_NULL = 0,
  SHT_PROGBITS = 1,
  SHT_SYMTAB = 2,
  SHT_STRTAB = 3,
  SHT_RELA = 4,
  SHT_HASH = 5,
  SHT_DYNAMIC = 6,
  SHT_NOTE = 7,
  SHT_NOBITS = 8,
  SHT_REL = 9,
  SHT_SHLIB = 10,
  SHT_DYNSYM = 11,
  SHT_INIT_ARRAY = 14,
  SHT_FINI_ARRAY = 15,
  SHT_PREINIT_ARRAY = 16,
  SHT_GROUP = 17,
  SHT_SYMTAB_SHNDX = 18,
  SHT_LOOS = 0x60000000,
  SHT_HIOS = 0x6fffffff,
  SHT_LOPROC = 0x70000000,
  SHT_HIPROC = 0x7fffffff,
  SHT_LOUSER = 0x80000000,
  SHT_HIUSER = 0xffffffff,
  // The remaining values are not in the standard.
  // Incremental build data.
  SHT_GNU_INCREMENTAL_INPUTS = 0x6fff4700,
  SHT_GNU_INCREMENTAL_SYMTAB = 0x6fff4701,
  SHT_GNU_INCREMENTAL_RELOCS = 0x6fff4702,
  SHT_GNU_INCREMENTAL_GOT_PLT = 0x6fff4703,
  // Object attributes.
  SHT_GNU_ATTRIBUTES = 0x6ffffff5,
  // GNU style dynamic hash table.
  SHT_GNU_HASH = 0x6ffffff6,
  // List of prelink dependencies.
  SHT_GNU_LIBLIST = 0x6ffffff7,
  // Versions defined by file.
  SHT_SUNW_verdef = 0x6ffffffd,
  SHT_GNU_verdef = 0x6ffffffd,
  // Versions needed by file.
  SHT_SUNW_verneed = 0x6ffffffe,
  SHT_GNU_verneed = 0x6ffffffe,
  // Symbol versions,
  SHT_SUNW_versym = 0x6fffffff,
  SHT_GNU_versym = 0x6fffffff,

  SHT_SPARC_GOTDATA = 0x70000000,

  // ARM-specific section types.
  // Exception Index table.
  SHT_ARM_EXIDX = 0x70000001,
  // BPABI DLL dynamic linking pre-emption map.
  SHT_ARM_PREEMPTMAP = 0x70000002,
  // Object file compatibility attributes.
  SHT_ARM_ATTRIBUTES = 0x70000003,
  // Support for debugging overlaid programs.
  SHT_ARM_DEBUGOVERLAY = 0x70000004,
  SHT_ARM_OVERLAYSECTION = 0x70000005,

  // x86_64 unwind information.
  SHT_X86_64_UNWIND = 0x70000001,

  // MIPS-specific section types.
  // Section contains register usage information.
  SHT_MIPS_REGINFO = 0x70000006,
  // Section contains miscellaneous options.
  SHT_MIPS_OPTIONS = 0x7000000d,
  // ABI related flags section.
  SHT_MIPS_ABIFLAGS = 0x7000002a,

  // AARCH64-specific section type.
  SHT_AARCH64_ATTRIBUTES = 0x70000003,

  // CSKY-specific section types.
  // Object file compatibility attributes.
  SHT_CSKY_ATTRIBUTES = 0x70000001,

  // Link editor is to sort the entries in this section based on the
  // address specified in the associated symbol table entry.
  SHT_ORDERED = 0x7fffffff
};

// The valid bit flags found in the Shdr sh_flags field.

enum SHF
{
  SHF_WRITE = 0x1,
  SHF_ALLOC = 0x2,
  SHF_EXECINSTR = 0x4,
  SHF_MERGE = 0x10,
  SHF_STRINGS = 0x20,
  SHF_INFO_LINK = 0x40,
  SHF_LINK_ORDER = 0x80,
  SHF_OS_NONCONFORMING = 0x100,
  SHF_GROUP = 0x200,
  SHF_TLS = 0x400,
  SHF_COMPRESSED = 0x800,
  SHF_MASKOS = 0x0ff00000,
  SHF_GNU_RETAIN = 0x200000,
  SHF_MASKPROC = 0xf0000000,

  // Indicates this section requires ordering in relation to
  // other sections of the same type.  Ordered sections are
  // combined within the section pointed to by the sh_link entry.
  // The sh_info values SHN_BEFORE and SHN_AFTER imply that the
  // sorted section is to precede or follow, respectively, all
  // other sections in the set being ordered.
  SHF_ORDERED = 0x40000000,
  // This section is excluded from input to the link-edit of an
  // executable or shared object.  This flag is ignored if SHF_ALLOC
  // is also set, or if relocations exist against the section.
  SHF_EXCLUDE = 0x80000000,

  // Section with data that is GP relative addressable.
  SHF_MIPS_GPREL = 0x10000000,

  // x86_64 specific large section.
  SHF_X86_64_LARGE = 0x10000000
};

// Values which appear in the first Elf_WXword of the section data
// of a SHF_COMPRESSED section.
enum
{
  ELFCOMPRESS_ZLIB = 1,
  ELFCOMPRESS_ZSTD = 2,
  ELFCOMPRESS_LOOS = 0x60000000,
  ELFCOMPRESS_HIOS = 0x6fffffff,
  ELFCOMPRESS_LOPROC = 0x70000000,
  ELFCOMPRESS_HIPROC = 0x7fffffff,
};

// Bit flags which appear in the first 32-bit word of the section data
// of a SHT_GROUP section.

enum
{
  GRP_COMDAT = 0x1,
  GRP_MASKOS = 0x0ff00000,
  GRP_MASKPROC = 0xf0000000
};

// The valid values found in the Phdr p_type field.

enum PT
{
  PT_NULL = 0,
  PT_LOAD = 1,
  PT_DYNAMIC = 2,
  PT_INTERP = 3,
  PT_NOTE = 4,
  PT_SHLIB = 5,
  PT_PHDR = 6,
  PT_TLS = 7,
  PT_LOOS = 0x60000000,
  PT_HIOS = 0x6fffffff,
  PT_LOPROC = 0x70000000,
  PT_HIPROC = 0x7fffffff,
  // The remaining values are not in the standard.
  // Frame unwind information.
  PT_GNU_EH_FRAME = 0x6474e550,
  PT_SUNW_EH_FRAME = 0x6474e550,
  // Stack flags.
  PT_GNU_STACK = 0x6474e551,
  // Read only after relocation.
  PT_GNU_RELRO = 0x6474e552,
  // Platform architecture compatibility information
  PT_ARM_ARCHEXT = 0x70000000,
  // Exception unwind tables
  PT_ARM_EXIDX = 0x70000001,
  // Register usage information.  Identifies one .reginfo section.
  PT_MIPS_REGINFO =0x70000000,
  // Runtime procedure table.
  PT_MIPS_RTPROC = 0x70000001,
  // .MIPS.options section.
  PT_MIPS_OPTIONS = 0x70000002,
  // .MIPS.abiflags section.
  PT_MIPS_ABIFLAGS = 0x70000003,
  // Platform architecture compatibility information
  PT_AARCH64_ARCHEXT = 0x70000000,
  // Exception unwind tables
  PT_AARCH64_UNWIND = 0x70000001,
  // 4k page table size
  PT_S390_PGSTE = 0x70000000,
};

// The valid bit flags found in the Phdr p_flags field.

enum PF
{
  PF_X = 0x1,
  PF_W = 0x2,
  PF_R = 0x4,
  PF_MASKOS = 0x0ff00000,
  PF_MASKPROC = 0xf0000000
};

// Symbol binding from Sym st_info field.

enum STB
{
  STB_LOCAL = 0,
  STB_GLOBAL = 1,
  STB_WEAK = 2,
  STB_LOOS = 10,
  STB_GNU_UNIQUE = 10,
  STB_HIOS = 12,
  STB_LOPROC = 13,
  STB_HIPROC = 15
};

// Symbol types from Sym st_info field.

enum STT
{
  STT_NOTYPE = 0,
  STT_OBJECT = 1,
  STT_FUNC = 2,
  STT_SECTION = 3,
  STT_FILE = 4,
  STT_COMMON = 5,
  STT_TLS = 6,

  // GNU extension: symbol value points to a function which is called
  // at runtime to determine the final value of the symbol.
  STT_GNU_IFUNC = 10,

  STT_LOOS = 10,
  STT_HIOS = 12,
  STT_LOPROC = 13,
  STT_HIPROC = 15,

  // The section type that must be used for register symbols on
  // Sparc.  These symbols initialize a global register.
  STT_SPARC_REGISTER = 13,

  // ARM: a THUMB function.  This is not defined in ARM ELF Specification but
  // used by the GNU tool-chain.
  STT_ARM_TFUNC = 13
};

inline STB
elf_st_bind(unsigned char info)
{
  return static_cast<STB>(info >> 4);
}

inline STT
elf_st_type(unsigned char info)
{
  return static_cast<STT>(info & 0xf);
}

inline unsigned char
elf_st_info(STB bind, STT type)
{
  return ((static_cast<unsigned char>(bind) << 4)
	  + (static_cast<unsigned char>(type) & 0xf));
}

// Symbol visibility from Sym st_other field.

enum STV
{
  STV_DEFAULT = 0,
  STV_INTERNAL = 1,
  STV_HIDDEN = 2,
  STV_PROTECTED = 3
};

inline STV
elf_st_visibility(unsigned char other)
{
  return static_cast<STV>(other & 0x3);
}

inline unsigned char
elf_st_nonvis(unsigned char other)
{
  return static_cast<STV>(other >> 2);
}

inline unsigned char
elf_st_other(STV vis, unsigned char nonvis)
{
  return ((nonvis << 2)
	  + (static_cast<unsigned char>(vis) & 3));
}

// Reloc information from Rel/Rela r_info field.

template<int size>
unsigned int
elf_r_sym(typename Elf_types<size>::Elf_WXword);

template<>
inline unsigned int
elf_r_sym<32>(Elf_Word v)
{
  return v >> 8;
}

template<>
inline unsigned int
elf_r_sym<64>(Elf_Xword v)
{
  return v >> 32;
}

template<int size>
unsigned int
elf_r_type(typename Elf_types<size>::Elf_WXword);

template<>
inline unsigned int
elf_r_type<32>(Elf_Word v)
{
  return v & 0xff;
}

template<>
inline unsigned int
elf_r_type<64>(Elf_Xword v)
{
  return v & 0xffffffff;
}

template<int size>
typename Elf_types<size>::Elf_WXword
elf_r_info(unsigned int s, unsigned int t);

template<>
inline Elf_Word
elf_r_info<32>(unsigned int s, unsigned int t)
{
  return (s << 8) + (t & 0xff);
}

template<>
inline Elf_Xword
elf_r_info<64>(unsigned int s, unsigned int t)
{
  return (static_cast<Elf_Xword>(s) << 32) + (t & 0xffffffff);
}

// Dynamic tags found in the PT_DYNAMIC segment.

enum DT
{
  DT_NULL = 0,
  DT_NEEDED = 1,
  DT_PLTRELSZ = 2,
  DT_PLTGOT = 3,
  DT_HASH = 4,
  DT_STRTAB = 5,
  DT_SYMTAB = 6,
  DT_RELA = 7,
  DT_RELASZ = 8,
  DT_RELAENT = 9,
  DT_STRSZ = 10,
  DT_SYMENT = 11,
  DT_INIT = 12,
  DT_FINI = 13,
  DT_SONAME = 14,
  DT_RPATH = 15,
  DT_SYMBOLIC = 16,
  DT_REL = 17,
  DT_RELSZ = 18,
  DT_RELENT = 19,
  DT_PLTREL = 20,
  DT_DEBUG = 21,
  DT_TEXTREL = 22,
  DT_JMPREL = 23,
  DT_BIND_NOW = 24,
  DT_INIT_ARRAY = 25,
  DT_FINI_ARRAY = 26,
  DT_INIT_ARRAYSZ = 27,
  DT_FINI_ARRAYSZ = 28,
  DT_RUNPATH = 29,
  DT_FLAGS = 30,

  // This is used to mark a range of dynamic tags.  It is not really
  // a tag value.
  DT_ENCODING = 32,

  DT_PREINIT_ARRAY = 32,
  DT_PREINIT_ARRAYSZ = 33,
  DT_LOOS = 0x6000000d,
  DT_HIOS = 0x6ffff000,
  DT_LOPROC = 0x70000000,
  DT_HIPROC = 0x7fffffff,

  // The remaining values are extensions used by GNU or Solaris.
  DT_VALRNGLO = 0x6ffffd00,
  DT_GNU_FLAGS_1 = 0x6ffffdf4,
  DT_GNU_PRELINKED = 0x6ffffdf5,
  DT_GNU_CONFLICTSZ = 0x6ffffdf6,
  DT_GNU_LIBLISTSZ = 0x6ffffdf7,
  DT_CHECKSUM = 0x6ffffdf8,
  DT_PLTPADSZ = 0x6ffffdf9,
  DT_MOVEENT = 0x6ffffdfa,
  DT_MOVESZ = 0x6ffffdfb,
  DT_FEATURE = 0x6ffffdfc,
  DT_POSFLAG_1 = 0x6ffffdfd,
  DT_SYMINSZ = 0x6ffffdfe,
  DT_SYMINENT = 0x6ffffdff,
  DT_VALRNGHI = 0x6ffffdff,

  DT_ADDRRNGLO = 0x6ffffe00,
  DT_GNU_HASH = 0x6ffffef5,
  DT_TLSDESC_PLT = 0x6ffffef6,
  DT_TLSDESC_GOT = 0x6ffffef7,
  DT_GNU_CONFLICT = 0x6ffffef8,
  DT_GNU_LIBLIST = 0x6ffffef9,
  DT_CONFIG = 0x6ffffefa,
  DT_DEPAUDIT = 0x6ffffefb,
  DT_AUDIT = 0x6ffffefc,
  DT_PLTPAD = 0x6ffffefd,
  DT_MOVETAB = 0x6ffffefe,
  DT_SYMINFO = 0x6ffffeff,
  DT_ADDRRNGHI = 0x6ffffeff,

  DT_RELACOUNT = 0x6ffffff9,
  DT_RELCOUNT = 0x6ffffffa,
  DT_FLAGS_1 = 0x6ffffffb,
  DT_VERDEF = 0x6ffffffc,
  DT_VERDEFNUM = 0x6ffffffd,
  DT_VERNEED = 0x6ffffffe,
  DT_VERNEEDNUM = 0x6fffffff,

  DT_VERSYM = 0x6ffffff0,

  // Specify the value of _GLOBAL_OFFSET_TABLE_.
  DT_PPC_GOT = 0x70000000,

  // Specify whether various optimisations are possible.
  DT_PPC_OPT = 0x70000001,

  // Specify the start of the .glink section.
  DT_PPC64_GLINK = 0x70000000,

  // Specify the start and size of the .opd section.
  DT_PPC64_OPD = 0x70000001,
  DT_PPC64_OPDSZ = 0x70000002,

  // Specify whether various optimisations are possible.
  DT_PPC64_OPT = 0x70000003,

  // The index of an STT_SPARC_REGISTER symbol within the DT_SYMTAB
  // symbol table.  One dynamic entry exists for every STT_SPARC_REGISTER
  // symbol in the symbol table.
  DT_SPARC_REGISTER = 0x70000001,

  // MIPS specific dynamic array tags.
  // 32 bit version number for runtime linker interface.
  DT_MIPS_RLD_VERSION = 0x70000001,
  // Time stamp.
  DT_MIPS_TIME_STAMP = 0x70000002,
  // Checksum of external strings and common sizes.
  DT_MIPS_ICHECKSUM = 0x70000003,
  // Index of version string in string table.
  DT_MIPS_IVERSION = 0x70000004,
  // 32 bits of flags.
  DT_MIPS_FLAGS = 0x70000005,
  // Base address of the segment.
  DT_MIPS_BASE_ADDRESS = 0x70000006,
  // ???
  DT_MIPS_MSYM = 0x70000007,
  // Address of .conflict section.
  DT_MIPS_CONFLICT = 0x70000008,
  // Address of .liblist section.
  DT_MIPS_LIBLIST = 0x70000009,
  // Number of local global offset table entries.
  DT_MIPS_LOCAL_GOTNO = 0x7000000a,
  // Number of entries in the .conflict section.
  DT_MIPS_CONFLICTNO = 0x7000000b,
  // Number of entries in the .liblist section.
  DT_MIPS_LIBLISTNO = 0x70000010,
  // Number of entries in the .dynsym section.
  DT_MIPS_SYMTABNO = 0x70000011,
  // Index of first external dynamic symbol not referenced locally.
  DT_MIPS_UNREFEXTNO = 0x70000012,
  // Index of first dynamic symbol in global offset table.
  DT_MIPS_GOTSYM = 0x70000013,
  // Number of page table entries in global offset table.
  DT_MIPS_HIPAGENO = 0x70000014,
  // Address of run time loader map, used for debugging.
  DT_MIPS_RLD_MAP = 0x70000016,
  // Delta C++ class definition.
  DT_MIPS_DELTA_CLASS = 0x70000017,
  // Number of entries in DT_MIPS_DELTA_CLASS.
  DT_MIPS_DELTA_CLASS_NO = 0x70000018,
  // Delta C++ class instances.
  DT_MIPS_DELTA_INSTANCE = 0x70000019,
  // Number of entries in DT_MIPS_DELTA_INSTANCE.
  DT_MIPS_DELTA_INSTANCE_NO = 0x7000001a,
  // Delta relocations.
  DT_MIPS_DELTA_RELOC = 0x7000001b,
  // Number of entries in DT_MIPS_DELTA_RELOC.
  DT_MIPS_DELTA_RELOC_NO = 0x7000001c,
  // Delta symbols that Delta relocations refer to.
  DT_MIPS_DELTA_SYM = 0x7000001d,
  // Number of entries in DT_MIPS_DELTA_SYM.
  DT_MIPS_DELTA_SYM_NO = 0x7000001e,
  // Delta symbols that hold class declarations.
  DT_MIPS_DELTA_CLASSSYM = 0x70000020,
  // Number of entries in DT_MIPS_DELTA_CLASSSYM.
  DT_MIPS_DELTA_CLASSSYM_NO = 0x70000021,
  // Flags indicating information about C++ flavor.
  DT_MIPS_CXX_FLAGS = 0x70000022,
  // Pixie information (???).
  DT_MIPS_PIXIE_INIT = 0x70000023,
  // Address of .MIPS.symlib
  DT_MIPS_SYMBOL_LIB = 0x70000024,
  // The GOT index of the first PTE for a segment
  DT_MIPS_LOCALPAGE_GOTIDX = 0x70000025,
  // The GOT index of the first PTE for a local symbol
  DT_MIPS_LOCAL_GOTIDX = 0x70000026,
  // The GOT index of the first PTE for a hidden symbol
  DT_MIPS_HIDDEN_GOTIDX = 0x70000027,
  // The GOT index of the first PTE for a protected symbol
  DT_MIPS_PROTECTED_GOTIDX = 0x70000028,
  // Address of `.MIPS.options'.
  DT_MIPS_OPTIONS = 0x70000029,
  // Address of `.interface'.
  DT_MIPS_INTERFACE = 0x7000002a,
  // ???
  DT_MIPS_DYNSTR_ALIGN = 0x7000002b,
  // Size of the .interface section.
  DT_MIPS_INTERFACE_SIZE = 0x7000002c,
  // Size of rld_text_resolve function stored in the GOT.
  DT_MIPS_RLD_TEXT_RESOLVE_ADDR = 0x7000002d,
  // Default suffix of DSO to be added by rld on dlopen() calls.
  DT_MIPS_PERF_SUFFIX = 0x7000002e,
  // Size of compact relocation section (O32).
  DT_MIPS_COMPACT_SIZE = 0x7000002f,
  // GP value for auxiliary GOTs.
  DT_MIPS_GP_VALUE = 0x70000030,
  // Address of auxiliary .dynamic.
  DT_MIPS_AUX_DYNAMIC = 0x70000031,
  // Address of the base of the PLTGOT.
  DT_MIPS_PLTGOT = 0x70000032,
  // Points to the base of a writable PLT.
  DT_MIPS_RWPLT = 0x70000034,
  // Relative offset of run time loader map, used for debugging.
  DT_MIPS_RLD_MAP_REL = 0x70000035,

  DT_AUXILIARY = 0x7ffffffd,
  DT_USED = 0x7ffffffe,
  DT_FILTER = 0x7fffffff
};

// Flags found in the DT_FLAGS dynamic element.

enum DF
{
  DF_ORIGIN = 0x1,
  DF_SYMBOLIC = 0x2,
  DF_TEXTREL = 0x4,
  DF_BIND_NOW = 0x8,
  DF_STATIC_TLS = 0x10
};

// Flags found in the DT_FLAGS_1 dynamic element.

enum DF_1
{
  DF_1_NOW = 0x1,
  DF_1_GLOBAL = 0x2,
  DF_1_GROUP = 0x4,
  DF_1_NODELETE = 0x8,
  DF_1_LOADFLTR = 0x10,
  DF_1_INITFIRST = 0x20,
  DF_1_NOOPEN = 0x40,
  DF_1_ORIGIN = 0x80,
  DF_1_DIRECT = 0x100,
  DF_1_TRANS = 0x200,
  DF_1_INTERPOSE = 0x400,
  DF_1_NODEFLIB = 0x800,
  DF_1_NODUMP = 0x1000,
  DF_1_CONLFAT = 0x2000,
  DF_1_PIE = 0x08000000
};

// Flags found in the DT_GNU_FLAGS_1 dynamic element.
enum DF_GNU_1
{
  DF_GNU_1_UNIQUE = 0x1,
};

// Version numbers which appear in the vd_version field of a Verdef
// structure.

const int VER_DEF_NONE = 0;
const int VER_DEF_CURRENT = 1;

// Version numbers which appear in the vn_version field of a Verneed
// structure.

const int VER_NEED_NONE = 0;
const int VER_NEED_CURRENT = 1;

// Bit flags which appear in vd_flags of Verdef and vna_flags of
// Vernaux.

const int VER_FLG_BASE = 0x1;
const int VER_FLG_WEAK = 0x2;
const int VER_FLG_INFO = 0x4;

// Special constants found in the SHT_GNU_versym entries.

const int VER_NDX_LOCAL = 0;
const int VER_NDX_GLOBAL = 1;

// A SHT_GNU_versym section holds 16-bit words.  This bit is set if
// the symbol is hidden and can only be seen when referenced using an
// explicit version number.  This is a GNU extension.

const int VERSYM_HIDDEN = 0x8000;

// This is the mask for the rest of the data in a word read from a
// SHT_GNU_versym section.

const int VERSYM_VERSION = 0x7fff;

// Note descriptor type codes for notes in a non-core file with an
// empty name.

enum
{
  // A version string.
  NT_VERSION = 1,
  // An architecture string.
  NT_ARCH = 2
};

// Note descriptor type codes for notes in a non-core file with the
// name "GNU".

enum
{
  // The minimum ABI level.  This is used by the dynamic linker to
  // describe the minimal kernel version on which a shared library may
  // be used.  Th value should be four words.  Word 0 is an OS
  // descriptor (see below).  Word 1 is the major version of the ABI.
  // Word 2 is the minor version.  Word 3 is the subminor version.
  NT_GNU_ABI_TAG = 1,
  // Hardware capabilities information.  Word 0 is the number of
  // entries.  Word 1 is a bitmask of enabled entries.  The rest of
  // the descriptor is a series of entries, where each entry is a
  // single byte followed by a nul terminated string.  The byte gives
  // the bit number to test if enabled in the bitmask.
  NT_GNU_HWCAP = 2,
  // The build ID as set by the linker's --build-id option.  The
  // format of the descriptor depends on the build ID style.
  NT_GNU_BUILD_ID = 3,
  // The version of gold used to link.  Th descriptor is just a
  // string.
  NT_GNU_GOLD_VERSION = 4,
  // Program property note, as described in "Linux Extensions to the gABI".
  NT_GNU_PROPERTY_TYPE_0 = 5,
  // FDO .note.package notes as defined on https://systemd.io/ELF_PACKAGE_METADATA/
  FDO_PACKAGING_METADATA = 0xcafe1a7e
};

// The OS values which may appear in word 0 of a NT_GNU_ABI_TAG note.

enum
{
  ELF_NOTE_OS_LINUX = 0,
  ELF_NOTE_OS_GNU = 1,
  ELF_NOTE_OS_SOLARIS2 = 2,
  ELF_NOTE_OS_FREEBSD = 3,
  ELF_NOTE_OS_NETBSD = 4,
  ELF_NOTE_OS_SYLLABLE = 5
};

// Program property types for NT_GNU_PROPERTY_TYPE_0.

enum
{
  GNU_PROPERTY_STACK_SIZE = 1,
  GNU_PROPERTY_NO_COPY_ON_PROTECTED = 2,
  GNU_PROPERTY_LOPROC = 0xc0000000,
  GNU_PROPERTY_X86_COMPAT_ISA_1_USED = 0xc0000000,
  GNU_PROPERTY_X86_COMPAT_ISA_1_NEEDED = 0xc0000001,
  GNU_PROPERTY_X86_UINT32_AND_LO = 0xc0000002,
  GNU_PROPERTY_X86_UINT32_AND_HI = 0xc0007fff,
  GNU_PROPERTY_X86_UINT32_OR_LO = 0xc0008000,
  GNU_PROPERTY_X86_UINT32_OR_HI = 0xc000ffff,
  GNU_PROPERTY_X86_UINT32_OR_AND_LO = 0xc0010000,
  GNU_PROPERTY_X86_UINT32_OR_AND_HI = 0xc0017fff,
  GNU_PROPERTY_X86_COMPAT_2_ISA_1_NEEDED = GNU_PROPERTY_X86_UINT32_OR_LO + 0,
  GNU_PROPERTY_X86_COMPAT_2_ISA_1_USED = GNU_PROPERTY_X86_UINT32_OR_AND_LO + 0,
  GNU_PROPERTY_X86_FEATURE_1_AND = GNU_PROPERTY_X86_UINT32_AND_LO + 0,
  GNU_PROPERTY_X86_ISA_1_NEEDED = GNU_PROPERTY_X86_UINT32_OR_LO + 2,
  GNU_PROPERTY_X86_FEATURE_2_NEEDED = GNU_PROPERTY_X86_UINT32_OR_LO + 1,
  GNU_PROPERTY_X86_ISA_1_USED = GNU_PROPERTY_X86_UINT32_OR_AND_LO + 2,
  GNU_PROPERTY_X86_FEATURE_2_USED = GNU_PROPERTY_X86_UINT32_OR_AND_LO + 1,
  GNU_PROPERTY_HIPROC = 0xdfffffff,
  GNU_PROPERTY_LOUSER = 0xe0000000,
  GNU_PROPERTY_HIUSER = 0xffffffff
};

} // End namespace elfcpp.

// Include internal details after defining the types.
#include "elfcpp_internal.h"

namespace elfcpp
{

// The offset of the ELF file header in the ELF file.

const int file_header_offset = 0;

// ELF structure sizes.

template<int size>
struct Elf_sizes
{
  // Size of ELF file header.
  static const int ehdr_size = sizeof(internal::Ehdr_data<size>);
  // Size of ELF segment header.
  static const int phdr_size = sizeof(internal::Phdr_data<size>);
  // Size of ELF section header.
  static const int shdr_size = sizeof(internal::Shdr_data<size>);
  // Size of ELF compression header.
  static const int chdr_size = sizeof(internal::Chdr_data<size>);
  // Size of ELF symbol table entry.
  static const int sym_size = sizeof(internal::Sym_data<size>);
  // Sizes of ELF reloc entries.
  static const int rel_size = sizeof(internal::Rel_data<size>);
  static const int rela_size = sizeof(internal::Rela_data<size>);
  // Size of ELF dynamic entry.
  static const int dyn_size = sizeof(internal::Dyn_data<size>);
  // Size of ELF version structures.
  static const int verdef_size = sizeof(internal::Verdef_data);
  static const int verdaux_size = sizeof(internal::Verdaux_data);
  static const int verneed_size = sizeof(internal::Verneed_data);
  static const int vernaux_size = sizeof(internal::Vernaux_data);
};

// Accessor class for the ELF file header.

template<int size, bool big_endian>
class Ehdr
{
 public:
  Ehdr(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Ehdr_data<size>*>(p))
  { }

  template<typename File>
  Ehdr(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Ehdr_data<size>*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  const unsigned char*
  get_e_ident() const
  { return this->p_->e_ident; }

  unsigned char
  get_ei_osabi() const
  { return this->p_->e_ident[EI_OSABI]; }

  unsigned char
  get_ei_abiversion() const
  { return this->p_->e_ident[EI_ABIVERSION]; }

  Elf_Half
  get_e_type() const
  { return Convert<16, big_endian>::convert_host(this->p_->e_type); }

  Elf_Half
  get_e_machine() const
  { return Convert<16, big_endian>::convert_host(this->p_->e_machine); }

  Elf_Word
  get_e_version() const
  { return Convert<32, big_endian>::convert_host(this->p_->e_version); }

  typename Elf_types<size>::Elf_Addr
  get_e_entry() const
  { return Convert<size, big_endian>::convert_host(this->p_->e_entry); }

  typename Elf_types<size>::Elf_Off
  get_e_phoff() const
  { return Convert<size, big_endian>::convert_host(this->p_->e_phoff); }

  typename Elf_types<size>::Elf_Off
  get_e_shoff() const
  { return Convert<size, big_endian>::convert_host(this->p_->e_shoff); }

  Elf_Word
  get_e_flags() const
  { return Convert<32, big_endian>::convert_host(this->p_->e_flags); }

  Elf_Half
  get_e_ehsize() const
  { return Convert<16, big_endian>::convert_host(this->p_->e_ehsize); }

  Elf_Half
  get_e_phentsize() const
  { return Convert<16, big_endian>::convert_host(this->p_->e_phentsize); }

  Elf_Half
  get_e_phnum() const
  { return Convert<16, big_endian>::convert_host(this->p_->e_phnum); }

  Elf_Half
  get_e_shentsize() const
  { return Convert<16, big_endian>::convert_host(this->p_->e_shentsize); }

  Elf_Half
  get_e_shnum() const
  { return Convert<16, big_endian>::convert_host(this->p_->e_shnum); }

  Elf_Half
  get_e_shstrndx() const
  { return Convert<16, big_endian>::convert_host(this->p_->e_shstrndx); }

 private:
  const internal::Ehdr_data<size>* p_;
};

// Write class for the ELF file header.

template<int size, bool big_endian>
class Ehdr_write
{
 public:
  Ehdr_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Ehdr_data<size>*>(p))
  { }

  void
  put_e_ident(const unsigned char v[EI_NIDENT]) const
  { memcpy(this->p_->e_ident, v, EI_NIDENT); }

  void
  put_e_type(Elf_Half v)
  { this->p_->e_type = Convert<16, big_endian>::convert_host(v); }

  void
  put_e_machine(Elf_Half v)
  { this->p_->e_machine = Convert<16, big_endian>::convert_host(v); }

  void
  put_e_version(Elf_Word v)
  { this->p_->e_version = Convert<32, big_endian>::convert_host(v); }

  void
  put_e_entry(typename Elf_types<size>::Elf_Addr v)
  { this->p_->e_entry = Convert<size, big_endian>::convert_host(v); }

  void
  put_e_phoff(typename Elf_types<size>::Elf_Off v)
  { this->p_->e_phoff = Convert<size, big_endian>::convert_host(v); }

  void
  put_e_shoff(typename Elf_types<size>::Elf_Off v)
  { this->p_->e_shoff = Convert<size, big_endian>::convert_host(v); }

  void
  put_e_flags(Elf_Word v)
  { this->p_->e_flags = Convert<32, big_endian>::convert_host(v); }

  void
  put_e_ehsize(Elf_Half v)
  { this->p_->e_ehsize = Convert<16, big_endian>::convert_host(v); }

  void
  put_e_phentsize(Elf_Half v)
  { this->p_->e_phentsize = Convert<16, big_endian>::convert_host(v); }

  void
  put_e_phnum(Elf_Half v)
  { this->p_->e_phnum = Convert<16, big_endian>::convert_host(v); }

  void
  put_e_shentsize(Elf_Half v)
  { this->p_->e_shentsize = Convert<16, big_endian>::convert_host(v); }

  void
  put_e_shnum(Elf_Half v)
  { this->p_->e_shnum = Convert<16, big_endian>::convert_host(v); }

  void
  put_e_shstrndx(Elf_Half v)
  { this->p_->e_shstrndx = Convert<16, big_endian>::convert_host(v); }

 private:
  internal::Ehdr_data<size>* p_;
};

// Accessor class for an ELF section header.

template<int size, bool big_endian>
class Shdr
{
 public:
  Shdr(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Shdr_data<size>*>(p))
  { }

  template<typename File>
  Shdr(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Shdr_data<size>*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  Elf_Word
  get_sh_name() const
  { return Convert<32, big_endian>::convert_host(this->p_->sh_name); }

  Elf_Word
  get_sh_type() const
  { return Convert<32, big_endian>::convert_host(this->p_->sh_type); }

  typename Elf_types<size>::Elf_WXword
  get_sh_flags() const
  { return Convert<size, big_endian>::convert_host(this->p_->sh_flags); }

  typename Elf_types<size>::Elf_Addr
  get_sh_addr() const
  { return Convert<size, big_endian>::convert_host(this->p_->sh_addr); }

  typename Elf_types<size>::Elf_Off
  get_sh_offset() const
  { return Convert<size, big_endian>::convert_host(this->p_->sh_offset); }

  typename Elf_types<size>::Elf_WXword
  get_sh_size() const
  { return Convert<size, big_endian>::convert_host(this->p_->sh_size); }

  Elf_Word
  get_sh_link() const
  { return Convert<32, big_endian>::convert_host(this->p_->sh_link); }

  Elf_Word
  get_sh_info() const
  { return Convert<32, big_endian>::convert_host(this->p_->sh_info); }

  typename Elf_types<size>::Elf_WXword
  get_sh_addralign() const
  { return
      Convert<size, big_endian>::convert_host(this->p_->sh_addralign); }

  typename Elf_types<size>::Elf_WXword
  get_sh_entsize() const
  { return Convert<size, big_endian>::convert_host(this->p_->sh_entsize); }

 private:
  const internal::Shdr_data<size>* p_;
};

// Write class for an ELF section header.

template<int size, bool big_endian>
class Shdr_write
{
 public:
  Shdr_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Shdr_data<size>*>(p))
  { }

  void
  put_sh_name(Elf_Word v)
  { this->p_->sh_name = Convert<32, big_endian>::convert_host(v); }

  void
  put_sh_type(Elf_Word v)
  { this->p_->sh_type = Convert<32, big_endian>::convert_host(v); }

  void
  put_sh_flags(typename Elf_types<size>::Elf_WXword v)
  { this->p_->sh_flags = Convert<size, big_endian>::convert_host(v); }

  void
  put_sh_addr(typename Elf_types<size>::Elf_Addr v)
  { this->p_->sh_addr = Convert<size, big_endian>::convert_host(v); }

  void
  put_sh_offset(typename Elf_types<size>::Elf_Off v)
  { this->p_->sh_offset = Convert<size, big_endian>::convert_host(v); }

  void
  put_sh_size(typename Elf_types<size>::Elf_WXword v)
  { this->p_->sh_size = Convert<size, big_endian>::convert_host(v); }

  void
  put_sh_link(Elf_Word v)
  { this->p_->sh_link = Convert<32, big_endian>::convert_host(v); }

  void
  put_sh_info(Elf_Word v)
  { this->p_->sh_info = Convert<32, big_endian>::convert_host(v); }

  void
  put_sh_addralign(typename Elf_types<size>::Elf_WXword v)
  { this->p_->sh_addralign = Convert<size, big_endian>::convert_host(v); }

  void
  put_sh_entsize(typename Elf_types<size>::Elf_WXword v)
  { this->p_->sh_entsize = Convert<size, big_endian>::convert_host(v); }

 private:
  internal::Shdr_data<size>* p_;
};

// Accessor class for an ELF compression header.

template<int size, bool big_endian>
class Chdr
{
 public:
  Chdr(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Chdr_data<size>*>(p))
  { }

  template<typename File>
  Chdr(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Chdr_data<size>*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  Elf_Word
  get_ch_type() const
  { return Convert<size, big_endian>::convert_host(this->p_->ch_type); }

  typename Elf_types<size>::Elf_WXword
  get_ch_size() const
  { return Convert<size, big_endian>::convert_host(this->p_->ch_size); }

  typename Elf_types<size>::Elf_WXword
  get_ch_addralign() const
  { return
      Convert<size, big_endian>::convert_host(this->p_->ch_addralign); }

 private:
  const internal::Chdr_data<size>* p_;
};

// Write class for an ELF compression header.

template<int size, bool big_endian>
class Chdr_write
{
 public:
  Chdr_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Chdr_data<size>*>(p))
  { }

  void
  put_ch_type(typename Elf_types<size>::Elf_WXword v)
  { this->p_->ch_type = Convert<size, big_endian>::convert_host(v); }

  void
  put_ch_size(typename Elf_types<size>::Elf_WXword v)
  { this->p_->ch_size = Convert<size, big_endian>::convert_host(v); }

  void
  put_ch_addralign(typename Elf_types<size>::Elf_WXword v)
  { this->p_->ch_addralign = Convert<size, big_endian>::convert_host(v); }

  void
  put_ch_reserved(Elf_Word);

 private:
  internal::Chdr_data<size>* p_;
};

template<>
inline void
elfcpp::Chdr_write<64, true>::put_ch_reserved(Elf_Word v)
{
  this->p_->ch_reserved = v;
}

template<>
inline void
elfcpp::Chdr_write<64, false>::put_ch_reserved(Elf_Word v)
{
  this->p_->ch_reserved = v;
}

// Accessor class for an ELF segment header.

template<int size, bool big_endian>
class Phdr
{
 public:
  Phdr(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Phdr_data<size>*>(p))
  { }

  template<typename File>
  Phdr(File* file, typename File::Location loc)
    : p_(reinterpret_cast<internal::Phdr_data<size>*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  Elf_Word
  get_p_type() const
  { return Convert<32, big_endian>::convert_host(this->p_->p_type); }

  typename Elf_types<size>::Elf_Off
  get_p_offset() const
  { return Convert<size, big_endian>::convert_host(this->p_->p_offset); }

  typename Elf_types<size>::Elf_Addr
  get_p_vaddr() const
  { return Convert<size, big_endian>::convert_host(this->p_->p_vaddr); }

  typename Elf_types<size>::Elf_Addr
  get_p_paddr() const
  { return Convert<size, big_endian>::convert_host(this->p_->p_paddr); }

  typename Elf_types<size>::Elf_WXword
  get_p_filesz() const
  { return Convert<size, big_endian>::convert_host(this->p_->p_filesz); }

  typename Elf_types<size>::Elf_WXword
  get_p_memsz() const
  { return Convert<size, big_endian>::convert_host(this->p_->p_memsz); }

  Elf_Word
  get_p_flags() const
  { return Convert<32, big_endian>::convert_host(this->p_->p_flags); }

  typename Elf_types<size>::Elf_WXword
  get_p_align() const
  { return Convert<size, big_endian>::convert_host(this->p_->p_align); }

 private:
  const internal::Phdr_data<size>* p_;
};

// Write class for an ELF segment header.

template<int size, bool big_endian>
class Phdr_write
{
 public:
  Phdr_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Phdr_data<size>*>(p))
  { }

  void
  put_p_type(Elf_Word v)
  { this->p_->p_type = Convert<32, big_endian>::convert_host(v); }

  void
  put_p_offset(typename Elf_types<size>::Elf_Off v)
  { this->p_->p_offset = Convert<size, big_endian>::convert_host(v); }

  void
  put_p_vaddr(typename Elf_types<size>::Elf_Addr v)
  { this->p_->p_vaddr = Convert<size, big_endian>::convert_host(v); }

  void
  put_p_paddr(typename Elf_types<size>::Elf_Addr v)
  { this->p_->p_paddr = Convert<size, big_endian>::convert_host(v); }

  void
  put_p_filesz(typename Elf_types<size>::Elf_WXword v)
  { this->p_->p_filesz = Convert<size, big_endian>::convert_host(v); }

  void
  put_p_memsz(typename Elf_types<size>::Elf_WXword v)
  { this->p_->p_memsz = Convert<size, big_endian>::convert_host(v); }

  void
  put_p_flags(Elf_Word v)
  { this->p_->p_flags = Convert<32, big_endian>::convert_host(v); }

  void
  put_p_align(typename Elf_types<size>::Elf_WXword v)
  { this->p_->p_align = Convert<size, big_endian>::convert_host(v); }

 private:
  internal::Phdr_data<size>* p_;
};

// Accessor class for an ELF symbol table entry.

template<int size, bool big_endian>
class Sym
{
 public:
  Sym(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Sym_data<size>*>(p))
  { }

  template<typename File>
  Sym(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Sym_data<size>*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  Elf_Word
  get_st_name() const
  { return Convert<32, big_endian>::convert_host(this->p_->st_name); }

  typename Elf_types<size>::Elf_Addr
  get_st_value() const
  { return Convert<size, big_endian>::convert_host(this->p_->st_value); }

  typename Elf_types<size>::Elf_WXword
  get_st_size() const
  { return Convert<size, big_endian>::convert_host(this->p_->st_size); }

  unsigned char
  get_st_info() const
  { return this->p_->st_info; }

  STB
  get_st_bind() const
  { return elf_st_bind(this->get_st_info()); }

  STT
  get_st_type() const
  { return elf_st_type(this->get_st_info()); }

  unsigned char
  get_st_other() const
  { return this->p_->st_other; }

  STV
  get_st_visibility() const
  { return elf_st_visibility(this->get_st_other()); }

  unsigned char
  get_st_nonvis() const
  { return elf_st_nonvis(this->get_st_other()); }

  Elf_Half
  get_st_shndx() const
  { return Convert<16, big_endian>::convert_host(this->p_->st_shndx); }

 private:
  const internal::Sym_data<size>* p_;
};

// Writer class for an ELF symbol table entry.

template<int size, bool big_endian>
class Sym_write
{
 public:
  Sym_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Sym_data<size>*>(p))
  { }

  void
  put_st_name(Elf_Word v)
  { this->p_->st_name = Convert<32, big_endian>::convert_host(v); }

  void
  put_st_value(typename Elf_types<size>::Elf_Addr v)
  { this->p_->st_value = Convert<size, big_endian>::convert_host(v); }

  void
  put_st_size(typename Elf_types<size>::Elf_WXword v)
  { this->p_->st_size = Convert<size, big_endian>::convert_host(v); }

  void
  put_st_info(unsigned char v)
  { this->p_->st_info = v; }

  void
  put_st_info(STB bind, STT type)
  { this->p_->st_info = elf_st_info(bind, type); }

  void
  put_st_other(unsigned char v)
  { this->p_->st_other = v; }

  void
  put_st_other(STV vis, unsigned char nonvis)
  { this->p_->st_other = elf_st_other(vis, nonvis); }

  void
  put_st_shndx(Elf_Half v)
  { this->p_->st_shndx = Convert<16, big_endian>::convert_host(v); }

  Sym<size, big_endian>
  sym()
  { return Sym<size, big_endian>(reinterpret_cast<unsigned char*>(this->p_)); }

 private:
  internal::Sym_data<size>* p_;
};

// Accessor classes for an ELF REL relocation entry.

template<int size, bool big_endian>
class Rel
{
 public:
  Rel(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Rel_data<size>*>(p))
  { }

  template<typename File>
  Rel(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Rel_data<size>*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  typename Elf_types<size>::Elf_Addr
  get_r_offset() const
  { return Convert<size, big_endian>::convert_host(this->p_->r_offset); }

  typename Elf_types<size>::Elf_WXword
  get_r_info() const
  { return Convert<size, big_endian>::convert_host(this->p_->r_info); }

 private:
  const internal::Rel_data<size>* p_;
};

// Writer class for an ELF Rel relocation.

template<int size, bool big_endian>
class Rel_write
{
 public:
  Rel_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Rel_data<size>*>(p))
  { }

  void
  put_r_offset(typename Elf_types<size>::Elf_Addr v)
  { this->p_->r_offset = Convert<size, big_endian>::convert_host(v); }

  void
  put_r_info(typename Elf_types<size>::Elf_WXword v)
  { this->p_->r_info = Convert<size, big_endian>::convert_host(v); }

 private:
  internal::Rel_data<size>* p_;
};

// Accessor class for an ELF Rela relocation.

template<int size, bool big_endian>
class Rela
{
 public:
  Rela(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Rela_data<size>*>(p))
  { }

  template<typename File>
  Rela(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Rela_data<size>*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  typename Elf_types<size>::Elf_Addr
  get_r_offset() const
  { return Convert<size, big_endian>::convert_host(this->p_->r_offset); }

  typename Elf_types<size>::Elf_WXword
  get_r_info() const
  { return Convert<size, big_endian>::convert_host(this->p_->r_info); }

  typename Elf_types<size>::Elf_Swxword
  get_r_addend() const
  { return Convert<size, big_endian>::convert_host(this->p_->r_addend); }

 private:
  const internal::Rela_data<size>* p_;
};

// Writer class for an ELF Rela relocation.

template<int size, bool big_endian>
class Rela_write
{
 public:
  Rela_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Rela_data<size>*>(p))
  { }

  void
  put_r_offset(typename Elf_types<size>::Elf_Addr v)
  { this->p_->r_offset = Convert<size, big_endian>::convert_host(v); }

  void
  put_r_info(typename Elf_types<size>::Elf_WXword v)
  { this->p_->r_info = Convert<size, big_endian>::convert_host(v); }

  void
  put_r_addend(typename Elf_types<size>::Elf_Swxword v)
  { this->p_->r_addend = Convert<size, big_endian>::convert_host(v); }

 private:
  internal::Rela_data<size>* p_;
};

// MIPS-64 has a non-standard relocation layout.

template<bool big_endian>
class Mips64_rel
{
 public:
  Mips64_rel(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Mips64_rel_data*>(p))
  { }

  template<typename File>
  Mips64_rel(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Mips64_rel_data*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  typename Elf_types<64>::Elf_Addr
  get_r_offset() const
  { return Convert<64, big_endian>::convert_host(this->p_->r_offset); }

  Elf_Word
  get_r_sym() const
  { return Convert<32, big_endian>::convert_host(this->p_->r_sym); }

  unsigned char
  get_r_ssym() const
  { return this->p_->r_ssym; }

  unsigned char
  get_r_type() const
  { return this->p_->r_type; }

  unsigned char
  get_r_type2() const
  { return this->p_->r_type2; }

  unsigned char
  get_r_type3() const
  { return this->p_->r_type3; }

 private:
  const internal::Mips64_rel_data* p_;
};

template<bool big_endian>
class Mips64_rel_write
{
 public:
  Mips64_rel_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Mips64_rel_data*>(p))
  { }

  void
  put_r_offset(typename Elf_types<64>::Elf_Addr v)
  { this->p_->r_offset = Convert<64, big_endian>::convert_host(v); }

  void
  put_r_sym(Elf_Word v)
  { this->p_->r_sym = Convert<32, big_endian>::convert_host(v); }

  void
  put_r_ssym(unsigned char v)
  { this->p_->r_ssym = v; }

  void
  put_r_type(unsigned char v)
  { this->p_->r_type = v; }

  void
  put_r_type2(unsigned char v)
  { this->p_->r_type2 = v; }

  void
  put_r_type3(unsigned char v)
  { this->p_->r_type3 = v; }

 private:
  internal::Mips64_rel_data* p_;
};

template<bool big_endian>
class Mips64_rela
{
 public:
  Mips64_rela(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Mips64_rela_data*>(p))
  { }

  template<typename File>
  Mips64_rela(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Mips64_rela_data*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  typename Elf_types<64>::Elf_Addr
  get_r_offset() const
  { return Convert<64, big_endian>::convert_host(this->p_->r_offset); }

  Elf_Word
  get_r_sym() const
  { return Convert<32, big_endian>::convert_host(this->p_->r_sym); }

  unsigned char
  get_r_ssym() const
  { return this->p_->r_ssym; }

  unsigned char
  get_r_type() const
  { return this->p_->r_type; }

  unsigned char
  get_r_type2() const
  { return this->p_->r_type2; }

  unsigned char
  get_r_type3() const
  { return this->p_->r_type3; }

  typename Elf_types<64>::Elf_Swxword
  get_r_addend() const
  { return Convert<64, big_endian>::convert_host(this->p_->r_addend); }

 private:
  const internal::Mips64_rela_data* p_;
};

template<bool big_endian>
class Mips64_rela_write
{
 public:
  Mips64_rela_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Mips64_rela_data*>(p))
  { }

  void
  put_r_offset(typename Elf_types<64>::Elf_Addr v)
  { this->p_->r_offset = Convert<64, big_endian>::convert_host(v); }

  void
  put_r_sym(Elf_Word v)
  { this->p_->r_sym = Convert<32, big_endian>::convert_host(v); }

  void
  put_r_ssym(unsigned char v)
  { this->p_->r_ssym = v; }

  void
  put_r_type(unsigned char v)
  { this->p_->r_type = v; }

  void
  put_r_type2(unsigned char v)
  { this->p_->r_type2 = v; }

  void
  put_r_type3(unsigned char v)
  { this->p_->r_type3 = v; }

  void
  put_r_addend(typename Elf_types<64>::Elf_Swxword v)
  { this->p_->r_addend = Convert<64, big_endian>::convert_host(v); }

 private:
  internal::Mips64_rela_data* p_;
};

// Accessor classes for entries in the ELF SHT_DYNAMIC section aka
// PT_DYNAMIC segment.

template<int size, bool big_endian>
class Dyn
{
 public:
  Dyn(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Dyn_data<size>*>(p))
  { }

  template<typename File>
  Dyn(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Dyn_data<size>*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  typename Elf_types<size>::Elf_Swxword
  get_d_tag() const
  { return Convert<size, big_endian>::convert_host(this->p_->d_tag); }

  typename Elf_types<size>::Elf_WXword
  get_d_val() const
  { return Convert<size, big_endian>::convert_host(this->p_->d_val); }

  typename Elf_types<size>::Elf_Addr
  get_d_ptr() const
  { return Convert<size, big_endian>::convert_host(this->p_->d_val); }

 private:
  const internal::Dyn_data<size>* p_;
};

// Write class for an entry in the SHT_DYNAMIC section.

template<int size, bool big_endian>
class Dyn_write
{
 public:
  Dyn_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Dyn_data<size>*>(p))
  { }

  void
  put_d_tag(typename Elf_types<size>::Elf_Swxword v)
  { this->p_->d_tag = Convert<size, big_endian>::convert_host(v); }

  void
  put_d_val(typename Elf_types<size>::Elf_WXword v)
  { this->p_->d_val = Convert<size, big_endian>::convert_host(v); }

  void
  put_d_ptr(typename Elf_types<size>::Elf_Addr v)
  { this->p_->d_val = Convert<size, big_endian>::convert_host(v); }

 private:
  internal::Dyn_data<size>* p_;
};

// Accessor classes for entries in the ELF SHT_GNU_verdef section.

template<int size, bool big_endian>
class Verdef
{
 public:
  Verdef(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Verdef_data*>(p))
  { }

  template<typename File>
  Verdef(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Verdef_data*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  Elf_Half
  get_vd_version() const
  { return Convert<16, big_endian>::convert_host(this->p_->vd_version); }

  Elf_Half
  get_vd_flags() const
  { return Convert<16, big_endian>::convert_host(this->p_->vd_flags); }

  Elf_Half
  get_vd_ndx() const
  { return Convert<16, big_endian>::convert_host(this->p_->vd_ndx); }

  Elf_Half
  get_vd_cnt() const
  { return Convert<16, big_endian>::convert_host(this->p_->vd_cnt); }

  Elf_Word
  get_vd_hash() const
  { return Convert<32, big_endian>::convert_host(this->p_->vd_hash); }

  Elf_Word
  get_vd_aux() const
  { return Convert<32, big_endian>::convert_host(this->p_->vd_aux); }

  Elf_Word
  get_vd_next() const
  { return Convert<32, big_endian>::convert_host(this->p_->vd_next); }

 private:
  const internal::Verdef_data* p_;
};

template<int size, bool big_endian>
class Verdef_write
{
 public:
  Verdef_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Verdef_data*>(p))
  { }

  void
  set_vd_version(Elf_Half v)
  { this->p_->vd_version = Convert<16, big_endian>::convert_host(v); }

  void
  set_vd_flags(Elf_Half v)
  { this->p_->vd_flags = Convert<16, big_endian>::convert_host(v); }

  void
  set_vd_ndx(Elf_Half v)
  { this->p_->vd_ndx = Convert<16, big_endian>::convert_host(v); }

  void
  set_vd_cnt(Elf_Half v)
  { this->p_->vd_cnt = Convert<16, big_endian>::convert_host(v); }

  void
  set_vd_hash(Elf_Word v)
  { this->p_->vd_hash = Convert<32, big_endian>::convert_host(v); }

  void
  set_vd_aux(Elf_Word v)
  { this->p_->vd_aux = Convert<32, big_endian>::convert_host(v); }

  void
  set_vd_next(Elf_Word v)
  { this->p_->vd_next = Convert<32, big_endian>::convert_host(v); }

 private:
  internal::Verdef_data* p_;
};

// Accessor classes for auxiliary entries in the ELF SHT_GNU_verdef
// section.

template<int size, bool big_endian>
class Verdaux
{
 public:
  Verdaux(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Verdaux_data*>(p))
  { }

  template<typename File>
  Verdaux(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Verdaux_data*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  Elf_Word
  get_vda_name() const
  { return Convert<32, big_endian>::convert_host(this->p_->vda_name); }

  Elf_Word
  get_vda_next() const
  { return Convert<32, big_endian>::convert_host(this->p_->vda_next); }

 private:
  const internal::Verdaux_data* p_;
};

template<int size, bool big_endian>
class Verdaux_write
{
 public:
  Verdaux_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Verdaux_data*>(p))
  { }

  void
  set_vda_name(Elf_Word v)
  { this->p_->vda_name = Convert<32, big_endian>::convert_host(v); }

  void
  set_vda_next(Elf_Word v)
  { this->p_->vda_next = Convert<32, big_endian>::convert_host(v); }

 private:
  internal::Verdaux_data* p_;
};

// Accessor classes for entries in the ELF SHT_GNU_verneed section.

template<int size, bool big_endian>
class Verneed
{
 public:
  Verneed(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Verneed_data*>(p))
  { }

  template<typename File>
  Verneed(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Verneed_data*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  Elf_Half
  get_vn_version() const
  { return Convert<16, big_endian>::convert_host(this->p_->vn_version); }

  Elf_Half
  get_vn_cnt() const
  { return Convert<16, big_endian>::convert_host(this->p_->vn_cnt); }

  Elf_Word
  get_vn_file() const
  { return Convert<32, big_endian>::convert_host(this->p_->vn_file); }

  Elf_Word
  get_vn_aux() const
  { return Convert<32, big_endian>::convert_host(this->p_->vn_aux); }

  Elf_Word
  get_vn_next() const
  { return Convert<32, big_endian>::convert_host(this->p_->vn_next); }

 private:
  const internal::Verneed_data* p_;
};

template<int size, bool big_endian>
class Verneed_write
{
 public:
  Verneed_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Verneed_data*>(p))
  { }

  void
  set_vn_version(Elf_Half v)
  { this->p_->vn_version = Convert<16, big_endian>::convert_host(v); }

  void
  set_vn_cnt(Elf_Half v)
  { this->p_->vn_cnt = Convert<16, big_endian>::convert_host(v); }

  void
  set_vn_file(Elf_Word v)
  { this->p_->vn_file = Convert<32, big_endian>::convert_host(v); }

  void
  set_vn_aux(Elf_Word v)
  { this->p_->vn_aux = Convert<32, big_endian>::convert_host(v); }

  void
  set_vn_next(Elf_Word v)
  { this->p_->vn_next = Convert<32, big_endian>::convert_host(v); }

 private:
  internal::Verneed_data* p_;
};

// Accessor classes for auxiliary entries in the ELF SHT_GNU_verneed
// section.

template<int size, bool big_endian>
class Vernaux
{
 public:
  Vernaux(const unsigned char* p)
    : p_(reinterpret_cast<const internal::Vernaux_data*>(p))
  { }

  template<typename File>
  Vernaux(File* file, typename File::Location loc)
    : p_(reinterpret_cast<const internal::Vernaux_data*>(
	   file->view(loc.file_offset, loc.data_size).data()))
  { }

  Elf_Word
  get_vna_hash() const
  { return Convert<32, big_endian>::convert_host(this->p_->vna_hash); }

  Elf_Half
  get_vna_flags() const
  { return Convert<16, big_endian>::convert_host(this->p_->vna_flags); }

  Elf_Half
  get_vna_other() const
  { return Convert<16, big_endian>::convert_host(this->p_->vna_other); }

  Elf_Word
  get_vna_name() const
  { return Convert<32, big_endian>::convert_host(this->p_->vna_name); }

  Elf_Word
  get_vna_next() const
  { return Convert<32, big_endian>::convert_host(this->p_->vna_next); }

 private:
  const internal::Vernaux_data* p_;
};

template<int size, bool big_endian>
class Vernaux_write
{
 public:
  Vernaux_write(unsigned char* p)
    : p_(reinterpret_cast<internal::Vernaux_data*>(p))
  { }

  void
  set_vna_hash(Elf_Word v)
  { this->p_->vna_hash = Convert<32, big_endian>::convert_host(v); }

  void
  set_vna_flags(Elf_Half v)
  { this->p_->vna_flags = Convert<16, big_endian>::convert_host(v); }

  void
  set_vna_other(Elf_Half v)
  { this->p_->vna_other = Convert<16, big_endian>::convert_host(v); }

  void
  set_vna_name(Elf_Word v)
  { this->p_->vna_name = Convert<32, big_endian>::convert_host(v); }

  void
  set_vna_next(Elf_Word v)
  { this->p_->vna_next = Convert<32, big_endian>::convert_host(v); }

 private:
  internal::Vernaux_data* p_;
};

} // End namespace elfcpp.

#endif // !defined(ELFPCP_H)
