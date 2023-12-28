// output.cc -- manage the output file for gold

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

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include "libiberty.h"

#include "dwarf.h"
#include "parameters.h"
#include "object.h"
#include "symtab.h"
#include "reloc.h"
#include "merge.h"
#include "descriptors.h"
#include "layout.h"
#include "output.h"

// For systems without mmap support.
#ifndef HAVE_MMAP
# define mmap gold_mmap
# define munmap gold_munmap
# define mremap gold_mremap
# ifndef MAP_FAILED
#  define MAP_FAILED (reinterpret_cast<void*>(-1))
# endif
# ifndef PROT_READ
#  define PROT_READ 0
# endif
# ifndef PROT_WRITE
#  define PROT_WRITE 0
# endif
# ifndef MAP_PRIVATE
#  define MAP_PRIVATE 0
# endif
# ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS 0
# endif
# ifndef MAP_SHARED
#  define MAP_SHARED 0
# endif

# ifndef ENOSYS
#  define ENOSYS EINVAL
# endif

static void *
gold_mmap(void *, size_t, int, int, int, off_t)
{
  errno = ENOSYS;
  return MAP_FAILED;
}

static int
gold_munmap(void *, size_t)
{
  errno = ENOSYS;
  return -1;
}

static void *
gold_mremap(void *, size_t, size_t, int)
{
  errno = ENOSYS;
  return MAP_FAILED;
}

#endif

#if defined(HAVE_MMAP) && !defined(HAVE_MREMAP)
# define mremap gold_mremap
extern "C" void *gold_mremap(void *, size_t, size_t, int);
#endif

// Some BSD systems still use MAP_ANON instead of MAP_ANONYMOUS
#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS  MAP_ANON
#endif

#ifndef MREMAP_MAYMOVE
# define MREMAP_MAYMOVE 1
#endif

// Mingw does not have S_ISLNK.
#ifndef S_ISLNK
# define S_ISLNK(mode) 0
#endif

namespace gold
{

// A wrapper around posix_fallocate.  If we don't have posix_fallocate,
// or the --no-posix-fallocate option is set, we try the fallocate
// system call directly.  If that fails, we use ftruncate to set
// the file size and hope that there is enough disk space.

static int
gold_fallocate(int o, off_t offset, off_t len)
{
  if (len <= 0)
    return 0;

#ifdef HAVE_POSIX_FALLOCATE
  if (parameters->options().posix_fallocate())
    {
      int err = ::posix_fallocate(o, offset, len);
      if (err != EINVAL && err != ENOSYS && err != EOPNOTSUPP)
	return err;
    }
#endif // defined(HAVE_POSIX_FALLOCATE)

#ifdef HAVE_FALLOCATE
  {
    errno = 0;
    int err = ::fallocate(o, 0, offset, len);
    if (err < 0 && errno != EINVAL && errno != ENOSYS && errno != EOPNOTSUPP)
      return errno;
  }
#endif // defined(HAVE_FALLOCATE)

  errno = 0;
  if (::ftruncate(o, offset + len) < 0)
    return errno;
  return 0;
}

// Output_data variables.

bool Output_data::allocated_sizes_are_fixed;

// Output_data methods.

Output_data::~Output_data()
{
}

// Return the default alignment for the target size.

uint64_t
Output_data::default_alignment()
{
  return Output_data::default_alignment_for_size(
      parameters->target().get_size());
}

// Return the default alignment for a size--32 or 64.

uint64_t
Output_data::default_alignment_for_size(int size)
{
  if (size == 32)
    return 4;
  else if (size == 64)
    return 8;
  else
    gold_unreachable();
}

// Output_section_header methods.  This currently assumes that the
// segment and section lists are complete at construction time.

Output_section_headers::Output_section_headers(
    const Layout* layout,
    const Layout::Segment_list* segment_list,
    const Layout::Section_list* section_list,
    const Layout::Section_list* unattached_section_list,
    const Stringpool* secnamepool,
    const Output_section* shstrtab_section)
  : layout_(layout),
    segment_list_(segment_list),
    section_list_(section_list),
    unattached_section_list_(unattached_section_list),
    secnamepool_(secnamepool),
    shstrtab_section_(shstrtab_section)
{
}

// Compute the current data size.

off_t
Output_section_headers::do_size() const
{
  // Count all the sections.  Start with 1 for the null section.
  off_t count = 1;
  if (!parameters->options().relocatable())
    {
      for (Layout::Segment_list::const_iterator p =
	     this->segment_list_->begin();
	   p != this->segment_list_->end();
	   ++p)
	if ((*p)->type() == elfcpp::PT_LOAD)
	  count += (*p)->output_section_count();
    }
  else
    {
      for (Layout::Section_list::const_iterator p =
	     this->section_list_->begin();
	   p != this->section_list_->end();
	   ++p)
	if (((*p)->flags() & elfcpp::SHF_ALLOC) != 0)
	  ++count;
    }
  count += this->unattached_section_list_->size();

  const int size = parameters->target().get_size();
  int shdr_size;
  if (size == 32)
    shdr_size = elfcpp::Elf_sizes<32>::shdr_size;
  else if (size == 64)
    shdr_size = elfcpp::Elf_sizes<64>::shdr_size;
  else
    gold_unreachable();

  return count * shdr_size;
}

// Write out the section headers.

void
Output_section_headers::do_write(Output_file* of)
{
  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      this->do_sized_write<32, false>(of);
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      this->do_sized_write<32, true>(of);
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      this->do_sized_write<64, false>(of);
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      this->do_sized_write<64, true>(of);
      break;
#endif
    default:
      gold_unreachable();
    }
}

template<int size, bool big_endian>
void
Output_section_headers::do_sized_write(Output_file* of)
{
  off_t all_shdrs_size = this->data_size();
  unsigned char* view = of->get_output_view(this->offset(), all_shdrs_size);

  const int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;
  unsigned char* v = view;

  {
    typename elfcpp::Shdr_write<size, big_endian> oshdr(v);
    oshdr.put_sh_name(0);
    oshdr.put_sh_type(elfcpp::SHT_NULL);
    oshdr.put_sh_flags(0);
    oshdr.put_sh_addr(0);
    oshdr.put_sh_offset(0);

    size_t section_count = (this->data_size()
			    / elfcpp::Elf_sizes<size>::shdr_size);
    if (section_count < elfcpp::SHN_LORESERVE)
      oshdr.put_sh_size(0);
    else
      oshdr.put_sh_size(section_count);

    unsigned int shstrndx = this->shstrtab_section_->out_shndx();
    if (shstrndx < elfcpp::SHN_LORESERVE)
      oshdr.put_sh_link(0);
    else
      oshdr.put_sh_link(shstrndx);

    size_t segment_count = this->segment_list_->size();
    oshdr.put_sh_info(segment_count >= elfcpp::PN_XNUM ? segment_count : 0);

    oshdr.put_sh_addralign(0);
    oshdr.put_sh_entsize(0);
  }

  v += shdr_size;

  unsigned int shndx = 1;
  if (!parameters->options().relocatable())
    {
      for (Layout::Segment_list::const_iterator p =
	     this->segment_list_->begin();
	   p != this->segment_list_->end();
	   ++p)
	v = (*p)->write_section_headers<size, big_endian>(this->layout_,
							  this->secnamepool_,
							  v,
							  &shndx);
    }
  else
    {
      for (Layout::Section_list::const_iterator p =
	     this->section_list_->begin();
	   p != this->section_list_->end();
	   ++p)
	{
	  // We do unallocated sections below, except that group
	  // sections have to come first.
	  if (((*p)->flags() & elfcpp::SHF_ALLOC) == 0
	      && (*p)->type() != elfcpp::SHT_GROUP)
	    continue;
	  gold_assert(shndx == (*p)->out_shndx());
	  elfcpp::Shdr_write<size, big_endian> oshdr(v);
	  (*p)->write_header(this->layout_, this->secnamepool_, &oshdr);
	  v += shdr_size;
	  ++shndx;
	}
    }

  for (Layout::Section_list::const_iterator p =
	 this->unattached_section_list_->begin();
       p != this->unattached_section_list_->end();
       ++p)
    {
      // For a relocatable link, we did unallocated group sections
      // above, since they have to come first.
      if ((*p)->type() == elfcpp::SHT_GROUP
	  && parameters->options().relocatable())
	continue;
      gold_assert(shndx == (*p)->out_shndx());
      elfcpp::Shdr_write<size, big_endian> oshdr(v);
      (*p)->write_header(this->layout_, this->secnamepool_, &oshdr);
      v += shdr_size;
      ++shndx;
    }

  of->write_output_view(this->offset(), all_shdrs_size, view);
}

// Output_segment_header methods.

Output_segment_headers::Output_segment_headers(
    const Layout::Segment_list& segment_list)
  : segment_list_(segment_list)
{
  this->set_current_data_size_for_child(this->do_size());
}

void
Output_segment_headers::do_write(Output_file* of)
{
  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      this->do_sized_write<32, false>(of);
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      this->do_sized_write<32, true>(of);
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      this->do_sized_write<64, false>(of);
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      this->do_sized_write<64, true>(of);
      break;
#endif
    default:
      gold_unreachable();
    }
}

template<int size, bool big_endian>
void
Output_segment_headers::do_sized_write(Output_file* of)
{
  const int phdr_size = elfcpp::Elf_sizes<size>::phdr_size;
  off_t all_phdrs_size = this->segment_list_.size() * phdr_size;
  gold_assert(all_phdrs_size == this->data_size());
  unsigned char* view = of->get_output_view(this->offset(),
					    all_phdrs_size);
  unsigned char* v = view;
  for (Layout::Segment_list::const_iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    {
      elfcpp::Phdr_write<size, big_endian> ophdr(v);
      (*p)->write_header(&ophdr);
      v += phdr_size;
    }

  gold_assert(v - view == all_phdrs_size);

  of->write_output_view(this->offset(), all_phdrs_size, view);
}

off_t
Output_segment_headers::do_size() const
{
  const int size = parameters->target().get_size();
  int phdr_size;
  if (size == 32)
    phdr_size = elfcpp::Elf_sizes<32>::phdr_size;
  else if (size == 64)
    phdr_size = elfcpp::Elf_sizes<64>::phdr_size;
  else
    gold_unreachable();

  return this->segment_list_.size() * phdr_size;
}

// Output_file_header methods.

Output_file_header::Output_file_header(Target* target,
				       const Symbol_table* symtab,
				       const Output_segment_headers* osh)
  : target_(target),
    symtab_(symtab),
    segment_header_(osh),
    section_header_(NULL),
    shstrtab_(NULL)
{
  this->set_data_size(this->do_size());
}

// Set the section table information for a file header.

void
Output_file_header::set_section_info(const Output_section_headers* shdrs,
				     const Output_section* shstrtab)
{
  this->section_header_ = shdrs;
  this->shstrtab_ = shstrtab;
}

// Write out the file header.

void
Output_file_header::do_write(Output_file* of)
{
  gold_assert(this->offset() == 0);

  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      this->do_sized_write<32, false>(of);
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      this->do_sized_write<32, true>(of);
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      this->do_sized_write<64, false>(of);
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      this->do_sized_write<64, true>(of);
      break;
#endif
    default:
      gold_unreachable();
    }
}

// Write out the file header with appropriate size and endianness.

template<int size, bool big_endian>
void
Output_file_header::do_sized_write(Output_file* of)
{
  gold_assert(this->offset() == 0);

  int ehdr_size = elfcpp::Elf_sizes<size>::ehdr_size;
  unsigned char* view = of->get_output_view(0, ehdr_size);
  elfcpp::Ehdr_write<size, big_endian> oehdr(view);

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

  elfcpp::ET e_type;
  if (parameters->options().relocatable())
    e_type = elfcpp::ET_REL;
  else if (parameters->options().output_is_position_independent())
    e_type = elfcpp::ET_DYN;
  else
    e_type = elfcpp::ET_EXEC;
  oehdr.put_e_type(e_type);

  oehdr.put_e_machine(this->target_->machine_code());
  oehdr.put_e_version(elfcpp::EV_CURRENT);

  oehdr.put_e_entry(this->entry<size>());

  if (this->segment_header_ == NULL)
    oehdr.put_e_phoff(0);
  else
    oehdr.put_e_phoff(this->segment_header_->offset());

  oehdr.put_e_shoff(this->section_header_->offset());
  oehdr.put_e_flags(this->target_->processor_specific_flags());
  oehdr.put_e_ehsize(elfcpp::Elf_sizes<size>::ehdr_size);

  if (this->segment_header_ == NULL)
    {
      oehdr.put_e_phentsize(0);
      oehdr.put_e_phnum(0);
    }
  else
    {
      oehdr.put_e_phentsize(elfcpp::Elf_sizes<size>::phdr_size);
      size_t phnum = (this->segment_header_->data_size()
		      / elfcpp::Elf_sizes<size>::phdr_size);
      if (phnum > elfcpp::PN_XNUM)
	phnum = elfcpp::PN_XNUM;
      oehdr.put_e_phnum(phnum);
    }

  oehdr.put_e_shentsize(elfcpp::Elf_sizes<size>::shdr_size);
  size_t section_count = (this->section_header_->data_size()
			  / elfcpp::Elf_sizes<size>::shdr_size);

  if (section_count < elfcpp::SHN_LORESERVE)
    oehdr.put_e_shnum(this->section_header_->data_size()
		      / elfcpp::Elf_sizes<size>::shdr_size);
  else
    oehdr.put_e_shnum(0);

  unsigned int shstrndx = this->shstrtab_->out_shndx();
  if (shstrndx < elfcpp::SHN_LORESERVE)
    oehdr.put_e_shstrndx(this->shstrtab_->out_shndx());
  else
    oehdr.put_e_shstrndx(elfcpp::SHN_XINDEX);

  // Let the target adjust the ELF header, e.g., to set EI_OSABI in
  // the e_ident field.
  this->target_->adjust_elf_header(view, ehdr_size);

  of->write_output_view(0, ehdr_size, view);
}

// Return the value to use for the entry address.

template<int size>
typename elfcpp::Elf_types<size>::Elf_Addr
Output_file_header::entry()
{
  const bool should_issue_warning = (parameters->options().entry() != NULL
				     && !parameters->options().relocatable()
				     && !parameters->options().shared());
  const char* entry = parameters->entry();
  Symbol* sym = this->symtab_->lookup(entry);

  typename Sized_symbol<size>::Value_type v;
  if (sym != NULL)
    {
      Sized_symbol<size>* ssym;
      ssym = this->symtab_->get_sized_symbol<size>(sym);
      if (!ssym->is_defined() && should_issue_warning)
	gold_warning("entry symbol '%s' exists but is not defined", entry);
      v = ssym->value();
    }
  else
    {
      // We couldn't find the entry symbol.  See if we can parse it as
      // a number.  This supports, e.g., -e 0x1000.
      char* endptr;
      v = strtoull(entry, &endptr, 0);
      if (*endptr != '\0')
	{
	  if (should_issue_warning)
	    gold_warning("cannot find entry symbol '%s'", entry);
	  v = 0;
	}
    }

  return v;
}

// Compute the current data size.

off_t
Output_file_header::do_size() const
{
  const int size = parameters->target().get_size();
  if (size == 32)
    return elfcpp::Elf_sizes<32>::ehdr_size;
  else if (size == 64)
    return elfcpp::Elf_sizes<64>::ehdr_size;
  else
    gold_unreachable();
}

// Output_data_const methods.

void
Output_data_const::do_write(Output_file* of)
{
  of->write(this->offset(), this->data_.data(), this->data_.size());
}

// Output_data_const_buffer methods.

void
Output_data_const_buffer::do_write(Output_file* of)
{
  of->write(this->offset(), this->p_, this->data_size());
}

// Output_section_data methods.

// Record the output section, and set the entry size and such.

void
Output_section_data::set_output_section(Output_section* os)
{
  gold_assert(this->output_section_ == NULL);
  this->output_section_ = os;
  this->do_adjust_output_section(os);
}

// Return the section index of the output section.

unsigned int
Output_section_data::do_out_shndx() const
{
  gold_assert(this->output_section_ != NULL);
  return this->output_section_->out_shndx();
}

// Set the alignment, which means we may need to update the alignment
// of the output section.

void
Output_section_data::set_addralign(uint64_t addralign)
{
  this->addralign_ = addralign;
  if (this->output_section_ != NULL
      && this->output_section_->addralign() < addralign)
    this->output_section_->set_addralign(addralign);
}

// Output_data_strtab methods.

// Set the final data size.

void
Output_data_strtab::set_final_data_size()
{
  this->strtab_->set_string_offsets();
  this->set_data_size(this->strtab_->get_strtab_size());
}

// Write out a string table.

void
Output_data_strtab::do_write(Output_file* of)
{
  this->strtab_->write(of, this->offset());
}

// Output_reloc methods.

// A reloc against a global symbol.

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    Symbol* gsym,
    unsigned int type,
    Output_data* od,
    Address address,
    bool is_relative,
    bool is_symbolless,
    bool use_plt_offset)
  : address_(address), local_sym_index_(GSYM_CODE), type_(type),
    is_relative_(is_relative), is_symbolless_(is_symbolless),
    is_section_symbol_(false), use_plt_offset_(use_plt_offset), shndx_(INVALID_CODE)
{
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.gsym = gsym;
  this->u2_.od = od;
  if (dynamic)
    this->set_needs_dynsym_index();
}

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    Symbol* gsym,
    unsigned int type,
    Sized_relobj<size, big_endian>* relobj,
    unsigned int shndx,
    Address address,
    bool is_relative,
    bool is_symbolless,
    bool use_plt_offset)
  : address_(address), local_sym_index_(GSYM_CODE), type_(type),
    is_relative_(is_relative), is_symbolless_(is_symbolless),
    is_section_symbol_(false), use_plt_offset_(use_plt_offset), shndx_(shndx)
{
  gold_assert(shndx != INVALID_CODE);
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.gsym = gsym;
  this->u2_.relobj = relobj;
  if (dynamic)
    this->set_needs_dynsym_index();
}

// A reloc against a local symbol.

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    Sized_relobj<size, big_endian>* relobj,
    unsigned int local_sym_index,
    unsigned int type,
    Output_data* od,
    Address address,
    bool is_relative,
    bool is_symbolless,
    bool is_section_symbol,
    bool use_plt_offset)
  : address_(address), local_sym_index_(local_sym_index), type_(type),
    is_relative_(is_relative), is_symbolless_(is_symbolless),
    is_section_symbol_(is_section_symbol), use_plt_offset_(use_plt_offset),
    shndx_(INVALID_CODE)
{
  gold_assert(local_sym_index != GSYM_CODE
	      && local_sym_index != INVALID_CODE);
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.relobj = relobj;
  this->u2_.od = od;
  if (dynamic)
    this->set_needs_dynsym_index();
}

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    Sized_relobj<size, big_endian>* relobj,
    unsigned int local_sym_index,
    unsigned int type,
    unsigned int shndx,
    Address address,
    bool is_relative,
    bool is_symbolless,
    bool is_section_symbol,
    bool use_plt_offset)
  : address_(address), local_sym_index_(local_sym_index), type_(type),
    is_relative_(is_relative), is_symbolless_(is_symbolless),
    is_section_symbol_(is_section_symbol), use_plt_offset_(use_plt_offset),
    shndx_(shndx)
{
  gold_assert(local_sym_index != GSYM_CODE
	      && local_sym_index != INVALID_CODE);
  gold_assert(shndx != INVALID_CODE);
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.relobj = relobj;
  this->u2_.relobj = relobj;
  if (dynamic)
    this->set_needs_dynsym_index();
}

// A reloc against the STT_SECTION symbol of an output section.

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    Output_section* os,
    unsigned int type,
    Output_data* od,
    Address address,
    bool is_relative)
  : address_(address), local_sym_index_(SECTION_CODE), type_(type),
    is_relative_(is_relative), is_symbolless_(is_relative),
    is_section_symbol_(true), use_plt_offset_(false), shndx_(INVALID_CODE)
{
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.os = os;
  this->u2_.od = od;
  if (dynamic)
    this->set_needs_dynsym_index();
  else
    os->set_needs_symtab_index();
}

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    Output_section* os,
    unsigned int type,
    Sized_relobj<size, big_endian>* relobj,
    unsigned int shndx,
    Address address,
    bool is_relative)
  : address_(address), local_sym_index_(SECTION_CODE), type_(type),
    is_relative_(is_relative), is_symbolless_(is_relative),
    is_section_symbol_(true), use_plt_offset_(false), shndx_(shndx)
{
  gold_assert(shndx != INVALID_CODE);
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.os = os;
  this->u2_.relobj = relobj;
  if (dynamic)
    this->set_needs_dynsym_index();
  else
    os->set_needs_symtab_index();
}

// An absolute or relative relocation.

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    unsigned int type,
    Output_data* od,
    Address address,
    bool is_relative)
  : address_(address), local_sym_index_(0), type_(type),
    is_relative_(is_relative), is_symbolless_(false),
    is_section_symbol_(false), use_plt_offset_(false), shndx_(INVALID_CODE)
{
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.relobj = NULL;
  this->u2_.od = od;
}

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    unsigned int type,
    Sized_relobj<size, big_endian>* relobj,
    unsigned int shndx,
    Address address,
    bool is_relative)
  : address_(address), local_sym_index_(0), type_(type),
    is_relative_(is_relative), is_symbolless_(false),
    is_section_symbol_(false), use_plt_offset_(false), shndx_(shndx)
{
  gold_assert(shndx != INVALID_CODE);
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.relobj = NULL;
  this->u2_.relobj = relobj;
}

// A target specific relocation.

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    unsigned int type,
    void* arg,
    Output_data* od,
    Address address)
  : address_(address), local_sym_index_(TARGET_CODE), type_(type),
    is_relative_(false), is_symbolless_(false),
    is_section_symbol_(false), use_plt_offset_(false), shndx_(INVALID_CODE)
{
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.arg = arg;
  this->u2_.od = od;
}

template<bool dynamic, int size, bool big_endian>
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::Output_reloc(
    unsigned int type,
    void* arg,
    Sized_relobj<size, big_endian>* relobj,
    unsigned int shndx,
    Address address)
  : address_(address), local_sym_index_(TARGET_CODE), type_(type),
    is_relative_(false), is_symbolless_(false),
    is_section_symbol_(false), use_plt_offset_(false), shndx_(shndx)
{
  gold_assert(shndx != INVALID_CODE);
  // this->type_ is a bitfield; make sure TYPE fits.
  gold_assert(this->type_ == type);
  this->u1_.arg = arg;
  this->u2_.relobj = relobj;
}

// Record that we need a dynamic symbol index for this relocation.

template<bool dynamic, int size, bool big_endian>
void
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::
set_needs_dynsym_index()
{
  if (this->is_symbolless_)
    return;
  switch (this->local_sym_index_)
    {
    case INVALID_CODE:
      gold_unreachable();

    case GSYM_CODE:
      this->u1_.gsym->set_needs_dynsym_entry();
      break;

    case SECTION_CODE:
      this->u1_.os->set_needs_dynsym_index();
      break;

    case TARGET_CODE:
      // The target must take care of this if necessary.
      break;

    case 0:
      break;

    default:
      {
	const unsigned int lsi = this->local_sym_index_;
	Sized_relobj_file<size, big_endian>* relobj =
	    this->u1_.relobj->sized_relobj();
	gold_assert(relobj != NULL);
	if (!this->is_section_symbol_)
	  relobj->set_needs_output_dynsym_entry(lsi);
	else
	  relobj->output_section(lsi)->set_needs_dynsym_index();
      }
      break;
    }
}

// Get the symbol index of a relocation.

template<bool dynamic, int size, bool big_endian>
unsigned int
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::get_symbol_index()
  const
{
  unsigned int index;
  if (this->is_symbolless_)
    return 0;
  switch (this->local_sym_index_)
    {
    case INVALID_CODE:
      gold_unreachable();

    case GSYM_CODE:
      if (this->u1_.gsym == NULL)
	index = 0;
      else if (dynamic)
	index = this->u1_.gsym->dynsym_index();
      else
	index = this->u1_.gsym->symtab_index();
      break;

    case SECTION_CODE:
      if (dynamic)
	index = this->u1_.os->dynsym_index();
      else
	index = this->u1_.os->symtab_index();
      break;

    case TARGET_CODE:
      index = parameters->target().reloc_symbol_index(this->u1_.arg,
						      this->type_);
      break;

    case 0:
      // Relocations without symbols use a symbol index of 0.
      index = 0;
      break;

    default:
      {
	const unsigned int lsi = this->local_sym_index_;
	Sized_relobj_file<size, big_endian>* relobj =
	    this->u1_.relobj->sized_relobj();
	gold_assert(relobj != NULL);
	if (!this->is_section_symbol_)
	  {
	    if (dynamic)
	      index = relobj->dynsym_index(lsi);
	    else
	      index = relobj->symtab_index(lsi);
	  }
	else
	  {
	    Output_section* os = relobj->output_section(lsi);
	    gold_assert(os != NULL);
	    if (dynamic)
	      index = os->dynsym_index();
	    else
	      index = os->symtab_index();
	  }
      }
      break;
    }
  gold_assert(index != -1U);
  return index;
}

// For a local section symbol, get the address of the offset ADDEND
// within the input section.

template<bool dynamic, int size, bool big_endian>
typename elfcpp::Elf_types<size>::Elf_Addr
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::
  local_section_offset(Addend addend) const
{
  gold_assert(this->local_sym_index_ != GSYM_CODE
	      && this->local_sym_index_ != SECTION_CODE
	      && this->local_sym_index_ != TARGET_CODE
	      && this->local_sym_index_ != INVALID_CODE
	      && this->local_sym_index_ != 0
	      && this->is_section_symbol_);
  const unsigned int lsi = this->local_sym_index_;
  Output_section* os = this->u1_.relobj->output_section(lsi);
  gold_assert(os != NULL);
  Address offset = this->u1_.relobj->get_output_section_offset(lsi);
  if (offset != invalid_address)
    return offset + addend;
  // This is a merge section.
  Sized_relobj_file<size, big_endian>* relobj =
      this->u1_.relobj->sized_relobj();
  gold_assert(relobj != NULL);
  offset = os->output_address(relobj, lsi, addend);
  gold_assert(offset != invalid_address);
  return offset;
}

// Get the output address of a relocation.

template<bool dynamic, int size, bool big_endian>
typename elfcpp::Elf_types<size>::Elf_Addr
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::get_address() const
{
  Address address = this->address_;
  if (this->shndx_ != INVALID_CODE)
    {
      Output_section* os = this->u2_.relobj->output_section(this->shndx_);
      gold_assert(os != NULL);
      Address off = this->u2_.relobj->get_output_section_offset(this->shndx_);
      if (off != invalid_address)
	address += os->address() + off;
      else
	{
	  Sized_relobj_file<size, big_endian>* relobj =
	      this->u2_.relobj->sized_relobj();
	  gold_assert(relobj != NULL);
	  address = os->output_address(relobj, this->shndx_, address);
	  gold_assert(address != invalid_address);
	}
    }
  else if (this->u2_.od != NULL)
    address += this->u2_.od->address();
  return address;
}

// Write out the offset and info fields of a Rel or Rela relocation
// entry.

template<bool dynamic, int size, bool big_endian>
template<typename Write_rel>
void
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::write_rel(
    Write_rel* wr) const
{
  wr->put_r_offset(this->get_address());
  unsigned int sym_index = this->get_symbol_index();
  wr->put_r_info(elfcpp::elf_r_info<size>(sym_index, this->type_));
}

// Write out a Rel relocation.

template<bool dynamic, int size, bool big_endian>
void
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::write(
    unsigned char* pov) const
{
  elfcpp::Rel_write<size, big_endian> orel(pov);
  this->write_rel(&orel);
}

// Get the value of the symbol referred to by a Rel relocation.

template<bool dynamic, int size, bool big_endian>
typename elfcpp::Elf_types<size>::Elf_Addr
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::symbol_value(
    Addend addend) const
{
  if (this->local_sym_index_ == GSYM_CODE)
    {
      const Sized_symbol<size>* sym;
      sym = static_cast<const Sized_symbol<size>*>(this->u1_.gsym);
      if (this->use_plt_offset_ && sym->has_plt_offset())
	return parameters->target().plt_address_for_global(sym);
      else
	return sym->value() + addend;
    }
  if (this->local_sym_index_ == SECTION_CODE)
    {
      gold_assert(!this->use_plt_offset_);
      return this->u1_.os->address() + addend;
    }
  gold_assert(this->local_sym_index_ != TARGET_CODE
	      && this->local_sym_index_ != INVALID_CODE
	      && this->local_sym_index_ != 0
	      && !this->is_section_symbol_);
  const unsigned int lsi = this->local_sym_index_;
  Sized_relobj_file<size, big_endian>* relobj =
      this->u1_.relobj->sized_relobj();
  gold_assert(relobj != NULL);
  if (this->use_plt_offset_)
    return parameters->target().plt_address_for_local(relobj, lsi);
  const Symbol_value<size>* symval = relobj->local_symbol(lsi);
  return symval->value(relobj, addend);
}

// Reloc comparison.  This function sorts the dynamic relocs for the
// benefit of the dynamic linker.  First we sort all relative relocs
// to the front.  Among relative relocs, we sort by output address.
// Among non-relative relocs, we sort by symbol index, then by output
// address.

template<bool dynamic, int size, bool big_endian>
int
Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>::
  compare(const Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>& r2)
    const
{
  if (this->is_relative_)
    {
      if (!r2.is_relative_)
	return -1;
      // Otherwise sort by reloc address below.
    }
  else if (r2.is_relative_)
    return 1;
  else
    {
      unsigned int sym1 = this->get_symbol_index();
      unsigned int sym2 = r2.get_symbol_index();
      if (sym1 < sym2)
	return -1;
      else if (sym1 > sym2)
	return 1;
      // Otherwise sort by reloc address.
    }

  section_offset_type addr1 = this->get_address();
  section_offset_type addr2 = r2.get_address();
  if (addr1 < addr2)
    return -1;
  else if (addr1 > addr2)
    return 1;

  // Final tie breaker, in order to generate the same output on any
  // host: reloc type.
  unsigned int type1 = this->type_;
  unsigned int type2 = r2.type_;
  if (type1 < type2)
    return -1;
  else if (type1 > type2)
    return 1;

  // These relocs appear to be exactly the same.
  return 0;
}

// Write out a Rela relocation.

template<bool dynamic, int size, bool big_endian>
void
Output_reloc<elfcpp::SHT_RELA, dynamic, size, big_endian>::write(
    unsigned char* pov) const
{
  elfcpp::Rela_write<size, big_endian> orel(pov);
  this->rel_.write_rel(&orel);
  Addend addend = this->addend_;
  if (this->rel_.is_target_specific())
    addend = parameters->target().reloc_addend(this->rel_.target_arg(),
					       this->rel_.type(), addend);
  else if (this->rel_.is_symbolless())
    addend = this->rel_.symbol_value(addend);
  else if (this->rel_.is_local_section_symbol())
    addend = this->rel_.local_section_offset(addend);
  orel.put_r_addend(addend);
}

// Output_data_reloc_base methods.

// Adjust the output section.

template<int sh_type, bool dynamic, int size, bool big_endian>
void
Output_data_reloc_base<sh_type, dynamic, size, big_endian>
    ::do_adjust_output_section(Output_section* os)
{
  if (sh_type == elfcpp::SHT_REL)
    os->set_entsize(elfcpp::Elf_sizes<size>::rel_size);
  else if (sh_type == elfcpp::SHT_RELA)
    os->set_entsize(elfcpp::Elf_sizes<size>::rela_size);
  else
    gold_unreachable();

  // A STT_GNU_IFUNC symbol may require a IRELATIVE reloc when doing a
  // static link.  The backends will generate a dynamic reloc section
  // to hold this.  In that case we don't want to link to the dynsym
  // section, because there isn't one.
  if (!dynamic)
    os->set_should_link_to_symtab();
  else if (parameters->doing_static_link())
    ;
  else
    os->set_should_link_to_dynsym();
}

// Standard relocation writer, which just calls Output_reloc::write().

template<int sh_type, bool dynamic, int size, bool big_endian>
struct Output_reloc_writer
{
  typedef Output_reloc<sh_type, dynamic, size, big_endian> Output_reloc_type;
  typedef std::vector<Output_reloc_type> Relocs;

  static void
  write(typename Relocs::const_iterator p, unsigned char* pov)
  { p->write(pov); }
};

// Write out relocation data.

template<int sh_type, bool dynamic, int size, bool big_endian>
void
Output_data_reloc_base<sh_type, dynamic, size, big_endian>::do_write(
    Output_file* of)
{
  typedef Output_reloc_writer<sh_type, dynamic, size, big_endian> Writer;
  this->do_write_generic<Writer>(of);
}

// Class Output_relocatable_relocs.

template<int sh_type, int size, bool big_endian>
void
Output_relocatable_relocs<sh_type, size, big_endian>::set_final_data_size()
{
  this->set_data_size(this->rr_->output_reloc_count()
		      * Reloc_types<sh_type, size, big_endian>::reloc_size);
}

// class Output_data_group.

template<int size, bool big_endian>
Output_data_group<size, big_endian>::Output_data_group(
    Sized_relobj_file<size, big_endian>* relobj,
    section_size_type entry_count,
    elfcpp::Elf_Word flags,
    std::vector<unsigned int>* input_shndxes)
  : Output_section_data(entry_count * 4, 4, false),
    relobj_(relobj),
    flags_(flags)
{
  this->input_shndxes_.swap(*input_shndxes);
}

// Write out the section group, which means translating the section
// indexes to apply to the output file.

template<int size, bool big_endian>
void
Output_data_group<size, big_endian>::do_write(Output_file* of)
{
  const off_t off = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(off, oview_size);

  elfcpp::Elf_Word* contents = reinterpret_cast<elfcpp::Elf_Word*>(oview);
  elfcpp::Swap<32, big_endian>::writeval(contents, this->flags_);
  ++contents;

  for (std::vector<unsigned int>::const_iterator p =
	 this->input_shndxes_.begin();
       p != this->input_shndxes_.end();
       ++p, ++contents)
    {
      Output_section* os = this->relobj_->output_section(*p);

      unsigned int output_shndx;
      if (os != NULL)
	output_shndx = os->out_shndx();
      else
	{
	  this->relobj_->error(_("section group retained but "
				 "group element discarded"));
	  output_shndx = 0;
	}

      elfcpp::Swap<32, big_endian>::writeval(contents, output_shndx);
    }

  size_t wrote = reinterpret_cast<unsigned char*>(contents) - oview;
  gold_assert(wrote == oview_size);

  of->write_output_view(off, oview_size, oview);

  // We no longer need this information.
  this->input_shndxes_.clear();
}

// Output_data_got::Got_entry methods.

// Write out the entry.

template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::Got_entry::write(
    Output_data_got_base* got,
    unsigned int got_indx,
    unsigned char* pov) const
{
  Valtype val = 0;

  switch (this->local_sym_index_)
    {
    case GSYM_CODE:
      {
	// If the symbol is resolved locally, we need to write out the
	// link-time value, which will be relocated dynamically by a
	// RELATIVE relocation.
	Symbol* gsym = this->u_.gsym;
	if (this->use_plt_or_tls_offset_ && gsym->has_plt_offset())
	  val = parameters->target().plt_address_for_global(gsym);
	else
	  {
	    switch (parameters->size_and_endianness())
	      {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
	      case Parameters::TARGET_32_LITTLE:
	      case Parameters::TARGET_32_BIG:
		{
		  // This cast is ugly.  We don't want to put a
		  // virtual method in Symbol, because we want Symbol
		  // to be as small as possible.
		  Sized_symbol<32>::Value_type v;
		  v = static_cast<Sized_symbol<32>*>(gsym)->value();
		  val = convert_types<Valtype, Sized_symbol<32>::Value_type>(v);
		}
		break;
#endif
#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
	      case Parameters::TARGET_64_LITTLE:
	      case Parameters::TARGET_64_BIG:
		{
		  Sized_symbol<64>::Value_type v;
		  v = static_cast<Sized_symbol<64>*>(gsym)->value();
		  val = convert_types<Valtype, Sized_symbol<64>::Value_type>(v);
		}
		break;
#endif
	      default:
		gold_unreachable();
	      }
	    // If this is a GOT entry for a known value global symbol,
	    // then the value should include the addend.  If the value
	    // is not known leave the value as zero; The GOT entry
	    // will be set by a dynamic relocation.
	    if (this->addend_ && gsym->final_value_is_known())
	      val += this->addend_;
	    if (this->use_plt_or_tls_offset_
		&& gsym->type() == elfcpp::STT_TLS)
	      val += parameters->target().tls_offset_for_global(gsym,
								got, got_indx,
								this->addend_);
	  }
      }
      break;

    case CONSTANT_CODE:
      val = this->u_.constant;
      break;

    case RESERVED_CODE:
      // If we're doing an incremental update, don't touch this GOT entry.
      if (parameters->incremental_update())
	return;
      val = this->u_.constant;
      break;

    default:
      {
	const Relobj* object = this->u_.object;
	const unsigned int lsi = this->local_sym_index_;
	bool is_tls = object->local_is_tls(lsi);
	if (this->use_plt_or_tls_offset_ && !is_tls)
	  val = parameters->target().plt_address_for_local(object, lsi);
	else
	  {
	    uint64_t lval = object->local_symbol_value(lsi, this->addend_);
	    val = convert_types<Valtype, uint64_t>(lval);
	    if (this->use_plt_or_tls_offset_ && is_tls)
	      val += parameters->target().tls_offset_for_local(object, lsi,
							       got, got_indx,
							       this->addend_);
	  }
      }
      break;
    }

  elfcpp::Swap<got_size, big_endian>::writeval(pov, val);
}

// Output_data_got methods.

// Add an entry for a global symbol to the GOT.  This returns true if
// this is a new GOT entry, false if the symbol already had a GOT
// entry.

template<int got_size, bool big_endian>
bool
Output_data_got<got_size, big_endian>::add_global(Symbol* gsym,
						  unsigned int got_type,
						  uint64_t addend)
{
  if (gsym->has_got_offset(got_type, addend))
    return false;

  unsigned int got_offset = this->add_got_entry(Got_entry(gsym, false, addend));
  gsym->set_got_offset(got_type, got_offset, addend);
  return true;
}

// Like add_global, but use the PLT offset.

template<int got_size, bool big_endian>
bool
Output_data_got<got_size, big_endian>::add_global_plt(Symbol* gsym,
						      unsigned int got_type,
						      uint64_t addend)
{
  if (gsym->has_got_offset(got_type, addend))
    return false;

  unsigned int got_offset = this->add_got_entry(Got_entry(gsym, true, addend));
  gsym->set_got_offset(got_type, got_offset, addend);
  return true;
}

// Add an entry for a global symbol to the GOT, and add a dynamic
// relocation of type R_TYPE for the GOT entry.

template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::add_global_with_rel(
    Symbol* gsym,
    unsigned int got_type,
    Output_data_reloc_generic* rel_dyn,
    unsigned int r_type,
    uint64_t addend)
{
  if (gsym->has_got_offset(got_type, addend))
    return;

  unsigned int got_offset = this->add_got_entry(Got_entry());
  gsym->set_got_offset(got_type, got_offset, addend);
  rel_dyn->add_global_generic(gsym, r_type, this, got_offset, addend);
}

// Add a pair of entries for a global symbol to the GOT, and add
// dynamic relocations of type R_TYPE_1 and R_TYPE_2, respectively.
// If R_TYPE_2 == 0, add the second entry with no relocation.
template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::add_global_pair_with_rel(
    Symbol* gsym,
    unsigned int got_type,
    Output_data_reloc_generic* rel_dyn,
    unsigned int r_type_1,
    unsigned int r_type_2,
    uint64_t addend)
{
  if (gsym->has_got_offset(got_type, addend))
    return;

  unsigned int got_offset = this->add_got_entry_pair(Got_entry(), Got_entry());
  gsym->set_got_offset(got_type, got_offset, addend);
  rel_dyn->add_global_generic(gsym, r_type_1, this, got_offset, addend);

  if (r_type_2 != 0)
    rel_dyn->add_global_generic(gsym, r_type_2, this,
				got_offset + got_size / 8, addend);
}

// Add an entry for a local symbol plus ADDEND to the GOT.  This returns
// true if this is a new GOT entry, false if the symbol already has a GOT
// entry.

template<int got_size, bool big_endian>
bool
Output_data_got<got_size, big_endian>::add_local(
    Relobj* object,
    unsigned int symndx,
    unsigned int got_type,
    uint64_t addend)
{
  if (object->local_has_got_offset(symndx, got_type, addend))
    return false;

  unsigned int got_offset = this->add_got_entry(Got_entry(object, symndx,
							  false, addend));
  object->set_local_got_offset(symndx, got_type, got_offset, addend);
  return true;
}

// Like add_local, but use the PLT offset.

template<int got_size, bool big_endian>
bool
Output_data_got<got_size, big_endian>::add_local_plt(
    Relobj* object,
    unsigned int symndx,
    unsigned int got_type,
    uint64_t addend)
{
  if (object->local_has_got_offset(symndx, got_type, addend))
    return false;

  unsigned int got_offset = this->add_got_entry(Got_entry(object, symndx,
							  true, addend));
  object->set_local_got_offset(symndx, got_type, got_offset, addend);
  return true;
}

// Add an entry for a local symbol plus ADDEND to the GOT, and add a dynamic
// relocation of type R_TYPE for the GOT entry.

template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::add_local_with_rel(
    Relobj* object,
    unsigned int symndx,
    unsigned int got_type,
    Output_data_reloc_generic* rel_dyn,
    unsigned int r_type,
    uint64_t addend)
{
  if (object->local_has_got_offset(symndx, got_type, addend))
    return;

  unsigned int got_offset = this->add_got_entry(Got_entry());
  object->set_local_got_offset(symndx, got_type, got_offset, addend);
  rel_dyn->add_local_generic(object, symndx, r_type, this, got_offset,
                             addend);
}

// Add a pair of entries for a local symbol plus ADDEND to the GOT, and add
// a dynamic relocation of type R_TYPE using the section symbol of
// the output section to which input section SHNDX maps, on the first.
// The first got entry will have a value of zero, the second the
// value of the local symbol.
template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::add_local_pair_with_rel(
    Relobj* object,
    unsigned int symndx,
    unsigned int shndx,
    unsigned int got_type,
    Output_data_reloc_generic* rel_dyn,
    unsigned int r_type,
    uint64_t addend)
{
  if (object->local_has_got_offset(symndx, got_type, addend))
    return;

  unsigned int got_offset =
      this->add_got_entry_pair(Got_entry(),
			       Got_entry(object, symndx, false, addend));
  object->set_local_got_offset(symndx, got_type, got_offset, addend);
  Output_section* os = object->output_section(shndx);
  rel_dyn->add_output_section_generic(os, r_type, this, got_offset, addend);
}

// Add a pair of entries for a local symbol to the GOT, and add
// a dynamic relocation of type R_TYPE using STN_UNDEF on the first.
// The first got entry will have a value of zero, the second the
// value of the local symbol offset by Target::tls_offset_for_local.
template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::add_local_tls_pair(
    Relobj* object,
    unsigned int symndx,
    unsigned int got_type,
    Output_data_reloc_generic* rel_dyn,
    unsigned int r_type,
    uint64_t addend)
{
  if (object->local_has_got_offset(symndx, got_type, addend))
    return;

  unsigned int got_offset
    = this->add_got_entry_pair(Got_entry(),
			       Got_entry(object, symndx, true, addend));
  object->set_local_got_offset(symndx, got_type, got_offset, addend);
  rel_dyn->add_local_generic(object, 0, r_type, this, got_offset, addend);
}

// Reserve a slot in the GOT for a local symbol or the second slot of a pair.

template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::reserve_local(
    unsigned int i,
    Relobj* object,
    unsigned int sym_index,
    unsigned int got_type,
    uint64_t addend)
{
  this->do_reserve_slot(i);
  object->set_local_got_offset(sym_index, got_type, this->got_offset(i), addend);
}

// Reserve a slot in the GOT for a global symbol.

template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::reserve_global(
    unsigned int i,
    Symbol* gsym,
    unsigned int got_type,
    uint64_t addend)
{
  this->do_reserve_slot(i);
  gsym->set_got_offset(got_type, this->got_offset(i), addend);
}

// Write out the GOT.

template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::do_write(Output_file* of)
{
  const int add = got_size / 8;

  const off_t off = this->offset();
  const off_t oview_size = this->data_size();
  unsigned char* const oview = of->get_output_view(off, oview_size);

  unsigned char* pov = oview;
  for (unsigned int i = 0; i < this->entries_.size(); ++i)
    {
      this->entries_[i].write(this, i, pov);
      pov += add;
    }

  gold_assert(pov - oview == oview_size);

  of->write_output_view(off, oview_size, oview);

  // We no longer need the GOT entries.
  this->entries_.clear();
}

// Create a new GOT entry and return its offset.

template<int got_size, bool big_endian>
unsigned int
Output_data_got<got_size, big_endian>::add_got_entry(Got_entry got_entry)
{
  if (!this->is_data_size_valid())
    {
      this->entries_.push_back(got_entry);
      this->set_got_size();
      return this->last_got_offset();
    }
  else
    {
      // For an incremental update, find an available slot.
      off_t got_offset = this->free_list_.allocate(got_size / 8,
						   got_size / 8, 0);
      if (got_offset == -1)
	gold_fallback(_("out of patch space (GOT);"
			" relink with --incremental-full"));
      unsigned int got_index = got_offset / (got_size / 8);
      gold_assert(got_index < this->entries_.size());
      this->entries_[got_index] = got_entry;
      return static_cast<unsigned int>(got_offset);
    }
}

// Create a pair of new GOT entries and return the offset of the first.

template<int got_size, bool big_endian>
unsigned int
Output_data_got<got_size, big_endian>::add_got_entry_pair(
    Got_entry got_entry_1,
    Got_entry got_entry_2)
{
  if (!this->is_data_size_valid())
    {
      unsigned int got_offset;
      this->entries_.push_back(got_entry_1);
      got_offset = this->last_got_offset();
      this->entries_.push_back(got_entry_2);
      this->set_got_size();
      return got_offset;
    }
  else
    {
      // For an incremental update, find an available pair of slots.
      off_t got_offset = this->free_list_.allocate(2 * got_size / 8,
						   got_size / 8, 0);
      if (got_offset == -1)
	gold_fallback(_("out of patch space (GOT);"
			" relink with --incremental-full"));
      unsigned int got_index = got_offset / (got_size / 8);
      gold_assert(got_index < this->entries_.size());
      this->entries_[got_index] = got_entry_1;
      this->entries_[got_index + 1] = got_entry_2;
      return static_cast<unsigned int>(got_offset);
    }
}

// Replace GOT entry I with a new value.

template<int got_size, bool big_endian>
void
Output_data_got<got_size, big_endian>::replace_got_entry(
    unsigned int i,
    Got_entry got_entry)
{
  gold_assert(i < this->entries_.size());
  this->entries_[i] = got_entry;
}

// Output_data_dynamic::Dynamic_entry methods.

// Write out the entry.

template<int size, bool big_endian>
void
Output_data_dynamic::Dynamic_entry::write(
    unsigned char* pov,
    const Stringpool* pool) const
{
  typename elfcpp::Elf_types<size>::Elf_WXword val;
  switch (this->offset_)
    {
    case DYNAMIC_NUMBER:
      val = this->u_.val;
      break;

    case DYNAMIC_SECTION_SIZE:
      val = this->u_.od->data_size();
      if (this->od2 != NULL)
	val += this->od2->data_size();
      break;

    case DYNAMIC_SYMBOL:
      {
	const Sized_symbol<size>* s =
	  static_cast<const Sized_symbol<size>*>(this->u_.sym);
	val = s->value();
      }
      break;

    case DYNAMIC_STRING:
      val = pool->get_offset(this->u_.str);
      break;

    case DYNAMIC_CUSTOM:
      val = parameters->target().dynamic_tag_custom_value(this->tag_);
      break;

    default:
      val = this->u_.od->address() + this->offset_;
      break;
    }

  elfcpp::Dyn_write<size, big_endian> dw(pov);
  dw.put_d_tag(this->tag_);
  dw.put_d_val(val);
}

// Output_data_dynamic methods.

// Adjust the output section to set the entry size.

void
Output_data_dynamic::do_adjust_output_section(Output_section* os)
{
  if (parameters->target().get_size() == 32)
    os->set_entsize(elfcpp::Elf_sizes<32>::dyn_size);
  else if (parameters->target().get_size() == 64)
    os->set_entsize(elfcpp::Elf_sizes<64>::dyn_size);
  else
    gold_unreachable();
}

// Get a dynamic entry offset.

unsigned int
Output_data_dynamic::get_entry_offset(elfcpp::DT tag) const
{
  int dyn_size;

  if (parameters->target().get_size() == 32)
    dyn_size = elfcpp::Elf_sizes<32>::dyn_size;
  else if (parameters->target().get_size() == 64)
    dyn_size = elfcpp::Elf_sizes<64>::dyn_size;
  else
    gold_unreachable();

  for (size_t i = 0; i < entries_.size(); ++i)
    if (entries_[i].tag() == tag)
      return i * dyn_size;

  return -1U;
}

// Set the final data size.

void
Output_data_dynamic::set_final_data_size()
{
  // Add the terminating entry if it hasn't been added.
  // Because of relaxation, we can run this multiple times.
  if (this->entries_.empty() || this->entries_.back().tag() != elfcpp::DT_NULL)
    {
      int extra = parameters->options().spare_dynamic_tags();
      for (int i = 0; i < extra; ++i)
	this->add_constant(elfcpp::DT_NULL, 0);
      this->add_constant(elfcpp::DT_NULL, 0);
    }

  int dyn_size;
  if (parameters->target().get_size() == 32)
    dyn_size = elfcpp::Elf_sizes<32>::dyn_size;
  else if (parameters->target().get_size() == 64)
    dyn_size = elfcpp::Elf_sizes<64>::dyn_size;
  else
    gold_unreachable();
  this->set_data_size(this->entries_.size() * dyn_size);
}

// Write out the dynamic entries.

void
Output_data_dynamic::do_write(Output_file* of)
{
  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      this->sized_write<32, false>(of);
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      this->sized_write<32, true>(of);
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      this->sized_write<64, false>(of);
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      this->sized_write<64, true>(of);
      break;
#endif
    default:
      gold_unreachable();
    }
}

template<int size, bool big_endian>
void
Output_data_dynamic::sized_write(Output_file* of)
{
  const int dyn_size = elfcpp::Elf_sizes<size>::dyn_size;

  const off_t offset = this->offset();
  const off_t oview_size = this->data_size();
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  unsigned char* pov = oview;
  for (typename Dynamic_entries::const_iterator p = this->entries_.begin();
       p != this->entries_.end();
       ++p)
    {
      p->write<size, big_endian>(pov, this->pool_);
      pov += dyn_size;
    }

  gold_assert(pov - oview == oview_size);

  of->write_output_view(offset, oview_size, oview);

  // We no longer need the dynamic entries.
  this->entries_.clear();
}

// Class Output_symtab_xindex.

void
Output_symtab_xindex::do_write(Output_file* of)
{
  const off_t offset = this->offset();
  const off_t oview_size = this->data_size();
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  memset(oview, 0, oview_size);

  if (parameters->target().is_big_endian())
    this->endian_do_write<true>(oview);
  else
    this->endian_do_write<false>(oview);

  of->write_output_view(offset, oview_size, oview);

  // We no longer need the data.
  this->entries_.clear();
}

template<bool big_endian>
void
Output_symtab_xindex::endian_do_write(unsigned char* const oview)
{
  for (Xindex_entries::const_iterator p = this->entries_.begin();
       p != this->entries_.end();
       ++p)
    {
      unsigned int symndx = p->first;
      gold_assert(static_cast<off_t>(symndx) * 4 < this->data_size());
      elfcpp::Swap<32, big_endian>::writeval(oview + symndx * 4, p->second);
    }
}

// Output_fill_debug_info methods.

// Return the minimum size needed for a dummy compilation unit header.

size_t
Output_fill_debug_info::do_minimum_hole_size() const
{
  // Compile unit header fields: unit_length, version, debug_abbrev_offset,
  // address_size.
  const size_t len = 4 + 2 + 4 + 1;
  // For type units, add type_signature, type_offset.
  if (this->is_debug_types_)
    return len + 8 + 4;
  return len;
}

// Write a dummy compilation unit header to fill a hole in the
// .debug_info or .debug_types section.

void
Output_fill_debug_info::do_write(Output_file* of, off_t off, size_t len) const
{
  gold_debug(DEBUG_INCREMENTAL, "fill_debug_info(%08lx, %08lx)",
	     static_cast<long>(off), static_cast<long>(len));

  gold_assert(len >= this->do_minimum_hole_size());

  unsigned char* const oview = of->get_output_view(off, len);
  unsigned char* pov = oview;

  // Write header fields: unit_length, version, debug_abbrev_offset,
  // address_size.
  if (this->is_big_endian())
    {
      elfcpp::Swap_unaligned<32, true>::writeval(pov, len - 4);
      elfcpp::Swap_unaligned<16, true>::writeval(pov + 4, this->version);
      elfcpp::Swap_unaligned<32, true>::writeval(pov + 6, 0);
    }
  else
    {
      elfcpp::Swap_unaligned<32, false>::writeval(pov, len - 4);
      elfcpp::Swap_unaligned<16, false>::writeval(pov + 4, this->version);
      elfcpp::Swap_unaligned<32, false>::writeval(pov + 6, 0);
    }
  pov += 4 + 2 + 4;
  *pov++ = 4;

  // For type units, the additional header fields -- type_signature,
  // type_offset -- can be filled with zeroes.

  // Fill the remainder of the free space with zeroes.  The first
  // zero should tell the consumer there are no DIEs to read in this
  // compilation unit.
  if (pov < oview + len)
    memset(pov, 0, oview + len - pov);

  of->write_output_view(off, len, oview);
}

// Output_fill_debug_line methods.

// Return the minimum size needed for a dummy line number program header.

size_t
Output_fill_debug_line::do_minimum_hole_size() const
{
  // Line number program header fields: unit_length, version, header_length,
  // minimum_instruction_length, default_is_stmt, line_base, line_range,
  // opcode_base, standard_opcode_lengths[], include_directories, filenames.
  const size_t len = 4 + 2 + 4 + this->header_length;
  return len;
}

// Write a dummy line number program header to fill a hole in the
// .debug_line section.

void
Output_fill_debug_line::do_write(Output_file* of, off_t off, size_t len) const
{
  gold_debug(DEBUG_INCREMENTAL, "fill_debug_line(%08lx, %08lx)",
	     static_cast<long>(off), static_cast<long>(len));

  gold_assert(len >= this->do_minimum_hole_size());

  unsigned char* const oview = of->get_output_view(off, len);
  unsigned char* pov = oview;

  // Write header fields: unit_length, version, header_length,
  // minimum_instruction_length, default_is_stmt, line_base, line_range,
  // opcode_base, standard_opcode_lengths[], include_directories, filenames.
  // We set the header_length field to cover the entire hole, so the
  // line number program is empty.
  if (this->is_big_endian())
    {
      elfcpp::Swap_unaligned<32, true>::writeval(pov, len - 4);
      elfcpp::Swap_unaligned<16, true>::writeval(pov + 4, this->version);
      elfcpp::Swap_unaligned<32, true>::writeval(pov + 6, len - (4 + 2 + 4));
    }
  else
    {
      elfcpp::Swap_unaligned<32, false>::writeval(pov, len - 4);
      elfcpp::Swap_unaligned<16, false>::writeval(pov + 4, this->version);
      elfcpp::Swap_unaligned<32, false>::writeval(pov + 6, len - (4 + 2 + 4));
    }
  pov += 4 + 2 + 4;
  *pov++ = 1;	// minimum_instruction_length
  *pov++ = 0;	// default_is_stmt
  *pov++ = 0;	// line_base
  *pov++ = 5;	// line_range
  *pov++ = 13;	// opcode_base
  *pov++ = 0;	// standard_opcode_lengths[1]
  *pov++ = 1;	// standard_opcode_lengths[2]
  *pov++ = 1;	// standard_opcode_lengths[3]
  *pov++ = 1;	// standard_opcode_lengths[4]
  *pov++ = 1;	// standard_opcode_lengths[5]
  *pov++ = 0;	// standard_opcode_lengths[6]
  *pov++ = 0;	// standard_opcode_lengths[7]
  *pov++ = 0;	// standard_opcode_lengths[8]
  *pov++ = 1;	// standard_opcode_lengths[9]
  *pov++ = 0;	// standard_opcode_lengths[10]
  *pov++ = 0;	// standard_opcode_lengths[11]
  *pov++ = 1;	// standard_opcode_lengths[12]
  *pov++ = 0;	// include_directories (empty)
  *pov++ = 0;	// filenames (empty)

  // Some consumers don't check the header_length field, and simply
  // start reading the line number program immediately following the
  // header.  For those consumers, we fill the remainder of the free
  // space with DW_LNS_set_basic_block opcodes.  These are effectively
  // no-ops: the resulting line table program will not create any rows.
  if (pov < oview + len)
    memset(pov, elfcpp::DW_LNS_set_basic_block, oview + len - pov);

  of->write_output_view(off, len, oview);
}

// Output_section::Input_section methods.

// Return the current data size.  For an input section we store the size here.
// For an Output_section_data, we have to ask it for the size.

off_t
Output_section::Input_section::current_data_size() const
{
  if (this->is_input_section())
    return this->u1_.data_size;
  else
    {
      this->u2_.posd->pre_finalize_data_size();
      return this->u2_.posd->current_data_size();
    }
}

// Return the data size.  For an input section we store the size here.
// For an Output_section_data, we have to ask it for the size.

off_t
Output_section::Input_section::data_size() const
{
  if (this->is_input_section())
    return this->u1_.data_size;
  else
    return this->u2_.posd->data_size();
}

// Return the object for an input section.

Relobj*
Output_section::Input_section::relobj() const
{
  if (this->is_input_section())
    return this->u2_.object;
  else if (this->is_merge_section())
    {
      gold_assert(this->u2_.pomb->first_relobj() != NULL);
      return this->u2_.pomb->first_relobj();
    }
  else if (this->is_relaxed_input_section())
    return this->u2_.poris->relobj();
  else
    gold_unreachable();
}

// Return the input section index for an input section.

unsigned int
Output_section::Input_section::shndx() const
{
  if (this->is_input_section())
    return this->shndx_;
  else if (this->is_merge_section())
    {
      gold_assert(this->u2_.pomb->first_relobj() != NULL);
      return this->u2_.pomb->first_shndx();
    }
  else if (this->is_relaxed_input_section())
    return this->u2_.poris->shndx();
  else
    gold_unreachable();
}

// Set the address and file offset.

void
Output_section::Input_section::set_address_and_file_offset(
    uint64_t address,
    off_t file_offset,
    off_t section_file_offset)
{
  if (this->is_input_section())
    this->u2_.object->set_section_offset(this->shndx_,
					 file_offset - section_file_offset);
  else
    this->u2_.posd->set_address_and_file_offset(address, file_offset);
}

// Reset the address and file offset.

void
Output_section::Input_section::reset_address_and_file_offset()
{
  if (!this->is_input_section())
    this->u2_.posd->reset_address_and_file_offset();
}

// Finalize the data size.

void
Output_section::Input_section::finalize_data_size()
{
  if (!this->is_input_section())
    this->u2_.posd->finalize_data_size();
}

// Try to turn an input offset into an output offset.  We want to
// return the output offset relative to the start of this
// Input_section in the output section.

inline bool
Output_section::Input_section::output_offset(
    const Relobj* object,
    unsigned int shndx,
    section_offset_type offset,
    section_offset_type* poutput) const
{
  if (!this->is_input_section())
    return this->u2_.posd->output_offset(object, shndx, offset, poutput);
  else
    {
      if (this->shndx_ != shndx || this->u2_.object != object)
	return false;
      *poutput = offset;
      return true;
    }
}

// Write out the data.  We don't have to do anything for an input
// section--they are handled via Object::relocate--but this is where
// we write out the data for an Output_section_data.

void
Output_section::Input_section::write(Output_file* of)
{
  if (!this->is_input_section())
    this->u2_.posd->write(of);
}

// Write the data to a buffer.  As for write(), we don't have to do
// anything for an input section.

void
Output_section::Input_section::write_to_buffer(unsigned char* buffer)
{
  if (!this->is_input_section())
    this->u2_.posd->write_to_buffer(buffer);
}

// Print to a map file.

void
Output_section::Input_section::print_to_mapfile(Mapfile* mapfile) const
{
  switch (this->shndx_)
    {
    case OUTPUT_SECTION_CODE:
    case MERGE_DATA_SECTION_CODE:
    case MERGE_STRING_SECTION_CODE:
      this->u2_.posd->print_to_mapfile(mapfile);
      break;

    case RELAXED_INPUT_SECTION_CODE:
      {
	Output_relaxed_input_section* relaxed_section =
	  this->relaxed_input_section();
	mapfile->print_input_section(relaxed_section->relobj(),
				     relaxed_section->shndx());
      }
      break;
    default:
      mapfile->print_input_section(this->u2_.object, this->shndx_);
      break;
    }
}

// Output_section methods.

// Construct an Output_section.  NAME will point into a Stringpool.

Output_section::Output_section(const char* name, elfcpp::Elf_Word type,
			       elfcpp::Elf_Xword flags)
  : name_(name),
    addralign_(0),
    entsize_(0),
    load_address_(0),
    link_section_(NULL),
    link_(0),
    info_section_(NULL),
    info_symndx_(NULL),
    info_(0),
    type_(type),
    flags_(flags),
    order_(ORDER_INVALID),
    out_shndx_(-1U),
    symtab_index_(0),
    dynsym_index_(0),
    input_sections_(),
    first_input_offset_(0),
    fills_(),
    postprocessing_buffer_(NULL),
    needs_symtab_index_(false),
    needs_dynsym_index_(false),
    should_link_to_symtab_(false),
    should_link_to_dynsym_(false),
    after_input_sections_(false),
    requires_postprocessing_(false),
    found_in_sections_clause_(false),
    has_load_address_(false),
    info_uses_section_index_(false),
    input_section_order_specified_(false),
    may_sort_attached_input_sections_(false),
    must_sort_attached_input_sections_(false),
    attached_input_sections_are_sorted_(false),
    is_relro_(false),
    is_small_section_(false),
    is_large_section_(false),
    generate_code_fills_at_write_(false),
    is_entsize_zero_(false),
    section_offsets_need_adjustment_(false),
    is_noload_(false),
    always_keeps_input_sections_(false),
    has_fixed_layout_(false),
    is_patch_space_allowed_(false),
    is_unique_segment_(false),
    tls_offset_(0),
    extra_segment_flags_(0),
    segment_alignment_(0),
    checkpoint_(NULL),
    lookup_maps_(new Output_section_lookup_maps),
    free_list_(),
    free_space_fill_(NULL),
    patch_space_(0),
    reloc_section_(NULL)
{
  // An unallocated section has no address.  Forcing this means that
  // we don't need special treatment for symbols defined in debug
  // sections.
  if ((flags & elfcpp::SHF_ALLOC) == 0)
    this->set_address(0);
}

Output_section::~Output_section()
{
  delete this->checkpoint_;
}

// Set the entry size.

void
Output_section::set_entsize(uint64_t v)
{
  if (this->is_entsize_zero_)
    ;
  else if (this->entsize_ == 0)
    this->entsize_ = v;
  else if (this->entsize_ != v)
    {
      this->entsize_ = 0;
      this->is_entsize_zero_ = 1;
    }
}

// Add the input section SHNDX, with header SHDR, named SECNAME, in
// OBJECT, to the Output_section.  RELOC_SHNDX is the index of a
// relocation section which applies to this section, or 0 if none, or
// -1U if more than one.  Return the offset of the input section
// within the output section.  Return -1 if the input section will
// receive special handling.  In the normal case we don't always keep
// track of input sections for an Output_section.  Instead, each
// Object keeps track of the Output_section for each of its input
// sections.  However, if HAVE_SECTIONS_SCRIPT is true, we do keep
// track of input sections here; this is used when SECTIONS appears in
// a linker script.

template<int size, bool big_endian>
off_t
Output_section::add_input_section(Layout* layout,
				  Sized_relobj_file<size, big_endian>* object,
				  unsigned int shndx,
				  const char* secname,
				  const elfcpp::Shdr<size, big_endian>& shdr,
				  unsigned int reloc_shndx,
				  bool have_sections_script)
{
  section_size_type input_section_size = shdr.get_sh_size();
  section_size_type uncompressed_size;
  elfcpp::Elf_Xword addralign = shdr.get_sh_addralign();
  if (object->section_is_compressed(shndx, &uncompressed_size,
				    &addralign))
    input_section_size = uncompressed_size;

  if ((addralign & (addralign - 1)) != 0)
    {
      object->error(_("invalid alignment %lu for section \"%s\""),
		    static_cast<unsigned long>(addralign), secname);
      addralign = 1;
    }

  if (addralign > this->addralign_)
    this->addralign_ = addralign;

  typename elfcpp::Elf_types<size>::Elf_WXword sh_flags = shdr.get_sh_flags();
  uint64_t entsize = shdr.get_sh_entsize();

  // .debug_str is a mergeable string section, but is not always so
  // marked by compilers.  Mark manually here so we can optimize.
  if (strcmp(secname, ".debug_str") == 0)
    {
      sh_flags |= (elfcpp::SHF_MERGE | elfcpp::SHF_STRINGS);
      entsize = 1;
    }

  this->update_flags_for_input_section(sh_flags);
  this->set_entsize(entsize);

  // If this is a SHF_MERGE section, we pass all the input sections to
  // a Output_data_merge.  We don't try to handle relocations for such
  // a section.  We don't try to handle empty merge sections--they
  // mess up the mappings, and are useless anyhow.
  // FIXME: Need to handle merge sections during incremental update.
  if ((sh_flags & elfcpp::SHF_MERGE) != 0
      && reloc_shndx == 0
      && shdr.get_sh_size() > 0
      && !parameters->incremental())
    {
      // Keep information about merged input sections for rebuilding fast
      // lookup maps if we have sections-script or we do relaxation.
      bool keeps_input_sections = (this->always_keeps_input_sections_
				   || have_sections_script
				   || parameters->target().may_relax());

      if (this->add_merge_input_section(object, shndx, sh_flags, entsize,
					addralign, keeps_input_sections))
	{
	  // Tell the relocation routines that they need to call the
	  // output_offset method to determine the final address.
	  return -1;
	}
    }

  off_t offset_in_section;

  if (this->has_fixed_layout())
    {
      // For incremental updates, find a chunk of unused space in the section.
      offset_in_section = this->free_list_.allocate(input_section_size,
						    addralign, 0);
      if (offset_in_section == -1)
	gold_fallback(_("out of patch space in section %s; "
			"relink with --incremental-full"),
		      this->name());
      return offset_in_section;
    }

  offset_in_section = this->current_data_size_for_child();
  off_t aligned_offset_in_section = align_address(offset_in_section,
						  addralign);
  this->set_current_data_size_for_child(aligned_offset_in_section
					+ input_section_size);

  // Determine if we want to delay code-fill generation until the output
  // section is written.  When the target is relaxing, we want to delay fill
  // generating to avoid adjusting them during relaxation.  Also, if we are
  // sorting input sections we must delay fill generation.
  if (!this->generate_code_fills_at_write_
      && !have_sections_script
      && (sh_flags & elfcpp::SHF_EXECINSTR) != 0
      && parameters->target().has_code_fill()
      && (parameters->target().may_relax()
	  || layout->is_section_ordering_specified()))
    {
      gold_assert(this->fills_.empty());
      this->generate_code_fills_at_write_ = true;
    }

  if (aligned_offset_in_section > offset_in_section
      && !this->generate_code_fills_at_write_
      && !have_sections_script
      && (sh_flags & elfcpp::SHF_EXECINSTR) != 0
      && parameters->target().has_code_fill())
    {
      // We need to add some fill data.  Using fill_list_ when
      // possible is an optimization, since we will often have fill
      // sections without input sections.
      off_t fill_len = aligned_offset_in_section - offset_in_section;
      if (this->input_sections_.empty())
	this->fills_.push_back(Fill(offset_in_section, fill_len));
      else
	{
	  std::string fill_data(parameters->target().code_fill(fill_len));
	  Output_data_const* odc = new Output_data_const(fill_data, 1);
	  this->input_sections_.push_back(Input_section(odc));
	}
    }

  // We need to keep track of this section if we are already keeping
  // track of sections, or if we are relaxing.  Also, if this is a
  // section which requires sorting, or which may require sorting in
  // the future, we keep track of the sections.  If the
  // --section-ordering-file option is used to specify the order of
  // sections, we need to keep track of sections.
  if (this->always_keeps_input_sections_
      || have_sections_script
      || !this->input_sections_.empty()
      || this->may_sort_attached_input_sections()
      || this->must_sort_attached_input_sections()
      || parameters->options().user_set_Map()
      || parameters->target().may_relax()
      || layout->is_section_ordering_specified())
    {
      Input_section isecn(object, shndx, input_section_size, addralign);
      /* If section ordering is requested by specifying a ordering file,
	 using --section-ordering-file, match the section name with
	 a pattern.  */
      if (parameters->options().section_ordering_file())
	{
	  unsigned int section_order_index =
	    layout->find_section_order_index(std::string(secname));
	  if (section_order_index != 0)
	    {
	      isecn.set_section_order_index(section_order_index);
	      this->set_input_section_order_specified();
	    }
	}
      this->input_sections_.push_back(isecn);
    }

  return aligned_offset_in_section;
}

// Add arbitrary data to an output section.

void
Output_section::add_output_section_data(Output_section_data* posd)
{
  Input_section inp(posd);
  this->add_output_section_data(&inp);

  if (posd->is_data_size_valid())
    {
      off_t offset_in_section;
      if (this->has_fixed_layout())
	{
	  // For incremental updates, find a chunk of unused space.
	  offset_in_section = this->free_list_.allocate(posd->data_size(),
							posd->addralign(), 0);
	  if (offset_in_section == -1)
	    gold_fallback(_("out of patch space in section %s; "
			    "relink with --incremental-full"),
			  this->name());
	  // Finalize the address and offset now.
	  uint64_t addr = this->address();
	  off_t offset = this->offset();
	  posd->set_address_and_file_offset(addr + offset_in_section,
					    offset + offset_in_section);
	}
      else
	{
	  offset_in_section = this->current_data_size_for_child();
	  off_t aligned_offset_in_section = align_address(offset_in_section,
							  posd->addralign());
	  this->set_current_data_size_for_child(aligned_offset_in_section
						+ posd->data_size());
	}
    }
  else if (this->has_fixed_layout())
    {
      // For incremental updates, arrange for the data to have a fixed layout.
      // This will mean that additions to the data must be allocated from
      // free space within the containing output section.
      uint64_t addr = this->address();
      posd->set_address(addr);
      posd->set_file_offset(0);
      // FIXME: This should eventually be unreachable.
      // gold_unreachable();
    }
}

// Add a relaxed input section.

void
Output_section::add_relaxed_input_section(Layout* layout,
					  Output_relaxed_input_section* poris,
					  const std::string& name)
{
  Input_section inp(poris);

  // If the --section-ordering-file option is used to specify the order of
  // sections, we need to keep track of sections.
  if (layout->is_section_ordering_specified())
    {
      unsigned int section_order_index =
	layout->find_section_order_index(name);
      if (section_order_index != 0)
	{
	  inp.set_section_order_index(section_order_index);
	  this->set_input_section_order_specified();
	}
    }

  this->add_output_section_data(&inp);
  if (this->lookup_maps_->is_valid())
    this->lookup_maps_->add_relaxed_input_section(poris->relobj(),
						  poris->shndx(), poris);

  // For a relaxed section, we use the current data size.  Linker scripts
  // get all the input sections, including relaxed one from an output
  // section and add them back to the same output section to compute the
  // output section size.  If we do not account for sizes of relaxed input
  // sections, an output section would be incorrectly sized.
  off_t offset_in_section = this->current_data_size_for_child();
  off_t aligned_offset_in_section = align_address(offset_in_section,
						  poris->addralign());
  this->set_current_data_size_for_child(aligned_offset_in_section
					+ poris->current_data_size());
}

// Add arbitrary data to an output section by Input_section.

void
Output_section::add_output_section_data(Input_section* inp)
{
  if (this->input_sections_.empty())
    this->first_input_offset_ = this->current_data_size_for_child();

  this->input_sections_.push_back(*inp);

  uint64_t addralign = inp->addralign();
  if (addralign > this->addralign_)
    this->addralign_ = addralign;

  inp->set_output_section(this);
}

// Add a merge section to an output section.

void
Output_section::add_output_merge_section(Output_section_data* posd,
					 bool is_string, uint64_t entsize)
{
  Input_section inp(posd, is_string, entsize);
  this->add_output_section_data(&inp);
}

// Add an input section to a SHF_MERGE section.

bool
Output_section::add_merge_input_section(Relobj* object, unsigned int shndx,
					uint64_t flags, uint64_t entsize,
					uint64_t addralign,
					bool keeps_input_sections)
{
  // We cannot merge sections with entsize == 0.
  if (entsize == 0)
    return false;

  bool is_string = (flags & elfcpp::SHF_STRINGS) != 0;

  // We cannot restore merged input section states.
  gold_assert(this->checkpoint_ == NULL);

  // Look up merge sections by required properties.
  // Currently, we only invalidate the lookup maps in script processing
  // and relaxation.  We should not have done either when we reach here.
  // So we assume that the lookup maps are valid to simply code.
  gold_assert(this->lookup_maps_->is_valid());
  Merge_section_properties msp(is_string, entsize, addralign);
  Output_merge_base* pomb = this->lookup_maps_->find_merge_section(msp);
  bool is_new = false;
  if (pomb != NULL)
    {
      gold_assert(pomb->is_string() == is_string
		  && pomb->entsize() == entsize
		  && pomb->addralign() == addralign);
    }
  else
    {
      // Create a new Output_merge_data or Output_merge_string_data.
      if (!is_string)
	pomb = new Output_merge_data(entsize, addralign);
      else
	{
	  switch (entsize)
	    {
	    case 1:
	      pomb = new Output_merge_string<char>(addralign);
	      break;
	    case 2:
	      pomb = new Output_merge_string<uint16_t>(addralign);
	      break;
	    case 4:
	      pomb = new Output_merge_string<uint32_t>(addralign);
	      break;
	    default:
	      return false;
	    }
	}
      // If we need to do script processing or relaxation, we need to keep
      // the original input sections to rebuild the fast lookup maps.
      if (keeps_input_sections)
	pomb->set_keeps_input_sections();
      is_new = true;
    }

  if (pomb->add_input_section(object, shndx))
    {
      // Add new merge section to this output section and link merge
      // section properties to new merge section in map.
      if (is_new)
	{
	  this->add_output_merge_section(pomb, is_string, entsize);
	  this->lookup_maps_->add_merge_section(msp, pomb);
	}

      return true;
    }
  else
    {
      // If add_input_section failed, delete new merge section to avoid
      // exporting empty merge sections in Output_section::get_input_section.
      if (is_new)
	delete pomb;
      return false;
    }
}

// Build a relaxation map to speed up relaxation of existing input sections.
// Look up to the first LIMIT elements in INPUT_SECTIONS.

void
Output_section::build_relaxation_map(
  const Input_section_list& input_sections,
  size_t limit,
  Relaxation_map* relaxation_map) const
{
  for (size_t i = 0; i < limit; ++i)
    {
      const Input_section& is(input_sections[i]);
      if (is.is_input_section() || is.is_relaxed_input_section())
	{
	  Section_id sid(is.relobj(), is.shndx());
	  (*relaxation_map)[sid] = i;
	}
    }
}

// Convert regular input sections in INPUT_SECTIONS into relaxed input
// sections in RELAXED_SECTIONS.  MAP is a prebuilt map from section id
// indices of INPUT_SECTIONS.

void
Output_section::convert_input_sections_in_list_to_relaxed_sections(
  const std::vector<Output_relaxed_input_section*>& relaxed_sections,
  const Relaxation_map& map,
  Input_section_list* input_sections)
{
  for (size_t i = 0; i < relaxed_sections.size(); ++i)
    {
      Output_relaxed_input_section* poris = relaxed_sections[i];
      Section_id sid(poris->relobj(), poris->shndx());
      Relaxation_map::const_iterator p = map.find(sid);
      gold_assert(p != map.end());
      gold_assert((*input_sections)[p->second].is_input_section());

      // Remember section order index of original input section
      // if it is set.  Copy it to the relaxed input section.
      unsigned int soi =
	(*input_sections)[p->second].section_order_index();
      (*input_sections)[p->second] = Input_section(poris);
      (*input_sections)[p->second].set_section_order_index(soi);
    }
}

// Convert regular input sections into relaxed input sections. RELAXED_SECTIONS
// is a vector of pointers to Output_relaxed_input_section or its derived
// classes.  The relaxed sections must correspond to existing input sections.

void
Output_section::convert_input_sections_to_relaxed_sections(
  const std::vector<Output_relaxed_input_section*>& relaxed_sections)
{
  gold_assert(parameters->target().may_relax());

  // We want to make sure that restore_states does not undo the effect of
  // this.  If there is no checkpoint active, just search the current
  // input section list and replace the sections there.  If there is
  // a checkpoint, also replace the sections there.

  // By default, we look at the whole list.
  size_t limit = this->input_sections_.size();

  if (this->checkpoint_ != NULL)
    {
      // Replace input sections with relaxed input section in the saved
      // copy of the input section list.
      if (this->checkpoint_->input_sections_saved())
	{
	  Relaxation_map map;
	  this->build_relaxation_map(
		    *(this->checkpoint_->input_sections()),
		    this->checkpoint_->input_sections()->size(),
		    &map);
	  this->convert_input_sections_in_list_to_relaxed_sections(
		    relaxed_sections,
		    map,
		    this->checkpoint_->input_sections());
	}
      else
	{
	  // We have not copied the input section list yet.  Instead, just
	  // look at the portion that would be saved.
	  limit = this->checkpoint_->input_sections_size();
	}
    }

  // Convert input sections in input_section_list.
  Relaxation_map map;
  this->build_relaxation_map(this->input_sections_, limit, &map);
  this->convert_input_sections_in_list_to_relaxed_sections(
	    relaxed_sections,
	    map,
	    &this->input_sections_);

  // Update fast look-up map.
  if (this->lookup_maps_->is_valid())
    for (size_t i = 0; i < relaxed_sections.size(); ++i)
      {
	Output_relaxed_input_section* poris = relaxed_sections[i];
	this->lookup_maps_->add_relaxed_input_section(poris->relobj(),
						      poris->shndx(), poris);
      }
}

// Update the output section flags based on input section flags.

void
Output_section::update_flags_for_input_section(elfcpp::Elf_Xword flags)
{
  // If we created the section with SHF_ALLOC clear, we set the
  // address.  If we are now setting the SHF_ALLOC flag, we need to
  // undo that.
  if ((this->flags_ & elfcpp::SHF_ALLOC) == 0
      && (flags & elfcpp::SHF_ALLOC) != 0)
    this->mark_address_invalid();

  this->flags_ |= (flags
		   & (elfcpp::SHF_WRITE
		      | elfcpp::SHF_ALLOC
		      | elfcpp::SHF_EXECINSTR));

  if ((flags & elfcpp::SHF_MERGE) == 0)
    this->flags_ &=~ elfcpp::SHF_MERGE;
  else
    {
      if (this->current_data_size_for_child() == 0)
	this->flags_ |= elfcpp::SHF_MERGE;
    }

  if ((flags & elfcpp::SHF_STRINGS) == 0)
    this->flags_ &=~ elfcpp::SHF_STRINGS;
  else
    {
      if (this->current_data_size_for_child() == 0)
	this->flags_ |= elfcpp::SHF_STRINGS;
    }
}

// Find the merge section into which an input section with index SHNDX in
// OBJECT has been added.  Return NULL if none found.

const Output_section_data*
Output_section::find_merge_section(const Relobj* object,
				   unsigned int shndx) const
{
  return object->find_merge_section(shndx);
}

// Build the lookup maps for relaxed sections.  This needs
// to be declared as a const method so that it is callable with a const
// Output_section pointer.  The method only updates states of the maps.

void
Output_section::build_lookup_maps() const
{
  this->lookup_maps_->clear();
  for (Input_section_list::const_iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      if (p->is_relaxed_input_section())
	{
	  Output_relaxed_input_section* poris = p->relaxed_input_section();
	  this->lookup_maps_->add_relaxed_input_section(poris->relobj(),
							poris->shndx(), poris);
	}
    }
}

// Find an relaxed input section corresponding to an input section
// in OBJECT with index SHNDX.

const Output_relaxed_input_section*
Output_section::find_relaxed_input_section(const Relobj* object,
					   unsigned int shndx) const
{
  if (!this->lookup_maps_->is_valid())
    this->build_lookup_maps();
  return this->lookup_maps_->find_relaxed_input_section(object, shndx);
}

// Given an address OFFSET relative to the start of input section
// SHNDX in OBJECT, return whether this address is being included in
// the final link.  This should only be called if SHNDX in OBJECT has
// a special mapping.

bool
Output_section::is_input_address_mapped(const Relobj* object,
					unsigned int shndx,
					off_t offset) const
{
  // Look at the Output_section_data_maps first.
  const Output_section_data* posd = this->find_merge_section(object, shndx);
  if (posd == NULL)
    posd = this->find_relaxed_input_section(object, shndx);

  if (posd != NULL)
    {
      section_offset_type output_offset;
      bool found = posd->output_offset(object, shndx, offset, &output_offset);
      // By default we assume that the address is mapped. See comment at the
      // end.
      if (!found)
        return true;
      return output_offset != -1;
    }

  // Fall back to the slow look-up.
  for (Input_section_list::const_iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      section_offset_type output_offset;
      if (p->output_offset(object, shndx, offset, &output_offset))
	return output_offset != -1;
    }

  // By default we assume that the address is mapped.  This should
  // only be called after we have passed all sections to Layout.  At
  // that point we should know what we are discarding.
  return true;
}

// Given an address OFFSET relative to the start of input section
// SHNDX in object OBJECT, return the output offset relative to the
// start of the input section in the output section.  This should only
// be called if SHNDX in OBJECT has a special mapping.

section_offset_type
Output_section::output_offset(const Relobj* object, unsigned int shndx,
			      section_offset_type offset) const
{
  // This can only be called meaningfully when we know the data size
  // of this.
  gold_assert(this->is_data_size_valid());

  // Look at the Output_section_data_maps first.
  const Output_section_data* posd = this->find_merge_section(object, shndx);
  if (posd == NULL)
    posd = this->find_relaxed_input_section(object, shndx);
  if (posd != NULL)
    {
      section_offset_type output_offset;
      bool found = posd->output_offset(object, shndx, offset, &output_offset);
      gold_assert(found);
      return output_offset;
    }

  // Fall back to the slow look-up.
  for (Input_section_list::const_iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      section_offset_type output_offset;
      if (p->output_offset(object, shndx, offset, &output_offset))
	return output_offset;
    }
  gold_unreachable();
}

// Return the output virtual address of OFFSET relative to the start
// of input section SHNDX in object OBJECT.

uint64_t
Output_section::output_address(const Relobj* object, unsigned int shndx,
			       off_t offset) const
{
  uint64_t addr = this->address() + this->first_input_offset_;

  // Look at the Output_section_data_maps first.
  const Output_section_data* posd = this->find_merge_section(object, shndx);
  if (posd == NULL)
    posd = this->find_relaxed_input_section(object, shndx);
  if (posd != NULL && posd->is_address_valid())
    {
      section_offset_type output_offset;
      bool found = posd->output_offset(object, shndx, offset, &output_offset);
      gold_assert(found);
      return posd->address() + output_offset;
    }

  // Fall back to the slow look-up.
  for (Input_section_list::const_iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      addr = align_address(addr, p->addralign());
      section_offset_type output_offset;
      if (p->output_offset(object, shndx, offset, &output_offset))
	{
	  if (output_offset == -1)
	    return -1ULL;
	  return addr + output_offset;
	}
      addr += p->data_size();
    }

  // If we get here, it means that we don't know the mapping for this
  // input section.  This might happen in principle if
  // add_input_section were called before add_output_section_data.
  // But it should never actually happen.

  gold_unreachable();
}

// Find the output address of the start of the merged section for
// input section SHNDX in object OBJECT.

bool
Output_section::find_starting_output_address(const Relobj* object,
					     unsigned int shndx,
					     uint64_t* paddr) const
{
  const Output_section_data* data = this->find_merge_section(object, shndx);
  if (data == NULL)
    return false;

  // FIXME: This becomes a bottle-neck if we have many relaxed sections.
  // Looking up the merge section map does not always work as we sometimes
  // find a merge section without its address set.
  uint64_t addr = this->address() + this->first_input_offset_;
  for (Input_section_list::const_iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      addr = align_address(addr, p->addralign());

      // It would be nice if we could use the existing output_offset
      // method to get the output offset of input offset 0.
      // Unfortunately we don't know for sure that input offset 0 is
      // mapped at all.
      if (!p->is_input_section() && p->output_section_data() == data)
	{
	  *paddr = addr;
	  return true;
	}

      addr += p->data_size();
    }

  // We couldn't find a merge output section for this input section.
  return false;
}

// Update the data size of an Output_section.

void
Output_section::update_data_size()
{
  if (this->input_sections_.empty())
      return;

  if (this->must_sort_attached_input_sections()
      || this->input_section_order_specified())
    this->sort_attached_input_sections();

  off_t off = this->first_input_offset_;
  for (Input_section_list::iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      off = align_address(off, p->addralign());
      off += p->current_data_size();
    }

  this->set_current_data_size_for_child(off);
}

// Set the data size of an Output_section.  This is where we handle
// setting the addresses of any Output_section_data objects.

void
Output_section::set_final_data_size()
{
  off_t data_size;

  if (this->input_sections_.empty())
    data_size = this->current_data_size_for_child();
  else
    {
      if (this->must_sort_attached_input_sections()
	  || this->input_section_order_specified())
	this->sort_attached_input_sections();

      uint64_t address = this->address();
      off_t startoff = this->offset();
      off_t off = this->first_input_offset_;
      for (Input_section_list::iterator p = this->input_sections_.begin();
	   p != this->input_sections_.end();
	   ++p)
	{
	  off = align_address(off, p->addralign());
	  p->set_address_and_file_offset(address + off, startoff + off,
					 startoff);
	  off += p->data_size();
	}
      data_size = off;
    }

  // For full incremental links, we want to allocate some patch space
  // in most sections for subsequent incremental updates.
  if (this->is_patch_space_allowed_ && parameters->incremental_full())
    {
      double pct = parameters->options().incremental_patch();
      size_t extra = static_cast<size_t>(data_size * pct);
      if (this->free_space_fill_ != NULL
	  && this->free_space_fill_->minimum_hole_size() > extra)
	extra = this->free_space_fill_->minimum_hole_size();
      off_t new_size = align_address(data_size + extra, this->addralign());
      this->patch_space_ = new_size - data_size;
      gold_debug(DEBUG_INCREMENTAL,
		 "set_final_data_size: %08lx + %08lx: section %s",
		 static_cast<long>(data_size),
		 static_cast<long>(this->patch_space_),
		 this->name());
      data_size = new_size;
    }

  this->set_data_size(data_size);
}

// Reset the address and file offset.

void
Output_section::do_reset_address_and_file_offset()
{
  // An unallocated section has no address.  Forcing this means that
  // we don't need special treatment for symbols defined in debug
  // sections.  We do the same in the constructor.  This does not
  // apply to NOLOAD sections though.
  if (((this->flags_ & elfcpp::SHF_ALLOC) == 0) && !this->is_noload_)
     this->set_address(0);

  for (Input_section_list::iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    p->reset_address_and_file_offset();

  // Remove any patch space that was added in set_final_data_size.
  if (this->patch_space_ > 0)
    {
      this->set_current_data_size_for_child(this->current_data_size_for_child()
					    - this->patch_space_);
      this->patch_space_ = 0;
    }
}

// Return true if address and file offset have the values after reset.

bool
Output_section::do_address_and_file_offset_have_reset_values() const
{
  if (this->is_offset_valid())
    return false;

  // An unallocated section has address 0 after its construction or a reset.
  if ((this->flags_ & elfcpp::SHF_ALLOC) == 0)
    return this->is_address_valid() && this->address() == 0;
  else
    return !this->is_address_valid();
}

// Set the TLS offset.  Called only for SHT_TLS sections.

void
Output_section::do_set_tls_offset(uint64_t tls_base)
{
  this->tls_offset_ = this->address() - tls_base;
}

// In a few cases we need to sort the input sections attached to an
// output section.  This is used to implement the type of constructor
// priority ordering implemented by the GNU linker, in which the
// priority becomes part of the section name and the sections are
// sorted by name.  We only do this for an output section if we see an
// attached input section matching ".ctors.*", ".dtors.*",
// ".init_array.*" or ".fini_array.*".

class Output_section::Input_section_sort_entry
{
 public:
  Input_section_sort_entry()
    : input_section_(), index_(-1U), section_name_()
  { }

  Input_section_sort_entry(const Input_section& input_section,
			   unsigned int index,
			   bool must_sort_attached_input_sections,
			   const char* output_section_name)
    : input_section_(input_section), index_(index), section_name_()
  {
    if ((input_section.is_input_section()
	 || input_section.is_relaxed_input_section())
	&& must_sort_attached_input_sections)
      {
	// This is only called single-threaded from Layout::finalize,
	// so it is OK to lock.  Unfortunately we have no way to pass
	// in a Task token.
	const Task* dummy_task = reinterpret_cast<const Task*>(-1);
	Object* obj = (input_section.is_input_section()
		       ? input_section.relobj()
		       : input_section.relaxed_input_section()->relobj());
	Task_lock_obj<Object> tl(dummy_task, obj);

	// This is a slow operation, which should be cached in
	// Layout::layout if this becomes a speed problem.
	this->section_name_ = obj->section_name(input_section.shndx());
      }
    else if (input_section.is_output_section_data()
    	     && must_sort_attached_input_sections)
      {
	// For linker-generated sections, use the output section name.
	this->section_name_.assign(output_section_name);
      }
  }

  // Return the Input_section.
  const Input_section&
  input_section() const
  {
    gold_assert(this->index_ != -1U);
    return this->input_section_;
  }

  // The index of this entry in the original list.  This is used to
  // make the sort stable.
  unsigned int
  index() const
  {
    gold_assert(this->index_ != -1U);
    return this->index_;
  }

  // The section name.
  const std::string&
  section_name() const
  {
    return this->section_name_;
  }

  // Return true if the section name has a priority.  This is assumed
  // to be true if it has a dot after the initial dot.
  bool
  has_priority() const
  {
    return this->section_name_.find('.', 1) != std::string::npos;
  }

  // Return the priority.  Believe it or not, gcc encodes the priority
  // differently for .ctors/.dtors and .init_array/.fini_array
  // sections.
  unsigned int
  get_priority() const
  {
    bool is_ctors;
    if (is_prefix_of(".ctors.", this->section_name_.c_str())
	|| is_prefix_of(".dtors.", this->section_name_.c_str()))
      is_ctors = true;
    else if (is_prefix_of(".init_array.", this->section_name_.c_str())
	     || is_prefix_of(".fini_array.", this->section_name_.c_str()))
      is_ctors = false;
    else
      return 0;
    char* end;
    unsigned long prio = strtoul((this->section_name_.c_str()
				  + (is_ctors ? 7 : 12)),
				 &end, 10);
    if (*end != '\0')
      return 0;
    else if (is_ctors)
      return 65535 - prio;
    else
      return prio;
  }

  // Return true if this an input file whose base name matches
  // FILE_NAME.  The base name must have an extension of ".o", and
  // must be exactly FILE_NAME.o or FILE_NAME, one character, ".o".
  // This is to match crtbegin.o as well as crtbeginS.o without
  // getting confused by other possibilities.  Overall matching the
  // file name this way is a dreadful hack, but the GNU linker does it
  // in order to better support gcc, and we need to be compatible.
  bool
  match_file_name(const char* file_name) const
  {
    if (this->input_section_.is_output_section_data())
      return false;
    return Layout::match_file_name(this->input_section_.relobj(), file_name);
  }

  // Returns 1 if THIS should appear before S in section order, -1 if S
  // appears before THIS and 0 if they are not comparable.
  int
  compare_section_ordering(const Input_section_sort_entry& s) const
  {
    unsigned int this_secn_index = this->input_section_.section_order_index();
    unsigned int s_secn_index = s.input_section().section_order_index();
    if (this_secn_index > 0 && s_secn_index > 0)
      {
	if (this_secn_index < s_secn_index)
	  return 1;
	else if (this_secn_index > s_secn_index)
	  return -1;
      }
    return 0;
  }

 private:
  // The Input_section we are sorting.
  Input_section input_section_;
  // The index of this Input_section in the original list.
  unsigned int index_;
  // The section name if there is one.
  std::string section_name_;
};

// Return true if S1 should come before S2 in the output section.

bool
Output_section::Input_section_sort_compare::operator()(
    const Output_section::Input_section_sort_entry& s1,
    const Output_section::Input_section_sort_entry& s2) const
{
  // crtbegin.o must come first.
  bool s1_begin = s1.match_file_name("crtbegin");
  bool s2_begin = s2.match_file_name("crtbegin");
  if (s1_begin || s2_begin)
    {
      if (!s1_begin)
	return false;
      if (!s2_begin)
	return true;
      return s1.index() < s2.index();
    }

  // crtend.o must come last.
  bool s1_end = s1.match_file_name("crtend");
  bool s2_end = s2.match_file_name("crtend");
  if (s1_end || s2_end)
    {
      if (!s1_end)
	return true;
      if (!s2_end)
	return false;
      return s1.index() < s2.index();
    }

  // A section with a priority follows a section without a priority.
  bool s1_has_priority = s1.has_priority();
  bool s2_has_priority = s2.has_priority();
  if (s1_has_priority && !s2_has_priority)
    return false;
  if (!s1_has_priority && s2_has_priority)
    return true;

  // Check if a section order exists for these sections through a section
  // ordering file.  If sequence_num is 0, an order does not exist.
  int sequence_num = s1.compare_section_ordering(s2);
  if (sequence_num != 0)
    return sequence_num == 1;

  // Otherwise we sort by name.
  int compare = s1.section_name().compare(s2.section_name());
  if (compare != 0)
    return compare < 0;

  // Otherwise we keep the input order.
  return s1.index() < s2.index();
}

// Return true if S1 should come before S2 in an .init_array or .fini_array
// output section.

bool
Output_section::Input_section_sort_init_fini_compare::operator()(
    const Output_section::Input_section_sort_entry& s1,
    const Output_section::Input_section_sort_entry& s2) const
{
  // A section without a priority follows a section with a priority.
  // This is the reverse of .ctors and .dtors sections.
  bool s1_has_priority = s1.has_priority();
  bool s2_has_priority = s2.has_priority();
  if (s1_has_priority && !s2_has_priority)
    return true;
  if (!s1_has_priority && s2_has_priority)
    return false;

  // .ctors and .dtors sections without priority come after
  // .init_array and .fini_array sections without priority.
  if (!s1_has_priority
      && (s1.section_name() == ".ctors" || s1.section_name() == ".dtors")
      && s1.section_name() != s2.section_name())
    return false;
  if (!s2_has_priority
      && (s2.section_name() == ".ctors" || s2.section_name() == ".dtors")
      && s2.section_name() != s1.section_name())
    return true;

  // Sort by priority if we can.
  if (s1_has_priority)
    {
      unsigned int s1_prio = s1.get_priority();
      unsigned int s2_prio = s2.get_priority();
      if (s1_prio < s2_prio)
	return true;
      else if (s1_prio > s2_prio)
	return false;
    }

  // Check if a section order exists for these sections through a section
  // ordering file.  If sequence_num is 0, an order does not exist.
  int sequence_num = s1.compare_section_ordering(s2);
  if (sequence_num != 0)
    return sequence_num == 1;

  // Otherwise we sort by name.
  int compare = s1.section_name().compare(s2.section_name());
  if (compare != 0)
    return compare < 0;

  // Otherwise we keep the input order.
  return s1.index() < s2.index();
}

// Return true if S1 should come before S2.  Sections that do not match
// any pattern in the section ordering file are placed ahead of the sections
// that match some pattern.

bool
Output_section::Input_section_sort_section_order_index_compare::operator()(
    const Output_section::Input_section_sort_entry& s1,
    const Output_section::Input_section_sort_entry& s2) const
{
  unsigned int s1_secn_index = s1.input_section().section_order_index();
  unsigned int s2_secn_index = s2.input_section().section_order_index();

  // Keep input order if section ordering cannot determine order.
  if (s1_secn_index == s2_secn_index)
    return s1.index() < s2.index();

  return s1_secn_index < s2_secn_index;
}

// Return true if S1 should come before S2.  This is the sort comparison
// function for .text to sort sections with prefixes
// .text.{unlikely,exit,startup,hot} before other sections.

bool
Output_section::Input_section_sort_section_prefix_special_ordering_compare
  ::operator()(
    const Output_section::Input_section_sort_entry& s1,
    const Output_section::Input_section_sort_entry& s2) const
{
  // Some input section names have special ordering requirements.
  const char *s1_section_name = s1.section_name().c_str();
  const char *s2_section_name = s2.section_name().c_str();
  int o1 = Layout::special_ordering_of_input_section(s1_section_name);
  int o2 = Layout::special_ordering_of_input_section(s2_section_name);
  if (o1 != o2)
    {
      if (o1 < 0)
	return false;
      else if (o2 < 0)
	return true;
      else
	return o1 < o2;
    }
  else if (is_prefix_of(".text.sorted", s1_section_name))
    return strcmp(s1_section_name, s2_section_name) <= 0;

  // Keep input order otherwise.
  return s1.index() < s2.index();
}

// Return true if S1 should come before S2.  This is the sort comparison
// function for sections to sort them by name.

bool
Output_section::Input_section_sort_section_name_compare
  ::operator()(
    const Output_section::Input_section_sort_entry& s1,
    const Output_section::Input_section_sort_entry& s2) const
{
  // We sort by name.
  int compare = s1.section_name().compare(s2.section_name());
  if (compare != 0)
    return compare < 0;

  // Keep input order otherwise.
  return s1.index() < s2.index();
}

// This updates the section order index of input sections according to the
// the order specified in the mapping from Section id to order index.

void
Output_section::update_section_layout(
  const Section_layout_order* order_map)
{
  for (Input_section_list::iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      if (p->is_input_section()
	  || p->is_relaxed_input_section())
	{
	  Relobj* obj = (p->is_input_section()
			 ? p->relobj()
			 : p->relaxed_input_section()->relobj());
	  unsigned int shndx = p->shndx();
	  Section_layout_order::const_iterator it
	    = order_map->find(Section_id(obj, shndx));
	  if (it == order_map->end())
	    continue;
	  unsigned int section_order_index = it->second;
	  if (section_order_index != 0)
	    {
	      p->set_section_order_index(section_order_index);
	      this->set_input_section_order_specified();
	    }
	}
    }
}

// Sort the input sections attached to an output section.

void
Output_section::sort_attached_input_sections()
{
  if (this->attached_input_sections_are_sorted_)
    return;

  if (this->checkpoint_ != NULL
      && !this->checkpoint_->input_sections_saved())
    this->checkpoint_->save_input_sections();

  // The only thing we know about an input section is the object and
  // the section index.  We need the section name.  Recomputing this
  // is slow but this is an unusual case.  If this becomes a speed
  // problem we can cache the names as required in Layout::layout.

  // We start by building a larger vector holding a copy of each
  // Input_section, plus its current index in the list and its name.
  std::vector<Input_section_sort_entry> sort_list;

  unsigned int i = 0;
  for (Input_section_list::iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p, ++i)
      sort_list.push_back(Input_section_sort_entry(*p, i,
			    this->must_sort_attached_input_sections(),
			    this->name()));

  // Sort the input sections.
  if (this->must_sort_attached_input_sections())
    {
      if (this->type() == elfcpp::SHT_PREINIT_ARRAY
	  || this->type() == elfcpp::SHT_INIT_ARRAY
	  || this->type() == elfcpp::SHT_FINI_ARRAY)
	std::sort(sort_list.begin(), sort_list.end(),
		  Input_section_sort_init_fini_compare());
      else if (strcmp(parameters->options().sort_section(), "name") == 0)
	std::sort(sort_list.begin(), sort_list.end(),
		  Input_section_sort_section_name_compare());
      else if (strcmp(this->name(), ".text") == 0)
	std::sort(sort_list.begin(), sort_list.end(),
		  Input_section_sort_section_prefix_special_ordering_compare());
      else
	std::sort(sort_list.begin(), sort_list.end(),
		  Input_section_sort_compare());
    }
  else
    {
      gold_assert(this->input_section_order_specified());
      std::sort(sort_list.begin(), sort_list.end(),
		Input_section_sort_section_order_index_compare());
    }

  // Copy the sorted input sections back to our list.
  this->input_sections_.clear();
  for (std::vector<Input_section_sort_entry>::iterator p = sort_list.begin();
       p != sort_list.end();
       ++p)
    this->input_sections_.push_back(p->input_section());
  sort_list.clear();

  // Remember that we sorted the input sections, since we might get
  // called again.
  this->attached_input_sections_are_sorted_ = true;
}

// Write the section header to *OSHDR.

template<int size, bool big_endian>
void
Output_section::write_header(const Layout* layout,
			     const Stringpool* secnamepool,
			     elfcpp::Shdr_write<size, big_endian>* oshdr) const
{
  oshdr->put_sh_name(secnamepool->get_offset(this->name_));
  oshdr->put_sh_type(this->type_);

  elfcpp::Elf_Xword flags = this->flags_;
  if (this->info_section_ != NULL && this->info_uses_section_index_)
    flags |= elfcpp::SHF_INFO_LINK;
  oshdr->put_sh_flags(flags);

  oshdr->put_sh_addr(this->address());
  oshdr->put_sh_offset(this->offset());
  oshdr->put_sh_size(this->data_size());
  if (this->link_section_ != NULL)
    oshdr->put_sh_link(this->link_section_->out_shndx());
  else if (this->should_link_to_symtab_)
    oshdr->put_sh_link(layout->symtab_section_shndx());
  else if (this->should_link_to_dynsym_)
    oshdr->put_sh_link(layout->dynsym_section()->out_shndx());
  else
    oshdr->put_sh_link(this->link_);

  elfcpp::Elf_Word info;
  if (this->info_section_ != NULL)
    {
      if (this->info_uses_section_index_)
	info = this->info_section_->out_shndx();
      else
	info = this->info_section_->symtab_index();
    }
  else if (this->info_symndx_ != NULL)
    info = this->info_symndx_->symtab_index();
  else
    info = this->info_;
  oshdr->put_sh_info(info);

  oshdr->put_sh_addralign(this->addralign_);
  oshdr->put_sh_entsize(this->entsize_);
}

// Write out the data.  For input sections the data is written out by
// Object::relocate, but we have to handle Output_section_data objects
// here.

void
Output_section::do_write(Output_file* of)
{
  gold_assert(!this->requires_postprocessing());

  // If the target performs relaxation, we delay filler generation until now.
  gold_assert(!this->generate_code_fills_at_write_ || this->fills_.empty());

  off_t output_section_file_offset = this->offset();
  for (Fill_list::iterator p = this->fills_.begin();
       p != this->fills_.end();
       ++p)
    {
      std::string fill_data(parameters->target().code_fill(p->length()));
      of->write(output_section_file_offset + p->section_offset(),
		fill_data.data(), fill_data.size());
    }

  off_t off = this->offset() + this->first_input_offset_;
  for (Input_section_list::iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      off_t aligned_off = align_address(off, p->addralign());
      if (this->generate_code_fills_at_write_ && (off != aligned_off))
	{
	  size_t fill_len = aligned_off - off;
	  std::string fill_data(parameters->target().code_fill(fill_len));
	  of->write(off, fill_data.data(), fill_data.size());
	}

      p->write(of);
      off = aligned_off + p->data_size();
    }

  // For incremental links, fill in unused chunks in debug sections
  // with dummy compilation unit headers.
  if (this->free_space_fill_ != NULL)
    {
      for (Free_list::Const_iterator p = this->free_list_.begin();
	   p != this->free_list_.end();
	   ++p)
	{
	  off_t off = p->start_;
	  size_t len = p->end_ - off;
	  this->free_space_fill_->write(of, this->offset() + off, len);
	}
      if (this->patch_space_ > 0)
	{
	  off_t off = this->current_data_size_for_child() - this->patch_space_;
	  this->free_space_fill_->write(of, this->offset() + off,
					this->patch_space_);
	}
    }
}

// If a section requires postprocessing, create the buffer to use.

void
Output_section::create_postprocessing_buffer()
{
  gold_assert(this->requires_postprocessing());

  if (this->postprocessing_buffer_ != NULL)
    return;

  if (!this->input_sections_.empty())
    {
      off_t off = this->first_input_offset_;
      for (Input_section_list::iterator p = this->input_sections_.begin();
	   p != this->input_sections_.end();
	   ++p)
	{
	  off = align_address(off, p->addralign());
	  p->finalize_data_size();
	  off += p->data_size();
	}
      this->set_current_data_size_for_child(off);
    }

  off_t buffer_size = this->current_data_size_for_child();
  this->postprocessing_buffer_ = new unsigned char[buffer_size];
}

// Write all the data of an Output_section into the postprocessing
// buffer.  This is used for sections which require postprocessing,
// such as compression.  Input sections are handled by
// Object::Relocate.

void
Output_section::write_to_postprocessing_buffer()
{
  gold_assert(this->requires_postprocessing());

  // If the target performs relaxation, we delay filler generation until now.
  gold_assert(!this->generate_code_fills_at_write_ || this->fills_.empty());

  unsigned char* buffer = this->postprocessing_buffer();
  for (Fill_list::iterator p = this->fills_.begin();
       p != this->fills_.end();
       ++p)
    {
      std::string fill_data(parameters->target().code_fill(p->length()));
      memcpy(buffer + p->section_offset(), fill_data.data(),
	     fill_data.size());
    }

  off_t off = this->first_input_offset_;
  for (Input_section_list::iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      off_t aligned_off = align_address(off, p->addralign());
      if (this->generate_code_fills_at_write_ && (off != aligned_off))
	{
	  size_t fill_len = aligned_off - off;
	  std::string fill_data(parameters->target().code_fill(fill_len));
	  memcpy(buffer + off, fill_data.data(), fill_data.size());
	}

      p->write_to_buffer(buffer + aligned_off);
      off = aligned_off + p->data_size();
    }
}

// Get the input sections for linker script processing.  We leave
// behind the Output_section_data entries.  Note that this may be
// slightly incorrect for merge sections.  We will leave them behind,
// but it is possible that the script says that they should follow
// some other input sections, as in:
//    .rodata { *(.rodata) *(.rodata.cst*) }
// For that matter, we don't handle this correctly:
//    .rodata { foo.o(.rodata.cst*) *(.rodata.cst*) }
// With luck this will never matter.

uint64_t
Output_section::get_input_sections(
    uint64_t address,
    const std::string& fill,
    std::list<Input_section>* input_sections)
{
  if (this->checkpoint_ != NULL
      && !this->checkpoint_->input_sections_saved())
    this->checkpoint_->save_input_sections();

  // Invalidate fast look-up maps.
  this->lookup_maps_->invalidate();

  uint64_t orig_address = address;

  address = align_address(address, this->addralign());

  Input_section_list remaining;
  for (Input_section_list::iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      if (p->is_input_section()
	  || p->is_relaxed_input_section()
	  || p->is_merge_section())
	input_sections->push_back(*p);
      else
	{
	  uint64_t aligned_address = align_address(address, p->addralign());
	  if (aligned_address != address && !fill.empty())
	    {
	      section_size_type length =
		convert_to_section_size_type(aligned_address - address);
	      std::string this_fill;
	      this_fill.reserve(length);
	      while (this_fill.length() + fill.length() <= length)
		this_fill += fill;
	      if (this_fill.length() < length)
		this_fill.append(fill, 0, length - this_fill.length());

	      Output_section_data* posd = new Output_data_const(this_fill, 0);
	      remaining.push_back(Input_section(posd));
	    }
	  address = aligned_address;

	  remaining.push_back(*p);

	  p->finalize_data_size();
	  address += p->data_size();
	}
    }

  this->input_sections_.swap(remaining);
  this->first_input_offset_ = 0;

  uint64_t data_size = address - orig_address;
  this->set_current_data_size_for_child(data_size);
  return data_size;
}

// Add a script input section.  SIS is an Output_section::Input_section,
// which can be either a plain input section or a special input section like
// a relaxed input section.  For a special input section, its size must be
// finalized.

void
Output_section::add_script_input_section(const Input_section& sis)
{
  uint64_t data_size = sis.data_size();
  uint64_t addralign = sis.addralign();
  if (addralign > this->addralign_)
    this->addralign_ = addralign;

  off_t offset_in_section = this->current_data_size_for_child();
  off_t aligned_offset_in_section = align_address(offset_in_section,
						  addralign);

  this->set_current_data_size_for_child(aligned_offset_in_section
					+ data_size);

  this->input_sections_.push_back(sis);

  // Update fast lookup maps if necessary.
  if (this->lookup_maps_->is_valid())
    {
      if (sis.is_relaxed_input_section())
	{
	  Output_relaxed_input_section* poris = sis.relaxed_input_section();
	  this->lookup_maps_->add_relaxed_input_section(poris->relobj(),
							poris->shndx(), poris);
	}
    }
}

// Save states for relaxation.

void
Output_section::save_states()
{
  gold_assert(this->checkpoint_ == NULL);
  Checkpoint_output_section* checkpoint =
    new Checkpoint_output_section(this->addralign_, this->flags_,
				  this->input_sections_,
				  this->first_input_offset_,
				  this->attached_input_sections_are_sorted_);
  this->checkpoint_ = checkpoint;
  gold_assert(this->fills_.empty());
}

void
Output_section::discard_states()
{
  gold_assert(this->checkpoint_ != NULL);
  delete this->checkpoint_;
  this->checkpoint_ = NULL;
  gold_assert(this->fills_.empty());

  // Simply invalidate the fast lookup maps since we do not keep
  // track of them.
  this->lookup_maps_->invalidate();
}

void
Output_section::restore_states()
{
  gold_assert(this->checkpoint_ != NULL);
  Checkpoint_output_section* checkpoint = this->checkpoint_;

  this->addralign_ = checkpoint->addralign();
  this->flags_ = checkpoint->flags();
  this->first_input_offset_ = checkpoint->first_input_offset();

  if (!checkpoint->input_sections_saved())
    {
      // If we have not copied the input sections, just resize it.
      size_t old_size = checkpoint->input_sections_size();
      gold_assert(this->input_sections_.size() >= old_size);
      this->input_sections_.resize(old_size);
    }
  else
    {
      // We need to copy the whole list.  This is not efficient for
      // extremely large output with hundreads of thousands of input
      // objects.  We may need to re-think how we should pass sections
      // to scripts.
      this->input_sections_ = *checkpoint->input_sections();
    }

  this->attached_input_sections_are_sorted_ =
    checkpoint->attached_input_sections_are_sorted();

  // Simply invalidate the fast lookup maps since we do not keep
  // track of them.
  this->lookup_maps_->invalidate();
}

// Update the section offsets of input sections in this.  This is required if
// relaxation causes some input sections to change sizes.

void
Output_section::adjust_section_offsets()
{
  if (!this->section_offsets_need_adjustment_)
    return;

  off_t off = 0;
  for (Input_section_list::iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    {
      off = align_address(off, p->addralign());
      if (p->is_input_section())
	p->relobj()->set_section_offset(p->shndx(), off);
      off += p->data_size();
    }

  this->section_offsets_need_adjustment_ = false;
}

// Print to the map file.

void
Output_section::do_print_to_mapfile(Mapfile* mapfile) const
{
  mapfile->print_output_section(this);

  for (Input_section_list::const_iterator p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    p->print_to_mapfile(mapfile);
}

// Print stats for merge sections to stderr.

void
Output_section::print_merge_stats()
{
  Input_section_list::iterator p;
  for (p = this->input_sections_.begin();
       p != this->input_sections_.end();
       ++p)
    p->print_merge_stats(this->name_);
}

// Set a fixed layout for the section.  Used for incremental update links.

void
Output_section::set_fixed_layout(uint64_t sh_addr, off_t sh_offset,
				 off_t sh_size, uint64_t sh_addralign)
{
  this->addralign_ = sh_addralign;
  this->set_current_data_size(sh_size);
  if ((this->flags_ & elfcpp::SHF_ALLOC) != 0)
    this->set_address(sh_addr);
  this->set_file_offset(sh_offset);
  this->finalize_data_size();
  this->free_list_.init(sh_size, false);
  this->has_fixed_layout_ = true;
}

// Reserve space within the fixed layout for the section.  Used for
// incremental update links.

void
Output_section::reserve(uint64_t sh_offset, uint64_t sh_size)
{
  this->free_list_.remove(sh_offset, sh_offset + sh_size);
}

// Allocate space from the free list for the section.  Used for
// incremental update links.

off_t
Output_section::allocate(off_t len, uint64_t addralign)
{
  return this->free_list_.allocate(len, addralign, 0);
}

// Output segment methods.

Output_segment::Output_segment(elfcpp::Elf_Word type, elfcpp::Elf_Word flags)
  : vaddr_(0),
    paddr_(0),
    memsz_(0),
    align_(0),
    max_align_(0),
    min_p_align_(0),
    offset_(0),
    filesz_(0),
    type_(type),
    flags_(flags),
    is_max_align_known_(false),
    are_addresses_set_(false),
    is_large_data_segment_(false),
    is_unique_segment_(false)
{
  // The ELF ABI specifies that a PT_TLS segment always has PF_R as
  // the flags.
  if (type == elfcpp::PT_TLS)
    this->flags_ = elfcpp::PF_R;
}

// Add an Output_section to a PT_LOAD Output_segment.

void
Output_segment::add_output_section_to_load(Layout* layout,
					   Output_section* os,
					   elfcpp::Elf_Word seg_flags)
{
  gold_assert(this->type() == elfcpp::PT_LOAD);
  gold_assert((os->flags() & elfcpp::SHF_ALLOC) != 0);
  gold_assert(!this->is_max_align_known_);
  gold_assert(os->is_large_data_section() == this->is_large_data_segment());

  this->update_flags_for_output_section(seg_flags);

  // We don't want to change the ordering if we have a linker script
  // with a SECTIONS clause.
  Output_section_order order = os->order();
  if (layout->script_options()->saw_sections_clause())
    order = static_cast<Output_section_order>(0);
  else
    gold_assert(order != ORDER_INVALID);

  this->output_lists_[order].push_back(os);
}

// Add an Output_section to a non-PT_LOAD Output_segment.

void
Output_segment::add_output_section_to_nonload(Output_section* os,
					      elfcpp::Elf_Word seg_flags)
{
  gold_assert(this->type() != elfcpp::PT_LOAD);
  gold_assert((os->flags() & elfcpp::SHF_ALLOC) != 0);
  gold_assert(!this->is_max_align_known_);

  this->update_flags_for_output_section(seg_flags);

  this->output_lists_[0].push_back(os);
}

// Remove an Output_section from this segment.  It is an error if it
// is not present.

void
Output_segment::remove_output_section(Output_section* os)
{
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    {
      Output_data_list* pdl = &this->output_lists_[i];
      for (Output_data_list::iterator p = pdl->begin(); p != pdl->end(); ++p)
	{
	  if (*p == os)
	    {
	      pdl->erase(p);
	      return;
	    }
	}
    }
  gold_unreachable();
}

// Add an Output_data (which need not be an Output_section) to the
// start of a segment.

void
Output_segment::add_initial_output_data(Output_data* od)
{
  gold_assert(!this->is_max_align_known_);
  Output_data_list::iterator p = this->output_lists_[0].begin();
  this->output_lists_[0].insert(p, od);
}

// Return true if this segment has any sections which hold actual
// data, rather than being a BSS section.

bool
Output_segment::has_any_data_sections() const
{
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    {
      const Output_data_list* pdl = &this->output_lists_[i];
      for (Output_data_list::const_iterator p = pdl->begin();
	   p != pdl->end();
	   ++p)
	{
	  if (!(*p)->is_section())
	    return true;
	  if ((*p)->output_section()->type() != elfcpp::SHT_NOBITS)
	    return true;
	}
    }
  return false;
}

// Return whether the first data section (not counting TLS sections)
// is a relro section.

bool
Output_segment::is_first_section_relro() const
{
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    {
      if (i == static_cast<int>(ORDER_TLS_BSS))
	continue;
      const Output_data_list* pdl = &this->output_lists_[i];
      if (!pdl->empty())
	{
	  Output_data* p = pdl->front();
	  return p->is_section() && p->output_section()->is_relro();
	}
    }
  return false;
}

// Return the maximum alignment of the Output_data in Output_segment.

uint64_t
Output_segment::maximum_alignment()
{
  if (!this->is_max_align_known_)
    {
      for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
	{
	  const Output_data_list* pdl = &this->output_lists_[i];
	  uint64_t addralign = Output_segment::maximum_alignment_list(pdl);
	  if (addralign > this->max_align_)
	    this->max_align_ = addralign;
	}
      this->is_max_align_known_ = true;
    }

  return this->max_align_;
}

// Return the maximum alignment of a list of Output_data.

uint64_t
Output_segment::maximum_alignment_list(const Output_data_list* pdl)
{
  uint64_t ret = 0;
  for (Output_data_list::const_iterator p = pdl->begin();
       p != pdl->end();
       ++p)
    {
      uint64_t addralign = (*p)->addralign();
      if (addralign > ret)
	ret = addralign;
    }
  return ret;
}

// Return whether this segment has any dynamic relocs.

bool
Output_segment::has_dynamic_reloc() const
{
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    if (this->has_dynamic_reloc_list(&this->output_lists_[i]))
      return true;
  return false;
}

// Return whether this Output_data_list has any dynamic relocs.

bool
Output_segment::has_dynamic_reloc_list(const Output_data_list* pdl) const
{
  for (Output_data_list::const_iterator p = pdl->begin();
       p != pdl->end();
       ++p)
    if ((*p)->has_dynamic_reloc())
      return true;
  return false;
}

// Set the section addresses for an Output_segment.  If RESET is true,
// reset the addresses first.  ADDR is the address and *POFF is the
// file offset.  Set the section indexes starting with *PSHNDX.
// INCREASE_RELRO is the size of the portion of the first non-relro
// section that should be included in the PT_GNU_RELRO segment.
// If this segment has relro sections, and has been aligned for
// that purpose, set *HAS_RELRO to TRUE.  Return the address of
// the immediately following segment.  Update *HAS_RELRO, *POFF,
// and *PSHNDX.

uint64_t
Output_segment::set_section_addresses(const Target* target,
				      Layout* layout, bool reset,
				      uint64_t addr,
				      unsigned int* increase_relro,
				      bool* has_relro,
				      off_t* poff,
				      unsigned int* pshndx)
{
  gold_assert(this->type_ == elfcpp::PT_LOAD);

  uint64_t last_relro_pad = 0;
  off_t orig_off = *poff;

  bool in_tls = false;

  // If we have relro sections, we need to pad forward now so that the
  // relro sections plus INCREASE_RELRO end on an abi page boundary.
  if (parameters->options().relro()
      && this->is_first_section_relro()
      && (!this->are_addresses_set_ || reset))
    {
      uint64_t relro_size = 0;
      off_t off = *poff;
      uint64_t max_align = 0;
      for (int i = 0; i <= static_cast<int>(ORDER_RELRO_LAST); ++i)
	{
	  Output_data_list* pdl = &this->output_lists_[i];
	  Output_data_list::iterator p;
	  for (p = pdl->begin(); p != pdl->end(); ++p)
	    {
	      if (!(*p)->is_section())
		break;
	      uint64_t align = (*p)->addralign();
	      if (align > max_align)
		max_align = align;
	      if ((*p)->is_section_flag_set(elfcpp::SHF_TLS))
		in_tls = true;
	      else if (in_tls)
		{
		  // Align the first non-TLS section to the alignment
		  // of the TLS segment.
		  align = max_align;
		  in_tls = false;
		}
	      // Ignore the size of the .tbss section.
	      if ((*p)->is_section_flag_set(elfcpp::SHF_TLS)
		  && (*p)->is_section_type(elfcpp::SHT_NOBITS))
		continue;
	      relro_size = align_address(relro_size, align);
	      if ((*p)->is_address_valid())
		relro_size += (*p)->data_size();
	      else
		{
		  // FIXME: This could be faster.
		  (*p)->set_address_and_file_offset(relro_size,
						    relro_size);
		  relro_size += (*p)->data_size();
		  (*p)->reset_address_and_file_offset();
		}
	    }
	  if (p != pdl->end())
	    break;
	}
      relro_size += *increase_relro;
      // Pad the total relro size to a multiple of the maximum
      // section alignment seen.
      uint64_t aligned_size = align_address(relro_size, max_align);
      // Note the amount of padding added after the last relro section.
      last_relro_pad = aligned_size - relro_size;
      *has_relro = true;

      uint64_t page_align = parameters->target().abi_pagesize();

      // Align to offset N such that (N + RELRO_SIZE) % PAGE_ALIGN == 0.
      uint64_t desired_align = page_align - (aligned_size % page_align);
      if (desired_align < off % page_align)
	off += page_align;
      off += desired_align - off % page_align;
      addr += off - orig_off;
      orig_off = off;
      *poff = off;
    }

  if (!reset && this->are_addresses_set_)
    {
      gold_assert(this->paddr_ == addr);
      addr = this->vaddr_;
    }
  else
    {
      this->vaddr_ = addr;
      this->paddr_ = addr;
      this->are_addresses_set_ = true;
    }

  in_tls = false;

  this->offset_ = orig_off;

  off_t off = 0;
  off_t foff = *poff;
  uint64_t ret = 0;
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    {
      if (i == static_cast<int>(ORDER_RELRO_LAST))
	{
	  *poff += last_relro_pad;
	  foff += last_relro_pad;
	  addr += last_relro_pad;
	  if (this->output_lists_[i].empty())
	    {
	      // If there is nothing in the ORDER_RELRO_LAST list,
	      // the padding will occur at the end of the relro
	      // segment, and we need to add it to *INCREASE_RELRO.
	      *increase_relro += last_relro_pad;
	    }
	}
      addr = this->set_section_list_addresses(layout, reset,
					      &this->output_lists_[i],
					      addr, poff, &foff, pshndx,
					      &in_tls);

      // FOFF tracks the last offset used for the file image,
      // and *POFF tracks the last offset used for the memory image.
      // When not using a linker script, bss sections should all
      // be processed in the ORDER_SMALL_BSS and later buckets.
      gold_assert(*poff == foff
		  || i == static_cast<int>(ORDER_TLS_BSS)
		  || i >= static_cast<int>(ORDER_SMALL_BSS)
		  || layout->script_options()->saw_sections_clause());

      this->filesz_ = foff - orig_off;
      off = foff;

      ret = addr;
    }

  // If the last section was a TLS section, align upward to the
  // alignment of the TLS segment, so that the overall size of the TLS
  // segment is aligned.
  if (in_tls)
    {
      uint64_t segment_align = layout->tls_segment()->maximum_alignment();
      *poff = align_address(*poff, segment_align);
    }

  this->memsz_ = *poff - orig_off;

  // Ignore the file offset adjustments made by the BSS Output_data
  // objects.
  *poff = off;

  // If code segments must contain only code, and this code segment is
  // page-aligned in the file, then fill it out to a whole page with
  // code fill (the tail of the segment will not be within any section).
  // Thus the entire code segment can be mapped from the file as whole
  // pages and that mapping will contain only valid instructions.
  if (target->isolate_execinstr() && (this->flags() & elfcpp::PF_X) != 0)
    {
      uint64_t abi_pagesize = target->abi_pagesize();
      if (orig_off % abi_pagesize == 0 && off % abi_pagesize != 0)
	{
	  size_t fill_size = abi_pagesize - (off % abi_pagesize);

	  std::string fill_data;
	  if (target->has_code_fill())
	    fill_data = target->code_fill(fill_size);
	  else
	    fill_data.resize(fill_size); // Zero fill.

	  Output_data_const* fill = new Output_data_const(fill_data, 0);
	  fill->set_address(this->vaddr_ + this->memsz_);
	  fill->set_file_offset(off);
	  layout->add_relax_output(fill);

	  off += fill_size;
	  gold_assert(off % abi_pagesize == 0);
	  ret += fill_size;
	  gold_assert(ret % abi_pagesize == 0);

	  gold_assert((uint64_t) this->filesz_ == this->memsz_);
	  this->memsz_ = this->filesz_ += fill_size;

	  *poff = off;
	}
    }

  return ret;
}

// Set the addresses and file offsets in a list of Output_data
// structures.

uint64_t
Output_segment::set_section_list_addresses(Layout* layout, bool reset,
					   Output_data_list* pdl,
					   uint64_t addr, off_t* poff,
					   off_t* pfoff,
					   unsigned int* pshndx,
					   bool* in_tls)
{
  off_t startoff = *poff;
  // For incremental updates, we may allocate non-fixed sections from
  // free space in the file.  This keeps track of the high-water mark.
  off_t maxoff = startoff;

  off_t off = startoff;
  off_t foff = *pfoff;
  for (Output_data_list::iterator p = pdl->begin();
       p != pdl->end();
       ++p)
    {
      bool is_bss = (*p)->is_section_type(elfcpp::SHT_NOBITS);
      bool is_tls = (*p)->is_section_flag_set(elfcpp::SHF_TLS);

      if (reset)
	(*p)->reset_address_and_file_offset();

      // When doing an incremental update or when using a linker script,
      // the section will most likely already have an address.
      if (!(*p)->is_address_valid())
	{
	  uint64_t align = (*p)->addralign();

	  if (is_tls)
	    {
	      // Give the first TLS section the alignment of the
	      // entire TLS segment.  Otherwise the TLS segment as a
	      // whole may be misaligned.
	      if (!*in_tls)
		{
		  Output_segment* tls_segment = layout->tls_segment();
		  gold_assert(tls_segment != NULL);
		  uint64_t segment_align = tls_segment->maximum_alignment();
		  gold_assert(segment_align >= align);
		  align = segment_align;

		  *in_tls = true;
		}
	    }
	  else
	    {
	      // If this is the first section after the TLS segment,
	      // align it to at least the alignment of the TLS
	      // segment, so that the size of the overall TLS segment
	      // is aligned.
	      if (*in_tls)
		{
		  uint64_t segment_align =
		      layout->tls_segment()->maximum_alignment();
		  if (segment_align > align)
		    align = segment_align;

		  *in_tls = false;
		}
	    }

	  if (!parameters->incremental_update())
	    {
	      gold_assert(off == foff || is_bss);
	      off = align_address(off, align);
	      if (is_tls || !is_bss)
		foff = off;
	      (*p)->set_address_and_file_offset(addr + (off - startoff), foff);
	    }
	  else
	    {
	      // Incremental update: allocate file space from free list.
	      (*p)->pre_finalize_data_size();
	      off_t current_size = (*p)->current_data_size();
	      off = layout->allocate(current_size, align, startoff);
	      foff = off;
	      if (off == -1)
		{
		  gold_assert((*p)->output_section() != NULL);
		  gold_fallback(_("out of patch space for section %s; "
				  "relink with --incremental-full"),
				(*p)->output_section()->name());
		}
	      (*p)->set_address_and_file_offset(addr + (off - startoff), foff);
	      if ((*p)->data_size() > current_size)
		{
		  gold_assert((*p)->output_section() != NULL);
		  gold_fallback(_("%s: section changed size; "
				  "relink with --incremental-full"),
				(*p)->output_section()->name());
		}
	    }
	}
      else if (parameters->incremental_update())
	{
	  // For incremental updates, use the fixed offset for the
	  // high-water mark computation.
	  off = (*p)->offset();
	  foff = off;
	}
      else
	{
	  // The script may have inserted a skip forward, but it
	  // better not have moved backward.
	  if ((*p)->address() >= addr + (off - startoff))
	    {
	      if (!is_bss && off > foff)
	        gold_warning(_("script places BSS section in the middle "
			       "of a LOAD segment; space will be allocated "
			       "in the file"));
	      off += (*p)->address() - (addr + (off - startoff));
	      if (is_tls || !is_bss)
		foff = off;
	    }
	  else
	    {
	      if (!layout->script_options()->saw_sections_clause())
		gold_unreachable();
	      else
		{
		  Output_section* os = (*p)->output_section();

		  // Cast to unsigned long long to avoid format warnings.
		  unsigned long long previous_dot =
		    static_cast<unsigned long long>(addr + (off - startoff));
		  unsigned long long dot =
		    static_cast<unsigned long long>((*p)->address());

		  if (os == NULL)
		    gold_error(_("dot moves backward in linker script "
				 "from 0x%llx to 0x%llx"), previous_dot, dot);
		  else
		    gold_error(_("address of section '%s' moves backward "
				 "from 0x%llx to 0x%llx"),
			       os->name(), previous_dot, dot);
		}
	    }
	  (*p)->set_file_offset(foff);
	  (*p)->finalize_data_size();
	}

      if (parameters->incremental_update())
	gold_debug(DEBUG_INCREMENTAL,
		   "set_section_list_addresses: %08lx %08lx %s",
		   static_cast<long>(off),
		   static_cast<long>((*p)->data_size()),
		   ((*p)->output_section() != NULL
		    ? (*p)->output_section()->name() : "(special)"));

      // We want to ignore the size of a SHF_TLS SHT_NOBITS
      // section.  Such a section does not affect the size of a
      // PT_LOAD segment.
      if (!is_tls || !is_bss)
	off += (*p)->data_size();

      // We don't allocate space in the file for SHT_NOBITS sections,
      // unless a script has force-placed one in the middle of a segment.
      if (!is_bss)
	foff = off;

      if (off > maxoff)
	maxoff = off;

      if ((*p)->is_section())
	{
	  (*p)->set_out_shndx(*pshndx);
	  ++*pshndx;
	}
    }

  *poff = maxoff;
  *pfoff = foff;
  return addr + (maxoff - startoff);
}

// For a non-PT_LOAD segment, set the offset from the sections, if
// any.  Add INCREASE to the file size and the memory size.

void
Output_segment::set_offset(unsigned int increase)
{
  gold_assert(this->type_ != elfcpp::PT_LOAD);

  gold_assert(!this->are_addresses_set_);

  // A non-load section only uses output_lists_[0].

  Output_data_list* pdl = &this->output_lists_[0];

  if (pdl->empty())
    {
      gold_assert(increase == 0);
      this->vaddr_ = 0;
      this->paddr_ = 0;
      this->are_addresses_set_ = true;
      this->memsz_ = 0;
      this->min_p_align_ = 0;
      this->offset_ = 0;
      this->filesz_ = 0;
      return;
    }

  // Find the first and last section by address.
  const Output_data* first = NULL;
  const Output_data* last_data = NULL;
  const Output_data* last_bss = NULL;
  for (Output_data_list::const_iterator p = pdl->begin();
       p != pdl->end();
       ++p)
    {
      if (first == NULL
	  || (*p)->address() < first->address()
	  || ((*p)->address() == first->address()
	      && (*p)->data_size() < first->data_size()))
	first = *p;
      const Output_data** plast;
      if ((*p)->is_section()
	  && (*p)->output_section()->type() == elfcpp::SHT_NOBITS)
	plast = &last_bss;
      else
	plast = &last_data;
      if (*plast == NULL
	  || (*p)->address() > (*plast)->address()
	  || ((*p)->address() == (*plast)->address()
	      && (*p)->data_size() > (*plast)->data_size()))
	*plast = *p;
    }

  this->vaddr_ = first->address();
  this->paddr_ = (first->has_load_address()
		  ? first->load_address()
		  : this->vaddr_);
  this->are_addresses_set_ = true;
  this->offset_ = first->offset();

  if (last_data == NULL)
    this->filesz_ = 0;
  else
    this->filesz_ = (last_data->address()
		     + last_data->data_size()
		     - this->vaddr_);

  const Output_data* last = last_bss != NULL ? last_bss : last_data;
  this->memsz_ = (last->address()
		  + last->data_size()
		  - this->vaddr_);

  this->filesz_ += increase;
  this->memsz_ += increase;

  // If this is a RELRO segment, verify that the segment ends at a
  // page boundary.
  if (this->type_ == elfcpp::PT_GNU_RELRO)
    {
      uint64_t page_align = parameters->target().abi_pagesize();
      uint64_t segment_end = this->vaddr_ + this->memsz_;
      if (parameters->incremental_update())
	{
	  // The INCREASE_RELRO calculation is bypassed for an incremental
	  // update, so we need to adjust the segment size manually here.
	  segment_end = align_address(segment_end, page_align);
	  this->memsz_ = segment_end - this->vaddr_;
	}
      else
	gold_assert(segment_end == align_address(segment_end, page_align));
    }

  // If this is a TLS segment, align the memory size.  The code in
  // set_section_list ensures that the section after the TLS segment
  // is aligned to give us room.
  if (this->type_ == elfcpp::PT_TLS)
    {
      uint64_t segment_align = this->maximum_alignment();
      gold_assert(this->vaddr_ == align_address(this->vaddr_, segment_align));
      this->memsz_ = align_address(this->memsz_, segment_align);
    }
}

// Set the TLS offsets of the sections in the PT_TLS segment.

void
Output_segment::set_tls_offsets()
{
  gold_assert(this->type_ == elfcpp::PT_TLS);

  for (Output_data_list::iterator p = this->output_lists_[0].begin();
       p != this->output_lists_[0].end();
       ++p)
    (*p)->set_tls_offset(this->vaddr_);
}

// Return the first section.

Output_section*
Output_segment::first_section() const
{
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    {
      const Output_data_list* pdl = &this->output_lists_[i];
      for (Output_data_list::const_iterator p = pdl->begin();
	   p != pdl->end();
	   ++p)
	{
	  if ((*p)->is_section())
	    return (*p)->output_section();
	}
    }
  return NULL;
}

// Return the number of Output_sections in an Output_segment.

unsigned int
Output_segment::output_section_count() const
{
  unsigned int ret = 0;
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    ret += this->output_section_count_list(&this->output_lists_[i]);
  return ret;
}

// Return the number of Output_sections in an Output_data_list.

unsigned int
Output_segment::output_section_count_list(const Output_data_list* pdl) const
{
  unsigned int count = 0;
  for (Output_data_list::const_iterator p = pdl->begin();
       p != pdl->end();
       ++p)
    {
      if ((*p)->is_section())
	++count;
    }
  return count;
}

// Return the section attached to the list segment with the lowest
// load address.  This is used when handling a PHDRS clause in a
// linker script.

Output_section*
Output_segment::section_with_lowest_load_address() const
{
  Output_section* found = NULL;
  uint64_t found_lma = 0;
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    this->lowest_load_address_in_list(&this->output_lists_[i], &found,
				      &found_lma);
  return found;
}

// Look through a list for a section with a lower load address.

void
Output_segment::lowest_load_address_in_list(const Output_data_list* pdl,
					    Output_section** found,
					    uint64_t* found_lma) const
{
  for (Output_data_list::const_iterator p = pdl->begin();
       p != pdl->end();
       ++p)
    {
      if (!(*p)->is_section())
	continue;
      Output_section* os = static_cast<Output_section*>(*p);
      uint64_t lma = (os->has_load_address()
		      ? os->load_address()
		      : os->address());
      if (*found == NULL || lma < *found_lma)
	{
	  *found = os;
	  *found_lma = lma;
	}
    }
}

// Write the segment data into *OPHDR.

template<int size, bool big_endian>
void
Output_segment::write_header(elfcpp::Phdr_write<size, big_endian>* ophdr)
{
  ophdr->put_p_type(this->type_);
  ophdr->put_p_offset(this->offset_);
  ophdr->put_p_vaddr(this->vaddr_);
  ophdr->put_p_paddr(this->paddr_);
  ophdr->put_p_filesz(this->filesz_);
  ophdr->put_p_memsz(this->memsz_);
  ophdr->put_p_flags(this->flags_);
  ophdr->put_p_align(std::max(this->min_p_align_, this->maximum_alignment()));
}

// Write the section headers into V.

template<int size, bool big_endian>
unsigned char*
Output_segment::write_section_headers(const Layout* layout,
				      const Stringpool* secnamepool,
				      unsigned char* v,
				      unsigned int* pshndx) const
{
  // Every section that is attached to a segment must be attached to a
  // PT_LOAD segment, so we only write out section headers for PT_LOAD
  // segments.
  if (this->type_ != elfcpp::PT_LOAD)
    return v;

  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    {
      const Output_data_list* pdl = &this->output_lists_[i];
      v = this->write_section_headers_list<size, big_endian>(layout,
							     secnamepool,
							     pdl,
							     v, pshndx);
    }

  return v;
}

template<int size, bool big_endian>
unsigned char*
Output_segment::write_section_headers_list(const Layout* layout,
					   const Stringpool* secnamepool,
					   const Output_data_list* pdl,
					   unsigned char* v,
					   unsigned int* pshndx) const
{
  const int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;
  for (Output_data_list::const_iterator p = pdl->begin();
       p != pdl->end();
       ++p)
    {
      if ((*p)->is_section())
	{
	  const Output_section* ps = static_cast<const Output_section*>(*p);
	  gold_assert(*pshndx == ps->out_shndx());
	  elfcpp::Shdr_write<size, big_endian> oshdr(v);
	  ps->write_header(layout, secnamepool, &oshdr);
	  v += shdr_size;
	  ++*pshndx;
	}
    }
  return v;
}

// Print the output sections to the map file.

void
Output_segment::print_sections_to_mapfile(Mapfile* mapfile) const
{
  if (this->type() != elfcpp::PT_LOAD)
    return;
  for (int i = 0; i < static_cast<int>(ORDER_MAX); ++i)
    this->print_section_list_to_mapfile(mapfile, &this->output_lists_[i]);
}

// Print an output section list to the map file.

void
Output_segment::print_section_list_to_mapfile(Mapfile* mapfile,
					      const Output_data_list* pdl) const
{
  for (Output_data_list::const_iterator p = pdl->begin();
       p != pdl->end();
       ++p)
    (*p)->print_to_mapfile(mapfile);
}

// Output_file methods.

Output_file::Output_file(const char* name)
  : name_(name),
    o_(-1),
    file_size_(0),
    base_(NULL),
    map_is_anonymous_(false),
    map_is_allocated_(false),
    is_temporary_(false)
{
}

// Try to open an existing file.  Returns false if the file doesn't
// exist, has a size of 0 or can't be mmapped.  If BASE_NAME is not
// NULL, open that file as the base for incremental linking, and
// copy its contents to the new output file.  This routine can
// be called for incremental updates, in which case WRITABLE should
// be true, or by the incremental-dump utility, in which case
// WRITABLE should be false.

bool
Output_file::open_base_file(const char* base_name, bool writable)
{
  // The name "-" means "stdout".
  if (strcmp(this->name_, "-") == 0)
    return false;

  bool use_base_file = base_name != NULL;
  if (!use_base_file)
    base_name = this->name_;
  else if (strcmp(base_name, this->name_) == 0)
    gold_fatal(_("%s: incremental base and output file name are the same"),
	       base_name);

  // Don't bother opening files with a size of zero.
  struct stat s;
  if (::stat(base_name, &s) != 0)
    {
      gold_info(_("%s: stat: %s"), base_name, strerror(errno));
      return false;
    }
  if (s.st_size == 0)
    {
      gold_info(_("%s: incremental base file is empty"), base_name);
      return false;
    }

  // If we're using a base file, we want to open it read-only.
  if (use_base_file)
    writable = false;

  int oflags = writable ? O_RDWR : O_RDONLY;
  int o = open_descriptor(-1, base_name, oflags, 0);
  if (o < 0)
    {
      gold_info(_("%s: open: %s"), base_name, strerror(errno));
      return false;
    }

  // If the base file and the output file are different, open a
  // new output file and read the contents from the base file into
  // the newly-mapped region.
  if (use_base_file)
    {
      this->open(s.st_size);
      ssize_t bytes_to_read = s.st_size;
      unsigned char* p = this->base_;
      while (bytes_to_read > 0)
	{
	  ssize_t len = ::read(o, p, bytes_to_read);
	  if (len < 0)
	    {
	      gold_info(_("%s: read failed: %s"), base_name, strerror(errno));
	      return false;
	    }
	  if (len == 0)
	    {
	      gold_info(_("%s: file too short: read only %lld of %lld bytes"),
			base_name,
			static_cast<long long>(s.st_size - bytes_to_read),
			static_cast<long long>(s.st_size));
	      return false;
	    }
	  p += len;
	  bytes_to_read -= len;
	}
      ::close(o);
      return true;
    }

  this->o_ = o;
  this->file_size_ = s.st_size;

  if (!this->map_no_anonymous(writable))
    {
      release_descriptor(o, true);
      this->o_ = -1;
      this->file_size_ = 0;
      return false;
    }

  return true;
}

// Open the output file.

void
Output_file::open(off_t file_size)
{
  this->file_size_ = file_size;

  // Unlink the file first; otherwise the open() may fail if the file
  // is busy (e.g. it's an executable that's currently being executed).
  //
  // However, the linker may be part of a system where a zero-length
  // file is created for it to write to, with tight permissions (gcc
  // 2.95 did something like this).  Unlinking the file would work
  // around those permission controls, so we only unlink if the file
  // has a non-zero size.  We also unlink only regular files to avoid
  // trouble with directories/etc.
  //
  // If we fail, continue; this command is merely a best-effort attempt
  // to improve the odds for open().

  // We let the name "-" mean "stdout"
  if (!this->is_temporary_)
    {
      if (strcmp(this->name_, "-") == 0)
	this->o_ = STDOUT_FILENO;
      else
	{
	  struct stat s;
	  if (::stat(this->name_, &s) == 0
	      && (S_ISREG (s.st_mode) || S_ISLNK (s.st_mode)))
	    {
	      if (s.st_size != 0)
		::unlink(this->name_);
	      else if (!parameters->options().relocatable())
		{
		  // If we don't unlink the existing file, add execute
		  // permission where read permissions already exist
		  // and where the umask permits.
		  int mask = ::umask(0);
		  ::umask(mask);
		  s.st_mode |= (s.st_mode & 0444) >> 2;
		  ::chmod(this->name_, s.st_mode & ~mask);
		}
	    }

	  int mode = parameters->options().relocatable() ? 0666 : 0777;
	  int o = open_descriptor(-1, this->name_, O_RDWR | O_CREAT | O_TRUNC,
				  mode);
	  if (o < 0)
	    gold_fatal(_("%s: open: %s"), this->name_, strerror(errno));
	  this->o_ = o;
	}
    }

  this->map();
}

// Resize the output file.

void
Output_file::resize(off_t file_size)
{
  // If the mmap is mapping an anonymous memory buffer, this is easy:
  // just mremap to the new size.  If it's mapping to a file, we want
  // to unmap to flush to the file, then remap after growing the file.
  if (this->map_is_anonymous_)
    {
      void* base;
      if (!this->map_is_allocated_)
	{
	  base = ::mremap(this->base_, this->file_size_, file_size,
			  MREMAP_MAYMOVE);
	  if (base == MAP_FAILED)
	    gold_fatal(_("%s: mremap: %s"), this->name_, strerror(errno));
	}
      else
	{
	  base = realloc(this->base_, file_size);
	  if (base == NULL)
	    gold_nomem();
	  if (file_size > this->file_size_)
	    memset(static_cast<char*>(base) + this->file_size_, 0,
		   file_size - this->file_size_);
	}
      this->base_ = static_cast<unsigned char*>(base);
      this->file_size_ = file_size;
    }
  else
    {
      this->unmap();
      this->file_size_ = file_size;
      if (!this->map_no_anonymous(true))
	gold_fatal(_("%s: mmap: %s"), this->name_, strerror(errno));
    }
}

// Map an anonymous block of memory which will later be written to the
// file.  Return whether the map succeeded.

bool
Output_file::map_anonymous()
{
  void* base = ::mmap(NULL, this->file_size_, PROT_READ | PROT_WRITE,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (base == MAP_FAILED)
    {
      base = malloc(this->file_size_);
      if (base == NULL)
	return false;
      memset(base, 0, this->file_size_);
      this->map_is_allocated_ = true;
    }
  this->base_ = static_cast<unsigned char*>(base);
  this->map_is_anonymous_ = true;
  return true;
}

// Map the file into memory.  Return whether the mapping succeeded.
// If WRITABLE is true, map with write access.

bool
Output_file::map_no_anonymous(bool writable)
{
  const int o = this->o_;

  // If the output file is not a regular file, don't try to mmap it;
  // instead, we'll mmap a block of memory (an anonymous buffer), and
  // then later write the buffer to the file.
  void* base;
  struct stat statbuf;
  if (o == STDOUT_FILENO || o == STDERR_FILENO
      || ::fstat(o, &statbuf) != 0
      || !S_ISREG(statbuf.st_mode)
      || this->is_temporary_)
    return false;

  // Ensure that we have disk space available for the file.  If we
  // don't do this, it is possible that we will call munmap, close,
  // and exit with dirty buffers still in the cache with no assigned
  // disk blocks.  If the disk is out of space at that point, the
  // output file will wind up incomplete, but we will have already
  // exited.  The alternative to fallocate would be to use fdatasync,
  // but that would be a more significant performance hit.
  if (writable)
    {
      int err = gold_fallocate(o, 0, this->file_size_);
      if (err != 0)
       gold_fatal(_("%s: %s"), this->name_, strerror(err));
    }

  // Map the file into memory.
  int prot = PROT_READ;
  if (writable)
    prot |= PROT_WRITE;
  base = ::mmap(NULL, this->file_size_, prot, MAP_SHARED, o, 0);

  // The mmap call might fail because of file system issues: the file
  // system might not support mmap at all, or it might not support
  // mmap with PROT_WRITE.
  if (base == MAP_FAILED)
    return false;

  this->map_is_anonymous_ = false;
  this->base_ = static_cast<unsigned char*>(base);
  return true;
}

// Map the file into memory.

void
Output_file::map()
{
  if (parameters->options().mmap_output_file()
      && this->map_no_anonymous(true))
    return;

  // The mmap call might fail because of file system issues: the file
  // system might not support mmap at all, or it might not support
  // mmap with PROT_WRITE.  I'm not sure which errno values we will
  // see in all cases, so if the mmap fails for any reason and we
  // don't care about file contents, try for an anonymous map.
  if (this->map_anonymous())
    return;

  gold_fatal(_("%s: mmap: failed to allocate %lu bytes for output file: %s"),
	     this->name_, static_cast<unsigned long>(this->file_size_),
	     strerror(errno));
}

// Unmap the file from memory.

void
Output_file::unmap()
{
  if (this->map_is_anonymous_)
    {
      // We've already written out the data, so there is no reason to
      // waste time unmapping or freeing the memory.
    }
  else
    {
      if (::munmap(this->base_, this->file_size_) < 0)
	gold_error(_("%s: munmap: %s"), this->name_, strerror(errno));
    }
  this->base_ = NULL;
}

// Close the output file.

void
Output_file::close()
{
  // If the map isn't file-backed, we need to write it now.
  if (this->map_is_anonymous_ && !this->is_temporary_)
    {
      size_t bytes_to_write = this->file_size_;
      size_t offset = 0;
      while (bytes_to_write > 0)
	{
	  ssize_t bytes_written = ::write(this->o_, this->base_ + offset,
					  bytes_to_write);
	  if (bytes_written == 0)
	    gold_error(_("%s: write: unexpected 0 return-value"), this->name_);
	  else if (bytes_written < 0)
	    gold_error(_("%s: write: %s"), this->name_, strerror(errno));
	  else
	    {
	      bytes_to_write -= bytes_written;
	      offset += bytes_written;
	    }
	}
    }
  this->unmap();

  // We don't close stdout or stderr
  if (this->o_ != STDOUT_FILENO
      && this->o_ != STDERR_FILENO
      && !this->is_temporary_)
    if (::close(this->o_) < 0)
      gold_error(_("%s: close: %s"), this->name_, strerror(errno));
  this->o_ = -1;
}

// Instantiate the templates we need.  We could use the configure
// script to restrict this to only the ones for implemented targets.

#ifdef HAVE_TARGET_32_LITTLE
template
off_t
Output_section::add_input_section<32, false>(
    Layout* layout,
    Sized_relobj_file<32, false>* object,
    unsigned int shndx,
    const char* secname,
    const elfcpp::Shdr<32, false>& shdr,
    unsigned int reloc_shndx,
    bool have_sections_script);
#endif

#ifdef HAVE_TARGET_32_BIG
template
off_t
Output_section::add_input_section<32, true>(
    Layout* layout,
    Sized_relobj_file<32, true>* object,
    unsigned int shndx,
    const char* secname,
    const elfcpp::Shdr<32, true>& shdr,
    unsigned int reloc_shndx,
    bool have_sections_script);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
off_t
Output_section::add_input_section<64, false>(
    Layout* layout,
    Sized_relobj_file<64, false>* object,
    unsigned int shndx,
    const char* secname,
    const elfcpp::Shdr<64, false>& shdr,
    unsigned int reloc_shndx,
    bool have_sections_script);
#endif

#ifdef HAVE_TARGET_64_BIG
template
off_t
Output_section::add_input_section<64, true>(
    Layout* layout,
    Sized_relobj_file<64, true>* object,
    unsigned int shndx,
    const char* secname,
    const elfcpp::Shdr<64, true>& shdr,
    unsigned int reloc_shndx,
    bool have_sections_script);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_reloc<elfcpp::SHT_REL, false, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_reloc<elfcpp::SHT_REL, false, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_reloc<elfcpp::SHT_REL, false, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_reloc<elfcpp::SHT_REL, false, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_reloc<elfcpp::SHT_REL, true, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_reloc<elfcpp::SHT_REL, true, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_reloc<elfcpp::SHT_REL, true, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_reloc<elfcpp::SHT_REL, true, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_reloc<elfcpp::SHT_RELA, false, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_reloc<elfcpp::SHT_RELA, false, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_reloc<elfcpp::SHT_RELA, false, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_reloc<elfcpp::SHT_RELA, false, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_reloc<elfcpp::SHT_RELA, true, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_reloc<elfcpp::SHT_RELA, true, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_reloc<elfcpp::SHT_RELA, true, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_reloc<elfcpp::SHT_RELA, true, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_data_reloc<elfcpp::SHT_REL, false, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_data_reloc<elfcpp::SHT_REL, false, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_data_reloc<elfcpp::SHT_REL, false, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_data_reloc<elfcpp::SHT_REL, false, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_data_reloc<elfcpp::SHT_REL, true, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_data_reloc<elfcpp::SHT_REL, true, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_data_reloc<elfcpp::SHT_REL, true, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_data_reloc<elfcpp::SHT_REL, true, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_data_reloc<elfcpp::SHT_RELA, false, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_data_reloc<elfcpp::SHT_RELA, false, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_data_reloc<elfcpp::SHT_RELA, false, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_data_reloc<elfcpp::SHT_RELA, false, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_data_reloc<elfcpp::SHT_RELA, true, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_data_reloc<elfcpp::SHT_RELA, true, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_data_reloc<elfcpp::SHT_RELA, true, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_data_reloc<elfcpp::SHT_RELA, true, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_relocatable_relocs<elfcpp::SHT_REL, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_relocatable_relocs<elfcpp::SHT_REL, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_relocatable_relocs<elfcpp::SHT_REL, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_relocatable_relocs<elfcpp::SHT_REL, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_relocatable_relocs<elfcpp::SHT_RELA, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_relocatable_relocs<elfcpp::SHT_RELA, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_relocatable_relocs<elfcpp::SHT_RELA, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_relocatable_relocs<elfcpp::SHT_RELA, 64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Output_data_group<32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Output_data_group<32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Output_data_group<64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Output_data_group<64, true>;
#endif

template
class Output_data_got<32, false>;

template
class Output_data_got<32, true>;

template
class Output_data_got<64, false>;

template
class Output_data_got<64, true>;

} // End namespace gold.
