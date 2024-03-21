// testfile.cc -- Dummy ELF objects for testing purposes.

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "gold.h"

#include "target.h"
#include "target-select.h"

#include "test.h"
#include "testfile.h"

namespace gold_testsuite
{

using namespace gold;

// A Target used for testing purposes.

template<int size, bool big_endian>
class Target_test : public Sized_target<size, big_endian>
{
 public:
  Target_test()
    : Sized_target<size, big_endian>(&test_target_info)
  { }

  void
  gc_process_relocs(Symbol_table*, Layout*,
		    Sized_relobj_file<size, big_endian>*,
		    unsigned int, unsigned int, const unsigned char*, size_t,
		    Output_section*, bool, size_t, const unsigned char*)
  { ERROR("call to Target_test::gc_process_relocs"); }

  void
  scan_relocs(Symbol_table*, Layout*, Sized_relobj_file<size, big_endian>*,
	      unsigned int, unsigned int, const unsigned char*, size_t,
	      Output_section*, bool, size_t, const unsigned char*)
  { ERROR("call to Target_test::scan_relocs"); }

  void
  relocate_section(const Relocate_info<size, big_endian>*, unsigned int,
		   const unsigned char*, size_t, Output_section*, bool,
		   unsigned char*, typename elfcpp::Elf_types<size>::Elf_Addr,
		   section_size_type, const Reloc_symbol_changes*)
  { ERROR("call to Target_test::relocate_section"); }

  void
  scan_relocatable_relocs(Symbol_table*, Layout*,
			  Sized_relobj_file<size, big_endian>*, unsigned int,
			  unsigned int, const unsigned char*,
			  size_t, Output_section*, bool, size_t,
			  const unsigned char*, Relocatable_relocs*)
  { ERROR("call to Target_test::scan_relocatable_relocs"); }

  void
  emit_relocs_scan(Symbol_table*, Layout*,
		   Sized_relobj_file<size, big_endian>*, unsigned int,
		   unsigned int, const unsigned char*,
		   size_t, Output_section*, bool, size_t,
		   const unsigned char*, Relocatable_relocs*)
  { ERROR("call to Target_test::emit_relocs_scan"); }

  void
  relocate_relocs(const Relocate_info<size, big_endian>*,
		  unsigned int, const unsigned char*, size_t,
		  Output_section*, typename elfcpp::Elf_types<size>::Elf_Off,
		  unsigned char*,
		  typename elfcpp::Elf_types<size>::Elf_Addr,
		  section_size_type, unsigned char*,
		  section_size_type)
  { ERROR("call to Target_test::relocate_relocs"); }

  static const Target::Target_info test_target_info;
};

template<int size, bool big_endian>
const Target::Target_info Target_test<size, big_endian>::test_target_info =
{
  size,					// size
  big_endian,				// is_big_endian
  static_cast<elfcpp::EM>(0xffff),	// machine_code
  false,				// has_make_symbol
  false,				// has_resolve
  false,				// has_code_fill
  false,				// is_default_stack_executable
  false,				// can_icf_inline_merge_sections
  '\0',					// wrap_char
  "/dummy",				// dynamic_linker
  0x08000000,				// default_text_segment_address
  0x1000,				// abi_pagesize
  0x1000,				// common_pagesize
  false,                                // isolate_execinstr
  0,                                    // rosegment_gap
  elfcpp::SHN_UNDEF,			// small_common_shndx
  elfcpp::SHN_UNDEF,			// large_common_shndx
  0,					// small_common_section_flags
  0,					// large_common_section_flags
  NULL,					// attributes_section
  NULL,					// attributes_vendor
  "_start",				// entry_symbol_name
  32,					// hash_entry_size
  elfcpp::SHT_PROGBITS,			// unwind_section_type
};

// The test targets.

#ifdef HAVE_TARGET_32_LITTLE
Target_test<32, false> target_test_32_little;
#endif

#ifdef HAVE_TARGET_32_BIG
Target_test<32, true> target_test_32_big;
#endif

#ifdef HAVE_TARGET_64_LITTLE
Target_test<64, false> target_test_64_little;
#endif

#ifdef HAVE_TARGET_64_BIG
Target_test<64, true> target_test_64_big;
#endif

// A pointer to the test targets.  This is used in CHECKs.

#ifdef HAVE_TARGET_32_LITTLE
Target* target_test_pointer_32_little = &target_test_32_little;
#endif

#ifdef HAVE_TARGET_32_BIG
Target* target_test_pointer_32_big = &target_test_32_big;
#endif

#ifdef HAVE_TARGET_64_LITTLE
Target* target_test_pointer_64_little = &target_test_64_little;
#endif

#ifdef HAVE_TARGET_64_BIG
Target* target_test_pointer_64_big = &target_test_64_big;
#endif

// Select the test targets.

template<int size, bool big_endian>
class Target_selector_test : public Target_selector
{
 public:
  Target_selector_test()
    : Target_selector(0xffff, size, big_endian, NULL, NULL)
  { }

  virtual Target*
  do_instantiate_target()
  {
    gold_unreachable();
    return NULL;
  }

  virtual Target*
  do_recognize(Input_file*, off_t, int, int, int)
  {
    if (size == 32)
      {
	if (!big_endian)
	  {
#ifdef HAVE_TARGET_32_LITTLE
	    return &target_test_32_little;
#endif
	  }
	else
	  {
#ifdef HAVE_TARGET_32_BIG
	    return &target_test_32_big;
#endif
	  }
      }
    else
      {
	if (!big_endian)
	  {
#ifdef HAVE_TARGET_64_LITTLE
	    return &target_test_64_little;
#endif
	  }
	else
	  {
#ifdef HAVE_TARGET_64_BIG
	    return &target_test_64_big;
#endif
	  }
      }

    return NULL;
  }

  virtual Target*
  do_recognize_by_name(const char*)
  { return NULL; }

  virtual void
  do_supported_names(std::vector<const char*>*)
  { }
};

// Register the test target selectors.  These don't need to be
// conditionally compiled, as they will return NULL if there is no
// support for them.

Target_selector_test<32, false> target_selector_test_32_little;
Target_selector_test<32, true> target_selector_test_32_big;
Target_selector_test<64, false> target_selector_test_64_little;
Target_selector_test<64, true> target_selector_test_64_big;

// A simple ELF object with one empty section, named ".test" and one
// globally visible symbol named "test".

const unsigned char test_file_1_32_little[] =
{
  // Ehdr
  // EI_MAG[0-3]
  0x7f, 'E', 'L', 'F',
  // EI_CLASS: 32 bit.
  1,
  // EI_DATA: little endian
  1,
  // EI_VERSION
  1,
  // EI_OSABI
  0,
  // EI_ABIVERSION
  0,
  // EI_PAD
  0, 0, 0, 0, 0, 0, 0,
  // e_type: ET_REL
  1, 0,
  // e_machine: a magic value used for testing.
  0xff, 0xff,
  // e_version
  1, 0, 0, 0,
  // e_entry
  0, 0, 0, 0,
  // e_phoff
  0, 0, 0, 0,
  // e_shoff: starts right after file header
  52, 0, 0, 0,
  // e_flags
  0, 0, 0, 0,
  // e_ehsize
  52, 0,
  // e_phentsize
  32, 0,
  // e_phnum
  0, 0,
  // e_shentsize
  40, 0,
  // e_shnum: dummy, .test, .symtab, .strtab, .shstrtab
  5, 0,
  // e_shstrndx
  4, 0,

  // Offset 52
  // Shdr 0: dummy entry
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 92
  // Shdr 1: .test
  // sh_name: after initial null
  1, 0, 0, 0,
  // sh_type: SHT_PROGBITS
  1, 0, 0, 0,
  // sh_flags: SHF_ALLOC
  2, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0,
  // sh_offset: after file header + 5 section headers
  252, 0, 0, 0,
  // sh_size
  0, 0, 0, 0,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  1, 0, 0, 0,
  // sh_entsize
  0, 0, 0, 0,

  // Offset 132
  // Shdr 2: .symtab
  // sh_name: 1 null byte + ".test\0"
  7, 0, 0, 0,
  // sh_type: SHT_SYMTAB
  2, 0, 0, 0,
  // sh_flags
  0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0,
  // sh_offset: after file header + 5 section headers + empty section
  252, 0, 0, 0,
  // sh_size: two symbols: dummy symbol + test symbol
  32, 0, 0, 0,
  // sh_link: to .strtab
  3, 0, 0, 0,
  // sh_info: one local symbol, the dummy symbol
  1, 0, 0, 0,
  // sh_addralign
  4, 0, 0, 0,
  // sh_entsize: size of symbol
  16, 0, 0, 0,

  // Offset 172
  // Shdr 3: .strtab
  // sh_name: 1 null byte + ".test\0" + ".symtab\0"
  15, 0, 0, 0,
  // sh_type: SHT_STRTAB
  3, 0, 0, 0,
  // sh_flags
  0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0,
  // sh_offset: after .symtab section.  284 == 0x11c
  0x1c, 0x1, 0, 0,
  // sh_size: 1 null byte + "test\0"
  6, 0, 0, 0,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  1, 0, 0, 0,
  // sh_entsize
  0, 0, 0, 0,

  // Offset 212
  // Shdr 4: .shstrtab
  // sh_name: 1 null byte + ".test\0" + ".symtab\0" + ".strtab\0"
  23, 0, 0, 0,
  // sh_type: SHT_STRTAB
  3, 0, 0, 0,
  // sh_flags
  0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0,
  // sh_offset: after .strtab section.  290 == 0x122
  0x22, 0x1, 0, 0,
  // sh_size: all section names
  33, 0, 0, 0,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  1, 0, 0, 0,
  // sh_entsize
  0, 0, 0, 0,

  // Offset 252
  // Contents of .symtab section
  // Symbol 0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 268
  // Symbol 1
  // st_name
  1, 0, 0, 0,
  // st_value
  0, 0, 0, 0,
  // st_size
  0, 0, 0, 0,
  // st_info: STT_NOTYPE, STB_GLOBAL
  0x10,
  // st_other
  0,
  // st_shndx: In .test
  1, 0,

  // Offset 284
  // Contents of .strtab section
  '\0',
  't', 'e', 's', 't', '\0',

  // Offset 290
  // Contents of .shstrtab section
  '\0',
  '.', 't', 'e', 's', 't', '\0',
  '.', 's', 'y', 'm', 't', 'a', 'b', '\0',
  '.', 's', 't', 'r', 't', 'a', 'b', '\0',
  '.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', '\0'
};

const unsigned int test_file_1_size_32_little = sizeof test_file_1_32_little;

// 32-bit big-endian version of test_file_1_32_little.

const unsigned char test_file_1_32_big[] =
{
  // Ehdr
  // EI_MAG[0-3]
  0x7f, 'E', 'L', 'F',
  // EI_CLASS: 32 bit.
  1,
  // EI_DATA: big endian
  2,
  // EI_VERSION
  1,
  // EI_OSABI
  0,
  // EI_ABIVERSION
  0,
  // EI_PAD
  0, 0, 0, 0, 0, 0, 0,
  // e_type: ET_REL
  0, 1,
  // e_machine: a magic value used for testing.
  0xff, 0xff,
  // e_version
  0, 0, 0, 1,
  // e_entry
  0, 0, 0, 0,
  // e_phoff
  0, 0, 0, 0,
  // e_shoff: starts right after file header
  0, 0, 0, 52,
  // e_flags
  0, 0, 0, 0,
  // e_ehsize
  0, 52,
  // e_phentsize
  0, 32,
  // e_phnum
  0, 0,
  // e_shentsize
  0, 40,
  // e_shnum: dummy, .test, .symtab, .strtab, .shstrtab
  0, 5,
  // e_shstrndx
  0, 4,

  // Offset 52
  // Shdr 0: dummy entry
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 92
  // Shdr 1: .test
  // sh_name: after initial null
  0, 0, 0, 1,
  // sh_type: SHT_PROGBITS
  0, 0, 0, 1,
  // sh_flags: SHF_ALLOC
  0, 0, 0, 2,
  // sh_addr
  0, 0, 0, 0,
  // sh_offset: after file header + 5 section headers
  0, 0, 0, 252,
  // sh_size
  0, 0, 0, 0,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  0, 0, 0, 1,
  // sh_entsize
  0, 0, 0, 0,

  // Offset 132
  // Shdr 2: .symtab
  // sh_name: 1 null byte + ".test\0"
  0, 0, 0, 7,
  // sh_type: SHT_SYMTAB
  0, 0, 0, 2,
  // sh_flags
  0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0,
  // sh_offset: after file header + 5 section headers + empty section
  0, 0, 0, 252,
  // sh_size: two symbols: dummy symbol + test symbol
  0, 0, 0, 32,
  // sh_link: to .strtab
  0, 0, 0, 3,
  // sh_info: one local symbol, the dummy symbol
  0, 0, 0, 1,
  // sh_addralign
  0, 0, 0, 4,
  // sh_entsize: size of symbol
  0, 0, 0, 16,

  // Offset 172
  // Shdr 3: .strtab
  // sh_name: 1 null byte + ".test\0" + ".symtab\0"
  0, 0, 0, 15,
  // sh_type: SHT_STRTAB
  0, 0, 0, 3,
  // sh_flags
  0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0,
  // sh_offset: after .symtab section.  284 == 0x11c
  0, 0, 0x1, 0x1c,
  // sh_size: 1 null byte + "test\0"
  0, 0, 0, 6,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  0, 0, 0, 1,
  // sh_entsize
  0, 0, 0, 0,

  // Offset 212
  // Shdr 4: .shstrtab
  // sh_name: 1 null byte + ".test\0" + ".symtab\0" + ".strtab\0"
  0, 0, 0, 23,
  // sh_type: SHT_STRTAB
  0, 0, 0, 3,
  // sh_flags
  0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0,
  // sh_offset: after .strtab section.  290 == 0x122
  0, 0, 0x1, 0x22,
  // sh_size: all section names
  0, 0, 0, 33,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  0, 0, 0, 1,
  // sh_entsize
  0, 0, 0, 0,

  // Offset 252
  // Contents of .symtab section
  // Symbol 0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 268
  // Symbol 1
  // st_name
  0, 0, 0, 1,
  // st_value
  0, 0, 0, 0,
  // st_size
  0, 0, 0, 0,
  // st_info: STT_NOTYPE, STB_GLOBAL
  0x10,
  // st_other
  0,
  // st_shndx: In .test
  0, 1,

  // Offset 284
  // Contents of .strtab section
  '\0',
  't', 'e', 's', 't', '\0',

  // Offset 290
  // Contents of .shstrtab section
  '\0',
  '.', 't', 'e', 's', 't', '\0',
  '.', 's', 'y', 'm', 't', 'a', 'b', '\0',
  '.', 's', 't', 'r', 't', 'a', 'b', '\0',
  '.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', '\0'
};

const unsigned int test_file_1_size_32_big = sizeof test_file_1_32_big;

// 64-bit little-endian version of test_file_1_32_little.

const unsigned char test_file_1_64_little[] =
{
  // Ehdr
  // EI_MAG[0-3]
  0x7f, 'E', 'L', 'F',
  // EI_CLASS: 64 bit.
  2,
  // EI_DATA: little endian
  1,
  // EI_VERSION
  1,
  // EI_OSABI
  0,
  // EI_ABIVERSION
  0,
  // EI_PAD
  0, 0, 0, 0, 0, 0, 0,
  // e_type: ET_REL
  1, 0,
  // e_machine: a magic value used for testing.
  0xff, 0xff,
  // e_version
  1, 0, 0, 0,
  // e_entry
  0, 0, 0, 0, 0, 0, 0, 0,
  // e_phoff
  0, 0, 0, 0, 0, 0, 0, 0,
  // e_shoff: starts right after file header
  64, 0, 0, 0, 0, 0, 0, 0,
  // e_flags
  0, 0, 0, 0,
  // e_ehsize
  64, 0,
  // e_phentsize
  56, 0,
  // e_phnum
  0, 0,
  // e_shentsize
  64, 0,
  // e_shnum: dummy, .test, .symtab, .strtab, .shstrtab
  5, 0,
  // e_shstrndx
  4, 0,

  // Offset 64
  // Shdr 0: dummy entry
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 128
  // Shdr 1: .test
  // sh_name: after initial null
  1, 0, 0, 0,
  // sh_type: SHT_PROGBITS
  1, 0, 0, 0,
  // sh_flags: SHF_ALLOC
  2, 0, 0, 0, 0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_offset: after file header + 5 section headers.  384 == 0x180.
  0x80, 0x1, 0, 0, 0, 0, 0, 0,
  // sh_size
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  1, 0, 0, 0, 0, 0, 0, 0,
  // sh_entsize
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 192
  // Shdr 2: .symtab
  // sh_name: 1 null byte + ".test\0"
  7, 0, 0, 0,
  // sh_type: SHT_SYMTAB
  2, 0, 0, 0,
  // sh_flags
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_offset: after file header + 5 section headers + empty section
  // 384 == 0x180.
  0x80, 0x1, 0, 0, 0, 0, 0, 0,
  // sh_size: two symbols: dummy symbol + test symbol
  48, 0, 0, 0, 0, 0, 0, 0,
  // sh_link: to .strtab
  3, 0, 0, 0,
  // sh_info: one local symbol, the dummy symbol
  1, 0, 0, 0,
  // sh_addralign
  8, 0, 0, 0, 0, 0, 0, 0,
  // sh_entsize: size of symbol
  24, 0, 0, 0, 0, 0, 0, 0,

  // Offset 256
  // Shdr 3: .strtab
  // sh_name: 1 null byte + ".test\0" + ".symtab\0"
  15, 0, 0, 0,
  // sh_type: SHT_STRTAB
  3, 0, 0, 0,
  // sh_flags
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_offset: after .symtab section.  432 == 0x1b0
  0xb0, 0x1, 0, 0, 0, 0, 0, 0,
  // sh_size: 1 null byte + "test\0"
  6, 0, 0, 0, 0, 0, 0, 0,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  1, 0, 0, 0, 0, 0, 0, 0,
  // sh_entsize
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 320
  // Shdr 4: .shstrtab
  // sh_name: 1 null byte + ".test\0" + ".symtab\0" + ".strtab\0"
  23, 0, 0, 0,
  // sh_type: SHT_STRTAB
  3, 0, 0, 0,
  // sh_flags
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_offset: after .strtab section.  438 == 0x1b6
  0xb6, 0x1, 0, 0, 0, 0, 0, 0,
  // sh_size: all section names
  33, 0, 0, 0, 0, 0, 0, 0,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  1, 0, 0, 0, 0, 0, 0, 0,
  // sh_entsize
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 384
  // Contents of .symtab section
  // Symbol 0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 408
  // Symbol 1
  // st_name
  1, 0, 0, 0,
  // st_info: STT_NOTYPE, STB_GLOBAL
  0x10,
  // st_other
  0,
  // st_shndx: In .test
  1, 0,
  // st_value
  0, 0, 0, 0, 0, 0, 0, 0,
  // st_size
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 432
  // Contents of .strtab section
  '\0',
  't', 'e', 's', 't', '\0',

  // Offset 438
  // Contents of .shstrtab section
  '\0',
  '.', 't', 'e', 's', 't', '\0',
  '.', 's', 'y', 'm', 't', 'a', 'b', '\0',
  '.', 's', 't', 'r', 't', 'a', 'b', '\0',
  '.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', '\0'
};

const unsigned int test_file_1_size_64_little = sizeof test_file_1_64_little;

// 64-bit big-endian version of test_file_1_32_little.

const unsigned char test_file_1_64_big[] =
{
  // Ehdr
  // EI_MAG[0-3]
  0x7f, 'E', 'L', 'F',
  // EI_CLASS: 64 bit.
  2,
  // EI_DATA: big endian
  2,
  // EI_VERSION
  1,
  // EI_OSABI
  0,
  // EI_ABIVERSION
  0,
  // EI_PAD
  0, 0, 0, 0, 0, 0, 0,
  // e_type: ET_REL
  0, 1,
  // e_machine: a magic value used for testing.
  0xff, 0xff,
  // e_version
  0, 0, 0, 1,
  // e_entry
  0, 0, 0, 0, 0, 0, 0, 0,
  // e_phoff
  0, 0, 0, 0, 0, 0, 0, 0,
  // e_shoff: starts right after file header
  0, 0, 0, 0, 0, 0, 0, 64,
  // e_flags
  0, 0, 0, 0,
  // e_ehsize
  0, 64,
  // e_phentsize
  0, 56,
  // e_phnum
  0, 0,
  // e_shentsize
  0, 64,
  // e_shnum: dummy, .test, .symtab, .strtab, .shstrtab
  0, 5,
  // e_shstrndx
  0, 4,

  // Offset 64
  // Shdr 0: dummy entry
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 128
  // Shdr 1: .test
  // sh_name: after initial null
  0, 0, 0, 1,
  // sh_type: SHT_PROGBITS
  0, 0, 0, 1,
  // sh_flags: SHF_ALLOC
  0, 0, 0, 0, 0, 0, 0, 2,
  // sh_addr
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_offset: after file header + 5 section headers.  384 == 0x180.
  0, 0, 0, 0, 0, 0, 0x1, 0x80,
  // sh_size
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  0, 0, 0, 0, 0, 0, 0, 1,
  // sh_entsize
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 192
  // Shdr 2: .symtab
  // sh_name: 1 null byte + ".test\0"
  0, 0, 0, 7,
  // sh_type: SHT_SYMTAB
  0, 0, 0, 2,
  // sh_flags
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_offset: after file header + 5 section headers + empty section
  // 384 == 0x180.
  0, 0, 0, 0, 0, 0, 0x1, 0x80,
  // sh_size: two symbols: dummy symbol + test symbol
  0, 0, 0, 0, 0, 0, 0, 48,
  // sh_link: to .strtab
  0, 0, 0, 3,
  // sh_info: one local symbol, the dummy symbol
  0, 0, 0, 1,
  // sh_addralign
  0, 0, 0, 0, 0, 0, 0, 8,
  // sh_entsize: size of symbol
  0, 0, 0, 0, 0, 0, 0, 24,

  // Offset 256
  // Shdr 3: .strtab
  // sh_name: 1 null byte + ".test\0" + ".symtab\0"
  0, 0, 0, 15,
  // sh_type: SHT_STRTAB
  0, 0, 0, 3,
  // sh_flags
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_offset: after .symtab section.  432 == 0x1b0
  0, 0, 0, 0, 0, 0, 0x1, 0xb0,
  // sh_size: 1 null byte + "test\0"
  0, 0, 0, 0, 0, 0, 0, 6,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  0, 0, 0, 0, 0, 0, 0, 1,
  // sh_entsize
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 320
  // Shdr 4: .shstrtab
  // sh_name: 1 null byte + ".test\0" + ".symtab\0" + ".strtab\0"
  0, 0, 0, 23,
  // sh_type: SHT_STRTAB
  0, 0, 0, 3,
  // sh_flags
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_addr
  0, 0, 0, 0, 0, 0, 0, 0,
  // sh_offset: after .strtab section.  438 == 0x1b6
  0, 0, 0, 0, 0, 0, 0x1, 0xb6,
  // sh_size: all section names
  0, 0, 0, 0, 0, 0, 0, 33,
  // sh_link
  0, 0, 0, 0,
  // sh_info
  0, 0, 0, 0,
  // sh_addralign
  0, 0, 0, 0, 0, 0, 0, 1,
  // sh_entsize
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 384
  // Contents of .symtab section
  // Symbol 0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 408
  // Symbol 1
  // st_name
  0, 0, 0, 1,
  // st_info: STT_NOTYPE, STB_GLOBAL
  0x10,
  // st_other
  0,
  // st_shndx: In .test
  0, 1,
  // st_value
  0, 0, 0, 0, 0, 0, 0, 0,
  // st_size
  0, 0, 0, 0, 0, 0, 0, 0,

  // Offset 432
  // Contents of .strtab section
  '\0',
  't', 'e', 's', 't', '\0',

  // Offset 438
  // Contents of .shstrtab section
  '\0',
  '.', 't', 'e', 's', 't', '\0',
  '.', 's', 'y', 'm', 't', 'a', 'b', '\0',
  '.', 's', 't', 'r', 't', 'a', 'b', '\0',
  '.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', '\0'
};

const unsigned int test_file_1_size_64_big = sizeof test_file_1_64_big;

} // End namespace gold_testsuite.
