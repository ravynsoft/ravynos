// tilegx.cc -- tilegx target support for gold.

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
// Written by Jiong Wang (jiwang@tilera.com)

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
#include "tilegx.h"
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

// the first got entry reserved
const int32_t TILEGX_GOT_RESERVE_COUNT = 1;

// the first two .got.plt entry reserved
const int32_t TILEGX_GOTPLT_RESERVE_COUNT = 2;

// 1. for both 64/32 bit mode, the instruction bundle is always 64bit.
// 2. thus .plt section should always be aligned to 64 bit.
const int32_t TILEGX_INST_BUNDLE_SIZE = 64;

namespace
{

using namespace gold;

// A class to handle the PLT data.
// This is an abstract base class that handles most of the linker details
// but does not know the actual contents of PLT entries.  The derived
// classes below fill in those details.

template<int size, bool big_endian>
class Output_data_plt_tilegx : public Output_section_data
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_RELA, true,size, big_endian>
    Reloc_section;

  Output_data_plt_tilegx(Layout* layout, uint64_t addralign,
                         Output_data_got<size, big_endian>* got,
                         Output_data_space* got_plt,
                         Output_data_space* got_irelative)
    : Output_section_data(addralign), layout_(layout),
      irelative_rel_(NULL), got_(got), got_plt_(got_plt),
      got_irelative_(got_irelative), count_(0),
      irelative_count_(0), free_list_()
  { this->init(layout); }

  Output_data_plt_tilegx(Layout* layout, uint64_t plt_entry_size,
                         Output_data_got<size, big_endian>* got,
                         Output_data_space* got_plt,
                         Output_data_space* got_irelative,
                         unsigned int plt_count)
    : Output_section_data((plt_count + 1) * plt_entry_size,
                          TILEGX_INST_BUNDLE_SIZE, false),
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
    Sized_relobj_file<size, big_endian>*, unsigned int);

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
  { return this->get_plt_entry_size(); }

  // Return the size of a PLT entry.
  unsigned int
  get_plt_entry_size() const
  { return plt_entry_size; }

  // Reserve a slot in the PLT for an existing symbol in an incremental update.
  void
  reserve_slot(unsigned int plt_index)
  {
    this->free_list_.remove((plt_index + 1) * this->get_plt_entry_size(),
                            (plt_index + 2) * this->get_plt_entry_size());
  }

  // Return the PLT address to use for a global symbol.
  uint64_t
  address_for_global(const Symbol*);

  // Return the PLT address to use for a local symbol.
  uint64_t
  address_for_local(const Relobj*, unsigned int symndx);

 protected:
  // Fill in the first PLT entry.
  void
  fill_first_plt_entry(unsigned char*);

  // Fill in a normal PLT entry.  Returns the offset into the entry that
  // should be the initial GOT slot value.
  void
  fill_plt_entry(unsigned char*,
                 typename elfcpp::Elf_types<size>::Elf_Addr,
                 unsigned int,
                 typename elfcpp::Elf_types<size>::Elf_Addr,
                 unsigned int, unsigned int);

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
  Output_data_got<size, big_endian>* got_;
  // The .got.plt section.
  Output_data_space* got_plt_;
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
  static const int plt_entry_size = 40;
  // The first entry in the PLT.
  static const unsigned char first_plt_entry[plt_entry_size];
  // Other entries in the PLT for an executable.
  static const unsigned char plt_entry[plt_entry_size];
};

// The tilegx target class.
// See the ABI at
//   http://www.tilera.com/scm
// TLS info comes from
//   http://people.redhat.com/drepper/tls.pdf

template<int size, bool big_endian>
class Target_tilegx : public Sized_target<size, big_endian>
{
 public:
  // TileGX use RELA
  typedef Output_data_reloc<elfcpp::SHT_RELA, true, size, big_endian>
    Reloc_section;

  Target_tilegx(const Target::Target_info* info = &tilegx_info)
    : Sized_target<size, big_endian>(info),
      got_(NULL), plt_(NULL), got_plt_(NULL), got_irelative_(NULL),
      global_offset_table_(NULL), tilegx_dynamic_(NULL), rela_dyn_(NULL),
      rela_irelative_(NULL), copy_relocs_(elfcpp::R_TILEGX_COPY),
      got_mod_index_offset_(-1U),
      tls_get_addr_sym_defined_(false)
  { }

  // Scan the relocations to look for symbol adjustments.
  void
  gc_process_relocs(Symbol_table* symtab,
                    Layout* layout,
                    Sized_relobj_file<size, big_endian>* object,
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
              Sized_relobj_file<size, big_endian>* object,
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
  relocate_section(const Relocate_info<size, big_endian>*,
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
                          Sized_relobj_file<size, big_endian>* object,
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
		   Sized_relobj_file<size, big_endian>* object,
		   unsigned int data_shndx,
		   unsigned int sh_type,
		   const unsigned char* prelocs,
		   size_t reloc_count,
		   Output_section* output_section,
		   bool needs_special_offset_handling,
		   size_t local_symbol_count,
		   const unsigned char* plocal_syms,
		   Relocatable_relocs* rr);

  // Relocate a section during a relocatable link.
  void
  relocate_relocs(
      const Relocate_info<size, big_endian>*,
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
  { return strcmp(sym->name(), "__tls_get_addr") == 0; }

  // define tilegx specific symbols
  virtual void
  do_define_standard_symbols(Symbol_table*, Layout*);

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
  // fold those functions whose pointer is defintely not taken.  For tilegx
  // pie binaries, safe ICF cannot be done by looking at relocation types.
  bool
  do_can_check_for_function_pointers() const
  { return true; }

  // Return the base for a DW_EH_PE_datarel encoding.
  uint64_t
  do_ehframe_datarel_base() const;

  // Return whether there is a GOT section.
  bool
  has_got_section() const
  { return this->got_ != NULL; }

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
                          Sized_relobj<size, big_endian>* obj,
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
  apply_relocation(const Relocate_info<size, big_endian>* relinfo,
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
    local(Symbol_table* symtab, Layout* layout, Target_tilegx* target,
          Sized_relobj_file<size, big_endian>* object,
          unsigned int data_shndx,
          Output_section* output_section,
          const elfcpp::Rela<size, big_endian>& reloc, unsigned int r_type,
          const elfcpp::Sym<size, big_endian>& lsym,
          bool is_discarded);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_tilegx* target,
           Sized_relobj_file<size, big_endian>* object,
           unsigned int data_shndx,
           Output_section* output_section,
           const elfcpp::Rela<size, big_endian>& reloc, unsigned int r_type,
           Symbol* gsym);

    inline bool
    local_reloc_may_be_function_pointer(Symbol_table* symtab, Layout* layout,
                            Target_tilegx* target,
                            Sized_relobj_file<size, big_endian>* object,
                            unsigned int data_shndx,
                            Output_section* output_section,
                            const elfcpp::Rela<size, big_endian>& reloc,
                            unsigned int r_type,
                            const elfcpp::Sym<size, big_endian>& lsym);

    inline bool
    global_reloc_may_be_function_pointer(Symbol_table* symtab, Layout* layout,
                            Target_tilegx* target,
                            Sized_relobj_file<size, big_endian>* object,
                            unsigned int data_shndx,
                            Output_section* output_section,
                            const elfcpp::Rela<size, big_endian>& reloc,
                            unsigned int r_type,
                            Symbol* gsym);

  private:
    static void
    unsupported_reloc_local(Sized_relobj_file<size, big_endian>*,
                            unsigned int r_type);

    static void
    unsupported_reloc_global(Sized_relobj_file<size, big_endian>*,
                             unsigned int r_type, Symbol*);

    void
    check_non_pic(Relobj*, unsigned int r_type);

    inline bool
    possible_function_pointer_reloc(unsigned int r_type);

    bool
    reloc_needs_plt_for_ifunc(Sized_relobj_file<size, big_endian>*,
                              unsigned int r_type);

    // Whether we have issued an error about a non-PIC compilation.
    bool issued_non_pic_error_;
  };

  // The class which implements relocation.
  class Relocate
  {
   public:
    Relocate()
    { }

    ~Relocate()
    {
    }

    // Do a relocation.  Return false if the caller should not issue
    // any warnings about this relocation.
    inline bool
    relocate(const Relocate_info<size, big_endian>*, unsigned int,
	     Target_tilegx*, Output_section*, size_t, const unsigned char*,
	     const Sized_symbol<size>*, const Symbol_value<size>*,
	     unsigned char*, typename elfcpp::Elf_types<size>::Elf_Addr,
	     section_size_type);
  };

  // Adjust TLS relocation type based on the options and whether this
  // is a local symbol.
  static tls::Tls_optimization
  optimize_tls_reloc(bool is_final, int r_type);

  // Get the GOT section, creating it if necessary.
  Output_data_got<size, big_endian>*
  got_section(Symbol_table*, Layout*);

  // Get the GOT PLT section.
  Output_data_space*
  got_plt_section() const
  {
    gold_assert(this->got_plt_ != NULL);
    return this->got_plt_;
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
                             Sized_relobj_file<size, big_endian>* relobj,
                             unsigned int local_sym_index);

  // Create a GOT entry for the TLS module index.
  unsigned int
  got_mod_index_entry(Symbol_table* symtab, Layout* layout,
                      Sized_relobj_file<size, big_endian>* object);

  // Get the PLT section.
  Output_data_plt_tilegx<size, big_endian>*
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
             Sized_relobj_file<size, big_endian>* object,
             unsigned int shndx, Output_section* output_section,
             Symbol* sym, const elfcpp::Rela<size, big_endian>& reloc)
  {
    unsigned int r_type = elfcpp::elf_r_type<size>(reloc.get_r_info());
    this->copy_relocs_.copy_reloc(symtab, layout,
                                  symtab->get_sized_symbol<size>(sym),
                                  object, shndx, output_section,
				  r_type, reloc.get_r_offset(),
				  reloc.get_r_addend(),
                                  this->rela_dyn_section(layout));
  }

  // Information about this specific target which we pass to the
  // general Target structure.
  static const Target::Target_info tilegx_info;

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
    Tlsdesc_info(Sized_relobj_file<size, big_endian>* a_object,
                 unsigned int a_r_sym)
      : object(a_object), r_sym(a_r_sym)
    { }

    // The object in which the local symbol is defined.
    Sized_relobj_file<size, big_endian>* object;
    // The local symbol index in the object.
    unsigned int r_sym;
  };

  // The GOT section.
  Output_data_got<size, big_endian>* got_;
  // The PLT section.
  Output_data_plt_tilegx<size, big_endian>* plt_;
  // The GOT PLT section.
  Output_data_space* got_plt_;
  // The GOT section for IRELATIVE relocations.
  Output_data_space* got_irelative_;
  // The _GLOBAL_OFFSET_TABLE_ symbol.
  Symbol* global_offset_table_;
  // The _TILEGX_DYNAMIC_ symbol.
  Symbol* tilegx_dynamic_;
  // The dynamic reloc section.
  Reloc_section* rela_dyn_;
  // The section to use for IRELATIVE relocs.
  Reloc_section* rela_irelative_;
  // Relocs saved to avoid a COPY reloc.
  Copy_relocs<elfcpp::SHT_RELA, size, big_endian> copy_relocs_;
  // Offset of the GOT entry for the TLS module index.
  unsigned int got_mod_index_offset_;
  // True if the _tls_get_addr symbol has been defined.
  bool tls_get_addr_sym_defined_;
};

template<>
const Target::Target_info Target_tilegx<64, false>::tilegx_info =
{
  64,                   // size
  false,                // is_big_endian
  elfcpp::EM_TILEGX,    // machine_code
  false,                // has_make_symbol
  false,                // has_resolve
  false,                // has_code_fill
  true,                 // is_default_stack_executable
  false,                // can_icf_inline_merge_sections
  '\0',                 // wrap_char
  "/lib/ld.so.1",       // program interpreter
  0x10000,              // default_text_segment_address
  0x10000,              // abi_pagesize (overridable by -z max-page-size)
  0x10000,              // common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,    // small_common_shndx
  elfcpp::SHN_UNDEF,    // large_common_shndx
  0,                    // small_common_section_flags
  0,                    // large_common_section_flags
  NULL,                 // attributes_section
  NULL,                 // attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

template<>
const Target::Target_info Target_tilegx<32, false>::tilegx_info =
{
  32,                   // size
  false,                // is_big_endian
  elfcpp::EM_TILEGX,    // machine_code
  false,                // has_make_symbol
  false,                // has_resolve
  false,                // has_code_fill
  true,                 // is_default_stack_executable
  false,                // can_icf_inline_merge_sections
  '\0',                 // wrap_char
  "/lib32/ld.so.1",     // program interpreter
  0x10000,              // default_text_segment_address
  0x10000,              // abi_pagesize (overridable by -z max-page-size)
  0x10000,              // common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,    // small_common_shndx
  elfcpp::SHN_UNDEF,    // large_common_shndx
  0,                    // small_common_section_flags
  0,                    // large_common_section_flags
  NULL,                 // attributes_section
  NULL,                 // attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

template<>
const Target::Target_info Target_tilegx<64, true>::tilegx_info =
{
  64,                   // size
  true,                 // is_big_endian
  elfcpp::EM_TILEGX,    // machine_code
  false,                // has_make_symbol
  false,                // has_resolve
  false,                // has_code_fill
  true,                 // is_default_stack_executable
  false,                // can_icf_inline_merge_sections
  '\0',                 // wrap_char
  "/lib/ld.so.1",       // program interpreter
  0x10000,              // default_text_segment_address
  0x10000,              // abi_pagesize (overridable by -z max-page-size)
  0x10000,              // common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,    // small_common_shndx
  elfcpp::SHN_UNDEF,    // large_common_shndx
  0,                    // small_common_section_flags
  0,                    // large_common_section_flags
  NULL,                 // attributes_section
  NULL,                 // attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

template<>
const Target::Target_info Target_tilegx<32, true>::tilegx_info =
{
  32,                   // size
  true,                 // is_big_endian
  elfcpp::EM_TILEGX,    // machine_code
  false,                // has_make_symbol
  false,                // has_resolve
  false,                // has_code_fill
  true,                 // is_default_stack_executable
  false,                // can_icf_inline_merge_sections
  '\0',                 // wrap_char
  "/lib32/ld.so.1",     // program interpreter
  0x10000,              // default_text_segment_address
  0x10000,              // abi_pagesize (overridable by -z max-page-size)
  0x10000,              // common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,    // small_common_shndx
  elfcpp::SHN_UNDEF,    // large_common_shndx
  0,                    // small_common_section_flags
  0,                    // large_common_section_flags
  NULL,                 // attributes_section
  NULL,                  // attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

// tilegx relocation handlers
template<int size, bool big_endian>
class Tilegx_relocate_functions
{
public:
  // overflow check will be supported later
  typedef enum
  {
    STATUS_OKAY,        // No error during relocation.
    STATUS_OVERFLOW,    // Relocation overflow.
    STATUS_BAD_RELOC    // Relocation cannot be applied.
  } Status;

  struct Tilegx_howto
  {
    // right shift operand by this number of bits.
    unsigned char srshift;

    // the offset to apply relocation.
    unsigned char doffset;

    // set to 1 for pc-relative relocation.
    unsigned char is_pcrel;

    // size in bits, or 0 if this table entry should be ignored.
    unsigned char bsize;

    // whether we need to check overflow.
    unsigned char overflow;
  };

  static const Tilegx_howto howto[elfcpp::R_TILEGX_NUM];

private:

  // Do a simple rela relocation
  template<int valsize>
  static inline void
  rela(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Swap<size, big_endian>::Valtype addend,
       elfcpp::Elf_Xword srshift, elfcpp::Elf_Xword doffset,
       elfcpp::Elf_Xword bitmask)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Valtype reloc = 0;
    if (size == 32)
      reloc = Bits<32>::sign_extend(psymval->value(object, addend)) >> srshift;
    else
      reloc = psymval->value(object, addend) >> srshift;

    elfcpp::Elf_Xword dst_mask = bitmask << doffset;

    val &= ~dst_mask;
    reloc &= bitmask;

    elfcpp::Swap<valsize, big_endian>::writeval(wv, val | (reloc<<doffset));
  }

  // Do a simple rela relocation
  template<int valsize>
  static inline void
  rela_ua(unsigned char* view,
          const Sized_relobj_file<size, big_endian>* object,
          const Symbol_value<size>* psymval,
          typename elfcpp::Swap<size, big_endian>::Valtype addend,
          elfcpp::Elf_Xword srshift, elfcpp::Elf_Xword doffset,
          elfcpp::Elf_Xword bitmask)
  {
    typedef typename elfcpp::Swap_unaligned<valsize, big_endian>::Valtype
      Valtype;
    unsigned char* wv = view;
    Valtype val = elfcpp::Swap_unaligned<valsize, big_endian>::readval(wv);
    Valtype reloc = 0;
    if (size == 32)
      reloc = Bits<32>::sign_extend(psymval->value(object, addend)) >> srshift;
    else
      reloc = psymval->value(object, addend) >> srshift;

    elfcpp::Elf_Xword dst_mask = bitmask << doffset;

    val &= ~dst_mask;
    reloc &= bitmask;

    elfcpp::Swap_unaligned<valsize, big_endian>::writeval(wv,
      val | (reloc<<doffset));
  }

  template<int valsize>
  static inline void
  rela(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Swap<size, big_endian>::Valtype addend,
       elfcpp::Elf_Xword srshift, elfcpp::Elf_Xword doffset1,
       elfcpp::Elf_Xword bitmask1, elfcpp::Elf_Xword doffset2,
       elfcpp::Elf_Xword bitmask2)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Valtype reloc = 0;
    if (size == 32)
      reloc = Bits<32>::sign_extend(psymval->value(object, addend)) >> srshift;
    else
      reloc = psymval->value(object, addend) >> srshift;

    elfcpp::Elf_Xword dst_mask = (bitmask1 << doffset1)
                                  | (bitmask2 << doffset2);
    val &= ~dst_mask;
    reloc = ((reloc & bitmask1) << doffset1)
             | ((reloc & bitmask2) << doffset2);

    elfcpp::Swap<valsize, big_endian>::writeval(wv, val | reloc);

  }

  // Do a simple PC relative relocation with a Symbol_value with the
  // addend in the relocation.
  template<int valsize>
  static inline void
  pcrela(unsigned char* view,
         const Sized_relobj_file<size, big_endian>* object,
         const Symbol_value<size>* psymval,
         typename elfcpp::Swap<size, big_endian>::Valtype addend,
         typename elfcpp::Elf_types<size>::Elf_Addr address,
         elfcpp::Elf_Xword srshift, elfcpp::Elf_Xword doffset,
         elfcpp::Elf_Xword bitmask)

  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Valtype reloc = 0;
    if (size == 32)
      reloc = Bits<32>::sign_extend(psymval->value(object, addend) - address)
               >> srshift;
    else
      reloc = (psymval->value(object, addend) - address) >> srshift;

    elfcpp::Elf_Xword dst_mask = bitmask << doffset;
    val &= ~dst_mask;
    reloc &= bitmask;

    elfcpp::Swap<valsize, big_endian>::writeval(wv, val | (reloc<<doffset));
  }

  template<int valsize>
  static inline void
  pcrela_ua(unsigned char* view,
           const Sized_relobj_file<size, big_endian>* object,
           const Symbol_value<size>* psymval,
           typename elfcpp::Swap<size, big_endian>::Valtype addend,
           typename elfcpp::Elf_types<size>::Elf_Addr address,
           elfcpp::Elf_Xword srshift, elfcpp::Elf_Xword doffset,
           elfcpp::Elf_Xword bitmask)

  {
    typedef typename elfcpp::Swap_unaligned<valsize, big_endian>::Valtype
      Valtype;
    unsigned char* wv = view;
    Valtype reloc = 0;
    if (size == 32)
      reloc = Bits<32>::sign_extend(psymval->value(object, addend) - address)
               >> srshift;
    else
      reloc = (psymval->value(object, addend) - address) >> srshift;

    reloc &= bitmask;

    elfcpp::Swap<valsize, big_endian>::writeval(wv, reloc << doffset);
  }

  template<int valsize>
  static inline void
  pcrela(unsigned char* view,
         const Sized_relobj_file<size, big_endian>* object,
         const Symbol_value<size>* psymval,
         typename elfcpp::Swap<size, big_endian>::Valtype addend,
         typename elfcpp::Elf_types<size>::Elf_Addr address,
         elfcpp::Elf_Xword srshift, elfcpp::Elf_Xword doffset1,
         elfcpp::Elf_Xword bitmask1, elfcpp::Elf_Xword doffset2,
         elfcpp::Elf_Xword bitmask2)

  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Valtype reloc = 0;
    if (size == 32)
      reloc = Bits<32>::sign_extend(psymval->value(object, addend) - address)
               >> srshift;
    else
      reloc = (psymval->value(object, addend) - address) >> srshift;

    elfcpp::Elf_Xword dst_mask = (bitmask1 << doffset1)
                                  | (bitmask2 << doffset2);
    val &= ~dst_mask;
    reloc = ((reloc & bitmask1) << doffset1)
             | ((reloc & bitmask2) << doffset2);

    elfcpp::Swap<valsize, big_endian>::writeval(wv, val | reloc);
  }

  typedef Tilegx_relocate_functions<size, big_endian> This;
  typedef Relocate_functions<size, big_endian> Base;

public:

  static inline void
  abs64(unsigned char* view,
        const Sized_relobj_file<size, big_endian>* object,
        const Symbol_value<size>* psymval,
        typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This::template rela_ua<64>(view, object, psymval, addend, 0, 0,
                               0xffffffffffffffffllu);
  }

  static inline void
  abs32(unsigned char* view,
        const Sized_relobj_file<size, big_endian>* object,
        const Symbol_value<size>* psymval,
        typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This::template rela_ua<32>(view, object, psymval, addend, 0, 0,
                               0xffffffff);
  }

  static inline void
  abs16(unsigned char* view,
        const Sized_relobj_file<size, big_endian>* object,
        const Symbol_value<size>* psymval,
        typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This::template rela_ua<16>(view, object, psymval, addend, 0, 0,
                               0xffff);
  }

  static inline void
  pc_abs64(unsigned char* view,
        const Sized_relobj_file<size, big_endian>* object,
        const Symbol_value<size>* psymval,
        typename elfcpp::Elf_types<size>::Elf_Addr addend,
	    typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This::template pcrela_ua<64>(view, object, psymval, addend, address, 0, 0,
                               0xffffffffffffffffllu);
  }

  static inline void
  pc_abs32(unsigned char* view,
        const Sized_relobj_file<size, big_endian>* object,
        const Symbol_value<size>* psymval,
        typename elfcpp::Elf_types<size>::Elf_Addr addend,
	    typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This::template pcrela_ua<32>(view, object, psymval, addend, address, 0, 0,
                                 0xffffffff);
  }

  static inline void
  pc_abs16(unsigned char* view,
        const Sized_relobj_file<size, big_endian>* object,
        const Symbol_value<size>* psymval,
        typename elfcpp::Elf_types<size>::Elf_Addr addend,
	    typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This::template pcrela_ua<16>(view, object, psymval, addend, address, 0, 0,
                                 0xffff);
  }

  static inline void
  imm_x_general(unsigned char* view,
                const Sized_relobj_file<size, big_endian>* object,
                const Symbol_value<size>* psymval,
                typename elfcpp::Elf_types<size>::Elf_Addr addend,
                Tilegx_howto &r_howto)
  {
    This::template rela<64>(view, object, psymval, addend,
                            (elfcpp::Elf_Xword)(r_howto.srshift),
                            (elfcpp::Elf_Xword)(r_howto.doffset),
                            (elfcpp::Elf_Xword)((1 << r_howto.bsize) - 1));
  }

  static inline void
  imm_x_pcrel_general(unsigned char* view,
                      const Sized_relobj_file<size, big_endian>* object,
                      const Symbol_value<size>* psymval,
                      typename elfcpp::Elf_types<size>::Elf_Addr addend,
                      typename elfcpp::Elf_types<size>::Elf_Addr address,
                      Tilegx_howto &r_howto)
  {
    This::template pcrela<64>(view, object, psymval, addend, address,
                              (elfcpp::Elf_Xword)(r_howto.srshift),
                              (elfcpp::Elf_Xword)(r_howto.doffset),
                              (elfcpp::Elf_Xword)((1 << r_howto.bsize) - 1));
  }

  static inline void
  imm_x_two_part_general(unsigned char* view,
                         const Sized_relobj_file<size, big_endian>* object,
                         const Symbol_value<size>* psymval,
                         typename elfcpp::Elf_types<size>::Elf_Addr addend,
                         typename elfcpp::Elf_types<size>::Elf_Addr address,
                         unsigned int r_type)
  {

    elfcpp::Elf_Xword doffset1 = 0llu;
    elfcpp::Elf_Xword doffset2 = 0llu;
    elfcpp::Elf_Xword dmask1   = 0llu;
    elfcpp::Elf_Xword dmask2   = 0llu;
    elfcpp::Elf_Xword rshift   = 0llu;
    unsigned int pc_rel        = 0;

    switch (r_type)
      {
      case elfcpp::R_TILEGX_BROFF_X1:
        doffset1 = 31llu;
        doffset2 = 37llu;
        dmask1   = 0x3fllu;
        dmask2   = 0x1ffc0llu;
        rshift   = 3llu;
        pc_rel   = 1;
        break;
      case elfcpp::R_TILEGX_DEST_IMM8_X1:
        doffset1 = 31llu;
        doffset2 = 43llu;
        dmask1   = 0x3fllu;
        dmask2   = 0xc0llu;
        rshift   = 0llu;
        break;
      }

    if (pc_rel)
      This::template pcrela<64>(view, object, psymval, addend, address,
                                rshift, doffset1, dmask1, doffset2, dmask2);
    else
      This::template rela<64>(view, object, psymval, addend, rshift,
                              doffset1, dmask1, doffset2, dmask2);

  }

  static inline void
  tls_relax(unsigned char* view, unsigned int r_type,
            tls::Tls_optimization opt_t)
  {

    const uint64_t TILEGX_X_MOVE_R0_R0 = 0x283bf8005107f000llu;
    const uint64_t TILEGX_Y_MOVE_R0_R0 = 0xae05f800540bf000llu;
    const uint64_t TILEGX_X_LD         = 0x286ae80000000000llu;
    const uint64_t TILEGX_X_LD4S       = 0x286a980000000000llu;
    const uint64_t TILEGX_X1_FULL_MASK = 0x3fffffff80000000llu;
    const uint64_t TILEGX_X0_RRR_MASK  = 0x000000007ffc0000llu;
    const uint64_t TILEGX_X1_RRR_MASK  = 0x3ffe000000000000llu;
    const uint64_t TILEGX_Y0_RRR_MASK  = 0x00000000780c0000llu;
    const uint64_t TILEGX_Y1_RRR_MASK  = 0x3c06000000000000llu;
    const uint64_t TILEGX_X0_RRR_SRCB_MASK = 0x000000007ffff000llu;
    const uint64_t TILEGX_X1_RRR_SRCB_MASK = 0x3ffff80000000000llu;
    const uint64_t TILEGX_Y0_RRR_SRCB_MASK = 0x00000000780ff000llu;
    const uint64_t TILEGX_Y1_RRR_SRCB_MASK = 0x3c07f80000000000llu;
    const uint64_t TILEGX_X_ADD_R0_R0_TP   = 0x2807a800500f5000llu;
    const uint64_t TILEGX_Y_ADD_R0_R0_TP   = 0x9a13a8002c275000llu;
    const uint64_t TILEGX_X_ADDX_R0_R0_TP  = 0x2805a800500b5000llu;
    const uint64_t TILEGX_Y_ADDX_R0_R0_TP  = 0x9a01a8002c035000llu;

    const uint64_t R_TILEGX_IMM8_X0_TLS_ADD_MASK =
      (TILEGX_X0_RRR_MASK | (0x3Fllu << 12));

    const uint64_t R_TILEGX_IMM8_X1_TLS_ADD_MASK =
      (TILEGX_X1_RRR_MASK | (0x3Fllu << 43));

    const uint64_t R_TILEGX_IMM8_Y0_TLS_ADD_MASK =
      (TILEGX_Y0_RRR_MASK | (0x3Fllu << 12));

    const uint64_t R_TILEGX_IMM8_Y1_TLS_ADD_MASK =
      (TILEGX_Y1_RRR_MASK | (0x3Fllu << 43));

    const uint64_t R_TILEGX_IMM8_X0_TLS_ADD_LE_MASK =
      (TILEGX_X0_RRR_SRCB_MASK | (0x3Fllu << 6));

    const uint64_t R_TILEGX_IMM8_X1_TLS_ADD_LE_MASK =
      (TILEGX_X1_RRR_SRCB_MASK | (0x3Fllu << 37));

    const uint64_t R_TILEGX_IMM8_Y0_TLS_ADD_LE_MASK =
      (TILEGX_Y0_RRR_SRCB_MASK | (0x3Fllu << 6));

    const uint64_t R_TILEGX_IMM8_Y1_TLS_ADD_LE_MASK =
      (TILEGX_Y1_RRR_SRCB_MASK | (0x3Fllu << 37));

    typedef typename elfcpp::Swap<64, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<64, big_endian>::readval(wv);
    Valtype reloc = 0;

    switch (r_type)
    {
      case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
        if (opt_t == tls::TLSOPT_NONE) {
          // GD/IE: 1. copy dest operand into the second source operand
          //        2. change the opcode to "add"
          reloc = (val & 0x3Fllu) << 12;  // featch the dest reg
          reloc |= ((size == 32
                     ? TILEGX_X_ADDX_R0_R0_TP
                     : TILEGX_X_ADD_R0_R0_TP)
                    & TILEGX_X0_RRR_MASK);  // change opcode
          val &= ~R_TILEGX_IMM8_X0_TLS_ADD_MASK;
        } else if (opt_t == tls::TLSOPT_TO_LE) {
          // LE: 1. copy dest operand into the first source operand
          //     2. change the opcode to "move"
          reloc = (val & 0x3Fllu) << 6;
          reloc |= (TILEGX_X_MOVE_R0_R0 & TILEGX_X0_RRR_SRCB_MASK);
          val &= ~R_TILEGX_IMM8_X0_TLS_ADD_LE_MASK;
        } else
          gold_unreachable();
        break;
      case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
        if (opt_t == tls::TLSOPT_NONE) {
          reloc = (val & (0x3Fllu << 31)) << 12;
          reloc |= ((size == 32
                     ? TILEGX_X_ADDX_R0_R0_TP
                     : TILEGX_X_ADD_R0_R0_TP)
                    & TILEGX_X1_RRR_MASK);
          val &= ~R_TILEGX_IMM8_X1_TLS_ADD_MASK;
        } else if (opt_t == tls::TLSOPT_TO_LE) {
          reloc = (val & (0x3Fllu << 31)) << 6;
          reloc |= (TILEGX_X_MOVE_R0_R0 & TILEGX_X1_RRR_SRCB_MASK);
          val &= ~R_TILEGX_IMM8_X1_TLS_ADD_LE_MASK;
        } else
          gold_unreachable();
        break;
      case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
        if (opt_t == tls::TLSOPT_NONE) {
          reloc = (val & 0x3Fllu) << 12;
          reloc |= ((size == 32
                     ? TILEGX_Y_ADDX_R0_R0_TP
                     : TILEGX_Y_ADD_R0_R0_TP)
                    & TILEGX_Y0_RRR_MASK);
          val &= ~R_TILEGX_IMM8_Y0_TLS_ADD_MASK;
        } else if (opt_t == tls::TLSOPT_TO_LE) {
          reloc = (val & 0x3Fllu) << 6;
          reloc |= (TILEGX_Y_MOVE_R0_R0 & TILEGX_Y0_RRR_SRCB_MASK);
          val &= ~R_TILEGX_IMM8_Y0_TLS_ADD_LE_MASK;
        } else
          gold_unreachable();
        break;
      case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
        if (opt_t == tls::TLSOPT_NONE) {
          reloc = (val & (0x3Fllu << 31)) << 12;
          reloc |= ((size == 32
                     ? TILEGX_Y_ADDX_R0_R0_TP
                     : TILEGX_Y_ADD_R0_R0_TP)
                    & TILEGX_Y1_RRR_MASK);
          val &= ~R_TILEGX_IMM8_Y1_TLS_ADD_MASK;
        } else if (opt_t == tls::TLSOPT_TO_LE) {
          reloc = (val & (0x3Fllu << 31)) << 6;
          reloc |= (TILEGX_Y_MOVE_R0_R0 & TILEGX_Y1_RRR_SRCB_MASK);
          val &= ~R_TILEGX_IMM8_Y1_TLS_ADD_LE_MASK;
        } else
          gold_unreachable();
        break;
      case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
        if (opt_t == tls::TLSOPT_NONE) {
          // GD see comments for optimize_tls_reloc
          reloc = TILEGX_X_MOVE_R0_R0 & TILEGX_X0_RRR_SRCB_MASK;
          val &= ~TILEGX_X0_RRR_SRCB_MASK;
        } else if (opt_t == tls::TLSOPT_TO_IE
                   || opt_t == tls::TLSOPT_TO_LE) {
          // IE/LE
          reloc = (size == 32
                   ? TILEGX_X_ADDX_R0_R0_TP
                   : TILEGX_X_ADD_R0_R0_TP)
                   & TILEGX_X0_RRR_SRCB_MASK;
          val &= ~TILEGX_X0_RRR_SRCB_MASK;
        }
        break;
      case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
        if (opt_t == tls::TLSOPT_NONE) {
          reloc = TILEGX_X_MOVE_R0_R0 & TILEGX_X1_RRR_SRCB_MASK;
          val &= ~TILEGX_X1_RRR_SRCB_MASK;
        } else if (opt_t == tls::TLSOPT_TO_IE
                   || opt_t == tls::TLSOPT_TO_LE) {
          reloc = (size == 32
                   ? TILEGX_X_ADDX_R0_R0_TP
                   : TILEGX_X_ADD_R0_R0_TP)
                   & TILEGX_X1_RRR_SRCB_MASK;
          val &= ~TILEGX_X1_RRR_SRCB_MASK;
        }
        break;
      case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
        if (opt_t == tls::TLSOPT_NONE) {
          reloc = TILEGX_Y_MOVE_R0_R0 & TILEGX_Y0_RRR_SRCB_MASK;
          val &= ~TILEGX_Y0_RRR_SRCB_MASK;
        } else if (opt_t == tls::TLSOPT_TO_IE
                   || opt_t == tls::TLSOPT_TO_LE) {
          reloc = (size == 32
                   ? TILEGX_Y_ADDX_R0_R0_TP
                   : TILEGX_Y_ADD_R0_R0_TP)
                   & TILEGX_Y0_RRR_SRCB_MASK;
          val &= ~TILEGX_Y0_RRR_SRCB_MASK;
        }
        break;
      case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
        if (opt_t == tls::TLSOPT_NONE) {
          reloc = TILEGX_Y_MOVE_R0_R0 & TILEGX_Y1_RRR_SRCB_MASK;
          val &= ~TILEGX_Y1_RRR_SRCB_MASK;
        } else if (opt_t == tls::TLSOPT_TO_IE
                   || opt_t == tls::TLSOPT_TO_LE) {
          reloc = (size == 32
                   ? TILEGX_Y_ADDX_R0_R0_TP
                   : TILEGX_Y_ADD_R0_R0_TP)
                   & TILEGX_Y1_RRR_SRCB_MASK;
          val &= ~TILEGX_Y1_RRR_SRCB_MASK;
        }
        break;
      case elfcpp::R_TILEGX_TLS_IE_LOAD:
        if (opt_t == tls::TLSOPT_NONE) {
          // IE
          reloc = (size == 32
                   ? TILEGX_X_LD4S
                   : TILEGX_X_LD)
                   & TILEGX_X1_RRR_SRCB_MASK;
          val &= ~TILEGX_X1_RRR_SRCB_MASK;
        } else if (opt_t == tls::TLSOPT_TO_LE) {
          // LE
          reloc = TILEGX_X_MOVE_R0_R0 & TILEGX_X1_RRR_SRCB_MASK;
          val &= ~TILEGX_X1_RRR_SRCB_MASK;
        } else
          gold_unreachable();
        break;
      case elfcpp::R_TILEGX_TLS_GD_CALL:
        if (opt_t == tls::TLSOPT_TO_IE) {
          // ld/ld4s r0, r0
          reloc = (size == 32
                  ? TILEGX_X_LD4S
                  : TILEGX_X_LD) & TILEGX_X1_FULL_MASK;
          val &= ~TILEGX_X1_FULL_MASK;
        } else if (opt_t == tls::TLSOPT_TO_LE) {
          // move r0, r0
          reloc = TILEGX_X_MOVE_R0_R0 & TILEGX_X1_FULL_MASK;
          val &= ~TILEGX_X1_FULL_MASK;
        } else
          // should be handled in ::relocate
          gold_unreachable();
        break;
      default:
        gold_unreachable();
        break;
    }
    elfcpp::Swap<64, big_endian>::writeval(wv, val | reloc);
  }
};

template<>
const Tilegx_relocate_functions<64, false>::Tilegx_howto
Tilegx_relocate_functions<64, false>::howto[elfcpp::R_TILEGX_NUM] =
{
  {  0,  0, 0,  0, 0}, // R_TILEGX_NONE
  {  0,  0, 0, 64, 0}, // R_TILEGX_64
  {  0,  0, 0, 32, 0}, // R_TILEGX_32
  {  0,  0, 0, 16, 0}, // R_TILEGX_16
  {  0,  0, 0,  8, 0}, // R_TILEGX_8
  {  0,  0, 1, 64, 0}, // R_TILEGX_64_PCREL
  {  0,  0, 1, 32, 0}, // R_TILEGX_32_PCREL
  {  0,  0, 1, 16, 0}, // R_TILEGX_16_PCREL
  {  0,  0, 1,  8, 0}, // R_TILEGX_8_PCREL
  {  0,  0, 0,  0, 0}, // R_TILEGX_HW0
  { 16,  0, 0,  0, 0}, // R_TILEGX_HW1
  { 32,  0, 0,  0, 0}, // R_TILEGX_HW2
  { 48,  0, 0,  0, 0}, // R_TILEGX_HW3
  {  0,  0, 0,  0, 0}, // R_TILEGX_HW0_LAST
  { 16,  0, 0,  0, 0}, // R_TILEGX_HW1_LAST
  { 32,  0, 0,  0, 0}, // R_TILEGX_HW2_LAST
  {  0,  0, 0,  0, 0}, // R_TILEGX_COPY
  {  0,  0, 0,  8, 0}, // R_TILEGX_GLOB_DAT
  {  0,  0, 0,  0, 0}, // R_TILEGX_JMP_SLOT
  {  0,  0, 0,  0, 0}, // R_TILEGX_RELATIVE
  {  3,  1, 1,  0, 0}, // R_TILEGX_BROFF_X1
  {  3, 31, 1, 27, 0}, // R_TILEGX_JUMPOFF_X1
  {  3, 31, 1, 27, 0}, // R_TILEGX_JUMPOFF_X1_PLT
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_Y0
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_Y1
  {  0,  1, 0,  8, 0}, // R_TILEGX_DEST_IMM8_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MT_IMM14_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MF_IMM14_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MMSTART_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_MMEND_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_Y0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_Y1
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0
  { 16, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW1
  { 16, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW1
  { 32, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW2
  { 32, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW2
  { 48, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW3
  { 48, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW3
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST
  { 32, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST
  { 32, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST
  {  0, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW0_PCREL
  {  0, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW0_PCREL
  { 16, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW1_PCREL
  { 16, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW1_PCREL
  { 32, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW2_PCREL
  { 32, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW2_PCREL
  { 48, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW3_PCREL
  { 48, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW3_PCREL
  {  0, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_PCREL
  {  0, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_PCREL
  { 16, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_PCREL
  { 16, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_PCREL
  { 32, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_PCREL
  { 32, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_PCREL
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_GOT
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_GOT
  {  0, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW0_PLT_PCREL
  {  0, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW0_PLT_PCREL
  { 16, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW1_PLT_PCREL
  { 16, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW1_PLT_PCREL
  { 32, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW2_PLT_PCREL
  { 32, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW2_PLT_PCREL
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_GOT
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_GOT
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_GOT
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_GOT
  { 32, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_GOT
  { 32, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_GOT
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_GD
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_GD
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_LE
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_LE
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IRELATIVE
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_IE
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_IE
  {  0, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL
  {  0, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL
  { 16, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL
  { 16, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL
  { 32, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL
  { 32, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPMOD64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPOFF64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_TPOFF64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPMOD32
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPOFF32
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_TPOFF32
  {  3, 31, 1, 27, 0}, // R_TILEGX_TLS_GD_CALL
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X0_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X1_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y0_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y1_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_IE_LOAD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X0_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X1_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y0_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y1_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_GNU_VTINHERIT
  {  0,  0, 0,  0, 0}, // R_TILEGX_GNU_VTENTRY
};

template<>
const Tilegx_relocate_functions<32, false>::Tilegx_howto
Tilegx_relocate_functions<32, false>::howto[elfcpp::R_TILEGX_NUM] =
{
  {  0,  0, 0,  0, 0}, // R_TILEGX_NONE
  {  0,  0, 0, 64, 0}, // R_TILEGX_64
  {  0,  0, 0, 32, 0}, // R_TILEGX_32
  {  0,  0, 0, 16, 0}, // R_TILEGX_16
  {  0,  0, 0,  8, 0}, // R_TILEGX_8
  {  0,  0, 1, 64, 0}, // R_TILEGX_64_PCREL
  {  0,  0, 1, 32, 0}, // R_TILEGX_32_PCREL
  {  0,  0, 1, 16, 0}, // R_TILEGX_16_PCREL
  {  0,  0, 1,  8, 0}, // R_TILEGX_8_PCREL
  {  0,  0, 0,  0, 0}, // R_TILEGX_HW0
  { 16,  0, 0,  0, 0}, // R_TILEGX_HW1
  { 31,  0, 0,  0, 0}, // R_TILEGX_HW2
  { 31,  0, 0,  0, 0}, // R_TILEGX_HW3
  {  0,  0, 0,  0, 0}, // R_TILEGX_HW0_LAST
  { 16,  0, 0,  0, 0}, // R_TILEGX_HW1_LAST
  { 31,  0, 0,  0, 0}, // R_TILEGX_HW2_LAST
  {  0,  0, 0,  0, 0}, // R_TILEGX_COPY
  {  0,  0, 0,  8, 0}, // R_TILEGX_GLOB_DAT
  {  0,  0, 0,  0, 0}, // R_TILEGX_JMP_SLOT
  {  0,  0, 0,  0, 0}, // R_TILEGX_RELATIVE
  {  3,  1, 1,  0, 0}, // R_TILEGX_BROFF_X1
  {  3, 31, 1, 27, 0}, // R_TILEGX_JUMPOFF_X1
  {  3, 31, 1, 27, 0}, // R_TILEGX_JUMPOFF_X1_PLT
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_Y0
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_Y1
  {  0,  1, 0,  8, 0}, // R_TILEGX_DEST_IMM8_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MT_IMM14_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MF_IMM14_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MMSTART_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_MMEND_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_Y0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_Y1
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0
  { 16, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW1
  { 16, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW1
  { 31, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW2
  { 31, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW2
  { 31, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW3
  { 31, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW3
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST
  { 31, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST
  { 31, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST
  {  0, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW0_PCREL
  {  0, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW0_PCREL
  { 16, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW1_PCREL
  { 16, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW1_PCREL
  { 31, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW2_PCREL
  { 31, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW2_PCREL
  { 31, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW3_PCREL
  { 31, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW3_PCREL
  {  0, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_PCREL
  {  0, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_PCREL
  { 16, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_PCREL
  { 16, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_PCREL
  { 31, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_PCREL
  { 31, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_PCREL
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_GOT
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_GOT
  {  0, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW0_PLT_PCREL
  {  0, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW0_PLT_PCREL
  { 16, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW1_PLT_PCREL
  { 16, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW1_PLT_PCREL
  { 31, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW2_PLT_PCREL
  { 31, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW2_PLT_PCREL
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_GOT
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_GOT
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_GOT
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_GOT
  { 31, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_GOT
  { 31, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_GOT
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_GD
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_GD
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_LE
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_LE
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IRELATIVE
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_IE
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_IE
  {  0, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL
  {  0, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL
  { 16, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL
  { 16, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL
  { 31, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL
  { 31, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPMOD64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPOFF64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_TPOFF64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPMOD32
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPOFF32
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_TPOFF32
  {  3, 31, 1, 27, 0}, // R_TILEGX_TLS_GD_CALL
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X0_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X1_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y0_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y1_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_IE_LOAD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X0_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X1_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y0_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y1_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_GNU_VTINHERIT
  {  0,  0, 0,  0, 0}, // R_TILEGX_GNU_VTENTRY
};

template<>
const Tilegx_relocate_functions<64, true>::Tilegx_howto
Tilegx_relocate_functions<64, true>::howto[elfcpp::R_TILEGX_NUM] =
{
  {  0,  0, 0,  0, 0}, // R_TILEGX_NONE
  {  0,  0, 0, 64, 0}, // R_TILEGX_64
  {  0,  0, 0, 32, 0}, // R_TILEGX_32
  {  0,  0, 0, 16, 0}, // R_TILEGX_16
  {  0,  0, 0,  8, 0}, // R_TILEGX_8
  {  0,  0, 1, 64, 0}, // R_TILEGX_64_PCREL
  {  0,  0, 1, 32, 0}, // R_TILEGX_32_PCREL
  {  0,  0, 1, 16, 0}, // R_TILEGX_16_PCREL
  {  0,  0, 1,  8, 0}, // R_TILEGX_8_PCREL
  {  0,  0, 0,  0, 0}, // R_TILEGX_HW0
  { 16,  0, 0,  0, 0}, // R_TILEGX_HW1
  { 32,  0, 0,  0, 0}, // R_TILEGX_HW2
  { 48,  0, 0,  0, 0}, // R_TILEGX_HW3
  {  0,  0, 0,  0, 0}, // R_TILEGX_HW0_LAST
  { 16,  0, 0,  0, 0}, // R_TILEGX_HW1_LAST
  { 32,  0, 0,  0, 0}, // R_TILEGX_HW2_LAST
  {  0,  0, 0,  0, 0}, // R_TILEGX_COPY
  {  0,  0, 0,  8, 0}, // R_TILEGX_GLOB_DAT
  {  0,  0, 0,  0, 0}, // R_TILEGX_JMP_SLOT
  {  0,  0, 0,  0, 0}, // R_TILEGX_RELATIVE
  {  3,  1, 1,  0, 0}, // R_TILEGX_BROFF_X1
  {  3, 31, 1, 27, 0}, // R_TILEGX_JUMPOFF_X1
  {  3, 31, 1, 27, 0}, // R_TILEGX_JUMPOFF_X1_PLT
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_Y0
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_Y1
  {  0,  1, 0,  8, 0}, // R_TILEGX_DEST_IMM8_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MT_IMM14_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MF_IMM14_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MMSTART_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_MMEND_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_Y0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_Y1
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0
  { 16, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW1
  { 16, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW1
  { 32, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW2
  { 32, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW2
  { 48, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW3
  { 48, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW3
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST
  { 32, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST
  { 32, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST
  {  0, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW0_PCREL
  {  0, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW0_PCREL
  { 16, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW1_PCREL
  { 16, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW1_PCREL
  { 32, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW2_PCREL
  { 32, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW2_PCREL
  { 48, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW3_PCREL
  { 48, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW3_PCREL
  {  0, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_PCREL
  {  0, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_PCREL
  { 16, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_PCREL
  { 16, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_PCREL
  { 32, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_PCREL
  { 32, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_PCREL
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_GOT
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_GOT
  {  0, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW0_PLT_PCREL
  {  0, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW0_PLT_PCREL
  { 16, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW1_PLT_PCREL
  { 16, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW1_PLT_PCREL
  { 32, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW2_PLT_PCREL
  { 32, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW2_PLT_PCREL
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_GOT
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_GOT
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_GOT
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_GOT
  { 32, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_GOT
  { 32, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_GOT
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_GD
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_GD
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_LE
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_LE
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IRELATIVE
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_IE
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_IE
  {  0, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL
  {  0, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL
  { 16, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL
  { 16, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL
  { 32, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL
  { 32, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPMOD64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPOFF64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_TPOFF64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPMOD32
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPOFF32
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_TPOFF32
  {  3, 31, 1, 27, 0}, // R_TILEGX_TLS_GD_CALL
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X0_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X1_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y0_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y1_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_IE_LOAD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X0_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X1_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y0_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y1_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_GNU_VTINHERIT
  {  0,  0, 0,  0, 0}, // R_TILEGX_GNU_VTENTRY
};

template<>
const Tilegx_relocate_functions<32, true>::Tilegx_howto
Tilegx_relocate_functions<32, true>::howto[elfcpp::R_TILEGX_NUM] =
{
  {  0,  0, 0,  0, 0}, // R_TILEGX_NONE
  {  0,  0, 0, 64, 0}, // R_TILEGX_64
  {  0,  0, 0, 32, 0}, // R_TILEGX_32
  {  0,  0, 0, 16, 0}, // R_TILEGX_16
  {  0,  0, 0,  8, 0}, // R_TILEGX_8
  {  0,  0, 1, 64, 0}, // R_TILEGX_64_PCREL
  {  0,  0, 1, 32, 0}, // R_TILEGX_32_PCREL
  {  0,  0, 1, 16, 0}, // R_TILEGX_16_PCREL
  {  0,  0, 1,  8, 0}, // R_TILEGX_8_PCREL
  {  0,  0, 0,  0, 0}, // R_TILEGX_HW0
  { 16,  0, 0,  0, 0}, // R_TILEGX_HW1
  { 31,  0, 0,  0, 0}, // R_TILEGX_HW2
  { 31,  0, 0,  0, 0}, // R_TILEGX_HW3
  {  0,  0, 0,  0, 0}, // R_TILEGX_HW0_LAST
  { 16,  0, 0,  0, 0}, // R_TILEGX_HW1_LAST
  { 31,  0, 0,  0, 0}, // R_TILEGX_HW2_LAST
  {  0,  0, 0,  0, 0}, // R_TILEGX_COPY
  {  0,  0, 0,  8, 0}, // R_TILEGX_GLOB_DAT
  {  0,  0, 0,  0, 0}, // R_TILEGX_JMP_SLOT
  {  0,  0, 0,  0, 0}, // R_TILEGX_RELATIVE
  {  3,  1, 1,  0, 0}, // R_TILEGX_BROFF_X1
  {  3, 31, 1, 27, 0}, // R_TILEGX_JUMPOFF_X1
  {  3, 31, 1, 27, 0}, // R_TILEGX_JUMPOFF_X1_PLT
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_Y0
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_IMM8_Y1
  {  0,  1, 0,  8, 0}, // R_TILEGX_DEST_IMM8_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MT_IMM14_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MF_IMM14_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_MMSTART_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_MMEND_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_X0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_X1
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_Y0
  {  0,  1, 0,  8, 0}, // R_TILEGX_SHAMT_Y1
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0
  { 16, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW1
  { 16, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW1
  { 31, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW2
  { 31, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW2
  { 31, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW3
  { 31, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW3
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST
  { 31, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST
  { 31, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST
  {  0, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW0_PCREL
  {  0, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW0_PCREL
  { 16, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW1_PCREL
  { 16, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW1_PCREL
  { 31, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW2_PCREL
  { 31, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW2_PCREL
  { 31, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW3_PCREL
  { 31, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW3_PCREL
  {  0, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_PCREL
  {  0, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_PCREL
  { 16, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_PCREL
  { 16, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_PCREL
  { 31, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_PCREL
  { 31, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_PCREL
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_GOT
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_GOT
  {  0, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW0_PLT_PCREL
  {  0, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW0_PLT_PCREL
  { 16, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW1_PLT_PCREL
  { 16, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW1_PLT_PCREL
  { 31, 12, 1, 16, 0}, // R_TILEGX_IMM16_X0_HW2_PLT_PCREL
  { 31, 43, 1, 16, 0}, // R_TILEGX_IMM16_X1_HW2_PLT_PCREL
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_GOT
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_GOT
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_GOT
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_GOT
  { 31, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_GOT
  { 31, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_GOT
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_GD
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_GD
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_LE
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_LE
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IRELATIVE
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0, 12, 0, 16, 0}, // R_TILEGX_IMM16_X0_HW0_TLS_IE
  {  0, 43, 0, 16, 0}, // R_TILEGX_IMM16_X1_HW0_TLS_IE
  {  0, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL
  {  0, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL
  { 16, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL
  { 16, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL
  { 31, 12, 1, 16, 1}, // R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL
  { 31, 43, 1, 16, 1}, // R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL
  {  0, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE
  {  0, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE
  { 16, 12, 0, 16, 1}, // R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE
  { 16, 43, 0, 16, 1}, // R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0,  0, 0,  0, 0}, // R_TILEGX_INVALID
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPMOD64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPOFF64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_TPOFF64
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPMOD32
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_DTPOFF32
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_TPOFF32
  {  3, 31, 1, 27, 0}, // R_TILEGX_TLS_GD_CALL
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X0_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X1_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y0_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y1_TLS_GD_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_TLS_IE_LOAD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X0_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_X1_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y0_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_IMM8_Y1_TLS_ADD
  {  0,  0, 0,  0, 0}, // R_TILEGX_GNU_VTINHERIT
  {  0,  0, 0,  0, 0}, // R_TILEGX_GNU_VTENTRY
};

// Get the GOT section, creating it if necessary.

template<int size, bool big_endian>
Output_data_got<size, big_endian>*
Target_tilegx<size, big_endian>::got_section(Symbol_table* symtab,
                                             Layout* layout)
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

      this->got_ = new Output_data_got<size, big_endian>();

      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC
                                       | elfcpp::SHF_WRITE),
                                      this->got_, got_order, true);

      // Define _GLOBAL_OFFSET_TABLE_ at the start of the PLT.
      this->global_offset_table_ =
        symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
                                      Symbol_table::PREDEFINED,
                                      this->got_,
                                      0, 0, elfcpp::STT_OBJECT,
                                      elfcpp::STB_LOCAL,
                                      elfcpp::STV_HIDDEN, 0,
                                      false, false);

      if (parameters->options().shared()) {
        // we need to keep the address of .dynamic section in the
        // first got entry for .so
        this->tilegx_dynamic_ =
          symtab->define_in_output_data("_TILEGX_DYNAMIC_", NULL,
                                        Symbol_table::PREDEFINED,
                                        layout->dynamic_section(),
                                        0, 0, elfcpp::STT_OBJECT,
                                        elfcpp::STB_LOCAL,
                                        elfcpp::STV_HIDDEN, 0,
                                        false, false);

        this->got_->add_global(this->tilegx_dynamic_, GOT_TYPE_STANDARD);
      } else
        // for executable, just set the first entry to zero.
        this->got_->set_current_data_size(size / 8);

      this->got_plt_ = new Output_data_space(size / 8, "** GOT PLT");
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC
                                       | elfcpp::SHF_WRITE),
                                      this->got_plt_, got_plt_order,
                                      is_got_plt_relro);

      // The first two entries are reserved.
      this->got_plt_->set_current_data_size
             (TILEGX_GOTPLT_RESERVE_COUNT * (size / 8));

      if (!is_got_plt_relro)
        {
          // Those bytes can go into the relro segment.
          layout->increase_relro(size / 8);
        }


      // If there are any IRELATIVE relocations, they get GOT entries
      // in .got.plt after the jump slot entries.
      this->got_irelative_
         = new Output_data_space(size / 8, "** GOT IRELATIVE PLT");
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC
                                       | elfcpp::SHF_WRITE),
                                      this->got_irelative_,
                                      got_plt_order, is_got_plt_relro);
    }

  return this->got_;
}

// Get the dynamic reloc section, creating it if necessary.

template<int size, bool big_endian>
typename Target_tilegx<size, big_endian>::Reloc_section*
Target_tilegx<size, big_endian>::rela_dyn_section(Layout* layout)
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

template<int size, bool big_endian>
typename Target_tilegx<size, big_endian>::Reloc_section*
Target_tilegx<size, big_endian>::rela_irelative_section(Layout* layout)
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

// Initialize the PLT section.

template<int size, bool big_endian>
void
Output_data_plt_tilegx<size, big_endian>::init(Layout* layout)
{
  this->rel_ = new Reloc_section(false);
  layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
                                  elfcpp::SHF_ALLOC, this->rel_,
                                  ORDER_DYNAMIC_PLT_RELOCS, false);
}

template<int size, bool big_endian>
void
Output_data_plt_tilegx<size, big_endian>::do_adjust_output_section(
  Output_section* os)
{
  os->set_entsize(this->get_plt_entry_size());
}

// Add an entry to the PLT.

template<int size, bool big_endian>
void
Output_data_plt_tilegx<size, big_endian>::add_entry(Symbol_table* symtab,
  Layout* layout, Symbol* gsym)
{
  gold_assert(!gsym->has_plt_offset());

  unsigned int plt_index;
  off_t plt_offset;
  section_offset_type got_offset;

  unsigned int* pcount;
  unsigned int reserved;
  Output_data_space* got;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      pcount = &this->irelative_count_;
      reserved = 0;
      got = this->got_irelative_;
    }
  else
    {
      pcount = &this->count_;
      reserved = TILEGX_GOTPLT_RESERVE_COUNT;
      got = this->got_plt_;
    }

  if (!this->is_data_size_valid())
    {
      plt_index = *pcount;

      // TILEGX .plt section layout
      //
      //  ----
      //   plt_header
      //  ----
      //   plt stub
      //  ----
      //   ...
      //  ----
      //
      // TILEGX .got.plt section layout
      //
      //  ----
      //  reserv1
      //  ----
      //  reserv2
      //  ----
      //   entries for normal function
      //  ----
      //   ...
      //  ----
      //   entries for ifunc
      //  ----
      //   ...
      //  ----
      if (got == this->got_irelative_)
        plt_offset = plt_index * this->get_plt_entry_size();
      else
        plt_offset = (plt_index + 1) * this->get_plt_entry_size();

      ++*pcount;

      got_offset = (plt_index + reserved) * (size / 8);
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
      plt_offset = this->free_list_.allocate(this->get_plt_entry_size(),
                                             this->get_plt_entry_size(), 0);
      if (plt_offset == -1)
        gold_fallback(_("out of patch space (PLT);"
                        " relink with --incremental-full"));

      // The GOT and PLT entries have a 1-1 correspondance, so the GOT offset
      // can be calculated from the PLT index, adjusting for the three
      // reserved entries at the beginning of the GOT.
      plt_index = plt_offset / this->get_plt_entry_size() - 1;
      got_offset = (plt_index + reserved) * (size / 8);
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

template<int size, bool big_endian>
unsigned int
Output_data_plt_tilegx<size, big_endian>::add_local_ifunc_entry(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, big_endian>* relobj,
    unsigned int local_sym_index)
{
  unsigned int plt_offset =
    this->irelative_count_ * this->get_plt_entry_size();
  ++this->irelative_count_;

  section_offset_type got_offset = this->got_irelative_->current_data_size();

  // Every PLT entry needs a GOT entry which points back to the PLT
  // entry.
  this->got_irelative_->set_current_data_size(got_offset + size / 8);

  // Every PLT entry needs a reloc.
  Reloc_section* rela = this->rela_irelative(symtab, layout);
  rela->add_symbolless_local_addend(relobj, local_sym_index,
                                    elfcpp::R_TILEGX_IRELATIVE,
                                    this->got_irelative_, got_offset, 0);

  return plt_offset;
}

// Add the relocation for a PLT entry.

template<int size, bool big_endian>
void
Output_data_plt_tilegx<size, big_endian>::add_relocation(Symbol_table* symtab,
                                             Layout* layout,
                                             Symbol* gsym,
                                             unsigned int got_offset)
{
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      Reloc_section* rela = this->rela_irelative(symtab, layout);
      rela->add_symbolless_global_addend(gsym, elfcpp::R_TILEGX_IRELATIVE,
                                         this->got_irelative_, got_offset, 0);
    }
  else
    {
      gsym->set_needs_dynsym_entry();
      this->rel_->add_global(gsym, elfcpp::R_TILEGX_JMP_SLOT, this->got_plt_,
                             got_offset, 0);
    }
}

// Return where the IRELATIVE relocations should go in the PLT.  These
// follow the JUMP_SLOT and the TLSDESC relocations.

template<int size, bool big_endian>
typename Output_data_plt_tilegx<size, big_endian>::Reloc_section*
Output_data_plt_tilegx<size, big_endian>::rela_irelative(Symbol_table* symtab,
                                                         Layout* layout)
{
  if (this->irelative_rel_ == NULL)
    {
      // case we see any later on.
      this->irelative_rel_ = new Reloc_section(false);
      layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
                                      elfcpp::SHF_ALLOC, this->irelative_rel_,
                                      ORDER_DYNAMIC_PLT_RELOCS, false);
      gold_assert(this->irelative_rel_->output_section()
                  == this->rel_->output_section());

      if (parameters->doing_static_link())
        {
          // A statically linked executable will only have a .rela.plt
          // section to hold R_TILEGX_IRELATIVE relocs for
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

template<int size, bool big_endian>
uint64_t
Output_data_plt_tilegx<size, big_endian>::address_for_global(
  const Symbol* gsym)
{
  uint64_t offset = 0;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    offset = (this->count_ + 1) * this->get_plt_entry_size();
  return this->address() + offset + gsym->plt_offset();
}

// Return the PLT address to use for a local symbol.  These are always
// IRELATIVE relocs.

template<int size, bool big_endian>
uint64_t
Output_data_plt_tilegx<size, big_endian>::address_for_local(
    const Relobj* object,
    unsigned int r_sym)
{
  return (this->address()
	  + (this->count_ + 1) * this->get_plt_entry_size()
	  + object->local_plt_offset(r_sym));
}

// Set the final size.
template<int size, bool big_endian>
void
Output_data_plt_tilegx<size, big_endian>::set_final_data_size()
{
  unsigned int count = this->count_ + this->irelative_count_;
  this->set_data_size((count + 1) * this->get_plt_entry_size());
}

// The first entry in the PLT for an executable.
template<>
const unsigned char
Output_data_plt_tilegx<64, false>::first_plt_entry[plt_entry_size] =
{
  0x00, 0x30, 0x48, 0x51,
  0x6e, 0x43, 0xa0, 0x18, // { ld_add r28, r27, 8 }
  0x00, 0x30, 0xbc, 0x35,
  0x00, 0x40, 0xde, 0x9e, // { ld r27, r27 }
  0xff, 0xaf, 0x30, 0x40,
  0x60, 0x73, 0x6a, 0x28, // { info 10 ; jr r27 }
  // padding
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

template<>
const unsigned char
Output_data_plt_tilegx<32, false>::first_plt_entry[plt_entry_size] =
{
  0x00, 0x30, 0x48, 0x51,
  0x6e, 0x23, 0x58, 0x18, // { ld4s_add r28, r27, 4 }
  0x00, 0x30, 0xbc, 0x35,
  0x00, 0x40, 0xde, 0x9c, // { ld4s r27, r27 }
  0xff, 0xaf, 0x30, 0x40,
  0x60, 0x73, 0x6a, 0x28, // { info 10 ; jr r27 }
  // padding
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

template<>
const unsigned char
Output_data_plt_tilegx<64, true>::first_plt_entry[plt_entry_size] =
{
  0x00, 0x30, 0x48, 0x51,
  0x6e, 0x43, 0xa0, 0x18, // { ld_add r28, r27, 8 }
  0x00, 0x30, 0xbc, 0x35,
  0x00, 0x40, 0xde, 0x9e, // { ld r27, r27 }
  0xff, 0xaf, 0x30, 0x40,
  0x60, 0x73, 0x6a, 0x28, // { info 10 ; jr r27 }
  // padding
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

template<>
const unsigned char
Output_data_plt_tilegx<32, true>::first_plt_entry[plt_entry_size] =
{
  0x00, 0x30, 0x48, 0x51,
  0x6e, 0x23, 0x58, 0x18, // { ld4s_add r28, r27, 4 }
  0x00, 0x30, 0xbc, 0x35,
  0x00, 0x40, 0xde, 0x9c, // { ld4s r27, r27 }
  0xff, 0xaf, 0x30, 0x40,
  0x60, 0x73, 0x6a, 0x28, // { info 10 ; jr r27 }
  // padding
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

template<int size, bool big_endian>
void
Output_data_plt_tilegx<size, big_endian>::fill_first_plt_entry(
  unsigned char* pov)
{
  memcpy(pov, first_plt_entry, plt_entry_size);
}

// Subsequent entries in the PLT for an executable.

template<>
const unsigned char
Output_data_plt_tilegx<64, false>::plt_entry[plt_entry_size] =
{
  0xdc, 0x0f, 0x00, 0x10,
  0x0d, 0xf0, 0x6a, 0x28, // { moveli r28, 0 ; lnk r26 }
  0xdb, 0x0f, 0x00, 0x10,
  0x8e, 0x03, 0x00, 0x38, // { moveli r27, 0 ; shl16insli r28, r28, 0 }
  0x9c, 0xc6, 0x0d, 0xd0,
  0x6d, 0x03, 0x00, 0x38, // { add r28, r26, r28 ; shl16insli r27, r27, 0 }
  0x9b, 0xb6, 0xc5, 0xad,
  0xff, 0x57, 0xe0, 0x8e, // { add r27, r26, r27 ; info 10 ; ld r28, r28 }
  0xdd, 0x0f, 0x00, 0x70,
  0x80, 0x73, 0x6a, 0x28, // { shl16insli r29, zero, 0 ; jr r28 }

};

template<>
const unsigned char
Output_data_plt_tilegx<32, false>::plt_entry[plt_entry_size] =
{
  0xdc, 0x0f, 0x00, 0x10,
  0x0d, 0xf0, 0x6a, 0x28, // { moveli r28, 0 ; lnk r26 }
  0xdb, 0x0f, 0x00, 0x10,
  0x8e, 0x03, 0x00, 0x38, // { moveli r27, 0 ; shl16insli r28, r28, 0 }
  0x9c, 0xc6, 0x0d, 0xd0,
  0x6d, 0x03, 0x00, 0x38, // { add r28, r26, r28 ; shl16insli r27, r27, 0 }
  0x9b, 0xb6, 0xc5, 0xad,
  0xff, 0x57, 0xe0, 0x8c, // { add r27, r26, r27 ; info 10 ; ld4s r28, r28 }
  0xdd, 0x0f, 0x00, 0x70,
  0x80, 0x73, 0x6a, 0x28, // { shl16insli r29, zero, 0 ; jr r28 }
};

template<>
const unsigned char
Output_data_plt_tilegx<64, true>::plt_entry[plt_entry_size] =
{
  0xdc, 0x0f, 0x00, 0x10,
  0x0d, 0xf0, 0x6a, 0x28, // { moveli r28, 0 ; lnk r26 }
  0xdb, 0x0f, 0x00, 0x10,
  0x8e, 0x03, 0x00, 0x38, // { moveli r27, 0 ; shl16insli r28, r28, 0 }
  0x9c, 0xc6, 0x0d, 0xd0,
  0x6d, 0x03, 0x00, 0x38, // { add r28, r26, r28 ; shl16insli r27, r27, 0 }
  0x9b, 0xb6, 0xc5, 0xad,
  0xff, 0x57, 0xe0, 0x8e, // { add r27, r26, r27 ; info 10 ; ld r28, r28 }
  0xdd, 0x0f, 0x00, 0x70,
  0x80, 0x73, 0x6a, 0x28, // { shl16insli r29, zero, 0 ; jr r28 }

};

template<>
const unsigned char
Output_data_plt_tilegx<32, true>::plt_entry[plt_entry_size] =
{
  0xdc, 0x0f, 0x00, 0x10,
  0x0d, 0xf0, 0x6a, 0x28, // { moveli r28, 0 ; lnk r26 }
  0xdb, 0x0f, 0x00, 0x10,
  0x8e, 0x03, 0x00, 0x38, // { moveli r27, 0 ; shl16insli r28, r28, 0 }
  0x9c, 0xc6, 0x0d, 0xd0,
  0x6d, 0x03, 0x00, 0x38, // { add r28, r26, r28 ; shl16insli r27, r27, 0 }
  0x9b, 0xb6, 0xc5, 0xad,
  0xff, 0x57, 0xe0, 0x8c, // { add r27, r26, r27 ; info 10 ; ld4s r28, r28 }
  0xdd, 0x0f, 0x00, 0x70,
  0x80, 0x73, 0x6a, 0x28, // { shl16insli r29, zero, 0 ; jr r28 }
};

template<int size, bool big_endian>
void
Output_data_plt_tilegx<size, big_endian>::fill_plt_entry(
                 unsigned char* pov,
                 typename elfcpp::Elf_types<size>::Elf_Addr gotplt_base,
                 unsigned int got_offset,
                 typename elfcpp::Elf_types<size>::Elf_Addr plt_base,
                 unsigned int plt_offset, unsigned int plt_index)
{

  const uint32_t TILEGX_IMM16_MASK = 0xFFFF;
  const uint32_t TILEGX_X0_IMM16_BITOFF = 12;
  const uint32_t TILEGX_X1_IMM16_BITOFF = 43;

  typedef typename elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::Valtype
    Valtype;
  memcpy(pov, plt_entry, plt_entry_size);

  // first bundle in plt stub - x0
  Valtype* wv = reinterpret_cast<Valtype*>(pov);
  Valtype val = elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::readval(wv);
  Valtype reloc =
    ((gotplt_base + got_offset) - (plt_base + plt_offset + 8)) >> 16;
  elfcpp::Elf_Xword dst_mask =
    (elfcpp::Elf_Xword)(TILEGX_IMM16_MASK) << TILEGX_X0_IMM16_BITOFF;
  val &= ~dst_mask;
  reloc &= TILEGX_IMM16_MASK;
  elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::writeval(wv,
    val | (reloc<<TILEGX_X0_IMM16_BITOFF));

  // second bundle in plt stub - x1
  wv = reinterpret_cast<Valtype*>(pov + 8);
  val = elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::readval(wv);
  reloc = (gotplt_base + got_offset) - (plt_base + plt_offset + 8);
  dst_mask = (elfcpp::Elf_Xword)(TILEGX_IMM16_MASK) << TILEGX_X1_IMM16_BITOFF;
  val &= ~dst_mask;
  reloc &= TILEGX_IMM16_MASK;
  elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::writeval(wv,
    val | (reloc<<TILEGX_X1_IMM16_BITOFF));

  // second bundle in plt stub - x0
  wv = reinterpret_cast<Valtype*>(pov + 8);
  val = elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::readval(wv);
  reloc = (gotplt_base - (plt_base + plt_offset + 8)) >> 16;
  dst_mask = (elfcpp::Elf_Xword)(TILEGX_IMM16_MASK) << TILEGX_X0_IMM16_BITOFF;
  val &= ~dst_mask;
  reloc &= TILEGX_IMM16_MASK;
  elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::writeval(wv,
    val | (reloc<<TILEGX_X0_IMM16_BITOFF));

  // third bundle in plt stub - x1
  wv = reinterpret_cast<Valtype*>(pov + 16);
  val = elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::readval(wv);
  reloc = gotplt_base - (plt_base + plt_offset + 8);
  dst_mask = (elfcpp::Elf_Xword)(TILEGX_IMM16_MASK) << TILEGX_X1_IMM16_BITOFF;
  val &= ~dst_mask;
  reloc &= TILEGX_IMM16_MASK;
  elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::writeval(wv,
    val | (reloc<<TILEGX_X1_IMM16_BITOFF));

  // fifth bundle in plt stub - carry plt_index x0
  wv = reinterpret_cast<Valtype*>(pov + 32);
  val = elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::readval(wv);
  dst_mask = (elfcpp::Elf_Xword)(TILEGX_IMM16_MASK) << TILEGX_X0_IMM16_BITOFF;
  val &= ~dst_mask;
  plt_index &= TILEGX_IMM16_MASK;
  elfcpp::Swap<TILEGX_INST_BUNDLE_SIZE, big_endian>::writeval(wv,
    val | (plt_index<<TILEGX_X0_IMM16_BITOFF));

}

// Write out the PLT.  This uses the hand-coded instructions above.

template<int size, bool big_endian>
void
Output_data_plt_tilegx<size, big_endian>::do_write(Output_file* of)
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
  typename elfcpp::Elf_types<size>::Elf_Addr got_address =
    this->got_plt_->address();

  this->fill_first_plt_entry(pov);
  pov += this->get_plt_entry_size();

  unsigned char* got_pov = got_view;

  // first entry of .got.plt are set to -1
  // second entry of .got.plt are set to 0
  memset(got_pov, 0xff, size / 8);
  got_pov += size / 8;
  memset(got_pov, 0x0, size / 8);
  got_pov += size / 8;

  unsigned int plt_offset = this->get_plt_entry_size();
  const unsigned int count = this->count_ + this->irelative_count_;
  unsigned int got_offset = (size / 8) * TILEGX_GOTPLT_RESERVE_COUNT;
  for (unsigned int plt_index = 0;
       plt_index < count;
       ++plt_index,
         pov += this->get_plt_entry_size(),
         got_pov += size / 8,
         plt_offset += this->get_plt_entry_size(),
         got_offset += size / 8)
    {
      // Set and adjust the PLT entry itself.
      this->fill_plt_entry(pov, got_address, got_offset,
                           plt_address, plt_offset, plt_index);

      // Initialize entry in .got.plt to plt start address
      elfcpp::Swap<size, big_endian>::writeval(got_pov, plt_address);
    }

  gold_assert(static_cast<section_size_type>(pov - oview) == oview_size);
  gold_assert(static_cast<section_size_type>(got_pov - got_view) == got_size);

  of->write_output_view(offset, oview_size, oview);
  of->write_output_view(got_file_offset, got_size, got_view);
}

// Create the PLT section.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::make_plt_section(Symbol_table* symtab,
                                                  Layout* layout)
{
  if (this->plt_ == NULL)
    {
      // Create the GOT sections first.
      this->got_section(symtab, layout);

      // Ensure that .rela.dyn always appears before .rela.plt,
      // because on TILE-Gx, .rela.dyn needs to include .rela.plt
      // in it's range.
      this->rela_dyn_section(layout);

      this->plt_ = new Output_data_plt_tilegx<size, big_endian>(layout,
        TILEGX_INST_BUNDLE_SIZE, this->got_, this->got_plt_,
        this->got_irelative_);

      layout->add_output_section_data(".plt", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC
                                       | elfcpp::SHF_EXECINSTR),
                                      this->plt_, ORDER_NON_RELRO_FIRST,
                                      false);

      // Make the sh_info field of .rela.plt point to .plt.
      Output_section* rela_plt_os = this->plt_->rela_plt()->output_section();
      rela_plt_os->set_info_section(this->plt_->output_section());
    }
}

// Create a PLT entry for a global symbol.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::make_plt_entry(Symbol_table* symtab,
                                                Layout* layout, Symbol* gsym)
{
  if (gsym->has_plt_offset())
    return;

  if (this->plt_ == NULL)
    this->make_plt_section(symtab, layout);

  this->plt_->add_entry(symtab, layout, gsym);
}

// Make a PLT entry for a local STT_GNU_IFUNC symbol.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::make_local_ifunc_plt_entry(
    Symbol_table* symtab, Layout* layout,
    Sized_relobj_file<size, big_endian>* relobj,
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

template<int size, bool big_endian>
unsigned int
Target_tilegx<size, big_endian>::plt_entry_count() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->entry_count();
}

// Return the offset of the first non-reserved PLT entry.

template<int size, bool big_endian>
unsigned int
Target_tilegx<size, big_endian>::first_plt_entry_offset() const
{
  return this->plt_->first_plt_entry_offset();
}

// Return the size of each PLT entry.

template<int size, bool big_endian>
unsigned int
Target_tilegx<size, big_endian>::plt_entry_size() const
{
  return this->plt_->get_plt_entry_size();
}

// Create the GOT and PLT sections for an incremental update.

template<int size, bool big_endian>
Output_data_got_base*
Target_tilegx<size, big_endian>::init_got_plt_for_update(Symbol_table* symtab,
                                       Layout* layout,
                                       unsigned int got_count,
                                       unsigned int plt_count)
{
  gold_assert(this->got_ == NULL);

  this->got_ =
    new Output_data_got<size, big_endian>((got_count
                                           + TILEGX_GOT_RESERVE_COUNT)
                                          * (size / 8));
  layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
                                  (elfcpp::SHF_ALLOC
                                   | elfcpp::SHF_WRITE),
                                  this->got_, ORDER_RELRO_LAST,
                                  true);

  // Define _GLOBAL_OFFSET_TABLE_ at the start of the GOT.
  this->global_offset_table_ =
    symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
                                  Symbol_table::PREDEFINED,
                                  this->got_,
                                  0, 0, elfcpp::STT_OBJECT,
                                  elfcpp::STB_LOCAL,
                                  elfcpp::STV_HIDDEN, 0,
                                  false, false);

  if (parameters->options().shared()) {
    this->tilegx_dynamic_ =
            symtab->define_in_output_data("_TILEGX_DYNAMIC_", NULL,
                            Symbol_table::PREDEFINED,
                            layout->dynamic_section(),
                            0, 0, elfcpp::STT_OBJECT,
                            elfcpp::STB_LOCAL,
                            elfcpp::STV_HIDDEN, 0,
                            false, false);

    this->got_->add_global(this->tilegx_dynamic_, GOT_TYPE_STANDARD);
  } else
    this->got_->set_current_data_size(size / 8);

  // Add the two reserved entries.
  this->got_plt_
     = new Output_data_space((plt_count + TILEGX_GOTPLT_RESERVE_COUNT)
                              * (size / 8), size / 8, "** GOT PLT");
  layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
                                  (elfcpp::SHF_ALLOC
                                   | elfcpp::SHF_WRITE),
                                  this->got_plt_, ORDER_NON_RELRO_FIRST,
                                  false);

  // If there are any IRELATIVE relocations, they get GOT entries in
  // .got.plt after the jump slot.
  this->got_irelative_
     = new Output_data_space(0, size / 8, "** GOT IRELATIVE PLT");
  layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
                                  elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE,
                                  this->got_irelative_,
                                  ORDER_NON_RELRO_FIRST, false);

  // Create the PLT section.
  this->plt_ = new Output_data_plt_tilegx<size, big_endian>(layout,
    this->plt_entry_size(), this->got_, this->got_plt_, this->got_irelative_,
    plt_count);

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

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::reserve_local_got_entry(
    unsigned int got_index,
    Sized_relobj<size, big_endian>* obj,
    unsigned int r_sym,
    unsigned int got_type)
{
  unsigned int got_offset = (got_index + TILEGX_GOT_RESERVE_COUNT)
                            * (size / 8);
  Reloc_section* rela_dyn = this->rela_dyn_section(NULL);

  this->got_->reserve_local(got_index, obj, r_sym, got_type);
  switch (got_type)
    {
    case GOT_TYPE_STANDARD:
      if (parameters->options().output_is_position_independent())
        rela_dyn->add_local_relative(obj, r_sym, elfcpp::R_TILEGX_RELATIVE,
                                     this->got_, got_offset, 0, false);
      break;
    case GOT_TYPE_TLS_OFFSET:
      rela_dyn->add_local(obj, r_sym,
                          size == 32 ? elfcpp::R_TILEGX_TLS_DTPOFF32
                                       : elfcpp::R_TILEGX_TLS_DTPOFF64,
                          this->got_, got_offset, 0);
      break;
    case GOT_TYPE_TLS_PAIR:
      this->got_->reserve_slot(got_index + 1);
      rela_dyn->add_local(obj, r_sym,
                          size == 32 ? elfcpp::R_TILEGX_TLS_DTPMOD32
                                       : elfcpp::R_TILEGX_TLS_DTPMOD64,
                          this->got_, got_offset, 0);
      break;
    case GOT_TYPE_TLS_DESC:
      gold_fatal(_("TLS_DESC not yet supported for incremental linking"));
      break;
    default:
      gold_unreachable();
    }
}

// Reserve a GOT entry for a global symbol, and regenerate any
// necessary dynamic relocations.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::reserve_global_got_entry(
  unsigned int got_index, Symbol* gsym, unsigned int got_type)
{
  unsigned int got_offset = (got_index + TILEGX_GOT_RESERVE_COUNT)
                            * (size / 8);
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
            rela_dyn->add_global(gsym, elfcpp::R_TILEGX_GLOB_DAT,
                                 this->got_, got_offset, 0);
          else
            rela_dyn->add_global_relative(gsym, elfcpp::R_TILEGX_RELATIVE,
                                          this->got_, got_offset, 0, false);
        }
      break;
    case GOT_TYPE_TLS_OFFSET:
      rela_dyn->add_global_relative(gsym,
                                    size == 32 ? elfcpp::R_TILEGX_TLS_TPOFF32
                                               : elfcpp::R_TILEGX_TLS_TPOFF64,
                                    this->got_, got_offset, 0, false);
      break;
    case GOT_TYPE_TLS_PAIR:
      this->got_->reserve_slot(got_index + 1);
      rela_dyn->add_global_relative(gsym,
                                    size == 32 ? elfcpp::R_TILEGX_TLS_DTPMOD32
                                               : elfcpp::R_TILEGX_TLS_DTPMOD64,
                                    this->got_, got_offset, 0, false);
      rela_dyn->add_global_relative(gsym,
                                    size == 32 ? elfcpp::R_TILEGX_TLS_DTPOFF32
                                               : elfcpp::R_TILEGX_TLS_DTPOFF64,
                                    this->got_, got_offset + size / 8,
                                    0, false);
      break;
    case GOT_TYPE_TLS_DESC:
      gold_fatal(_("TLS_DESC not yet supported for TILEGX"));
      break;
    default:
      gold_unreachable();
    }
}

// Register an existing PLT entry for a global symbol.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::register_global_plt_entry(
  Symbol_table* symtab, Layout* layout, unsigned int plt_index, Symbol* gsym)
{
  gold_assert(this->plt_ != NULL);
  gold_assert(!gsym->has_plt_offset());

  this->plt_->reserve_slot(plt_index);

  gsym->set_plt_offset((plt_index + 1) * this->plt_entry_size());

  unsigned int got_offset = (plt_index + 2) * (size / 8);
  this->plt_->add_relocation(symtab, layout, gsym, got_offset);
}

// Force a COPY relocation for a given symbol.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::emit_copy_reloc(
    Symbol_table* symtab, Symbol* sym, Output_section* os, off_t offset)
{
  this->copy_relocs_.emit_copy_reloc(symtab,
                                     symtab->get_sized_symbol<size>(sym),
                                     os,
                                     offset,
                                     this->rela_dyn_section(NULL));
}

// Create a GOT entry for the TLS module index.

template<int size, bool big_endian>
unsigned int
Target_tilegx<size, big_endian>::got_mod_index_entry(Symbol_table* symtab,
                                  Layout* layout,
                                  Sized_relobj_file<size, big_endian>* object)
{
  if (this->got_mod_index_offset_ == -1U)
    {
      gold_assert(symtab != NULL && layout != NULL && object != NULL);
      Reloc_section* rela_dyn = this->rela_dyn_section(layout);
      Output_data_got<size, big_endian>* got
         = this->got_section(symtab, layout);
      unsigned int got_offset = got->add_constant(0);
      rela_dyn->add_local(object, 0,
                          size == 32 ? elfcpp::R_TILEGX_TLS_DTPMOD32
                                       : elfcpp::R_TILEGX_TLS_DTPMOD64, got,
                          got_offset, 0);
      got->add_constant(0);
      this->got_mod_index_offset_ = got_offset;
    }
  return this->got_mod_index_offset_;
}

// Optimize the TLS relocation type based on what we know about the
// symbol.  IS_FINAL is true if the final address of this symbol is
// known at link time.
//
// the transformation rules is described below:
//
//   compiler GD reference
//    |
//    V
//     moveli      tmp, hw1_last_tls_gd(x)     X0/X1
//     shl16insli  r0,  tmp, hw0_tls_gd(x)     X0/X1
//     addi        r0, got, tls_add(x)         Y0/Y1/X0/X1
//     jal         tls_gd_call(x)              X1
//     addi        adr, r0,  tls_gd_add(x)     Y0/Y1/X0/X1
//
//     linker tranformation of GD insn sequence
//      |
//      V
//      ==> GD:
//       moveli      tmp, hw1_last_tls_gd(x)     X0/X1
//       shl16insli  r0,  tmp, hw0_tls_gd(x)     X0/X1
//       add         r0,  got, r0                Y0/Y1/X0/X1
//       jal         plt(__tls_get_addr)         X1
//       move        adr, r0                     Y0/Y1/X0/X1
//      ==> IE:
//       moveli      tmp, hw1_last_tls_ie(x)     X0/X1
//       shl16insli  r0,  tmp, hw0_tls_ie(x)     X0/X1
//       add         r0,  got, r0                Y0/Y1/X0/X1
//       ld          r0,  r0                     X1
//       add         adr, r0, tp                 Y0/Y1/X0/X1
//      ==> LE:
//       moveli      tmp, hw1_last_tls_le(x)     X0/X1
//       shl16insli  r0,  tmp, hw0_tls_le(x)     X0/X1
//       move        r0,  r0                     Y0/Y1/X0/X1
//       move        r0,  r0                     Y0/Y1/X0/X1
//       add         adr, r0, tp                 Y0/Y1/X0/X1
//
//
//   compiler IE reference
//    |
//    V
//     moveli      tmp, hw1_last_tls_ie(x)     X0/X1
//     shl16insli  tmp, tmp, hw0_tls_ie(x)     X0/X1
//     addi        tmp, got, tls_add(x)        Y0/Y1/X0/X1
//     ld_tls      tmp, tmp, tls_ie_load(x)    X1
//     add         adr, tmp, tp                Y0/Y1/X0/X1
//
//     linker transformation for IE insn sequence
//      |
//      V
//      ==> IE:
//       moveli      tmp, hw1_last_tls_ie(x)     X0/X1
//       shl16insli  tmp, tmp, hw0_tls_ie(x)     X0/X1
//       add         tmp, got, tmp               Y0/Y1/X0/X1
//       ld          tmp, tmp                    X1
//       add         adr, tmp, tp                Y0/Y1/X0/X1
//      ==> LE:
//       moveli      tmp, hw1_last_tls_le(x)     X0/X1
//       shl16insli  tmp, tmp, hw0_tls_le(x)     X0/X1
//       move        tmp, tmp                    Y0/Y1/X0/X1
//       move        tmp, tmp                    Y0/Y1/X0/X1
//
//
//   compiler LE reference
//    |
//    V
//     moveli        tmp, hw1_last_tls_le(x)     X0/X1
//     shl16insli    tmp, tmp, hw0_tls_le(x)     X0/X1
//     add           adr, tmp, tp                Y0/Y1/X0/X1

template<int size, bool big_endian>
tls::Tls_optimization
Target_tilegx<size, big_endian>::optimize_tls_reloc(bool is_final, int r_type)
{
  // If we are generating a shared library, then we can't do anything
  // in the linker.
  if (parameters->options().shared())
    return tls::TLSOPT_NONE;

  switch (r_type)
    {
    // unique GD relocations
    case elfcpp::R_TILEGX_TLS_GD_CALL:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
      // These are General-Dynamic which permits fully general TLS
      // access.  Since we know that we are generating an executable,
      // we can convert this to Initial-Exec.  If we also know that
      // this is a local symbol, we can further switch to Local-Exec.
      if (is_final)
        return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_TO_IE;

    // unique IE relocations
    case elfcpp::R_TILEGX_TLS_IE_LOAD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
      // These are Initial-Exec relocs which get the thread offset
      // from the GOT.  If we know that we are linking against the
      // local symbol, we can switch to Local-Exec, which links the
      // thread offset into the instruction.
      if (is_final)
        return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_NONE;

    // could be created for both GD and IE
    // but they are expanded into the same
    // instruction in GD and IE.
    case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
      if (is_final)
        return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_NONE;

    // unique LE relocations
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
      // When we already have Local-Exec, there is nothing further we
      // can do.
      return tls::TLSOPT_NONE;

    default:
      gold_unreachable();
    }
}

// Get the Reference_flags for a particular relocation.

template<int size, bool big_endian>
int
Target_tilegx<size, big_endian>::Scan::get_reference_flags(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_TILEGX_NONE:
    case elfcpp::R_TILEGX_GNU_VTINHERIT:
    case elfcpp::R_TILEGX_GNU_VTENTRY:
      // No symbol reference.
      return 0;

    case elfcpp::R_TILEGX_64:
    case elfcpp::R_TILEGX_32:
    case elfcpp::R_TILEGX_16:
    case elfcpp::R_TILEGX_8:
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_TILEGX_BROFF_X1:
    case elfcpp::R_TILEGX_64_PCREL:
    case elfcpp::R_TILEGX_32_PCREL:
    case elfcpp::R_TILEGX_16_PCREL:
    case elfcpp::R_TILEGX_8_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
      return Symbol::RELATIVE_REF;

    case elfcpp::R_TILEGX_JUMPOFF_X1:
    case elfcpp::R_TILEGX_JUMPOFF_X1_PLT:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL:
      return Symbol::FUNCTION_CALL | Symbol::RELATIVE_REF;

    case elfcpp::R_TILEGX_IMM16_X0_HW0:
    case elfcpp::R_TILEGX_IMM16_X1_HW0:
    case elfcpp::R_TILEGX_IMM16_X0_HW1:
    case elfcpp::R_TILEGX_IMM16_X1_HW1:
    case elfcpp::R_TILEGX_IMM16_X0_HW2:
    case elfcpp::R_TILEGX_IMM16_X1_HW2:
    case elfcpp::R_TILEGX_IMM16_X0_HW3:
    case elfcpp::R_TILEGX_IMM16_X1_HW3:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST:
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_TILEGX_IMM16_X0_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_GOT:
      // Absolute in GOT.
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_TLS_DTPOFF64:
    case elfcpp::R_TILEGX_TLS_DTPMOD32:
    case elfcpp::R_TILEGX_TLS_DTPOFF32:
    case elfcpp::R_TILEGX_TLS_TPOFF32:
    case elfcpp::R_TILEGX_TLS_GD_CALL:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_TLS_IE_LOAD:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
      return Symbol::TLS_REF;

    case elfcpp::R_TILEGX_COPY:
    case elfcpp::R_TILEGX_GLOB_DAT:
    case elfcpp::R_TILEGX_JMP_SLOT:
    case elfcpp::R_TILEGX_RELATIVE:
    case elfcpp::R_TILEGX_TLS_TPOFF64:
    case elfcpp::R_TILEGX_TLS_DTPMOD64:
    default:
      // Not expected.  We will give an error later.
      return 0;
    }
}

// Report an unsupported relocation against a local symbol.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::Scan::unsupported_reloc_local(
     Sized_relobj_file<size, big_endian>* object,
     unsigned int r_type)
{
  gold_error(_("%s: unsupported reloc %u against local symbol"),
             object->name().c_str(), r_type);
}

// We are about to emit a dynamic relocation of type R_TYPE.  If the
// dynamic linker does not support it, issue an error.
template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::Scan::check_non_pic(Relobj* object,
                                                     unsigned int r_type)
{
  switch (r_type)
    {
      // These are the relocation types supported by glibc for tilegx
      // which should always work.
    case elfcpp::R_TILEGX_RELATIVE:
    case elfcpp::R_TILEGX_GLOB_DAT:
    case elfcpp::R_TILEGX_JMP_SLOT:
    case elfcpp::R_TILEGX_TLS_DTPMOD64:
    case elfcpp::R_TILEGX_TLS_DTPOFF64:
    case elfcpp::R_TILEGX_TLS_TPOFF64:
    case elfcpp::R_TILEGX_8:
    case elfcpp::R_TILEGX_16:
    case elfcpp::R_TILEGX_32:
    case elfcpp::R_TILEGX_64:
    case elfcpp::R_TILEGX_COPY:
    case elfcpp::R_TILEGX_IMM16_X0_HW0:
    case elfcpp::R_TILEGX_IMM16_X1_HW0:
    case elfcpp::R_TILEGX_IMM16_X0_HW1:
    case elfcpp::R_TILEGX_IMM16_X1_HW1:
    case elfcpp::R_TILEGX_IMM16_X0_HW2:
    case elfcpp::R_TILEGX_IMM16_X1_HW2:
    case elfcpp::R_TILEGX_IMM16_X0_HW3:
    case elfcpp::R_TILEGX_IMM16_X1_HW3:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST:
    case elfcpp::R_TILEGX_BROFF_X1:
    case elfcpp::R_TILEGX_JUMPOFF_X1:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
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

    case elfcpp::R_TILEGX_NONE:
      gold_unreachable();
    }
}

// Return whether we need to make a PLT entry for a relocation of the
// given type against a STT_GNU_IFUNC symbol.

template<int size, bool big_endian>
bool
Target_tilegx<size, big_endian>::Scan::reloc_needs_plt_for_ifunc(
     Sized_relobj_file<size, big_endian>* object, unsigned int r_type)
{
  int flags = Scan::get_reference_flags(r_type);
  if (flags & Symbol::TLS_REF)
    gold_error(_("%s: unsupported TLS reloc %u for IFUNC symbol"),
               object->name().c_str(), r_type);
  return flags != 0;
}

// Scan a relocation for a local symbol.

template<int size, bool big_endian>
inline void
Target_tilegx<size, big_endian>::Scan::local(Symbol_table* symtab,
                                 Layout* layout,
                                 Target_tilegx<size, big_endian>* target,
                                 Sized_relobj_file<size, big_endian>* object,
                                 unsigned int data_shndx,
                                 Output_section* output_section,
                                 const elfcpp::Rela<size, big_endian>& reloc,
                                 unsigned int r_type,
                                 const elfcpp::Sym<size, big_endian>& lsym,
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
    case elfcpp::R_TILEGX_NONE:
    case elfcpp::R_TILEGX_GNU_VTINHERIT:
    case elfcpp::R_TILEGX_GNU_VTENTRY:
      break;

    // If building a shared library (or a position-independent
    // executable), because the runtime address needs plus
    // the module base address, so generate a R_TILEGX_RELATIVE.
    case elfcpp::R_TILEGX_32:
    case elfcpp::R_TILEGX_64:
      if (parameters->options().output_is_position_independent())
        {
          unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
          Reloc_section* rela_dyn = target->rela_dyn_section(layout);
          rela_dyn->add_local_relative(object, r_sym,
                                       elfcpp::R_TILEGX_RELATIVE,
                                       output_section, data_shndx,
                                       reloc.get_r_offset(),
                                       reloc.get_r_addend(), is_ifunc);
        }
      break;

    // If building a shared library (or a position-independent
    // executable), we need to create a dynamic relocation for this
    // location.
    case elfcpp::R_TILEGX_8:
    case elfcpp::R_TILEGX_16:
    case elfcpp::R_TILEGX_IMM16_X0_HW0:
    case elfcpp::R_TILEGX_IMM16_X1_HW0:
    case elfcpp::R_TILEGX_IMM16_X0_HW1:
    case elfcpp::R_TILEGX_IMM16_X1_HW1:
    case elfcpp::R_TILEGX_IMM16_X0_HW2:
    case elfcpp::R_TILEGX_IMM16_X1_HW2:
    case elfcpp::R_TILEGX_IMM16_X0_HW3:
    case elfcpp::R_TILEGX_IMM16_X1_HW3:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST:
      if (parameters->options().output_is_position_independent())
        {
          this->check_non_pic(object, r_type);

          Reloc_section* rela_dyn = target->rela_dyn_section(layout);
          unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
          if (lsym.get_st_type() != elfcpp::STT_SECTION)
            rela_dyn->add_local(object, r_sym, r_type, output_section,
                                data_shndx, reloc.get_r_offset(),
                                reloc.get_r_addend());
          else
            {
              gold_assert(lsym.get_st_value() == 0);
              rela_dyn->add_symbolless_local_addend(object, r_sym, r_type,
                                                    output_section,
                                                    data_shndx,
                                                    reloc.get_r_offset(),
                                                    reloc.get_r_addend());

            }
        }
      break;

    // R_TILEGX_JUMPOFF_X1_PLT against local symbol
    // may happen for ifunc case.
    case elfcpp::R_TILEGX_JUMPOFF_X1_PLT:
    case elfcpp::R_TILEGX_JUMPOFF_X1:
    case elfcpp::R_TILEGX_64_PCREL:
    case elfcpp::R_TILEGX_32_PCREL:
    case elfcpp::R_TILEGX_16_PCREL:
    case elfcpp::R_TILEGX_8_PCREL:
    case elfcpp::R_TILEGX_BROFF_X1:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL:
      break;

    case elfcpp::R_TILEGX_IMM16_X0_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_GOT:
      {
        // The symbol requires a GOT entry.
        Output_data_got<size, big_endian>* got
           = target->got_section(symtab, layout);
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
            // tilegx dynamic linker will not update local got entry,
            // so, if we are generating a shared object, we need to add a
            // dynamic relocation for this symbol's GOT entry to inform
            // dynamic linker plus the load base explicitly.
            if (parameters->options().output_is_position_independent())
              {
               unsigned int got_offset
                  = object->local_got_offset(r_sym, GOT_TYPE_STANDARD);

                Reloc_section* rela_dyn = target->rela_dyn_section(layout);
                rela_dyn->add_local_relative(object, r_sym,
                                             r_type,
                                             got, got_offset, 0, is_ifunc);
              }
          }
      }
      break;

    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_TLS_GD_CALL:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_TLS_IE_LOAD:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
      {
         bool output_is_shared = parameters->options().shared();
         const tls::Tls_optimization opt_t =
          Target_tilegx<size, big_endian>::optimize_tls_reloc(
            !output_is_shared, r_type);

         switch (r_type)
           {
             case elfcpp::R_TILEGX_TLS_GD_CALL:
               // FIXME: predefine __tls_get_addr
               //
               // R_TILEGX_TLS_GD_CALL implicitly reference __tls_get_addr,
               // while all other target, x86/arm/mips/powerpc/sparc
               // generate tls relocation against __tls_get_addr explicitly,
               // so for TILEGX, we need the following hack.
               if (opt_t == tls::TLSOPT_NONE) {
                 if (!target->tls_get_addr_sym_defined_) {
                   Symbol* sym = NULL;
                   options::parse_set(NULL, "__tls_get_addr",
                                     (gold::options::String_set*)
                                     &parameters->options().undefined());
                   symtab->add_undefined_symbols_from_command_line(layout);
                   target->tls_get_addr_sym_defined_ = true;
                   sym = symtab->lookup("__tls_get_addr");
                   sym->set_in_reg();
                 }
                 target->make_plt_entry(symtab, layout,
                                        symtab->lookup("__tls_get_addr"));
               }
               break;

             // only make effect when applying relocation
             case elfcpp::R_TILEGX_TLS_IE_LOAD:
             case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
             case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
             case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
             case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
             case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
             case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
             case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
             case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
               break;

             // GD: requires two GOT entry for module index and offset
             // IE: requires one GOT entry for tp-relative offset
             // LE: shouldn't happen for global symbol
             case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_GD:
             case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_GD:
             case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
             case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
             case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
             case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
               {
                 if (opt_t == tls::TLSOPT_NONE) {
                   Output_data_got<size, big_endian> *got
                      = target->got_section(symtab, layout);
                   unsigned int r_sym
                      = elfcpp::elf_r_sym<size>(reloc.get_r_info());
                   unsigned int shndx = lsym.get_st_shndx();
                   bool is_ordinary;
                   shndx = object->adjust_sym_shndx(r_sym, shndx,
                                                    &is_ordinary);
                   if (!is_ordinary)
                     object->error(_("local symbol %u has bad shndx %u"),
                                   r_sym, shndx);
                   else
                     got->add_local_pair_with_rel(object, r_sym, shndx,
                                           GOT_TYPE_TLS_PAIR,
                                           target->rela_dyn_section(layout),
                                           size == 32
                                           ? elfcpp::R_TILEGX_TLS_DTPMOD32
                                           : elfcpp::R_TILEGX_TLS_DTPMOD64);
                  } else if (opt_t == tls::TLSOPT_TO_IE) {
                    Output_data_got<size, big_endian>* got
                       = target->got_section(symtab, layout);
                    Reloc_section* rela_dyn
                       = target->rela_dyn_section(layout);
                    unsigned int r_sym
                       = elfcpp::elf_r_sym<size>(reloc.get_r_info());
                    unsigned int off = got->add_constant(0);
                    object->set_local_got_offset(r_sym,
                                                 GOT_TYPE_TLS_OFFSET,off);
                    rela_dyn->add_symbolless_local_addend(object, r_sym,
                                            size == 32
                                            ? elfcpp::R_TILEGX_TLS_TPOFF32
                                            : elfcpp::R_TILEGX_TLS_TPOFF64,
                                            got, off, 0);
                  } else if (opt_t != tls::TLSOPT_TO_LE)
                    // only TO_LE is allowed for local symbol
                    unsupported_reloc_local(object, r_type);
               }
               break;

             // IE
             case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_IE:
             case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_IE:
             case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
             case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
             case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
             case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
               {
                 layout->set_has_static_tls();
                 if (opt_t == tls::TLSOPT_NONE) {
                   Output_data_got<size, big_endian>* got
                      = target->got_section(symtab, layout);
                   Reloc_section* rela_dyn
                      = target->rela_dyn_section(layout);
                   unsigned int r_sym
                      = elfcpp::elf_r_sym<size>(reloc.get_r_info());
                   unsigned int off = got->add_constant(0);
                   object->set_local_got_offset(r_sym,
                                                GOT_TYPE_TLS_OFFSET, off);
                   rela_dyn->add_symbolless_local_addend(object, r_sym,
                                            size == 32
                                            ? elfcpp::R_TILEGX_TLS_TPOFF32
                                            : elfcpp::R_TILEGX_TLS_TPOFF64,
                                            got, off, 0);
                 } else if (opt_t != tls::TLSOPT_TO_LE)
                   unsupported_reloc_local(object, r_type);
               }
               break;

             // LE
             case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_LE:
             case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_LE:
             case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
             case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
             case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
             case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
               layout->set_has_static_tls();
               if (parameters->options().shared()) {
                 // defer to dynamic linker
                 gold_assert(lsym.get_st_type() != elfcpp::STT_SECTION);
                 unsigned int r_sym
                    = elfcpp::elf_r_sym<size>(reloc.get_r_info());
                 Reloc_section* rela_dyn = target->rela_dyn_section(layout);
                 rela_dyn->add_symbolless_local_addend(object, r_sym, r_type,
                                                  output_section, data_shndx,
                                                  reloc.get_r_offset(), 0);
               }
               break;

             default:
               gold_unreachable();
           }
      }
      break;

    case elfcpp::R_TILEGX_COPY:
    case elfcpp::R_TILEGX_GLOB_DAT:
    case elfcpp::R_TILEGX_JMP_SLOT:
    case elfcpp::R_TILEGX_RELATIVE:
      // These are outstanding tls relocs, which are unexpected when linking
    case elfcpp::R_TILEGX_TLS_TPOFF32:
    case elfcpp::R_TILEGX_TLS_TPOFF64:
    case elfcpp::R_TILEGX_TLS_DTPMOD32:
    case elfcpp::R_TILEGX_TLS_DTPMOD64:
    case elfcpp::R_TILEGX_TLS_DTPOFF32:
    case elfcpp::R_TILEGX_TLS_DTPOFF64:
      gold_error(_("%s: unexpected reloc %u in object file"),
                 object->name().c_str(), r_type);
      break;

    default:
      gold_error(_("%s: unsupported reloc %u against local symbol"),
                 object->name().c_str(), r_type);
      break;
    }
}


// Report an unsupported relocation against a global symbol.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::Scan::unsupported_reloc_global(
    Sized_relobj_file<size, big_endian>* object,
    unsigned int r_type,
    Symbol* gsym)
{
  gold_error(_("%s: unsupported reloc %u against global symbol %s"),
             object->name().c_str(), r_type, gsym->demangled_name().c_str());
}

// Returns true if this relocation type could be that of a function pointer.
template<int size, bool big_endian>
inline bool
Target_tilegx<size, big_endian>::Scan::possible_function_pointer_reloc(
  unsigned int r_type)
{
  switch (r_type)
    {
      case elfcpp::R_TILEGX_IMM16_X0_HW0:
      case elfcpp::R_TILEGX_IMM16_X1_HW0:
      case elfcpp::R_TILEGX_IMM16_X0_HW1:
      case elfcpp::R_TILEGX_IMM16_X1_HW1:
      case elfcpp::R_TILEGX_IMM16_X0_HW2:
      case elfcpp::R_TILEGX_IMM16_X1_HW2:
      case elfcpp::R_TILEGX_IMM16_X0_HW3:
      case elfcpp::R_TILEGX_IMM16_X1_HW3:
      case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST:
      case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST:
      case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST:
      case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST:
      case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST:
      case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST:
      case elfcpp::R_TILEGX_IMM16_X0_HW0_PCREL:
      case elfcpp::R_TILEGX_IMM16_X1_HW0_PCREL:
      case elfcpp::R_TILEGX_IMM16_X0_HW1_PCREL:
      case elfcpp::R_TILEGX_IMM16_X1_HW1_PCREL:
      case elfcpp::R_TILEGX_IMM16_X0_HW2_PCREL:
      case elfcpp::R_TILEGX_IMM16_X1_HW2_PCREL:
      case elfcpp::R_TILEGX_IMM16_X0_HW3_PCREL:
      case elfcpp::R_TILEGX_IMM16_X1_HW3_PCREL:
      case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
      case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
      case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
      case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
      case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
      case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
      case elfcpp::R_TILEGX_IMM16_X0_HW0_GOT:
      case elfcpp::R_TILEGX_IMM16_X1_HW0_GOT:
      case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_GOT:
      case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_GOT:
      case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_GOT:
      case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_GOT:
      {
        return true;
      }
    }
  return false;
}

// For safe ICF, scan a relocation for a local symbol to check if it
// corresponds to a function pointer being taken.  In that case mark
// the function whose pointer was taken as not foldable.

template<int size, bool big_endian>
inline bool
Target_tilegx<size, big_endian>::Scan::local_reloc_may_be_function_pointer(
  Symbol_table* ,
  Layout* ,
  Target_tilegx<size, big_endian>* ,
  Sized_relobj_file<size, big_endian>* ,
  unsigned int ,
  Output_section* ,
  const elfcpp::Rela<size, big_endian>& ,
  unsigned int r_type,
  const elfcpp::Sym<size, big_endian>&)
{
  return possible_function_pointer_reloc(r_type);
}

// For safe ICF, scan a relocation for a global symbol to check if it
// corresponds to a function pointer being taken.  In that case mark
// the function whose pointer was taken as not foldable.

template<int size, bool big_endian>
inline bool
Target_tilegx<size, big_endian>::Scan::global_reloc_may_be_function_pointer(
  Symbol_table*,
  Layout* ,
  Target_tilegx<size, big_endian>* ,
  Sized_relobj_file<size, big_endian>* ,
  unsigned int ,
  Output_section* ,
  const elfcpp::Rela<size, big_endian>& ,
  unsigned int r_type,
  Symbol* gsym)
{
  // GOT is not a function.
  if (strcmp(gsym->name(), "_GLOBAL_OFFSET_TABLE_") == 0)
    return false;

  // When building a shared library, do not fold symbols whose visibility
  // is hidden, internal or protected.
  return ((parameters->options().shared()
           && (gsym->visibility() == elfcpp::STV_INTERNAL
               || gsym->visibility() == elfcpp::STV_PROTECTED
               || gsym->visibility() == elfcpp::STV_HIDDEN))
          || possible_function_pointer_reloc(r_type));
}

// Scan a relocation for a global symbol.

template<int size, bool big_endian>
inline void
Target_tilegx<size, big_endian>::Scan::global(Symbol_table* symtab,
                            Layout* layout,
                            Target_tilegx<size, big_endian>* target,
                            Sized_relobj_file<size, big_endian>* object,
                            unsigned int data_shndx,
                            Output_section* output_section,
                            const elfcpp::Rela<size, big_endian>& reloc,
                            unsigned int r_type,
                            Symbol* gsym)
{
  // A reference to _GLOBAL_OFFSET_TABLE_ implies that we need a got
  // section.  We check here to avoid creating a dynamic reloc against
  // _GLOBAL_OFFSET_TABLE_.
  if (!target->has_got_section()
      && strcmp(gsym->name(), "_GLOBAL_OFFSET_TABLE_") == 0)
    target->got_section(symtab, layout);

  // A STT_GNU_IFUNC symbol may require a PLT entry.
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && this->reloc_needs_plt_for_ifunc(object, r_type))
    target->make_plt_entry(symtab, layout, gsym);

  switch (r_type)
    {
    case elfcpp::R_TILEGX_NONE:
    case elfcpp::R_TILEGX_GNU_VTINHERIT:
    case elfcpp::R_TILEGX_GNU_VTENTRY:
      break;

    case elfcpp::R_TILEGX_DEST_IMM8_X1:
    case elfcpp::R_TILEGX_IMM16_X0_HW0:
    case elfcpp::R_TILEGX_IMM16_X1_HW0:
    case elfcpp::R_TILEGX_IMM16_X0_HW1:
    case elfcpp::R_TILEGX_IMM16_X1_HW1:
    case elfcpp::R_TILEGX_IMM16_X0_HW2:
    case elfcpp::R_TILEGX_IMM16_X1_HW2:
    case elfcpp::R_TILEGX_IMM16_X0_HW3:
    case elfcpp::R_TILEGX_IMM16_X1_HW3:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST:
    case elfcpp::R_TILEGX_64:
    case elfcpp::R_TILEGX_32:
    case elfcpp::R_TILEGX_16:
    case elfcpp::R_TILEGX_8:
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
            else if (((size == 64 && r_type == elfcpp::R_TILEGX_64)
                      || (size == 32 && r_type == elfcpp::R_TILEGX_32))
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
                unsigned int r_type = elfcpp::R_TILEGX_IRELATIVE;
                rela_dyn->add_symbolless_global_addend(gsym, r_type,
                                                   output_section, object,
                                                   data_shndx,
                                                   reloc.get_r_offset(),
                                                   reloc.get_r_addend());
              } else if ((r_type == elfcpp::R_TILEGX_64
                          || r_type == elfcpp::R_TILEGX_32)
                         && gsym->can_use_relative_reloc(false))
              {
                Reloc_section* rela_dyn = target->rela_dyn_section(layout);
                rela_dyn->add_global_relative(gsym, elfcpp::R_TILEGX_RELATIVE,
                                              output_section, object,
                                              data_shndx,
                                              reloc.get_r_offset(),
                                              reloc.get_r_addend(), false);
              }
            else
              {
                this->check_non_pic(object, r_type);
                Reloc_section* rela_dyn = target->rela_dyn_section(layout);
                rela_dyn->add_global(gsym, r_type, output_section, object,
                                     data_shndx, reloc.get_r_offset(),
                                     reloc.get_r_addend());
              }
          }
      }
      break;

    case elfcpp::R_TILEGX_BROFF_X1:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
    case elfcpp::R_TILEGX_64_PCREL:
    case elfcpp::R_TILEGX_32_PCREL:
    case elfcpp::R_TILEGX_16_PCREL:
    case elfcpp::R_TILEGX_8_PCREL:
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
                this->check_non_pic(object, r_type);
                Reloc_section* rela_dyn = target->rela_dyn_section(layout);
                rela_dyn->add_global(gsym, r_type, output_section, object,
                                     data_shndx, reloc.get_r_offset(),
                                     reloc.get_r_addend());
              }
          }
      }
      break;

    case elfcpp::R_TILEGX_IMM16_X0_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_GOT:
      {
        // The symbol requires a GOT entry.
        Output_data_got<size, big_endian>* got
           = target->got_section(symtab, layout);
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
                                       elfcpp::R_TILEGX_GLOB_DAT);
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
                                                  r_type,
                                                  got, got_off, 0, false);
                  }
              }
          }
      }
      break;

    // a minor difference here for R_TILEGX_JUMPOFF_X1
    // between bfd linker and gold linker for gold, when
    // R_TILEGX_JUMPOFF_X1 against global symbol, we
    // turn it into JUMPOFF_X1_PLT, otherwise the distance
    // to the symbol function may overflow at runtime.
    case elfcpp::R_TILEGX_JUMPOFF_X1:

    case elfcpp::R_TILEGX_JUMPOFF_X1_PLT:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL:
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


    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_TLS_GD_CALL:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_TLS_IE_LOAD:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
      {
         const bool is_final = gsym->final_value_is_known();
         const tls::Tls_optimization opt_t =
          Target_tilegx<size, big_endian>::optimize_tls_reloc(is_final,
                                                              r_type);

         switch (r_type)
           {
              // only expand to plt against __tls_get_addr in GD model
              case elfcpp::R_TILEGX_TLS_GD_CALL:
                if (opt_t == tls::TLSOPT_NONE) {
                  // FIXME:  it's better '__tls_get_addr' referenced explicitly
                  if (!target->tls_get_addr_sym_defined_) {
                    Symbol* sym = NULL;
                    options::parse_set(NULL, "__tls_get_addr",
                                       (gold::options::String_set*)
                                       &parameters->options().undefined());
                    symtab->add_undefined_symbols_from_command_line(layout);
                    target->tls_get_addr_sym_defined_ = true;
                    sym = symtab->lookup("__tls_get_addr");
                    sym->set_in_reg();
                  }
                  target->make_plt_entry(symtab, layout,
                                         symtab->lookup("__tls_get_addr"));
                }
                break;

              // only make effect when applying relocation
              case elfcpp::R_TILEGX_TLS_IE_LOAD:
              case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
              case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
              case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
              case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
              case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
              case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
              case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
              case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
                break;

              // GD: requires two GOT entry for module index and offset
              // IE: requires one GOT entry for tp-relative offset
              // LE: shouldn't happen for global symbol
              case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_GD:
              case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_GD:
              case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
              case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
              case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
              case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
                {
                  if (opt_t == tls::TLSOPT_NONE) {
                      Output_data_got<size, big_endian>* got
                        = target->got_section(symtab, layout);
                      got->add_global_pair_with_rel(gsym, GOT_TYPE_TLS_PAIR,
                                             target->rela_dyn_section(layout),
                                             size == 32
                                           ? elfcpp::R_TILEGX_TLS_DTPMOD32
                                           : elfcpp::R_TILEGX_TLS_DTPMOD64,
                                             size == 32
                                           ? elfcpp::R_TILEGX_TLS_DTPOFF32
                                           : elfcpp::R_TILEGX_TLS_DTPOFF64);
                  } else if (opt_t == tls::TLSOPT_TO_IE) {
                    // Create a GOT entry for the tp-relative offset.
                    Output_data_got<size, big_endian>* got
                       = target->got_section(symtab, layout);
                    got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
                                           target->rela_dyn_section(layout),
                                           size == 32
                                           ? elfcpp::R_TILEGX_TLS_TPOFF32
                                           : elfcpp::R_TILEGX_TLS_TPOFF64);
                  } else if (opt_t != tls::TLSOPT_TO_LE)
                    // exteranl symbol should not be optimized to TO_LE
                    unsupported_reloc_global(object, r_type, gsym);
                }
                break;

              // IE
              case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_IE:
              case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_IE:
              case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
              case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
              case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
              case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
                {
                    layout->set_has_static_tls();
                  if (opt_t == tls::TLSOPT_NONE) {
                    // Create a GOT entry for the tp-relative offset.
                    Output_data_got<size, big_endian>* got
                       = target->got_section(symtab, layout);
                    got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
                                           target->rela_dyn_section(layout),
                                           size == 32
                                           ? elfcpp::R_TILEGX_TLS_TPOFF32
                                           : elfcpp::R_TILEGX_TLS_TPOFF64);
                  } else if (opt_t != tls::TLSOPT_TO_LE)
                    unsupported_reloc_global(object, r_type, gsym);
                }
                break;

              // LE
              case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_LE:
              case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_LE:
              case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
              case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
              case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
              case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
                  layout->set_has_static_tls();
                if (parameters->options().shared()) {
                  // defer to dynamic linker
                  Reloc_section* rela_dyn = target->rela_dyn_section(layout);
                  rela_dyn->add_symbolless_global_addend(gsym, r_type,
                                                      output_section, object,
                                                      data_shndx,
                                                      reloc.get_r_offset(), 0);
                  }
                break;

              default:
                gold_unreachable();
           }
      }
      break;

    // below are outstanding relocs
    // should not existed in static linking stage
    case elfcpp::R_TILEGX_COPY:
    case elfcpp::R_TILEGX_GLOB_DAT:
    case elfcpp::R_TILEGX_JMP_SLOT:
    case elfcpp::R_TILEGX_RELATIVE:
    case elfcpp::R_TILEGX_TLS_TPOFF32:
    case elfcpp::R_TILEGX_TLS_TPOFF64:
    case elfcpp::R_TILEGX_TLS_DTPMOD32:
    case elfcpp::R_TILEGX_TLS_DTPMOD64:
    case elfcpp::R_TILEGX_TLS_DTPOFF32:
    case elfcpp::R_TILEGX_TLS_DTPOFF64:
      gold_error(_("%s: unexpected reloc %u in object file"),
                 object->name().c_str(), r_type);
      break;

    default:
      gold_error(_("%s: unsupported reloc %u against global symbol %s"),
                 object->name().c_str(), r_type,
                 gsym->demangled_name().c_str());
      break;
    }
}

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::gc_process_relocs(Symbol_table* symtab,
                                  Layout* layout,
                                  Sized_relobj_file<size, big_endian>* object,
                                  unsigned int data_shndx,
                                  unsigned int sh_type,
                                  const unsigned char* prelocs,
                                  size_t reloc_count,
                                  Output_section* output_section,
                                  bool needs_special_offset_handling,
                                  size_t local_symbol_count,
                                  const unsigned char* plocal_symbols)
{
  typedef Target_tilegx<size, big_endian> Tilegx;
  typedef typename Target_tilegx<size, big_endian>::Scan Scan;
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;

  if (sh_type == elfcpp::SHT_REL)
    {
      return;
    }

   gold::gc_process_relocs<size, big_endian, Tilegx, Scan, Classify_reloc>(
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

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::scan_relocs(Symbol_table* symtab,
                                 Layout* layout,
                                 Sized_relobj_file<size, big_endian>* object,
                                 unsigned int data_shndx,
                                 unsigned int sh_type,
                                 const unsigned char* prelocs,
                                 size_t reloc_count,
                                 Output_section* output_section,
                                 bool needs_special_offset_handling,
                                 size_t local_symbol_count,
                                 const unsigned char* plocal_symbols)
{
  typedef Target_tilegx<size, big_endian> Tilegx;
  typedef typename Target_tilegx<size, big_endian>::Scan Scan;
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;

  if (sh_type == elfcpp::SHT_REL)
    {
      gold_error(_("%s: unsupported REL reloc section"),
                 object->name().c_str());
      return;
    }

  gold::scan_relocs<size, big_endian, Tilegx, Scan, Classify_reloc>(
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

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::do_define_standard_symbols(
    Symbol_table* symtab,
    Layout* layout)
{
  Output_section* feedback_section = layout->find_output_section(".feedback");

  if (feedback_section != NULL)
    {
      symtab->define_in_output_data("__feedback_section_end",
                    NULL,
                    Symbol_table::PREDEFINED,
                    feedback_section,
                    0,
                    0,
                    elfcpp::STT_NOTYPE,
                    elfcpp::STB_GLOBAL,
                    elfcpp::STV_HIDDEN,
                    0,
                    true, // offset_is_from_end
                    false);
    }
}

// Finalize the sections.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::do_finalize_sections(
    Layout* layout,
    const Input_objects*,
    Symbol_table* symtab)
{
  const Reloc_section* rel_plt = (this->plt_ == NULL
                                  ? NULL
                                  : this->plt_->rela_plt());
  layout->add_target_dynamic_tags(false, this->got_plt_, rel_plt,
				  this->rela_dyn_, true, true, false);

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

      // If the .got section is more than 0x8000 bytes, we add
      // 0x8000 to the value of _GLOBAL_OFFSET_TABLE_, so that 16
      // bit relocations have a greater chance of working.
      if (data_size >= 0x8000)
        symtab->get_sized_symbol<size>(sym)->set_value(
          symtab->get_sized_symbol<size>(sym)->value() + 0x8000);
    }

  if (parameters->doing_static_link()
      && (this->plt_ == NULL || !this->plt_->has_irelative_section()))
    {
      // If linking statically, make sure that the __rela_iplt symbols
      // were defined if necessary, even if we didn't create a PLT.
      static const Define_symbol_in_segment syms[] =
        {
          {
            "__rela_iplt_start",        // name
            elfcpp::PT_LOAD,            // segment_type
            elfcpp::PF_W,               // segment_flags_set
            elfcpp::PF(0),              // segment_flags_clear
            0,                          // value
            0,                          // size
            elfcpp::STT_NOTYPE,         // type
            elfcpp::STB_GLOBAL,         // binding
            elfcpp::STV_HIDDEN,         // visibility
            0,                          // nonvis
            Symbol::SEGMENT_START,      // offset_from_base
            true                        // only_if_ref
          },
          {
            "__rela_iplt_end",          // name
            elfcpp::PT_LOAD,            // segment_type
            elfcpp::PF_W,               // segment_flags_set
            elfcpp::PF(0),              // segment_flags_clear
            0,                          // value
            0,                          // size
            elfcpp::STT_NOTYPE,         // type
            elfcpp::STB_GLOBAL,         // binding
            elfcpp::STV_HIDDEN,         // visibility
            0,                          // nonvis
            Symbol::SEGMENT_START,      // offset_from_base
            true                        // only_if_ref
          }
        };

      symtab->define_symbols(layout, 2, syms,
                             layout->script_options()->saw_sections_clause());
    }
}

// Perform a relocation.

template<int size, bool big_endian>
inline bool
Target_tilegx<size, big_endian>::Relocate::relocate(
    const Relocate_info<size, big_endian>* relinfo,
    unsigned int,
    Target_tilegx<size, big_endian>* target,
    Output_section*,
    size_t relnum,
    const unsigned char* preloc,
    const Sized_symbol<size>* gsym,
    const Symbol_value<size>* psymval,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type)
{
  if (view == NULL)
    return true;

  typedef Tilegx_relocate_functions<size, big_endian> TilegxReloc;
  typename TilegxReloc::Tilegx_howto r_howto;

  const elfcpp::Rela<size, big_endian> rela(preloc);
  unsigned int r_type = elfcpp::elf_r_type<size>(rela.get_r_info());
  const Sized_relobj_file<size, big_endian>* object = relinfo->object;

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

  elfcpp::Elf_Xword addend = rela.get_r_addend();

  // Get the GOT offset if needed.
  // For tilegx, the GOT pointer points to the start of the GOT section.
  bool have_got_offset = false;
  int got_offset = 0;
  int got_base = target->got_ != NULL
                 ? target->got_->current_data_size() >= 0x8000 ? 0x8000 : 0
                 : 0;
  unsigned int got_type = GOT_TYPE_STANDARD;
  bool always_apply_relocation = false;
  switch (r_type)
    {
    case elfcpp::R_TILEGX_IMM16_X0_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_GOT:
      if (gsym != NULL)
        {
          gold_assert(gsym->has_got_offset(got_type));
          got_offset = gsym->got_offset(got_type) - got_base;
        }
      else
        {
          unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
          gold_assert(object->local_has_got_offset(r_sym, got_type));
          got_offset =
            object->local_got_offset(r_sym, got_type) - got_base;
        }
      have_got_offset = true;
      break;

    default:
      break;
    }

  r_howto = TilegxReloc::howto[r_type];
  switch (r_type)
    {
    case elfcpp::R_TILEGX_NONE:
    case elfcpp::R_TILEGX_GNU_VTINHERIT:
    case elfcpp::R_TILEGX_GNU_VTENTRY:
      break;

    case elfcpp::R_TILEGX_IMM16_X0_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_GOT:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_GOT:
      gold_assert(have_got_offset);
      symval.set_output_value(got_offset);
      psymval = &symval;
      always_apply_relocation = true;
      addend = 0;
      // Fall through.

    // when under PIC mode, these relocations are deferred to rtld
    case elfcpp::R_TILEGX_IMM16_X0_HW0:
    case elfcpp::R_TILEGX_IMM16_X1_HW0:
    case elfcpp::R_TILEGX_IMM16_X0_HW1:
    case elfcpp::R_TILEGX_IMM16_X1_HW1:
    case elfcpp::R_TILEGX_IMM16_X0_HW2:
    case elfcpp::R_TILEGX_IMM16_X1_HW2:
    case elfcpp::R_TILEGX_IMM16_X0_HW3:
    case elfcpp::R_TILEGX_IMM16_X1_HW3:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST:
      if (always_apply_relocation
          || !parameters->options().output_is_position_independent())
        TilegxReloc::imm_x_general(view, object, psymval, addend, r_howto);
      break;

    case elfcpp::R_TILEGX_JUMPOFF_X1:
    case elfcpp::R_TILEGX_JUMPOFF_X1_PLT:
      gold_assert(gsym == NULL
                  || gsym->has_plt_offset()
                  || gsym->final_value_is_known()
                  || (gsym->is_defined()
                      && !gsym->is_from_dynobj()
                      && !gsym->is_preemptible()));
      TilegxReloc::imm_x_pcrel_general(view, object, psymval, addend,
                                       address, r_howto);
      break;


    case elfcpp::R_TILEGX_IMM16_X0_HW0_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW3_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL:
    case elfcpp::R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
      TilegxReloc::imm_x_pcrel_general(view, object, psymval, addend,
                                       address, r_howto);
      break;

    case elfcpp::R_TILEGX_BROFF_X1:
    case elfcpp::R_TILEGX_DEST_IMM8_X1:
      TilegxReloc::imm_x_two_part_general(view, object, psymval,
                                          addend, address, r_type);
      break;


    // below are general relocation types, which can be
    // handled by target-independent handlers
    case elfcpp::R_TILEGX_64:
      TilegxReloc::abs64(view, object, psymval, addend);
      break;

    case elfcpp::R_TILEGX_64_PCREL:
      TilegxReloc::pc_abs64(view, object, psymval, addend, address);
      break;

    case elfcpp::R_TILEGX_32:
      TilegxReloc::abs32(view, object, psymval, addend);
      break;

    case elfcpp::R_TILEGX_32_PCREL:
      TilegxReloc::pc_abs32(view, object, psymval, addend, address);
      break;

    case elfcpp::R_TILEGX_16:
      TilegxReloc::abs16(view, object, psymval, addend);
      break;

    case elfcpp::R_TILEGX_16_PCREL:
      TilegxReloc::pc_abs16(view, object, psymval, addend, address);
      break;

    case elfcpp::R_TILEGX_8:
      Relocate_functions<size, big_endian>::rela8(view, object,
                                                  psymval, addend);
      break;

    case elfcpp::R_TILEGX_8_PCREL:
      Relocate_functions<size, big_endian>::pcrela8(view, object,
                                                    psymval, addend, address);
      break;

    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
    case elfcpp::R_TILEGX_TLS_GD_CALL:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
    case elfcpp::R_TILEGX_TLS_IE_LOAD:
    case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
    case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
      {
        const bool is_final = (gsym == NULL
                               ? !parameters->options().shared()
                               : gsym->final_value_is_known());
        tls::Tls_optimization opt_t =
          Target_tilegx<size, big_endian>::optimize_tls_reloc(is_final,
                                                              r_type);

        switch (r_type)
          {

            case elfcpp::R_TILEGX_TLS_GD_CALL:
              {
                if (opt_t == tls::TLSOPT_NONE) {
                  Symbol *tls_sym = relinfo->symtab->lookup("__tls_get_addr");
                  symval.set_output_value(
                    target->plt_address_for_global(tls_sym));
                  psymval = &symval;
                  TilegxReloc::imm_x_pcrel_general(view, object, psymval,
                                                   addend, address, r_howto);
                }
                else if (opt_t == tls::TLSOPT_TO_IE
                         || opt_t == tls::TLSOPT_TO_LE)
                  TilegxReloc::tls_relax(view, r_type, opt_t);
              }
              break;

            // XX_TLS_GD is the same as normal X_GOT relocation
            // except allocating a got entry pair,
            case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_GD:
            case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_GD:
            case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
            case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
            case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
            case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
              if (opt_t == tls::TLSOPT_NONE) {
                got_type = GOT_TYPE_TLS_PAIR;
                have_got_offset = true;
              } else if (opt_t == tls::TLSOPT_TO_IE) {
                got_type = GOT_TYPE_TLS_OFFSET;
                have_got_offset = true;
              }
              goto do_update_value;
            // XX_TLS_IE is the same as normal X_GOT relocation
            // except allocating one additional runtime relocation
            case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_IE:
            case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_IE:
            case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
            case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
            case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
            case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
              if (opt_t == tls::TLSOPT_NONE) {
                got_type = GOT_TYPE_TLS_OFFSET;
                have_got_offset = true;
              }
	      // Fall through.
            do_update_value:
              if (have_got_offset) {
                if (gsym != NULL) {
                  gold_assert(gsym->has_got_offset(got_type));
                  got_offset = gsym->got_offset(got_type) - got_base;
                } else {
                  unsigned int r_sym
                     = elfcpp::elf_r_sym<size>(rela.get_r_info());
                  gold_assert(object->local_has_got_offset(r_sym, got_type));
                  got_offset =
                    object->local_got_offset(r_sym, got_type) - got_base;
                }
              }

              if (opt_t == tls::TLSOPT_NONE
                  || opt_t == tls::TLSOPT_TO_IE) {
                // for both GD/IE, these relocations
                // actually calculate got offset, so
                // there behavior are the same
                gold_assert(have_got_offset);
                symval.set_output_value(got_offset);
                psymval = &symval;
                addend = 0;
                TilegxReloc::imm_x_general(view, object, psymval,
                                           addend, r_howto);
                break;
              } // else if (opt_t == tls::TLSOPT_TO_LE)
                //   both GD/IE are turned into LE, which
                //   is absolute relocation.
                // Fall through.

            // LE
            //
            // tp
            // |
            // V
            //  t_var1 | t_var2 | t_var3 | ...
            //  --------------------------------------------------
            //
            //  so offset to tp should be negative, we get offset
            //  from the following formular for LE
            //
            //    t_var1_off = t_var1_sym_value - tls_section_start
            //
            case elfcpp::R_TILEGX_IMM16_X0_HW0_TLS_LE:
            case elfcpp::R_TILEGX_IMM16_X1_HW0_TLS_LE:
            case elfcpp::R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
            case elfcpp::R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
            case elfcpp::R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
            case elfcpp::R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
              {
                Output_segment *tls_segment = relinfo->layout->tls_segment();
                if (tls_segment == NULL) {
                  gold_assert(parameters->errors()->error_count() > 0
                              || issue_undefined_symbol_error(gsym));
                  return false;
                }

                typename elfcpp::Elf_types<size>::Elf_Addr value
                  = psymval->value(relinfo->object, 0);
                symval.set_output_value(value);
                psymval = &symval;
                TilegxReloc::imm_x_general(view, object, psymval,
                                           addend, r_howto);
              }
              break;

            // tls relaxation
            case elfcpp::R_TILEGX_TLS_IE_LOAD:
            case elfcpp::R_TILEGX_IMM8_X0_TLS_ADD:
            case elfcpp::R_TILEGX_IMM8_X1_TLS_ADD:
            case elfcpp::R_TILEGX_IMM8_Y0_TLS_ADD:
            case elfcpp::R_TILEGX_IMM8_Y1_TLS_ADD:
            case elfcpp::R_TILEGX_IMM8_X0_TLS_GD_ADD:
            case elfcpp::R_TILEGX_IMM8_X1_TLS_GD_ADD:
            case elfcpp::R_TILEGX_IMM8_Y0_TLS_GD_ADD:
            case elfcpp::R_TILEGX_IMM8_Y1_TLS_GD_ADD:
              TilegxReloc::tls_relax(view, r_type, opt_t);
              break;

            default:
              gold_unreachable();
          }
      }
      break;

    // below are outstanding relocs
    // should not existed in static linking stage
    case elfcpp::R_TILEGX_COPY:
    case elfcpp::R_TILEGX_GLOB_DAT:
    case elfcpp::R_TILEGX_JMP_SLOT:
    case elfcpp::R_TILEGX_RELATIVE:
    case elfcpp::R_TILEGX_TLS_TPOFF32:
    case elfcpp::R_TILEGX_TLS_TPOFF64:
    case elfcpp::R_TILEGX_TLS_DTPMOD32:
    case elfcpp::R_TILEGX_TLS_DTPMOD64:
    case elfcpp::R_TILEGX_TLS_DTPOFF32:
    case elfcpp::R_TILEGX_TLS_DTPOFF64:
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

  return true;
}

// Relocate section data.

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::relocate_section(
    const Relocate_info<size, big_endian>* relinfo,
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
  typedef Target_tilegx<size, big_endian> Tilegx;
  typedef typename Target_tilegx<size, big_endian>::Relocate Tilegx_relocate;
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::relocate_section<size, big_endian, Tilegx, Tilegx_relocate,
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

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::apply_relocation(
    const Relocate_info<size, big_endian>* relinfo,
    typename elfcpp::Elf_types<size>::Elf_Addr r_offset,
    unsigned int r_type,
    typename elfcpp::Elf_types<size>::Elf_Swxword r_addend,
    const Symbol* gsym,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr address,
    section_size_type view_size)
{
  gold::apply_relocation<size, big_endian, Target_tilegx<size, big_endian>,
                         typename Target_tilegx<size, big_endian>::Relocate>(
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

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::scan_relocatable_relocs(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, big_endian>* object,
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
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;
  typedef gold::Default_scan_relocatable_relocs<Classify_reloc>
      Scan_relocatable_relocs;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::scan_relocatable_relocs<size, big_endian, Scan_relocatable_relocs>(
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

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::emit_relocs_scan(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, big_endian>* object,
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
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;
  typedef gold::Default_emit_relocs_strategy<Classify_reloc>
      Emit_relocs_strategy;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::scan_relocatable_relocs<size, big_endian, Emit_relocs_strategy>(
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

template<int size, bool big_endian>
void
Target_tilegx<size, big_endian>::relocate_relocs(
    const Relocate_info<size, big_endian>* relinfo,
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
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::relocate_relocs<size, big_endian, Classify_reloc>(
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

template<int size, bool big_endian>
uint64_t
Target_tilegx<size, big_endian>::do_dynsym_value(const Symbol* gsym) const
{
  gold_assert(gsym->is_from_dynobj() && gsym->has_plt_offset());
  return this->plt_address_for_global(gsym);
}

// Return the value to use for the base of a DW_EH_PE_datarel offset
// in an FDE.  Solaris and SVR4 use DW_EH_PE_datarel because their
// assembler can not write out the difference between two labels in
// different sections, so instead of using a pc-relative value they
// use an offset from the GOT.

template<int size, bool big_endian>
uint64_t
Target_tilegx<size, big_endian>::do_ehframe_datarel_base() const
{
  gold_assert(this->global_offset_table_ != NULL);
  Symbol* sym = this->global_offset_table_;
  Sized_symbol<size>* ssym = static_cast<Sized_symbol<size>*>(sym);
  return ssym->value();
}

// The selector for tilegx object files.

template<int size, bool big_endian>
class Target_selector_tilegx : public Target_selector
{
public:
  Target_selector_tilegx()
    : Target_selector(elfcpp::EM_TILEGX, size, big_endian,
                      (size == 64
                       ? (big_endian ? "elf64-tilegx-be" : "elf64-tilegx-le")
                          : (big_endian ? "elf32-tilegx-be"
                                           : "elf32-tilegx-le")),
                      (size == 64
                       ? (big_endian ? "elf64tilegx_be" : "elf64tilegx")
                          : (big_endian ? "elf32tilegx_be" : "elf32tilegx")))
  { }

  Target*
  do_instantiate_target()
  { return new Target_tilegx<size, big_endian>(); }

};

Target_selector_tilegx<64, false> target_selector_tilegx64_le;
Target_selector_tilegx<32, false> target_selector_tilegx32_le;
Target_selector_tilegx<64, true> target_selector_tilegx64_be;
Target_selector_tilegx<32, true> target_selector_tilegx32_be;
} // End anonymous namespace.
