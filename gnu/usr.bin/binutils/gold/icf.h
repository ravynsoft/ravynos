// icf.h --  Identical Code Folding

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Sriraman Tallam <tmsriram@google.com>.

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

#ifndef GOLD_ICF_H
#define GOLD_ICF_H

#include <vector>

#include "elfcpp.h"
#include "symtab.h"
#include "object.h"

namespace gold
{

class Object;
class Input_objects;
class Symbol_table;

class Icf
{
 public:
  typedef std::vector<Section_id> Sections_reachable_info;
  typedef std::vector<Symbol*> Symbol_info;
  typedef std::vector<std::pair<long long, long long> > Addend_info;
  typedef std::vector<uint64_t> Offset_info;
  typedef std::vector<unsigned int> Reloc_addend_size_info;
  typedef Unordered_map<Section_id,
                        unsigned int,
                        Section_id_hash> Uniq_secn_id_map;
  typedef Unordered_set<Section_id, Section_id_hash> Secn_fptr_taken_set;

  typedef struct
  {
    // This stores the section corresponding to the reloc.
    Sections_reachable_info section_info;
    // This stores the symbol corresponding to the reloc.
    Symbol_info symbol_info;
    // This stores the symbol value and the addend for a reloc.
    Addend_info addend_info;
    Offset_info offset_info;
    Reloc_addend_size_info reloc_addend_size_info;
  } Reloc_info;

  typedef Unordered_map<Section_id, Reloc_info,
                        Section_id_hash> Reloc_info_list;

  // A region of some other section that should be considered part of
  // a section for ICF purposes. This is used to avoid folding sections
  // that have identical text and relocations but different .eh_frame
  // information.
  typedef struct
  {
	Section_id section;
	section_offset_type offset;
	section_size_type length;
  } Extra_identity_info;

  typedef std::multimap<Section_id, Extra_identity_info> Extra_identity_list;

  Icf()
  : id_section_(), section_id_(), kept_section_id_(),
    fptr_section_id_(),
    icf_ready_(false),
    reloc_info_list_()
  { }

  // Returns the kept folded identical section corresponding to
  // dup_obj and dup_shndx.
  Section_id
  get_folded_section(Relobj* dup_obj, unsigned int dup_shndx);

  // Forms groups of identical sections where the first member
  // of each group is the kept section during folding.
  void
  find_identical_sections(const Input_objects* input_objects,
                          Symbol_table* symtab);

  // This is set when ICF has been run and the groups of
  // identical sections have been formed.
  void
  icf_ready()
  { this->icf_ready_ = true; }

  // Returns true if ICF has been run.
  bool
  is_icf_ready()
  { return this->icf_ready_; }

  // Unfolds the section denoted by OBJ and SHNDX if folded.
  void
  unfold_section(Relobj* obj, unsigned int shndx);

  // Returns the kept section corresponding to the
  // given section.
  bool
  is_section_folded(Relobj* obj, unsigned int shndx);

  // Given an object and a section index, this returns true if the
  // pointer of the function defined in this section is taken.
  bool
  section_has_function_pointers(Relobj* obj, unsigned int shndx)
  {
    return (this->fptr_section_id_.find(Section_id(obj, shndx))
            != this->fptr_section_id_.end());
  }

  // Records that a pointer of the function defined in this section
  // is taken.
  void
  set_section_has_function_pointers(Relobj* obj, unsigned int shndx)
  {
    this->fptr_section_id_.insert(Section_id(obj, shndx));
  }

  // Checks if the section_name should be searched for relocs
  // corresponding to taken function pointers.  Ignores eh_frame
  // and vtable sections.
  inline bool
  check_section_for_function_pointers(const std::string& section_name,
                                      Target* target)
  {
    return (parameters->options().icf_safe_folding()
	    && target->can_check_for_function_pointers()
	    && target->section_may_have_icf_unsafe_pointers(
	        section_name.c_str()));
  }

  // Returns a map of a section to info (Reloc_info) about its relocations.
  Reloc_info_list&
  reloc_info_list()
  { return this->reloc_info_list_; }

  // Returns a map from section to region of a different section that should
  // be considered part of the key section for ICF purposes.
  Extra_identity_list&
  extra_identity_list()
  { return this->extra_identity_list_; }

  // Returns a mapping of each section to a unique integer.
  Uniq_secn_id_map&
  section_to_int_map()
  { return this->section_id_; }

 private:

  bool
  add_ehframe_links(Relobj* object, unsigned int ehframe_shndx,
		    Reloc_info& ehframe_relocs);

  // Maps integers to sections.
  std::vector<Section_id> id_section_;
  // Does the reverse.
  Uniq_secn_id_map section_id_;
  // Given a section id, this maps it to the id of the kept
  // section.  If the id's are the same then this section is
  // not folded.
  std::vector<unsigned int> kept_section_id_;
  // Given a section id, this says if the pointer to this
  // function is taken in which case it is dangerous to fold
  // this function.
  Secn_fptr_taken_set fptr_section_id_;
  // Flag to indicate if ICF has been run.
  bool icf_ready_;
  // This list is populated by gc_process_relocs in gc.h.
  Reloc_info_list reloc_info_list_;
  // Regions of other sections that should be considered part of
  // each section for ICF purposes.
  Extra_identity_list extra_identity_list_;
};

// This function returns true if this section corresponds to a function that
// should be considered by icf as a possible candidate for folding.  Some
// earlier gcc versions, like 4.0.3, put constructors and destructors in
// .gnu.linkonce.t sections and hence should be included too.
// The mechanism used to safely fold functions referenced by .eh_frame
// requires folding .gcc_except_table sections as well; see "Notes regarding
// C++ exception handling" at the top of icf.cc for an explanation why.
inline bool
is_section_foldable_candidate(const std::string& section_name)
{
  const char* section_name_cstr = section_name.c_str();
  return (is_prefix_of(".text", section_name_cstr)
          || is_prefix_of(".gcc_except_table", section_name_cstr)
          || is_prefix_of(".gnu.linkonce.t", section_name_cstr));
}

} // End of namespace gold.

#endif
