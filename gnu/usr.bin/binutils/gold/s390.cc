// s390.cc -- s390 target support for gold.

// Copyright (C) 2015-2023 Free Software Foundation, Inc.
// Written by Marcin Kościelnicki <koriakin@0x04.net>.

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
#include "s390.h"
#include "object.h"
#include "symtab.h"
#include "layout.h"
#include "output.h"
#include "copy-relocs.h"
#include "target.h"
#include "target-reloc.h"
#include "target-select.h"
#include "tls.h"
#include "gc.h"
#include "icf.h"

namespace
{

using namespace gold;

// A class to handle the .got.plt section.

template<int size>
class Output_data_got_plt_s390 : public Output_section_data_build
{
 public:
  Output_data_got_plt_s390(Layout* layout)
    : Output_section_data_build(size/8),
      layout_(layout)
  { }

  Output_data_got_plt_s390(Layout* layout, off_t data_size)
    : Output_section_data_build(data_size, size/8),
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

template<int size>
class Output_data_plt_s390 : public Output_section_data
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_RELA, true, size, true>
    Reloc_section;

  Output_data_plt_s390(Layout* layout,
                         Output_data_got<size, true>* got,
                         Output_data_got_plt_s390<size>* got_plt,
                         Output_data_space* got_irelative)
    : Output_section_data(4), layout_(layout),
      irelative_rel_(NULL), got_(got), got_plt_(got_plt),
      got_irelative_(got_irelative), count_(0),
      irelative_count_(0), free_list_()
  { this->init(layout); }

  Output_data_plt_s390(Layout* layout,
                         Output_data_got<size, true>* got,
                         Output_data_got_plt_s390<size>* got_plt,
                         Output_data_space* got_irelative,
                         unsigned int plt_count)
    : Output_section_data((plt_count + 1) * plt_entry_size,
                          4, false),
      layout_(layout), irelative_rel_(NULL), got_(got),
      got_plt_(got_plt), got_irelative_(got_irelative), count_(plt_count),
      irelative_count_(0), free_list_()
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
  add_local_ifunc_entry(Symbol_table*, Layout*,
    Sized_relobj_file<size, true>*, unsigned int);

  // Add the relocation for a PLT entry.
  void
  add_relocation(Symbol_table*, Layout*, Symbol*, unsigned int);

  // Return the .rela.plt section data.
  Reloc_section*
  rela_plt()
  { return this->rel_; }

  // Return where the IRELATIVE relocations should go in the PLT
  // relocations.
  Reloc_section*
  rela_irelative(Symbol_table*, Layout*);

  // Return whether we created a section for IRELATIVE relocations.
  bool
  has_irelative_section() const
  { return this->irelative_rel_ != NULL; }

  // Return the number of PLT entries.
  unsigned int
  entry_count() const
  { return this->count_ + this->irelative_count_; }

  // Return the offset of the first non-reserved PLT entry.
  unsigned int
  first_plt_entry_offset()
  { return plt_entry_size; }

  // Return the size of a PLT entry.
  unsigned int
  get_plt_entry_size() const
  { return plt_entry_size; }

  // Reserve a slot in the PLT for an existing symbol in an incremental update.
  void
  reserve_slot(unsigned int plt_index)
  {
    this->free_list_.remove((plt_index + 1) * plt_entry_size,
                            (plt_index + 2) * plt_entry_size);
  }

  // Return the PLT address to use for a global symbol.
  uint64_t
  address_for_global(const Symbol*);

  // Return the PLT address to use for a local symbol.
  uint64_t
  address_for_local(const Relobj*, unsigned int symndx);

  // Add .eh_frame information for the PLT.
  void
  add_eh_frame(Layout* layout)
  {
	  (void)layout;
    layout->add_eh_frame_for_plt(this,
				 plt_eh_frame_cie,
				 plt_eh_frame_cie_size,
				 plt_eh_frame_fde,
				 plt_eh_frame_fde_size);
  }

 protected:
  // Fill in the first PLT entry.
  void
  fill_first_plt_entry(unsigned char* pov,
		       typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		       typename elfcpp::Elf_types<size>::Elf_Addr plt_address);

  // Fill in a normal PLT entry.  Returns the offset into the entry that
  // should be the initial GOT slot value.
  unsigned int
  fill_plt_entry(unsigned char* pov,
		 typename elfcpp::Elf_types<size>::Elf_Addr got_address,
		 typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
		 unsigned int got_offset,
		 unsigned int plt_offset,
		 unsigned int plt_rel_offset);

  void
  do_adjust_output_section(Output_section* os);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** PLT")); }

 private:
  // Set the final size.
  void
  set_final_data_size();

  // Write out the PLT data.
  void
  do_write(Output_file*);

  // A pointer to the Layout class, so that we can find the .dynamic
  // section when we write out the GOT PLT section.
  Layout* layout_;
  // The reloc section.
  Reloc_section* rel_;
  // The IRELATIVE relocs, if necessary.  These must follow the
  // regular PLT relocations.
  Reloc_section* irelative_rel_;
  // The .got section.
  Output_data_got<size, true>* got_;
  // The .got.plt section.
  Output_data_got_plt_s390<size>* got_plt_;
  // The part of the .got.plt section used for IRELATIVE relocs.
  Output_data_space* got_irelative_;
  // The number of PLT entries.
  unsigned int count_;
  // Number of PLT entries with R_TILEGX_IRELATIVE relocs.  These
  // follow the regular PLT entries.
  unsigned int irelative_count_;
  // List of available regions within the section, for incremental
  // update links.
  Free_list free_list_;

  // The size of an entry in the PLT.
  static const int plt_entry_size = 0x20;
  // The first entry in the PLT.
  static const unsigned char first_plt_entry_32_abs[plt_entry_size];
  static const unsigned char first_plt_entry_32_pic[plt_entry_size];
  static const unsigned char first_plt_entry_64[plt_entry_size];
  // Other entries in the PLT for an executable.
  static const unsigned char plt_entry_32_abs[plt_entry_size];
  static const unsigned char plt_entry_32_pic12[plt_entry_size];
  static const unsigned char plt_entry_32_pic16[plt_entry_size];
  static const unsigned char plt_entry_32_pic[plt_entry_size];
  static const unsigned char plt_entry_64[plt_entry_size];

  // The .eh_frame unwind information for the PLT.
  static const int plt_eh_frame_cie_size = 12;
  static const unsigned char plt_eh_frame_cie[plt_eh_frame_cie_size];
  static const int plt_eh_frame_fde_size = 12;
  static const unsigned char plt_eh_frame_fde[plt_eh_frame_fde_size];
};


template<int size>
class Target_s390 : public Sized_target<size, true>
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_RELA, true, size, true> Reloc_section;

  Target_s390()
    : Sized_target<size, true>(&s390_info),
      got_(NULL), plt_(NULL), got_plt_(NULL), got_irelative_(NULL),
      global_offset_table_(NULL), rela_dyn_(NULL),
      rela_irelative_(NULL), copy_relocs_(elfcpp::R_390_COPY),
      got_mod_index_offset_(-1U), tls_base_symbol_defined_(false),
      layout_(NULL)
  { }

  // Scan the relocations to look for symbol adjustments.
  void
  gc_process_relocs(Symbol_table* symtab,
		    Layout* layout,
		    Sized_relobj_file<size, true>* object,
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
	      Sized_relobj_file<size, true>* object,
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
  relocate_section(const Relocate_info<size, true>*,
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
			  Sized_relobj_file<size, true>* object,
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
		   Sized_relobj_file<size, true>* object,
		   unsigned int data_shndx,
		   unsigned int sh_type,
		   const unsigned char* prelocs,
		   size_t reloc_count,
		   Output_section* output_section,
		   bool needs_special_offset_handling,
		   size_t local_symbol_count,
		   const unsigned char* plocal_syms,
		   Relocatable_relocs* rr);

  // Return a string used to fill a code section with nops.
  std::string
  do_code_fill(section_size_type length) const;

  // Emit relocations for a section.
  void
  relocate_relocs(
      const Relocate_info<size, true>*,
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

  // Return whether SYM is defined by the ABI.
  bool
  do_is_defined_by_abi(const Symbol* sym) const
  { return strcmp(sym->name(), "__tls_get_offset") == 0; }

  // Return the PLT address to use for a global symbol.
  uint64_t
  do_plt_address_for_global(const Symbol* gsym) const
  { return this->plt_section()->address_for_global(gsym); }

  uint64_t
  do_plt_address_for_local(const Relobj* relobj, unsigned int symndx) const
  { return this->plt_section()->address_for_local(relobj, symndx); }

  // Return the offset to use for the GOT_INDX'th got entry which is
  // for a local tls symbol specified by OBJECT, SYMNDX.
  int64_t
  do_tls_offset_for_local(const Relobj* object,
			  unsigned int symndx,
			  Output_data_got_base* got,
			  unsigned int got_indx,
			  uint64_t addend) const;

  // Return the offset to use for the GOT_INDX'th got entry which is
  // for global tls symbol GSYM.
  int64_t
  do_tls_offset_for_global(Symbol* gsym,
			   Output_data_got_base* got,
			   unsigned int got_indx,
			   uint64_t addend) const;

  // This function should be defined in targets that can use relocation
  // types to determine (implemented in local_reloc_may_be_function_pointer
  // and global_reloc_may_be_function_pointer)
  // if a function's pointer is taken.  ICF uses this in safe mode to only
  // fold those functions whose pointer is defintely not taken.
  bool
  do_can_check_for_function_pointers() const
  { return true; }

  // Return whether SYM is call to a non-split function.
  bool
  do_is_call_to_non_split(const Symbol* sym, const unsigned char* preloc,
			  const unsigned char* view,
			  section_size_type view_size) const;

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
    return this->got_size() / (size / 8);
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
			  Sized_relobj<size, true>* obj,
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
  apply_relocation(const Relocate_info<size, true>* relinfo,
		   typename elfcpp::Elf_types<size>::Elf_Addr r_offset,
		   unsigned int r_type,
		   typename elfcpp::Elf_types<size>::Elf_Swxword r_addend,
		   const Symbol* gsym,
		   unsigned char* view,
		   typename elfcpp::Elf_types<size>::Elf_Addr address,
		   section_size_type view_size);

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
    local(Symbol_table* symtab, Layout* layout, Target_s390* target,
	  Sized_relobj_file<size, true>* object,
	  unsigned int data_shndx,
	  Output_section* output_section,
	  const elfcpp::Rela<size, true>& reloc, unsigned int r_type,
	  const elfcpp::Sym<size, true>& lsym,
	  bool is_discarded);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_s390* target,
	   Sized_relobj_file<size, true>* object,
	   unsigned int data_shndx,
	   Output_section* output_section,
	   const elfcpp::Rela<size, true>& reloc, unsigned int r_type,
	   Symbol* gsym);

    inline bool
    local_reloc_may_be_function_pointer(Symbol_table* symtab, Layout* layout,
					Target_s390* target,
					Sized_relobj_file<size, true>* object,
					unsigned int data_shndx,
					Output_section* output_section,
					const elfcpp::Rela<size, true>& reloc,
					unsigned int r_type,
					const elfcpp::Sym<size, true>& lsym);

    inline bool
    global_reloc_may_be_function_pointer(Symbol_table* symtab, Layout* layout,
					 Target_s390* target,
					 Sized_relobj_file<size, true>* object,
					 unsigned int data_shndx,
					 Output_section* output_section,
					 const elfcpp::Rela<size, true>& reloc,
					 unsigned int r_type,
					 Symbol* gsym);

  private:
    static void
    unsupported_reloc_local(Sized_relobj_file<size, true>*,
			    unsigned int r_type);

    static void
    unsupported_reloc_global(Sized_relobj_file<size, true>*,
			     unsigned int r_type, Symbol*);

    void
    check_non_pic(Relobj*, unsigned int r_type);

    inline bool
    possible_function_pointer_reloc(unsigned int r_type);

    bool
    reloc_needs_plt_for_ifunc(Sized_relobj_file<size, true>*,
			      unsigned int r_type);

    // Whether we have issued an error about a non-PIC compilation.
    bool issued_non_pic_error_;
  };

  // The class which implements relocation.
  class Relocate
  {
   public:
    // Do a relocation.  Return false if the caller should not issue
    // any warnings about this relocation.
    inline bool
    relocate(const Relocate_info<size, true>*, unsigned int,
	     Target_s390*, Output_section*, size_t, const unsigned char*,
	     const Sized_symbol<size>*, const Symbol_value<size>*,
	     unsigned char*, typename elfcpp::Elf_types<size>::Elf_Addr,
	     section_size_type);

   private:
    // Do a TLS relocation.
    inline typename elfcpp::Elf_types<size>::Elf_Addr
    relocate_tls(const Relocate_info<size, true>*, Target_s390*,
		 size_t relnum, const elfcpp::Rela<size, true>&,
		 unsigned int r_type, const Sized_symbol<size>*,
		 const Symbol_value<size>*,
		 unsigned char*, section_size_type);

    // Do a TLS General-Dynamic to Initial-Exec transition.
    inline void
    tls_gd_to_ie(const Relocate_info<size, true>*, size_t relnum,
		 const elfcpp::Rela<size, true>&,
		 unsigned char* view,
		 section_size_type view_size);

    // Do a TLS General-Dynamic to Local-Exec transition.
    inline void
    tls_gd_to_le(const Relocate_info<size, true>*, size_t relnum,
		 const elfcpp::Rela<size, true>&,
		 unsigned char* view,
		 section_size_type view_size);

    // Do a TLS Local-Dynamic to Local-Exec transition.
    inline void
    tls_ld_to_le(const Relocate_info<size, true>*, size_t relnum,
		 const elfcpp::Rela<size, true>&,
		 unsigned char* view,
		 section_size_type view_size);

    // Do a TLS Initial-Exec to Local-Exec transition.
    static inline void
    tls_ie_to_le(const Relocate_info<size, true>*, size_t relnum,
		 const elfcpp::Rela<size, true>&,
		 unsigned char* view,
		 section_size_type view_size);
  };

  // Adjust TLS relocation type based on the options and whether this
  // is a local symbol.
  static tls::Tls_optimization
  optimize_tls_reloc(bool is_final, int r_type);

  // Get the GOT section.
  const Output_data_got<size, true>*
  got_section() const
  {
    gold_assert(this->got_ != NULL);
    return this->got_;
  }

  // Get the GOT section, creating it if necessary.
  Output_data_got<size, true>*
  got_section(Symbol_table*, Layout*);

  typename elfcpp::Elf_types<size>::Elf_Addr
  got_address() const
  {
    gold_assert(this->got_ != NULL);
    return this->got_plt_->address();
  }

  typename elfcpp::Elf_types<size>::Elf_Addr
  got_main_offset() const
  {
    gold_assert(this->got_ != NULL);
    return this->got_->address() - this->got_address();
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
			     Sized_relobj_file<size, true>* relobj,
			     unsigned int local_sym_index);

  // Create a GOT entry for the TLS module index.
  unsigned int
  got_mod_index_entry(Symbol_table* symtab, Layout* layout,
		      Sized_relobj_file<size, true>* object);

  // Get the PLT section.
  Output_data_plt_s390<size>*
  plt_section() const
  {
    gold_assert(this->plt_ != NULL);
    return this->plt_;
  }

  // Get the dynamic reloc section, creating it if necessary.
  Reloc_section*
  rela_dyn_section(Layout*);

  // Get the section to use for IRELATIVE relocations.
  Reloc_section*
  rela_irelative_section(Layout*);

  // Add a potential copy relocation.
  void
  copy_reloc(Symbol_table* symtab, Layout* layout,
	     Sized_relobj_file<size, true>* object,
	     unsigned int shndx, Output_section* output_section,
	     Symbol* sym, const elfcpp::Rela<size, true>& reloc)
  {
    unsigned int r_type = elfcpp::elf_r_type<size>(reloc.get_r_info());
    this->copy_relocs_.copy_reloc(symtab, layout,
				  symtab->get_sized_symbol<size>(sym),
				  object, shndx, output_section,
				  r_type, reloc.get_r_offset(),
				  reloc.get_r_addend(),
				  this->rela_dyn_section(layout));
  }

  // A function for targets to call.  Return whether BYTES/LEN matches
  // VIEW/VIEW_SIZE at OFFSET.  Like the one in Target, but takes
  // an unsigned char * parameter.
  bool
  match_view_u(const unsigned char* view, section_size_type view_size,
     section_offset_type offset, const unsigned char* bytes, size_t len) const
    {
      return this->match_view(view, view_size, offset,
			      reinterpret_cast<const char*>(bytes), len);
    }

  // Information about this specific target which we pass to the
  // general Target structure.
  static Target::Target_info s390_info;

  // The types of GOT entries needed for this platform.
  // These values are exposed to the ABI in an incremental link.
  // Do not renumber existing values without changing the version
  // number of the .gnu_incremental_inputs section.
  enum Got_type
  {
    GOT_TYPE_STANDARD = 0,      // GOT entry for a regular symbol
    GOT_TYPE_TLS_OFFSET = 1,    // GOT entry for TLS offset
    GOT_TYPE_TLS_PAIR = 2,      // GOT entry for TLS module/offset pair
  };

  // The GOT section.
  Output_data_got<size, true>* got_;
  // The PLT section.
  Output_data_plt_s390<size>* plt_;
  // The GOT PLT section.
  Output_data_got_plt_s390<size>* got_plt_;
  // The GOT section for IRELATIVE relocations.
  Output_data_space* got_irelative_;
  // The _GLOBAL_OFFSET_TABLE_ symbol.
  Symbol* global_offset_table_;
  // The dynamic reloc section.
  Reloc_section* rela_dyn_;
  // The section to use for IRELATIVE relocs.
  Reloc_section* rela_irelative_;
  // Relocs saved to avoid a COPY reloc.
  Copy_relocs<elfcpp::SHT_RELA, size, true> copy_relocs_;
  // Offset of the GOT entry for the TLS module index.
  unsigned int got_mod_index_offset_;
  // True if the _TLS_MODULE_BASE_ symbol has been defined.
  bool tls_base_symbol_defined_;
  // For use in do_tls_offset_for_*
  Layout *layout_;

  // Code sequences for -fsplit-stack matching.
  static const unsigned char ss_code_bras_8[];
  static const unsigned char ss_code_l_basr[];
  static const unsigned char ss_code_a_basr[];
  static const unsigned char ss_code_larl[];
  static const unsigned char ss_code_brasl[];
  static const unsigned char ss_code_jg[];
  static const unsigned char ss_code_jgl[];

  // Variable code sequence matchers for -fsplit-stack.
  bool ss_match_st_r14(unsigned char* view,
		       section_size_type view_size,
		       section_offset_type *offset) const;
  bool ss_match_l_r14(unsigned char* view,
		      section_size_type view_size,
		      section_offset_type *offset) const;
  bool ss_match_mcount(unsigned char* view,
		       section_size_type view_size,
		       section_offset_type *offset) const;
  bool ss_match_ear(unsigned char* view,
		    section_size_type view_size,
		    section_offset_type *offset) const;
  bool ss_match_c(unsigned char* view,
		  section_size_type view_size,
		  section_offset_type *offset) const;
  bool ss_match_l(unsigned char* view,
		  section_size_type view_size,
		  section_offset_type *offset,
		  int *guard_reg) const;
  bool ss_match_ahi(unsigned char* view,
		    section_size_type view_size,
		    section_offset_type *offset,
		    int guard_reg,
		    uint32_t *arg) const;
  bool ss_match_alfi(unsigned char* view,
		     section_size_type view_size,
		     section_offset_type *offset,
		     int guard_reg,
		     uint32_t *arg) const;
  bool ss_match_cr(unsigned char* view,
		   section_size_type view_size,
		   section_offset_type *offset,
		   int guard_reg) const;
};

template<>
Target::Target_info Target_s390<32>::s390_info =
{
  32,			// size
  true,			// is_big_endian
  elfcpp::EM_S390,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/lib/ld.so.1",	// dynamic_linker
  0x00400000,		// default_text_segment_address
  4 * 1024,		// abi_pagesize (overridable by -z max-page-size)
  4 * 1024,		// common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,	// small_common_shndx
  elfcpp::SHN_UNDEF,	// large_common_shndx
  0,			// small_common_section_flags
  0,			// large_common_section_flags
  NULL,			// attributes_section
  NULL,			// attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

template<>
Target::Target_info Target_s390<64>::s390_info =
{
  64,			// size
  true,			// is_big_endian
  elfcpp::EM_S390,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/lib/ld64.so.1",	// dynamic_linker
  0x80000000ll,		// default_text_segment_address
  4 * 1024,		// abi_pagesize (overridable by -z max-page-size)
  4 * 1024,		// common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,	// small_common_shndx
  elfcpp::SHN_UNDEF,	// large_common_shndx
  0,			// small_common_section_flags
  0,			// large_common_section_flags
  NULL,			// attributes_section
  NULL,			// attributes_vendor
  "_start",		// entry_symbol_name
  64,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

template<int size>
class S390_relocate_functions
{
public:
  enum Overflow_check
  {
    CHECK_NONE,
    CHECK_SIGNED,
    CHECK_UNSIGNED,
    CHECK_BITFIELD,
    CHECK_LOW_INSN,
    CHECK_HIGH_INSN
  };

  enum Status
  {
    STATUS_OK,
    STATUS_OVERFLOW
  };

private:
  typedef S390_relocate_functions<size> This;
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;

  template<int valsize>
  static inline bool
  has_overflow_signed(Address value)
  {
    // limit = 1 << (valsize - 1) without shift count exceeding size of type
    Address limit = static_cast<Address>(1) << ((valsize - 1) >> 1);
    limit <<= ((valsize - 1) >> 1);
    limit <<= ((valsize - 1) - 2 * ((valsize - 1) >> 1));
    return value + limit > (limit << 1) - 1;
  }

  template<int valsize>
  static inline bool
  has_overflow_unsigned(Address value)
  {
    Address limit = static_cast<Address>(1) << ((valsize - 1) >> 1);
    limit <<= ((valsize - 1) >> 1);
    limit <<= ((valsize - 1) - 2 * ((valsize - 1) >> 1));
    return value > (limit << 1) - 1;
  }

  template<int fieldsize>
  static inline void
  rela(unsigned char* view, Address mask, Address value)
  {
    typedef typename elfcpp::Swap<fieldsize, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<fieldsize, true>::readval(view);
    val &= ~mask;
    value &= mask;
    elfcpp::Swap<fieldsize, true>::writeval(wv, val | value);
  }

public:
  // R_390_12, R_390_GOT12, R_390_GOTPLT12, R_390_GOTIE12
  static inline Status
  rela12(unsigned char* view, Address value)
  {
    if (This::template has_overflow_unsigned<12>(value))
      return STATUS_OVERFLOW;
    This::template rela<16>(view, 0x0fff, value);
    return STATUS_OK;
  }

  // R_390_16, R_390_GOT16, R_390_GOTPLT16, R_390_GOTOFF16, R_390_PLTOFF16
  static inline Status
  rela16(unsigned char* view, Address value)
  {
    if (This::template has_overflow_signed<16>(value))
      return STATUS_OVERFLOW;
    This::template rela<16>(view, 0xffff, value);
    return STATUS_OK;
  }

  // R_390_20, R_390_GOT20, R_390_GOTPLT20, R_390_GOTIE20
  static inline Status
  rela20(unsigned char* view, Address value)
  {
    if (This::template has_overflow_signed<20>(value))
      return STATUS_OVERFLOW;
    This::template rela<16>(view, 0x0fff, value);
    This::template rela<16>(view + 2, 0xff00, value >> (12 - 8));
    return STATUS_OK;
  }

  // R_390_PC12DBL, R_390_PLT12DBL
  static inline Status
  pcrela12dbl(unsigned char* view, Address value, Address address)
  {
    value -= address;
    if ((value & 1) != 0)
      return STATUS_OVERFLOW;
    if (This::template has_overflow_signed<13>(value))
      return STATUS_OVERFLOW;
    value >>= 1;
    This::template rela<16>(view, 0x0fff, value);
    return STATUS_OK;
  }

  // R_390_PC16DBL, R_390_PLT16DBL
  static inline Status
  pcrela16dbl(unsigned char* view, Address value, Address address)
  {
    value -= address;
    if ((value & 1) != 0)
      return STATUS_OVERFLOW;
    if (This::template has_overflow_signed<17>(value))
      return STATUS_OVERFLOW;
    value >>= 1;
    This::template rela<16>(view, 0xffff, value);
    return STATUS_OK;
  }

  // R_390_PC24DBL, R_390_PLT24DBL
  static inline Status
  pcrela24dbl(unsigned char* view, Address value, Address address)
  {
    value -= address;
    if ((value & 1) != 0)
      return STATUS_OVERFLOW;
    if (This::template has_overflow_signed<25>(value))
      return STATUS_OVERFLOW;
    value >>= 1;
    // Swap doesn't take 24-bit fields well...
    This::template rela<8>(view, 0xff, value >> 16);
    This::template rela<16>(view + 1, 0xffff, value);
    return STATUS_OK;
  }

  // R_390_PC32DBL, R_390_PLT32DBL, R_390_GOTPCDBL, R_390_GOTENT, R_390_GOTPLTENT
  static inline Status
  pcrela32dbl(unsigned char* view, Address value, Address address)
  {
    Address reloc = value - address;
    if ((reloc & 1) != 0)
      {
	gold_warning(_("R_390_PC32DBL target misaligned at %llx"), (long long)address);
	// Wait for a fix for https://sourceware.org/bugzilla/show_bug.cgi?id=18960
	// return STATUS_OVERFLOW;
      }
    if (This::template has_overflow_signed<33>(reloc))
      return STATUS_OVERFLOW;
    reloc >>= 1;
    if (value < address && size == 32)
      reloc |= 0x80000000;
    This::template rela<32>(view, 0xffffffff, reloc);
    return STATUS_OK;
  }

};

// Initialize the PLT section.

template<int size>
void
Output_data_plt_s390<size>::init(Layout* layout)
{
  this->rel_ = new Reloc_section(false);
  layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
				  elfcpp::SHF_ALLOC, this->rel_,
				  ORDER_DYNAMIC_PLT_RELOCS, false);
}

template<int size>
void
Output_data_plt_s390<size>::do_adjust_output_section(Output_section* os)
{
  os->set_entsize(plt_entry_size);
}

// Add an entry to the PLT.

template<int size>
void
Output_data_plt_s390<size>::add_entry(Symbol_table* symtab, Layout* layout,
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
      plt_offset = plt_index * plt_entry_size;

      ++*pcount;

      got_offset = (plt_index - offset + reserved) * size / 8;
      gold_assert(got_offset == got->current_data_size());

      // Every PLT entry needs a GOT entry which points back to the PLT
      // entry (this will be changed by the dynamic linker, normally
      // lazily when the function is called).
      got->set_current_data_size(got_offset + size / 8);
    }
  else
    {
      // FIXME: This is probably not correct for IRELATIVE relocs.

      // For incremental updates, find an available slot.
      plt_offset = this->free_list_.allocate(plt_entry_size,
					     plt_entry_size, 0);
      if (plt_offset == -1)
	gold_fallback(_("out of patch space (PLT);"
			" relink with --incremental-full"));

      // The GOT and PLT entries have a 1-1 correspondance, so the GOT offset
      // can be calculated from the PLT index, adjusting for the three
      // reserved entries at the beginning of the GOT.
      plt_index = plt_offset / plt_entry_size - 1;
      got_offset = (plt_index - offset + reserved) * size / 8;
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
Output_data_plt_s390<size>::add_local_ifunc_entry(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, true>* relobj,
    unsigned int local_sym_index)
{
  unsigned int plt_offset = this->irelative_count_ * plt_entry_size;
  ++this->irelative_count_;

  section_offset_type got_offset = this->got_irelative_->current_data_size();

  // Every PLT entry needs a GOT entry which points back to the PLT
  // entry.
  this->got_irelative_->set_current_data_size(got_offset + size / 8);

  // Every PLT entry needs a reloc.
  Reloc_section* rela = this->rela_irelative(symtab, layout);
  rela->add_symbolless_local_addend(relobj, local_sym_index,
				    elfcpp::R_390_IRELATIVE,
				    this->got_irelative_, got_offset, 0);

  return plt_offset;
}

// Add the relocation for a PLT entry.

template<int size>
void
Output_data_plt_s390<size>::add_relocation(Symbol_table* symtab,
					     Layout* layout,
					     Symbol* gsym,
					     unsigned int got_offset)
{
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      Reloc_section* rela = this->rela_irelative(symtab, layout);
      rela->add_symbolless_global_addend(gsym, elfcpp::R_390_IRELATIVE,
					 this->got_irelative_, got_offset, 0);
    }
  else
    {
      gsym->set_needs_dynsym_entry();
      this->rel_->add_global(gsym, elfcpp::R_390_JMP_SLOT, this->got_plt_,
			     got_offset, 0);
    }
}

// Return where the IRELATIVE relocations should go in the PLT.  These
// follow the JUMP_SLOT and the TLSDESC relocations.

template<int size>
typename Output_data_plt_s390<size>::Reloc_section*
Output_data_plt_s390<size>::rela_irelative(Symbol_table* symtab,
					     Layout* layout)
{
  if (this->irelative_rel_ == NULL)
    {
      this->irelative_rel_ = new Reloc_section(false);
      layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
				      elfcpp::SHF_ALLOC, this->irelative_rel_,
				      ORDER_DYNAMIC_PLT_RELOCS, false);
      gold_assert(this->irelative_rel_->output_section()
		  == this->rel_->output_section());

      if (parameters->doing_static_link())
	{
	  // A statically linked executable will only have a .rela.plt
	  // section to hold R_390_IRELATIVE relocs for
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
Output_data_plt_s390<size>::address_for_global(const Symbol* gsym)
{
  uint64_t offset = 0;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    offset = (this->count_ + 1) * plt_entry_size;
  return this->address() + offset + gsym->plt_offset();
}

// Return the PLT address to use for a local symbol.  These are always
// IRELATIVE relocs.

template<int size>
uint64_t
Output_data_plt_s390<size>::address_for_local(const Relobj* object,
						unsigned int r_sym)
{
  return (this->address()
	  + (this->count_ + 1) * plt_entry_size
	  + object->local_plt_offset(r_sym));
}

// Set the final size.
template<int size>
void
Output_data_plt_s390<size>::set_final_data_size()
{
  unsigned int count = this->count_ + this->irelative_count_;
  this->set_data_size((count + 1) * plt_entry_size);
}

template<int size>
const unsigned char
Output_data_plt_s390<size>::first_plt_entry_32_abs[plt_entry_size] =
{
  0x50, 0x10, 0xf0, 0x1c, // st %r1, 28(%r15)
  0x0d, 0x10, // basr %r1, %r0
  0x58, 0x10, 0x10, 0x12, // l %r1, 18(%r1)
  0xd2, 0x03, 0xf0, 0x18, 0x10, 0x04, // mvc 24(4,%r15), 4(%r1)
  0x58, 0x10, 0x10, 0x08, // l %r1, 8(%r1)
  0x07, 0xf1, // br %r1
  0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // _GLOBAL_OFFSET_TABLE_ (to fill)
  0x00, 0x00, 0x00, 0x00, // padding
};

template<int size>
const unsigned char
Output_data_plt_s390<size>::first_plt_entry_32_pic[plt_entry_size] =
{
  0x50, 0x10, 0xf0, 0x1c, // st %r1, 28(%r15)
  0x58, 0x10, 0xc0, 0x04, // l %r1, 4(%r12)
  0x50, 0x10, 0xf0, 0x18, // st %r1, 24(%r15)
  0x58, 0x10, 0xc0, 0x08, // l %r1, 8(%r12)
  0x07, 0xf1, // br %r1
  0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // padding
};

template<int size>
const unsigned char
Output_data_plt_s390<size>::first_plt_entry_64[plt_entry_size] =
{
  0xe3, 0x10, 0xf0, 0x38, 0x00, 0x24, // stg %r1, 56(%r15)
  0xc0, 0x10, 0x00, 0x00, 0x00, 0x00, // larl %r1, _GLOBAL_OFFSET_TABLE_ (to fill)
  0xd2, 0x07, 0xf0, 0x30, 0x10, 0x08, // mvc 48(8,%r15), 8(%r1)
  0xe3, 0x10, 0x10, 0x10, 0x00, 0x04, // lg %r1, 16(%r1)
  0x07, 0xf1, // br %r1
  0x07, 0x00, // nopr
  0x07, 0x00, // nopr
  0x07, 0x00, // nopr
};

template<int size>
void
Output_data_plt_s390<size>::fill_first_plt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address)
{
  if (size == 64)
    {
      memcpy(pov, first_plt_entry_64, plt_entry_size);
      S390_relocate_functions<size>::pcrela32dbl(pov + 8, got_address, (plt_address + 6));
    }
  else if (!parameters->options().output_is_position_independent())
    {
      memcpy(pov, first_plt_entry_32_abs, plt_entry_size);
      elfcpp::Swap<32, true>::writeval(pov + 24, got_address);
    }
  else
    {
      memcpy(pov, first_plt_entry_32_pic, plt_entry_size);
    }
}

template<int size>
const unsigned char
Output_data_plt_s390<size>::plt_entry_32_abs[plt_entry_size] =
{
  // first part
  0x0d, 0x10, // basr %r1, %r0
  0x58, 0x10, 0x10, 0x16, // l %r1, 22(%r1)
  0x58, 0x10, 0x10, 0x00, // l %r1, 0(%r1)
  0x07, 0xf1, // br %r1
  // second part
  0x0d, 0x10, // basr %r1, %r0
  0x58, 0x10, 0x10, 0x0e, // l %r1, 14(%r1)
  0xa7, 0xf4, 0x00, 0x00, // j first_plt_entry (to fill)
  0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // _GLOBAL_OFFSET_TABLE_+sym@gotplt (to fill)
  0x00, 0x00, 0x00, 0x00, // offset of relocation in .rela.plt (to fill)
};

template<int size>
const unsigned char
Output_data_plt_s390<size>::plt_entry_32_pic12[plt_entry_size] =
{
  // first part
  0x58, 0x10, 0xc0, 0x00, // l %r1, sym@gotplt(%r12) (to fill)
  0x07, 0xf1, // br %r1
  0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // padding
  // second part
  0x0d, 0x10, // basr %r1, %r0
  0x58, 0x10, 0x10, 0x0e, // l %r1, 14(%r1)
  0xa7, 0xf4, 0x00, 0x00, // j first_plt_entry (to fill)
  0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // offset of relocation in .rela.plt (to fill)
};

template<int size>
const unsigned char
Output_data_plt_s390<size>::plt_entry_32_pic16[plt_entry_size] =
{
  // first part
  0xa7, 0x18, 0x00, 0x00, // lhi %r1, sym@gotplt (to fill)
  0x58, 0x11, 0xc0, 0x00, // l %r1, 0(%r1, %r12)
  0x07, 0xf1, // br %r1
  0x00, 0x00, // padding
  // second part
  0x0d, 0x10, // basr %r1, %r0
  0x58, 0x10, 0x10, 0x0e, // l %r1, 14(%r1)
  0xa7, 0xf4, 0x00, 0x00, // j first_plt_entry (to fill)
  0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // offset of relocation in .rela.plt (to fill)
};

template<int size>
const unsigned char
Output_data_plt_s390<size>::plt_entry_32_pic[plt_entry_size] =
{
  // first part
  0x0d, 0x10, // basr %r1, %r0
  0x58, 0x10, 0x10, 0x16, // l %r1, 22(%r1)
  0x58, 0x11, 0xc0, 0x00, // l %r1, 0(%r1, %r12)
  0x07, 0xf1, // br %r1
  // second part
  0x0d, 0x10, // basr %r1, %r0
  0x58, 0x10, 0x10, 0x0e, // l %r1, 14(%r1)
  0xa7, 0xf4, 0x00, 0x00, // j first_plt_entry (to fill)
  0x00, 0x00, // padding
  0x00, 0x00, 0x00, 0x00, // sym@gotplt (to fill)
  0x00, 0x00, 0x00, 0x00, // offset of relocation in .rela.plt (to fill)
};

template<int size>
const unsigned char
Output_data_plt_s390<size>::plt_entry_64[plt_entry_size] =
{
  // first part
  0xc0, 0x10, 0x00, 0x00, 0x00, 0x00, // larl %r1, _GLOBAL_OFFSET_TABLE_+off (to fill)
  0xe3, 0x10, 0x10, 0x00, 0x00, 0x04, // lg %r1, 0(%r1)
  0x07, 0xf1, // br %r1
  // second part
  0x0d, 0x10, // basr %r1, %r0
  0xe3, 0x10, 0x10, 0x0c, 0x00, 0x14, // lgf %r1, 12(%r1)
  0xc0, 0xf4, 0x00, 0x00, 0x00, 0x00, // jg first_plt_entry (to fill)
  0x00, 0x00, 0x00, 0x00, // offset of relocation in .rela.plt (to fill)
};

template<int size>
unsigned int
Output_data_plt_s390<size>::fill_plt_entry(
    unsigned char* pov,
    typename elfcpp::Elf_types<size>::Elf_Addr got_address,
    typename elfcpp::Elf_types<size>::Elf_Addr plt_address,
    unsigned int got_offset,
    unsigned int plt_offset,
    unsigned int plt_rel_offset)
{
  if (size == 64)
  {
    memcpy(pov, plt_entry_64, plt_entry_size);
    S390_relocate_functions<size>::pcrela32dbl(pov + 2, got_address + got_offset, plt_address + plt_offset);
    S390_relocate_functions<size>::pcrela32dbl(pov + 24, plt_address, plt_address + plt_offset + 22);
  }
  else
  {
    if (!parameters->options().output_is_position_independent())
      {
	memcpy(pov, plt_entry_32_abs, plt_entry_size);
	elfcpp::Swap<32, true>::writeval(pov + 24, got_address + got_offset);
      }
    else
      {
	if (got_offset < 0x1000)
	  {
	    memcpy(pov, plt_entry_32_pic12, plt_entry_size);
	    S390_relocate_functions<size>::rela12(pov + 2, got_offset);
	  }
	else if (got_offset < 0x8000)
	  {
	    memcpy(pov, plt_entry_32_pic16, plt_entry_size);
	    S390_relocate_functions<size>::rela16(pov + 2, got_offset);
	  }
	else
	  {
	    memcpy(pov, plt_entry_32_pic, plt_entry_size);
	    elfcpp::Swap<32, true>::writeval(pov + 24, got_offset);
	  }
      }
    typename elfcpp::Elf_types<size>::Elf_Addr target = plt_address;
    if (plt_offset >= 0x10000)
      {
	// Would overflow pcrela16dbl - aim at the farthest previous jump
	// we can reach.
	if (plt_offset > 0x10000)
	  {
	    // Use the full range of pcrel16dbl.
	    target = plt_address + plt_offset - 0x10000 + 18;
	  }
	else
	  {
	    // if plt_offset is exactly 0x10000, the above would aim at 18th byte
	    // of first_plt_entry, which doesn't have the jump back like the others.
	    // Aim at the next entry instead.
	    target = plt_address + plt_offset - 0xffe0 + 18;
	  }
      }
    S390_relocate_functions<size>::pcrela16dbl(pov + 20, target, plt_address + plt_offset + 18);
  }
  elfcpp::Swap<32, true>::writeval(pov + 28, plt_rel_offset);
  if (size == 64)
    return 14;
  else
    return 12;
}

// The .eh_frame unwind information for the PLT.

template<>
const unsigned char
Output_data_plt_s390<32>::plt_eh_frame_cie[plt_eh_frame_cie_size] =
{
  1,				// CIE version.
  'z',				// Augmentation: augmentation size included.
  'R',				// Augmentation: FDE encoding included.
  '\0',				// End of augmentation string.
  1,				// Code alignment factor.
  0x7c,				// Data alignment factor.
  14,				// Return address column.
  1,				// Augmentation size.
  (elfcpp::DW_EH_PE_pcrel	// FDE encoding.
   | elfcpp::DW_EH_PE_sdata4),
  elfcpp::DW_CFA_def_cfa, 15, 0x60,	// DW_CFA_def_cfa: r15 ofs 0x60.
};

template<>
const unsigned char
Output_data_plt_s390<64>::plt_eh_frame_cie[plt_eh_frame_cie_size] =
{
  1,				// CIE version.
  'z',				// Augmentation: augmentation size included.
  'R',				// Augmentation: FDE encoding included.
  '\0',				// End of augmentation string.
  1,				// Code alignment factor.
  0x78,				// Data alignment factor.
  14,				// Return address column.
  1,				// Augmentation size.
  (elfcpp::DW_EH_PE_pcrel	// FDE encoding.
   | elfcpp::DW_EH_PE_sdata4),
  elfcpp::DW_CFA_def_cfa, 15, 0xa0,	// DW_CFA_def_cfa: r15 ofs 0xa0.
};

template<int size>
const unsigned char
Output_data_plt_s390<size>::plt_eh_frame_fde[plt_eh_frame_fde_size] =
{
  0, 0, 0, 0,				// Replaced with offset to .plt.
  0, 0, 0, 0,				// Replaced with size of .plt.
  0,					// Augmentation size.
  elfcpp::DW_CFA_nop,
  elfcpp::DW_CFA_nop,
  elfcpp::DW_CFA_nop
};

// Write out the PLT.  This uses the hand-coded instructions above,
// and adjusts them as needed.

template<int size>
void
Output_data_plt_s390<size>::do_write(Output_file* of)
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
  // The base address of the PLT portion of the .got section,
  // which is where the GOT pointer will point, and where the
  // three reserved GOT entries are located.
  typename elfcpp::Elf_types<size>::Elf_Addr got_address
    = this->got_plt_->address();

  this->fill_first_plt_entry(pov, got_address, plt_address);
  pov += this->get_plt_entry_size();

  unsigned char* got_pov = got_view;

  const int rel_size = elfcpp::Elf_sizes<size>::rela_size;

  unsigned int plt_offset = this->get_plt_entry_size();
  unsigned int plt_rel_offset = 0;
  unsigned int got_offset = 3 * size / 8;
  const unsigned int count = this->count_ + this->irelative_count_;
  // The first three entries in the GOT are reserved, and are written
  // by Output_data_got_plt_s390::do_write.
  got_pov += 3 * size / 8;

  for (unsigned int plt_index = 0;
       plt_index < count;
       ++plt_index,
	 pov += plt_entry_size,
	 got_pov += size / 8,
	 plt_offset += plt_entry_size,
	 plt_rel_offset += rel_size,
	 got_offset += size / 8)
    {
      // Set and adjust the PLT entry itself.
      unsigned int lazy_offset = this->fill_plt_entry(pov,
						      got_address, plt_address,
						      got_offset, plt_offset,
						      plt_rel_offset);

      // Set the entry in the GOT.
      elfcpp::Swap<size, true>::writeval(got_pov,
					plt_address + plt_offset + lazy_offset);
    }

  gold_assert(static_cast<section_size_type>(pov - oview) == oview_size);
  gold_assert(static_cast<section_size_type>(got_pov - got_view) == got_size);

  of->write_output_view(offset, oview_size, oview);
  of->write_output_view(got_file_offset, got_size, got_view);
}

// Get the GOT section, creating it if necessary.

template<int size>
Output_data_got<size, true>*
Target_s390<size>::got_section(Symbol_table* symtab, Layout* layout)
{
  if (this->got_ == NULL)
    {
      gold_assert(symtab != NULL && layout != NULL);

      // When using -z now, we can treat .got as a relro section.
      // Without -z now, it is modified after program startup by lazy
      // PLT relocations.
      bool is_got_relro = parameters->options().now();
      Output_section_order got_order = (is_got_relro
					? ORDER_RELRO_LAST
					: ORDER_DATA);

      // The old GNU linker creates a .got.plt section.  We just
      // create another set of data in the .got section.  Note that we
      // always create a PLT if we create a GOT, although the PLT
      // might be empty.
      this->got_plt_ = new Output_data_got_plt_s390<size>(layout);
      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE),
				      this->got_plt_, got_order, is_got_relro);

      // The first three entries are reserved.
      this->got_plt_->set_current_data_size(3 * size / 8);

      // If there are any IRELATIVE relocations, they get GOT entries
      // in .got.plt after the jump slot entries.
      this->got_irelative_ = new Output_data_space(size / 8, "** GOT IRELATIVE PLT");
      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE),
				      this->got_irelative_,
				      got_order, is_got_relro);

      // Unlike some targets (.e.g x86), S/390 does not use separate .got and
      // .got.plt sections in output.  The output .got section contains both
      // PLT and non-PLT GOT entries.
      this->got_ = new Output_data_got<size, true>();

      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE),
				      this->got_, got_order, is_got_relro);

      // Define _GLOBAL_OFFSET_TABLE_ at the start of the GOT.
      this->global_offset_table_ =
        symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
				      Symbol_table::PREDEFINED,
				      this->got_plt_,
				      0, 0, elfcpp::STT_OBJECT,
				      elfcpp::STB_LOCAL,
				      elfcpp::STV_HIDDEN, 0,
				      false, false);

    }
  return this->got_;
}

// Get the dynamic reloc section, creating it if necessary.

template<int size>
typename Target_s390<size>::Reloc_section*
Target_s390<size>::rela_dyn_section(Layout* layout)
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
typename Target_s390<size>::Reloc_section*
Target_s390<size>::rela_irelative_section(Layout* layout)
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

// Write the first three reserved words of the .got.plt section.
// The remainder of the section is written while writing the PLT
// in Output_data_plt_s390::do_write.

template<int size>
void
Output_data_got_plt_s390<size>::do_write(Output_file* of)
{
  // The first entry in the GOT is the address of the .dynamic section
  // aka the PT_DYNAMIC segment.  The next two entries are reserved.
  // We saved space for them when we created the section in
  // Target_x86_64::got_section.
  const off_t got_file_offset = this->offset();
  gold_assert(this->data_size() >= 3 * size / 8);
  unsigned char* const got_view =
      of->get_output_view(got_file_offset, 3 * size / 8);
  Output_section* dynamic = this->layout_->dynamic_section();
  uint64_t dynamic_addr = dynamic == NULL ? 0 : dynamic->address();
  elfcpp::Swap<size, true>::writeval(got_view, dynamic_addr);
  memset(got_view + size / 8, 0, 2 * size / 8);
  of->write_output_view(got_file_offset, 3 * size / 8, got_view);
}

// Create the PLT section.

template<int size>
void
Target_s390<size>::make_plt_section(Symbol_table* symtab, Layout* layout)
{
  if (this->plt_ == NULL)
    {
      // Create the GOT sections first.
      this->got_section(symtab, layout);

      // Ensure that .rela.dyn always appears before .rela.plt  This is
      // necessary due to how, on 32-bit S/390 and some other targets,
      // .rela.dyn needs to include .rela.plt in it's range.
      this->rela_dyn_section(layout);

      this->plt_ = new Output_data_plt_s390<size>(layout,
		      this->got_, this->got_plt_, this->got_irelative_);

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

// Create a PLT entry for a global symbol.

template<int size>
void
Target_s390<size>::make_plt_entry(Symbol_table* symtab, Layout* layout,
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
Target_s390<size>::make_local_ifunc_plt_entry(
    Symbol_table* symtab, Layout* layout,
    Sized_relobj_file<size, true>* relobj,
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
Target_s390<size>::plt_entry_count() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->entry_count();
}

// Return the offset of the first non-reserved PLT entry.

template<int size>
unsigned int
Target_s390<size>::first_plt_entry_offset() const
{
  return this->plt_->first_plt_entry_offset();
}

// Return the size of each PLT entry.

template<int size>
unsigned int
Target_s390<size>::plt_entry_size() const
{
  return this->plt_->get_plt_entry_size();
}

// Create the GOT and PLT sections for an incremental update.

template<int size>
Output_data_got_base*
Target_s390<size>::init_got_plt_for_update(Symbol_table* symtab,
				       Layout* layout,
				       unsigned int got_count,
				       unsigned int plt_count)
{
  gold_assert(this->got_ == NULL);

  // Add the three reserved entries.
  this->got_plt_ = new Output_data_got_plt_s390<size>(layout, (plt_count + 3) * size / 8);
  layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				  (elfcpp::SHF_ALLOC
				   | elfcpp::SHF_WRITE),
				  this->got_plt_, ORDER_NON_RELRO_FIRST,
				  false);

  // If there are any IRELATIVE relocations, they get GOT entries in
  // .got.plt after the jump slot entries.
  this->got_irelative_ = new Output_data_space(0, size / 8, "** GOT IRELATIVE PLT");
  layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				  elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE,
				  this->got_irelative_,
				  ORDER_NON_RELRO_FIRST, false);

  this->got_ = new Output_data_got<size, true>(got_count * size / 8);
  layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				  (elfcpp::SHF_ALLOC
				   | elfcpp::SHF_WRITE),
				  this->got_, ORDER_RELRO_LAST,
				  true);

  // Define _GLOBAL_OFFSET_TABLE_ at the start of the PLT.
  this->global_offset_table_ =
    symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
				  Symbol_table::PREDEFINED,
				  this->got_plt_,
				  0, 0, elfcpp::STT_OBJECT,
				  elfcpp::STB_LOCAL,
				  elfcpp::STV_HIDDEN, 0,
				  false, false);

  // Create the PLT section.
  this->plt_ = new Output_data_plt_s390<size>(layout,
		  this->got_, this->got_plt_, this->got_irelative_, plt_count);

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
Target_s390<size>::reserve_local_got_entry(
    unsigned int got_index,
    Sized_relobj<size, true>* obj,
    unsigned int r_sym,
    unsigned int got_type)
{
  unsigned int got_offset = got_index * size / 8;
  Reloc_section* rela_dyn = this->rela_dyn_section(NULL);

  this->got_->reserve_local(got_index, obj, r_sym, got_type);
  switch (got_type)
    {
    case GOT_TYPE_STANDARD:
      if (parameters->options().output_is_position_independent())
	rela_dyn->add_local_relative(obj, r_sym, elfcpp::R_390_RELATIVE,
				     this->got_, got_offset, 0, false);
      break;
    case GOT_TYPE_TLS_OFFSET:
      rela_dyn->add_local(obj, r_sym, elfcpp::R_390_TLS_TPOFF,
			  this->got_, got_offset, 0);
      break;
    case GOT_TYPE_TLS_PAIR:
      this->got_->reserve_slot(got_index + 1);
      rela_dyn->add_local(obj, r_sym, elfcpp::R_390_TLS_DTPMOD,
			  this->got_, got_offset, 0);
      break;
    default:
      gold_unreachable();
    }
}

// Reserve a GOT entry for a global symbol, and regenerate any
// necessary dynamic relocations.

template<int size>
void
Target_s390<size>::reserve_global_got_entry(unsigned int got_index,
					      Symbol* gsym,
					      unsigned int got_type)
{
  unsigned int got_offset = got_index * size / 8;
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
	    rela_dyn->add_global(gsym, elfcpp::R_390_GLOB_DAT,
				 this->got_, got_offset, 0);
	  else
	    rela_dyn->add_global_relative(gsym, elfcpp::R_390_RELATIVE,
					  this->got_, got_offset, 0, false);
	}
      break;
    case GOT_TYPE_TLS_OFFSET:
      rela_dyn->add_global_relative(gsym, elfcpp::R_390_TLS_TPOFF,
				    this->got_, got_offset, 0, false);
      break;
    case GOT_TYPE_TLS_PAIR:
      this->got_->reserve_slot(got_index + 1);
      rela_dyn->add_global_relative(gsym, elfcpp::R_390_TLS_DTPMOD,
				    this->got_, got_offset, 0, false);
      rela_dyn->add_global_relative(gsym, elfcpp::R_390_TLS_DTPOFF,
				    this->got_, got_offset + size / 8, 0, false);
      break;
    default:
      gold_unreachable();
    }
}

// Register an existing PLT entry for a global symbol.

template<int size>
void
Target_s390<size>::register_global_plt_entry(Symbol_table* symtab,
					       Layout* layout,
					       unsigned int plt_index,
					       Symbol* gsym)
{
  gold_assert(this->plt_ != NULL);
  gold_assert(!gsym->has_plt_offset());

  this->plt_->reserve_slot(plt_index);

  gsym->set_plt_offset((plt_index + 1) * this->plt_entry_size());

  unsigned int got_offset = (plt_index + 3) * size / 8;
  this->plt_->add_relocation(symtab, layout, gsym, got_offset);
}

// Force a COPY relocation for a given symbol.

template<int size>
void
Target_s390<size>::emit_copy_reloc(
    Symbol_table* symtab, Symbol* sym, Output_section* os, off_t offset)
{
  this->copy_relocs_.emit_copy_reloc(symtab,
				     symtab->get_sized_symbol<size>(sym),
				     os,
				     offset,
				     this->rela_dyn_section(NULL));
}

// Create a GOT entry for the TLS module index.

template<int size>
unsigned int
Target_s390<size>::got_mod_index_entry(Symbol_table* symtab, Layout* layout,
					 Sized_relobj_file<size, true>* object)
{
  if (this->got_mod_index_offset_ == -1U)
    {
      gold_assert(symtab != NULL && layout != NULL && object != NULL);
      Reloc_section* rela_dyn = this->rela_dyn_section(layout);
      Output_data_got<size, true>* got = this->got_section(symtab, layout);
      unsigned int got_offset = got->add_constant(0);
      rela_dyn->add_local(object, 0, elfcpp::R_390_TLS_DTPMOD, got,
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
Target_s390<size>::optimize_tls_reloc(bool is_final, int r_type)
{
  // If we are generating a shared library, then we can't do anything
  // in the linker.
  if (parameters->options().shared())
    return tls::TLSOPT_NONE;

  switch (r_type)
    {
    case elfcpp::R_390_TLS_GD32:
    case elfcpp::R_390_TLS_GD64:
    case elfcpp::R_390_TLS_GDCALL:
      // These are General-Dynamic which permits fully general TLS
      // access.  Since we know that we are generating an executable,
      // we can convert this to Initial-Exec.  If we also know that
      // this is a local symbol, we can further switch to Local-Exec.
      if (is_final)
	return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_TO_IE;

    case elfcpp::R_390_TLS_LDM32:
    case elfcpp::R_390_TLS_LDM64:
    case elfcpp::R_390_TLS_LDO32:
    case elfcpp::R_390_TLS_LDO64:
    case elfcpp::R_390_TLS_LDCALL:
      // This is Local-Dynamic, which refers to a local symbol in the
      // dynamic TLS block.  Since we know that we generating an
      // executable, we can switch to Local-Exec.
      return tls::TLSOPT_TO_LE;

    case elfcpp::R_390_TLS_IE32:
    case elfcpp::R_390_TLS_IE64:
    case elfcpp::R_390_TLS_GOTIE32:
    case elfcpp::R_390_TLS_GOTIE64:
    case elfcpp::R_390_TLS_LOAD:
      // These are Initial-Exec relocs which get the thread offset
      // from the GOT.  If we know that we are linking against the
      // local symbol, we can switch to Local-Exec, which links the
      // thread offset into the instruction.
      if (is_final)
	return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_NONE;

    case elfcpp::R_390_TLS_GOTIE12:
    case elfcpp::R_390_TLS_IEENT:
    case elfcpp::R_390_TLS_GOTIE20:
      // These are Initial-Exec, but cannot be optimized.
      return tls::TLSOPT_NONE;

    case elfcpp::R_390_TLS_LE32:
    case elfcpp::R_390_TLS_LE64:
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
Target_s390<size>::Scan::get_reference_flags(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_390_NONE:
    case elfcpp::R_390_GNU_VTINHERIT:
    case elfcpp::R_390_GNU_VTENTRY:
    case elfcpp::R_390_GOTPC:
    case elfcpp::R_390_GOTPCDBL:
      // No symbol reference.
      return 0;

    case elfcpp::R_390_64:
    case elfcpp::R_390_32:
    case elfcpp::R_390_20:
    case elfcpp::R_390_16:
    case elfcpp::R_390_12:
    case elfcpp::R_390_8:
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_390_PC12DBL:
    case elfcpp::R_390_PC16:
    case elfcpp::R_390_PC16DBL:
    case elfcpp::R_390_PC24DBL:
    case elfcpp::R_390_PC32:
    case elfcpp::R_390_PC32DBL:
    case elfcpp::R_390_PC64:
    case elfcpp::R_390_GOTOFF16:
    case elfcpp::R_390_GOTOFF32:
    case elfcpp::R_390_GOTOFF64:
      return Symbol::RELATIVE_REF;

    case elfcpp::R_390_PLT12DBL:
    case elfcpp::R_390_PLT16DBL:
    case elfcpp::R_390_PLT24DBL:
    case elfcpp::R_390_PLT32:
    case elfcpp::R_390_PLT32DBL:
    case elfcpp::R_390_PLT64:
    case elfcpp::R_390_PLTOFF16:
    case elfcpp::R_390_PLTOFF32:
    case elfcpp::R_390_PLTOFF64:
      return Symbol::FUNCTION_CALL | Symbol::RELATIVE_REF;

    case elfcpp::R_390_GOT12:
    case elfcpp::R_390_GOT16:
    case elfcpp::R_390_GOT20:
    case elfcpp::R_390_GOT32:
    case elfcpp::R_390_GOT64:
    case elfcpp::R_390_GOTENT:
    case elfcpp::R_390_GOTPLT12:
    case elfcpp::R_390_GOTPLT16:
    case elfcpp::R_390_GOTPLT20:
    case elfcpp::R_390_GOTPLT32:
    case elfcpp::R_390_GOTPLT64:
    case elfcpp::R_390_GOTPLTENT:
      // Absolute in GOT.
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_390_TLS_GD32:          // Global-dynamic
    case elfcpp::R_390_TLS_GD64:
    case elfcpp::R_390_TLS_GDCALL:
    case elfcpp::R_390_TLS_LDM32:         // Local-dynamic
    case elfcpp::R_390_TLS_LDM64:
    case elfcpp::R_390_TLS_LDO32:
    case elfcpp::R_390_TLS_LDO64:
    case elfcpp::R_390_TLS_LDCALL:
    case elfcpp::R_390_TLS_IE32:          // Initial-exec
    case elfcpp::R_390_TLS_IE64:
    case elfcpp::R_390_TLS_IEENT:
    case elfcpp::R_390_TLS_GOTIE12:
    case elfcpp::R_390_TLS_GOTIE20:
    case elfcpp::R_390_TLS_GOTIE32:
    case elfcpp::R_390_TLS_GOTIE64:
    case elfcpp::R_390_TLS_LOAD:
    case elfcpp::R_390_TLS_LE32:          // Local-exec
    case elfcpp::R_390_TLS_LE64:
      return Symbol::TLS_REF;

    case elfcpp::R_390_COPY:
    case elfcpp::R_390_GLOB_DAT:
    case elfcpp::R_390_JMP_SLOT:
    case elfcpp::R_390_RELATIVE:
    case elfcpp::R_390_IRELATIVE:
    case elfcpp::R_390_TLS_TPOFF:
    case elfcpp::R_390_TLS_DTPOFF:
    case elfcpp::R_390_TLS_DTPMOD:
    default:
      // Not expected.  We will give an error later.
      return 0;
    }
}

// Report an unsupported relocation against a local symbol.

template<int size>
void
Target_s390<size>::Scan::unsupported_reloc_local(
     Sized_relobj_file<size, true>* object,
     unsigned int r_type)
{
  gold_error(_("%s: unsupported reloc %u against local symbol"),
	     object->name().c_str(), r_type);
}

// We are about to emit a dynamic relocation of type R_TYPE.  If the
// dynamic linker does not support it, issue an error.

template<int size>
void
Target_s390<size>::Scan::check_non_pic(Relobj* object, unsigned int r_type)
{
  gold_assert(r_type != elfcpp::R_390_NONE);

  if (size == 64)
    {
      switch (r_type)
	{
	  // These are the relocation types supported by glibc for s390 64-bit.
	case elfcpp::R_390_RELATIVE:
	case elfcpp::R_390_IRELATIVE:
	case elfcpp::R_390_COPY:
	case elfcpp::R_390_GLOB_DAT:
	case elfcpp::R_390_JMP_SLOT:
	case elfcpp::R_390_TLS_DTPMOD:
	case elfcpp::R_390_TLS_DTPOFF:
	case elfcpp::R_390_TLS_TPOFF:
	case elfcpp::R_390_8:
	case elfcpp::R_390_16:
	case elfcpp::R_390_32:
	case elfcpp::R_390_64:
	case elfcpp::R_390_PC16:
	case elfcpp::R_390_PC16DBL:
	case elfcpp::R_390_PC32:
	case elfcpp::R_390_PC32DBL:
	case elfcpp::R_390_PC64:
	  return;

	default:
	  break;
	}
    }
  else
    {
      switch (r_type)
	{
	  // These are the relocation types supported by glibc for s390 32-bit.
	case elfcpp::R_390_RELATIVE:
	case elfcpp::R_390_IRELATIVE:
	case elfcpp::R_390_COPY:
	case elfcpp::R_390_GLOB_DAT:
	case elfcpp::R_390_JMP_SLOT:
	case elfcpp::R_390_TLS_DTPMOD:
	case elfcpp::R_390_TLS_DTPOFF:
	case elfcpp::R_390_TLS_TPOFF:
	case elfcpp::R_390_8:
	case elfcpp::R_390_16:
	case elfcpp::R_390_32:
	case elfcpp::R_390_PC16:
	case elfcpp::R_390_PC16DBL:
	case elfcpp::R_390_PC32:
	case elfcpp::R_390_PC32DBL:
	  return;

	default:
	  break;
	}
    }

  // This prevents us from issuing more than one error per reloc
  // section.  But we can still wind up issuing more than one
  // error per object file.
  if (this->issued_non_pic_error_)
    return;
  gold_assert(parameters->options().output_is_position_independent());
  object->error(_("requires unsupported dynamic reloc; "
		  "recompile with -fPIC"));
  this->issued_non_pic_error_ = true;
  return;
}

// Return whether we need to make a PLT entry for a relocation of the
// given type against a STT_GNU_IFUNC symbol.

template<int size>
bool
Target_s390<size>::Scan::reloc_needs_plt_for_ifunc(
     Sized_relobj_file<size, true>* object,
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
Target_s390<size>::Scan::local(Symbol_table* symtab,
				 Layout* layout,
				 Target_s390<size>* target,
				 Sized_relobj_file<size, true>* object,
				 unsigned int data_shndx,
				 Output_section* output_section,
				 const elfcpp::Rela<size, true>& reloc,
				 unsigned int r_type,
				 const elfcpp::Sym<size, true>& lsym,
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
    case elfcpp::R_390_NONE:
    case elfcpp::R_390_GNU_VTINHERIT:
    case elfcpp::R_390_GNU_VTENTRY:
      break;

    case elfcpp::R_390_64:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for this
      // location.  The relocation applied at link time will apply the
      // link-time value, so we flag the location with an
      // R_390_RELATIVE relocation so the dynamic loader can
      // relocate it easily.
      if (parameters->options().output_is_position_independent() && size == 64)
	{
	  unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
	  Reloc_section* rela_dyn = target->rela_dyn_section(layout);
	  rela_dyn->add_local_relative(object, r_sym,
				       elfcpp::R_390_RELATIVE,
				       output_section, data_shndx,
				       reloc.get_r_offset(),
				       reloc.get_r_addend(), is_ifunc);
	}
      break;

    case elfcpp::R_390_32:
    case elfcpp::R_390_20:
    case elfcpp::R_390_16:
    case elfcpp::R_390_12:
    case elfcpp::R_390_8:
      if (parameters->options().output_is_position_independent())
	{
	  if (size == 32 && r_type == elfcpp::R_390_32)
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
	      Reloc_section* rela_dyn = target->rela_dyn_section(layout);
	      rela_dyn->add_local_relative(object, r_sym,
					   elfcpp::R_390_RELATIVE,
					   output_section, data_shndx,
					   reloc.get_r_offset(),
					   reloc.get_r_addend(), is_ifunc);
	      break;
	    }

	  check_non_pic(object, r_type);

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

    case elfcpp::R_390_PC12DBL:
    case elfcpp::R_390_PC16:
    case elfcpp::R_390_PC16DBL:
    case elfcpp::R_390_PC24DBL:
    case elfcpp::R_390_PC32:
    case elfcpp::R_390_PC32DBL:
    case elfcpp::R_390_PC64:
      break;

    case elfcpp::R_390_PLT12DBL:
    case elfcpp::R_390_PLT16DBL:
    case elfcpp::R_390_PLT24DBL:
    case elfcpp::R_390_PLT32:
    case elfcpp::R_390_PLT32DBL:
    case elfcpp::R_390_PLT64:
      // Since we know this is a local symbol, we can handle this as a
      // PC32 reloc.
      break;

    case elfcpp::R_390_GOTPC:
    case elfcpp::R_390_GOTPCDBL:
    case elfcpp::R_390_GOTOFF16:
    case elfcpp::R_390_GOTOFF32:
    case elfcpp::R_390_GOTOFF64:
    case elfcpp::R_390_PLTOFF16:
    case elfcpp::R_390_PLTOFF32:
    case elfcpp::R_390_PLTOFF64:
      // We need a GOT section.
      target->got_section(symtab, layout);
      // For PLTOFF*, we'd normally want a PLT section, but since we
      // know this is a local symbol, no PLT is needed.
      break;

    case elfcpp::R_390_GOT12:
    case elfcpp::R_390_GOT16:
    case elfcpp::R_390_GOT20:
    case elfcpp::R_390_GOT32:
    case elfcpp::R_390_GOT64:
    case elfcpp::R_390_GOTENT:
    case elfcpp::R_390_GOTPLT12:
    case elfcpp::R_390_GOTPLT16:
    case elfcpp::R_390_GOTPLT20:
    case elfcpp::R_390_GOTPLT32:
    case elfcpp::R_390_GOTPLT64:
    case elfcpp::R_390_GOTPLTENT:
      {
	// The symbol requires a GOT section.
	Output_data_got<size, true>* got = target->got_section(symtab, layout);

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
		unsigned int got_offset =
		  object->local_got_offset(r_sym, GOT_TYPE_STANDARD);
		rela_dyn->add_local_relative(object, r_sym,
					     elfcpp::R_390_RELATIVE,
					     got, got_offset, 0, is_ifunc);
	      }
	  }
	// For GOTPLT*, we'd normally want a PLT section, but since
	// we know this is a local symbol, no PLT is needed.
      }
      break;

    case elfcpp::R_390_COPY:
    case elfcpp::R_390_GLOB_DAT:
    case elfcpp::R_390_JMP_SLOT:
    case elfcpp::R_390_RELATIVE:
    case elfcpp::R_390_IRELATIVE:
      // These are outstanding tls relocs, which are unexpected when linking
    case elfcpp::R_390_TLS_TPOFF:
    case elfcpp::R_390_TLS_DTPOFF:
    case elfcpp::R_390_TLS_DTPMOD:
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

      // These are initial tls relocs, which are expected when linking
    case elfcpp::R_390_TLS_GD32:          // Global-dynamic
    case elfcpp::R_390_TLS_GD64:
    case elfcpp::R_390_TLS_GDCALL:
    case elfcpp::R_390_TLS_LDM32:         // Local-dynamic
    case elfcpp::R_390_TLS_LDM64:
    case elfcpp::R_390_TLS_LDO32:
    case elfcpp::R_390_TLS_LDO64:
    case elfcpp::R_390_TLS_LDCALL:
    case elfcpp::R_390_TLS_IE32:          // Initial-exec
    case elfcpp::R_390_TLS_IE64:
    case elfcpp::R_390_TLS_IEENT:
    case elfcpp::R_390_TLS_GOTIE12:
    case elfcpp::R_390_TLS_GOTIE20:
    case elfcpp::R_390_TLS_GOTIE32:
    case elfcpp::R_390_TLS_GOTIE64:
    case elfcpp::R_390_TLS_LOAD:
    case elfcpp::R_390_TLS_LE32:          // Local-exec
    case elfcpp::R_390_TLS_LE64:
      {
	bool output_is_shared = parameters->options().shared();
	const tls::Tls_optimization optimized_type
	    = Target_s390<size>::optimize_tls_reloc(!output_is_shared,
						      r_type);
	switch (r_type)
	  {
	  case elfcpp::R_390_TLS_GD32:       // General-dynamic
	  case elfcpp::R_390_TLS_GD64:
	  case elfcpp::R_390_TLS_GDCALL:
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Output_data_got<size, true>* got
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
					       elfcpp::R_390_TLS_DTPMOD);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_390_TLS_LDM32:       // Local-dynamic
	  case elfcpp::R_390_TLS_LDM64:
	  case elfcpp::R_390_TLS_LDCALL:
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_390_TLS_LDO32:
	  case elfcpp::R_390_TLS_LDO64:
	    break;

	  case elfcpp::R_390_TLS_IE32:    // Initial-exec
	  case elfcpp::R_390_TLS_IE64:
	    // These two involve an absolute address
	    if (parameters->options().shared()
		&& optimized_type == tls::TLSOPT_NONE)
	      {
		if ((size == 32 && r_type == elfcpp::R_390_TLS_IE32) ||
		    (size == 64 && r_type == elfcpp::R_390_TLS_IE64))
		  {
		    // We need to create a dynamic relocation.
		    Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		    unsigned int r_sym =
			elfcpp::elf_r_sym<size>(reloc.get_r_info());
		    rela_dyn->add_local_relative(object, r_sym,
						elfcpp::R_390_RELATIVE,
						output_section, data_shndx,
						reloc.get_r_offset(),
						reloc.get_r_addend(), false);
		  }
		else
		  {
		    unsupported_reloc_local(object, r_type);
		  }
	      }
	    // Fall through.
	  case elfcpp::R_390_TLS_IEENT:
	  case elfcpp::R_390_TLS_GOTIE12:
	  case elfcpp::R_390_TLS_GOTIE20:
	  case elfcpp::R_390_TLS_GOTIE32:
	  case elfcpp::R_390_TLS_GOTIE64:
	  case elfcpp::R_390_TLS_LOAD:
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		if (!output_is_shared)
		  {
		    // We're making an executable, and the symbol is local, but
		    // we cannot optimize to LE.  Make a const GOT entry instead.
		    Output_data_got<size, true>* got
			= target->got_section(symtab, layout);
		    unsigned int r_sym
			= elfcpp::elf_r_sym<size>(reloc.get_r_info());
		    got->add_local_plt(object, r_sym, GOT_TYPE_TLS_OFFSET);
		  }
		else
		{
		  // Create a GOT entry for the tp-relative offset.
		  Output_data_got<size, true>* got
		      = target->got_section(symtab, layout);
		  unsigned int r_sym
		      = elfcpp::elf_r_sym<size>(reloc.get_r_info());
		  got->add_local_with_rel(object, r_sym, GOT_TYPE_TLS_OFFSET,
					  target->rela_dyn_section(layout),
					  elfcpp::R_390_TLS_TPOFF);
		}
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_390_TLS_LE32:     // Local-exec
	  case elfcpp::R_390_TLS_LE64:
	    layout->set_has_static_tls();
	    if (output_is_shared)
	    {
	      // We need to create a dynamic relocation.
	      if ((size == 32 && r_type == elfcpp::R_390_TLS_LE32) ||
	          (size == 64 && r_type == elfcpp::R_390_TLS_LE64))
		{
		  Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		  unsigned int r_sym
		      = elfcpp::elf_r_sym<size>(reloc.get_r_info());
		  gold_assert(lsym.get_st_type() != elfcpp::STT_SECTION);
		  rela_dyn->add_local(object, r_sym, elfcpp::R_390_TLS_TPOFF,
				      output_section, data_shndx,
				      reloc.get_r_offset(),
				      reloc.get_r_addend());
		}
	      else
		{
		  unsupported_reloc_local(object, r_type);
		}
	    }
	    break;

	  default:
	    gold_unreachable();
	  }
      }
      break;

    default:
      gold_error(_("%s: unsupported reloc %u against local symbol"),
		 object->name().c_str(), r_type);
      break;
    }
}

// Scan a relocation for a global symbol.

template<int size>
inline void
Target_s390<size>::Scan::global(Symbol_table* symtab,
			    Layout* layout,
			    Target_s390<size>* target,
			    Sized_relobj_file<size, true>* object,
			    unsigned int data_shndx,
			    Output_section* output_section,
			    const elfcpp::Rela<size, true>& reloc,
			    unsigned int r_type,
			    Symbol* gsym)
{
  // A STT_GNU_IFUNC symbol may require a PLT entry.
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && this->reloc_needs_plt_for_ifunc(object, r_type))
    target->make_plt_entry(symtab, layout, gsym);

  switch (r_type)
    {
    case elfcpp::R_390_NONE:
    case elfcpp::R_390_GNU_VTINHERIT:
    case elfcpp::R_390_GNU_VTENTRY:
      break;

    case elfcpp::R_390_64:
    case elfcpp::R_390_32:
    case elfcpp::R_390_20:
    case elfcpp::R_390_16:
    case elfcpp::R_390_12:
    case elfcpp::R_390_8:
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
	    else if (((size == 64 && r_type == elfcpp::R_390_64)
		      || (size == 32 && r_type == elfcpp::R_390_32))
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
		unsigned int r_type = elfcpp::R_390_IRELATIVE;
		rela_dyn->add_symbolless_global_addend(gsym, r_type,
						       output_section, object,
						       data_shndx,
						       reloc.get_r_offset(),
						       reloc.get_r_addend());
	      }
	    else if (((size == 64 && r_type == elfcpp::R_390_64)
		      || (size == 32 && r_type == elfcpp::R_390_32))
		     && gsym->can_use_relative_reloc(false))
	      {
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_global_relative(gsym, elfcpp::R_390_RELATIVE,
					      output_section, object,
					      data_shndx,
					      reloc.get_r_offset(),
					      reloc.get_r_addend(), false);
	      }
	    else
	      {
		check_non_pic(object, r_type);
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_global(gsym, r_type, output_section, object,
				     data_shndx, reloc.get_r_offset(),
				     reloc.get_r_addend());
	      }
	  }
      }
      break;

    case elfcpp::R_390_PC12DBL:
    case elfcpp::R_390_PC16:
    case elfcpp::R_390_PC16DBL:
    case elfcpp::R_390_PC24DBL:
    case elfcpp::R_390_PC32:
    case elfcpp::R_390_PC32DBL:
    case elfcpp::R_390_PC64:
      {
	// Make a PLT entry if necessary.
	if (gsym->needs_plt_entry())
	  {
	    target->make_plt_entry(symtab, layout, gsym);
	    // larl is often used to take address of a function.  Aim the
	    // symbol at the PLT entry.
	    if (gsym->is_from_dynobj() && !parameters->options().shared())
	      gsym->set_needs_dynsym_value();
	  }
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
		check_non_pic(object, r_type);
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_global(gsym, r_type, output_section, object,
				     data_shndx, reloc.get_r_offset(),
				     reloc.get_r_addend());
	      }
	  }
      }
      break;

    case elfcpp::R_390_PLT12DBL:
    case elfcpp::R_390_PLT16DBL:
    case elfcpp::R_390_PLT24DBL:
    case elfcpp::R_390_PLT32:
    case elfcpp::R_390_PLT32DBL:
    case elfcpp::R_390_PLT64:
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

    case elfcpp::R_390_GOTPC:
    case elfcpp::R_390_GOTPCDBL:
    case elfcpp::R_390_GOTOFF16:
    case elfcpp::R_390_GOTOFF32:
    case elfcpp::R_390_GOTOFF64:
    case elfcpp::R_390_PLTOFF16:
    case elfcpp::R_390_PLTOFF32:
    case elfcpp::R_390_PLTOFF64:
      // We need a GOT section.
      target->got_section(symtab, layout);
      // For PLTOFF*, we also need a PLT entry (but only if the
      // symbol is not fully resolved).
      if ((r_type == elfcpp::R_390_PLTOFF16
           || r_type == elfcpp::R_390_PLTOFF32
	   || r_type == elfcpp::R_390_PLTOFF64)
	  && !gsym->final_value_is_known())
	target->make_plt_entry(symtab, layout, gsym);
      break;

    case elfcpp::R_390_GOT12:
    case elfcpp::R_390_GOT16:
    case elfcpp::R_390_GOT20:
    case elfcpp::R_390_GOT32:
    case elfcpp::R_390_GOT64:
    case elfcpp::R_390_GOTENT:
    case elfcpp::R_390_GOTPLT12:
    case elfcpp::R_390_GOTPLT16:
    case elfcpp::R_390_GOTPLT20:
    case elfcpp::R_390_GOTPLT32:
    case elfcpp::R_390_GOTPLT64:
    case elfcpp::R_390_GOTPLTENT:
      {
	// The symbol requires a GOT entry.
	Output_data_got<size, true>* got = target->got_section(symtab, layout);

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
				       elfcpp::R_390_GLOB_DAT);
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
						  elfcpp::R_390_RELATIVE,
						  got, got_off, 0, false);
		  }
	      }
	  }
      }
      break;

    case elfcpp::R_390_COPY:
    case elfcpp::R_390_GLOB_DAT:
    case elfcpp::R_390_JMP_SLOT:
    case elfcpp::R_390_RELATIVE:
    case elfcpp::R_390_IRELATIVE:
      // These are outstanding tls relocs, which are unexpected when linking
    case elfcpp::R_390_TLS_TPOFF:
    case elfcpp::R_390_TLS_DTPOFF:
    case elfcpp::R_390_TLS_DTPMOD:
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

      // These are initial tls relocs, which are expected for global()
    case elfcpp::R_390_TLS_GD32:          // Global-dynamic
    case elfcpp::R_390_TLS_GD64:
    case elfcpp::R_390_TLS_GDCALL:
    case elfcpp::R_390_TLS_LDM32:         // Local-dynamic
    case elfcpp::R_390_TLS_LDM64:
    case elfcpp::R_390_TLS_LDO32:
    case elfcpp::R_390_TLS_LDO64:
    case elfcpp::R_390_TLS_LDCALL:
    case elfcpp::R_390_TLS_IE32:          // Initial-exec
    case elfcpp::R_390_TLS_IE64:
    case elfcpp::R_390_TLS_IEENT:
    case elfcpp::R_390_TLS_GOTIE12:
    case elfcpp::R_390_TLS_GOTIE20:
    case elfcpp::R_390_TLS_GOTIE32:
    case elfcpp::R_390_TLS_GOTIE64:
    case elfcpp::R_390_TLS_LOAD:
    case elfcpp::R_390_TLS_LE32:          // Local-exec
    case elfcpp::R_390_TLS_LE64:
      {
	// For the optimizable Initial-Exec model, we can treat undef symbols
	// as final when building an executable.
	const bool is_final = (gsym->final_value_is_known() ||
			       ((r_type == elfcpp::R_390_TLS_IE32 ||
			         r_type == elfcpp::R_390_TLS_IE64 ||
			         r_type == elfcpp::R_390_TLS_GOTIE32 ||
			         r_type == elfcpp::R_390_TLS_GOTIE64) &&
			        gsym->is_undefined() &&
				parameters->options().output_is_executable()));
	const tls::Tls_optimization optimized_type
	    = Target_s390<size>::optimize_tls_reloc(is_final, r_type);
	switch (r_type)
	  {
	  case elfcpp::R_390_TLS_GD32:       // General-dynamic
	  case elfcpp::R_390_TLS_GD64:
	  case elfcpp::R_390_TLS_GDCALL:
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Output_data_got<size, true>* got
		    = target->got_section(symtab, layout);
		got->add_global_pair_with_rel(gsym, GOT_TYPE_TLS_PAIR,
					      target->rela_dyn_section(layout),
					      elfcpp::R_390_TLS_DTPMOD,
					      elfcpp::R_390_TLS_DTPOFF);
	      }
	    else if (optimized_type == tls::TLSOPT_TO_IE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<size, true>* got
		    = target->got_section(symtab, layout);
		got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
					 target->rela_dyn_section(layout),
					 elfcpp::R_390_TLS_TPOFF);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_390_TLS_LDM32:       // Local-dynamic
	  case elfcpp::R_390_TLS_LDM64:
	  case elfcpp::R_390_TLS_LDCALL:
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_390_TLS_LDO32:
	  case elfcpp::R_390_TLS_LDO64:
	    break;

	  case elfcpp::R_390_TLS_IE32:    // Initial-exec
	  case elfcpp::R_390_TLS_IE64:
	    // These two involve an absolute address
	    if (parameters->options().shared())
	      {
		if ((size == 32 && r_type == elfcpp::R_390_TLS_IE32) ||
		    (size == 64 && r_type == elfcpp::R_390_TLS_IE64))
		  {
		    // We need to create a dynamic relocation.
		    Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		    rela_dyn->add_global_relative(gsym, elfcpp::R_390_RELATIVE,
						  output_section, object,
						  data_shndx,
						  reloc.get_r_offset(),
						  reloc.get_r_addend(), false);
		  }
		else
		  {
		    unsupported_reloc_global(object, r_type, gsym);
		  }
	      }
	    // Fall through.
	  case elfcpp::R_390_TLS_IEENT:
	  case elfcpp::R_390_TLS_GOTIE12:
	  case elfcpp::R_390_TLS_GOTIE20:
	  case elfcpp::R_390_TLS_GOTIE32:
	  case elfcpp::R_390_TLS_GOTIE64:
	  case elfcpp::R_390_TLS_LOAD:
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		if (is_final && !parameters->options().shared())
		  {
		    // We're making an executable, and the symbol is local, but
		    // we cannot optimize to LE.  Make a const GOT entry instead.
		    Output_data_got<size, true>* got
			= target->got_section(symtab, layout);
		    got->add_global_plt(gsym, GOT_TYPE_TLS_OFFSET);
		  }
		else
		  {
		    // Create a GOT entry for the tp-relative offset.
		    Output_data_got<size, true>* got
			= target->got_section(symtab, layout);
		    got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
					     target->rela_dyn_section(layout),
					     elfcpp::R_390_TLS_TPOFF);
		  }
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_390_TLS_LE32:     // Local-exec
	  case elfcpp::R_390_TLS_LE64:
	    layout->set_has_static_tls();
	    if (parameters->options().shared())
	      {
		// We need to create a dynamic relocation.
		if ((size == 32 && r_type == elfcpp::R_390_TLS_LE32) ||
		    (size == 64 && r_type == elfcpp::R_390_TLS_LE64))
		  {
		    Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		    rela_dyn->add_global(gsym, elfcpp::R_390_TLS_TPOFF,
					 output_section, object,
					 data_shndx, reloc.get_r_offset(),
					 reloc.get_r_addend());
		  }
		else
		  {
		    unsupported_reloc_global(object, r_type, gsym);
		  }
	      }
	    break;

	  default:
	    gold_unreachable();
	  }
      }
      break;

    default:
      gold_error(_("%s: unsupported reloc %u against global symbol %s"),
		 object->name().c_str(), r_type,
		 gsym->demangled_name().c_str());
      break;
    }
}


// Report an unsupported relocation against a global symbol.

template<int size>
void
Target_s390<size>::Scan::unsupported_reloc_global(
    Sized_relobj_file<size, true>* object,
    unsigned int r_type,
    Symbol* gsym)
{
  gold_error(_("%s: unsupported reloc %u against global symbol %s"),
	     object->name().c_str(), r_type, gsym->demangled_name().c_str());
}

// Returns true if this relocation type could be that of a function pointer.
template<int size>
inline bool
Target_s390<size>::Scan::possible_function_pointer_reloc(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_390_32:
    case elfcpp::R_390_64:
    case elfcpp::R_390_PC32DBL: // could be used by larl insn
    case elfcpp::R_390_GOT12:
    case elfcpp::R_390_GOT16:
    case elfcpp::R_390_GOT20:
    case elfcpp::R_390_GOT32:
    case elfcpp::R_390_GOT64:
    case elfcpp::R_390_GOTENT:
    case elfcpp::R_390_GOTOFF16:
    case elfcpp::R_390_GOTOFF32:
    case elfcpp::R_390_GOTOFF64:
      return true;
    }
  return false;
}

// For safe ICF, scan a relocation for a local symbol to check if it
// corresponds to a function pointer being taken.  In that case mark
// the function whose pointer was taken as not foldable.

template<int size>
inline bool
Target_s390<size>::Scan::local_reloc_may_be_function_pointer(
  Symbol_table* ,
  Layout* ,
  Target_s390<size>* ,
  Sized_relobj_file<size, true>* ,
  unsigned int ,
  Output_section* ,
  const elfcpp::Rela<size, true>& ,
  unsigned int r_type,
  const elfcpp::Sym<size, true>&)
{
  // When building a shared library, do not fold any local symbols.
  return (parameters->options().shared()
	  || possible_function_pointer_reloc(r_type));
}

// For safe ICF, scan a relocation for a global symbol to check if it
// corresponds to a function pointer being taken.  In that case mark
// the function whose pointer was taken as not foldable.

template<int size>
inline bool
Target_s390<size>::Scan::global_reloc_may_be_function_pointer(
  Symbol_table*,
  Layout* ,
  Target_s390<size>* ,
  Sized_relobj_file<size, true>* ,
  unsigned int ,
  Output_section* ,
  const elfcpp::Rela<size, true>& ,
  unsigned int r_type,
  Symbol* gsym)
{
  // When building a shared library, do not fold symbols whose visibility
  // is hidden, internal or protected.
  return ((parameters->options().shared()
	   && (gsym->visibility() == elfcpp::STV_INTERNAL
	       || gsym->visibility() == elfcpp::STV_PROTECTED
	       || gsym->visibility() == elfcpp::STV_HIDDEN))
	  || possible_function_pointer_reloc(r_type));
}

template<int size>
void
Target_s390<size>::gc_process_relocs(Symbol_table* symtab,
				       Layout* layout,
				       Sized_relobj_file<size, true>* object,
				       unsigned int data_shndx,
				       unsigned int sh_type,
				       const unsigned char* prelocs,
				       size_t reloc_count,
				       Output_section* output_section,
				       bool needs_special_offset_handling,
				       size_t local_symbol_count,
				       const unsigned char* plocal_symbols)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, true>
      Classify_reloc;

  if (sh_type == elfcpp::SHT_REL)
    return;

  gold::gc_process_relocs<size, true, Target_s390<size>, Scan, Classify_reloc>(
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

// Perform a relocation.

template<int size>
inline bool
Target_s390<size>::Relocate::relocate(
    const Relocate_info<size, true>* relinfo,
    unsigned int,
    Target_s390<size>* target,
    Output_section*,
    size_t relnum,
    const unsigned char* preloc,
    const Sized_symbol<size>* gsym,
    const Symbol_value<size>* psymval,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size)
{
  if (view == NULL)
    return true;

  const elfcpp::Rela<size, true> rela(preloc);
  unsigned int r_type = elfcpp::elf_r_type<size>(rela.get_r_info());
  const Sized_relobj_file<size, true>* object = relinfo->object;

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

  typename elfcpp::Elf_types<size>::Elf_Addr value = 0;

  switch (r_type)
    {
    case elfcpp::R_390_PLT64:
    case elfcpp::R_390_PLT32:
    case elfcpp::R_390_PLT32DBL:
    case elfcpp::R_390_PLT24DBL:
    case elfcpp::R_390_PLT16DBL:
    case elfcpp::R_390_PLT12DBL:
      gold_assert(gsym == NULL
		  || gsym->has_plt_offset()
		  || gsym->final_value_is_known()
		  || (gsym->is_defined()
		      && !gsym->is_from_dynobj()
		      && !gsym->is_preemptible()));
      // Fall through.
    case elfcpp::R_390_8:
    case elfcpp::R_390_12:
    case elfcpp::R_390_16:
    case elfcpp::R_390_20:
    case elfcpp::R_390_32:
    case elfcpp::R_390_64:
    case elfcpp::R_390_PC16:
    case elfcpp::R_390_PC32:
    case elfcpp::R_390_PC64:
    case elfcpp::R_390_PC32DBL:
    case elfcpp::R_390_PC24DBL:
    case elfcpp::R_390_PC16DBL:
    case elfcpp::R_390_PC12DBL:
      value = psymval->value(object, addend);
      break;

    case elfcpp::R_390_GOTPC:
    case elfcpp::R_390_GOTPCDBL:
      gold_assert(gsym != NULL);
      value = target->got_address() + addend;
      break;

    case elfcpp::R_390_PLTOFF64:
    case elfcpp::R_390_PLTOFF32:
    case elfcpp::R_390_PLTOFF16:
      gold_assert(gsym == NULL
		  || gsym->has_plt_offset()
		  || gsym->final_value_is_known());
      // Fall through.
    case elfcpp::R_390_GOTOFF64:
    case elfcpp::R_390_GOTOFF32:
    case elfcpp::R_390_GOTOFF16:
      value = (psymval->value(object, addend)
	       - target->got_address());
      break;

    case elfcpp::R_390_GOT12:
    case elfcpp::R_390_GOT16:
    case elfcpp::R_390_GOT20:
    case elfcpp::R_390_GOT32:
    case elfcpp::R_390_GOT64:
    case elfcpp::R_390_GOTENT:
    case elfcpp::R_390_GOTPLT12:
    case elfcpp::R_390_GOTPLT16:
    case elfcpp::R_390_GOTPLT20:
    case elfcpp::R_390_GOTPLT32:
    case elfcpp::R_390_GOTPLT64:
    case elfcpp::R_390_GOTPLTENT:
      {
        unsigned int got_offset = 0;
        if (gsym != NULL)
	  {
	    gold_assert(gsym->has_got_offset(GOT_TYPE_STANDARD));
	    got_offset = gsym->got_offset(GOT_TYPE_STANDARD);
	  }
        else
	  {
	    unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	    gold_assert(object->local_has_got_offset(r_sym, GOT_TYPE_STANDARD));
	    got_offset = object->local_got_offset(r_sym, GOT_TYPE_STANDARD);
	  }
        value = got_offset + target->got_main_offset() + addend;
      }
      break;

      // These are initial tls relocs, which are expected when linking
    case elfcpp::R_390_TLS_LOAD:
    case elfcpp::R_390_TLS_GDCALL:          // Global-dynamic
    case elfcpp::R_390_TLS_GD32:
    case elfcpp::R_390_TLS_GD64:
    case elfcpp::R_390_TLS_LDCALL:          // Local-dynamic
    case elfcpp::R_390_TLS_LDM32:
    case elfcpp::R_390_TLS_LDM64:
    case elfcpp::R_390_TLS_LDO32:
    case elfcpp::R_390_TLS_LDO64:
    case elfcpp::R_390_TLS_GOTIE12:         // Initial-exec
    case elfcpp::R_390_TLS_GOTIE20:
    case elfcpp::R_390_TLS_GOTIE32:
    case elfcpp::R_390_TLS_GOTIE64:
    case elfcpp::R_390_TLS_IE32:
    case elfcpp::R_390_TLS_IE64:
    case elfcpp::R_390_TLS_IEENT:
    case elfcpp::R_390_TLS_LE32:            // Local-exec
    case elfcpp::R_390_TLS_LE64:
      value = this->relocate_tls(relinfo, target, relnum, rela, r_type, gsym, psymval,
			 view, view_size);
      break;

    default:
      break;
    }

  typename S390_relocate_functions<size>::Status status
      = S390_relocate_functions<size>::STATUS_OK;

  switch (r_type)
    {
    case elfcpp::R_390_NONE:
    case elfcpp::R_390_GNU_VTINHERIT:
    case elfcpp::R_390_GNU_VTENTRY:
    case elfcpp::R_390_TLS_GDCALL:
    case elfcpp::R_390_TLS_LDCALL:
    case elfcpp::R_390_TLS_LOAD:
      break;

    case elfcpp::R_390_64:
    case elfcpp::R_390_GOT64:
    case elfcpp::R_390_GOTPLT64:
    case elfcpp::R_390_PLTOFF64:
    case elfcpp::R_390_GOTOFF64:
    case elfcpp::R_390_TLS_GD64:
    case elfcpp::R_390_TLS_LDM64:
    case elfcpp::R_390_TLS_LDO64:
    case elfcpp::R_390_TLS_GOTIE64:
    case elfcpp::R_390_TLS_IE64:
    case elfcpp::R_390_TLS_LE64:
      Relocate_functions<size, true>::rela64(view, value, 0);
      break;

    case elfcpp::R_390_32:
    case elfcpp::R_390_GOT32:
    case elfcpp::R_390_GOTPLT32:
    case elfcpp::R_390_PLTOFF32:
    case elfcpp::R_390_GOTOFF32:
    case elfcpp::R_390_TLS_GD32:
    case elfcpp::R_390_TLS_LDM32:
    case elfcpp::R_390_TLS_LDO32:
    case elfcpp::R_390_TLS_GOTIE32:
    case elfcpp::R_390_TLS_IE32:
    case elfcpp::R_390_TLS_LE32:
      Relocate_functions<size, true>::rela32(view, value, 0);
      break;

    case elfcpp::R_390_20:
    case elfcpp::R_390_GOT20:
    case elfcpp::R_390_GOTPLT20:
    case elfcpp::R_390_TLS_GOTIE20:
      status = S390_relocate_functions<size>::rela20(view, value);
      break;

    case elfcpp::R_390_16:
    case elfcpp::R_390_GOT16:
    case elfcpp::R_390_GOTPLT16:
    case elfcpp::R_390_PLTOFF16:
    case elfcpp::R_390_GOTOFF16:
      status = S390_relocate_functions<size>::rela16(view, value);
      break;

    case elfcpp::R_390_12:
    case elfcpp::R_390_GOT12:
    case elfcpp::R_390_GOTPLT12:
    case elfcpp::R_390_TLS_GOTIE12:
      status = S390_relocate_functions<size>::rela12(view, value);
      break;

    case elfcpp::R_390_8:
      Relocate_functions<size, true>::rela8(view, value, 0);
      break;

    case elfcpp::R_390_PC16:
      Relocate_functions<size, true>::pcrela16(view, value, 0,
					       address);
      break;

    case elfcpp::R_390_PLT64:
    case elfcpp::R_390_PC64:
      Relocate_functions<size, true>::pcrela64(view, value, 0, address);
      break;

    case elfcpp::R_390_PLT32:
    case elfcpp::R_390_PC32:
    case elfcpp::R_390_GOTPC:
      Relocate_functions<size, true>::pcrela32(view, value, 0, address);
      break;

    case elfcpp::R_390_PLT32DBL:
    case elfcpp::R_390_PC32DBL:
    case elfcpp::R_390_GOTPCDBL:
      status = S390_relocate_functions<size>::pcrela32dbl(view, value, address);
      break;

    case elfcpp::R_390_PLT24DBL:
    case elfcpp::R_390_PC24DBL:
      status = S390_relocate_functions<size>::pcrela24dbl(view, value, address);
      break;

    case elfcpp::R_390_PLT16DBL:
    case elfcpp::R_390_PC16DBL:
      status = S390_relocate_functions<size>::pcrela16dbl(view, value, address);
      break;

    case elfcpp::R_390_PLT12DBL:
    case elfcpp::R_390_PC12DBL:
      status = S390_relocate_functions<size>::pcrela12dbl(view, value, address);
      break;

    case elfcpp::R_390_GOTENT:
    case elfcpp::R_390_GOTPLTENT:
    case elfcpp::R_390_TLS_IEENT:
      value += target->got_address();
      status = S390_relocate_functions<size>::pcrela32dbl(view, value, address);
      break;

    case elfcpp::R_390_COPY:
    case elfcpp::R_390_GLOB_DAT:
    case elfcpp::R_390_JMP_SLOT:
    case elfcpp::R_390_RELATIVE:
    case elfcpp::R_390_IRELATIVE:
      // These are outstanding tls relocs, which are unexpected when linking
    case elfcpp::R_390_TLS_TPOFF:
    case elfcpp::R_390_TLS_DTPMOD:
    case elfcpp::R_390_TLS_DTPOFF:
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unexpected reloc %u in object file"),
			     r_type);
      break;

    default:
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;
    }

  if (status != S390_relocate_functions<size>::STATUS_OK)
    {
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("relocation overflow"));
    }

  return true;
}

// Perform a TLS relocation.

template<int size>
inline typename elfcpp::Elf_types<size>::Elf_Addr
Target_s390<size>::Relocate::relocate_tls(
    const Relocate_info<size, true>* relinfo,
    Target_s390<size>* target,
    size_t relnum,
    const elfcpp::Rela<size, true>& rela,
    unsigned int r_type,
    const Sized_symbol<size>* gsym,
    const Symbol_value<size>* psymval,
    unsigned char* view,
    section_size_type view_size)
{
  Output_segment* tls_segment = relinfo->layout->tls_segment();

  const Sized_relobj_file<size, true>* object = relinfo->object;
  const elfcpp::Elf_Xword addend = rela.get_r_addend();
  elfcpp::Shdr<size, true> data_shdr(relinfo->data_shdr);
  bool is_allocatable = (data_shdr.get_sh_flags() & elfcpp::SHF_ALLOC) != 0;

  typename elfcpp::Elf_types<size>::Elf_Addr value
      = psymval->value(relinfo->object, addend);

  const bool is_final = (gsym == NULL
			 ? !parameters->options().shared()
			 : gsym->final_value_is_known());
  tls::Tls_optimization optimized_type
      = Target_s390<size>::optimize_tls_reloc(is_final, r_type);
  switch (r_type)
    {
    case elfcpp::R_390_TLS_GDCALL:            // Global-dynamic marker
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return 0;
	    }
	  this->tls_gd_to_le(relinfo, relnum, rela, view, view_size);
	  break;
	}
      else
	{
	  if (optimized_type == tls::TLSOPT_TO_IE)
	    {
	      this->tls_gd_to_ie(relinfo, relnum, rela, view, view_size);
	      break;
	    }
	  else if (optimized_type == tls::TLSOPT_NONE)
	    {
	      break;
	    }
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"), r_type);
      break;

    case elfcpp::R_390_TLS_GD32:            // Global-dynamic
    case elfcpp::R_390_TLS_GD64:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return 0;
	    }
	  return value - tls_segment->memsz();
	}
      else
	{
	  unsigned int got_type = (optimized_type == tls::TLSOPT_TO_IE
				   ? GOT_TYPE_TLS_OFFSET
				   : GOT_TYPE_TLS_PAIR);
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(got_type));
	      return (gsym->got_offset(got_type)
		      + target->got_main_offset()
		      + addend);
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      return (object->local_got_offset(r_sym, got_type)
		      + target->got_main_offset()
		      + addend);
	    }
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"), r_type);
      break;

    case elfcpp::R_390_TLS_LDCALL:            // Local-dynamic marker
      // This is a marker relocation. If the sequence is being turned to LE,
      // we modify the instruction, otherwise the instruction is untouched.
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return 0;
	    }
	  this->tls_ld_to_le(relinfo, relnum, rela, view, view_size);
	  break;
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  break;
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"), r_type);
      break;

    case elfcpp::R_390_TLS_LDM32:            // Local-dynamic module
    case elfcpp::R_390_TLS_LDM64:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return 0;
	    }
	  // Doesn't matter what we fill it with - it's going to be unused.
	  return 0;
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the module index.
	  return (target->got_mod_index_entry(NULL, NULL, NULL)
		  + addend
		  + target->got_main_offset());
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"), r_type);
      break;

    case elfcpp::R_390_TLS_LDO32:         // Local-dynamic offset
    case elfcpp::R_390_TLS_LDO64:
      // This relocation type is used in debugging information.
      // In that case we need to not optimize the value.  If the
      // section is not allocatable, then we assume we should not
      // optimize this reloc.
      if (optimized_type == tls::TLSOPT_TO_LE && is_allocatable)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return 0;
	    }
	  value -= tls_segment->memsz();
	}
      return value;

    case elfcpp::R_390_TLS_LOAD:         // Initial-exec marker
      // This is a marker relocation. If the sequence is being turned to LE,
      // we modify the instruction, otherwise the instruction is untouched.
      if (gsym != NULL
	  && gsym->is_undefined()
	  && parameters->options().output_is_executable())
	{
	  Target_s390<size>::Relocate::tls_ie_to_le(relinfo, relnum,
						      rela, view,
						      view_size);
	  break;
	}
      else if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return 0;
	    }
	  Target_s390<size>::Relocate::tls_ie_to_le(relinfo, relnum,
						      rela, view,
						      view_size);
	  break;
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  break;
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc type %u"),
			     r_type);
      break;

    case elfcpp::R_390_TLS_GOTIE12:       // Initial-exec, not optimizable
    case elfcpp::R_390_TLS_GOTIE20:
    case elfcpp::R_390_TLS_IEENT:
    case elfcpp::R_390_TLS_GOTIE32:       // Initial-exec, optimizable
    case elfcpp::R_390_TLS_GOTIE64:
    case elfcpp::R_390_TLS_IE32:
    case elfcpp::R_390_TLS_IE64:
      if (gsym != NULL
	  && gsym->is_undefined()
	  && parameters->options().output_is_executable()
	  // These three cannot be optimized to LE, no matter what
	  && r_type != elfcpp::R_390_TLS_GOTIE12
	  && r_type != elfcpp::R_390_TLS_GOTIE20
	  && r_type != elfcpp::R_390_TLS_IEENT)
	{
          return value;
	}
      else if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return 0;
	    }
          return value - tls_segment->memsz();
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the tp-relative offset of the symbol.
	  unsigned int got_offset;
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(GOT_TYPE_TLS_OFFSET));
	      got_offset = gsym->got_offset(GOT_TYPE_TLS_OFFSET);
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym,
						       GOT_TYPE_TLS_OFFSET));
	      got_offset = object->local_got_offset(r_sym, GOT_TYPE_TLS_OFFSET);
	    }
	  got_offset += target->got_main_offset();
	  if (r_type == elfcpp::R_390_TLS_IE32
	      || r_type == elfcpp::R_390_TLS_IE64)
	    return target->got_address() + got_offset + addend;
	  else
	    return got_offset + addend;
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc type %u"),
			     r_type);
      break;

    case elfcpp::R_390_TLS_LE32:          // Local-exec
    case elfcpp::R_390_TLS_LE64:
      if (tls_segment == NULL)
	{
	  gold_assert(parameters->errors()->error_count() > 0
		      || issue_undefined_symbol_error(gsym));
	  return 0;
	}
      return value - tls_segment->memsz();
    }
  return 0;
}

// Do a relocation in which we convert a TLS General-Dynamic to an
// Initial-Exec.

template<int size>
inline void
Target_s390<size>::Relocate::tls_gd_to_ie(
    const Relocate_info<size, true>* relinfo,
    size_t relnum,
    const elfcpp::Rela<size, true>& rela,
    unsigned char* view,
    section_size_type view_size)
{
  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 4);
  if (view[0] == 0x4d)
    {
      // bas, don't care about details
      // Change to l %r2, 0(%r2, %r12)
      view[0] = 0x58;
      view[1] = 0x22;
      view[2] = 0xc0;
      view[3] = 0x00;
      return;
    }
  else if (view[0] == 0xc0)
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 6);
      // brasl %r14, __tls_get_offset@plt
      if (view[1] == 0xe5)
	{
	  // Change to l/lg %r2, 0(%r2, %r12)
	  // There was a PLT32DBL reloc at the last 4 bytes, overwrite its result.
	  if (size == 32)
	    {
	      // l
	      view[0] = 0x58;
	      view[1] = 0x22;
	      view[2] = 0xc0;
	      view[3] = 0x00;
	      // nop
	      view[4] = 0x07;
	      view[5] = 0x07;
	    }
	  else
	    {
	      // lg
	      view[0] = 0xe3;
	      view[1] = 0x22;
	      view[2] = 0xc0;
	      view[3] = 0;
	      view[4] = 0;
	      view[5] = 0x04;
	    }
	  return;
	}
    }
  gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			 _("unsupported op for GD to IE"));
}

// Do a relocation in which we convert a TLS General-Dynamic to a
// Local-Exec.

template<int size>
inline void
Target_s390<size>::Relocate::tls_gd_to_le(
    const Relocate_info<size, true>* relinfo,
    size_t relnum,
    const elfcpp::Rela<size, true>& rela,
    unsigned char* view,
    section_size_type view_size)
{
  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 2);
  if (view[0] == 0x0d)
    {
      // basr, change to nop
      view[0] = 0x07;
      view[1] = 0x07;
    }
  else if (view[0] == 0x4d)
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 4);
      // bas, don't care about details, change to nop
      view[0] = 0x47;
      view[1] = 0;
      view[2] = 0;
      view[3] = 0;
      return;
    }
  else if (view[0] == 0xc0)
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 6);
      // brasl %r14, __tls_get_offset@plt
      if (view[1] == 0xe5)
	{
	  // Change to nop jump. There was a PLT32DBL reloc at the last
	  // 4 bytes, overwrite its result.
	  view[1] = 0x04;
	  view[2] = 0;
	  view[3] = 0;
	  view[4] = 0;
	  view[5] = 0;
	  return;
	}
    }
  gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			 _("unsupported op for GD to LE"));
}

template<int size>
inline void
Target_s390<size>::Relocate::tls_ld_to_le(
    const Relocate_info<size, true>* relinfo,
    size_t relnum,
    const elfcpp::Rela<size, true>& rela,
    unsigned char* view,
    section_size_type view_size)
{
  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 4);

  if (view[0] == 0x0d)
    {
      // basr, change to nop
      view[0] = 0x07;
      view[1] = 0x07;
    }
  else if (view[0] == 0x4d)
    {
      // bas, don't care about details, change to nop
      view[0] = 0x47;
      view[1] = 0;
      view[2] = 0;
      view[3] = 0;
      return;
    }
  else if (view[0] == 0xc0)
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 6);
      // brasl %r14, __tls_get_offset@plt
      if (view[1] == 0xe5)
	{
	  // Change to nop jump. There was a PLT32DBL reloc at the last
	  // 4 bytes, overwrite its result.
	  view[1] = 0x04;
	  view[2] = 0;
	  view[3] = 0;
	  view[4] = 0;
	  view[5] = 0;
	  return;
	}
    }
  gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			 _("unsupported op for LD to LE"));
}

// Do a relocation in which we convert a TLS Initial-Exec to a
// Local-Exec.

template<int size>
inline void
Target_s390<size>::Relocate::tls_ie_to_le(
    const Relocate_info<size, true>* relinfo,
    size_t relnum,
    const elfcpp::Rela<size, true>& rela,
    unsigned char* view,
    section_size_type view_size)
{
  tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 4);

  if (view[0] == 0x58)
    {
      // l %rX, 0(%rY) or l %rX, 0(%rY, %r12)
      if ((view[2] & 0x0f) != 0 || view[3] != 0)
	goto err;
      int rx = view[1] >> 4 & 0xf;
      int ry = view[1] & 0xf;
      int rz = view[2] >> 4 & 0xf;
      if (rz == 0)
	{
	}
      else if (ry == 0)
	{
	  ry = rz;
	}
      else if (rz == 12)
	{
	}
      else if (ry == 12)
	{
	  ry = rz;
	}
      else
	goto err;
      // to lr %rX, $rY
      view[0] = 0x18;
      view[1] = rx << 4 | ry;
      // and insert a nop
      view[2] = 0x07;
      view[3] = 0x00;
    }
  else if (view[0] == 0xe3)
    {
      tls::check_range(relinfo, relnum, rela.get_r_offset(), view_size, 6);
      // lg %rX, 0(%rY) or lg %rX, 0(%rY, %r12)
      if ((view[2] & 0x0f) != 0 ||
	  view[3] != 0 ||
	  view[4] != 0 ||
	  view[5] != 0x04)
	goto err;
      int rx = view[1] >> 4 & 0xf;
      int ry = view[1] & 0xf;
      int rz = view[2] >> 4 & 0xf;
      if (rz == 0)
	{
	}
      else if (ry == 0)
	{
	  ry = rz;
	}
      else if (rz == 12)
	{
	}
      else if (ry == 12)
	{
	  ry = rz;
	}
      else
	goto err;
      // to sllg %rX, $rY, 0
      view[0] = 0xeb;
      view[1] = rx << 4 | ry;
      view[2] = 0x00;
      view[3] = 0x00;
      view[4] = 0x00;
      view[5] = 0x0d;
    }
  else
    {
err:
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported op for IE to LE"));
    }
}

// Scan relocations for a section.

template<int size>
void
Target_s390<size>::scan_relocs(Symbol_table* symtab,
				 Layout* layout,
				 Sized_relobj_file<size, true>* object,
				 unsigned int data_shndx,
				 unsigned int sh_type,
				 const unsigned char* prelocs,
				 size_t reloc_count,
				 Output_section* output_section,
				 bool needs_special_offset_handling,
				 size_t local_symbol_count,
				 const unsigned char* plocal_symbols)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, true>
      Classify_reloc;

  if (sh_type == elfcpp::SHT_REL)
    {
      gold_error(_("%s: unsupported REL reloc section"),
		 object->name().c_str());
      return;
    }

  gold::scan_relocs<size, true, Target_s390<size>, Scan, Classify_reloc>(
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
Target_s390<size>::do_finalize_sections(
    Layout* layout,
    const Input_objects*,
    Symbol_table* symtab)
{
  const Reloc_section* rel_plt = (this->plt_ == NULL
				  ? NULL
				  : this->plt_->rela_plt());
  layout->add_target_dynamic_tags(false, this->got_plt_, rel_plt,
				  this->rela_dyn_, true, size == 32, false);

  this->layout_ = layout;

  // Emit any relocs we saved in an attempt to avoid generating COPY
  // relocs.
  if (this->copy_relocs_.any_saved_relocs())
    this->copy_relocs_.emit(this->rela_dyn_section(layout));

  // Set the size of the _GLOBAL_OFFSET_TABLE_ symbol to the size of
  // the .got section.
  Symbol* sym = this->global_offset_table_;
  if (sym != NULL)
    {
      uint64_t data_size = this->got_->current_data_size();
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

// Scan the relocs during a relocatable link.

template<int size>
void
Target_s390<size>::scan_relocatable_relocs(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, true>* object,
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
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, true>
      Classify_reloc;
  typedef gold::Default_scan_relocatable_relocs<Classify_reloc>
      Scan_relocatable_relocs;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::scan_relocatable_relocs<size, true, Scan_relocatable_relocs>(
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
Target_s390<size>::emit_relocs_scan(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, true>* object,
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
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, true>
      Classify_reloc;
  typedef gold::Default_emit_relocs_strategy<Classify_reloc>
      Emit_relocs_strategy;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::scan_relocatable_relocs<size, true, Emit_relocs_strategy>(
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
Target_s390<size>::relocate_relocs(
    const Relocate_info<size, true>* relinfo,
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
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, true>
      Classify_reloc;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::relocate_relocs<size, true, Classify_reloc>(
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

// Return the offset to use for the GOT_INDX'th got entry which is
// for a local tls symbol specified by OBJECT, SYMNDX.
template<int size>
int64_t
Target_s390<size>::do_tls_offset_for_local(
    const Relobj*,
    unsigned int,
    Output_data_got_base*,
    unsigned int,
    uint64_t) const
{
  // The only way we can get called is when IEENT/GOTIE12/GOTIE20
  // couldn't be optimised to LE.
  Output_segment* tls_segment = layout_->tls_segment();
  return -tls_segment->memsz();
}

// Return the offset to use for the GOT_INDX'th got entry which is
// for global tls symbol GSYM.
template<int size>
int64_t
Target_s390<size>::do_tls_offset_for_global(
    Symbol*,
    Output_data_got_base*,
    unsigned int,
    uint64_t) const
{
  Output_segment* tls_segment = layout_->tls_segment();
  return -tls_segment->memsz();
}

// Return the value to use for a dynamic which requires special
// treatment.  This is how we support equality comparisons of function
// pointers across shared library boundaries, as described in the
// processor specific ABI supplement.

template<int size>
uint64_t
Target_s390<size>::do_dynsym_value(const Symbol* gsym) const
{
  gold_assert(gsym->is_from_dynobj() && gsym->has_plt_offset());
  return this->plt_address_for_global(gsym);
}

// Return a string used to fill a code section with nops to take up
// the specified length.

template<int size>
std::string
Target_s390<size>::do_code_fill(section_size_type length) const
{
  if (length & 1)
    gold_warning(_("S/390 code fill of odd length requested"));
  return std::string(length, static_cast<char>(0x07));
}

// Return whether SYM should be treated as a call to a non-split
// function.  We don't want that to be true of a larl instruction
// that merely loads its address.

template<int size>
bool
Target_s390<size>::do_is_call_to_non_split(const Symbol* sym,
					   const unsigned char* preloc,
					   const unsigned char* view,
					   section_size_type view_size) const
{
  if (sym->type() != elfcpp::STT_FUNC)
    return false;
  typename Reloc_types<elfcpp::SHT_RELA, size, true>::Reloc reloc(preloc);
  typename elfcpp::Elf_types<size>::Elf_WXword r_info
    = reloc.get_r_info();
  unsigned int r_type = elfcpp::elf_r_type<size>(r_info);
  section_offset_type offset = reloc.get_r_offset();
  switch (r_type)
    {
    // PLT refs always involve calling the function.
    case elfcpp::R_390_PLT12DBL:
    case elfcpp::R_390_PLT16DBL:
    case elfcpp::R_390_PLT24DBL:
    case elfcpp::R_390_PLT32:
    case elfcpp::R_390_PLT32DBL:
    case elfcpp::R_390_PLT64:
    case elfcpp::R_390_PLTOFF16:
    case elfcpp::R_390_PLTOFF32:
    case elfcpp::R_390_PLTOFF64:
    // Could be used for calls for -msmall-exec.
    case elfcpp::R_390_PC16DBL:
      return true;

    // Tricky case.  When used in a brasl, jg, and other branch instructions,
    // it's a call or a sibcall.  However, when used in larl, it only loads
    // the function's address - not a call.
    case elfcpp::R_390_PC32DBL:
      {
	if (offset < 2
	    || offset + 4 > static_cast<section_offset_type>(view_size))
	  {
	    // Should not happen.
	    gold_error(_("instruction with PC32DBL not wholly within section"));
	    return false;
	  }

	uint8_t op0 = view[offset-2];
	uint8_t op1 = view[offset-1] & 0xf;

	// LARL
	if (op0 == 0xc0 && op1 == 0)
	  return false;

	// Otherwise, it's either a call instruction, a branch instruction
	// (used as a sibcall), or a data manipulation instruction (which
	// has no business being used on a function, and can be ignored).
        return true;
      }

    // Otherwise, it's probably not a call.
    default:
      return false;
    }
}

// Code sequences to match below.

template<int size>
const unsigned char
Target_s390<size>::ss_code_bras_8[] = {
  0xa7, 0x15, 0x00, 0x06,		// bras %r1, .+0xc
};

template<int size>
const unsigned char
Target_s390<size>::ss_code_l_basr[] = {
  0x58, 0xe0, 0x10, 0x00,		// l %r14, 0(%r1)
  0x58, 0x10, 0x10, 0x04,		// l %r1, 4(%r1)
  0x0d, 0xee,				// basr %r14, %r14
};

template<int size>
const unsigned char
Target_s390<size>::ss_code_a_basr[] = {
  0x18, 0xe1,				// lr %r14, %r1
  0x5a, 0xe0, 0x10, 0x00,		// a %r14, 0(%r1)
  0x5a, 0x10, 0x10, 0x04,		// a %r1, 4(%r1)
  0x0d, 0xee,				// basr %r14, %r14
};

template<int size>
const unsigned char
Target_s390<size>::ss_code_larl[] = {
  0xc0, 0x10,				// larl %r1, ...
};

template<int size>
const unsigned char
Target_s390<size>::ss_code_brasl[] = {
  0xc0, 0xe5,				// brasl %r14, ...
};

template<int size>
const unsigned char
Target_s390<size>::ss_code_jg[] = {
  0xc0, 0xf4,				// jg ...
};

template<int size>
const unsigned char
Target_s390<size>::ss_code_jgl[] = {
  0xc0, 0x44,				// jgl ...
};

template<>
bool
Target_s390<32>::ss_match_st_r14(unsigned char* view,
				 section_size_type view_size,
				 section_offset_type *offset) const
{
  static const unsigned char ss_code_st_r14[] = {
    0x50, 0xe0, 0xf0, 0x04,		// st %r14, 4(%r15)
  };
  if (!this->match_view_u(view, view_size, *offset, ss_code_st_r14,
			  sizeof ss_code_st_r14))
    return false;
  *offset += sizeof ss_code_st_r14;
  return true;
}

template<>
bool
Target_s390<64>::ss_match_st_r14(unsigned char* view,
				 section_size_type view_size,
				 section_offset_type *offset) const
{
  static const unsigned char ss_code_st_r14[] = {
    0xe3, 0xe0, 0xf0, 0x08, 0x00, 0x24	// stg %r14, 8(%r15)
  };
  if (!this->match_view_u(view, view_size, *offset, ss_code_st_r14,
			  sizeof ss_code_st_r14))
    return false;
  *offset += sizeof ss_code_st_r14;
  return true;
}

template<>
bool
Target_s390<32>::ss_match_l_r14(unsigned char* view,
				section_size_type view_size,
				section_offset_type *offset) const
{
  static const unsigned char ss_code_l_r14[] = {
    0x58, 0xe0, 0xf0, 0x04,		// l %r14, 4(%r15)
  };
  if (!this->match_view_u(view, view_size, *offset, ss_code_l_r14,
			  sizeof ss_code_l_r14))
    return false;
  *offset += sizeof ss_code_l_r14;
  return true;
}

template<>
bool
Target_s390<64>::ss_match_l_r14(unsigned char* view,
				section_size_type view_size,
				section_offset_type *offset) const
{
  static const unsigned char ss_code_l_r14[] = {
    0xe3, 0xe0, 0xf0, 0x08, 0x00, 0x04	// lg %r14, 8(%r15)
  };
  if (!this->match_view_u(view, view_size, *offset, ss_code_l_r14,
			  sizeof ss_code_l_r14))
    return false;
  *offset += sizeof ss_code_l_r14;
  return true;
}

template<int size>
bool
Target_s390<size>::ss_match_mcount(unsigned char* view,
				   section_size_type view_size,
				   section_offset_type *offset) const
{
  // Match the mcount call sequence.
  section_offset_type myoff = *offset;

  // First, look for the store instruction saving %r14.
  if (!this->ss_match_st_r14(view, view_size, &myoff))
    return false;

  // Now, param load and the actual call.
  if (this->match_view_u(view, view_size, myoff, ss_code_larl,
			 sizeof ss_code_larl))
    {
      myoff += sizeof ss_code_larl + 4;

      // After larl, expect a brasl.
      if (!this->match_view_u(view, view_size, myoff, ss_code_brasl,
			      sizeof ss_code_brasl))
	return false;
      myoff += sizeof ss_code_brasl + 4;
    }
  else if (size == 32 &&
	   this->match_view_u(view, view_size, myoff, ss_code_bras_8,
			      sizeof ss_code_bras_8))
    {
      // The bras skips over a block of 8 bytes, loading its address
      // to %r1.
      myoff += sizeof ss_code_bras_8 + 8;

      // Now, there are two sequences used for actual load and call,
      // absolute and PIC.
      if (this->match_view_u(view, view_size, myoff, ss_code_l_basr,
			     sizeof ss_code_l_basr))
        myoff += sizeof ss_code_l_basr;
      else if (this->match_view_u(view, view_size, myoff, ss_code_a_basr,
				  sizeof ss_code_a_basr))
        myoff += sizeof ss_code_a_basr;
      else
	return false;
    }
  else
    return false;

  // Finally, a load bringing %r14 back.
  if (!this->ss_match_l_r14(view, view_size, &myoff))
    return false;

  // Found it.
  *offset = myoff;
  return true;
}

template<>
bool
Target_s390<32>::ss_match_ear(unsigned char* view,
				section_size_type view_size,
				section_offset_type *offset) const
{
  static const unsigned char ss_code_ear[] = {
    0xb2, 0x4f, 0x00, 0x10,		// ear %r1, %a0
  };
  if (!this->match_view_u(view, view_size, *offset, ss_code_ear,
			  sizeof ss_code_ear))
    return false;
  *offset += sizeof ss_code_ear;
  return true;
}

template<>
bool
Target_s390<64>::ss_match_ear(unsigned char* view,
				section_size_type view_size,
				section_offset_type *offset) const
{
  static const unsigned char ss_code_ear[] = {
    0xb2, 0x4f, 0x00, 0x10,		// ear %r1, %a0
    0xeb, 0x11, 0x00, 0x20, 0x00, 0x0d,	// sllg %r1,%r1,32
    0xb2, 0x4f, 0x00, 0x11,		// ear %r1, %a1
  };
  if (!this->match_view_u(view, view_size, *offset, ss_code_ear,
			  sizeof ss_code_ear))
    return false;
  *offset += sizeof ss_code_ear;
  return true;
}

template<>
bool
Target_s390<32>::ss_match_c(unsigned char* view,
				section_size_type view_size,
				section_offset_type *offset) const
{
  static const unsigned char ss_code_c[] = {
    0x59, 0xf0, 0x10, 0x20,		// c %r15, 0x20(%r1)
  };
  if (!this->match_view_u(view, view_size, *offset, ss_code_c,
			  sizeof ss_code_c))
    return false;
  *offset += sizeof ss_code_c;
  return true;
}

template<>
bool
Target_s390<64>::ss_match_c(unsigned char* view,
				section_size_type view_size,
				section_offset_type *offset) const
{
  static const unsigned char ss_code_c[] = {
    0xe3, 0xf0, 0x10, 0x38, 0x00, 0x20,	// cg %r15, 0x38(%r1)
  };
  if (!this->match_view_u(view, view_size, *offset, ss_code_c,
			  sizeof ss_code_c))
    return false;
  *offset += sizeof ss_code_c;
  return true;
}

template<>
bool
Target_s390<32>::ss_match_l(unsigned char* view,
			    section_size_type view_size,
			    section_offset_type *offset,
			    int *guard_reg) const
{
  // l %guard_reg, 0x20(%r1)
  if (convert_to_section_size_type(*offset + 4) > view_size
      || view[*offset] != 0x58
      || (view[*offset + 1] & 0xf) != 0x0
      || view[*offset + 2] != 0x10
      || view[*offset + 3] != 0x20)
    return false;
  *offset += 4;
  *guard_reg = view[*offset + 1] >> 4 & 0xf;
  return true;
}

template<>
bool
Target_s390<64>::ss_match_l(unsigned char* view,
			    section_size_type view_size,
			    section_offset_type *offset,
			    int *guard_reg) const
{
  // lg %guard_reg, 0x38(%r1)
  if (convert_to_section_size_type(*offset + 6) > view_size
      || view[*offset] != 0xe3
      || (view[*offset + 1] & 0xf) != 0x0
      || view[*offset + 2] != 0x10
      || view[*offset + 3] != 0x38
      || view[*offset + 4] != 0x00
      || view[*offset + 5] != 0x04)
    return false;
  *offset += 6;
  *guard_reg = view[*offset + 1] >> 4 & 0xf;
  return true;
}

template<int size>
bool
Target_s390<size>::ss_match_ahi(unsigned char* view,
				section_size_type view_size,
				section_offset_type *offset,
				int guard_reg,
				uint32_t *arg) const
{
  int op = size == 32 ? 0xa : 0xb;
  // a[g]hi %guard_reg, <arg>
  if (convert_to_section_size_type(*offset + 4) > view_size
      || view[*offset] != 0xa7
      || view[*offset + 1] != (guard_reg << 4 | op)
      // Disallow negative size.
      || view[*offset + 2] & 0x80)
    return false;
  *arg = elfcpp::Swap<16, true>::readval(view + *offset + 2);
  *offset += 4;
  return true;
}

template<int size>
bool
Target_s390<size>::ss_match_alfi(unsigned char* view,
				 section_size_type view_size,
				 section_offset_type *offset,
				 int guard_reg,
				 uint32_t *arg) const
{
  int op = size == 32 ? 0xb : 0xa;
  // al[g]fi %guard_reg, <arg>
  if (convert_to_section_size_type(*offset + 6) > view_size
      || view[*offset] != 0xc2
      || view[*offset + 1] != (guard_reg << 4 | op))
    return false;
  *arg = elfcpp::Swap<32, true>::readval(view + *offset + 2);
  *offset += 6;
  return true;
}

template<>
bool
Target_s390<32>::ss_match_cr(unsigned char* view,
			     section_size_type view_size,
			     section_offset_type *offset,
			     int guard_reg) const
{
  // cr %r15, %guard_reg
  if (convert_to_section_size_type(*offset + 2) > view_size
      || view[*offset] != 0x19
      || view[*offset + 1] != (0xf0 | guard_reg))
    return false;
  *offset += 2;
  return true;
}

template<>
bool
Target_s390<64>::ss_match_cr(unsigned char* view,
			     section_size_type view_size,
			     section_offset_type *offset,
			     int guard_reg) const
{
  // cgr %r15, %guard_reg
  if (convert_to_section_size_type(*offset + 4) > view_size
      || view[*offset] != 0xb9
      || view[*offset + 1] != 0x20
      || view[*offset + 2] != 0x00
      || view[*offset + 3] != (0xf0 | guard_reg))
    return false;
  *offset += 4;
  return true;
}


// FNOFFSET in section SHNDX in OBJECT is the start of a function
// compiled with -fsplit-stack.  The function calls non-split-stack
// code.  We have to change the function so that it always ensures
// that it has enough stack space to run some random function.

template<int size>
void
Target_s390<size>::do_calls_non_split(Relobj* object, unsigned int shndx,
				      section_offset_type fnoffset,
				      section_size_type,
				      const unsigned char *prelocs,
				      size_t reloc_count,
				      unsigned char* view,
				      section_size_type view_size,
				      std::string*,
				      std::string*) const
{
  // true if there's a conditional call to __morestack in the function,
  // false if there's an unconditional one.
  bool conditional = false;
  // Offset of the byte after the compare insn, if conditional.
  section_offset_type cmpend = 0;
  // Type and immediate offset of the add instruction that adds frame size
  // to guard.
  enum {
    SS_ADD_NONE,
    SS_ADD_AHI,
    SS_ADD_ALFI,
  } fsadd_type = SS_ADD_NONE;
  section_offset_type fsadd_offset = 0;
  uint32_t fsadd_frame_size = 0;
  // Register used for loading guard.  Usually r1, but can also be r0 or r2-r5.
  int guard_reg;
  // Offset of the conditional jump.
  section_offset_type jump_offset = 0;
  // Section view and offset of param block.
  section_offset_type param_offset = 0;
  unsigned char *param_view = 0;
  section_size_type param_view_size = 0;
  // Current position in function.
  section_offset_type curoffset = fnoffset;
  // And the position of split-stack prologue.
  section_offset_type ssoffset;
  // Frame size.
  typename elfcpp::Elf_types<size>::Elf_Addr frame_size;
  // Relocation parsing.
  typedef typename Reloc_types<elfcpp::SHT_RELA, size, true>::Reloc Reltype;
  const int reloc_size = Reloc_types<elfcpp::SHT_RELA, size, true>::reloc_size;
  const unsigned char *pr = prelocs;

  // If the function was compiled with -pg, the profiling code may come before
  // the split-stack prologue.  Skip it.

  this->ss_match_mcount(view, view_size, &curoffset);
  ssoffset = curoffset;

  // First, figure out if there's a conditional call by looking for the
  // extract-tp, add, cmp sequence.

  if (this->ss_match_ear(view, view_size, &curoffset))
    {
      // Found extract-tp, now look for an add and compare.
      conditional = true;
      if (this->ss_match_c(view, view_size, &curoffset))
	{
	  // Found a direct compare of stack pointer with the guard,
	  // we're done here.
	}
      else if (this->ss_match_l(view, view_size, &curoffset, &guard_reg))
	{
	  // Found a load of guard to register, look for an add and compare.
          if (this->ss_match_ahi(view, view_size, &curoffset, guard_reg,
				 &fsadd_frame_size))
	    {
	      fsadd_type = SS_ADD_AHI;
	      fsadd_offset = curoffset - 2;
	    }
	  else if (this->ss_match_alfi(view, view_size, &curoffset, guard_reg,
				       &fsadd_frame_size))
	    {
	      fsadd_type = SS_ADD_ALFI;
	      fsadd_offset = curoffset - 4;
	    }
	  else
            {
	      goto bad;
            }
	  // Now, there has to be a compare.
          if (!this->ss_match_cr(view, view_size, &curoffset, guard_reg))
	    goto bad;
	}
      else
        {
	  goto bad;
        }
      cmpend = curoffset;
    }

  // Second, look for the call.
  if (!this->match_view_u(view, view_size, curoffset, ss_code_larl,
			  sizeof ss_code_larl))
    goto bad;
  curoffset += sizeof ss_code_larl;

  // Find out larl's operand.  It should be a local symbol in .rodata
  // section.
  for (size_t i = 0; i < reloc_count; ++i, pr += reloc_size)
    {
      Reltype reloc(pr);
      if (static_cast<section_offset_type>(reloc.get_r_offset())
          == curoffset)
        {
          typename elfcpp::Elf_types<size>::Elf_WXword r_info
            = reloc.get_r_info();
          unsigned int r_sym = elfcpp::elf_r_sym<size>(r_info);
          unsigned int r_type = elfcpp::elf_r_type<size>(r_info);
          if (r_type != elfcpp::R_390_PC32DBL)
            goto bad;
          if (r_sym >= object->local_symbol_count())
            goto bad;
          Sized_relobj_file<size, true> *object_sized =
            static_cast<Sized_relobj_file<size, true> *>(object);
          const Symbol_value<size>* sym = object_sized->local_symbol(r_sym);
          bool param_shndx_ordinary;
          const unsigned int param_shndx =
            sym->input_shndx(&param_shndx_ordinary);
          if (!param_shndx_ordinary)
            goto bad;
          param_offset = sym->input_value() + reloc.get_r_addend() - 2
                         - object->output_section(param_shndx)->address()
                         - object->output_section_offset(param_shndx);
          param_view = object->get_output_view(param_shndx,
                                                  &param_view_size);
          break;
        }
    }

  if (!param_view)
    goto bad;

  curoffset += 4;

  // Now, there has to be a jump to __morestack.
  jump_offset = curoffset;

  if (this->match_view_u(view, view_size, curoffset,
                       conditional ? ss_code_jgl : ss_code_jg,
                       sizeof ss_code_jg))
    curoffset += sizeof ss_code_jg;
  else
    goto bad;

  curoffset += 4;

  // Read the frame size.
  if (convert_to_section_size_type(param_offset + size / 8) > param_view_size)
    goto bad;
  frame_size = elfcpp::Swap<size, true>::readval(param_view + param_offset);

  // Sanity check.
  if (fsadd_type != SS_ADD_NONE && fsadd_frame_size != frame_size)
    goto bad;

  // Bump the frame size.
  frame_size += parameters->options().split_stack_adjust_size();

  // Store it to the param block.
  elfcpp::Swap<size, true>::writeval(param_view + param_offset, frame_size);

  if (!conditional)
    {
      // If the call was already unconditional, we're done.
    }
  else if (frame_size <= 0xffffffff && fsadd_type == SS_ADD_ALFI)
    {
      // Using alfi to add the frame size, and it still fits.  Adjust it.
      elfcpp::Swap_unaligned<32, true>::writeval(view + fsadd_offset,
						 frame_size);
    }
  else
    {
      // We were either relying on the backoff area, or used ahi to load
      // frame size.  This won't fly, as our new frame size is too large.
      // Convert the sequence to unconditional by nopping out the comparison,
      // and rewiring the jump.
      this->set_view_to_nop(view, view_size, ssoffset, cmpend - ssoffset);

      // The jump is jgl, we'll mutate it to jg.
      view[jump_offset+1] = 0xf4;
    }

  return;

bad:
  if (!object->has_no_split_stack())
      object->error(_("failed to match split-stack sequence at "
		      "section %u offset %0zx"),
		    shndx, static_cast<size_t>(fnoffset));
}

// Relocate section data.

template<int size>
void
Target_s390<size>::relocate_section(
    const Relocate_info<size, true>* relinfo,
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
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, true>
      Classify_reloc;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::relocate_section<size, true, Target_s390<size>, Relocate,
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
Target_s390<size>::apply_relocation(
    const Relocate_info<size, true>* relinfo,
    typename elfcpp::Elf_types<size>::Elf_Addr r_offset,
    unsigned int r_type,
    typename elfcpp::Elf_types<size>::Elf_Swxword r_addend,
    const Symbol* gsym,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size)
{
  gold::apply_relocation<size, true, Target_s390<size>,
			 typename Target_s390<size>::Relocate>(
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

// The selector for s390 object files.

template<int size>
class Target_selector_s390 : public Target_selector
{
public:
  Target_selector_s390()
    : Target_selector(elfcpp::EM_S390, size, true,
		      (size == 64 ? "elf64-s390" : "elf32-s390"),
		      (size == 64 ? "elf64_s390" : "elf32_s390"))
  { }

  virtual Target*
  do_instantiate_target()
  { return new Target_s390<size>(); }
};

Target_selector_s390<32> target_selector_s390;
Target_selector_s390<64> target_selector_s390x;

} // End anonymous namespace.
