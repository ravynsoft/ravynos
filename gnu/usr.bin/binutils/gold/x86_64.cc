// x86_64.cc -- x86_64 target support for gold.

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

#include <cstring>

#include "elfcpp.h"
#include "dwarf.h"
#include "parameters.h"
#include "reloc.h"
#include "x86_64.h"
#include "object.h"
#include "symtab.h"
#include "layout.h"
#include "output.h"
#include "copy-relocs.h"
#include "target.h"
#include "target-reloc.h"
#include "target-select.h"
#include "tls.h"
#include "freebsd.h"
#include "nacl.h"
#include "gc.h"
#include "icf.h"

namespace
{

using namespace gold;

// A class to handle the .got.plt section.

class Output_data_got_plt_x86_64 : public Output_section_data_build
{
 public:
  Output_data_got_plt_x86_64(Layout* layout)
    : Output_section_data_build(8),
      layout_(layout)
  { }

  Output_data_got_plt_x86_64(Layout* layout, off_t data_size)
    : Output_section_data_build(data_size, 8),
      layout_(layout)
  { }

 protected:
  // Write out the PLT data.
  void
  do_write(Output_file*);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, "** GOT PLT"); }

 private:
  // A pointer to the Layout class, so that we can find the .dynamic
  // section when we write out the GOT PLT section.
  Layout* layout_;
};

// A class to handle the PLT data.
// This is an abstract base class that handles most of the linker details
// but does not know the actual contents of PLT entries.  The derived
// classes below fill in those details.

template<int size>
class Output_data_plt_x86_64 : public Output_section_data
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_RELA, true, size, false> Reloc_section;

  Output_data_plt_x86_64(Layout* layout, uint64_t addralign,
			 Output_data_got<64, false>* got,
			 Output_data_got_plt_x86_64* got_plt,
			 Output_data_space* got_irelative)
    : Output_section_data(addralign), tlsdesc_rel_(NULL),
      irelative_rel_(NULL), got_(got), got_plt_(got_plt),
      got_irelative_(got_irelative), count_(0), irelative_count_(0),
      tlsdesc_got_offset_(-1U), free_list_()
  { this->init(layout); }

  Output_data_plt_x86_64(Layout* layout, uint64_t plt_entry_size,
			 Output_data_got<64, false>* got,
			 Output_data_got_plt_x86_64* got_plt,
			 Output_data_space* got_irelative,
			 unsigned int plt_count)
    : Output_section_data((plt_count + 1) * plt_entry_size,
			  plt_entry_size, false),
      tlsdesc_rel_(NULL), irelative_rel_(NULL), got_(got),
      got_plt_(got_plt), got_irelative_(got_irelative), count_(plt_count),
      irelative_count_(0), tlsdesc_got_offset_(-1U), free_list_()
  {
    this->init(layout);

    // Initialize the free list and reserve the first entry.
    this->free_list_.init((plt_count + 1) * plt_entry_size, false);
    this->free_list_.remove(0, plt_entry_size);
  }

  // Initialize the PLT section.
  void
  init(Layout* layout);

  // Add an entry to the PLT.
  void
  add_entry(Symbol_table*, Layout*, Symbol* gsym);

  // Add an entry to the PLT for a local STT_GNU_IFUNC symbol.
  unsigned int
  add_local_ifunc_entry(Symbol_table* symtab, Layout*,
			Sized_relobj_file<size, false>* relobj,
			unsigned int local_sym_index);

  // Add the relocation for a PLT entry.
  void
  add_relocation(Symbol_table*, Layout*, Symbol* gsym,
		 unsigned int got_offset);

  // Add the reserved TLSDESC_PLT entry to the PLT.
  void
  reserve_tlsdesc_entry(unsigned int got_offset)
  { this->tlsdesc_got_offset_ = got_offset; }

  // Return true if a TLSDESC_PLT entry has been reserved.
  bool
  has_tlsdesc_entry() const
  { return this->tlsdesc_got_offset_ != -1U; }

  // Return the GOT offset for the reserved TLSDESC_PLT entry.
  unsigned int
  get_tlsdesc_got_offset() const
  { return this->tlsdesc_got_offset_; }

  // Return the offset of the reserved TLSDESC_PLT entry.
  unsigned int
  get_tlsdesc_plt_offset() const
  {
    return ((this->count_ + this->irelative_count_ + 1)
	    * this->get_plt_entry_size());
  }

  // Return the .rela.plt section data.
  Reloc_section*
  rela_plt()
  { return this->rel_; }

  // Return where the TLSDESC relocations should go.
  Reloc_section*
  rela_tlsdesc(Layout*);

  // Return where the IRELATIVE relocations should go in the PLT
  // relocations.
  Reloc_section*
  rela_irelative(Symbol_table*, Layout*);

  // Return whether we created a section for IRELATIVE relocations.
  bool
  has_irelative_section() const
  { return this->irelative_rel_ != NULL; }

  // Get count of regular PLT entries.
  unsigned int
  regular_count() const
  { return this->count_; }

  // Return the total number of PLT entries.
  unsigned int
  entry_count() const
  { return this->count_ + this->irelative_count_; }

  // Return the offset of the first non-reserved PLT entry.
  unsigned int
  first_plt_entry_offset()
  { return this->get_plt_entry_size(); }

  // Return the size of a PLT entry.
  unsigned int
  get_plt_entry_size() const
  { return this->do_get_plt_entry_size(); }

  // Reserve a slot in the PLT for an existing symbol in an incremental update.
  void
  reserve_slot(unsigned int plt_index)
  {
    this->free_list_.remove((plt_index + 1) * this->get_plt_entry_size(),
			    (plt_index + 2) * this->get_plt_entry_size());
  }

  // Return the PLT address to use for a global symbol.
  uint64_t
  address_for_global(const Symbol* sym)
  { return do_address_for_global(sym); }

  // Return the PLT address to use for a local symbol.
  uint64_t
  address_for_local(const Relobj* obj, unsigned int symndx)
  { return do_address_for_local(obj, symndx); }

  // Add .eh_frame information for the PLT.
  void
  add_eh_frame(Layout* layout)
  { this->do_add_eh_frame(layout); }

 protected:
  Output_data_got<64, false>*
  got() const
  { return this->got_; }

  Output_data_got_plt_x86_64*
  got_plt() const
  { return this->got_plt_; }

  Output_data_space*
  got_irelative() const
  { return this->got_irelative_; }

  // Fill in the first PLT entry.
  void
  fill_first_plt_entry(unsigned char* pov,
		       typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		       typename elfcpp::Elf_types<size>::Elf_Addr plt_address)
  { this->do_fill_first_plt_entry(pov, got_address, plt_address); }

  // Fill in a normal PLT entry.  Returns the offset into the entry that
  // should be the initial GOT slot value.
  unsigned int
  fill_plt_entry(unsigned char* pov,
		 typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		 typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
		 unsigned int got_offset,
		 unsigned int plt_offset,
		 unsigned int plt_index)
  {
    return this->do_fill_plt_entry(pov, got_address, plt_address,
				   got_offset, plt_offset, plt_index);
  }

  // Fill in the reserved TLSDESC PLT entry.
  void
  fill_tlsdesc_entry(unsigned char* pov,
		     typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		     typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
		     typename elfcpp::Elf_types<size>::Elf_Addr got_base,
		     unsigned int tlsdesc_got_offset,
		     unsigned int plt_offset)
  {
    this->do_fill_tlsdesc_entry(pov, got_address, plt_address, got_base,
				tlsdesc_got_offset, plt_offset);
  }

  virtual unsigned int
  do_get_plt_entry_size() const = 0;

  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  typename elfcpp::Elf_types<size>::Elf_Addr got_addr,
			  typename elfcpp::Elf_types<size>::Elf_Addr plt_addr)
    = 0;

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_index) = 0;

  virtual void
  do_fill_tlsdesc_entry(unsigned char* pov,
			typename elfcpp::Elf_types<size>::Elf_Addr got_address,
			typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
			typename elfcpp::Elf_types<size>::Elf_Addr got_base,
			unsigned int tlsdesc_got_offset,
			unsigned int plt_offset) = 0;

  // Return the PLT address to use for a global symbol.
  virtual uint64_t
  do_address_for_global(const Symbol* sym);

  // Return the PLT address to use for a local symbol.
  virtual uint64_t
  do_address_for_local(const Relobj* obj, unsigned int symndx);

  virtual void
  do_add_eh_frame(Layout* layout) = 0;

  void
  do_adjust_output_section(Output_section* os);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** PLT")); }

  // The CIE of the .eh_frame unwind information for the PLT.
  static const int plt_eh_frame_cie_size = 16;
  static const unsigned char plt_eh_frame_cie[plt_eh_frame_cie_size];

 private:
  // Set the final size.
  void
  set_final_data_size();

  // Write out the PLT data.
  void
  do_write(Output_file*);

  // The reloc section.
  Reloc_section* rel_;
  // The TLSDESC relocs, if necessary.  These must follow the regular
  // PLT relocs.
  Reloc_section* tlsdesc_rel_;
  // The IRELATIVE relocs, if necessary.  These must follow the
  // regular PLT relocations and the TLSDESC relocations.
  Reloc_section* irelative_rel_;
  // The .got section.
  Output_data_got<64, false>* got_;
  // The .got.plt section.
  Output_data_got_plt_x86_64* got_plt_;
  // The part of the .got.plt section used for IRELATIVE relocs.
  Output_data_space* got_irelative_;
  // The number of PLT entries.
  unsigned int count_;
  // Number of PLT entries with R_X86_64_IRELATIVE relocs.  These
  // follow the regular PLT entries.
  unsigned int irelative_count_;
  // Offset of the reserved TLSDESC_GOT entry when needed.
  unsigned int tlsdesc_got_offset_;
  // List of available regions within the section, for incremental
  // update links.
  Free_list free_list_;
};

template<int size>
class Output_data_plt_x86_64_standard : public Output_data_plt_x86_64<size>
{
 public:
  Output_data_plt_x86_64_standard(Layout* layout,
				  Output_data_got<64, false>* got,
				  Output_data_got_plt_x86_64* got_plt,
				  Output_data_space* got_irelative)
    : Output_data_plt_x86_64<size>(layout, plt_entry_size,
				   got, got_plt, got_irelative)
  { }

  Output_data_plt_x86_64_standard(Layout* layout,
				  Output_data_got<64, false>* got,
				  Output_data_got_plt_x86_64* got_plt,
				  Output_data_space* got_irelative,
				  unsigned int plt_count)
    : Output_data_plt_x86_64<size>(layout, plt_entry_size,
				   got, got_plt, got_irelative,
				   plt_count)
  { }

 protected:
  virtual unsigned int
  do_get_plt_entry_size() const
  { return plt_entry_size; }

  virtual void
  do_add_eh_frame(Layout* layout)
  {
    layout->add_eh_frame_for_plt(this,
				 this->plt_eh_frame_cie,
				 this->plt_eh_frame_cie_size,
				 plt_eh_frame_fde,
				 plt_eh_frame_fde_size);
  }

  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  typename elfcpp::Elf_types<size>::Elf_Addr got_addr,
			  typename elfcpp::Elf_types<size>::Elf_Addr plt_addr);

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_index);

  virtual void
  do_fill_tlsdesc_entry(unsigned char* pov,
			typename elfcpp::Elf_types<size>::Elf_Addr got_address,
			typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
			typename elfcpp::Elf_types<size>::Elf_Addr got_base,
			unsigned int tlsdesc_got_offset,
			unsigned int plt_offset);

 private:
  // The size of an entry in the PLT.
  static const int plt_entry_size = 16;

  // The first entry in the PLT.
  // From the AMD64 ABI: "Unlike Intel386 ABI, this ABI uses the same
  // procedure linkage table for both programs and shared objects."
  static const unsigned char first_plt_entry[plt_entry_size];

  // Other entries in the PLT for an executable.
  static const unsigned char plt_entry[plt_entry_size];

  // The reserved TLSDESC entry in the PLT for an executable.
  static const unsigned char tlsdesc_plt_entry[plt_entry_size];

  // The .eh_frame unwind information for the PLT.
  static const int plt_eh_frame_fde_size = 32;
  static const unsigned char plt_eh_frame_fde[plt_eh_frame_fde_size];
};

// We use this PLT when Indirect Branch Tracking (IBT) is enabled.

template <int size>
class Output_data_plt_x86_64_ibt : public Output_data_plt_x86_64<size>
{
 public:
  Output_data_plt_x86_64_ibt(Layout* layout,
			     Output_data_got<64, false>* got,
			     Output_data_got_plt_x86_64* got_plt,
			     Output_data_space* got_irelative)
    : Output_data_plt_x86_64<size>(layout, plt_entry_size,
				   got, got_plt, got_irelative),
      aplt_offset_(0)
  { }

  Output_data_plt_x86_64_ibt(Layout* layout,
			     Output_data_got<64, false>* got,
			     Output_data_got_plt_x86_64* got_plt,
			     Output_data_space* got_irelative,
			     unsigned int plt_count)
    : Output_data_plt_x86_64<size>(layout, plt_entry_size,
				   got, got_plt, got_irelative,
				   plt_count),
      aplt_offset_(0)
  { }

 protected:
  virtual unsigned int
  do_get_plt_entry_size() const
  { return plt_entry_size; }

  // Return the PLT address to use for a global symbol.
  uint64_t
  do_address_for_global(const Symbol*);

  // Return the PLT address to use for a local symbol.
  uint64_t
  do_address_for_local(const Relobj*, unsigned int symndx);

  virtual void
  do_add_eh_frame(Layout* layout)
  {
    layout->add_eh_frame_for_plt(this,
				 this->plt_eh_frame_cie,
				 this->plt_eh_frame_cie_size,
				 plt_eh_frame_fde,
				 plt_eh_frame_fde_size);
  }

  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  typename elfcpp::Elf_types<size>::Elf_Addr got_addr,
			  typename elfcpp::Elf_types<size>::Elf_Addr plt_addr);

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_index);

  virtual void
  do_fill_tlsdesc_entry(unsigned char* pov,
			typename elfcpp::Elf_types<size>::Elf_Addr got_address,
			typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
			typename elfcpp::Elf_types<size>::Elf_Addr got_base,
			unsigned int tlsdesc_got_offset,
			unsigned int plt_offset);

  void
  fill_aplt_entry(unsigned char* pov,
		  typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		  typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
		  unsigned int got_offset,
		  unsigned int plt_offset,
		  unsigned int plt_index);

 private:
  // Set the final size.
  void
  set_final_data_size();

  // Write out the PLT data.
  void
  do_write(Output_file*);

  // Offset of the Additional PLT (if using -z bndplt).
  unsigned int aplt_offset_;

  // The size of an entry in the PLT.
  static const int plt_entry_size = 16;

  // The size of an entry in the additional PLT.
  static const int aplt_entry_size = 16;

  // The first entry in the PLT.
  // From the AMD64 ABI: "Unlike Intel386 ABI, this ABI uses the same
  // procedure linkage table for both programs and shared objects."
  static const unsigned char first_plt_entry[plt_entry_size];

  // Other entries in the PLT for an executable.
  static const unsigned char plt_entry[plt_entry_size];

  // Entries in the additional PLT.
  static const unsigned char aplt_entry[aplt_entry_size];

  // The reserved TLSDESC entry in the PLT for an executable.
  static const unsigned char tlsdesc_plt_entry[plt_entry_size];

  // The .eh_frame unwind information for the PLT.
  static const int plt_eh_frame_fde_size = 32;
  static const unsigned char plt_eh_frame_fde[plt_eh_frame_fde_size];
};

template<int size>
class Lazy_view
{
 public:
  Lazy_view(Sized_relobj_file<size, false>* object, unsigned int data_shndx)
    : object_(object), data_shndx_(data_shndx), view_(NULL), view_size_(0)
  { }

  inline unsigned char
  operator[](size_t offset)
  {
    if (this->view_ == NULL)
      this->view_ = this->object_->section_contents(this->data_shndx_,
                                                    &this->view_size_,
                                                    true);
    if (offset >= this->view_size_)
      return 0;
    return this->view_[offset];
  }

 private:
  Sized_relobj_file<size, false>* object_;
  unsigned int data_shndx_;
  const unsigned char* view_;
  section_size_type view_size_;
};

// The x86_64 target class.
// See the ABI at
//   http://www.x86-64.org/documentation/abi.pdf
// TLS info comes from
//   http://people.redhat.com/drepper/tls.pdf
//   http://www.lsd.ic.unicamp.br/~oliva/writeups/TLS/RFC-TLSDESC-x86.txt

template<int size>
class Target_x86_64 : public Sized_target<size, false>
{
 public:
  // In the x86_64 ABI (p 68), it says "The AMD64 ABI architectures
  // uses only Elf64_Rela relocation entries with explicit addends."
  typedef Output_data_reloc<elfcpp::SHT_RELA, true, size, false> Reloc_section;

  Target_x86_64(const Target::Target_info* info = &x86_64_info)
    : Sized_target<size, false>(info),
      got_(NULL), plt_(NULL), got_plt_(NULL), got_irelative_(NULL),
      got_tlsdesc_(NULL), global_offset_table_(NULL), rela_dyn_(NULL),
      rela_irelative_(NULL), copy_relocs_(elfcpp::R_X86_64_COPY),
      got_mod_index_offset_(-1U), tlsdesc_reloc_info_(),
      tls_base_symbol_defined_(false), isa_1_used_(0), isa_1_needed_(0),
      feature_1_(0), feature_2_used_(0), feature_2_needed_(0),
      object_isa_1_used_(0), object_feature_1_(0),
      object_feature_2_used_(0), seen_first_object_(false)
  { }

  // Hook for a new output section.
  void
  do_new_output_section(Output_section*) const;

  // Scan the relocations to look for symbol adjustments.
  void
  gc_process_relocs(Symbol_table* symtab,
		    Layout* layout,
		    Sized_relobj_file<size, false>* object,
		    unsigned int data_shndx,
		    unsigned int sh_type,
		    const unsigned char* prelocs,
		    size_t reloc_count,
		    Output_section* output_section,
		    bool needs_special_offset_handling,
		    size_t local_symbol_count,
		    const unsigned char* plocal_symbols);

  // Scan the relocations to look for symbol adjustments.
  void
  scan_relocs(Symbol_table* symtab,
	      Layout* layout,
	      Sized_relobj_file<size, false>* object,
	      unsigned int data_shndx,
	      unsigned int sh_type,
	      const unsigned char* prelocs,
	      size_t reloc_count,
	      Output_section* output_section,
	      bool needs_special_offset_handling,
	      size_t local_symbol_count,
	      const unsigned char* plocal_symbols);

  // Finalize the sections.
  void
  do_finalize_sections(Layout*, const Input_objects*, Symbol_table*);

  // Return the value to use for a dynamic which requires special
  // treatment.
  uint64_t
  do_dynsym_value(const Symbol*) const;

  // Relocate a section.
  void
  relocate_section(const Relocate_info<size, false>*,
		   unsigned int sh_type,
		   const unsigned char* prelocs,
		   size_t reloc_count,
		   Output_section* output_section,
		   bool needs_special_offset_handling,
		   unsigned char* view,
		   typename elfcpp::Elf_types<size>::Elf_Addr view_address,
		   section_size_type view_size,
		   const Reloc_symbol_changes*);

  // Scan the relocs during a relocatable link.
  void
  scan_relocatable_relocs(Symbol_table* symtab,
			  Layout* layout,
			  Sized_relobj_file<size, false>* object,
			  unsigned int data_shndx,
			  unsigned int sh_type,
			  const unsigned char* prelocs,
			  size_t reloc_count,
			  Output_section* output_section,
			  bool needs_special_offset_handling,
			  size_t local_symbol_count,
			  const unsigned char* plocal_symbols,
			  Relocatable_relocs*);

  // Scan the relocs for --emit-relocs.
  void
  emit_relocs_scan(Symbol_table* symtab,
		   Layout* layout,
		   Sized_relobj_file<size, false>* object,
		   unsigned int data_shndx,
		   unsigned int sh_type,
		   const unsigned char* prelocs,
		   size_t reloc_count,
		   Output_section* output_section,
		   bool needs_special_offset_handling,
		   size_t local_symbol_count,
		   const unsigned char* plocal_syms,
		   Relocatable_relocs* rr);

  // Emit relocations for a section.
  void
  relocate_relocs(
      const Relocate_info<size, false>*,
      unsigned int sh_type,
      const unsigned char* prelocs,
      size_t reloc_count,
      Output_section* output_section,
      typename elfcpp::Elf_types<size>::Elf_Off offset_in_output_section,
      unsigned char* view,
      typename elfcpp::Elf_types<size>::Elf_Addr view_address,
      section_size_type view_size,
      unsigned char* reloc_view,
      section_size_type reloc_view_size);

  // Return a string used to fill a code section with nops.
  std::string
  do_code_fill(section_size_type length) const;

  // Return whether SYM is defined by the ABI.
  bool
  do_is_defined_by_abi(const Symbol* sym) const
  { return strcmp(sym->name(), "__tls_get_addr") == 0; }

  // Return the symbol index to use for a target specific relocation.
  // The only target specific relocation is R_X86_64_TLSDESC for a
  // local symbol, which is an absolute reloc.
  unsigned int
  do_reloc_symbol_index(void*, unsigned int r_type) const
  {
    gold_assert(r_type == elfcpp::R_X86_64_TLSDESC);
    return 0;
  }

  // Return the addend to use for a target specific relocation.
  uint64_t
  do_reloc_addend(void* arg, unsigned int r_type, uint64_t addend) const;

  // Return the PLT section.
  uint64_t
  do_plt_address_for_global(const Symbol* gsym) const
  { return this->plt_section()->address_for_global(gsym); }

  uint64_t
  do_plt_address_for_local(const Relobj* relobj, unsigned int symndx) const
  { return this->plt_section()->address_for_local(relobj, symndx); }

  // This function should be defined in targets that can use relocation
  // types to determine (implemented in local_reloc_may_be_function_pointer
  // and global_reloc_may_be_function_pointer)
  // if a function's pointer is taken.  ICF uses this in safe mode to only
  // fold those functions whose pointer is defintely not taken.  For x86_64
  // pie binaries, safe ICF cannot be done by looking at only relocation
  // types, and for certain cases (e.g. R_X86_64_PC32), the instruction
  // opcode is checked as well to distinguish a function call from taking
  // a function's pointer.
  bool
  do_can_check_for_function_pointers() const
  { return true; }

  // Return the base for a DW_EH_PE_datarel encoding.
  uint64_t
  do_ehframe_datarel_base() const;

  // Adjust -fsplit-stack code which calls non-split-stack code.
  void
  do_calls_non_split(Relobj* object, unsigned int shndx,
		     section_offset_type fnoffset, section_size_type fnsize,
		     const unsigned char* prelocs, size_t reloc_count,
		     unsigned char* view, section_size_type view_size,
		     std::string* from, std::string* to) const;

  // Return the size of the GOT section.
  section_size_type
  got_size() const
  {
    gold_assert(this->got_ != NULL);
    return this->got_->data_size();
  }

  // Return the number of entries in the GOT.
  unsigned int
  got_entry_count() const
  {
    if (this->got_ == NULL)
      return 0;
    return this->got_size() / 8;
  }

  // Return the number of entries in the PLT.
  unsigned int
  plt_entry_count() const;

  // Return the offset of the first non-reserved PLT entry.
  unsigned int
  first_plt_entry_offset() const;

  // Return the size of each PLT entry.
  unsigned int
  plt_entry_size() const;

  // Return the size of each GOT entry.
  unsigned int
  got_entry_size() const
  { return 8; };

  // Create the GOT section for an incremental update.
  Output_data_got_base*
  init_got_plt_for_update(Symbol_table* symtab,
			  Layout* layout,
			  unsigned int got_count,
			  unsigned int plt_count);

  // Reserve a GOT entry for a local symbol, and regenerate any
  // necessary dynamic relocations.
  void
  reserve_local_got_entry(unsigned int got_index,
			  Sized_relobj<size, false>* obj,
			  unsigned int r_sym,
			  unsigned int got_type);

  // Reserve a GOT entry for a global symbol, and regenerate any
  // necessary dynamic relocations.
  void
  reserve_global_got_entry(unsigned int got_index, Symbol* gsym,
			   unsigned int got_type);

  // Register an existing PLT entry for a global symbol.
  void
  register_global_plt_entry(Symbol_table*, Layout*, unsigned int plt_index,
			    Symbol* gsym);

  // Force a COPY relocation for a given symbol.
  void
  emit_copy_reloc(Symbol_table*, Symbol*, Output_section*, off_t);

  // Apply an incremental relocation.
  void
  apply_relocation(const Relocate_info<size, false>* relinfo,
		   typename elfcpp::Elf_types<size>::Elf_Addr r_offset,
		   unsigned int r_type,
		   typename elfcpp::Elf_types<size>::Elf_Swxword r_addend,
		   const Symbol* gsym,
		   unsigned char* view,
		   typename elfcpp::Elf_types<size>::Elf_Addr address,
		   section_size_type view_size);

  // Add a new reloc argument, returning the index in the vector.
  size_t
  add_tlsdesc_info(Sized_relobj_file<size, false>* object, unsigned int r_sym)
  {
    this->tlsdesc_reloc_info_.push_back(Tlsdesc_info(object, r_sym));
    return this->tlsdesc_reloc_info_.size() - 1;
  }

  Output_data_plt_x86_64<size>*
  make_data_plt(Layout* layout,
		Output_data_got<64, false>* got,
		Output_data_got_plt_x86_64* got_plt,
		Output_data_space* got_irelative)
  {
    return this->do_make_data_plt(layout, got, got_plt, got_irelative);
  }

  Output_data_plt_x86_64<size>*
  make_data_plt(Layout* layout,
		Output_data_got<64, false>* got,
		Output_data_got_plt_x86_64* got_plt,
		Output_data_space* got_irelative,
		unsigned int plt_count)
  {
    return this->do_make_data_plt(layout, got, got_plt, got_irelative,
				  plt_count);
  }

  virtual Output_data_plt_x86_64<size>*
  do_make_data_plt(Layout* layout,
		   Output_data_got<64, false>* got,
		   Output_data_got_plt_x86_64* got_plt,
		   Output_data_space* got_irelative);

  virtual Output_data_plt_x86_64<size>*
  do_make_data_plt(Layout* layout,
		   Output_data_got<64, false>* got,
		   Output_data_got_plt_x86_64* got_plt,
		   Output_data_space* got_irelative,
		   unsigned int plt_count);

 private:
  // The class which scans relocations.
  class Scan
  {
  public:
    Scan()
      : issued_non_pic_error_(false)
    { }

    static inline int
    get_reference_flags(unsigned int r_type);

    inline void
    local(Symbol_table* symtab, Layout* layout, Target_x86_64* target,
	  Sized_relobj_file<size, false>* object,
	  unsigned int data_shndx,
	  Output_section* output_section,
	  const elfcpp::Rela<size, false>& reloc, unsigned int r_type,
	  const elfcpp::Sym<size, false>& lsym,
	  bool is_discarded);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_x86_64* target,
	   Sized_relobj_file<size, false>* object,
	   unsigned int data_shndx,
	   Output_section* output_section,
	   const elfcpp::Rela<size, false>& reloc, unsigned int r_type,
	   Symbol* gsym);

    inline bool
    local_reloc_may_be_function_pointer(Symbol_table* symtab, Layout* layout,
					Target_x86_64* target,
					Sized_relobj_file<size, false>* object,
					unsigned int data_shndx,
					Output_section* output_section,
					const elfcpp::Rela<size, false>& reloc,
					unsigned int r_type,
					const elfcpp::Sym<size, false>& lsym);

    inline bool
    global_reloc_may_be_function_pointer(Symbol_table* symtab, Layout* layout,
					 Target_x86_64* target,
					 Sized_relobj_file<size, false>* object,
					 unsigned int data_shndx,
					 Output_section* output_section,
					 const elfcpp::Rela<size, false>& reloc,
					 unsigned int r_type,
					 Symbol* gsym);

  private:
    static void
    unsupported_reloc_local(Sized_relobj_file<size, false>*,
			    unsigned int r_type);

    static void
    unsupported_reloc_global(Sized_relobj_file<size, false>*,
			     unsigned int r_type, Symbol*);

    void
    check_non_pic(Relobj*, unsigned int r_type, Symbol*);

    inline bool
    possible_function_pointer_reloc(Sized_relobj_file<size, false>* src_obj,
                                    unsigned int src_indx,
                                    unsigned int r_offset,
                                    unsigned int r_type);

    bool
    reloc_needs_plt_for_ifunc(Sized_relobj_file<size, false>*,
			      unsigned int r_type);

    // Whether we have issued an error about a non-PIC compilation.
    bool issued_non_pic_error_;
  };

  // The class which implements relocation.
  class Relocate
  {
   public:
    Relocate()
      : skip_call_tls_get_addr_(false)
    { }

    ~Relocate()
    {
      if (this->skip_call_tls_get_addr_)
	{
	  // FIXME: This needs to specify the location somehow.
	  gold_error(_("missing expected TLS relocation"));
	}
    }

    // Do a relocation.  Return false if the caller should not issue
    // any warnings about this relocation.
    inline bool
    relocate(const Relocate_info<size, false>*, unsigned int,
	     Target_x86_64*, Output_section*, size_t, const unsigned char*,
	     const Sized_symbol<size>*, const Symbol_value<size>*,
	     unsigned char*, typename elfcpp::Elf_types<size>::Elf_Addr,
	     section_size_type);

   private:
    // Do a TLS relocation.
    inline void
    relocate_tls(const Relocate_info<size, false>*, Target_x86_64*,
		 size_t relnum, const elfcpp::Rela<size, false>&,
		 unsigned int r_type, const Sized_symbol<size>*,
		 const Symbol_value<size>*,
		 unsigned char*, typename elfcpp::Elf_types<size>::Elf_Addr,
		 section_size_type);

    // Do a TLS General-Dynamic to Initial-Exec transition.
    inline void
    tls_gd_to_ie(const Relocate_info<size, false>*, size_t relnum,
		 const elfcpp::Rela<size, false>&, unsigned int r_type,
		 typename elfcpp::Elf_types<size>::Elf_Addr value,
		 unsigned char* view,
		 typename elfcpp::Elf_types<size>::Elf_Addr,
		 section_size_type view_size);

    // Do a TLS General-Dynamic to Local-Exec transition.
    inline void
    tls_gd_to_le(const Relocate_info<size, false>*, size_t relnum,
		 Output_segment* tls_segment,
		 const elfcpp::Rela<size, false>&, unsigned int r_type,
		 typename elfcpp::Elf_types<size>::Elf_Addr value,
		 unsigned char* view,
		 section_size_type view_size);

    // Do a TLSDESC-style General-Dynamic to Initial-Exec transition.
    inline void
    tls_desc_gd_to_ie(const Relocate_info<size, false>*, size_t relnum,
		      const elfcpp::Rela<size, false>&, unsigned int r_type,
		      typename elfcpp::Elf_types<size>::Elf_Addr value,
		      unsigned char* view,
		      typename elfcpp::Elf_types<size>::Elf_Addr,
		      section_size_type view_size);

    // Do a TLSDESC-style General-Dynamic to Local-Exec transition.
    inline void
    tls_desc_gd_to_le(const Relocate_info<size, false>*, size_t relnum,
		      Output_segment* tls_segment,
		      const elfcpp::Rela<size, false>&, unsigned int r_type,
		      typename elfcpp::Elf_types<size>::Elf_Addr value,
		      unsigned char* view,
		      section_size_type view_size);

    // Do a TLS Local-Dynamic to Local-Exec transition.
    inline void
    tls_ld_to_le(const Relocate_info<size, false>*, size_t relnum,
		 Output_segment* tls_segment,
		 const elfcpp::Rela<size, false>&, unsigned int r_type,
		 typename elfcpp::Elf_types<size>::Elf_Addr value,
		 unsigned char* view,
		 section_size_type view_size);

    // Do a TLS Initial-Exec to Local-Exec transition.
    static inline void
    tls_ie_to_le(const Relocate_info<size, false>*, size_t relnum,
		 Output_segment* tls_segment,
		 const elfcpp::Rela<size, false>&, unsigned int r_type,
		 typename elfcpp::Elf_types<size>::Elf_Addr value,
		 unsigned char* view,
		 section_size_type view_size);

    // This is set if we should skip the next reloc, which should be a
    // PLT32 reloc against ___tls_get_addr.
    bool skip_call_tls_get_addr_;
  };

  // Check if relocation against this symbol is a candidate for
  // conversion from
  // mov foo@GOTPCREL(%rip), %reg
  // to lea foo(%rip), %reg.
  template<class View_type>
  static inline bool
  can_convert_mov_to_lea(const Symbol* gsym, unsigned int r_type,
                         size_t r_offset, View_type* view)
  {
    gold_assert(gsym != NULL);
    // We cannot do the conversion unless it's one of these relocations.
    if (r_type != elfcpp::R_X86_64_GOTPCREL
        && r_type != elfcpp::R_X86_64_GOTPCRELX
        && r_type != elfcpp::R_X86_64_REX_GOTPCRELX)
      return false;
    // We cannot convert references to IFUNC symbols, or to symbols that
    // are not local to the current module.
    // We can't do predefined symbols because they may become undefined
    // (e.g., __ehdr_start when the headers aren't mapped to a segment).
    if (gsym->type() == elfcpp::STT_GNU_IFUNC
        || gsym->is_undefined()
        || gsym->is_predefined()
        || gsym->is_from_dynobj()
        || gsym->is_preemptible())
      return false;
    // If we are building a shared object and the symbol is protected, we may
    // need to go through the GOT.
    if (parameters->options().shared()
        && gsym->visibility() == elfcpp::STV_PROTECTED)
      return false;
    // We cannot convert references to the _DYNAMIC symbol.
    if (strcmp(gsym->name(), "_DYNAMIC") == 0)
      return false;
    // Check for a MOV opcode.
    return (*view)[r_offset - 2] == 0x8b;
  }

  // Convert
  // callq *foo@GOTPCRELX(%rip) to
  // addr32 callq foo
  // and jmpq *foo@GOTPCRELX(%rip) to
  // jmpq foo
  // nop
  template<class View_type>
  static inline bool
  can_convert_callq_to_direct(const Symbol* gsym, unsigned int r_type,
			      size_t r_offset, View_type* view)
  {
    gold_assert(gsym != NULL);
    // We cannot do the conversion unless it's a GOTPCRELX relocation.
    if (r_type != elfcpp::R_X86_64_GOTPCRELX)
      return false;
    // We cannot convert references to IFUNC symbols, or to symbols that
    // are not local to the current module.
    if (gsym->type() == elfcpp::STT_GNU_IFUNC
        || gsym->is_undefined ()
        || gsym->is_from_dynobj()
        || gsym->is_preemptible())
      return false;
    // Check for a CALLQ or JMPQ opcode.
    return ((*view)[r_offset - 2] == 0xff
            && ((*view)[r_offset - 1] == 0x15
                || (*view)[r_offset - 1] == 0x25));
  }

  // Adjust TLS relocation type based on the options and whether this
  // is a local symbol.
  static tls::Tls_optimization
  optimize_tls_reloc(bool is_final, int r_type);

  // Get the GOT section, creating it if necessary.
  Output_data_got<64, false>*
  got_section(Symbol_table*, Layout*);

  // Get the GOT PLT section.
  Output_data_got_plt_x86_64*
  got_plt_section() const
  {
    gold_assert(this->got_plt_ != NULL);
    return this->got_plt_;
  }

  // Get the GOT section for TLSDESC entries.
  Output_data_got<64, false>*
  got_tlsdesc_section() const
  {
    gold_assert(this->got_tlsdesc_ != NULL);
    return this->got_tlsdesc_;
  }

  // Create the PLT section.
  void
  make_plt_section(Symbol_table* symtab, Layout* layout);

  // Create a PLT entry for a global symbol.
  void
  make_plt_entry(Symbol_table*, Layout*, Symbol*);

  // Create a PLT entry for a local STT_GNU_IFUNC symbol.
  void
  make_local_ifunc_plt_entry(Symbol_table*, Layout*,
			     Sized_relobj_file<size, false>* relobj,
			     unsigned int local_sym_index);

  // Define the _TLS_MODULE_BASE_ symbol in the TLS segment.
  void
  define_tls_base_symbol(Symbol_table*, Layout*);

  // Create the reserved PLT and GOT entries for the TLS descriptor resolver.
  void
  reserve_tlsdesc_entries(Symbol_table* symtab, Layout* layout);

  // Create a GOT entry for the TLS module index.
  unsigned int
  got_mod_index_entry(Symbol_table* symtab, Layout* layout,
		      Sized_relobj_file<size, false>* object);

  // Get the PLT section.
  Output_data_plt_x86_64<size>*
  plt_section() const
  {
    gold_assert(this->plt_ != NULL);
    return this->plt_;
  }

  // Get the dynamic reloc section, creating it if necessary.
  Reloc_section*
  rela_dyn_section(Layout*);

  // Get the section to use for TLSDESC relocations.
  Reloc_section*
  rela_tlsdesc_section(Layout*) const;

  // Get the section to use for IRELATIVE relocations.
  Reloc_section*
  rela_irelative_section(Layout*);

  // Add a potential copy relocation.
  void
  copy_reloc(Symbol_table* symtab, Layout* layout,
	     Sized_relobj_file<size, false>* object,
	     unsigned int shndx, Output_section* output_section,
	     Symbol* sym, const elfcpp::Rela<size, false>& reloc)
  {
    unsigned int r_type = elfcpp::elf_r_type<size>(reloc.get_r_info());
    this->copy_relocs_.copy_reloc(symtab, layout,
				  symtab->get_sized_symbol<size>(sym),
				  object, shndx, output_section,
				  r_type, reloc.get_r_offset(),
				  reloc.get_r_addend(),
				  this->rela_dyn_section(layout));
  }

  // Record a target-specific program property in the .note.gnu.property
  // section.
  void
  record_gnu_property(unsigned int, unsigned int, size_t,
		      const unsigned char*, const Object*);

  // Merge the target-specific program properties from the current object.
  void
  merge_gnu_properties(const Object*);

  // Finalize the target-specific program properties and add them back to
  // the layout.
  void
  do_finalize_gnu_properties(Layout*) const;

  // Information about this specific target which we pass to the
  // general Target structure.
  static const Target::Target_info x86_64_info;

  // The types of GOT entries needed for this platform.
  // These values are exposed to the ABI in an incremental link.
  // Do not renumber existing values without changing the version
  // number of the .gnu_incremental_inputs section.
  enum Got_type
  {
    GOT_TYPE_STANDARD = 0,      // GOT entry for a regular symbol
    GOT_TYPE_TLS_OFFSET = 1,    // GOT entry for TLS offset
    GOT_TYPE_TLS_PAIR = 2,      // GOT entry for TLS module/offset pair
    GOT_TYPE_TLS_DESC = 3       // GOT entry for TLS_DESC pair
  };

  // This type is used as the argument to the target specific
  // relocation routines.  The only target specific reloc is
  // R_X86_64_TLSDESC against a local symbol.
  struct Tlsdesc_info
  {
    Tlsdesc_info(Sized_relobj_file<size, false>* a_object, unsigned int a_r_sym)
      : object(a_object), r_sym(a_r_sym)
    { }

    // The object in which the local symbol is defined.
    Sized_relobj_file<size, false>* object;
    // The local symbol index in the object.
    unsigned int r_sym;
  };

  // The GOT section.
  Output_data_got<64, false>* got_;
  // The PLT section.
  Output_data_plt_x86_64<size>* plt_;
  // The GOT PLT section.
  Output_data_got_plt_x86_64* got_plt_;
  // The GOT section for IRELATIVE relocations.
  Output_data_space* got_irelative_;
  // The GOT section for TLSDESC relocations.
  Output_data_got<64, false>* got_tlsdesc_;
  // The _GLOBAL_OFFSET_TABLE_ symbol.
  Symbol* global_offset_table_;
  // The dynamic reloc section.
  Reloc_section* rela_dyn_;
  // The section to use for IRELATIVE relocs.
  Reloc_section* rela_irelative_;
  // Relocs saved to avoid a COPY reloc.
  Copy_relocs<elfcpp::SHT_RELA, size, false> copy_relocs_;
  // Offset of the GOT entry for the TLS module index.
  unsigned int got_mod_index_offset_;
  // We handle R_X86_64_TLSDESC against a local symbol as a target
  // specific relocation.  Here we store the object and local symbol
  // index for the relocation.
  std::vector<Tlsdesc_info> tlsdesc_reloc_info_;
  // True if the _TLS_MODULE_BASE_ symbol has been defined.
  bool tls_base_symbol_defined_;
  // Target-specific program properties, from .note.gnu.property section.
  // Each bit represents a specific feature.
  uint32_t isa_1_used_;
  uint32_t isa_1_needed_;
  uint32_t feature_1_;
  uint32_t feature_2_used_;
  uint32_t feature_2_needed_;
  // Target-specific properties from the current object.
  // These bits get ORed into ISA_1_USED_ after all properties for the object
  // have been processed. But if either is all zeroes (as when the property
  // is absent from an object), the result should be all zeroes.
  // (See PR ld/23486.)
  uint32_t object_isa_1_used_;
  // These bits get ANDed into FEATURE_1_ after all properties for the object
  // have been processed.
  uint32_t object_feature_1_;
  uint32_t object_feature_2_used_;
  // Whether we have seen our first object, for use in initializing FEATURE_1_.
  bool seen_first_object_;
};

template<>
const Target::Target_info Target_x86_64<64>::x86_64_info =
{
  64,			// size
  false,		// is_big_endian
  elfcpp::EM_X86_64,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/lib/ld64.so.1",     // program interpreter
  0x400000,		// default_text_segment_address
  0x1000,		// abi_pagesize (overridable by -z max-page-size)
  0x1000,		// common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,	// small_common_shndx
  elfcpp::SHN_X86_64_LCOMMON,	// large_common_shndx
  0,			// small_common_section_flags
  elfcpp::SHF_X86_64_LARGE,	// large_common_section_flags
  NULL,			// attributes_section
  NULL,			// attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_X86_64_UNWIND,	// unwind_section_type
};

template<>
const Target::Target_info Target_x86_64<32>::x86_64_info =
{
  32,			// size
  false,		// is_big_endian
  elfcpp::EM_X86_64,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/libx32/ldx32.so.1", // program interpreter
  0x400000,		// default_text_segment_address
  0x1000,		// abi_pagesize (overridable by -z max-page-size)
  0x1000,		// common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,	// small_common_shndx
  elfcpp::SHN_X86_64_LCOMMON,	// large_common_shndx
  0,			// small_common_section_flags
  elfcpp::SHF_X86_64_LARGE,	// large_common_section_flags
  NULL,			// attributes_section
  NULL,			// attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_X86_64_UNWIND,	// unwind_section_type
};

// This is called when a new output section is created.  This is where
// we handle the SHF_X86_64_LARGE.

template<int size>
void
Target_x86_64<size>::do_new_output_section(Output_section* os) const
{
  if ((os->flags() & elfcpp::SHF_X86_64_LARGE) != 0)
    os->set_is_large_section();
}

// Get the GOT section, creating it if necessary.

template<int size>
Output_data_got<64, false>*
Target_x86_64<size>::got_section(Symbol_table* symtab, Layout* layout)
{
  if (this->got_ == NULL)
    {
      gold_assert(symtab != NULL && layout != NULL);

      // When using -z now, we can treat .got.plt as a relro section.
      // Without -z now, it is modified after program startup by lazy
      // PLT relocations.
      bool is_got_plt_relro = parameters->options().now();
      Output_section_order got_order = (is_got_plt_relro
					? ORDER_RELRO
					: ORDER_RELRO_LAST);
      Output_section_order got_plt_order = (is_got_plt_relro
					    ? ORDER_RELRO
					    : ORDER_NON_RELRO_FIRST);

      this->got_ = new Output_data_got<64, false>();

      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_, got_order, true);

      this->got_plt_ = new Output_data_got_plt_x86_64(layout);
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_plt_, got_plt_order,
				      is_got_plt_relro);

      // The first three entries are reserved.
      this->got_plt_->set_current_data_size(3 * 8);

      if (!is_got_plt_relro)
	{
	  // Those bytes can go into the relro segment.
	  layout->increase_relro(3 * 8);
	}

      // Define _GLOBAL_OFFSET_TABLE_ at the start of the PLT.
      this->global_offset_table_ =
	symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
				      Symbol_table::PREDEFINED,
				      this->got_plt_,
				      0, 0, elfcpp::STT_OBJECT,
				      elfcpp::STB_LOCAL,
				      elfcpp::STV_HIDDEN, 0,
				      false, false);

      // If there are any IRELATIVE relocations, they get GOT entries
      // in .got.plt after the jump slot entries.
      this->got_irelative_ = new Output_data_space(8, "** GOT IRELATIVE PLT");
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_irelative_,
				      got_plt_order, is_got_plt_relro);

      // If there are any TLSDESC relocations, they get GOT entries in
      // .got.plt after the jump slot and IRELATIVE entries.
      this->got_tlsdesc_ = new Output_data_got<64, false>();
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_tlsdesc_,
				      got_plt_order, is_got_plt_relro);
    }

  return this->got_;
}

// Get the dynamic reloc section, creating it if necessary.

template<int size>
typename Target_x86_64<size>::Reloc_section*
Target_x86_64<size>::rela_dyn_section(Layout* layout)
{
  if (this->rela_dyn_ == NULL)
    {
      gold_assert(layout != NULL);
      this->rela_dyn_ = new Reloc_section(parameters->options().combreloc());
      layout->add_output_section_data(".rela.dyn", elfcpp::SHT_RELA,
				      elfcpp::SHF_ALLOC, this->rela_dyn_,
				      ORDER_DYNAMIC_RELOCS, false);
    }
  return this->rela_dyn_;
}

// Get the section to use for IRELATIVE relocs, creating it if
// necessary.  These go in .rela.dyn, but only after all other dynamic
// relocations.  They need to follow the other dynamic relocations so
// that they can refer to global variables initialized by those
// relocs.

template<int size>
typename Target_x86_64<size>::Reloc_section*
Target_x86_64<size>::rela_irelative_section(Layout* layout)
{
  if (this->rela_irelative_ == NULL)
    {
      // Make sure we have already created the dynamic reloc section.
      this->rela_dyn_section(layout);
      this->rela_irelative_ = new Reloc_section(false);
      layout->add_output_section_data(".rela.dyn", elfcpp::SHT_RELA,
				      elfcpp::SHF_ALLOC, this->rela_irelative_,
				      ORDER_DYNAMIC_RELOCS, false);
      gold_assert(this->rela_dyn_->output_section()
		  == this->rela_irelative_->output_section());
    }
  return this->rela_irelative_;
}

// Record a target-specific program property from the .note.gnu.property
// section.
template<int size>
void
Target_x86_64<size>::record_gnu_property(
    unsigned int, unsigned int pr_type,
    size_t pr_datasz, const unsigned char* pr_data,
    const Object* object)
{
  uint32_t val = 0;

  switch (pr_type)
    {
    case elfcpp::GNU_PROPERTY_X86_COMPAT_ISA_1_USED:
    case elfcpp::GNU_PROPERTY_X86_COMPAT_ISA_1_NEEDED:
    case elfcpp::GNU_PROPERTY_X86_COMPAT_2_ISA_1_USED:
    case elfcpp::GNU_PROPERTY_X86_COMPAT_2_ISA_1_NEEDED:
    case elfcpp::GNU_PROPERTY_X86_ISA_1_USED:
    case elfcpp::GNU_PROPERTY_X86_ISA_1_NEEDED:
    case elfcpp::GNU_PROPERTY_X86_FEATURE_1_AND:
    case elfcpp::GNU_PROPERTY_X86_FEATURE_2_USED:
    case elfcpp::GNU_PROPERTY_X86_FEATURE_2_NEEDED:
      if (pr_datasz != 4)
	{
	  gold_warning(_("%s: corrupt .note.gnu.property section "
			 "(pr_datasz for property %d is not 4)"),
		       object->name().c_str(), pr_type);
	  return;
	}
      val = elfcpp::Swap<32, false>::readval(pr_data);
      break;
    default:
      gold_warning(_("%s: unknown program property type 0x%x "
		     "in .note.gnu.property section"),
		   object->name().c_str(), pr_type);
      break;
    }

  switch (pr_type)
    {
    case elfcpp::GNU_PROPERTY_X86_ISA_1_USED:
      this->object_isa_1_used_ |= val;
      break;
    case elfcpp::GNU_PROPERTY_X86_ISA_1_NEEDED:
      this->isa_1_needed_ |= val;
      break;
    case elfcpp::GNU_PROPERTY_X86_FEATURE_1_AND:
      // If we see multiple feature props in one object, OR them together.
      this->object_feature_1_ |= val;
      break;
    case elfcpp::GNU_PROPERTY_X86_FEATURE_2_USED:
      this->object_feature_2_used_ |= val;
      break;
    case elfcpp::GNU_PROPERTY_X86_FEATURE_2_NEEDED:
      this->feature_2_needed_ |= val;
      break;
    }
}

// Merge the target-specific program properties from the current object.
template<int size>
void
Target_x86_64<size>::merge_gnu_properties(const Object*)
{
  if (this->seen_first_object_)
    {
      // If any object is missing the ISA_1_USED property, we must omit
      // it from the output file.
      if (this->object_isa_1_used_ == 0)
	this->isa_1_used_ = 0;
      else if (this->isa_1_used_ != 0)
	this->isa_1_used_ |= this->object_isa_1_used_;
      this->feature_1_ &= this->object_feature_1_;
      // If any object is missing the FEATURE_2_USED property, we must
      // omit it from the output file.
      if (this->object_feature_2_used_ == 0)
	this->feature_2_used_ = 0;
      else if (this->feature_2_used_ != 0)
	this->feature_2_used_ |= this->object_feature_2_used_;
    }
  else
    {
      this->isa_1_used_ = this->object_isa_1_used_;
      this->feature_1_ = this->object_feature_1_;
      this->feature_2_used_ = this->object_feature_2_used_;
      this->seen_first_object_ = true;
    }
  this->object_isa_1_used_ = 0;
  this->object_feature_1_ = 0;
  this->object_feature_2_used_ = 0;
}

static inline void
add_property(Layout* layout, unsigned int pr_type, uint32_t val)
{
  unsigned char buf[4];
  elfcpp::Swap<32, false>::writeval(buf, val);
  layout->add_gnu_property(elfcpp::NT_GNU_PROPERTY_TYPE_0, pr_type, 4, buf);
}

// Finalize the target-specific program properties and add them back to
// the layout.
template<int size>
void
Target_x86_64<size>::do_finalize_gnu_properties(Layout* layout) const
{
  if (this->isa_1_used_ != 0)
    add_property(layout, elfcpp::GNU_PROPERTY_X86_ISA_1_USED,
		 this->isa_1_used_);
  if (this->isa_1_needed_ != 0)
    add_property(layout, elfcpp::GNU_PROPERTY_X86_ISA_1_NEEDED,
		 this->isa_1_needed_);
  if (this->feature_1_ != 0)
    add_property(layout, elfcpp::GNU_PROPERTY_X86_FEATURE_1_AND,
		 this->feature_1_);
  if (this->feature_2_used_ != 0)
    add_property(layout, elfcpp::GNU_PROPERTY_X86_FEATURE_2_USED,
		 this->feature_2_used_);
  if (this->feature_2_needed_ != 0)
    add_property(layout, elfcpp::GNU_PROPERTY_X86_FEATURE_2_NEEDED,
		 this->feature_2_needed_);
}

// Write the first three reserved words of the .got.plt section.
// The remainder of the section is written while writing the PLT
// in Output_data_plt_i386::do_write.

void
Output_data_got_plt_x86_64::do_write(Output_file* of)
{
  // The first entry in the GOT is the address of the .dynamic section
  // aka the PT_DYNAMIC segment.  The next two entries are reserved.
  // We saved space for them when we created the section in
  // Target_x86_64::got_section.
  const off_t got_file_offset = this->offset();
  gold_assert(this->data_size() >= 24);
  unsigned char* const got_view = of->get_output_view(got_file_offset, 24);
  Output_section* dynamic = this->layout_->dynamic_section();
  uint64_t dynamic_addr = dynamic == NULL ? 0 : dynamic->address();
  elfcpp::Swap<64, false>::writeval(got_view, dynamic_addr);
  memset(got_view + 8, 0, 16);
  of->write_output_view(got_file_offset, 24, got_view);
}

// Initialize the PLT section.

template<int size>
void
Output_data_plt_x86_64<size>::init(Layout* layout)
{
  this->rel_ = new Reloc_section(false);
  layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
				  elfcpp::SHF_ALLOC, this->rel_,
				  ORDER_DYNAMIC_PLT_RELOCS, false);
}

template<int size>
void
Output_data_plt_x86_64<size>::do_adjust_output_section(Output_section* os)
{
  os->set_entsize(this->get_plt_entry_size());
}

// Add an entry to the PLT.

template<int size>
void
Output_data_plt_x86_64<size>::add_entry(Symbol_table* symtab, Layout* layout,
					Symbol* gsym)
{
  gold_assert(!gsym->has_plt_offset());

  unsigned int plt_index;
  off_t plt_offset;
  section_offset_type got_offset;

  unsigned int* pcount;
  unsigned int offset;
  unsigned int reserved;
  Output_section_data_build* got;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      pcount = &this->irelative_count_;
      offset = 0;
      reserved = 0;
      got = this->got_irelative_;
    }
  else
    {
      pcount = &this->count_;
      offset = 1;
      reserved = 3;
      got = this->got_plt_;
    }

  if (!this->is_data_size_valid())
    {
      // Note that when setting the PLT offset for a non-IRELATIVE
      // entry we skip the initial reserved PLT entry.
      plt_index = *pcount + offset;
      plt_offset = plt_index * this->get_plt_entry_size();

      ++*pcount;

      got_offset = (plt_index - offset + reserved) * 8;
      gold_assert(got_offset == got->current_data_size());

      // Every PLT entry needs a GOT entry which points back to the PLT
      // entry (this will be changed by the dynamic linker, normally
      // lazily when the function is called).
      got->set_current_data_size(got_offset + 8);
    }
  else
    {
      // FIXME: This is probably not correct for IRELATIVE relocs.

      // For incremental updates, find an available slot.
      plt_offset = this->free_list_.allocate(this->get_plt_entry_size(),
					     this->get_plt_entry_size(), 0);
      if (plt_offset == -1)
	gold_fallback(_("out of patch space (PLT);"
			" relink with --incremental-full"));

      // The GOT and PLT entries have a 1-1 correspondance, so the GOT offset
      // can be calculated from the PLT index, adjusting for the three
      // reserved entries at the beginning of the GOT.
      plt_index = plt_offset / this->get_plt_entry_size() - 1;
      got_offset = (plt_index - offset + reserved) * 8;
    }

  gsym->set_plt_offset(plt_offset);

  // Every PLT entry needs a reloc.
  this->add_relocation(symtab, layout, gsym, got_offset);

  // Note that we don't need to save the symbol.  The contents of the
  // PLT are independent of which symbols are used.  The symbols only
  // appear in the relocations.
}

// Add an entry to the PLT for a local STT_GNU_IFUNC symbol.  Return
// the PLT offset.

template<int size>
unsigned int
Output_data_plt_x86_64<size>::add_local_ifunc_entry(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, false>* relobj,
    unsigned int local_sym_index)
{
  unsigned int plt_offset = this->irelative_count_ * this->get_plt_entry_size();
  ++this->irelative_count_;

  section_offset_type got_offset = this->got_irelative_->current_data_size();

  // Every PLT entry needs a GOT entry which points back to the PLT
  // entry.
  this->got_irelative_->set_current_data_size(got_offset + 8);

  // Every PLT entry needs a reloc.
  Reloc_section* rela = this->rela_irelative(symtab, layout);
  rela->add_symbolless_local_addend(relobj, local_sym_index,
				    elfcpp::R_X86_64_IRELATIVE,
				    this->got_irelative_, got_offset, 0);

  return plt_offset;
}

// Add the relocation for a PLT entry.

template<int size>
void
Output_data_plt_x86_64<size>::add_relocation(Symbol_table* symtab,
					     Layout* layout,
					     Symbol* gsym,
					     unsigned int got_offset)
{
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      Reloc_section* rela = this->rela_irelative(symtab, layout);
      rela->add_symbolless_global_addend(gsym, elfcpp::R_X86_64_IRELATIVE,
					 this->got_irelative_, got_offset, 0);
    }
  else
    {
      gsym->set_needs_dynsym_entry();
      this->rel_->add_global(gsym, elfcpp::R_X86_64_JUMP_SLOT, this->got_plt_,
			     got_offset, 0);
    }
}

// Return where the TLSDESC relocations should go, creating it if
// necessary.  These follow the JUMP_SLOT relocations.

template<int size>
typename Output_data_plt_x86_64<size>::Reloc_section*
Output_data_plt_x86_64<size>::rela_tlsdesc(Layout* layout)
{
  if (this->tlsdesc_rel_ == NULL)
    {
      this->tlsdesc_rel_ = new Reloc_section(false);
      layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
				      elfcpp::SHF_ALLOC, this->tlsdesc_rel_,
				      ORDER_DYNAMIC_PLT_RELOCS, false);
      gold_assert(this->tlsdesc_rel_->output_section()
		  == this->rel_->output_section());
    }
  return this->tlsdesc_rel_;
}

// Return where the IRELATIVE relocations should go in the PLT.  These
// follow the JUMP_SLOT and the TLSDESC relocations.

template<int size>
typename Output_data_plt_x86_64<size>::Reloc_section*
Output_data_plt_x86_64<size>::rela_irelative(Symbol_table* symtab,
					     Layout* layout)
{
  if (this->irelative_rel_ == NULL)
    {
      // Make sure we have a place for the TLSDESC relocations, in
      // case we see any later on.
      this->rela_tlsdesc(layout);
      this->irelative_rel_ = new Reloc_section(false);
      layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
				      elfcpp::SHF_ALLOC, this->irelative_rel_,
				      ORDER_DYNAMIC_PLT_RELOCS, false);
      gold_assert(this->irelative_rel_->output_section()
		  == this->rel_->output_section());

      if (parameters->doing_static_link())
	{
	  // A statically linked executable will only have a .rela.plt
	  // section to hold R_X86_64_IRELATIVE relocs for
	  // STT_GNU_IFUNC symbols.  The library will use these
	  // symbols to locate the IRELATIVE relocs at program startup
	  // time.
	  symtab->define_in_output_data("__rela_iplt_start", NULL,
					Symbol_table::PREDEFINED,
					this->irelative_rel_, 0, 0,
					elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
					elfcpp::STV_HIDDEN, 0, false, true);
	  symtab->define_in_output_data("__rela_iplt_end", NULL,
					Symbol_table::PREDEFINED,
					this->irelative_rel_, 0, 0,
					elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
					elfcpp::STV_HIDDEN, 0, true, true);
	}
    }
  return this->irelative_rel_;
}

// Return the PLT address to use for a global symbol.

template<int size>
uint64_t
Output_data_plt_x86_64<size>::do_address_for_global(const Symbol* gsym)
{
  uint64_t offset = 0;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    offset = (this->count_ + 1) * this->get_plt_entry_size();
  return this->address() + offset + gsym->plt_offset();
}

// Return the PLT address to use for a local symbol.  These are always
// IRELATIVE relocs.

template<int size>
uint64_t
Output_data_plt_x86_64<size>::do_address_for_local(const Relobj* object,
						   unsigned int r_sym)
{
  return (this->address()
	  + (this->count_ + 1) * this->get_plt_entry_size()
	  + object->local_plt_offset(r_sym));
}

// Set the final size.
template<int size>
void
Output_data_plt_x86_64<size>::set_final_data_size()
{
  // Number of regular and IFUNC PLT entries, plus the first entry.
  unsigned int count = this->count_ + this->irelative_count_ + 1;
  // Count the TLSDESC entry, if present.
  if (this->has_tlsdesc_entry())
    ++count;
  this->set_data_size(count * this->get_plt_entry_size());
}

// The first entry in the PLT for an executable.

template<int size>
const unsigned char
Output_data_plt_x86_64_standard<size>::first_plt_entry[plt_entry_size] =
{
  // From AMD64 ABI Draft 0.98, page 76
  0xff, 0x35,	// pushq contents of memory address
  0, 0, 0, 0,	// replaced with address of .got + 8
  0xff, 0x25,	// jmp indirect
  0, 0, 0, 0,	// replaced with address of .got + 16
  0x90, 0x90, 0x90, 0x90   // noop (x4)
};

template<int size>
void
Output_data_plt_x86_64_standard<size>::do_fill_first_plt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address)
{
  memcpy(pov, first_plt_entry, plt_entry_size);
  // We do a jmp relative to the PC at the end of this instruction.
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2,
					      (got_address + 8
					       - (plt_address + 6)));
  elfcpp::Swap<32, false>::writeval(pov + 8,
				    (got_address + 16
				     - (plt_address + 12)));
}

// Subsequent entries in the PLT for an executable.

template<int size>
const unsigned char
Output_data_plt_x86_64_standard<size>::plt_entry[plt_entry_size] =
{
  // From AMD64 ABI Draft 0.98, page 76
  0xff, 0x25,	// jmpq indirect
  0, 0, 0, 0,	// replaced with address of symbol in .got
  0x68,		// pushq immediate
  0, 0, 0, 0,	// replaced with offset into relocation table
  0xe9,		// jmpq relative
  0, 0, 0, 0	// replaced with offset to start of .plt
};

template<int size>
unsigned int
Output_data_plt_x86_64_standard<size>::do_fill_plt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
    unsigned int got_offset,
    unsigned int plt_offset,
    unsigned int plt_index)
{
  // Check PC-relative offset overflow in PLT entry.
  uint64_t plt_got_pcrel_offset = (got_address + got_offset
				   - (plt_address + plt_offset + 6));
  if (Bits<32>::has_overflow(plt_got_pcrel_offset))
    gold_error(_("PC-relative offset overflow in PLT entry %d"),
	       plt_index + 1);

  memcpy(pov, plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2,
					      plt_got_pcrel_offset);

  elfcpp::Swap_unaligned<32, false>::writeval(pov + 7, plt_index);
  elfcpp::Swap<32, false>::writeval(pov + 12,
				    - (plt_offset + plt_entry_size));

  return 6;
}

// The reserved TLSDESC entry in the PLT for an executable.

template<int size>
const unsigned char
Output_data_plt_x86_64_standard<size>::tlsdesc_plt_entry[plt_entry_size] =
{
  // From Alexandre Oliva, "Thread-Local Storage Descriptors for IA32
  // and AMD64/EM64T", Version 0.9.4 (2005-10-10).
  0xff, 0x35,	// pushq x(%rip)
  0, 0, 0, 0,	// replaced with address of linkmap GOT entry (at PLTGOT + 8)
  0xff,	0x25,	// jmpq *y(%rip)
  0, 0, 0, 0,	// replaced with offset of reserved TLSDESC_GOT entry
  0x0f,	0x1f,	// nop
  0x40, 0
};

template<int size>
void
Output_data_plt_x86_64_standard<size>::do_fill_tlsdesc_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
    typename elfcpp::Elf_types<size>::Elf_Addr got_base,
    unsigned int tlsdesc_got_offset,
    unsigned int plt_offset)
{
  memcpy(pov, tlsdesc_plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2,
					      (got_address + 8
					       - (plt_address + plt_offset
						  + 6)));
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 8,
					      (got_base
					       + tlsdesc_got_offset
					       - (plt_address + plt_offset
						  + 12)));
}

// Return the APLT address to use for a global symbol (for IBT).

template<int size>
uint64_t
Output_data_plt_x86_64_ibt<size>::do_address_for_global(const Symbol* gsym)
{
  uint64_t offset = this->aplt_offset_;
  // Convert the PLT offset into an APLT offset.
  unsigned int plt_offset = gsym->plt_offset();
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    offset += this->regular_count() * aplt_entry_size;
  else
    plt_offset -= plt_entry_size;
  plt_offset = plt_offset / (plt_entry_size / aplt_entry_size);
  return this->address() + offset + plt_offset;
}

// Return the PLT address to use for a local symbol.  These are always
// IRELATIVE relocs.

template<int size>
uint64_t
Output_data_plt_x86_64_ibt<size>::do_address_for_local(const Relobj* object,
						 unsigned int r_sym)
{
  // Convert the PLT offset into an APLT offset.
  const Sized_relobj_file<size, false>* sized_relobj =
    static_cast<const Sized_relobj_file<size, false>*>(object);
  const Symbol_value<size>* psymval = sized_relobj->local_symbol(r_sym);
  unsigned int plt_offset = ((object->local_plt_offset(r_sym)
			      - (psymval->is_ifunc_symbol()
				 ? 0 : plt_entry_size))
			     / (plt_entry_size / aplt_entry_size));
  return (this->address()
	  + this->aplt_offset_
	  + this->regular_count() * aplt_entry_size
	  + plt_offset);
}

// Set the final size.

template<int size>
void
Output_data_plt_x86_64_ibt<size>::set_final_data_size()
{
  // Number of regular and IFUNC PLT entries.
  unsigned int count = this->entry_count();
  // Count the first entry and the TLSDESC entry, if present.
  unsigned int extra = this->has_tlsdesc_entry() ? 2 : 1;
  unsigned int plt_size = (count + extra) * plt_entry_size;
  // Offset of the APLT.
  this->aplt_offset_ = plt_size;
  // Size of the APLT.
  plt_size += count * aplt_entry_size;
  this->set_data_size(plt_size);
}

// The first entry in the IBT PLT.

template<int size>
const unsigned char
Output_data_plt_x86_64_ibt<size>::first_plt_entry[plt_entry_size] =
{
  0xff, 0x35,		 // pushq contents of memory address
  0, 0, 0, 0,		 // replaced with address of .got + 8
  0xff, 0x25,		 // jmp indirect
  0, 0, 0, 0,		 // replaced with address of .got + 16
  0x90, 0x90, 0x90, 0x90 // noop (x4)
};

template<int size>
void
Output_data_plt_x86_64_ibt<size>::do_fill_first_plt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address)
{
  // Offsets to the addresses needing relocation.
  const unsigned int roff1 = 2;
  const unsigned int roff2 = 8;

  memcpy(pov, first_plt_entry, plt_entry_size);
  // We do a jmp relative to the PC at the end of this instruction.
  elfcpp::Swap_unaligned<32, false>::writeval(pov + roff1,
					      (got_address + 8
					       - (plt_address + roff1 + 4)));
  elfcpp::Swap<32, false>::writeval(pov + roff2,
				    (got_address + 16
				     - (plt_address + roff2 + 4)));
}

// Subsequent entries in the IBT PLT.

template<int size>
const unsigned char
Output_data_plt_x86_64_ibt<size>::plt_entry[plt_entry_size] =
{
  // From AMD64 ABI Draft 1.0-rc1, Chapter 13.
  0xf3, 0x0f, 0x1e, 0xfa,	// endbr64
  0x68,				// pushq immediate
  0, 0, 0, 0,			// replaced with offset into relocation table
  0xe9,				// jmpq relative
  0, 0, 0, 0,			// replaced with offset to start of .plt
  0x90, 0x90			// nop
};

// Entries in the IBT Additional PLT.

template<int size>
const unsigned char
Output_data_plt_x86_64_ibt<size>::aplt_entry[aplt_entry_size] =
{
  // From AMD64 ABI Draft 1.0-rc1, Chapter 13.
  0xf3, 0x0f, 0x1e, 0xfa,	// endbr64
  0xff, 0x25,			// jmpq indirect
  0, 0, 0, 0,			// replaced with address of symbol in .got
  0x0f, 0x1f, 0x04, 0x00,	// nop
  0x90, 0x90			// nop
};

template<int size>
unsigned int
Output_data_plt_x86_64_ibt<size>::do_fill_plt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr,
    typename elfcpp::Elf_types<size>::Elf_Addr,
    unsigned int,
    unsigned int plt_offset,
    unsigned int plt_index)
{
  // Offsets to the addresses needing relocation.
  const unsigned int roff1 = 5;
  const unsigned int roff2 = 10;

  memcpy(pov, plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + roff1, plt_index);
  elfcpp::Swap<32, false>::writeval(pov + roff2, -(plt_offset + roff2 + 4));
  return 0;
}

template<int size>
void
Output_data_plt_x86_64_ibt<size>::fill_aplt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
    unsigned int got_offset,
    unsigned int plt_offset,
    unsigned int plt_index)
{
  // Offset to the address needing relocation.
  const unsigned int roff = 6;

  // Check PC-relative offset overflow in PLT entry.
  uint64_t plt_got_pcrel_offset = (got_address + got_offset
				   - (plt_address + plt_offset + roff + 4));
  if (Bits<32>::has_overflow(plt_got_pcrel_offset))
    gold_error(_("PC-relative offset overflow in APLT entry %d"),
	       plt_index + 1);

  memcpy(pov, aplt_entry, aplt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + roff, plt_got_pcrel_offset);
}

// The reserved TLSDESC entry in the IBT PLT for an executable.

template<int size>
const unsigned char
Output_data_plt_x86_64_ibt<size>::tlsdesc_plt_entry[plt_entry_size] =
{
  // From Alexandre Oliva, "Thread-Local Storage Descriptors for IA32
  // and AMD64/EM64T", Version 0.9.4 (2005-10-10).
  0xf3, 0x0f, 0x1e, 0xfa, // endbr64
  0xff, 0x35,		// pushq x(%rip)
  0, 0, 0, 0,		// replaced with address of linkmap GOT entry (at PLTGOT + 8)
  0xff, 0x25,		// jmpq *y(%rip)
  0, 0, 0, 0,		// replaced with offset of reserved TLSDESC_GOT entry
};

template<int size>
void
Output_data_plt_x86_64_ibt<size>::do_fill_tlsdesc_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
    typename elfcpp::Elf_types<size>::Elf_Addr got_base,
    unsigned int tlsdesc_got_offset,
    unsigned int plt_offset)
{
  memcpy(pov, tlsdesc_plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 6,
					      (got_address + 8
					       - (plt_address + plt_offset
						  + 10)));
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 12,
					      (got_base
					       + tlsdesc_got_offset
					       - (plt_address + plt_offset
						  + 16)));
}

// The .eh_frame unwind information for the PLT.

template<int size>
const unsigned char
Output_data_plt_x86_64<size>::plt_eh_frame_cie[plt_eh_frame_cie_size] =
{
  1,				// CIE version.
  'z',				// Augmentation: augmentation size included.
  'R',				// Augmentation: FDE encoding included.
  '\0',				// End of augmentation string.
  1,				// Code alignment factor.
  0x78,				// Data alignment factor.
  16,				// Return address column.
  1,				// Augmentation size.
  (elfcpp::DW_EH_PE_pcrel	// FDE encoding.
   | elfcpp::DW_EH_PE_sdata4),
  elfcpp::DW_CFA_def_cfa, 7, 8,	// DW_CFA_def_cfa: r7 (rsp) ofs 8.
  elfcpp::DW_CFA_offset + 16, 1,// DW_CFA_offset: r16 (rip) at cfa-8.
  elfcpp::DW_CFA_nop,		// Align to 16 bytes.
  elfcpp::DW_CFA_nop
};

template<int size>
const unsigned char
Output_data_plt_x86_64_standard<size>::plt_eh_frame_fde[plt_eh_frame_fde_size] =
{
  0, 0, 0, 0,				// Replaced with offset to .plt.
  0, 0, 0, 0,				// Replaced with size of .plt.
  0,					// Augmentation size.
  elfcpp::DW_CFA_def_cfa_offset, 16,	// DW_CFA_def_cfa_offset: 16.
  elfcpp::DW_CFA_advance_loc + 6,	// Advance 6 to __PLT__ + 6.
  elfcpp::DW_CFA_def_cfa_offset, 24,	// DW_CFA_def_cfa_offset: 24.
  elfcpp::DW_CFA_advance_loc + 10,	// Advance 10 to __PLT__ + 16.
  elfcpp::DW_CFA_def_cfa_expression,	// DW_CFA_def_cfa_expression.
  11,					// Block length.
  elfcpp::DW_OP_breg7, 8,		// Push %rsp + 8.
  elfcpp::DW_OP_breg16, 0,		// Push %rip.
  elfcpp::DW_OP_lit15,			// Push 0xf.
  elfcpp::DW_OP_and,			// & (%rip & 0xf).
  elfcpp::DW_OP_lit11,			// Push 0xb.
  elfcpp::DW_OP_ge,			// >= ((%rip & 0xf) >= 0xb)
  elfcpp::DW_OP_lit3,			// Push 3.
  elfcpp::DW_OP_shl,			// << (((%rip & 0xf) >= 0xb) << 3)
  elfcpp::DW_OP_plus,			// + ((((%rip&0xf)>=0xb)<<3)+%rsp+8
  elfcpp::DW_CFA_nop,			// Align to 32 bytes.
  elfcpp::DW_CFA_nop,
  elfcpp::DW_CFA_nop,
  elfcpp::DW_CFA_nop
};

// The .eh_frame unwind information for the PLT.
template<int size>
const unsigned char
Output_data_plt_x86_64_ibt<size>::plt_eh_frame_fde[plt_eh_frame_fde_size] =
{
  0, 0, 0, 0,				// Replaced with offset to .plt.
  0, 0, 0, 0,				// Replaced with size of .plt.
  0,					// Augmentation size.
  elfcpp::DW_CFA_def_cfa_offset, 16,	// DW_CFA_def_cfa_offset: 16.
  elfcpp::DW_CFA_advance_loc + 6,	// Advance 6 to __PLT__ + 6.
  elfcpp::DW_CFA_def_cfa_offset, 24,	// DW_CFA_def_cfa_offset: 24.
  elfcpp::DW_CFA_advance_loc + 10,	// Advance 10 to __PLT__ + 16.
  elfcpp::DW_CFA_def_cfa_expression,	// DW_CFA_def_cfa_expression.
  11,					// Block length.
  elfcpp::DW_OP_breg7, 8,		// Push %rsp + 8.
  elfcpp::DW_OP_breg16, 0,		// Push %rip.
  elfcpp::DW_OP_lit15,			// Push 0xf.
  elfcpp::DW_OP_and,			// & (%rip & 0xf).
  elfcpp::DW_OP_lit9,			// Push 9.
  elfcpp::DW_OP_ge,			// >= ((%rip & 0xf) >= 9)
  elfcpp::DW_OP_lit3,			// Push 3.
  elfcpp::DW_OP_shl,			// << (((%rip & 0xf) >= 9) << 3)
  elfcpp::DW_OP_plus,			// + ((((%rip&0xf)>=9)<<3)+%rsp+8
  elfcpp::DW_CFA_nop,			// Align to 32 bytes.
  elfcpp::DW_CFA_nop,
  elfcpp::DW_CFA_nop,
  elfcpp::DW_CFA_nop
};

// Write out the PLT.  This uses the hand-coded instructions above,
// and adjusts them as needed.  This is specified by the AMD64 ABI.

template<int size>
void
Output_data_plt_x86_64<size>::do_write(Output_file* of)
{
  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  const off_t got_file_offset = this->got_plt_->offset();
  gold_assert(parameters->incremental_update()
	      || (got_file_offset + this->got_plt_->data_size()
		  == this->got_irelative_->offset()));
  const section_size_type got_size =
    convert_to_section_size_type(this->got_plt_->data_size()
				 + this->got_irelative_->data_size());
  unsigned char* const got_view = of->get_output_view(got_file_offset,
						      got_size);

  unsigned char* pov = oview;

  // The base address of the .plt section.
  typename elfcpp::Elf_types<size>::Elf_Addr plt_address = this->address();
  // The base address of the .got section.
  typename elfcpp::Elf_types<size>::Elf_Addr got_base = this->got_->address();
  // The base address of the PLT portion of the .got section,
  // which is where the GOT pointer will point, and where the
  // three reserved GOT entries are located.
  typename elfcpp::Elf_types<size>::Elf_Addr got_address
    = this->got_plt_->address();

  this->fill_first_plt_entry(pov, got_address, plt_address);
  pov += this->get_plt_entry_size();

  // The first three entries in the GOT are reserved, and are written
  // by Output_data_got_plt_x86_64::do_write.
  unsigned char* got_pov = got_view + 24;

  unsigned int plt_offset = this->get_plt_entry_size();
  unsigned int got_offset = 24;
  const unsigned int count = this->count_ + this->irelative_count_;
  for (unsigned int plt_index = 0;
       plt_index < count;
       ++plt_index,
	 pov += this->get_plt_entry_size(),
	 got_pov += 8,
	 plt_offset += this->get_plt_entry_size(),
	 got_offset += 8)
    {
      // Set and adjust the PLT entry itself.
      unsigned int lazy_offset = this->fill_plt_entry(pov,
						      got_address, plt_address,
						      got_offset, plt_offset,
						      plt_index);

      // Set the entry in the GOT.
      elfcpp::Swap<64, false>::writeval(got_pov,
					plt_address + plt_offset + lazy_offset);
    }

  if (this->has_tlsdesc_entry())
    {
      // Set and adjust the reserved TLSDESC PLT entry.
      unsigned int tlsdesc_got_offset = this->get_tlsdesc_got_offset();
      this->fill_tlsdesc_entry(pov, got_address, plt_address, got_base,
			       tlsdesc_got_offset, plt_offset);
      pov += this->get_plt_entry_size();
    }

  gold_assert(static_cast<section_size_type>(pov - oview) == oview_size);
  gold_assert(static_cast<section_size_type>(got_pov - got_view) == got_size);

  of->write_output_view(offset, oview_size, oview);
  of->write_output_view(got_file_offset, got_size, got_view);
}

// Write out the IBT PLT.

template<int size>
void
Output_data_plt_x86_64_ibt<size>::do_write(Output_file* of)
{
  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  Output_data_got<64, false>* got = this->got();
  Output_data_got_plt_x86_64* got_plt = this->got_plt();
  Output_data_space* got_irelative = this->got_irelative();

  const off_t got_file_offset = got_plt->offset();
  gold_assert(parameters->incremental_update()
	      || (got_file_offset + got_plt->data_size()
		  == got_irelative->offset()));
  const section_size_type got_size =
    convert_to_section_size_type(got_plt->data_size()
				 + got_irelative->data_size());
  unsigned char* const got_view = of->get_output_view(got_file_offset,
						      got_size);

  unsigned char* pov = oview;

  // The base address of the .plt section.
  elfcpp::Elf_types<64>::Elf_Addr plt_address = this->address();
  // The base address of the .got section.
  elfcpp::Elf_types<64>::Elf_Addr got_base = got->address();
  // The base address of the PLT portion of the .got section,
  // which is where the GOT pointer will point, and where the
  // three reserved GOT entries are located.
  elfcpp::Elf_types<64>::Elf_Addr got_address = got_plt->address();

  this->fill_first_plt_entry(pov, got_address, plt_address);
  pov += plt_entry_size;

  // The first three entries in the GOT are reserved, and are written
  // by Output_data_got_plt_x86_64::do_write.
  unsigned char* got_pov = got_view + 24;

  unsigned int plt_offset = plt_entry_size;
  unsigned int got_offset = 24;
  const unsigned int count = this->entry_count();
  for (unsigned int plt_index = 0;
       plt_index < count;
       ++plt_index,
	 pov += plt_entry_size,
	 got_pov += 8,
	 plt_offset += plt_entry_size,
	 got_offset += 8)
    {
      // Set and adjust the PLT entry itself.
      unsigned int lazy_offset = this->fill_plt_entry(pov,
						      got_address, plt_address,
						      got_offset, plt_offset,
						      plt_index);

      // Set the entry in the GOT.
      elfcpp::Swap<64, false>::writeval(got_pov,
					plt_address + plt_offset + lazy_offset);
    }

  if (this->has_tlsdesc_entry())
    {
      // Set and adjust the reserved TLSDESC PLT entry.
      unsigned int tlsdesc_got_offset = this->get_tlsdesc_got_offset();
      this->fill_tlsdesc_entry(pov, got_address, plt_address, got_base,
			       tlsdesc_got_offset, plt_offset);
      pov += this->get_plt_entry_size();
      plt_offset += plt_entry_size;
    }

  // Write the additional PLT.
  got_offset = 24;
  for (unsigned int plt_index = 0;
       plt_index < count;
       ++plt_index,
	 pov += aplt_entry_size,
	 plt_offset += aplt_entry_size,
	 got_offset += 8)
    {
      // Set and adjust the APLT entry.
      this->fill_aplt_entry(pov, got_address, plt_address, got_offset,
			    plt_offset, plt_index);
    }

  gold_assert(static_cast<section_size_type>(pov - oview) == oview_size);
  gold_assert(static_cast<section_size_type>(got_pov - got_view) == got_size);

  of->write_output_view(offset, oview_size, oview);
  of->write_output_view(got_file_offset, got_size, got_view);
}

// Create the PLT section.

template<int size>
void
Target_x86_64<size>::make_plt_section(Symbol_table* symtab, Layout* layout)
{
  if (this->plt_ == NULL)
    {
      // Create the GOT sections first.
      this->got_section(symtab, layout);

      this->plt_ = this->make_data_plt(layout, this->got_, this->got_plt_,
				       this->got_irelative_);

      // Add unwind information if requested.
      if (parameters->options().ld_generated_unwind_info())
	this->plt_->add_eh_frame(layout);

      layout->add_output_section_data(".plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_EXECINSTR),
				      this->plt_, ORDER_PLT, false);

      // Make the sh_info field of .rela.plt point to .plt.
      Output_section* rela_plt_os = this->plt_->rela_plt()->output_section();
      rela_plt_os->set_info_section(this->plt_->output_section());
    }
}

template<>
Output_data_plt_x86_64<32>*
Target_x86_64<32>::do_make_data_plt(Layout* layout,
				    Output_data_got<64, false>* got,
				    Output_data_got_plt_x86_64* got_plt,
				    Output_data_space* got_irelative)
{
  if (this->feature_1_ & elfcpp::GNU_PROPERTY_X86_FEATURE_1_IBT)
    return new Output_data_plt_x86_64_ibt<32>(layout, got, got_plt,
					      got_irelative);
  return new Output_data_plt_x86_64_standard<32>(layout, got, got_plt,
						 got_irelative);
}

template<>
Output_data_plt_x86_64<64>*
Target_x86_64<64>::do_make_data_plt(Layout* layout,
				    Output_data_got<64, false>* got,
				    Output_data_got_plt_x86_64* got_plt,
				    Output_data_space* got_irelative)
{
  if (this->feature_1_ & elfcpp::GNU_PROPERTY_X86_FEATURE_1_IBT)
    return new Output_data_plt_x86_64_ibt<64>(layout, got, got_plt,
					      got_irelative);
  else
    return new Output_data_plt_x86_64_standard<64>(layout, got, got_plt,
						   got_irelative);
}

template<>
Output_data_plt_x86_64<32>*
Target_x86_64<32>::do_make_data_plt(Layout* layout,
				    Output_data_got<64, false>* got,
				    Output_data_got_plt_x86_64* got_plt,
				    Output_data_space* got_irelative,
				    unsigned int plt_count)
{
  if (this->feature_1_ & elfcpp::GNU_PROPERTY_X86_FEATURE_1_IBT)
    return new Output_data_plt_x86_64_ibt<32>(layout, got, got_plt,
					      got_irelative, plt_count);
  return new Output_data_plt_x86_64_standard<32>(layout, got, got_plt,
						 got_irelative, plt_count);
}

template<>
Output_data_plt_x86_64<64>*
Target_x86_64<64>::do_make_data_plt(Layout* layout,
				    Output_data_got<64, false>* got,
				    Output_data_got_plt_x86_64* got_plt,
				    Output_data_space* got_irelative,
				    unsigned int plt_count)
{
  if (this->feature_1_ & elfcpp::GNU_PROPERTY_X86_FEATURE_1_IBT)
    return new Output_data_plt_x86_64_ibt<64>(layout, got, got_plt,
					      got_irelative, plt_count);
  else
    return new Output_data_plt_x86_64_standard<64>(layout, got, got_plt,
						   got_irelative,
						   plt_count);
}

// Return the section for TLSDESC relocations.

template<int size>
typename Target_x86_64<size>::Reloc_section*
Target_x86_64<size>::rela_tlsdesc_section(Layout* layout) const
{
  return this->plt_section()->rela_tlsdesc(layout);
}

// Create a PLT entry for a global symbol.

template<int size>
void
Target_x86_64<size>::make_plt_entry(Symbol_table* symtab, Layout* layout,
				    Symbol* gsym)
{
  if (gsym->has_plt_offset())
    return;

  if (this->plt_ == NULL)
    this->make_plt_section(symtab, layout);

  this->plt_->add_entry(symtab, layout, gsym);
}

// Make a PLT entry for a local STT_GNU_IFUNC symbol.

template<int size>
void
Target_x86_64<size>::make_local_ifunc_plt_entry(
    Symbol_table* symtab, Layout* layout,
    Sized_relobj_file<size, false>* relobj,
    unsigned int local_sym_index)
{
  if (relobj->local_has_plt_offset(local_sym_index))
    return;
  if (this->plt_ == NULL)
    this->make_plt_section(symtab, layout);
  unsigned int plt_offset = this->plt_->add_local_ifunc_entry(symtab, layout,
							      relobj,
							      local_sym_index);
  relobj->set_local_plt_offset(local_sym_index, plt_offset);
}

// Return the number of entries in the PLT.

template<int size>
unsigned int
Target_x86_64<size>::plt_entry_count() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->entry_count();
}

// Return the offset of the first non-reserved PLT entry.

template<int size>
unsigned int
Target_x86_64<size>::first_plt_entry_offset() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->first_plt_entry_offset();
}

// Return the size of each PLT entry.

template<int size>
unsigned int
Target_x86_64<size>::plt_entry_size() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->get_plt_entry_size();
}

// Create the GOT and PLT sections for an incremental update.

template<int size>
Output_data_got_base*
Target_x86_64<size>::init_got_plt_for_update(Symbol_table* symtab,
				       Layout* layout,
				       unsigned int got_count,
				       unsigned int plt_count)
{
  gold_assert(this->got_ == NULL);

  this->got_ = new Output_data_got<64, false>(got_count * 8);
  layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				  (elfcpp::SHF_ALLOC
				   | elfcpp::SHF_WRITE),
				  this->got_, ORDER_RELRO_LAST,
				  true);

  // Add the three reserved entries.
  this->got_plt_ = new Output_data_got_plt_x86_64(layout, (plt_count + 3) * 8);
  layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				  (elfcpp::SHF_ALLOC
				   | elfcpp::SHF_WRITE),
				  this->got_plt_, ORDER_NON_RELRO_FIRST,
				  false);

  // Define _GLOBAL_OFFSET_TABLE_ at the start of the PLT.
  this->global_offset_table_ =
    symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
				  Symbol_table::PREDEFINED,
				  this->got_plt_,
				  0, 0, elfcpp::STT_OBJECT,
				  elfcpp::STB_LOCAL,
				  elfcpp::STV_HIDDEN, 0,
				  false, false);

  // If there are any TLSDESC relocations, they get GOT entries in
  // .got.plt after the jump slot entries.
  // FIXME: Get the count for TLSDESC entries.
  this->got_tlsdesc_ = new Output_data_got<64, false>(0);
  layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				  elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE,
				  this->got_tlsdesc_,
				  ORDER_NON_RELRO_FIRST, false);

  // If there are any IRELATIVE relocations, they get GOT entries in
  // .got.plt after the jump slot and TLSDESC entries.
  this->got_irelative_ = new Output_data_space(0, 8, "** GOT IRELATIVE PLT");
  layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				  elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE,
				  this->got_irelative_,
				  ORDER_NON_RELRO_FIRST, false);

  // Create the PLT section.
  this->plt_ = this->make_data_plt(layout, this->got_,
				   this->got_plt_,
				   this->got_irelative_,
				   plt_count);

  // Add unwind information if requested.
  if (parameters->options().ld_generated_unwind_info())
    this->plt_->add_eh_frame(layout);

  layout->add_output_section_data(".plt", elfcpp::SHT_PROGBITS,
				  elfcpp::SHF_ALLOC | elfcpp::SHF_EXECINSTR,
				  this->plt_, ORDER_PLT, false);

  // Make the sh_info field of .rela.plt point to .plt.
  Output_section* rela_plt_os = this->plt_->rela_plt()->output_section();
  rela_plt_os->set_info_section(this->plt_->output_section());

  // Create the rela_dyn section.
  this->rela_dyn_section(layout);

  return this->got_;
}

// Reserve a GOT entry for a local symbol, and regenerate any
// necessary dynamic relocations.

template<int size>
void
Target_x86_64<size>::reserve_local_got_entry(
    unsigned int got_index,
    Sized_relobj<size, false>* obj,
    unsigned int r_sym,
    unsigned int got_type)
{
  unsigned int got_offset = got_index * 8;
  Reloc_section* rela_dyn = this->rela_dyn_section(NULL);

  this->got_->reserve_local(got_index, obj, r_sym, got_type);
  switch (got_type)
    {
    case GOT_TYPE_STANDARD:
      if (parameters->options().output_is_position_independent())
	rela_dyn->add_local_relative(obj, r_sym, elfcpp::R_X86_64_RELATIVE,
				     this->got_, got_offset, 0, false);
      break;
    case GOT_TYPE_TLS_OFFSET:
      rela_dyn->add_local(obj, r_sym, elfcpp::R_X86_64_TPOFF64,
			  this->got_, got_offset, 0);
      break;
    case GOT_TYPE_TLS_PAIR:
      this->got_->reserve_slot(got_index + 1);
      rela_dyn->add_local(obj, r_sym, elfcpp::R_X86_64_DTPMOD64,
			  this->got_, got_offset, 0);
      break;
    case GOT_TYPE_TLS_DESC:
      gold_fatal(_("TLS_DESC not yet supported for incremental linking"));
      // this->got_->reserve_slot(got_index + 1);
      // rela_dyn->add_target_specific(elfcpp::R_X86_64_TLSDESC, arg,
      //			       this->got_, got_offset, 0);
      break;
    default:
      gold_unreachable();
    }
}

// Reserve a GOT entry for a global symbol, and regenerate any
// necessary dynamic relocations.

template<int size>
void
Target_x86_64<size>::reserve_global_got_entry(unsigned int got_index,
					      Symbol* gsym,
					      unsigned int got_type)
{
  unsigned int got_offset = got_index * 8;
  Reloc_section* rela_dyn = this->rela_dyn_section(NULL);

  this->got_->reserve_global(got_index, gsym, got_type);
  switch (got_type)
    {
    case GOT_TYPE_STANDARD:
      if (!gsym->final_value_is_known())
	{
	  if (gsym->is_from_dynobj()
	      || gsym->is_undefined()
	      || gsym->is_preemptible()
	      || gsym->type() == elfcpp::STT_GNU_IFUNC)
	    rela_dyn->add_global(gsym, elfcpp::R_X86_64_GLOB_DAT,
				 this->got_, got_offset, 0);
	  else
	    rela_dyn->add_global_relative(gsym, elfcpp::R_X86_64_RELATIVE,
					  this->got_, got_offset, 0, false);
	}
      break;
    case GOT_TYPE_TLS_OFFSET:
      rela_dyn->add_global_relative(gsym, elfcpp::R_X86_64_TPOFF64,
				    this->got_, got_offset, 0, false);
      break;
    case GOT_TYPE_TLS_PAIR:
      this->got_->reserve_slot(got_index + 1);
      rela_dyn->add_global_relative(gsym, elfcpp::R_X86_64_DTPMOD64,
				    this->got_, got_offset, 0, false);
      rela_dyn->add_global_relative(gsym, elfcpp::R_X86_64_DTPOFF64,
				    this->got_, got_offset + 8, 0, false);
      break;
    case GOT_TYPE_TLS_DESC:
      this->got_->reserve_slot(got_index + 1);
      rela_dyn->add_global_relative(gsym, elfcpp::R_X86_64_TLSDESC,
				    this->got_, got_offset, 0, false);
      break;
    default:
      gold_unreachable();
    }
}

// Register an existing PLT entry for a global symbol.

template<int size>
void
Target_x86_64<size>::register_global_plt_entry(Symbol_table* symtab,
					       Layout* layout,
					       unsigned int plt_index,
					       Symbol* gsym)
{
  gold_assert(this->plt_ != NULL);
  gold_assert(!gsym->has_plt_offset());

  this->plt_->reserve_slot(plt_index);

  gsym->set_plt_offset((plt_index + 1) * this->plt_entry_size());

  unsigned int got_offset = (plt_index + 3) * 8;
  this->plt_->add_relocation(symtab, layout, gsym, got_offset);
}

// Force a COPY relocation for a given symbol.

template<int size>
void
Target_x86_64<size>::emit_copy_reloc(
    Symbol_table* symtab, Symbol* sym, Output_section* os, off_t offset)
{
  this->copy_relocs_.emit_copy_reloc(symtab,
				     symtab->get_sized_symbol<size>(sym),
				     os,
				     offset,
				     this->rela_dyn_section(NULL));
}

// Define the _TLS_MODULE_BASE_ symbol in the TLS segment.

template<int size>
void
Target_x86_64<size>::define_tls_base_symbol(Symbol_table* symtab,
					    Layout* layout)
{
  if (this->tls_base_symbol_defined_)
    return;

  Output_segment* tls_segment = layout->tls_segment();
  if (tls_segment != NULL)
    {
      bool is_exec = parameters->options().output_is_executable();
      symtab->define_in_output_segment("_TLS_MODULE_BASE_", NULL,
				       Symbol_table::PREDEFINED,
				       tls_segment, 0, 0,
				       elfcpp::STT_TLS,
				       elfcpp::STB_LOCAL,
				       elfcpp::STV_HIDDEN, 0,
				       (is_exec
					? Symbol::SEGMENT_END
					: Symbol::SEGMENT_START),
				       true);
    }
  this->tls_base_symbol_defined_ = true;
}

// Create the reserved PLT and GOT entries for the TLS descriptor resolver.

template<int size>
void
Target_x86_64<size>::reserve_tlsdesc_entries(Symbol_table* symtab,
					     Layout* layout)
{
  if (this->plt_ == NULL)
    this->make_plt_section(symtab, layout);

  if (!this->plt_->has_tlsdesc_entry())
    {
      // Allocate the TLSDESC_GOT entry.
      Output_data_got<64, false>* got = this->got_section(symtab, layout);
      unsigned int got_offset = got->add_constant(0);

      // Allocate the TLSDESC_PLT entry.
      this->plt_->reserve_tlsdesc_entry(got_offset);
    }
}

// Create a GOT entry for the TLS module index.

template<int size>
unsigned int
Target_x86_64<size>::got_mod_index_entry(Symbol_table* symtab, Layout* layout,
					 Sized_relobj_file<size, false>* object)
{
  if (this->got_mod_index_offset_ == -1U)
    {
      gold_assert(symtab != NULL && layout != NULL && object != NULL);
      Reloc_section* rela_dyn = this->rela_dyn_section(layout);
      Output_data_got<64, false>* got = this->got_section(symtab, layout);
      unsigned int got_offset = got->add_constant(0);
      rela_dyn->add_local(object, 0, elfcpp::R_X86_64_DTPMOD64, got,
			  got_offset, 0);
      got->add_constant(0);
      this->got_mod_index_offset_ = got_offset;
    }
  return this->got_mod_index_offset_;
}

// Optimize the TLS relocation type based on what we know about the
// symbol.  IS_FINAL is true if the final address of this symbol is
// known at link time.

template<int size>
tls::Tls_optimization
Target_x86_64<size>::optimize_tls_reloc(bool is_final, int r_type)
{
  // If we are generating a shared library, then we can't do anything
  // in the linker.
  if (parameters->options().shared())
    return tls::TLSOPT_NONE;

  switch (r_type)
    {
    case elfcpp::R_X86_64_TLSGD:
    case elfcpp::R_X86_64_GOTPC32_TLSDESC:
    case elfcpp::R_X86_64_TLSDESC_CALL:
      // These are General-Dynamic which permits fully general TLS
      // access.  Since we know that we are generating an executable,
      // we can convert this to Initial-Exec.  If we also know that
      // this is a local symbol, we can further switch to Local-Exec.
      if (is_final)
	return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_TO_IE;

    case elfcpp::R_X86_64_TLSLD:
      // This is Local-Dynamic, which refers to a local symbol in the
      // dynamic TLS block.  Since we know that we generating an
      // executable, we can switch to Local-Exec.
      return tls::TLSOPT_TO_LE;

    case elfcpp::R_X86_64_DTPOFF32:
    case elfcpp::R_X86_64_DTPOFF64:
      // Another Local-Dynamic reloc.
      return tls::TLSOPT_TO_LE;

    case elfcpp::R_X86_64_GOTTPOFF:
      // These are Initial-Exec relocs which get the thread offset
      // from the GOT.  If we know that we are linking against the
      // local symbol, we can switch to Local-Exec, which links the
      // thread offset into the instruction.
      if (is_final)
	return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_NONE;

    case elfcpp::R_X86_64_TPOFF32:
      // When we already have Local-Exec, there is nothing further we
      // can do.
      return tls::TLSOPT_NONE;

    default:
      gold_unreachable();
    }
}

// Get the Reference_flags for a particular relocation.

template<int size>
int
Target_x86_64<size>::Scan::get_reference_flags(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_X86_64_NONE:
    case elfcpp::R_X86_64_GNU_VTINHERIT:
    case elfcpp::R_X86_64_GNU_VTENTRY:
    case elfcpp::R_X86_64_GOTPC32:
    case elfcpp::R_X86_64_GOTPC64:
      // No symbol reference.
      return 0;

    case elfcpp::R_X86_64_64:
    case elfcpp::R_X86_64_32:
    case elfcpp::R_X86_64_32S:
    case elfcpp::R_X86_64_16:
    case elfcpp::R_X86_64_8:
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_X86_64_PC64:
    case elfcpp::R_X86_64_PC32:
    case elfcpp::R_X86_64_PC16:
    case elfcpp::R_X86_64_PC8:
    case elfcpp::R_X86_64_GOTOFF64:
      return Symbol::RELATIVE_REF;

    case elfcpp::R_X86_64_PLT32:
    case elfcpp::R_X86_64_PLTOFF64:
      return Symbol::FUNCTION_CALL | Symbol::RELATIVE_REF;

    case elfcpp::R_X86_64_GOT64:
    case elfcpp::R_X86_64_GOT32:
    case elfcpp::R_X86_64_GOTPCREL64:
    case elfcpp::R_X86_64_GOTPCREL:
    case elfcpp::R_X86_64_GOTPCRELX:
    case elfcpp::R_X86_64_REX_GOTPCRELX:
    case elfcpp::R_X86_64_GOTPLT64:
      // Absolute in GOT.
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_X86_64_TLSGD:            // Global-dynamic
    case elfcpp::R_X86_64_GOTPC32_TLSDESC:  // Global-dynamic (from ~oliva url)
    case elfcpp::R_X86_64_TLSDESC_CALL:
    case elfcpp::R_X86_64_TLSLD:            // Local-dynamic
    case elfcpp::R_X86_64_DTPOFF32:
    case elfcpp::R_X86_64_DTPOFF64:
    case elfcpp::R_X86_64_GOTTPOFF:         // Initial-exec
    case elfcpp::R_X86_64_TPOFF32:          // Local-exec
      return Symbol::TLS_REF;

    case elfcpp::R_X86_64_COPY:
    case elfcpp::R_X86_64_GLOB_DAT:
    case elfcpp::R_X86_64_JUMP_SLOT:
    case elfcpp::R_X86_64_RELATIVE:
    case elfcpp::R_X86_64_IRELATIVE:
    case elfcpp::R_X86_64_TPOFF64:
    case elfcpp::R_X86_64_DTPMOD64:
    case elfcpp::R_X86_64_TLSDESC:
    case elfcpp::R_X86_64_SIZE32:
    case elfcpp::R_X86_64_SIZE64:
    default:
      // Not expected.  We will give an error later.
      return 0;
    }
}

// Report an unsupported relocation against a local symbol.

template<int size>
void
Target_x86_64<size>::Scan::unsupported_reloc_local(
     Sized_relobj_file<size, false>* object,
     unsigned int r_type)
{
  gold_error(_("%s: unsupported reloc %u against local symbol"),
	     object->name().c_str(), r_type);
}

// We are about to emit a dynamic relocation of type R_TYPE.  If the
// dynamic linker does not support it, issue an error.  The GNU linker
// only issues a non-PIC error for an allocated read-only section.
// Here we know the section is allocated, but we don't know that it is
// read-only.  But we check for all the relocation types which the
// glibc dynamic linker supports, so it seems appropriate to issue an
// error even if the section is not read-only.  If GSYM is not NULL,
// it is the symbol the relocation is against; if it is NULL, the
// relocation is against a local symbol.

template<int size>
void
Target_x86_64<size>::Scan::check_non_pic(Relobj* object, unsigned int r_type,
					 Symbol* gsym)
{
  switch (r_type)
    {
      // These are the relocation types supported by glibc for x86_64
      // which should always work.
    case elfcpp::R_X86_64_RELATIVE:
    case elfcpp::R_X86_64_IRELATIVE:
    case elfcpp::R_X86_64_GLOB_DAT:
    case elfcpp::R_X86_64_JUMP_SLOT:
    case elfcpp::R_X86_64_DTPMOD64:
    case elfcpp::R_X86_64_DTPOFF64:
    case elfcpp::R_X86_64_TPOFF64:
    case elfcpp::R_X86_64_64:
    case elfcpp::R_X86_64_COPY:
      return;

      // glibc supports these reloc types, but they can overflow.
    case elfcpp::R_X86_64_PC32:
      // A PC relative reference is OK against a local symbol or if
      // the symbol is defined locally.
      if (gsym == NULL
	  || (!gsym->is_from_dynobj()
	      && !gsym->is_undefined()
	      && !gsym->is_preemptible()))
	return;
      // Fall through.
    case elfcpp::R_X86_64_32:
      // R_X86_64_32 is OK for x32.
      if (size == 32 && r_type == elfcpp::R_X86_64_32)
	return;
      if (this->issued_non_pic_error_)
	return;
      gold_assert(parameters->options().output_is_position_independent());
      if (gsym == NULL)
	object->error(_("requires dynamic R_X86_64_32 reloc which may "
			"overflow at runtime; recompile with -fPIC"));
      else
	{
	  const char *r_name;
	  switch (r_type)
	    {
	    case elfcpp::R_X86_64_32:
	      r_name = "R_X86_64_32";
	      break;
	    case elfcpp::R_X86_64_PC32:
	      r_name = "R_X86_64_PC32";
	      break;
	    default:
	      gold_unreachable();
	      break;
	    }
	  object->error(_("requires dynamic %s reloc against '%s' "
			  "which may overflow at runtime; recompile "
			  "with -fPIC"),
			r_name, gsym->name());
	}
      this->issued_non_pic_error_ = true;
      return;

    default:
      // This prevents us from issuing more than one error per reloc
      // section.  But we can still wind up issuing more than one
      // error per object file.
      if (this->issued_non_pic_error_)
	return;
      gold_assert(parameters->options().output_is_position_independent());
      object->error(_("requires unsupported dynamic reloc %u; "
		      "recompile with -fPIC"),
		    r_type);
      this->issued_non_pic_error_ = true;
      return;

    case elfcpp::R_X86_64_NONE:
      gold_unreachable();
    }
}

// Return whether we need to make a PLT entry for a relocation of the
// given type against a STT_GNU_IFUNC symbol.

template<int size>
bool
Target_x86_64<size>::Scan::reloc_needs_plt_for_ifunc(
     Sized_relobj_file<size, false>* object,
     unsigned int r_type)
{
  int flags = Scan::get_reference_flags(r_type);
  if (flags & Symbol::TLS_REF)
    gold_error(_("%s: unsupported TLS reloc %u for IFUNC symbol"),
	       object->name().c_str(), r_type);
  return flags != 0;
}

// Scan a relocation for a local symbol.

template<int size>
inline void
Target_x86_64<size>::Scan::local(Symbol_table* symtab,
				 Layout* layout,
				 Target_x86_64<size>* target,
				 Sized_relobj_file<size, false>* object,
				 unsigned int data_shndx,
				 Output_section* output_section,
				 const elfcpp::Rela<size, false>& reloc,
				 unsigned int r_type,
				 const elfcpp::Sym<size, false>& lsym,
				 bool is_discarded)
{
  if (is_discarded)
    return;

  // A local STT_GNU_IFUNC symbol may require a PLT entry.
  bool is_ifunc = lsym.get_st_type() == elfcpp::STT_GNU_IFUNC;
  if (is_ifunc && this->reloc_needs_plt_for_ifunc(object, r_type))
    {
      unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
      target->make_local_ifunc_plt_entry(symtab, layout, object, r_sym);
    }

  switch (r_type)
    {
    case elfcpp::R_X86_64_NONE:
    case elfcpp::R_X86_64_GNU_VTINHERIT:
    case elfcpp::R_X86_64_GNU_VTENTRY:
      break;

    case elfcpp::R_X86_64_64:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for this
      // location.  The relocation applied at link time will apply the
      // link-time value, so we flag the location with an
      // R_X86_64_RELATIVE relocation so the dynamic loader can
      // relocate it easily.
      if (parameters->options().output_is_position_independent())
	{
	  unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
	  Reloc_section* rela_dyn = target->rela_dyn_section(layout);
	  rela_dyn->add_local_relative(object, r_sym,
				       (size == 32
					? elfcpp::R_X86_64_RELATIVE64
					: elfcpp::R_X86_64_RELATIVE),
				       output_section, data_shndx,
				       reloc.get_r_offset(),
				       reloc.get_r_addend(), is_ifunc);
	}
      break;

    case elfcpp::R_X86_64_32:
    case elfcpp::R_X86_64_32S:
    case elfcpp::R_X86_64_16:
    case elfcpp::R_X86_64_8:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for this
      // location.  We can't use an R_X86_64_RELATIVE relocation
      // because that is always a 64-bit relocation.
      if (parameters->options().output_is_position_independent())
	{
	  // Use R_X86_64_RELATIVE relocation for R_X86_64_32 under x32.
	  if (size == 32 && r_type == elfcpp::R_X86_64_32)
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
	      Reloc_section* rela_dyn = target->rela_dyn_section(layout);
	      rela_dyn->add_local_relative(object, r_sym,
					   elfcpp::R_X86_64_RELATIVE,
					   output_section, data_shndx,
					   reloc.get_r_offset(),
					   reloc.get_r_addend(), is_ifunc);
	      break;
	    }

	  this->check_non_pic(object, r_type, NULL);

	  Reloc_section* rela_dyn = target->rela_dyn_section(layout);
	  unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
	  if (lsym.get_st_type() != elfcpp::STT_SECTION)
	    rela_dyn->add_local(object, r_sym, r_type, output_section,
				data_shndx, reloc.get_r_offset(),
				reloc.get_r_addend());
	  else
	    {
	      gold_assert(lsym.get_st_value() == 0);
	      unsigned int shndx = lsym.get_st_shndx();
	      bool is_ordinary;
	      shndx = object->adjust_sym_shndx(r_sym, shndx,
					       &is_ordinary);
	      if (!is_ordinary)
		object->error(_("section symbol %u has bad shndx %u"),
			      r_sym, shndx);
	      else
		rela_dyn->add_local_section(object, shndx,
					    r_type, output_section,
					    data_shndx, reloc.get_r_offset(),
					    reloc.get_r_addend());
	    }
	}
      break;

    case elfcpp::R_X86_64_PC64:
    case elfcpp::R_X86_64_PC32:
    case elfcpp::R_X86_64_PC16:
    case elfcpp::R_X86_64_PC8:
      break;

    case elfcpp::R_X86_64_PLT32:
      // Since we know this is a local symbol, we can handle this as a
      // PC32 reloc.
      break;

    case elfcpp::R_X86_64_GOTPC32:
    case elfcpp::R_X86_64_GOTOFF64:
    case elfcpp::R_X86_64_GOTPC64:
    case elfcpp::R_X86_64_PLTOFF64:
      // We need a GOT section.
      target->got_section(symtab, layout);
      // For PLTOFF64, we'd normally want a PLT section, but since we
      // know this is a local symbol, no PLT is needed.
      break;

    case elfcpp::R_X86_64_GOT64:
    case elfcpp::R_X86_64_GOT32:
    case elfcpp::R_X86_64_GOTPCREL64:
    case elfcpp::R_X86_64_GOTPCREL:
    case elfcpp::R_X86_64_GOTPCRELX:
    case elfcpp::R_X86_64_REX_GOTPCRELX:
    case elfcpp::R_X86_64_GOTPLT64:
      {
	// The symbol requires a GOT section.
	Output_data_got<64, false>* got = target->got_section(symtab, layout);

	// If the relocation symbol isn't IFUNC,
	// and is local, then we will convert
	// mov foo@GOTPCREL(%rip), %reg
	// to lea foo(%rip), %reg.
	// in Relocate::relocate.
	if (!parameters->incremental()
	    && (r_type == elfcpp::R_X86_64_GOTPCREL
		|| r_type == elfcpp::R_X86_64_GOTPCRELX
		|| r_type == elfcpp::R_X86_64_REX_GOTPCRELX)
	    && reloc.get_r_addend() == -4
	    && reloc.get_r_offset() >= 2
	    && !is_ifunc)
	  {
	    section_size_type stype;
	    const unsigned char* view = object->section_contents(data_shndx,
								 &stype, true);
	    if (view[reloc.get_r_offset() - 2] == 0x8b)
	      break;
	  }

	// The symbol requires a GOT entry.
	unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());

	// For a STT_GNU_IFUNC symbol we want the PLT offset.  That
	// lets function pointers compare correctly with shared
	// libraries.  Otherwise we would need an IRELATIVE reloc.
	bool is_new;
	if (is_ifunc)
	  is_new = got->add_local_plt(object, r_sym, GOT_TYPE_STANDARD);
	else
	  is_new = got->add_local(object, r_sym, GOT_TYPE_STANDARD);
	if (is_new)
	  {
	    // If we are generating a shared object, we need to add a
	    // dynamic relocation for this symbol's GOT entry.
	    if (parameters->options().output_is_position_independent())
	      {
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		// R_X86_64_RELATIVE assumes a 64-bit relocation.
		if (r_type != elfcpp::R_X86_64_GOT32)
		  {
		    unsigned int got_offset =
		      object->local_got_offset(r_sym, GOT_TYPE_STANDARD);
		    rela_dyn->add_local_relative(object, r_sym,
						 elfcpp::R_X86_64_RELATIVE,
						 got, got_offset, 0, is_ifunc);
		  }
		else
		  {
		    this->check_non_pic(object, r_type, NULL);

		    gold_assert(lsym.get_st_type() != elfcpp::STT_SECTION);
		    rela_dyn->add_local(
			object, r_sym, r_type, got,
			object->local_got_offset(r_sym, GOT_TYPE_STANDARD), 0);
		  }
	      }
	  }
	// For GOTPLT64, we'd normally want a PLT section, but since
	// we know this is a local symbol, no PLT is needed.
      }
      break;

    case elfcpp::R_X86_64_COPY:
    case elfcpp::R_X86_64_GLOB_DAT:
    case elfcpp::R_X86_64_JUMP_SLOT:
    case elfcpp::R_X86_64_RELATIVE:
    case elfcpp::R_X86_64_IRELATIVE:
      // These are outstanding tls relocs, which are unexpected when linking
    case elfcpp::R_X86_64_TPOFF64:
    case elfcpp::R_X86_64_DTPMOD64:
    case elfcpp::R_X86_64_TLSDESC:
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

      // These are initial tls relocs, which are expected when linking
    case elfcpp::R_X86_64_TLSGD:            // Global-dynamic
    case elfcpp::R_X86_64_GOTPC32_TLSDESC:  // Global-dynamic (from ~oliva url)
    case elfcpp::R_X86_64_TLSDESC_CALL:
    case elfcpp::R_X86_64_TLSLD:            // Local-dynamic
    case elfcpp::R_X86_64_DTPOFF32:
    case elfcpp::R_X86_64_DTPOFF64:
    case elfcpp::R_X86_64_GOTTPOFF:         // Initial-exec
    case elfcpp::R_X86_64_TPOFF32:          // Local-exec
      {
	bool output_is_shared = parameters->options().shared();
	const tls::Tls_optimization optimized_type
	    = Target_x86_64<size>::optimize_tls_reloc(!output_is_shared,
						      r_type);
	switch (r_type)
	  {
	  case elfcpp::R_X86_64_TLSGD:       // General-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Output_data_got<64, false>* got
		    = target->got_section(symtab, layout);
		unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
		unsigned int shndx = lsym.get_st_shndx();
		bool is_ordinary;
		shndx = object->adjust_sym_shndx(r_sym, shndx, &is_ordinary);
		if (!is_ordinary)
		  object->error(_("local symbol %u has bad shndx %u"),
			      r_sym, shndx);
		else
		  got->add_local_pair_with_rel(object, r_sym,
					       shndx,
					       GOT_TYPE_TLS_PAIR,
					       target->rela_dyn_section(layout),
					       elfcpp::R_X86_64_DTPMOD64);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_X86_64_GOTPC32_TLSDESC:
	    target->define_tls_base_symbol(symtab, layout);
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create reserved PLT and GOT entries for the resolver.
		target->reserve_tlsdesc_entries(symtab, layout);

		// Generate a double GOT entry with an
		// R_X86_64_TLSDESC reloc.  The R_X86_64_TLSDESC reloc
		// is resolved lazily, so the GOT entry needs to be in
		// an area in .got.plt, not .got.  Call got_section to
		// make sure the section has been created.
		target->got_section(symtab, layout);
		Output_data_got<64, false>* got = target->got_tlsdesc_section();
		unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
		if (!object->local_has_got_offset(r_sym, GOT_TYPE_TLS_DESC))
		  {
		    unsigned int got_offset = got->add_constant(0);
		    got->add_constant(0);
		    object->set_local_got_offset(r_sym, GOT_TYPE_TLS_DESC,
						 got_offset);
		    Reloc_section* rt = target->rela_tlsdesc_section(layout);
		    // We store the arguments we need in a vector, and
		    // use the index into the vector as the parameter
		    // to pass to the target specific routines.
		    uintptr_t intarg = target->add_tlsdesc_info(object, r_sym);
		    void* arg = reinterpret_cast<void*>(intarg);
		    rt->add_target_specific(elfcpp::R_X86_64_TLSDESC, arg,
					    got, got_offset, 0);
		  }
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_X86_64_TLSDESC_CALL:
	    break;

	  case elfcpp::R_X86_64_TLSLD:       // Local-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_X86_64_DTPOFF32:
	  case elfcpp::R_X86_64_DTPOFF64:
	    break;

	  case elfcpp::R_X86_64_GOTTPOFF:    // Initial-exec
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<64, false>* got
		    = target->got_section(symtab, layout);
		unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
		got->add_local_with_rel(object, r_sym, GOT_TYPE_TLS_OFFSET,
					target->rela_dyn_section(layout),
					elfcpp::R_X86_64_TPOFF64);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_X86_64_TPOFF32:     // Local-exec
	    layout->set_has_static_tls();
	    if (output_is_shared)
	      unsupported_reloc_local(object, r_type);
	    break;

	  default:
	    gold_unreachable();
	  }
      }
      break;

    case elfcpp::R_X86_64_SIZE32:
    case elfcpp::R_X86_64_SIZE64:
    default:
      gold_error(_("%s: unsupported reloc %u against local symbol"),
		 object->name().c_str(), r_type);
      break;
    }
}


// Report an unsupported relocation against a global symbol.

template<int size>
void
Target_x86_64<size>::Scan::unsupported_reloc_global(
    Sized_relobj_file<size, false>* object,
    unsigned int r_type,
    Symbol* gsym)
{
  gold_error(_("%s: unsupported reloc %u against global symbol %s"),
	     object->name().c_str(), r_type, gsym->demangled_name().c_str());
}

// Returns true if this relocation type could be that of a function pointer.
template<int size>
inline bool
Target_x86_64<size>::Scan::possible_function_pointer_reloc(
    Sized_relobj_file<size, false>* src_obj,
    unsigned int src_indx,
    unsigned int r_offset,
    unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_X86_64_64:
    case elfcpp::R_X86_64_32:
    case elfcpp::R_X86_64_32S:
    case elfcpp::R_X86_64_16:
    case elfcpp::R_X86_64_8:
    case elfcpp::R_X86_64_GOT64:
    case elfcpp::R_X86_64_GOT32:
    case elfcpp::R_X86_64_GOTPCREL64:
    case elfcpp::R_X86_64_GOTPCREL:
    case elfcpp::R_X86_64_GOTPCRELX:
    case elfcpp::R_X86_64_REX_GOTPCRELX:
    case elfcpp::R_X86_64_GOTPLT64:
      {
	return true;
      }
    case elfcpp::R_X86_64_PC32:
      {
        // This relocation may be used both for function calls and
        // for taking address of a function. We distinguish between
        // them by checking the opcodes.
        uint64_t sh_flags = src_obj->section_flags(src_indx);
        bool is_executable = (sh_flags & elfcpp::SHF_EXECINSTR) != 0;
        if (is_executable)
          {
            section_size_type stype;
            const unsigned char* view = src_obj->section_contents(src_indx,
                                                                  &stype,
                                                                  true);

            // call
            if (r_offset >= 1
                && view[r_offset - 1] == 0xe8)
              return false;

            // jmp
            if (r_offset >= 1
                && view[r_offset - 1] == 0xe9)
              return false;

            // jo/jno/jb/jnb/je/jne/jna/ja/js/jns/jp/jnp/jl/jge/jle/jg
            if (r_offset >= 2
                && view[r_offset - 2] == 0x0f
                && view[r_offset - 1] >= 0x80
                && view[r_offset - 1] <= 0x8f)
              return false;
          }

        // Be conservative and treat all others as function pointers.
        return true;
      }
    }
  return false;
}

// For safe ICF, scan a relocation for a local symbol to check if it
// corresponds to a function pointer being taken.  In that case mark
// the function whose pointer was taken as not foldable.

template<int size>
inline bool
Target_x86_64<size>::Scan::local_reloc_may_be_function_pointer(
  Symbol_table* ,
  Layout* ,
  Target_x86_64<size>* ,
  Sized_relobj_file<size, false>* src_obj,
  unsigned int src_indx,
  Output_section* ,
  const elfcpp::Rela<size, false>& reloc,
  unsigned int r_type,
  const elfcpp::Sym<size, false>&)
{
  return possible_function_pointer_reloc(src_obj, src_indx,
                                         reloc.get_r_offset(), r_type);
}

// For safe ICF, scan a relocation for a global symbol to check if it
// corresponds to a function pointer being taken.  In that case mark
// the function whose pointer was taken as not foldable.

template<int size>
inline bool
Target_x86_64<size>::Scan::global_reloc_may_be_function_pointer(
  Symbol_table*,
  Layout* ,
  Target_x86_64<size>* ,
  Sized_relobj_file<size, false>* src_obj,
  unsigned int src_indx,
  Output_section* ,
  const elfcpp::Rela<size, false>& reloc,
  unsigned int r_type,
  Symbol*)
{
  return possible_function_pointer_reloc(src_obj, src_indx,
                                         reloc.get_r_offset(), r_type);
}

// Scan a relocation for a global symbol.

template<int size>
inline void
Target_x86_64<size>::Scan::global(Symbol_table* symtab,
			    Layout* layout,
			    Target_x86_64<size>* target,
			    Sized_relobj_file<size, false>* object,
			    unsigned int data_shndx,
			    Output_section* output_section,
			    const elfcpp::Rela<size, false>& reloc,
			    unsigned int r_type,
			    Symbol* gsym)
{
  // A STT_GNU_IFUNC symbol may require a PLT entry.
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && this->reloc_needs_plt_for_ifunc(object, r_type))
    target->make_plt_entry(symtab, layout, gsym);

  switch (r_type)
    {
    case elfcpp::R_X86_64_NONE:
    case elfcpp::R_X86_64_GNU_VTINHERIT:
    case elfcpp::R_X86_64_GNU_VTENTRY:
      break;

    case elfcpp::R_X86_64_64:
    case elfcpp::R_X86_64_32:
    case elfcpp::R_X86_64_32S:
    case elfcpp::R_X86_64_16:
    case elfcpp::R_X86_64_8:
      {
	// Make a PLT entry if necessary.
	if (gsym->needs_plt_entry())
	  {
	    target->make_plt_entry(symtab, layout, gsym);
	    // Since this is not a PC-relative relocation, we may be
	    // taking the address of a function. In that case we need to
	    // set the entry in the dynamic symbol table to the address of
	    // the PLT entry.
	    if (gsym->is_from_dynobj() && !parameters->options().shared())
	      gsym->set_needs_dynsym_value();
	  }
	// Make a dynamic relocation if necessary.
	if (gsym->needs_dynamic_reloc(Scan::get_reference_flags(r_type)))
	  {
	    if (!parameters->options().output_is_position_independent()
		&& gsym->may_need_copy_reloc())
	      {
		target->copy_reloc(symtab, layout, object,
				   data_shndx, output_section, gsym, reloc);
	      }
	    else if (((size == 64 && r_type == elfcpp::R_X86_64_64)
		      || (size == 32 && r_type == elfcpp::R_X86_64_32))
		     && gsym->type() == elfcpp::STT_GNU_IFUNC
		     && gsym->can_use_relative_reloc(false)
		     && !gsym->is_from_dynobj()
		     && !gsym->is_undefined()
		     && !gsym->is_preemptible())
	      {
		// Use an IRELATIVE reloc for a locally defined
		// STT_GNU_IFUNC symbol.  This makes a function
		// address in a PIE executable match the address in a
		// shared library that it links against.
		Reloc_section* rela_dyn =
		  target->rela_irelative_section(layout);
		unsigned int r_type = elfcpp::R_X86_64_IRELATIVE;
		rela_dyn->add_symbolless_global_addend(gsym, r_type,
						       output_section, object,
						       data_shndx,
						       reloc.get_r_offset(),
						       reloc.get_r_addend());
	      }
	    else if (((size == 64 && r_type == elfcpp::R_X86_64_64)
		      || (size == 32 && r_type == elfcpp::R_X86_64_32))
		     && gsym->can_use_relative_reloc(false))
	      {
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_global_relative(gsym, elfcpp::R_X86_64_RELATIVE,
					      output_section, object,
					      data_shndx,
					      reloc.get_r_offset(),
					      reloc.get_r_addend(), false);
	      }
	    else
	      {
		this->check_non_pic(object, r_type, gsym);
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_global(gsym, r_type, output_section, object,
				     data_shndx, reloc.get_r_offset(),
				     reloc.get_r_addend());
	      }
	  }
      }
      break;

    case elfcpp::R_X86_64_PC64:
    case elfcpp::R_X86_64_PC32:
    case elfcpp::R_X86_64_PC16:
    case elfcpp::R_X86_64_PC8:
      {
	// Make a PLT entry if necessary.
	if (gsym->needs_plt_entry())
	  target->make_plt_entry(symtab, layout, gsym);
	// Make a dynamic relocation if necessary.
	if (gsym->needs_dynamic_reloc(Scan::get_reference_flags(r_type)))
	  {
	    if (parameters->options().output_is_executable()
		&& gsym->may_need_copy_reloc())
	      {
		target->copy_reloc(symtab, layout, object,
				   data_shndx, output_section, gsym, reloc);
	      }
	    else
	      {
		this->check_non_pic(object, r_type, gsym);
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_global(gsym, r_type, output_section, object,
				     data_shndx, reloc.get_r_offset(),
				     reloc.get_r_addend());
	      }
	  }
      }
      break;

    case elfcpp::R_X86_64_GOT64:
    case elfcpp::R_X86_64_GOT32:
    case elfcpp::R_X86_64_GOTPCREL64:
    case elfcpp::R_X86_64_GOTPCREL:
    case elfcpp::R_X86_64_GOTPCRELX:
    case elfcpp::R_X86_64_REX_GOTPCRELX:
    case elfcpp::R_X86_64_GOTPLT64:
      {
	// The symbol requires a GOT entry.
	Output_data_got<64, false>* got = target->got_section(symtab, layout);

	// If we convert this from
	// mov foo@GOTPCREL(%rip), %reg
	// to lea foo(%rip), %reg.
	// OR
	// if we convert
	// (callq|jmpq) *foo@GOTPCRELX(%rip) to
	// (callq|jmpq) foo
	// in Relocate::relocate, then there is nothing to do here.
	// We cannot make these optimizations in incremental linking mode,
	// because we look at the opcode to decide whether or not to make
	// change, and during an incremental update, the change may have
	// already been applied.

        Lazy_view<size> view(object, data_shndx);
        size_t r_offset = reloc.get_r_offset();
        if (!parameters->incremental()
	    && reloc.get_r_addend() == -4
	    && r_offset >= 2
            && Target_x86_64<size>::can_convert_mov_to_lea(gsym, r_type,
                                                           r_offset, &view))
          break;

	if (!parameters->incremental()
	    && r_offset >= 2
	    && Target_x86_64<size>::can_convert_callq_to_direct(gsym, r_type,
								r_offset,
								&view))
          break;

	if (gsym->final_value_is_known())
	  {
	    // For a STT_GNU_IFUNC symbol we want the PLT address.
	    if (gsym->type() == elfcpp::STT_GNU_IFUNC)
	      got->add_global_plt(gsym, GOT_TYPE_STANDARD);
	    else
	      got->add_global(gsym, GOT_TYPE_STANDARD);
	  }
	else
	  {
	    // If this symbol is not fully resolved, we need to add a
	    // dynamic relocation for it.
	    Reloc_section* rela_dyn = target->rela_dyn_section(layout);

	    // Use a GLOB_DAT rather than a RELATIVE reloc if:
	    //
	    // 1) The symbol may be defined in some other module.
	    //
	    // 2) We are building a shared library and this is a
	    // protected symbol; using GLOB_DAT means that the dynamic
	    // linker can use the address of the PLT in the main
	    // executable when appropriate so that function address
	    // comparisons work.
	    //
	    // 3) This is a STT_GNU_IFUNC symbol in position dependent
	    // code, again so that function address comparisons work.
	    if (gsym->is_from_dynobj()
		|| gsym->is_undefined()
		|| gsym->is_preemptible()
		|| (gsym->visibility() == elfcpp::STV_PROTECTED
		    && parameters->options().shared())
		|| (gsym->type() == elfcpp::STT_GNU_IFUNC
		    && parameters->options().output_is_position_independent()))
	      got->add_global_with_rel(gsym, GOT_TYPE_STANDARD, rela_dyn,
				       elfcpp::R_X86_64_GLOB_DAT);
	    else
	      {
		// For a STT_GNU_IFUNC symbol we want to write the PLT
		// offset into the GOT, so that function pointer
		// comparisons work correctly.
		bool is_new;
		if (gsym->type() != elfcpp::STT_GNU_IFUNC)
		  is_new = got->add_global(gsym, GOT_TYPE_STANDARD);
		else
		  {
		    is_new = got->add_global_plt(gsym, GOT_TYPE_STANDARD);
		    // Tell the dynamic linker to use the PLT address
		    // when resolving relocations.
		    if (gsym->is_from_dynobj()
			&& !parameters->options().shared())
		      gsym->set_needs_dynsym_value();
		  }
		if (is_new)
		  {
		    unsigned int got_off = gsym->got_offset(GOT_TYPE_STANDARD);
		    rela_dyn->add_global_relative(gsym,
						  elfcpp::R_X86_64_RELATIVE,
						  got, got_off, 0, false);
		  }
	      }
	  }
      }
      break;

    case elfcpp::R_X86_64_PLT32:
      // If the symbol is fully resolved, this is just a PC32 reloc.
      // Otherwise we need a PLT entry.
      if (gsym->final_value_is_known())
	break;
      // If building a shared library, we can also skip the PLT entry
      // if the symbol is defined in the output file and is protected
      // or hidden.
      if (gsym->is_defined()
	  && !gsym->is_from_dynobj()
	  && !gsym->is_preemptible())
	break;
      target->make_plt_entry(symtab, layout, gsym);
      break;

    case elfcpp::R_X86_64_GOTPC32:
    case elfcpp::R_X86_64_GOTOFF64:
    case elfcpp::R_X86_64_GOTPC64:
    case elfcpp::R_X86_64_PLTOFF64:
      // We need a GOT section.
      target->got_section(symtab, layout);
      // For PLTOFF64, we also need a PLT entry (but only if the
      // symbol is not fully resolved).
      if (r_type == elfcpp::R_X86_64_PLTOFF64
	  && !gsym->final_value_is_known())
	target->make_plt_entry(symtab, layout, gsym);
      break;

    case elfcpp::R_X86_64_COPY:
    case elfcpp::R_X86_64_GLOB_DAT:
    case elfcpp::R_X86_64_JUMP_SLOT:
    case elfcpp::R_X86_64_RELATIVE:
    case elfcpp::R_X86_64_IRELATIVE:
      // These are outstanding tls relocs, which are unexpected when linking
    case elfcpp::R_X86_64_TPOFF64:
    case elfcpp::R_X86_64_DTPMOD64:
    case elfcpp::R_X86_64_TLSDESC:
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

      // These are initial tls relocs, which are expected for global()
    case elfcpp::R_X86_64_TLSGD:            // Global-dynamic
    case elfcpp::R_X86_64_GOTPC32_TLSDESC:  // Global-dynamic (from ~oliva url)
    case elfcpp::R_X86_64_TLSDESC_CALL:
    case elfcpp::R_X86_64_TLSLD:            // Local-dynamic
    case elfcpp::R_X86_64_DTPOFF32:
    case elfcpp::R_X86_64_DTPOFF64:
    case elfcpp::R_X86_64_GOTTPOFF:         // Initial-exec
    case elfcpp::R_X86_64_TPOFF32:          // Local-exec
      {
	// For the Initial-Exec model, we can treat undef symbols as final
	// when building an executable.
	const bool is_final = (gsym->final_value_is_known() ||
			       (r_type == elfcpp::R_X86_64_GOTTPOFF &&
			        gsym->is_undefined() &&
				parameters->options().output_is_executable()));
	const tls::Tls_optimization optimized_type
	    = Target_x86_64<size>::optimize_tls_reloc(is_final, r_type);
	switch (r_type)
	  {
	  case elfcpp::R_X86_64_TLSGD:       // General-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Output_data_got<64, false>* got
		    = target->got_section(symtab, layout);
		got->add_global_pair_with_rel(gsym, GOT_TYPE_TLS_PAIR,
					      target->rela_dyn_section(layout),
					      elfcpp::R_X86_64_DTPMOD64,
					      elfcpp::R_X86_64_DTPOFF64);
	      }
	    else if (optimized_type == tls::TLSOPT_TO_IE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<64, false>* got
		    = target->got_section(symtab, layout);
		got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
					 target->rela_dyn_section(layout),
					 elfcpp::R_X86_64_TPOFF64);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_X86_64_GOTPC32_TLSDESC:
	    target->define_tls_base_symbol(symtab, layout);
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create reserved PLT and GOT entries for the resolver.
		target->reserve_tlsdesc_entries(symtab, layout);

		// Create a double GOT entry with an R_X86_64_TLSDESC
		// reloc.  The R_X86_64_TLSDESC reloc is resolved
		// lazily, so the GOT entry needs to be in an area in
		// .got.plt, not .got.  Call got_section to make sure
		// the section has been created.
		target->got_section(symtab, layout);
		Output_data_got<64, false>* got = target->got_tlsdesc_section();
		Reloc_section* rt = target->rela_tlsdesc_section(layout);
		got->add_global_pair_with_rel(gsym, GOT_TYPE_TLS_DESC, rt,
					      elfcpp::R_X86_64_TLSDESC, 0);
	      }
	    else if (optimized_type == tls::TLSOPT_TO_IE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<64, false>* got
		    = target->got_section(symtab, layout);
		got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
					 target->rela_dyn_section(layout),
					 elfcpp::R_X86_64_TPOFF64);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_X86_64_TLSDESC_CALL:
	    break;

	  case elfcpp::R_X86_64_TLSLD:       // Local-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_X86_64_DTPOFF32:
	  case elfcpp::R_X86_64_DTPOFF64:
	    break;

	  case elfcpp::R_X86_64_GOTTPOFF:    // Initial-exec
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<64, false>* got
		    = target->got_section(symtab, layout);
		got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
					 target->rela_dyn_section(layout),
					 elfcpp::R_X86_64_TPOFF64);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_X86_64_TPOFF32:     // Local-exec
	    layout->set_has_static_tls();
	    if (parameters->options().shared())
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  default:
	    gold_unreachable();
	  }
      }
      break;

    case elfcpp::R_X86_64_SIZE32:
    case elfcpp::R_X86_64_SIZE64:
    default:
      gold_error(_("%s: unsupported reloc %u against global symbol %s"),
		 object->name().c_str(), r_type,
		 gsym->demangled_name().c_str());
      break;
    }
}

template<int size>
void
Target_x86_64<size>::gc_process_relocs(Symbol_table* symtab,
				       Layout* layout,
				       Sized_relobj_file<size, false>* object,
				       unsigned int data_shndx,
				       unsigned int sh_type,
				       const unsigned char* prelocs,
				       size_t reloc_count,
				       Output_section* output_section,
				       bool needs_special_offset_handling,
				       size_t local_symbol_count,
				       const unsigned char* plocal_symbols)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, false>
      Classify_reloc;

  if (sh_type == elfcpp::SHT_REL)
    {
      return;
    }

   gold::gc_process_relocs<size, false, Target_x86_64<size>, Scan,
			   Classify_reloc>(
    symtab,
    layout,
    this,
    object,
    data_shndx,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    local_symbol_count,
    plocal_symbols);

}
// Scan relocations for a section.

template<int size>
void
Target_x86_64<size>::scan_relocs(Symbol_table* symtab,
				 Layout* layout,
				 Sized_relobj_file<size, false>* object,
				 unsigned int data_shndx,
				 unsigned int sh_type,
				 const unsigned char* prelocs,
				 size_t reloc_count,
				 Output_section* output_section,
				 bool needs_special_offset_handling,
				 size_t local_symbol_count,
				 const unsigned char* plocal_symbols)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, false>
      Classify_reloc;

  if (sh_type == elfcpp::SHT_REL)
    {
      gold_error(_("%s: unsupported REL reloc section"),
		 object->name().c_str());
      return;
    }

  gold::scan_relocs<size, false, Target_x86_64<size>, Scan, Classify_reloc>(
    symtab,
    layout,
    this,
    object,
    data_shndx,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    local_symbol_count,
    plocal_symbols);
}

// Finalize the sections.

template<int size>
void
Target_x86_64<size>::do_finalize_sections(
    Layout* layout,
    const Input_objects*,
    Symbol_table* symtab)
{
  const Reloc_section* rel_plt = (this->plt_ == NULL
				  ? NULL
				  : this->plt_->rela_plt());
  layout->add_target_dynamic_tags(false, this->got_plt_, rel_plt,
				  this->rela_dyn_, true, false, false);

  // Fill in some more dynamic tags.
  Output_data_dynamic* const odyn = layout->dynamic_data();
  if (odyn != NULL)
    {
      if (this->plt_ != NULL
	  && this->plt_->output_section() != NULL
	  && this->plt_->has_tlsdesc_entry())
	{
	  unsigned int plt_offset = this->plt_->get_tlsdesc_plt_offset();
	  unsigned int got_offset = this->plt_->get_tlsdesc_got_offset();
	  this->got_->finalize_data_size();
	  odyn->add_section_plus_offset(elfcpp::DT_TLSDESC_PLT,
					this->plt_, plt_offset);
	  odyn->add_section_plus_offset(elfcpp::DT_TLSDESC_GOT,
					this->got_, got_offset);
	}
    }

  // Emit any relocs we saved in an attempt to avoid generating COPY
  // relocs.
  if (this->copy_relocs_.any_saved_relocs())
    this->copy_relocs_.emit(this->rela_dyn_section(layout));

  // Set the size of the _GLOBAL_OFFSET_TABLE_ symbol to the size of
  // the .got.plt section.
  Symbol* sym = this->global_offset_table_;
  if (sym != NULL)
    {
      uint64_t data_size = this->got_plt_->current_data_size();
      symtab->get_sized_symbol<size>(sym)->set_symsize(data_size);
    }

  if (parameters->doing_static_link()
      && (this->plt_ == NULL || !this->plt_->has_irelative_section()))
    {
      // If linking statically, make sure that the __rela_iplt symbols
      // were defined if necessary, even if we didn't create a PLT.
      static const Define_symbol_in_segment syms[] =
	{
	  {
	    "__rela_iplt_start",	// name
	    elfcpp::PT_LOAD,		// segment_type
	    elfcpp::PF_W,		// segment_flags_set
	    elfcpp::PF(0),		// segment_flags_clear
	    0,				// value
	    0,				// size
	    elfcpp::STT_NOTYPE,		// type
	    elfcpp::STB_GLOBAL,		// binding
	    elfcpp::STV_HIDDEN,		// visibility
	    0,				// nonvis
	    Symbol::SEGMENT_START,	// offset_from_base
	    true			// only_if_ref
	  },
	  {
	    "__rela_iplt_end",		// name
	    elfcpp::PT_LOAD,		// segment_type
	    elfcpp::PF_W,		// segment_flags_set
	    elfcpp::PF(0),		// segment_flags_clear
	    0,				// value
	    0,				// size
	    elfcpp::STT_NOTYPE,		// type
	    elfcpp::STB_GLOBAL,		// binding
	    elfcpp::STV_HIDDEN,		// visibility
	    0,				// nonvis
	    Symbol::SEGMENT_START,	// offset_from_base
	    true			// only_if_ref
	  }
	};

      symtab->define_symbols(layout, 2, syms,
			     layout->script_options()->saw_sections_clause());
    }
}

// For x32, we need to handle PC-relative relocations using full 64-bit
// arithmetic, so that we can detect relocation overflows properly.
// This class overrides the pcrela32_check methods from the defaults in
// Relocate_functions in reloc.h.

template<int size>
class X86_64_relocate_functions : public Relocate_functions<size, false>
{
 public:
  typedef Relocate_functions<size, false> Base;

  // Do a simple PC relative relocation with the addend in the
  // relocation.
  static inline typename Base::Reloc_status
  pcrela32_check(unsigned char* view,
		 typename elfcpp::Elf_types<64>::Elf_Addr value,
		 typename elfcpp::Elf_types<64>::Elf_Swxword addend,
		 typename elfcpp::Elf_types<64>::Elf_Addr address)
  {
    typedef typename elfcpp::Swap<32, false>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    value = value + addend - address;
    elfcpp::Swap<32, false>::writeval(wv, value);
    return (Bits<32>::has_overflow(value)
	    ? Base::RELOC_OVERFLOW : Base::RELOC_OK);
  }

  // Do a simple PC relative relocation with a Symbol_value with the
  // addend in the relocation.
  static inline typename Base::Reloc_status
  pcrela32_check(unsigned char* view,
		 const Sized_relobj_file<size, false>* object,
		 const Symbol_value<size>* psymval,
		 typename elfcpp::Elf_types<64>::Elf_Swxword addend,
		 typename elfcpp::Elf_types<64>::Elf_Addr address)
  {
    typedef typename elfcpp::Swap<32, false>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    typename elfcpp::Elf_types<64>::Elf_Addr value;
    if (addend >= 0)
      value = psymval->value(object, addend);
    else
      {
	// For negative addends, get the symbol value without
	// the addend, then add the addend using 64-bit arithmetic.
	value = psymval->value(object, 0);
	value += addend;
      }
    value -= address;
    elfcpp::Swap<32, false>::writeval(wv, value);
    return (Bits<32>::has_overflow(value)
	    ? Base::RELOC_OVERFLOW : Base::RELOC_OK);
  }
};

// Perform a relocation.

template<int size>
inline bool
Target_x86_64<size>::Relocate::relocate(
    const Relocate_info<size, false>* relinfo,
    unsigned int,
    Target_x86_64<size>* target,
    Output_section*,
    size_t relnum,
    const unsigned char* preloc,
    const Sized_symbol<size>* gsym,
    const Symbol_value<size>* psymval,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size)
{
  typedef X86_64_relocate_functions<size> Reloc_funcs;
  const elfcpp::Rela<size, false> rela(preloc);
  unsigned int r_type = elfcpp::elf_r_type<size>(rela.get_r_info());

  if (this->skip_call_tls_get_addr_)
    {
      if ((r_type != elfcpp::R_X86_64_PLT32
	   && r_type != elfcpp::R_X86_64_GOTPCREL
	   && r_type != elfcpp::R_X86_64_GOTPCRELX
	   && r_type != elfcpp::R_X86_64_PC32)
	  || gsym == NULL
	  || strcmp(gsym->name(), "__tls_get_addr") != 0)
	{
	  gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
				 _("missing expected TLS relocation"));
	  this->skip_call_tls_get_addr_ = false;
	}
      else
	{
	  this->skip_call_tls_get_addr_ = false;
	  return false;
	}
    }

  if (view == NULL)
    return true;

  const Sized_relobj_file<size, false>* object = relinfo->object;

  // Pick the value to use for symbols defined in the PLT.
  Symbol_value<size> symval;
  if (gsym != NULL
      && gsym->use_plt_offset(Scan::get_reference_flags(r_type)))
    {
      symval.set_output_value(target->plt_address_for_global(gsym));
      psymval = &symval;
    }
  else if (gsym == NULL && psymval->is_ifunc_symbol())
    {
      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
      if (object->local_has_plt_offset(r_sym))
	{
	  symval.set_output_value(target->plt_address_for_local(object, r_sym));
	  psymval = &symval;
	}
    }

  const elfcpp::Elf_Xword addend = rela.get_r_addend();

  // Get the GOT offset if needed.
  // The GOT pointer points to the end of the GOT section.
  // We need to subtract the size of the GOT section to get
  // the actual offset to use in the relocation.
  bool have_got_offset = false;
  // Since the actual offset is always negative, we use signed int to
  // support 64-bit GOT relocations.
  int got_offset = 0;
  switch (r_type)
    {
    case elfcpp::R_X86_64_GOT32:
    case elfcpp::R_X86_64_GOT64:
    case elfcpp::R_X86_64_GOTPLT64:
    case elfcpp::R_X86_64_GOTPCREL64:
      if (gsym != NULL)
	{
	  gold_assert(gsym->has_got_offset(GOT_TYPE_STANDARD));
	  got_offset = gsym->got_offset(GOT_TYPE_STANDARD) - target->got_size();
	}
      else
	{
	  unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	  gold_assert(object->local_has_got_offset(r_sym, GOT_TYPE_STANDARD));
	  got_offset = (object->local_got_offset(r_sym, GOT_TYPE_STANDARD)
			- target->got_size());
	}
      have_got_offset = true;
      break;

    default:
      break;
    }

  typename Reloc_funcs::Reloc_status rstatus = Reloc_funcs::RELOC_OK;

  switch (r_type)
    {
    case elfcpp::R_X86_64_NONE:
    case elfcpp::R_X86_64_GNU_VTINHERIT:
    case elfcpp::R_X86_64_GNU_VTENTRY:
      break;

    case elfcpp::R_X86_64_64:
      Reloc_funcs::rela64(view, object, psymval, addend);
      break;

    case elfcpp::R_X86_64_PC64:
      Reloc_funcs::pcrela64(view, object, psymval, addend,
					      address);
      break;

    case elfcpp::R_X86_64_32:
      rstatus = Reloc_funcs::rela32_check(view, object, psymval, addend,
					  Reloc_funcs::CHECK_UNSIGNED);
      break;

    case elfcpp::R_X86_64_32S:
      rstatus = Reloc_funcs::rela32_check(view, object, psymval, addend,
					  Reloc_funcs::CHECK_SIGNED);
      break;

    case elfcpp::R_X86_64_PC32:
      rstatus = Reloc_funcs::pcrela32_check(view, object, psymval, addend,
					    address);
      break;

    case elfcpp::R_X86_64_16:
      Reloc_funcs::rela16(view, object, psymval, addend);
      break;

    case elfcpp::R_X86_64_PC16:
      Reloc_funcs::pcrela16(view, object, psymval, addend, address);
      break;

    case elfcpp::R_X86_64_8:
      Reloc_funcs::rela8(view, object, psymval, addend);
      break;

    case elfcpp::R_X86_64_PC8:
      Reloc_funcs::pcrela8(view, object, psymval, addend, address);
      break;

    case elfcpp::R_X86_64_PLT32:
      gold_assert(gsym == NULL
		  || gsym->has_plt_offset()
		  || gsym->final_value_is_known()
		  || (gsym->is_defined()
		      && !gsym->is_from_dynobj()
		      && !gsym->is_preemptible()));
      // Note: while this code looks the same as for R_X86_64_PC32, it
      // behaves differently because psymval was set to point to
      // the PLT entry, rather than the symbol, in Scan::global().
      rstatus = Reloc_funcs::pcrela32_check(view, object, psymval, addend,
					    address);
      break;

    case elfcpp::R_X86_64_PLTOFF64:
      {
	gold_assert(gsym);
	gold_assert(gsym->has_plt_offset()
		    || gsym->final_value_is_known());
	typename elfcpp::Elf_types<size>::Elf_Addr got_address;
	// This is the address of GLOBAL_OFFSET_TABLE.
	got_address = target->got_plt_section()->address();
	Reloc_funcs::rela64(view, object, psymval, addend - got_address);
      }
      break;

    case elfcpp::R_X86_64_GOT32:
      gold_assert(have_got_offset);
      Reloc_funcs::rela32(view, got_offset, addend);
      break;

    case elfcpp::R_X86_64_GOTPC32:
      {
	gold_assert(gsym);
	typename elfcpp::Elf_types<size>::Elf_Addr value;
	value = target->got_plt_section()->address();
	Reloc_funcs::pcrela32_check(view, value, addend, address);
      }
      break;

    case elfcpp::R_X86_64_GOT64:
    case elfcpp::R_X86_64_GOTPLT64:
      // R_X86_64_GOTPLT64 is obsolete and treated the same as
      // GOT64.
      gold_assert(have_got_offset);
      Reloc_funcs::rela64(view, got_offset, addend);
      break;

    case elfcpp::R_X86_64_GOTPC64:
      {
	gold_assert(gsym);
	typename elfcpp::Elf_types<size>::Elf_Addr value;
	value = target->got_plt_section()->address();
	Reloc_funcs::pcrela64(view, value, addend, address);
      }
      break;

    case elfcpp::R_X86_64_GOTOFF64:
      {
	typename elfcpp::Elf_types<size>::Elf_Addr reladdr;
	reladdr = target->got_plt_section()->address();
	Reloc_funcs::pcrela64(view, object, psymval, addend, reladdr);
      }
      break;

    case elfcpp::R_X86_64_GOTPCREL:
    case elfcpp::R_X86_64_GOTPCRELX:
    case elfcpp::R_X86_64_REX_GOTPCRELX:
      {
      bool converted_p = false;

      if (rela.get_r_addend() == -4)
	{
	  // Convert
	  // mov foo@GOTPCREL(%rip), %reg
	  // to lea foo(%rip), %reg.
	  // if possible.
	  if (!parameters->incremental()
	      && ((gsym == NULL
		   && rela.get_r_offset() >= 2
		   && view[-2] == 0x8b
		   && !psymval->is_ifunc_symbol())
		  || (gsym != NULL
		      && rela.get_r_offset() >= 2
		      && Target_x86_64<size>::can_convert_mov_to_lea(gsym,
								     r_type,
								     0,
								     &view))))
	    {
	      view[-2] = 0x8d;
	      Reloc_funcs::pcrela32(view, object, psymval, addend, address);
	      converted_p = true;
	    }
	  // Convert
	  // callq *foo@GOTPCRELX(%rip) to
	  // addr32 callq foo
	  // and jmpq *foo@GOTPCRELX(%rip) to
	  // jmpq foo
	  // nop
	  else if (!parameters->incremental()
		   && gsym != NULL
		   && rela.get_r_offset() >= 2
		   && Target_x86_64<size>::can_convert_callq_to_direct(gsym,
								       r_type,
								       0,
								       &view))
	    {
	      if (view[-1] == 0x15)
		{
		  // Convert callq *foo@GOTPCRELX(%rip) to addr32 callq.
		  // Opcode of addr32 is 0x67 and opcode of direct callq
		  // is 0xe8.
		  view[-2] = 0x67;
		  view[-1] = 0xe8;
		  // Convert GOTPCRELX to 32-bit pc relative reloc.
		  Reloc_funcs::pcrela32(view, object, psymval, addend,
					address);
		  converted_p = true;
		}
	      else
		{
		  // Convert jmpq *foo@GOTPCRELX(%rip) to
		  // jmpq foo
		  // nop
		  // The opcode of direct jmpq is 0xe9.
		  view[-2] = 0xe9;
		  // The opcode of nop is 0x90.
		  view[3] = 0x90;
		  // Convert GOTPCRELX to 32-bit pc relative reloc.  jmpq
		  // is rip relative and since the instruction following
		  // the jmpq is now the nop, offset the address by 1
		  // byte.  The start of the relocation also moves ahead
		  // by 1 byte.
		  Reloc_funcs::pcrela32(&view[-1], object, psymval, addend,
					address - 1);
		  converted_p = true;
		}
	    }
	}

      if (!converted_p)
	{
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(GOT_TYPE_STANDARD));
	      got_offset = (gsym->got_offset(GOT_TYPE_STANDARD)
			    - target->got_size());
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym,
						       GOT_TYPE_STANDARD));
	      got_offset = (object->local_got_offset(r_sym, GOT_TYPE_STANDARD)
			    - target->got_size());
	    }
	  typename elfcpp::Elf_types<size>::Elf_Addr value;
	  value = target->got_plt_section()->address() + got_offset;
	  Reloc_funcs::pcrela32_check(view, value, addend, address);
	}
      }
      break;

    case elfcpp::R_X86_64_GOTPCREL64:
      {
	gold_assert(have_got_offset);
	typename elfcpp::Elf_types<size>::Elf_Addr value;
	value = target->got_plt_section()->address() + got_offset;
	Reloc_funcs::pcrela64(view, value, addend, address);
      }
      break;

    case elfcpp::R_X86_64_COPY:
    case elfcpp::R_X86_64_GLOB_DAT:
    case elfcpp::R_X86_64_JUMP_SLOT:
    case elfcpp::R_X86_64_RELATIVE:
    case elfcpp::R_X86_64_IRELATIVE:
      // These are outstanding tls relocs, which are unexpected when linking
    case elfcpp::R_X86_64_TPOFF64:
    case elfcpp::R_X86_64_DTPMOD64:
    case elfcpp::R_X86_64_TLSDESC:
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unexpected reloc %u in object file"),
			     r_type);
      break;

      // These are initial tls relocs, which are expected when linking
    case elfcpp::R_X86_64_TLSGD:            // Global-dynamic
    case elfcpp::R_X86_64_GOTPC32_TLSDESC:  // Global-dynamic (from ~oliva url)
    case elfcpp::R_X86_64_TLSDESC_CALL:
    case elfcpp::R_X86_64_TLSLD:            // Local-dynamic
    case elfcpp::R_X86_64_DTPOFF32:
    case elfcpp::R_X86_64_DTPOFF64:
    case elfcpp::R_X86_64_GOTTPOFF:         // Initial-exec
    case elfcpp::R_X86_64_TPOFF32:          // Local-exec
      this->relocate_tls(relinfo, target, relnum, rela, r_type, gsym, psymval,
			 view, address, view_size);
      break;

    case elfcpp::R_X86_64_SIZE32:
    case elfcpp::R_X86_64_SIZE64:
    default:
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;
    }

  if (rstatus == Reloc_funcs::RELOC_OVERFLOW)
    {
      if (gsym == NULL)
        {
	  unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	  gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
				 _("relocation overflow: "
				   "reference to local symbol %u in %s"),
				 r_sym, object->name().c_str());
        }
      else if (gsym->is_defined() && gsym->source() == Symbol::FROM_OBJECT)
        {
	  gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
				 _("relocation overflow: "
				   "reference to '%s' defined in %s"),
				 gsym->name(),
				 gsym->object()->name().c_str());
        }
      else
        {
	  gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
				 _("relocation overflow: reference to '%s'"),
				 gsym->name());
        }
    }

  return true;
}

// Perform a TLS relocation.

template<int size>
inline void
Target_x86_64<size>::Relocate::relocate_tls(
    const Relocate_info<size, false>* relinfo,
    Target_x86_64<size>* target,
    size_t relnum,
    const elfcpp::Rela<size, false>& rela,
    unsigned int r_type,
    const Sized_symbol<size>* gsym,
    const Symbol_value<size>* psymval,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size)
{
  Output_segment* tls_segment = relinfo->layout->tls_segment();

  const Sized_relobj_file<size, false>* object = relinfo->object;
  const elfcpp::Elf_Xword addend = rela.get_r_addend();
  elfcpp::Shdr<size, false> data_shdr(relinfo->data_shdr);
  bool is_executable = (data_shdr.get_sh_flags() & elfcpp::SHF_EXECINSTR) != 0;

  typename elfcpp::Elf_types<size>::Elf_Addr value = psymval->value(relinfo->object, 0);

  const bool is_final = (gsym == NULL
			 ? !parameters->options().shared()
			 : gsym->final_value_is_known());
  tls::Tls_optimization optimized_type
      = Target_x86_64<size>::optimize_tls_reloc(is_final, r_type);
  switch (r_type)
    {
    case elfcpp::R_X86_64_TLSGD:            // Global-dynamic
      if (!is_executable && optimized_type == tls::TLSOPT_TO_LE)
	{
	  // If this code sequence is used in a non-executable section,
	  // we will not optimize the R_X86_64_DTPOFF32/64 relocation,
	  // on the assumption that it's being used by itself in a debug
	  // section.  Therefore, in the unlikely event that the code
	  // sequence appears in a non-executable section, we simply
	  // leave it unoptimized.
	  optimized_type = tls::TLSOPT_NONE;
	}
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  this->tls_gd_to_le(relinfo, relnum, tls_segment,
			     rela, r_type, value, view,
			     view_size);
	  break;
	}
      else
	{
	  unsigned int got_type = (optimized_type == tls::TLSOPT_TO_IE
				   ? GOT_TYPE_TLS_OFFSET
				   : GOT_TYPE_TLS_PAIR);
	  unsigned int got_offset;
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(got_type));
	      got_offset = gsym->got_offset(got_type) - target->got_size();
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      got_offset = (object->local_got_offset(r_sym, got_type)
			    - target->got_size());
	    }
	  if (optimized_type == tls::TLSOPT_TO_IE)
	    {
	      value = target->got_plt_section()->address() + got_offset;
	      this->tls_gd_to_ie(relinfo, relnum, rela, r_type,
				 value, view, address, view_size);
	      break;
	    }
	  else if (optimized_type == tls::TLSOPT_NONE)
	    {
	      // Relocate the field with the offset of the pair of GOT
	      // entries.
	      value = target->got_plt_section()->address() + got_offset;
	      Relocate_functions<size, false>::pcrela32(view, value, addend,
							address);
	      break;
	    }
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"), r_type);
      break;

    case elfcpp::R_X86_64_GOTPC32_TLSDESC:  // Global-dynamic (from ~oliva url)
    case elfcpp::R_X86_64_TLSDESC_CALL:
      if (!is_executable && optimized_type == tls::TLSOPT_TO_LE)
	{
	  // See above comment for R_X86_64_TLSGD.
	  optimized_type = tls::TLSOPT_NONE;
	}
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  this->tls_desc_gd_to_le(relinfo, relnum, tls_segment,
				  rela, r_type, value, view,
				  view_size);
	  break;
	}
      else
	{
	  unsigned int got_type = (optimized_type == tls::TLSOPT_TO_IE
				   ? GOT_TYPE_TLS_OFFSET
				   : GOT_TYPE_TLS_DESC);
	  unsigned int got_offset = 0;
	  if (r_type == elfcpp::R_X86_64_GOTPC32_TLSDESC
	      && optimized_type == tls::TLSOPT_NONE)
	    {
	      // We created GOT entries in the .got.tlsdesc portion of
	      // the .got.plt section, but the offset stored in the
	      // symbol is the offset within .got.tlsdesc.
	      got_offset = (target->got_size()
			    + target->got_plt_section()->data_size());
	    }
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(got_type));
	      got_offset += gsym->got_offset(got_type) - target->got_size();
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      got_offset += (object->local_got_offset(r_sym, got_type)
			     - target->got_size());
	    }
	  if (optimized_type == tls::TLSOPT_TO_IE)
	    {
	      value = target->got_plt_section()->address() + got_offset;
	      this->tls_desc_gd_to_ie(relinfo, relnum,
				      rela, r_type, value, view, address,
				      view_size);
	      break;
	    }
	  else if (optimized_type == tls::TLSOPT_NONE)
	    {
	      if (r_type == elfcpp::R_X86_64_GOTPC32_TLSDESC)
		{
		  // Relocate the field with the offset of the pair of GOT
		  // entries.
		  value = target->got_plt_section()->address() + got_offset;
		  Relocate_functions<size, false>::pcrela32(view, value, addend,
							    address);
		}
	      break;
	    }
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"), r_type);
      break;

    case elfcpp::R_X86_64_TLSLD:            // Local-dynamic
      if (!is_executable && optimized_type == tls::TLSOPT_TO_LE)
	{
	  // See above comment for R_X86_64_TLSGD.
	  optimized_type = tls::TLSOPT_NONE;
	}
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  this->tls_ld_to_le(relinfo, relnum, tls_segment, rela, r_type,
			     value, view, view_size);
	  break;
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the module index.
	  unsigned int got_offset;
	  got_offset = (target->got_mod_index_entry(NULL, NULL, NULL)
			- target->got_size());
	  value = target->got_plt_section()->address() + got_offset;
	  Relocate_functions<size, false>::pcrela32(view, value, addend,
						    address);
	  break;
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"), r_type);
      break;

    case elfcpp::R_X86_64_DTPOFF32:
      // This relocation type is used in debugging information.
      // In that case we need to not optimize the value.  If the
      // section is not executable, then we assume we should not
      // optimize this reloc.  See comments above for R_X86_64_TLSGD,
      // R_X86_64_GOTPC32_TLSDESC, R_X86_64_TLSDESC_CALL, and
      // R_X86_64_TLSLD.
      if (optimized_type == tls::TLSOPT_TO_LE && is_executable)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  value -= tls_segment->memsz();
	}
      Relocate_functions<size, false>::rela32(view, value, addend);
      break;

    case elfcpp::R_X86_64_DTPOFF64:
      // See R_X86_64_DTPOFF32, just above, for why we check for is_executable.
      if (optimized_type == tls::TLSOPT_TO_LE && is_executable)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  value -= tls_segment->memsz();
	}
      Relocate_functions<size, false>::rela64(view, value, addend);
      break;

    case elfcpp::R_X86_64_GOTTPOFF:         // Initial-exec
      if (gsym != NULL
	  && gsym->is_undefined()
	  && parameters->options().output_is_executable())
	{
	  Target_x86_64<size>::Relocate::tls_ie_to_le(relinfo, relnum,
						      NULL, rela,
						      r_type, value, view,
						      view_size);
	  break;
	}
      else if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  Target_x86_64<size>::Relocate::tls_ie_to_le(relinfo, relnum,
						      tls_segment, rela,
						      r_type, value, view,
						      view_size);
	  break;
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the tp-relative offset of the symbol.
	  unsigned int got_offset;
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(GOT_TYPE_TLS_OFFSET));
	      got_offset = (gsym->got_offset(GOT_TYPE_TLS_OFFSET)
			    - target->got_size());
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym,
						       GOT_TYPE_TLS_OFFSET));
	      got_offset = (object->local_got_offset(r_sym, GOT_TYPE_TLS_OFFSET)
			    - target->got_size());
	    }
	  value = target->got_plt_section()->address() + got_offset;
	  Relocate_functions<size, false>::pcrela32(view, value, addend,
						    address);
	  break;
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc type %u"),
			     r_type);
      break;

    case elfcpp::R_X86_64_TPOFF32:          // Local-exec
      if (tls_segment == NULL)
	{
	  gold_assert(parameters->errors()->error_count() > 0
		      || issue_undefined_symbol_error(gsym));
	  return;
	}
      value -= tls_segment->memsz();
      Relocate_functions<size, false>::rela32(view, value, addend);
      break;
    }
}

// Do a relocation in which we convert a TLS General-Dynamic to an
// Initial-Exec.

template<int size>
inline void
Target_x86_64<size>::Relocate::tls_gd_to_ie(
    const Relocate_info<size, false>* relinfo,
    size_t relnum,
    const elfcpp::Rela<size, false>& rela,
    unsigned int,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size)
{
  // For SIZE == 64:
  //	.byte 0x66; leaq foo@tlsgd(%rip),%rdi;
  //	.word 0x6666; rex64; call __tls_get_addr@PLT
  //	==> movq %fs:0,%rax; addq x@gottpoff(%rip),%rax
  //	.byte 0x66; leaq foo@tlsgd(%rip),%rdi;
  //	.word 0x66; rex64; call *__tls_get_addr@GOTPCREL(%rip)
  //	==> movq %fs:0,%rax; addq x@gottpoff(%rip),%rax
  // For SIZE == 32:
  //	leaq foo@tlsgd(%rip),%rdi;
  //	.word 0x6666; rex64; call __tls_get_addr@PLT
  //	==> movl %fs:0,%eax; addq x@gottpoff(%rip),%rax
  //	leaq foo@tlsgd(%rip),%rdi;
  //	.word 0x66; rex64; call *__tls_get_addr@GOTPCREL(%rip)
  //	==> movl %fs:0,%eax; addq x@gottpoff(%rip),%rax

  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 12);
  tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		 (memcmp(view + 4, "\x66\x66\x48\xe8", 4) == 0
		  || memcmp(view + 4, "\x66\x48\xff", 3) == 0));

  if (size == 64)
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size,
		       -4);
      tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		     (memcmp(view - 4, "\x66\x48\x8d\x3d", 4) == 0));
      memcpy(view - 4, "\x64\x48\x8b\x04\x25\0\0\0\0\x48\x03\x05\0\0\0\0",
	     16);
    }
  else
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size,
		       -3);
      tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		     (memcmp(view - 3, "\x48\x8d\x3d", 3) == 0));
      memcpy(view - 3, "\x64\x8b\x04\x25\0\0\0\0\x48\x03\x05\0\0\0\0",
	     15);
    }

  const elfcpp::Elf_Xword addend = rela.get_r_addend();
  Relocate_functions<size, false>::pcrela32(view + 8, value, addend - 8,
					    address);

  // The next reloc should be a PLT32 reloc against __tls_get_addr.
  // We can skip it.
  this->skip_call_tls_get_addr_ = true;
}

// Do a relocation in which we convert a TLS General-Dynamic to a
// Local-Exec.

template<int size>
inline void
Target_x86_64<size>::Relocate::tls_gd_to_le(
    const Relocate_info<size, false>* relinfo,
    size_t relnum,
    Output_segment* tls_segment,
    const elfcpp::Rela<size, false>& rela,
    unsigned int,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    unsigned char* view,
    section_size_type view_size)
{
  // For SIZE == 64:
  //	.byte 0x66; leaq foo@tlsgd(%rip),%rdi;
  //	.word 0x6666; rex64; call __tls_get_addr@PLT
  //	==> movq %fs:0,%rax; leaq x@tpoff(%rax),%rax
  //	.byte 0x66; leaq foo@tlsgd(%rip),%rdi;
  //	.word 0x66; rex64; call *__tls_get_addr@GOTPCREL(%rip)
  //	==> movq %fs:0,%rax; leaq x@tpoff(%rax),%rax
  // For SIZE == 32:
  //	leaq foo@tlsgd(%rip),%rdi;
  //	.word 0x6666; rex64; call __tls_get_addr@PLT
  //	==> movl %fs:0,%eax; leaq x@tpoff(%rax),%rax
  //	leaq foo@tlsgd(%rip),%rdi;
  //	.word 0x66; rex64; call *__tls_get_addr@GOTPCREL(%rip)
  //	==> movl %fs:0,%eax; leaq x@tpoff(%rax),%rax

  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 12);
  tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		 (memcmp(view + 4, "\x66\x66\x48\xe8", 4) == 0
		  || memcmp(view + 4, "\x66\x48\xff", 3) == 0));

  if (size == 64)
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size,
		       -4);
      tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		     (memcmp(view - 4, "\x66\x48\x8d\x3d", 4) == 0));
      memcpy(view - 4, "\x64\x48\x8b\x04\x25\0\0\0\0\x48\x8d\x80\0\0\0\0",
	     16);
    }
  else
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size,
		       -3);
      tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		     (memcmp(view - 3, "\x48\x8d\x3d", 3) == 0));

      memcpy(view - 3, "\x64\x8b\x04\x25\0\0\0\0\x48\x8d\x80\0\0\0\0",
	     15);
    }

  value -= tls_segment->memsz();
  Relocate_functions<size, false>::rela32(view + 8, value, 0);

  // The next reloc should be a PLT32 reloc against __tls_get_addr.
  // We can skip it.
  this->skip_call_tls_get_addr_ = true;
}

// Do a TLSDESC-style General-Dynamic to Initial-Exec transition.

template<int size>
inline void
Target_x86_64<size>::Relocate::tls_desc_gd_to_ie(
    const Relocate_info<size, false>* relinfo,
    size_t relnum,
    const elfcpp::Rela<size, false>& rela,
    unsigned int r_type,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size)
{
  if (r_type == elfcpp::R_X86_64_GOTPC32_TLSDESC)
    {
      // LP64: leaq foo@tlsdesc(%rip), %rax
      //       ==> movq foo@gottpoff(%rip), %rax
      // X32:  rex leal foo@tlsdesc(%rip), %eax
      //       ==> rex movl foo@gottpoff(%rip), %eax
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, -3);
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 4);
      tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		     (((view[-3] & 0xfb) == 0x48
		       || (size == 32 && (view[-3] & 0xfb) == 0x40))
		      && view[-2] == 0x8d
		      && (view[-1] & 0xc7) == 0x05));
      view[-2] = 0x8b;
      const elfcpp::Elf_Xword addend = rela.get_r_addend();
      Relocate_functions<size, false>::pcrela32(view, value, addend, address);
    }
  else
    {
      // LP64: call *foo@tlscall(%rax)
      //       ==> xchg %ax, %ax
      // X32:  call *foo@tlscall(%eax)
      //       ==> nopl (%rax)
      gold_assert(r_type == elfcpp::R_X86_64_TLSDESC_CALL);
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 2);
      int prefix = 0;
      if (size == 32 && view[0] == 0x67)
	{
	  tls::check_range(relinfo, relnum, rela.get_r_offset(),
			   view_size, 3);
	  prefix = 1;
	}
      tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		     view[prefix] == 0xff && view[prefix + 1] == 0x10);
      if (prefix)
	{
	  view[0] = 0x0f;
	  view[1] = 0x1f;
	  view[2] = 0x00;
	}
      else
	{
	  view[0] = 0x66;
	  view[1] = 0x90;
	}
    }
}

// Do a TLSDESC-style General-Dynamic to Local-Exec transition.

template<int size>
inline void
Target_x86_64<size>::Relocate::tls_desc_gd_to_le(
    const Relocate_info<size, false>* relinfo,
    size_t relnum,
    Output_segment* tls_segment,
    const elfcpp::Rela<size, false>& rela,
    unsigned int r_type,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    unsigned char* view,
    section_size_type view_size)
{
  if (r_type == elfcpp::R_X86_64_GOTPC32_TLSDESC)
    {
      // LP64: leaq foo@tlsdesc(%rip), %rax
      //       ==> movq foo@tpoff, %rax
      // X32:  rex leal foo@tlsdesc(%rip), %eax
      //       ==> rex movl foo@tpoff, %eax
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, -3);
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 4);
      tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		     (((view[-3] & 0xfb) == 0x48
		       || (size == 32 && (view[-3] & 0xfb) == 0x40))
		      && view[-2] == 0x8d
		      && (view[-1] & 0xc7) == 0x05));
      view[-3] = (view[-3] & 0x48) | ((view[-3] >> 2) & 1);
      view[-2] = 0xc7;
      view[-1] = 0xc0 | ((view[-1] >> 3) & 7);
      value -= tls_segment->memsz();
      Relocate_functions<size, false>::rela32(view, value, 0);
    }
  else
    {
      // LP64: call *foo@tlscall(%rax)
      //       ==> xchg %ax, %ax
      // X32:  call *foo@tlscall(%eax)
      //       ==> nopl (%rax)
      gold_assert(r_type == elfcpp::R_X86_64_TLSDESC_CALL);
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 2);
      int prefix = 0;
      if (size == 32 && view[0] == 0x67)
	{
	  tls::check_range(relinfo, relnum, rela.get_r_offset(),
			   view_size, 3);
	  prefix = 1;
	}
      tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		     view[prefix] == 0xff && view[prefix + 1] == 0x10);
      if (prefix)
	{
	  view[0] = 0x0f;
	  view[1] = 0x1f;
	  view[2] = 0x00;
	}
      else
	{
	  view[0] = 0x66;
	  view[1] = 0x90;
	}
    }
}

template<int size>
inline void
Target_x86_64<size>::Relocate::tls_ld_to_le(
    const Relocate_info<size, false>* relinfo,
    size_t relnum,
    Output_segment*,
    const elfcpp::Rela<size, false>& rela,
    unsigned int,
    typename elfcpp::Elf_types<size>::Elf_Addr,
    unsigned char* view,
    section_size_type view_size)
{
  // leaq foo@tlsld(%rip),%rdi; call __tls_get_addr@plt;
  // For SIZE == 64:
  // ... leq foo@dtpoff(%rax),%reg
  // ==> .word 0x6666; .byte 0x66; movq %fs:0,%rax ... leaq x@tpoff(%rax),%rdx
  // For SIZE == 32:
  // ... leq foo@dtpoff(%rax),%reg
  // ==> nopl 0x0(%rax); movl %fs:0,%eax ... leaq x@tpoff(%rax),%rdx
  // leaq foo@tlsld(%rip),%rdi; call *__tls_get_addr@GOTPCREL(%rip)
  // For SIZE == 64:
  // ... leq foo@dtpoff(%rax),%reg
  // ==> .word 0x6666; .byte 0x6666; movq %fs:0,%rax ... leaq x@tpoff(%rax),%rdx
  // For SIZE == 32:
  // ... leq foo@dtpoff(%rax),%reg
  // ==> nopw 0x0(%rax); movl %fs:0,%eax ... leaq x@tpoff(%rax),%rdx

  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, -3);
  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 9);

  tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		 view[-3] == 0x48 && view[-2] == 0x8d && view[-1] == 0x3d);

  tls::check_tls(relinfo, relnum, rela.get_r_offset(),
		 view[4] == 0xe8 || view[4] == 0xff);

  if (view[4] == 0xe8)
    {
      if (size == 64)
	memcpy(view - 3, "\x66\x66\x66\x64\x48\x8b\x04\x25\0\0\0\0", 12);
      else
	memcpy(view - 3, "\x0f\x1f\x40\x00\x64\x8b\x04\x25\0\0\0\0", 12);
    }
  else
    {
      if (size == 64)
	memcpy(view - 3, "\x66\x66\x66\x66\x64\x48\x8b\x04\x25\0\0\0\0",
	       13);
      else
	memcpy(view - 3, "\x66\x0f\x1f\x40\x00\x64\x8b\x04\x25\0\0\0\0",
	       13);
    }

  // The next reloc should be a PLT32 reloc against __tls_get_addr.
  // We can skip it.
  this->skip_call_tls_get_addr_ = true;
}

// Do a relocation in which we convert a TLS Initial-Exec to a
// Local-Exec.

template<int size>
inline void
Target_x86_64<size>::Relocate::tls_ie_to_le(
    const Relocate_info<size, false>* relinfo,
    size_t relnum,
    Output_segment* tls_segment,
    const elfcpp::Rela<size, false>& rela,
    unsigned int,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    unsigned char* view,
    section_size_type view_size)
{
  // We need to examine the opcodes to figure out which instruction we
  // are looking at.

  // movq foo@gottpoff(%rip),%reg  ==>  movq $YY,%reg
  // addq foo@gottpoff(%rip),%reg  ==>  addq $YY,%reg

  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, -3);
  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 4);

  unsigned char op1 = view[-3];
  unsigned char op2 = view[-2];
  unsigned char op3 = view[-1];
  unsigned char reg = op3 >> 3;

  if (op2 == 0x8b)
    {
      // movq
      if (op1 == 0x4c)
	view[-3] = 0x49;
      else if (size == 32 && op1 == 0x44)
	view[-3] = 0x41;
      view[-2] = 0xc7;
      view[-1] = 0xc0 | reg;
    }
  else if (reg == 4)
    {
      // Special handling for %rsp.
      if (op1 == 0x4c)
	view[-3] = 0x49;
      else if (size == 32 && op1 == 0x44)
	view[-3] = 0x41;
      view[-2] = 0x81;
      view[-1] = 0xc0 | reg;
    }
  else
    {
      // addq
      if (op1 == 0x4c)
	view[-3] = 0x4d;
      else if (size == 32 && op1 == 0x44)
	view[-3] = 0x45;
      view[-2] = 0x8d;
      view[-1] = 0x80 | reg | (reg << 3);
    }

  if (tls_segment != NULL)
    value -= tls_segment->memsz();
  Relocate_functions<size, false>::rela32(view, value, 0);
}

// Relocate section data.

template<int size>
void
Target_x86_64<size>::relocate_section(
    const Relocate_info<size, false>* relinfo,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size,
    const Reloc_symbol_changes* reloc_symbol_changes)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, false>
      Classify_reloc;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::relocate_section<size, false, Target_x86_64<size>, Relocate,
			 gold::Default_comdat_behavior, Classify_reloc>(
    relinfo,
    this,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    view,
    address,
    view_size,
    reloc_symbol_changes);
}

// Apply an incremental relocation.  Incremental relocations always refer
// to global symbols.

template<int size>
void
Target_x86_64<size>::apply_relocation(
    const Relocate_info<size, false>* relinfo,
    typename elfcpp::Elf_types<size>::Elf_Addr r_offset,
    unsigned int r_type,
    typename elfcpp::Elf_types<size>::Elf_Swxword r_addend,
    const Symbol* gsym,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size)
{
  gold::apply_relocation<size, false, Target_x86_64<size>,
			 typename Target_x86_64<size>::Relocate>(
    relinfo,
    this,
    r_offset,
    r_type,
    r_addend,
    gsym,
    view,
    address,
    view_size);
}

// Scan the relocs during a relocatable link.

template<int size>
void
Target_x86_64<size>::scan_relocatable_relocs(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, false>* object,
    unsigned int data_shndx,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    size_t local_symbol_count,
    const unsigned char* plocal_symbols,
    Relocatable_relocs* rr)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, false>
      Classify_reloc;
  typedef gold::Default_scan_relocatable_relocs<Classify_reloc>
      Scan_relocatable_relocs;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::scan_relocatable_relocs<size, false, Scan_relocatable_relocs>(
    symtab,
    layout,
    object,
    data_shndx,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    local_symbol_count,
    plocal_symbols,
    rr);
}

// Scan the relocs for --emit-relocs.

template<int size>
void
Target_x86_64<size>::emit_relocs_scan(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, false>* object,
    unsigned int data_shndx,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    size_t local_symbol_count,
    const unsigned char* plocal_syms,
    Relocatable_relocs* rr)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, false>
      Classify_reloc;
  typedef gold::Default_emit_relocs_strategy<Classify_reloc>
      Emit_relocs_strategy;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::scan_relocatable_relocs<size, false, Emit_relocs_strategy>(
    symtab,
    layout,
    object,
    data_shndx,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    local_symbol_count,
    plocal_syms,
    rr);
}

// Relocate a section during a relocatable link.

template<int size>
void
Target_x86_64<size>::relocate_relocs(
    const Relocate_info<size, false>* relinfo,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    typename elfcpp::Elf_types<size>::Elf_Off offset_in_output_section,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr view_address,
    section_size_type view_size,
    unsigned char* reloc_view,
    section_size_type reloc_view_size)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, false>
      Classify_reloc;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::relocate_relocs<size, false, Classify_reloc>(
    relinfo,
    prelocs,
    reloc_count,
    output_section,
    offset_in_output_section,
    view,
    view_address,
    view_size,
    reloc_view,
    reloc_view_size);
}

// Return the value to use for a dynamic which requires special
// treatment.  This is how we support equality comparisons of function
// pointers across shared library boundaries, as described in the
// processor specific ABI supplement.

template<int size>
uint64_t
Target_x86_64<size>::do_dynsym_value(const Symbol* gsym) const
{
  gold_assert(gsym->is_from_dynobj() && gsym->has_plt_offset());
  return this->plt_address_for_global(gsym);
}

// Return a string used to fill a code section with nops to take up
// the specified length.

template<int size>
std::string
Target_x86_64<size>::do_code_fill(section_size_type length) const
{
  if (length >= 16)
    {
      // Build a jmpq instruction to skip over the bytes.
      unsigned char jmp[5];
      jmp[0] = 0xe9;
      elfcpp::Swap_unaligned<32, false>::writeval(jmp + 1, length - 5);
      return (std::string(reinterpret_cast<char*>(&jmp[0]), 5)
	      + std::string(length - 5, static_cast<char>(0x90)));
    }

  // Nop sequences of various lengths.
  const char nop1[1] = { '\x90' };                 // nop
  const char nop2[2] = { '\x66', '\x90' };         // xchg %ax %ax
  const char nop3[3] = { '\x0f', '\x1f', '\x00' }; // nop (%rax)
  const char nop4[4] = { '\x0f', '\x1f', '\x40',   // nop 0(%rax)
			 '\x00'};
  const char nop5[5] = { '\x0f', '\x1f', '\x44',   // nop 0(%rax,%rax,1)
			 '\x00', '\x00' };
  const char nop6[6] = { '\x66', '\x0f', '\x1f',   // nopw 0(%rax,%rax,1)
			 '\x44', '\x00', '\x00' };
  const char nop7[7] = { '\x0f', '\x1f', '\x80',   // nopl 0L(%rax)
			 '\x00', '\x00', '\x00',
			 '\x00' };
  const char nop8[8] = { '\x0f', '\x1f', '\x84',   // nopl 0L(%rax,%rax,1)
			 '\x00', '\x00', '\x00',
			 '\x00', '\x00' };
  const char nop9[9] = { '\x66', '\x0f', '\x1f',   // nopw 0L(%rax,%rax,1)
			 '\x84', '\x00', '\x00',
			 '\x00', '\x00', '\x00' };
  const char nop10[10] = { '\x66', '\x2e', '\x0f', // nopw %cs:0L(%rax,%rax,1)
			   '\x1f', '\x84', '\x00',
			   '\x00', '\x00', '\x00',
			   '\x00' };
  const char nop11[11] = { '\x66', '\x66', '\x2e', // data16
			   '\x0f', '\x1f', '\x84', // nopw %cs:0L(%rax,%rax,1)
			   '\x00', '\x00', '\x00',
			   '\x00', '\x00' };
  const char nop12[12] = { '\x66', '\x66', '\x66', // data16; data16
			   '\x2e', '\x0f', '\x1f', // nopw %cs:0L(%rax,%rax,1)
			   '\x84', '\x00', '\x00',
			   '\x00', '\x00', '\x00' };
  const char nop13[13] = { '\x66', '\x66', '\x66', // data16; data16; data16
			   '\x66', '\x2e', '\x0f', // nopw %cs:0L(%rax,%rax,1)
			   '\x1f', '\x84', '\x00',
			   '\x00', '\x00', '\x00',
			   '\x00' };
  const char nop14[14] = { '\x66', '\x66', '\x66', // data16; data16; data16
			   '\x66', '\x66', '\x2e', // data16
			   '\x0f', '\x1f', '\x84', // nopw %cs:0L(%rax,%rax,1)
			   '\x00', '\x00', '\x00',
			   '\x00', '\x00' };
  const char nop15[15] = { '\x66', '\x66', '\x66', // data16; data16; data16
			   '\x66', '\x66', '\x66', // data16; data16
			   '\x2e', '\x0f', '\x1f', // nopw %cs:0L(%rax,%rax,1)
			   '\x84', '\x00', '\x00',
			   '\x00', '\x00', '\x00' };

  const char* nops[16] = {
    NULL,
    nop1, nop2, nop3, nop4, nop5, nop6, nop7,
    nop8, nop9, nop10, nop11, nop12, nop13, nop14, nop15
  };

  return std::string(nops[length], length);
}

// Return the addend to use for a target specific relocation.  The
// only target specific relocation is R_X86_64_TLSDESC for a local
// symbol.  We want to set the addend is the offset of the local
// symbol in the TLS segment.

template<int size>
uint64_t
Target_x86_64<size>::do_reloc_addend(void* arg, unsigned int r_type,
				     uint64_t) const
{
  gold_assert(r_type == elfcpp::R_X86_64_TLSDESC);
  uintptr_t intarg = reinterpret_cast<uintptr_t>(arg);
  gold_assert(intarg < this->tlsdesc_reloc_info_.size());
  const Tlsdesc_info& ti(this->tlsdesc_reloc_info_[intarg]);
  const Symbol_value<size>* psymval = ti.object->local_symbol(ti.r_sym);
  gold_assert(psymval->is_tls_symbol());
  // The value of a TLS symbol is the offset in the TLS segment.
  return psymval->value(ti.object, 0);
}

// Return the value to use for the base of a DW_EH_PE_datarel offset
// in an FDE.  Solaris and SVR4 use DW_EH_PE_datarel because their
// assembler can not write out the difference between two labels in
// different sections, so instead of using a pc-relative value they
// use an offset from the GOT.

template<int size>
uint64_t
Target_x86_64<size>::do_ehframe_datarel_base() const
{
  gold_assert(this->global_offset_table_ != NULL);
  Symbol* sym = this->global_offset_table_;
  Sized_symbol<size>* ssym = static_cast<Sized_symbol<size>*>(sym);
  return ssym->value();
}

// FNOFFSET in section SHNDX in OBJECT is the start of a function
// compiled with -fsplit-stack.  The function calls non-split-stack
// code.  We have to change the function so that it always ensures
// that it has enough stack space to run some random function.

static const unsigned char cmp_insn_32[] = { 0x64, 0x3b, 0x24, 0x25 };
static const unsigned char lea_r10_insn_32[] = { 0x44, 0x8d, 0x94, 0x24 };
static const unsigned char lea_r11_insn_32[] = { 0x44, 0x8d, 0x9c, 0x24 };

static const unsigned char cmp_insn_64[] = { 0x64, 0x48, 0x3b, 0x24, 0x25 };
static const unsigned char lea_r10_insn_64[] = { 0x4c, 0x8d, 0x94, 0x24 };
static const unsigned char lea_r11_insn_64[] = { 0x4c, 0x8d, 0x9c, 0x24 };

template<int size>
void
Target_x86_64<size>::do_calls_non_split(Relobj* object, unsigned int shndx,
					section_offset_type fnoffset,
					section_size_type fnsize,
					const unsigned char*,
					size_t,
					unsigned char* view,
					section_size_type view_size,
					std::string* from,
					std::string* to) const
{
  const char* const cmp_insn = reinterpret_cast<const char*>
      (size == 32 ? cmp_insn_32 : cmp_insn_64);
  const char* const lea_r10_insn = reinterpret_cast<const char*>
      (size == 32 ? lea_r10_insn_32 : lea_r10_insn_64);
  const char* const lea_r11_insn = reinterpret_cast<const char*>
      (size == 32 ? lea_r11_insn_32 : lea_r11_insn_64);

  const size_t cmp_insn_len =
      (size == 32 ? sizeof(cmp_insn_32) : sizeof(cmp_insn_64));
  const size_t lea_r10_insn_len =
      (size == 32 ? sizeof(lea_r10_insn_32) : sizeof(lea_r10_insn_64));
  const size_t lea_r11_insn_len =
      (size == 32 ? sizeof(lea_r11_insn_32) : sizeof(lea_r11_insn_64));
  const size_t nop_len = (size == 32 ? 7 : 8);

  // The function starts with a comparison of the stack pointer and a
  // field in the TCB.  This is followed by a jump.

  // cmp %fs:NN,%rsp
  if (this->match_view(view, view_size, fnoffset, cmp_insn, cmp_insn_len)
      && fnsize > nop_len + 1)
    {
      // We will call __morestack if the carry flag is set after this
      // comparison.  We turn the comparison into an stc instruction
      // and some nops.
      view[fnoffset] = '\xf9';
      this->set_view_to_nop(view, view_size, fnoffset + 1, nop_len);
    }
  // lea NN(%rsp),%r10
  // lea NN(%rsp),%r11
  else if ((this->match_view(view, view_size, fnoffset,
			     lea_r10_insn, lea_r10_insn_len)
	    || this->match_view(view, view_size, fnoffset,
				lea_r11_insn, lea_r11_insn_len))
	   && fnsize > 8)
    {
      // This is loading an offset from the stack pointer for a
      // comparison.  The offset is negative, so we decrease the
      // offset by the amount of space we need for the stack.  This
      // means we will avoid calling __morestack if there happens to
      // be plenty of space on the stack already.
      unsigned char* pval = view + fnoffset + 4;
      uint32_t val = elfcpp::Swap_unaligned<32, false>::readval(pval);
      val -= parameters->options().split_stack_adjust_size();
      elfcpp::Swap_unaligned<32, false>::writeval(pval, val);
    }
  else
    {
      if (!object->has_no_split_stack())
	object->error(_("failed to match split-stack sequence at "
			"section %u offset %0zx"),
		      shndx, static_cast<size_t>(fnoffset));
      return;
    }

  // We have to change the function so that it calls
  // __morestack_non_split instead of __morestack.  The former will
  // allocate additional stack space.
  *from = "__morestack";
  *to = "__morestack_non_split";
}

// The selector for x86_64 object files.  Note this is never instantiated
// directly.  It's only used in Target_selector_x86_64_nacl, below.

template<int size>
class Target_selector_x86_64 : public Target_selector_freebsd
{
public:
  Target_selector_x86_64()
    : Target_selector_freebsd(elfcpp::EM_X86_64, size, false,
			      (size == 64
			       ? "elf64-x86-64" : "elf32-x86-64"),
			      (size == 64
			       ? "elf64-x86-64-freebsd"
			       : "elf32-x86-64-freebsd"),
			      (size == 64 ? "elf_x86_64" : "elf32_x86_64"))
  { }

  Target*
  do_instantiate_target()
  { return new Target_x86_64<size>(); }

};

// NaCl variant.  It uses different PLT contents.

template<int size>
class Output_data_plt_x86_64_nacl : public Output_data_plt_x86_64<size>
{
 public:
  Output_data_plt_x86_64_nacl(Layout* layout,
			      Output_data_got<64, false>* got,
			      Output_data_got_plt_x86_64* got_plt,
			      Output_data_space* got_irelative)
    : Output_data_plt_x86_64<size>(layout, plt_entry_size,
				   got, got_plt, got_irelative)
  { }

  Output_data_plt_x86_64_nacl(Layout* layout,
			      Output_data_got<64, false>* got,
			      Output_data_got_plt_x86_64* got_plt,
			      Output_data_space* got_irelative,
			      unsigned int plt_count)
    : Output_data_plt_x86_64<size>(layout, plt_entry_size,
				   got, got_plt, got_irelative,
				   plt_count)
  { }

 protected:
  virtual unsigned int
  do_get_plt_entry_size() const
  { return plt_entry_size; }

  virtual void
  do_add_eh_frame(Layout* layout)
  {
    layout->add_eh_frame_for_plt(this,
				 this->plt_eh_frame_cie,
				 this->plt_eh_frame_cie_size,
				 plt_eh_frame_fde,
				 plt_eh_frame_fde_size);
  }

  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  typename elfcpp::Elf_types<size>::Elf_Addr got_addr,
			  typename elfcpp::Elf_types<size>::Elf_Addr plt_addr);

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_index);

  virtual void
  do_fill_tlsdesc_entry(unsigned char* pov,
			typename elfcpp::Elf_types<size>::Elf_Addr got_address,
			typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
			typename elfcpp::Elf_types<size>::Elf_Addr got_base,
			unsigned int tlsdesc_got_offset,
			unsigned int plt_offset);

 private:
  // The size of an entry in the PLT.
  static const int plt_entry_size = 64;

  // The first entry in the PLT.
  static const unsigned char first_plt_entry[plt_entry_size];

  // Other entries in the PLT for an executable.
  static const unsigned char plt_entry[plt_entry_size];

  // The reserved TLSDESC entry in the PLT for an executable.
  static const unsigned char tlsdesc_plt_entry[plt_entry_size];

  // The .eh_frame unwind information for the PLT.
  static const int plt_eh_frame_fde_size = 32;
  static const unsigned char plt_eh_frame_fde[plt_eh_frame_fde_size];
};

template<int size>
class Target_x86_64_nacl : public Target_x86_64<size>
{
 public:
  Target_x86_64_nacl()
    : Target_x86_64<size>(&x86_64_nacl_info)
  { }

  virtual Output_data_plt_x86_64<size>*
  do_make_data_plt(Layout* layout,
		   Output_data_got<64, false>* got,
		   Output_data_got_plt_x86_64* got_plt,
		   Output_data_space* got_irelative)
  {
    return new Output_data_plt_x86_64_nacl<size>(layout, got, got_plt,
						 got_irelative);
  }

  virtual Output_data_plt_x86_64<size>*
  do_make_data_plt(Layout* layout,
		   Output_data_got<64, false>* got,
		   Output_data_got_plt_x86_64* got_plt,
		   Output_data_space* got_irelative,
		   unsigned int plt_count)
  {
    return new Output_data_plt_x86_64_nacl<size>(layout, got, got_plt,
						 got_irelative,
						 plt_count);
  }

  virtual std::string
  do_code_fill(section_size_type length) const;

 private:
  static const Target::Target_info x86_64_nacl_info;
};

template<>
const Target::Target_info Target_x86_64_nacl<64>::x86_64_nacl_info =
{
  64,			// size
  false,		// is_big_endian
  elfcpp::EM_X86_64,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/lib64/ld-nacl-x86-64.so.1", // dynamic_linker
  0x20000,		// default_text_segment_address
  0x10000,		// abi_pagesize (overridable by -z max-page-size)
  0x10000,		// common_pagesize (overridable by -z common-page-size)
  true,                 // isolate_execinstr
  0x10000000,           // rosegment_gap
  elfcpp::SHN_UNDEF,	// small_common_shndx
  elfcpp::SHN_X86_64_LCOMMON,	// large_common_shndx
  0,			// small_common_section_flags
  elfcpp::SHF_X86_64_LARGE,	// large_common_section_flags
  NULL,			// attributes_section
  NULL,			// attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_X86_64_UNWIND,	// unwind_section_type
};

template<>
const Target::Target_info Target_x86_64_nacl<32>::x86_64_nacl_info =
{
  32,			// size
  false,		// is_big_endian
  elfcpp::EM_X86_64,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/lib/ld-nacl-x86-64.so.1", // dynamic_linker
  0x20000,		// default_text_segment_address
  0x10000,		// abi_pagesize (overridable by -z max-page-size)
  0x10000,		// common_pagesize (overridable by -z common-page-size)
  true,                 // isolate_execinstr
  0x10000000,           // rosegment_gap
  elfcpp::SHN_UNDEF,	// small_common_shndx
  elfcpp::SHN_X86_64_LCOMMON,	// large_common_shndx
  0,			// small_common_section_flags
  elfcpp::SHF_X86_64_LARGE,	// large_common_section_flags
  NULL,			// attributes_section
  NULL,			// attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_X86_64_UNWIND,	// unwind_section_type
};

#define	NACLMASK	0xe0            // 32-byte alignment mask.

// The first entry in the PLT.

template<int size>
const unsigned char
Output_data_plt_x86_64_nacl<size>::first_plt_entry[plt_entry_size] =
{
  0xff, 0x35,                         // pushq contents of memory address
  0, 0, 0, 0,                         // replaced with address of .got + 8
  0x4c, 0x8b, 0x1d,                   // mov GOT+16(%rip), %r11
  0, 0, 0, 0,                         // replaced with address of .got + 16
  0x41, 0x83, 0xe3, NACLMASK,         // and $-32, %r11d
  0x4d, 0x01, 0xfb,                   // add %r15, %r11
  0x41, 0xff, 0xe3,                   // jmpq *%r11

  // 9-byte nop sequence to pad out to the next 32-byte boundary.
  0x66, 0x0f, 0x1f, 0x84, 0, 0, 0, 0, 0, // nopw 0x0(%rax,%rax,1)

  // 32 bytes of nop to pad out to the standard size
  0x66, 0x66, 0x66, 0x66, 0x66, 0x66,    // excess data32 prefixes
  0x2e, 0x0f, 0x1f, 0x84, 0, 0, 0, 0, 0, // nopw %cs:0x0(%rax,%rax,1)
  0x66, 0x66, 0x66, 0x66, 0x66, 0x66,    // excess data32 prefixes
  0x2e, 0x0f, 0x1f, 0x84, 0, 0, 0, 0, 0, // nopw %cs:0x0(%rax,%rax,1)
  0x66,                                  // excess data32 prefix
  0x90                                   // nop
};

template<int size>
void
Output_data_plt_x86_64_nacl<size>::do_fill_first_plt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address)
{
  memcpy(pov, first_plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2,
					      (got_address + 8
					       - (plt_address + 2 + 4)));
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 9,
					      (got_address + 16
					       - (plt_address + 9 + 4)));
}

// Subsequent entries in the PLT.

template<int size>
const unsigned char
Output_data_plt_x86_64_nacl<size>::plt_entry[plt_entry_size] =
{
  0x4c, 0x8b, 0x1d,              // mov name@GOTPCREL(%rip),%r11
  0, 0, 0, 0,                    // replaced with address of symbol in .got
  0x41, 0x83, 0xe3, NACLMASK,    // and $-32, %r11d
  0x4d, 0x01, 0xfb,              // add %r15, %r11
  0x41, 0xff, 0xe3,              // jmpq *%r11

  // 15-byte nop sequence to pad out to the next 32-byte boundary.
  0x66, 0x66, 0x66, 0x66, 0x66, 0x66,    // excess data32 prefixes
  0x2e, 0x0f, 0x1f, 0x84, 0, 0, 0, 0, 0, // nopw %cs:0x0(%rax,%rax,1)

  // Lazy GOT entries point here (32-byte aligned).
  0x68,                       // pushq immediate
  0, 0, 0, 0,                 // replaced with index into relocation table
  0xe9,                       // jmp relative
  0, 0, 0, 0,                 // replaced with offset to start of .plt0

  // 22 bytes of nop to pad out to the standard size.
  0x66, 0x66, 0x66, 0x66, 0x66, 0x66,    // excess data32 prefixes
  0x2e, 0x0f, 0x1f, 0x84, 0, 0, 0, 0, 0, // nopw %cs:0x0(%rax,%rax,1)
  0x0f, 0x1f, 0x80, 0, 0, 0, 0,          // nopl 0x0(%rax)
};

template<int size>
unsigned int
Output_data_plt_x86_64_nacl<size>::do_fill_plt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
    unsigned int got_offset,
    unsigned int plt_offset,
    unsigned int plt_index)
{
  memcpy(pov, plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 3,
					      (got_address + got_offset
					       - (plt_address + plt_offset
						  + 3 + 4)));

  elfcpp::Swap_unaligned<32, false>::writeval(pov + 33, plt_index);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 38,
					      - (plt_offset + 38 + 4));

  return 32;
}

// The reserved TLSDESC entry in the PLT.

template<int size>
const unsigned char
Output_data_plt_x86_64_nacl<size>::tlsdesc_plt_entry[plt_entry_size] =
{
  0xff, 0x35,			// pushq x(%rip)
  0, 0, 0, 0,	// replaced with address of linkmap GOT entry (at PLTGOT + 8)
  0x4c, 0x8b, 0x1d,		// mov y(%rip),%r11
  0, 0, 0, 0,	// replaced with offset of reserved TLSDESC_GOT entry
  0x41, 0x83, 0xe3, NACLMASK,	// and $-32, %r11d
  0x4d, 0x01, 0xfb,             // add %r15, %r11
  0x41, 0xff, 0xe3,             // jmpq *%r11

  // 41 bytes of nop to pad out to the standard size.
  0x66, 0x66, 0x66, 0x66, 0x66, 0x66,    // excess data32 prefixes
  0x2e, 0x0f, 0x1f, 0x84, 0, 0, 0, 0, 0, // nopw %cs:0x0(%rax,%rax,1)
  0x66, 0x66, 0x66, 0x66, 0x66, 0x66,    // excess data32 prefixes
  0x2e, 0x0f, 0x1f, 0x84, 0, 0, 0, 0, 0, // nopw %cs:0x0(%rax,%rax,1)
  0x66, 0x66,                            // excess data32 prefixes
  0x2e, 0x0f, 0x1f, 0x84, 0, 0, 0, 0, 0, // nopw %cs:0x0(%rax,%rax,1)
};

template<int size>
void
Output_data_plt_x86_64_nacl<size>::do_fill_tlsdesc_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
    typename elfcpp::Elf_types<size>::Elf_Addr got_base,
    unsigned int tlsdesc_got_offset,
    unsigned int plt_offset)
{
  memcpy(pov, tlsdesc_plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2,
					      (got_address + 8
					       - (plt_address + plt_offset
						  + 2 + 4)));
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 9,
					      (got_base
					       + tlsdesc_got_offset
					       - (plt_address + plt_offset
						  + 9 + 4)));
}

// The .eh_frame unwind information for the PLT.

template<int size>
const unsigned char
Output_data_plt_x86_64_nacl<size>::plt_eh_frame_fde[plt_eh_frame_fde_size] =
{
  0, 0, 0, 0,				// Replaced with offset to .plt.
  0, 0, 0, 0,				// Replaced with size of .plt.
  0,					// Augmentation size.
  elfcpp::DW_CFA_def_cfa_offset, 16,	// DW_CFA_def_cfa_offset: 16.
  elfcpp::DW_CFA_advance_loc + 6,	// Advance 6 to __PLT__ + 6.
  elfcpp::DW_CFA_def_cfa_offset, 24,	// DW_CFA_def_cfa_offset: 24.
  elfcpp::DW_CFA_advance_loc + 58,	// Advance 58 to __PLT__ + 64.
  elfcpp::DW_CFA_def_cfa_expression,	// DW_CFA_def_cfa_expression.
  13,					// Block length.
  elfcpp::DW_OP_breg7, 8,		// Push %rsp + 8.
  elfcpp::DW_OP_breg16, 0,		// Push %rip.
  elfcpp::DW_OP_const1u, 63,		// Push 0x3f.
  elfcpp::DW_OP_and,			// & (%rip & 0x3f).
  elfcpp::DW_OP_const1u, 37,            // Push 0x25.
  elfcpp::DW_OP_ge,			// >= ((%rip & 0x3f) >= 0x25)
  elfcpp::DW_OP_lit3,			// Push 3.
  elfcpp::DW_OP_shl,			// << (((%rip & 0x3f) >= 0x25) << 3)
  elfcpp::DW_OP_plus,			// + ((((%rip&0x3f)>=0x25)<<3)+%rsp+8
  elfcpp::DW_CFA_nop,			// Align to 32 bytes.
  elfcpp::DW_CFA_nop
};

// Return a string used to fill a code section with nops.
// For NaCl, long NOPs are only valid if they do not cross
// bundle alignment boundaries, so keep it simple with one-byte NOPs.
template<int size>
std::string
Target_x86_64_nacl<size>::do_code_fill(section_size_type length) const
{
  return std::string(length, static_cast<char>(0x90));
}

// The selector for x86_64-nacl object files.

template<int size>
class Target_selector_x86_64_nacl
  : public Target_selector_nacl<Target_selector_x86_64<size>,
				Target_x86_64_nacl<size> >
{
 public:
  Target_selector_x86_64_nacl()
    : Target_selector_nacl<Target_selector_x86_64<size>,
			   Target_x86_64_nacl<size> >("x86-64",
						      size == 64
						      ? "elf64-x86-64-nacl"
						      : "elf32-x86-64-nacl",
						      size == 64
						      ? "elf_x86_64_nacl"
						      : "elf32_x86_64_nacl")
  { }
};

Target_selector_x86_64_nacl<64> target_selector_x86_64;
Target_selector_x86_64_nacl<32> target_selector_x32;

} // End anonymous namespace.
