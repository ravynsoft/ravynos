// mips.cc -- mips target support for gold.

// Copyright (C) 2011-2023 Free Software Foundation, Inc.
// Written by Sasa Stankovic <sasa.stankovic@imgtec.com>
//        and Aleksandar Simeonov <aleksandar.simeonov@rt-rk.com>.
// This file contains borrowed and adapted code from bfd/elfxx-mips.c.

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

#include <algorithm>
#include <set>
#include <sstream>
#include "demangle.h"

#include "elfcpp.h"
#include "parameters.h"
#include "reloc.h"
#include "mips.h"
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
#include "attributes.h"
#include "nacl.h"

namespace
{
using namespace gold;

template<int size, bool big_endian>
class Mips_output_data_plt;

template<int size, bool big_endian>
class Mips_output_data_got;

template<int size, bool big_endian>
class Target_mips;

template<int size, bool big_endian>
class Mips_output_section_reginfo;

template<int size, bool big_endian>
class Mips_output_section_options;

template<int size, bool big_endian>
class Mips_output_data_la25_stub;

template<int size, bool big_endian>
class Mips_output_data_mips_stubs;

template<int size>
class Mips_symbol;

template<int size, bool big_endian>
class Mips_got_info;

template<int size, bool big_endian>
class Mips_relobj;

class Mips16_stub_section_base;

template<int size, bool big_endian>
class Mips16_stub_section;

// The ABI says that every symbol used by dynamic relocations must have
// a global GOT entry.  Among other things, this provides the dynamic
// linker with a free, directly-indexed cache.  The GOT can therefore
// contain symbols that are not referenced by GOT relocations themselves
// (in other words, it may have symbols that are not referenced by things
// like R_MIPS_GOT16 and R_MIPS_GOT_PAGE).

// GOT relocations are less likely to overflow if we put the associated
// GOT entries towards the beginning.  We therefore divide the global
// GOT entries into two areas: "normal" and "reloc-only".  Entries in
// the first area can be used for both dynamic relocations and GP-relative
// accesses, while those in the "reloc-only" area are for dynamic
// relocations only.

// These GGA_* ("Global GOT Area") values are organised so that lower
// values are more general than higher values.  Also, non-GGA_NONE
// values are ordered by the position of the area in the GOT.

enum Global_got_area
{
  GGA_NORMAL = 0,
  GGA_RELOC_ONLY = 1,
  GGA_NONE = 2
};

// The types of GOT entries needed for this platform.
// These values are exposed to the ABI in an incremental link.
// Do not renumber existing values without changing the version
// number of the .gnu_incremental_inputs section.
enum Got_type
{
  GOT_TYPE_STANDARD = 0,      // GOT entry for a regular symbol
  GOT_TYPE_TLS_OFFSET = 1,    // GOT entry for TLS offset
  GOT_TYPE_TLS_PAIR = 2,      // GOT entry for TLS module/offset pair

  // GOT entries for multi-GOT. We support up to 1024 GOTs in multi-GOT links.
  GOT_TYPE_STANDARD_MULTIGOT = 3,
  GOT_TYPE_TLS_OFFSET_MULTIGOT = GOT_TYPE_STANDARD_MULTIGOT + 1024,
  GOT_TYPE_TLS_PAIR_MULTIGOT = GOT_TYPE_TLS_OFFSET_MULTIGOT + 1024
};

// TLS type of GOT entry.
enum Got_tls_type
{
  GOT_TLS_NONE = 0,
  GOT_TLS_GD = 1,
  GOT_TLS_LDM = 2,
  GOT_TLS_IE = 4
};

// Values found in the r_ssym field of a relocation entry.
enum Special_relocation_symbol
{
  RSS_UNDEF = 0,    // None - value is zero.
  RSS_GP = 1,       // Value of GP.
  RSS_GP0 = 2,      // Value of GP in object being relocated.
  RSS_LOC = 3       // Address of location being relocated.
};

// Whether the section is readonly.
static inline bool
is_readonly_section(Output_section* output_section)
{
  elfcpp::Elf_Xword section_flags = output_section->flags();
  elfcpp::Elf_Word section_type = output_section->type();

  if (section_type == elfcpp::SHT_NOBITS)
    return false;

  if (section_flags & elfcpp::SHF_WRITE)
    return false;

  return true;
}

// Return TRUE if a relocation of type R_TYPE from OBJECT might
// require an la25 stub.  See also local_pic_function, which determines
// whether the destination function ever requires a stub.
template<int size, bool big_endian>
static inline bool
relocation_needs_la25_stub(Mips_relobj<size, big_endian>* object,
                           unsigned int r_type, bool target_is_16_bit_code)
{
  // We specifically ignore branches and jumps from EF_PIC objects,
  // where the onus is on the compiler or programmer to perform any
  // necessary initialization of $25.  Sometimes such initialization
  // is unnecessary; for example, -mno-shared functions do not use
  // the incoming value of $25, and may therefore be called directly.
  if (object->is_pic())
    return false;

  switch (r_type)
    {
    case elfcpp::R_MIPS_26:
    case elfcpp::R_MIPS_PC16:
    case elfcpp::R_MIPS_PC21_S2:
    case elfcpp::R_MIPS_PC26_S2:
    case elfcpp::R_MICROMIPS_26_S1:
    case elfcpp::R_MICROMIPS_PC7_S1:
    case elfcpp::R_MICROMIPS_PC10_S1:
    case elfcpp::R_MICROMIPS_PC16_S1:
    case elfcpp::R_MICROMIPS_PC23_S2:
      return true;

    case elfcpp::R_MIPS16_26:
      return !target_is_16_bit_code;

    default:
      return false;
    }
}

// Return true if SYM is a locally-defined PIC function, in the sense
// that it or its fn_stub might need $25 to be valid on entry.
// Note that MIPS16 functions set up $gp using PC-relative instructions,
// so they themselves never need $25 to be valid.  Only non-MIPS16
// entry points are of interest here.
template<int size, bool big_endian>
static inline bool
local_pic_function(Mips_symbol<size>* sym)
{
  bool def_regular = (sym->source() == Symbol::FROM_OBJECT
                      && !sym->object()->is_dynamic()
                      && !sym->is_undefined());

  if (sym->is_defined() && def_regular)
    {
      Mips_relobj<size, big_endian>* object =
        static_cast<Mips_relobj<size, big_endian>*>(sym->object());

      if ((object->is_pic() || sym->is_pic())
          && (!sym->is_mips16()
              || (sym->has_mips16_fn_stub() && sym->need_fn_stub())))
        return true;
    }
  return false;
}

static inline bool
hi16_reloc(int r_type)
{
  return (r_type == elfcpp::R_MIPS_HI16
          || r_type == elfcpp::R_MIPS16_HI16
          || r_type == elfcpp::R_MICROMIPS_HI16
          || r_type == elfcpp::R_MIPS_PCHI16);
}

static inline bool
lo16_reloc(int r_type)
{
  return (r_type == elfcpp::R_MIPS_LO16
          || r_type == elfcpp::R_MIPS16_LO16
          || r_type == elfcpp::R_MICROMIPS_LO16
          || r_type == elfcpp::R_MIPS_PCLO16);
}

static inline bool
got16_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_GOT16
          || r_type == elfcpp::R_MIPS16_GOT16
          || r_type == elfcpp::R_MICROMIPS_GOT16);
}

static inline bool
call_lo16_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_CALL_LO16
          || r_type == elfcpp::R_MICROMIPS_CALL_LO16);
}

static inline bool
got_lo16_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_GOT_LO16
          || r_type == elfcpp::R_MICROMIPS_GOT_LO16);
}

static inline bool
eh_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_EH);
}

static inline bool
got_disp_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_GOT_DISP
          || r_type == elfcpp::R_MICROMIPS_GOT_DISP);
}

static inline bool
got_page_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_GOT_PAGE
          || r_type == elfcpp::R_MICROMIPS_GOT_PAGE);
}

static inline bool
tls_gd_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_TLS_GD
          || r_type == elfcpp::R_MIPS16_TLS_GD
          || r_type == elfcpp::R_MICROMIPS_TLS_GD);
}

static inline bool
tls_gottprel_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_TLS_GOTTPREL
          || r_type == elfcpp::R_MIPS16_TLS_GOTTPREL
          || r_type == elfcpp::R_MICROMIPS_TLS_GOTTPREL);
}

static inline bool
tls_ldm_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_TLS_LDM
          || r_type == elfcpp::R_MIPS16_TLS_LDM
          || r_type == elfcpp::R_MICROMIPS_TLS_LDM);
}

static inline bool
mips16_call_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS16_26
          || r_type == elfcpp::R_MIPS16_CALL16);
}

static inline bool
jal_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MIPS_26
          || r_type == elfcpp::R_MIPS16_26
          || r_type == elfcpp::R_MICROMIPS_26_S1);
}

static inline bool
micromips_branch_reloc(unsigned int r_type)
{
  return (r_type == elfcpp::R_MICROMIPS_26_S1
          || r_type == elfcpp::R_MICROMIPS_PC16_S1
          || r_type == elfcpp::R_MICROMIPS_PC10_S1
          || r_type == elfcpp::R_MICROMIPS_PC7_S1);
}

// Check if R_TYPE is a MIPS16 reloc.
static inline bool
mips16_reloc(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_MIPS16_26:
    case elfcpp::R_MIPS16_GPREL:
    case elfcpp::R_MIPS16_GOT16:
    case elfcpp::R_MIPS16_CALL16:
    case elfcpp::R_MIPS16_HI16:
    case elfcpp::R_MIPS16_LO16:
    case elfcpp::R_MIPS16_TLS_GD:
    case elfcpp::R_MIPS16_TLS_LDM:
    case elfcpp::R_MIPS16_TLS_DTPREL_HI16:
    case elfcpp::R_MIPS16_TLS_DTPREL_LO16:
    case elfcpp::R_MIPS16_TLS_GOTTPREL:
    case elfcpp::R_MIPS16_TLS_TPREL_HI16:
    case elfcpp::R_MIPS16_TLS_TPREL_LO16:
      return true;

    default:
      return false;
    }
}

// Check if R_TYPE is a microMIPS reloc.
static inline bool
micromips_reloc(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_MICROMIPS_26_S1:
    case elfcpp::R_MICROMIPS_HI16:
    case elfcpp::R_MICROMIPS_LO16:
    case elfcpp::R_MICROMIPS_GPREL16:
    case elfcpp::R_MICROMIPS_LITERAL:
    case elfcpp::R_MICROMIPS_GOT16:
    case elfcpp::R_MICROMIPS_PC7_S1:
    case elfcpp::R_MICROMIPS_PC10_S1:
    case elfcpp::R_MICROMIPS_PC16_S1:
    case elfcpp::R_MICROMIPS_CALL16:
    case elfcpp::R_MICROMIPS_GOT_DISP:
    case elfcpp::R_MICROMIPS_GOT_PAGE:
    case elfcpp::R_MICROMIPS_GOT_OFST:
    case elfcpp::R_MICROMIPS_GOT_HI16:
    case elfcpp::R_MICROMIPS_GOT_LO16:
    case elfcpp::R_MICROMIPS_SUB:
    case elfcpp::R_MICROMIPS_HIGHER:
    case elfcpp::R_MICROMIPS_HIGHEST:
    case elfcpp::R_MICROMIPS_CALL_HI16:
    case elfcpp::R_MICROMIPS_CALL_LO16:
    case elfcpp::R_MICROMIPS_SCN_DISP:
    case elfcpp::R_MICROMIPS_JALR:
    case elfcpp::R_MICROMIPS_HI0_LO16:
    case elfcpp::R_MICROMIPS_TLS_GD:
    case elfcpp::R_MICROMIPS_TLS_LDM:
    case elfcpp::R_MICROMIPS_TLS_DTPREL_HI16:
    case elfcpp::R_MICROMIPS_TLS_DTPREL_LO16:
    case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
    case elfcpp::R_MICROMIPS_TLS_TPREL_HI16:
    case elfcpp::R_MICROMIPS_TLS_TPREL_LO16:
    case elfcpp::R_MICROMIPS_GPREL7_S2:
    case elfcpp::R_MICROMIPS_PC23_S2:
      return true;

    default:
      return false;
    }
}

static inline bool
is_matching_lo16_reloc(unsigned int high_reloc, unsigned int lo16_reloc)
{
  switch (high_reloc)
    {
    case elfcpp::R_MIPS_HI16:
    case elfcpp::R_MIPS_GOT16:
      return lo16_reloc == elfcpp::R_MIPS_LO16;
    case elfcpp::R_MIPS_PCHI16:
      return lo16_reloc == elfcpp::R_MIPS_PCLO16;
    case elfcpp::R_MIPS16_HI16:
    case elfcpp::R_MIPS16_GOT16:
      return lo16_reloc == elfcpp::R_MIPS16_LO16;
    case elfcpp::R_MICROMIPS_HI16:
    case elfcpp::R_MICROMIPS_GOT16:
      return lo16_reloc == elfcpp::R_MICROMIPS_LO16;
    default:
      return false;
    }
}

// This class is used to hold information about one GOT entry.
// There are three types of entry:
//
//    (1) a SYMBOL + OFFSET address, where SYMBOL is local to an input object
//          (object != NULL, symndx >= 0, tls_type != GOT_TLS_LDM)
//    (2) a SYMBOL address, where SYMBOL is not local to an input object
//          (sym != NULL, symndx == -1)
//    (3) a TLS LDM slot (there's only one of these per GOT.)
//          (object != NULL, symndx == 0, tls_type == GOT_TLS_LDM)

template<int size, bool big_endian>
class Mips_got_entry
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;

 public:
  Mips_got_entry(Mips_relobj<size, big_endian>* object, unsigned int symndx,
                 Mips_address addend, unsigned char tls_type,
                 unsigned int shndx, bool is_section_symbol)
    : addend_(addend), symndx_(symndx), tls_type_(tls_type),
      is_section_symbol_(is_section_symbol), shndx_(shndx)
  { this->d.object = object; }

  Mips_got_entry(Mips_symbol<size>* sym, unsigned char tls_type)
    : addend_(0), symndx_(-1U), tls_type_(tls_type),
      is_section_symbol_(false), shndx_(-1U)
  { this->d.sym = sym; }

  // Return whether this entry is for a local symbol.
  bool
  is_for_local_symbol() const
  { return this->symndx_ != -1U; }

  // Return whether this entry is for a global symbol.
  bool
  is_for_global_symbol() const
  { return this->symndx_ == -1U; }

  // Return the hash of this entry.
  size_t
  hash() const
  {
    if (this->tls_type_ == GOT_TLS_LDM)
      return this->symndx_ + (1 << 18);

    size_t name_hash_value = gold::string_hash<char>(
        (this->symndx_ != -1U)
         ? this->d.object->name().c_str()
         : this->d.sym->name());
    size_t addend = this->addend_;
    return name_hash_value ^ this->symndx_ ^ (addend << 16);
  }

  // Return whether this entry is equal to OTHER.
  bool
  equals(Mips_got_entry<size, big_endian>* other) const
  {
    if (this->symndx_ != other->symndx_
        || this->tls_type_ != other->tls_type_)
      return false;

    if (this->tls_type_ == GOT_TLS_LDM)
      return true;

    return (((this->symndx_ != -1U)
              ? (this->d.object == other->d.object)
              : (this->d.sym == other->d.sym))
            && (this->addend_ == other->addend_));
  }

  // Return input object that needs this GOT entry.
  Mips_relobj<size, big_endian>*
  object() const
  {
    gold_assert(this->symndx_ != -1U);
    return this->d.object;
  }

  // Return local symbol index for local GOT entries.
  unsigned int
  symndx() const
  {
    gold_assert(this->symndx_ != -1U);
    return this->symndx_;
  }

  // Return the relocation addend for local GOT entries.
  Mips_address
  addend() const
  { return this->addend_; }

  // Return global symbol for global GOT entries.
  Mips_symbol<size>*
  sym() const
  {
    gold_assert(this->symndx_ == -1U);
    return this->d.sym;
  }

  // Return whether this is a TLS GOT entry.
  bool
  is_tls_entry() const
  { return this->tls_type_ != GOT_TLS_NONE; }

  // Return TLS type of this GOT entry.
  unsigned char
  tls_type() const
  { return this->tls_type_; }

  // Return section index of the local symbol for local GOT entries.
  unsigned int
  shndx() const
  { return this->shndx_; }

  // Return whether this is a STT_SECTION symbol.
  bool
  is_section_symbol() const
  { return this->is_section_symbol_; }

 private:
  // The addend.
  Mips_address addend_;

  // The index of the symbol if we have a local symbol; -1 otherwise.
  unsigned int symndx_;

  union
  {
    // The input object for local symbols that needs the GOT entry.
    Mips_relobj<size, big_endian>* object;
    // If symndx == -1, the global symbol corresponding to this GOT entry.  The
    // symbol's entry is in the local area if mips_sym->global_got_area is
    // GGA_NONE, otherwise it is in the global area.
    Mips_symbol<size>* sym;
  } d;

  // The TLS type of this GOT entry.  An LDM GOT entry will be a local
  // symbol entry with r_symndx == 0.
  unsigned char tls_type_;

  // Whether this is a STT_SECTION symbol.
  bool is_section_symbol_;

  // For local GOT entries, section index of the local symbol.
  unsigned int shndx_;
};

// Hash for Mips_got_entry.

template<int size, bool big_endian>
class Mips_got_entry_hash
{
 public:
  size_t
  operator()(Mips_got_entry<size, big_endian>* entry) const
  { return entry->hash(); }
};

// Equality for Mips_got_entry.

template<int size, bool big_endian>
class Mips_got_entry_eq
{
 public:
  bool
  operator()(Mips_got_entry<size, big_endian>* e1,
             Mips_got_entry<size, big_endian>* e2) const
  { return e1->equals(e2); }
};

// Hash for Mips_symbol.

template<int size>
class Mips_symbol_hash
{
 public:
  size_t
  operator()(Mips_symbol<size>* sym) const
  { return sym->hash(); }
};

// Got_page_range.  This class describes a range of addends: [MIN_ADDEND,
// MAX_ADDEND].  The instances form a non-overlapping list that is sorted by
// increasing MIN_ADDEND.

struct Got_page_range
{
  Got_page_range()
    : next(NULL), min_addend(0), max_addend(0)
  { }

  Got_page_range* next;
  int min_addend;
  int max_addend;

  // Return the maximum number of GOT page entries required.
  int
  get_max_pages()
  { return (this->max_addend - this->min_addend + 0x1ffff) >> 16; }
};

// Got_page_entry.  This class describes the range of addends that are applied
// to page relocations against a given symbol.

struct Got_page_entry
{
  Got_page_entry()
    : object(NULL), symndx(-1U), ranges(NULL)
  { }

  Got_page_entry(Object* object_, unsigned int symndx_)
    : object(object_), symndx(symndx_), ranges(NULL)
  { }

  // The input object that needs the GOT page entry.
  Object* object;
  // The index of the symbol, as stored in the relocation r_info.
  unsigned int symndx;
  // The ranges for this page entry.
  Got_page_range* ranges;
};

// Hash for Got_page_entry.

struct Got_page_entry_hash
{
  size_t
  operator()(Got_page_entry* entry) const
  { return reinterpret_cast<uintptr_t>(entry->object) + entry->symndx; }
};

// Equality for Got_page_entry.

struct Got_page_entry_eq
{
  bool
  operator()(Got_page_entry* entry1, Got_page_entry* entry2) const
  {
    return entry1->object == entry2->object && entry1->symndx == entry2->symndx;
  }
};

// This class is used to hold .got information when linking.

template<int size, bool big_endian>
class Mips_got_info
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;
  typedef Output_data_reloc<elfcpp::SHT_REL, true, size, big_endian>
    Reloc_section;
  typedef Unordered_map<unsigned int, unsigned int> Got_page_offsets;

  // Unordered set of GOT entries.
  typedef Unordered_set<Mips_got_entry<size, big_endian>*,
      Mips_got_entry_hash<size, big_endian>,
      Mips_got_entry_eq<size, big_endian> > Got_entry_set;

  // Unordered set of GOT page entries.
  typedef Unordered_set<Got_page_entry*,
      Got_page_entry_hash, Got_page_entry_eq> Got_page_entry_set;

  // Unordered set of global GOT entries.
  typedef Unordered_set<Mips_symbol<size>*, Mips_symbol_hash<size> >
      Global_got_entry_set;

 public:
  Mips_got_info()
    : local_gotno_(0), page_gotno_(0), global_gotno_(0), reloc_only_gotno_(0),
      tls_gotno_(0), tls_ldm_offset_(-1U), global_got_symbols_(),
      got_entries_(), got_page_entries_(), got_page_offset_start_(0),
      got_page_offset_next_(0), got_page_offsets_(), next_(NULL), index_(-1U),
      offset_(0)
  { }

  // Reserve GOT entry for a GOT relocation of type R_TYPE against symbol
  // SYMNDX + ADDEND, where SYMNDX is a local symbol in section SHNDX in OBJECT.
  void
  record_local_got_symbol(Mips_relobj<size, big_endian>* object,
                          unsigned int symndx, Mips_address addend,
                          unsigned int r_type, unsigned int shndx,
                          bool is_section_symbol);

  // Reserve GOT entry for a GOT relocation of type R_TYPE against MIPS_SYM,
  // in OBJECT.  FOR_CALL is true if the caller is only interested in
  // using the GOT entry for calls.  DYN_RELOC is true if R_TYPE is a dynamic
  // relocation.
  void
  record_global_got_symbol(Mips_symbol<size>* mips_sym,
                           Mips_relobj<size, big_endian>* object,
                           unsigned int r_type, bool dyn_reloc, bool for_call);

  // Add ENTRY to master GOT and to OBJECT's GOT.
  void
  record_got_entry(Mips_got_entry<size, big_endian>* entry,
                   Mips_relobj<size, big_endian>* object);

  // Record that OBJECT has a page relocation against symbol SYMNDX and
  // that ADDEND is the addend for that relocation.
  void
  record_got_page_entry(Mips_relobj<size, big_endian>* object,
                        unsigned int symndx, int addend);

  // Create all entries that should be in the local part of the GOT.
  void
  add_local_entries(Target_mips<size, big_endian>* target, Layout* layout);

  // Create GOT page entries.
  void
  add_page_entries(Target_mips<size, big_endian>* target, Layout* layout);

  // Create global GOT entries, both GGA_NORMAL and GGA_RELOC_ONLY.
  void
  add_global_entries(Target_mips<size, big_endian>* target, Layout* layout,
                     unsigned int non_reloc_only_global_gotno);

  // Create global GOT entries that should be in the GGA_RELOC_ONLY area.
  void
  add_reloc_only_entries(Mips_output_data_got<size, big_endian>* got);

  // Create TLS GOT entries.
  void
  add_tls_entries(Target_mips<size, big_endian>* target, Layout* layout);

  // Decide whether the symbol needs an entry in the global part of the primary
  // GOT, setting global_got_area accordingly.  Count the number of global
  // symbols that are in the primary GOT only because they have dynamic
  // relocations R_MIPS_REL32 against them (reloc_only_gotno).
  void
  count_got_symbols(Symbol_table* symtab);

  // Return the offset of GOT page entry for VALUE.
  unsigned int
  get_got_page_offset(Mips_address value,
                      Mips_output_data_got<size, big_endian>* got);

  // Count the number of GOT entries required.
  void
  count_got_entries();

  // Count the number of GOT entries required by ENTRY.  Accumulate the result.
  void
  count_got_entry(Mips_got_entry<size, big_endian>* entry);

  // Add FROM's GOT entries.
  void
  add_got_entries(Mips_got_info<size, big_endian>* from);

  // Add FROM's GOT page entries.
  void
  add_got_page_count(Mips_got_info<size, big_endian>* from);

  // Return GOT size.
  unsigned int
  got_size() const
  { return ((2 + this->local_gotno_ + this->page_gotno_ + this->global_gotno_
             + this->tls_gotno_) * size/8);
  }

  // Return the number of local GOT entries.
  unsigned int
  local_gotno() const
  { return this->local_gotno_; }

  // Return the maximum number of page GOT entries needed.
  unsigned int
  page_gotno() const
  { return this->page_gotno_; }

  // Return the number of global GOT entries.
  unsigned int
  global_gotno() const
  { return this->global_gotno_; }

  // Set the number of global GOT entries.
  void
  set_global_gotno(unsigned int global_gotno)
  { this->global_gotno_ = global_gotno; }

  // Return the number of GGA_RELOC_ONLY global GOT entries.
  unsigned int
  reloc_only_gotno() const
  { return this->reloc_only_gotno_; }

  // Return the number of TLS GOT entries.
  unsigned int
  tls_gotno() const
  { return this->tls_gotno_; }

  // Return the GOT type for this GOT.  Used for multi-GOT links only.
  unsigned int
  multigot_got_type(unsigned int got_type) const
  {
    switch (got_type)
      {
      case GOT_TYPE_STANDARD:
        return GOT_TYPE_STANDARD_MULTIGOT + this->index_;
      case GOT_TYPE_TLS_OFFSET:
        return GOT_TYPE_TLS_OFFSET_MULTIGOT + this->index_;
      case GOT_TYPE_TLS_PAIR:
        return GOT_TYPE_TLS_PAIR_MULTIGOT + this->index_;
      default:
        gold_unreachable();
      }
  }

  // Remove lazy-binding stubs for global symbols in this GOT.
  void
  remove_lazy_stubs(Target_mips<size, big_endian>* target);

  // Return offset of this GOT from the start of .got section.
  unsigned int
  offset() const
  { return this->offset_; }

  // Set offset of this GOT from the start of .got section.
  void
  set_offset(unsigned int offset)
  { this->offset_ = offset; }

  // Set index of this GOT in multi-GOT links.
  void
  set_index(unsigned int index)
  { this->index_ = index; }

  // Return next GOT in multi-GOT links.
  Mips_got_info<size, big_endian>*
  next() const
  { return this->next_; }

  // Set next GOT in multi-GOT links.
  void
  set_next(Mips_got_info<size, big_endian>* next)
  { this->next_ = next; }

  // Return the offset of TLS LDM entry for this GOT.
  unsigned int
  tls_ldm_offset() const
  { return this->tls_ldm_offset_; }

  // Set the offset of TLS LDM entry for this GOT.
  void
  set_tls_ldm_offset(unsigned int tls_ldm_offset)
  { this->tls_ldm_offset_ = tls_ldm_offset; }

  Global_got_entry_set&
  global_got_symbols()
  { return this->global_got_symbols_; }

  // Return the GOT_TLS_* type required by relocation type R_TYPE.
  static int
  mips_elf_reloc_tls_type(unsigned int r_type)
  {
    if (tls_gd_reloc(r_type))
      return GOT_TLS_GD;

    if (tls_ldm_reloc(r_type))
      return GOT_TLS_LDM;

    if (tls_gottprel_reloc(r_type))
      return GOT_TLS_IE;

    return GOT_TLS_NONE;
  }

  // Return the number of GOT slots needed for GOT TLS type TYPE.
  static int
  mips_tls_got_entries(unsigned int type)
  {
    switch (type)
      {
      case GOT_TLS_GD:
      case GOT_TLS_LDM:
        return 2;

      case GOT_TLS_IE:
        return 1;

      case GOT_TLS_NONE:
        return 0;

      default:
        gold_unreachable();
      }
  }

 private:
  // The number of local GOT entries.
  unsigned int local_gotno_;
  // The maximum number of page GOT entries needed.
  unsigned int page_gotno_;
  // The number of global GOT entries.
  unsigned int global_gotno_;
  // The number of global GOT entries that are in the GGA_RELOC_ONLY area.
  unsigned int reloc_only_gotno_;
  // The number of TLS GOT entries.
  unsigned int tls_gotno_;
  // The offset of TLS LDM entry for this GOT.
  unsigned int tls_ldm_offset_;
  // All symbols that have global GOT entry.
  Global_got_entry_set global_got_symbols_;
  // A hash table holding GOT entries.
  Got_entry_set got_entries_;
  // A hash table of GOT page entries (only used in master GOT).
  Got_page_entry_set got_page_entries_;
  // The offset of first GOT page entry for this GOT.
  unsigned int got_page_offset_start_;
  // The offset of next available GOT page entry for this GOT.
  unsigned int got_page_offset_next_;
  // A hash table that maps GOT page entry value to the GOT offset where
  // the entry is located.
  Got_page_offsets got_page_offsets_;
  // In multi-GOT links, a pointer to the next GOT.
  Mips_got_info<size, big_endian>* next_;
  // Index of this GOT in multi-GOT links.
  unsigned int index_;
  // The offset of this GOT in multi-GOT links.
  unsigned int offset_;
};

// This is a helper class used during relocation scan.  It records GOT16 addend.

template<int size, bool big_endian>
struct got16_addend
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;

  got16_addend(const Sized_relobj_file<size, big_endian>* _object,
               unsigned int _shndx, unsigned int _r_type, unsigned int _r_sym,
               Mips_address _addend)
    : object(_object), shndx(_shndx), r_type(_r_type), r_sym(_r_sym),
      addend(_addend)
  { }

  const Sized_relobj_file<size, big_endian>* object;
  unsigned int shndx;
  unsigned int r_type;
  unsigned int r_sym;
  Mips_address addend;
};

// .MIPS.abiflags section content

template<bool big_endian>
struct Mips_abiflags
{
  typedef typename elfcpp::Swap<8, big_endian>::Valtype Valtype8;
  typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype16;
  typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype32;

  Mips_abiflags()
    : version(0), isa_level(0), isa_rev(0), gpr_size(0), cpr1_size(0),
      cpr2_size(0), fp_abi(0), isa_ext(0), ases(0), flags1(0), flags2(0)
  { }

  // Version of flags structure.
  Valtype16 version;
  // The level of the ISA: 1-5, 32, 64.
  Valtype8 isa_level;
  // The revision of ISA: 0 for MIPS V and below, 1-n otherwise.
  Valtype8 isa_rev;
  // The size of general purpose registers.
  Valtype8 gpr_size;
  // The size of co-processor 1 registers.
  Valtype8 cpr1_size;
  // The size of co-processor 2 registers.
  Valtype8 cpr2_size;
  // The floating-point ABI.
  Valtype8 fp_abi;
  // Processor-specific extension.
  Valtype32 isa_ext;
  // Mask of ASEs used.
  Valtype32 ases;
  // Mask of general flags.
  Valtype32 flags1;
  Valtype32 flags2;
};

// Mips_symbol class.  Holds additional symbol information needed for Mips.

template<int size>
class Mips_symbol : public Sized_symbol<size>
{
 public:
  Mips_symbol()
    : need_fn_stub_(false), has_nonpic_branches_(false), la25_stub_offset_(-1U),
      has_static_relocs_(false), no_lazy_stub_(false), lazy_stub_offset_(0),
      pointer_equality_needed_(false), global_got_area_(GGA_NONE),
      global_gotoffset_(-1U), got_only_for_calls_(true), has_lazy_stub_(false),
      needs_mips_plt_(false), needs_comp_plt_(false), mips_plt_offset_(-1U),
      comp_plt_offset_(-1U), mips16_fn_stub_(NULL), mips16_call_stub_(NULL),
      mips16_call_fp_stub_(NULL), applied_secondary_got_fixup_(false)
  { }

  // Return whether this is a MIPS16 symbol.
  bool
  is_mips16() const
  {
    // (st_other & STO_MIPS16) == STO_MIPS16
    return ((this->nonvis() & (elfcpp::STO_MIPS16 >> 2))
            == elfcpp::STO_MIPS16 >> 2);
  }

  // Return whether this is a microMIPS symbol.
  bool
  is_micromips() const
  {
    // (st_other & STO_MIPS_ISA) == STO_MICROMIPS
    return ((this->nonvis() & (elfcpp::STO_MIPS_ISA >> 2))
            == elfcpp::STO_MICROMIPS >> 2);
  }

  // Return whether the symbol needs MIPS16 fn_stub.
  bool
  need_fn_stub() const
  { return this->need_fn_stub_; }

  // Set that the symbol needs MIPS16 fn_stub.
  void
  set_need_fn_stub()
  { this->need_fn_stub_ = true; }

  // Return whether this symbol is referenced by branch relocations from
  // any non-PIC input file.
  bool
  has_nonpic_branches() const
  { return this->has_nonpic_branches_; }

  // Set that this symbol is referenced by branch relocations from
  // any non-PIC input file.
  void
  set_has_nonpic_branches()
  { this->has_nonpic_branches_ = true; }

  // Return the offset of the la25 stub for this symbol from the start of the
  // la25 stub section.
  unsigned int
  la25_stub_offset() const
  { return this->la25_stub_offset_; }

  // Set the offset of the la25 stub for this symbol from the start of the
  // la25 stub section.
  void
  set_la25_stub_offset(unsigned int offset)
  { this->la25_stub_offset_ = offset; }

  // Return whether the symbol has la25 stub.  This is true if this symbol is
  // for a PIC function, and there are non-PIC branches and jumps to it.
  bool
  has_la25_stub() const
  { return this->la25_stub_offset_ != -1U; }

  // Return whether there is a relocation against this symbol that must be
  // resolved by the static linker (that is, the relocation cannot possibly
  // be made dynamic).
  bool
  has_static_relocs() const
  { return this->has_static_relocs_; }

  // Set that there is a relocation against this symbol that must be resolved
  // by the static linker (that is, the relocation cannot possibly be made
  // dynamic).
  void
  set_has_static_relocs()
  { this->has_static_relocs_ = true; }

  // Return whether we must not create a lazy-binding stub for this symbol.
  bool
  no_lazy_stub() const
  { return this->no_lazy_stub_; }

  // Set that we must not create a lazy-binding stub for this symbol.
  void
  set_no_lazy_stub()
  { this->no_lazy_stub_ = true; }

  // Return the offset of the lazy-binding stub for this symbol from the start
  // of .MIPS.stubs section.
  unsigned int
  lazy_stub_offset() const
  { return this->lazy_stub_offset_; }

  // Set the offset of the lazy-binding stub for this symbol from the start
  // of .MIPS.stubs section.
  void
  set_lazy_stub_offset(unsigned int offset)
  { this->lazy_stub_offset_ = offset; }

  // Return whether there are any relocations for this symbol where
  // pointer equality matters.
  bool
  pointer_equality_needed() const
  { return this->pointer_equality_needed_; }

  // Set that there are relocations for this symbol where pointer equality
  // matters.
  void
  set_pointer_equality_needed()
  { this->pointer_equality_needed_ = true; }

  // Return global GOT area where this symbol in located.
  Global_got_area
  global_got_area() const
  { return this->global_got_area_; }

  // Set global GOT area where this symbol in located.
  void
  set_global_got_area(Global_got_area global_got_area)
  { this->global_got_area_ = global_got_area; }

  // Return the global GOT offset for this symbol.  For multi-GOT links, this
  // returns the offset from the start of .got section to the first GOT entry
  // for the symbol.  Note that in multi-GOT links the symbol can have entry
  // in more than one GOT.
  unsigned int
  global_gotoffset() const
  { return this->global_gotoffset_; }

  // Set the global GOT offset for this symbol.  Note that in multi-GOT links
  // the symbol can have entry in more than one GOT.  This method will set
  // the offset only if it is less than current offset.
  void
  set_global_gotoffset(unsigned int offset)
  {
    if (this->global_gotoffset_ == -1U || offset < this->global_gotoffset_)
      this->global_gotoffset_ = offset;
  }

  // Return whether all GOT relocations for this symbol are for calls.
  bool
  got_only_for_calls() const
  { return this->got_only_for_calls_; }

  // Set that there is a GOT relocation for this symbol that is not for call.
  void
  set_got_not_only_for_calls()
  { this->got_only_for_calls_ = false; }

  // Return whether this is a PIC symbol.
  bool
  is_pic() const
  {
    // (st_other & STO_MIPS_FLAGS) == STO_MIPS_PIC
    return ((this->nonvis() & (elfcpp::STO_MIPS_FLAGS >> 2))
            == (elfcpp::STO_MIPS_PIC >> 2));
  }

  // Set the flag in st_other field that marks this symbol as PIC.
  void
  set_pic()
  {
    if (this->is_mips16())
      // (st_other & ~(STO_MIPS16 | STO_MIPS_FLAGS)) | STO_MIPS_PIC
      this->set_nonvis((this->nonvis()
                        & ~((elfcpp::STO_MIPS16 >> 2)
                            | (elfcpp::STO_MIPS_FLAGS >> 2)))
                       | (elfcpp::STO_MIPS_PIC >> 2));
    else
      // (other & ~STO_MIPS_FLAGS) | STO_MIPS_PIC
      this->set_nonvis((this->nonvis() & ~(elfcpp::STO_MIPS_FLAGS >> 2))
                       | (elfcpp::STO_MIPS_PIC >> 2));
  }

  // Set the flag in st_other field that marks this symbol as PLT.
  void
  set_mips_plt()
  {
    if (this->is_mips16())
      // (st_other & (STO_MIPS16 | ~STO_MIPS_FLAGS)) | STO_MIPS_PLT
      this->set_nonvis((this->nonvis()
                        & ((elfcpp::STO_MIPS16 >> 2)
                           | ~(elfcpp::STO_MIPS_FLAGS >> 2)))
                       | (elfcpp::STO_MIPS_PLT >> 2));

    else
      // (st_other & ~STO_MIPS_FLAGS) | STO_MIPS_PLT
      this->set_nonvis((this->nonvis() & ~(elfcpp::STO_MIPS_FLAGS >> 2))
                       | (elfcpp::STO_MIPS_PLT >> 2));
  }

  // Downcast a base pointer to a Mips_symbol pointer.
  static Mips_symbol<size>*
  as_mips_sym(Symbol* sym)
  { return static_cast<Mips_symbol<size>*>(sym); }

  // Downcast a base pointer to a Mips_symbol pointer.
  static const Mips_symbol<size>*
  as_mips_sym(const Symbol* sym)
  { return static_cast<const Mips_symbol<size>*>(sym); }

  // Return whether the symbol has lazy-binding stub.
  bool
  has_lazy_stub() const
  { return this->has_lazy_stub_; }

  // Set whether the symbol has lazy-binding stub.
  void
  set_has_lazy_stub(bool has_lazy_stub)
  { this->has_lazy_stub_ = has_lazy_stub; }

  // Return whether the symbol needs a standard PLT entry.
  bool
  needs_mips_plt() const
  { return this->needs_mips_plt_; }

  // Set whether the symbol needs a standard PLT entry.
  void
  set_needs_mips_plt(bool needs_mips_plt)
  { this->needs_mips_plt_ = needs_mips_plt; }

  // Return whether the symbol needs a compressed (MIPS16 or microMIPS) PLT
  // entry.
  bool
  needs_comp_plt() const
  { return this->needs_comp_plt_; }

  // Set whether the symbol needs a compressed (MIPS16 or microMIPS) PLT entry.
  void
  set_needs_comp_plt(bool needs_comp_plt)
  { this->needs_comp_plt_ = needs_comp_plt; }

  // Return standard PLT entry offset, or -1 if none.
  unsigned int
  mips_plt_offset() const
  { return this->mips_plt_offset_; }

  // Set standard PLT entry offset.
  void
  set_mips_plt_offset(unsigned int mips_plt_offset)
  { this->mips_plt_offset_ = mips_plt_offset; }

  // Return whether the symbol has standard PLT entry.
  bool
  has_mips_plt_offset() const
  { return this->mips_plt_offset_ != -1U; }

  // Return compressed (MIPS16 or microMIPS) PLT entry offset, or -1 if none.
  unsigned int
  comp_plt_offset() const
  { return this->comp_plt_offset_; }

  // Set compressed (MIPS16 or microMIPS) PLT entry offset.
  void
  set_comp_plt_offset(unsigned int comp_plt_offset)
  { this->comp_plt_offset_ = comp_plt_offset; }

  // Return whether the symbol has compressed (MIPS16 or microMIPS) PLT entry.
  bool
  has_comp_plt_offset() const
  { return this->comp_plt_offset_ != -1U; }

  // Return MIPS16 fn stub for a symbol.
  template<bool big_endian>
  Mips16_stub_section<size, big_endian>*
  get_mips16_fn_stub() const
  {
    return static_cast<Mips16_stub_section<size, big_endian>*>(mips16_fn_stub_);
  }

  // Set MIPS16 fn stub for a symbol.
  void
  set_mips16_fn_stub(Mips16_stub_section_base* stub)
  { this->mips16_fn_stub_ = stub; }

  // Return whether symbol has MIPS16 fn stub.
  bool
  has_mips16_fn_stub() const
  { return this->mips16_fn_stub_ != NULL; }

  // Return MIPS16 call stub for a symbol.
  template<bool big_endian>
  Mips16_stub_section<size, big_endian>*
  get_mips16_call_stub() const
  {
    return static_cast<Mips16_stub_section<size, big_endian>*>(
      mips16_call_stub_);
  }

  // Set MIPS16 call stub for a symbol.
  void
  set_mips16_call_stub(Mips16_stub_section_base* stub)
  { this->mips16_call_stub_ = stub; }

  // Return whether symbol has MIPS16 call stub.
  bool
  has_mips16_call_stub() const
  { return this->mips16_call_stub_ != NULL; }

  // Return MIPS16 call_fp stub for a symbol.
  template<bool big_endian>
  Mips16_stub_section<size, big_endian>*
  get_mips16_call_fp_stub() const
  {
    return static_cast<Mips16_stub_section<size, big_endian>*>(
      mips16_call_fp_stub_);
  }

  // Set MIPS16 call_fp stub for a symbol.
  void
  set_mips16_call_fp_stub(Mips16_stub_section_base* stub)
  { this->mips16_call_fp_stub_ = stub; }

  // Return whether symbol has MIPS16 call_fp stub.
  bool
  has_mips16_call_fp_stub() const
  { return this->mips16_call_fp_stub_ != NULL; }

  bool
  get_applied_secondary_got_fixup() const
  { return applied_secondary_got_fixup_; }

  void
  set_applied_secondary_got_fixup()
  { this->applied_secondary_got_fixup_ = true; }

  // Return the hash of this symbol.
  size_t
  hash() const
  {
    return gold::string_hash<char>(this->name());
  }

 private:
  // Whether the symbol needs MIPS16 fn_stub.  This is true if this symbol
  // appears in any relocs other than a 16 bit call.
  bool need_fn_stub_;

  // True if this symbol is referenced by branch relocations from
  // any non-PIC input file.  This is used to determine whether an
  // la25 stub is required.
  bool has_nonpic_branches_;

  // The offset of the la25 stub for this symbol from the start of the
  // la25 stub section.
  unsigned int la25_stub_offset_;

  // True if there is a relocation against this symbol that must be
  // resolved by the static linker (that is, the relocation cannot
  // possibly be made dynamic).
  bool has_static_relocs_;

  // Whether we must not create a lazy-binding stub for this symbol.
  // This is true if the symbol has relocations related to taking the
  // function's address.
  bool no_lazy_stub_;

  // The offset of the lazy-binding stub for this symbol from the start of
  // .MIPS.stubs section.
  unsigned int lazy_stub_offset_;

  // True if there are any relocations for this symbol where pointer equality
  // matters.
  bool pointer_equality_needed_;

  // Global GOT area where this symbol in located, or GGA_NONE if symbol is not
  // in the global part of the GOT.
  Global_got_area global_got_area_;

  // The global GOT offset for this symbol.  For multi-GOT links, this is offset
  // from the start of .got section to the first GOT entry for the symbol.
  // Note that in multi-GOT links the symbol can have entry in more than one GOT.
  unsigned int global_gotoffset_;

  // Whether all GOT relocations for this symbol are for calls.
  bool got_only_for_calls_;
  // Whether the symbol has lazy-binding stub.
  bool has_lazy_stub_;
  // Whether the symbol needs a standard PLT entry.
  bool needs_mips_plt_;
  // Whether the symbol needs a compressed (MIPS16 or microMIPS) PLT entry.
  bool needs_comp_plt_;
  // Standard PLT entry offset, or -1 if none.
  unsigned int mips_plt_offset_;
  // Compressed (MIPS16 or microMIPS) PLT entry offset, or -1 if none.
  unsigned int comp_plt_offset_;
  // MIPS16 fn stub for a symbol.
  Mips16_stub_section_base* mips16_fn_stub_;
  // MIPS16 call stub for a symbol.
  Mips16_stub_section_base* mips16_call_stub_;
  // MIPS16 call_fp stub for a symbol.
  Mips16_stub_section_base* mips16_call_fp_stub_;

  bool applied_secondary_got_fixup_;
};

// Mips16_stub_section class.

// The mips16 compiler uses a couple of special sections to handle
// floating point arguments.

// Section names that look like .mips16.fn.FNNAME contain stubs that
// copy floating point arguments from the fp regs to the gp regs and
// then jump to FNNAME.  If any 32 bit function calls FNNAME, the
// call should be redirected to the stub instead.  If no 32 bit
// function calls FNNAME, the stub should be discarded.  We need to
// consider any reference to the function, not just a call, because
// if the address of the function is taken we will need the stub,
// since the address might be passed to a 32 bit function.

// Section names that look like .mips16.call.FNNAME contain stubs
// that copy floating point arguments from the gp regs to the fp
// regs and then jump to FNNAME.  If FNNAME is a 32 bit function,
// then any 16 bit function that calls FNNAME should be redirected
// to the stub instead.  If FNNAME is not a 32 bit function, the
// stub should be discarded.

// .mips16.call.fp.FNNAME sections are similar, but contain stubs
// which call FNNAME and then copy the return value from the fp regs
// to the gp regs.  These stubs store the return address in $18 while
// calling FNNAME; any function which might call one of these stubs
// must arrange to save $18 around the call.  (This case is not
// needed for 32 bit functions that call 16 bit functions, because
// 16 bit functions always return floating point values in both
// $f0/$f1 and $2/$3.)

// Note that in all cases FNNAME might be defined statically.
// Therefore, FNNAME is not used literally.  Instead, the relocation
// information will indicate which symbol the section is for.

// We record any stubs that we find in the symbol table.

// TODO(sasa): All mips16 stub sections should be emitted in the .text section.

class Mips16_stub_section_base { };

template<int size, bool big_endian>
class Mips16_stub_section : public Mips16_stub_section_base
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;

 public:
  Mips16_stub_section(Mips_relobj<size, big_endian>* object, unsigned int shndx)
    : object_(object), shndx_(shndx), r_sym_(0), gsym_(NULL),
      found_r_mips_none_(false)
  {
    gold_assert(object->is_mips16_fn_stub_section(shndx)
                || object->is_mips16_call_stub_section(shndx)
                || object->is_mips16_call_fp_stub_section(shndx));
  }

  // Return the object of this stub section.
  Mips_relobj<size, big_endian>*
  object() const
  { return this->object_; }

  // Return the size of a section.
  uint64_t
  section_size() const
  { return this->object_->section_size(this->shndx_); }

  // Return section index of this stub section.
  unsigned int
  shndx() const
  { return this->shndx_; }

  // Return symbol index, if stub is for a local function.
  unsigned int
  r_sym() const
  { return this->r_sym_; }

  // Return symbol, if stub is for a global function.
  Mips_symbol<size>*
  gsym() const
  { return this->gsym_; }

  // Return whether stub is for a local function.
  bool
  is_for_local_function() const
  { return this->gsym_ == NULL; }

  // This method is called when a new relocation R_TYPE for local symbol R_SYM
  // is found in the stub section.  Try to find stub target.
  void
  new_local_reloc_found(unsigned int r_type, unsigned int r_sym)
  {
    // To find target symbol for this stub, trust the first R_MIPS_NONE
    // relocation, if any.  Otherwise trust the first relocation, whatever
    // its kind.
    if (this->found_r_mips_none_)
      return;
    if (r_type == elfcpp::R_MIPS_NONE)
      {
        this->r_sym_ = r_sym;
        this->gsym_ = NULL;
        this->found_r_mips_none_ = true;
      }
    else if (!is_target_found())
      this->r_sym_ = r_sym;
  }

  // This method is called when a new relocation R_TYPE for global symbol GSYM
  // is found in the stub section.  Try to find stub target.
  void
  new_global_reloc_found(unsigned int r_type, Mips_symbol<size>* gsym)
  {
    // To find target symbol for this stub, trust the first R_MIPS_NONE
    // relocation, if any.  Otherwise trust the first relocation, whatever
    // its kind.
    if (this->found_r_mips_none_)
      return;
    if (r_type == elfcpp::R_MIPS_NONE)
      {
        this->gsym_ = gsym;
        this->r_sym_ = 0;
        this->found_r_mips_none_ = true;
      }
    else if (!is_target_found())
      this->gsym_ = gsym;
  }

  // Return whether we found the stub target.
  bool
  is_target_found() const
  { return this->r_sym_ != 0 || this->gsym_ != NULL;  }

  // Return whether this is a fn stub.
  bool
  is_fn_stub() const
  { return this->object_->is_mips16_fn_stub_section(this->shndx_); }

  // Return whether this is a call stub.
  bool
  is_call_stub() const
  { return this->object_->is_mips16_call_stub_section(this->shndx_); }

  // Return whether this is a call_fp stub.
  bool
  is_call_fp_stub() const
  { return this->object_->is_mips16_call_fp_stub_section(this->shndx_); }

  // Return the output address.
  Mips_address
  output_address() const
  {
    return (this->object_->output_section(this->shndx_)->address()
            + this->object_->output_section_offset(this->shndx_));
  }

 private:
  // The object of this stub section.
  Mips_relobj<size, big_endian>* object_;
  // The section index of this stub section.
  unsigned int shndx_;
  // The symbol index, if stub is for a local function.
  unsigned int r_sym_;
  // The symbol, if stub is for a global function.
  Mips_symbol<size>* gsym_;
  // True if we found R_MIPS_NONE relocation in this stub.
  bool found_r_mips_none_;
};

// Mips_relobj class.

template<int size, bool big_endian>
class Mips_relobj : public Sized_relobj_file<size, big_endian>
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;
  typedef std::map<unsigned int, Mips16_stub_section<size, big_endian>*>
    Mips16_stubs_int_map;
  typedef typename elfcpp::Swap<size, big_endian>::Valtype Valtype;

 public:
  Mips_relobj(const std::string& name, Input_file* input_file, off_t offset,
              const typename elfcpp::Ehdr<size, big_endian>& ehdr)
    : Sized_relobj_file<size, big_endian>(name, input_file, offset, ehdr),
      processor_specific_flags_(0), local_symbol_is_mips16_(),
      local_symbol_is_micromips_(), mips16_stub_sections_(),
      local_non_16bit_calls_(), local_16bit_calls_(), local_mips16_fn_stubs_(),
      local_mips16_call_stubs_(), gp_(0), has_reginfo_section_(false),
      merge_processor_specific_data_(true), got_info_(NULL),
      section_is_mips16_fn_stub_(), section_is_mips16_call_stub_(),
      section_is_mips16_call_fp_stub_(), pdr_shndx_(-1U),
      attributes_section_data_(NULL), abiflags_(NULL), gprmask_(0),
      cprmask1_(0), cprmask2_(0), cprmask3_(0), cprmask4_(0)
  {
    this->is_pic_ = (ehdr.get_e_flags() & elfcpp::EF_MIPS_PIC) != 0;
    this->is_n32_ = elfcpp::abi_n32(ehdr.get_e_flags());
  }

  ~Mips_relobj()
  { delete this->attributes_section_data_; }

  // Downcast a base pointer to a Mips_relobj pointer.  This is
  // not type-safe but we only use Mips_relobj not the base class.
  static Mips_relobj<size, big_endian>*
  as_mips_relobj(Relobj* relobj)
  { return static_cast<Mips_relobj<size, big_endian>*>(relobj); }

  // Downcast a base pointer to a Mips_relobj pointer.  This is
  // not type-safe but we only use Mips_relobj not the base class.
  static const Mips_relobj<size, big_endian>*
  as_mips_relobj(const Relobj* relobj)
  { return static_cast<const Mips_relobj<size, big_endian>*>(relobj); }

  // Processor-specific flags in ELF file header.  This is valid only after
  // reading symbols.
  elfcpp::Elf_Word
  processor_specific_flags() const
  { return this->processor_specific_flags_; }

  // Whether a local symbol is MIPS16 symbol.  R_SYM is the symbol table
  // index.  This is only valid after do_count_local_symbol is called.
  bool
  local_symbol_is_mips16(unsigned int r_sym) const
  {
    gold_assert(r_sym < this->local_symbol_is_mips16_.size());
    return this->local_symbol_is_mips16_[r_sym];
  }

  // Whether a local symbol is microMIPS symbol.  R_SYM is the symbol table
  // index.  This is only valid after do_count_local_symbol is called.
  bool
  local_symbol_is_micromips(unsigned int r_sym) const
  {
    gold_assert(r_sym < this->local_symbol_is_micromips_.size());
    return this->local_symbol_is_micromips_[r_sym];
  }

  // Get or create MIPS16 stub section.
  Mips16_stub_section<size, big_endian>*
  get_mips16_stub_section(unsigned int shndx)
  {
    typename Mips16_stubs_int_map::const_iterator it =
      this->mips16_stub_sections_.find(shndx);
    if (it != this->mips16_stub_sections_.end())
      return (*it).second;

    Mips16_stub_section<size, big_endian>* stub_section =
      new Mips16_stub_section<size, big_endian>(this, shndx);
    this->mips16_stub_sections_.insert(
      std::pair<unsigned int, Mips16_stub_section<size, big_endian>*>(
        stub_section->shndx(), stub_section));
    return stub_section;
  }

  // Return MIPS16 fn stub section for local symbol R_SYM, or NULL if this
  // object doesn't have fn stub for R_SYM.
  Mips16_stub_section<size, big_endian>*
  get_local_mips16_fn_stub(unsigned int r_sym) const
  {
    typename Mips16_stubs_int_map::const_iterator it =
      this->local_mips16_fn_stubs_.find(r_sym);
    if (it != this->local_mips16_fn_stubs_.end())
      return (*it).second;
    return NULL;
  }

  // Record that this object has MIPS16 fn stub for local symbol.  This method
  // is only called if we decided not to discard the stub.
  void
  add_local_mips16_fn_stub(Mips16_stub_section<size, big_endian>* stub)
  {
    gold_assert(stub->is_for_local_function());
    unsigned int r_sym = stub->r_sym();
    this->local_mips16_fn_stubs_.insert(
      std::pair<unsigned int, Mips16_stub_section<size, big_endian>*>(
        r_sym, stub));
  }

  // Return MIPS16 call stub section for local symbol R_SYM, or NULL if this
  // object doesn't have call stub for R_SYM.
  Mips16_stub_section<size, big_endian>*
  get_local_mips16_call_stub(unsigned int r_sym) const
  {
    typename Mips16_stubs_int_map::const_iterator it =
      this->local_mips16_call_stubs_.find(r_sym);
    if (it != this->local_mips16_call_stubs_.end())
      return (*it).second;
    return NULL;
  }

  // Record that this object has MIPS16 call stub for local symbol.  This method
  // is only called if we decided not to discard the stub.
  void
  add_local_mips16_call_stub(Mips16_stub_section<size, big_endian>* stub)
  {
    gold_assert(stub->is_for_local_function());
    unsigned int r_sym = stub->r_sym();
    this->local_mips16_call_stubs_.insert(
      std::pair<unsigned int, Mips16_stub_section<size, big_endian>*>(
        r_sym, stub));
  }

  // Record that we found "non 16-bit" call relocation against local symbol
  // SYMNDX.  This reloc would need to refer to a MIPS16 fn stub, if there
  // is one.
  void
  add_local_non_16bit_call(unsigned int symndx)
  { this->local_non_16bit_calls_.insert(symndx); }

  // Return true if there is any "non 16-bit" call relocation against local
  // symbol SYMNDX in this object.
  bool
  has_local_non_16bit_call_relocs(unsigned int symndx)
  {
    return (this->local_non_16bit_calls_.find(symndx)
            != this->local_non_16bit_calls_.end());
  }

  // Record that we found 16-bit call relocation R_MIPS16_26 against local
  // symbol SYMNDX.  Local MIPS16 call or call_fp stubs will only be needed
  // if there is some R_MIPS16_26 relocation that refers to the stub symbol.
  void
  add_local_16bit_call(unsigned int symndx)
  { this->local_16bit_calls_.insert(symndx); }

  // Return true if there is any 16-bit call relocation R_MIPS16_26 against local
  // symbol SYMNDX in this object.
  bool
  has_local_16bit_call_relocs(unsigned int symndx)
  {
    return (this->local_16bit_calls_.find(symndx)
            != this->local_16bit_calls_.end());
  }

  // Get gp value that was used to create this object.
  Mips_address
  gp_value() const
  { return this->gp_; }

  // Return whether the object is a PIC object.
  bool
  is_pic() const
  { return this->is_pic_; }

  // Return whether the object uses N32 ABI.
  bool
  is_n32() const
  { return this->is_n32_; }

  // Return whether the object uses N64 ABI.
  bool
  is_n64() const
  { return size == 64; }

  // Return whether the object uses NewABI conventions.
  bool
  is_newabi() const
  { return this->is_n32() || this->is_n64(); }

  // Return Mips_got_info for this object.
  Mips_got_info<size, big_endian>*
  get_got_info() const
  { return this->got_info_; }

  // Return Mips_got_info for this object.  Create new info if it doesn't exist.
  Mips_got_info<size, big_endian>*
  get_or_create_got_info()
  {
    if (!this->got_info_)
      this->got_info_ = new Mips_got_info<size, big_endian>();
    return this->got_info_;
  }

  // Set Mips_got_info for this object.
  void
  set_got_info(Mips_got_info<size, big_endian>* got_info)
  { this->got_info_ = got_info; }

  // Whether a section SHDNX is a MIPS16 stub section.  This is only valid
  // after do_read_symbols is called.
  bool
  is_mips16_stub_section(unsigned int shndx)
  {
    return (is_mips16_fn_stub_section(shndx)
            || is_mips16_call_stub_section(shndx)
            || is_mips16_call_fp_stub_section(shndx));
  }

  // Return TRUE if relocations in section SHNDX can refer directly to a
  // MIPS16 function rather than to a hard-float stub.  This is only valid
  // after do_read_symbols is called.
  bool
  section_allows_mips16_refs(unsigned int shndx)
  {
    return (this->is_mips16_stub_section(shndx) || shndx == this->pdr_shndx_);
  }

  // Whether a section SHDNX is a MIPS16 fn stub section.  This is only valid
  // after do_read_symbols is called.
  bool
  is_mips16_fn_stub_section(unsigned int shndx)
  {
    gold_assert(shndx < this->section_is_mips16_fn_stub_.size());
    return this->section_is_mips16_fn_stub_[shndx];
  }

  // Whether a section SHDNX is a MIPS16 call stub section.  This is only valid
  // after do_read_symbols is called.
  bool
  is_mips16_call_stub_section(unsigned int shndx)
  {
    gold_assert(shndx < this->section_is_mips16_call_stub_.size());
    return this->section_is_mips16_call_stub_[shndx];
  }

  // Whether a section SHDNX is a MIPS16 call_fp stub section.  This is only
  // valid after do_read_symbols is called.
  bool
  is_mips16_call_fp_stub_section(unsigned int shndx)
  {
    gold_assert(shndx < this->section_is_mips16_call_fp_stub_.size());
    return this->section_is_mips16_call_fp_stub_[shndx];
  }

  // Discard MIPS16 stub secions that are not needed.
  void
  discard_mips16_stub_sections(Symbol_table* symtab);

  // Return whether there is a .reginfo section.
  bool
  has_reginfo_section() const
  { return this->has_reginfo_section_; }

  // Return whether we want to merge processor-specific data.
  bool
  merge_processor_specific_data() const
  { return this->merge_processor_specific_data_; }

  // Return gprmask from the .reginfo section of this object.
  Valtype
  gprmask() const
  { return this->gprmask_; }

  // Return cprmask1 from the .reginfo section of this object.
  Valtype
  cprmask1() const
  { return this->cprmask1_; }

  // Return cprmask2 from the .reginfo section of this object.
  Valtype
  cprmask2() const
  { return this->cprmask2_; }

  // Return cprmask3 from the .reginfo section of this object.
  Valtype
  cprmask3() const
  { return this->cprmask3_; }

  // Return cprmask4 from the .reginfo section of this object.
  Valtype
  cprmask4() const
  { return this->cprmask4_; }

  // This is the contents of the .MIPS.abiflags section if there is one.
  Mips_abiflags<big_endian>*
  abiflags()
  { return this->abiflags_; }

  // This is the contents of the .gnu.attribute section if there is one.
  const Attributes_section_data*
  attributes_section_data() const
  { return this->attributes_section_data_; }

 protected:
  // Count the local symbols.
  void
  do_count_local_symbols(Stringpool_template<char>*,
                         Stringpool_template<char>*);

  // Read the symbol information.
  void
  do_read_symbols(Read_symbols_data* sd);

 private:
  // The name of the options section.
  const char* mips_elf_options_section_name()
  { return this->is_newabi() ? ".MIPS.options" : ".options"; }

  // processor-specific flags in ELF file header.
  elfcpp::Elf_Word processor_specific_flags_;

  // Bit vector to tell if a local symbol is a MIPS16 symbol or not.
  // This is only valid after do_count_local_symbol is called.
  std::vector<bool> local_symbol_is_mips16_;

  // Bit vector to tell if a local symbol is a microMIPS symbol or not.
  // This is only valid after do_count_local_symbol is called.
  std::vector<bool> local_symbol_is_micromips_;

  // Map from section index to the MIPS16 stub for that section.  This contains
  // all stubs found in this object.
  Mips16_stubs_int_map mips16_stub_sections_;

  // Local symbols that have "non 16-bit" call relocation.  This relocation
  // would need to refer to a MIPS16 fn stub, if there is one.
  std::set<unsigned int> local_non_16bit_calls_;

  // Local symbols that have 16-bit call relocation R_MIPS16_26.  Local MIPS16
  // call or call_fp stubs will only be needed if there is some R_MIPS16_26
  // relocation that refers to the stub symbol.
  std::set<unsigned int> local_16bit_calls_;

  // Map from local symbol index to the MIPS16 fn stub for that symbol.
  // This contains only the stubs that we decided not to discard.
  Mips16_stubs_int_map local_mips16_fn_stubs_;

  // Map from local symbol index to the MIPS16 call stub for that symbol.
  // This contains only the stubs that we decided not to discard.
  Mips16_stubs_int_map local_mips16_call_stubs_;

  // gp value that was used to create this object.
  Mips_address gp_;
  // Whether the object is a PIC object.
  bool is_pic_ : 1;
  // Whether the object uses N32 ABI.
  bool is_n32_ : 1;
  // Whether the object contains a .reginfo section.
  bool has_reginfo_section_ : 1;
  // Whether we merge processor-specific data of this object to output.
  bool merge_processor_specific_data_ : 1;
  // The Mips_got_info for this object.
  Mips_got_info<size, big_endian>* got_info_;

  // Bit vector to tell if a section is a MIPS16 fn stub section or not.
  // This is only valid after do_read_symbols is called.
  std::vector<bool> section_is_mips16_fn_stub_;

  // Bit vector to tell if a section is a MIPS16 call stub section or not.
  // This is only valid after do_read_symbols is called.
  std::vector<bool> section_is_mips16_call_stub_;

  // Bit vector to tell if a section is a MIPS16 call_fp stub section or not.
  // This is only valid after do_read_symbols is called.
  std::vector<bool> section_is_mips16_call_fp_stub_;

  // .pdr section index.
  unsigned int pdr_shndx_;

  // Object attributes if there is a .gnu.attributes section or NULL.
  Attributes_section_data* attributes_section_data_;

  // Object abiflags if there is a .MIPS.abiflags section or NULL.
  Mips_abiflags<big_endian>* abiflags_;

  // gprmask from the .reginfo section of this object.
  Valtype gprmask_;
  // cprmask1 from the .reginfo section of this object.
  Valtype cprmask1_;
  // cprmask2 from the .reginfo section of this object.
  Valtype cprmask2_;
  // cprmask3 from the .reginfo section of this object.
  Valtype cprmask3_;
  // cprmask4 from the .reginfo section of this object.
  Valtype cprmask4_;
};

// Mips_output_data_got class.

template<int size, bool big_endian>
class Mips_output_data_got : public Output_data_got<size, big_endian>
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;
  typedef Output_data_reloc<elfcpp::SHT_REL, true, size, big_endian>
    Reloc_section;
  typedef typename elfcpp::Swap<size, big_endian>::Valtype Valtype;

 public:
  Mips_output_data_got(Target_mips<size, big_endian>* target,
      Symbol_table* symtab, Layout* layout)
    : Output_data_got<size, big_endian>(), target_(target),
      symbol_table_(symtab), layout_(layout), static_relocs_(), got_view_(NULL),
      first_global_got_dynsym_index_(-1U), primary_got_(NULL),
      secondary_got_relocs_()
  {
    this->master_got_info_ = new Mips_got_info<size, big_endian>();
    this->set_addralign(16);
  }

  // Reserve GOT entry for a GOT relocation of type R_TYPE against symbol
  // SYMNDX + ADDEND, where SYMNDX is a local symbol in section SHNDX in OBJECT.
  void
  record_local_got_symbol(Mips_relobj<size, big_endian>* object,
                          unsigned int symndx, Mips_address addend,
                          unsigned int r_type, unsigned int shndx,
                          bool is_section_symbol)
  {
    this->master_got_info_->record_local_got_symbol(object, symndx, addend,
                                                    r_type, shndx,
                                                    is_section_symbol);
  }

  // Reserve GOT entry for a GOT relocation of type R_TYPE against MIPS_SYM,
  // in OBJECT.  FOR_CALL is true if the caller is only interested in
  // using the GOT entry for calls.  DYN_RELOC is true if R_TYPE is a dynamic
  // relocation.
  void
  record_global_got_symbol(Mips_symbol<size>* mips_sym,
                           Mips_relobj<size, big_endian>* object,
                           unsigned int r_type, bool dyn_reloc, bool for_call)
  {
    this->master_got_info_->record_global_got_symbol(mips_sym, object, r_type,
                                                     dyn_reloc, for_call);
  }

  // Record that OBJECT has a page relocation against symbol SYMNDX and
  // that ADDEND is the addend for that relocation.
  void
  record_got_page_entry(Mips_relobj<size, big_endian>* object,
                        unsigned int symndx, int addend)
  { this->master_got_info_->record_got_page_entry(object, symndx, addend); }

  // Add a static entry for the GOT entry at OFFSET.  GSYM is a global
  // symbol and R_TYPE is the code of a dynamic relocation that needs to be
  // applied in a static link.
  void
  add_static_reloc(unsigned int got_offset, unsigned int r_type,
                   Mips_symbol<size>* gsym)
  { this->static_relocs_.push_back(Static_reloc(got_offset, r_type, gsym)); }

  // Add a static reloc for the GOT entry at OFFSET.  RELOBJ is an object
  // defining a local symbol with INDEX.  R_TYPE is the code of a dynamic
  // relocation that needs to be applied in a static link.
  void
  add_static_reloc(unsigned int got_offset, unsigned int r_type,
                   Sized_relobj_file<size, big_endian>* relobj,
                   unsigned int index)
  {
    this->static_relocs_.push_back(Static_reloc(got_offset, r_type, relobj,
                                                index));
  }

  // Record that global symbol GSYM has R_TYPE dynamic relocation in the
  // secondary GOT at OFFSET.
  void
  add_secondary_got_reloc(unsigned int got_offset, unsigned int r_type,
                          Mips_symbol<size>* gsym)
  {
    this->secondary_got_relocs_.push_back(Static_reloc(got_offset,
                                                       r_type, gsym));
  }

  // Update GOT entry at OFFSET with VALUE.
  void
  update_got_entry(unsigned int offset, Mips_address value)
  {
    elfcpp::Swap<size, big_endian>::writeval(this->got_view_ + offset, value);
  }

  // Return the number of entries in local part of the GOT.  This includes
  // local entries, page entries and 2 reserved entries.
  unsigned int
  get_local_gotno() const
  {
    if (!this->multi_got())
      {
        return (2 + this->master_got_info_->local_gotno()
                + this->master_got_info_->page_gotno());
      }
    else
      return 2 + this->primary_got_->local_gotno() + this->primary_got_->page_gotno();
  }

  // Return dynamic symbol table index of the first symbol with global GOT
  // entry.
  unsigned int
  first_global_got_dynsym_index() const
  { return this->first_global_got_dynsym_index_; }

  // Set dynamic symbol table index of the first symbol with global GOT entry.
  void
  set_first_global_got_dynsym_index(unsigned int index)
  { this->first_global_got_dynsym_index_ = index; }

  // Lay out the GOT.  Add local, global and TLS entries.  If GOT is
  // larger than 64K, create multi-GOT.
  void
  lay_out_got(Layout* layout, Symbol_table* symtab,
              const Input_objects* input_objects);

  // Create multi-GOT.  For every GOT, add local, global and TLS entries.
  void
  lay_out_multi_got(Layout* layout, const Input_objects* input_objects);

  // Attempt to merge GOTs of different input objects.
  void
  merge_gots(const Input_objects* input_objects);

  // Consider merging FROM, which is OBJECT's GOT, into TO.  Return false if
  // this would lead to overflow, true if they were merged successfully.
  bool
  merge_got_with(Mips_got_info<size, big_endian>* from,
                 Mips_relobj<size, big_endian>* object,
                 Mips_got_info<size, big_endian>* to);

  // Return the offset of GOT page entry for VALUE.  For multi-GOT links,
  // use OBJECT's GOT.
  unsigned int
  get_got_page_offset(Mips_address value,
                      const Mips_relobj<size, big_endian>* object)
  {
    Mips_got_info<size, big_endian>* g = (!this->multi_got()
                                          ? this->master_got_info_
                                          : object->get_got_info());
    gold_assert(g != NULL);
    return g->get_got_page_offset(value, this);
  }

  // Return the GOT offset of type GOT_TYPE of the global symbol
  // GSYM.  For multi-GOT links, use OBJECT's GOT.
  unsigned int got_offset(const Symbol* gsym, unsigned int got_type,
                          Mips_relobj<size, big_endian>* object) const
  {
    if (!this->multi_got())
      return gsym->got_offset(got_type);
    else
      {
        Mips_got_info<size, big_endian>* g = object->get_got_info();
        gold_assert(g != NULL);
        return gsym->got_offset(g->multigot_got_type(got_type));
      }
  }

  // Return the GOT offset of type GOT_TYPE of the local symbol
  // SYMNDX.
  unsigned int
  got_offset(unsigned int symndx, unsigned int got_type,
             Sized_relobj_file<size, big_endian>* object,
             uint64_t addend) const
  { return object->local_got_offset(symndx, got_type, addend); }

  // Return the offset of TLS LDM entry.  For multi-GOT links, use OBJECT's GOT.
  unsigned int
  tls_ldm_offset(Mips_relobj<size, big_endian>* object) const
  {
    Mips_got_info<size, big_endian>* g = (!this->multi_got()
                                          ? this->master_got_info_
                                          : object->get_got_info());
    gold_assert(g != NULL);
    return g->tls_ldm_offset();
  }

  // Set the offset of TLS LDM entry.  For multi-GOT links, use OBJECT's GOT.
  void
  set_tls_ldm_offset(unsigned int tls_ldm_offset,
                     Mips_relobj<size, big_endian>* object)
  {
    Mips_got_info<size, big_endian>* g = (!this->multi_got()
                                          ? this->master_got_info_
                                          : object->get_got_info());
    gold_assert(g != NULL);
    g->set_tls_ldm_offset(tls_ldm_offset);
  }

  // Return true for multi-GOT links.
  bool
  multi_got() const
  { return this->primary_got_ != NULL; }

  // Return the offset of OBJECT's GOT from the start of .got section.
  unsigned int
  get_got_offset(const Mips_relobj<size, big_endian>* object)
  {
    if (!this->multi_got())
      return 0;
    else
      {
        Mips_got_info<size, big_endian>* g = object->get_got_info();
        return g != NULL ? g->offset() : 0;
      }
  }

  // Create global GOT entries that should be in the GGA_RELOC_ONLY area.
  void
  add_reloc_only_entries()
  { this->master_got_info_->add_reloc_only_entries(this); }

  // Return offset of the primary GOT's entry for global symbol.
  unsigned int
  get_primary_got_offset(const Mips_symbol<size>* sym) const
  {
    gold_assert(sym->global_got_area() != GGA_NONE);
    return (this->get_local_gotno() + sym->dynsym_index()
            - this->first_global_got_dynsym_index()) * size/8;
  }

  // For the entry at offset GOT_OFFSET, return its offset from the gp.
  // Input argument GOT_OFFSET is always global offset from the start of
  // .got section, for both single and multi-GOT links.
  // For single GOT links, this returns GOT_OFFSET - 0x7FF0.  For multi-GOT
  // links, the return value is object_got_offset - 0x7FF0, where
  // object_got_offset is offset in the OBJECT's GOT.
  int
  gp_offset(unsigned int got_offset,
            const Mips_relobj<size, big_endian>* object) const
  {
    return (this->address() + got_offset
            - this->target_->adjusted_gp_value(object));
  }

 protected:
  // Write out the GOT table.
  void
  do_write(Output_file*);

 private:

  // This class represent dynamic relocations that need to be applied by
  // gold because we are using TLS relocations in a static link.
  class Static_reloc
  {
   public:
    Static_reloc(unsigned int got_offset, unsigned int r_type,
                 Mips_symbol<size>* gsym)
      : got_offset_(got_offset), r_type_(r_type), symbol_is_global_(true)
    { this->u_.global.symbol = gsym; }

    Static_reloc(unsigned int got_offset, unsigned int r_type,
          Sized_relobj_file<size, big_endian>* relobj, unsigned int index)
      : got_offset_(got_offset), r_type_(r_type), symbol_is_global_(false)
    {
      this->u_.local.relobj = relobj;
      this->u_.local.index = index;
    }

    // Return the GOT offset.
    unsigned int
    got_offset() const
    { return this->got_offset_; }

    // Relocation type.
    unsigned int
    r_type() const
    { return this->r_type_; }

    // Whether the symbol is global or not.
    bool
    symbol_is_global() const
    { return this->symbol_is_global_; }

    // For a relocation against a global symbol, the global symbol.
    Mips_symbol<size>*
    symbol() const
    {
      gold_assert(this->symbol_is_global_);
      return this->u_.global.symbol;
    }

    // For a relocation against a local symbol, the defining object.
    Sized_relobj_file<size, big_endian>*
    relobj() const
    {
      gold_assert(!this->symbol_is_global_);
      return this->u_.local.relobj;
    }

    // For a relocation against a local symbol, the local symbol index.
    unsigned int
    index() const
    {
      gold_assert(!this->symbol_is_global_);
      return this->u_.local.index;
    }

   private:
    // GOT offset of the entry to which this relocation is applied.
    unsigned int got_offset_;
    // Type of relocation.
    unsigned int r_type_;
    // Whether this relocation is against a global symbol.
    bool symbol_is_global_;
    // A global or local symbol.
    union
    {
      struct
      {
        // For a global symbol, the symbol itself.
        Mips_symbol<size>* symbol;
      } global;
      struct
      {
        // For a local symbol, the object defining object.
        Sized_relobj_file<size, big_endian>* relobj;
        // For a local symbol, the symbol index.
        unsigned int index;
      } local;
    } u_;
  };

  // The target.
  Target_mips<size, big_endian>* target_;
  // The symbol table.
  Symbol_table* symbol_table_;
  // The layout.
  Layout* layout_;
  // Static relocs to be applied to the GOT.
  std::vector<Static_reloc> static_relocs_;
  // .got section view.
  unsigned char* got_view_;
  // The dynamic symbol table index of the first symbol with global GOT entry.
  unsigned int first_global_got_dynsym_index_;
  // The master GOT information.
  Mips_got_info<size, big_endian>* master_got_info_;
  // The  primary GOT information.
  Mips_got_info<size, big_endian>* primary_got_;
  // Secondary GOT fixups.
  std::vector<Static_reloc> secondary_got_relocs_;
};

// A class to handle LA25 stubs - non-PIC interface to a PIC function. There are
// two ways of creating these interfaces.  The first is to add:
//
//      lui     $25,%hi(func)
//      j       func
//      addiu   $25,$25,%lo(func)
//
// to a separate trampoline section.  The second is to add:
//
//      lui     $25,%hi(func)
//      addiu   $25,$25,%lo(func)
//
// immediately before a PIC function "func", but only if a function is at the
// beginning of the section, and the section is not too heavily aligned (i.e we
// would need to add no more than 2 nops before the stub.)
//
// We only create stubs of the first type.

template<int size, bool big_endian>
class Mips_output_data_la25_stub : public Output_section_data
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;

 public:
  Mips_output_data_la25_stub()
  : Output_section_data(size == 32 ? 4 : 8), symbols_()
  { }

  // Create LA25 stub for a symbol.
  void
  create_la25_stub(Symbol_table* symtab, Target_mips<size, big_endian>* target,
                   Mips_symbol<size>* gsym);

  // Return output address of a stub.
  Mips_address
  stub_address(const Mips_symbol<size>* sym) const
  {
    gold_assert(sym->has_la25_stub());
    return this->address() + sym->la25_stub_offset();
  }

 protected:
  void
  do_adjust_output_section(Output_section* os)
  { os->set_entsize(0); }

 private:
  // Template for standard LA25 stub.
  static const uint32_t la25_stub_entry[];
  // Template for microMIPS LA25 stub.
  static const uint32_t la25_stub_micromips_entry[];

  // Set the final size.
  void
  set_final_data_size()
  { this->set_data_size(this->symbols_.size() * 16); }

  // Create a symbol for SYM stub's value and size, to help make the
  // disassembly easier to read.
  void
  create_stub_symbol(Mips_symbol<size>* sym, Symbol_table* symtab,
                     Target_mips<size, big_endian>* target, uint64_t symsize);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _(".LA25.stubs")); }

  // Write out the LA25 stub section.
  void
  do_write(Output_file*);

  // Symbols that have LA25 stubs.
  std::vector<Mips_symbol<size>*> symbols_;
};

// MIPS-specific relocation writer.

template<int sh_type, bool dynamic, int size, bool big_endian>
struct Mips_output_reloc_writer;

template<int sh_type, bool dynamic, bool big_endian>
struct Mips_output_reloc_writer<sh_type, dynamic, 32, big_endian>
{
  typedef Output_reloc<sh_type, dynamic, 32, big_endian> Output_reloc_type;
  typedef std::vector<Output_reloc_type> Relocs;

  static void
  write(typename Relocs::const_iterator p, unsigned char* pov)
  { p->write(pov); }
};

template<int sh_type, bool dynamic, bool big_endian>
struct Mips_output_reloc_writer<sh_type, dynamic, 64, big_endian>
{
  typedef Output_reloc<sh_type, dynamic, 64, big_endian> Output_reloc_type;
  typedef std::vector<Output_reloc_type> Relocs;

  static void
  write(typename Relocs::const_iterator p, unsigned char* pov)
  {
    elfcpp::Mips64_rel_write<big_endian> orel(pov);
    orel.put_r_offset(p->get_address());
    orel.put_r_sym(p->get_symbol_index());
    orel.put_r_ssym(RSS_UNDEF);
    orel.put_r_type(p->type());
    if (p->type() == elfcpp::R_MIPS_REL32)
      orel.put_r_type2(elfcpp::R_MIPS_64);
    else
      orel.put_r_type2(elfcpp::R_MIPS_NONE);
    orel.put_r_type3(elfcpp::R_MIPS_NONE);
  }
};

template<int sh_type, bool dynamic, int size, bool big_endian>
class Mips_output_data_reloc : public Output_data_reloc<sh_type, dynamic,
                                                        size, big_endian>
{
 public:
  Mips_output_data_reloc(bool sort_relocs)
    : Output_data_reloc<sh_type, dynamic, size, big_endian>(sort_relocs)
  { }

 protected:
  // Write out the data.
  void
  do_write(Output_file* of)
  {
    typedef Mips_output_reloc_writer<sh_type, dynamic, size,
        big_endian> Writer;
    this->template do_write_generic<Writer>(of);
  }
};


// A class to handle the PLT data.

template<int size, bool big_endian>
class Mips_output_data_plt : public Output_section_data
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;
  typedef Mips_output_data_reloc<elfcpp::SHT_REL, true,
                                 size, big_endian> Reloc_section;

 public:
  // Create the PLT section.  The ordinary .got section is an argument,
  // since we need to refer to the start.
  Mips_output_data_plt(Layout* layout, Output_data_space* got_plt,
                       Target_mips<size, big_endian>* target)
    : Output_section_data(size == 32 ? 4 : 8), got_plt_(got_plt), symbols_(),
      plt_mips_offset_(0), plt_comp_offset_(0), plt_header_size_(0),
      target_(target)
  {
    this->rel_ = new Reloc_section(false);
    layout->add_output_section_data(".rel.plt", elfcpp::SHT_REL,
                                    elfcpp::SHF_ALLOC, this->rel_,
                                    ORDER_DYNAMIC_PLT_RELOCS, false);
  }

  // Add an entry to the PLT for a symbol referenced by r_type relocation.
  void
  add_entry(Mips_symbol<size>* gsym, unsigned int r_type);

  // Return the .rel.plt section data.
  Reloc_section*
  rel_plt() const
  { return this->rel_; }

  // Return the number of PLT entries.
  unsigned int
  entry_count() const
  { return this->symbols_.size(); }

  // Return the offset of the first non-reserved PLT entry.
  unsigned int
  first_plt_entry_offset() const
  { return sizeof(plt0_entry_o32); }

  // Return the size of a PLT entry.
  unsigned int
  plt_entry_size() const
  { return sizeof(plt_entry); }

  // Set final PLT offsets.  For each symbol, determine whether standard or
  // compressed (MIPS16 or microMIPS) PLT entry is used.
  void
  set_plt_offsets();

  // Return the offset of the first standard PLT entry.
  unsigned int
  first_mips_plt_offset() const
  { return this->plt_header_size_; }

  // Return the offset of the first compressed PLT entry.
  unsigned int
  first_comp_plt_offset() const
  { return this->plt_header_size_ + this->plt_mips_offset_; }

  // Return whether there are any standard PLT entries.
  bool
  has_standard_entries() const
  { return this->plt_mips_offset_ > 0; }

  // Return the output address of standard PLT entry.
  Mips_address
  mips_entry_address(const Mips_symbol<size>* sym) const
  {
    gold_assert (sym->has_mips_plt_offset());
    return (this->address() + this->first_mips_plt_offset()
            + sym->mips_plt_offset());
  }

  // Return the output address of compressed (MIPS16 or microMIPS) PLT entry.
  Mips_address
  comp_entry_address(const Mips_symbol<size>* sym) const
  {
    gold_assert (sym->has_comp_plt_offset());
    return (this->address() + this->first_comp_plt_offset()
            + sym->comp_plt_offset());
  }

 protected:
  void
  do_adjust_output_section(Output_section* os)
  { os->set_entsize(0); }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _(".plt")); }

 private:
  // Template for the first PLT entry.
  static const uint32_t plt0_entry_o32[];
  static const uint32_t plt0_entry_n32[];
  static const uint32_t plt0_entry_n64[];
  static const uint32_t plt0_entry_micromips_o32[];
  static const uint32_t plt0_entry_micromips32_o32[];

  // Template for subsequent PLT entries.
  static const uint32_t plt_entry[];
  static const uint32_t plt_entry_r6[];
  static const uint32_t plt_entry_mips16_o32[];
  static const uint32_t plt_entry_micromips_o32[];
  static const uint32_t plt_entry_micromips32_o32[];

  // Set the final size.
  void
  set_final_data_size()
  {
    this->set_data_size(this->plt_header_size_ + this->plt_mips_offset_
                        + this->plt_comp_offset_);
  }

  // Write out the PLT data.
  void
  do_write(Output_file*);

  // Return whether the plt header contains microMIPS code.  For the sake of
  // cache alignment always use a standard header whenever any standard entries
  // are present even if microMIPS entries are present as well.  This also lets
  // the microMIPS header rely on the value of $v0 only set by microMIPS
  // entries, for a small size reduction.
  bool
  is_plt_header_compressed() const
  {
    gold_assert(this->plt_mips_offset_ + this->plt_comp_offset_ != 0);
    return this->target_->is_output_micromips() && this->plt_mips_offset_ == 0;
  }

  // Return the size of the PLT header.
  unsigned int
  get_plt_header_size() const
  {
    if (this->target_->is_output_n64())
      return 4 * sizeof(plt0_entry_n64) / sizeof(plt0_entry_n64[0]);
    else if (this->target_->is_output_n32())
      return 4 * sizeof(plt0_entry_n32) / sizeof(plt0_entry_n32[0]);
    else if (!this->is_plt_header_compressed())
      return 4 * sizeof(plt0_entry_o32) / sizeof(plt0_entry_o32[0]);
    else if (this->target_->use_32bit_micromips_instructions())
      return (2 * sizeof(plt0_entry_micromips32_o32)
              / sizeof(plt0_entry_micromips32_o32[0]));
    else
      return (2 * sizeof(plt0_entry_micromips_o32)
              / sizeof(plt0_entry_micromips_o32[0]));
  }

  // Return the PLT header entry.
  const uint32_t*
  get_plt_header_entry() const
  {
    if (this->target_->is_output_n64())
      return plt0_entry_n64;
    else if (this->target_->is_output_n32())
      return plt0_entry_n32;
    else if (!this->is_plt_header_compressed())
      return plt0_entry_o32;
    else if (this->target_->use_32bit_micromips_instructions())
      return plt0_entry_micromips32_o32;
    else
      return plt0_entry_micromips_o32;
  }

  // Return the size of the standard PLT entry.
  unsigned int
  standard_plt_entry_size() const
  { return 4 * sizeof(plt_entry) / sizeof(plt_entry[0]); }

  // Return the size of the compressed PLT entry.
  unsigned int
  compressed_plt_entry_size() const
  {
    gold_assert(!this->target_->is_output_newabi());

    if (!this->target_->is_output_micromips())
      return (2 * sizeof(plt_entry_mips16_o32)
              / sizeof(plt_entry_mips16_o32[0]));
    else if (this->target_->use_32bit_micromips_instructions())
      return (2 * sizeof(plt_entry_micromips32_o32)
              / sizeof(plt_entry_micromips32_o32[0]));
    else
      return (2 * sizeof(plt_entry_micromips_o32)
              / sizeof(plt_entry_micromips_o32[0]));
  }

  // The reloc section.
  Reloc_section* rel_;
  // The .got.plt section.
  Output_data_space* got_plt_;
  // Symbols that have PLT entry.
  std::vector<Mips_symbol<size>*> symbols_;
  // The offset of the next standard PLT entry to create.
  unsigned int plt_mips_offset_;
  // The offset of the next compressed PLT entry to create.
  unsigned int plt_comp_offset_;
  // The size of the PLT header in bytes.
  unsigned int plt_header_size_;
  // The target.
  Target_mips<size, big_endian>* target_;
};

// A class to handle the .MIPS.stubs data.

template<int size, bool big_endian>
class Mips_output_data_mips_stubs : public Output_section_data
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;

  // Unordered set of .MIPS.stubs entries.
  typedef Unordered_set<Mips_symbol<size>*, Mips_symbol_hash<size> >
      Mips_stubs_entry_set;

 public:
   Mips_output_data_mips_stubs(Target_mips<size, big_endian>* target)
     : Output_section_data(size == 32 ? 4 : 8), symbols_(), dynsym_count_(-1U),
       stub_offsets_are_set_(false), target_(target)
   { }

  // Create entry for a symbol.
  void
  make_entry(Mips_symbol<size>*);

  // Remove entry for a symbol.
  void
  remove_entry(Mips_symbol<size>* gsym);

  // Set stub offsets for symbols.  This method expects that the number of
  // entries in dynamic symbol table is set.
  void
  set_lazy_stub_offsets();

  void
  set_needs_dynsym_value();

   // Set the number of entries in dynamic symbol table.
  void
  set_dynsym_count(unsigned int dynsym_count)
  { this->dynsym_count_ = dynsym_count; }

  // Return maximum size of the stub, ie. the stub size if the dynamic symbol
  // count is greater than 0x10000.  If the dynamic symbol count is less than
  // 0x10000, the stub will be 4 bytes smaller.
  // There's no disadvantage from using microMIPS code here, so for the sake of
  // pure-microMIPS binaries we prefer it whenever there's any microMIPS code in
  // output produced at all.  This has a benefit of stubs being shorter by
  // 4 bytes each too, unless in the insn32 mode.
  unsigned int
  stub_max_size() const
  {
    if (!this->target_->is_output_micromips()
        || this->target_->use_32bit_micromips_instructions())
      return 20;
    else
      return 16;
  }

  // Return the size of the stub.  This method expects that the final dynsym
  // count is set.
  unsigned int
  stub_size() const
  {
    gold_assert(this->dynsym_count_ != -1U);
    if (this->dynsym_count_ > 0x10000)
      return this->stub_max_size();
    else
      return this->stub_max_size() - 4;
  }

  // Return output address of a stub.
  Mips_address
  stub_address(const Mips_symbol<size>* sym) const
  {
    gold_assert(sym->has_lazy_stub());
    return this->address() + sym->lazy_stub_offset();
  }

 protected:
  void
  do_adjust_output_section(Output_section* os)
  { os->set_entsize(0); }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _(".MIPS.stubs")); }

 private:
  static const uint32_t lazy_stub_normal_1[];
  static const uint32_t lazy_stub_normal_1_n64[];
  static const uint32_t lazy_stub_normal_2[];
  static const uint32_t lazy_stub_normal_2_n64[];
  static const uint32_t lazy_stub_big[];
  static const uint32_t lazy_stub_big_n64[];

  static const uint32_t lazy_stub_micromips_normal_1[];
  static const uint32_t lazy_stub_micromips_normal_1_n64[];
  static const uint32_t lazy_stub_micromips_normal_2[];
  static const uint32_t lazy_stub_micromips_normal_2_n64[];
  static const uint32_t lazy_stub_micromips_big[];
  static const uint32_t lazy_stub_micromips_big_n64[];

  static const uint32_t lazy_stub_micromips32_normal_1[];
  static const uint32_t lazy_stub_micromips32_normal_1_n64[];
  static const uint32_t lazy_stub_micromips32_normal_2[];
  static const uint32_t lazy_stub_micromips32_normal_2_n64[];
  static const uint32_t lazy_stub_micromips32_big[];
  static const uint32_t lazy_stub_micromips32_big_n64[];

  // Set the final size.
  void
  set_final_data_size()
  { this->set_data_size(this->symbols_.size() * this->stub_max_size()); }

  // Write out the .MIPS.stubs data.
  void
  do_write(Output_file*);

  // .MIPS.stubs symbols
  Mips_stubs_entry_set symbols_;
  // Number of entries in dynamic symbol table.
  unsigned int dynsym_count_;
  // Whether the stub offsets are set.
  bool stub_offsets_are_set_;
  // The target.
  Target_mips<size, big_endian>* target_;
};

// This class handles Mips .reginfo output section.

template<int size, bool big_endian>
class Mips_output_section_reginfo : public Output_section_data
{
  typedef typename elfcpp::Swap<size, big_endian>::Valtype Valtype;

 public:
  Mips_output_section_reginfo(Target_mips<size, big_endian>* target,
                              Valtype gprmask, Valtype cprmask1,
                              Valtype cprmask2, Valtype cprmask3,
                              Valtype cprmask4)
    : Output_section_data(24, 4, true), target_(target),
      gprmask_(gprmask), cprmask1_(cprmask1), cprmask2_(cprmask2),
      cprmask3_(cprmask3), cprmask4_(cprmask4)
  { }

 protected:
  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _(".reginfo")); }

  // Write out reginfo section.
  void
  do_write(Output_file* of);

 private:
  Target_mips<size, big_endian>* target_;

  // gprmask of the output .reginfo section.
  Valtype gprmask_;
  // cprmask1 of the output .reginfo section.
  Valtype cprmask1_;
  // cprmask2 of the output .reginfo section.
  Valtype cprmask2_;
  // cprmask3 of the output .reginfo section.
  Valtype cprmask3_;
  // cprmask4 of the output .reginfo section.
  Valtype cprmask4_;
};

// This class handles .MIPS.options output section.

template<int size, bool big_endian>
class Mips_output_section_options : public Output_section
{
 public:
  Mips_output_section_options(const char* name, elfcpp::Elf_Word type,
                              elfcpp::Elf_Xword flags,
                              Target_mips<size, big_endian>* target)
    : Output_section(name, type, flags), target_(target)
  {
    // After the input sections are written, we only need to update
    // ri_gp_value field of ODK_REGINFO entries.
    this->set_after_input_sections();
  }

 protected:
  // Write out option section.
  void
  do_write(Output_file* of);

 private:
  Target_mips<size, big_endian>* target_;
};

// This class handles .MIPS.abiflags output section.

template<int size, bool big_endian>
class Mips_output_section_abiflags : public Output_section_data
{
 public:
  Mips_output_section_abiflags(const Mips_abiflags<big_endian>& abiflags)
    : Output_section_data(24, 8, true), abiflags_(abiflags)
  { }

 protected:
  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _(".MIPS.abiflags")); }

  void
  do_write(Output_file* of);

 private:
  const Mips_abiflags<big_endian>& abiflags_;
};

// The MIPS target has relocation types which default handling of relocatable
// relocation cannot process.  So we have to extend the default code.

template<bool big_endian, typename Classify_reloc>
class Mips_scan_relocatable_relocs :
  public Default_scan_relocatable_relocs<Classify_reloc>
{
 public:
  // Return the strategy to use for a local symbol which is a section
  // symbol, given the relocation type.
  inline Relocatable_relocs::Reloc_strategy
  local_section_strategy(unsigned int r_type, Relobj* object)
  {
    if (Classify_reloc::sh_type == elfcpp::SHT_RELA)
      return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_RELA;
    else
      {
        switch (r_type)
          {
          case elfcpp::R_MIPS_26:
            return Relocatable_relocs::RELOC_SPECIAL;

          default:
            return Default_scan_relocatable_relocs<Classify_reloc>::
                local_section_strategy(r_type, object);
          }
      }
  }
};

// Mips_copy_relocs class.  The only difference from the base class is the
// method emit_mips, which should be called instead of Copy_reloc_entry::emit.
// Mips cannot convert all relocation types to dynamic relocs.  If a reloc
// cannot be made dynamic, a COPY reloc is emitted.

template<int sh_type, int size, bool big_endian>
class Mips_copy_relocs : public Copy_relocs<sh_type, size, big_endian>
{
 public:
  Mips_copy_relocs()
    : Copy_relocs<sh_type, size, big_endian>(elfcpp::R_MIPS_COPY)
  { }

  // Emit any saved relocations which turn out to be needed.  This is
  // called after all the relocs have been scanned.
  void
  emit_mips(Output_data_reloc<sh_type, true, size, big_endian>*,
            Symbol_table*, Layout*, Target_mips<size, big_endian>*);

 private:
  typedef typename Copy_relocs<sh_type, size, big_endian>::Copy_reloc_entry
    Copy_reloc_entry;

  // Emit this reloc if appropriate.  This is called after we have
  // scanned all the relocations, so we know whether we emitted a
  // COPY relocation for SYM_.
  void
  emit_entry(Copy_reloc_entry& entry,
             Output_data_reloc<sh_type, true, size, big_endian>* reloc_section,
             Symbol_table* symtab, Layout* layout,
             Target_mips<size, big_endian>* target);
};


// Return true if the symbol SYM should be considered to resolve local
// to the current module, and false otherwise.  The logic is taken from
// GNU ld's method _bfd_elf_symbol_refs_local_p.
static bool
symbol_refs_local(const Symbol* sym, bool has_dynsym_entry,
                  bool local_protected)
{
  // If it's a local sym, of course we resolve locally.
  if (sym == NULL)
    return true;

  // STV_HIDDEN or STV_INTERNAL ones must be local.
  if (sym->visibility() == elfcpp::STV_HIDDEN
      || sym->visibility() == elfcpp::STV_INTERNAL)
    return true;

  // If we don't have a definition in a regular file, then we can't
  // resolve locally.  The sym is either undefined or dynamic.
  if (sym->is_from_dynobj() || sym->is_undefined())
    return false;

  // Forced local symbols resolve locally.
  if (sym->is_forced_local())
    return true;

  // As do non-dynamic symbols.
  if (!has_dynsym_entry)
    return true;

  // At this point, we know the symbol is defined and dynamic.  In an
  // executable it must resolve locally, likewise when building symbolic
  // shared libraries.
  if (parameters->options().output_is_executable()
      || parameters->options().Bsymbolic())
    return true;

  // Now deal with defined dynamic symbols in shared libraries.  Ones
  // with default visibility might not resolve locally.
  if (sym->visibility() == elfcpp::STV_DEFAULT)
    return false;

  // STV_PROTECTED non-function symbols are local.
  if (sym->type() != elfcpp::STT_FUNC)
    return true;

  // Function pointer equality tests may require that STV_PROTECTED
  // symbols be treated as dynamic symbols.  If the address of a
  // function not defined in an executable is set to that function's
  // plt entry in the executable, then the address of the function in
  // a shared library must also be the plt entry in the executable.
  return local_protected;
}

// Return TRUE if references to this symbol always reference the symbol in this
// object.
static bool
symbol_references_local(const Symbol* sym, bool has_dynsym_entry)
{
  return symbol_refs_local(sym, has_dynsym_entry, false);
}

// Return TRUE if calls to this symbol always call the version in this object.
static bool
symbol_calls_local(const Symbol* sym, bool has_dynsym_entry)
{
  return symbol_refs_local(sym, has_dynsym_entry, true);
}

// Compare GOT offsets of two symbols.

template<int size, bool big_endian>
static bool
got_offset_compare(Symbol* sym1, Symbol* sym2)
{
  Mips_symbol<size>* mips_sym1 = Mips_symbol<size>::as_mips_sym(sym1);
  Mips_symbol<size>* mips_sym2 = Mips_symbol<size>::as_mips_sym(sym2);
  unsigned int area1 = mips_sym1->global_got_area();
  unsigned int area2 = mips_sym2->global_got_area();
  gold_assert(area1 != GGA_NONE && area1 != GGA_NONE);

  // GGA_NORMAL entries always come before GGA_RELOC_ONLY.
  if (area1 != area2)
    return area1 < area2;

  return mips_sym1->global_gotoffset() < mips_sym2->global_gotoffset();
}

// This method divides dynamic symbols into symbols that have GOT entry, and
// symbols that don't have GOT entry.  It also sorts symbols with the GOT entry.
// Mips ABI requires that symbols with the GOT entry must be at the end of
// dynamic symbol table, and the order in dynamic symbol table must match the
// order in GOT.

template<int size, bool big_endian>
static void
reorder_dyn_symbols(std::vector<Symbol*>* dyn_symbols,
                    std::vector<Symbol*>* non_got_symbols,
                    std::vector<Symbol*>* got_symbols)
{
  for (std::vector<Symbol*>::iterator p = dyn_symbols->begin();
       p != dyn_symbols->end();
       ++p)
    {
      Mips_symbol<size>* mips_sym = Mips_symbol<size>::as_mips_sym(*p);
      if (mips_sym->global_got_area() == GGA_NORMAL
          || mips_sym->global_got_area() == GGA_RELOC_ONLY)
        got_symbols->push_back(mips_sym);
      else
        non_got_symbols->push_back(mips_sym);
    }

  std::sort(got_symbols->begin(), got_symbols->end(),
            got_offset_compare<size, big_endian>);
}

// Functor class for processing the global symbol table.

template<int size, bool big_endian>
class Symbol_visitor_check_symbols
{
 public:
  Symbol_visitor_check_symbols(Target_mips<size, big_endian>* target,
    Layout* layout, Symbol_table* symtab)
    : target_(target), layout_(layout), symtab_(symtab)
  { }

  void
  operator()(Sized_symbol<size>* sym)
  {
    Mips_symbol<size>* mips_sym = Mips_symbol<size>::as_mips_sym(sym);
    if (local_pic_function<size, big_endian>(mips_sym))
      {
        // SYM is a function that might need $25 to be valid on entry.
        // If we're creating a non-PIC relocatable object, mark SYM as
        // being PIC.  If we're creating a non-relocatable object with
        // non-PIC branches and jumps to SYM, make sure that SYM has an la25
        // stub.
        if (parameters->options().relocatable())
          {
            if (!parameters->options().output_is_position_independent())
              mips_sym->set_pic();
          }
        else if (mips_sym->has_nonpic_branches())
          {
            this->target_->la25_stub_section(layout_)
                ->create_la25_stub(this->symtab_, this->target_, mips_sym);
          }
      }
  }

 private:
  Target_mips<size, big_endian>* target_;
  Layout* layout_;
  Symbol_table* symtab_;
};

// Relocation types, parameterized by SHT_REL vs. SHT_RELA, size,
// and endianness. The relocation format for MIPS-64 is non-standard.

template<int sh_type, int size, bool big_endian>
struct Mips_reloc_types;

template<bool big_endian>
struct Mips_reloc_types<elfcpp::SHT_REL, 32, big_endian>
{
  typedef typename elfcpp::Rel<32, big_endian> Reloc;
  typedef typename elfcpp::Rel_write<32, big_endian> Reloc_write;

  static typename elfcpp::Elf_types<32>::Elf_Swxword
  get_r_addend(const Reloc*)
  { return 0; }

  static inline void
  set_reloc_addend(Reloc_write*,
		   typename elfcpp::Elf_types<32>::Elf_Swxword)
  { gold_unreachable(); }
};

template<bool big_endian>
struct Mips_reloc_types<elfcpp::SHT_RELA, 32, big_endian>
{
  typedef typename elfcpp::Rela<32, big_endian> Reloc;
  typedef typename elfcpp::Rela_write<32, big_endian> Reloc_write;

  static typename elfcpp::Elf_types<32>::Elf_Swxword
  get_r_addend(const Reloc* reloc)
  { return reloc->get_r_addend(); }

  static inline void
  set_reloc_addend(Reloc_write* p,
		   typename elfcpp::Elf_types<32>::Elf_Swxword val)
  { p->put_r_addend(val); }
};

template<bool big_endian>
struct Mips_reloc_types<elfcpp::SHT_REL, 64, big_endian>
{
  typedef typename elfcpp::Mips64_rel<big_endian> Reloc;
  typedef typename elfcpp::Mips64_rel_write<big_endian> Reloc_write;

  static typename elfcpp::Elf_types<64>::Elf_Swxword
  get_r_addend(const Reloc*)
  { return 0; }

  static inline void
  set_reloc_addend(Reloc_write*,
		   typename elfcpp::Elf_types<64>::Elf_Swxword)
  { gold_unreachable(); }
};

template<bool big_endian>
struct Mips_reloc_types<elfcpp::SHT_RELA, 64, big_endian>
{
  typedef typename elfcpp::Mips64_rela<big_endian> Reloc;
  typedef typename elfcpp::Mips64_rela_write<big_endian> Reloc_write;

  static typename elfcpp::Elf_types<64>::Elf_Swxword
  get_r_addend(const Reloc* reloc)
  { return reloc->get_r_addend(); }

  static inline void
  set_reloc_addend(Reloc_write* p,
		   typename elfcpp::Elf_types<64>::Elf_Swxword val)
  { p->put_r_addend(val); }
};

// Forward declaration.
static unsigned int
mips_get_size_for_reloc(unsigned int, Relobj*);

// A class for inquiring about properties of a relocation,
// used while scanning relocs during a relocatable link and
// garbage collection.

template<int sh_type_, int size, bool big_endian>
class Mips_classify_reloc;

template<int sh_type_, bool big_endian>
class Mips_classify_reloc<sh_type_, 32, big_endian> :
    public gold::Default_classify_reloc<sh_type_, 32, big_endian>
{
 public:
  typedef typename Mips_reloc_types<sh_type_, 32, big_endian>::Reloc
      Reltype;
  typedef typename Mips_reloc_types<sh_type_, 32, big_endian>::Reloc_write
      Reltype_write;

  // Return the symbol referred to by the relocation.
  static inline unsigned int
  get_r_sym(const Reltype* reloc)
  { return elfcpp::elf_r_sym<32>(reloc->get_r_info()); }

  // Return the type of the relocation.
  static inline unsigned int
  get_r_type(const Reltype* reloc)
  { return elfcpp::elf_r_type<32>(reloc->get_r_info()); }

  static inline unsigned int
  get_r_type2(const Reltype*)
  { return 0; }

  static inline unsigned int
  get_r_type3(const Reltype*)
  { return 0; }

  static inline unsigned int
  get_r_ssym(const Reltype*)
  { return 0; }

  // Return the explicit addend of the relocation (return 0 for SHT_REL).
  static inline unsigned int
  get_r_addend(const Reltype* reloc)
  {
    if (sh_type_ == elfcpp::SHT_REL)
      return 0;
    return Mips_reloc_types<sh_type_, 32, big_endian>::get_r_addend(reloc);
  }

  // Write the r_info field to a new reloc, using the r_info field from
  // the original reloc, replacing the r_sym field with R_SYM.
  static inline void
  put_r_info(Reltype_write* new_reloc, Reltype* reloc, unsigned int r_sym)
  {
    unsigned int r_type = elfcpp::elf_r_type<32>(reloc->get_r_info());
    new_reloc->put_r_info(elfcpp::elf_r_info<32>(r_sym, r_type));
  }

  // Write the r_addend field to a new reloc.
  static inline void
  put_r_addend(Reltype_write* to,
	       typename elfcpp::Elf_types<32>::Elf_Swxword addend)
  { Mips_reloc_types<sh_type_, 32, big_endian>::set_reloc_addend(to, addend); }

  // Return the size of the addend of the relocation (only used for SHT_REL).
  static unsigned int
  get_size_for_reloc(unsigned int r_type, Relobj* obj)
  { return mips_get_size_for_reloc(r_type, obj); }
};

template<int sh_type_, bool big_endian>
class Mips_classify_reloc<sh_type_, 64, big_endian> :
    public gold::Default_classify_reloc<sh_type_, 64, big_endian>
{
 public:
  typedef typename Mips_reloc_types<sh_type_, 64, big_endian>::Reloc
      Reltype;
  typedef typename Mips_reloc_types<sh_type_, 64, big_endian>::Reloc_write
      Reltype_write;

  // Return the symbol referred to by the relocation.
  static inline unsigned int
  get_r_sym(const Reltype* reloc)
  { return reloc->get_r_sym(); }

  // Return the r_type of the relocation.
  static inline unsigned int
  get_r_type(const Reltype* reloc)
  { return reloc->get_r_type(); }

  // Return the r_type2 of the relocation.
  static inline unsigned int
  get_r_type2(const Reltype* reloc)
  { return reloc->get_r_type2(); }

  // Return the r_type3 of the relocation.
  static inline unsigned int
  get_r_type3(const Reltype* reloc)
  { return reloc->get_r_type3(); }

  // Return the special symbol of the relocation.
  static inline unsigned int
  get_r_ssym(const Reltype* reloc)
  { return reloc->get_r_ssym(); }

  // Return the explicit addend of the relocation (return 0 for SHT_REL).
  static inline typename elfcpp::Elf_types<64>::Elf_Swxword
  get_r_addend(const Reltype* reloc)
  {
    if (sh_type_ == elfcpp::SHT_REL)
      return 0;
    return Mips_reloc_types<sh_type_, 64, big_endian>::get_r_addend(reloc);
  }

  // Write the r_info field to a new reloc, using the r_info field from
  // the original reloc, replacing the r_sym field with R_SYM.
  static inline void
  put_r_info(Reltype_write* new_reloc, Reltype* reloc, unsigned int r_sym)
  {
    new_reloc->put_r_sym(r_sym);
    new_reloc->put_r_ssym(reloc->get_r_ssym());
    new_reloc->put_r_type3(reloc->get_r_type3());
    new_reloc->put_r_type2(reloc->get_r_type2());
    new_reloc->put_r_type(reloc->get_r_type());
  }

  // Write the r_addend field to a new reloc.
  static inline void
  put_r_addend(Reltype_write* to,
	       typename elfcpp::Elf_types<64>::Elf_Swxword addend)
  { Mips_reloc_types<sh_type_, 64, big_endian>::set_reloc_addend(to, addend); }

  // Return the size of the addend of the relocation (only used for SHT_REL).
  static unsigned int
  get_size_for_reloc(unsigned int r_type, Relobj* obj)
  { return mips_get_size_for_reloc(r_type, obj); }
};

template<int size, bool big_endian>
class Target_mips : public Sized_target<size, big_endian>
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;
  typedef Mips_output_data_reloc<elfcpp::SHT_REL, true, size, big_endian>
    Reloc_section;
  typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype32;
  typedef typename elfcpp::Swap<size, big_endian>::Valtype Valtype;
  typedef typename Mips_reloc_types<elfcpp::SHT_REL, size, big_endian>::Reloc
      Reltype;
  typedef typename Mips_reloc_types<elfcpp::SHT_RELA, size, big_endian>::Reloc
      Relatype;

 public:
  Target_mips(const Target::Target_info* info = &mips_info)
    : Sized_target<size, big_endian>(info), got_(NULL), gp_(NULL), plt_(NULL),
      got_plt_(NULL), rel_dyn_(NULL), rld_map_(NULL), copy_relocs_(),
      dyn_relocs_(), la25_stub_(NULL), mips_mach_extensions_(),
      mips_stubs_(NULL), attributes_section_data_(NULL), abiflags_(NULL),
      mach_(0), layout_(NULL), got16_addends_(), has_abiflags_section_(false),
      entry_symbol_is_compressed_(false), insn32_(false)
  {
    this->add_machine_extensions();
  }

  // The offset of $gp from the beginning of the .got section.
  static const unsigned int MIPS_GP_OFFSET = 0x7ff0;

  // The maximum size of the GOT for it to be addressable using 16-bit
  // offsets from $gp.
  static const unsigned int MIPS_GOT_MAX_SIZE = MIPS_GP_OFFSET + 0x7fff;

  // Make a new symbol table entry for the Mips target.
  Sized_symbol<size>*
  make_symbol(const char*, elfcpp::STT, Object*, unsigned int, uint64_t)
  { return new Mips_symbol<size>(); }

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

  // Relocate a section.
  void
  relocate_section(const Relocate_info<size, big_endian>*,
                   unsigned int sh_type,
                   const unsigned char* prelocs,
                   size_t reloc_count,
                   Output_section* output_section,
                   bool needs_special_offset_handling,
                   unsigned char* view,
                   Mips_address view_address,
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
                  Mips_address view_address,
                  section_size_type view_size,
                  unsigned char* reloc_view,
                  section_size_type reloc_view_size);

  // Perform target-specific processing in a relocatable link.  This is
  // only used if we use the relocation strategy RELOC_SPECIAL.
  void
  relocate_special_relocatable(const Relocate_info<size, big_endian>* relinfo,
                               unsigned int sh_type,
                               const unsigned char* preloc_in,
                               size_t relnum,
                               Output_section* output_section,
                               typename elfcpp::Elf_types<size>::Elf_Off
                                 offset_in_output_section,
                               unsigned char* view,
                               Mips_address view_address,
                               section_size_type view_size,
                               unsigned char* preloc_out);

  // Return whether SYM is defined by the ABI.
  bool
  do_is_defined_by_abi(const Symbol* sym) const
  {
    return ((strcmp(sym->name(), "__gnu_local_gp") == 0)
            || (strcmp(sym->name(), "_gp_disp") == 0)
            || (strcmp(sym->name(), "___tls_get_addr") == 0));
  }

  // Return the number of entries in the GOT.
  unsigned int
  got_entry_count() const
  {
    if (!this->has_got_section())
      return 0;
    return this->got_size() / (size/8);
  }

  // Return the number of entries in the PLT.
  unsigned int
  plt_entry_count() const
  {
    if (this->plt_ == NULL)
      return 0;
    return this->plt_->entry_count();
  }

  // Return the offset of the first non-reserved PLT entry.
  unsigned int
  first_plt_entry_offset() const
  { return this->plt_->first_plt_entry_offset(); }

  // Return the size of each PLT entry.
  unsigned int
  plt_entry_size() const
  { return this->plt_->plt_entry_size(); }

  // Get the GOT section, creating it if necessary.
  Mips_output_data_got<size, big_endian>*
  got_section(Symbol_table*, Layout*);

  // Get the GOT section.
  Mips_output_data_got<size, big_endian>*
  got_section() const
  {
    gold_assert(this->got_ != NULL);
    return this->got_;
  }

  // Get the .MIPS.stubs section, creating it if necessary.
  Mips_output_data_mips_stubs<size, big_endian>*
  mips_stubs_section(Layout* layout);

  // Get the .MIPS.stubs section.
  Mips_output_data_mips_stubs<size, big_endian>*
  mips_stubs_section() const
  {
    gold_assert(this->mips_stubs_ != NULL);
    return this->mips_stubs_;
  }

  // Get the LA25 stub section, creating it if necessary.
  Mips_output_data_la25_stub<size, big_endian>*
  la25_stub_section(Layout*);

  // Get the LA25 stub section.
  Mips_output_data_la25_stub<size, big_endian>*
  la25_stub_section()
  {
    gold_assert(this->la25_stub_ != NULL);
    return this->la25_stub_;
  }

  // Get gp value.  It has the value of .got + 0x7FF0.
  Mips_address
  gp_value() const
  {
    if (this->gp_ != NULL)
      return this->gp_->value();
    return 0;
  }

  // Get gp value.  It has the value of .got + 0x7FF0.  Adjust it for
  // multi-GOT links so that OBJECT's GOT + 0x7FF0 is returned.
  Mips_address
  adjusted_gp_value(const Mips_relobj<size, big_endian>* object)
  {
    if (this->gp_ == NULL)
      return 0;

    bool multi_got = false;
    if (this->has_got_section())
      multi_got = this->got_section()->multi_got();
    if (!multi_got)
      return this->gp_->value();
    else
      return this->gp_->value() + this->got_section()->get_got_offset(object);
  }

  // Get the dynamic reloc section, creating it if necessary.
  Reloc_section*
  rel_dyn_section(Layout*);

  bool
  do_has_custom_set_dynsym_indexes() const
  { return true; }

  // Don't emit input .reginfo/.MIPS.abiflags sections to
  // output .reginfo/.MIPS.abiflags.
  bool
  do_should_include_section(elfcpp::Elf_Word sh_type) const
  {
    return ((sh_type != elfcpp::SHT_MIPS_REGINFO)
             && (sh_type != elfcpp::SHT_MIPS_ABIFLAGS));
  }

  // Set the dynamic symbol indexes.  INDEX is the index of the first
  // global dynamic symbol.  Pointers to the symbols are stored into the
  // vector SYMS.  The names are added to DYNPOOL.  This returns an
  // updated dynamic symbol index.
  unsigned int
  do_set_dynsym_indexes(std::vector<Symbol*>* dyn_symbols, unsigned int index,
                        std::vector<Symbol*>* syms, Stringpool* dynpool,
                        Versions* versions, Symbol_table* symtab) const;

  // Remove .MIPS.stubs entry for a symbol.
  void
  remove_lazy_stub_entry(Mips_symbol<size>* sym)
  {
    if (this->mips_stubs_ != NULL)
      this->mips_stubs_->remove_entry(sym);
  }

  // The value to write into got[1] for SVR4 targets, to identify it is
  // a GNU object.  The dynamic linker can then use got[1] to store the
  // module pointer.
  uint64_t
  mips_elf_gnu_got1_mask()
  {
    if (this->is_output_n64())
      return (uint64_t)1 << 63;
    else
      return 1 << 31;
  }

  // Whether the output has microMIPS code.  This is valid only after
  // merge_obj_e_flags() is called.
  bool
  is_output_micromips() const
  {
    gold_assert(this->are_processor_specific_flags_set());
    return elfcpp::is_micromips(this->processor_specific_flags());
  }

  // Whether the output uses N32 ABI.  This is valid only after
  // merge_obj_e_flags() is called.
  bool
  is_output_n32() const
  {
    gold_assert(this->are_processor_specific_flags_set());
    return elfcpp::abi_n32(this->processor_specific_flags());
  }

  // Whether the output uses R6 ISA.  This is valid only after
  // merge_obj_e_flags() is called.
  bool
  is_output_r6() const
  {
    gold_assert(this->are_processor_specific_flags_set());
    return elfcpp::r6_isa(this->processor_specific_flags());
  }

  // Whether the output uses N64 ABI.
  bool
  is_output_n64() const
  { return size == 64; }

  // Whether the output uses NEWABI.  This is valid only after
  // merge_obj_e_flags() is called.
  bool
  is_output_newabi() const
  { return this->is_output_n32() || this->is_output_n64(); }

  // Whether we can only use 32-bit microMIPS instructions.
  bool
  use_32bit_micromips_instructions() const
  { return this->insn32_; }

  // Return the r_sym field from a relocation.
  unsigned int
  get_r_sym(const unsigned char* preloc) const
  {
    // Since REL and RELA relocs share the same structure through
    // the r_info field, we can just use REL here.
    Reltype rel(preloc);
    return Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>::
	get_r_sym(&rel);
  }

 protected:
  // Return the value to use for a dynamic symbol which requires special
  // treatment.  This is how we support equality comparisons of function
  // pointers across shared library boundaries, as described in the
  // processor specific ABI supplement.
  uint64_t
  do_dynsym_value(const Symbol* gsym) const;

  // Make an ELF object.
  Object*
  do_make_elf_object(const std::string&, Input_file*, off_t,
                     const elfcpp::Ehdr<size, big_endian>& ehdr);

  Object*
  do_make_elf_object(const std::string&, Input_file*, off_t,
                     const elfcpp::Ehdr<size, !big_endian>&)
  { gold_unreachable(); }

  // Make an output section.
  Output_section*
  do_make_output_section(const char* name, elfcpp::Elf_Word type,
                         elfcpp::Elf_Xword flags)
    {
      if (type == elfcpp::SHT_MIPS_OPTIONS)
        return new Mips_output_section_options<size, big_endian>(name, type,
                                                                 flags, this);
      else
        return new Output_section(name, type, flags);
    }

  // Adjust ELF file header.
  void
  do_adjust_elf_header(unsigned char* view, int len);

  // Get the custom dynamic tag value.
  unsigned int
  do_dynamic_tag_custom_value(elfcpp::DT) const;

  // Adjust the value written to the dynamic symbol table.
  virtual void
  do_adjust_dyn_symbol(const Symbol* sym, unsigned char* view) const
  {
    elfcpp::Sym<size, big_endian> isym(view);
    elfcpp::Sym_write<size, big_endian> osym(view);
    const Mips_symbol<size>* mips_sym = Mips_symbol<size>::as_mips_sym(sym);

    // Keep dynamic compressed symbols odd.  This allows the dynamic linker
    // to treat compressed symbols like any other.
    Mips_address value = isym.get_st_value();
    if (mips_sym->is_mips16() && value != 0)
      {
        if (!mips_sym->has_mips16_fn_stub())
          value |= 1;
        else
          {
            // If we have a MIPS16 function with a stub, the dynamic symbol
            // must refer to the stub, since only the stub uses the standard
            // calling conventions.  Stub contains MIPS32 code, so don't add +1
            // in this case.

            // There is a code which does this in the method
            // Target_mips::do_dynsym_value, but that code will only be
            // executed if the symbol is from dynobj.
            // TODO(sasa): GNU ld also changes the value in non-dynamic symbol
            // table.

            Mips16_stub_section<size, big_endian>* fn_stub =
              mips_sym->template get_mips16_fn_stub<big_endian>();
            value = fn_stub->output_address();
            osym.put_st_size(fn_stub->section_size());
          }

        osym.put_st_value(value);
        osym.put_st_other(elfcpp::elf_st_other(sym->visibility(),
                          mips_sym->nonvis() - (elfcpp::STO_MIPS16 >> 2)));
      }
    else if ((mips_sym->is_micromips()
              // Stubs are always microMIPS if there is any microMIPS code in
              // the output.
              || (this->is_output_micromips() && mips_sym->has_lazy_stub()))
             && value != 0)
      {
        osym.put_st_value(value | 1);
        osym.put_st_other(elfcpp::elf_st_other(sym->visibility(),
                          mips_sym->nonvis() - (elfcpp::STO_MICROMIPS >> 2)));
      }
  }

 private:
  // The class which scans relocations.
  class Scan
  {
   public:
    Scan()
    { }

    static inline int
    get_reference_flags(unsigned int r_type);

    inline void
    local(Symbol_table* symtab, Layout* layout, Target_mips* target,
          Sized_relobj_file<size, big_endian>* object,
          unsigned int data_shndx,
          Output_section* output_section,
          const Reltype& reloc, unsigned int r_type,
          const elfcpp::Sym<size, big_endian>& lsym,
          bool is_discarded);

    inline void
    local(Symbol_table* symtab, Layout* layout, Target_mips* target,
          Sized_relobj_file<size, big_endian>* object,
          unsigned int data_shndx,
          Output_section* output_section,
          const Relatype& reloc, unsigned int r_type,
          const elfcpp::Sym<size, big_endian>& lsym,
          bool is_discarded);

    inline void
    local(Symbol_table* symtab, Layout* layout, Target_mips* target,
          Sized_relobj_file<size, big_endian>* object,
          unsigned int data_shndx,
          Output_section* output_section,
          const Relatype* rela,
          const Reltype* rel,
          unsigned int rel_type,
          unsigned int r_type,
          const elfcpp::Sym<size, big_endian>& lsym,
          bool is_discarded);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_mips* target,
           Sized_relobj_file<size, big_endian>* object,
           unsigned int data_shndx,
           Output_section* output_section,
           const Reltype& reloc, unsigned int r_type,
           Symbol* gsym);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_mips* target,
           Sized_relobj_file<size, big_endian>* object,
           unsigned int data_shndx,
           Output_section* output_section,
           const Relatype& reloc, unsigned int r_type,
           Symbol* gsym);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_mips* target,
           Sized_relobj_file<size, big_endian>* object,
           unsigned int data_shndx,
           Output_section* output_section,
           const Relatype* rela,
           const Reltype* rel,
           unsigned int rel_type,
           unsigned int r_type,
           Symbol* gsym);

    inline bool
    local_reloc_may_be_function_pointer(Symbol_table* , Layout*,
                                        Target_mips*,
                                        Sized_relobj_file<size, big_endian>*,
                                        unsigned int,
                                        Output_section*,
                                        const Reltype&,
                                        unsigned int,
                                        const elfcpp::Sym<size, big_endian>&)
    { return false; }

    inline bool
    global_reloc_may_be_function_pointer(Symbol_table*, Layout*,
                                         Target_mips*,
                                         Sized_relobj_file<size, big_endian>*,
                                         unsigned int,
                                         Output_section*,
                                         const Reltype&,
                                         unsigned int, Symbol*)
    { return false; }

    inline bool
    local_reloc_may_be_function_pointer(Symbol_table*, Layout*,
                                        Target_mips*,
                                        Sized_relobj_file<size, big_endian>*,
                                        unsigned int,
                                        Output_section*,
                                        const Relatype&,
                                        unsigned int,
                                        const elfcpp::Sym<size, big_endian>&)
    { return false; }

    inline bool
    global_reloc_may_be_function_pointer(Symbol_table*, Layout*,
                                         Target_mips*,
                                         Sized_relobj_file<size, big_endian>*,
                                         unsigned int,
                                         Output_section*,
                                         const Relatype&,
                                         unsigned int, Symbol*)
    { return false; }
   private:
    static void
    unsupported_reloc_local(Sized_relobj_file<size, big_endian>*,
                            unsigned int r_type);

    static void
    unsupported_reloc_global(Sized_relobj_file<size, big_endian>*,
                             unsigned int r_type, Symbol*);
  };

  // The class which implements relocation.
  class Relocate
  {
   public:
    Relocate()
      : calculated_value_(0), calculate_only_(false)
    { }

    ~Relocate()
    { }

    // Return whether a R_MIPS_32/R_MIPS_64 relocation needs to be applied.
    inline bool
    should_apply_static_reloc(const Mips_symbol<size>* gsym,
                              unsigned int r_type,
                              Output_section* output_section,
                              Target_mips* target);

    // Do a relocation.  Return false if the caller should not issue
    // any warnings about this relocation.
    inline bool
    relocate(const Relocate_info<size, big_endian>*, unsigned int,
	     Target_mips*, Output_section*, size_t, const unsigned char*,
	     const Sized_symbol<size>*, const Symbol_value<size>*,
	     unsigned char*, Mips_address, section_size_type);

   private:
    // Result of the relocation.
    Valtype calculated_value_;
    // Whether we have to calculate relocation instead of applying it.
    bool calculate_only_;
  };

  // This POD class holds the dynamic relocations that should be emitted instead
  // of R_MIPS_32, R_MIPS_REL32 and R_MIPS_64 relocations.  We will emit these
  // relocations if it turns out that the symbol does not have static
  // relocations.
  class Dyn_reloc
  {
   public:
    Dyn_reloc(Mips_symbol<size>* sym, unsigned int r_type,
              Mips_relobj<size, big_endian>* relobj, unsigned int shndx,
              Output_section* output_section, Mips_address r_offset)
      : sym_(sym), r_type_(r_type), relobj_(relobj),
        shndx_(shndx), output_section_(output_section),
        r_offset_(r_offset)
    { }

    // Emit this reloc if appropriate.  This is called after we have
    // scanned all the relocations, so we know whether the symbol has
    // static relocations.
    void
    emit(Reloc_section* rel_dyn, Mips_output_data_got<size, big_endian>* got,
         Symbol_table* symtab)
    {
      if (!this->sym_->has_static_relocs())
        {
          got->record_global_got_symbol(this->sym_, this->relobj_,
                                        this->r_type_, true, false);
          if (!symbol_references_local(this->sym_,
                                this->sym_->should_add_dynsym_entry(symtab)))
            rel_dyn->add_global(this->sym_, this->r_type_,
                                this->output_section_, this->relobj_,
                                this->shndx_, this->r_offset_);
          else
            rel_dyn->add_symbolless_global_addend(this->sym_, this->r_type_,
                                          this->output_section_, this->relobj_,
                                          this->shndx_, this->r_offset_);
        }
    }

   private:
    Mips_symbol<size>* sym_;
    unsigned int r_type_;
    Mips_relobj<size, big_endian>* relobj_;
    unsigned int shndx_;
    Output_section* output_section_;
    Mips_address r_offset_;
  };

  // Adjust TLS relocation type based on the options and whether this
  // is a local symbol.
  static tls::Tls_optimization
  optimize_tls_reloc(bool is_final, int r_type);

  // Return whether there is a GOT section.
  bool
  has_got_section() const
  { return this->got_ != NULL; }

  // Check whether the given ELF header flags describe a 32-bit binary.
  bool
  mips_32bit_flags(elfcpp::Elf_Word);

  enum Mips_mach {
    mach_mips3000             = 3000,
    mach_mips3900             = 3900,
    mach_mips4000             = 4000,
    mach_mips4010             = 4010,
    mach_mips4100             = 4100,
    mach_mips4111             = 4111,
    mach_mips4120             = 4120,
    mach_mips4300             = 4300,
    mach_mips4400             = 4400,
    mach_mips4600             = 4600,
    mach_mips4650             = 4650,
    mach_mips5000             = 5000,
    mach_mips5400             = 5400,
    mach_mips5500             = 5500,
    mach_mips5900             = 5900,
    mach_mips6000             = 6000,
    mach_mips7000             = 7000,
    mach_mips8000             = 8000,
    mach_mips9000             = 9000,
    mach_mips10000            = 10000,
    mach_mips12000            = 12000,
    mach_mips14000            = 14000,
    mach_mips16000            = 16000,
    mach_mips16               = 16,
    mach_mips5                = 5,
    mach_mips_loongson_2e     = 3001,
    mach_mips_loongson_2f     = 3002,
    mach_mips_gs464           = 3003,
    mach_mips_gs464e          = 3004,
    mach_mips_gs264e          = 3005,
    mach_mips_sb1             = 12310201, // octal 'SB', 01
    mach_mips_octeon          = 6501,
    mach_mips_octeonp         = 6601,
    mach_mips_octeon2         = 6502,
    mach_mips_octeon3         = 6503,
    mach_mips_xlr             = 887682,   // decimal 'XLR'
    mach_mipsisa32            = 32,
    mach_mipsisa32r2          = 33,
    mach_mipsisa32r3          = 34,
    mach_mipsisa32r5          = 36,
    mach_mipsisa32r6          = 37,
    mach_mipsisa64            = 64,
    mach_mipsisa64r2          = 65,
    mach_mipsisa64r3          = 66,
    mach_mipsisa64r5          = 68,
    mach_mipsisa64r6          = 69,
    mach_mips_micromips       = 96
  };

  // Return the MACH for a MIPS e_flags value.
  unsigned int
  elf_mips_mach(elfcpp::Elf_Word);

  // Return the MACH for each .MIPS.abiflags ISA Extension.
  unsigned int
  mips_isa_ext_mach(unsigned int);

  // Return the .MIPS.abiflags value representing each ISA Extension.
  unsigned int
  mips_isa_ext(unsigned int);

  // Update the isa_level, isa_rev, isa_ext fields of abiflags.
  void
  update_abiflags_isa(const std::string&, elfcpp::Elf_Word,
                      Mips_abiflags<big_endian>*);

  // Infer the content of the ABI flags based on the elf header.
  void
  infer_abiflags(Mips_relobj<size, big_endian>*, Mips_abiflags<big_endian>*);

  // Create abiflags from elf header or from .MIPS.abiflags section.
  void
  create_abiflags(Mips_relobj<size, big_endian>*, Mips_abiflags<big_endian>*);

  // Return the meaning of fp_abi, or "unknown" if not known.
  const char*
  fp_abi_string(int);

  // Select fp_abi.
  int
  select_fp_abi(const std::string&, int, int);

  // Merge attributes from input object.
  void
  merge_obj_attributes(const std::string&, const Attributes_section_data*);

  // Merge abiflags from input object.
  void
  merge_obj_abiflags(const std::string&, Mips_abiflags<big_endian>*);

  // Check whether machine EXTENSION is an extension of machine BASE.
  bool
  mips_mach_extends(unsigned int, unsigned int);

  // Merge file header flags from input object.
  void
  merge_obj_e_flags(const std::string&, elfcpp::Elf_Word);

  // Encode ISA level and revision as a single value.
  int
  level_rev(unsigned char isa_level, unsigned char isa_rev) const
  { return (isa_level << 3) | isa_rev; }

  // True if we are linking for CPUs that are faster if JAL is converted to BAL.
  static inline bool
  jal_to_bal()
  { return false; }

  // True if we are linking for CPUs that are faster if JALR is converted to
  // BAL.  This should be safe for all architectures.  We enable this predicate
  // for all CPUs.
  static inline bool
  jalr_to_bal()
  { return true; }

  // True if we are linking for CPUs that are faster if JR is converted to B.
  // This should be safe for all architectures.  We enable this predicate for
  // all CPUs.
  static inline bool
  jr_to_b()
  { return true; }

  // Return the size of the GOT section.
  section_size_type
  got_size() const
  {
    gold_assert(this->got_ != NULL);
    return this->got_->data_size();
  }

  // Create a PLT entry for a global symbol referenced by r_type relocation.
  void
  make_plt_entry(Symbol_table*, Layout*, Mips_symbol<size>*,
                 unsigned int r_type);

  // Get the PLT section.
  Mips_output_data_plt<size, big_endian>*
  plt_section() const
  {
    gold_assert(this->plt_ != NULL);
    return this->plt_;
  }

  // Get the GOT PLT section.
  const Mips_output_data_plt<size, big_endian>*
  got_plt_section() const
  {
    gold_assert(this->got_plt_ != NULL);
    return this->got_plt_;
  }

  // Copy a relocation against a global symbol.
  void
  copy_reloc(Symbol_table* symtab, Layout* layout,
             Sized_relobj_file<size, big_endian>* object,
             unsigned int shndx, Output_section* output_section,
             Symbol* sym, unsigned int r_type, Mips_address r_offset)
  {
    this->copy_relocs_.copy_reloc(symtab, layout,
                                  symtab->get_sized_symbol<size>(sym),
                                  object, shndx, output_section,
                                  r_type, r_offset, 0,
                                  this->rel_dyn_section(layout));
  }

  void
  dynamic_reloc(Mips_symbol<size>* sym, unsigned int r_type,
                Mips_relobj<size, big_endian>* relobj,
                unsigned int shndx, Output_section* output_section,
                Mips_address r_offset)
  {
    this->dyn_relocs_.push_back(Dyn_reloc(sym, r_type, relobj, shndx,
                                          output_section, r_offset));
  }

  // Calculate value of _gp symbol.
  void
  set_gp(Layout*, Symbol_table*);

  const char*
  elf_mips_abi_name(elfcpp::Elf_Word e_flags);
  const char*
  elf_mips_mach_name(elfcpp::Elf_Word e_flags);

  // Adds entries that describe how machines relate to one another.  The entries
  // are ordered topologically with MIPS I extensions listed last.  First
  // element is extension, second element is base.
  void
  add_machine_extensions()
  {
    // MIPS64r2 extensions.
    this->add_extension(mach_mips_octeon3, mach_mips_octeon2);
    this->add_extension(mach_mips_octeon2, mach_mips_octeonp);
    this->add_extension(mach_mips_octeonp, mach_mips_octeon);
    this->add_extension(mach_mips_octeon, mach_mipsisa64r2);
    this->add_extension(mach_mips_gs264e, mach_mips_gs464e);
    this->add_extension(mach_mips_gs464e, mach_mips_gs464);
    this->add_extension(mach_mips_gs464, mach_mipsisa64r2);

    // MIPS64 extensions.
    this->add_extension(mach_mipsisa64r2, mach_mipsisa64);
    this->add_extension(mach_mips_sb1, mach_mipsisa64);
    this->add_extension(mach_mips_xlr, mach_mipsisa64);

    // MIPS V extensions.
    this->add_extension(mach_mipsisa64, mach_mips5);

    // R10000 extensions.
    this->add_extension(mach_mips12000, mach_mips10000);
    this->add_extension(mach_mips14000, mach_mips10000);
    this->add_extension(mach_mips16000, mach_mips10000);

    // R5000 extensions.  Note: the vr5500 ISA is an extension of the core
    // vr5400 ISA, but doesn't include the multimedia stuff.  It seems
    // better to allow vr5400 and vr5500 code to be merged anyway, since
    // many libraries will just use the core ISA.  Perhaps we could add
    // some sort of ASE flag if this ever proves a problem.
    this->add_extension(mach_mips5500, mach_mips5400);
    this->add_extension(mach_mips5400, mach_mips5000);

    // MIPS IV extensions.
    this->add_extension(mach_mips5, mach_mips8000);
    this->add_extension(mach_mips10000, mach_mips8000);
    this->add_extension(mach_mips5000, mach_mips8000);
    this->add_extension(mach_mips7000, mach_mips8000);
    this->add_extension(mach_mips9000, mach_mips8000);

    // VR4100 extensions.
    this->add_extension(mach_mips4120, mach_mips4100);
    this->add_extension(mach_mips4111, mach_mips4100);

    // MIPS III extensions.
    this->add_extension(mach_mips_loongson_2e, mach_mips4000);
    this->add_extension(mach_mips_loongson_2f, mach_mips4000);
    this->add_extension(mach_mips8000, mach_mips4000);
    this->add_extension(mach_mips4650, mach_mips4000);
    this->add_extension(mach_mips4600, mach_mips4000);
    this->add_extension(mach_mips4400, mach_mips4000);
    this->add_extension(mach_mips4300, mach_mips4000);
    this->add_extension(mach_mips4100, mach_mips4000);
    this->add_extension(mach_mips4010, mach_mips4000);
    this->add_extension(mach_mips5900, mach_mips4000);

    // MIPS32 extensions.
    this->add_extension(mach_mipsisa32r2, mach_mipsisa32);

    // MIPS II extensions.
    this->add_extension(mach_mips4000, mach_mips6000);
    this->add_extension(mach_mipsisa32, mach_mips6000);

    // MIPS I extensions.
    this->add_extension(mach_mips6000, mach_mips3000);
    this->add_extension(mach_mips3900, mach_mips3000);
  }

  // Add value to MIPS extenstions.
  void
  add_extension(unsigned int base, unsigned int extension)
  {
    std::pair<unsigned int, unsigned int> ext(base, extension);
    this->mips_mach_extensions_.push_back(ext);
  }

  // Return the number of entries in the .dynsym section.
  unsigned int get_dt_mips_symtabno() const
  {
    return ((unsigned int)(this->layout_->dynsym_section()->data_size()
                           / elfcpp::Elf_sizes<size>::sym_size));
    // TODO(sasa): Entry size is MIPS_ELF_SYM_SIZE.
  }

  // Information about this specific target which we pass to the
  // general Target structure.
  static const Target::Target_info mips_info;
  // The GOT section.
  Mips_output_data_got<size, big_endian>* got_;
  // gp symbol.  It has the value of .got + 0x7FF0.
  Sized_symbol<size>* gp_;
  // The PLT section.
  Mips_output_data_plt<size, big_endian>* plt_;
  // The GOT PLT section.
  Output_data_space* got_plt_;
  // The dynamic reloc section.
  Reloc_section* rel_dyn_;
  // The .rld_map section.
  Output_data_zero_fill* rld_map_;
  // Relocs saved to avoid a COPY reloc.
  Mips_copy_relocs<elfcpp::SHT_REL, size, big_endian> copy_relocs_;

  // A list of dyn relocs to be saved.
  std::vector<Dyn_reloc> dyn_relocs_;

  // The LA25 stub section.
  Mips_output_data_la25_stub<size, big_endian>* la25_stub_;
  // Architecture extensions.
  std::vector<std::pair<unsigned int, unsigned int> > mips_mach_extensions_;
  // .MIPS.stubs
  Mips_output_data_mips_stubs<size, big_endian>* mips_stubs_;

  // Attributes section data in output.
  Attributes_section_data* attributes_section_data_;
  // .MIPS.abiflags section data in output.
  Mips_abiflags<big_endian>* abiflags_;

  unsigned int mach_;
  Layout* layout_;

  typename std::list<got16_addend<size, big_endian> > got16_addends_;

  // Whether there is an input .MIPS.abiflags section.
  bool has_abiflags_section_;

  // Whether the entry symbol is mips16 or micromips.
  bool entry_symbol_is_compressed_;

  // Whether we can use only 32-bit microMIPS instructions.
  // TODO(sasa): This should be a linker option.
  bool insn32_;
};

// Helper structure for R_MIPS*_HI16/LO16 and R_MIPS*_GOT16/LO16 relocations.
// It records high part of the relocation pair.

template<int size, bool big_endian>
struct reloc_high
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;

  reloc_high(unsigned char* _view, const Mips_relobj<size, big_endian>* _object,
             const Symbol_value<size>* _psymval, Mips_address _addend,
             unsigned int _r_type, unsigned int _r_sym, bool _extract_addend,
             Mips_address _address = 0, bool _gp_disp = false)
    : view(_view), object(_object), psymval(_psymval), addend(_addend),
      r_type(_r_type), r_sym(_r_sym), extract_addend(_extract_addend),
      address(_address), gp_disp(_gp_disp)
  { }

  unsigned char* view;
  const Mips_relobj<size, big_endian>* object;
  const Symbol_value<size>* psymval;
  Mips_address addend;
  unsigned int r_type;
  unsigned int r_sym;
  bool extract_addend;
  Mips_address address;
  bool gp_disp;
};

template<int size, bool big_endian>
class Mips_relocate_functions : public Relocate_functions<size, big_endian>
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Mips_address;
  typedef typename elfcpp::Swap<size, big_endian>::Valtype Valtype;
  typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype16;
  typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype32;
  typedef typename elfcpp::Swap<64, big_endian>::Valtype Valtype64;

 public:
  typedef enum
  {
    STATUS_OKAY,            // No error during relocation.
    STATUS_OVERFLOW,        // Relocation overflow.
    STATUS_BAD_RELOC,       // Relocation cannot be applied.
    STATUS_PCREL_UNALIGNED  // Unaligned PC-relative relocation.
  } Status;

 private:
  typedef Relocate_functions<size, big_endian> Base;
  typedef Mips_relocate_functions<size, big_endian> This;

  static typename std::list<reloc_high<size, big_endian> > hi16_relocs;
  static typename std::list<reloc_high<size, big_endian> > got16_relocs;
  static typename std::list<reloc_high<size, big_endian> > pchi16_relocs;

  template<int valsize>
  static inline typename This::Status
  check_overflow(Valtype value)
  {
    if (size == 32)
      return (Bits<valsize>::has_overflow32(value)
              ? This::STATUS_OVERFLOW
              : This::STATUS_OKAY);

    return (Bits<valsize>::has_overflow(value)
            ? This::STATUS_OVERFLOW
            : This::STATUS_OKAY);
  }

  static inline bool
  should_shuffle_micromips_reloc(unsigned int r_type)
  {
    return (micromips_reloc(r_type)
            && r_type != elfcpp::R_MICROMIPS_PC7_S1
            && r_type != elfcpp::R_MICROMIPS_PC10_S1);
  }

 public:
  //   R_MIPS16_26 is used for the mips16 jal and jalx instructions.
  //   Most mips16 instructions are 16 bits, but these instructions
  //   are 32 bits.
  //
  //   The format of these instructions is:
  //
  //   +--------------+--------------------------------+
  //   |     JALX     | X|   Imm 20:16  |   Imm 25:21  |
  //   +--------------+--------------------------------+
  //   |                Immediate  15:0                |
  //   +-----------------------------------------------+
  //
  //   JALX is the 5-bit value 00011.  X is 0 for jal, 1 for jalx.
  //   Note that the immediate value in the first word is swapped.
  //
  //   When producing a relocatable object file, R_MIPS16_26 is
  //   handled mostly like R_MIPS_26.  In particular, the addend is
  //   stored as a straight 26-bit value in a 32-bit instruction.
  //   (gas makes life simpler for itself by never adjusting a
  //   R_MIPS16_26 reloc to be against a section, so the addend is
  //   always zero).  However, the 32 bit instruction is stored as 2
  //   16-bit values, rather than a single 32-bit value.  In a
  //   big-endian file, the result is the same; in a little-endian
  //   file, the two 16-bit halves of the 32 bit value are swapped.
  //   This is so that a disassembler can recognize the jal
  //   instruction.
  //
  //   When doing a final link, R_MIPS16_26 is treated as a 32 bit
  //   instruction stored as two 16-bit values.  The addend A is the
  //   contents of the targ26 field.  The calculation is the same as
  //   R_MIPS_26.  When storing the calculated value, reorder the
  //   immediate value as shown above, and don't forget to store the
  //   value as two 16-bit values.
  //
  //   To put it in MIPS ABI terms, the relocation field is T-targ26-16,
  //   defined as
  //
  //   big-endian:
  //   +--------+----------------------+
  //   |        |                      |
  //   |        |    targ26-16         |
  //   |31    26|25                   0|
  //   +--------+----------------------+
  //
  //   little-endian:
  //   +----------+------+-------------+
  //   |          |      |             |
  //   |  sub1    |      |     sub2    |
  //   |0        9|10  15|16         31|
  //   +----------+--------------------+
  //   where targ26-16 is sub1 followed by sub2 (i.e., the addend field A is
  //   ((sub1 << 16) | sub2)).
  //
  //   When producing a relocatable object file, the calculation is
  //   (((A < 2) | ((P + 4) & 0xf0000000) + S) >> 2)
  //   When producing a fully linked file, the calculation is
  //   let R = (((A < 2) | ((P + 4) & 0xf0000000) + S) >> 2)
  //   ((R & 0x1f0000) << 5) | ((R & 0x3e00000) >> 5) | (R & 0xffff)
  //
  //   The table below lists the other MIPS16 instruction relocations.
  //   Each one is calculated in the same way as the non-MIPS16 relocation
  //   given on the right, but using the extended MIPS16 layout of 16-bit
  //   immediate fields:
  //
  //      R_MIPS16_GPREL          R_MIPS_GPREL16
  //      R_MIPS16_GOT16          R_MIPS_GOT16
  //      R_MIPS16_CALL16         R_MIPS_CALL16
  //      R_MIPS16_HI16           R_MIPS_HI16
  //      R_MIPS16_LO16           R_MIPS_LO16
  //
  //   A typical instruction will have a format like this:
  //
  //   +--------------+--------------------------------+
  //   |    EXTEND    |     Imm 10:5    |   Imm 15:11  |
  //   +--------------+--------------------------------+
  //   |    Major     |   rx   |   ry   |   Imm  4:0   |
  //   +--------------+--------------------------------+
  //
  //   EXTEND is the five bit value 11110.  Major is the instruction
  //   opcode.
  //
  //   All we need to do here is shuffle the bits appropriately.
  //   As above, the two 16-bit halves must be swapped on a
  //   little-endian system.

  // Similar to MIPS16, the two 16-bit halves in microMIPS must be swapped
  // on a little-endian system.  This does not apply to R_MICROMIPS_PC7_S1
  // and R_MICROMIPS_PC10_S1 relocs that apply to 16-bit instructions.

  static void
  mips_reloc_unshuffle(unsigned char* view, unsigned int r_type,
                       bool jal_shuffle)
  {
    if (!mips16_reloc(r_type)
        && !should_shuffle_micromips_reloc(r_type))
      return;

    // Pick up the first and second halfwords of the instruction.
    Valtype16 first = elfcpp::Swap<16, big_endian>::readval(view);
    Valtype16 second = elfcpp::Swap<16, big_endian>::readval(view + 2);
    Valtype32 val;

    if (micromips_reloc(r_type)
        || (r_type == elfcpp::R_MIPS16_26 && !jal_shuffle))
      val = first << 16 | second;
    else if (r_type != elfcpp::R_MIPS16_26)
      val = (((first & 0xf800) << 16) | ((second & 0xffe0) << 11)
             | ((first & 0x1f) << 11) | (first & 0x7e0) | (second & 0x1f));
    else
      val = (((first & 0xfc00) << 16) | ((first & 0x3e0) << 11)
             | ((first & 0x1f) << 21) | second);

    elfcpp::Swap<32, big_endian>::writeval(view, val);
  }

  static void
  mips_reloc_shuffle(unsigned char* view, unsigned int r_type, bool jal_shuffle)
  {
    if (!mips16_reloc(r_type)
        && !should_shuffle_micromips_reloc(r_type))
      return;

    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(view);
    Valtype16 first, second;

    if (micromips_reloc(r_type)
        || (r_type == elfcpp::R_MIPS16_26 && !jal_shuffle))
      {
        second = val & 0xffff;
        first = val >> 16;
      }
    else if (r_type != elfcpp::R_MIPS16_26)
      {
        second = ((val >> 11) & 0xffe0) | (val & 0x1f);
        first = ((val >> 16) & 0xf800) | ((val >> 11) & 0x1f) | (val & 0x7e0);
      }
    else
      {
        second = val & 0xffff;
        first = ((val >> 16) & 0xfc00) | ((val >> 11) & 0x3e0)
                 | ((val >> 21) & 0x1f);
      }

    elfcpp::Swap<16, big_endian>::writeval(view + 2, second);
    elfcpp::Swap<16, big_endian>::writeval(view, first);
  }

  // R_MIPS_16: S + sign-extend(A)
  static inline typename This::Status
  rel16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
        const Symbol_value<size>* psymval, Mips_address addend_a,
        bool extract_addend, bool calculate_only, Valtype* calculated_value)
  {
    Valtype16* wv = reinterpret_cast<Valtype16*>(view);
    Valtype16 val = elfcpp::Swap<16, big_endian>::readval(wv);

    Valtype addend = (extract_addend ? Bits<16>::sign_extend32(val)
                                     : addend_a);

    Valtype x = psymval->value(object, addend);
    val = Bits<16>::bit_select32(val, x, 0xffffU);

    if (calculate_only)
      {
        *calculated_value = x;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<16, big_endian>::writeval(wv, val);

    return check_overflow<16>(x);
  }

  // R_MIPS_32: S + A
  static inline typename This::Status
  rel32(unsigned char* view, const Mips_relobj<size, big_endian>* object,
        const Symbol_value<size>* psymval, Mips_address addend_a,
        bool extract_addend, bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype addend = (extract_addend
                        ? elfcpp::Swap<32, big_endian>::readval(wv)
                        : addend_a);
    Valtype x = psymval->value(object, addend);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, x);

    return This::STATUS_OKAY;
  }

  // R_MIPS_JALR, R_MICROMIPS_JALR
  static inline typename This::Status
  reljalr(unsigned char* view, const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address address,
          Mips_address addend_a, bool extract_addend, bool cross_mode_jump,
          unsigned int r_type, bool jalr_to_bal, bool jr_to_b,
          bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype addend = extract_addend ? 0 : addend_a;
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    // Try converting J(AL)R to B(AL), if the target is in range.
    if (r_type == elfcpp::R_MIPS_JALR
        && !cross_mode_jump
        && ((jalr_to_bal && val == 0x0320f809)    // jalr t9
            || (jr_to_b && val == 0x03200008)))   // jr t9
      {
        int offset = psymval->value(object, addend) - (address + 4);
        if (!Bits<18>::has_overflow32(offset))
          {
            if (val == 0x03200008)   // jr t9
              val = 0x10000000 | (((Valtype32)offset >> 2) & 0xffff);  // b addr
            else
              val = 0x04110000 | (((Valtype32)offset >> 2) & 0xffff); //bal addr
          }
      }

    if (calculate_only)
      *calculated_value = val;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_PC32: S + A - P
  static inline typename This::Status
  relpc32(unsigned char* view, const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address address,
          Mips_address addend_a, bool extract_addend, bool calculate_only,
          Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype addend = (extract_addend
                        ? elfcpp::Swap<32, big_endian>::readval(wv)
                        : addend_a);
    Valtype x = psymval->value(object, addend) - address;

    if (calculate_only)
       *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, x);

    return This::STATUS_OKAY;
  }

  // R_MIPS_26, R_MIPS16_26, R_MICROMIPS_26_S1
  static inline typename This::Status
  rel26(unsigned char* view, const Mips_relobj<size, big_endian>* object,
        const Symbol_value<size>* psymval, Mips_address address,
        bool local, Mips_address addend_a, bool extract_addend,
        const Symbol* gsym, bool cross_mode_jump, unsigned int r_type,
        bool jal_to_bal, bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend;
    if (extract_addend)
      {
        if (r_type == elfcpp::R_MICROMIPS_26_S1)
          addend = (val & 0x03ffffff) << 1;
        else
          addend = (val & 0x03ffffff) << 2;
      }
    else
      addend = addend_a;

    // Make sure the target of JALX is word-aligned.  Bit 0 must be
    // the correct ISA mode selector and bit 1 must be 0.
    if (!calculate_only && cross_mode_jump
        && (psymval->value(object, 0) & 3) != (r_type == elfcpp::R_MIPS_26))
      {
        gold_warning(_("JALX to a non-word-aligned address"));
        return This::STATUS_BAD_RELOC;
      }

    // Shift is 2, unusually, for microMIPS JALX.
    unsigned int shift =
        (!cross_mode_jump && r_type == elfcpp::R_MICROMIPS_26_S1) ? 1 : 2;

    Valtype x;
    if (local)
      x = addend | ((address + 4) & (0xfc000000 << shift));
    else
      {
        if (shift == 1)
          x = Bits<27>::sign_extend32(addend);
        else
          x = Bits<28>::sign_extend32(addend);
      }
    x = psymval->value(object, x) >> shift;

    if (!calculate_only && !local && !gsym->is_weak_undefined()
        && ((x >> 26) != ((address + 4) >> (26 + shift))))
      return This::STATUS_OVERFLOW;

    val = Bits<32>::bit_select32(val, x, 0x03ffffff);

    // If required, turn JAL into JALX.
    if (cross_mode_jump)
      {
        bool ok;
        Valtype32 opcode = val >> 26;
        Valtype32 jalx_opcode;

        // Check to see if the opcode is already JAL or JALX.
        if (r_type == elfcpp::R_MIPS16_26)
          {
            ok = (opcode == 0x6) || (opcode == 0x7);
            jalx_opcode = 0x7;
          }
        else if (r_type == elfcpp::R_MICROMIPS_26_S1)
          {
            ok = (opcode == 0x3d) || (opcode == 0x3c);
            jalx_opcode = 0x3c;
          }
        else
          {
            ok = (opcode == 0x3) || (opcode == 0x1d);
            jalx_opcode = 0x1d;
          }

        // If the opcode is not JAL or JALX, there's a problem.  We cannot
        // convert J or JALS to JALX.
        if (!calculate_only && !ok)
          {
            gold_error(_("Unsupported jump between ISA modes; consider "
                         "recompiling with interlinking enabled."));
            return This::STATUS_BAD_RELOC;
          }

        // Make this the JALX opcode.
        val = (val & ~(0x3f << 26)) | (jalx_opcode << 26);
      }

    // Try converting JAL to BAL, if the target is in range.
    if (!parameters->options().relocatable()
        && !cross_mode_jump
        && ((jal_to_bal
            && r_type == elfcpp::R_MIPS_26
            && (val >> 26) == 0x3)))    // jal addr
      {
        Valtype32 dest = (x << 2) | (((address + 4) >> 28) << 28);
        int offset = dest - (address + 4);
        if (!Bits<18>::has_overflow32(offset))
          {
            if (val == 0x03200008)   // jr t9
              val = 0x10000000 | (((Valtype32)offset >> 2) & 0xffff);  // b addr
            else
              val = 0x04110000 | (((Valtype32)offset >> 2) & 0xffff); //bal addr
          }
      }

    if (calculate_only)
      *calculated_value = val;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_PC16
  static inline typename This::Status
  relpc16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address address,
          Mips_address addend_a, bool extract_addend, bool calculate_only,
          Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend
                      ? Bits<18>::sign_extend32((val & 0xffff) << 2)
                      : addend_a);

    Valtype x = psymval->value(object, addend) - address;
    val = Bits<16>::bit_select32(val, x >> 2, 0xffff);

    if (calculate_only)
      {
        *calculated_value = x >> 2;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    if (psymval->value(object, addend) & 3)
      return This::STATUS_PCREL_UNALIGNED;

    return check_overflow<18>(x);
  }

  // R_MIPS_PC21_S2
  static inline typename This::Status
  relpc21(unsigned char* view, const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address address,
          Mips_address addend_a, bool extract_addend, bool calculate_only,
          Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend
                      ? Bits<23>::sign_extend32((val & 0x1fffff) << 2)
                      : addend_a);

    Valtype x = psymval->value(object, addend) - address;
    val = Bits<21>::bit_select32(val, x >> 2, 0x1fffff);

    if (calculate_only)
      {
        *calculated_value = x >> 2;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    if (psymval->value(object, addend) & 3)
      return This::STATUS_PCREL_UNALIGNED;

    return check_overflow<23>(x);
  }

  // R_MIPS_PC26_S2
  static inline typename This::Status
  relpc26(unsigned char* view, const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address address,
          Mips_address addend_a, bool extract_addend, bool calculate_only,
          Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend
                      ? Bits<28>::sign_extend32((val & 0x3ffffff) << 2)
                      : addend_a);

    Valtype x = psymval->value(object, addend) - address;
    val = Bits<26>::bit_select32(val, x >> 2, 0x3ffffff);

    if (calculate_only)
      {
        *calculated_value = x >> 2;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    if (psymval->value(object, addend) & 3)
      return This::STATUS_PCREL_UNALIGNED;

    return check_overflow<28>(x);
  }

  // R_MIPS_PC18_S3
  static inline typename This::Status
  relpc18(unsigned char* view, const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address address,
          Mips_address addend_a, bool extract_addend, bool calculate_only,
          Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend
                      ? Bits<21>::sign_extend32((val & 0x3ffff) << 3)
                      : addend_a);

    Valtype x = psymval->value(object, addend) - ((address | 7) ^ 7);
    val = Bits<18>::bit_select32(val, x >> 3, 0x3ffff);

    if (calculate_only)
      {
        *calculated_value = x >> 3;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    if (psymval->value(object, addend) & 7)
      return This::STATUS_PCREL_UNALIGNED;

    return check_overflow<21>(x);
  }

  // R_MIPS_PC19_S2
  static inline typename This::Status
  relpc19(unsigned char* view, const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address address,
          Mips_address addend_a, bool extract_addend, bool calculate_only,
          Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend
                      ? Bits<21>::sign_extend32((val & 0x7ffff) << 2)
                      : addend_a);

    Valtype x = psymval->value(object, addend) - address;
    val = Bits<19>::bit_select32(val, x >> 2, 0x7ffff);

    if (calculate_only)
      {
        *calculated_value = x >> 2;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    if (psymval->value(object, addend) & 3)
      return This::STATUS_PCREL_UNALIGNED;

    return check_overflow<21>(x);
  }

  // R_MIPS_PCHI16
  static inline typename This::Status
  relpchi16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
            const Symbol_value<size>* psymval, Mips_address addend,
            Mips_address address, unsigned int r_sym, bool extract_addend)
  {
    // Record the relocation.  It will be resolved when we find pclo16 part.
    pchi16_relocs.push_back(reloc_high<size, big_endian>(view, object, psymval,
                            addend, 0, r_sym, extract_addend, address));
    return This::STATUS_OKAY;
  }

  // R_MIPS_PCHI16
  static inline typename This::Status
  do_relpchi16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
             const Symbol_value<size>* psymval, Mips_address addend_hi,
             Mips_address address, bool extract_addend, Valtype32 addend_lo,
             bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend ? ((val & 0xffff) << 16) + addend_lo
                                       : addend_hi);

    Valtype value = psymval->value(object, addend) - address;
    Valtype x = ((value + 0x8000) >> 16) & 0xffff;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_PCLO16
  static inline typename This::Status
  relpclo16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
            const Symbol_value<size>* psymval, Mips_address addend_a,
            bool extract_addend, Mips_address address, unsigned int r_sym,
            unsigned int rel_type, bool calculate_only,
            Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend ? Bits<16>::sign_extend32(val & 0xffff)
                                     : addend_a);

    if (rel_type == elfcpp::SHT_REL)
      {
        // Resolve pending R_MIPS_PCHI16 relocations.
        typename std::list<reloc_high<size, big_endian> >::iterator it =
            pchi16_relocs.begin();
        while (it != pchi16_relocs.end())
          {
            reloc_high<size, big_endian> pchi16 = *it;
            if (pchi16.r_sym == r_sym)
              {
                do_relpchi16(pchi16.view, pchi16.object, pchi16.psymval,
                             pchi16.addend, pchi16.address,
                             pchi16.extract_addend, addend, calculate_only,
                             calculated_value);
                it = pchi16_relocs.erase(it);
              }
            else
              ++it;
          }
      }

    // Resolve R_MIPS_PCLO16 relocation.
    Valtype x = psymval->value(object, addend) - address;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MICROMIPS_PC7_S1
  static inline typename This::Status
  relmicromips_pc7_s1(unsigned char* view,
                      const Mips_relobj<size, big_endian>* object,
                      const Symbol_value<size>* psymval, Mips_address address,
                      Mips_address addend_a, bool extract_addend,
                      bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = extract_addend ? Bits<8>::sign_extend32((val & 0x7f) << 1)
                                    : addend_a;

    Valtype x = psymval->value(object, addend) - address;
    val = Bits<16>::bit_select32(val, x >> 1, 0x7f);

    if (calculate_only)
      {
        *calculated_value = x >> 1;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return check_overflow<8>(x);
  }

  // R_MICROMIPS_PC10_S1
  static inline typename This::Status
  relmicromips_pc10_s1(unsigned char* view,
                       const Mips_relobj<size, big_endian>* object,
                       const Symbol_value<size>* psymval, Mips_address address,
                       Mips_address addend_a, bool extract_addend,
                       bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend
                      ? Bits<11>::sign_extend32((val & 0x3ff) << 1)
                      : addend_a);

    Valtype x = psymval->value(object, addend) - address;
    val = Bits<16>::bit_select32(val, x >> 1, 0x3ff);

    if (calculate_only)
      {
        *calculated_value = x >> 1;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return check_overflow<11>(x);
  }

  // R_MICROMIPS_PC16_S1
  static inline typename This::Status
  relmicromips_pc16_s1(unsigned char* view,
                       const Mips_relobj<size, big_endian>* object,
                       const Symbol_value<size>* psymval, Mips_address address,
                       Mips_address addend_a, bool extract_addend,
                       bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend
                      ? Bits<17>::sign_extend32((val & 0xffff) << 1)
                      : addend_a);

    Valtype x = psymval->value(object, addend) - address;
    val = Bits<16>::bit_select32(val, x >> 1, 0xffff);

    if (calculate_only)
      {
        *calculated_value = x >> 1;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return check_overflow<17>(x);
  }

  // R_MIPS_HI16, R_MIPS16_HI16, R_MICROMIPS_HI16,
  static inline typename This::Status
  relhi16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address addend,
          Mips_address address, bool gp_disp, unsigned int r_type,
          unsigned int r_sym, bool extract_addend)
  {
    // Record the relocation.  It will be resolved when we find lo16 part.
    hi16_relocs.push_back(reloc_high<size, big_endian>(view, object, psymval,
                          addend, r_type, r_sym, extract_addend, address,
                          gp_disp));
    return This::STATUS_OKAY;
  }

  // R_MIPS_HI16, R_MIPS16_HI16, R_MICROMIPS_HI16,
  static inline typename This::Status
  do_relhi16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
             const Symbol_value<size>* psymval, Mips_address addend_hi,
             Mips_address address, bool is_gp_disp, unsigned int r_type,
             bool extract_addend, Valtype32 addend_lo,
             Target_mips<size, big_endian>* target, bool calculate_only,
             Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend ? ((val & 0xffff) << 16) + addend_lo
                                       : addend_hi);

    Valtype32 value;
    if (!is_gp_disp)
      value = psymval->value(object, addend);
    else
      {
        // For MIPS16 ABI code we generate this sequence
        //    0: li      $v0,%hi(_gp_disp)
        //    4: addiupc $v1,%lo(_gp_disp)
        //    8: sll     $v0,16
        //   12: addu    $v0,$v1
        //   14: move    $gp,$v0
        // So the offsets of hi and lo relocs are the same, but the
        // base $pc is that used by the ADDIUPC instruction at $t9 + 4.
        // ADDIUPC clears the low two bits of the instruction address,
        // so the base is ($t9 + 4) & ~3.
        Valtype32 gp_disp;
        if (r_type == elfcpp::R_MIPS16_HI16)
          gp_disp = (target->adjusted_gp_value(object)
                     - ((address + 4) & ~0x3));
        // The microMIPS .cpload sequence uses the same assembly
        // instructions as the traditional psABI version, but the
        // incoming $t9 has the low bit set.
        else if (r_type == elfcpp::R_MICROMIPS_HI16)
          gp_disp = target->adjusted_gp_value(object) - address - 1;
        else
          gp_disp = target->adjusted_gp_value(object) - address;
        value = gp_disp + addend;
      }
    Valtype x = ((value + 0x8000) >> 16) & 0xffff;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      {
        *calculated_value = x;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return (is_gp_disp ? check_overflow<16>(x)
                       : This::STATUS_OKAY);
  }

  // R_MIPS_GOT16, R_MIPS16_GOT16, R_MICROMIPS_GOT16
  static inline typename This::Status
  relgot16_local(unsigned char* view,
                 const Mips_relobj<size, big_endian>* object,
                 const Symbol_value<size>* psymval, Mips_address addend_a,
                 bool extract_addend, unsigned int r_type, unsigned int r_sym)
  {
    // Record the relocation.  It will be resolved when we find lo16 part.
    got16_relocs.push_back(reloc_high<size, big_endian>(view, object, psymval,
                           addend_a, r_type, r_sym, extract_addend));
    return This::STATUS_OKAY;
  }

  // R_MIPS_GOT16, R_MIPS16_GOT16, R_MICROMIPS_GOT16
  static inline typename This::Status
  do_relgot16_local(unsigned char* view,
                    const Mips_relobj<size, big_endian>* object,
                    const Symbol_value<size>* psymval, Mips_address addend_hi,
                    bool extract_addend, Valtype32 addend_lo,
                    Target_mips<size, big_endian>* target, bool calculate_only,
                    Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend ? ((val & 0xffff) << 16) + addend_lo
                                       : addend_hi);

    // Find GOT page entry.
    Mips_address value = ((psymval->value(object, addend) + 0x8000) >> 16)
                          & 0xffff;
    value <<= 16;
    unsigned int got_offset =
      target->got_section()->get_got_page_offset(value, object);

    // Resolve the relocation.
    Valtype x = target->got_section()->gp_offset(got_offset, object);
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      {
        *calculated_value = x;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return check_overflow<16>(x);
  }

  // R_MIPS_LO16, R_MIPS16_LO16, R_MICROMIPS_LO16, R_MICROMIPS_HI0_LO16
  static inline typename This::Status
  rello16(Target_mips<size, big_endian>* target, unsigned char* view,
          const Mips_relobj<size, big_endian>* object,
          const Symbol_value<size>* psymval, Mips_address addend_a,
          bool extract_addend, Mips_address address, bool is_gp_disp,
          unsigned int r_type, unsigned int r_sym, unsigned int rel_type,
          bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend = (extract_addend ? Bits<16>::sign_extend32(val & 0xffff)
                                     : addend_a);

    if (rel_type == elfcpp::SHT_REL)
      {
        typename This::Status reloc_status = This::STATUS_OKAY;
        // Resolve pending R_MIPS_HI16 relocations.
        typename std::list<reloc_high<size, big_endian> >::iterator it =
          hi16_relocs.begin();
        while (it != hi16_relocs.end())
          {
            reloc_high<size, big_endian> hi16 = *it;
            if (hi16.r_sym == r_sym
                && is_matching_lo16_reloc(hi16.r_type, r_type))
              {
                mips_reloc_unshuffle(hi16.view, hi16.r_type, false);
                reloc_status = do_relhi16(hi16.view, hi16.object, hi16.psymval,
                                       hi16.addend, hi16.address, hi16.gp_disp,
                                       hi16.r_type, hi16.extract_addend, addend,
                                       target, calculate_only, calculated_value);
                mips_reloc_shuffle(hi16.view, hi16.r_type, false);
                if (reloc_status == This::STATUS_OVERFLOW)
                  return This::STATUS_OVERFLOW;
                it = hi16_relocs.erase(it);
              }
            else
              ++it;
          }

        // Resolve pending local R_MIPS_GOT16 relocations.
        typename std::list<reloc_high<size, big_endian> >::iterator it2 =
          got16_relocs.begin();
        while (it2 != got16_relocs.end())
          {
            reloc_high<size, big_endian> got16 = *it2;
            if (got16.r_sym == r_sym
                && is_matching_lo16_reloc(got16.r_type, r_type))
              {
                mips_reloc_unshuffle(got16.view, got16.r_type, false);

                reloc_status = do_relgot16_local(got16.view, got16.object,
                                     got16.psymval, got16.addend,
                                     got16.extract_addend, addend, target,
                                     calculate_only, calculated_value);

                mips_reloc_shuffle(got16.view, got16.r_type, false);
                if (reloc_status == This::STATUS_OVERFLOW)
                  return This::STATUS_OVERFLOW;
                it2 = got16_relocs.erase(it2);
              }
            else
              ++it2;
          }
      }

    // Resolve R_MIPS_LO16 relocation.
    Valtype x;
    if (!is_gp_disp)
      x = psymval->value(object, addend);
    else
      {
        // See the comment for R_MIPS16_HI16 above for the reason
        // for this conditional.
        Valtype32 gp_disp;
        if (r_type == elfcpp::R_MIPS16_LO16)
          gp_disp = target->adjusted_gp_value(object) - (address & ~0x3);
        else if (r_type == elfcpp::R_MICROMIPS_LO16
                 || r_type == elfcpp::R_MICROMIPS_HI0_LO16)
          gp_disp = target->adjusted_gp_value(object) - address + 3;
        else
          gp_disp = target->adjusted_gp_value(object) - address + 4;
        // The MIPS ABI requires checking the R_MIPS_LO16 relocation
        // for overflow.  Relocations against _gp_disp are normally
        // generated from the .cpload pseudo-op.  It generates code
        // that normally looks like this:

        //   lui    $gp,%hi(_gp_disp)
        //   addiu  $gp,$gp,%lo(_gp_disp)
        //   addu   $gp,$gp,$t9

        // Here $t9 holds the address of the function being called,
        // as required by the MIPS ELF ABI.  The R_MIPS_LO16
        // relocation can easily overflow in this situation, but the
        // R_MIPS_HI16 relocation will handle the overflow.
        // Therefore, we consider this a bug in the MIPS ABI, and do
        // not check for overflow here.
        x = gp_disp + addend;
      }
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_CALL16, R_MIPS16_CALL16, R_MICROMIPS_CALL16
  // R_MIPS_GOT16, R_MIPS16_GOT16, R_MICROMIPS_GOT16
  // R_MIPS_TLS_GD, R_MIPS16_TLS_GD, R_MICROMIPS_TLS_GD
  // R_MIPS_TLS_GOTTPREL, R_MIPS16_TLS_GOTTPREL, R_MICROMIPS_TLS_GOTTPREL
  // R_MIPS_TLS_LDM, R_MIPS16_TLS_LDM, R_MICROMIPS_TLS_LDM
  // R_MIPS_GOT_DISP, R_MICROMIPS_GOT_DISP
  static inline typename This::Status
  relgot(unsigned char* view, int gp_offset, bool calculate_only,
         Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype x = gp_offset;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      {
        *calculated_value = x;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return check_overflow<16>(x);
  }

  // R_MIPS_EH
  static inline typename This::Status
  releh(unsigned char* view, int gp_offset, bool calculate_only,
        Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype x = gp_offset;

    if (calculate_only)
      {
        *calculated_value = x;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, x);

    return check_overflow<32>(x);
  }

  // R_MIPS_GOT_PAGE, R_MICROMIPS_GOT_PAGE
  static inline typename This::Status
  relgotpage(Target_mips<size, big_endian>* target, unsigned char* view,
             const Mips_relobj<size, big_endian>* object,
             const Symbol_value<size>* psymval, Mips_address addend_a,
             bool extract_addend, bool calculate_only,
             Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(view);
    Valtype addend = extract_addend ? val & 0xffff : addend_a;

    // Find a GOT page entry that points to within 32KB of symbol + addend.
    Mips_address value = (psymval->value(object, addend) + 0x8000) & ~0xffff;
    unsigned int  got_offset =
      target->got_section()->get_got_page_offset(value, object);

    Valtype x = target->got_section()->gp_offset(got_offset, object);
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      {
        *calculated_value = x;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return check_overflow<16>(x);
  }

  // R_MIPS_GOT_OFST, R_MICROMIPS_GOT_OFST
  static inline typename This::Status
  relgotofst(Target_mips<size, big_endian>* target, unsigned char* view,
             const Mips_relobj<size, big_endian>* object,
             const Symbol_value<size>* psymval, Mips_address addend_a,
             bool extract_addend, bool local, bool calculate_only,
             Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(view);
    Valtype addend = extract_addend ? val & 0xffff : addend_a;

    // For a local symbol, find a GOT page entry that points to within 32KB of
    // symbol + addend.  Relocation value is the offset of the GOT page entry's
    // value from symbol + addend.
    // For a global symbol, relocation value is addend.
    Valtype x;
    if (local)
      {
        // Find GOT page entry.
        Mips_address value = ((psymval->value(object, addend) + 0x8000)
                              & ~0xffff);
        target->got_section()->get_got_page_offset(value, object);

        x = psymval->value(object, addend) - value;
      }
    else
      x = addend;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      {
        *calculated_value = x;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return check_overflow<16>(x);
  }

  // R_MIPS_GOT_HI16, R_MIPS_CALL_HI16,
  // R_MICROMIPS_GOT_HI16, R_MICROMIPS_CALL_HI16
  static inline typename This::Status
  relgot_hi16(unsigned char* view, int gp_offset, bool calculate_only,
              Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype x = gp_offset;
    x = ((x + 0x8000) >> 16) & 0xffff;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_GOT_LO16, R_MIPS_CALL_LO16,
  // R_MICROMIPS_GOT_LO16, R_MICROMIPS_CALL_LO16
  static inline typename This::Status
  relgot_lo16(unsigned char* view, int gp_offset, bool calculate_only,
              Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype x = gp_offset;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_GPREL16, R_MIPS16_GPREL, R_MIPS_LITERAL, R_MICROMIPS_LITERAL
  // R_MICROMIPS_GPREL7_S2, R_MICROMIPS_GPREL16
  static inline typename This::Status
  relgprel(unsigned char* view, const Mips_relobj<size, big_endian>* object,
           const Symbol_value<size>* psymval, Mips_address gp,
           Mips_address addend_a, bool extract_addend, bool local,
           unsigned int r_type, bool calculate_only,
           Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);

    Valtype addend;
    if (extract_addend)
      {
        if (r_type == elfcpp::R_MICROMIPS_GPREL7_S2)
          addend = (val & 0x7f) << 2;
        else
          addend = val & 0xffff;
        // Only sign-extend the addend if it was extracted from the
        // instruction.  If the addend was separate, leave it alone,
        // otherwise we may lose significant bits.
        addend = Bits<16>::sign_extend32(addend);
      }
    else
      addend = addend_a;

    Valtype x = psymval->value(object, addend) - gp;

    // If the symbol was local, any earlier relocatable links will
    // have adjusted its addend with the gp offset, so compensate
    // for that now.  Don't do it for symbols forced local in this
    // link, though, since they won't have had the gp offset applied
    // to them before.
    if (local)
      x += object->gp_value();

    if (r_type == elfcpp::R_MICROMIPS_GPREL7_S2)
      val = Bits<32>::bit_select32(val, x, 0x7f);
    else
      val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      {
        *calculated_value = x;
        return This::STATUS_OKAY;
      }
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    if (check_overflow<16>(x) == This::STATUS_OVERFLOW)
      {
        gold_error(_("small-data section exceeds 64KB; lower small-data size "
                     "limit (see option -G)"));
        return This::STATUS_OVERFLOW;
      }
    return This::STATUS_OKAY;
  }

  // R_MIPS_GPREL32
  static inline typename This::Status
  relgprel32(unsigned char* view, const Mips_relobj<size, big_endian>* object,
             const Symbol_value<size>* psymval, Mips_address gp,
             Mips_address addend_a, bool extract_addend, bool calculate_only,
             Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype addend = extract_addend ? val : addend_a;

    // R_MIPS_GPREL32 relocations are defined for local symbols only.
    Valtype x = psymval->value(object, addend) + object->gp_value() - gp;

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, x);

    return This::STATUS_OKAY;
 }

  // R_MIPS_TLS_TPREL_HI16, R_MIPS16_TLS_TPREL_HI16, R_MICROMIPS_TLS_TPREL_HI16
  // R_MIPS_TLS_DTPREL_HI16, R_MIPS16_TLS_DTPREL_HI16,
  // R_MICROMIPS_TLS_DTPREL_HI16
  static inline typename This::Status
  tlsrelhi16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
             const Symbol_value<size>* psymval, Valtype32 tp_offset,
             Mips_address addend_a, bool extract_addend, bool calculate_only,
             Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype addend = extract_addend ? val & 0xffff : addend_a;

    // tls symbol values are relative to tls_segment()->vaddr()
    Valtype x = ((psymval->value(object, addend) - tp_offset) + 0x8000) >> 16;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_TLS_TPREL_LO16, R_MIPS16_TLS_TPREL_LO16, R_MICROMIPS_TLS_TPREL_LO16,
  // R_MIPS_TLS_DTPREL_LO16, R_MIPS16_TLS_DTPREL_LO16,
  // R_MICROMIPS_TLS_DTPREL_LO16,
  static inline typename This::Status
  tlsrello16(unsigned char* view, const Mips_relobj<size, big_endian>* object,
             const Symbol_value<size>* psymval, Valtype32 tp_offset,
             Mips_address addend_a, bool extract_addend, bool calculate_only,
             Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype addend = extract_addend ? val & 0xffff : addend_a;

    // tls symbol values are relative to tls_segment()->vaddr()
    Valtype x = psymval->value(object, addend) - tp_offset;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_TLS_TPREL32, R_MIPS_TLS_TPREL64,
  // R_MIPS_TLS_DTPREL32, R_MIPS_TLS_DTPREL64
  static inline typename This::Status
  tlsrel32(unsigned char* view, const Mips_relobj<size, big_endian>* object,
           const Symbol_value<size>* psymval, Valtype32 tp_offset,
           Mips_address addend_a, bool extract_addend, bool calculate_only,
           Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype addend = extract_addend ? val : addend_a;

    // tls symbol values are relative to tls_segment()->vaddr()
    Valtype x = psymval->value(object, addend) - tp_offset;

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, x);

    return This::STATUS_OKAY;
  }

  // R_MIPS_SUB, R_MICROMIPS_SUB
  static inline typename This::Status
  relsub(unsigned char* view, const Mips_relobj<size, big_endian>* object,
         const Symbol_value<size>* psymval, Mips_address addend_a,
         bool extract_addend, bool calculate_only, Valtype* calculated_value)
  {
    Valtype64* wv = reinterpret_cast<Valtype64*>(view);
    Valtype64 addend = (extract_addend
                        ? elfcpp::Swap<64, big_endian>::readval(wv)
                        : addend_a);

    Valtype64 x = psymval->value(object, -addend);
    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<64, big_endian>::writeval(wv, x);

    return This::STATUS_OKAY;
  }

  // R_MIPS_64: S + A
  static inline typename This::Status
  rel64(unsigned char* view, const Mips_relobj<size, big_endian>* object,
        const Symbol_value<size>* psymval, Mips_address addend_a,
        bool extract_addend, bool calculate_only, Valtype* calculated_value,
        bool apply_addend_only)
  {
    Valtype64* wv = reinterpret_cast<Valtype64*>(view);
    Valtype64 addend = (extract_addend
                        ? elfcpp::Swap<64, big_endian>::readval(wv)
                        : addend_a);

    Valtype64 x = psymval->value(object, addend);
    if (calculate_only)
      *calculated_value = x;
    else
      {
        if (apply_addend_only)
          x = addend;
        elfcpp::Swap<64, big_endian>::writeval(wv, x);
      }

    return This::STATUS_OKAY;
  }

  // R_MIPS_HIGHER, R_MICROMIPS_HIGHER
  static inline typename This::Status
  relhigher(unsigned char* view, const Mips_relobj<size, big_endian>* object,
            const Symbol_value<size>* psymval, Mips_address addend_a,
            bool extract_addend, bool calculate_only, Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype addend = (extract_addend ? Bits<16>::sign_extend32(val & 0xffff)
                                     : addend_a);

    Valtype x = psymval->value(object, addend);
    x = ((x + (uint64_t) 0x80008000) >> 32) & 0xffff;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }

  // R_MIPS_HIGHEST, R_MICROMIPS_HIGHEST
  static inline typename This::Status
  relhighest(unsigned char* view, const Mips_relobj<size, big_endian>* object,
             const Symbol_value<size>* psymval, Mips_address addend_a,
             bool extract_addend, bool calculate_only,
             Valtype* calculated_value)
  {
    Valtype32* wv = reinterpret_cast<Valtype32*>(view);
    Valtype32 val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype addend = (extract_addend ? Bits<16>::sign_extend32(val & 0xffff)
                                     : addend_a);

    Valtype x = psymval->value(object, addend);
    x = ((x + (uint64_t) 0x800080008000llu) >> 48) & 0xffff;
    val = Bits<32>::bit_select32(val, x, 0xffff);

    if (calculate_only)
      *calculated_value = x;
    else
      elfcpp::Swap<32, big_endian>::writeval(wv, val);

    return This::STATUS_OKAY;
  }
};

template<int size, bool big_endian>
typename std::list<reloc_high<size, big_endian> >
    Mips_relocate_functions<size, big_endian>::hi16_relocs;

template<int size, bool big_endian>
typename std::list<reloc_high<size, big_endian> >
    Mips_relocate_functions<size, big_endian>::got16_relocs;

template<int size, bool big_endian>
typename std::list<reloc_high<size, big_endian> >
    Mips_relocate_functions<size, big_endian>::pchi16_relocs;

// Mips_got_info methods.

// Reserve GOT entry for a GOT relocation of type R_TYPE against symbol
// SYMNDX + ADDEND, where SYMNDX is a local symbol in section SHNDX in OBJECT.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::record_local_got_symbol(
    Mips_relobj<size, big_endian>* object, unsigned int symndx,
    Mips_address addend, unsigned int r_type, unsigned int shndx,
    bool is_section_symbol)
{
  Mips_got_entry<size, big_endian>* entry =
    new Mips_got_entry<size, big_endian>(object, symndx, addend,
                                         mips_elf_reloc_tls_type(r_type),
                                         shndx, is_section_symbol);
  this->record_got_entry(entry, object);
}

// Reserve GOT entry for a GOT relocation of type R_TYPE against MIPS_SYM,
// in OBJECT.  FOR_CALL is true if the caller is only interested in
// using the GOT entry for calls.  DYN_RELOC is true if R_TYPE is a dynamic
// relocation.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::record_global_got_symbol(
    Mips_symbol<size>* mips_sym, Mips_relobj<size, big_endian>* object,
    unsigned int r_type, bool dyn_reloc, bool for_call)
{
  if (!for_call)
    mips_sym->set_got_not_only_for_calls();

  // A global symbol in the GOT must also be in the dynamic symbol table.
  if (!mips_sym->needs_dynsym_entry() && !mips_sym->is_forced_local())
    {
      switch (mips_sym->visibility())
        {
        case elfcpp::STV_INTERNAL:
        case elfcpp::STV_HIDDEN:
          mips_sym->set_is_forced_local();
          break;
        default:
          mips_sym->set_needs_dynsym_entry();
          break;
        }
    }

  unsigned char tls_type = mips_elf_reloc_tls_type(r_type);
  if (tls_type == GOT_TLS_NONE)
    this->global_got_symbols_.insert(mips_sym);

  if (dyn_reloc)
    {
      if (mips_sym->global_got_area() == GGA_NONE)
        mips_sym->set_global_got_area(GGA_RELOC_ONLY);
      return;
    }

  Mips_got_entry<size, big_endian>* entry =
    new Mips_got_entry<size, big_endian>(mips_sym, tls_type);

  this->record_got_entry(entry, object);
}

// Add ENTRY to master GOT and to OBJECT's GOT.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::record_got_entry(
    Mips_got_entry<size, big_endian>* entry,
    Mips_relobj<size, big_endian>* object)
{
  this->got_entries_.insert(entry);

  // Create the GOT entry for the OBJECT's GOT.
  Mips_got_info<size, big_endian>* g = object->get_or_create_got_info();
  Mips_got_entry<size, big_endian>* entry2 =
    new Mips_got_entry<size, big_endian>(*entry);

  g->got_entries_.insert(entry2);
}

// Record that OBJECT has a page relocation against symbol SYMNDX and
// that ADDEND is the addend for that relocation.
// This function creates an upper bound on the number of GOT slots
// required; no attempt is made to combine references to non-overridable
// global symbols across multiple input files.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::record_got_page_entry(
    Mips_relobj<size, big_endian>* object, unsigned int symndx, int addend)
{
  struct Got_page_range **range_ptr, *range;
  int old_pages, new_pages;

  // Find the Got_page_entry for this symbol.
  Got_page_entry* entry = new Got_page_entry(object, symndx);
  typename Got_page_entry_set::iterator it =
    this->got_page_entries_.find(entry);
  if (it != this->got_page_entries_.end())
    entry = *it;
  else
    this->got_page_entries_.insert(entry);

  // Get the object's GOT, but we don't need to insert an entry here.
  Mips_got_info<size, big_endian>* g2 = object->get_or_create_got_info();

  // Skip over ranges whose maximum extent cannot share a page entry
  // with ADDEND.
  range_ptr = &entry->ranges;
  while (*range_ptr && addend > (*range_ptr)->max_addend + 0xffff)
    range_ptr = &(*range_ptr)->next;

  // If we scanned to the end of the list, or found a range whose
  // minimum extent cannot share a page entry with ADDEND, create
  // a new singleton range.
  range = *range_ptr;
  if (!range || addend < range->min_addend - 0xffff)
    {
      range = new Got_page_range();
      range->next = *range_ptr;
      range->min_addend = addend;
      range->max_addend = addend;

      *range_ptr = range;
      ++this->page_gotno_;
      ++g2->page_gotno_;
      return;
    }

  // Remember how many pages the old range contributed.
  old_pages = range->get_max_pages();

  // Update the ranges.
  if (addend < range->min_addend)
    range->min_addend = addend;
  else if (addend > range->max_addend)
    {
      if (range->next && addend >= range->next->min_addend - 0xffff)
        {
          old_pages += range->next->get_max_pages();
          range->max_addend = range->next->max_addend;
          range->next = range->next->next;
        }
      else
        range->max_addend = addend;
    }

  // Record any change in the total estimate.
  new_pages = range->get_max_pages();
  if (old_pages != new_pages)
    {
      this->page_gotno_ += new_pages - old_pages;
      g2->page_gotno_ += new_pages - old_pages;
    }
}

// Create all entries that should be in the local part of the GOT.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::add_local_entries(
    Target_mips<size, big_endian>* target, Layout* layout)
{
  Mips_output_data_got<size, big_endian>* got = target->got_section();
  // First two GOT entries are reserved.  The first entry will be filled at
  // runtime.  The second entry will be used by some runtime loaders.
  got->add_constant(0);
  got->add_constant(target->mips_elf_gnu_got1_mask());

  for (typename Got_entry_set::iterator
       p = this->got_entries_.begin();
       p != this->got_entries_.end();
       ++p)
    {
      Mips_got_entry<size, big_endian>* entry = *p;
      if (entry->is_for_local_symbol() && !entry->is_tls_entry())
        {
          got->add_local(entry->object(), entry->symndx(),
                         GOT_TYPE_STANDARD, entry->addend());
          unsigned int got_offset = entry->object()->local_got_offset(
              entry->symndx(), GOT_TYPE_STANDARD, entry->addend());
          if (got->multi_got() && this->index_ > 0
              && parameters->options().output_is_position_independent())
          {
            if (!entry->is_section_symbol())
              target->rel_dyn_section(layout)->add_local(entry->object(),
                  entry->symndx(), elfcpp::R_MIPS_REL32, got, got_offset);
            else
              target->rel_dyn_section(layout)->add_symbolless_local_addend(
                  entry->object(), entry->symndx(), elfcpp::R_MIPS_REL32,
                  got, got_offset);
          }
        }
    }

  this->add_page_entries(target, layout);

  // Add global entries that should be in the local area.
  for (typename Got_entry_set::iterator
       p = this->got_entries_.begin();
       p != this->got_entries_.end();
       ++p)
    {
      Mips_got_entry<size, big_endian>* entry = *p;
      if (!entry->is_for_global_symbol())
        continue;

      Mips_symbol<size>* mips_sym = entry->sym();
      if (mips_sym->global_got_area() == GGA_NONE && !entry->is_tls_entry())
        {
          unsigned int got_type;
          if (!got->multi_got())
            got_type = GOT_TYPE_STANDARD;
          else
            got_type = GOT_TYPE_STANDARD_MULTIGOT + this->index_;
          if (got->add_global(mips_sym, got_type))
            {
              mips_sym->set_global_gotoffset(mips_sym->got_offset(got_type));
              if (got->multi_got() && this->index_ > 0
                  && parameters->options().output_is_position_independent())
                target->rel_dyn_section(layout)->add_symbolless_global_addend(
                    mips_sym, elfcpp::R_MIPS_REL32, got,
                    mips_sym->got_offset(got_type));
            }
        }
    }
}

// Create GOT page entries.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::add_page_entries(
    Target_mips<size, big_endian>* target, Layout* layout)
{
  if (this->page_gotno_ == 0)
    return;

  Mips_output_data_got<size, big_endian>* got = target->got_section();
  this->got_page_offset_start_ = got->add_constant(0);
  if (got->multi_got() && this->index_ > 0
      && parameters->options().output_is_position_independent())
    target->rel_dyn_section(layout)->add_absolute(elfcpp::R_MIPS_REL32, got,
                                                  this->got_page_offset_start_);
  int num_entries = this->page_gotno_;
  unsigned int prev_offset = this->got_page_offset_start_;
  while (--num_entries > 0)
    {
      unsigned int next_offset = got->add_constant(0);
      if (got->multi_got() && this->index_ > 0
          && parameters->options().output_is_position_independent())
        target->rel_dyn_section(layout)->add_absolute(elfcpp::R_MIPS_REL32, got,
                                                      next_offset);
      gold_assert(next_offset == prev_offset + size/8);
      prev_offset = next_offset;
    }
  this->got_page_offset_next_ = this->got_page_offset_start_;
}

// Create global GOT entries, both GGA_NORMAL and GGA_RELOC_ONLY.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::add_global_entries(
    Target_mips<size, big_endian>* target, Layout* layout,
    unsigned int non_reloc_only_global_gotno)
{
  Mips_output_data_got<size, big_endian>* got = target->got_section();
  // Add GGA_NORMAL entries.
  unsigned int count = 0;
  for (typename Got_entry_set::iterator
       p = this->got_entries_.begin();
       p != this->got_entries_.end();
       ++p)
    {
      Mips_got_entry<size, big_endian>* entry = *p;
      if (!entry->is_for_global_symbol())
        continue;

      Mips_symbol<size>* mips_sym = entry->sym();
      if (mips_sym->global_got_area() != GGA_NORMAL)
        continue;

      unsigned int got_type;
      if (!got->multi_got())
        got_type = GOT_TYPE_STANDARD;
      else
        // In multi-GOT links, global symbol can be in both primary and
        // secondary GOT(s).  By creating custom GOT type
        // (GOT_TYPE_STANDARD_MULTIGOT + got_index) we ensure that symbol
        // is added to secondary GOT(s).
        got_type = GOT_TYPE_STANDARD_MULTIGOT + this->index_;
      if (!got->add_global(mips_sym, got_type))
        continue;

      mips_sym->set_global_gotoffset(mips_sym->got_offset(got_type));
      if (got->multi_got() && this->index_ == 0)
        count++;
      if (got->multi_got() && this->index_ > 0)
        {
          if (parameters->options().output_is_position_independent()
              || (!parameters->doing_static_link()
                  && mips_sym->is_from_dynobj() && !mips_sym->is_undefined()))
            {
              target->rel_dyn_section(layout)->add_global(
                  mips_sym, elfcpp::R_MIPS_REL32, got,
                  mips_sym->got_offset(got_type));
              got->add_secondary_got_reloc(mips_sym->got_offset(got_type),
                                           elfcpp::R_MIPS_REL32, mips_sym);
            }
        }
    }

  if (!got->multi_got() || this->index_ == 0)
    {
      if (got->multi_got())
        {
          // We need to allocate space in the primary GOT for GGA_NORMAL entries
          // of secondary GOTs, to ensure that GOT offsets of GGA_RELOC_ONLY
          // entries correspond to dynamic symbol indexes.
          while (count < non_reloc_only_global_gotno)
            {
              got->add_constant(0);
              ++count;
            }
        }

      // Add GGA_RELOC_ONLY entries.
      got->add_reloc_only_entries();
    }
}

// Create global GOT entries that should be in the GGA_RELOC_ONLY area.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::add_reloc_only_entries(
    Mips_output_data_got<size, big_endian>* got)
{
  for (typename Global_got_entry_set::iterator
       p = this->global_got_symbols_.begin();
       p != this->global_got_symbols_.end();
       ++p)
    {
      Mips_symbol<size>* mips_sym = *p;
      if (mips_sym->global_got_area() == GGA_RELOC_ONLY)
        {
          unsigned int got_type;
          if (!got->multi_got())
            got_type = GOT_TYPE_STANDARD;
          else
            got_type = GOT_TYPE_STANDARD_MULTIGOT;
          if (got->add_global(mips_sym, got_type))
            mips_sym->set_global_gotoffset(mips_sym->got_offset(got_type));
        }
    }
}

// Create TLS GOT entries.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::add_tls_entries(
    Target_mips<size, big_endian>* target, Layout* layout)
{
  Mips_output_data_got<size, big_endian>* got = target->got_section();
  // Add local tls entries.
  for (typename Got_entry_set::iterator
       p = this->got_entries_.begin();
       p != this->got_entries_.end();
       ++p)
    {
      Mips_got_entry<size, big_endian>* entry = *p;
      if (!entry->is_tls_entry() || !entry->is_for_local_symbol())
        continue;

      if (entry->tls_type() == GOT_TLS_GD)
        {
          unsigned int got_type = GOT_TYPE_TLS_PAIR;
          unsigned int r_type1 = (size == 32 ? elfcpp::R_MIPS_TLS_DTPMOD32
                                             : elfcpp::R_MIPS_TLS_DTPMOD64);
          unsigned int r_type2 = (size == 32 ? elfcpp::R_MIPS_TLS_DTPREL32
                                             : elfcpp::R_MIPS_TLS_DTPREL64);

          if (!parameters->doing_static_link())
            {
              got->add_local_pair_with_rel(entry->object(), entry->symndx(),
                                           entry->shndx(), got_type,
                                           target->rel_dyn_section(layout),
                                           r_type1, entry->addend());
              unsigned int got_offset =
                entry->object()->local_got_offset(entry->symndx(), got_type,
                                                  entry->addend());
              got->add_static_reloc(got_offset + size/8, r_type2,
                                    entry->object(), entry->symndx());
            }
          else
            {
              // We are doing a static link.  Mark it as belong to module 1,
              // the executable.
              unsigned int got_offset = got->add_constant(1);
              entry->object()->set_local_got_offset(entry->symndx(), got_type,
                                                    got_offset,
                                                    entry->addend());
              got->add_constant(0);
              got->add_static_reloc(got_offset + size/8, r_type2,
                                    entry->object(), entry->symndx());
            }
        }
      else if (entry->tls_type() == GOT_TLS_IE)
        {
          unsigned int got_type = GOT_TYPE_TLS_OFFSET;
          unsigned int r_type = (size == 32 ? elfcpp::R_MIPS_TLS_TPREL32
                                            : elfcpp::R_MIPS_TLS_TPREL64);
          if (!parameters->doing_static_link())
            got->add_local_with_rel(entry->object(), entry->symndx(), got_type,
                                    target->rel_dyn_section(layout), r_type,
                                    entry->addend());
          else
            {
              got->add_local(entry->object(), entry->symndx(), got_type,
                             entry->addend());
              unsigned int got_offset =
                  entry->object()->local_got_offset(entry->symndx(), got_type,
                                                    entry->addend());
              got->add_static_reloc(got_offset, r_type, entry->object(),
                                    entry->symndx());
            }
        }
      else if (entry->tls_type() == GOT_TLS_LDM)
        {
          unsigned int r_type = (size == 32 ? elfcpp::R_MIPS_TLS_DTPMOD32
                                            : elfcpp::R_MIPS_TLS_DTPMOD64);
          unsigned int got_offset;
          if (!parameters->doing_static_link())
            {
              got_offset = got->add_constant(0);
              target->rel_dyn_section(layout)->add_local(
                  entry->object(), 0, r_type, got, got_offset);
            }
          else
            // We are doing a static link.  Just mark it as belong to module 1,
            // the executable.
            got_offset = got->add_constant(1);

          got->add_constant(0);
          got->set_tls_ldm_offset(got_offset, entry->object());
        }
      else
        gold_unreachable();
    }

  // Add global tls entries.
  for (typename Got_entry_set::iterator
       p = this->got_entries_.begin();
       p != this->got_entries_.end();
       ++p)
    {
      Mips_got_entry<size, big_endian>* entry = *p;
      if (!entry->is_tls_entry() || !entry->is_for_global_symbol())
        continue;

      Mips_symbol<size>* mips_sym = entry->sym();
      if (entry->tls_type() == GOT_TLS_GD)
        {
          unsigned int got_type;
          if (!got->multi_got())
            got_type = GOT_TYPE_TLS_PAIR;
          else
            got_type = GOT_TYPE_TLS_PAIR_MULTIGOT + this->index_;
          unsigned int r_type1 = (size == 32 ? elfcpp::R_MIPS_TLS_DTPMOD32
                                             : elfcpp::R_MIPS_TLS_DTPMOD64);
          unsigned int r_type2 = (size == 32 ? elfcpp::R_MIPS_TLS_DTPREL32
                                             : elfcpp::R_MIPS_TLS_DTPREL64);
          if (!parameters->doing_static_link())
            got->add_global_pair_with_rel(mips_sym, got_type,
                             target->rel_dyn_section(layout), r_type1, r_type2);
          else
            {
              // Add a GOT pair for for R_MIPS_TLS_GD.  The creates a pair of
              // GOT entries.  The first one is initialized to be 1, which is the
              // module index for the main executable and the second one 0.  A
              // reloc of the type R_MIPS_TLS_DTPREL32/64 will be created for
              // the second GOT entry and will be applied by gold.
              unsigned int got_offset = got->add_constant(1);
              mips_sym->set_got_offset(got_type, got_offset);
              got->add_constant(0);
              got->add_static_reloc(got_offset + size/8, r_type2, mips_sym);
            }
        }
      else if (entry->tls_type() == GOT_TLS_IE)
        {
          unsigned int got_type;
          if (!got->multi_got())
            got_type = GOT_TYPE_TLS_OFFSET;
          else
            got_type = GOT_TYPE_TLS_OFFSET_MULTIGOT + this->index_;
          unsigned int r_type = (size == 32 ? elfcpp::R_MIPS_TLS_TPREL32
                                            : elfcpp::R_MIPS_TLS_TPREL64);
          if (!parameters->doing_static_link())
            got->add_global_with_rel(mips_sym, got_type,
                                     target->rel_dyn_section(layout), r_type);
          else
            {
              got->add_global(mips_sym, got_type);
              unsigned int got_offset = mips_sym->got_offset(got_type);
              got->add_static_reloc(got_offset, r_type, mips_sym);
            }
        }
      else
        gold_unreachable();
    }
}

// Decide whether the symbol needs an entry in the global part of the primary
// GOT, setting global_got_area accordingly.  Count the number of global
// symbols that are in the primary GOT only because they have dynamic
// relocations R_MIPS_REL32 against them (reloc_only_gotno).

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::count_got_symbols(Symbol_table* symtab)
{
  for (typename Global_got_entry_set::iterator
       p = this->global_got_symbols_.begin();
       p != this->global_got_symbols_.end();
       ++p)
    {
      Mips_symbol<size>* sym = *p;
      // Make a final decision about whether the symbol belongs in the
      // local or global GOT.  Symbols that bind locally can (and in the
      // case of forced-local symbols, must) live in the local GOT.
      // Those that are aren't in the dynamic symbol table must also
      // live in the local GOT.

      if (!sym->should_add_dynsym_entry(symtab)
          || (sym->got_only_for_calls()
              ? symbol_calls_local(sym, sym->should_add_dynsym_entry(symtab))
              : symbol_references_local(sym,
                                        sym->should_add_dynsym_entry(symtab))))
        // The symbol belongs in the local GOT.  We no longer need this
        // entry if it was only used for relocations; those relocations
        // will be against the null or section symbol instead.
        sym->set_global_got_area(GGA_NONE);
      else if (sym->global_got_area() == GGA_RELOC_ONLY)
        {
          ++this->reloc_only_gotno_;
          ++this->global_gotno_ ;
        }
    }
}

// Return the offset of GOT page entry for VALUE.  Initialize the entry with
// VALUE if it is not initialized.

template<int size, bool big_endian>
unsigned int
Mips_got_info<size, big_endian>::get_got_page_offset(Mips_address value,
    Mips_output_data_got<size, big_endian>* got)
{
  typename Got_page_offsets::iterator it = this->got_page_offsets_.find(value);
  if (it != this->got_page_offsets_.end())
    return it->second;

  gold_assert(this->got_page_offset_next_ < this->got_page_offset_start_
              + (size/8) * this->page_gotno_);

  unsigned int got_offset = this->got_page_offset_next_;
  this->got_page_offsets_[value] = got_offset;
  this->got_page_offset_next_ += size/8;
  got->update_got_entry(got_offset, value);
  return got_offset;
}

// Remove lazy-binding stubs for global symbols in this GOT.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::remove_lazy_stubs(
    Target_mips<size, big_endian>* target)
{
  for (typename Got_entry_set::iterator
       p = this->got_entries_.begin();
       p != this->got_entries_.end();
       ++p)
    {
      Mips_got_entry<size, big_endian>* entry = *p;
      if (entry->is_for_global_symbol())
        target->remove_lazy_stub_entry(entry->sym());
    }
}

// Count the number of GOT entries required.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::count_got_entries()
{
  for (typename Got_entry_set::iterator
       p = this->got_entries_.begin();
       p != this->got_entries_.end();
       ++p)
    {
      this->count_got_entry(*p);
    }
}

// Count the number of GOT entries required by ENTRY.  Accumulate the result.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::count_got_entry(
    Mips_got_entry<size, big_endian>* entry)
{
  if (entry->is_tls_entry())
    this->tls_gotno_ += mips_tls_got_entries(entry->tls_type());
  else if (entry->is_for_local_symbol()
           || entry->sym()->global_got_area() == GGA_NONE)
    ++this->local_gotno_;
  else
    ++this->global_gotno_;
}

// Add FROM's GOT entries.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::add_got_entries(
    Mips_got_info<size, big_endian>* from)
{
  for (typename Got_entry_set::iterator
       p = from->got_entries_.begin();
       p != from->got_entries_.end();
       ++p)
    {
      Mips_got_entry<size, big_endian>* entry = *p;
      if (this->got_entries_.find(entry) == this->got_entries_.end())
        {
          Mips_got_entry<size, big_endian>* entry2 =
            new Mips_got_entry<size, big_endian>(*entry);
          this->got_entries_.insert(entry2);
          this->count_got_entry(entry);
        }
    }
}

// Add FROM's GOT page entries.

template<int size, bool big_endian>
void
Mips_got_info<size, big_endian>::add_got_page_count(
    Mips_got_info<size, big_endian>* from)
{
  this->page_gotno_ += from->page_gotno_;
}

// Mips_output_data_got methods.

// Lay out the GOT.  Add local, global and TLS entries.  If GOT is
// larger than 64K, create multi-GOT.

template<int size, bool big_endian>
void
Mips_output_data_got<size, big_endian>::lay_out_got(Layout* layout,
    Symbol_table* symtab, const Input_objects* input_objects)
{
  // Decide which symbols need to go in the global part of the GOT and
  // count the number of reloc-only GOT symbols.
  this->master_got_info_->count_got_symbols(symtab);

  // Count the number of GOT entries.
  this->master_got_info_->count_got_entries();

  unsigned int got_size = this->master_got_info_->got_size();
  if (got_size > Target_mips<size, big_endian>::MIPS_GOT_MAX_SIZE)
    this->lay_out_multi_got(layout, input_objects);
  else
    {
      // Record that all objects use single GOT.
      for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
           p != input_objects->relobj_end();
           ++p)
        {
          Mips_relobj<size, big_endian>* object =
            Mips_relobj<size, big_endian>::as_mips_relobj(*p);
          if (object->get_got_info() != NULL)
            object->set_got_info(this->master_got_info_);
        }

      this->master_got_info_->add_local_entries(this->target_, layout);
      this->master_got_info_->add_global_entries(this->target_, layout,
                                                 /*not used*/-1U);
      this->master_got_info_->add_tls_entries(this->target_, layout);
    }
}

// Create multi-GOT.  For every GOT, add local, global and TLS entries.

template<int size, bool big_endian>
void
Mips_output_data_got<size, big_endian>::lay_out_multi_got(Layout* layout,
    const Input_objects* input_objects)
{
  // Try to merge the GOTs of input objects together, as long as they
  // don't seem to exceed the maximum GOT size, choosing one of them
  // to be the primary GOT.
  this->merge_gots(input_objects);

  // Every symbol that is referenced in a dynamic relocation must be
  // present in the primary GOT.
  this->primary_got_->set_global_gotno(this->master_got_info_->global_gotno());

  // Add GOT entries.
  unsigned int i = 0;
  unsigned int offset = 0;
  Mips_got_info<size, big_endian>* g = this->primary_got_;
  do
    {
      g->set_index(i);
      g->set_offset(offset);

      g->add_local_entries(this->target_, layout);
      if (i == 0)
        g->add_global_entries(this->target_, layout,
                              (this->master_got_info_->global_gotno()
                               - this->master_got_info_->reloc_only_gotno()));
      else
        g->add_global_entries(this->target_, layout, /*not used*/-1U);
      g->add_tls_entries(this->target_, layout);

      // Forbid global symbols in every non-primary GOT from having
      // lazy-binding stubs.
      if (i > 0)
        g->remove_lazy_stubs(this->target_);

      ++i;
      offset += g->got_size();
      g = g->next();
    }
  while (g);
}

// Attempt to merge GOTs of different input objects.  Try to use as much as
// possible of the primary GOT, since it doesn't require explicit dynamic
// relocations, but don't use objects that would reference global symbols
// out of the addressable range.  Failing the primary GOT, attempt to merge
// with the current GOT, or finish the current GOT and then make make the new
// GOT current.

template<int size, bool big_endian>
void
Mips_output_data_got<size, big_endian>::merge_gots(
    const Input_objects* input_objects)
{
  gold_assert(this->primary_got_ == NULL);
  Mips_got_info<size, big_endian>* current = NULL;

  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      Mips_relobj<size, big_endian>* object =
        Mips_relobj<size, big_endian>::as_mips_relobj(*p);

      Mips_got_info<size, big_endian>* g = object->get_got_info();
      if (g == NULL)
        continue;

      g->count_got_entries();

      // Work out the number of page, local and TLS entries.
      unsigned int estimate = this->master_got_info_->page_gotno();
      if (estimate > g->page_gotno())
        estimate = g->page_gotno();
      estimate += g->local_gotno() + g->tls_gotno();

      // We place TLS GOT entries after both locals and globals.  The globals
      // for the primary GOT may overflow the normal GOT size limit, so be
      // sure not to merge a GOT which requires TLS with the primary GOT in that
      // case.  This doesn't affect non-primary GOTs.
      estimate += (g->tls_gotno() > 0 ? this->master_got_info_->global_gotno()
                                      : g->global_gotno());

      unsigned int max_count =
        Target_mips<size, big_endian>::MIPS_GOT_MAX_SIZE / (size/8) - 2;
      if (estimate <= max_count)
        {
          // If we don't have a primary GOT, use it as
          // a starting point for the primary GOT.
          if (!this->primary_got_)
            {
              this->primary_got_ = g;
              continue;
            }

          // Try merging with the primary GOT.
          if (this->merge_got_with(g, object, this->primary_got_))
            continue;
        }

      // If we can merge with the last-created GOT, do it.
      if (current && this->merge_got_with(g, object, current))
        continue;

      // Well, we couldn't merge, so create a new GOT.  Don't check if it
      // fits; if it turns out that it doesn't, we'll get relocation
      // overflows anyway.
      g->set_next(current);
      current = g;
    }

  // If we do not find any suitable primary GOT, create an empty one.
  if (this->primary_got_ == NULL)
    this->primary_got_ = new Mips_got_info<size, big_endian>();

  // Link primary GOT with secondary GOTs.
  this->primary_got_->set_next(current);
}

// Consider merging FROM, which is OBJECT's GOT, into TO.  Return false if
// this would lead to overflow, true if they were merged successfully.

template<int size, bool big_endian>
bool
Mips_output_data_got<size, big_endian>::merge_got_with(
    Mips_got_info<size, big_endian>* from,
    Mips_relobj<size, big_endian>* object,
    Mips_got_info<size, big_endian>* to)
{
  // Work out how many page entries we would need for the combined GOT.
  unsigned int estimate = this->master_got_info_->page_gotno();
  if (estimate >= from->page_gotno() + to->page_gotno())
    estimate = from->page_gotno() + to->page_gotno();

  // Conservatively estimate how many local and TLS entries would be needed.
  estimate += from->local_gotno() + to->local_gotno();
  estimate += from->tls_gotno() + to->tls_gotno();

  // If we're merging with the primary got, any TLS relocations will
  // come after the full set of global entries.  Otherwise estimate those
  // conservatively as well.
  if (to == this->primary_got_ && (from->tls_gotno() + to->tls_gotno()) > 0)
    estimate += this->master_got_info_->global_gotno();
  else
    estimate += from->global_gotno() + to->global_gotno();

  // Bail out if the combined GOT might be too big.
  unsigned int max_count =
    Target_mips<size, big_endian>::MIPS_GOT_MAX_SIZE / (size/8) - 2;
  if (estimate > max_count)
    return false;

  // Transfer the object's GOT information from FROM to TO.
  to->add_got_entries(from);
  to->add_got_page_count(from);

  // Record that OBJECT should use output GOT TO.
  object->set_got_info(to);

  return true;
}

// Write out the GOT.

template<int size, bool big_endian>
void
Mips_output_data_got<size, big_endian>::do_write(Output_file* of)
{
  typedef Unordered_set<Mips_symbol<size>*, Mips_symbol_hash<size> >
      Mips_stubs_entry_set;

  // Call parent to write out GOT.
  Output_data_got<size, big_endian>::do_write(of);

  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  // Needed for fixing values of .got section.
  this->got_view_ = oview;

  // Write lazy stub addresses.
  for (typename Mips_stubs_entry_set::iterator
       p = this->master_got_info_->global_got_symbols().begin();
       p != this->master_got_info_->global_got_symbols().end();
       ++p)
    {
      Mips_symbol<size>* mips_sym = *p;
      if (mips_sym->has_lazy_stub())
        {
          Valtype* wv = reinterpret_cast<Valtype*>(
            oview + this->get_primary_got_offset(mips_sym));
          Valtype value =
            this->target_->mips_stubs_section()->stub_address(mips_sym);
          elfcpp::Swap<size, big_endian>::writeval(wv, value);
        }
    }

  // Add +1 to GGA_NONE nonzero MIPS16 and microMIPS entries.
  for (typename Mips_stubs_entry_set::iterator
       p = this->master_got_info_->global_got_symbols().begin();
       p != this->master_got_info_->global_got_symbols().end();
       ++p)
    {
      Mips_symbol<size>* mips_sym = *p;
      if (!this->multi_got()
          && (mips_sym->is_mips16() || mips_sym->is_micromips())
          && mips_sym->global_got_area() == GGA_NONE
          && mips_sym->has_got_offset(GOT_TYPE_STANDARD))
        {
          Valtype* wv = reinterpret_cast<Valtype*>(
            oview + mips_sym->got_offset(GOT_TYPE_STANDARD));
          Valtype value = elfcpp::Swap<size, big_endian>::readval(wv);
          if (value != 0)
            {
              value |= 1;
              elfcpp::Swap<size, big_endian>::writeval(wv, value);
            }
        }
    }

  if (!this->secondary_got_relocs_.empty())
    {
      // Fixup for the secondary GOT R_MIPS_REL32 relocs.  For global
      // secondary GOT entries with non-zero initial value copy the value
      // to the corresponding primary GOT entry, and set the secondary GOT
      // entry to zero.
      // TODO(sasa): This is workaround.  It needs to be investigated further.

      for (size_t i = 0; i < this->secondary_got_relocs_.size(); ++i)
        {
          Static_reloc& reloc(this->secondary_got_relocs_[i]);
          if (reloc.symbol_is_global())
            {
              Mips_symbol<size>* gsym = reloc.symbol();
              gold_assert(gsym != NULL);

              unsigned got_offset = reloc.got_offset();
              gold_assert(got_offset < oview_size);

              // Find primary GOT entry.
              Valtype* wv_prim = reinterpret_cast<Valtype*>(
                oview + this->get_primary_got_offset(gsym));

              // Find secondary GOT entry.
              Valtype* wv_sec = reinterpret_cast<Valtype*>(oview + got_offset);

              Valtype value = elfcpp::Swap<size, big_endian>::readval(wv_sec);
              if (value != 0)
                {
                  elfcpp::Swap<size, big_endian>::writeval(wv_prim, value);
                  elfcpp::Swap<size, big_endian>::writeval(wv_sec, 0);
                  gsym->set_applied_secondary_got_fixup();
                }
            }
        }

      of->write_output_view(offset, oview_size, oview);
    }

  // We are done if there is no fix up.
  if (this->static_relocs_.empty())
    return;

  Output_segment* tls_segment = this->layout_->tls_segment();
  gold_assert(tls_segment != NULL);

  for (size_t i = 0; i < this->static_relocs_.size(); ++i)
    {
      Static_reloc& reloc(this->static_relocs_[i]);

      Mips_address value;
      if (!reloc.symbol_is_global())
        {
          Sized_relobj_file<size, big_endian>* object = reloc.relobj();
          const Symbol_value<size>* psymval =
            object->local_symbol(reloc.index());

          // We are doing static linking.  Issue an error and skip this
          // relocation if the symbol is undefined or in a discarded_section.
          bool is_ordinary;
          unsigned int shndx = psymval->input_shndx(&is_ordinary);
          if ((shndx == elfcpp::SHN_UNDEF)
              || (is_ordinary
                  && shndx != elfcpp::SHN_UNDEF
                  && !object->is_section_included(shndx)
                  && !this->symbol_table_->is_section_folded(object, shndx)))
            {
              gold_error(_("undefined or discarded local symbol %u from "
                           " object %s in GOT"),
                         reloc.index(), reloc.relobj()->name().c_str());
              continue;
            }

          value = psymval->value(object, 0);
        }
      else
        {
          const Mips_symbol<size>* gsym = reloc.symbol();
          gold_assert(gsym != NULL);

          // We are doing static linking.  Issue an error and skip this
          // relocation if the symbol is undefined or in a discarded_section
          // unless it is a weakly_undefined symbol.
          if ((gsym->is_defined_in_discarded_section() || gsym->is_undefined())
              && !gsym->is_weak_undefined())
            {
              gold_error(_("undefined or discarded symbol %s in GOT"),
                         gsym->name());
              continue;
            }

          if (!gsym->is_weak_undefined())
            value = gsym->value();
          else
            value = 0;
        }

      unsigned got_offset = reloc.got_offset();
      gold_assert(got_offset < oview_size);

      Valtype* wv = reinterpret_cast<Valtype*>(oview + got_offset);
      Valtype x;

      switch (reloc.r_type())
        {
        case elfcpp::R_MIPS_TLS_DTPMOD32:
        case elfcpp::R_MIPS_TLS_DTPMOD64:
          x = value;
          break;
        case elfcpp::R_MIPS_TLS_DTPREL32:
        case elfcpp::R_MIPS_TLS_DTPREL64:
          x = value - elfcpp::DTP_OFFSET;
          break;
        case elfcpp::R_MIPS_TLS_TPREL32:
        case elfcpp::R_MIPS_TLS_TPREL64:
          x = value - elfcpp::TP_OFFSET;
          break;
        default:
          gold_unreachable();
          break;
        }

      elfcpp::Swap<size, big_endian>::writeval(wv, x);
    }

  of->write_output_view(offset, oview_size, oview);
}

// Mips_relobj methods.

// Count the local symbols.  The Mips backend needs to know if a symbol
// is a MIPS16 or microMIPS function or not.  For global symbols, it is easy
// because the Symbol object keeps the ELF symbol type and st_other field.
// For local symbol it is harder because we cannot access this information.
// So we override the do_count_local_symbol in parent and scan local symbols to
// mark MIPS16 and microMIPS functions.  This is not the most efficient way but
// I do not want to slow down other ports by calling a per symbol target hook
// inside Sized_relobj_file<size, big_endian>::do_count_local_symbols.

template<int size, bool big_endian>
void
Mips_relobj<size, big_endian>::do_count_local_symbols(
    Stringpool_template<char>* pool,
    Stringpool_template<char>* dynpool)
{
  // Ask parent to count the local symbols.
  Sized_relobj_file<size, big_endian>::do_count_local_symbols(pool, dynpool);
  const unsigned int loccount = this->local_symbol_count();
  if (loccount == 0)
    return;

  // Initialize the mips16 and micromips function bit-vector.
  this->local_symbol_is_mips16_.resize(loccount, false);
  this->local_symbol_is_micromips_.resize(loccount, false);

  // Read the symbol table section header.
  const unsigned int symtab_shndx = this->symtab_shndx();
  elfcpp::Shdr<size, big_endian>
    symtabshdr(this, this->elf_file()->section_header(symtab_shndx));
  gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);

  // Read the local symbols.
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  gold_assert(loccount == symtabshdr.get_sh_info());
  off_t locsize = loccount * sym_size;
  const unsigned char* psyms = this->get_view(symtabshdr.get_sh_offset(),
                                              locsize, true, true);

  // Loop over the local symbols and mark any MIPS16 or microMIPS local symbols.

  // Skip the first dummy symbol.
  psyms += sym_size;
  for (unsigned int i = 1; i < loccount; ++i, psyms += sym_size)
    {
      elfcpp::Sym<size, big_endian> sym(psyms);
      unsigned char st_other = sym.get_st_other();
      this->local_symbol_is_mips16_[i] = elfcpp::elf_st_is_mips16(st_other);
      this->local_symbol_is_micromips_[i] =
        elfcpp::elf_st_is_micromips(st_other);
    }
}

// Read the symbol information.

template<int size, bool big_endian>
void
Mips_relobj<size, big_endian>::do_read_symbols(Read_symbols_data* sd)
{
  // Call parent class to read symbol information.
  this->base_read_symbols(sd);

  // If this input file is a binary file, it has no processor
  // specific data.
  Input_file::Format format = this->input_file()->format();
  if (format != Input_file::FORMAT_ELF)
    {
      gold_assert(format == Input_file::FORMAT_BINARY);
      this->merge_processor_specific_data_ = false;
      return;
    }

  // Read processor-specific flags in ELF file header.
  const unsigned char* pehdr = this->get_view(elfcpp::file_header_offset,
                                            elfcpp::Elf_sizes<size>::ehdr_size,
                                            true, false);
  elfcpp::Ehdr<size, big_endian> ehdr(pehdr);
  this->processor_specific_flags_ = ehdr.get_e_flags();

  // Get the section names.
  const unsigned char* pnamesu = sd->section_names->data();
  const char* pnames = reinterpret_cast<const char*>(pnamesu);

  // Initialize the mips16 stub section bit-vectors.
  this->section_is_mips16_fn_stub_.resize(this->shnum(), false);
  this->section_is_mips16_call_stub_.resize(this->shnum(), false);
  this->section_is_mips16_call_fp_stub_.resize(this->shnum(), false);

  const size_t shdr_size = elfcpp::Elf_sizes<size>::shdr_size;
  const unsigned char* pshdrs = sd->section_headers->data();
  const unsigned char* ps = pshdrs + shdr_size;
  bool must_merge_processor_specific_data = false;
  for (unsigned int i = 1; i < this->shnum(); ++i, ps += shdr_size)
    {
      elfcpp::Shdr<size, big_endian> shdr(ps);

      // Sometimes an object has no contents except the section name string
      // table and an empty symbol table with the undefined symbol.  We
      // don't want to merge processor-specific data from such an object.
      if (shdr.get_sh_type() == elfcpp::SHT_SYMTAB)
        {
          // Symbol table is not empty.
          const typename elfcpp::Elf_types<size>::Elf_WXword sym_size =
            elfcpp::Elf_sizes<size>::sym_size;
          if (shdr.get_sh_size() > sym_size)
            must_merge_processor_specific_data = true;
        }
      else if (shdr.get_sh_type() != elfcpp::SHT_STRTAB)
        // If this is neither an empty symbol table nor a string table,
        // be conservative.
        must_merge_processor_specific_data = true;

      if (shdr.get_sh_type() == elfcpp::SHT_MIPS_REGINFO)
        {
          this->has_reginfo_section_ = true;
          // Read the gp value that was used to create this object.  We need the
          // gp value while processing relocs.  The .reginfo section is not used
          // in the 64-bit MIPS ELF ABI.
          section_offset_type section_offset = shdr.get_sh_offset();
          section_size_type section_size =
            convert_to_section_size_type(shdr.get_sh_size());
          const unsigned char* view =
             this->get_view(section_offset, section_size, true, false);

          this->gp_ = elfcpp::Swap<size, big_endian>::readval(view + 20);

          // Read the rest of .reginfo.
          this->gprmask_ = elfcpp::Swap<size, big_endian>::readval(view);
          this->cprmask1_ = elfcpp::Swap<size, big_endian>::readval(view + 4);
          this->cprmask2_ = elfcpp::Swap<size, big_endian>::readval(view + 8);
          this->cprmask3_ = elfcpp::Swap<size, big_endian>::readval(view + 12);
          this->cprmask4_ = elfcpp::Swap<size, big_endian>::readval(view + 16);
        }

      if (shdr.get_sh_type() == elfcpp::SHT_GNU_ATTRIBUTES)
        {
          gold_assert(this->attributes_section_data_ == NULL);
          section_offset_type section_offset = shdr.get_sh_offset();
          section_size_type section_size =
            convert_to_section_size_type(shdr.get_sh_size());
          const unsigned char* view =
            this->get_view(section_offset, section_size, true, false);
          this->attributes_section_data_ =
            new Attributes_section_data(view, section_size);
        }

      if (shdr.get_sh_type() == elfcpp::SHT_MIPS_ABIFLAGS)
        {
          gold_assert(this->abiflags_ == NULL);
          section_offset_type section_offset = shdr.get_sh_offset();
          section_size_type section_size =
            convert_to_section_size_type(shdr.get_sh_size());
          const unsigned char* view =
            this->get_view(section_offset, section_size, true, false);
          this->abiflags_ = new Mips_abiflags<big_endian>();

          this->abiflags_->version =
            elfcpp::Swap<16, big_endian>::readval(view);
          if (this->abiflags_->version != 0)
            {
              gold_error(_("%s: .MIPS.abiflags section has "
                           "unsupported version %u"),
                         this->name().c_str(),
                         this->abiflags_->version);
              break;
            }
          this->abiflags_->isa_level =
            elfcpp::Swap<8, big_endian>::readval(view + 2);
          this->abiflags_->isa_rev =
            elfcpp::Swap<8, big_endian>::readval(view + 3);
          this->abiflags_->gpr_size =
            elfcpp::Swap<8, big_endian>::readval(view + 4);
          this->abiflags_->cpr1_size =
            elfcpp::Swap<8, big_endian>::readval(view + 5);
          this->abiflags_->cpr2_size =
            elfcpp::Swap<8, big_endian>::readval(view + 6);
          this->abiflags_->fp_abi =
            elfcpp::Swap<8, big_endian>::readval(view + 7);
          this->abiflags_->isa_ext =
            elfcpp::Swap<32, big_endian>::readval(view + 8);
          this->abiflags_->ases =
            elfcpp::Swap<32, big_endian>::readval(view + 12);
          this->abiflags_->flags1 =
            elfcpp::Swap<32, big_endian>::readval(view + 16);
          this->abiflags_->flags2 =
            elfcpp::Swap<32, big_endian>::readval(view + 20);
        }

      // In the 64-bit ABI, .MIPS.options section holds register information.
      // A SHT_MIPS_OPTIONS section contains a series of options, each of which
      // starts with this header:
      //
      // typedef struct
      // {
      //   // Type of option.
      //   unsigned char kind[1];
      //   // Size of option descriptor, including header.
      //   unsigned char size[1];
      //   // Section index of affected section, or 0 for global option.
      //   unsigned char section[2];
      //   // Information specific to this kind of option.
      //   unsigned char info[4];
      // };
      //
      // For a SHT_MIPS_OPTIONS section, look for a ODK_REGINFO entry, and set
      // the gp value based on what we find.  We may see both SHT_MIPS_REGINFO
      // and SHT_MIPS_OPTIONS/ODK_REGINFO; in that case, they should agree.

      if (shdr.get_sh_type() == elfcpp::SHT_MIPS_OPTIONS)
        {
          section_offset_type section_offset = shdr.get_sh_offset();
          section_size_type section_size =
            convert_to_section_size_type(shdr.get_sh_size());
          const unsigned char* view =
             this->get_view(section_offset, section_size, true, false);
          const unsigned char* end = view + section_size;

          while (view + 8 <= end)
            {
              unsigned char kind = elfcpp::Swap<8, big_endian>::readval(view);
              unsigned char sz = elfcpp::Swap<8, big_endian>::readval(view + 1);
              if (sz < 8)
                {
                  gold_error(_("%s: Warning: bad `%s' option size %u smaller "
                               "than its header"),
                             this->name().c_str(),
                             this->mips_elf_options_section_name(), sz);
                  break;
                }

              if (this->is_n64() && kind == elfcpp::ODK_REGINFO)
                {
                  // In the 64 bit ABI, an ODK_REGINFO option is the following
                  // structure.  The info field of the options header is not
                  // used.
                  //
                  // typedef struct
                  // {
                  //   // Mask of general purpose registers used.
                  //   unsigned char ri_gprmask[4];
                  //   // Padding.
                  //   unsigned char ri_pad[4];
                  //   // Mask of co-processor registers used.
                  //   unsigned char ri_cprmask[4][4];
                  //   // GP register value for this object file.
                  //   unsigned char ri_gp_value[8];
                  // };

                  this->gp_ = elfcpp::Swap<size, big_endian>::readval(view
                                                                      + 32);
                }
              else if (kind == elfcpp::ODK_REGINFO)
                {
                  // In the 32 bit ABI, an ODK_REGINFO option is the following
                  // structure.  The info field of the options header is not
                  // used.  The same structure is used in .reginfo section.
                  //
                  // typedef struct
                  // {
                  //   unsigned char ri_gprmask[4];
                  //   unsigned char ri_cprmask[4][4];
                  //   unsigned char ri_gp_value[4];
                  // };

                  this->gp_ = elfcpp::Swap<size, big_endian>::readval(view
                                                                      + 28);
                }
              view += sz;
            }
        }

      const char* name = pnames + shdr.get_sh_name();
      this->section_is_mips16_fn_stub_[i] = is_prefix_of(".mips16.fn", name);
      this->section_is_mips16_call_stub_[i] =
        is_prefix_of(".mips16.call.", name);
      this->section_is_mips16_call_fp_stub_[i] =
        is_prefix_of(".mips16.call.fp.", name);

      if (strcmp(name, ".pdr") == 0)
        {
          gold_assert(this->pdr_shndx_ == -1U);
          this->pdr_shndx_ = i;
        }
    }

  // This is rare.
  if (!must_merge_processor_specific_data)
    this->merge_processor_specific_data_ = false;
}

// Discard MIPS16 stub secions that are not needed.

template<int size, bool big_endian>
void
Mips_relobj<size, big_endian>::discard_mips16_stub_sections(Symbol_table* symtab)
{
  for (typename Mips16_stubs_int_map::const_iterator
       it = this->mips16_stub_sections_.begin();
       it != this->mips16_stub_sections_.end(); ++it)
    {
      Mips16_stub_section<size, big_endian>* stub_section = it->second;
      if (!stub_section->is_target_found())
        {
          gold_error(_("no relocation found in mips16 stub section '%s'"),
                     stub_section->object()
                       ->section_name(stub_section->shndx()).c_str());
        }

      bool discard = false;
      if (stub_section->is_for_local_function())
        {
          if (stub_section->is_fn_stub())
            {
              // This stub is for a local symbol.  This stub will only
              // be needed if there is some relocation in this object,
              // other than a 16 bit function call, which refers to this
              // symbol.
              if (!this->has_local_non_16bit_call_relocs(stub_section->r_sym()))
                discard = true;
              else
                this->add_local_mips16_fn_stub(stub_section);
            }
          else
            {
              // This stub is for a local symbol.  This stub will only
              // be needed if there is some relocation (R_MIPS16_26) in
              // this object that refers to this symbol.
              gold_assert(stub_section->is_call_stub()
                          || stub_section->is_call_fp_stub());
              if (!this->has_local_16bit_call_relocs(stub_section->r_sym()))
                discard = true;
              else
                this->add_local_mips16_call_stub(stub_section);
            }
        }
      else
        {
          Mips_symbol<size>* gsym = stub_section->gsym();
          if (stub_section->is_fn_stub())
            {
              if (gsym->has_mips16_fn_stub())
                // We already have a stub for this function.
                discard = true;
              else
                {
                  gsym->set_mips16_fn_stub(stub_section);
                  if (gsym->should_add_dynsym_entry(symtab))
                    {
                      // If we have a MIPS16 function with a stub, the
                      // dynamic symbol must refer to the stub, since only
                      // the stub uses the standard calling conventions.
                      gsym->set_need_fn_stub();
                      if (gsym->is_from_dynobj())
                        gsym->set_needs_dynsym_value();
                    }
                }
              if (!gsym->need_fn_stub())
                discard = true;
            }
          else if (stub_section->is_call_stub())
            {
              if (gsym->is_mips16())
                // We don't need the call_stub; this is a 16 bit
                // function, so calls from other 16 bit functions are
                // OK.
                discard = true;
              else if (gsym->has_mips16_call_stub())
                // We already have a stub for this function.
                discard = true;
              else
                gsym->set_mips16_call_stub(stub_section);
            }
          else
            {
              gold_assert(stub_section->is_call_fp_stub());
              if (gsym->is_mips16())
                // We don't need the call_stub; this is a 16 bit
                // function, so calls from other 16 bit functions are
                // OK.
                discard = true;
              else if (gsym->has_mips16_call_fp_stub())
                // We already have a stub for this function.
                discard = true;
              else
                gsym->set_mips16_call_fp_stub(stub_section);
            }
        }
      if (discard)
        this->set_output_section(stub_section->shndx(), NULL);
   }
}

// Mips_output_data_la25_stub methods.

// Template for standard LA25 stub.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_la25_stub<size, big_endian>::la25_stub_entry[] =
{
  0x3c190000,           // lui $25,%hi(func)
  0x08000000,           // j func
  0x27390000,           // add $25,$25,%lo(func)
  0x00000000            // nop
};

// Template for microMIPS LA25 stub.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_la25_stub<size, big_endian>::la25_stub_micromips_entry[] =
{
  0x41b9, 0x0000,       // lui t9,%hi(func)
  0xd400, 0x0000,       // j func
  0x3339, 0x0000,       // addiu t9,t9,%lo(func)
  0x0000, 0x0000        // nop
};

// Create la25 stub for a symbol.

template<int size, bool big_endian>
void
Mips_output_data_la25_stub<size, big_endian>::create_la25_stub(
    Symbol_table* symtab, Target_mips<size, big_endian>* target,
    Mips_symbol<size>* gsym)
{
  if (!gsym->has_la25_stub())
    {
      gsym->set_la25_stub_offset(this->symbols_.size() * 16);
      this->symbols_.push_back(gsym);
      this->create_stub_symbol(gsym, symtab, target, 16);
    }
}

// Create a symbol for SYM stub's value and size, to help make the disassembly
// easier to read.

template<int size, bool big_endian>
void
Mips_output_data_la25_stub<size, big_endian>::create_stub_symbol(
    Mips_symbol<size>* sym, Symbol_table* symtab,
    Target_mips<size, big_endian>* target, uint64_t symsize)
{
  std::string name(".pic.");
  name += sym->name();

  unsigned int offset = sym->la25_stub_offset();
  if (sym->is_micromips())
    offset |= 1;

  // Make it a local function.
  Symbol* new_sym = symtab->define_in_output_data(name.c_str(), NULL,
                                      Symbol_table::PREDEFINED,
                                      target->la25_stub_section(),
                                      offset, symsize, elfcpp::STT_FUNC,
                                      elfcpp::STB_LOCAL,
                                      elfcpp::STV_DEFAULT, 0,
                                      false, false);
  new_sym->set_is_forced_local();
}

// Write out la25 stubs.  This uses the hand-coded instructions above,
// and adjusts them as needed.

template<int size, bool big_endian>
void
Mips_output_data_la25_stub<size, big_endian>::do_write(Output_file* of)
{
  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  for (typename std::vector<Mips_symbol<size>*>::iterator
       p = this->symbols_.begin();
       p != this->symbols_.end();
       ++p)
    {
      Mips_symbol<size>* sym = *p;
      unsigned char* pov = oview + sym->la25_stub_offset();

      Mips_address target = sym->value();
      if (!sym->is_micromips())
        {
          elfcpp::Swap<32, big_endian>::writeval(pov,
              la25_stub_entry[0] | (((target + 0x8000) >> 16) & 0xffff));
          elfcpp::Swap<32, big_endian>::writeval(pov + 4,
              la25_stub_entry[1] | ((target >> 2) & 0x3ffffff));
          elfcpp::Swap<32, big_endian>::writeval(pov + 8,
              la25_stub_entry[2] | (target & 0xffff));
          elfcpp::Swap<32, big_endian>::writeval(pov + 12, la25_stub_entry[3]);
        }
      else
        {
          target |= 1;
          // First stub instruction.  Paste high 16-bits of the target.
          elfcpp::Swap<16, big_endian>::writeval(pov,
                                                 la25_stub_micromips_entry[0]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 2,
              ((target + 0x8000) >> 16) & 0xffff);
          // Second stub instruction.  Paste low 26-bits of the target, shifted
          // right by 1.
          elfcpp::Swap<16, big_endian>::writeval(pov + 4,
              la25_stub_micromips_entry[2] | ((target >> 17) & 0x3ff));
          elfcpp::Swap<16, big_endian>::writeval(pov + 6,
              la25_stub_micromips_entry[3] | ((target >> 1) & 0xffff));
          // Third stub instruction.  Paste low 16-bits of the target.
          elfcpp::Swap<16, big_endian>::writeval(pov + 8,
                                                 la25_stub_micromips_entry[4]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 10, target & 0xffff);
          // Fourth stub instruction.
          elfcpp::Swap<16, big_endian>::writeval(pov + 12,
                                                 la25_stub_micromips_entry[6]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 14,
                                                 la25_stub_micromips_entry[7]);
        }
    }

  of->write_output_view(offset, oview_size, oview);
}

// Mips_output_data_plt methods.

// The format of the first PLT entry in an O32 executable.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::plt0_entry_o32[] =
{
  0x3c1c0000,         // lui $28, %hi(&GOTPLT[0])
  0x8f990000,         // lw $25, %lo(&GOTPLT[0])($28)
  0x279c0000,         // addiu $28, $28, %lo(&GOTPLT[0])
  0x031cc023,         // subu $24, $24, $28
  0x03e07825,         // or $15, $31, zero
  0x0018c082,         // srl $24, $24, 2
  0x0320f809,         // jalr $25
  0x2718fffe          // subu $24, $24, 2
};

// The format of the first PLT entry in an N32 executable.  Different
// because gp ($28) is not available; we use t2 ($14) instead.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::plt0_entry_n32[] =
{
  0x3c0e0000,         // lui $14, %hi(&GOTPLT[0])
  0x8dd90000,         // lw $25, %lo(&GOTPLT[0])($14)
  0x25ce0000,         // addiu $14, $14, %lo(&GOTPLT[0])
  0x030ec023,         // subu $24, $24, $14
  0x03e07825,         // or $15, $31, zero
  0x0018c082,         // srl $24, $24, 2
  0x0320f809,         // jalr $25
  0x2718fffe          // subu $24, $24, 2
};

// The format of the first PLT entry in an N64 executable.  Different
// from N32 because of the increased size of GOT entries.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::plt0_entry_n64[] =
{
  0x3c0e0000,         // lui $14, %hi(&GOTPLT[0])
  0xddd90000,         // ld $25, %lo(&GOTPLT[0])($14)
  0x25ce0000,         // addiu $14, $14, %lo(&GOTPLT[0])
  0x030ec023,         // subu $24, $24, $14
  0x03e07825,         // or $15, $31, zero
  0x0018c0c2,         // srl $24, $24, 3
  0x0320f809,         // jalr $25
  0x2718fffe          // subu $24, $24, 2
};

// The format of the microMIPS first PLT entry in an O32 executable.
// We rely on v0 ($2) rather than t8 ($24) to contain the address
// of the GOTPLT entry handled, so this stub may only be used when
// all the subsequent PLT entries are microMIPS code too.
//
// The trailing NOP is for alignment and correct disassembly only.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::
plt0_entry_micromips_o32[] =
{
  0x7980, 0x0000,      // addiupc $3, (&GOTPLT[0]) - .
  0xff23, 0x0000,      // lw $25, 0($3)
  0x0535,              // subu $2, $2, $3
  0x2525,              // srl $2, $2, 2
  0x3302, 0xfffe,      // subu $24, $2, 2
  0x0dff,              // move $15, $31
  0x45f9,              // jalrs $25
  0x0f83,              // move $28, $3
  0x0c00               // nop
};

// The format of the microMIPS first PLT entry in an O32 executable
// in the insn32 mode.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::
plt0_entry_micromips32_o32[] =
{
  0x41bc, 0x0000,      // lui $28, %hi(&GOTPLT[0])
  0xff3c, 0x0000,      // lw $25, %lo(&GOTPLT[0])($28)
  0x339c, 0x0000,      // addiu $28, $28, %lo(&GOTPLT[0])
  0x0398, 0xc1d0,      // subu $24, $24, $28
  0x001f, 0x7a90,      // or $15, $31, zero
  0x0318, 0x1040,      // srl $24, $24, 2
  0x03f9, 0x0f3c,      // jalr $25
  0x3318, 0xfffe       // subu $24, $24, 2
};

// The format of subsequent standard entries in the PLT.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::plt_entry[] =
{
  0x3c0f0000,           // lui $15, %hi(.got.plt entry)
  0x01f90000,           // l[wd] $25, %lo(.got.plt entry)($15)
  0x03200008,           // jr $25
  0x25f80000            // addiu $24, $15, %lo(.got.plt entry)
};

// The format of subsequent R6 PLT entries.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::plt_entry_r6[] =
{
  0x3c0f0000,           // lui $15, %hi(.got.plt entry)
  0x01f90000,           // l[wd] $25, %lo(.got.plt entry)($15)
  0x03200009,           // jr $25
  0x25f80000            // addiu $24, $15, %lo(.got.plt entry)
};

// The format of subsequent MIPS16 o32 PLT entries.  We use v1 ($3) as a
// temporary because t8 ($24) and t9 ($25) are not directly addressable.
// Note that this differs from the GNU ld which uses both v0 ($2) and v1 ($3).
// We cannot use v0 because MIPS16 call stubs from the CS toolchain expect
// target function address in register v0.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::plt_entry_mips16_o32[] =
{
  0xb303,              // lw $3, 12($pc)
  0x651b,              // move $24, $3
  0x9b60,              // lw $3, 0($3)
  0xeb00,              // jr $3
  0x653b,              // move $25, $3
  0x6500,              // nop
  0x0000, 0x0000       // .word (.got.plt entry)
};

// The format of subsequent microMIPS o32 PLT entries.  We use v0 ($2)
// as a temporary because t8 ($24) is not addressable with ADDIUPC.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::
plt_entry_micromips_o32[] =
{
  0x7900, 0x0000,      // addiupc $2, (.got.plt entry) - .
  0xff22, 0x0000,      // lw $25, 0($2)
  0x4599,              // jr $25
  0x0f02               // move $24, $2
};

// The format of subsequent microMIPS o32 PLT entries in the insn32 mode.
template<int size, bool big_endian>
const uint32_t Mips_output_data_plt<size, big_endian>::
plt_entry_micromips32_o32[] =
{
  0x41af, 0x0000,      // lui $15, %hi(.got.plt entry)
  0xff2f, 0x0000,      // lw $25, %lo(.got.plt entry)($15)
  0x0019, 0x0f3c,      // jr $25
  0x330f, 0x0000       // addiu $24, $15, %lo(.got.plt entry)
};

// Add an entry to the PLT for a symbol referenced by r_type relocation.

template<int size, bool big_endian>
void
Mips_output_data_plt<size, big_endian>::add_entry(Mips_symbol<size>* gsym,
                                                  unsigned int r_type)
{
  gold_assert(!gsym->has_plt_offset());

  // Final PLT offset for a symbol will be set in method set_plt_offsets().
  gsym->set_plt_offset(this->entry_count() * sizeof(plt_entry)
                       + sizeof(plt0_entry_o32));
  this->symbols_.push_back(gsym);

  // Record whether the relocation requires a standard MIPS
  // or a compressed code entry.
  if (jal_reloc(r_type))
   {
     if (r_type == elfcpp::R_MIPS_26)
       gsym->set_needs_mips_plt(true);
     else
       gsym->set_needs_comp_plt(true);
   }

  section_offset_type got_offset = this->got_plt_->current_data_size();

  // Every PLT entry needs a GOT entry which points back to the PLT
  // entry (this will be changed by the dynamic linker, normally
  // lazily when the function is called).
  this->got_plt_->set_current_data_size(got_offset + size/8);

  gsym->set_needs_dynsym_entry();
  this->rel_->add_global(gsym, elfcpp::R_MIPS_JUMP_SLOT, this->got_plt_,
                         got_offset);
}

// Set final PLT offsets.  For each symbol, determine whether standard or
// compressed (MIPS16 or microMIPS) PLT entry is used.

template<int size, bool big_endian>
void
Mips_output_data_plt<size, big_endian>::set_plt_offsets()
{
  // The sizes of individual PLT entries.
  unsigned int plt_mips_entry_size = this->standard_plt_entry_size();
  unsigned int plt_comp_entry_size = (!this->target_->is_output_newabi()
                                      ? this->compressed_plt_entry_size() : 0);

  for (typename std::vector<Mips_symbol<size>*>::const_iterator
       p = this->symbols_.begin(); p != this->symbols_.end(); ++p)
    {
      Mips_symbol<size>* mips_sym = *p;

      // There are no defined MIPS16 or microMIPS PLT entries for n32 or n64,
      // so always use a standard entry there.
      //
      // If the symbol has a MIPS16 call stub and gets a PLT entry, then
      // all MIPS16 calls will go via that stub, and there is no benefit
      // to having a MIPS16 entry.  And in the case of call_stub a
      // standard entry actually has to be used as the stub ends with a J
      // instruction.
      if (this->target_->is_output_newabi()
          || mips_sym->has_mips16_call_stub()
          || mips_sym->has_mips16_call_fp_stub())
        {
          mips_sym->set_needs_mips_plt(true);
          mips_sym->set_needs_comp_plt(false);
        }

      // Otherwise, if there are no direct calls to the function, we
      // have a free choice of whether to use standard or compressed
      // entries.  Prefer microMIPS entries if the object is known to
      // contain microMIPS code, so that it becomes possible to create
      // pure microMIPS binaries.  Prefer standard entries otherwise,
      // because MIPS16 ones are no smaller and are usually slower.
      if (!mips_sym->needs_mips_plt() && !mips_sym->needs_comp_plt())
        {
          if (this->target_->is_output_micromips())
            mips_sym->set_needs_comp_plt(true);
          else
            mips_sym->set_needs_mips_plt(true);
        }

      if (mips_sym->needs_mips_plt())
        {
          mips_sym->set_mips_plt_offset(this->plt_mips_offset_);
          this->plt_mips_offset_ += plt_mips_entry_size;
        }
      if (mips_sym->needs_comp_plt())
        {
          mips_sym->set_comp_plt_offset(this->plt_comp_offset_);
          this->plt_comp_offset_ += plt_comp_entry_size;
        }
    }

    // Figure out the size of the PLT header if we know that we are using it.
    if (this->plt_mips_offset_ + this->plt_comp_offset_ != 0)
      this->plt_header_size_ = this->get_plt_header_size();
}

// Write out the PLT.  This uses the hand-coded instructions above,
// and adjusts them as needed.

template<int size, bool big_endian>
void
Mips_output_data_plt<size, big_endian>::do_write(Output_file* of)
{
  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  const off_t gotplt_file_offset = this->got_plt_->offset();
  const section_size_type gotplt_size =
    convert_to_section_size_type(this->got_plt_->data_size());
  unsigned char* const gotplt_view = of->get_output_view(gotplt_file_offset,
                                                         gotplt_size);
  unsigned char* pov = oview;

  Mips_address plt_address = this->address();

  // Calculate the address of .got.plt.
  Mips_address gotplt_addr = this->got_plt_->address();
  Mips_address gotplt_addr_high = ((gotplt_addr + 0x8000) >> 16) & 0xffff;
  Mips_address gotplt_addr_low = gotplt_addr & 0xffff;

  // The PLT sequence is not safe for N64 if .got.plt's address can
  // not be loaded in two instructions.
  gold_assert((gotplt_addr & ~(Mips_address) 0x7fffffff) == 0
              || ~(gotplt_addr | 0x7fffffff) == 0);

  // Write the PLT header.
  const uint32_t* plt0_entry = this->get_plt_header_entry();
  if (plt0_entry == plt0_entry_micromips_o32)
    {
      // Write microMIPS PLT header.
      gold_assert(gotplt_addr % 4 == 0);

      Mips_address gotpc_offset = gotplt_addr - ((plt_address | 3) ^ 3);

      // ADDIUPC has a span of +/-16MB, check we're in range.
      if (gotpc_offset + 0x1000000 >= 0x2000000)
       {
         gold_error(_(".got.plt offset of %ld from .plt beyond the range of "
                    "ADDIUPC"), (long)gotpc_offset);
         return;
       }

      elfcpp::Swap<16, big_endian>::writeval(pov,
                 plt0_entry[0] | ((gotpc_offset >> 18) & 0x7f));
      elfcpp::Swap<16, big_endian>::writeval(pov + 2,
                                             (gotpc_offset >> 2) & 0xffff);
      pov += 4;
      for (unsigned int i = 2;
           i < (sizeof(plt0_entry_micromips_o32)
                / sizeof(plt0_entry_micromips_o32[0]));
           i++)
        {
          elfcpp::Swap<16, big_endian>::writeval(pov, plt0_entry[i]);
          pov += 2;
        }
    }
  else if (plt0_entry == plt0_entry_micromips32_o32)
    {
      // Write microMIPS PLT header in insn32 mode.
      elfcpp::Swap<16, big_endian>::writeval(pov, plt0_entry[0]);
      elfcpp::Swap<16, big_endian>::writeval(pov + 2, gotplt_addr_high);
      elfcpp::Swap<16, big_endian>::writeval(pov + 4, plt0_entry[2]);
      elfcpp::Swap<16, big_endian>::writeval(pov + 6, gotplt_addr_low);
      elfcpp::Swap<16, big_endian>::writeval(pov + 8, plt0_entry[4]);
      elfcpp::Swap<16, big_endian>::writeval(pov + 10, gotplt_addr_low);
      pov += 12;
      for (unsigned int i = 6;
           i < (sizeof(plt0_entry_micromips32_o32)
                / sizeof(plt0_entry_micromips32_o32[0]));
           i++)
        {
          elfcpp::Swap<16, big_endian>::writeval(pov, plt0_entry[i]);
          pov += 2;
        }
    }
  else
    {
      // Write standard PLT header.
      elfcpp::Swap<32, big_endian>::writeval(pov,
                                             plt0_entry[0] | gotplt_addr_high);
      elfcpp::Swap<32, big_endian>::writeval(pov + 4,
                                             plt0_entry[1] | gotplt_addr_low);
      elfcpp::Swap<32, big_endian>::writeval(pov + 8,
                                             plt0_entry[2] | gotplt_addr_low);
      pov += 12;
      for (int i = 3; i < 8; i++)
        {
          elfcpp::Swap<32, big_endian>::writeval(pov, plt0_entry[i]);
          pov += 4;
        }
    }


  unsigned char* gotplt_pov = gotplt_view;
  unsigned int got_entry_size = size/8; // TODO(sasa): MIPS_ELF_GOT_SIZE

  // The first two entries in .got.plt are reserved.
  elfcpp::Swap<size, big_endian>::writeval(gotplt_pov, 0);
  elfcpp::Swap<size, big_endian>::writeval(gotplt_pov + got_entry_size, 0);

  unsigned int gotplt_offset = 2 * got_entry_size;
  gotplt_pov += 2 * got_entry_size;

  // Calculate the address of the PLT header.
  Mips_address header_address = (plt_address
                                 + (this->is_plt_header_compressed() ? 1 : 0));

  // Initialize compressed PLT area view.
  unsigned char* pov2 = pov + this->plt_mips_offset_;

  // Write the PLT entries.
  for (typename std::vector<Mips_symbol<size>*>::const_iterator
       p = this->symbols_.begin();
       p != this->symbols_.end();
       ++p, gotplt_pov += got_entry_size, gotplt_offset += got_entry_size)
    {
      Mips_symbol<size>* mips_sym = *p;

      // Calculate the address of the .got.plt entry.
      uint32_t gotplt_entry_addr = (gotplt_addr + gotplt_offset);
      uint32_t gotplt_entry_addr_hi = (((gotplt_entry_addr + 0x8000) >> 16)
                                       & 0xffff);
      uint32_t gotplt_entry_addr_lo = gotplt_entry_addr & 0xffff;

      // Initially point the .got.plt entry at the PLT header.
      if (this->target_->is_output_n64())
        elfcpp::Swap<64, big_endian>::writeval(gotplt_pov, header_address);
      else
        elfcpp::Swap<32, big_endian>::writeval(gotplt_pov, header_address);

      // Now handle the PLT itself.  First the standard entry.
      if (mips_sym->has_mips_plt_offset())
        {
          // Pick the load opcode (LW or LD).
          uint64_t load = this->target_->is_output_n64() ? 0xdc000000
                                                         : 0x8c000000;

          const uint32_t* entry = this->target_->is_output_r6() ? plt_entry_r6
                                                                : plt_entry;

          // Fill in the PLT entry itself.
          elfcpp::Swap<32, big_endian>::writeval(pov,
              entry[0] | gotplt_entry_addr_hi);
          elfcpp::Swap<32, big_endian>::writeval(pov + 4,
              entry[1] | gotplt_entry_addr_lo | load);
          elfcpp::Swap<32, big_endian>::writeval(pov + 8, entry[2]);
          elfcpp::Swap<32, big_endian>::writeval(pov + 12,
              entry[3] | gotplt_entry_addr_lo);
          pov += 16;
        }

      // Now the compressed entry.  They come after any standard ones.
      if (mips_sym->has_comp_plt_offset())
        {
          if (!this->target_->is_output_micromips())
            {
              // Write MIPS16 PLT entry.
              const uint32_t* plt_entry = plt_entry_mips16_o32;

              elfcpp::Swap<16, big_endian>::writeval(pov2, plt_entry[0]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 2, plt_entry[1]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 4, plt_entry[2]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 6, plt_entry[3]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 8, plt_entry[4]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 10, plt_entry[5]);
              elfcpp::Swap<32, big_endian>::writeval(pov2 + 12,
                                                     gotplt_entry_addr);
              pov2 += 16;
            }
          else if (this->target_->use_32bit_micromips_instructions())
            {
              // Write microMIPS PLT entry in insn32 mode.
              const uint32_t* plt_entry = plt_entry_micromips32_o32;

              elfcpp::Swap<16, big_endian>::writeval(pov2, plt_entry[0]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 2,
                                                     gotplt_entry_addr_hi);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 4, plt_entry[2]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 6,
                                                     gotplt_entry_addr_lo);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 8, plt_entry[4]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 10, plt_entry[5]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 12, plt_entry[6]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 14,
                                                     gotplt_entry_addr_lo);
              pov2 += 16;
            }
          else
            {
              // Write microMIPS PLT entry.
              const uint32_t* plt_entry = plt_entry_micromips_o32;

              gold_assert(gotplt_entry_addr % 4 == 0);

              Mips_address loc_address = plt_address + pov2 - oview;
              int gotpc_offset = gotplt_entry_addr - ((loc_address | 3) ^ 3);

              // ADDIUPC has a span of +/-16MB, check we're in range.
              if (gotpc_offset + 0x1000000 >= 0x2000000)
                {
                  gold_error(_(".got.plt offset of %ld from .plt beyond the "
                             "range of ADDIUPC"), (long)gotpc_offset);
                  return;
                }

              elfcpp::Swap<16, big_endian>::writeval(pov2,
                          plt_entry[0] | ((gotpc_offset >> 18) & 0x7f));
              elfcpp::Swap<16, big_endian>::writeval(
                  pov2 + 2, (gotpc_offset >> 2) & 0xffff);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 4, plt_entry[2]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 6, plt_entry[3]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 8, plt_entry[4]);
              elfcpp::Swap<16, big_endian>::writeval(pov2 + 10, plt_entry[5]);
              pov2 += 12;
            }
        }
    }

  // Check the number of bytes written for standard entries.
  gold_assert(static_cast<section_size_type>(
      pov - oview - this->plt_header_size_) == this->plt_mips_offset_);
  // Check the number of bytes written for compressed entries.
  gold_assert((static_cast<section_size_type>(pov2 - pov)
               == this->plt_comp_offset_));
  // Check the total number of bytes written.
  gold_assert(static_cast<section_size_type>(pov2 - oview) == oview_size);

  gold_assert(static_cast<section_size_type>(gotplt_pov - gotplt_view)
              == gotplt_size);

  of->write_output_view(offset, oview_size, oview);
  of->write_output_view(gotplt_file_offset, gotplt_size, gotplt_view);
}

// Mips_output_data_mips_stubs methods.

// The format of the lazy binding stub when dynamic symbol count is less than
// 64K, dynamic symbol index is less than 32K, and ABI is not N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_normal_1[4] =
{
  0x8f998010,         // lw t9,0x8010(gp)
  0x03e07825,         // or t7,ra,zero
  0x0320f809,         // jalr t9,ra
  0x24180000          // addiu t8,zero,DYN_INDEX sign extended
};

// The format of the lazy binding stub when dynamic symbol count is less than
// 64K, dynamic symbol index is less than 32K, and ABI is N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_normal_1_n64[4] =
{
  0xdf998010,         // ld t9,0x8010(gp)
  0x03e07825,         // or t7,ra,zero
  0x0320f809,         // jalr t9,ra
  0x64180000          // daddiu t8,zero,DYN_INDEX sign extended
};

// The format of the lazy binding stub when dynamic symbol count is less than
// 64K, dynamic symbol index is between 32K and 64K, and ABI is not N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_normal_2[4] =
{
  0x8f998010,         // lw t9,0x8010(gp)
  0x03e07825,         // or t7,ra,zero
  0x0320f809,         // jalr t9,ra
  0x34180000          // ori t8,zero,DYN_INDEX unsigned
};

// The format of the lazy binding stub when dynamic symbol count is less than
// 64K, dynamic symbol index is between 32K and 64K, and ABI is N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_normal_2_n64[4] =
{
  0xdf998010,         // ld t9,0x8010(gp)
  0x03e07825,         // or t7,ra,zero
  0x0320f809,         // jalr t9,ra
  0x34180000          // ori t8,zero,DYN_INDEX unsigned
};

// The format of the lazy binding stub when dynamic symbol count is greater than
// 64K, and ABI is not N64.
template<int size, bool big_endian>
const uint32_t Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_big[5] =
{
  0x8f998010,         // lw t9,0x8010(gp)
  0x03e07825,         // or t7,ra,zero
  0x3c180000,         // lui t8,DYN_INDEX
  0x0320f809,         // jalr t9,ra
  0x37180000          // ori t8,t8,DYN_INDEX
};

// The format of the lazy binding stub when dynamic symbol count is greater than
// 64K, and ABI is N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_big_n64[5] =
{
  0xdf998010,         // ld t9,0x8010(gp)
  0x03e07825,         // or t7,ra,zero
  0x3c180000,         // lui t8,DYN_INDEX
  0x0320f809,         // jalr t9,ra
  0x37180000          // ori t8,t8,DYN_INDEX
};

// microMIPS stubs.

// The format of the microMIPS lazy binding stub when dynamic symbol count is
// less than 64K, dynamic symbol index is less than 32K, and ABI is not N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_micromips_normal_1[] =
{
  0xff3c, 0x8010,     // lw t9,0x8010(gp)
  0x0dff,             // move t7,ra
  0x45d9,             // jalr t9
  0x3300, 0x0000      // addiu t8,zero,DYN_INDEX sign extended
};

// The format of the microMIPS lazy binding stub when dynamic symbol count is
// less than 64K, dynamic symbol index is less than 32K, and ABI is N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::
lazy_stub_micromips_normal_1_n64[] =
{
  0xdf3c, 0x8010,     // ld t9,0x8010(gp)
  0x0dff,             // move t7,ra
  0x45d9,             // jalr t9
  0x5f00, 0x0000      // daddiu t8,zero,DYN_INDEX sign extended
};

// The format of the microMIPS lazy binding stub when dynamic symbol
// count is less than 64K, dynamic symbol index is between 32K and 64K,
// and ABI is not N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_micromips_normal_2[] =
{
  0xff3c, 0x8010,     // lw t9,0x8010(gp)
  0x0dff,             // move t7,ra
  0x45d9,             // jalr t9
  0x5300, 0x0000      // ori t8,zero,DYN_INDEX unsigned
};

// The format of the microMIPS lazy binding stub when dynamic symbol
// count is less than 64K, dynamic symbol index is between 32K and 64K,
// and ABI is N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::
lazy_stub_micromips_normal_2_n64[] =
{
  0xdf3c, 0x8010,     // ld t9,0x8010(gp)
  0x0dff,             // move t7,ra
  0x45d9,             // jalr t9
  0x5300, 0x0000      // ori t8,zero,DYN_INDEX unsigned
};

// The format of the microMIPS lazy binding stub when dynamic symbol count is
// greater than 64K, and ABI is not N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_micromips_big[] =
{
  0xff3c, 0x8010,     // lw t9,0x8010(gp)
  0x0dff,             // move t7,ra
  0x41b8, 0x0000,     // lui t8,DYN_INDEX
  0x45d9,             // jalr t9
  0x5318, 0x0000      // ori t8,t8,DYN_INDEX
};

// The format of the microMIPS lazy binding stub when dynamic symbol count is
// greater than 64K, and ABI is N64.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_micromips_big_n64[] =
{
  0xdf3c, 0x8010,     // ld t9,0x8010(gp)
  0x0dff,             // move t7,ra
  0x41b8, 0x0000,     // lui t8,DYN_INDEX
  0x45d9,             // jalr t9
  0x5318, 0x0000      // ori t8,t8,DYN_INDEX
};

// 32-bit microMIPS stubs.

// The format of the microMIPS lazy binding stub when dynamic symbol count is
// less than 64K, dynamic symbol index is less than 32K, ABI is not N64, and we
// can use only 32-bit instructions.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::
lazy_stub_micromips32_normal_1[] =
{
  0xff3c, 0x8010,     // lw t9,0x8010(gp)
  0x001f, 0x7a90,     // or t7,ra,zero
  0x03f9, 0x0f3c,     // jalr ra,t9
  0x3300, 0x0000      // addiu t8,zero,DYN_INDEX sign extended
};

// The format of the microMIPS lazy binding stub when dynamic symbol count is
// less than 64K, dynamic symbol index is less than 32K, ABI is N64, and we can
// use only 32-bit instructions.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::
lazy_stub_micromips32_normal_1_n64[] =
{
  0xdf3c, 0x8010,     // ld t9,0x8010(gp)
  0x001f, 0x7a90,     // or t7,ra,zero
  0x03f9, 0x0f3c,     // jalr ra,t9
  0x5f00, 0x0000      // daddiu t8,zero,DYN_INDEX sign extended
};

// The format of the microMIPS lazy binding stub when dynamic symbol
// count is less than 64K, dynamic symbol index is between 32K and 64K,
// ABI is not N64, and we can use only 32-bit instructions.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::
lazy_stub_micromips32_normal_2[] =
{
  0xff3c, 0x8010,     // lw t9,0x8010(gp)
  0x001f, 0x7a90,     // or t7,ra,zero
  0x03f9, 0x0f3c,     // jalr ra,t9
  0x5300, 0x0000      // ori t8,zero,DYN_INDEX unsigned
};

// The format of the microMIPS lazy binding stub when dynamic symbol
// count is less than 64K, dynamic symbol index is between 32K and 64K,
// ABI is N64, and we can use only 32-bit instructions.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::
lazy_stub_micromips32_normal_2_n64[] =
{
  0xdf3c, 0x8010,     // ld t9,0x8010(gp)
  0x001f, 0x7a90,     // or t7,ra,zero
  0x03f9, 0x0f3c,     // jalr ra,t9
  0x5300, 0x0000      // ori t8,zero,DYN_INDEX unsigned
};

// The format of the microMIPS lazy binding stub when dynamic symbol count is
// greater than 64K, ABI is not N64, and we can use only 32-bit instructions.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_micromips32_big[] =
{
  0xff3c, 0x8010,     // lw t9,0x8010(gp)
  0x001f, 0x7a90,     // or t7,ra,zero
  0x41b8, 0x0000,     // lui t8,DYN_INDEX
  0x03f9, 0x0f3c,     // jalr ra,t9
  0x5318, 0x0000      // ori t8,t8,DYN_INDEX
};

// The format of the microMIPS lazy binding stub when dynamic symbol count is
// greater than 64K, ABI is N64, and we can use only 32-bit instructions.
template<int size, bool big_endian>
const uint32_t
Mips_output_data_mips_stubs<size, big_endian>::lazy_stub_micromips32_big_n64[] =
{
  0xdf3c, 0x8010,     // ld t9,0x8010(gp)
  0x001f, 0x7a90,     // or t7,ra,zero
  0x41b8, 0x0000,     // lui t8,DYN_INDEX
  0x03f9, 0x0f3c,     // jalr ra,t9
  0x5318, 0x0000      // ori t8,t8,DYN_INDEX
};

// Create entry for a symbol.

template<int size, bool big_endian>
void
Mips_output_data_mips_stubs<size, big_endian>::make_entry(
    Mips_symbol<size>* gsym)
{
  if (!gsym->has_lazy_stub() && !gsym->has_plt_offset())
    {
      this->symbols_.insert(gsym);
      gsym->set_has_lazy_stub(true);
    }
}

// Remove entry for a symbol.

template<int size, bool big_endian>
void
Mips_output_data_mips_stubs<size, big_endian>::remove_entry(
    Mips_symbol<size>* gsym)
{
  if (gsym->has_lazy_stub())
    {
      this->symbols_.erase(gsym);
      gsym->set_has_lazy_stub(false);
    }
}

// Set stub offsets for symbols.  This method expects that the number of
// entries in dynamic symbol table is set.

template<int size, bool big_endian>
void
Mips_output_data_mips_stubs<size, big_endian>::set_lazy_stub_offsets()
{
  gold_assert(this->dynsym_count_ != -1U);

  if (this->stub_offsets_are_set_)
    return;

  unsigned int stub_size = this->stub_size();
  unsigned int offset = 0;
  for (typename Mips_stubs_entry_set::const_iterator
       p = this->symbols_.begin();
       p != this->symbols_.end();
       ++p, offset += stub_size)
    {
      Mips_symbol<size>* mips_sym = *p;
      mips_sym->set_lazy_stub_offset(offset);
    }
  this->stub_offsets_are_set_ = true;
}

template<int size, bool big_endian>
void
Mips_output_data_mips_stubs<size, big_endian>::set_needs_dynsym_value()
{
  for (typename Mips_stubs_entry_set::const_iterator
       p = this->symbols_.begin(); p != this->symbols_.end(); ++p)
    {
      Mips_symbol<size>* sym = *p;
      if (sym->is_from_dynobj())
        sym->set_needs_dynsym_value();
    }
}

// Write out the .MIPS.stubs.  This uses the hand-coded instructions and
// adjusts them as needed.

template<int size, bool big_endian>
void
Mips_output_data_mips_stubs<size, big_endian>::do_write(Output_file* of)
{
  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  bool big_stub = this->dynsym_count_ > 0x10000;

  unsigned char* pov = oview;
  for (typename Mips_stubs_entry_set::const_iterator
       p = this->symbols_.begin(); p != this->symbols_.end(); ++p)
    {
      Mips_symbol<size>* sym = *p;
      const uint32_t* lazy_stub;
      bool n64 = this->target_->is_output_n64();

      if (!this->target_->is_output_micromips())
        {
          // Write standard (non-microMIPS) stub.
          if (!big_stub)
            {
              if (sym->dynsym_index() & ~0x7fff)
                // Dynsym index is between 32K and 64K.
                lazy_stub = n64 ? lazy_stub_normal_2_n64 : lazy_stub_normal_2;
              else
                // Dynsym index is less than 32K.
                lazy_stub = n64 ? lazy_stub_normal_1_n64 : lazy_stub_normal_1;
            }
          else
            lazy_stub = n64 ? lazy_stub_big_n64 : lazy_stub_big;

          unsigned int i = 0;
          elfcpp::Swap<32, big_endian>::writeval(pov, lazy_stub[i]);
          elfcpp::Swap<32, big_endian>::writeval(pov + 4, lazy_stub[i + 1]);
          pov += 8;

          i += 2;
          if (big_stub)
            {
              // LUI instruction of the big stub.  Paste high 16 bits of the
              // dynsym index.
              elfcpp::Swap<32, big_endian>::writeval(pov,
                  lazy_stub[i] | ((sym->dynsym_index() >> 16) & 0x7fff));
              pov += 4;
              i += 1;
            }
          elfcpp::Swap<32, big_endian>::writeval(pov, lazy_stub[i]);
          // Last stub instruction.  Paste low 16 bits of the dynsym index.
          elfcpp::Swap<32, big_endian>::writeval(pov + 4,
              lazy_stub[i + 1] | (sym->dynsym_index() & 0xffff));
          pov += 8;
        }
      else if (this->target_->use_32bit_micromips_instructions())
        {
          // Write microMIPS stub in insn32 mode.
          if (!big_stub)
            {
              if (sym->dynsym_index() & ~0x7fff)
                // Dynsym index is between 32K and 64K.
                lazy_stub = n64 ? lazy_stub_micromips32_normal_2_n64
                                : lazy_stub_micromips32_normal_2;
              else
                // Dynsym index is less than 32K.
                lazy_stub = n64 ? lazy_stub_micromips32_normal_1_n64
                                : lazy_stub_micromips32_normal_1;
            }
          else
            lazy_stub = n64 ? lazy_stub_micromips32_big_n64
                            : lazy_stub_micromips32_big;

          unsigned int i = 0;
          // First stub instruction.  We emit 32-bit microMIPS instructions by
          // emitting two 16-bit parts because on microMIPS the 16-bit part of
          // the instruction where the opcode is must always come first, for
          // both little and big endian.
          elfcpp::Swap<16, big_endian>::writeval(pov, lazy_stub[i]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 2, lazy_stub[i + 1]);
          // Second stub instruction.
          elfcpp::Swap<16, big_endian>::writeval(pov + 4, lazy_stub[i + 2]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 6, lazy_stub[i + 3]);
          pov += 8;
          i += 4;
          if (big_stub)
            {
              // LUI instruction of the big stub.  Paste high 16 bits of the
              // dynsym index.
              elfcpp::Swap<16, big_endian>::writeval(pov, lazy_stub[i]);
              elfcpp::Swap<16, big_endian>::writeval(pov + 2,
                  (sym->dynsym_index() >> 16) & 0x7fff);
              pov += 4;
              i += 2;
            }
          elfcpp::Swap<16, big_endian>::writeval(pov, lazy_stub[i]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 2, lazy_stub[i + 1]);
          // Last stub instruction.  Paste low 16 bits of the dynsym index.
          elfcpp::Swap<16, big_endian>::writeval(pov + 4, lazy_stub[i + 2]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 6,
              sym->dynsym_index() & 0xffff);
          pov += 8;
        }
      else
        {
          // Write microMIPS stub.
          if (!big_stub)
            {
              if (sym->dynsym_index() & ~0x7fff)
                // Dynsym index is between 32K and 64K.
                lazy_stub = n64 ? lazy_stub_micromips_normal_2_n64
                                : lazy_stub_micromips_normal_2;
              else
                // Dynsym index is less than 32K.
                lazy_stub = n64 ? lazy_stub_micromips_normal_1_n64
                                : lazy_stub_micromips_normal_1;
            }
          else
            lazy_stub = n64 ? lazy_stub_micromips_big_n64
                            : lazy_stub_micromips_big;

          unsigned int i = 0;
          // First stub instruction.  We emit 32-bit microMIPS instructions by
          // emitting two 16-bit parts because on microMIPS the 16-bit part of
          // the instruction where the opcode is must always come first, for
          // both little and big endian.
          elfcpp::Swap<16, big_endian>::writeval(pov, lazy_stub[i]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 2, lazy_stub[i + 1]);
          // Second stub instruction.
          elfcpp::Swap<16, big_endian>::writeval(pov + 4, lazy_stub[i + 2]);
          pov += 6;
          i += 3;
          if (big_stub)
            {
              // LUI instruction of the big stub.  Paste high 16 bits of the
              // dynsym index.
              elfcpp::Swap<16, big_endian>::writeval(pov, lazy_stub[i]);
              elfcpp::Swap<16, big_endian>::writeval(pov + 2,
                  (sym->dynsym_index() >> 16) & 0x7fff);
              pov += 4;
              i += 2;
            }
          elfcpp::Swap<16, big_endian>::writeval(pov, lazy_stub[i]);
          // Last stub instruction.  Paste low 16 bits of the dynsym index.
          elfcpp::Swap<16, big_endian>::writeval(pov + 2, lazy_stub[i + 1]);
          elfcpp::Swap<16, big_endian>::writeval(pov + 4,
              sym->dynsym_index() & 0xffff);
          pov += 6;
        }
    }

  // We always allocate 20 bytes for every stub, because final dynsym count is
  // not known in method do_finalize_sections.  There are 4 unused bytes per
  // stub if final dynsym count is less than 0x10000.
  unsigned int used = pov - oview;
  unsigned int unused = big_stub ? 0 : this->symbols_.size() * 4;
  gold_assert(static_cast<section_size_type>(used + unused) == oview_size);

  // Fill the unused space with zeroes.
  // TODO(sasa): Can we strip unused bytes during the relaxation?
  if (unused > 0)
    memset(pov, 0, unused);

  of->write_output_view(offset, oview_size, oview);
}

// Mips_output_section_reginfo methods.

template<int size, bool big_endian>
void
Mips_output_section_reginfo<size, big_endian>::do_write(Output_file* of)
{
  off_t offset = this->offset();
  off_t data_size = this->data_size();

  unsigned char* view = of->get_output_view(offset, data_size);
  elfcpp::Swap<size, big_endian>::writeval(view, this->gprmask_);
  elfcpp::Swap<size, big_endian>::writeval(view + 4, this->cprmask1_);
  elfcpp::Swap<size, big_endian>::writeval(view + 8, this->cprmask2_);
  elfcpp::Swap<size, big_endian>::writeval(view + 12, this->cprmask3_);
  elfcpp::Swap<size, big_endian>::writeval(view + 16, this->cprmask4_);
  // Write the gp value.
  elfcpp::Swap<size, big_endian>::writeval(view + 20,
                                           this->target_->gp_value());

  of->write_output_view(offset, data_size, view);
}

// Mips_output_section_options methods.

template<int size, bool big_endian>
void
Mips_output_section_options<size, big_endian>::do_write(Output_file* of)
{
  off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* view = of->get_output_view(offset, oview_size);
  const unsigned char* end = view + oview_size;

  while (view + 8 <= end)
    {
      unsigned char kind = elfcpp::Swap<8, big_endian>::readval(view);
      unsigned char sz = elfcpp::Swap<8, big_endian>::readval(view + 1);
      if (sz < 8)
        {
          gold_error(_("Warning: bad `%s' option size %u smaller "
                       "than its header in output section"),
                     this->name(), sz);
          break;
        }

      // Only update ri_gp_value (GP register value) field of ODK_REGINFO entry.
      if (this->target_->is_output_n64() && kind == elfcpp::ODK_REGINFO)
        elfcpp::Swap<size, big_endian>::writeval(view + 32,
                                                 this->target_->gp_value());
      else if (kind == elfcpp::ODK_REGINFO)
        elfcpp::Swap<size, big_endian>::writeval(view + 28,
                                                 this->target_->gp_value());

      view += sz;
    }

  of->write_output_view(offset, oview_size, view);
}

// Mips_output_section_abiflags methods.

template<int size, bool big_endian>
void
Mips_output_section_abiflags<size, big_endian>::do_write(Output_file* of)
{
  off_t offset = this->offset();
  off_t data_size = this->data_size();

  unsigned char* view = of->get_output_view(offset, data_size);
  elfcpp::Swap<16, big_endian>::writeval(view, this->abiflags_.version);
  elfcpp::Swap<8, big_endian>::writeval(view + 2, this->abiflags_.isa_level);
  elfcpp::Swap<8, big_endian>::writeval(view + 3, this->abiflags_.isa_rev);
  elfcpp::Swap<8, big_endian>::writeval(view + 4, this->abiflags_.gpr_size);
  elfcpp::Swap<8, big_endian>::writeval(view + 5, this->abiflags_.cpr1_size);
  elfcpp::Swap<8, big_endian>::writeval(view + 6, this->abiflags_.cpr2_size);
  elfcpp::Swap<8, big_endian>::writeval(view + 7, this->abiflags_.fp_abi);
  elfcpp::Swap<32, big_endian>::writeval(view + 8, this->abiflags_.isa_ext);
  elfcpp::Swap<32, big_endian>::writeval(view + 12, this->abiflags_.ases);
  elfcpp::Swap<32, big_endian>::writeval(view + 16, this->abiflags_.flags1);
  elfcpp::Swap<32, big_endian>::writeval(view + 20, this->abiflags_.flags2);

  of->write_output_view(offset, data_size, view);
}

// Mips_copy_relocs methods.

// Emit any saved relocs.

template<int sh_type, int size, bool big_endian>
void
Mips_copy_relocs<sh_type, size, big_endian>::emit_mips(
    Output_data_reloc<sh_type, true, size, big_endian>* reloc_section,
    Symbol_table* symtab, Layout* layout, Target_mips<size, big_endian>* target)
{
  for (typename Copy_relocs<sh_type, size, big_endian>::
       Copy_reloc_entries::iterator p = this->entries_.begin();
       p != this->entries_.end();
       ++p)
    emit_entry(*p, reloc_section, symtab, layout, target);

  // We no longer need the saved information.
  this->entries_.clear();
}

// Emit the reloc if appropriate.

template<int sh_type, int size, bool big_endian>
void
Mips_copy_relocs<sh_type, size, big_endian>::emit_entry(
    Copy_reloc_entry& entry,
    Output_data_reloc<sh_type, true, size, big_endian>* reloc_section,
    Symbol_table* symtab, Layout* layout, Target_mips<size, big_endian>* target)
{
  // If the symbol is no longer defined in a dynamic object, then we
  // emitted a COPY relocation, and we do not want to emit this
  // dynamic relocation.
  if (!entry.sym_->is_from_dynobj())
    return;

  bool can_make_dynamic = (entry.reloc_type_ == elfcpp::R_MIPS_32
                           || entry.reloc_type_ == elfcpp::R_MIPS_REL32
                           || entry.reloc_type_ == elfcpp::R_MIPS_64);

  Mips_symbol<size>* sym = Mips_symbol<size>::as_mips_sym(entry.sym_);
  if (can_make_dynamic && !sym->has_static_relocs())
    {
      Mips_relobj<size, big_endian>* object =
        Mips_relobj<size, big_endian>::as_mips_relobj(entry.relobj_);
      target->got_section(symtab, layout)->record_global_got_symbol(
                          sym, object, entry.reloc_type_, true, false);
      if (!symbol_references_local(sym, sym->should_add_dynsym_entry(symtab)))
        target->rel_dyn_section(layout)->add_global(sym, elfcpp::R_MIPS_REL32,
            entry.output_section_, entry.relobj_, entry.shndx_, entry.address_);
      else
        target->rel_dyn_section(layout)->add_symbolless_global_addend(
            sym, elfcpp::R_MIPS_REL32, entry.output_section_, entry.relobj_,
            entry.shndx_, entry.address_);
    }
  else
    this->make_copy_reloc(symtab, layout,
                          static_cast<Sized_symbol<size>*>(entry.sym_),
                          entry.relobj_,
                          reloc_section);
}

// Target_mips methods.

// Return the value to use for a dynamic symbol which requires special
// treatment.  This is how we support equality comparisons of function
// pointers across shared library boundaries, as described in the
// processor specific ABI supplement.

template<int size, bool big_endian>
uint64_t
Target_mips<size, big_endian>::do_dynsym_value(const Symbol* gsym) const
{
  uint64_t value = 0;
  const Mips_symbol<size>* mips_sym = Mips_symbol<size>::as_mips_sym(gsym);

  if (!mips_sym->has_lazy_stub())
    {
      if (mips_sym->has_plt_offset())
        {
          // We distinguish between PLT entries and lazy-binding stubs by
          // giving the former an st_other value of STO_MIPS_PLT.  Set the
          // value to the stub address if there are any relocations in the
          // binary where pointer equality matters.
          if (mips_sym->pointer_equality_needed())
            {
              // Prefer a standard MIPS PLT entry.
              if (mips_sym->has_mips_plt_offset())
                value = this->plt_section()->mips_entry_address(mips_sym);
              else
                value = this->plt_section()->comp_entry_address(mips_sym) + 1;
            }
          else
            value = 0;
        }
    }
  else
    {
      // First, set stub offsets for symbols.  This method expects that the
      // number of entries in dynamic symbol table is set.
      this->mips_stubs_section()->set_lazy_stub_offsets();

      // The run-time linker uses the st_value field of the symbol
      // to reset the global offset table entry for this external
      // to its stub address when unlinking a shared object.
      value = this->mips_stubs_section()->stub_address(mips_sym);
    }

  if (mips_sym->has_mips16_fn_stub())
    {
      // If we have a MIPS16 function with a stub, the dynamic symbol must
      // refer to the stub, since only the stub uses the standard calling
      // conventions.
      value = mips_sym->template
              get_mips16_fn_stub<big_endian>()->output_address();
    }

  return value;
}

// Get the dynamic reloc section, creating it if necessary.  It's always
// .rel.dyn, even for MIPS64.

template<int size, bool big_endian>
typename Target_mips<size, big_endian>::Reloc_section*
Target_mips<size, big_endian>::rel_dyn_section(Layout* layout)
{
  if (this->rel_dyn_ == NULL)
    {
      gold_assert(layout != NULL);
      this->rel_dyn_ = new Reloc_section(parameters->options().combreloc());
      layout->add_output_section_data(".rel.dyn", elfcpp::SHT_REL,
                                      elfcpp::SHF_ALLOC, this->rel_dyn_,
                                      ORDER_DYNAMIC_RELOCS, false);

      // First entry in .rel.dyn has to be null.
      // This is hack - we define dummy output data and set its address to 0,
      // and define absolute R_MIPS_NONE relocation with offset 0 against it.
      // This ensures that the entry is null.
      Output_data* od = new Output_data_zero_fill(0, 0);
      od->set_address(0);
      this->rel_dyn_->add_absolute(elfcpp::R_MIPS_NONE, od, 0);
    }
  return this->rel_dyn_;
}

// Get the GOT section, creating it if necessary.

template<int size, bool big_endian>
Mips_output_data_got<size, big_endian>*
Target_mips<size, big_endian>::got_section(Symbol_table* symtab,
                                           Layout* layout)
{
  if (this->got_ == NULL)
    {
      gold_assert(symtab != NULL && layout != NULL);

      this->got_ = new Mips_output_data_got<size, big_endian>(this, symtab,
                                                              layout);
      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE |
                                      elfcpp::SHF_MIPS_GPREL),
                                      this->got_, ORDER_DATA, false);

      // Define _GLOBAL_OFFSET_TABLE_ at the start of the .got section.
      symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
                                    Symbol_table::PREDEFINED,
                                    this->got_,
                                    0, 0, elfcpp::STT_OBJECT,
                                    elfcpp::STB_GLOBAL,
                                    elfcpp::STV_HIDDEN, 0,
                                    false, false);
    }

  return this->got_;
}

// Calculate value of _gp symbol.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::set_gp(Layout* layout, Symbol_table* symtab)
{
  gold_assert(this->gp_ == NULL);

  Sized_symbol<size>* gp =
    static_cast<Sized_symbol<size>*>(symtab->lookup("_gp"));

  // Set _gp symbol if the linker script hasn't created it.
  if (gp == NULL || gp->source() != Symbol::IS_CONSTANT)
    {
      // If there is no .got section, gp should be based on .sdata.
      Output_data* gp_section = (this->got_ != NULL
                                 ? this->got_->output_section()
                                 : layout->find_output_section(".sdata"));

      if (gp_section != NULL)
        gp = static_cast<Sized_symbol<size>*>(symtab->define_in_output_data(
                                          "_gp", NULL, Symbol_table::PREDEFINED,
                                          gp_section, MIPS_GP_OFFSET, 0,
                                          elfcpp::STT_NOTYPE,
                                          elfcpp::STB_LOCAL,
                                          elfcpp::STV_DEFAULT,
                                          0, false, false));
    }

  this->gp_ = gp;
}

// Set the dynamic symbol indexes.  INDEX is the index of the first
// global dynamic symbol.  Pointers to the symbols are stored into the
// vector SYMS.  The names are added to DYNPOOL.  This returns an
// updated dynamic symbol index.

template<int size, bool big_endian>
unsigned int
Target_mips<size, big_endian>::do_set_dynsym_indexes(
    std::vector<Symbol*>* dyn_symbols, unsigned int index,
    std::vector<Symbol*>* syms, Stringpool* dynpool,
    Versions* versions, Symbol_table* symtab) const
{
  std::vector<Symbol*> non_got_symbols;
  std::vector<Symbol*> got_symbols;

  reorder_dyn_symbols<size, big_endian>(dyn_symbols, &non_got_symbols,
                                        &got_symbols);

  for (std::vector<Symbol*>::iterator p = non_got_symbols.begin();
       p != non_got_symbols.end();
       ++p)
    {
      Symbol* sym = *p;

      // Note that SYM may already have a dynamic symbol index, since
      // some symbols appear more than once in the symbol table, with
      // and without a version.

      if (!sym->has_dynsym_index())
        {
          sym->set_dynsym_index(index);
          ++index;
          syms->push_back(sym);
          dynpool->add(sym->name(), false, NULL);

          // Record any version information.
          if (sym->version() != NULL)
            versions->record_version(symtab, dynpool, sym);

          // If the symbol is defined in a dynamic object and is
          // referenced in a regular object, then mark the dynamic
          // object as needed.  This is used to implement --as-needed.
          if (sym->is_from_dynobj() && sym->in_reg())
            sym->object()->set_is_needed();
        }
    }

  for (std::vector<Symbol*>::iterator p = got_symbols.begin();
       p != got_symbols.end();
       ++p)
    {
      Symbol* sym = *p;
      if (!sym->has_dynsym_index())
        {
          // Record any version information.
          if (sym->version() != NULL)
            versions->record_version(symtab, dynpool, sym);
        }
    }

  index = versions->finalize(symtab, index, syms);

  int got_sym_count = 0;
  for (std::vector<Symbol*>::iterator p = got_symbols.begin();
       p != got_symbols.end();
       ++p)
    {
      Symbol* sym = *p;

      if (!sym->has_dynsym_index())
        {
          ++got_sym_count;
          sym->set_dynsym_index(index);
          ++index;
          syms->push_back(sym);
          dynpool->add(sym->name(), false, NULL);

          // If the symbol is defined in a dynamic object and is
          // referenced in a regular object, then mark the dynamic
          // object as needed.  This is used to implement --as-needed.
          if (sym->is_from_dynobj() && sym->in_reg())
            sym->object()->set_is_needed();
        }
    }

  // Set index of the first symbol that has .got entry.
  this->got_->set_first_global_got_dynsym_index(
    got_sym_count > 0 ? index - got_sym_count : -1U);

  if (this->mips_stubs_ != NULL)
    this->mips_stubs_->set_dynsym_count(index);

  return index;
}

// Create a PLT entry for a global symbol referenced by r_type relocation.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::make_plt_entry(Symbol_table* symtab,
                                              Layout* layout,
                                              Mips_symbol<size>* gsym,
                                              unsigned int r_type)
{
  if (gsym->has_lazy_stub() || gsym->has_plt_offset())
    return;

  if (this->plt_ == NULL)
    {
      // Create the GOT section first.
      this->got_section(symtab, layout);

      this->got_plt_ = new Output_data_space(4, "** GOT PLT");
      layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE),
                                      this->got_plt_, ORDER_DATA, false);

      // The first two entries are reserved.
      this->got_plt_->set_current_data_size(2 * size/8);

      this->plt_ = new Mips_output_data_plt<size, big_endian>(layout,
                                                              this->got_plt_,
                                                              this);
      layout->add_output_section_data(".plt", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC
                                       | elfcpp::SHF_EXECINSTR),
                                      this->plt_, ORDER_PLT, false);

      // Make the sh_info field of .rel.plt point to .plt.
      Output_section* rel_plt_os = this->plt_->rel_plt()->output_section();
      rel_plt_os->set_info_section(this->plt_->output_section());
    }

  this->plt_->add_entry(gsym, r_type);
}


// Get the .MIPS.stubs section, creating it if necessary.

template<int size, bool big_endian>
Mips_output_data_mips_stubs<size, big_endian>*
Target_mips<size, big_endian>::mips_stubs_section(Layout* layout)
{
  if (this->mips_stubs_ == NULL)
    {
      this->mips_stubs_ =
        new Mips_output_data_mips_stubs<size, big_endian>(this);
      layout->add_output_section_data(".MIPS.stubs", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC
                                       | elfcpp::SHF_EXECINSTR),
                                      this->mips_stubs_, ORDER_PLT, false);
    }
  return this->mips_stubs_;
}

// Get the LA25 stub section, creating it if necessary.

template<int size, bool big_endian>
Mips_output_data_la25_stub<size, big_endian>*
Target_mips<size, big_endian>::la25_stub_section(Layout* layout)
{
  if (this->la25_stub_ == NULL)
    {
      this->la25_stub_ = new Mips_output_data_la25_stub<size, big_endian>();
      layout->add_output_section_data(".text", elfcpp::SHT_PROGBITS,
                                      (elfcpp::SHF_ALLOC
                                       | elfcpp::SHF_EXECINSTR),
                                      this->la25_stub_, ORDER_TEXT, false);
    }
  return this->la25_stub_;
}

// Process the relocations to determine unreferenced sections for
// garbage collection.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::gc_process_relocs(
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
  typedef Target_mips<size, big_endian> Mips;

  if (sh_type == elfcpp::SHT_REL)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>
          Classify_reloc;

      gold::gc_process_relocs<size, big_endian, Mips, Scan, Classify_reloc>(
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
  else if (sh_type == elfcpp::SHT_RELA)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
          Classify_reloc;

      gold::gc_process_relocs<size, big_endian, Mips, Scan, Classify_reloc>(
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
  else
    gold_unreachable();
}

// Scan relocations for a section.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::scan_relocs(
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
  typedef Target_mips<size, big_endian> Mips;

  if (sh_type == elfcpp::SHT_REL)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>
	  Classify_reloc;

      gold::scan_relocs<size, big_endian, Mips, Scan, Classify_reloc>(
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
  else if (sh_type == elfcpp::SHT_RELA)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
	  Classify_reloc;

      gold::scan_relocs<size, big_endian, Mips, Scan, Classify_reloc>(
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
}

template<int size, bool big_endian>
bool
Target_mips<size, big_endian>::mips_32bit_flags(elfcpp::Elf_Word flags)
{
  return ((flags & elfcpp::EF_MIPS_32BITMODE) != 0
          || (flags & elfcpp::EF_MIPS_ABI) == elfcpp::E_MIPS_ABI_O32
          || (flags & elfcpp::EF_MIPS_ABI) == elfcpp::E_MIPS_ABI_EABI32
          || (flags & elfcpp::EF_MIPS_ARCH) == elfcpp::E_MIPS_ARCH_1
          || (flags & elfcpp::EF_MIPS_ARCH) == elfcpp::E_MIPS_ARCH_2
          || (flags & elfcpp::EF_MIPS_ARCH) == elfcpp::E_MIPS_ARCH_32
          || (flags & elfcpp::EF_MIPS_ARCH) == elfcpp::E_MIPS_ARCH_32R2
          || (flags & elfcpp::EF_MIPS_ARCH) == elfcpp::E_MIPS_ARCH_32R6);
}

// Return the MACH for a MIPS e_flags value.
template<int size, bool big_endian>
unsigned int
Target_mips<size, big_endian>::elf_mips_mach(elfcpp::Elf_Word flags)
{
  switch (flags & elfcpp::EF_MIPS_MACH)
    {
    case elfcpp::E_MIPS_MACH_3900:
      return mach_mips3900;

    case elfcpp::E_MIPS_MACH_4010:
      return mach_mips4010;

    case elfcpp::E_MIPS_MACH_4100:
      return mach_mips4100;

    case elfcpp::E_MIPS_MACH_4111:
      return mach_mips4111;

    case elfcpp::E_MIPS_MACH_4120:
      return mach_mips4120;

    case elfcpp::E_MIPS_MACH_4650:
      return mach_mips4650;

    case elfcpp::E_MIPS_MACH_5400:
      return mach_mips5400;

    case elfcpp::E_MIPS_MACH_5500:
      return mach_mips5500;

    case elfcpp::E_MIPS_MACH_5900:
      return mach_mips5900;

    case elfcpp::E_MIPS_MACH_9000:
      return mach_mips9000;

    case elfcpp::E_MIPS_MACH_SB1:
      return mach_mips_sb1;

    case elfcpp::E_MIPS_MACH_LS2E:
      return mach_mips_loongson_2e;

    case elfcpp::E_MIPS_MACH_LS2F:
      return mach_mips_loongson_2f;

    case elfcpp::E_MIPS_MACH_GS464:
      return mach_mips_gs464;

    case elfcpp::E_MIPS_MACH_GS464E:
      return mach_mips_gs464e;

    case elfcpp::E_MIPS_MACH_GS264E:
      return mach_mips_gs264e;

    case elfcpp::E_MIPS_MACH_OCTEON3:
      return mach_mips_octeon3;

    case elfcpp::E_MIPS_MACH_OCTEON2:
      return mach_mips_octeon2;

    case elfcpp::E_MIPS_MACH_OCTEON:
      return mach_mips_octeon;

    case elfcpp::E_MIPS_MACH_XLR:
      return mach_mips_xlr;

    default:
      switch (flags & elfcpp::EF_MIPS_ARCH)
        {
        default:
        case elfcpp::E_MIPS_ARCH_1:
          return mach_mips3000;

        case elfcpp::E_MIPS_ARCH_2:
          return mach_mips6000;

        case elfcpp::E_MIPS_ARCH_3:
          return mach_mips4000;

        case elfcpp::E_MIPS_ARCH_4:
          return mach_mips8000;

        case elfcpp::E_MIPS_ARCH_5:
          return mach_mips5;

        case elfcpp::E_MIPS_ARCH_32:
          return mach_mipsisa32;

        case elfcpp::E_MIPS_ARCH_64:
          return mach_mipsisa64;

        case elfcpp::E_MIPS_ARCH_32R2:
          return mach_mipsisa32r2;

        case elfcpp::E_MIPS_ARCH_32R6:
          return mach_mipsisa32r6;

        case elfcpp::E_MIPS_ARCH_64R2:
          return mach_mipsisa64r2;

        case elfcpp::E_MIPS_ARCH_64R6:
          return mach_mipsisa64r6;
        }
    }

  return 0;
}

// Return the MACH for each .MIPS.abiflags ISA Extension.

template<int size, bool big_endian>
unsigned int
Target_mips<size, big_endian>::mips_isa_ext_mach(unsigned int isa_ext)
{
  switch (isa_ext)
    {
    case elfcpp::AFL_EXT_3900:
      return mach_mips3900;

    case elfcpp::AFL_EXT_4010:
      return mach_mips4010;

    case elfcpp::AFL_EXT_4100:
      return mach_mips4100;

    case elfcpp::AFL_EXT_4111:
      return mach_mips4111;

    case elfcpp::AFL_EXT_4120:
      return mach_mips4120;

    case elfcpp::AFL_EXT_4650:
      return mach_mips4650;

    case elfcpp::AFL_EXT_5400:
      return mach_mips5400;

    case elfcpp::AFL_EXT_5500:
      return mach_mips5500;

    case elfcpp::AFL_EXT_5900:
      return mach_mips5900;

    case elfcpp::AFL_EXT_10000:
      return mach_mips10000;

    case elfcpp::AFL_EXT_LOONGSON_2E:
      return mach_mips_loongson_2e;

    case elfcpp::AFL_EXT_LOONGSON_2F:
      return mach_mips_loongson_2f;

    case elfcpp::AFL_EXT_SB1:
      return mach_mips_sb1;

    case elfcpp::AFL_EXT_OCTEON:
      return mach_mips_octeon;

    case elfcpp::AFL_EXT_OCTEONP:
      return mach_mips_octeonp;

    case elfcpp::AFL_EXT_OCTEON2:
      return mach_mips_octeon2;

    case elfcpp::AFL_EXT_XLR:
      return mach_mips_xlr;

    default:
      return mach_mips3000;
    }
}

// Return the .MIPS.abiflags value representing each ISA Extension.

template<int size, bool big_endian>
unsigned int
Target_mips<size, big_endian>::mips_isa_ext(unsigned int mips_mach)
{
  switch (mips_mach)
    {
    case mach_mips3900:
      return elfcpp::AFL_EXT_3900;

    case mach_mips4010:
      return elfcpp::AFL_EXT_4010;

    case mach_mips4100:
      return elfcpp::AFL_EXT_4100;

    case mach_mips4111:
      return elfcpp::AFL_EXT_4111;

    case mach_mips4120:
      return elfcpp::AFL_EXT_4120;

    case mach_mips4650:
      return elfcpp::AFL_EXT_4650;

    case mach_mips5400:
      return elfcpp::AFL_EXT_5400;

    case mach_mips5500:
      return elfcpp::AFL_EXT_5500;

    case mach_mips5900:
      return elfcpp::AFL_EXT_5900;

    case mach_mips10000:
      return elfcpp::AFL_EXT_10000;

    case mach_mips_loongson_2e:
      return elfcpp::AFL_EXT_LOONGSON_2E;

    case mach_mips_loongson_2f:
      return elfcpp::AFL_EXT_LOONGSON_2F;

    case mach_mips_sb1:
      return elfcpp::AFL_EXT_SB1;

    case mach_mips_octeon:
      return elfcpp::AFL_EXT_OCTEON;

    case mach_mips_octeonp:
      return elfcpp::AFL_EXT_OCTEONP;

    case mach_mips_octeon3:
      return elfcpp::AFL_EXT_OCTEON3;

    case mach_mips_octeon2:
      return elfcpp::AFL_EXT_OCTEON2;

    case mach_mips_xlr:
      return elfcpp::AFL_EXT_XLR;

    default:
      return 0;
    }
}

// Update the isa_level, isa_rev, isa_ext fields of abiflags.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::update_abiflags_isa(const std::string& name,
    elfcpp::Elf_Word e_flags, Mips_abiflags<big_endian>* abiflags)
{
  int new_isa = 0;
  switch (e_flags & elfcpp::EF_MIPS_ARCH)
    {
    case elfcpp::E_MIPS_ARCH_1:
      new_isa = this->level_rev(1, 0);
      break;
    case elfcpp::E_MIPS_ARCH_2:
      new_isa = this->level_rev(2, 0);
      break;
    case elfcpp::E_MIPS_ARCH_3:
      new_isa = this->level_rev(3, 0);
      break;
    case elfcpp::E_MIPS_ARCH_4:
      new_isa = this->level_rev(4, 0);
      break;
    case elfcpp::E_MIPS_ARCH_5:
      new_isa = this->level_rev(5, 0);
      break;
    case elfcpp::E_MIPS_ARCH_32:
      new_isa = this->level_rev(32, 1);
      break;
    case elfcpp::E_MIPS_ARCH_32R2:
      new_isa = this->level_rev(32, 2);
      break;
    case elfcpp::E_MIPS_ARCH_32R6:
      new_isa = this->level_rev(32, 6);
      break;
    case elfcpp::E_MIPS_ARCH_64:
      new_isa = this->level_rev(64, 1);
      break;
    case elfcpp::E_MIPS_ARCH_64R2:
      new_isa = this->level_rev(64, 2);
      break;
    case elfcpp::E_MIPS_ARCH_64R6:
      new_isa = this->level_rev(64, 6);
      break;
    default:
      gold_error(_("%s: Unknown architecture %s"), name.c_str(),
                 this->elf_mips_mach_name(e_flags));
    }

  if (new_isa > this->level_rev(abiflags->isa_level, abiflags->isa_rev))
    {
      // Decode a single value into level and revision.
      abiflags->isa_level = new_isa >> 3;
      abiflags->isa_rev = new_isa & 0x7;
    }

  // Update the isa_ext if needed.
  if (this->mips_mach_extends(this->mips_isa_ext_mach(abiflags->isa_ext),
      this->elf_mips_mach(e_flags)))
    abiflags->isa_ext = this->mips_isa_ext(this->elf_mips_mach(e_flags));
}

// Infer the content of the ABI flags based on the elf header.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::infer_abiflags(
    Mips_relobj<size, big_endian>* relobj, Mips_abiflags<big_endian>* abiflags)
{
  const Attributes_section_data* pasd = relobj->attributes_section_data();
  int attr_fp_abi = elfcpp::Val_GNU_MIPS_ABI_FP_ANY;
  elfcpp::Elf_Word e_flags = relobj->processor_specific_flags();

  this->update_abiflags_isa(relobj->name(), e_flags, abiflags);
  if (pasd != NULL)
    {
      // Read fp_abi from the .gnu.attribute section.
      const Object_attribute* attr =
        pasd->known_attributes(Object_attribute::OBJ_ATTR_GNU);
      attr_fp_abi = attr[elfcpp::Tag_GNU_MIPS_ABI_FP].int_value();
    }

  abiflags->fp_abi = attr_fp_abi;
  abiflags->cpr1_size = elfcpp::AFL_REG_NONE;
  abiflags->cpr2_size = elfcpp::AFL_REG_NONE;
  abiflags->gpr_size = this->mips_32bit_flags(e_flags) ? elfcpp::AFL_REG_32
                                                       : elfcpp::AFL_REG_64;

  if (abiflags->fp_abi == elfcpp::Val_GNU_MIPS_ABI_FP_SINGLE
      || abiflags->fp_abi == elfcpp::Val_GNU_MIPS_ABI_FP_XX
      || (abiflags->fp_abi == elfcpp::Val_GNU_MIPS_ABI_FP_DOUBLE
      && abiflags->gpr_size == elfcpp::AFL_REG_32))
    abiflags->cpr1_size = elfcpp::AFL_REG_32;
  else if (abiflags->fp_abi == elfcpp::Val_GNU_MIPS_ABI_FP_DOUBLE
           || abiflags->fp_abi == elfcpp::Val_GNU_MIPS_ABI_FP_64
           || abiflags->fp_abi == elfcpp::Val_GNU_MIPS_ABI_FP_64A)
    abiflags->cpr1_size = elfcpp::AFL_REG_64;

  if (e_flags & elfcpp::EF_MIPS_ARCH_ASE_MDMX)
    abiflags->ases |= elfcpp::AFL_ASE_MDMX;
  if (e_flags & elfcpp::EF_MIPS_ARCH_ASE_M16)
    abiflags->ases |= elfcpp::AFL_ASE_MIPS16;
  if (e_flags & elfcpp::EF_MIPS_ARCH_ASE_MICROMIPS)
    abiflags->ases |= elfcpp::AFL_ASE_MICROMIPS;

  if (abiflags->fp_abi != elfcpp::Val_GNU_MIPS_ABI_FP_ANY
      && abiflags->fp_abi != elfcpp::Val_GNU_MIPS_ABI_FP_SOFT
      && abiflags->fp_abi != elfcpp::Val_GNU_MIPS_ABI_FP_64A
      && abiflags->isa_level >= 32
      && abiflags->ases != elfcpp::AFL_ASE_LOONGSON_EXT)
    abiflags->flags1 |= elfcpp::AFL_FLAGS1_ODDSPREG;
}

// Create abiflags from elf header or from .MIPS.abiflags section.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::create_abiflags(
    Mips_relobj<size, big_endian>* relobj,
    Mips_abiflags<big_endian>* abiflags)
{
  Mips_abiflags<big_endian>* sec_abiflags = relobj->abiflags();
  Mips_abiflags<big_endian> header_abiflags;

  this->infer_abiflags(relobj, &header_abiflags);

  if (sec_abiflags == NULL)
    {
      // If there is no input .MIPS.abiflags section, use abiflags created
      // from elf header.
      *abiflags = header_abiflags;
      return;
    }

  this->has_abiflags_section_ = true;

  // It is not possible to infer the correct ISA revision for R3 or R5
  // so drop down to R2 for the checks.
  unsigned char isa_rev = sec_abiflags->isa_rev;
  if (isa_rev == 3 || isa_rev == 5)
    isa_rev = 2;

  // Check compatibility between abiflags created from elf header
  // and abiflags from .MIPS.abiflags section in this object file.
  if (this->level_rev(sec_abiflags->isa_level, isa_rev)
      < this->level_rev(header_abiflags.isa_level, header_abiflags.isa_rev))
    gold_warning(_("%s: Inconsistent ISA between e_flags and .MIPS.abiflags"),
                 relobj->name().c_str());
  if (header_abiflags.fp_abi != elfcpp::Val_GNU_MIPS_ABI_FP_ANY
      && sec_abiflags->fp_abi != header_abiflags.fp_abi)
    gold_warning(_("%s: Inconsistent FP ABI between .gnu.attributes and "
                   ".MIPS.abiflags"), relobj->name().c_str());
  if ((sec_abiflags->ases & header_abiflags.ases) != header_abiflags.ases)
    gold_warning(_("%s: Inconsistent ASEs between e_flags and .MIPS.abiflags"),
                 relobj->name().c_str());
  // The isa_ext is allowed to be an extension of what can be inferred
  // from e_flags.
  if (!this->mips_mach_extends(this->mips_isa_ext_mach(header_abiflags.isa_ext),
                               this->mips_isa_ext_mach(sec_abiflags->isa_ext)))
    gold_warning(_("%s: Inconsistent ISA extensions between e_flags and "
                   ".MIPS.abiflags"), relobj->name().c_str());
  if (sec_abiflags->flags2 != 0)
    gold_warning(_("%s: Unexpected flag in the flags2 field of "
                   ".MIPS.abiflags (0x%x)"), relobj->name().c_str(),
                                             sec_abiflags->flags2);
  // Use abiflags from .MIPS.abiflags section.
  *abiflags = *sec_abiflags;
}

// Return the meaning of fp_abi, or "unknown" if not known.

template<int size, bool big_endian>
const char*
Target_mips<size, big_endian>::fp_abi_string(int fp)
{
  switch (fp)
    {
    case elfcpp::Val_GNU_MIPS_ABI_FP_DOUBLE:
      return "-mdouble-float";
    case elfcpp::Val_GNU_MIPS_ABI_FP_SINGLE:
      return "-msingle-float";
    case elfcpp::Val_GNU_MIPS_ABI_FP_SOFT:
      return "-msoft-float";
    case elfcpp::Val_GNU_MIPS_ABI_FP_OLD_64:
      return _("-mips32r2 -mfp64 (12 callee-saved)");
    case elfcpp::Val_GNU_MIPS_ABI_FP_XX:
      return "-mfpxx";
    case elfcpp::Val_GNU_MIPS_ABI_FP_64:
      return "-mgp32 -mfp64";
    case elfcpp::Val_GNU_MIPS_ABI_FP_64A:
      return "-mgp32 -mfp64 -mno-odd-spreg";
    default:
      return "unknown";
    }
}

// Select fp_abi.

template<int size, bool big_endian>
int
Target_mips<size, big_endian>::select_fp_abi(const std::string& name, int in_fp,
                                             int out_fp)
{
  if (in_fp == out_fp)
    return out_fp;

  if (out_fp == elfcpp::Val_GNU_MIPS_ABI_FP_ANY)
    return in_fp;
  else if (out_fp == elfcpp::Val_GNU_MIPS_ABI_FP_XX
           && (in_fp == elfcpp::Val_GNU_MIPS_ABI_FP_DOUBLE
               || in_fp == elfcpp::Val_GNU_MIPS_ABI_FP_64
               || in_fp == elfcpp::Val_GNU_MIPS_ABI_FP_64A))
    return in_fp;
  else if (in_fp == elfcpp::Val_GNU_MIPS_ABI_FP_XX
           && (out_fp == elfcpp::Val_GNU_MIPS_ABI_FP_DOUBLE
               || out_fp == elfcpp::Val_GNU_MIPS_ABI_FP_64
               || out_fp == elfcpp::Val_GNU_MIPS_ABI_FP_64A))
    return out_fp; // Keep the current setting.
  else if (out_fp == elfcpp::Val_GNU_MIPS_ABI_FP_64A
           && in_fp == elfcpp::Val_GNU_MIPS_ABI_FP_64)
    return in_fp;
  else if (in_fp == elfcpp::Val_GNU_MIPS_ABI_FP_64A
           && out_fp == elfcpp::Val_GNU_MIPS_ABI_FP_64)
    return out_fp; // Keep the current setting.
  else if (in_fp != elfcpp::Val_GNU_MIPS_ABI_FP_ANY)
    gold_warning(_("%s: FP ABI %s is incompatible with %s"), name.c_str(),
                 fp_abi_string(in_fp), fp_abi_string(out_fp));
  return out_fp;
}

// Merge attributes from input object.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::merge_obj_attributes(const std::string& name,
    const Attributes_section_data* pasd)
{
  // Return if there is no attributes section data.
  if (pasd == NULL)
    return;

  // If output has no object attributes, just copy.
  if (this->attributes_section_data_ == NULL)
    {
      this->attributes_section_data_ = new Attributes_section_data(*pasd);
      return;
    }

  Object_attribute* out_attr = this->attributes_section_data_->known_attributes(
      Object_attribute::OBJ_ATTR_GNU);

  out_attr[elfcpp::Tag_GNU_MIPS_ABI_FP].set_type(1);
  out_attr[elfcpp::Tag_GNU_MIPS_ABI_FP].set_int_value(this->abiflags_->fp_abi);

  // Merge Tag_compatibility attributes and any common GNU ones.
  this->attributes_section_data_->merge(name.c_str(), pasd);
}

// Merge abiflags from input object.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::merge_obj_abiflags(const std::string& name,
    Mips_abiflags<big_endian>* in_abiflags)
{
  // If output has no abiflags, just copy.
  if (this->abiflags_ == NULL)
  {
    this->abiflags_ = new Mips_abiflags<big_endian>(*in_abiflags);
    return;
  }

  this->abiflags_->fp_abi = this->select_fp_abi(name, in_abiflags->fp_abi,
                                                this->abiflags_->fp_abi);

  // Merge abiflags.
  this->abiflags_->isa_level = std::max(this->abiflags_->isa_level,
                                        in_abiflags->isa_level);
  this->abiflags_->isa_rev = std::max(this->abiflags_->isa_rev,
                                      in_abiflags->isa_rev);
  this->abiflags_->gpr_size = std::max(this->abiflags_->gpr_size,
                                       in_abiflags->gpr_size);
  this->abiflags_->cpr1_size = std::max(this->abiflags_->cpr1_size,
                                        in_abiflags->cpr1_size);
  this->abiflags_->cpr2_size = std::max(this->abiflags_->cpr2_size,
                                        in_abiflags->cpr2_size);
  this->abiflags_->ases |= in_abiflags->ases;
  this->abiflags_->flags1 |= in_abiflags->flags1;
}

// Check whether machine EXTENSION is an extension of machine BASE.
template<int size, bool big_endian>
bool
Target_mips<size, big_endian>::mips_mach_extends(unsigned int base,
                                                 unsigned int extension)
{
  if (extension == base)
    return true;

  if ((base == mach_mipsisa32)
      && this->mips_mach_extends(mach_mipsisa64, extension))
    return true;

  if ((base == mach_mipsisa32r2)
      && this->mips_mach_extends(mach_mipsisa64r2, extension))
    return true;

  for (unsigned int i = 0; i < this->mips_mach_extensions_.size(); ++i)
    if (extension == this->mips_mach_extensions_[i].first)
      {
        extension = this->mips_mach_extensions_[i].second;
        if (extension == base)
          return true;
      }

  return false;
}

// Merge file header flags from input object.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::merge_obj_e_flags(const std::string& name,
                                                 elfcpp::Elf_Word in_flags)
{
  // If flags are not set yet, just copy them.
  if (!this->are_processor_specific_flags_set())
    {
      this->set_processor_specific_flags(in_flags);
      this->mach_ = this->elf_mips_mach(in_flags);
      return;
    }

  elfcpp::Elf_Word new_flags = in_flags;
  elfcpp::Elf_Word old_flags = this->processor_specific_flags();
  elfcpp::Elf_Word merged_flags = this->processor_specific_flags();
  merged_flags |= new_flags & elfcpp::EF_MIPS_NOREORDER;

  // Check flag compatibility.
  new_flags &= ~elfcpp::EF_MIPS_NOREORDER;
  old_flags &= ~elfcpp::EF_MIPS_NOREORDER;

  // Some IRIX 6 BSD-compatibility objects have this bit set.  It
  // doesn't seem to matter.
  new_flags &= ~elfcpp::EF_MIPS_XGOT;
  old_flags &= ~elfcpp::EF_MIPS_XGOT;

  // MIPSpro generates ucode info in n64 objects.  Again, we should
  // just be able to ignore this.
  new_flags &= ~elfcpp::EF_MIPS_UCODE;
  old_flags &= ~elfcpp::EF_MIPS_UCODE;

  if (new_flags == old_flags)
    {
      this->set_processor_specific_flags(merged_flags);
      return;
    }

  if (((new_flags & (elfcpp::EF_MIPS_PIC | elfcpp::EF_MIPS_CPIC)) != 0)
      != ((old_flags & (elfcpp::EF_MIPS_PIC | elfcpp::EF_MIPS_CPIC)) != 0))
    gold_warning(_("%s: linking abicalls files with non-abicalls files"),
                 name.c_str());

  if (new_flags & (elfcpp::EF_MIPS_PIC | elfcpp::EF_MIPS_CPIC))
    merged_flags |= elfcpp::EF_MIPS_CPIC;
  if (!(new_flags & elfcpp::EF_MIPS_PIC))
    merged_flags &= ~elfcpp::EF_MIPS_PIC;

  new_flags &= ~(elfcpp::EF_MIPS_PIC | elfcpp::EF_MIPS_CPIC);
  old_flags &= ~(elfcpp::EF_MIPS_PIC | elfcpp::EF_MIPS_CPIC);

  // Compare the ISAs.
  if (mips_32bit_flags(old_flags) != mips_32bit_flags(new_flags))
    gold_error(_("%s: linking 32-bit code with 64-bit code"), name.c_str());
  else if (!this->mips_mach_extends(this->elf_mips_mach(in_flags), this->mach_))
    {
      // Output ISA isn't the same as, or an extension of, input ISA.
      if (this->mips_mach_extends(this->mach_, this->elf_mips_mach(in_flags)))
        {
          // Copy the architecture info from input object to output.  Also copy
          // the 32-bit flag (if set) so that we continue to recognise
          // output as a 32-bit binary.
          this->mach_ = this->elf_mips_mach(in_flags);
          merged_flags &= ~(elfcpp::EF_MIPS_ARCH | elfcpp::EF_MIPS_MACH);
          merged_flags |= (new_flags & (elfcpp::EF_MIPS_ARCH
                           | elfcpp::EF_MIPS_MACH | elfcpp::EF_MIPS_32BITMODE));

          // Update the ABI flags isa_level, isa_rev, isa_ext fields.
          this->update_abiflags_isa(name, merged_flags, this->abiflags_);

          // Copy across the ABI flags if output doesn't use them
          // and if that was what caused us to treat input object as 32-bit.
          if ((old_flags & elfcpp::EF_MIPS_ABI) == 0
              && this->mips_32bit_flags(new_flags)
              && !this->mips_32bit_flags(new_flags & ~elfcpp::EF_MIPS_ABI))
            merged_flags |= new_flags & elfcpp::EF_MIPS_ABI;
        }
      else
        // The ISAs aren't compatible.
        gold_error(_("%s: linking %s module with previous %s modules"),
                   name.c_str(), this->elf_mips_mach_name(in_flags),
                   this->elf_mips_mach_name(merged_flags));
    }

  new_flags &= (~(elfcpp::EF_MIPS_ARCH | elfcpp::EF_MIPS_MACH
                | elfcpp::EF_MIPS_32BITMODE));
  old_flags &= (~(elfcpp::EF_MIPS_ARCH | elfcpp::EF_MIPS_MACH
                | elfcpp::EF_MIPS_32BITMODE));

  // Compare ABIs.
  if ((new_flags & elfcpp::EF_MIPS_ABI) != (old_flags & elfcpp::EF_MIPS_ABI))
    {
      // Only error if both are set (to different values).
      if ((new_flags & elfcpp::EF_MIPS_ABI)
           && (old_flags & elfcpp::EF_MIPS_ABI))
        gold_error(_("%s: ABI mismatch: linking %s module with "
                     "previous %s modules"), name.c_str(),
                   this->elf_mips_abi_name(in_flags),
                   this->elf_mips_abi_name(merged_flags));

      new_flags &= ~elfcpp::EF_MIPS_ABI;
      old_flags &= ~elfcpp::EF_MIPS_ABI;
    }

  // Compare ASEs.  Forbid linking MIPS16 and microMIPS ASE modules together
  // and allow arbitrary mixing of the remaining ASEs (retain the union).
  if ((new_flags & elfcpp::EF_MIPS_ARCH_ASE)
      != (old_flags & elfcpp::EF_MIPS_ARCH_ASE))
    {
      int old_micro = old_flags & elfcpp::EF_MIPS_ARCH_ASE_MICROMIPS;
      int new_micro = new_flags & elfcpp::EF_MIPS_ARCH_ASE_MICROMIPS;
      int old_m16 = old_flags & elfcpp::EF_MIPS_ARCH_ASE_M16;
      int new_m16 = new_flags & elfcpp::EF_MIPS_ARCH_ASE_M16;
      int micro_mis = old_m16 && new_micro;
      int m16_mis = old_micro && new_m16;

      if (m16_mis || micro_mis)
        gold_error(_("%s: ASE mismatch: linking %s module with "
                     "previous %s modules"), name.c_str(),
                   m16_mis ? "MIPS16" : "microMIPS",
                   m16_mis ? "microMIPS" : "MIPS16");

      merged_flags |= new_flags & elfcpp::EF_MIPS_ARCH_ASE;

      new_flags &= ~ elfcpp::EF_MIPS_ARCH_ASE;
      old_flags &= ~ elfcpp::EF_MIPS_ARCH_ASE;
    }

  // Compare NaN encodings.
  if ((new_flags & elfcpp::EF_MIPS_NAN2008) != (old_flags & elfcpp::EF_MIPS_NAN2008))
    {
      gold_error(_("%s: linking %s module with previous %s modules"),
                 name.c_str(),
                 (new_flags & elfcpp::EF_MIPS_NAN2008
                  ? "-mnan=2008" : "-mnan=legacy"),
                 (old_flags & elfcpp::EF_MIPS_NAN2008
                  ? "-mnan=2008" : "-mnan=legacy"));

      new_flags &= ~elfcpp::EF_MIPS_NAN2008;
      old_flags &= ~elfcpp::EF_MIPS_NAN2008;
    }

  // Compare FP64 state.
  if ((new_flags & elfcpp::EF_MIPS_FP64) != (old_flags & elfcpp::EF_MIPS_FP64))
    {
      gold_error(_("%s: linking %s module with previous %s modules"),
                 name.c_str(),
                 (new_flags & elfcpp::EF_MIPS_FP64
                  ? "-mfp64" : "-mfp32"),
                 (old_flags & elfcpp::EF_MIPS_FP64
                  ? "-mfp64" : "-mfp32"));

      new_flags &= ~elfcpp::EF_MIPS_FP64;
      old_flags &= ~elfcpp::EF_MIPS_FP64;
    }

  // Warn about any other mismatches.
  if (new_flags != old_flags)
    gold_error(_("%s: uses different e_flags (0x%x) fields than previous "
                 "modules (0x%x)"), name.c_str(), new_flags, old_flags);

  this->set_processor_specific_flags(merged_flags);
}

// Adjust ELF file header.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::do_adjust_elf_header(
    unsigned char* view,
    int len)
{
  gold_assert(len == elfcpp::Elf_sizes<size>::ehdr_size);

  elfcpp::Ehdr<size, big_endian> ehdr(view);
  unsigned char e_ident[elfcpp::EI_NIDENT];
  elfcpp::Elf_Word flags = this->processor_specific_flags();
  memcpy(e_ident, ehdr.get_e_ident(), elfcpp::EI_NIDENT);

  unsigned char ei_abiversion = 0;
  elfcpp::Elf_Half type = ehdr.get_e_type();
  if (type == elfcpp::ET_EXEC
      && parameters->options().copyreloc()
      && (flags & (elfcpp::EF_MIPS_PIC | elfcpp::EF_MIPS_CPIC))
          == elfcpp::EF_MIPS_CPIC)
    ei_abiversion = 1;

  if (this->abiflags_ != NULL
      && (this->abiflags_->fp_abi == elfcpp::Val_GNU_MIPS_ABI_FP_64
          || this->abiflags_->fp_abi == elfcpp::Val_GNU_MIPS_ABI_FP_64A))
    ei_abiversion = 3;

  e_ident[elfcpp::EI_ABIVERSION] = ei_abiversion;
  elfcpp::Ehdr_write<size, big_endian> oehdr(view);
  oehdr.put_e_ident(e_ident);

  if (this->entry_symbol_is_compressed_)
    oehdr.put_e_entry(ehdr.get_e_entry() + 1);
}

// do_make_elf_object to override the same function in the base class.
// We need to use a target-specific sub-class of
// Sized_relobj_file<size, big_endian> to store Mips specific information.
// Hence we need to have our own ELF object creation.

template<int size, bool big_endian>
Object*
Target_mips<size, big_endian>::do_make_elf_object(
    const std::string& name,
    Input_file* input_file,
    off_t offset, const elfcpp::Ehdr<size, big_endian>& ehdr)
{
  int et = ehdr.get_e_type();
  // ET_EXEC files are valid input for --just-symbols/-R,
  // and we treat them as relocatable objects.
  if (et == elfcpp::ET_REL
      || (et == elfcpp::ET_EXEC && input_file->just_symbols()))
    {
      Mips_relobj<size, big_endian>* obj =
        new Mips_relobj<size, big_endian>(name, input_file, offset, ehdr);
      obj->setup();
      return obj;
    }
  else if (et == elfcpp::ET_DYN)
    {
      // TODO(sasa): Should we create Mips_dynobj?
      return Target::do_make_elf_object(name, input_file, offset, ehdr);
    }
  else
    {
      gold_error(_("%s: unsupported ELF file type %d"),
                 name.c_str(), et);
      return NULL;
    }
}

// Finalize the sections.

template <int size, bool big_endian>
void
Target_mips<size, big_endian>::do_finalize_sections(Layout* layout,
                                        const Input_objects* input_objects,
                                        Symbol_table* symtab)
{
  const bool relocatable = parameters->options().relocatable();

  // Add +1 to MIPS16 and microMIPS init_ and _fini symbols so that DT_INIT and
  // DT_FINI have correct values.
  Mips_symbol<size>* init = static_cast<Mips_symbol<size>*>(
      symtab->lookup(parameters->options().init()));
  if (init != NULL && (init->is_mips16() || init->is_micromips()))
    init->set_value(init->value() | 1);
  Mips_symbol<size>* fini = static_cast<Mips_symbol<size>*>(
      symtab->lookup(parameters->options().fini()));
  if (fini != NULL && (fini->is_mips16() || fini->is_micromips()))
    fini->set_value(fini->value() | 1);

  // Check whether the entry symbol is mips16 or micromips.  This is needed to
  // adjust entry address in ELF header.
  Mips_symbol<size>* entry =
    static_cast<Mips_symbol<size>*>(symtab->lookup(this->entry_symbol_name()));
  this->entry_symbol_is_compressed_ = (entry != NULL && (entry->is_mips16()
                                       || entry->is_micromips()));

  if (!parameters->doing_static_link()
      && (strcmp(parameters->options().hash_style(), "gnu") == 0
          || strcmp(parameters->options().hash_style(), "both") == 0))
    {
      // .gnu.hash and the MIPS ABI require .dynsym to be sorted in different
      // ways.  .gnu.hash needs symbols to be grouped by hash code whereas the
      // MIPS ABI requires a mapping between the GOT and the symbol table.
      gold_error(".gnu.hash is incompatible with the MIPS ABI");
    }

  // Check whether the final section that was scanned has HI16 or GOT16
  // relocations without the corresponding LO16 part.
  if (this->got16_addends_.size() > 0)
      gold_error("Can't find matching LO16 reloc");

  Valtype gprmask = 0;
  Valtype cprmask1 = 0;
  Valtype cprmask2 = 0;
  Valtype cprmask3 = 0;
  Valtype cprmask4 = 0;
  bool has_reginfo_section = false;

  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      Mips_relobj<size, big_endian>* relobj =
        Mips_relobj<size, big_endian>::as_mips_relobj(*p);

      // Check for any mips16 stub sections that we can discard.
      if (!relocatable)
        relobj->discard_mips16_stub_sections(symtab);

      if (!relobj->merge_processor_specific_data())
        continue;

      // Merge .reginfo contents of input objects.
      if (relobj->has_reginfo_section())
        {
          has_reginfo_section = true;
          gprmask |= relobj->gprmask();
          cprmask1 |= relobj->cprmask1();
          cprmask2 |= relobj->cprmask2();
          cprmask3 |= relobj->cprmask3();
          cprmask4 |= relobj->cprmask4();
        }

      // Merge processor specific flags.
      Mips_abiflags<big_endian> in_abiflags;

      this->create_abiflags(relobj, &in_abiflags);
      this->merge_obj_e_flags(relobj->name(),
                              relobj->processor_specific_flags());
      this->merge_obj_abiflags(relobj->name(), &in_abiflags);
      this->merge_obj_attributes(relobj->name(),
                                 relobj->attributes_section_data());
    }

  // Create a .gnu.attributes section if we have merged any attributes
  // from inputs.
  if (this->attributes_section_data_ != NULL)
    {
      Output_attributes_section_data* attributes_section =
        new Output_attributes_section_data(*this->attributes_section_data_);
      layout->add_output_section_data(".gnu.attributes",
                                      elfcpp::SHT_GNU_ATTRIBUTES, 0,
                                      attributes_section, ORDER_INVALID, false);
    }

  // Create .MIPS.abiflags output section if there is an input section.
  if (this->has_abiflags_section_)
    {
      Mips_output_section_abiflags<size, big_endian>* abiflags_section =
        new Mips_output_section_abiflags<size, big_endian>(*this->abiflags_);

      Output_section* os =
        layout->add_output_section_data(".MIPS.abiflags",
                                        elfcpp::SHT_MIPS_ABIFLAGS,
                                        elfcpp::SHF_ALLOC,
                                        abiflags_section, ORDER_INVALID, false);

      if (!relocatable && os != NULL)
        {
          Output_segment* abiflags_segment =
            layout->make_output_segment(elfcpp::PT_MIPS_ABIFLAGS, elfcpp::PF_R);
          abiflags_segment->add_output_section_to_nonload(os, elfcpp::PF_R);
        }
    }

  if (has_reginfo_section && !parameters->options().gc_sections())
    {
      // Create .reginfo output section.
      Mips_output_section_reginfo<size, big_endian>* reginfo_section =
        new Mips_output_section_reginfo<size, big_endian>(this, gprmask,
                                                          cprmask1, cprmask2,
                                                          cprmask3, cprmask4);

      Output_section* os =
        layout->add_output_section_data(".reginfo", elfcpp::SHT_MIPS_REGINFO,
                                        elfcpp::SHF_ALLOC, reginfo_section,
                                        ORDER_INVALID, false);

      if (!relocatable && os != NULL)
        {
          Output_segment* reginfo_segment =
            layout->make_output_segment(elfcpp::PT_MIPS_REGINFO,
                                        elfcpp::PF_R);
          reginfo_segment->add_output_section_to_nonload(os, elfcpp::PF_R);
        }
    }

  if (this->plt_ != NULL)
    {
      // Set final PLT offsets for symbols.
      this->plt_section()->set_plt_offsets();

      // Define _PROCEDURE_LINKAGE_TABLE_ at the start of the .plt section.
      // Set STO_MICROMIPS flag if the output has microMIPS code, but only if
      // there are no standard PLT entries present.
      unsigned char nonvis = 0;
      if (this->is_output_micromips()
          && !this->plt_section()->has_standard_entries())
        nonvis = elfcpp::STO_MICROMIPS >> 2;
      symtab->define_in_output_data("_PROCEDURE_LINKAGE_TABLE_", NULL,
                                    Symbol_table::PREDEFINED,
                                    this->plt_,
                                    0, 0, elfcpp::STT_FUNC,
                                    elfcpp::STB_LOCAL,
                                    elfcpp::STV_DEFAULT, nonvis,
                                    false, false);
    }

  if (this->mips_stubs_ != NULL)
    {
      // Define _MIPS_STUBS_ at the start of the .MIPS.stubs section.
      unsigned char nonvis = 0;
      if (this->is_output_micromips())
        nonvis = elfcpp::STO_MICROMIPS >> 2;
      symtab->define_in_output_data("_MIPS_STUBS_", NULL,
                                    Symbol_table::PREDEFINED,
                                    this->mips_stubs_,
                                    0, 0, elfcpp::STT_FUNC,
                                    elfcpp::STB_LOCAL,
                                    elfcpp::STV_DEFAULT, nonvis,
                                    false, false);
    }

  if (!relocatable && !parameters->doing_static_link())
    // In case there is no .got section, create one.
    this->got_section(symtab, layout);

  // Emit any relocs we saved in an attempt to avoid generating COPY
  // relocs.
  if (this->copy_relocs_.any_saved_relocs())
    this->copy_relocs_.emit_mips(this->rel_dyn_section(layout), symtab, layout,
                                 this);

  // Set _gp value.
  this->set_gp(layout, symtab);

  // Emit dynamic relocs.
  for (typename std::vector<Dyn_reloc>::iterator p = this->dyn_relocs_.begin();
       p != this->dyn_relocs_.end();
       ++p)
    p->emit(this->rel_dyn_section(layout), this->got_section(), symtab);

  if (this->has_got_section())
    this->got_section()->lay_out_got(layout, symtab, input_objects);

  if (this->mips_stubs_ != NULL)
    this->mips_stubs_->set_needs_dynsym_value();

  // Check for functions that might need $25 to be valid on entry.
  // TODO(sasa): Can we do this without iterating over all symbols?
  typedef Symbol_visitor_check_symbols<size, big_endian> Symbol_visitor;
  symtab->for_all_symbols<size, Symbol_visitor>(Symbol_visitor(this, layout,
                                                               symtab));

  // Add NULL segment.
  if (!relocatable)
    layout->make_output_segment(elfcpp::PT_NULL, 0);

  // Fill in some more dynamic tags.
  // TODO(sasa): Add more dynamic tags.
  const Reloc_section* rel_plt = (this->plt_ == NULL
                                  ? NULL : this->plt_->rel_plt());
  layout->add_target_dynamic_tags(true, this->got_, rel_plt,
				  this->rel_dyn_, true, false, false);

  Output_data_dynamic* const odyn = layout->dynamic_data();
  if (odyn != NULL
      && !relocatable
      && !parameters->doing_static_link())
  {
    unsigned int d_val;
    // This element holds a 32-bit version id for the Runtime
    // Linker Interface.  This will start at integer value 1.
    d_val = 0x01;
    odyn->add_constant(elfcpp::DT_MIPS_RLD_VERSION, d_val);

    // Dynamic flags
    d_val = elfcpp::RHF_NOTPOT;
    odyn->add_constant(elfcpp::DT_MIPS_FLAGS, d_val);

    // Save layout for using when emitting custom dynamic tags.
    this->layout_ = layout;

    // This member holds the base address of the segment.
    odyn->add_custom(elfcpp::DT_MIPS_BASE_ADDRESS);

    // This member holds the number of entries in the .dynsym section.
    odyn->add_custom(elfcpp::DT_MIPS_SYMTABNO);

    // This member holds the index of the first dynamic symbol
    // table entry that corresponds to an entry in the global offset table.
    odyn->add_custom(elfcpp::DT_MIPS_GOTSYM);

    // This member holds the number of local GOT entries.
    odyn->add_constant(elfcpp::DT_MIPS_LOCAL_GOTNO,
                       this->got_->get_local_gotno());

    if (this->plt_ != NULL)
      // DT_MIPS_PLTGOT dynamic tag
      odyn->add_section_address(elfcpp::DT_MIPS_PLTGOT, this->got_plt_);

    if (!parameters->options().shared())
      {
        this->rld_map_ = new Output_data_zero_fill(size / 8, size / 8);

        layout->add_output_section_data(".rld_map", elfcpp::SHT_PROGBITS,
                                        (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE),
                                        this->rld_map_, ORDER_INVALID, false);

        // __RLD_MAP will be filled in by the runtime loader to contain
        // a pointer to the _r_debug structure.
        Symbol* rld_map = symtab->define_in_output_data("__RLD_MAP", NULL,
                                            Symbol_table::PREDEFINED,
                                            this->rld_map_,
                                            0, 0, elfcpp::STT_OBJECT,
                                            elfcpp::STB_GLOBAL,
                                            elfcpp::STV_DEFAULT, 0,
                                            false, false);

        if (!rld_map->is_forced_local())
          rld_map->set_needs_dynsym_entry();

        if (!parameters->options().pie())
          // This member holds the absolute address of the debug pointer.
          odyn->add_section_address(elfcpp::DT_MIPS_RLD_MAP, this->rld_map_);
        else
          // This member holds the offset to the debug pointer,
          // relative to the address of the tag.
          odyn->add_custom(elfcpp::DT_MIPS_RLD_MAP_REL);
      }
  }
}

// Get the custom dynamic tag value.
template<int size, bool big_endian>
unsigned int
Target_mips<size, big_endian>::do_dynamic_tag_custom_value(elfcpp::DT tag) const
{
  switch (tag)
    {
    case elfcpp::DT_MIPS_BASE_ADDRESS:
      {
        // The base address of the segment.
        // At this point, the segment list has been sorted into final order,
        // so just return vaddr of the first readable PT_LOAD segment.
        Output_segment* seg =
          this->layout_->find_output_segment(elfcpp::PT_LOAD, elfcpp::PF_R, 0);
        gold_assert(seg != NULL);
        return seg->vaddr();
      }

    case elfcpp::DT_MIPS_SYMTABNO:
      // The number of entries in the .dynsym section.
      return this->get_dt_mips_symtabno();

    case elfcpp::DT_MIPS_GOTSYM:
      {
        // The index of the first dynamic symbol table entry that corresponds
        // to an entry in the GOT.
        if (this->got_->first_global_got_dynsym_index() != -1U)
          return this->got_->first_global_got_dynsym_index();
        else
          // In case if we don't have global GOT symbols we default to setting
          // DT_MIPS_GOTSYM to the same value as DT_MIPS_SYMTABNO.
          return this->get_dt_mips_symtabno();
      }

    case elfcpp::DT_MIPS_RLD_MAP_REL:
      {
        // The MIPS_RLD_MAP_REL tag stores the offset to the debug pointer,
        // relative to the address of the tag.
        Output_data_dynamic* const odyn = this->layout_->dynamic_data();
        unsigned int entry_offset =
          odyn->get_entry_offset(elfcpp::DT_MIPS_RLD_MAP_REL);
        gold_assert(entry_offset != -1U);
        return this->rld_map_->address() - (odyn->address() + entry_offset);
      }
    default:
      gold_error(_("Unknown dynamic tag 0x%x"), (unsigned int)tag);
    }

  return (unsigned int)-1;
}

// Relocate section data.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::relocate_section(
                        const Relocate_info<size, big_endian>* relinfo,
                        unsigned int sh_type,
                        const unsigned char* prelocs,
                        size_t reloc_count,
                        Output_section* output_section,
                        bool needs_special_offset_handling,
                        unsigned char* view,
                        Mips_address address,
                        section_size_type view_size,
                        const Reloc_symbol_changes* reloc_symbol_changes)
{
  typedef Target_mips<size, big_endian> Mips;
  typedef typename Target_mips<size, big_endian>::Relocate Mips_relocate;

  if (sh_type == elfcpp::SHT_REL)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>
	  Classify_reloc;

      gold::relocate_section<size, big_endian, Mips, Mips_relocate,
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
  else if (sh_type == elfcpp::SHT_RELA)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
	  Classify_reloc;

      gold::relocate_section<size, big_endian, Mips, Mips_relocate,
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
}

// Return the size of a relocation while scanning during a relocatable
// link.

unsigned int
mips_get_size_for_reloc(unsigned int r_type, Relobj* object)
{
  switch (r_type)
    {
    case elfcpp::R_MIPS_NONE:
    case elfcpp::R_MIPS_TLS_DTPMOD64:
    case elfcpp::R_MIPS_TLS_DTPREL64:
    case elfcpp::R_MIPS_TLS_TPREL64:
      return 0;

    case elfcpp::R_MIPS_32:
    case elfcpp::R_MIPS_TLS_DTPMOD32:
    case elfcpp::R_MIPS_TLS_DTPREL32:
    case elfcpp::R_MIPS_TLS_TPREL32:
    case elfcpp::R_MIPS_REL32:
    case elfcpp::R_MIPS_PC32:
    case elfcpp::R_MIPS_GPREL32:
    case elfcpp::R_MIPS_JALR:
    case elfcpp::R_MIPS_EH:
      return 4;

    case elfcpp::R_MIPS_16:
    case elfcpp::R_MIPS_HI16:
    case elfcpp::R_MIPS_LO16:
    case elfcpp::R_MIPS_HIGHER:
    case elfcpp::R_MIPS_HIGHEST:
    case elfcpp::R_MIPS_GPREL16:
    case elfcpp::R_MIPS16_HI16:
    case elfcpp::R_MIPS16_LO16:
    case elfcpp::R_MIPS_PC16:
    case elfcpp::R_MIPS_PCHI16:
    case elfcpp::R_MIPS_PCLO16:
    case elfcpp::R_MIPS_GOT16:
    case elfcpp::R_MIPS16_GOT16:
    case elfcpp::R_MIPS_CALL16:
    case elfcpp::R_MIPS16_CALL16:
    case elfcpp::R_MIPS_GOT_HI16:
    case elfcpp::R_MIPS_CALL_HI16:
    case elfcpp::R_MIPS_GOT_LO16:
    case elfcpp::R_MIPS_CALL_LO16:
    case elfcpp::R_MIPS_TLS_DTPREL_HI16:
    case elfcpp::R_MIPS_TLS_DTPREL_LO16:
    case elfcpp::R_MIPS_TLS_TPREL_HI16:
    case elfcpp::R_MIPS_TLS_TPREL_LO16:
    case elfcpp::R_MIPS16_GPREL:
    case elfcpp::R_MIPS_GOT_DISP:
    case elfcpp::R_MIPS_LITERAL:
    case elfcpp::R_MIPS_GOT_PAGE:
    case elfcpp::R_MIPS_GOT_OFST:
    case elfcpp::R_MIPS_TLS_GD:
    case elfcpp::R_MIPS_TLS_LDM:
    case elfcpp::R_MIPS_TLS_GOTTPREL:
      return 2;

    // These relocations are not byte sized
    case elfcpp::R_MIPS_26:
    case elfcpp::R_MIPS16_26:
    case elfcpp::R_MIPS_PC21_S2:
    case elfcpp::R_MIPS_PC26_S2:
    case elfcpp::R_MIPS_PC18_S3:
    case elfcpp::R_MIPS_PC19_S2:
      return 4;

    case elfcpp::R_MIPS_COPY:
    case elfcpp::R_MIPS_JUMP_SLOT:
      object->error(_("unexpected reloc %u in object file"), r_type);
      return 0;

    default:
      object->error(_("unsupported reloc %u in object file"), r_type);
      return 0;
  }
}

// Scan the relocs during a relocatable link.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::scan_relocatable_relocs(
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
  if (sh_type == elfcpp::SHT_REL)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>
          Classify_reloc;
      typedef Mips_scan_relocatable_relocs<big_endian, Classify_reloc>
          Scan_relocatable_relocs;

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
  else if (sh_type == elfcpp::SHT_RELA)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
          Classify_reloc;
      typedef Mips_scan_relocatable_relocs<big_endian, Classify_reloc>
          Scan_relocatable_relocs;

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
  else
    gold_unreachable();
}

// Scan the relocs for --emit-relocs.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::emit_relocs_scan(
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
  if (sh_type == elfcpp::SHT_REL)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>
          Classify_reloc;
      typedef gold::Default_emit_relocs_strategy<Classify_reloc>
          Emit_relocs_strategy;

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
  else if (sh_type == elfcpp::SHT_RELA)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
          Classify_reloc;
      typedef gold::Default_emit_relocs_strategy<Classify_reloc>
          Emit_relocs_strategy;

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
  else
    gold_unreachable();
}

// Emit relocations for a section.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::relocate_relocs(
                        const Relocate_info<size, big_endian>* relinfo,
                        unsigned int sh_type,
                        const unsigned char* prelocs,
                        size_t reloc_count,
                        Output_section* output_section,
                        typename elfcpp::Elf_types<size>::Elf_Off
                          offset_in_output_section,
                        unsigned char* view,
                        Mips_address view_address,
                        section_size_type view_size,
                        unsigned char* reloc_view,
                        section_size_type reloc_view_size)
{
  if (sh_type == elfcpp::SHT_REL)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>
          Classify_reloc;

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
  else if (sh_type == elfcpp::SHT_RELA)
    {
      typedef Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>
          Classify_reloc;

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
  else
    gold_unreachable();
}

// Perform target-specific processing in a relocatable link.  This is
// only used if we use the relocation strategy RELOC_SPECIAL.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::relocate_special_relocatable(
    const Relocate_info<size, big_endian>* relinfo,
    unsigned int sh_type,
    const unsigned char* preloc_in,
    size_t relnum,
    Output_section* output_section,
    typename elfcpp::Elf_types<size>::Elf_Off offset_in_output_section,
    unsigned char* view,
    Mips_address view_address,
    section_size_type,
    unsigned char* preloc_out)
{
  // We can only handle REL type relocation sections.
  gold_assert(sh_type == elfcpp::SHT_REL);

  typedef typename Reloc_types<elfcpp::SHT_REL, size, big_endian>::Reloc
    Reltype;
  typedef typename Reloc_types<elfcpp::SHT_REL, size, big_endian>::Reloc_write
    Reltype_write;

  typedef Mips_relocate_functions<size, big_endian> Reloc_funcs;

  const Mips_address invalid_address = static_cast<Mips_address>(0) - 1;

  Mips_relobj<size, big_endian>* object =
    Mips_relobj<size, big_endian>::as_mips_relobj(relinfo->object);
  const unsigned int local_count = object->local_symbol_count();

  Reltype reloc(preloc_in);
  Reltype_write reloc_write(preloc_out);

  elfcpp::Elf_types<32>::Elf_WXword r_info = reloc.get_r_info();
  const unsigned int r_sym = elfcpp::elf_r_sym<size>(r_info);
  const unsigned int r_type = elfcpp::elf_r_type<size>(r_info);

  // Get the new symbol index.
  // We only use RELOC_SPECIAL strategy in local relocations.
  gold_assert(r_sym < local_count);

  // We are adjusting a section symbol.  We need to find
  // the symbol table index of the section symbol for
  // the output section corresponding to input section
  // in which this symbol is defined.
  bool is_ordinary;
  unsigned int shndx = object->local_symbol_input_shndx(r_sym, &is_ordinary);
  gold_assert(is_ordinary);
  Output_section* os = object->output_section(shndx);
  gold_assert(os != NULL);
  gold_assert(os->needs_symtab_index());
  unsigned int new_symndx = os->symtab_index();

  // Get the new offset--the location in the output section where
  // this relocation should be applied.

  Mips_address offset = reloc.get_r_offset();
  Mips_address new_offset;
  if (offset_in_output_section != invalid_address)
    new_offset = offset + offset_in_output_section;
  else
    {
      section_offset_type sot_offset =
        convert_types<section_offset_type, Mips_address>(offset);
      section_offset_type new_sot_offset =
        output_section->output_offset(object, relinfo->data_shndx,
                                      sot_offset);
      gold_assert(new_sot_offset != -1);
      new_offset = new_sot_offset;
    }

  // In an object file, r_offset is an offset within the section.
  // In an executable or dynamic object, generated by
  // --emit-relocs, r_offset is an absolute address.
  if (!parameters->options().relocatable())
    {
      new_offset += view_address;
      if (offset_in_output_section != invalid_address)
        new_offset -= offset_in_output_section;
    }

  reloc_write.put_r_offset(new_offset);
  reloc_write.put_r_info(elfcpp::elf_r_info<32>(new_symndx, r_type));

  // Handle the reloc addend.
  // The relocation uses a section symbol in the input file.
  // We are adjusting it to use a section symbol in the output
  // file.  The input section symbol refers to some address in
  // the input section.  We need the relocation in the output
  // file to refer to that same address.  This adjustment to
  // the addend is the same calculation we use for a simple
  // absolute relocation for the input section symbol.
  Valtype calculated_value = 0;
  const Symbol_value<size>* psymval = object->local_symbol(r_sym);

  unsigned char* paddend = view + offset;
  typename Reloc_funcs::Status reloc_status = Reloc_funcs::STATUS_OKAY;
  switch (r_type)
    {
    case elfcpp::R_MIPS_26:
      reloc_status = Reloc_funcs::rel26(paddend, object, psymval,
          offset_in_output_section, true, 0, sh_type == elfcpp::SHT_REL, NULL,
          false /*TODO(sasa): cross mode jump*/, r_type, this->jal_to_bal(),
          false, &calculated_value);
      break;

    default:
      gold_unreachable();
    }

  // Report any errors.
  switch (reloc_status)
    {
    case Reloc_funcs::STATUS_OKAY:
      break;
    case Reloc_funcs::STATUS_OVERFLOW:
      gold_error_at_location(relinfo, relnum, reloc.get_r_offset(),
			     _("relocation overflow: "
			       "%u against local symbol %u in %s"),
			     r_type, r_sym, object->name().c_str());
      break;
    case Reloc_funcs::STATUS_BAD_RELOC:
      gold_error_at_location(relinfo, relnum, reloc.get_r_offset(),
        _("unexpected opcode while processing relocation"));
      break;
    default:
      gold_unreachable();
    }
}

// Optimize the TLS relocation type based on what we know about the
// symbol.  IS_FINAL is true if the final address of this symbol is
// known at link time.

template<int size, bool big_endian>
tls::Tls_optimization
Target_mips<size, big_endian>::optimize_tls_reloc(bool, int)
{
  // FIXME: Currently we do not do any TLS optimization.
  return tls::TLSOPT_NONE;
}

// Scan a relocation for a local symbol.

template<int size, bool big_endian>
inline void
Target_mips<size, big_endian>::Scan::local(
                        Symbol_table* symtab,
                        Layout* layout,
                        Target_mips<size, big_endian>* target,
                        Sized_relobj_file<size, big_endian>* object,
                        unsigned int data_shndx,
                        Output_section* output_section,
                        const Relatype* rela,
                        const Reltype* rel,
                        unsigned int rel_type,
                        unsigned int r_type,
                        const elfcpp::Sym<size, big_endian>& lsym,
                        bool is_discarded)
{
  if (is_discarded)
    return;

  Mips_address r_offset;
  unsigned int r_sym;
  typename elfcpp::Elf_types<size>::Elf_Swxword r_addend;

  if (rel_type == elfcpp::SHT_RELA)
    {
      r_offset = rela->get_r_offset();
      r_sym = Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>::
	  get_r_sym(rela);
      r_addend = rela->get_r_addend();
    }
  else
    {
      r_offset = rel->get_r_offset();
      r_sym = Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>::
	  get_r_sym(rel);
      r_addend = 0;
    }

  Mips_relobj<size, big_endian>* mips_obj =
    Mips_relobj<size, big_endian>::as_mips_relobj(object);

  if (mips_obj->is_mips16_stub_section(data_shndx))
    {
      mips_obj->get_mips16_stub_section(data_shndx)
              ->new_local_reloc_found(r_type, r_sym);
    }

  if (r_type == elfcpp::R_MIPS_NONE)
    // R_MIPS_NONE is used in mips16 stub sections, to define the target of the
    // mips16 stub.
    return;

  if (!mips16_call_reloc(r_type)
      && !mips_obj->section_allows_mips16_refs(data_shndx))
    // This reloc would need to refer to a MIPS16 hard-float stub, if
    // there is one.  We ignore MIPS16 stub sections and .pdr section when
    // looking for relocs that would need to refer to MIPS16 stubs.
    mips_obj->add_local_non_16bit_call(r_sym);

  if (r_type == elfcpp::R_MIPS16_26
      && !mips_obj->section_allows_mips16_refs(data_shndx))
    mips_obj->add_local_16bit_call(r_sym);

  switch (r_type)
    {
    case elfcpp::R_MIPS_GOT16:
    case elfcpp::R_MIPS_CALL16:
    case elfcpp::R_MIPS_CALL_HI16:
    case elfcpp::R_MIPS_CALL_LO16:
    case elfcpp::R_MIPS_GOT_HI16:
    case elfcpp::R_MIPS_GOT_LO16:
    case elfcpp::R_MIPS_GOT_PAGE:
    case elfcpp::R_MIPS_GOT_OFST:
    case elfcpp::R_MIPS_GOT_DISP:
    case elfcpp::R_MIPS_TLS_GOTTPREL:
    case elfcpp::R_MIPS_TLS_GD:
    case elfcpp::R_MIPS_TLS_LDM:
    case elfcpp::R_MIPS16_GOT16:
    case elfcpp::R_MIPS16_CALL16:
    case elfcpp::R_MIPS16_TLS_GOTTPREL:
    case elfcpp::R_MIPS16_TLS_GD:
    case elfcpp::R_MIPS16_TLS_LDM:
    case elfcpp::R_MICROMIPS_GOT16:
    case elfcpp::R_MICROMIPS_CALL16:
    case elfcpp::R_MICROMIPS_CALL_HI16:
    case elfcpp::R_MICROMIPS_CALL_LO16:
    case elfcpp::R_MICROMIPS_GOT_HI16:
    case elfcpp::R_MICROMIPS_GOT_LO16:
    case elfcpp::R_MICROMIPS_GOT_PAGE:
    case elfcpp::R_MICROMIPS_GOT_OFST:
    case elfcpp::R_MICROMIPS_GOT_DISP:
    case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
    case elfcpp::R_MICROMIPS_TLS_GD:
    case elfcpp::R_MICROMIPS_TLS_LDM:
    case elfcpp::R_MIPS_EH:
      // We need a GOT section.
      target->got_section(symtab, layout);
      break;

    default:
      break;
    }

  if (call_lo16_reloc(r_type)
      || got_lo16_reloc(r_type)
      || got_disp_reloc(r_type)
      || eh_reloc(r_type))
    {
      // We may need a local GOT entry for this relocation.  We
      // don't count R_MIPS_GOT_PAGE because we can estimate the
      // maximum number of pages needed by looking at the size of
      // the segment.  Similar comments apply to R_MIPS*_GOT16 and
      // R_MIPS*_CALL16.  We don't count R_MIPS_GOT_HI16, or
      // R_MIPS_CALL_HI16 because these are always followed by an
      // R_MIPS_GOT_LO16 or R_MIPS_CALL_LO16.
      Mips_output_data_got<size, big_endian>* got =
        target->got_section(symtab, layout);
      bool is_section_symbol = lsym.get_st_type() == elfcpp::STT_SECTION;
      got->record_local_got_symbol(mips_obj, r_sym, r_addend, r_type, -1U,
                                   is_section_symbol);
    }

  switch (r_type)
    {
    case elfcpp::R_MIPS_CALL16:
    case elfcpp::R_MIPS16_CALL16:
    case elfcpp::R_MICROMIPS_CALL16:
      gold_error(_("CALL16 reloc at 0x%lx not against global symbol "),
                 (unsigned long)r_offset);
      return;

    case elfcpp::R_MIPS_GOT_PAGE:
    case elfcpp::R_MICROMIPS_GOT_PAGE:
    case elfcpp::R_MIPS16_GOT16:
    case elfcpp::R_MIPS_GOT16:
    case elfcpp::R_MIPS_GOT_HI16:
    case elfcpp::R_MIPS_GOT_LO16:
    case elfcpp::R_MICROMIPS_GOT16:
    case elfcpp::R_MICROMIPS_GOT_HI16:
    case elfcpp::R_MICROMIPS_GOT_LO16:
      {
        // This relocation needs a page entry in the GOT.
        // Get the section contents.
        section_size_type view_size = 0;
        const unsigned char* view = object->section_contents(data_shndx,
                                                             &view_size, false);
        view += r_offset;

        Valtype32 val = elfcpp::Swap<32, big_endian>::readval(view);
        Valtype32 addend = (rel_type == elfcpp::SHT_REL ? val & 0xffff
                                                        : r_addend);

        if (rel_type == elfcpp::SHT_REL && got16_reloc(r_type))
          target->got16_addends_.push_back(got16_addend<size, big_endian>(
              object, data_shndx, r_type, r_sym, addend));
        else
          target->got_section()->record_got_page_entry(mips_obj, r_sym, addend);
        break;
      }

    case elfcpp::R_MIPS_HI16:
    case elfcpp::R_MIPS_PCHI16:
    case elfcpp::R_MIPS16_HI16:
    case elfcpp::R_MICROMIPS_HI16:
      // Record the reloc so that we can check whether the corresponding LO16
      // part exists.
      if (rel_type == elfcpp::SHT_REL)
        target->got16_addends_.push_back(got16_addend<size, big_endian>(
            object, data_shndx, r_type, r_sym, 0));
      break;

    case elfcpp::R_MIPS_LO16:
    case elfcpp::R_MIPS_PCLO16:
    case elfcpp::R_MIPS16_LO16:
    case elfcpp::R_MICROMIPS_LO16:
      {
        if (rel_type != elfcpp::SHT_REL)
          break;

        // Find corresponding GOT16/HI16 relocation.

        // According to the MIPS ELF ABI, the R_MIPS_LO16 relocation must
        // be immediately following.  However, for the IRIX6 ABI, the next
        // relocation may be a composed relocation consisting of several
        // relocations for the same address.  In that case, the R_MIPS_LO16
        // relocation may occur as one of these.  We permit a similar
        // extension in general, as that is useful for GCC.

        // In some cases GCC dead code elimination removes the LO16 but
        // keeps the corresponding HI16.  This is strictly speaking a
        // violation of the ABI but not immediately harmful.

        typename std::list<got16_addend<size, big_endian> >::iterator it =
          target->got16_addends_.begin();
        while (it != target->got16_addends_.end())
          {
            got16_addend<size, big_endian> _got16_addend = *it;

            // TODO(sasa): Split got16_addends_ list into two lists - one for
            // GOT16 relocs and the other for HI16 relocs.

            // Report an error if we find HI16 or GOT16 reloc from the
            // previous section without the matching LO16 part.
            if (_got16_addend.object != object
                || _got16_addend.shndx != data_shndx)
              {
                gold_error("Can't find matching LO16 reloc");
                break;
              }

            if (_got16_addend.r_sym != r_sym
                || !is_matching_lo16_reloc(_got16_addend.r_type, r_type))
              {
                ++it;
                continue;
              }

            // We found a matching HI16 or GOT16 reloc for this LO16 reloc.
            // For GOT16, we need to calculate combined addend and record GOT page
            // entry.
            if (got16_reloc(_got16_addend.r_type))
              {

                section_size_type view_size = 0;
                const unsigned char* view = object->section_contents(data_shndx,
                                                                     &view_size,
                                                                     false);
                view += r_offset;

                Valtype32 val = elfcpp::Swap<32, big_endian>::readval(view);
                int32_t addend = Bits<16>::sign_extend32(val & 0xffff);

                addend = (_got16_addend.addend << 16) + addend;
                target->got_section()->record_got_page_entry(mips_obj, r_sym,
                                                             addend);
              }

            it = target->got16_addends_.erase(it);
          }
        break;
      }
    }

  switch (r_type)
    {
    case elfcpp::R_MIPS_32:
    case elfcpp::R_MIPS_REL32:
    case elfcpp::R_MIPS_64:
      {
        if (parameters->options().output_is_position_independent())
          {
            // If building a shared library (or a position-independent
            // executable), we need to create a dynamic relocation for
            // this location.
            if (is_readonly_section(output_section))
              break;
            Reloc_section* rel_dyn = target->rel_dyn_section(layout);
            rel_dyn->add_symbolless_local_addend(object, r_sym,
                                                 elfcpp::R_MIPS_REL32,
                                                 output_section, data_shndx,
                                                 r_offset);
          }
        break;
      }

    case elfcpp::R_MIPS_TLS_GOTTPREL:
    case elfcpp::R_MIPS16_TLS_GOTTPREL:
    case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
    case elfcpp::R_MIPS_TLS_LDM:
    case elfcpp::R_MIPS16_TLS_LDM:
    case elfcpp::R_MICROMIPS_TLS_LDM:
    case elfcpp::R_MIPS_TLS_GD:
    case elfcpp::R_MIPS16_TLS_GD:
    case elfcpp::R_MICROMIPS_TLS_GD:
      {
        bool output_is_shared = parameters->options().shared();
        const tls::Tls_optimization optimized_type
            = Target_mips<size, big_endian>::optimize_tls_reloc(
                                             !output_is_shared, r_type);
        switch (r_type)
          {
          case elfcpp::R_MIPS_TLS_GD:
          case elfcpp::R_MIPS16_TLS_GD:
          case elfcpp::R_MICROMIPS_TLS_GD:
            if (optimized_type == tls::TLSOPT_NONE)
              {
                // Create a pair of GOT entries for the module index and
                // dtv-relative offset.
                Mips_output_data_got<size, big_endian>* got =
                  target->got_section(symtab, layout);
                unsigned int shndx = lsym.get_st_shndx();
                bool is_ordinary;
                shndx = object->adjust_sym_shndx(r_sym, shndx, &is_ordinary);
                if (!is_ordinary)
                  {
                    object->error(_("local symbol %u has bad shndx %u"),
                                  r_sym, shndx);
                    break;
                  }
                got->record_local_got_symbol(mips_obj, r_sym, r_addend, r_type,
                                             shndx, false);
              }
            else
              {
                // FIXME: TLS optimization not supported yet.
                gold_unreachable();
              }
            break;

          case elfcpp::R_MIPS_TLS_LDM:
          case elfcpp::R_MIPS16_TLS_LDM:
          case elfcpp::R_MICROMIPS_TLS_LDM:
            if (optimized_type == tls::TLSOPT_NONE)
              {
                // We always record LDM symbols as local with index 0.
                target->got_section()->record_local_got_symbol(mips_obj, 0,
                                                               r_addend, r_type,
                                                               -1U, false);
              }
            else
              {
                // FIXME: TLS optimization not supported yet.
                gold_unreachable();
              }
            break;
          case elfcpp::R_MIPS_TLS_GOTTPREL:
          case elfcpp::R_MIPS16_TLS_GOTTPREL:
          case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
            layout->set_has_static_tls();
            if (optimized_type == tls::TLSOPT_NONE)
              {
                // Create a GOT entry for the tp-relative offset.
                Mips_output_data_got<size, big_endian>* got =
                  target->got_section(symtab, layout);
                got->record_local_got_symbol(mips_obj, r_sym, r_addend, r_type,
                                             -1U, false);
              }
            else
              {
                // FIXME: TLS optimization not supported yet.
                gold_unreachable();
              }
            break;

          default:
            gold_unreachable();
        }
      }
      break;

    default:
      break;
    }

  // Refuse some position-dependent relocations when creating a
  // shared library.  Do not refuse R_MIPS_32 / R_MIPS_64; they're
  // not PIC, but we can create dynamic relocations and the result
  // will be fine.  Also do not refuse R_MIPS_LO16, which can be
  // combined with R_MIPS_GOT16.
  if (parameters->options().shared())
    {
      switch (r_type)
        {
        case elfcpp::R_MIPS16_HI16:
        case elfcpp::R_MIPS_HI16:
        case elfcpp::R_MIPS_HIGHER:
        case elfcpp::R_MIPS_HIGHEST:
        case elfcpp::R_MICROMIPS_HI16:
        case elfcpp::R_MICROMIPS_HIGHER:
        case elfcpp::R_MICROMIPS_HIGHEST:
          // Don't refuse a high part relocation if it's against
          // no symbol (e.g. part of a compound relocation).
          if (r_sym == 0)
            break;
	  // Fall through.

        case elfcpp::R_MIPS16_26:
        case elfcpp::R_MIPS_26:
        case elfcpp::R_MICROMIPS_26_S1:
          gold_error(_("%s: relocation %u against `%s' can not be used when "
                       "making a shared object; recompile with -fPIC"),
                     object->name().c_str(), r_type, "a local symbol");
        default:
          break;
        }
    }
}

template<int size, bool big_endian>
inline void
Target_mips<size, big_endian>::Scan::local(
                        Symbol_table* symtab,
                        Layout* layout,
                        Target_mips<size, big_endian>* target,
                        Sized_relobj_file<size, big_endian>* object,
                        unsigned int data_shndx,
                        Output_section* output_section,
                        const Reltype& reloc,
                        unsigned int r_type,
                        const elfcpp::Sym<size, big_endian>& lsym,
                        bool is_discarded)
{
  if (is_discarded)
    return;

  local(
    symtab,
    layout,
    target,
    object,
    data_shndx,
    output_section,
    (const Relatype*) NULL,
    &reloc,
    elfcpp::SHT_REL,
    r_type,
    lsym, is_discarded);
}


template<int size, bool big_endian>
inline void
Target_mips<size, big_endian>::Scan::local(
                        Symbol_table* symtab,
                        Layout* layout,
                        Target_mips<size, big_endian>* target,
                        Sized_relobj_file<size, big_endian>* object,
                        unsigned int data_shndx,
                        Output_section* output_section,
                        const Relatype& reloc,
                        unsigned int r_type,
                        const elfcpp::Sym<size, big_endian>& lsym,
                        bool is_discarded)
{
  if (is_discarded)
    return;

  local(
    symtab,
    layout,
    target,
    object,
    data_shndx,
    output_section,
    &reloc,
    (const Reltype*) NULL,
    elfcpp::SHT_RELA,
    r_type,
    lsym, is_discarded);
}

// Scan a relocation for a global symbol.

template<int size, bool big_endian>
inline void
Target_mips<size, big_endian>::Scan::global(
                                Symbol_table* symtab,
                                Layout* layout,
                                Target_mips<size, big_endian>* target,
                                Sized_relobj_file<size, big_endian>* object,
                                unsigned int data_shndx,
                                Output_section* output_section,
                                const Relatype* rela,
                                const Reltype* rel,
                                unsigned int rel_type,
                                unsigned int r_type,
                                Symbol* gsym)
{
  Mips_address r_offset;
  unsigned int r_sym;
  typename elfcpp::Elf_types<size>::Elf_Swxword r_addend;

  if (rel_type == elfcpp::SHT_RELA)
    {
      r_offset = rela->get_r_offset();
      r_sym = Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>::
	  get_r_sym(rela);
      r_addend = rela->get_r_addend();
    }
  else
    {
      r_offset = rel->get_r_offset();
      r_sym = Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>::
	  get_r_sym(rel);
      r_addend = 0;
    }

  Mips_relobj<size, big_endian>* mips_obj =
    Mips_relobj<size, big_endian>::as_mips_relobj(object);
  Mips_symbol<size>* mips_sym = Mips_symbol<size>::as_mips_sym(gsym);

  if (mips_obj->is_mips16_stub_section(data_shndx))
    {
      mips_obj->get_mips16_stub_section(data_shndx)
              ->new_global_reloc_found(r_type, mips_sym);
    }

  if (r_type == elfcpp::R_MIPS_NONE)
    // R_MIPS_NONE is used in mips16 stub sections, to define the target of the
    // mips16 stub.
    return;

  if (!mips16_call_reloc(r_type)
      && !mips_obj->section_allows_mips16_refs(data_shndx))
    // This reloc would need to refer to a MIPS16 hard-float stub, if
    // there is one.  We ignore MIPS16 stub sections and .pdr section when
    // looking for relocs that would need to refer to MIPS16 stubs.
    mips_sym->set_need_fn_stub();

  // We need PLT entries if there are static-only relocations against
  // an externally-defined function.  This can technically occur for
  // shared libraries if there are branches to the symbol, although it
  // is unlikely that this will be used in practice due to the short
  // ranges involved.  It can occur for any relative or absolute relocation
  // in executables; in that case, the PLT entry becomes the function's
  // canonical address.
  bool static_reloc = false;

  // Set CAN_MAKE_DYNAMIC to true if we can convert this
  // relocation into a dynamic one.
  bool can_make_dynamic = false;
  switch (r_type)
    {
    case elfcpp::R_MIPS_GOT16:
    case elfcpp::R_MIPS_CALL16:
    case elfcpp::R_MIPS_CALL_HI16:
    case elfcpp::R_MIPS_CALL_LO16:
    case elfcpp::R_MIPS_GOT_HI16:
    case elfcpp::R_MIPS_GOT_LO16:
    case elfcpp::R_MIPS_GOT_PAGE:
    case elfcpp::R_MIPS_GOT_OFST:
    case elfcpp::R_MIPS_GOT_DISP:
    case elfcpp::R_MIPS_TLS_GOTTPREL:
    case elfcpp::R_MIPS_TLS_GD:
    case elfcpp::R_MIPS_TLS_LDM:
    case elfcpp::R_MIPS16_GOT16:
    case elfcpp::R_MIPS16_CALL16:
    case elfcpp::R_MIPS16_TLS_GOTTPREL:
    case elfcpp::R_MIPS16_TLS_GD:
    case elfcpp::R_MIPS16_TLS_LDM:
    case elfcpp::R_MICROMIPS_GOT16:
    case elfcpp::R_MICROMIPS_CALL16:
    case elfcpp::R_MICROMIPS_CALL_HI16:
    case elfcpp::R_MICROMIPS_CALL_LO16:
    case elfcpp::R_MICROMIPS_GOT_HI16:
    case elfcpp::R_MICROMIPS_GOT_LO16:
    case elfcpp::R_MICROMIPS_GOT_PAGE:
    case elfcpp::R_MICROMIPS_GOT_OFST:
    case elfcpp::R_MICROMIPS_GOT_DISP:
    case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
    case elfcpp::R_MICROMIPS_TLS_GD:
    case elfcpp::R_MICROMIPS_TLS_LDM:
    case elfcpp::R_MIPS_EH:
      // We need a GOT section.
      target->got_section(symtab, layout);
      break;

    // This is just a hint; it can safely be ignored.  Don't set
    // has_static_relocs for the corresponding symbol.
    case elfcpp::R_MIPS_JALR:
    case elfcpp::R_MICROMIPS_JALR:
      break;

    case elfcpp::R_MIPS_GPREL16:
    case elfcpp::R_MIPS_GPREL32:
    case elfcpp::R_MIPS16_GPREL:
    case elfcpp::R_MICROMIPS_GPREL16:
      // TODO(sasa)
      // GP-relative relocations always resolve to a definition in a
      // regular input file, ignoring the one-definition rule.  This is
      // important for the GP setup sequence in NewABI code, which
      // always resolves to a local function even if other relocations
      // against the symbol wouldn't.
      //constrain_symbol_p = FALSE;
      break;

    case elfcpp::R_MIPS_32:
    case elfcpp::R_MIPS_REL32:
    case elfcpp::R_MIPS_64:
      if ((parameters->options().shared()
          || (strcmp(gsym->name(), "__gnu_local_gp") != 0
          && (!is_readonly_section(output_section)
          || mips_obj->is_pic())))
          && (output_section->flags() & elfcpp::SHF_ALLOC) != 0)
        {
          if (r_type != elfcpp::R_MIPS_REL32)
            mips_sym->set_pointer_equality_needed();
          can_make_dynamic = true;
          break;
        }
      // Fall through.

    default:
      // Most static relocations require pointer equality, except
      // for branches.
      mips_sym->set_pointer_equality_needed();
      // Fall through.

    case elfcpp::R_MIPS_26:
    case elfcpp::R_MIPS_PC16:
    case elfcpp::R_MIPS_PC21_S2:
    case elfcpp::R_MIPS_PC26_S2:
    case elfcpp::R_MIPS16_26:
    case elfcpp::R_MICROMIPS_26_S1:
    case elfcpp::R_MICROMIPS_PC7_S1:
    case elfcpp::R_MICROMIPS_PC10_S1:
    case elfcpp::R_MICROMIPS_PC16_S1:
    case elfcpp::R_MICROMIPS_PC23_S2:
      static_reloc = true;
      mips_sym->set_has_static_relocs();
      break;
    }

  // If there are call relocations against an externally-defined symbol,
  // see whether we can create a MIPS lazy-binding stub for it.  We can
  // only do this if all references to the function are through call
  // relocations, and in that case, the traditional lazy-binding stubs
  // are much more efficient than PLT entries.
  switch (r_type)
    {
    case elfcpp::R_MIPS16_CALL16:
    case elfcpp::R_MIPS_CALL16:
    case elfcpp::R_MIPS_CALL_HI16:
    case elfcpp::R_MIPS_CALL_LO16:
    case elfcpp::R_MIPS_JALR:
    case elfcpp::R_MICROMIPS_CALL16:
    case elfcpp::R_MICROMIPS_CALL_HI16:
    case elfcpp::R_MICROMIPS_CALL_LO16:
    case elfcpp::R_MICROMIPS_JALR:
      if (!mips_sym->no_lazy_stub())
        {
          if ((mips_sym->needs_plt_entry() && mips_sym->is_from_dynobj())
              // Calls from shared objects to undefined symbols of type
              // STT_NOTYPE need lazy-binding stub.
              || (mips_sym->is_undefined() && parameters->options().shared()))
            target->mips_stubs_section(layout)->make_entry(mips_sym);
        }
      break;
    default:
      {
        // We must not create a stub for a symbol that has relocations
        // related to taking the function's address.
        mips_sym->set_no_lazy_stub();
        target->remove_lazy_stub_entry(mips_sym);
        break;
      }
  }

  if (relocation_needs_la25_stub<size, big_endian>(mips_obj, r_type,
                                                   mips_sym->is_mips16()))
    mips_sym->set_has_nonpic_branches();

  // R_MIPS_HI16 against _gp_disp is used for $gp setup,
  // and has a special meaning.
  bool gp_disp_against_hi16 = (!mips_obj->is_newabi()
                               && strcmp(gsym->name(), "_gp_disp") == 0
                               && (hi16_reloc(r_type) || lo16_reloc(r_type)));
  if (static_reloc && gsym->needs_plt_entry())
    {
      target->make_plt_entry(symtab, layout, mips_sym, r_type);

      // Since this is not a PC-relative relocation, we may be
      // taking the address of a function.  In that case we need to
      // set the entry in the dynamic symbol table to the address of
      // the PLT entry.
      if (gsym->is_from_dynobj() && !parameters->options().shared())
        {
          gsym->set_needs_dynsym_value();
          // We distinguish between PLT entries and lazy-binding stubs by
          // giving the former an st_other value of STO_MIPS_PLT.  Set the
          // flag if there are any relocations in the binary where pointer
          // equality matters.
          if (mips_sym->pointer_equality_needed())
            mips_sym->set_mips_plt();
        }
    }
  if ((static_reloc || can_make_dynamic) && !gp_disp_against_hi16)
    {
      // Absolute addressing relocations.
      // Make a dynamic relocation if necessary.
      if (gsym->needs_dynamic_reloc(Scan::get_reference_flags(r_type)))
        {
          if (gsym->may_need_copy_reloc())
            {
              target->copy_reloc(symtab, layout, object, data_shndx,
                                 output_section, gsym, r_type, r_offset);
            }
          else if (can_make_dynamic)
            {
              // Create .rel.dyn section.
              target->rel_dyn_section(layout);
              target->dynamic_reloc(mips_sym, elfcpp::R_MIPS_REL32, mips_obj,
                                    data_shndx, output_section, r_offset);
            }
          else
            gold_error(_("non-dynamic relocations refer to dynamic symbol %s"),
                       gsym->name());
        }
    }

  bool for_call = false;
  switch (r_type)
    {
    case elfcpp::R_MIPS_CALL16:
    case elfcpp::R_MIPS16_CALL16:
    case elfcpp::R_MICROMIPS_CALL16:
    case elfcpp::R_MIPS_CALL_HI16:
    case elfcpp::R_MIPS_CALL_LO16:
    case elfcpp::R_MICROMIPS_CALL_HI16:
    case elfcpp::R_MICROMIPS_CALL_LO16:
      for_call = true;
      // Fall through.

    case elfcpp::R_MIPS16_GOT16:
    case elfcpp::R_MIPS_GOT16:
    case elfcpp::R_MIPS_GOT_HI16:
    case elfcpp::R_MIPS_GOT_LO16:
    case elfcpp::R_MICROMIPS_GOT16:
    case elfcpp::R_MICROMIPS_GOT_HI16:
    case elfcpp::R_MICROMIPS_GOT_LO16:
    case elfcpp::R_MIPS_GOT_DISP:
    case elfcpp::R_MICROMIPS_GOT_DISP:
    case elfcpp::R_MIPS_EH:
      {
        // The symbol requires a GOT entry.
        Mips_output_data_got<size, big_endian>* got =
          target->got_section(symtab, layout);
        got->record_global_got_symbol(mips_sym, mips_obj, r_type, false,
                                      for_call);
        mips_sym->set_global_got_area(GGA_NORMAL);
      }
      break;

    case elfcpp::R_MIPS_GOT_PAGE:
    case elfcpp::R_MICROMIPS_GOT_PAGE:
      {
        // This relocation needs a page entry in the GOT.
        // Get the section contents.
        section_size_type view_size = 0;
        const unsigned char* view =
          object->section_contents(data_shndx, &view_size, false);
        view += r_offset;

        Valtype32 val = elfcpp::Swap<32, big_endian>::readval(view);
        Valtype32 addend = (rel_type == elfcpp::SHT_REL ? val & 0xffff
                                                        : r_addend);
        Mips_output_data_got<size, big_endian>* got =
          target->got_section(symtab, layout);
        got->record_got_page_entry(mips_obj, r_sym, addend);

        // If this is a global, overridable symbol, GOT_PAGE will
        // decay to GOT_DISP, so we'll need a GOT entry for it.
        bool def_regular = (mips_sym->source() == Symbol::FROM_OBJECT
                            && !mips_sym->object()->is_dynamic()
                            && !mips_sym->is_undefined());
        if (!def_regular
            || (parameters->options().output_is_position_independent()
                && !parameters->options().Bsymbolic()
                && !mips_sym->is_forced_local()))
          {
            got->record_global_got_symbol(mips_sym, mips_obj, r_type, false,
                                          for_call);
            mips_sym->set_global_got_area(GGA_NORMAL);
          }
      }
      break;

    case elfcpp::R_MIPS_TLS_GOTTPREL:
    case elfcpp::R_MIPS16_TLS_GOTTPREL:
    case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
    case elfcpp::R_MIPS_TLS_LDM:
    case elfcpp::R_MIPS16_TLS_LDM:
    case elfcpp::R_MICROMIPS_TLS_LDM:
    case elfcpp::R_MIPS_TLS_GD:
    case elfcpp::R_MIPS16_TLS_GD:
    case elfcpp::R_MICROMIPS_TLS_GD:
      {
        const bool is_final = gsym->final_value_is_known();
        const tls::Tls_optimization optimized_type =
          Target_mips<size, big_endian>::optimize_tls_reloc(is_final, r_type);

        switch (r_type)
          {
          case elfcpp::R_MIPS_TLS_GD:
          case elfcpp::R_MIPS16_TLS_GD:
          case elfcpp::R_MICROMIPS_TLS_GD:
            if (optimized_type == tls::TLSOPT_NONE)
              {
                // Create a pair of GOT entries for the module index and
                // dtv-relative offset.
                Mips_output_data_got<size, big_endian>* got =
                  target->got_section(symtab, layout);
                got->record_global_got_symbol(mips_sym, mips_obj, r_type, false,
                                              false);
              }
            else
              {
                // FIXME: TLS optimization not supported yet.
                gold_unreachable();
              }
            break;

          case elfcpp::R_MIPS_TLS_LDM:
          case elfcpp::R_MIPS16_TLS_LDM:
          case elfcpp::R_MICROMIPS_TLS_LDM:
            if (optimized_type == tls::TLSOPT_NONE)
              {
                // We always record LDM symbols as local with index 0.
                target->got_section()->record_local_got_symbol(mips_obj, 0,
                                                               r_addend, r_type,
                                                               -1U, false);
              }
            else
              {
                // FIXME: TLS optimization not supported yet.
                gold_unreachable();
              }
            break;
          case elfcpp::R_MIPS_TLS_GOTTPREL:
          case elfcpp::R_MIPS16_TLS_GOTTPREL:
          case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
            layout->set_has_static_tls();
            if (optimized_type == tls::TLSOPT_NONE)
              {
                // Create a GOT entry for the tp-relative offset.
                Mips_output_data_got<size, big_endian>* got =
                  target->got_section(symtab, layout);
                got->record_global_got_symbol(mips_sym, mips_obj, r_type, false,
                                              false);
              }
            else
              {
                // FIXME: TLS optimization not supported yet.
                gold_unreachable();
              }
            break;

          default:
            gold_unreachable();
        }
      }
      break;
    case elfcpp::R_MIPS_COPY:
    case elfcpp::R_MIPS_JUMP_SLOT:
      // These are relocations which should only be seen by the
      // dynamic linker, and should never be seen here.
      gold_error(_("%s: unexpected reloc %u in object file"),
                 object->name().c_str(), r_type);
      break;

    default:
      break;
    }

  // Refuse some position-dependent relocations when creating a
  // shared library.  Do not refuse R_MIPS_32 / R_MIPS_64; they're
  // not PIC, but we can create dynamic relocations and the result
  // will be fine.  Also do not refuse R_MIPS_LO16, which can be
  // combined with R_MIPS_GOT16.
  if (parameters->options().shared())
    {
      switch (r_type)
        {
        case elfcpp::R_MIPS16_HI16:
        case elfcpp::R_MIPS_HI16:
        case elfcpp::R_MIPS_HIGHER:
        case elfcpp::R_MIPS_HIGHEST:
        case elfcpp::R_MICROMIPS_HI16:
        case elfcpp::R_MICROMIPS_HIGHER:
        case elfcpp::R_MICROMIPS_HIGHEST:
          // Don't refuse a high part relocation if it's against
          // no symbol (e.g. part of a compound relocation).
          if (r_sym == 0)
            break;

          // R_MIPS_HI16 against _gp_disp is used for $gp setup,
          // and has a special meaning.
          if (!mips_obj->is_newabi() && strcmp(gsym->name(), "_gp_disp") == 0)
            break;
	  // Fall through.

        case elfcpp::R_MIPS16_26:
        case elfcpp::R_MIPS_26:
        case elfcpp::R_MICROMIPS_26_S1:
          gold_error(_("%s: relocation %u against `%s' can not be used when "
                       "making a shared object; recompile with -fPIC"),
                     object->name().c_str(), r_type, gsym->name());
        default:
          break;
        }
    }
}

template<int size, bool big_endian>
inline void
Target_mips<size, big_endian>::Scan::global(
                                Symbol_table* symtab,
                                Layout* layout,
                                Target_mips<size, big_endian>* target,
                                Sized_relobj_file<size, big_endian>* object,
                                unsigned int data_shndx,
                                Output_section* output_section,
                                const Relatype& reloc,
                                unsigned int r_type,
                                Symbol* gsym)
{
  global(
    symtab,
    layout,
    target,
    object,
    data_shndx,
    output_section,
    &reloc,
    (const Reltype*) NULL,
    elfcpp::SHT_RELA,
    r_type,
    gsym);
}

template<int size, bool big_endian>
inline void
Target_mips<size, big_endian>::Scan::global(
                                Symbol_table* symtab,
                                Layout* layout,
                                Target_mips<size, big_endian>* target,
                                Sized_relobj_file<size, big_endian>* object,
                                unsigned int data_shndx,
                                Output_section* output_section,
                                const Reltype& reloc,
                                unsigned int r_type,
                                Symbol* gsym)
{
  global(
    symtab,
    layout,
    target,
    object,
    data_shndx,
    output_section,
    (const Relatype*) NULL,
    &reloc,
    elfcpp::SHT_REL,
    r_type,
    gsym);
}

// Return whether a R_MIPS_32/R_MIPS64 relocation needs to be applied.
// In cases where Scan::local() or Scan::global() has created
// a dynamic relocation, the addend of the relocation is carried
// in the data, and we must not apply the static relocation.

template<int size, bool big_endian>
inline bool
Target_mips<size, big_endian>::Relocate::should_apply_static_reloc(
    const Mips_symbol<size>* gsym,
    unsigned int r_type,
    Output_section* output_section,
    Target_mips* target)
{
  // If the output section is not allocated, then we didn't call
  // scan_relocs, we didn't create a dynamic reloc, and we must apply
  // the reloc here.
  if ((output_section->flags() & elfcpp::SHF_ALLOC) == 0)
      return true;

  if (gsym == NULL)
    return true;
  else
    {
      // For global symbols, we use the same helper routines used in the
      // scan pass.
      if (gsym->needs_dynamic_reloc(Scan::get_reference_flags(r_type))
          && !gsym->may_need_copy_reloc())
        {
          // We have generated dynamic reloc (R_MIPS_REL32).

          bool multi_got = false;
          if (target->has_got_section())
            multi_got = target->got_section()->multi_got();
          bool has_got_offset;
          if (!multi_got)
            has_got_offset = gsym->has_got_offset(GOT_TYPE_STANDARD);
          else
            has_got_offset = gsym->global_gotoffset() != -1U;
          if (!has_got_offset)
            return true;
          else
            // Apply the relocation only if the symbol is in the local got.
            // Do not apply the relocation if the symbol is in the global
            // got.
            return symbol_references_local(gsym, gsym->has_dynsym_index());
        }
      else
        // We have not generated dynamic reloc.
        return true;
    }
}

// Perform a relocation.

template<int size, bool big_endian>
inline bool
Target_mips<size, big_endian>::Relocate::relocate(
                        const Relocate_info<size, big_endian>* relinfo,
                        unsigned int rel_type,
                        Target_mips* target,
                        Output_section* output_section,
                        size_t relnum,
                        const unsigned char* preloc,
                        const Sized_symbol<size>* gsym,
                        const Symbol_value<size>* psymval,
                        unsigned char* view,
                        Mips_address address,
                        section_size_type)
{
  Mips_address r_offset;
  unsigned int r_sym;
  unsigned int r_type;
  unsigned int r_type2;
  unsigned int r_type3;
  unsigned char r_ssym;
  typename elfcpp::Elf_types<size>::Elf_Swxword r_addend;
  // r_offset and r_type of the next relocation is needed for resolving multiple
  // consecutive relocations with the same offset.
  Mips_address next_r_offset = static_cast<Mips_address>(0) - 1;
  unsigned int next_r_type = elfcpp::R_MIPS_NONE;

  elfcpp::Shdr<size, big_endian> shdr(relinfo->reloc_shdr);
  size_t reloc_count = shdr.get_sh_size() / shdr.get_sh_entsize();

  if (rel_type == elfcpp::SHT_RELA)
    {
      const Relatype rela(preloc);
      r_offset = rela.get_r_offset();
      r_sym = Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>::
	  get_r_sym(&rela);
      r_type = Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>::
	  get_r_type(&rela);
      r_type2 = Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>::
          get_r_type2(&rela);
      r_type3 = Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>::
          get_r_type3(&rela);
      r_ssym = Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>::
          get_r_ssym(&rela);
      r_addend = rela.get_r_addend();
      // If this is not last relocation, get r_offset and r_type of the next
      // relocation.
      if (relnum + 1 < reloc_count)
        {
          const int reloc_size = elfcpp::Elf_sizes<size>::rela_size;
          const Relatype next_rela(preloc + reloc_size);
          next_r_offset = next_rela.get_r_offset();
          next_r_type =
            Mips_classify_reloc<elfcpp::SHT_RELA, size, big_endian>::
              get_r_type(&next_rela);
        }
    }
  else
    {
      const Reltype rel(preloc);
      r_offset = rel.get_r_offset();
      r_sym = Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>::
	  get_r_sym(&rel);
      r_type = Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>::
	  get_r_type(&rel);
      r_ssym = 0;
      r_type2 = elfcpp::R_MIPS_NONE;
      r_type3 = elfcpp::R_MIPS_NONE;
      r_addend = 0;
      // If this is not last relocation, get r_offset and r_type of the next
      // relocation.
      if (relnum + 1 < reloc_count)
        {
          const int reloc_size = elfcpp::Elf_sizes<size>::rel_size;
          const Reltype next_rel(preloc + reloc_size);
          next_r_offset = next_rel.get_r_offset();
          next_r_type = Mips_classify_reloc<elfcpp::SHT_REL, size, big_endian>::
            get_r_type(&next_rel);
        }
    }

  typedef Mips_relocate_functions<size, big_endian> Reloc_funcs;
  typename Reloc_funcs::Status reloc_status = Reloc_funcs::STATUS_OKAY;

  Mips_relobj<size, big_endian>* object =
      Mips_relobj<size, big_endian>::as_mips_relobj(relinfo->object);

  bool target_is_16_bit_code = false;
  bool target_is_micromips_code = false;
  bool cross_mode_jump;

  Symbol_value<size> symval;

  const Mips_symbol<size>* mips_sym = Mips_symbol<size>::as_mips_sym(gsym);

  bool changed_symbol_value = false;
  if (gsym == NULL)
    {
      target_is_16_bit_code = object->local_symbol_is_mips16(r_sym);
      target_is_micromips_code = object->local_symbol_is_micromips(r_sym);
      if (target_is_16_bit_code || target_is_micromips_code)
        {
          // MIPS16/microMIPS text labels should be treated as odd.
          symval.set_output_value(psymval->value(object, 1));
          psymval = &symval;
          changed_symbol_value = true;
        }
    }
  else
    {
      target_is_16_bit_code = mips_sym->is_mips16();
      target_is_micromips_code = mips_sym->is_micromips();

      // If this is a mips16/microMIPS text symbol, add 1 to the value to make
      // it odd.  This will cause something like .word SYM to come up with
      // the right value when it is loaded into the PC.

      if ((mips_sym->is_mips16() || mips_sym->is_micromips())
          && psymval->value(object, 0) != 0)
        {
          symval.set_output_value(psymval->value(object, 0) | 1);
          psymval = &symval;
          changed_symbol_value = true;
        }

      // Pick the value to use for symbols defined in shared objects.
      if (mips_sym->use_plt_offset(Scan::get_reference_flags(r_type))
          || mips_sym->has_lazy_stub())
        {
          Mips_address value;
          if (!mips_sym->has_lazy_stub())
            {
              // Prefer a standard MIPS PLT entry.
              if (mips_sym->has_mips_plt_offset())
                {
                  value = target->plt_section()->mips_entry_address(mips_sym);
                  target_is_micromips_code = false;
                  target_is_16_bit_code = false;
                }
              else
                {
                  value = (target->plt_section()->comp_entry_address(mips_sym)
                           + 1);
                  if (target->is_output_micromips())
                    target_is_micromips_code = true;
                  else
                    target_is_16_bit_code = true;
                }
            }
          else
            value = target->mips_stubs_section()->stub_address(mips_sym);

          symval.set_output_value(value);
          psymval = &symval;
        }
    }

  // TRUE if the symbol referred to by this relocation is "_gp_disp".
  // Note that such a symbol must always be a global symbol.
  bool gp_disp = (gsym != NULL && (strcmp(gsym->name(), "_gp_disp") == 0)
                  && !object->is_newabi());

  // TRUE if the symbol referred to by this relocation is "__gnu_local_gp".
  // Note that such a symbol must always be a global symbol.
  bool gnu_local_gp = gsym && (strcmp(gsym->name(), "__gnu_local_gp") == 0);


  if (gp_disp)
    {
      if (!hi16_reloc(r_type) && !lo16_reloc(r_type))
        gold_error_at_location(relinfo, relnum, r_offset,
          _("relocations against _gp_disp are permitted only"
            " with R_MIPS_HI16 and R_MIPS_LO16 relocations."));
    }
  else if (gnu_local_gp)
    {
      // __gnu_local_gp is _gp symbol.
      symval.set_output_value(target->adjusted_gp_value(object));
      psymval = &symval;
    }

  // If this is a reference to a 16-bit function with a stub, we need
  // to redirect the relocation to the stub unless:
  //
  // (a) the relocation is for a MIPS16 JAL;
  //
  // (b) the relocation is for a MIPS16 PIC call, and there are no
  //     non-MIPS16 uses of the GOT slot; or
  //
  // (c) the section allows direct references to MIPS16 functions.
  if (r_type != elfcpp::R_MIPS16_26
      && ((mips_sym != NULL
           && mips_sym->has_mips16_fn_stub()
           && (r_type != elfcpp::R_MIPS16_CALL16 || mips_sym->need_fn_stub()))
          || (mips_sym == NULL
              && object->get_local_mips16_fn_stub(r_sym) != NULL))
      && !object->section_allows_mips16_refs(relinfo->data_shndx))
    {
      // This is a 32- or 64-bit call to a 16-bit function.  We should
      // have already noticed that we were going to need the
      // stub.
      Mips_address value;
      if (mips_sym == NULL)
        value = object->get_local_mips16_fn_stub(r_sym)->output_address();
      else
        {
          gold_assert(mips_sym->need_fn_stub());
          if (mips_sym->has_la25_stub())
            value = target->la25_stub_section()->stub_address(mips_sym);
          else
            {
              value = mips_sym->template
                      get_mips16_fn_stub<big_endian>()->output_address();
            }
          }
      symval.set_output_value(value);
      psymval = &symval;
      changed_symbol_value = true;

      // The target is 16-bit, but the stub isn't.
      target_is_16_bit_code = false;
    }
  // If this is a MIPS16 call with a stub, that is made through the PLT or
  // to a standard MIPS function, we need to redirect the call to the stub.
  // Note that we specifically exclude R_MIPS16_CALL16 from this behavior;
  // indirect calls should use an indirect stub instead.
  else if (r_type == elfcpp::R_MIPS16_26
           && ((mips_sym != NULL
                && (mips_sym->has_mips16_call_stub()
                    || mips_sym->has_mips16_call_fp_stub()))
               || (mips_sym == NULL
                   && object->get_local_mips16_call_stub(r_sym) != NULL))
           && ((mips_sym != NULL && mips_sym->has_plt_offset())
               || !target_is_16_bit_code))
    {
      Mips16_stub_section<size, big_endian>* call_stub;
      if (mips_sym == NULL)
        call_stub = object->get_local_mips16_call_stub(r_sym);
      else
        {
          // If both call_stub and call_fp_stub are defined, we can figure
          // out which one to use by checking which one appears in the input
          // file.
          if (mips_sym->has_mips16_call_stub()
              && mips_sym->has_mips16_call_fp_stub())
            {
              call_stub = NULL;
              for (unsigned int i = 1; i < object->shnum(); ++i)
                {
                  if (object->is_mips16_call_fp_stub_section(i))
                    {
                      call_stub = mips_sym->template
                                  get_mips16_call_fp_stub<big_endian>();
                      break;
                    }

                }
              if (call_stub == NULL)
                call_stub =
                  mips_sym->template get_mips16_call_stub<big_endian>();
            }
          else if (mips_sym->has_mips16_call_stub())
            call_stub = mips_sym->template get_mips16_call_stub<big_endian>();
          else
            call_stub = mips_sym->template get_mips16_call_fp_stub<big_endian>();
        }

      symval.set_output_value(call_stub->output_address());
      psymval = &symval;
      changed_symbol_value = true;
    }
  // If this is a direct call to a PIC function, redirect to the
  // non-PIC stub.
  else if (mips_sym != NULL
           && mips_sym->has_la25_stub()
           && relocation_needs_la25_stub<size, big_endian>(
                                       object, r_type, target_is_16_bit_code))
    {
      Mips_address value = target->la25_stub_section()->stub_address(mips_sym);
      if (mips_sym->is_micromips())
        value += 1;
      symval.set_output_value(value);
      psymval = &symval;
    }
  // For direct MIPS16 and microMIPS calls make sure the compressed PLT
  // entry is used if a standard PLT entry has also been made.
  else if ((r_type == elfcpp::R_MIPS16_26
            || r_type == elfcpp::R_MICROMIPS_26_S1)
          && mips_sym != NULL
          && mips_sym->has_plt_offset()
          && mips_sym->has_comp_plt_offset()
          && mips_sym->has_mips_plt_offset())
    {
      Mips_address value = (target->plt_section()->comp_entry_address(mips_sym)
                            + 1);
      symval.set_output_value(value);
      psymval = &symval;

      target_is_16_bit_code = !target->is_output_micromips();
      target_is_micromips_code = target->is_output_micromips();
    }

  // Make sure MIPS16 and microMIPS are not used together.
  if ((r_type == elfcpp::R_MIPS16_26 && target_is_micromips_code)
      || (micromips_branch_reloc(r_type) && target_is_16_bit_code))
   {
      gold_error(_("MIPS16 and microMIPS functions cannot call each other"));
   }

  // Calls from 16-bit code to 32-bit code and vice versa require the
  // mode change.  However, we can ignore calls to undefined weak symbols,
  // which should never be executed at runtime.  This exception is important
  // because the assembly writer may have "known" that any definition of the
  // symbol would be 16-bit code, and that direct jumps were therefore
  // acceptable.
  cross_mode_jump =
    (!(gsym != NULL && gsym->is_weak_undefined())
     && ((r_type == elfcpp::R_MIPS16_26 && !target_is_16_bit_code)
         || (r_type == elfcpp::R_MICROMIPS_26_S1 && !target_is_micromips_code)
         || ((r_type == elfcpp::R_MIPS_26 || r_type == elfcpp::R_MIPS_JALR)
             && (target_is_16_bit_code || target_is_micromips_code))));

  bool local = (mips_sym == NULL
                || (mips_sym->got_only_for_calls()
                    ? symbol_calls_local(mips_sym, mips_sym->has_dynsym_index())
                    : symbol_references_local(mips_sym,
                                              mips_sym->has_dynsym_index())));

  // Global R_MIPS_GOT_PAGE/R_MICROMIPS_GOT_PAGE relocations are equivalent
  // to R_MIPS_GOT_DISP/R_MICROMIPS_GOT_DISP.  The addend is applied by the
  // corresponding R_MIPS_GOT_OFST/R_MICROMIPS_GOT_OFST.
  if (got_page_reloc(r_type) && !local)
    r_type = (micromips_reloc(r_type) ? elfcpp::R_MICROMIPS_GOT_DISP
                                      : elfcpp::R_MIPS_GOT_DISP);

  unsigned int got_offset = 0;
  int gp_offset = 0;

  // Whether we have to extract addend from instruction.
  bool extract_addend = rel_type == elfcpp::SHT_REL;
  unsigned int r_types[3] = { r_type, r_type2, r_type3 };

  Reloc_funcs::mips_reloc_unshuffle(view, r_type, false);

  // For Mips64 N64 ABI, there may be up to three operations specified per
  // record, by the fields r_type, r_type2, and r_type3. The first operation
  // takes its addend from the relocation record. Each subsequent operation
  // takes as its addend the result of the previous operation.
  // The first operation in a record which references a symbol uses the symbol
  // implied by r_sym. The next operation in a record which references a symbol
  // uses the special symbol value given by the r_ssym field. A third operation
  // in a record which references a symbol will assume a NULL symbol,
  // i.e. value zero.

  // TODO(Vladimir)
  // Check if a record references to a symbol.
  for (unsigned int i = 0; i < 3; ++i)
    {
      if (r_types[i] == elfcpp::R_MIPS_NONE)
        break;

      // If we didn't apply previous relocation, use its result as addend
      // for current.
      if (this->calculate_only_)
        {
          r_addend = this->calculated_value_;
          extract_addend = false;
        }

      // In the N32 and 64-bit ABIs there may be multiple consecutive
      // relocations for the same offset.  In that case we are
      // supposed to treat the output of each relocation as the addend
      // for the next.  For N64 ABI, we are checking offsets only in a
      // third operation in a record (r_type3).
      this->calculate_only_ =
        (object->is_n64() && i < 2
         ? r_types[i+1] != elfcpp::R_MIPS_NONE
         : (r_offset == next_r_offset) && (next_r_type != elfcpp::R_MIPS_NONE));

      if (object->is_n64())
        {
          if (i == 1)
            {
              // Handle special symbol for r_type2 relocation type.
              switch (r_ssym)
                {
                case RSS_UNDEF:
                  symval.set_output_value(0);
                  break;
                case RSS_GP:
                  symval.set_output_value(target->gp_value());
                  break;
                case RSS_GP0:
                  symval.set_output_value(object->gp_value());
                  break;
                case RSS_LOC:
                  symval.set_output_value(address);
                  break;
                default:
                  gold_unreachable();
                }
              psymval = &symval;
            }
          else if (i == 2)
           {
            // For r_type3 symbol value is 0.
            symval.set_output_value(0);
           }
        }

      bool update_got_entry = false;
      switch (r_types[i])
        {
        case elfcpp::R_MIPS_NONE:
          break;
        case elfcpp::R_MIPS_16:
          reloc_status = Reloc_funcs::rel16(view, object, psymval, r_addend,
                                            extract_addend,
                                            this->calculate_only_,
                                            &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_32:
          if (should_apply_static_reloc(mips_sym, r_types[i], output_section,
                                        target))
            reloc_status = Reloc_funcs::rel32(view, object, psymval, r_addend,
                                              extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          if (mips_sym != NULL
              && (mips_sym->is_mips16() || mips_sym->is_micromips())
              && mips_sym->global_got_area() == GGA_RELOC_ONLY)
            {
              // If mips_sym->has_mips16_fn_stub() is false, symbol value is
              // already updated by adding +1.
              if (mips_sym->has_mips16_fn_stub())
                {
                  gold_assert(mips_sym->need_fn_stub());
                  Mips16_stub_section<size, big_endian>* fn_stub =
                    mips_sym->template get_mips16_fn_stub<big_endian>();

                  symval.set_output_value(fn_stub->output_address());
                  psymval = &symval;
                }
              got_offset = mips_sym->global_gotoffset();
              update_got_entry = true;
            }
          break;

        case elfcpp::R_MIPS_64:
          if (should_apply_static_reloc(mips_sym, r_types[i], output_section,
                                        target))
            reloc_status = Reloc_funcs::rel64(view, object, psymval, r_addend,
                                              extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_, false);
          else if (target->is_output_n64() && r_addend != 0)
            // Only apply the addend.  The static relocation was RELA, but the
            // dynamic relocation is REL, so we need to apply the addend.
            reloc_status = Reloc_funcs::rel64(view, object, psymval, r_addend,
                                              extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_, true);
          break;
        case elfcpp::R_MIPS_REL32:
          gold_unreachable();

        case elfcpp::R_MIPS_PC32:
          reloc_status = Reloc_funcs::relpc32(view, object, psymval, address,
                                              r_addend, extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          break;

        case elfcpp::R_MIPS16_26:
          // The calculation for R_MIPS16_26 is just the same as for an
          // R_MIPS_26.  It's only the storage of the relocated field into
          // the output file that's different.  So, we just fall through to the
          // R_MIPS_26 case here.
        case elfcpp::R_MIPS_26:
        case elfcpp::R_MICROMIPS_26_S1:
          reloc_status = Reloc_funcs::rel26(view, object, psymval, address,
              gsym == NULL, r_addend, extract_addend, gsym, cross_mode_jump,
              r_types[i], target->jal_to_bal(), this->calculate_only_,
              &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_HI16:
        case elfcpp::R_MIPS16_HI16:
        case elfcpp::R_MICROMIPS_HI16:
          if (rel_type == elfcpp::SHT_RELA)
            reloc_status = Reloc_funcs::do_relhi16(view, object, psymval,
                                                   r_addend, address,
                                                   gp_disp, r_types[i],
                                                   extract_addend, 0,
                                                   target,
                                                   this->calculate_only_,
                                                   &this->calculated_value_);
          else if (rel_type == elfcpp::SHT_REL)
            reloc_status = Reloc_funcs::relhi16(view, object, psymval, r_addend,
                                                address, gp_disp, r_types[i],
                                                r_sym, extract_addend);
          else
            gold_unreachable();
          break;

        case elfcpp::R_MIPS_LO16:
        case elfcpp::R_MIPS16_LO16:
        case elfcpp::R_MICROMIPS_LO16:
        case elfcpp::R_MICROMIPS_HI0_LO16:
          reloc_status = Reloc_funcs::rello16(target, view, object, psymval,
                                              r_addend, extract_addend, address,
                                              gp_disp, r_types[i], r_sym,
                                              rel_type, this->calculate_only_,
                                              &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_LITERAL:
        case elfcpp::R_MICROMIPS_LITERAL:
          // Because we don't merge literal sections, we can handle this
          // just like R_MIPS_GPREL16.  In the long run, we should merge
          // shared literals, and then we will need to additional work
          // here.

          // Fall through.

        case elfcpp::R_MIPS_GPREL16:
        case elfcpp::R_MIPS16_GPREL:
        case elfcpp::R_MICROMIPS_GPREL7_S2:
        case elfcpp::R_MICROMIPS_GPREL16:
          reloc_status = Reloc_funcs::relgprel(view, object, psymval,
                                             target->adjusted_gp_value(object),
                                             r_addend, extract_addend,
                                             gsym == NULL, r_types[i],
                                             this->calculate_only_,
                                             &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_PC16:
          reloc_status = Reloc_funcs::relpc16(view, object, psymval, address,
                                              r_addend, extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_PC21_S2:
          reloc_status = Reloc_funcs::relpc21(view, object, psymval, address,
                                              r_addend, extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_PC26_S2:
          reloc_status = Reloc_funcs::relpc26(view, object, psymval, address,
                                              r_addend, extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_PC18_S3:
          reloc_status = Reloc_funcs::relpc18(view, object, psymval, address,
                                              r_addend, extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_PC19_S2:
          reloc_status = Reloc_funcs::relpc19(view, object, psymval, address,
                                              r_addend, extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_PCHI16:
          if (rel_type == elfcpp::SHT_RELA)
            reloc_status = Reloc_funcs::do_relpchi16(view, object, psymval,
                                                     r_addend, address,
                                                     extract_addend, 0,
                                                     this->calculate_only_,
                                                     &this->calculated_value_);
          else if (rel_type == elfcpp::SHT_REL)
            reloc_status = Reloc_funcs::relpchi16(view, object, psymval,
                                                  r_addend, address, r_sym,
                                                  extract_addend);
          else
            gold_unreachable();
          break;

        case elfcpp::R_MIPS_PCLO16:
          reloc_status = Reloc_funcs::relpclo16(view, object, psymval, r_addend,
                                                extract_addend, address, r_sym,
                                                rel_type, this->calculate_only_,
                                                &this->calculated_value_);
          break;
        case elfcpp::R_MICROMIPS_PC7_S1:
          reloc_status = Reloc_funcs::relmicromips_pc7_s1(view, object, psymval,
                                                      address, r_addend,
                                                      extract_addend,
                                                      this->calculate_only_,
                                                      &this->calculated_value_);
          break;
        case elfcpp::R_MICROMIPS_PC10_S1:
          reloc_status = Reloc_funcs::relmicromips_pc10_s1(view, object,
                                                      psymval, address,
                                                      r_addend, extract_addend,
                                                      this->calculate_only_,
                                                      &this->calculated_value_);
          break;
        case elfcpp::R_MICROMIPS_PC16_S1:
          reloc_status = Reloc_funcs::relmicromips_pc16_s1(view, object,
                                                      psymval, address,
                                                      r_addend, extract_addend,
                                                      this->calculate_only_,
                                                      &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_GPREL32:
          reloc_status = Reloc_funcs::relgprel32(view, object, psymval,
                                              target->adjusted_gp_value(object),
                                              r_addend, extract_addend,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_GOT_HI16:
        case elfcpp::R_MIPS_CALL_HI16:
        case elfcpp::R_MICROMIPS_GOT_HI16:
        case elfcpp::R_MICROMIPS_CALL_HI16:
          if (gsym != NULL)
            got_offset = target->got_section()->got_offset(gsym,
                                                           GOT_TYPE_STANDARD,
                                                           object);
          else
            got_offset = target->got_section()->got_offset(r_sym,
                                                           GOT_TYPE_STANDARD,
                                                           object, r_addend);
          gp_offset = target->got_section()->gp_offset(got_offset, object);
          reloc_status = Reloc_funcs::relgot_hi16(view, gp_offset,
                                                  this->calculate_only_,
                                                  &this->calculated_value_);
          update_got_entry = changed_symbol_value;
          break;

        case elfcpp::R_MIPS_GOT_LO16:
        case elfcpp::R_MIPS_CALL_LO16:
        case elfcpp::R_MICROMIPS_GOT_LO16:
        case elfcpp::R_MICROMIPS_CALL_LO16:
          if (gsym != NULL)
            got_offset = target->got_section()->got_offset(gsym,
                                                           GOT_TYPE_STANDARD,
                                                           object);
          else
            got_offset = target->got_section()->got_offset(r_sym,
                                                           GOT_TYPE_STANDARD,
                                                           object, r_addend);
          gp_offset = target->got_section()->gp_offset(got_offset, object);
          reloc_status = Reloc_funcs::relgot_lo16(view, gp_offset,
                                                  this->calculate_only_,
                                                  &this->calculated_value_);
          update_got_entry = changed_symbol_value;
          break;

        case elfcpp::R_MIPS_GOT_DISP:
        case elfcpp::R_MICROMIPS_GOT_DISP:
        case elfcpp::R_MIPS_EH:
          if (gsym != NULL)
            got_offset = target->got_section()->got_offset(gsym,
                                                           GOT_TYPE_STANDARD,
                                                           object);
          else
            got_offset = target->got_section()->got_offset(r_sym,
                                                           GOT_TYPE_STANDARD,
                                                           object, r_addend);
          gp_offset = target->got_section()->gp_offset(got_offset, object);
          if (eh_reloc(r_types[i]))
            reloc_status = Reloc_funcs::releh(view, gp_offset,
                                              this->calculate_only_,
                                              &this->calculated_value_);
          else
            reloc_status = Reloc_funcs::relgot(view, gp_offset,
                                               this->calculate_only_,
                                               &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_CALL16:
        case elfcpp::R_MIPS16_CALL16:
        case elfcpp::R_MICROMIPS_CALL16:
          gold_assert(gsym != NULL);
          got_offset = target->got_section()->got_offset(gsym,
                                                         GOT_TYPE_STANDARD,
                                                         object);
          gp_offset = target->got_section()->gp_offset(got_offset, object);
          reloc_status = Reloc_funcs::relgot(view, gp_offset,
                                             this->calculate_only_,
                                             &this->calculated_value_);
          // TODO(sasa): We should also initialize update_got_entry
          // in other place swhere relgot is called.
          update_got_entry = changed_symbol_value;
          break;

        case elfcpp::R_MIPS_GOT16:
        case elfcpp::R_MIPS16_GOT16:
        case elfcpp::R_MICROMIPS_GOT16:
          if (gsym != NULL)
            {
              got_offset = target->got_section()->got_offset(gsym,
                                                             GOT_TYPE_STANDARD,
                                                             object);
              gp_offset = target->got_section()->gp_offset(got_offset, object);
              reloc_status = Reloc_funcs::relgot(view, gp_offset,
                                                 this->calculate_only_,
                                                 &this->calculated_value_);
            }
          else
            {
              if (rel_type == elfcpp::SHT_RELA)
                reloc_status = Reloc_funcs::do_relgot16_local(view, object,
                                                      psymval, r_addend,
                                                      extract_addend, 0,
                                                      target,
                                                      this->calculate_only_,
                                                      &this->calculated_value_);
              else if (rel_type == elfcpp::SHT_REL)
                reloc_status = Reloc_funcs::relgot16_local(view, object,
                                                           psymval, r_addend,
                                                           extract_addend,
                                                           r_types[i], r_sym);
              else
                gold_unreachable();
            }
          update_got_entry = changed_symbol_value;
          break;

        case elfcpp::R_MIPS_TLS_GD:
        case elfcpp::R_MIPS16_TLS_GD:
        case elfcpp::R_MICROMIPS_TLS_GD:
          if (gsym != NULL)
            got_offset = target->got_section()->got_offset(gsym,
                                                           GOT_TYPE_TLS_PAIR,
                                                           object);
          else
            got_offset = target->got_section()->got_offset(r_sym,
                                                           GOT_TYPE_TLS_PAIR,
                                                           object, r_addend);
          gp_offset = target->got_section()->gp_offset(got_offset, object);
          reloc_status = Reloc_funcs::relgot(view, gp_offset,
                                             this->calculate_only_,
                                             &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_TLS_GOTTPREL:
        case elfcpp::R_MIPS16_TLS_GOTTPREL:
        case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
          if (gsym != NULL)
            got_offset = target->got_section()->got_offset(gsym,
                                                           GOT_TYPE_TLS_OFFSET,
                                                           object);
          else
            got_offset = target->got_section()->got_offset(r_sym,
                                                           GOT_TYPE_TLS_OFFSET,
                                                           object, r_addend);
          gp_offset = target->got_section()->gp_offset(got_offset, object);
          reloc_status = Reloc_funcs::relgot(view, gp_offset,
                                             this->calculate_only_,
                                             &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_TLS_LDM:
        case elfcpp::R_MIPS16_TLS_LDM:
        case elfcpp::R_MICROMIPS_TLS_LDM:
          // Relocate the field with the offset of the GOT entry for
          // the module index.
          got_offset = target->got_section()->tls_ldm_offset(object);
          gp_offset = target->got_section()->gp_offset(got_offset, object);
          reloc_status = Reloc_funcs::relgot(view, gp_offset,
                                             this->calculate_only_,
                                             &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_GOT_PAGE:
        case elfcpp::R_MICROMIPS_GOT_PAGE:
          reloc_status = Reloc_funcs::relgotpage(target, view, object, psymval,
                                                 r_addend, extract_addend,
                                                 this->calculate_only_,
                                                 &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_GOT_OFST:
        case elfcpp::R_MICROMIPS_GOT_OFST:
          reloc_status = Reloc_funcs::relgotofst(target, view, object, psymval,
                                                 r_addend, extract_addend,
                                                 local, this->calculate_only_,
                                                 &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_JALR:
        case elfcpp::R_MICROMIPS_JALR:
          // This relocation is only a hint.  In some cases, we optimize
          // it into a bal instruction.  But we don't try to optimize
          // when the symbol does not resolve locally.
          if (gsym == NULL
              || symbol_calls_local(gsym, gsym->has_dynsym_index()))
            reloc_status = Reloc_funcs::reljalr(view, object, psymval, address,
                                                r_addend, extract_addend,
                                                cross_mode_jump, r_types[i],
                                                target->jalr_to_bal(),
                                                target->jr_to_b(),
                                                this->calculate_only_,
                                                &this->calculated_value_);
          break;

        case elfcpp::R_MIPS_TLS_DTPREL_HI16:
        case elfcpp::R_MIPS16_TLS_DTPREL_HI16:
        case elfcpp::R_MICROMIPS_TLS_DTPREL_HI16:
          reloc_status = Reloc_funcs::tlsrelhi16(view, object, psymval,
                                                 elfcpp::DTP_OFFSET, r_addend,
                                                 extract_addend,
                                                 this->calculate_only_,
                                                 &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_TLS_DTPREL_LO16:
        case elfcpp::R_MIPS16_TLS_DTPREL_LO16:
        case elfcpp::R_MICROMIPS_TLS_DTPREL_LO16:
          reloc_status = Reloc_funcs::tlsrello16(view, object, psymval,
                                                 elfcpp::DTP_OFFSET, r_addend,
                                                 extract_addend,
                                                 this->calculate_only_,
                                                 &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_TLS_DTPREL32:
        case elfcpp::R_MIPS_TLS_DTPREL64:
          reloc_status = Reloc_funcs::tlsrel32(view, object, psymval,
                                               elfcpp::DTP_OFFSET, r_addend,
                                               extract_addend,
                                               this->calculate_only_,
                                               &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_TLS_TPREL_HI16:
        case elfcpp::R_MIPS16_TLS_TPREL_HI16:
        case elfcpp::R_MICROMIPS_TLS_TPREL_HI16:
          reloc_status = Reloc_funcs::tlsrelhi16(view, object, psymval,
                                                 elfcpp::TP_OFFSET, r_addend,
                                                 extract_addend,
                                                 this->calculate_only_,
                                                 &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_TLS_TPREL_LO16:
        case elfcpp::R_MIPS16_TLS_TPREL_LO16:
        case elfcpp::R_MICROMIPS_TLS_TPREL_LO16:
          reloc_status = Reloc_funcs::tlsrello16(view, object, psymval,
                                                 elfcpp::TP_OFFSET, r_addend,
                                                 extract_addend,
                                                 this->calculate_only_,
                                                 &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_TLS_TPREL32:
        case elfcpp::R_MIPS_TLS_TPREL64:
          reloc_status = Reloc_funcs::tlsrel32(view, object, psymval,
                                               elfcpp::TP_OFFSET, r_addend,
                                               extract_addend,
                                               this->calculate_only_,
                                               &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_SUB:
        case elfcpp::R_MICROMIPS_SUB:
          reloc_status = Reloc_funcs::relsub(view, object, psymval, r_addend,
                                             extract_addend,
                                             this->calculate_only_,
                                             &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_HIGHER:
        case elfcpp::R_MICROMIPS_HIGHER:
          reloc_status = Reloc_funcs::relhigher(view, object, psymval, r_addend,
                                                extract_addend,
                                                this->calculate_only_,
                                                &this->calculated_value_);
          break;
        case elfcpp::R_MIPS_HIGHEST:
        case elfcpp::R_MICROMIPS_HIGHEST:
          reloc_status = Reloc_funcs::relhighest(view, object, psymval,
                                                 r_addend, extract_addend,
                                                 this->calculate_only_,
                                                 &this->calculated_value_);
          break;
        default:
          gold_error_at_location(relinfo, relnum, r_offset,
                                 _("unsupported reloc %u"), r_types[i]);
          break;
        }

      if (update_got_entry)
        {
          Mips_output_data_got<size, big_endian>* got = target->got_section();
          if (mips_sym != NULL && mips_sym->get_applied_secondary_got_fixup())
            got->update_got_entry(got->get_primary_got_offset(mips_sym),
                                  psymval->value(object, 0));
          else
            got->update_got_entry(got_offset, psymval->value(object, 0));
        }
    }

  bool jal_shuffle = jal_reloc(r_type);
  Reloc_funcs::mips_reloc_shuffle(view, r_type, jal_shuffle);

  // Report any errors.
  switch (reloc_status)
    {
    case Reloc_funcs::STATUS_OKAY:
      break;
    case Reloc_funcs::STATUS_OVERFLOW:
      if (gsym == NULL)
        gold_error_at_location(relinfo, relnum, r_offset,
                               _("relocation overflow: "
                                 "%u against local symbol %u in %s"),
                               r_type, r_sym, object->name().c_str());
      else if (gsym->is_defined() && gsym->source() == Symbol::FROM_OBJECT)
        gold_error_at_location(relinfo, relnum, r_offset,
                               _("relocation overflow: "
                                 "%u against '%s' defined in %s"),
                               r_type, gsym->demangled_name().c_str(),
                               gsym->object()->name().c_str());
      else
        gold_error_at_location(relinfo, relnum, r_offset,
                               _("relocation overflow: %u against '%s'"),
                               r_type, gsym->demangled_name().c_str());
      break;
    case Reloc_funcs::STATUS_BAD_RELOC:
      gold_error_at_location(relinfo, relnum, r_offset,
        _("unexpected opcode while processing relocation"));
      break;
    case Reloc_funcs::STATUS_PCREL_UNALIGNED:
      gold_error_at_location(relinfo, relnum, r_offset,
        _("unaligned PC-relative relocation"));
      break;
    default:
      gold_unreachable();
    }

  return true;
}

// Get the Reference_flags for a particular relocation.

template<int size, bool big_endian>
int
Target_mips<size, big_endian>::Scan::get_reference_flags(
                       unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_MIPS_NONE:
      // No symbol reference.
      return 0;

    case elfcpp::R_MIPS_16:
    case elfcpp::R_MIPS_32:
    case elfcpp::R_MIPS_64:
    case elfcpp::R_MIPS_HI16:
    case elfcpp::R_MIPS_LO16:
    case elfcpp::R_MIPS_HIGHER:
    case elfcpp::R_MIPS_HIGHEST:
    case elfcpp::R_MIPS16_HI16:
    case elfcpp::R_MIPS16_LO16:
    case elfcpp::R_MICROMIPS_HI16:
    case elfcpp::R_MICROMIPS_LO16:
    case elfcpp::R_MICROMIPS_HIGHER:
    case elfcpp::R_MICROMIPS_HIGHEST:
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_MIPS_26:
    case elfcpp::R_MIPS16_26:
    case elfcpp::R_MICROMIPS_26_S1:
      return Symbol::FUNCTION_CALL | Symbol::ABSOLUTE_REF;

    case elfcpp::R_MIPS_PC18_S3:
    case elfcpp::R_MIPS_PC19_S2:
    case elfcpp::R_MIPS_PCHI16:
    case elfcpp::R_MIPS_PCLO16:
    case elfcpp::R_MIPS_GPREL32:
    case elfcpp::R_MIPS_GPREL16:
    case elfcpp::R_MIPS_REL32:
    case elfcpp::R_MIPS16_GPREL:
      return Symbol::RELATIVE_REF;

    case elfcpp::R_MIPS_PC16:
    case elfcpp::R_MIPS_PC32:
    case elfcpp::R_MIPS_PC21_S2:
    case elfcpp::R_MIPS_PC26_S2:
    case elfcpp::R_MIPS_JALR:
    case elfcpp::R_MICROMIPS_JALR:
      return Symbol::FUNCTION_CALL | Symbol::RELATIVE_REF;

    case elfcpp::R_MIPS_GOT16:
    case elfcpp::R_MIPS_CALL16:
    case elfcpp::R_MIPS_GOT_DISP:
    case elfcpp::R_MIPS_GOT_HI16:
    case elfcpp::R_MIPS_GOT_LO16:
    case elfcpp::R_MIPS_CALL_HI16:
    case elfcpp::R_MIPS_CALL_LO16:
    case elfcpp::R_MIPS_LITERAL:
    case elfcpp::R_MIPS_GOT_PAGE:
    case elfcpp::R_MIPS_GOT_OFST:
    case elfcpp::R_MIPS16_GOT16:
    case elfcpp::R_MIPS16_CALL16:
    case elfcpp::R_MICROMIPS_GOT16:
    case elfcpp::R_MICROMIPS_CALL16:
    case elfcpp::R_MICROMIPS_GOT_HI16:
    case elfcpp::R_MICROMIPS_GOT_LO16:
    case elfcpp::R_MICROMIPS_CALL_HI16:
    case elfcpp::R_MICROMIPS_CALL_LO16:
    case elfcpp::R_MIPS_EH:
      // Absolute in GOT.
      return Symbol::RELATIVE_REF;

    case elfcpp::R_MIPS_TLS_DTPMOD32:
    case elfcpp::R_MIPS_TLS_DTPREL32:
    case elfcpp::R_MIPS_TLS_DTPMOD64:
    case elfcpp::R_MIPS_TLS_DTPREL64:
    case elfcpp::R_MIPS_TLS_GD:
    case elfcpp::R_MIPS_TLS_LDM:
    case elfcpp::R_MIPS_TLS_DTPREL_HI16:
    case elfcpp::R_MIPS_TLS_DTPREL_LO16:
    case elfcpp::R_MIPS_TLS_GOTTPREL:
    case elfcpp::R_MIPS_TLS_TPREL32:
    case elfcpp::R_MIPS_TLS_TPREL64:
    case elfcpp::R_MIPS_TLS_TPREL_HI16:
    case elfcpp::R_MIPS_TLS_TPREL_LO16:
    case elfcpp::R_MIPS16_TLS_GD:
    case elfcpp::R_MIPS16_TLS_GOTTPREL:
    case elfcpp::R_MICROMIPS_TLS_GD:
    case elfcpp::R_MICROMIPS_TLS_GOTTPREL:
    case elfcpp::R_MICROMIPS_TLS_TPREL_HI16:
    case elfcpp::R_MICROMIPS_TLS_TPREL_LO16:
      return Symbol::TLS_REF;

    case elfcpp::R_MIPS_COPY:
    case elfcpp::R_MIPS_JUMP_SLOT:
    default:
      // Not expected.  We will give an error later.
      return 0;
    }
}

// Report an unsupported relocation against a local symbol.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::Scan::unsupported_reloc_local(
                        Sized_relobj_file<size, big_endian>* object,
                        unsigned int r_type)
{
  gold_error(_("%s: unsupported reloc %u against local symbol"),
             object->name().c_str(), r_type);
}

// Report an unsupported relocation against a global symbol.

template<int size, bool big_endian>
void
Target_mips<size, big_endian>::Scan::unsupported_reloc_global(
                        Sized_relobj_file<size, big_endian>* object,
                        unsigned int r_type,
                        Symbol* gsym)
{
  gold_error(_("%s: unsupported reloc %u against global symbol %s"),
             object->name().c_str(), r_type, gsym->demangled_name().c_str());
}

// Return printable name for ABI.
template<int size, bool big_endian>
const char*
Target_mips<size, big_endian>::elf_mips_abi_name(elfcpp::Elf_Word e_flags)
{
  switch (e_flags & elfcpp::EF_MIPS_ABI)
    {
    case 0:
      if ((e_flags & elfcpp::EF_MIPS_ABI2) != 0)
        return "N32";
      else if (size == 64)
        return "64";
      else
        return "none";
    case elfcpp::E_MIPS_ABI_O32:
      return "O32";
    case elfcpp::E_MIPS_ABI_O64:
      return "O64";
    case elfcpp::E_MIPS_ABI_EABI32:
      return "EABI32";
    case elfcpp::E_MIPS_ABI_EABI64:
      return "EABI64";
    default:
      return "unknown abi";
    }
}

template<int size, bool big_endian>
const char*
Target_mips<size, big_endian>::elf_mips_mach_name(elfcpp::Elf_Word e_flags)
{
  switch (e_flags & elfcpp::EF_MIPS_MACH)
    {
    case elfcpp::E_MIPS_MACH_3900:
      return "mips:3900";
    case elfcpp::E_MIPS_MACH_4010:
      return "mips:4010";
    case elfcpp::E_MIPS_MACH_4100:
      return "mips:4100";
    case elfcpp::E_MIPS_MACH_4111:
      return "mips:4111";
    case elfcpp::E_MIPS_MACH_4120:
      return "mips:4120";
    case elfcpp::E_MIPS_MACH_4650:
      return "mips:4650";
    case elfcpp::E_MIPS_MACH_5400:
      return "mips:5400";
    case elfcpp::E_MIPS_MACH_5500:
      return "mips:5500";
    case elfcpp::E_MIPS_MACH_5900:
      return "mips:5900";
    case elfcpp::E_MIPS_MACH_SB1:
      return "mips:sb1";
    case elfcpp::E_MIPS_MACH_9000:
      return "mips:9000";
    case elfcpp::E_MIPS_MACH_LS2E:
      return "mips:loongson_2e";
    case elfcpp::E_MIPS_MACH_LS2F:
      return "mips:loongson_2f";
    case elfcpp::E_MIPS_MACH_GS464:
      return "mips:gs464";
    case elfcpp::E_MIPS_MACH_GS464E:
      return "mips:gs464e";
    case elfcpp::E_MIPS_MACH_GS264E:
      return "mips:gs264e";
    case elfcpp::E_MIPS_MACH_OCTEON:
      return "mips:octeon";
    case elfcpp::E_MIPS_MACH_OCTEON2:
      return "mips:octeon2";
    case elfcpp::E_MIPS_MACH_OCTEON3:
      return "mips:octeon3";
    case elfcpp::E_MIPS_MACH_XLR:
      return "mips:xlr";
    default:
      switch (e_flags & elfcpp::EF_MIPS_ARCH)
        {
        default:
        case elfcpp::E_MIPS_ARCH_1:
          return "mips:3000";

        case elfcpp::E_MIPS_ARCH_2:
          return "mips:6000";

        case elfcpp::E_MIPS_ARCH_3:
          return "mips:4000";

        case elfcpp::E_MIPS_ARCH_4:
          return "mips:8000";

        case elfcpp::E_MIPS_ARCH_5:
          return "mips:mips5";

        case elfcpp::E_MIPS_ARCH_32:
          return "mips:isa32";

        case elfcpp::E_MIPS_ARCH_64:
          return "mips:isa64";

        case elfcpp::E_MIPS_ARCH_32R2:
          return "mips:isa32r2";

        case elfcpp::E_MIPS_ARCH_32R6:
          return "mips:isa32r6";

        case elfcpp::E_MIPS_ARCH_64R2:
          return "mips:isa64r2";

        case elfcpp::E_MIPS_ARCH_64R6:
          return "mips:isa64r6";
        }
    }
    return "unknown CPU";
}

template<int size, bool big_endian>
const Target::Target_info Target_mips<size, big_endian>::mips_info =
{
  size,                 // size
  big_endian,           // is_big_endian
  elfcpp::EM_MIPS,      // machine_code
  true,                 // has_make_symbol
  false,                // has_resolve
  false,                // has_code_fill
  true,                 // is_default_stack_executable
  false,                // can_icf_inline_merge_sections
  '\0',                 // wrap_char
  size == 32 ? "/lib/ld.so.1" : "/lib64/ld.so.1",      // dynamic_linker
  0x400000,             // default_text_segment_address
  64 * 1024,            // abi_pagesize (overridable by -z max-page-size)
  4 * 1024,             // common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,    // small_common_shndx
  elfcpp::SHN_UNDEF,    // large_common_shndx
  0,                    // small_common_section_flags
  0,                    // large_common_section_flags
  NULL,                 // attributes_section
  NULL,                 // attributes_vendor
  "__start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

template<int size, bool big_endian>
class Target_mips_nacl : public Target_mips<size, big_endian>
{
 public:
  Target_mips_nacl()
    : Target_mips<size, big_endian>(&mips_nacl_info)
  { }

 private:
  static const Target::Target_info mips_nacl_info;
};

template<int size, bool big_endian>
const Target::Target_info Target_mips_nacl<size, big_endian>::mips_nacl_info =
{
  size,                 // size
  big_endian,           // is_big_endian
  elfcpp::EM_MIPS,      // machine_code
  true,                 // has_make_symbol
  false,                // has_resolve
  false,                // has_code_fill
  true,                 // is_default_stack_executable
  false,                // can_icf_inline_merge_sections
  '\0',                 // wrap_char
  "/lib/ld.so.1",       // dynamic_linker
  0x20000,              // default_text_segment_address
  0x10000,              // abi_pagesize (overridable by -z max-page-size)
  0x10000,              // common_pagesize (overridable by -z common-page-size)
  true,                 // isolate_execinstr
  0x10000000,           // rosegment_gap
  elfcpp::SHN_UNDEF,    // small_common_shndx
  elfcpp::SHN_UNDEF,    // large_common_shndx
  0,                    // small_common_section_flags
  0,                    // large_common_section_flags
  NULL,                 // attributes_section
  NULL,                 // attributes_vendor
  "_start",             // entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

// Target selector for Mips.  Note this is never instantiated directly.
// It's only used in Target_selector_mips_nacl, below.

template<int size, bool big_endian>
class Target_selector_mips : public Target_selector
{
public:
  Target_selector_mips()
    : Target_selector(elfcpp::EM_MIPS, size, big_endian,
                (size == 64 ?
                  (big_endian ? "elf64-tradbigmips" : "elf64-tradlittlemips") :
                  (big_endian ? "elf32-tradbigmips" : "elf32-tradlittlemips")),
                (size == 64 ?
                  (big_endian ? "elf64btsmip" : "elf64ltsmip") :
                  (big_endian ? "elf32btsmip" : "elf32ltsmip")))
  { }

  Target* do_instantiate_target()
  { return new Target_mips<size, big_endian>(); }
};

template<int size, bool big_endian>
class Target_selector_mips_nacl
  : public Target_selector_nacl<Target_selector_mips<size, big_endian>,
                                Target_mips_nacl<size, big_endian> >
{
 public:
  Target_selector_mips_nacl()
    : Target_selector_nacl<Target_selector_mips<size, big_endian>,
                           Target_mips_nacl<size, big_endian> >(
        // NaCl currently supports only MIPS32 little-endian.
        "mipsel", "elf32-tradlittlemips-nacl", "elf32-tradlittlemips-nacl")
  { }
};

Target_selector_mips_nacl<32, true> target_selector_mips32;
Target_selector_mips_nacl<32, false> target_selector_mips32el;
Target_selector_mips_nacl<64, true> target_selector_mips64;
Target_selector_mips_nacl<64, false> target_selector_mips64el;

} // End anonymous namespace.
