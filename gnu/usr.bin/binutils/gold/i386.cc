// i386.cc -- i386 target support for gold.

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
#include "i386.h"
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

namespace
{

using namespace gold;

// A class to handle the .got.plt section.

class Output_data_got_plt_i386 : public Output_section_data_build
{
 public:
  Output_data_got_plt_i386(Layout* layout)
    : Output_section_data_build(4),
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

class Output_data_plt_i386 : public Output_section_data
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_REL, true, 32, false> Reloc_section;

  Output_data_plt_i386(Layout*, uint64_t addralign,
		       Output_data_got_plt_i386*, Output_data_space*);

  // Add an entry to the PLT.
  void
  add_entry(Symbol_table*, Layout*, Symbol* gsym);

  // Add an entry to the PLT for a local STT_GNU_IFUNC symbol.
  unsigned int
  add_local_ifunc_entry(Symbol_table*, Layout*,
			Sized_relobj_file<32, false>* relobj,
			unsigned int local_sym_index);

  // Return the .rel.plt section data.
  Reloc_section*
  rel_plt() const
  { return this->rel_; }

  // Return where the TLS_DESC relocations should go.
  Reloc_section*
  rel_tls_desc(Layout*);

  // Return where the IRELATIVE relocations should go.
  Reloc_section*
  rel_irelative(Symbol_table*, Layout*);

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
  { return this->do_get_plt_entry_size(); }

  // Return the PLT address to use for a global symbol.
  uint64_t
  address_for_global(const Symbol*);

  // Return the PLT address to use for a local symbol.
  uint64_t
  address_for_local(const Relobj*, unsigned int symndx);

  // Add .eh_frame information for the PLT.
  void
  add_eh_frame(Layout* layout)
  { this->do_add_eh_frame(layout); }

 protected:
  // Fill the first PLT entry, given the pointer to the PLT section data
  // and the runtime address of the GOT.
  void
  fill_first_plt_entry(unsigned char* pov,
		       elfcpp::Elf_types<32>::Elf_Addr got_address)
  { this->do_fill_first_plt_entry(pov, got_address); }

  // Fill a normal PLT entry, given the pointer to the entry's data in the
  // section, the runtime address of the GOT, the offset into the GOT of
  // the corresponding slot, the offset into the relocation section of the
  // corresponding reloc, and the offset of this entry within the whole
  // PLT.  Return the offset from this PLT entry's runtime address that
  // should be used to compute the initial value of the GOT slot.
  unsigned int
  fill_plt_entry(unsigned char* pov,
		 elfcpp::Elf_types<32>::Elf_Addr got_address,
		 unsigned int got_offset,
		 unsigned int plt_offset,
		 unsigned int plt_rel_offset)
  {
    return this->do_fill_plt_entry(pov, got_address, got_offset,
				   plt_offset, plt_rel_offset);
  }

  virtual unsigned int
  do_get_plt_entry_size() const = 0;

  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  elfcpp::Elf_types<32>::Elf_Addr got_address) = 0;

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    elfcpp::Elf_types<32>::Elf_Addr got_address,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_rel_offset) = 0;

  virtual void
  do_add_eh_frame(Layout*) = 0;

  void
  do_adjust_output_section(Output_section* os);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** PLT")); }

  // The .eh_frame unwind information for the PLT.
  // The CIE is common across variants of the PLT format.
  static const int plt_eh_frame_cie_size = 16;
  static const unsigned char plt_eh_frame_cie[plt_eh_frame_cie_size];

 private:
  // Set the final size.
  void
  set_final_data_size()
  {
    this->set_data_size((this->count_ + this->irelative_count_ + 1)
			* this->get_plt_entry_size());
  }

  // Write out the PLT data.
  void
  do_write(Output_file*);

  // We keep a list of global STT_GNU_IFUNC symbols, each with its
  // offset in the GOT.
  struct Global_ifunc
  {
    Symbol* sym;
    unsigned int got_offset;
  };

  // We keep a list of local STT_GNU_IFUNC symbols, each with its
  // offset in the GOT.
  struct Local_ifunc
  {
    Sized_relobj_file<32, false>* object;
    unsigned int local_sym_index;
    unsigned int got_offset;
  };

  // The reloc section.
  Reloc_section* rel_;
  // The TLS_DESC relocations, if necessary.  These must follow the
  // regular PLT relocs.
  Reloc_section* tls_desc_rel_;
  // The IRELATIVE relocations, if necessary.  These must follow the
  // regular relocatoins and the TLS_DESC relocations.
  Reloc_section* irelative_rel_;
  // The .got.plt section.
  Output_data_got_plt_i386* got_plt_;
  // The part of the .got.plt section used for IRELATIVE relocs.
  Output_data_space* got_irelative_;
  // The number of PLT entries.
  unsigned int count_;
  // Number of PLT entries with R_386_IRELATIVE relocs.  These follow
  // the regular PLT entries.
  unsigned int irelative_count_;
  // Global STT_GNU_IFUNC symbols.
  std::vector<Global_ifunc> global_ifuncs_;
  // Local STT_GNU_IFUNC symbols.
  std::vector<Local_ifunc> local_ifuncs_;
};

// This is an abstract class for the standard PLT layout.
// The derived classes below handle the actual PLT contents
// for the executable (non-PIC) and shared-library (PIC) cases.
// The unwind information is uniform across those two, so it's here.

class Output_data_plt_i386_standard : public Output_data_plt_i386
{
 public:
  Output_data_plt_i386_standard(Layout* layout,
				Output_data_got_plt_i386* got_plt,
				Output_data_space* got_irelative)
    : Output_data_plt_i386(layout, plt_entry_size, got_plt, got_irelative)
  { }

 protected:
  virtual unsigned int
  do_get_plt_entry_size() const
  { return plt_entry_size; }

  virtual void
  do_add_eh_frame(Layout* layout)
  {
    layout->add_eh_frame_for_plt(this, plt_eh_frame_cie, plt_eh_frame_cie_size,
				 plt_eh_frame_fde, plt_eh_frame_fde_size);
  }

  // The size of an entry in the PLT.
  static const int plt_entry_size = 16;

  // The .eh_frame unwind information for the PLT.
  static const int plt_eh_frame_fde_size = 32;
  static const unsigned char plt_eh_frame_fde[plt_eh_frame_fde_size];
};

// Actually fill the PLT contents for an executable (non-PIC).

class Output_data_plt_i386_exec : public Output_data_plt_i386_standard
{
public:
  Output_data_plt_i386_exec(Layout* layout,
			    Output_data_got_plt_i386* got_plt,
			    Output_data_space* got_irelative)
    : Output_data_plt_i386_standard(layout, got_plt, got_irelative)
  { }

 protected:
  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  elfcpp::Elf_types<32>::Elf_Addr got_address);

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    elfcpp::Elf_types<32>::Elf_Addr got_address,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_rel_offset);

 private:
  // The first entry in the PLT for an executable.
  static const unsigned char first_plt_entry[plt_entry_size];

  // Other entries in the PLT for an executable.
  static const unsigned char plt_entry[plt_entry_size];
};

// Actually fill the PLT contents for a shared library (PIC).

class Output_data_plt_i386_dyn : public Output_data_plt_i386_standard
{
 public:
  Output_data_plt_i386_dyn(Layout* layout,
			   Output_data_got_plt_i386* got_plt,
			   Output_data_space* got_irelative)
    : Output_data_plt_i386_standard(layout, got_plt, got_irelative)
  { }

 protected:
  virtual void
  do_fill_first_plt_entry(unsigned char* pov, elfcpp::Elf_types<32>::Elf_Addr);

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    elfcpp::Elf_types<32>::Elf_Addr,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_rel_offset);

 private:
  // The first entry in the PLT for a shared object.
  static const unsigned char first_plt_entry[plt_entry_size];

  // Other entries in the PLT for a shared object.
  static const unsigned char plt_entry[plt_entry_size];
};

// The i386 target class.
// TLS info comes from
//   http://people.redhat.com/drepper/tls.pdf
//   http://www.lsd.ic.unicamp.br/~oliva/writeups/TLS/RFC-TLSDESC-x86.txt

class Target_i386 : public Sized_target<32, false>
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_REL, true, 32, false> Reloc_section;

  Target_i386(const Target::Target_info* info = &i386_info)
    : Sized_target<32, false>(info),
      got_(NULL), plt_(NULL), got_plt_(NULL), got_irelative_(NULL),
      got_tlsdesc_(NULL), global_offset_table_(NULL), rel_dyn_(NULL),
      rel_irelative_(NULL), copy_relocs_(elfcpp::R_386_COPY),
      got_mod_index_offset_(-1U), tls_base_symbol_defined_(false)
  { }

  // Process the relocations to determine unreferenced sections for
  // garbage collection.
  void
  gc_process_relocs(Symbol_table* symtab,
		    Layout* layout,
		    Sized_relobj_file<32, false>* object,
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
	      Sized_relobj_file<32, false>* object,
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
  relocate_section(const Relocate_info<32, false>*,
		   unsigned int sh_type,
		   const unsigned char* prelocs,
		   size_t reloc_count,
		   Output_section* output_section,
		   bool needs_special_offset_handling,
		   unsigned char* view,
		   elfcpp::Elf_types<32>::Elf_Addr view_address,
		   section_size_type view_size,
		   const Reloc_symbol_changes*);

  // Scan the relocs during a relocatable link.
  void
  scan_relocatable_relocs(Symbol_table* symtab,
			  Layout* layout,
			  Sized_relobj_file<32, false>* object,
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
		   Sized_relobj_file<32, false>* object,
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
  relocate_relocs(const Relocate_info<32, false>*,
		  unsigned int sh_type,
		  const unsigned char* prelocs,
		  size_t reloc_count,
		  Output_section* output_section,
		  elfcpp::Elf_types<32>::Elf_Off offset_in_output_section,
		  unsigned char* view,
		  elfcpp::Elf_types<32>::Elf_Addr view_address,
		  section_size_type view_size,
		  unsigned char* reloc_view,
		  section_size_type reloc_view_size);

  // Return a string used to fill a code section with nops.
  std::string
  do_code_fill(section_size_type length) const;

  // Return whether SYM is defined by the ABI.
  bool
  do_is_defined_by_abi(const Symbol* sym) const
  { return strcmp(sym->name(), "___tls_get_addr") == 0; }

  // Return whether a symbol name implies a local label.  The UnixWare
  // 2.1 cc generates temporary symbols that start with .X, so we
  // recognize them here.  FIXME: do other SVR4 compilers also use .X?.
  // If so, we should move the .X recognition into
  // Target::do_is_local_label_name.
  bool
  do_is_local_label_name(const char* name) const
  {
    if (name[0] == '.' && name[1] == 'X')
      return true;
    return Target::do_is_local_label_name(name);
  }

  // Return the PLT address to use for a global symbol.
  uint64_t
  do_plt_address_for_global(const Symbol* gsym) const
  { return this->plt_section()->address_for_global(gsym); }

  uint64_t
  do_plt_address_for_local(const Relobj* relobj, unsigned int symndx) const
  { return this->plt_section()->address_for_local(relobj, symndx); }

  // We can tell whether we take the address of a function.
  inline bool
  do_can_check_for_function_pointers() const
  { return true; }

  // Return the base for a DW_EH_PE_datarel encoding.
  uint64_t
  do_ehframe_datarel_base() const;

  // Return whether SYM is call to a non-split function.
  bool
  do_is_call_to_non_split(const Symbol* sym, const unsigned char*,
			  const unsigned char*, section_size_type) const;

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
    return this->got_size() / 4;
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

 protected:
  // Instantiate the plt_ member.
  // This chooses the right PLT flavor for an executable or a shared object.
  Output_data_plt_i386*
  make_data_plt(Layout* layout,
		Output_data_got_plt_i386* got_plt,
		Output_data_space* got_irelative,
		bool dyn)
  { return this->do_make_data_plt(layout, got_plt, got_irelative, dyn); }

  virtual Output_data_plt_i386*
  do_make_data_plt(Layout* layout,
		   Output_data_got_plt_i386* got_plt,
		   Output_data_space* got_irelative,
		   bool dyn)
  {
    if (dyn)
      return new Output_data_plt_i386_dyn(layout, got_plt, got_irelative);
    else
      return new Output_data_plt_i386_exec(layout, got_plt, got_irelative);
  }

 private:
  // The class which scans relocations.
  struct Scan
  {
    static inline int

    get_reference_flags(unsigned int r_type);

    inline void
    local(Symbol_table* symtab, Layout* layout, Target_i386* target,
	  Sized_relobj_file<32, false>* object,
	  unsigned int data_shndx,
	  Output_section* output_section,
	  const elfcpp::Rel<32, false>& reloc, unsigned int r_type,
	  const elfcpp::Sym<32, false>& lsym,
	  bool is_discarded);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_i386* target,
	   Sized_relobj_file<32, false>* object,
	   unsigned int data_shndx,
	   Output_section* output_section,
	   const elfcpp::Rel<32, false>& reloc, unsigned int r_type,
	   Symbol* gsym);

    inline bool
    local_reloc_may_be_function_pointer(Symbol_table* symtab, Layout* layout,
					Target_i386* target,
					Sized_relobj_file<32, false>* object,
					unsigned int data_shndx,
					Output_section* output_section,
					const elfcpp::Rel<32, false>& reloc,
					unsigned int r_type,
					const elfcpp::Sym<32, false>& lsym);

    inline bool
    global_reloc_may_be_function_pointer(Symbol_table* symtab, Layout* layout,
					 Target_i386* target,
					 Sized_relobj_file<32, false>* object,
					 unsigned int data_shndx,
					 Output_section* output_section,
					 const elfcpp::Rel<32, false>& reloc,
					 unsigned int r_type,
					 Symbol* gsym);

    inline bool
    possible_function_pointer_reloc(unsigned int r_type);

    bool
    reloc_needs_plt_for_ifunc(Sized_relobj_file<32, false>*,
			      unsigned int r_type);

    static void
    unsupported_reloc_local(Sized_relobj_file<32, false>*, unsigned int r_type);

    static void
    unsupported_reloc_global(Sized_relobj_file<32, false>*, unsigned int r_type,
			     Symbol*);
  };

  // The class which implements relocation.
  class Relocate
  {
   public:
    Relocate()
      : skip_call_tls_get_addr_(false),
	local_dynamic_type_(LOCAL_DYNAMIC_NONE)
    { }

    ~Relocate()
    {
      if (this->skip_call_tls_get_addr_)
	{
	  // FIXME: This needs to specify the location somehow.
	  gold_error(_("missing expected TLS relocation"));
	}
    }

    // Return whether the static relocation needs to be applied.
    inline bool
    should_apply_static_reloc(const Sized_symbol<32>* gsym,
			      unsigned int r_type,
			      bool is_32bit,
			      Output_section* output_section);

    // Do a relocation.  Return false if the caller should not issue
    // any warnings about this relocation.
    inline bool
    relocate(const Relocate_info<32, false>*, unsigned int,
	     Target_i386*, Output_section*, size_t, const unsigned char*,
	     const Sized_symbol<32>*, const Symbol_value<32>*,
	     unsigned char*, elfcpp::Elf_types<32>::Elf_Addr,
	     section_size_type);

   private:
    // Do a TLS relocation.
    inline void
    relocate_tls(const Relocate_info<32, false>*, Target_i386* target,
		 size_t relnum, const elfcpp::Rel<32, false>&,
		 unsigned int r_type, const Sized_symbol<32>*,
		 const Symbol_value<32>*,
		 unsigned char*, elfcpp::Elf_types<32>::Elf_Addr,
		 section_size_type);

    // Do a TLS General-Dynamic to Initial-Exec transition.
    inline void
    tls_gd_to_ie(const Relocate_info<32, false>*, size_t relnum,
		 const elfcpp::Rel<32, false>&, unsigned int r_type,
		 elfcpp::Elf_types<32>::Elf_Addr value,
		 unsigned char* view,
		 section_size_type view_size);

    // Do a TLS General-Dynamic to Local-Exec transition.
    inline void
    tls_gd_to_le(const Relocate_info<32, false>*, size_t relnum,
		 Output_segment* tls_segment,
		 const elfcpp::Rel<32, false>&, unsigned int r_type,
		 elfcpp::Elf_types<32>::Elf_Addr value,
		 unsigned char* view,
		 section_size_type view_size);

    // Do a TLS_GOTDESC or TLS_DESC_CALL General-Dynamic to Initial-Exec
    // transition.
    inline void
    tls_desc_gd_to_ie(const Relocate_info<32, false>*, size_t relnum,
		      const elfcpp::Rel<32, false>&, unsigned int r_type,
		      elfcpp::Elf_types<32>::Elf_Addr value,
		      unsigned char* view,
		      section_size_type view_size);

    // Do a TLS_GOTDESC or TLS_DESC_CALL General-Dynamic to Local-Exec
    // transition.
    inline void
    tls_desc_gd_to_le(const Relocate_info<32, false>*, size_t relnum,
		      Output_segment* tls_segment,
		      const elfcpp::Rel<32, false>&, unsigned int r_type,
		      elfcpp::Elf_types<32>::Elf_Addr value,
		      unsigned char* view,
		      section_size_type view_size);

    // Do a TLS Local-Dynamic to Local-Exec transition.
    inline void
    tls_ld_to_le(const Relocate_info<32, false>*, size_t relnum,
		 Output_segment* tls_segment,
		 const elfcpp::Rel<32, false>&, unsigned int r_type,
		 elfcpp::Elf_types<32>::Elf_Addr value,
		 unsigned char* view,
		 section_size_type view_size);

    // Do a TLS Initial-Exec to Local-Exec transition.
    static inline void
    tls_ie_to_le(const Relocate_info<32, false>*, size_t relnum,
		 Output_segment* tls_segment,
		 const elfcpp::Rel<32, false>&, unsigned int r_type,
		 elfcpp::Elf_types<32>::Elf_Addr value,
		 unsigned char* view,
		 section_size_type view_size);

    // We need to keep track of which type of local dynamic relocation
    // we have seen, so that we can optimize R_386_TLS_LDO_32 correctly.
    enum Local_dynamic_type
    {
      LOCAL_DYNAMIC_NONE,
      LOCAL_DYNAMIC_SUN,
      LOCAL_DYNAMIC_GNU
    };

    // This is set if we should skip the next reloc, which should be a
    // PLT32 reloc against ___tls_get_addr.
    bool skip_call_tls_get_addr_;
    // The type of local dynamic relocation we have seen in the section
    // being relocated, if any.
    Local_dynamic_type local_dynamic_type_;
  };

  // A class for inquiring about properties of a relocation,
  // used while scanning relocs during a relocatable link and
  // garbage collection.
  class Classify_reloc :
      public gold::Default_classify_reloc<elfcpp::SHT_REL, 32, false>
  {
   public:
    typedef Reloc_types<elfcpp::SHT_REL, 32, false>::Reloc Reltype;

    // Return the explicit addend of the relocation (return 0 for SHT_REL).
    static elfcpp::Elf_types<32>::Elf_Swxword
    get_r_addend(const Reltype*)
    { return 0; }

    // Return the size of the addend of the relocation (only used for SHT_REL).
    static unsigned int
    get_size_for_reloc(unsigned int, Relobj*);
  };

  // Adjust TLS relocation type based on the options and whether this
  // is a local symbol.
  static tls::Tls_optimization
  optimize_tls_reloc(bool is_final, int r_type);

  // Check if relocation against this symbol is a candidate for
  // conversion from
  // mov foo@GOT(%reg), %reg
  // to
  // lea foo@GOTOFF(%reg), %reg.
  static bool
  can_convert_mov_to_lea(const Symbol* gsym)
  {
    gold_assert(gsym != NULL);
    return (gsym->type() != elfcpp::STT_GNU_IFUNC
	    && !gsym->is_undefined ()
	    && !gsym->is_from_dynobj()
	    && !gsym->is_preemptible()
	    && (!parameters->options().shared()
		|| (gsym->visibility() != elfcpp::STV_DEFAULT
		    && gsym->visibility() != elfcpp::STV_PROTECTED)
		|| parameters->options().Bsymbolic())
	    && strcmp(gsym->name(), "_DYNAMIC") != 0);
  }

  // Get the GOT section, creating it if necessary.
  Output_data_got<32, false>*
  got_section(Symbol_table*, Layout*);

  // Get the GOT PLT section.
  Output_data_got_plt_i386*
  got_plt_section() const
  {
    gold_assert(this->got_plt_ != NULL);
    return this->got_plt_;
  }

  // Get the GOT section for TLSDESC entries.
  Output_data_got<32, false>*
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
			     Sized_relobj_file<32, false>* relobj,
			     unsigned int local_sym_index);

  // Define the _TLS_MODULE_BASE_ symbol in the TLS segment.
  void
  define_tls_base_symbol(Symbol_table*, Layout*);

  // Create a GOT entry for the TLS module index.
  unsigned int
  got_mod_index_entry(Symbol_table* symtab, Layout* layout,
		      Sized_relobj_file<32, false>* object);

  // Get the PLT section.
  Output_data_plt_i386*
  plt_section() const
  {
    gold_assert(this->plt_ != NULL);
    return this->plt_;
  }

  // Get the dynamic reloc section, creating it if necessary.
  Reloc_section*
  rel_dyn_section(Layout*);

  // Get the section to use for TLS_DESC relocations.
  Reloc_section*
  rel_tls_desc_section(Layout*) const;

  // Get the section to use for IRELATIVE relocations.
  Reloc_section*
  rel_irelative_section(Layout*);

  // Add a potential copy relocation.
  void
  copy_reloc(Symbol_table* symtab, Layout* layout,
	     Sized_relobj_file<32, false>* object,
	     unsigned int shndx, Output_section* output_section,
	     Symbol* sym, const elfcpp::Rel<32, false>& reloc)
  {
    unsigned int r_type = elfcpp::elf_r_type<32>(reloc.get_r_info());
    this->copy_relocs_.copy_reloc(symtab, layout,
				  symtab->get_sized_symbol<32>(sym),
				  object, shndx, output_section,
				  r_type, reloc.get_r_offset(), 0,
				  this->rel_dyn_section(layout));
  }

  // Information about this specific target which we pass to the
  // general Target structure.
  static const Target::Target_info i386_info;

  // The types of GOT entries needed for this platform.
  // These values are exposed to the ABI in an incremental link.
  // Do not renumber existing values without changing the version
  // number of the .gnu_incremental_inputs section.
  enum Got_type
  {
    GOT_TYPE_STANDARD = 0,      // GOT entry for a regular symbol
    GOT_TYPE_TLS_NOFFSET = 1,   // GOT entry for negative TLS offset
    GOT_TYPE_TLS_OFFSET = 2,    // GOT entry for positive TLS offset
    GOT_TYPE_TLS_PAIR = 3,      // GOT entry for TLS module/offset pair
    GOT_TYPE_TLS_DESC = 4       // GOT entry for TLS_DESC pair
  };

  // The GOT section.
  Output_data_got<32, false>* got_;
  // The PLT section.
  Output_data_plt_i386* plt_;
  // The GOT PLT section.
  Output_data_got_plt_i386* got_plt_;
  // The GOT section for IRELATIVE relocations.
  Output_data_space* got_irelative_;
  // The GOT section for TLSDESC relocations.
  Output_data_got<32, false>* got_tlsdesc_;
  // The _GLOBAL_OFFSET_TABLE_ symbol.
  Symbol* global_offset_table_;
  // The dynamic reloc section.
  Reloc_section* rel_dyn_;
  // The section to use for IRELATIVE relocs.
  Reloc_section* rel_irelative_;
  // Relocs saved to avoid a COPY reloc.
  Copy_relocs<elfcpp::SHT_REL, 32, false> copy_relocs_;
  // Offset of the GOT entry for the TLS module index.
  unsigned int got_mod_index_offset_;
  // True if the _TLS_MODULE_BASE_ symbol has been defined.
  bool tls_base_symbol_defined_;
};

const Target::Target_info Target_i386::i386_info =
{
  32,			// size
  false,		// is_big_endian
  elfcpp::EM_386,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/usr/lib/libc.so.1",	// dynamic_linker
  0x08048000,		// default_text_segment_address
  0x1000,		// abi_pagesize (overridable by -z max-page-size)
  0x1000,		// common_pagesize (overridable by -z common-page-size)
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

// Get the GOT section, creating it if necessary.

Output_data_got<32, false>*
Target_i386::got_section(Symbol_table* symtab, Layout* layout)
{
  if (this->got_ == NULL)
    {
      gold_assert(symtab != NULL && layout != NULL);

      this->got_ = new Output_data_got<32, false>();

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

      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_, got_order, true);

      this->got_plt_ = new Output_data_got_plt_i386(layout);
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_plt_, got_plt_order,
				      is_got_plt_relro);

      // The first three entries are reserved.
      this->got_plt_->set_current_data_size(3 * 4);

      if (!is_got_plt_relro)
	{
	  // Those bytes can go into the relro segment.
	  layout->increase_relro(3 * 4);
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
      // in .got.plt after the jump slot relocations.
      this->got_irelative_ = new Output_data_space(4, "** GOT IRELATIVE PLT");
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_irelative_,
				      got_plt_order, is_got_plt_relro);

      // If there are any TLSDESC relocations, they get GOT entries in
      // .got.plt after the jump slot entries.
      this->got_tlsdesc_ = new Output_data_got<32, false>();
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_tlsdesc_,
				      got_plt_order, is_got_plt_relro);
    }

  return this->got_;
}

// Get the dynamic reloc section, creating it if necessary.

Target_i386::Reloc_section*
Target_i386::rel_dyn_section(Layout* layout)
{
  if (this->rel_dyn_ == NULL)
    {
      gold_assert(layout != NULL);
      this->rel_dyn_ = new Reloc_section(parameters->options().combreloc());
      layout->add_output_section_data(".rel.dyn", elfcpp::SHT_REL,
				      elfcpp::SHF_ALLOC, this->rel_dyn_,
				      ORDER_DYNAMIC_RELOCS, false);
    }
  return this->rel_dyn_;
}

// Get the section to use for IRELATIVE relocs, creating it if
// necessary.  These go in .rel.dyn, but only after all other dynamic
// relocations.  They need to follow the other dynamic relocations so
// that they can refer to global variables initialized by those
// relocs.

Target_i386::Reloc_section*
Target_i386::rel_irelative_section(Layout* layout)
{
  if (this->rel_irelative_ == NULL)
    {
      // Make sure we have already create the dynamic reloc section.
      this->rel_dyn_section(layout);
      this->rel_irelative_ = new Reloc_section(false);
      layout->add_output_section_data(".rel.dyn", elfcpp::SHT_REL,
				      elfcpp::SHF_ALLOC, this->rel_irelative_,
				      ORDER_DYNAMIC_RELOCS, false);
      gold_assert(this->rel_dyn_->output_section()
		  == this->rel_irelative_->output_section());
    }
  return this->rel_irelative_;
}

// Write the first three reserved words of the .got.plt section.
// The remainder of the section is written while writing the PLT
// in Output_data_plt_i386::do_write.

void
Output_data_got_plt_i386::do_write(Output_file* of)
{
  // The first entry in the GOT is the address of the .dynamic section
  // aka the PT_DYNAMIC segment.  The next two entries are reserved.
  // We saved space for them when we created the section in
  // Target_i386::got_section.
  const off_t got_file_offset = this->offset();
  gold_assert(this->data_size() >= 12);
  unsigned char* const got_view = of->get_output_view(got_file_offset, 12);
  Output_section* dynamic = this->layout_->dynamic_section();
  uint32_t dynamic_addr = dynamic == NULL ? 0 : dynamic->address();
  elfcpp::Swap<32, false>::writeval(got_view, dynamic_addr);
  memset(got_view + 4, 0, 8);
  of->write_output_view(got_file_offset, 12, got_view);
}

// Create the PLT section.  The ordinary .got section is an argument,
// since we need to refer to the start.  We also create our own .got
// section just for PLT entries.

Output_data_plt_i386::Output_data_plt_i386(Layout* layout,
					   uint64_t addralign,
					   Output_data_got_plt_i386* got_plt,
					   Output_data_space* got_irelative)
  : Output_section_data(addralign),
    tls_desc_rel_(NULL), irelative_rel_(NULL), got_plt_(got_plt),
    got_irelative_(got_irelative), count_(0), irelative_count_(0),
    global_ifuncs_(), local_ifuncs_()
{
  this->rel_ = new Reloc_section(false);
  layout->add_output_section_data(".rel.plt", elfcpp::SHT_REL,
				  elfcpp::SHF_ALLOC, this->rel_,
				  ORDER_DYNAMIC_PLT_RELOCS, false);
}

void
Output_data_plt_i386::do_adjust_output_section(Output_section* os)
{
  // UnixWare sets the entsize of .plt to 4, and so does the old GNU
  // linker, and so do we.
  os->set_entsize(4);
}

// Add an entry to the PLT.

void
Output_data_plt_i386::add_entry(Symbol_table* symtab, Layout* layout,
				Symbol* gsym)
{
  gold_assert(!gsym->has_plt_offset());

  // Every PLT entry needs a reloc.
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      gsym->set_plt_offset(this->irelative_count_ * this->get_plt_entry_size());
      ++this->irelative_count_;
      section_offset_type got_offset =
	this->got_irelative_->current_data_size();
      this->got_irelative_->set_current_data_size(got_offset + 4);
      Reloc_section* rel = this->rel_irelative(symtab, layout);
      rel->add_symbolless_global_addend(gsym, elfcpp::R_386_IRELATIVE,
					this->got_irelative_, got_offset);
      struct Global_ifunc gi;
      gi.sym = gsym;
      gi.got_offset = got_offset;
      this->global_ifuncs_.push_back(gi);
    }
  else
    {
      // When setting the PLT offset we skip the initial reserved PLT
      // entry.
      gsym->set_plt_offset((this->count_ + 1) * this->get_plt_entry_size());

      ++this->count_;

      section_offset_type got_offset = this->got_plt_->current_data_size();

      // Every PLT entry needs a GOT entry which points back to the
      // PLT entry (this will be changed by the dynamic linker,
      // normally lazily when the function is called).
      this->got_plt_->set_current_data_size(got_offset + 4);

      gsym->set_needs_dynsym_entry();
      this->rel_->add_global(gsym, elfcpp::R_386_JUMP_SLOT, this->got_plt_,
			     got_offset);
    }

  // Note that we don't need to save the symbol.  The contents of the
  // PLT are independent of which symbols are used.  The symbols only
  // appear in the relocations.
}

// Add an entry to the PLT for a local STT_GNU_IFUNC symbol.  Return
// the PLT offset.

unsigned int
Output_data_plt_i386::add_local_ifunc_entry(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<32, false>* relobj,
    unsigned int local_sym_index)
{
  unsigned int plt_offset = this->irelative_count_ * this->get_plt_entry_size();
  ++this->irelative_count_;

  section_offset_type got_offset = this->got_irelative_->current_data_size();

  // Every PLT entry needs a GOT entry which points back to the PLT
  // entry.
  this->got_irelative_->set_current_data_size(got_offset + 4);

  // Every PLT entry needs a reloc.
  Reloc_section* rel = this->rel_irelative(symtab, layout);
  rel->add_symbolless_local_addend(relobj, local_sym_index,
				   elfcpp::R_386_IRELATIVE,
				   this->got_irelative_, got_offset);

  struct Local_ifunc li;
  li.object = relobj;
  li.local_sym_index = local_sym_index;
  li.got_offset = got_offset;
  this->local_ifuncs_.push_back(li);

  return plt_offset;
}

// Return where the TLS_DESC relocations should go, creating it if
// necessary. These follow the JUMP_SLOT relocations.

Output_data_plt_i386::Reloc_section*
Output_data_plt_i386::rel_tls_desc(Layout* layout)
{
  if (this->tls_desc_rel_ == NULL)
    {
      this->tls_desc_rel_ = new Reloc_section(false);
      layout->add_output_section_data(".rel.plt", elfcpp::SHT_REL,
				      elfcpp::SHF_ALLOC, this->tls_desc_rel_,
				      ORDER_DYNAMIC_PLT_RELOCS, false);
      gold_assert(this->tls_desc_rel_->output_section()
		  == this->rel_->output_section());
    }
  return this->tls_desc_rel_;
}

// Return where the IRELATIVE relocations should go in the PLT.  These
// follow the JUMP_SLOT and TLS_DESC relocations.

Output_data_plt_i386::Reloc_section*
Output_data_plt_i386::rel_irelative(Symbol_table* symtab, Layout* layout)
{
  if (this->irelative_rel_ == NULL)
    {
      // Make sure we have a place for the TLS_DESC relocations, in
      // case we see any later on.
      this->rel_tls_desc(layout);
      this->irelative_rel_ = new Reloc_section(false);
      layout->add_output_section_data(".rel.plt", elfcpp::SHT_REL,
				      elfcpp::SHF_ALLOC, this->irelative_rel_,
				      ORDER_DYNAMIC_PLT_RELOCS, false);
      gold_assert(this->irelative_rel_->output_section()
		  == this->rel_->output_section());

      if (parameters->doing_static_link())
	{
	  // A statically linked executable will only have a .rel.plt
	  // section to hold R_386_IRELATIVE relocs for STT_GNU_IFUNC
	  // symbols.  The library will use these symbols to locate
	  // the IRELATIVE relocs at program startup time.
	  symtab->define_in_output_data("__rel_iplt_start", NULL,
					Symbol_table::PREDEFINED,
					this->irelative_rel_, 0, 0,
					elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
					elfcpp::STV_HIDDEN, 0, false, true);
	  symtab->define_in_output_data("__rel_iplt_end", NULL,
					Symbol_table::PREDEFINED,
					this->irelative_rel_, 0, 0,
					elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
					elfcpp::STV_HIDDEN, 0, true, true);
	}
    }
  return this->irelative_rel_;
}

// Return the PLT address to use for a global symbol.

uint64_t
Output_data_plt_i386::address_for_global(const Symbol* gsym)
{
  uint64_t offset = 0;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    offset = (this->count_ + 1) * this->get_plt_entry_size();
  return this->address() + offset + gsym->plt_offset();
}

// Return the PLT address to use for a local symbol.  These are always
// IRELATIVE relocs.

uint64_t
Output_data_plt_i386::address_for_local(const Relobj* object,
					unsigned int r_sym)
{
  return (this->address()
	  + (this->count_ + 1) * this->get_plt_entry_size()
	  + object->local_plt_offset(r_sym));
}

// The first entry in the PLT for an executable.

const unsigned char Output_data_plt_i386_exec::first_plt_entry[plt_entry_size] =
{
  0xff, 0x35,	// pushl contents of memory address
  0, 0, 0, 0,	// replaced with address of .got + 4
  0xff, 0x25,	// jmp indirect
  0, 0, 0, 0,	// replaced with address of .got + 8
  0, 0, 0, 0	// unused
};

void
Output_data_plt_i386_exec::do_fill_first_plt_entry(
    unsigned char* pov,
    elfcpp::Elf_types<32>::Elf_Addr got_address)
{
  memcpy(pov, first_plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2, got_address + 4);
  elfcpp::Swap<32, false>::writeval(pov + 8, got_address + 8);
}

// The first entry in the PLT for a shared object.

const unsigned char Output_data_plt_i386_dyn::first_plt_entry[plt_entry_size] =
{
  0xff, 0xb3, 4, 0, 0, 0,	// pushl 4(%ebx)
  0xff, 0xa3, 8, 0, 0, 0,	// jmp *8(%ebx)
  0, 0, 0, 0			// unused
};

void
Output_data_plt_i386_dyn::do_fill_first_plt_entry(
    unsigned char* pov,
    elfcpp::Elf_types<32>::Elf_Addr)
{
  memcpy(pov, first_plt_entry, plt_entry_size);
}

// Subsequent entries in the PLT for an executable.

const unsigned char Output_data_plt_i386_exec::plt_entry[plt_entry_size] =
{
  0xff, 0x25,	// jmp indirect
  0, 0, 0, 0,	// replaced with address of symbol in .got
  0x68,		// pushl immediate
  0, 0, 0, 0,	// replaced with offset into relocation table
  0xe9,		// jmp relative
  0, 0, 0, 0	// replaced with offset to start of .plt
};

unsigned int
Output_data_plt_i386_exec::do_fill_plt_entry(
    unsigned char* pov,
    elfcpp::Elf_types<32>::Elf_Addr got_address,
    unsigned int got_offset,
    unsigned int plt_offset,
    unsigned int plt_rel_offset)
{
  memcpy(pov, plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2,
					      got_address + got_offset);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 7, plt_rel_offset);
  elfcpp::Swap<32, false>::writeval(pov + 12, - (plt_offset + 12 + 4));
  return 6;
}

// Subsequent entries in the PLT for a shared object.

const unsigned char Output_data_plt_i386_dyn::plt_entry[plt_entry_size] =
{
  0xff, 0xa3,	// jmp *offset(%ebx)
  0, 0, 0, 0,	// replaced with offset of symbol in .got
  0x68,		// pushl immediate
  0, 0, 0, 0,	// replaced with offset into relocation table
  0xe9,		// jmp relative
  0, 0, 0, 0	// replaced with offset to start of .plt
};

unsigned int
Output_data_plt_i386_dyn::do_fill_plt_entry(unsigned char* pov,
					    elfcpp::Elf_types<32>::Elf_Addr,
					    unsigned int got_offset,
					    unsigned int plt_offset,
					    unsigned int plt_rel_offset)
{
  memcpy(pov, plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2, got_offset);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 7, plt_rel_offset);
  elfcpp::Swap<32, false>::writeval(pov + 12, - (plt_offset + 12 + 4));
  return 6;
}

// The .eh_frame unwind information for the PLT.

const unsigned char
Output_data_plt_i386::plt_eh_frame_cie[plt_eh_frame_cie_size] =
{
  1,				// CIE version.
  'z',				// Augmentation: augmentation size included.
  'R',				// Augmentation: FDE encoding included.
  '\0',				// End of augmentation string.
  1,				// Code alignment factor.
  0x7c,				// Data alignment factor.
  8,				// Return address column.
  1,				// Augmentation size.
  (elfcpp::DW_EH_PE_pcrel	// FDE encoding.
   | elfcpp::DW_EH_PE_sdata4),
  elfcpp::DW_CFA_def_cfa, 4, 4,	// DW_CFA_def_cfa: r4 (esp) ofs 4.
  elfcpp::DW_CFA_offset + 8, 1,	// DW_CFA_offset: r8 (eip) at cfa-4.
  elfcpp::DW_CFA_nop,		// Align to 16 bytes.
  elfcpp::DW_CFA_nop
};

const unsigned char
Output_data_plt_i386_standard::plt_eh_frame_fde[plt_eh_frame_fde_size] =
{
  0, 0, 0, 0,				// Replaced with offset to .plt.
  0, 0, 0, 0,				// Replaced with size of .plt.
  0,					// Augmentation size.
  elfcpp::DW_CFA_def_cfa_offset, 8,	// DW_CFA_def_cfa_offset: 8.
  elfcpp::DW_CFA_advance_loc + 6,	// Advance 6 to __PLT__ + 6.
  elfcpp::DW_CFA_def_cfa_offset, 12,	// DW_CFA_def_cfa_offset: 12.
  elfcpp::DW_CFA_advance_loc + 10,	// Advance 10 to __PLT__ + 16.
  elfcpp::DW_CFA_def_cfa_expression,	// DW_CFA_def_cfa_expression.
  11,					// Block length.
  elfcpp::DW_OP_breg4, 4,		// Push %esp + 4.
  elfcpp::DW_OP_breg8, 0,		// Push %eip.
  elfcpp::DW_OP_lit15,			// Push 0xf.
  elfcpp::DW_OP_and,			// & (%eip & 0xf).
  elfcpp::DW_OP_lit11,			// Push 0xb.
  elfcpp::DW_OP_ge,			// >= ((%eip & 0xf) >= 0xb)
  elfcpp::DW_OP_lit2,			// Push 2.
  elfcpp::DW_OP_shl,			// << (((%eip & 0xf) >= 0xb) << 2)
  elfcpp::DW_OP_plus,			// + ((((%eip&0xf)>=0xb)<<2)+%esp+4
  elfcpp::DW_CFA_nop,			// Align to 32 bytes.
  elfcpp::DW_CFA_nop,
  elfcpp::DW_CFA_nop,
  elfcpp::DW_CFA_nop
};

// Write out the PLT.  This uses the hand-coded instructions above,
// and adjusts them as needed.  This is all specified by the i386 ELF
// Processor Supplement.

void
Output_data_plt_i386::do_write(Output_file* of)
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

  elfcpp::Elf_types<32>::Elf_Addr plt_address = this->address();
  elfcpp::Elf_types<32>::Elf_Addr got_address = this->got_plt_->address();

  this->fill_first_plt_entry(pov, got_address);
  pov += this->get_plt_entry_size();

  // The first three entries in the GOT are reserved, and are written
  // by Output_data_got_plt_i386::do_write.
  unsigned char* got_pov = got_view + 12;

  const int rel_size = elfcpp::Elf_sizes<32>::rel_size;

  unsigned int plt_offset = this->get_plt_entry_size();
  unsigned int plt_rel_offset = 0;
  unsigned int got_offset = 12;
  const unsigned int count = this->count_ + this->irelative_count_;
  for (unsigned int i = 0;
       i < count;
       ++i,
	 pov += this->get_plt_entry_size(),
	 got_pov += 4,
	 plt_offset += this->get_plt_entry_size(),
	 plt_rel_offset += rel_size,
	 got_offset += 4)
    {
      // Set and adjust the PLT entry itself.
      unsigned int lazy_offset = this->fill_plt_entry(pov,
						      got_address,
						      got_offset,
						      plt_offset,
						      plt_rel_offset);

      // Set the entry in the GOT.
      elfcpp::Swap<32, false>::writeval(got_pov,
					plt_address + plt_offset + lazy_offset);
    }

  // If any STT_GNU_IFUNC symbols have PLT entries, we need to change
  // the GOT to point to the actual symbol value, rather than point to
  // the PLT entry.  That will let the dynamic linker call the right
  // function when resolving IRELATIVE relocations.
  unsigned char* got_irelative_view = got_view + this->got_plt_->data_size();
  for (std::vector<Global_ifunc>::const_iterator p =
	 this->global_ifuncs_.begin();
       p != this->global_ifuncs_.end();
       ++p)
    {
      const Sized_symbol<32>* ssym =
	static_cast<const Sized_symbol<32>*>(p->sym);
      elfcpp::Swap<32, false>::writeval(got_irelative_view + p->got_offset,
					ssym->value());
    }

  for (std::vector<Local_ifunc>::const_iterator p =
	 this->local_ifuncs_.begin();
       p != this->local_ifuncs_.end();
       ++p)
    {
      const Symbol_value<32>* psymval =
	p->object->local_symbol(p->local_sym_index);
      elfcpp::Swap<32, false>::writeval(got_irelative_view + p->got_offset,
					psymval->value(p->object, 0));
    }

  gold_assert(static_cast<section_size_type>(pov - oview) == oview_size);
  gold_assert(static_cast<section_size_type>(got_pov - got_view) == got_size);

  of->write_output_view(offset, oview_size, oview);
  of->write_output_view(got_file_offset, got_size, got_view);
}

// Create the PLT section.

void
Target_i386::make_plt_section(Symbol_table* symtab, Layout* layout)
{
  if (this->plt_ == NULL)
    {
      // Create the GOT sections first.
      this->got_section(symtab, layout);

      const bool dyn = parameters->options().output_is_position_independent();
      this->plt_ = this->make_data_plt(layout,
				       this->got_plt_,
				       this->got_irelative_,
				       dyn);

      // Add unwind information if requested.
      if (parameters->options().ld_generated_unwind_info())
	this->plt_->add_eh_frame(layout);

      layout->add_output_section_data(".plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_EXECINSTR),
				      this->plt_, ORDER_PLT, false);

      // Make the sh_info field of .rel.plt point to .plt.
      Output_section* rel_plt_os = this->plt_->rel_plt()->output_section();
      rel_plt_os->set_info_section(this->plt_->output_section());
    }
}

// Create a PLT entry for a global symbol.

void
Target_i386::make_plt_entry(Symbol_table* symtab, Layout* layout, Symbol* gsym)
{
  if (gsym->has_plt_offset())
    return;
  if (this->plt_ == NULL)
    this->make_plt_section(symtab, layout);
  this->plt_->add_entry(symtab, layout, gsym);
}

// Make a PLT entry for a local STT_GNU_IFUNC symbol.

void
Target_i386::make_local_ifunc_plt_entry(Symbol_table* symtab, Layout* layout,
					Sized_relobj_file<32, false>* relobj,
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

unsigned int
Target_i386::plt_entry_count() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->entry_count();
}

// Return the offset of the first non-reserved PLT entry.

unsigned int
Target_i386::first_plt_entry_offset() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->first_plt_entry_offset();
}

// Return the size of each PLT entry.

unsigned int
Target_i386::plt_entry_size() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->get_plt_entry_size();
}

// Get the section to use for TLS_DESC relocations.

Target_i386::Reloc_section*
Target_i386::rel_tls_desc_section(Layout* layout) const
{
  return this->plt_section()->rel_tls_desc(layout);
}

// Define the _TLS_MODULE_BASE_ symbol in the TLS segment.

void
Target_i386::define_tls_base_symbol(Symbol_table* symtab, Layout* layout)
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

// Create a GOT entry for the TLS module index.

unsigned int
Target_i386::got_mod_index_entry(Symbol_table* symtab, Layout* layout,
				 Sized_relobj_file<32, false>* object)
{
  if (this->got_mod_index_offset_ == -1U)
    {
      gold_assert(symtab != NULL && layout != NULL && object != NULL);
      Reloc_section* rel_dyn = this->rel_dyn_section(layout);
      Output_data_got<32, false>* got = this->got_section(symtab, layout);
      unsigned int got_offset = got->add_constant(0);
      rel_dyn->add_local(object, 0, elfcpp::R_386_TLS_DTPMOD32, got,
			 got_offset);
      got->add_constant(0);
      this->got_mod_index_offset_ = got_offset;
    }
  return this->got_mod_index_offset_;
}

// Optimize the TLS relocation type based on what we know about the
// symbol.  IS_FINAL is true if the final address of this symbol is
// known at link time.

tls::Tls_optimization
Target_i386::optimize_tls_reloc(bool is_final, int r_type)
{
  // If we are generating a shared library, then we can't do anything
  // in the linker.
  if (parameters->options().shared())
    return tls::TLSOPT_NONE;

  switch (r_type)
    {
    case elfcpp::R_386_TLS_GD:
    case elfcpp::R_386_TLS_GOTDESC:
    case elfcpp::R_386_TLS_DESC_CALL:
      // These are General-Dynamic which permits fully general TLS
      // access.  Since we know that we are generating an executable,
      // we can convert this to Initial-Exec.  If we also know that
      // this is a local symbol, we can further switch to Local-Exec.
      if (is_final)
	return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_TO_IE;

    case elfcpp::R_386_TLS_LDM:
      // This is Local-Dynamic, which refers to a local symbol in the
      // dynamic TLS block.  Since we know that we generating an
      // executable, we can switch to Local-Exec.
      return tls::TLSOPT_TO_LE;

    case elfcpp::R_386_TLS_LDO_32:
      // Another type of Local-Dynamic relocation.
      return tls::TLSOPT_TO_LE;

    case elfcpp::R_386_TLS_IE:
    case elfcpp::R_386_TLS_GOTIE:
    case elfcpp::R_386_TLS_IE_32:
      // These are Initial-Exec relocs which get the thread offset
      // from the GOT.  If we know that we are linking against the
      // local symbol, we can switch to Local-Exec, which links the
      // thread offset into the instruction.
      if (is_final)
	return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_NONE;

    case elfcpp::R_386_TLS_LE:
    case elfcpp::R_386_TLS_LE_32:
      // When we already have Local-Exec, there is nothing further we
      // can do.
      return tls::TLSOPT_NONE;

    default:
      gold_unreachable();
    }
}

// Get the Reference_flags for a particular relocation.

int
Target_i386::Scan::get_reference_flags(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_386_NONE:
    case elfcpp::R_386_GNU_VTINHERIT:
    case elfcpp::R_386_GNU_VTENTRY:
    case elfcpp::R_386_GOTPC:
      // No symbol reference.
      return 0;

    case elfcpp::R_386_32:
    case elfcpp::R_386_16:
    case elfcpp::R_386_8:
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_386_PC32:
    case elfcpp::R_386_PC16:
    case elfcpp::R_386_PC8:
    case elfcpp::R_386_GOTOFF:
      return Symbol::RELATIVE_REF;

    case elfcpp::R_386_PLT32:
      return Symbol::FUNCTION_CALL | Symbol::RELATIVE_REF;

    case elfcpp::R_386_GOT32:
    case elfcpp::R_386_GOT32X:
      // Absolute in GOT.
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_386_TLS_GD:            // Global-dynamic
    case elfcpp::R_386_TLS_GOTDESC:       // Global-dynamic (from ~oliva url)
    case elfcpp::R_386_TLS_DESC_CALL:
    case elfcpp::R_386_TLS_LDM:           // Local-dynamic
    case elfcpp::R_386_TLS_LDO_32:        // Alternate local-dynamic
    case elfcpp::R_386_TLS_IE:            // Initial-exec
    case elfcpp::R_386_TLS_IE_32:
    case elfcpp::R_386_TLS_GOTIE:
    case elfcpp::R_386_TLS_LE:            // Local-exec
    case elfcpp::R_386_TLS_LE_32:
      return Symbol::TLS_REF;

    case elfcpp::R_386_COPY:
    case elfcpp::R_386_GLOB_DAT:
    case elfcpp::R_386_JUMP_SLOT:
    case elfcpp::R_386_RELATIVE:
    case elfcpp::R_386_IRELATIVE:
    case elfcpp::R_386_TLS_TPOFF:
    case elfcpp::R_386_TLS_DTPMOD32:
    case elfcpp::R_386_TLS_DTPOFF32:
    case elfcpp::R_386_TLS_TPOFF32:
    case elfcpp::R_386_TLS_DESC:
    case elfcpp::R_386_32PLT:
    case elfcpp::R_386_TLS_GD_32:
    case elfcpp::R_386_TLS_GD_PUSH:
    case elfcpp::R_386_TLS_GD_CALL:
    case elfcpp::R_386_TLS_GD_POP:
    case elfcpp::R_386_TLS_LDM_32:
    case elfcpp::R_386_TLS_LDM_PUSH:
    case elfcpp::R_386_TLS_LDM_CALL:
    case elfcpp::R_386_TLS_LDM_POP:
    case elfcpp::R_386_USED_BY_INTEL_200:
    default:
      // Not expected.  We will give an error later.
      return 0;
    }
}

// Report an unsupported relocation against a local symbol.

void
Target_i386::Scan::unsupported_reloc_local(Sized_relobj_file<32, false>* object,
					   unsigned int r_type)
{
  gold_error(_("%s: unsupported reloc %u against local symbol"),
	     object->name().c_str(), r_type);
}

// Return whether we need to make a PLT entry for a relocation of a
// given type against a STT_GNU_IFUNC symbol.

bool
Target_i386::Scan::reloc_needs_plt_for_ifunc(
    Sized_relobj_file<32, false>* object,
    unsigned int r_type)
{
  int flags = Scan::get_reference_flags(r_type);
  if (flags & Symbol::TLS_REF)
    gold_error(_("%s: unsupported TLS reloc %u for IFUNC symbol"),
	       object->name().c_str(), r_type);
  return flags != 0;
}

// Scan a relocation for a local symbol.

inline void
Target_i386::Scan::local(Symbol_table* symtab,
			 Layout* layout,
			 Target_i386* target,
			 Sized_relobj_file<32, false>* object,
			 unsigned int data_shndx,
			 Output_section* output_section,
			 const elfcpp::Rel<32, false>& reloc,
			 unsigned int r_type,
			 const elfcpp::Sym<32, false>& lsym,
			 bool is_discarded)
{
  if (is_discarded)
    return;

  // A local STT_GNU_IFUNC symbol may require a PLT entry.
  if (lsym.get_st_type() == elfcpp::STT_GNU_IFUNC
      && this->reloc_needs_plt_for_ifunc(object, r_type))
    {
      unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
      target->make_local_ifunc_plt_entry(symtab, layout, object, r_sym);
    }

  switch (r_type)
    {
    case elfcpp::R_386_NONE:
    case elfcpp::R_386_GNU_VTINHERIT:
    case elfcpp::R_386_GNU_VTENTRY:
      break;

    case elfcpp::R_386_32:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for
      // this location. The relocation applied at link time will
      // apply the link-time value, so we flag the location with
      // an R_386_RELATIVE relocation so the dynamic loader can
      // relocate it easily.
      if (parameters->options().output_is_position_independent())
	{
	  Reloc_section* rel_dyn = target->rel_dyn_section(layout);
	  unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
	  rel_dyn->add_local_relative(object, r_sym, elfcpp::R_386_RELATIVE,
				      output_section, data_shndx,
				      reloc.get_r_offset());
	}
      break;

    case elfcpp::R_386_16:
    case elfcpp::R_386_8:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for
      // this location. Because the addend needs to remain in the
      // data section, we need to be careful not to apply this
      // relocation statically.
      if (parameters->options().output_is_position_independent())
	{
	  Reloc_section* rel_dyn = target->rel_dyn_section(layout);
	  unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
	  if (lsym.get_st_type() != elfcpp::STT_SECTION)
	    rel_dyn->add_local(object, r_sym, r_type, output_section,
			       data_shndx, reloc.get_r_offset());
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
		rel_dyn->add_local_section(object, shndx,
					   r_type, output_section,
					   data_shndx, reloc.get_r_offset());
	    }
	}
      break;

    case elfcpp::R_386_PC32:
    case elfcpp::R_386_PC16:
    case elfcpp::R_386_PC8:
      break;

    case elfcpp::R_386_PLT32:
      // Since we know this is a local symbol, we can handle this as a
      // PC32 reloc.
      break;

    case elfcpp::R_386_GOTOFF:
    case elfcpp::R_386_GOTPC:
      // We need a GOT section.
      target->got_section(symtab, layout);
      break;

    case elfcpp::R_386_GOT32:
    case elfcpp::R_386_GOT32X:
      {
	// We need GOT section.
	Output_data_got<32, false>* got = target->got_section(symtab, layout);

	// If the relocation symbol isn't IFUNC,
	// and is local, then we will convert
	// mov foo@GOT(%reg), %reg
	// to
	// lea foo@GOTOFF(%reg), %reg
	// in Relocate::relocate.
	if (reloc.get_r_offset() >= 2
	    && lsym.get_st_type() != elfcpp::STT_GNU_IFUNC)
	  {
	    section_size_type stype;
	    const unsigned char* view = object->section_contents(data_shndx,
								 &stype, true);
	    if (view[reloc.get_r_offset() - 2] == 0x8b)
	      break;
	  }

	// Otherwise, the symbol requires a GOT entry.
	unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());

	// For a STT_GNU_IFUNC symbol we want the PLT offset.  That
	// lets function pointers compare correctly with shared
	// libraries.  Otherwise we would need an IRELATIVE reloc.
	bool is_new;
	if (lsym.get_st_type() == elfcpp::STT_GNU_IFUNC)
	  is_new = got->add_local_plt(object, r_sym, GOT_TYPE_STANDARD);
	else
	  is_new = got->add_local(object, r_sym, GOT_TYPE_STANDARD);
	if (is_new)
	  {
	    // If we are generating a shared object, we need to add a
	    // dynamic RELATIVE relocation for this symbol's GOT entry.
	    if (parameters->options().output_is_position_independent())
	      {
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		unsigned int got_offset =
		  object->local_got_offset(r_sym, GOT_TYPE_STANDARD);
		rel_dyn->add_local_relative(object, r_sym,
					    elfcpp::R_386_RELATIVE,
					    got, got_offset);
	      }
	  }
      }
      break;

      // These are relocations which should only be seen by the
      // dynamic linker, and should never be seen here.
    case elfcpp::R_386_COPY:
    case elfcpp::R_386_GLOB_DAT:
    case elfcpp::R_386_JUMP_SLOT:
    case elfcpp::R_386_RELATIVE:
    case elfcpp::R_386_IRELATIVE:
    case elfcpp::R_386_TLS_TPOFF:
    case elfcpp::R_386_TLS_DTPMOD32:
    case elfcpp::R_386_TLS_DTPOFF32:
    case elfcpp::R_386_TLS_TPOFF32:
    case elfcpp::R_386_TLS_DESC:
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

      // These are initial TLS relocs, which are expected when
      // linking.
    case elfcpp::R_386_TLS_GD:            // Global-dynamic
    case elfcpp::R_386_TLS_GOTDESC:       // Global-dynamic (from ~oliva url)
    case elfcpp::R_386_TLS_DESC_CALL:
    case elfcpp::R_386_TLS_LDM:           // Local-dynamic
    case elfcpp::R_386_TLS_LDO_32:        // Alternate local-dynamic
    case elfcpp::R_386_TLS_IE:            // Initial-exec
    case elfcpp::R_386_TLS_IE_32:
    case elfcpp::R_386_TLS_GOTIE:
    case elfcpp::R_386_TLS_LE:            // Local-exec
    case elfcpp::R_386_TLS_LE_32:
      {
	bool output_is_shared = parameters->options().shared();
	const tls::Tls_optimization optimized_type
	    = Target_i386::optimize_tls_reloc(!output_is_shared, r_type);
	switch (r_type)
	  {
	  case elfcpp::R_386_TLS_GD:          // Global-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Output_data_got<32, false>* got
		    = target->got_section(symtab, layout);
		unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
		unsigned int shndx = lsym.get_st_shndx();
		bool is_ordinary;
		shndx = object->adjust_sym_shndx(r_sym, shndx, &is_ordinary);
		if (!is_ordinary)
		  object->error(_("local symbol %u has bad shndx %u"),
			      r_sym, shndx);
		else
		  got->add_local_pair_with_rel(object, r_sym, shndx,
					       GOT_TYPE_TLS_PAIR,
					       target->rel_dyn_section(layout),
					       elfcpp::R_386_TLS_DTPMOD32);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_386_TLS_GOTDESC:     // Global-dynamic (from ~oliva)
	    target->define_tls_base_symbol(symtab, layout);
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a double GOT entry with an R_386_TLS_DESC
		// reloc.  The R_386_TLS_DESC reloc is resolved
		// lazily, so the GOT entry needs to be in an area in
		// .got.plt, not .got.  Call got_section to make sure
		// the section has been created.
		target->got_section(symtab, layout);
		Output_data_got<32, false>* got = target->got_tlsdesc_section();
		unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
		if (!object->local_has_got_offset(r_sym, GOT_TYPE_TLS_DESC))
		  {
		    unsigned int got_offset = got->add_constant(0);
		    // The local symbol value is stored in the second
		    // GOT entry.
		    got->add_local(object, r_sym, GOT_TYPE_TLS_DESC);
		    // That set the GOT offset of the local symbol to
		    // point to the second entry, but we want it to
		    // point to the first.
		    object->set_local_got_offset(r_sym, GOT_TYPE_TLS_DESC,
						 got_offset);
		    Reloc_section* rt = target->rel_tls_desc_section(layout);
		    rt->add_absolute(elfcpp::R_386_TLS_DESC, got, got_offset);
		  }
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_386_TLS_DESC_CALL:
	    break;

	  case elfcpp::R_386_TLS_LDM:         // Local-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_386_TLS_LDO_32:      // Alternate local-dynamic
	    break;

	  case elfcpp::R_386_TLS_IE:          // Initial-exec
	  case elfcpp::R_386_TLS_IE_32:
	  case elfcpp::R_386_TLS_GOTIE:
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// For the R_386_TLS_IE relocation, we need to create a
		// dynamic relocation when building a shared library.
		if (r_type == elfcpp::R_386_TLS_IE
		    && parameters->options().shared())
		  {
		    Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		    unsigned int r_sym
			= elfcpp::elf_r_sym<32>(reloc.get_r_info());
		    rel_dyn->add_local_relative(object, r_sym,
						elfcpp::R_386_RELATIVE,
						output_section, data_shndx,
						reloc.get_r_offset());
		  }
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<32, false>* got
		    = target->got_section(symtab, layout);
		unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
		unsigned int dyn_r_type = (r_type == elfcpp::R_386_TLS_IE_32
					   ? elfcpp::R_386_TLS_TPOFF32
					   : elfcpp::R_386_TLS_TPOFF);
		unsigned int got_type = (r_type == elfcpp::R_386_TLS_IE_32
					 ? GOT_TYPE_TLS_OFFSET
					 : GOT_TYPE_TLS_NOFFSET);
		got->add_local_with_rel(object, r_sym, got_type,
					target->rel_dyn_section(layout),
					dyn_r_type);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_386_TLS_LE:          // Local-exec
	  case elfcpp::R_386_TLS_LE_32:
	    layout->set_has_static_tls();
	    if (output_is_shared)
	      {
		// We need to create a dynamic relocation.
		gold_assert(lsym.get_st_type() != elfcpp::STT_SECTION);
		unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
		unsigned int dyn_r_type = (r_type == elfcpp::R_386_TLS_LE_32
					   ? elfcpp::R_386_TLS_TPOFF32
					   : elfcpp::R_386_TLS_TPOFF);
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_local(object, r_sym, dyn_r_type, output_section,
				   data_shndx, reloc.get_r_offset());
	      }
	    break;

	  default:
	    gold_unreachable();
	  }
      }
      break;

    case elfcpp::R_386_32PLT:
    case elfcpp::R_386_TLS_GD_32:
    case elfcpp::R_386_TLS_GD_PUSH:
    case elfcpp::R_386_TLS_GD_CALL:
    case elfcpp::R_386_TLS_GD_POP:
    case elfcpp::R_386_TLS_LDM_32:
    case elfcpp::R_386_TLS_LDM_PUSH:
    case elfcpp::R_386_TLS_LDM_CALL:
    case elfcpp::R_386_TLS_LDM_POP:
    case elfcpp::R_386_USED_BY_INTEL_200:
    default:
      unsupported_reloc_local(object, r_type);
      break;
    }
}

// Report an unsupported relocation against a global symbol.

void
Target_i386::Scan::unsupported_reloc_global(
    Sized_relobj_file<32, false>* object,
    unsigned int r_type,
    Symbol* gsym)
{
  gold_error(_("%s: unsupported reloc %u against global symbol %s"),
	     object->name().c_str(), r_type, gsym->demangled_name().c_str());
}

inline bool
Target_i386::Scan::possible_function_pointer_reloc(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_386_32:
    case elfcpp::R_386_16:
    case elfcpp::R_386_8:
    case elfcpp::R_386_GOTOFF:
    case elfcpp::R_386_GOT32:
    case elfcpp::R_386_GOT32X:
      {
	return true;
      }
    default:
      return false;
    }
  return false;
}

inline bool
Target_i386::Scan::local_reloc_may_be_function_pointer(
  Symbol_table* ,
  Layout* ,
  Target_i386* ,
  Sized_relobj_file<32, false>* ,
  unsigned int ,
  Output_section* ,
  const elfcpp::Rel<32, false>& ,
  unsigned int r_type,
  const elfcpp::Sym<32, false>&)
{
  return possible_function_pointer_reloc(r_type);
}

inline bool
Target_i386::Scan::global_reloc_may_be_function_pointer(
  Symbol_table* ,
  Layout* ,
  Target_i386* ,
  Sized_relobj_file<32, false>* ,
  unsigned int ,
  Output_section* ,
  const elfcpp::Rel<32, false>& ,
  unsigned int r_type,
  Symbol*)
{
  return possible_function_pointer_reloc(r_type);
}

// Scan a relocation for a global symbol.

inline void
Target_i386::Scan::global(Symbol_table* symtab,
				 Layout* layout,
				 Target_i386* target,
				 Sized_relobj_file<32, false>* object,
				 unsigned int data_shndx,
				 Output_section* output_section,
				 const elfcpp::Rel<32, false>& reloc,
				 unsigned int r_type,
				 Symbol* gsym)
{
  // A STT_GNU_IFUNC symbol may require a PLT entry.
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && this->reloc_needs_plt_for_ifunc(object, r_type))
    target->make_plt_entry(symtab, layout, gsym);

  switch (r_type)
    {
    case elfcpp::R_386_NONE:
    case elfcpp::R_386_GNU_VTINHERIT:
    case elfcpp::R_386_GNU_VTENTRY:
      break;

    case elfcpp::R_386_32:
    case elfcpp::R_386_16:
    case elfcpp::R_386_8:
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
	    else if (r_type == elfcpp::R_386_32
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
		Reloc_section* rel_dyn = target->rel_irelative_section(layout);
		rel_dyn->add_symbolless_global_addend(gsym,
						      elfcpp::R_386_IRELATIVE,
						      output_section,
						      object, data_shndx,
						      reloc.get_r_offset());
	      }
	    else if (r_type == elfcpp::R_386_32
		     && gsym->can_use_relative_reloc(false))
	      {
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_global_relative(gsym, elfcpp::R_386_RELATIVE,
					     output_section, object,
					     data_shndx, reloc.get_r_offset());
	      }
	    else
	      {
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_global(gsym, r_type, output_section, object,
				    data_shndx, reloc.get_r_offset());
	      }
	  }
      }
      break;

    case elfcpp::R_386_PC32:
    case elfcpp::R_386_PC16:
    case elfcpp::R_386_PC8:
      {
	// Make a PLT entry if necessary.
	if (gsym->needs_plt_entry())
	  {
	    // These relocations are used for function calls only in
	    // non-PIC code.  For a 32-bit relocation in a shared library,
	    // we'll need a text relocation anyway, so we can skip the
	    // PLT entry and let the dynamic linker bind the call directly
	    // to the target.  For smaller relocations, we should use a
	    // PLT entry to ensure that the call can reach.
	    if (!parameters->options().shared()
		|| r_type != elfcpp::R_386_PC32)
	      target->make_plt_entry(symtab, layout, gsym);
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
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_global(gsym, r_type, output_section, object,
				    data_shndx, reloc.get_r_offset());
	      }
	  }
      }
      break;

    case elfcpp::R_386_GOT32:
    case elfcpp::R_386_GOT32X:
      {
	// The symbol requires a GOT section.
	Output_data_got<32, false>* got = target->got_section(symtab, layout);

	// If we convert this from
	// mov foo@GOT(%reg), %reg
	// to
	// lea foo@GOTOFF(%reg), %reg
	// in Relocate::relocate, then there is nothing to do here.
	if (reloc.get_r_offset() >= 2
	    && Target_i386::can_convert_mov_to_lea(gsym))
	  {
	    section_size_type stype;
	    const unsigned char* view = object->section_contents(data_shndx,
								 &stype, true);
	    if (view[reloc.get_r_offset() - 2] == 0x8b)
	      break;
	  }

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
	    // GOT entry with a dynamic relocation.
	    Reloc_section* rel_dyn = target->rel_dyn_section(layout);

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
	      got->add_global_with_rel(gsym, GOT_TYPE_STANDARD,
				       rel_dyn, elfcpp::R_386_GLOB_DAT);
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
		    rel_dyn->add_global_relative(gsym, elfcpp::R_386_RELATIVE,
						 got, got_off);
		  }
	      }
	  }
      }
      break;

    case elfcpp::R_386_PLT32:
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

    case elfcpp::R_386_GOTOFF:
      // A GOT-relative reference must resolve locally.
      if (!gsym->is_defined())
        gold_error(_("%s: relocation R_386_GOTOFF against undefined symbol %s"
		     " cannot be used when making a shared object"),
		   object->name().c_str(), gsym->name());
      else if (gsym->is_from_dynobj())
        gold_error(_("%s: relocation R_386_GOTOFF against external symbol %s"
		     " cannot be used when making a shared object"),
		   object->name().c_str(), gsym->name());
      else if (gsym->is_preemptible())
        gold_error(_("%s: relocation R_386_GOTOFF against preemptible symbol %s"
		     " cannot be used when making a shared object"),
		   object->name().c_str(), gsym->name());
      // We need a GOT section.
      target->got_section(symtab, layout);
      break;

    case elfcpp::R_386_GOTPC:
      // We need a GOT section.
      target->got_section(symtab, layout);
      break;

      // These are relocations which should only be seen by the
      // dynamic linker, and should never be seen here.
    case elfcpp::R_386_COPY:
    case elfcpp::R_386_GLOB_DAT:
    case elfcpp::R_386_JUMP_SLOT:
    case elfcpp::R_386_RELATIVE:
    case elfcpp::R_386_IRELATIVE:
    case elfcpp::R_386_TLS_TPOFF:
    case elfcpp::R_386_TLS_DTPMOD32:
    case elfcpp::R_386_TLS_DTPOFF32:
    case elfcpp::R_386_TLS_TPOFF32:
    case elfcpp::R_386_TLS_DESC:
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

      // These are initial tls relocs, which are expected when
      // linking.
    case elfcpp::R_386_TLS_GD:            // Global-dynamic
    case elfcpp::R_386_TLS_GOTDESC:       // Global-dynamic (from ~oliva url)
    case elfcpp::R_386_TLS_DESC_CALL:
    case elfcpp::R_386_TLS_LDM:           // Local-dynamic
    case elfcpp::R_386_TLS_LDO_32:        // Alternate local-dynamic
    case elfcpp::R_386_TLS_IE:            // Initial-exec
    case elfcpp::R_386_TLS_IE_32:
    case elfcpp::R_386_TLS_GOTIE:
    case elfcpp::R_386_TLS_LE:            // Local-exec
    case elfcpp::R_386_TLS_LE_32:
      {
	const bool is_final = gsym->final_value_is_known();
	const tls::Tls_optimization optimized_type
	    = Target_i386::optimize_tls_reloc(is_final, r_type);
	switch (r_type)
	  {
	  case elfcpp::R_386_TLS_GD:          // Global-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Output_data_got<32, false>* got
		    = target->got_section(symtab, layout);
		got->add_global_pair_with_rel(gsym, GOT_TYPE_TLS_PAIR,
					     target->rel_dyn_section(layout),
					     elfcpp::R_386_TLS_DTPMOD32,
					     elfcpp::R_386_TLS_DTPOFF32);
	      }
	    else if (optimized_type == tls::TLSOPT_TO_IE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<32, false>* got
		    = target->got_section(symtab, layout);
		got->add_global_with_rel(gsym, GOT_TYPE_TLS_NOFFSET,
					 target->rel_dyn_section(layout),
					 elfcpp::R_386_TLS_TPOFF);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_386_TLS_GOTDESC:     // Global-dynamic (~oliva url)
	    target->define_tls_base_symbol(symtab, layout);
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a double GOT entry with an R_386_TLS_DESC
		// reloc.  The R_386_TLS_DESC reloc is resolved
		// lazily, so the GOT entry needs to be in an area in
		// .got.plt, not .got.  Call got_section to make sure
		// the section has been created.
		target->got_section(symtab, layout);
		Output_data_got<32, false>* got = target->got_tlsdesc_section();
		Reloc_section* rt = target->rel_tls_desc_section(layout);
		got->add_global_pair_with_rel(gsym, GOT_TYPE_TLS_DESC, rt,
					     elfcpp::R_386_TLS_DESC, 0);
	      }
	    else if (optimized_type == tls::TLSOPT_TO_IE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<32, false>* got
		    = target->got_section(symtab, layout);
		got->add_global_with_rel(gsym, GOT_TYPE_TLS_NOFFSET,
					 target->rel_dyn_section(layout),
					 elfcpp::R_386_TLS_TPOFF);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_386_TLS_DESC_CALL:
	    break;

	  case elfcpp::R_386_TLS_LDM:         // Local-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_386_TLS_LDO_32:      // Alternate local-dynamic
	    break;

	  case elfcpp::R_386_TLS_IE:          // Initial-exec
	  case elfcpp::R_386_TLS_IE_32:
	  case elfcpp::R_386_TLS_GOTIE:
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// For the R_386_TLS_IE relocation, we need to create a
		// dynamic relocation when building a shared library.
		if (r_type == elfcpp::R_386_TLS_IE
		    && parameters->options().shared())
		  {
		    Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		    rel_dyn->add_global_relative(gsym, elfcpp::R_386_RELATIVE,
						 output_section, object,
						 data_shndx,
						 reloc.get_r_offset());
		  }
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<32, false>* got
		    = target->got_section(symtab, layout);
		unsigned int dyn_r_type = (r_type == elfcpp::R_386_TLS_IE_32
					   ? elfcpp::R_386_TLS_TPOFF32
					   : elfcpp::R_386_TLS_TPOFF);
		unsigned int got_type = (r_type == elfcpp::R_386_TLS_IE_32
					 ? GOT_TYPE_TLS_OFFSET
					 : GOT_TYPE_TLS_NOFFSET);
		got->add_global_with_rel(gsym, got_type,
					 target->rel_dyn_section(layout),
					 dyn_r_type);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_386_TLS_LE:          // Local-exec
	  case elfcpp::R_386_TLS_LE_32:
	    layout->set_has_static_tls();
	    if (parameters->options().shared())
	      {
		// We need to create a dynamic relocation.
		unsigned int dyn_r_type = (r_type == elfcpp::R_386_TLS_LE_32
					   ? elfcpp::R_386_TLS_TPOFF32
					   : elfcpp::R_386_TLS_TPOFF);
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_global(gsym, dyn_r_type, output_section, object,
				    data_shndx, reloc.get_r_offset());
	      }
	    break;

	  default:
	    gold_unreachable();
	  }
      }
      break;

    case elfcpp::R_386_32PLT:
    case elfcpp::R_386_TLS_GD_32:
    case elfcpp::R_386_TLS_GD_PUSH:
    case elfcpp::R_386_TLS_GD_CALL:
    case elfcpp::R_386_TLS_GD_POP:
    case elfcpp::R_386_TLS_LDM_32:
    case elfcpp::R_386_TLS_LDM_PUSH:
    case elfcpp::R_386_TLS_LDM_CALL:
    case elfcpp::R_386_TLS_LDM_POP:
    case elfcpp::R_386_USED_BY_INTEL_200:
    default:
      unsupported_reloc_global(object, r_type, gsym);
      break;
    }
}

// Process relocations for gc.

void
Target_i386::gc_process_relocs(Symbol_table* symtab,
				      Layout* layout,
				      Sized_relobj_file<32, false>* object,
				      unsigned int data_shndx,
				      unsigned int,
				      const unsigned char* prelocs,
				      size_t reloc_count,
				      Output_section* output_section,
				      bool needs_special_offset_handling,
				      size_t local_symbol_count,
				      const unsigned char* plocal_symbols)
{
  gold::gc_process_relocs<32, false, Target_i386, Scan, Classify_reloc>(
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

void
Target_i386::scan_relocs(Symbol_table* symtab,
				Layout* layout,
				Sized_relobj_file<32, false>* object,
				unsigned int data_shndx,
				unsigned int sh_type,
				const unsigned char* prelocs,
				size_t reloc_count,
				Output_section* output_section,
				bool needs_special_offset_handling,
				size_t local_symbol_count,
				const unsigned char* plocal_symbols)
{
  if (sh_type == elfcpp::SHT_RELA)
    {
      gold_error(_("%s: unsupported RELA reloc section"),
		 object->name().c_str());
      return;
    }

  gold::scan_relocs<32, false, Target_i386, Scan, Classify_reloc>(
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

void
Target_i386::do_finalize_sections(
    Layout* layout,
    const Input_objects*,
    Symbol_table* symtab)
{
  const Reloc_section* rel_plt = (this->plt_ == NULL
				  ? NULL
				  : this->plt_->rel_plt());
  layout->add_target_dynamic_tags(true, this->got_plt_, rel_plt,
				  this->rel_dyn_, true, false, false);

  // Emit any relocs we saved in an attempt to avoid generating COPY
  // relocs.
  if (this->copy_relocs_.any_saved_relocs())
    this->copy_relocs_.emit(this->rel_dyn_section(layout));

  // Set the size of the _GLOBAL_OFFSET_TABLE_ symbol to the size of
  // the .got.plt section.
  Symbol* sym = this->global_offset_table_;
  if (sym != NULL)
    {
      uint32_t data_size = this->got_plt_->current_data_size();
      symtab->get_sized_symbol<32>(sym)->set_symsize(data_size);
    }

  if (parameters->doing_static_link()
      && (this->plt_ == NULL || !this->plt_->has_irelative_section()))
    {
      // If linking statically, make sure that the __rel_iplt symbols
      // were defined if necessary, even if we didn't create a PLT.
      static const Define_symbol_in_segment syms[] =
	{
	  {
	    "__rel_iplt_start",		// name
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
	    "__rel_iplt_end",		// name
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

// Return whether a direct absolute static relocation needs to be applied.
// In cases where Scan::local() or Scan::global() has created
// a dynamic relocation other than R_386_RELATIVE, the addend
// of the relocation is carried in the data, and we must not
// apply the static relocation.

inline bool
Target_i386::Relocate::should_apply_static_reloc(const Sized_symbol<32>* gsym,
						 unsigned int r_type,
						 bool is_32bit,
						 Output_section* output_section)
{
  // If the output section is not allocated, then we didn't call
  // scan_relocs, we didn't create a dynamic reloc, and we must apply
  // the reloc here.
  if ((output_section->flags() & elfcpp::SHF_ALLOC) == 0)
    return true;

  int ref_flags = Scan::get_reference_flags(r_type);

  // For local symbols, we will have created a non-RELATIVE dynamic
  // relocation only if (a) the output is position independent,
  // (b) the relocation is absolute (not pc- or segment-relative), and
  // (c) the relocation is not 32 bits wide.
  if (gsym == NULL)
    return !(parameters->options().output_is_position_independent()
	     && (ref_flags & Symbol::ABSOLUTE_REF)
	     && !is_32bit);

  // For global symbols, we use the same helper routines used in the
  // scan pass.  If we did not create a dynamic relocation, or if we
  // created a RELATIVE dynamic relocation, we should apply the static
  // relocation.
  bool has_dyn = gsym->needs_dynamic_reloc(ref_flags);
  bool is_rel = (ref_flags & Symbol::ABSOLUTE_REF)
		&& gsym->can_use_relative_reloc(ref_flags
						& Symbol::FUNCTION_CALL);
  return !has_dyn || is_rel;
}

// Perform a relocation.

inline bool
Target_i386::Relocate::relocate(const Relocate_info<32, false>* relinfo,
				unsigned int,
				Target_i386* target,
				Output_section* output_section,
				size_t relnum,
				const unsigned char* preloc,
				const Sized_symbol<32>* gsym,
				const Symbol_value<32>* psymval,
				unsigned char* view,
				elfcpp::Elf_types<32>::Elf_Addr address,
				section_size_type view_size)
{
  const elfcpp::Rel<32, false> rel(preloc);
  unsigned int r_type = elfcpp::elf_r_type<32>(rel.get_r_info());

  if (this->skip_call_tls_get_addr_)
    {
      if ((r_type != elfcpp::R_386_PLT32
	   && r_type != elfcpp::R_386_GOT32X
	   && r_type != elfcpp::R_386_PC32)
	  || gsym == NULL
	  || strcmp(gsym->name(), "___tls_get_addr") != 0)
	{
	  gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
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

  const Sized_relobj_file<32, false>* object = relinfo->object;

  // Pick the value to use for symbols defined in shared objects.
  Symbol_value<32> symval;
  if (gsym != NULL
      && gsym->type() == elfcpp::STT_GNU_IFUNC
      && r_type == elfcpp::R_386_32
      && gsym->needs_dynamic_reloc(Scan::get_reference_flags(r_type))
      && gsym->can_use_relative_reloc(false)
      && !gsym->is_from_dynobj()
      && !gsym->is_undefined()
      && !gsym->is_preemptible())
    {
      // In this case we are generating a R_386_IRELATIVE reloc.  We
      // want to use the real value of the symbol, not the PLT offset.
    }
  else if (gsym != NULL
	   && gsym->use_plt_offset(Scan::get_reference_flags(r_type)))
    {
      symval.set_output_value(target->plt_address_for_global(gsym));
      psymval = &symval;
    }
  else if (gsym == NULL && psymval->is_ifunc_symbol())
    {
      unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());
      if (object->local_has_plt_offset(r_sym))
	{
	  symval.set_output_value(target->plt_address_for_local(object, r_sym));
	  psymval = &symval;
	}
    }

  bool baseless;

  switch (r_type)
    {
    case elfcpp::R_386_NONE:
    case elfcpp::R_386_GNU_VTINHERIT:
    case elfcpp::R_386_GNU_VTENTRY:
      break;

    case elfcpp::R_386_32:
      if (should_apply_static_reloc(gsym, r_type, true, output_section))
	Relocate_functions<32, false>::rel32(view, object, psymval);
      break;

    case elfcpp::R_386_PC32:
      if (should_apply_static_reloc(gsym, r_type, true, output_section))
	Relocate_functions<32, false>::pcrel32(view, object, psymval, address);
      break;

    case elfcpp::R_386_16:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	Relocate_functions<32, false>::rel16(view, object, psymval);
      break;

    case elfcpp::R_386_PC16:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	Relocate_functions<32, false>::pcrel16(view, object, psymval, address);
      break;

    case elfcpp::R_386_8:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	Relocate_functions<32, false>::rel8(view, object, psymval);
      break;

    case elfcpp::R_386_PC8:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	Relocate_functions<32, false>::pcrel8(view, object, psymval, address);
      break;

    case elfcpp::R_386_PLT32:
      gold_assert(gsym == NULL
		  || gsym->has_plt_offset()
		  || gsym->final_value_is_known()
		  || (gsym->is_defined()
		      && !gsym->is_from_dynobj()
		      && !gsym->is_preemptible()));
      Relocate_functions<32, false>::pcrel32(view, object, psymval, address);
      break;

    case elfcpp::R_386_GOT32:
    case elfcpp::R_386_GOT32X:
      baseless = (view[-1] & 0xc7) == 0x5;
      // R_386_GOT32 and R_386_GOT32X don't work without base register
      // when generating a position-independent output file.
      if (baseless
	  && parameters->options().output_is_position_independent())
	{
	  if(gsym)
	    gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
				   _("unexpected reloc %u against global symbol %s without base register in object file when generating a position-independent output file"),
				   r_type, gsym->demangled_name().c_str());
	  else
	    gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
				   _("unexpected reloc %u against local symbol without base register in object file when generating a position-independent output file"),
				   r_type);
	}

      // Convert
      // mov foo@GOT(%reg), %reg
      // to
      // lea foo@GOTOFF(%reg), %reg
      // if possible.
      if (rel.get_r_offset() >= 2
	  && view[-2] == 0x8b
	  && ((gsym == NULL && !psymval->is_ifunc_symbol())
	      || (gsym != NULL
		  && Target_i386::can_convert_mov_to_lea(gsym))))
	{
	  view[-2] = 0x8d;
	  elfcpp::Elf_types<32>::Elf_Addr value;
	  value = psymval->value(object, 0);
	  // Don't subtract the .got.plt section address for baseless
	  // addressing.
	  if (!baseless)
	    value -= target->got_plt_section()->address();
	  Relocate_functions<32, false>::rel32(view, value);
	}
      else
	{
	  // The GOT pointer points to the end of the GOT section.
	  // We need to subtract the size of the GOT section to get
	  // the actual offset to use in the relocation.
	  unsigned int got_offset = 0;
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(GOT_TYPE_STANDARD));
	      got_offset = (gsym->got_offset(GOT_TYPE_STANDARD)
			    - target->got_size());
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, GOT_TYPE_STANDARD));
	      got_offset = (object->local_got_offset(r_sym, GOT_TYPE_STANDARD)
			    - target->got_size());
	    }
	  // Add the .got.plt section address for baseless addressing.
	  if (baseless)
	    got_offset += target->got_plt_section()->address();
	  Relocate_functions<32, false>::rel32(view, got_offset);
	}
      break;

    case elfcpp::R_386_GOTOFF:
      {
	elfcpp::Elf_types<32>::Elf_Addr reladdr;
	reladdr = target->got_plt_section()->address();
	Relocate_functions<32, false>::pcrel32(view, object, psymval, reladdr);
      }
      break;

    case elfcpp::R_386_GOTPC:
      {
	elfcpp::Elf_types<32>::Elf_Addr value;
	value = target->got_plt_section()->address();
	Relocate_functions<32, false>::pcrel32(view, value, address);
      }
      break;

    case elfcpp::R_386_COPY:
    case elfcpp::R_386_GLOB_DAT:
    case elfcpp::R_386_JUMP_SLOT:
    case elfcpp::R_386_RELATIVE:
    case elfcpp::R_386_IRELATIVE:
      // These are outstanding tls relocs, which are unexpected when
      // linking.
    case elfcpp::R_386_TLS_TPOFF:
    case elfcpp::R_386_TLS_DTPMOD32:
    case elfcpp::R_386_TLS_DTPOFF32:
    case elfcpp::R_386_TLS_TPOFF32:
    case elfcpp::R_386_TLS_DESC:
      gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			     _("unexpected reloc %u in object file"),
			     r_type);
      break;

      // These are initial tls relocs, which are expected when
      // linking.
    case elfcpp::R_386_TLS_GD:             // Global-dynamic
    case elfcpp::R_386_TLS_GOTDESC:        // Global-dynamic (from ~oliva url)
    case elfcpp::R_386_TLS_DESC_CALL:
    case elfcpp::R_386_TLS_LDM:            // Local-dynamic
    case elfcpp::R_386_TLS_LDO_32:         // Alternate local-dynamic
    case elfcpp::R_386_TLS_IE:             // Initial-exec
    case elfcpp::R_386_TLS_IE_32:
    case elfcpp::R_386_TLS_GOTIE:
    case elfcpp::R_386_TLS_LE:             // Local-exec
    case elfcpp::R_386_TLS_LE_32:
      this->relocate_tls(relinfo, target, relnum, rel, r_type, gsym, psymval,
			 view, address, view_size);
      break;

    case elfcpp::R_386_32PLT:
    case elfcpp::R_386_TLS_GD_32:
    case elfcpp::R_386_TLS_GD_PUSH:
    case elfcpp::R_386_TLS_GD_CALL:
    case elfcpp::R_386_TLS_GD_POP:
    case elfcpp::R_386_TLS_LDM_32:
    case elfcpp::R_386_TLS_LDM_PUSH:
    case elfcpp::R_386_TLS_LDM_CALL:
    case elfcpp::R_386_TLS_LDM_POP:
    case elfcpp::R_386_USED_BY_INTEL_200:
    default:
      gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;
    }

  return true;
}

// Perform a TLS relocation.

inline void
Target_i386::Relocate::relocate_tls(const Relocate_info<32, false>* relinfo,
				    Target_i386* target,
				    size_t relnum,
				    const elfcpp::Rel<32, false>& rel,
				    unsigned int r_type,
				    const Sized_symbol<32>* gsym,
				    const Symbol_value<32>* psymval,
				    unsigned char* view,
				    elfcpp::Elf_types<32>::Elf_Addr,
				    section_size_type view_size)
{
  Output_segment* tls_segment = relinfo->layout->tls_segment();

  const Sized_relobj_file<32, false>* object = relinfo->object;

  elfcpp::Elf_types<32>::Elf_Addr value = psymval->value(object, 0);

  const bool is_final = (gsym == NULL
			 ? !parameters->options().shared()
			 : gsym->final_value_is_known());
  const tls::Tls_optimization optimized_type
      = Target_i386::optimize_tls_reloc(is_final, r_type);
  switch (r_type)
    {
    case elfcpp::R_386_TLS_GD:           // Global-dynamic
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  this->tls_gd_to_le(relinfo, relnum, tls_segment,
			     rel, r_type, value, view,
			     view_size);
	  break;
	}
      else
	{
	  unsigned int got_type = (optimized_type == tls::TLSOPT_TO_IE
				   ? GOT_TYPE_TLS_NOFFSET
				   : GOT_TYPE_TLS_PAIR);
	  unsigned int got_offset;
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(got_type));
	      got_offset = gsym->got_offset(got_type) - target->got_size();
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      got_offset = (object->local_got_offset(r_sym, got_type)
			    - target->got_size());
	    }
	  if (optimized_type == tls::TLSOPT_TO_IE)
	    {
	      this->tls_gd_to_ie(relinfo, relnum, rel, r_type,
				 got_offset, view, view_size);
	      break;
	    }
	  else if (optimized_type == tls::TLSOPT_NONE)
	    {
	      // Relocate the field with the offset of the pair of GOT
	      // entries.
	      Relocate_functions<32, false>::rel32(view, got_offset);
	      break;
	    }
	}
      gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;

    case elfcpp::R_386_TLS_GOTDESC:      // Global-dynamic (from ~oliva url)
    case elfcpp::R_386_TLS_DESC_CALL:
      this->local_dynamic_type_ = LOCAL_DYNAMIC_GNU;
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  this->tls_desc_gd_to_le(relinfo, relnum, tls_segment,
				  rel, r_type, value, view,
				  view_size);
	  break;
	}
      else
	{
	  unsigned int got_type = (optimized_type == tls::TLSOPT_TO_IE
				   ? GOT_TYPE_TLS_NOFFSET
				   : GOT_TYPE_TLS_DESC);
	  unsigned int got_offset = 0;
	  if (r_type == elfcpp::R_386_TLS_GOTDESC
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
	      unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      got_offset += (object->local_got_offset(r_sym, got_type)
			     - target->got_size());
	    }
	  if (optimized_type == tls::TLSOPT_TO_IE)
	    {
	      this->tls_desc_gd_to_ie(relinfo, relnum, rel, r_type,
				      got_offset, view, view_size);
	      break;
	    }
	  else if (optimized_type == tls::TLSOPT_NONE)
	    {
	      if (r_type == elfcpp::R_386_TLS_GOTDESC)
		{
		  // Relocate the field with the offset of the pair of GOT
		  // entries.
		  Relocate_functions<32, false>::rel32(view, got_offset);
		}
	      break;
	    }
	}
      gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;

    case elfcpp::R_386_TLS_LDM:          // Local-dynamic
      if (this->local_dynamic_type_ == LOCAL_DYNAMIC_SUN)
	{
	  gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
				 _("both SUN and GNU model "
				   "TLS relocations"));
	  break;
	}
      this->local_dynamic_type_ = LOCAL_DYNAMIC_GNU;
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  this->tls_ld_to_le(relinfo, relnum, tls_segment, rel, r_type,
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
	  Relocate_functions<32, false>::rel32(view, got_offset);
	  break;
	}
      gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;

    case elfcpp::R_386_TLS_LDO_32:       // Alternate local-dynamic
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  // This reloc can appear in debugging sections, in which
	  // case we must not convert to local-exec.  We decide what
	  // to do based on whether the section is marked as
	  // containing executable code.  That is what the GNU linker
	  // does as well.
	  elfcpp::Shdr<32, false> shdr(relinfo->data_shdr);
	  if ((shdr.get_sh_flags() & elfcpp::SHF_EXECINSTR) != 0)
	    {
	      if (tls_segment == NULL)
		{
		  gold_assert(parameters->errors()->error_count() > 0
			      || issue_undefined_symbol_error(gsym));
		  return;
		}
	      value -= tls_segment->memsz();
	    }
	}
      Relocate_functions<32, false>::rel32(view, value);
      break;

    case elfcpp::R_386_TLS_IE:           // Initial-exec
    case elfcpp::R_386_TLS_GOTIE:
    case elfcpp::R_386_TLS_IE_32:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  Target_i386::Relocate::tls_ie_to_le(relinfo, relnum, tls_segment,
					      rel, r_type, value, view,
					      view_size);
	  break;
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the tp-relative offset of the symbol.
	  unsigned int got_type = (r_type == elfcpp::R_386_TLS_IE_32
				   ? GOT_TYPE_TLS_OFFSET
				   : GOT_TYPE_TLS_NOFFSET);
	  unsigned int got_offset;
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(got_type));
	      got_offset = gsym->got_offset(got_type);
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      got_offset = object->local_got_offset(r_sym, got_type);
	    }
	  // For the R_386_TLS_IE relocation, we need to apply the
	  // absolute address of the GOT entry.
	  if (r_type == elfcpp::R_386_TLS_IE)
	    got_offset += target->got_plt_section()->address();
	  // All GOT offsets are relative to the end of the GOT.
	  got_offset -= target->got_size();
	  Relocate_functions<32, false>::rel32(view, got_offset);
	  break;
	}
      gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;

    case elfcpp::R_386_TLS_LE:           // Local-exec
      // If we're creating a shared library, a dynamic relocation will
      // have been created for this location, so do not apply it now.
      if (!parameters->options().shared())
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  value -= tls_segment->memsz();
	  Relocate_functions<32, false>::rel32(view, value);
	}
      break;

    case elfcpp::R_386_TLS_LE_32:
      // If we're creating a shared library, a dynamic relocation will
      // have been created for this location, so do not apply it now.
      if (!parameters->options().shared())
	{
	  if (tls_segment == NULL)
	    {
	      gold_assert(parameters->errors()->error_count() > 0
			  || issue_undefined_symbol_error(gsym));
	      return;
	    }
	  value = tls_segment->memsz() - value;
	  Relocate_functions<32, false>::rel32(view, value);
	}
      break;
    }
}

// Do a relocation in which we convert a TLS General-Dynamic to a
// Local-Exec.

inline void
Target_i386::Relocate::tls_gd_to_le(const Relocate_info<32, false>* relinfo,
				    size_t relnum,
				    Output_segment* tls_segment,
				    const elfcpp::Rel<32, false>& rel,
				    unsigned int,
				    elfcpp::Elf_types<32>::Elf_Addr value,
				    unsigned char* view,
				    section_size_type view_size)
{
  // leal foo(,%ebx,1),%eax; call ___tls_get_addr@PLT
  //  ==> movl %gs:0,%eax; subl $foo@tpoff,%eax
  // leal foo(%ebx),%eax; call ___tls_get_addr@PLT
  //  ==> movl %gs:0,%eax; subl $foo@tpoff,%eax
  // leal foo(%reg),%eax; call *___tls_get_addr@GOT(%reg)
  //  ==> movl %gs:0,%eax; subl $foo@tpoff,%eax

  tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -2);
  tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 9);

  unsigned char op1 = view[-1];
  unsigned char op2 = view[-2];
  unsigned char op3 = view[4];

  tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		 op2 == 0x8d || op2 == 0x04);
  tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		 op3 == 0xe8 || op3 == 0xff);

  int roff = 5;

  if (op2 == 0x04)
    {
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -3);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(), view[-3] == 0x8d);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     ((op1 & 0xc7) == 0x05 && op1 != (4 << 3)));
      memcpy(view - 3, "\x65\xa1\0\0\0\0\x81\xe8\0\0\0", 12);
    }
  else
    {
      unsigned char reg = op1 & 7;
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     ((op1 & 0xf8) == 0x80
		      && reg != 4
		      && reg != 0
		      && (op3 == 0xe8 || (view[5] & 0x7) == reg)));
      if (op3 == 0xff
	  || (rel.get_r_offset() + 9 < view_size
	      && view[9] == 0x90))
	{
	  // There is an indirect call or a trailing nop.  Use the size
	  // byte subl.
	  memcpy(view - 2, "\x65\xa1\0\0\0\0\x81\xe8\0\0\0", 12);
	  roff = 6;
	}
      else
	{
	  // Use the five byte subl.
	  memcpy(view - 2, "\x65\xa1\0\0\0\0\x2d\0\0\0", 11);
	}
    }

  value = tls_segment->memsz() - value;
  Relocate_functions<32, false>::rel32(view + roff, value);

  // The next reloc should be a PLT32 reloc against __tls_get_addr.
  // We can skip it.
  this->skip_call_tls_get_addr_ = true;
}

// Do a relocation in which we convert a TLS General-Dynamic to an
// Initial-Exec.

inline void
Target_i386::Relocate::tls_gd_to_ie(const Relocate_info<32, false>* relinfo,
				    size_t relnum,
				    const elfcpp::Rel<32, false>& rel,
				    unsigned int,
				    elfcpp::Elf_types<32>::Elf_Addr value,
				    unsigned char* view,
				    section_size_type view_size)
{
  // leal foo(,%ebx,1),%eax; call ___tls_get_addr@PLT
  //  ==> movl %gs:0,%eax; addl foo@gotntpoff(%ebx),%eax
  // leal foo(%ebx),%eax; call ___tls_get_addr@PLT; nop
  //  ==> movl %gs:0,%eax; addl foo@gotntpoff(%ebx),%eax
  // leal foo(%reg),%eax; call *___tls_get_addr@GOT(%reg)
  //  ==> movl %gs:0,%eax; addl foo@gotntpoff(%reg),%eax

  tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -2);
  tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 9);

  unsigned char op1 = view[-1];
  unsigned char op2 = view[-2];
  unsigned char op3 = view[4];

  tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		 op2 == 0x8d || op2 == 0x04);
  tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		 op3 == 0xe8 || op3 == 0xff);

  int roff;

  if (op2 == 0x04)
    {
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -3);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(), view[-3] == 0x8d);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     ((op1 & 0xc7) == 0x05 && op1 != (4 << 3)));
      roff = 5;
    }
  else
    {
      unsigned char reg = op1 & 7;
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 10);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     ((op1 & 0xf8) == 0x80
		      && reg != 4
		      && reg != 0
		      && ((op3 == 0xe8 && view[9] == 0x90)
			   || (view[5] & 0x7) == reg)));
      roff = 6;
    }

  memcpy(view + roff - 8, "\x65\xa1\0\0\0\0\x03\x83\0\0\0", 12);
  Relocate_functions<32, false>::rel32(view + roff, value);

  // The next reloc should be a PLT32 reloc against __tls_get_addr.
  // We can skip it.
  this->skip_call_tls_get_addr_ = true;
}

// Do a relocation in which we convert a TLS_GOTDESC or TLS_DESC_CALL
// General-Dynamic to a Local-Exec.

inline void
Target_i386::Relocate::tls_desc_gd_to_le(
    const Relocate_info<32, false>* relinfo,
    size_t relnum,
    Output_segment* tls_segment,
    const elfcpp::Rel<32, false>& rel,
    unsigned int r_type,
    elfcpp::Elf_types<32>::Elf_Addr value,
    unsigned char* view,
    section_size_type view_size)
{
  if (r_type == elfcpp::R_386_TLS_GOTDESC)
    {
      // leal foo@TLSDESC(%ebx), %eax
      // ==> leal foo@NTPOFF, %eax
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -2);
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 4);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     view[-2] == 0x8d && view[-1] == 0x83);
      view[-1] = 0x05;
      value -= tls_segment->memsz();
      Relocate_functions<32, false>::rel32(view, value);
    }
  else
    {
      // call *foo@TLSCALL(%eax)
      // ==> nop; nop
      gold_assert(r_type == elfcpp::R_386_TLS_DESC_CALL);
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 2);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     view[0] == 0xff && view[1] == 0x10);
      view[0] = 0x66;
      view[1] = 0x90;
    }
}

// Do a relocation in which we convert a TLS_GOTDESC or TLS_DESC_CALL
// General-Dynamic to an Initial-Exec.

inline void
Target_i386::Relocate::tls_desc_gd_to_ie(
    const Relocate_info<32, false>* relinfo,
    size_t relnum,
    const elfcpp::Rel<32, false>& rel,
    unsigned int r_type,
    elfcpp::Elf_types<32>::Elf_Addr value,
    unsigned char* view,
    section_size_type view_size)
{
  if (r_type == elfcpp::R_386_TLS_GOTDESC)
    {
      // leal foo@TLSDESC(%ebx), %eax
      // ==> movl foo@GOTNTPOFF(%ebx), %eax
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -2);
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 4);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     view[-2] == 0x8d && view[-1] == 0x83);
      view[-2] = 0x8b;
      Relocate_functions<32, false>::rel32(view, value);
    }
  else
    {
      // call *foo@TLSCALL(%eax)
      // ==> nop; nop
      gold_assert(r_type == elfcpp::R_386_TLS_DESC_CALL);
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 2);
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     view[0] == 0xff && view[1] == 0x10);
      view[0] = 0x66;
      view[1] = 0x90;
    }
}

// Do a relocation in which we convert a TLS Local-Dynamic to a
// Local-Exec.

inline void
Target_i386::Relocate::tls_ld_to_le(const Relocate_info<32, false>* relinfo,
				    size_t relnum,
				    Output_segment*,
				    const elfcpp::Rel<32, false>& rel,
				    unsigned int,
				    elfcpp::Elf_types<32>::Elf_Addr,
				    unsigned char* view,
				    section_size_type view_size)
{
  // leal foo(%ebx), %eax; call ___tls_get_addr@PLT
  // ==> movl %gs:0,%eax; nop; leal 0(%esi,1),%esi
  // leal foo(%reg), %eax; call call *___tls_get_addr@GOT(%reg)
  // ==> movl %gs:0,%eax; leal (%esi),%esi

  tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -2);

  unsigned char op1 = view[-1];
  unsigned char op2 = view[-2];
  unsigned char op3 = view[4];

  tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		 op3 == 0xe8 || op3 == 0xff);
  tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size,
		   op3 == 0xe8 ? 9 : 10);

  // FIXME: Does this test really always pass?
  tls::check_tls(relinfo, relnum, rel.get_r_offset(), op2 == 0x8d);

  unsigned char reg = op1 & 7;
  tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		 ((op1 & 0xf8) == 0x80
		  && reg != 4
		  && reg != 0
		  && (op3 == 0xe8 || (view[5] & 0x7) == reg)));

  if (op3 == 0xe8)
    memcpy(view - 2, "\x65\xa1\0\0\0\0\x90\x8d\x74\x26\0", 11);
  else
    memcpy(view - 2, "\x65\xa1\0\0\0\0\x8d\xb6\0\0\0\0", 12);

  // The next reloc should be a PLT32 reloc against __tls_get_addr.
  // We can skip it.
  this->skip_call_tls_get_addr_ = true;
}

// Do a relocation in which we convert a TLS Initial-Exec to a
// Local-Exec.

inline void
Target_i386::Relocate::tls_ie_to_le(const Relocate_info<32, false>* relinfo,
				    size_t relnum,
				    Output_segment* tls_segment,
				    const elfcpp::Rel<32, false>& rel,
				    unsigned int r_type,
				    elfcpp::Elf_types<32>::Elf_Addr value,
				    unsigned char* view,
				    section_size_type view_size)
{
  // We have to actually change the instructions, which means that we
  // need to examine the opcodes to figure out which instruction we
  // are looking at.
  if (r_type == elfcpp::R_386_TLS_IE)
    {
      // movl %gs:XX,%eax  ==>  movl $YY,%eax
      // movl %gs:XX,%reg  ==>  movl $YY,%reg
      // addl %gs:XX,%reg  ==>  addl $YY,%reg
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -1);
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 4);

      unsigned char op1 = view[-1];
      if (op1 == 0xa1)
	{
	  // movl XX,%eax  ==>  movl $YY,%eax
	  view[-1] = 0xb8;
	}
      else
	{
	  tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -2);

	  unsigned char op2 = view[-2];
	  if (op2 == 0x8b)
	    {
	      // movl XX,%reg  ==>  movl $YY,%reg
	      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
			     (op1 & 0xc7) == 0x05);
	      view[-2] = 0xc7;
	      view[-1] = 0xc0 | ((op1 >> 3) & 7);
	    }
	  else if (op2 == 0x03)
	    {
	      // addl XX,%reg  ==>  addl $YY,%reg
	      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
			     (op1 & 0xc7) == 0x05);
	      view[-2] = 0x81;
	      view[-1] = 0xc0 | ((op1 >> 3) & 7);
	    }
	  else
	    tls::check_tls(relinfo, relnum, rel.get_r_offset(), 0);
	}
    }
  else
    {
      // subl %gs:XX(%reg1),%reg2  ==>  subl $YY,%reg2
      // movl %gs:XX(%reg1),%reg2  ==>  movl $YY,%reg2
      // addl %gs:XX(%reg1),%reg2  ==>  addl $YY,$reg2
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, -2);
      tls::check_range(relinfo, relnum, rel.get_r_offset(), view_size, 4);

      unsigned char op1 = view[-1];
      unsigned char op2 = view[-2];
      tls::check_tls(relinfo, relnum, rel.get_r_offset(),
		     (op1 & 0xc0) == 0x80 && (op1 & 7) != 4);
      if (op2 == 0x8b)
	{
	  // movl %gs:XX(%reg1),%reg2  ==>  movl $YY,%reg2
	  view[-2] = 0xc7;
	  view[-1] = 0xc0 | ((op1 >> 3) & 7);
	}
      else if (op2 == 0x2b)
	{
	  // subl %gs:XX(%reg1),%reg2  ==>  subl $YY,%reg2
	  view[-2] = 0x81;
	  view[-1] = 0xe8 | ((op1 >> 3) & 7);
	}
      else if (op2 == 0x03)
	{
	  // addl %gs:XX(%reg1),%reg2  ==>  addl $YY,$reg2
	  view[-2] = 0x81;
	  view[-1] = 0xc0 | ((op1 >> 3) & 7);
	}
      else
	tls::check_tls(relinfo, relnum, rel.get_r_offset(), 0);
    }

  value = tls_segment->memsz() - value;
  if (r_type == elfcpp::R_386_TLS_IE || r_type == elfcpp::R_386_TLS_GOTIE)
    value = - value;

  Relocate_functions<32, false>::rel32(view, value);
}

// Relocate section data.

void
Target_i386::relocate_section(const Relocate_info<32, false>* relinfo,
			      unsigned int sh_type,
			      const unsigned char* prelocs,
			      size_t reloc_count,
			      Output_section* output_section,
			      bool needs_special_offset_handling,
			      unsigned char* view,
			      elfcpp::Elf_types<32>::Elf_Addr address,
			      section_size_type view_size,
			      const Reloc_symbol_changes* reloc_symbol_changes)
{
  gold_assert(sh_type == elfcpp::SHT_REL);

  gold::relocate_section<32, false, Target_i386, Relocate,
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

// Return the size of a relocation while scanning during a relocatable
// link.

unsigned int
Target_i386::Classify_reloc::get_size_for_reloc(
    unsigned int r_type,
    Relobj* object)
{
  switch (r_type)
    {
    case elfcpp::R_386_NONE:
    case elfcpp::R_386_GNU_VTINHERIT:
    case elfcpp::R_386_GNU_VTENTRY:
    case elfcpp::R_386_TLS_GD:            // Global-dynamic
    case elfcpp::R_386_TLS_GOTDESC:       // Global-dynamic (from ~oliva url)
    case elfcpp::R_386_TLS_DESC_CALL:
    case elfcpp::R_386_TLS_LDM:           // Local-dynamic
    case elfcpp::R_386_TLS_LDO_32:        // Alternate local-dynamic
    case elfcpp::R_386_TLS_IE:            // Initial-exec
    case elfcpp::R_386_TLS_IE_32:
    case elfcpp::R_386_TLS_GOTIE:
    case elfcpp::R_386_TLS_LE:            // Local-exec
    case elfcpp::R_386_TLS_LE_32:
      return 0;

    case elfcpp::R_386_32:
    case elfcpp::R_386_PC32:
    case elfcpp::R_386_GOT32:
    case elfcpp::R_386_GOT32X:
    case elfcpp::R_386_PLT32:
    case elfcpp::R_386_GOTOFF:
    case elfcpp::R_386_GOTPC:
     return 4;

    case elfcpp::R_386_16:
    case elfcpp::R_386_PC16:
      return 2;

    case elfcpp::R_386_8:
    case elfcpp::R_386_PC8:
      return 1;

      // These are relocations which should only be seen by the
      // dynamic linker, and should never be seen here.
    case elfcpp::R_386_COPY:
    case elfcpp::R_386_GLOB_DAT:
    case elfcpp::R_386_JUMP_SLOT:
    case elfcpp::R_386_RELATIVE:
    case elfcpp::R_386_IRELATIVE:
    case elfcpp::R_386_TLS_TPOFF:
    case elfcpp::R_386_TLS_DTPMOD32:
    case elfcpp::R_386_TLS_DTPOFF32:
    case elfcpp::R_386_TLS_TPOFF32:
    case elfcpp::R_386_TLS_DESC:
      object->error(_("unexpected reloc %u in object file"), r_type);
      return 0;

    case elfcpp::R_386_32PLT:
    case elfcpp::R_386_TLS_GD_32:
    case elfcpp::R_386_TLS_GD_PUSH:
    case elfcpp::R_386_TLS_GD_CALL:
    case elfcpp::R_386_TLS_GD_POP:
    case elfcpp::R_386_TLS_LDM_32:
    case elfcpp::R_386_TLS_LDM_PUSH:
    case elfcpp::R_386_TLS_LDM_CALL:
    case elfcpp::R_386_TLS_LDM_POP:
    case elfcpp::R_386_USED_BY_INTEL_200:
    default:
      object->error(_("unsupported reloc %u in object file"), r_type);
      return 0;
    }
}

// Scan the relocs during a relocatable link.

void
Target_i386::scan_relocatable_relocs(Symbol_table* symtab,
				     Layout* layout,
				     Sized_relobj_file<32, false>* object,
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
  typedef gold::Default_scan_relocatable_relocs<Classify_reloc>
      Scan_relocatable_relocs;

  gold_assert(sh_type == elfcpp::SHT_REL);

  gold::scan_relocatable_relocs<32, false, Scan_relocatable_relocs>(
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

void
Target_i386::emit_relocs_scan(Symbol_table* symtab,
			      Layout* layout,
			      Sized_relobj_file<32, false>* object,
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
  typedef gold::Default_classify_reloc<elfcpp::SHT_REL, 32, false>
      Classify_reloc;
  typedef gold::Default_emit_relocs_strategy<Classify_reloc>
      Emit_relocs_strategy;

  gold_assert(sh_type == elfcpp::SHT_REL);

  gold::scan_relocatable_relocs<32, false, Emit_relocs_strategy>(
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

// Emit relocations for a section.

void
Target_i386::relocate_relocs(
    const Relocate_info<32, false>* relinfo,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    elfcpp::Elf_types<32>::Elf_Off offset_in_output_section,
    unsigned char* view,
    elfcpp::Elf_types<32>::Elf_Addr view_address,
    section_size_type view_size,
    unsigned char* reloc_view,
    section_size_type reloc_view_size)
{
  gold_assert(sh_type == elfcpp::SHT_REL);

  gold::relocate_relocs<32, false, Classify_reloc>(
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

uint64_t
Target_i386::do_dynsym_value(const Symbol* gsym) const
{
  gold_assert(gsym->is_from_dynobj() && gsym->has_plt_offset());
  return this->plt_address_for_global(gsym);
}

// Return a string used to fill a code section with nops to take up
// the specified length.

std::string
Target_i386::do_code_fill(section_size_type length) const
{
  if (length >= 16)
    {
      // Build a jmp instruction to skip over the bytes.
      unsigned char jmp[5];
      jmp[0] = 0xe9;
      elfcpp::Swap_unaligned<32, false>::writeval(jmp + 1, length - 5);
      return (std::string(reinterpret_cast<char*>(&jmp[0]), 5)
	      + std::string(length - 5, static_cast<char>(0x90)));
    }

  // Nop sequences of various lengths.
  const char nop1[1] = { '\x90' };                   // nop
  const char nop2[2] = { '\x66', '\x90' };           // xchg %ax %ax
  const char nop3[3] = { '\x8d', '\x76', '\x00' };   // leal 0(%esi),%esi
  const char nop4[4] = { '\x8d', '\x74', '\x26',     // leal 0(%esi,1),%esi
			 '\x00'};
  const char nop5[5] = { '\x90', '\x8d', '\x74',     // nop
			 '\x26', '\x00' };           // leal 0(%esi,1),%esi
  const char nop6[6] = { '\x8d', '\xb6', '\x00',     // leal 0L(%esi),%esi
			 '\x00', '\x00', '\x00' };
  const char nop7[7] = { '\x8d', '\xb4', '\x26',     // leal 0L(%esi,1),%esi
			 '\x00', '\x00', '\x00',
			 '\x00' };
  const char nop8[8] = { '\x90', '\x8d', '\xb4',     // nop
			 '\x26', '\x00', '\x00',     // leal 0L(%esi,1),%esi
			 '\x00', '\x00' };
  const char nop9[9] = { '\x89', '\xf6', '\x8d',     // movl %esi,%esi
			 '\xbc', '\x27', '\x00',     // leal 0L(%edi,1),%edi
			 '\x00', '\x00', '\x00' };
  const char nop10[10] = { '\x8d', '\x76', '\x00',   // leal 0(%esi),%esi
			   '\x8d', '\xbc', '\x27',   // leal 0L(%edi,1),%edi
			   '\x00', '\x00', '\x00',
			   '\x00' };
  const char nop11[11] = { '\x8d', '\x74', '\x26',   // leal 0(%esi,1),%esi
			   '\x00', '\x8d', '\xbc',   // leal 0L(%edi,1),%edi
			   '\x27', '\x00', '\x00',
			   '\x00', '\x00' };
  const char nop12[12] = { '\x8d', '\xb6', '\x00',   // leal 0L(%esi),%esi
			   '\x00', '\x00', '\x00',   // leal 0L(%edi),%edi
			   '\x8d', '\xbf', '\x00',
			   '\x00', '\x00', '\x00' };
  const char nop13[13] = { '\x8d', '\xb6', '\x00',   // leal 0L(%esi),%esi
			   '\x00', '\x00', '\x00',   // leal 0L(%edi,1),%edi
			   '\x8d', '\xbc', '\x27',
			   '\x00', '\x00', '\x00',
			   '\x00' };
  const char nop14[14] = { '\x8d', '\xb4', '\x26',   // leal 0L(%esi,1),%esi
			   '\x00', '\x00', '\x00',   // leal 0L(%edi,1),%edi
			   '\x00', '\x8d', '\xbc',
			   '\x27', '\x00', '\x00',
			   '\x00', '\x00' };
  const char nop15[15] = { '\xeb', '\x0d', '\x90',   // jmp .+15
			   '\x90', '\x90', '\x90',   // nop,nop,nop,...
			   '\x90', '\x90', '\x90',
			   '\x90', '\x90', '\x90',
			   '\x90', '\x90', '\x90' };

  const char* nops[16] = {
    NULL,
    nop1, nop2, nop3, nop4, nop5, nop6, nop7,
    nop8, nop9, nop10, nop11, nop12, nop13, nop14, nop15
  };

  return std::string(nops[length], length);
}

// Return the value to use for the base of a DW_EH_PE_datarel offset
// in an FDE.  Solaris and SVR4 use DW_EH_PE_datarel because their
// assembler can not write out the difference between two labels in
// different sections, so instead of using a pc-relative value they
// use an offset from the GOT.

uint64_t
Target_i386::do_ehframe_datarel_base() const
{
  gold_assert(this->global_offset_table_ != NULL);
  Symbol* sym = this->global_offset_table_;
  Sized_symbol<32>* ssym = static_cast<Sized_symbol<32>*>(sym);
  return ssym->value();
}

// Return whether SYM should be treated as a call to a non-split
// function.  We don't want that to be true of a call to a
// get_pc_thunk function.

bool
Target_i386::do_is_call_to_non_split(const Symbol* sym,
				     const unsigned char*,
				     const unsigned char*,
				     section_size_type) const
{
  return (sym->type() == elfcpp::STT_FUNC
	  && !is_prefix_of("__i686.get_pc_thunk.", sym->name()));
}

// FNOFFSET in section SHNDX in OBJECT is the start of a function
// compiled with -fsplit-stack.  The function calls non-split-stack
// code.  We have to change the function so that it always ensures
// that it has enough stack space to run some random function.

void
Target_i386::do_calls_non_split(Relobj* object, unsigned int shndx,
				       section_offset_type fnoffset,
				       section_size_type fnsize,
				       const unsigned char*,
				       size_t,
				       unsigned char* view,
				       section_size_type view_size,
				       std::string* from,
				       std::string* to) const
{
  // The function starts with a comparison of the stack pointer and a
  // field in the TCB.  This is followed by a jump.

  // cmp %gs:NN,%esp
  if (this->match_view(view, view_size, fnoffset, "\x65\x3b\x25", 3)
      && fnsize > 7)
    {
      // We will call __morestack if the carry flag is set after this
      // comparison.  We turn the comparison into an stc instruction
      // and some nops.
      view[fnoffset] = '\xf9';
      this->set_view_to_nop(view, view_size, fnoffset + 1, 6);
    }
  // lea NN(%esp),%ecx
  // lea NN(%esp),%edx
  else if ((this->match_view(view, view_size, fnoffset, "\x8d\x8c\x24", 3)
	    || this->match_view(view, view_size, fnoffset, "\x8d\x94\x24", 3))
	   && fnsize > 7)
    {
      // This is loading an offset from the stack pointer for a
      // comparison.  The offset is negative, so we decrease the
      // offset by the amount of space we need for the stack.  This
      // means we will avoid calling __morestack if there happens to
      // be plenty of space on the stack already.
      unsigned char* pval = view + fnoffset + 3;
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

// The selector for i386 object files.  Note this is never instantiated
// directly.  It's only used in Target_selector_i386_nacl, below.

class Target_selector_i386 : public Target_selector_freebsd
{
public:
  Target_selector_i386()
    : Target_selector_freebsd(elfcpp::EM_386, 32, false,
			      "elf32-i386", "elf32-i386-freebsd",
			      "elf_i386")
  { }

  Target*
  do_instantiate_target()
  { return new Target_i386(); }
};

// NaCl variant.  It uses different PLT contents.

class Output_data_plt_i386_nacl : public Output_data_plt_i386
{
 public:
  Output_data_plt_i386_nacl(Layout* layout,
			    Output_data_got_plt_i386* got_plt,
			    Output_data_space* got_irelative)
    : Output_data_plt_i386(layout, plt_entry_size, got_plt, got_irelative)
  { }

 protected:
  virtual unsigned int
  do_get_plt_entry_size() const
  { return plt_entry_size; }

  virtual void
  do_add_eh_frame(Layout* layout)
  {
    layout->add_eh_frame_for_plt(this, plt_eh_frame_cie, plt_eh_frame_cie_size,
				 plt_eh_frame_fde, plt_eh_frame_fde_size);
  }

  // The size of an entry in the PLT.
  static const int plt_entry_size = 64;

  // The .eh_frame unwind information for the PLT.
  static const int plt_eh_frame_fde_size = 32;
  static const unsigned char plt_eh_frame_fde[plt_eh_frame_fde_size];
};

class Output_data_plt_i386_nacl_exec : public Output_data_plt_i386_nacl
{
public:
  Output_data_plt_i386_nacl_exec(Layout* layout,
				 Output_data_got_plt_i386* got_plt,
				 Output_data_space* got_irelative)
    : Output_data_plt_i386_nacl(layout, got_plt, got_irelative)
  { }

 protected:
  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  elfcpp::Elf_types<32>::Elf_Addr got_address);

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    elfcpp::Elf_types<32>::Elf_Addr got_address,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_rel_offset);

 private:
  // The first entry in the PLT for an executable.
  static const unsigned char first_plt_entry[plt_entry_size];

  // Other entries in the PLT for an executable.
  static const unsigned char plt_entry[plt_entry_size];
};

class Output_data_plt_i386_nacl_dyn : public Output_data_plt_i386_nacl
{
 public:
  Output_data_plt_i386_nacl_dyn(Layout* layout,
				Output_data_got_plt_i386* got_plt,
				Output_data_space* got_irelative)
    : Output_data_plt_i386_nacl(layout, got_plt, got_irelative)
  { }

 protected:
  virtual void
  do_fill_first_plt_entry(unsigned char* pov, elfcpp::Elf_types<32>::Elf_Addr);

  virtual unsigned int
  do_fill_plt_entry(unsigned char* pov,
		    elfcpp::Elf_types<32>::Elf_Addr,
		    unsigned int got_offset,
		    unsigned int plt_offset,
		    unsigned int plt_rel_offset);

 private:
  // The first entry in the PLT for a shared object.
  static const unsigned char first_plt_entry[plt_entry_size];

  // Other entries in the PLT for a shared object.
  static const unsigned char plt_entry[plt_entry_size];
};

class Target_i386_nacl : public Target_i386
{
 public:
  Target_i386_nacl()
    : Target_i386(&i386_nacl_info)
  { }

 protected:
  virtual Output_data_plt_i386*
  do_make_data_plt(Layout* layout,
		   Output_data_got_plt_i386* got_plt,
		   Output_data_space* got_irelative,
		   bool dyn)
  {
    if (dyn)
      return new Output_data_plt_i386_nacl_dyn(layout, got_plt, got_irelative);
    else
      return new Output_data_plt_i386_nacl_exec(layout, got_plt, got_irelative);
  }

  virtual std::string
  do_code_fill(section_size_type length) const;

 private:
  static const Target::Target_info i386_nacl_info;
};

const Target::Target_info Target_i386_nacl::i386_nacl_info =
{
  32,			// size
  false,		// is_big_endian
  elfcpp::EM_386,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/lib/ld-nacl-x86-32.so.1", // dynamic_linker
  0x20000,		// default_text_segment_address
  0x10000,		// abi_pagesize (overridable by -z max-page-size)
  0x10000,		// common_pagesize (overridable by -z common-page-size)
  true,                 // isolate_execinstr
  0x10000000,           // rosegment_gap
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

#define	NACLMASK	0xe0            // 32-byte alignment mask

const unsigned char
Output_data_plt_i386_nacl_exec::first_plt_entry[plt_entry_size] =
{
  0xff, 0x35,                          // pushl contents of memory address
  0, 0, 0, 0,                          // replaced with address of .got + 4
  0x8b, 0x0d,                          // movl contents of address, %ecx
  0, 0, 0, 0,                          // replaced with address of .got + 8
  0x83, 0xe1, NACLMASK,                // andl $NACLMASK, %ecx
  0xff, 0xe1,                          // jmp *%ecx
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90
};

void
Output_data_plt_i386_nacl_exec::do_fill_first_plt_entry(
    unsigned char* pov,
    elfcpp::Elf_types<32>::Elf_Addr got_address)
{
  memcpy(pov, first_plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2, got_address + 4);
  elfcpp::Swap<32, false>::writeval(pov + 8, got_address + 8);
}

// The first entry in the PLT for a shared object.

const unsigned char
Output_data_plt_i386_nacl_dyn::first_plt_entry[plt_entry_size] =
{
  0xff, 0xb3, 4, 0, 0, 0,	// pushl 4(%ebx)
  0x8b, 0x4b, 0x08,		// mov 0x8(%ebx), %ecx
  0x83, 0xe1, NACLMASK,         // andl $NACLMASK, %ecx
  0xff, 0xe1,                   // jmp *%ecx
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90,  // nops
  0x90, 0x90, 0x90, 0x90, 0x90   // nops
};

void
Output_data_plt_i386_nacl_dyn::do_fill_first_plt_entry(
    unsigned char* pov,
    elfcpp::Elf_types<32>::Elf_Addr)
{
  memcpy(pov, first_plt_entry, plt_entry_size);
}

// Subsequent entries in the PLT for an executable.

const unsigned char
Output_data_plt_i386_nacl_exec::plt_entry[plt_entry_size] =
{
  0x8b, 0x0d,                    // movl contents of address, %ecx */
  0, 0, 0, 0,                    // replaced with address of symbol in .got
  0x83, 0xe1, NACLMASK,          // andl $NACLMASK, %ecx
  0xff, 0xe1,                    // jmp *%ecx

  // Pad to the next 32-byte boundary with nop instructions.
  0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,

  // Lazy GOT entries point here (32-byte aligned).
  0x68,                       // pushl immediate
  0, 0, 0, 0,                 // replaced with offset into relocation table
  0xe9,                       // jmp relative
  0, 0, 0, 0,                 // replaced with offset to start of .plt

  // Pad to the next 32-byte boundary with nop instructions.
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90
};

unsigned int
Output_data_plt_i386_nacl_exec::do_fill_plt_entry(
    unsigned char* pov,
    elfcpp::Elf_types<32>::Elf_Addr got_address,
    unsigned int got_offset,
    unsigned int plt_offset,
    unsigned int plt_rel_offset)
{
  memcpy(pov, plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2,
					      got_address + got_offset);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 33, plt_rel_offset);
  elfcpp::Swap<32, false>::writeval(pov + 38, - (plt_offset + 38 + 4));
  return 32;
}

// Subsequent entries in the PLT for a shared object.

const unsigned char
Output_data_plt_i386_nacl_dyn::plt_entry[plt_entry_size] =
{
  0x8b, 0x8b,          // movl offset(%ebx), %ecx
  0, 0, 0, 0,          // replaced with offset of symbol in .got
  0x83, 0xe1, 0xe0,    // andl $NACLMASK, %ecx
  0xff, 0xe1,          // jmp *%ecx

  // Pad to the next 32-byte boundary with nop instructions.
  0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,

  // Lazy GOT entries point here (32-byte aligned).
  0x68,                // pushl immediate
  0, 0, 0, 0,          // replaced with offset into relocation table.
  0xe9,                // jmp relative
  0, 0, 0, 0,          // replaced with offset to start of .plt.

  // Pad to the next 32-byte boundary with nop instructions.
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90
};

unsigned int
Output_data_plt_i386_nacl_dyn::do_fill_plt_entry(
    unsigned char* pov,
    elfcpp::Elf_types<32>::Elf_Addr,
    unsigned int got_offset,
    unsigned int plt_offset,
    unsigned int plt_rel_offset)
{
  memcpy(pov, plt_entry, plt_entry_size);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 2, got_offset);
  elfcpp::Swap_unaligned<32, false>::writeval(pov + 33, plt_rel_offset);
  elfcpp::Swap<32, false>::writeval(pov + 38, - (plt_offset + 38 + 4));
  return 32;
}

const unsigned char
Output_data_plt_i386_nacl::plt_eh_frame_fde[plt_eh_frame_fde_size] =
{
  0, 0, 0, 0,				// Replaced with offset to .plt.
  0, 0, 0, 0,				// Replaced with size of .plt.
  0,					// Augmentation size.
  elfcpp::DW_CFA_def_cfa_offset, 8,	// DW_CFA_def_cfa_offset: 8.
  elfcpp::DW_CFA_advance_loc + 6,	// Advance 6 to __PLT__ + 6.
  elfcpp::DW_CFA_def_cfa_offset, 12,	// DW_CFA_def_cfa_offset: 12.
  elfcpp::DW_CFA_advance_loc + 58,	// Advance 58 to __PLT__ + 64.
  elfcpp::DW_CFA_def_cfa_expression,	// DW_CFA_def_cfa_expression.
  13,					// Block length.
  elfcpp::DW_OP_breg4, 4,		// Push %esp + 4.
  elfcpp::DW_OP_breg8, 0,		// Push %eip.
  elfcpp::DW_OP_const1u, 63,            // Push 0x3f.
  elfcpp::DW_OP_and,			// & (%eip & 0x3f).
  elfcpp::DW_OP_const1u, 37,            // Push 0x25.
  elfcpp::DW_OP_ge,			// >= ((%eip & 0x3f) >= 0x25)
  elfcpp::DW_OP_lit2,			// Push 2.
  elfcpp::DW_OP_shl,			// << (((%eip & 0x3f) >= 0x25) << 2)
  elfcpp::DW_OP_plus,			// + ((((%eip&0x3f)>=0x25)<<2)+%esp+4
  elfcpp::DW_CFA_nop,			// Align to 32 bytes.
  elfcpp::DW_CFA_nop
};

// Return a string used to fill a code section with nops.
// For NaCl, long NOPs are only valid if they do not cross
// bundle alignment boundaries, so keep it simple with one-byte NOPs.
std::string
Target_i386_nacl::do_code_fill(section_size_type length) const
{
  return std::string(length, static_cast<char>(0x90));
}

// The selector for i386-nacl object files.

class Target_selector_i386_nacl
  : public Target_selector_nacl<Target_selector_i386, Target_i386_nacl>
{
 public:
  Target_selector_i386_nacl()
    : Target_selector_nacl<Target_selector_i386,
			   Target_i386_nacl>("x86-32",
					     "elf32-i386-nacl",
					     "elf_i386_nacl")
  { }
};

Target_selector_i386_nacl target_selector_i386;

// IAMCU variant.  It uses EM_IAMCU, not EM_386.

class Target_iamcu : public Target_i386
{
 public:
  Target_iamcu()
    : Target_i386(&iamcu_info)
  { }

 private:
  // Information about this specific target which we pass to the
  // general Target structure.
  static const Target::Target_info iamcu_info;
};

const Target::Target_info Target_iamcu::iamcu_info =
{
  32,			// size
  false,		// is_big_endian
  elfcpp::EM_IAMCU,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  true,			// has_code_fill
  true,			// is_default_stack_executable
  true,			// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/usr/lib/libc.so.1",	// dynamic_linker
  0x08048000,		// default_text_segment_address
  0x1000,		// abi_pagesize (overridable by -z max-page-size)
  0x1000,		// common_pagesize (overridable by -z common-page-size)
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

class Target_selector_iamcu : public Target_selector
{
public:
  Target_selector_iamcu()
    : Target_selector(elfcpp::EM_IAMCU, 32, false, "elf32-iamcu",
		      "elf_iamcu")
  { }

  Target*
  do_instantiate_target()
  { return new Target_iamcu(); }
};

Target_selector_iamcu target_selector_iamcu;

} // End anonymous namespace.
