// sparc.cc -- sparc target support for gold.

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by David S. Miller <davem@davemloft.net>.

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
#include <cstdio>
#include <cstring>

#include "elfcpp.h"
#include "parameters.h"
#include "reloc.h"
#include "sparc.h"
#include "object.h"
#include "symtab.h"
#include "layout.h"
#include "output.h"
#include "copy-relocs.h"
#include "target.h"
#include "target-reloc.h"
#include "target-select.h"
#include "tls.h"
#include "errors.h"
#include "gc.h"

namespace
{

using namespace gold;

template<int size, bool big_endian>
class Output_data_plt_sparc;

template<int size, bool big_endian>
class Target_sparc : public Sized_target<size, big_endian>
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_RELA, true, size, big_endian> Reloc_section;

  Target_sparc()
    : Sized_target<size, big_endian>(&sparc_info),
      got_(NULL), plt_(NULL), rela_dyn_(NULL), rela_ifunc_(NULL),
      copy_relocs_(elfcpp::R_SPARC_COPY),
      got_mod_index_offset_(-1U), tls_get_addr_sym_(NULL),
      elf_machine_(sparc_info.machine_code), elf_flags_(0),
      elf_flags_set_(false), register_syms_()
  {
  }

  // Make a new symbol table entry.
  Sized_symbol<size>*
  make_symbol(const char*, elfcpp::STT, Object*, unsigned int, uint64_t);

  // Process the relocations to determine unreferenced sections for
  // garbage collection.
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

  // Emit relocations for a section.
  void
  relocate_relocs(const Relocate_info<size, big_endian>*,
		  unsigned int sh_type,
		  const unsigned char* prelocs,
		  size_t reloc_count,
		  Output_section* output_section,
		  typename elfcpp::Elf_types<size>::Elf_Off
                    offset_in_output_section,
		  unsigned char* view,
		  typename elfcpp::Elf_types<size>::Elf_Addr view_address,
		  section_size_type view_size,
		  unsigned char* reloc_view,
		  section_size_type reloc_view_size);

  // Return whether SYM is defined by the ABI.
  bool
  do_is_defined_by_abi(const Symbol* sym) const
  { return strcmp(sym->name(), "___tls_get_addr") == 0; }

  // Return the PLT address to use for a global symbol.
  uint64_t
  do_plt_address_for_global(const Symbol* gsym) const
  { return this->plt_section()->address_for_global(gsym); }

  uint64_t
  do_plt_address_for_local(const Relobj* relobj, unsigned int symndx) const
  { return this->plt_section()->address_for_local(relobj, symndx); }

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

  // Return the address of the GOT.
  uint64_t
  got_address() const
  {
    if (this->got_ == NULL)
      return 0;
    return this->got_->address();
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
  // Make an ELF object.
  Object*
  do_make_elf_object(const std::string&, Input_file*, off_t,
		     const elfcpp::Ehdr<size, big_endian>& ehdr);

  void
  do_adjust_elf_header(unsigned char* view, int len);

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
    local(Symbol_table* symtab, Layout* layout, Target_sparc* target,
	  Sized_relobj_file<size, big_endian>* object,
	  unsigned int data_shndx,
	  Output_section* output_section,
	  const elfcpp::Rela<size, big_endian>& reloc, unsigned int r_type,
	  const elfcpp::Sym<size, big_endian>& lsym,
	  bool is_discarded);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_sparc* target,
	   Sized_relobj_file<size, big_endian>* object,
	   unsigned int data_shndx,
	   Output_section* output_section,
	   const elfcpp::Rela<size, big_endian>& reloc, unsigned int r_type,
	   Symbol* gsym);

    inline bool
    local_reloc_may_be_function_pointer(Symbol_table* , Layout* ,
					Target_sparc* ,
					Sized_relobj_file<size, big_endian>* ,
					unsigned int ,
					Output_section* ,
					const elfcpp::Rela<size, big_endian>& ,
					unsigned int ,
					const elfcpp::Sym<size, big_endian>&)
    { return false; }

    inline bool
    global_reloc_may_be_function_pointer(Symbol_table* , Layout* ,
					 Target_sparc* ,
					 Sized_relobj_file<size, big_endian>* ,
					 unsigned int ,
					 Output_section* ,
					 const elfcpp::Rela<size,
							    big_endian>& ,
					 unsigned int , Symbol*)
    { return false; }


  private:
    static void
    unsupported_reloc_local(Sized_relobj_file<size, big_endian>*,
			    unsigned int r_type);

    static void
    unsupported_reloc_global(Sized_relobj_file<size, big_endian>*,
			     unsigned int r_type, Symbol*);

    static void
    generate_tls_call(Symbol_table* symtab, Layout* layout,
		      Target_sparc* target);

    void
    check_non_pic(Relobj*, unsigned int r_type);

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
      : ignore_gd_add_(false), reloc_adjust_addr_(NULL)
    { }

    ~Relocate()
    {
      if (this->ignore_gd_add_)
	{
	  // FIXME: This needs to specify the location somehow.
	  gold_error(_("missing expected TLS relocation"));
	}
    }

    // Do a relocation.  Return false if the caller should not issue
    // any warnings about this relocation.
    inline bool
    relocate(const Relocate_info<size, big_endian>*, unsigned int,
	     Target_sparc*, Output_section*, size_t, const unsigned char*,
	     const Sized_symbol<size>*, const Symbol_value<size>*,
	     unsigned char*, typename elfcpp::Elf_types<size>::Elf_Addr,
	     section_size_type);

   private:
    // Do a TLS relocation.
    inline void
    relocate_tls(const Relocate_info<size, big_endian>*, Target_sparc* target,
		 size_t relnum, const elfcpp::Rela<size, big_endian>&,
		 unsigned int r_type, const Sized_symbol<size>*,
		 const Symbol_value<size>*,
		 unsigned char*,
		 typename elfcpp::Elf_types<size>::Elf_Addr,
		 section_size_type);

    inline void
    relax_call(Target_sparc<size, big_endian>* target,
	       unsigned char* view,
	       const elfcpp::Rela<size, big_endian>& rela,
	       section_size_type view_size);

    // Ignore the next relocation which should be R_SPARC_TLS_GD_ADD
    bool ignore_gd_add_;

    // If we hit a reloc at this view address, adjust it back by 4 bytes.
    unsigned char *reloc_adjust_addr_;
  };

  // Get the GOT section, creating it if necessary.
  Output_data_got<size, big_endian>*
  got_section(Symbol_table*, Layout*);

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

  // Return the gsym for "__tls_get_addr".  Cache if not already
  // cached.
  Symbol*
  tls_get_addr_sym(Symbol_table* symtab)
  {
    if (!this->tls_get_addr_sym_)
      this->tls_get_addr_sym_ = symtab->lookup("__tls_get_addr", NULL);
    gold_assert(this->tls_get_addr_sym_);
    return this->tls_get_addr_sym_;
  }

  // Get the PLT section.
  Output_data_plt_sparc<size, big_endian>*
  plt_section() const
  {
    gold_assert(this->plt_ != NULL);
    return this->plt_;
  }

  // Get the dynamic reloc section, creating it if necessary.
  Reloc_section*
  rela_dyn_section(Layout*);

  // Get the section to use for IFUNC relocations.
  Reloc_section*
  rela_ifunc_section(Layout*);

  // Copy a relocation against a global symbol.
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
  static Target::Target_info sparc_info;

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

  struct Register_symbol
  {
    Register_symbol()
      : name(NULL), shndx(0), obj(NULL)
    { }
    const char* name;
    unsigned int shndx;
    Object* obj;
  };

  // The GOT section.
  Output_data_got<size, big_endian>* got_;
  // The PLT section.
  Output_data_plt_sparc<size, big_endian>* plt_;
  // The dynamic reloc section.
  Reloc_section* rela_dyn_;
  // The section to use for IFUNC relocs.
  Reloc_section* rela_ifunc_;
  // Relocs saved to avoid a COPY reloc.
  Copy_relocs<elfcpp::SHT_RELA, size, big_endian> copy_relocs_;
  // Offset of the GOT entry for the TLS module index;
  unsigned int got_mod_index_offset_;
  // Cached pointer to __tls_get_addr symbol
  Symbol* tls_get_addr_sym_;
  // Accumulated elf machine type
  elfcpp::Elf_Half elf_machine_;
  // Accumulated elf header flags
  elfcpp::Elf_Word elf_flags_;
  // Whether elf_flags_ has been set for the first time yet
  bool elf_flags_set_;
  // STT_SPARC_REGISTER symbols (%g2, %g3, %g6, %g7).
  Register_symbol register_syms_[4];
};

template<>
Target::Target_info Target_sparc<32, true>::sparc_info =
{
  32,			// size
  true,			// is_big_endian
  elfcpp::EM_SPARC,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  false,		// has_code_fill
  true,			// is_default_stack_executable
  false,		// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/usr/lib/ld.so.1",	// dynamic_linker
  0x00010000,		// default_text_segment_address
  64 * 1024,		// abi_pagesize (overridable by -z max-page-size)
  8 * 1024,		// common_pagesize (overridable by -z common-page-size)
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
Target::Target_info Target_sparc<64, true>::sparc_info =
{
  64,			// size
  true,			// is_big_endian
  elfcpp::EM_SPARCV9,	// machine_code
  true,			// has_make_symbol
  false,		// has_resolve
  false,		// has_code_fill
  true,			// is_default_stack_executable
  false,		// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/usr/lib/sparcv9/ld.so.1",	// dynamic_linker
  0x100000,		// default_text_segment_address
  64 * 1024,		// abi_pagesize (overridable by -z max-page-size)
  8 * 1024,		// common_pagesize (overridable by -z common-page-size)
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

// We have to take care here, even when operating in little-endian
// mode, sparc instructions are still big endian.
template<int size, bool big_endian>
class Sparc_relocate_functions
{
private:
  // Do a simple relocation with the addend in the relocation.
  template<int valsize>
  static inline void
  rela(unsigned char* view,
       unsigned int right_shift,
       typename elfcpp::Elf_types<valsize>::Elf_Addr dst_mask,
       typename elfcpp::Swap<size, big_endian>::Valtype value,
       typename elfcpp::Swap<size, big_endian>::Valtype addend)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Valtype reloc = ((value + addend) >> right_shift);

    val &= ~dst_mask;
    reloc &= dst_mask;

    elfcpp::Swap<valsize, big_endian>::writeval(wv, val | reloc);
  }

  // Do a simple relocation using a symbol value with the addend in
  // the relocation.
  template<int valsize>
  static inline void
  rela(unsigned char* view,
       unsigned int right_shift,
       typename elfcpp::Elf_types<valsize>::Elf_Addr dst_mask,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Swap<valsize, big_endian>::Valtype addend)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Valtype reloc = (psymval->value(object, addend) >> right_shift);

    val &= ~dst_mask;
    reloc &= dst_mask;

    elfcpp::Swap<valsize, big_endian>::writeval(wv, val | reloc);
  }

  // Do a simple relocation using a symbol value with the addend in
  // the relocation, unaligned.
  template<int valsize>
  static inline void
  rela_ua(unsigned char* view,
	  unsigned int right_shift, elfcpp::Elf_Xword dst_mask,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  typename elfcpp::Swap<size, big_endian>::Valtype addend)
  {
    typedef typename elfcpp::Swap_unaligned<valsize,
	    big_endian>::Valtype Valtype;
    unsigned char* wv = view;
    Valtype val = elfcpp::Swap_unaligned<valsize, big_endian>::readval(wv);
    Valtype reloc = (psymval->value(object, addend) >> right_shift);

    val &= ~dst_mask;
    reloc &= dst_mask;

    elfcpp::Swap_unaligned<valsize, big_endian>::writeval(wv, val | reloc);
  }

  // Do a simple PC relative relocation with a Symbol_value with the
  // addend in the relocation.
  template<int valsize>
  static inline void
  pcrela(unsigned char* view,
	 unsigned int right_shift,
	 typename elfcpp::Elf_types<valsize>::Elf_Addr dst_mask,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 typename elfcpp::Swap<size, big_endian>::Valtype addend,
	 typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Valtype reloc = ((psymval->value(object, addend) - address)
		     >> right_shift);

    val &= ~dst_mask;
    reloc &= dst_mask;

    elfcpp::Swap<valsize, big_endian>::writeval(wv, val | reloc);
  }

  template<int valsize>
  static inline void
  pcrela_unaligned(unsigned char* view,
		   const Sized_relobj_file<size, big_endian>* object,
		   const Symbol_value<size>* psymval,
		   typename elfcpp::Swap<size, big_endian>::Valtype addend,
		   typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    typedef typename elfcpp::Swap_unaligned<valsize,
	    big_endian>::Valtype Valtype;
    unsigned char* wv = view;
    Valtype reloc = (psymval->value(object, addend) - address);

    elfcpp::Swap_unaligned<valsize, big_endian>::writeval(wv, reloc);
  }

  typedef Sparc_relocate_functions<size, big_endian> This;
  typedef Sparc_relocate_functions<size, true> This_insn;

public:
  // R_SPARC_WDISP30: (Symbol + Addend - Address) >> 2
  static inline void
  wdisp30(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   typename elfcpp::Elf_types<size>::Elf_Addr addend,
	   typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 2, 0x3fffffff, object,
				   psymval, addend, address);
  }

  // R_SPARC_WDISP22: (Symbol + Addend - Address) >> 2
  static inline void
  wdisp22(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   typename elfcpp::Elf_types<size>::Elf_Addr addend,
	   typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 2, 0x003fffff, object,
				   psymval, addend, address);
  }

  // R_SPARC_WDISP19: (Symbol + Addend - Address) >> 2
  static inline void
  wdisp19(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  typename elfcpp::Elf_types<size>::Elf_Addr addend,
	  typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 2, 0x0007ffff, object,
				   psymval, addend, address);
  }

  // R_SPARC_WDISP16: (Symbol + Addend - Address) >> 2
  static inline void
  wdisp16(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  typename elfcpp::Elf_types<size>::Elf_Addr addend,
	  typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    Valtype reloc = ((psymval->value(object, addend) - address)
		     >> 2);

    // The relocation value is split between the low 14 bits,
    // and bits 20-21.
    val &= ~((0x3 << 20) | 0x3fff);
    reloc = (((reloc & 0xc000) << (20 - 14))
	     | (reloc & 0x3ffff));

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }

  // R_SPARC_WDISP10: (Symbol + Addend - Address) >> 2
  static inline void
  wdisp10(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  typename elfcpp::Elf_types<size>::Elf_Addr addend,
	  typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    Valtype reloc = ((psymval->value(object, addend) - address)
		     >> 2);

    // The relocation value is split between the low bits 5-12,
    // and high bits 19-20.
    val &= ~((0x3 << 19) | (0xff << 5));
    reloc = (((reloc & 0x300) << (19 - 8))
	     | ((reloc & 0xff) << (5 - 0)));

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }

  // R_SPARC_PC22: (Symbol + Addend - Address) >> 10
  static inline void
  pc22(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend,
       typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 10, 0x003fffff, object,
				   psymval, addend, address);
  }

  // R_SPARC_PC10: (Symbol + Addend - Address) & 0x3ff
  static inline void
  pc10(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend,
       typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 0, 0x000003ff, object,
				   psymval, addend, address);
  }

  // R_SPARC_HI22: (Symbol + Addend) >> 10
  static inline void
  hi22(unsigned char* view,
       typename elfcpp::Elf_types<size>::Elf_Addr value,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 10, 0x003fffff, value, addend);
  }

  // R_SPARC_HI22: (Symbol + Addend) >> 10
  static inline void
  hi22(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 10, 0x003fffff, object, psymval, addend);
  }

  // R_SPARC_PCPLT22: (Symbol + Addend - Address) >> 10
  static inline void
  pcplt22(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  typename elfcpp::Elf_types<size>::Elf_Addr addend,
	  typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 10, 0x003fffff, object,
				   psymval, addend, address);
  }

  // R_SPARC_LO10: (Symbol + Addend) & 0x3ff
  static inline void
  lo10(unsigned char* view,
       typename elfcpp::Elf_types<size>::Elf_Addr value,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x000003ff, value, addend);
  }

  // R_SPARC_LO10: (Symbol + Addend) & 0x3ff
  static inline void
  lo10(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x000003ff, object, psymval, addend);
  }

  // R_SPARC_LO10: (Symbol + Addend) & 0x3ff
  static inline void
  lo10(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend,
       typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 0, 0x000003ff, object,
				   psymval, addend, address);
  }

  // R_SPARC_OLO10: ((Symbol + Addend) & 0x3ff) + Addend2
  static inline void
  olo10(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval,
	typename elfcpp::Elf_types<size>::Elf_Addr addend,
	typename elfcpp::Elf_types<size>::Elf_Addr addend2)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    Valtype reloc = psymval->value(object, addend);

    val &= ~0x1fff;
    reloc &= 0x3ff;
    reloc += addend2;
    reloc &= 0x1fff;

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }

  // R_SPARC_22: (Symbol + Addend)
  static inline void
  rela32_22(unsigned char* view,
	    const Sized_relobj_file<size, big_endian>* object,
	    const Symbol_value<size>* psymval,
	    typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x003fffff, object, psymval, addend);
  }

  // R_SPARC_13: (Symbol + Addend)
  static inline void
  rela32_13(unsigned char* view,
	    typename elfcpp::Elf_types<size>::Elf_Addr value,
	    typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x00001fff, value, addend);
  }

  // R_SPARC_13: (Symbol + Addend)
  static inline void
  rela32_13(unsigned char* view,
	    const Sized_relobj_file<size, big_endian>* object,
	    const Symbol_value<size>* psymval,
	    typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x00001fff, object, psymval, addend);
  }

  // R_SPARC_UA16: (Symbol + Addend)
  static inline void
  ua16(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This::template rela_ua<16>(view, 0, 0xffff, object, psymval, addend);
  }

  // R_SPARC_UA32: (Symbol + Addend)
  static inline void
  ua32(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This::template rela_ua<32>(view, 0, 0xffffffff, object, psymval, addend);
  }

  // R_SPARC_UA64: (Symbol + Addend)
  static inline void
  ua64(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This::template rela_ua<64>(view, 0, ~(elfcpp::Elf_Xword) 0,
			       object, psymval, addend);
  }

  // R_SPARC_DISP8: (Symbol + Addend - Address)
  static inline void
  disp8(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval,
	typename elfcpp::Elf_types<size>::Elf_Addr addend,
	typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This::template pcrela_unaligned<8>(view, object, psymval,
				       addend, address);
  }

  // R_SPARC_DISP16: (Symbol + Addend - Address)
  static inline void
  disp16(unsigned char* view,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 typename elfcpp::Elf_types<size>::Elf_Addr addend,
	 typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This::template pcrela_unaligned<16>(view, object, psymval,
					addend, address);
  }

  // R_SPARC_DISP32: (Symbol + Addend - Address)
  static inline void
  disp32(unsigned char* view,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 typename elfcpp::Elf_types<size>::Elf_Addr addend,
	 typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This::template pcrela_unaligned<32>(view, object, psymval,
					addend, address);
  }

  // R_SPARC_DISP64: (Symbol + Addend - Address)
  static inline void
  disp64(unsigned char* view,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 elfcpp::Elf_Xword addend,
	 typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This::template pcrela_unaligned<64>(view, object, psymval,
					addend, address);
  }

  // R_SPARC_H34: (Symbol + Addend) >> 12
  static inline void
  h34(unsigned char* view,
      const Sized_relobj_file<size, big_endian>* object,
      const Symbol_value<size>* psymval,
      typename elfcpp::Elf_types<size>::Elf_Addr  addend)
  {
    This_insn::template rela<32>(view, 12, 0x003fffff, object, psymval, addend);
  }

  // R_SPARC_H44: (Symbol + Addend) >> 22
  static inline void
  h44(unsigned char* view,
      const Sized_relobj_file<size, big_endian>* object,
      const Symbol_value<size>* psymval,
      typename elfcpp::Elf_types<size>::Elf_Addr  addend)
  {
    This_insn::template rela<32>(view, 22, 0x003fffff, object, psymval, addend);
  }

  // R_SPARC_M44: ((Symbol + Addend) >> 12) & 0x3ff
  static inline void
  m44(unsigned char* view,
      const Sized_relobj_file<size, big_endian>* object,
      const Symbol_value<size>* psymval,
      typename elfcpp::Elf_types<size>::Elf_Addr  addend)
  {
    This_insn::template rela<32>(view, 12, 0x000003ff, object, psymval, addend);
  }

  // R_SPARC_L44: (Symbol + Addend) & 0xfff
  static inline void
  l44(unsigned char* view,
      const Sized_relobj_file<size, big_endian>* object,
      const Symbol_value<size>* psymval,
      typename elfcpp::Elf_types<size>::Elf_Addr  addend)
  {
    This_insn::template rela<32>(view, 0, 0x00000fff, object, psymval, addend);
  }

  // R_SPARC_HH22: (Symbol + Addend) >> 42
  static inline void
  hh22(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 42, 0x003fffff, object, psymval, addend);
  }

  // R_SPARC_PC_HH22: (Symbol + Addend - Address) >> 42
  static inline void
  pc_hh22(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  typename elfcpp::Elf_types<size>::Elf_Addr addend,
	  typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 42, 0x003fffff, object,
				   psymval, addend, address);
  }

  // R_SPARC_HM10: ((Symbol + Addend) >> 32) & 0x3ff
  static inline void
  hm10(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 32, 0x000003ff, object, psymval, addend);
  }

  // R_SPARC_PC_HM10: ((Symbol + Addend - Address) >> 32) & 0x3ff
  static inline void
  pc_hm10(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  typename elfcpp::Elf_types<size>::Elf_Addr addend,
	  typename elfcpp::Elf_types<size>::Elf_Addr address)
  {
    This_insn::template pcrela<32>(view, 32, 0x000003ff, object,
				   psymval, addend, address);
  }

  // R_SPARC_11: (Symbol + Addend)
  static inline void
  rela32_11(unsigned char* view,
	    const Sized_relobj_file<size, big_endian>* object,
	    const Symbol_value<size>* psymval,
	    typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x000007ff, object, psymval, addend);
  }

  // R_SPARC_10: (Symbol + Addend)
  static inline void
  rela32_10(unsigned char* view,
	    const Sized_relobj_file<size, big_endian>* object,
	    const Symbol_value<size>* psymval,
	    typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x000003ff, object, psymval, addend);
  }

  // R_SPARC_7: (Symbol + Addend)
  static inline void
  rela32_7(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x0000007f, object, psymval, addend);
  }

  // R_SPARC_6: (Symbol + Addend)
  static inline void
  rela32_6(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x0000003f, object, psymval, addend);
  }

  // R_SPARC_5: (Symbol + Addend)
  static inline void
  rela32_5(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::template rela<32>(view, 0, 0x0000001f, object, psymval, addend);
  }

  // R_SPARC_TLS_LDO_HIX22: @dtpoff(Symbol + Addend) >> 10
  static inline void
  ldo_hix22(unsigned char* view,
	    typename elfcpp::Elf_types<size>::Elf_Addr value,
	    typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    This_insn::hi22(view, value, addend);
  }

  // R_SPARC_TLS_LDO_LOX10: @dtpoff(Symbol + Addend) & 0x3ff
  static inline void
  ldo_lox10(unsigned char* view,
	    typename elfcpp::Elf_types<size>::Elf_Addr value,
	    typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    Valtype reloc = (value + addend);

    val &= ~0x1fff;
    reloc &= 0x3ff;

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }

  // R_SPARC_TLS_LE_HIX22: (@tpoff(Symbol + Addend) ^ 0xffffffffffffffff) >> 10
  static inline void
  hix22(unsigned char* view,
	typename elfcpp::Elf_types<size>::Elf_Addr value,
	typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    Valtype reloc = (value + addend);

    val &= ~0x3fffff;

    reloc ^= ~(Valtype)0;
    reloc >>= 10;

    reloc &= 0x3fffff;

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }

  // R_SPARC_GOTDATA_OP_HIX22: @gdopoff(Symbol + Addend) >> 10
  static inline void
  gdop_hix22(unsigned char* view,
	     typename elfcpp::Elf_types<size>::Elf_Addr value)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    int32_t reloc = static_cast<int32_t>(value);

    val &= ~0x3fffff;

    if (reloc < 0)
      reloc ^= ~static_cast<int32_t>(0);
    reloc >>= 10;

    reloc &= 0x3fffff;

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }

  // R_SPARC_HIX22: ((Symbol + Addend) ^ 0xffffffffffffffff) >> 10
  static inline void
  hix22(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval,
	typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    Valtype reloc = psymval->value(object, addend);

    val &= ~0x3fffff;

    reloc ^= ~(Valtype)0;
    reloc >>= 10;

    reloc &= 0x3fffff;

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }


  // R_SPARC_TLS_LE_LOX10: (@tpoff(Symbol + Addend) & 0x3ff) | 0x1c00
  static inline void
  lox10(unsigned char* view,
	typename elfcpp::Elf_types<size>::Elf_Addr value,
	typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    Valtype reloc = (value + addend);

    val &= ~0x1fff;
    reloc &= 0x3ff;
    reloc |= 0x1c00;

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }

  // R_SPARC_GOTDATA_OP_LOX10: (@gdopoff(Symbol + Addend) & 0x3ff) | 0x1c00
  static inline void
  gdop_lox10(unsigned char* view,
	     typename elfcpp::Elf_types<size>::Elf_Addr value)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    int32_t reloc = static_cast<int32_t>(value);

    if (reloc < 0)
      reloc = (reloc & 0x3ff) | 0x1c00;
    else
      reloc = (reloc & 0x3ff);

    val &= ~0x1fff;
    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }

  // R_SPARC_LOX10: ((Symbol + Addend) & 0x3ff) | 0x1c00
  static inline void
  lox10(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval,
	typename elfcpp::Elf_types<size>::Elf_Addr addend)
  {
    typedef typename elfcpp::Swap<32, true>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, true>::readval(wv);
    Valtype reloc = psymval->value(object, addend);

    val &= ~0x1fff;
    reloc &= 0x3ff;
    reloc |= 0x1c00;

    elfcpp::Swap<32, true>::writeval(wv, val | reloc);
  }
};

// Get the GOT section, creating it if necessary.

template<int size, bool big_endian>
Output_data_got<size, big_endian>*
Target_sparc<size, big_endian>::got_section(Symbol_table* symtab,
					    Layout* layout)
{
  if (this->got_ == NULL)
    {
      gold_assert(symtab != NULL && layout != NULL);

      this->got_ = new Output_data_got<size, big_endian>();

      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_WRITE),
				      this->got_, ORDER_RELRO, true);

      // Define _GLOBAL_OFFSET_TABLE_ at the start of the .got section.
      symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
				    Symbol_table::PREDEFINED,
				    this->got_,
				    0, 0, elfcpp::STT_OBJECT,
				    elfcpp::STB_LOCAL,
				    elfcpp::STV_HIDDEN, 0,
				    false, false);
    }

  return this->got_;
}

// Get the dynamic reloc section, creating it if necessary.

template<int size, bool big_endian>
typename Target_sparc<size, big_endian>::Reloc_section*
Target_sparc<size, big_endian>::rela_dyn_section(Layout* layout)
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

// Get the section to use for IFUNC relocs, creating it if
// necessary.  These go in .rela.dyn, but only after all other dynamic
// relocations.  They need to follow the other dynamic relocations so
// that they can refer to global variables initialized by those
// relocs.

template<int size, bool big_endian>
typename Target_sparc<size, big_endian>::Reloc_section*
Target_sparc<size, big_endian>::rela_ifunc_section(Layout* layout)
{
  if (this->rela_ifunc_ == NULL)
    {
      // Make sure we have already created the dynamic reloc section.
      this->rela_dyn_section(layout);
      this->rela_ifunc_ = new Reloc_section(false);
      layout->add_output_section_data(".rela.dyn", elfcpp::SHT_RELA,
				      elfcpp::SHF_ALLOC, this->rela_ifunc_,
				      ORDER_DYNAMIC_RELOCS, false);
      gold_assert(this->rela_dyn_->output_section()
		  == this->rela_ifunc_->output_section());
    }
  return this->rela_ifunc_;
}

// A class to handle the PLT data.

template<int size, bool big_endian>
class Output_data_plt_sparc : public Output_section_data
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_RELA, true,
			    size, big_endian> Reloc_section;

  Output_data_plt_sparc(Layout*);

  // Add an entry to the PLT.
  void add_entry(Symbol_table* symtab, Layout* layout, Symbol* gsym);

  // Add an entry to the PLT for a local STT_GNU_IFUNC symbol.
  unsigned int
  add_local_ifunc_entry(Symbol_table*, Layout*,
			Sized_relobj_file<size, big_endian>* relobj,
			unsigned int local_sym_index);

  // Return the .rela.plt section data.
  const Reloc_section* rel_plt() const
  {
    return this->rel_;
  }

  // Return where the IFUNC relocations should go.
  Reloc_section*
  rela_ifunc(Symbol_table*, Layout*);

  void
  emit_pending_ifunc_relocs();

  // Return whether we created a section for IFUNC relocations.
  bool
  has_ifunc_section() const
  { return this->ifunc_rel_ != NULL; }

  // Return the number of PLT entries.
  unsigned int
  entry_count() const
  { return this->count_ + this->ifunc_count_; }

  // Return the offset of the first non-reserved PLT entry.
  static unsigned int
  first_plt_entry_offset()
  { return 4 * base_plt_entry_size; }

  // Return the size of a PLT entry.
  static unsigned int
  get_plt_entry_size()
  { return base_plt_entry_size; }

  // Return the PLT address to use for a global symbol.
  uint64_t
  address_for_global(const Symbol*);

  // Return the PLT address to use for a local symbol.
  uint64_t
  address_for_local(const Relobj*, unsigned int symndx);

 protected:
  void do_adjust_output_section(Output_section* os);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** PLT")); }

 private:
  // The size of an entry in the PLT.
  static const int base_plt_entry_size = (size == 32 ? 12 : 32);

  static const unsigned int plt_entries_per_block = 160;
  static const unsigned int plt_insn_chunk_size = 24;
  static const unsigned int plt_pointer_chunk_size = 8;
  static const unsigned int plt_block_size =
    (plt_entries_per_block
     * (plt_insn_chunk_size + plt_pointer_chunk_size));

  section_offset_type
  plt_index_to_offset(unsigned int index)
  {
    section_offset_type offset;

    if (size == 32 || index < 32768)
      offset = index * base_plt_entry_size;
    else
      {
	unsigned int ext_index = index - 32768;

	offset = (32768 * base_plt_entry_size)
	  + ((ext_index / plt_entries_per_block)
	     * plt_block_size)
	  + ((ext_index % plt_entries_per_block)
	     * plt_insn_chunk_size);
      }
    return offset;
  }

  // Set the final size.
  void
  set_final_data_size()
  {
    unsigned int full_count = this->entry_count() + 4;
    unsigned int extra = (size == 32 ? 4 : 0);
    section_offset_type sz = plt_index_to_offset(full_count) + extra;

    return this->set_data_size(sz);
  }

  // Write out the PLT data.
  void
  do_write(Output_file*);

  struct Global_ifunc
  {
    Reloc_section* rel;
    Symbol* gsym;
    unsigned int plt_index;
  };

  struct Local_ifunc
  {
    Reloc_section* rel;
    Sized_relobj_file<size, big_endian>* object;
    unsigned int local_sym_index;
    unsigned int plt_index;
  };

  // The reloc section.
  Reloc_section* rel_;
  // The IFUNC relocations, if necessary.  These must follow the
  // regular relocations.
  Reloc_section* ifunc_rel_;
  // The number of PLT entries.
  unsigned int count_;
  // The number of PLT entries for IFUNC symbols.
  unsigned int ifunc_count_;
  // Global STT_GNU_IFUNC symbols.
  std::vector<Global_ifunc> global_ifuncs_;
  // Local STT_GNU_IFUNC symbols.
  std::vector<Local_ifunc> local_ifuncs_;
};

// Define the constants as required by C++ standard.

template<int size, bool big_endian>
const int Output_data_plt_sparc<size, big_endian>::base_plt_entry_size;

template<int size, bool big_endian>
const unsigned int
Output_data_plt_sparc<size, big_endian>::plt_entries_per_block;

template<int size, bool big_endian>
const unsigned int Output_data_plt_sparc<size, big_endian>::plt_insn_chunk_size;

template<int size, bool big_endian>
const unsigned int
Output_data_plt_sparc<size, big_endian>::plt_pointer_chunk_size;

template<int size, bool big_endian>
const unsigned int Output_data_plt_sparc<size, big_endian>::plt_block_size;

// Create the PLT section.  The ordinary .got section is an argument,
// since we need to refer to the start.

template<int size, bool big_endian>
Output_data_plt_sparc<size, big_endian>::Output_data_plt_sparc(Layout* layout)
  : Output_section_data(size == 32 ? 4 : 8), ifunc_rel_(NULL),
    count_(0), ifunc_count_(0), global_ifuncs_(), local_ifuncs_()
{
  this->rel_ = new Reloc_section(false);
  layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
				  elfcpp::SHF_ALLOC, this->rel_,
				  ORDER_DYNAMIC_PLT_RELOCS, false);
}

template<int size, bool big_endian>
void
Output_data_plt_sparc<size, big_endian>::do_adjust_output_section(Output_section* os)
{
  os->set_entsize(0);
}

// Add an entry to the PLT.

template<int size, bool big_endian>
void
Output_data_plt_sparc<size, big_endian>::add_entry(Symbol_table* symtab,
						   Layout* layout,
						   Symbol* gsym)
{
  gold_assert(!gsym->has_plt_offset());

  section_offset_type plt_offset;
  unsigned int index;

  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      index = this->ifunc_count_;
      plt_offset = plt_index_to_offset(index);
      gsym->set_plt_offset(plt_offset);
      ++this->ifunc_count_;
      Reloc_section* rel = this->rela_ifunc(symtab, layout);

      struct Global_ifunc gi;
      gi.rel = rel;
      gi.gsym = gsym;
      gi.plt_index = index;
      this->global_ifuncs_.push_back(gi);
    }
  else
    {
      plt_offset = plt_index_to_offset(this->count_ + 4);
      gsym->set_plt_offset(plt_offset);
      ++this->count_;
      gsym->set_needs_dynsym_entry();
      this->rel_->add_global(gsym, elfcpp::R_SPARC_JMP_SLOT, this,
			     plt_offset, 0);
    }

  // Note that we don't need to save the symbol.  The contents of the
  // PLT are independent of which symbols are used.  The symbols only
  // appear in the relocations.
}

template<int size, bool big_endian>
unsigned int
Output_data_plt_sparc<size, big_endian>::add_local_ifunc_entry(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<size, big_endian>* relobj,
    unsigned int local_sym_index)
{
  unsigned int index = this->ifunc_count_;
  section_offset_type plt_offset;

  plt_offset = plt_index_to_offset(index);
  ++this->ifunc_count_;

  Reloc_section* rel = this->rela_ifunc(symtab, layout);

  struct Local_ifunc li;
  li.rel = rel;
  li.object = relobj;
  li.local_sym_index = local_sym_index;
  li.plt_index = index;
  this->local_ifuncs_.push_back(li);

  return plt_offset;
}

// Emit any pending IFUNC plt relocations.

template<int size, bool big_endian>
void
Output_data_plt_sparc<size, big_endian>::emit_pending_ifunc_relocs()
{
  // Emit any pending IFUNC relocs.
  for (typename std::vector<Global_ifunc>::const_iterator p =
	 this->global_ifuncs_.begin();
       p != this->global_ifuncs_.end();
       ++p)
    {
      section_offset_type plt_offset;
      unsigned int index;

      index = this->count_ + p->plt_index + 4;
      plt_offset = this->plt_index_to_offset(index);
      p->rel->add_symbolless_global_addend(p->gsym, elfcpp::R_SPARC_JMP_IREL,
					   this, plt_offset, 0);
    }

  for (typename std::vector<Local_ifunc>::const_iterator p =
	 this->local_ifuncs_.begin();
       p != this->local_ifuncs_.end();
       ++p)
    {
      section_offset_type plt_offset;
      unsigned int index;

      index = this->count_ + p->plt_index + 4;
      plt_offset = this->plt_index_to_offset(index);
      p->rel->add_symbolless_local_addend(p->object, p->local_sym_index,
					  elfcpp::R_SPARC_JMP_IREL,
					  this, plt_offset, 0);
    }
}

// Return where the IFUNC relocations should go in the PLT.  These
// follow the non-IFUNC relocations.

template<int size, bool big_endian>
typename Output_data_plt_sparc<size, big_endian>::Reloc_section*
Output_data_plt_sparc<size, big_endian>::rela_ifunc(
	Symbol_table* symtab,
	Layout* layout)
{
  if (this->ifunc_rel_ == NULL)
    {
      this->ifunc_rel_ = new Reloc_section(false);
      layout->add_output_section_data(".rela.plt", elfcpp::SHT_RELA,
				      elfcpp::SHF_ALLOC, this->ifunc_rel_,
				      ORDER_DYNAMIC_PLT_RELOCS, false);
      gold_assert(this->ifunc_rel_->output_section()
		  == this->rel_->output_section());

      if (parameters->doing_static_link())
	{
	  // A statically linked executable will only have a .rel.plt
	  // section to hold R_SPARC_IRELATIVE and R_SPARC_JMP_IREL
	  // relocs for STT_GNU_IFUNC symbols.  The library will use
	  // these symbols to locate the IRELATIVE and JMP_IREL relocs
	  // at program startup time.
	  symtab->define_in_output_data("__rela_iplt_start", NULL,
					Symbol_table::PREDEFINED,
					this->ifunc_rel_, 0, 0,
					elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
					elfcpp::STV_HIDDEN, 0, false, true);
	  symtab->define_in_output_data("__rela_iplt_end", NULL,
					Symbol_table::PREDEFINED,
					this->ifunc_rel_, 0, 0,
					elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
					elfcpp::STV_HIDDEN, 0, true, true);
	}
    }
  return this->ifunc_rel_;
}

// Return the PLT address to use for a global symbol.

template<int size, bool big_endian>
uint64_t
Output_data_plt_sparc<size, big_endian>::address_for_global(const Symbol* gsym)
{
  uint64_t offset = 0;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    offset = plt_index_to_offset(this->count_ + 4);
  return this->address() + offset + gsym->plt_offset();
}

// Return the PLT address to use for a local symbol.  These are always
// IRELATIVE relocs.

template<int size, bool big_endian>
uint64_t
Output_data_plt_sparc<size, big_endian>::address_for_local(
	const Relobj* object,
	unsigned int r_sym)
{
  return (this->address()
	  + plt_index_to_offset(this->count_ + 4)
	  + object->local_plt_offset(r_sym));
}

static const unsigned int sparc_nop = 0x01000000;
static const unsigned int sparc_sethi_g1 = 0x03000000;
static const unsigned int sparc_branch_always = 0x30800000;
static const unsigned int sparc_branch_always_pt = 0x30680000;
static const unsigned int sparc_mov = 0x80100000;
static const unsigned int sparc_mov_g0_o0 = 0x90100000;
static const unsigned int sparc_mov_o7_g5 = 0x8a10000f;
static const unsigned int sparc_call_plus_8 = 0x40000002;
static const unsigned int sparc_ldx_o7_imm_g1 = 0xc25be000;
static const unsigned int sparc_jmpl_o7_g1_g1 = 0x83c3c001;
static const unsigned int sparc_mov_g5_o7 = 0x9e100005;

// Write out the PLT.

template<int size, bool big_endian>
void
Output_data_plt_sparc<size, big_endian>::do_write(Output_file* of)
{
  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);
  unsigned char* pov = oview;

  memset(pov, 0, base_plt_entry_size * 4);
  pov += this->first_plt_entry_offset();

  unsigned int plt_offset = base_plt_entry_size * 4;
  const unsigned int count = this->entry_count();

  if (size == 64)
    {
      unsigned int limit;

      limit = (count > 32768 ? 32768 : count);

      for (unsigned int i = 0; i < limit; ++i)
	{
	  elfcpp::Swap<32, true>::writeval(pov + 0x00,
					   sparc_sethi_g1 + plt_offset);
	  elfcpp::Swap<32, true>::writeval(pov + 0x04,
					   sparc_branch_always_pt +
					   (((base_plt_entry_size -
					      (plt_offset + 4)) >> 2) &
					    0x7ffff));
	  elfcpp::Swap<32, true>::writeval(pov + 0x08, sparc_nop);
	  elfcpp::Swap<32, true>::writeval(pov + 0x0c, sparc_nop);
	  elfcpp::Swap<32, true>::writeval(pov + 0x10, sparc_nop);
	  elfcpp::Swap<32, true>::writeval(pov + 0x14, sparc_nop);
	  elfcpp::Swap<32, true>::writeval(pov + 0x18, sparc_nop);
	  elfcpp::Swap<32, true>::writeval(pov + 0x1c, sparc_nop);

	  pov += base_plt_entry_size;
	  plt_offset += base_plt_entry_size;
	}

      if (count > 32768)
	{
	  unsigned int ext_cnt = count - 32768;
	  unsigned int blks = ext_cnt / plt_entries_per_block;

	  for (unsigned int i = 0; i < blks; ++i)
	    {
	      unsigned int data_off = (plt_entries_per_block
				       * plt_insn_chunk_size) - 4;

	      for (unsigned int j = 0; j < plt_entries_per_block; ++j)
		{
		  elfcpp::Swap<32, true>::writeval(pov + 0x00,
						   sparc_mov_o7_g5);
		  elfcpp::Swap<32, true>::writeval(pov + 0x04,
						   sparc_call_plus_8);
		  elfcpp::Swap<32, true>::writeval(pov + 0x08,
						   sparc_nop);
		  elfcpp::Swap<32, true>::writeval(pov + 0x0c,
						   sparc_ldx_o7_imm_g1 +
						   (data_off & 0x1fff));
		  elfcpp::Swap<32, true>::writeval(pov + 0x10,
						   sparc_jmpl_o7_g1_g1);
		  elfcpp::Swap<32, true>::writeval(pov + 0x14,
						   sparc_mov_g5_o7);

		  elfcpp::Swap<64, big_endian>::writeval(
				pov + 0x4 + data_off,
				(elfcpp::Elf_Xword) (oview - (pov + 0x04)));

		  pov += plt_insn_chunk_size;
		  data_off -= 16;
		}
	    }

	  unsigned int sub_blk_cnt = ext_cnt % plt_entries_per_block;
	  for (unsigned int i = 0; i < sub_blk_cnt; ++i)
	    {
	      unsigned int data_off = (sub_blk_cnt
				       * plt_insn_chunk_size) - 4;

	      for (unsigned int j = 0; j < plt_entries_per_block; ++j)
		{
		  elfcpp::Swap<32, true>::writeval(pov + 0x00,
						   sparc_mov_o7_g5);
		  elfcpp::Swap<32, true>::writeval(pov + 0x04,
						   sparc_call_plus_8);
		  elfcpp::Swap<32, true>::writeval(pov + 0x08,
						   sparc_nop);
		  elfcpp::Swap<32, true>::writeval(pov + 0x0c,
						   sparc_ldx_o7_imm_g1 +
						   (data_off & 0x1fff));
		  elfcpp::Swap<32, true>::writeval(pov + 0x10,
						   sparc_jmpl_o7_g1_g1);
		  elfcpp::Swap<32, true>::writeval(pov + 0x14,
						   sparc_mov_g5_o7);

		  elfcpp::Swap<64, big_endian>::writeval(
				pov + 0x4 + data_off,
				(elfcpp::Elf_Xword) (oview - (pov + 0x04)));

		  pov += plt_insn_chunk_size;
		  data_off -= 16;
		}
	    }
	}
    }
  else
    {
      for (unsigned int i = 0; i < count; ++i)
	{
	  elfcpp::Swap<32, true>::writeval(pov + 0x00,
					   sparc_sethi_g1 + plt_offset);
	  elfcpp::Swap<32, true>::writeval(pov + 0x04,
					   sparc_branch_always +
					   (((- (plt_offset + 4)) >> 2) &
					    0x003fffff));
	  elfcpp::Swap<32, true>::writeval(pov + 0x08, sparc_nop);

	  pov += base_plt_entry_size;
	  plt_offset += base_plt_entry_size;
	}

      elfcpp::Swap<32, true>::writeval(pov, sparc_nop);
      pov += 4;
    }

  gold_assert(static_cast<section_size_type>(pov - oview) == oview_size);

  of->write_output_view(offset, oview_size, oview);
}

// Create the PLT section.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::make_plt_section(Symbol_table* symtab,
						 Layout* layout)
{
  // Create the GOT sections first.
  this->got_section(symtab, layout);

  // Ensure that .rela.dyn always appears before .rela.plt  This is
  // necessary due to how, on Sparc and some other targets, .rela.dyn
  // needs to include .rela.plt in it's range.
  this->rela_dyn_section(layout);

  this->plt_ = new Output_data_plt_sparc<size, big_endian>(layout);
  layout->add_output_section_data(".plt", elfcpp::SHT_PROGBITS,
				  (elfcpp::SHF_ALLOC
				   | elfcpp::SHF_EXECINSTR
				   | elfcpp::SHF_WRITE),
				  this->plt_, ORDER_NON_RELRO_FIRST, false);

  // Define _PROCEDURE_LINKAGE_TABLE_ at the start of the .plt section.
  symtab->define_in_output_data("_PROCEDURE_LINKAGE_TABLE_", NULL,
				Symbol_table::PREDEFINED,
				this->plt_,
				0, 0, elfcpp::STT_OBJECT,
				elfcpp::STB_LOCAL,
				elfcpp::STV_HIDDEN, 0,
				false, false);
}

// Create a PLT entry for a global symbol.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::make_plt_entry(Symbol_table* symtab,
					       Layout* layout,
					       Symbol* gsym)
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
Target_sparc<size, big_endian>::make_local_ifunc_plt_entry(
	Symbol_table* symtab,
	Layout* layout,
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
Target_sparc<size, big_endian>::plt_entry_count() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->entry_count();
}

// Return the offset of the first non-reserved PLT entry.

template<int size, bool big_endian>
unsigned int
Target_sparc<size, big_endian>::first_plt_entry_offset() const
{
  return Output_data_plt_sparc<size, big_endian>::first_plt_entry_offset();
}

// Return the size of each PLT entry.

template<int size, bool big_endian>
unsigned int
Target_sparc<size, big_endian>::plt_entry_size() const
{
  return Output_data_plt_sparc<size, big_endian>::get_plt_entry_size();
}

// Create a GOT entry for the TLS module index.

template<int size, bool big_endian>
unsigned int
Target_sparc<size, big_endian>::got_mod_index_entry(
     Symbol_table* symtab,
     Layout* layout,
     Sized_relobj_file<size, big_endian>* object)
{
  if (this->got_mod_index_offset_ == -1U)
    {
      gold_assert(symtab != NULL && layout != NULL && object != NULL);
      Reloc_section* rela_dyn = this->rela_dyn_section(layout);
      Output_data_got<size, big_endian>* got;
      unsigned int got_offset;

      got = this->got_section(symtab, layout);
      got_offset = got->add_constant(0);
      rela_dyn->add_local(object, 0,
			  (size == 64 ?
			   elfcpp::R_SPARC_TLS_DTPMOD64 :
			   elfcpp::R_SPARC_TLS_DTPMOD32), got,
			  got_offset, 0);
      got->add_constant(0);
      this->got_mod_index_offset_ = got_offset;
    }
  return this->got_mod_index_offset_;
}

// Optimize the TLS relocation type based on what we know about the
// symbol.  IS_FINAL is true if the final address of this symbol is
// known at link time.

static tls::Tls_optimization
optimize_tls_reloc(bool is_final, int r_type)
{
  // If we are generating a shared library, then we can't do anything
  // in the linker.
  if (parameters->options().shared())
    return tls::TLSOPT_NONE;

  switch (r_type)
    {
    case elfcpp::R_SPARC_TLS_GD_HI22: // Global-dynamic
    case elfcpp::R_SPARC_TLS_GD_LO10:
    case elfcpp::R_SPARC_TLS_GD_ADD:
    case elfcpp::R_SPARC_TLS_GD_CALL:
      // These are General-Dynamic which permits fully general TLS
      // access.  Since we know that we are generating an executable,
      // we can convert this to Initial-Exec.  If we also know that
      // this is a local symbol, we can further switch to Local-Exec.
      if (is_final)
	return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_TO_IE;

    case elfcpp::R_SPARC_TLS_LDM_HI22:	// Local-dynamic
    case elfcpp::R_SPARC_TLS_LDM_LO10:
    case elfcpp::R_SPARC_TLS_LDM_ADD:
    case elfcpp::R_SPARC_TLS_LDM_CALL:
      // This is Local-Dynamic, which refers to a local symbol in the
      // dynamic TLS block.  Since we know that we generating an
      // executable, we can switch to Local-Exec.
      return tls::TLSOPT_TO_LE;

    case elfcpp::R_SPARC_TLS_LDO_HIX22:	// Alternate local-dynamic
    case elfcpp::R_SPARC_TLS_LDO_LOX10:
    case elfcpp::R_SPARC_TLS_LDO_ADD:
      // Another type of Local-Dynamic relocation.
      return tls::TLSOPT_TO_LE;

    case elfcpp::R_SPARC_TLS_IE_HI22:	// Initial-exec
    case elfcpp::R_SPARC_TLS_IE_LO10:
    case elfcpp::R_SPARC_TLS_IE_LD:
    case elfcpp::R_SPARC_TLS_IE_LDX:
    case elfcpp::R_SPARC_TLS_IE_ADD:
      // These are Initial-Exec relocs which get the thread offset
      // from the GOT.  If we know that we are linking against the
      // local symbol, we can switch to Local-Exec, which links the
      // thread offset into the instruction.
      if (is_final)
	return tls::TLSOPT_TO_LE;
      return tls::TLSOPT_NONE;

    case elfcpp::R_SPARC_TLS_LE_HIX22:	// Local-exec
    case elfcpp::R_SPARC_TLS_LE_LOX10:
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
Target_sparc<size, big_endian>::Scan::get_reference_flags(unsigned int r_type)
{
  r_type &= 0xff;
  switch (r_type)
    {
    case elfcpp::R_SPARC_NONE:
    case elfcpp::R_SPARC_REGISTER:
    case elfcpp::R_SPARC_GNU_VTINHERIT:
    case elfcpp::R_SPARC_GNU_VTENTRY:
      // No symbol reference.
      return 0;

    case elfcpp::R_SPARC_UA64:
    case elfcpp::R_SPARC_64:
    case elfcpp::R_SPARC_HIX22:
    case elfcpp::R_SPARC_LOX10:
    case elfcpp::R_SPARC_H34:
    case elfcpp::R_SPARC_H44:
    case elfcpp::R_SPARC_M44:
    case elfcpp::R_SPARC_L44:
    case elfcpp::R_SPARC_HH22:
    case elfcpp::R_SPARC_HM10:
    case elfcpp::R_SPARC_LM22:
    case elfcpp::R_SPARC_HI22:
    case elfcpp::R_SPARC_LO10:
    case elfcpp::R_SPARC_OLO10:
    case elfcpp::R_SPARC_UA32:
    case elfcpp::R_SPARC_32:
    case elfcpp::R_SPARC_UA16:
    case elfcpp::R_SPARC_16:
    case elfcpp::R_SPARC_11:
    case elfcpp::R_SPARC_10:
    case elfcpp::R_SPARC_8:
    case elfcpp::R_SPARC_7:
    case elfcpp::R_SPARC_6:
    case elfcpp::R_SPARC_5:
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_SPARC_DISP8:
    case elfcpp::R_SPARC_DISP16:
    case elfcpp::R_SPARC_DISP32:
    case elfcpp::R_SPARC_DISP64:
    case elfcpp::R_SPARC_PC_HH22:
    case elfcpp::R_SPARC_PC_HM10:
    case elfcpp::R_SPARC_PC_LM22:
    case elfcpp::R_SPARC_PC10:
    case elfcpp::R_SPARC_PC22:
    case elfcpp::R_SPARC_WDISP30:
    case elfcpp::R_SPARC_WDISP22:
    case elfcpp::R_SPARC_WDISP19:
    case elfcpp::R_SPARC_WDISP16:
    case elfcpp::R_SPARC_WDISP10:
      return Symbol::RELATIVE_REF;

    case elfcpp::R_SPARC_PLT64:
    case elfcpp::R_SPARC_PLT32:
    case elfcpp::R_SPARC_HIPLT22:
    case elfcpp::R_SPARC_LOPLT10:
    case elfcpp::R_SPARC_PCPLT10:
      return Symbol::FUNCTION_CALL | Symbol::ABSOLUTE_REF;

    case elfcpp::R_SPARC_PCPLT32:
    case elfcpp::R_SPARC_PCPLT22:
    case elfcpp::R_SPARC_WPLT30:
      return Symbol::FUNCTION_CALL | Symbol::RELATIVE_REF;

    case elfcpp::R_SPARC_GOTDATA_OP:
    case elfcpp::R_SPARC_GOTDATA_OP_HIX22:
    case elfcpp::R_SPARC_GOTDATA_OP_LOX10:
    case elfcpp::R_SPARC_GOT10:
    case elfcpp::R_SPARC_GOT13:
    case elfcpp::R_SPARC_GOT22:
      // Absolute in GOT.
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_SPARC_TLS_GD_HI22: // Global-dynamic
    case elfcpp::R_SPARC_TLS_GD_LO10:
    case elfcpp::R_SPARC_TLS_GD_ADD:
    case elfcpp::R_SPARC_TLS_GD_CALL:
    case elfcpp::R_SPARC_TLS_LDM_HI22:	// Local-dynamic
    case elfcpp::R_SPARC_TLS_LDM_LO10:
    case elfcpp::R_SPARC_TLS_LDM_ADD:
    case elfcpp::R_SPARC_TLS_LDM_CALL:
    case elfcpp::R_SPARC_TLS_LDO_HIX22:	// Alternate local-dynamic
    case elfcpp::R_SPARC_TLS_LDO_LOX10:
    case elfcpp::R_SPARC_TLS_LDO_ADD:
    case elfcpp::R_SPARC_TLS_LE_HIX22:
    case elfcpp::R_SPARC_TLS_LE_LOX10:
    case elfcpp::R_SPARC_TLS_IE_HI22:	// Initial-exec
    case elfcpp::R_SPARC_TLS_IE_LO10:
    case elfcpp::R_SPARC_TLS_IE_LD:
    case elfcpp::R_SPARC_TLS_IE_LDX:
    case elfcpp::R_SPARC_TLS_IE_ADD:
      return Symbol::TLS_REF;

    case elfcpp::R_SPARC_COPY:
    case elfcpp::R_SPARC_GLOB_DAT:
    case elfcpp::R_SPARC_JMP_SLOT:
    case elfcpp::R_SPARC_JMP_IREL:
    case elfcpp::R_SPARC_RELATIVE:
    case elfcpp::R_SPARC_IRELATIVE:
    case elfcpp::R_SPARC_TLS_DTPMOD64:
    case elfcpp::R_SPARC_TLS_DTPMOD32:
    case elfcpp::R_SPARC_TLS_DTPOFF64:
    case elfcpp::R_SPARC_TLS_DTPOFF32:
    case elfcpp::R_SPARC_TLS_TPOFF64:
    case elfcpp::R_SPARC_TLS_TPOFF32:
    default:
      // Not expected.  We will give an error later.
      return 0;
    }
}

// Generate a PLT entry slot for a call to __tls_get_addr
template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::Scan::generate_tls_call(Symbol_table* symtab,
							Layout* layout,
							Target_sparc<size, big_endian>* target)
{
  Symbol* gsym = target->tls_get_addr_sym(symtab);

  target->make_plt_entry(symtab, layout, gsym);
}

// Report an unsupported relocation against a local symbol.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::Scan::unsupported_reloc_local(
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
Target_sparc<size, big_endian>::Scan::check_non_pic(Relobj* object, unsigned int r_type)
{
  gold_assert(r_type != elfcpp::R_SPARC_NONE);

  if (size == 64)
    {
      switch (r_type)
	{
	  // These are the relocation types supported by glibc for sparc 64-bit.
	case elfcpp::R_SPARC_RELATIVE:
	case elfcpp::R_SPARC_IRELATIVE:
	case elfcpp::R_SPARC_COPY:
	case elfcpp::R_SPARC_32:
	case elfcpp::R_SPARC_64:
	case elfcpp::R_SPARC_GLOB_DAT:
	case elfcpp::R_SPARC_JMP_SLOT:
	case elfcpp::R_SPARC_JMP_IREL:
	case elfcpp::R_SPARC_TLS_DTPMOD64:
	case elfcpp::R_SPARC_TLS_DTPOFF64:
	case elfcpp::R_SPARC_TLS_TPOFF64:
	case elfcpp::R_SPARC_TLS_LE_HIX22:
	case elfcpp::R_SPARC_TLS_LE_LOX10:
	case elfcpp::R_SPARC_8:
	case elfcpp::R_SPARC_16:
	case elfcpp::R_SPARC_DISP8:
	case elfcpp::R_SPARC_DISP16:
	case elfcpp::R_SPARC_DISP32:
	case elfcpp::R_SPARC_WDISP30:
	case elfcpp::R_SPARC_LO10:
	case elfcpp::R_SPARC_HI22:
	case elfcpp::R_SPARC_OLO10:
	case elfcpp::R_SPARC_H34:
	case elfcpp::R_SPARC_H44:
	case elfcpp::R_SPARC_M44:
	case elfcpp::R_SPARC_L44:
	case elfcpp::R_SPARC_HH22:
	case elfcpp::R_SPARC_HM10:
	case elfcpp::R_SPARC_LM22:
	case elfcpp::R_SPARC_UA16:
	case elfcpp::R_SPARC_UA32:
	case elfcpp::R_SPARC_UA64:
	  return;

	default:
	  break;
	}
    }
  else
    {
      switch (r_type)
	{
	  // These are the relocation types supported by glibc for sparc 32-bit.
	case elfcpp::R_SPARC_RELATIVE:
	case elfcpp::R_SPARC_IRELATIVE:
	case elfcpp::R_SPARC_COPY:
	case elfcpp::R_SPARC_GLOB_DAT:
	case elfcpp::R_SPARC_32:
	case elfcpp::R_SPARC_JMP_SLOT:
	case elfcpp::R_SPARC_JMP_IREL:
	case elfcpp::R_SPARC_TLS_DTPMOD32:
	case elfcpp::R_SPARC_TLS_DTPOFF32:
	case elfcpp::R_SPARC_TLS_TPOFF32:
	case elfcpp::R_SPARC_TLS_LE_HIX22:
	case elfcpp::R_SPARC_TLS_LE_LOX10:
	case elfcpp::R_SPARC_8:
	case elfcpp::R_SPARC_16:
	case elfcpp::R_SPARC_DISP8:
	case elfcpp::R_SPARC_DISP16:
	case elfcpp::R_SPARC_DISP32:
	case elfcpp::R_SPARC_LO10:
	case elfcpp::R_SPARC_WDISP30:
	case elfcpp::R_SPARC_HI22:
	case elfcpp::R_SPARC_UA16:
	case elfcpp::R_SPARC_UA32:
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

template<int size, bool big_endian>
bool
Target_sparc<size, big_endian>::Scan::reloc_needs_plt_for_ifunc(
     Sized_relobj_file<size, big_endian>* object,
     unsigned int r_type)
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
Target_sparc<size, big_endian>::Scan::local(
			Symbol_table* symtab,
			Layout* layout,
			Target_sparc<size, big_endian>* target,
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

  bool is_ifunc = lsym.get_st_type() == elfcpp::STT_GNU_IFUNC;
  unsigned int orig_r_type = r_type;
  r_type &= 0xff;

  if (is_ifunc
      && this->reloc_needs_plt_for_ifunc(object, r_type))
    {
      unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
      target->make_local_ifunc_plt_entry(symtab, layout, object, r_sym);
    }

  switch (r_type)
    {
    case elfcpp::R_SPARC_NONE:
    case elfcpp::R_SPARC_REGISTER:
    case elfcpp::R_SPARC_GNU_VTINHERIT:
    case elfcpp::R_SPARC_GNU_VTENTRY:
      break;

    case elfcpp::R_SPARC_64:
    case elfcpp::R_SPARC_32:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for
      // this location. The relocation applied at link time will
      // apply the link-time value, so we flag the location with
      // an R_SPARC_RELATIVE relocation so the dynamic loader can
      // relocate it easily.
      if (parameters->options().output_is_position_independent()
	  && ((size == 64 && r_type == elfcpp::R_SPARC_64)
	      || (size == 32 && r_type == elfcpp::R_SPARC_32)))
	{
	  Reloc_section* rela_dyn = target->rela_dyn_section(layout);
	  unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
	  rela_dyn->add_local_relative(object, r_sym, elfcpp::R_SPARC_RELATIVE,
				       output_section, data_shndx,
				       reloc.get_r_offset(),
				       reloc.get_r_addend(), is_ifunc);
	  break;
	}
      // Fall through.

    case elfcpp::R_SPARC_HIX22:
    case elfcpp::R_SPARC_LOX10:
    case elfcpp::R_SPARC_H34:
    case elfcpp::R_SPARC_H44:
    case elfcpp::R_SPARC_M44:
    case elfcpp::R_SPARC_L44:
    case elfcpp::R_SPARC_HH22:
    case elfcpp::R_SPARC_HM10:
    case elfcpp::R_SPARC_LM22:
    case elfcpp::R_SPARC_UA64:
    case elfcpp::R_SPARC_UA32:
    case elfcpp::R_SPARC_UA16:
    case elfcpp::R_SPARC_HI22:
    case elfcpp::R_SPARC_LO10:
    case elfcpp::R_SPARC_OLO10:
    case elfcpp::R_SPARC_16:
    case elfcpp::R_SPARC_11:
    case elfcpp::R_SPARC_10:
    case elfcpp::R_SPARC_8:
    case elfcpp::R_SPARC_7:
    case elfcpp::R_SPARC_6:
    case elfcpp::R_SPARC_5:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for
      // this location.
      if (parameters->options().output_is_position_independent())
	{
	  Reloc_section* rela_dyn = target->rela_dyn_section(layout);
	  unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());

	  check_non_pic(object, r_type);
	  if (lsym.get_st_type() != elfcpp::STT_SECTION)
	    {
	      rela_dyn->add_local(object, r_sym, orig_r_type, output_section,
				  data_shndx, reloc.get_r_offset(),
				  reloc.get_r_addend());
	    }
	  else
	    {
	      gold_assert(lsym.get_st_value() == 0);
	      rela_dyn->add_symbolless_local_addend(object, r_sym, orig_r_type,
						    output_section, data_shndx,
						    reloc.get_r_offset(),
						    reloc.get_r_addend());
	    }
	}
      break;

    case elfcpp::R_SPARC_WDISP30:
    case elfcpp::R_SPARC_WPLT30:
    case elfcpp::R_SPARC_WDISP22:
    case elfcpp::R_SPARC_WDISP19:
    case elfcpp::R_SPARC_WDISP16:
    case elfcpp::R_SPARC_WDISP10:
    case elfcpp::R_SPARC_DISP8:
    case elfcpp::R_SPARC_DISP16:
    case elfcpp::R_SPARC_DISP32:
    case elfcpp::R_SPARC_DISP64:
    case elfcpp::R_SPARC_PC10:
    case elfcpp::R_SPARC_PC22:
      break;

    case elfcpp::R_SPARC_GOTDATA_OP:
    case elfcpp::R_SPARC_GOTDATA_OP_HIX22:
    case elfcpp::R_SPARC_GOTDATA_OP_LOX10:
      // We will optimize this into a GOT relative relocation
      // and code transform the GOT load into an addition.
      break;

    case elfcpp::R_SPARC_GOT10:
    case elfcpp::R_SPARC_GOT13:
    case elfcpp::R_SPARC_GOT22:
      {
	// The symbol requires a GOT entry.
	Output_data_got<size, big_endian>* got;
	unsigned int r_sym;

	got = target->got_section(symtab, layout);
	r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());

	// If we are generating a shared object, we need to add a
	// dynamic relocation for this symbol's GOT entry.
	if (parameters->options().output_is_position_independent())
	  {
	    if (!object->local_has_got_offset(r_sym, GOT_TYPE_STANDARD))
	      {
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		unsigned int off = got->add_constant(0);
		object->set_local_got_offset(r_sym, GOT_TYPE_STANDARD, off);
		rela_dyn->add_local_relative(object, r_sym,
					     elfcpp::R_SPARC_RELATIVE,
					     got, off, 0, is_ifunc);
	      }
	  }
	else
	  got->add_local(object, r_sym, GOT_TYPE_STANDARD);
      }
      break;

      // These are initial TLS relocs, which are expected when
      // linking.
    case elfcpp::R_SPARC_TLS_GD_HI22: // Global-dynamic
    case elfcpp::R_SPARC_TLS_GD_LO10:
    case elfcpp::R_SPARC_TLS_GD_ADD:
    case elfcpp::R_SPARC_TLS_GD_CALL:
    case elfcpp::R_SPARC_TLS_LDM_HI22 :	// Local-dynamic
    case elfcpp::R_SPARC_TLS_LDM_LO10:
    case elfcpp::R_SPARC_TLS_LDM_ADD:
    case elfcpp::R_SPARC_TLS_LDM_CALL:
    case elfcpp::R_SPARC_TLS_LDO_HIX22:	// Alternate local-dynamic
    case elfcpp::R_SPARC_TLS_LDO_LOX10:
    case elfcpp::R_SPARC_TLS_LDO_ADD:
    case elfcpp::R_SPARC_TLS_IE_HI22:	// Initial-exec
    case elfcpp::R_SPARC_TLS_IE_LO10:
    case elfcpp::R_SPARC_TLS_IE_LD:
    case elfcpp::R_SPARC_TLS_IE_LDX:
    case elfcpp::R_SPARC_TLS_IE_ADD:
    case elfcpp::R_SPARC_TLS_LE_HIX22:	// Local-exec
    case elfcpp::R_SPARC_TLS_LE_LOX10:
      {
	bool output_is_shared = parameters->options().shared();
	const tls::Tls_optimization optimized_type
	    = optimize_tls_reloc(!output_is_shared, r_type);
	switch (r_type)
	  {
	  case elfcpp::R_SPARC_TLS_GD_HI22: // Global-dynamic
	  case elfcpp::R_SPARC_TLS_GD_LO10:
	  case elfcpp::R_SPARC_TLS_GD_ADD:
	  case elfcpp::R_SPARC_TLS_GD_CALL:
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Output_data_got<size, big_endian>* got
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
					       lsym.get_st_shndx(),
					       GOT_TYPE_TLS_PAIR,
					       target->rela_dyn_section(layout),
					       (size == 64
						? elfcpp::R_SPARC_TLS_DTPMOD64
						: elfcpp::R_SPARC_TLS_DTPMOD32));
		if (r_type == elfcpp::R_SPARC_TLS_GD_CALL)
		  generate_tls_call(symtab, layout, target);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_SPARC_TLS_LDM_HI22 :	// Local-dynamic
	  case elfcpp::R_SPARC_TLS_LDM_LO10:
	  case elfcpp::R_SPARC_TLS_LDM_ADD:
	  case elfcpp::R_SPARC_TLS_LDM_CALL:
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);

		if (r_type == elfcpp::R_SPARC_TLS_LDM_CALL)
		  generate_tls_call(symtab, layout, target);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_SPARC_TLS_LDO_HIX22:	// Alternate local-dynamic
	  case elfcpp::R_SPARC_TLS_LDO_LOX10:
	  case elfcpp::R_SPARC_TLS_LDO_ADD:
	    break;

	  case elfcpp::R_SPARC_TLS_IE_HI22:	// Initial-exec
	  case elfcpp::R_SPARC_TLS_IE_LO10:
	  case elfcpp::R_SPARC_TLS_IE_LD:
	  case elfcpp::R_SPARC_TLS_IE_LDX:
	  case elfcpp::R_SPARC_TLS_IE_ADD:
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<size, big_endian>* got
		  = target->got_section(symtab, layout);
		unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());

		if (!object->local_has_got_offset(r_sym, GOT_TYPE_TLS_OFFSET))
		  {
		    Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		    unsigned int off = got->add_constant(0);

		    object->set_local_got_offset(r_sym, GOT_TYPE_TLS_OFFSET, off);

		    rela_dyn->add_symbolless_local_addend(object, r_sym,
							  (size == 64 ?
							   elfcpp::R_SPARC_TLS_TPOFF64 :
							   elfcpp::R_SPARC_TLS_TPOFF32),
							  got, off, 0);
		  }
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_local(object, r_type);
	    break;

	  case elfcpp::R_SPARC_TLS_LE_HIX22:	// Local-exec
	  case elfcpp::R_SPARC_TLS_LE_LOX10:
	    layout->set_has_static_tls();
	    if (output_is_shared)
	      {
		// We need to create a dynamic relocation.
		gold_assert(lsym.get_st_type() != elfcpp::STT_SECTION);
		unsigned int r_sym = elfcpp::elf_r_sym<size>(reloc.get_r_info());
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_symbolless_local_addend(object, r_sym, r_type,
						      output_section, data_shndx,
						      reloc.get_r_offset(), 0);
	      }
	    break;
	  }
      }
      break;

      // These are relocations which should only be seen by the
      // dynamic linker, and should never be seen here.
    case elfcpp::R_SPARC_COPY:
    case elfcpp::R_SPARC_GLOB_DAT:
    case elfcpp::R_SPARC_JMP_SLOT:
    case elfcpp::R_SPARC_JMP_IREL:
    case elfcpp::R_SPARC_RELATIVE:
    case elfcpp::R_SPARC_IRELATIVE:
    case elfcpp::R_SPARC_TLS_DTPMOD64:
    case elfcpp::R_SPARC_TLS_DTPMOD32:
    case elfcpp::R_SPARC_TLS_DTPOFF64:
    case elfcpp::R_SPARC_TLS_DTPOFF32:
    case elfcpp::R_SPARC_TLS_TPOFF64:
    case elfcpp::R_SPARC_TLS_TPOFF32:
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

    default:
      unsupported_reloc_local(object, r_type);
      break;
    }
}

// Report an unsupported relocation against a global symbol.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::Scan::unsupported_reloc_global(
			Sized_relobj_file<size, big_endian>* object,
			unsigned int r_type,
			Symbol* gsym)
{
  gold_error(_("%s: unsupported reloc %u against global symbol %s"),
	     object->name().c_str(), r_type, gsym->demangled_name().c_str());
}

// Scan a relocation for a global symbol.

template<int size, bool big_endian>
inline void
Target_sparc<size, big_endian>::Scan::global(
				Symbol_table* symtab,
				Layout* layout,
				Target_sparc<size, big_endian>* target,
				Sized_relobj_file<size, big_endian>* object,
				unsigned int data_shndx,
				Output_section* output_section,
				const elfcpp::Rela<size, big_endian>& reloc,
				unsigned int r_type,
				Symbol* gsym)
{
  unsigned int orig_r_type = r_type;
  bool is_ifunc = gsym->type() == elfcpp::STT_GNU_IFUNC;

  // A reference to _GLOBAL_OFFSET_TABLE_ implies that we need a got
  // section.  We check here to avoid creating a dynamic reloc against
  // _GLOBAL_OFFSET_TABLE_.
  if (!target->has_got_section()
      && strcmp(gsym->name(), "_GLOBAL_OFFSET_TABLE_") == 0)
    target->got_section(symtab, layout);

  r_type &= 0xff;

  // A STT_GNU_IFUNC symbol may require a PLT entry.
  if (is_ifunc
      && this->reloc_needs_plt_for_ifunc(object, r_type))
    target->make_plt_entry(symtab, layout, gsym);

  switch (r_type)
    {
    case elfcpp::R_SPARC_NONE:
    case elfcpp::R_SPARC_REGISTER:
    case elfcpp::R_SPARC_GNU_VTINHERIT:
    case elfcpp::R_SPARC_GNU_VTENTRY:
      break;

    case elfcpp::R_SPARC_PLT64:
    case elfcpp::R_SPARC_PLT32:
    case elfcpp::R_SPARC_HIPLT22:
    case elfcpp::R_SPARC_LOPLT10:
    case elfcpp::R_SPARC_PCPLT32:
    case elfcpp::R_SPARC_PCPLT22:
    case elfcpp::R_SPARC_PCPLT10:
    case elfcpp::R_SPARC_WPLT30:
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

    case elfcpp::R_SPARC_DISP8:
    case elfcpp::R_SPARC_DISP16:
    case elfcpp::R_SPARC_DISP32:
    case elfcpp::R_SPARC_DISP64:
    case elfcpp::R_SPARC_PC_HH22:
    case elfcpp::R_SPARC_PC_HM10:
    case elfcpp::R_SPARC_PC_LM22:
    case elfcpp::R_SPARC_PC10:
    case elfcpp::R_SPARC_PC22:
    case elfcpp::R_SPARC_WDISP30:
    case elfcpp::R_SPARC_WDISP22:
    case elfcpp::R_SPARC_WDISP19:
    case elfcpp::R_SPARC_WDISP16:
    case elfcpp::R_SPARC_WDISP10:
      {
	if (gsym->needs_plt_entry())
	  target->make_plt_entry(symtab, layout, gsym);
	// Make a dynamic relocation if necessary.
	if (gsym->needs_dynamic_reloc(Scan::get_reference_flags(r_type)))
	  {
	    if (parameters->options().output_is_executable()
		&& gsym->may_need_copy_reloc())
	      {
		target->copy_reloc(symtab, layout, object,
				   data_shndx, output_section, gsym,
				   reloc);
	      }
	    else
	      {
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		check_non_pic(object, r_type);
		rela_dyn->add_global(gsym, orig_r_type, output_section, object,
				     data_shndx, reloc.get_r_offset(),
				     reloc.get_r_addend());
	      }
	  }
      }
      break;

    case elfcpp::R_SPARC_UA64:
    case elfcpp::R_SPARC_64:
    case elfcpp::R_SPARC_HIX22:
    case elfcpp::R_SPARC_LOX10:
    case elfcpp::R_SPARC_H34:
    case elfcpp::R_SPARC_H44:
    case elfcpp::R_SPARC_M44:
    case elfcpp::R_SPARC_L44:
    case elfcpp::R_SPARC_HH22:
    case elfcpp::R_SPARC_HM10:
    case elfcpp::R_SPARC_LM22:
    case elfcpp::R_SPARC_HI22:
    case elfcpp::R_SPARC_LO10:
    case elfcpp::R_SPARC_OLO10:
    case elfcpp::R_SPARC_UA32:
    case elfcpp::R_SPARC_32:
    case elfcpp::R_SPARC_UA16:
    case elfcpp::R_SPARC_16:
    case elfcpp::R_SPARC_11:
    case elfcpp::R_SPARC_10:
    case elfcpp::R_SPARC_8:
    case elfcpp::R_SPARC_7:
    case elfcpp::R_SPARC_6:
    case elfcpp::R_SPARC_5:
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
	    unsigned int r_off = reloc.get_r_offset();

	    // The assembler can sometimes emit unaligned relocations
	    // for dwarf2 cfi directives.
	    switch (r_type)
	      {
	      case elfcpp::R_SPARC_16:
		if (r_off & 0x1)
		  orig_r_type = r_type = elfcpp::R_SPARC_UA16;
		break;
	      case elfcpp::R_SPARC_32:
		if (r_off & 0x3)
		  orig_r_type = r_type = elfcpp::R_SPARC_UA32;
		break;
	      case elfcpp::R_SPARC_64:
		if (r_off & 0x7)
		  orig_r_type = r_type = elfcpp::R_SPARC_UA64;
		break;
	      case elfcpp::R_SPARC_UA16:
		if (!(r_off & 0x1))
		  orig_r_type = r_type = elfcpp::R_SPARC_16;
		break;
	      case elfcpp::R_SPARC_UA32:
		if (!(r_off & 0x3))
		  orig_r_type = r_type = elfcpp::R_SPARC_32;
		break;
	      case elfcpp::R_SPARC_UA64:
		if (!(r_off & 0x7))
		  orig_r_type = r_type = elfcpp::R_SPARC_64;
		break;
	      }

	    if (!parameters->options().output_is_position_independent()
		&& gsym->may_need_copy_reloc())
	      {
		target->copy_reloc(symtab, layout, object,
				   data_shndx, output_section, gsym, reloc);
	      }
	    else if (((size == 64 && r_type == elfcpp::R_SPARC_64)
		      || (size == 32 && r_type == elfcpp::R_SPARC_32))
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
		  target->rela_ifunc_section(layout);
		unsigned int r_type = elfcpp::R_SPARC_IRELATIVE;
		rela_dyn->add_symbolless_global_addend(gsym, r_type,
						       output_section, object,
						       data_shndx,
						       reloc.get_r_offset(),
						       reloc.get_r_addend());
	      }
	    else if (((size == 64 && r_type == elfcpp::R_SPARC_64)
		      || (size == 32 && r_type == elfcpp::R_SPARC_32))
		     && gsym->can_use_relative_reloc(false))
	      {
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_global_relative(gsym, elfcpp::R_SPARC_RELATIVE,
					      output_section, object,
					      data_shndx, reloc.get_r_offset(),
					      reloc.get_r_addend(), is_ifunc);
	      }
	    else
	      {
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);

		check_non_pic(object, r_type);
		if (gsym->is_from_dynobj()
		    || gsym->is_undefined()
		    || gsym->is_preemptible())
		  rela_dyn->add_global(gsym, orig_r_type, output_section,
				       object, data_shndx,
				       reloc.get_r_offset(),
				       reloc.get_r_addend());
		else
		  rela_dyn->add_symbolless_global_addend(gsym, orig_r_type,
							 output_section,
							 object, data_shndx,
							 reloc.get_r_offset(),
							 reloc.get_r_addend());
	      }
	  }
      }
      break;

    case elfcpp::R_SPARC_GOTDATA_OP:
    case elfcpp::R_SPARC_GOTDATA_OP_HIX22:
    case elfcpp::R_SPARC_GOTDATA_OP_LOX10:
      if (gsym->is_defined()
	  && !gsym->is_from_dynobj()
	  && !gsym->is_preemptible()
	  && !is_ifunc)
	{
	  // We will optimize this into a GOT relative relocation
	  // and code transform the GOT load into an addition.
	  break;
	}
      // Fall through.
    case elfcpp::R_SPARC_GOT10:
    case elfcpp::R_SPARC_GOT13:
    case elfcpp::R_SPARC_GOT22:
      {
	// The symbol requires a GOT entry.
	Output_data_got<size, big_endian>* got;

	got = target->got_section(symtab, layout);
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
	    bool is_ifunc = gsym->type() == elfcpp::STT_GNU_IFUNC;

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
	    Reloc_section* rela_dyn = target->rela_dyn_section(layout);
	    if (gsym->is_from_dynobj()
		|| gsym->is_undefined()
		|| gsym->is_preemptible()
		|| (gsym->visibility() == elfcpp::STV_PROTECTED
		    && parameters->options().shared())
		|| (gsym->type() == elfcpp::STT_GNU_IFUNC
		    && parameters->options().output_is_position_independent()
		    && !gsym->is_forced_local()))
	      {
		unsigned int r_type = elfcpp::R_SPARC_GLOB_DAT;

		// If this symbol is forced local, this relocation will
		// not work properly.  That's because ld.so on sparc
		// (and 32-bit powerpc) expects st_value in the r_addend
		// of relocations for STB_LOCAL symbols.  Curiously the
		// BFD linker does not promote global hidden symbols to be
		// STB_LOCAL in the dynamic symbol table like Gold does.
		gold_assert(!gsym->is_forced_local());
		got->add_global_with_rel(gsym, GOT_TYPE_STANDARD, rela_dyn,
					 r_type);
	      }
	    else if (!gsym->has_got_offset(GOT_TYPE_STANDARD))
	      {
		unsigned int off = got->add_constant(0);

		gsym->set_got_offset(GOT_TYPE_STANDARD, off);
		if (is_ifunc)
		  {
		    // Tell the dynamic linker to use the PLT address
		    // when resolving relocations.
		    if (gsym->is_from_dynobj()
			&& !parameters->options().shared())
		      gsym->set_needs_dynsym_value();
		  }
		rela_dyn->add_global_relative(gsym, elfcpp::R_SPARC_RELATIVE,
					      got, off, 0, is_ifunc);
	      }
	  }
      }
      break;

      // These are initial tls relocs, which are expected when
      // linking.
    case elfcpp::R_SPARC_TLS_GD_HI22: // Global-dynamic
    case elfcpp::R_SPARC_TLS_GD_LO10:
    case elfcpp::R_SPARC_TLS_GD_ADD:
    case elfcpp::R_SPARC_TLS_GD_CALL:
    case elfcpp::R_SPARC_TLS_LDM_HI22:	// Local-dynamic
    case elfcpp::R_SPARC_TLS_LDM_LO10:
    case elfcpp::R_SPARC_TLS_LDM_ADD:
    case elfcpp::R_SPARC_TLS_LDM_CALL:
    case elfcpp::R_SPARC_TLS_LDO_HIX22:	// Alternate local-dynamic
    case elfcpp::R_SPARC_TLS_LDO_LOX10:
    case elfcpp::R_SPARC_TLS_LDO_ADD:
    case elfcpp::R_SPARC_TLS_LE_HIX22:
    case elfcpp::R_SPARC_TLS_LE_LOX10:
    case elfcpp::R_SPARC_TLS_IE_HI22:	// Initial-exec
    case elfcpp::R_SPARC_TLS_IE_LO10:
    case elfcpp::R_SPARC_TLS_IE_LD:
    case elfcpp::R_SPARC_TLS_IE_LDX:
    case elfcpp::R_SPARC_TLS_IE_ADD:
      {
	const bool is_final = gsym->final_value_is_known();
	const tls::Tls_optimization optimized_type
	    = optimize_tls_reloc(is_final, r_type);
	switch (r_type)
	  {
	  case elfcpp::R_SPARC_TLS_GD_HI22: // Global-dynamic
	  case elfcpp::R_SPARC_TLS_GD_LO10:
	  case elfcpp::R_SPARC_TLS_GD_ADD:
	  case elfcpp::R_SPARC_TLS_GD_CALL:
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Output_data_got<size, big_endian>* got
		    = target->got_section(symtab, layout);
		got->add_global_pair_with_rel(gsym, GOT_TYPE_TLS_PAIR,
					      target->rela_dyn_section(layout),
					      (size == 64
					       ? elfcpp::R_SPARC_TLS_DTPMOD64
					       : elfcpp::R_SPARC_TLS_DTPMOD32),
					      (size == 64
					       ? elfcpp::R_SPARC_TLS_DTPOFF64
					       : elfcpp::R_SPARC_TLS_DTPOFF32));

		// Emit R_SPARC_WPLT30 against "__tls_get_addr"
		if (r_type == elfcpp::R_SPARC_TLS_GD_CALL)
		  generate_tls_call(symtab, layout, target);
	      }
	    else if (optimized_type == tls::TLSOPT_TO_IE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<size, big_endian>* got
		    = target->got_section(symtab, layout);
		got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
					 target->rela_dyn_section(layout),
					 (size == 64 ?
					  elfcpp::R_SPARC_TLS_TPOFF64 :
					  elfcpp::R_SPARC_TLS_TPOFF32));
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_SPARC_TLS_LDM_HI22:	// Local-dynamic
	  case elfcpp::R_SPARC_TLS_LDM_LO10:
	  case elfcpp::R_SPARC_TLS_LDM_ADD:
	  case elfcpp::R_SPARC_TLS_LDM_CALL:
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);

		if (r_type == elfcpp::R_SPARC_TLS_LDM_CALL)
		  generate_tls_call(symtab, layout, target);
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;

	  case elfcpp::R_SPARC_TLS_LDO_HIX22:	// Alternate local-dynamic
	  case elfcpp::R_SPARC_TLS_LDO_LOX10:
	  case elfcpp::R_SPARC_TLS_LDO_ADD:
	    break;

	  case elfcpp::R_SPARC_TLS_LE_HIX22:
	  case elfcpp::R_SPARC_TLS_LE_LOX10:
	    layout->set_has_static_tls();
	    if (parameters->options().shared())
	      {
		Reloc_section* rela_dyn = target->rela_dyn_section(layout);
		rela_dyn->add_symbolless_global_addend(gsym, orig_r_type,
						       output_section, object,
						       data_shndx, reloc.get_r_offset(),
						       0);
	      }
	    break;

	  case elfcpp::R_SPARC_TLS_IE_HI22:	// Initial-exec
	  case elfcpp::R_SPARC_TLS_IE_LO10:
	  case elfcpp::R_SPARC_TLS_IE_LD:
	  case elfcpp::R_SPARC_TLS_IE_LDX:
	  case elfcpp::R_SPARC_TLS_IE_ADD:
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Output_data_got<size, big_endian>* got
		  = target->got_section(symtab, layout);
		got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
					 target->rela_dyn_section(layout),
					 (size == 64
					  ? elfcpp::R_SPARC_TLS_TPOFF64
					  : elfcpp::R_SPARC_TLS_TPOFF32));
	      }
	    else if (optimized_type != tls::TLSOPT_TO_LE)
	      unsupported_reloc_global(object, r_type, gsym);
	    break;
	  }
      }
      break;

      // These are relocations which should only be seen by the
      // dynamic linker, and should never be seen here.
    case elfcpp::R_SPARC_COPY:
    case elfcpp::R_SPARC_GLOB_DAT:
    case elfcpp::R_SPARC_JMP_SLOT:
    case elfcpp::R_SPARC_JMP_IREL:
    case elfcpp::R_SPARC_RELATIVE:
    case elfcpp::R_SPARC_IRELATIVE:
    case elfcpp::R_SPARC_TLS_DTPMOD64:
    case elfcpp::R_SPARC_TLS_DTPMOD32:
    case elfcpp::R_SPARC_TLS_DTPOFF64:
    case elfcpp::R_SPARC_TLS_DTPOFF32:
    case elfcpp::R_SPARC_TLS_TPOFF64:
    case elfcpp::R_SPARC_TLS_TPOFF32:
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

    default:
      unsupported_reloc_global(object, r_type, gsym);
      break;
    }
}

// Make a new symbol table entry.
// STT_SPARC_REGISTER symbols require special handling,
// so we intercept these symbols and keep track of them separately.
// We will resolve register symbols here and output them at symbol
// finalization time.

template<int size, bool big_endian>
Sized_symbol<size>*
Target_sparc<size, big_endian>::make_symbol(const char* name,
					    elfcpp::STT type,
					    Object* object,
					    unsigned int shndx,
					    uint64_t value)
{
  // REGISTER symbols are used only on SPARC-64.
  if (size == 64 && type == elfcpp::STT_SPARC_REGISTER)
    {
      // Ignore REGISTER symbols in dynamic objects.
      if (object->is_dynamic())
	return NULL;
      // Only registers 2, 3, 6, and 7 can be declared global.
      int reg = value;
      switch (reg)
	{
	case 2: case 3:
	  reg -= 2;
	  break;
	case 6: case 7:
	  reg -= 4;
	  break;
	default:
	  gold_error(_("%s: only registers %%g[2367] can be declared "
		       "using STT_REGISTER"),
		     object->name().c_str());
	  return NULL;
	}
      Register_symbol& rsym = this->register_syms_[reg];
      if (rsym.name == NULL)
	{
	  rsym.name = name;
	  rsym.shndx = shndx;
	  rsym.obj = object;
	}
      else
	{
	  if (strcmp(rsym.name, name) != 0)
	    {
	      gold_error(_("%s: register %%g%d declared as '%s'; "
			   "previously declared as '%s' in %s"),
			 object->name().c_str(),
			 static_cast<int>(value),
			 *name ? name : "#scratch",
			 *rsym.name ? rsym.name : "#scratch",
			 rsym.obj->name().c_str());
	      return NULL;
	    }
	}
      return NULL;
    }
  return new Sized_symbol<size>();
}

// Process relocations for gc.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::gc_process_relocs(
			Symbol_table* symtab,
			Layout* layout,
			Sized_relobj_file<size, big_endian>* object,
			unsigned int data_shndx,
			unsigned int,
			const unsigned char* prelocs,
			size_t reloc_count,
			Output_section* output_section,
			bool needs_special_offset_handling,
			size_t local_symbol_count,
			const unsigned char* plocal_symbols)
{
  typedef Target_sparc<size, big_endian> Sparc;
  typedef typename Target_sparc<size, big_endian>::Scan Scan;
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;

  gold::gc_process_relocs<size, big_endian, Sparc, Scan, Classify_reloc>(
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
Target_sparc<size, big_endian>::scan_relocs(
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
			const unsigned char* plocal_symbols)
{
  typedef Target_sparc<size, big_endian> Sparc;
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;

  if (sh_type == elfcpp::SHT_REL)
    {
      gold_error(_("%s: unsupported REL reloc section"),
		 object->name().c_str());
      return;
    }

  gold::scan_relocs<size, big_endian, Sparc, Scan, Classify_reloc>(
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

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::do_finalize_sections(
    Layout* layout,
    const Input_objects*,
    Symbol_table* symtab)
{
  if (this->plt_)
    this->plt_->emit_pending_ifunc_relocs();

  // Fill in some more dynamic tags.
  const Reloc_section* rel_plt = (this->plt_ == NULL
				  ? NULL
				  : this->plt_->rel_plt());
  layout->add_target_dynamic_tags(false, this->plt_, rel_plt,
				  this->rela_dyn_, true, true, false);

  // Emit any relocs we saved in an attempt to avoid generating COPY
  // relocs.
  if (this->copy_relocs_.any_saved_relocs())
    this->copy_relocs_.emit(this->rela_dyn_section(layout));

  if (parameters->doing_static_link()
      && (this->plt_ == NULL || !this->plt_->has_ifunc_section()))
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

  for (int reg = 0; reg < 4; ++reg)
    {
      Register_symbol& rsym = this->register_syms_[reg];
      if (rsym.name != NULL)
	{
	  int value = reg < 3 ? reg + 2 : reg + 4;
	  Sized_symbol<size>* sym = new Sized_symbol<size>();
	  if (rsym.shndx == elfcpp::SHN_UNDEF)
	    sym->init_undefined(rsym.name, NULL, value,
				elfcpp::STT_SPARC_REGISTER, elfcpp::STB_GLOBAL,
				elfcpp::STV_DEFAULT, 0);
	  else
	    sym->init_constant(rsym.name, NULL, value, 0,
			       elfcpp::STT_SPARC_REGISTER, elfcpp::STB_GLOBAL,
			       elfcpp::STV_DEFAULT, 0, false);
	  symtab->add_target_global_symbol(sym);
	  layout->add_target_specific_dynamic_tag(elfcpp::DT_SPARC_REGISTER,
						  value);
	}
    }
}

// Perform a relocation.

template<int size, bool big_endian>
inline bool
Target_sparc<size, big_endian>::Relocate::relocate(
			const Relocate_info<size, big_endian>* relinfo,
			unsigned int,
			Target_sparc* target,
			Output_section*,
			size_t relnum,
			const unsigned char* preloc,
			const Sized_symbol<size>* gsym,
			const Symbol_value<size>* psymval,
			unsigned char* view,
			typename elfcpp::Elf_types<size>::Elf_Addr address,
			section_size_type view_size)
{
  const elfcpp::Rela<size, big_endian> rela(preloc);
  unsigned int r_type = elfcpp::elf_r_type<size>(rela.get_r_info());
  bool orig_is_ifunc = psymval->is_ifunc_symbol();
  r_type &= 0xff;

  if (this->ignore_gd_add_)
    {
      if (r_type != elfcpp::R_SPARC_TLS_GD_ADD)
	gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			       _("missing expected TLS relocation"));
      else
	{
	  this->ignore_gd_add_ = false;
	  return false;
	}
    }

  if (view == NULL)
    return true;

  if (this->reloc_adjust_addr_ == view)
    view -= 4;

  typedef Sparc_relocate_functions<size, big_endian> Reloc;
  const Sized_relobj_file<size, big_endian>* object = relinfo->object;

  // Pick the value to use for symbols defined in shared objects.
  Symbol_value<size> symval;
  if (gsym != NULL
      && gsym->use_plt_offset(Scan::get_reference_flags(r_type)))
    {
      elfcpp::Elf_Xword value;

      value = target->plt_address_for_global(gsym);

      symval.set_output_value(value);

      psymval = &symval;
    }
  else if (gsym == NULL && orig_is_ifunc)
    {
      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
      if (object->local_has_plt_offset(r_sym))
	{
	  symval.set_output_value(target->plt_address_for_local(object, r_sym));
	  psymval = &symval;
	}
    }

  const elfcpp::Elf_Xword addend = rela.get_r_addend();

  // Get the GOT offset if needed.  Unlike i386 and x86_64, our GOT
  // pointer points to the beginning, not the end, of the table.
  // So we just use the plain offset.
  unsigned int got_offset = 0;
  bool gdop_valid = false;
  switch (r_type)
    {
    case elfcpp::R_SPARC_GOTDATA_OP:
    case elfcpp::R_SPARC_GOTDATA_OP_HIX22:
    case elfcpp::R_SPARC_GOTDATA_OP_LOX10:
      // If this is local, we did not create a GOT entry because we
      // intend to transform this into a GOT relative relocation.
      if (gsym == NULL
	  || (gsym->is_defined()
	      && !gsym->is_from_dynobj()
	      && !gsym->is_preemptible()
	      && !orig_is_ifunc))
	{
	  got_offset = psymval->value(object, addend) - target->got_address();
	  gdop_valid = true;
	  break;
	}
      // Fall through.
    case elfcpp::R_SPARC_GOT10:
    case elfcpp::R_SPARC_GOT13:
    case elfcpp::R_SPARC_GOT22:
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
      break;

    default:
      break;
    }

  switch (r_type)
    {
    case elfcpp::R_SPARC_NONE:
    case elfcpp::R_SPARC_REGISTER:
    case elfcpp::R_SPARC_GNU_VTINHERIT:
    case elfcpp::R_SPARC_GNU_VTENTRY:
      break;

    case elfcpp::R_SPARC_8:
      Relocate_functions<size, big_endian>::rela8(view, object,
						  psymval, addend);
      break;

    case elfcpp::R_SPARC_16:
      if (rela.get_r_offset() & 0x1)
	{
	  // The assembler can sometimes emit unaligned relocations
	  // for dwarf2 cfi directives.
	  Reloc::ua16(view, object, psymval, addend);
	}
      else
	Relocate_functions<size, big_endian>::rela16(view, object,
						     psymval, addend);
      break;

    case elfcpp::R_SPARC_32:
      if (!parameters->options().output_is_position_independent())
	{
	  if (rela.get_r_offset() & 0x3)
	    {
	      // The assembler can sometimes emit unaligned relocations
	      // for dwarf2 cfi directives.
	      Reloc::ua32(view, object, psymval, addend);
	    }
	  else
	    Relocate_functions<size, big_endian>::rela32(view, object,
							 psymval, addend);
	}
      break;

    case elfcpp::R_SPARC_DISP8:
      Reloc::disp8(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_DISP16:
      Reloc::disp16(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_DISP32:
      Reloc::disp32(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_DISP64:
      Reloc::disp64(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_WDISP30:
    case elfcpp::R_SPARC_WPLT30:
      Reloc::wdisp30(view, object, psymval, addend, address);
      if (target->may_relax())
	relax_call(target, view, rela, view_size);
      break;

    case elfcpp::R_SPARC_WDISP22:
      Reloc::wdisp22(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_WDISP19:
      Reloc::wdisp19(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_WDISP16:
      Reloc::wdisp16(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_WDISP10:
      Reloc::wdisp10(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_HI22:
      Reloc::hi22(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_22:
      Reloc::rela32_22(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_13:
      Reloc::rela32_13(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_LO10:
      Reloc::lo10(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_GOTDATA_OP_LOX10:
      if (gdop_valid)
	{
	  Reloc::gdop_lox10(view, got_offset);
	  break;
	}
      // Fall through.
    case elfcpp::R_SPARC_GOT10:
      Reloc::lo10(view, got_offset, addend);
      break;

    case elfcpp::R_SPARC_GOTDATA_OP:
      if (gdop_valid)
	{
	  typedef typename elfcpp::Swap<32, true>::Valtype Insntype;
	  Insntype* wv = reinterpret_cast<Insntype*>(view);
	  Insntype val;

	  // {ld,ldx} [%rs1 + %rs2], %rd --> add %rs1, %rs2, %rd
	  val = elfcpp::Swap<32, true>::readval(wv);
	  val = 0x80000000 | (val & 0x3e07c01f);
	  elfcpp::Swap<32, true>::writeval(wv, val);
	}
      break;

    case elfcpp::R_SPARC_GOT13:
      Reloc::rela32_13(view, got_offset, addend);
      break;

    case elfcpp::R_SPARC_GOTDATA_OP_HIX22:
      if (gdop_valid)
	{
	  Reloc::gdop_hix22(view, got_offset);
	  break;
	}
      // Fall through.
    case elfcpp::R_SPARC_GOT22:
      Reloc::hi22(view, got_offset, addend);
      break;

    case elfcpp::R_SPARC_PC10:
      Reloc::pc10(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_PC22:
      Reloc::pc22(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_TLS_DTPOFF32:
    case elfcpp::R_SPARC_UA32:
      Reloc::ua32(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_PLT64:
      Relocate_functions<size, big_endian>::rela64(view, object,
						   psymval, addend);
      break;

    case elfcpp::R_SPARC_PLT32:
      Relocate_functions<size, big_endian>::rela32(view, object,
						   psymval, addend);
      break;

    case elfcpp::R_SPARC_HIPLT22:
      Reloc::hi22(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_LOPLT10:
      Reloc::lo10(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_PCPLT32:
      Reloc::disp32(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_PCPLT22:
      Reloc::pcplt22(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_PCPLT10:
      Reloc::lo10(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_64:
      if (!parameters->options().output_is_position_independent())
	{
	  if (rela.get_r_offset() & 0x7)
	    {
	      // The assembler can sometimes emit unaligned relocations
	      // for dwarf2 cfi directives.
	      Reloc::ua64(view, object, psymval, addend);
	    }
	  else
	    Relocate_functions<size, big_endian>::rela64(view, object,
							 psymval, addend);
	}
      break;

    case elfcpp::R_SPARC_OLO10:
      {
	unsigned int addend2 = rela.get_r_info() & 0xffffffff;
	addend2 = ((addend2 >> 8) ^ 0x800000) - 0x800000;
	Reloc::olo10(view, object, psymval, addend, addend2);
      }
      break;

    case elfcpp::R_SPARC_HH22:
      Reloc::hh22(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_PC_HH22:
      Reloc::pc_hh22(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_HM10:
      Reloc::hm10(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_PC_HM10:
      Reloc::pc_hm10(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_LM22:
      Reloc::hi22(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_PC_LM22:
      Reloc::pcplt22(view, object, psymval, addend, address);
      break;

    case elfcpp::R_SPARC_11:
      Reloc::rela32_11(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_10:
      Reloc::rela32_10(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_7:
      Reloc::rela32_7(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_6:
      Reloc::rela32_6(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_5:
      Reloc::rela32_5(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_HIX22:
      Reloc::hix22(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_LOX10:
      Reloc::lox10(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_H34:
      Reloc::h34(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_H44:
      Reloc::h44(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_M44:
      Reloc::m44(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_L44:
      Reloc::l44(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_TLS_DTPOFF64:
    case elfcpp::R_SPARC_UA64:
      Reloc::ua64(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_UA16:
      Reloc::ua16(view, object, psymval, addend);
      break;

    case elfcpp::R_SPARC_TLS_GD_HI22:
    case elfcpp::R_SPARC_TLS_GD_LO10:
    case elfcpp::R_SPARC_TLS_GD_ADD:
    case elfcpp::R_SPARC_TLS_GD_CALL:
    case elfcpp::R_SPARC_TLS_LDM_HI22:
    case elfcpp::R_SPARC_TLS_LDM_LO10:
    case elfcpp::R_SPARC_TLS_LDM_ADD:
    case elfcpp::R_SPARC_TLS_LDM_CALL:
    case elfcpp::R_SPARC_TLS_LDO_HIX22:
    case elfcpp::R_SPARC_TLS_LDO_LOX10:
    case elfcpp::R_SPARC_TLS_LDO_ADD:
    case elfcpp::R_SPARC_TLS_IE_HI22:
    case elfcpp::R_SPARC_TLS_IE_LO10:
    case elfcpp::R_SPARC_TLS_IE_LD:
    case elfcpp::R_SPARC_TLS_IE_LDX:
    case elfcpp::R_SPARC_TLS_IE_ADD:
    case elfcpp::R_SPARC_TLS_LE_HIX22:
    case elfcpp::R_SPARC_TLS_LE_LOX10:
      this->relocate_tls(relinfo, target, relnum, rela,
			 r_type, gsym, psymval, view,
			 address, view_size);
      break;

    case elfcpp::R_SPARC_COPY:
    case elfcpp::R_SPARC_GLOB_DAT:
    case elfcpp::R_SPARC_JMP_SLOT:
    case elfcpp::R_SPARC_JMP_IREL:
    case elfcpp::R_SPARC_RELATIVE:
    case elfcpp::R_SPARC_IRELATIVE:
      // These are outstanding tls relocs, which are unexpected when
      // linking.
    case elfcpp::R_SPARC_TLS_DTPMOD64:
    case elfcpp::R_SPARC_TLS_DTPMOD32:
    case elfcpp::R_SPARC_TLS_TPOFF64:
    case elfcpp::R_SPARC_TLS_TPOFF32:
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

// Perform a TLS relocation.

template<int size, bool big_endian>
inline void
Target_sparc<size, big_endian>::Relocate::relocate_tls(
			const Relocate_info<size, big_endian>* relinfo,
			Target_sparc<size, big_endian>* target,
			size_t relnum,
			const elfcpp::Rela<size, big_endian>& rela,
			unsigned int r_type,
			const Sized_symbol<size>* gsym,
			const Symbol_value<size>* psymval,
			unsigned char* view,
			typename elfcpp::Elf_types<size>::Elf_Addr address,
			section_size_type)
{
  Output_segment* tls_segment = relinfo->layout->tls_segment();
  typedef Sparc_relocate_functions<size, big_endian> Reloc;
  const Sized_relobj_file<size, big_endian>* object = relinfo->object;
  typedef typename elfcpp::Swap<32, true>::Valtype Insntype;

  const elfcpp::Elf_Xword addend = rela.get_r_addend();
  typename elfcpp::Elf_types<size>::Elf_Addr value = psymval->value(object, 0);

  const bool is_final =
    (gsym == NULL
     ? !parameters->options().shared()
     : gsym->final_value_is_known());
  const tls::Tls_optimization optimized_type
      = optimize_tls_reloc(is_final, r_type);

  switch (r_type)
    {
    case elfcpp::R_SPARC_TLS_GD_HI22:
    case elfcpp::R_SPARC_TLS_GD_LO10:
    case elfcpp::R_SPARC_TLS_GD_ADD:
    case elfcpp::R_SPARC_TLS_GD_CALL:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  Insntype* wv = reinterpret_cast<Insntype*>(view);
	  Insntype val;

	  value -= tls_segment->memsz();

	  switch (r_type)
	    {
	    case elfcpp::R_SPARC_TLS_GD_HI22:
	      // TLS_GD_HI22 --> TLS_LE_HIX22
	      Reloc::hix22(view, value, addend);
	      break;

	    case elfcpp::R_SPARC_TLS_GD_LO10:
	      // TLS_GD_LO10 --> TLS_LE_LOX10
	      Reloc::lox10(view, value, addend);
	      break;

	    case elfcpp::R_SPARC_TLS_GD_ADD:
	      // add %reg1, %reg2, %reg3 --> mov %g7, %reg2, %reg3
	      val = elfcpp::Swap<32, true>::readval(wv);
	      val = (val & ~0x7c000) | 0x1c000;
	      elfcpp::Swap<32, true>::writeval(wv, val);
	      break;
	    case elfcpp::R_SPARC_TLS_GD_CALL:
	      // call __tls_get_addr --> nop
	      elfcpp::Swap<32, true>::writeval(wv, sparc_nop);
	      break;
	    }
	  break;
	}
      else
	{
	  unsigned int got_type = (optimized_type == tls::TLSOPT_TO_IE
				   ? GOT_TYPE_TLS_OFFSET
				   : GOT_TYPE_TLS_PAIR);
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(got_type));
	      value = gsym->got_offset(got_type);
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      value = object->local_got_offset(r_sym, got_type);
	    }
	  if (optimized_type == tls::TLSOPT_TO_IE)
	    {
	      Insntype* wv = reinterpret_cast<Insntype*>(view);
	      Insntype val;

	      switch (r_type)
		{
		case elfcpp::R_SPARC_TLS_GD_HI22:
		  // TLS_GD_HI22 --> TLS_IE_HI22
		  Reloc::hi22(view, value, addend);
		  break;

		case elfcpp::R_SPARC_TLS_GD_LO10:
		  // TLS_GD_LO10 --> TLS_IE_LO10
		  Reloc::lo10(view, value, addend);
		  break;

		case elfcpp::R_SPARC_TLS_GD_ADD:
		  // add %reg1, %reg2, %reg3 --> ld [%reg1 + %reg2], %reg3
		  val = elfcpp::Swap<32, true>::readval(wv);

		  if (size == 64)
		    val |= 0xc0580000;
		  else
		    val |= 0xc0000000;

		  elfcpp::Swap<32, true>::writeval(wv, val);
		  break;

		case elfcpp::R_SPARC_TLS_GD_CALL:
		  // The compiler can put the TLS_GD_ADD instruction
		  // into the delay slot of the call.  If so, we need
		  // to transpose the two instructions so that the
		  // new sequence works properly.
		  //
		  // The test we use is if the instruction in the
		  // delay slot is an add with destination register
		  // equal to %o0
		  val = elfcpp::Swap<32, true>::readval(wv + 1);
		  if ((val & 0x81f80000) == 0x80000000
		      && ((val >> 25) & 0x1f) == 0x8)
		    {
		      if (size == 64)
			val |= 0xc0580000;
		      else
			val |= 0xc0000000;

		      elfcpp::Swap<32, true>::writeval(wv, val);

		      wv += 1;
		      this->ignore_gd_add_ = true;
		    }
		  else
		    {
		      // Even if the delay slot isn't the TLS_GD_ADD
		      // instruction, we still have to handle the case
		      // where it sets up %o0 in some other way.
		      elfcpp::Swap<32, true>::writeval(wv, val);
		      wv += 1;
		      this->reloc_adjust_addr_ = view + 4;
		    }
		  // call __tls_get_addr --> add %g7, %o0, %o0
		  elfcpp::Swap<32, true>::writeval(wv, 0x9001c008);
		  break;
		}
	      break;
	    }
	  else if (optimized_type == tls::TLSOPT_NONE)
	    {
	      switch (r_type)
		{
		case elfcpp::R_SPARC_TLS_GD_HI22:
		  Reloc::hi22(view, value, addend);
		  break;
		case elfcpp::R_SPARC_TLS_GD_LO10:
		  Reloc::lo10(view, value, addend);
		  break;
		case elfcpp::R_SPARC_TLS_GD_ADD:
		  break;
		case elfcpp::R_SPARC_TLS_GD_CALL:
		  {
		    Symbol_value<size> symval;
		    elfcpp::Elf_Xword value;
		    Symbol* tsym;

		    tsym = target->tls_get_addr_sym_;
		    gold_assert(tsym);
		    value = (target->plt_section()->address() +
			     tsym->plt_offset());
		    symval.set_output_value(value);
		    Reloc::wdisp30(view, object, &symval, addend, address);
		  }
		  break;
		}
	      break;
	    }
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;

    case elfcpp::R_SPARC_TLS_LDM_HI22:
    case elfcpp::R_SPARC_TLS_LDM_LO10:
    case elfcpp::R_SPARC_TLS_LDM_ADD:
    case elfcpp::R_SPARC_TLS_LDM_CALL:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  Insntype* wv = reinterpret_cast<Insntype*>(view);

	  switch (r_type)
	    {
	    case elfcpp::R_SPARC_TLS_LDM_HI22:
	    case elfcpp::R_SPARC_TLS_LDM_LO10:
	    case elfcpp::R_SPARC_TLS_LDM_ADD:
	      elfcpp::Swap<32, true>::writeval(wv, sparc_nop);
	      break;

	    case elfcpp::R_SPARC_TLS_LDM_CALL:
	      elfcpp::Swap<32, true>::writeval(wv, sparc_mov_g0_o0);
	      break;
	    }
	  break;
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the module index.
	  unsigned int got_offset;

	  got_offset = target->got_mod_index_entry(NULL, NULL, NULL);
	  switch (r_type)
	    {
	    case elfcpp::R_SPARC_TLS_LDM_HI22:
	      Reloc::hi22(view, got_offset, addend);
	      break;
	    case elfcpp::R_SPARC_TLS_LDM_LO10:
	      Reloc::lo10(view, got_offset, addend);
	      break;
	    case elfcpp::R_SPARC_TLS_LDM_ADD:
	      break;
	    case elfcpp::R_SPARC_TLS_LDM_CALL:
	      {
		Symbol_value<size> symval;
		elfcpp::Elf_Xword value;
		Symbol* tsym;

		tsym = target->tls_get_addr_sym_;
		gold_assert(tsym);
		value = (target->plt_section()->address() +
			 tsym->plt_offset());
		symval.set_output_value(value);
		Reloc::wdisp30(view, object, &symval, addend, address);
	      }
	      break;
	    }
	  break;
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;

      // These relocs can appear in debugging sections, in which case
      // we won't see the TLS_LDM relocs.  The local_dynamic_type
      // field tells us this.
    case elfcpp::R_SPARC_TLS_LDO_HIX22:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  value -= tls_segment->memsz();
	  Reloc::hix22(view, value, addend);
	}
      else
	Reloc::ldo_hix22(view, value, addend);
      break;
    case elfcpp::R_SPARC_TLS_LDO_LOX10:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  value -= tls_segment->memsz();
	  Reloc::lox10(view, value, addend);
	}
      else
	Reloc::ldo_lox10(view, value, addend);
      break;
    case elfcpp::R_SPARC_TLS_LDO_ADD:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  Insntype* wv = reinterpret_cast<Insntype*>(view);
	  Insntype val;

	  // add %reg1, %reg2, %reg3 --> add %g7, %reg2, %reg3
	  val = elfcpp::Swap<32, true>::readval(wv);
	  val = (val & ~0x7c000) | 0x1c000;
	  elfcpp::Swap<32, true>::writeval(wv, val);
	}
      break;

      // When optimizing IE --> LE, the only relocation that is handled
      // differently is R_SPARC_TLS_IE_LD, it is rewritten from
      // 'ld{,x} [rs1 + rs2], rd' into 'mov rs2, rd' or simply a NOP is
      // rs2 and rd are the same.
    case elfcpp::R_SPARC_TLS_IE_LD:
    case elfcpp::R_SPARC_TLS_IE_LDX:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  Insntype* wv = reinterpret_cast<Insntype*>(view);
	  Insntype val = elfcpp::Swap<32, true>::readval(wv);
	  Insntype rs2 = val & 0x1f;
	  Insntype rd = (val >> 25) & 0x1f;

	  if (rs2 == rd)
	    val = sparc_nop;
	  else
	    val = sparc_mov | (val & 0x3e00001f);

	  elfcpp::Swap<32, true>::writeval(wv, val);
	}
      break;

    case elfcpp::R_SPARC_TLS_IE_HI22:
    case elfcpp::R_SPARC_TLS_IE_LO10:
      if (optimized_type == tls::TLSOPT_TO_LE)
	{
	  value -= tls_segment->memsz();
	  switch (r_type)
	    {
	    case elfcpp::R_SPARC_TLS_IE_HI22:
	      // IE_HI22 --> LE_HIX22
	      Reloc::hix22(view, value, addend);
	      break;
	    case elfcpp::R_SPARC_TLS_IE_LO10:
	      // IE_LO10 --> LE_LOX10
	      Reloc::lox10(view, value, addend);
	      break;
	    }
	  break;
	}
      else if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the tp-relative offset of the symbol.
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(GOT_TYPE_TLS_OFFSET));
	      value = gsym->got_offset(GOT_TYPE_TLS_OFFSET);
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym,
						       GOT_TYPE_TLS_OFFSET));
	      value = object->local_got_offset(r_sym,
					       GOT_TYPE_TLS_OFFSET);
	    }
	  switch (r_type)
	    {
	    case elfcpp::R_SPARC_TLS_IE_HI22:
	      Reloc::hi22(view, value, addend);
	      break;
	    case elfcpp::R_SPARC_TLS_IE_LO10:
	      Reloc::lo10(view, value, addend);
	      break;
	    }
	  break;
	}
      gold_error_at_location(relinfo, relnum, rela.get_r_offset(),
			     _("unsupported reloc %u"),
			     r_type);
      break;

    case elfcpp::R_SPARC_TLS_IE_ADD:
      // This seems to be mainly so that we can find the addition
      // instruction if there is one.  There doesn't seem to be any
      // actual relocation to apply.
      break;

    case elfcpp::R_SPARC_TLS_LE_HIX22:
      // If we're creating a shared library, a dynamic relocation will
      // have been created for this location, so do not apply it now.
      if (!parameters->options().shared())
	{
	  value -= tls_segment->memsz();
	  Reloc::hix22(view, value, addend);
	}
      break;

    case elfcpp::R_SPARC_TLS_LE_LOX10:
      // If we're creating a shared library, a dynamic relocation will
      // have been created for this location, so do not apply it now.
      if (!parameters->options().shared())
	{
	  value -= tls_segment->memsz();
	  Reloc::lox10(view, value, addend);
	}
      break;
    }
}

// Relax a call instruction.

template<int size, bool big_endian>
inline void
Target_sparc<size, big_endian>::Relocate::relax_call(
    Target_sparc<size, big_endian>* target,
    unsigned char* view,
    const elfcpp::Rela<size, big_endian>& rela,
    section_size_type view_size)
{
  typedef typename elfcpp::Swap<32, true>::Valtype Insntype;
  Insntype *wv = reinterpret_cast<Insntype*>(view);
  Insntype call_insn, delay_insn, set_insn;
  uint32_t op3, reg, off;

  // This code tries to relax call instructions that meet
  // certain criteria.
  //
  // The first criteria is that the call must be such that the return
  // address which the call writes into %o7 is unused.  Two sequences
  // meet this criteria, and are used to implement tail calls.
  //
  // Leaf function tail call:
  //
  // or %o7, %g0, %ANY_REG
  // call FUNC
  //  or %ANY_REG, %g0, %o7
  //
  // Non-leaf function tail call:
  //
  // call FUNC
  //  restore
  //
  // The second criteria is that the call destination is close.  If
  // the displacement can fit in a signed 22-bit immediate field of a
  // pre-V9 branch, we can do it.  If we are generating a 64-bit
  // object or a 32-bit object with ELF machine type EF_SPARC32PLUS,
  // and the displacement fits in a signed 19-bit immediate field,
  // then we can use a V9 branch.

  // Make sure the delay instruction can be safely accessed.
  if (rela.get_r_offset() + 8 > view_size)
    return;

  call_insn = elfcpp::Swap<32, true>::readval(wv);
  delay_insn = elfcpp::Swap<32, true>::readval(wv + 1);

  // Make sure it is really a call instruction.
  if (((call_insn >> 30) & 0x3) != 1)
    return;

  if (((delay_insn >> 30) & 0x3) != 2)
    return;

  // Accept only a restore or an integer arithmetic operation whose
  // sole side effect is to write the %o7 register (and perhaps set
  // the condition codes, which are considered clobbered across
  // function calls).
  //
  // For example, we don't want to match a tagged addition or
  // subtraction.  We also don't want to match something like a
  // divide.
  //
  // Specifically we accept add{,cc}, and{,cc}, or{,cc},
  // xor{,cc}, sub{,cc}, andn{,cc}, orn{,cc}, and xnor{,cc}.

  op3 = (delay_insn >> 19) & 0x3f;
  reg = (delay_insn >> 25) & 0x1f;
  if (op3 != 0x3d
      && ((op3 & 0x28) != 0 || reg != 15))
    return;

  // For non-restore instructions, make sure %o7 isn't
  // an input.
  if (op3 != 0x3d)
    {
      // First check RS1
      reg = (delay_insn >> 14) & 0x1f;
      if (reg == 15)
	return;

      // And if non-immediate, check RS2
      if (((delay_insn >> 13) & 1) == 0)
	{
	  reg = (delay_insn & 0x1f);
	  if (reg == 15)
	    return;
	}
    }

  // Now check the branch distance.  We are called after the
  // call has been relocated, so we just have to peek at the
  // offset contained in the instruction.
  off = call_insn & 0x3fffffff;
  if ((off & 0x3fe00000) != 0
      && (off & 0x3fe00000) != 0x3fe00000)
    return;

  if ((size == 64 || target->elf_machine_ == elfcpp::EM_SPARC32PLUS)
      && ((off & 0x3c0000) == 0
	  || (off & 0x3c0000) == 0x3c0000))
    {
      // ba,pt %xcc, FUNC
      call_insn = 0x10680000 | (off & 0x07ffff);
    }
  else
    {
      // ba FUNC
      call_insn = 0x10800000 | (off & 0x3fffff);
    }
  elfcpp::Swap<32, true>::writeval(wv, call_insn);

  // See if we can NOP out the delay slot instruction.  We peek
  // at the instruction before the call to make sure we're dealing
  // with exactly the:
  //
  // or %o7, %g0, %ANY_REG
  // call
  //  or %ANY_REG, %g0, %o7
  //
  // case.  Otherwise this might be a tricky piece of hand written
  // assembler calculating %o7 in some non-trivial way, and therefore
  // we can't be sure that NOP'ing out the delay slot is safe.
  if (op3 == 0x02
      && rela.get_r_offset() >= 4)
    {
      if ((delay_insn & ~(0x1f << 14)) != 0x9e100000)
	return;

      set_insn = elfcpp::Swap<32, true>::readval(wv - 1);
      if ((set_insn & ~(0x1f << 25)) != 0x8013c000)
	return;

      reg = (set_insn >> 25) & 0x1f;
      if (reg == 0 || reg == 15)
	return;
      if (reg != ((delay_insn >> 14) & 0x1f))
	return;

      // All tests pass, nop it out.
      elfcpp::Swap<32, true>::writeval(wv + 1, sparc_nop);
    }
}

// Relocate section data.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::relocate_section(
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
  typedef Target_sparc<size, big_endian> Sparc;
  typedef typename Target_sparc<size, big_endian>::Relocate Sparc_relocate;
  typedef gold::Default_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
      Classify_reloc;

  gold_assert(sh_type == elfcpp::SHT_RELA);

  gold::relocate_section<size, big_endian, Sparc, Sparc_relocate,
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

// Scan the relocs during a relocatable link.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::scan_relocatable_relocs(
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
Target_sparc<size, big_endian>::emit_relocs_scan(
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

// Emit relocations for a section.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::relocate_relocs(
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
Target_sparc<size, big_endian>::do_dynsym_value(const Symbol* gsym) const
{
  gold_assert(gsym->is_from_dynobj() && gsym->has_plt_offset());
  return this->plt_section()->address() + gsym->plt_offset();
}

// do_make_elf_object to override the same function in the base class.
// We need to use a target-specific sub-class of
// Sized_relobj_file<size, big_endian> to process SPARC specific bits
// of the ELF headers.  Hence we need to have our own ELF object creation.

template<int size, bool big_endian>
Object*
Target_sparc<size, big_endian>::do_make_elf_object(
    const std::string& name,
    Input_file* input_file,
    off_t offset, const elfcpp::Ehdr<size, big_endian>& ehdr)
{
  elfcpp::Elf_Half machine = ehdr.get_e_machine();
  elfcpp::Elf_Word flags = ehdr.get_e_flags();
  elfcpp::Elf_Word omm, mm;

  switch (machine)
    {
    case elfcpp::EM_SPARC32PLUS:
      this->elf_machine_ = elfcpp::EM_SPARC32PLUS;
      break;

    case elfcpp::EM_SPARC:
    case elfcpp::EM_SPARCV9:
      break;

    default:
      break;
    }

  if (!this->elf_flags_set_)
    {
      this->elf_flags_ = flags;
      this->elf_flags_set_ = true;
    }
  else
    {
      // Accumulate cpu feature bits.
      this->elf_flags_ |= (flags & (elfcpp::EF_SPARC_32PLUS
				    | elfcpp::EF_SPARC_SUN_US1
				    | elfcpp::EF_SPARC_HAL_R1
				    | elfcpp::EF_SPARC_SUN_US3));

      // Bump the memory model setting to the most restrictive
      // one we encounter.
      omm = (this->elf_flags_ & elfcpp::EF_SPARCV9_MM);
      mm = (flags & elfcpp::EF_SPARCV9_MM);
      if (omm != mm)
	{
	  if (mm == elfcpp::EF_SPARCV9_TSO)
	    {
	      this->elf_flags_ &= ~elfcpp::EF_SPARCV9_MM;
	      this->elf_flags_ |= elfcpp::EF_SPARCV9_TSO;
	    }
	  else if (mm == elfcpp::EF_SPARCV9_PSO
		   && omm == elfcpp::EF_SPARCV9_RMO)
	    {
	      this->elf_flags_ &= ~elfcpp::EF_SPARCV9_MM;
	      this->elf_flags_ |= elfcpp::EF_SPARCV9_PSO;
	    }
	}
    }

  // Validate that the little-endian flag matches how we've
  // been instantiated.
  if (!(flags & elfcpp::EF_SPARC_LEDATA) != big_endian)
    {
      if (big_endian)
	gold_error(_("%s: little endian elf flag set on BE object"),
		     name.c_str());
      else
	gold_error(_("%s: little endian elf flag clear on LE object"),
		     name.c_str());
    }

  return Target::do_make_elf_object(name, input_file, offset, ehdr);
}

// Adjust ELF file header.

template<int size, bool big_endian>
void
Target_sparc<size, big_endian>::do_adjust_elf_header(
    unsigned char* view,
    int len)
{
  elfcpp::Ehdr_write<size, big_endian> oehdr(view);

  oehdr.put_e_machine(this->elf_machine_);
  oehdr.put_e_flags(this->elf_flags_);

  Sized_target<size, big_endian>::do_adjust_elf_header(view, len);
}

// The selector for sparc object files.

template<int size, bool big_endian>
class Target_selector_sparc : public Target_selector
{
public:
  Target_selector_sparc()
    : Target_selector(elfcpp::EM_NONE, size, big_endian,
		      (size == 64 ? "elf64-sparc" : "elf32-sparc"),
		      (size == 64 ? "elf64_sparc" : "elf32_sparc"))
  { }

  virtual Target*
  do_recognize(Input_file*, off_t, int machine, int, int)
  {
    switch (size)
      {
      case 64:
	if (machine != elfcpp::EM_SPARCV9)
	  return NULL;
	break;

      case 32:
	if (machine != elfcpp::EM_SPARC
	    && machine != elfcpp::EM_SPARC32PLUS)
	  return NULL;
	break;

      default:
	return NULL;
      }

    return this->instantiate_target();
  }

  virtual Target*
  do_instantiate_target()
  { return new Target_sparc<size, big_endian>(); }
};

Target_selector_sparc<32, true> target_selector_sparc32;
Target_selector_sparc<64, true> target_selector_sparc64;

} // End anonymous namespace.
