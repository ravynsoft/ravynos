// gc.h -- garbage collection of unused sections

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

#ifndef GOLD_GC_H
#define GOLD_GC_H

#include <vector>

#include "elfcpp.h"
#include "symtab.h"
#include "object.h"
#include "icf.h"

namespace gold
{

class Object;

template<int size, bool big_endian>
class Sized_relobj_file;

class Output_section;
class General_options;
class Layout;

class Garbage_collection
{
 public:

  typedef Unordered_set<Section_id, Section_id_hash> Sections_reachable;
  typedef std::map<Section_id, Sections_reachable> Section_ref;
  typedef std::vector<Section_id> Worklist_type;
  // This maps the name of the section which can be represented as a C
  // identifier (cident) to the list of sections that have that name.
  // Different object files can have cident sections with the same name.
  typedef std::map<std::string, Sections_reachable> Cident_section_map;

  Garbage_collection()
  : is_worklist_ready_(false)
  { }

  // Accessor methods for the private members.

  Sections_reachable&
  referenced_list()
  { return referenced_list_; }

  Section_ref&
  section_reloc_map()
  { return this->section_reloc_map_; }

  Worklist_type&
  worklist()
  { return this->work_list_; }

  bool
  is_worklist_ready()
  { return this->is_worklist_ready_; }

  void
  worklist_ready()
  { this->is_worklist_ready_ = true; }

  void
  do_transitive_closure();

  bool
  is_section_garbage(Relobj* obj, unsigned int shndx)
  { return (this->referenced_list().find(Section_id(obj, shndx))
            == this->referenced_list().end()); }

  Cident_section_map*
  cident_sections()
  { return &cident_sections_; }

  void
  add_cident_section(std::string section_name,
		     Section_id secn)
  { this->cident_sections_[section_name].insert(secn); }

  // Add a reference from the SRC_SHNDX-th section of SRC_OBJECT to
  // DST_SHNDX-th section of DST_OBJECT.
  void
  add_reference(Relobj* src_object, unsigned int src_shndx,
		Relobj* dst_object, unsigned int dst_shndx)
  {
    Section_id src_id(src_object, src_shndx);
    Section_id dst_id(dst_object, dst_shndx);
    Sections_reachable& reachable = this->section_reloc_map_[src_id];
    reachable.insert(dst_id);
  }

 private:

  Worklist_type work_list_;
  bool is_worklist_ready_;
  Section_ref section_reloc_map_;
  Sections_reachable referenced_list_;
  Cident_section_map cident_sections_;
};

// Data to pass between successive invocations of do_layout
// in object.cc while garbage collecting.  This data structure
// is filled by using the data from Read_symbols_data.

struct Symbols_data
{
  // Section headers.
  unsigned char* section_headers_data;
  // Section names.
  unsigned char* section_names_data;
  // Size of section name data in bytes.
  section_size_type section_names_size;
  // Symbol data.
  unsigned char* symbols_data;
  // Size of symbol data in bytes.
  section_size_type symbols_size;
  // Offset of external symbols within symbol data.  This structure
  // sometimes contains only external symbols, in which case this will
  // be zero.  Sometimes it contains all symbols.
  section_offset_type external_symbols_offset;
  // Symbol names.
  unsigned char* symbol_names_data;
  // Size of symbol name data in bytes.
  section_size_type symbol_names_size;
};

// Relocations of type SHT_REL store the addend value in their bytes.
// This function returns the size of the embedded addend which is
// nothing but the size of the relocation.

template<typename Classify_reloc>
inline unsigned int
get_embedded_addend_size(int r_type, Relobj* obj)
{
  if (Classify_reloc::sh_type == elfcpp::SHT_REL)
    return Classify_reloc::get_size_for_reloc(r_type, obj);
  return 0;
}

// This function implements the generic part of reloc
// processing to map a section to all the sections it
// references through relocs.  It is called only during
// garbage collection (--gc-sections) and identical code
// folding (--icf).

template<int size, bool big_endian, typename Target_type,
	 typename Scan, typename Classify_reloc>
inline void
gc_process_relocs(
    Symbol_table* symtab,
    Layout*,
    Target_type* target,
    Sized_relobj_file<size, big_endian>* src_obj,
    unsigned int src_indx,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section*,
    bool,
    size_t local_count,
    const unsigned char* plocal_syms)
{
  Scan scan;

  typedef typename Classify_reloc::Reltype Reltype;
  const int reloc_size = Classify_reloc::reloc_size;
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  Icf::Sections_reachable_info* secvec = NULL;
  Icf::Symbol_info* symvec = NULL;
  Icf::Addend_info* addendvec = NULL;
  Icf::Offset_info* offsetvec = NULL;
  Icf::Reloc_addend_size_info* reloc_addend_size_vec = NULL;
  bool is_icf_tracked = false;
  const char* cident_section_name = NULL;

  std::string src_section_name = (parameters->options().icf_enabled()
                                  ? src_obj->section_name(src_indx)
                                  : "");

  bool check_section_for_function_pointers = false;

  if (parameters->options().icf_enabled()
      && (is_section_foldable_candidate(src_section_name)
          || is_prefix_of(".eh_frame", src_section_name.c_str())))
    {
      is_icf_tracked = true;
      Section_id src_id(src_obj, src_indx);
      Icf::Reloc_info* reloc_info =
        &symtab->icf()->reloc_info_list()[src_id];
      secvec = &reloc_info->section_info;
      symvec = &reloc_info->symbol_info;
      addendvec = &reloc_info->addend_info;
      offsetvec = &reloc_info->offset_info;
      reloc_addend_size_vec = &reloc_info->reloc_addend_size_info;
    }

  check_section_for_function_pointers =
    symtab->icf()->check_section_for_function_pointers(src_section_name,
                                                       target);

  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Reltype reloc(prelocs);
      unsigned int r_sym = Classify_reloc::get_r_sym(&reloc);
      unsigned int r_type = Classify_reloc::get_r_type(&reloc);
      typename elfcpp::Elf_types<size>::Elf_Swxword addend =
	  Classify_reloc::get_r_addend(&reloc);
      Relobj* dst_obj;
      unsigned int dst_indx;
      typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
      Address dst_off;

      if (r_sym < local_count)
        {
          gold_assert(plocal_syms != NULL);
          typename elfcpp::Sym<size, big_endian> lsym(plocal_syms
                                                      + r_sym * sym_size);
	  dst_indx = lsym.get_st_shndx();
          bool is_ordinary;
	  dst_indx = src_obj->adjust_sym_shndx(r_sym, dst_indx, &is_ordinary);
          dst_obj = src_obj;
	  dst_off = lsym.get_st_value() + addend;

          if (is_icf_tracked)
            {
	      Address symvalue = dst_off - addend;
	      if (is_ordinary) 
		(*secvec).push_back(Section_id(src_obj, dst_indx));
	      else
		(*secvec).push_back(Section_id(static_cast<Relobj*>(NULL), 0));
              // If the target of the relocation is an STT_SECTION symbol,
              // make a note of that by storing -1 in the symbol vector.
              if (lsym.get_st_type() == elfcpp::STT_SECTION)
		(*symvec).push_back(reinterpret_cast<Symbol*>(-1));
	      else
		(*symvec).push_back(NULL);
	      (*addendvec).push_back(std::make_pair(
					static_cast<long long>(symvalue),
					static_cast<long long>(addend)));
              uint64_t reloc_offset =
                convert_to_section_size_type(reloc.get_r_offset());
	      (*offsetvec).push_back(reloc_offset);
              (*reloc_addend_size_vec).push_back(
                get_embedded_addend_size<Classify_reloc>(r_type, src_obj));
            }

	  // When doing safe folding, check to see if this relocation is that
	  // of a function pointer being taken.
	  if (is_ordinary
	      && check_section_for_function_pointers
              && lsym.get_st_type() != elfcpp::STT_OBJECT
 	      && scan.local_reloc_may_be_function_pointer(symtab, NULL, target,
							  src_obj, src_indx,
			                       		  NULL, reloc, r_type,
							  lsym))
            symtab->icf()->set_section_has_function_pointers(
              src_obj, lsym.get_st_shndx());

          if (!is_ordinary || dst_indx == src_indx)
            continue;
        }
      else
        {
          Symbol* gsym = src_obj->global_symbol(r_sym);
          gold_assert(gsym != NULL);
          if (gsym->is_forwarder())
            gsym = symtab->resolve_forwards(gsym);

          dst_obj = NULL;
          dst_indx = 0;
          bool is_ordinary = false;
          if (gsym->source() == Symbol::FROM_OBJECT
	      && !gsym->object()->is_dynamic())
            {
              dst_obj = static_cast<Relobj*>(gsym->object());
              dst_indx = gsym->shndx(&is_ordinary);
            }
	  dst_off = static_cast<const Sized_symbol<size>*>(gsym)->value();
	  dst_off += addend;

	  // When doing safe folding, check to see if this relocation is that
	  // of a function pointer being taken.
	  if (gsym->source() == Symbol::FROM_OBJECT
              && gsym->type() == elfcpp::STT_FUNC
              && check_section_for_function_pointers
              && dst_obj != NULL
              && (!is_ordinary
                  || scan.global_reloc_may_be_function_pointer(
                       symtab, NULL, target, src_obj, src_indx, NULL, reloc,
                       r_type, gsym)))
            symtab->icf()->set_section_has_function_pointers(dst_obj, dst_indx);

          // If the symbol name matches '__start_XXX' then the section with
          // the C identifier like name 'XXX' should not be garbage collected.
          // A similar treatment to symbols with the name '__stop_XXX'.
          if (is_prefix_of(cident_section_start_prefix, gsym->name()))
            {
              cident_section_name = (gsym->name() 
                                     + strlen(cident_section_start_prefix));
            }
          else if (is_prefix_of(cident_section_stop_prefix, gsym->name()))
            {
              cident_section_name = (gsym->name() 
                                     + strlen(cident_section_stop_prefix));
            }
          if (is_icf_tracked)
            {
	      Address symvalue = dst_off - addend;
              if (is_ordinary && dst_obj != NULL)
		(*secvec).push_back(Section_id(dst_obj, dst_indx));
	      else
		(*secvec).push_back(Section_id(static_cast<Relobj*>(NULL), 0));
              (*symvec).push_back(gsym);
	      (*addendvec).push_back(std::make_pair(
					static_cast<long long>(symvalue),
					static_cast<long long>(addend)));
              uint64_t reloc_offset =
                convert_to_section_size_type(reloc.get_r_offset());
	      (*offsetvec).push_back(reloc_offset);
              (*reloc_addend_size_vec).push_back(
                get_embedded_addend_size<Classify_reloc>(r_type, src_obj));
	    }

          if (dst_obj == NULL)
            continue;
          if (!is_ordinary)
            continue;
        }
      if (parameters->options().gc_sections())
        {
	  symtab->gc()->add_reference(src_obj, src_indx, dst_obj, dst_indx);
	  parameters->sized_target<size, big_endian>()
	    ->gc_add_reference(symtab, src_obj, src_indx, dst_obj, dst_indx,
			       dst_off);
          if (cident_section_name != NULL)
            {
              Garbage_collection::Cident_section_map::iterator ele =
                symtab->gc()->cident_sections()->find(std::string(cident_section_name));
              if (ele == symtab->gc()->cident_sections()->end())
                continue;
	      Section_id src_id(src_obj, src_indx);
              Garbage_collection::Sections_reachable&
                v(symtab->gc()->section_reloc_map()[src_id]);
              Garbage_collection::Sections_reachable& cident_secn(ele->second);
              for (Garbage_collection::Sections_reachable::iterator it_v
                     = cident_secn.begin();
                   it_v != cident_secn.end();
                   ++it_v)
                {
                  v.insert(*it_v);
                }
            }
        }
    }
  return;
}

} // End of namespace gold.

#endif
