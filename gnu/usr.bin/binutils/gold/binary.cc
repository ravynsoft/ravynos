// binary.cc -- binary input files for gold

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
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

#include <cerrno>
#include <cstring>

#include "elfcpp.h"
#include "stringpool.h"
#include "fileread.h"
#include "output.h"
#include "binary.h"

// safe-ctype.h interferes with macros defined by the system <ctype.h>.
// Some C++ system headers might include <ctype.h> and rely on its macro
// definitions being intact.  So make sure that safe-ctype.h is included
// only after any C++ system headers, whether directly here (above) or via
// other local header files (e.g. #include <string> in "binary.h").
#include "safe-ctype.h"

// Support for reading binary files as input.  These become blobs in
// the final output.  These files are treated as though they have a
// single .data section and define three symbols:
// _binary_FILENAME_start, _binary_FILENAME_end, _binary_FILENAME_size.
// The FILENAME is the name of the input file, with any
// non-alphanumeric character changed to an underscore.

// We implement this by creating an ELF file in memory.

namespace gold
{

// class Binary_to_elf.

Binary_to_elf::Binary_to_elf(elfcpp::EM machine, int size, bool big_endian,
			     const std::string& filename)
  : elf_machine_(machine), size_(size), big_endian_(big_endian),
    filename_(filename), data_(NULL), filesize_(0)
{
}

Binary_to_elf::~Binary_to_elf()
{
  if (this->data_ != NULL)
    delete[] this->data_;
}

// Given FILENAME, create a buffer which looks like an ELF file with
// the contents of FILENAME as the contents of the only section.  The
// TASK parameters is mainly for debugging, and records who holds
// locks.

bool
Binary_to_elf::convert(const Task* task)
{
  if (this->size_ == 32)
    {
      if (!this->big_endian_)
	{
#ifdef HAVE_TARGET_32_LITTLE
	  return this->sized_convert<32, false>(task);
#else
	  gold_unreachable();
#endif
	}
      else
	{
#ifdef HAVE_TARGET_32_BIG
	  return this->sized_convert<32, true>(task);
#else
	  gold_unreachable();
#endif
	}
    }
  else if (this->size_ == 64)
    {
      if (!this->big_endian_)
	{
#ifdef HAVE_TARGET_64_LITTLE
	  return this->sized_convert<64, false>(task);
#else
	  gold_unreachable();
#endif
	}
      else
	{
#ifdef HAVE_TARGET_64_BIG
	  return this->sized_convert<64, true>(task);
#else
	  gold_unreachable();
#endif
	}
    }
  else
    gold_unreachable();
}

// We are going to create:
// * The ELF file header.
// * Five sections: null section, .data, .symtab, .strtab, .shstrtab
// * The contents of the file.
// * Four symbols: null, begin, end, size.
// * Three symbol names.
// * Four section names.

template<int size, bool big_endian>
bool
Binary_to_elf::sized_convert(const Task* task)
{
  // Read the input file.

  File_read f;
  if (!f.open(task, this->filename_))
    {
      gold_error(_("cannot open %s: %s:"), this->filename_.c_str(),
		 strerror(errno));
      return false;
    }

  section_size_type filesize = convert_to_section_size_type(f.filesize());
  const unsigned char* fileview;
  if (filesize == 0)
    fileview = NULL;
  else
    fileview = f.get_view(0, 0, filesize, false, false);

  unsigned int align;
  if (size == 32)
    align = 4;
  else if (size == 64)
    align = 8;
  else
    gold_unreachable();
  section_size_type aligned_filesize = align_address(filesize, align);

  // Build the stringpool for the symbol table.

  std::string mangled_name = this->filename_;
  for (std::string::iterator p = mangled_name.begin();
       p != mangled_name.end();
       ++p)
    if (!ISALNUM(*p))
      *p = '_';
  mangled_name = "_binary_" + mangled_name;
  std::string start_symbol_name = mangled_name + "_start";
  std::string end_symbol_name = mangled_name + "_end";
  std::string size_symbol_name = mangled_name + "_size";

  Stringpool strtab;
  strtab.add(start_symbol_name.c_str(), false, NULL);
  strtab.add(end_symbol_name.c_str(), false, NULL);
  strtab.add(size_symbol_name.c_str(), false, NULL);
  strtab.set_string_offsets();

  // Build the stringpool for the section name table.

  Stringpool shstrtab;
  shstrtab.add(".data", false, NULL);
  shstrtab.add(".symtab", false, NULL);
  shstrtab.add(".strtab", false, NULL);
  shstrtab.add(".shstrtab", false, NULL);
  shstrtab.set_string_offsets();

  // Work out the size of the generated file, and the offsets of the
  // various sections, and allocate a buffer.

  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  size_t output_size = (elfcpp::Elf_sizes<size>::ehdr_size
			+ 5 * elfcpp::Elf_sizes<size>::shdr_size);
  size_t data_offset = output_size;
  output_size += aligned_filesize;
  size_t symtab_offset = output_size;
  output_size += 4 * sym_size;
  size_t strtab_offset = output_size;
  output_size += strtab.get_strtab_size();
  size_t shstrtab_offset = output_size;
  output_size += shstrtab.get_strtab_size();

  unsigned char* buffer = new unsigned char[output_size];

  // Write out the data.

  unsigned char* pout = buffer;

  this->write_file_header<size, big_endian>(&pout);

  this->write_section_header<size, big_endian>("", &shstrtab, elfcpp::SHT_NULL,
					       0, 0, 0, 0, 0,
					       0, 0, &pout);
  // Having the section be named ".data", having it be writable, and
  // giving it an alignment of 1 is because the GNU linker does it
  // that way, and existing linker script expect it.
  this->write_section_header<size, big_endian>(".data", &shstrtab,
					       elfcpp::SHT_PROGBITS,
					       (elfcpp::SHF_ALLOC
						| elfcpp::SHF_WRITE),
					       data_offset,
					       filesize, 0, 0,
					       1, 0, &pout);
  this->write_section_header<size, big_endian>(".symtab", &shstrtab,
					       elfcpp::SHT_SYMTAB,
					       0, symtab_offset, 4 * sym_size,
					       3, 1, align, sym_size, &pout);
  this->write_section_header<size, big_endian>(".strtab", &shstrtab,
					       elfcpp::SHT_STRTAB,
					       0, strtab_offset,
					       strtab.get_strtab_size(),
					       0, 0, 1, 0, &pout);
  this->write_section_header<size, big_endian>(".shstrtab", &shstrtab,
					       elfcpp::SHT_STRTAB,
					       0, shstrtab_offset,
					       shstrtab.get_strtab_size(),
					       0, 0, 1, 0, &pout);

  if (filesize > 0)
    {
      memcpy(pout, fileview, filesize);
      pout += filesize;
      memset(pout, 0, aligned_filesize - filesize);
      pout += aligned_filesize - filesize;
    }

  this->write_symbol<size, big_endian>("", &strtab, 0, 0, 0, &pout);
  this->write_symbol<size, big_endian>(start_symbol_name, &strtab, 0, filesize,
				       1, &pout);
  this->write_symbol<size, big_endian>(end_symbol_name, &strtab, filesize, 0,
				       1, &pout);
  this->write_symbol<size, big_endian>(size_symbol_name, &strtab, filesize, 0,
				       elfcpp::SHN_ABS, &pout);

  strtab.write_to_buffer(pout, strtab.get_strtab_size());
  pout += strtab.get_strtab_size();

  shstrtab.write_to_buffer(pout, shstrtab.get_strtab_size());
  pout += shstrtab.get_strtab_size();

  gold_assert(static_cast<size_t>(pout - buffer) == output_size);

  this->data_ = buffer;
  this->filesize_ = output_size;

  f.unlock(task);

  return true;
}

// Write out the file header.

template<int size, bool big_endian>
void
Binary_to_elf::write_file_header(unsigned char** ppout)
{
  elfcpp::Ehdr_write<size, big_endian> oehdr(*ppout);

  unsigned char e_ident[elfcpp::EI_NIDENT];
  memset(e_ident, 0, elfcpp::EI_NIDENT);
  e_ident[elfcpp::EI_MAG0] = elfcpp::ELFMAG0;
  e_ident[elfcpp::EI_MAG1] = elfcpp::ELFMAG1;
  e_ident[elfcpp::EI_MAG2] = elfcpp::ELFMAG2;
  e_ident[elfcpp::EI_MAG3] = elfcpp::ELFMAG3;
  if (size == 32)
    e_ident[elfcpp::EI_CLASS] = elfcpp::ELFCLASS32;
  else if (size == 64)
    e_ident[elfcpp::EI_CLASS] = elfcpp::ELFCLASS64;
  else
    gold_unreachable();
  e_ident[elfcpp::EI_DATA] = (big_endian
			      ? elfcpp::ELFDATA2MSB
			      : elfcpp::ELFDATA2LSB);
  e_ident[elfcpp::EI_VERSION] = elfcpp::EV_CURRENT;
  oehdr.put_e_ident(e_ident);

  oehdr.put_e_type(elfcpp::ET_REL);
  oehdr.put_e_machine(this->elf_machine_);
  oehdr.put_e_version(elfcpp::EV_CURRENT);
  oehdr.put_e_entry(0);
  oehdr.put_e_phoff(0);
  oehdr.put_e_shoff(elfcpp::Elf_sizes<size>::ehdr_size);
  oehdr.put_e_flags(0);
  oehdr.put_e_ehsize(elfcpp::Elf_sizes<size>::ehdr_size);
  oehdr.put_e_phentsize(0);
  oehdr.put_e_phnum(0);
  oehdr.put_e_shentsize(elfcpp::Elf_sizes<size>::shdr_size);
  oehdr.put_e_shnum(5);
  oehdr.put_e_shstrndx(4);

  *ppout += elfcpp::Elf_sizes<size>::ehdr_size;
}

// Write out a section header.

template<int size, bool big_endian>
void
Binary_to_elf::write_section_header(
    const char* name,
    const Stringpool* shstrtab,
    elfcpp::SHT type,
    unsigned int flags,
    section_size_type offset,
    section_size_type section_size,
    unsigned int link,
    unsigned int info,
    unsigned int addralign,
    unsigned int entsize,
    unsigned char** ppout)
{
  elfcpp::Shdr_write<size, big_endian> oshdr(*ppout);

  oshdr.put_sh_name(*name == '\0' ? 0 : shstrtab->get_offset(name));
  oshdr.put_sh_type(type);
  oshdr.put_sh_flags(flags);
  oshdr.put_sh_addr(0);
  oshdr.put_sh_offset(offset);
  oshdr.put_sh_size(section_size);
  oshdr.put_sh_link(link);
  oshdr.put_sh_info(info);
  oshdr.put_sh_addralign(addralign);
  oshdr.put_sh_entsize(entsize);

  *ppout += elfcpp::Elf_sizes<size>::shdr_size;
}

// Write out a symbol.

template<int size, bool big_endian>
void
Binary_to_elf::write_symbol(
    const std::string& name,
    const Stringpool* strtab,
    section_size_type value,
    typename elfcpp::Elf_types<32>::Elf_WXword st_size,
    unsigned int shndx,
    unsigned char** ppout)
{
  unsigned char* pout = *ppout;

  elfcpp::Sym_write<size, big_endian> osym(pout);
  osym.put_st_name(name.empty() ? 0 : strtab->get_offset(name.c_str()));
  osym.put_st_value(value);
  osym.put_st_size(st_size);
  osym.put_st_info(name.empty() ? elfcpp::STB_LOCAL : elfcpp::STB_GLOBAL,
		   elfcpp::STT_NOTYPE);
  osym.put_st_other(elfcpp::STV_DEFAULT, 0);
  osym.put_st_shndx(shndx);

  *ppout += elfcpp::Elf_sizes<size>::sym_size;
}

} // End namespace gold.
