// symtab.cc -- the gold symbol table

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
#include <stdint.h>
#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include "demangle.h"

#include "gc.h"
#include "object.h"
#include "dwarf_reader.h"
#include "dynobj.h"
#include "output.h"
#include "target.h"
#include "workqueue.h"
#include "symtab.h"
#include "script.h"
#include "plugin.h"
#include "incremental.h"

namespace gold
{

// Class Symbol.

// Initialize fields in Symbol.  This initializes everything except
// u1_, u2_ and source_.

void
Symbol::init_fields(const char* name, const char* version,
		    elfcpp::STT type, elfcpp::STB binding,
		    elfcpp::STV visibility, unsigned char nonvis)
{
  this->name_ = name;
  this->version_ = version;
  this->symtab_index_ = 0;
  this->dynsym_index_ = 0;
  this->got_offsets_.init();
  this->plt_offset_ = -1U;
  this->type_ = type;
  this->binding_ = binding;
  this->visibility_ = visibility;
  this->nonvis_ = nonvis;
  this->is_def_ = false;
  this->is_forwarder_ = false;
  this->has_alias_ = false;
  this->needs_dynsym_entry_ = false;
  this->in_reg_ = false;
  this->in_dyn_ = false;
  this->has_warning_ = false;
  this->is_copied_from_dynobj_ = false;
  this->is_forced_local_ = false;
  this->is_ordinary_shndx_ = false;
  this->in_real_elf_ = false;
  this->is_defined_in_discarded_section_ = false;
  this->undef_binding_set_ = false;
  this->undef_binding_weak_ = false;
  this->is_predefined_ = false;
  this->is_protected_ = false;
  this->non_zero_localentry_ = false;
}

// Return the demangled version of the symbol's name, but only
// if the --demangle flag was set.

static std::string
demangle(const char* name)
{
  if (!parameters->options().do_demangle())
    return name;

  // cplus_demangle allocates memory for the result it returns,
  // and returns NULL if the name is already demangled.
  char* demangled_name = cplus_demangle(name, DMGL_ANSI | DMGL_PARAMS);
  if (demangled_name == NULL)
    return name;

  std::string retval(demangled_name);
  free(demangled_name);
  return retval;
}

std::string
Symbol::demangled_name() const
{
  return demangle(this->name());
}

// Initialize the fields in the base class Symbol for SYM in OBJECT.

template<int size, bool big_endian>
void
Symbol::init_base_object(const char* name, const char* version, Object* object,
			 const elfcpp::Sym<size, big_endian>& sym,
			 unsigned int st_shndx, bool is_ordinary)
{
  this->init_fields(name, version, sym.get_st_type(), sym.get_st_bind(),
		    sym.get_st_visibility(), sym.get_st_nonvis());
  this->u1_.object = object;
  this->u2_.shndx = st_shndx;
  this->is_ordinary_shndx_ = is_ordinary;
  this->source_ = FROM_OBJECT;
  this->in_reg_ = !object->is_dynamic();
  this->in_dyn_ = object->is_dynamic();
  this->in_real_elf_ = object->pluginobj() == NULL;
}

// Initialize the fields in the base class Symbol for a symbol defined
// in an Output_data.

void
Symbol::init_base_output_data(const char* name, const char* version,
			      Output_data* od, elfcpp::STT type,
			      elfcpp::STB binding, elfcpp::STV visibility,
			      unsigned char nonvis, bool offset_is_from_end,
			      bool is_predefined)
{
  this->init_fields(name, version, type, binding, visibility, nonvis);
  this->u1_.output_data = od;
  this->u2_.offset_is_from_end = offset_is_from_end;
  this->source_ = IN_OUTPUT_DATA;
  this->in_reg_ = true;
  this->in_real_elf_ = true;
  this->is_predefined_ = is_predefined;
}

// Initialize the fields in the base class Symbol for a symbol defined
// in an Output_segment.

void
Symbol::init_base_output_segment(const char* name, const char* version,
				 Output_segment* os, elfcpp::STT type,
				 elfcpp::STB binding, elfcpp::STV visibility,
				 unsigned char nonvis,
				 Segment_offset_base offset_base,
				 bool is_predefined)
{
  this->init_fields(name, version, type, binding, visibility, nonvis);
  this->u1_.output_segment = os;
  this->u2_.offset_base = offset_base;
  this->source_ = IN_OUTPUT_SEGMENT;
  this->in_reg_ = true;
  this->in_real_elf_ = true;
  this->is_predefined_ = is_predefined;
}

// Initialize the fields in the base class Symbol for a symbol defined
// as a constant.

void
Symbol::init_base_constant(const char* name, const char* version,
			   elfcpp::STT type, elfcpp::STB binding,
			   elfcpp::STV visibility, unsigned char nonvis,
			   bool is_predefined)
{
  this->init_fields(name, version, type, binding, visibility, nonvis);
  this->source_ = IS_CONSTANT;
  this->in_reg_ = true;
  this->in_real_elf_ = true;
  this->is_predefined_ = is_predefined;
}

// Initialize the fields in the base class Symbol for an undefined
// symbol.

void
Symbol::init_base_undefined(const char* name, const char* version,
			    elfcpp::STT type, elfcpp::STB binding,
			    elfcpp::STV visibility, unsigned char nonvis)
{
  this->init_fields(name, version, type, binding, visibility, nonvis);
  this->dynsym_index_ = -1U;
  this->source_ = IS_UNDEFINED;
  this->in_reg_ = true;
  this->in_real_elf_ = true;
}

// Allocate a common symbol in the base.

void
Symbol::allocate_base_common(Output_data* od)
{
  gold_assert(this->is_common());
  this->source_ = IN_OUTPUT_DATA;
  this->u1_.output_data = od;
  this->u2_.offset_is_from_end = false;
}

// Initialize the fields in Sized_symbol for SYM in OBJECT.

template<int size>
template<bool big_endian>
void
Sized_symbol<size>::init_object(const char* name, const char* version,
				Object* object,
				const elfcpp::Sym<size, big_endian>& sym,
				unsigned int st_shndx, bool is_ordinary)
{
  this->init_base_object(name, version, object, sym, st_shndx, is_ordinary);
  this->value_ = sym.get_st_value();
  this->symsize_ = sym.get_st_size();
}

// Initialize the fields in Sized_symbol for a symbol defined in an
// Output_data.

template<int size>
void
Sized_symbol<size>::init_output_data(const char* name, const char* version,
				     Output_data* od, Value_type value,
				     Size_type symsize, elfcpp::STT type,
				     elfcpp::STB binding,
				     elfcpp::STV visibility,
				     unsigned char nonvis,
				     bool offset_is_from_end,
				     bool is_predefined)
{
  this->init_base_output_data(name, version, od, type, binding, visibility,
			      nonvis, offset_is_from_end, is_predefined);
  this->value_ = value;
  this->symsize_ = symsize;
}

// Initialize the fields in Sized_symbol for a symbol defined in an
// Output_segment.

template<int size>
void
Sized_symbol<size>::init_output_segment(const char* name, const char* version,
					Output_segment* os, Value_type value,
					Size_type symsize, elfcpp::STT type,
					elfcpp::STB binding,
					elfcpp::STV visibility,
					unsigned char nonvis,
					Segment_offset_base offset_base,
					bool is_predefined)
{
  this->init_base_output_segment(name, version, os, type, binding, visibility,
				 nonvis, offset_base, is_predefined);
  this->value_ = value;
  this->symsize_ = symsize;
}

// Initialize the fields in Sized_symbol for a symbol defined as a
// constant.

template<int size>
void
Sized_symbol<size>::init_constant(const char* name, const char* version,
				  Value_type value, Size_type symsize,
				  elfcpp::STT type, elfcpp::STB binding,
				  elfcpp::STV visibility, unsigned char nonvis,
				  bool is_predefined)
{
  this->init_base_constant(name, version, type, binding, visibility, nonvis,
			   is_predefined);
  this->value_ = value;
  this->symsize_ = symsize;
}

// Initialize the fields in Sized_symbol for an undefined symbol.

template<int size>
void
Sized_symbol<size>::init_undefined(const char* name, const char* version,
				   Value_type value, elfcpp::STT type,
				   elfcpp::STB binding, elfcpp::STV visibility,
				   unsigned char nonvis)
{
  this->init_base_undefined(name, version, type, binding, visibility, nonvis);
  this->value_ = value;
  this->symsize_ = 0;
}

// Return an allocated string holding the symbol's name as
// name@version.  This is used for relocatable links.

std::string
Symbol::versioned_name() const
{
  gold_assert(this->version_ != NULL);
  std::string ret = this->name_;
  ret.push_back('@');
  if (this->is_def_)
    ret.push_back('@');
  ret += this->version_;
  return ret;
}

// Return true if SHNDX represents a common symbol.

bool
Symbol::is_common_shndx(unsigned int shndx)
{
  return (shndx == elfcpp::SHN_COMMON
	  || shndx == parameters->target().small_common_shndx()
	  || shndx == parameters->target().large_common_shndx());
}

// Allocate a common symbol.

template<int size>
void
Sized_symbol<size>::allocate_common(Output_data* od, Value_type value)
{
  this->allocate_base_common(od);
  this->value_ = value;
}

// The ""'s around str ensure str is a string literal, so sizeof works.
#define strprefix(var, str)   (strncmp(var, str, sizeof("" str "") - 1) == 0)

// Return true if this symbol should be added to the dynamic symbol
// table.

bool
Symbol::should_add_dynsym_entry(Symbol_table* symtab) const
{
  // If the symbol is only present on plugin files, the plugin decided we
  // don't need it.
  if (!this->in_real_elf())
    return false;

  // If the symbol is used by a dynamic relocation, we need to add it.
  if (this->needs_dynsym_entry())
    return true;

  // If this symbol's section is not added, the symbol need not be added. 
  // The section may have been GCed.  Note that export_dynamic is being 
  // overridden here.  This should not be done for shared objects.
  if (parameters->options().gc_sections() 
      && !parameters->options().shared()
      && this->source() == Symbol::FROM_OBJECT
      && !this->object()->is_dynamic())
    {
      Relobj* relobj = static_cast<Relobj*>(this->object());
      bool is_ordinary;
      unsigned int shndx = this->shndx(&is_ordinary);
      if (is_ordinary && shndx != elfcpp::SHN_UNDEF
          && !relobj->is_section_included(shndx)
          && !symtab->is_section_folded(relobj, shndx))
        return false;
    }

  // If the symbol was forced dynamic in a --dynamic-list file
  // or an --export-dynamic-symbol option, add it.
  if (!this->is_from_dynobj()
      && (parameters->options().in_dynamic_list(this->name())
	  || parameters->options().is_export_dynamic_symbol(this->name())))
    {
      if (!this->is_forced_local())
        return true;
      gold_warning(_("Cannot export local symbol '%s'"),
		   this->demangled_name().c_str());
      return false;
    }

  // If the symbol was forced local in a version script, do not add it.
  if (this->is_forced_local())
    return false;

  // If dynamic-list-data was specified, add any STT_OBJECT.
  if (parameters->options().dynamic_list_data()
      && !this->is_from_dynobj()
      && this->type() == elfcpp::STT_OBJECT)
    return true;

  // If --dynamic-list-cpp-new was specified, add any new/delete symbol.
  // If --dynamic-list-cpp-typeinfo was specified, add any typeinfo symbols.
  if ((parameters->options().dynamic_list_cpp_new()
       || parameters->options().dynamic_list_cpp_typeinfo())
      && !this->is_from_dynobj())
    {
      // TODO(csilvers): We could probably figure out if we're an operator
      //                 new/delete or typeinfo without the need to demangle.
      char* demangled_name = cplus_demangle(this->name(),
                                            DMGL_ANSI | DMGL_PARAMS);
      if (demangled_name == NULL)
        {
          // Not a C++ symbol, so it can't satisfy these flags
        }
      else if (parameters->options().dynamic_list_cpp_new()
               && (strprefix(demangled_name, "operator new")
                   || strprefix(demangled_name, "operator delete")))
        {
          free(demangled_name);
          return true;
        }
      else if (parameters->options().dynamic_list_cpp_typeinfo()
               && (strprefix(demangled_name, "typeinfo name for")
                   || strprefix(demangled_name, "typeinfo for")))
        {
          free(demangled_name);
          return true;
        }
      else
        free(demangled_name);
    }

  // If exporting all symbols or building a shared library,
  // or the symbol should be globally unique (GNU_UNIQUE),
  // and the symbol is defined in a regular object and is
  // externally visible, we need to add it.
  if ((parameters->options().export_dynamic()
       || parameters->options().shared()
       || (parameters->options().gnu_unique()
           && this->binding() == elfcpp::STB_GNU_UNIQUE))
      && !this->is_from_dynobj()
      && !this->is_undefined()
      && this->is_externally_visible())
    return true;

  return false;
}

// Return true if the final value of this symbol is known at link
// time.

bool
Symbol::final_value_is_known() const
{
  // If we are not generating an executable, then no final values are
  // known, since they will change at runtime, with the exception of
  // TLS symbols in a position-independent executable.
  if ((parameters->options().output_is_position_independent()
       || parameters->options().relocatable())
      && !(this->type() == elfcpp::STT_TLS
           && parameters->options().pie()))
    return false;

  // If the symbol is not from an object file, and is not undefined,
  // then it is defined, and known.
  if (this->source_ != FROM_OBJECT)
    {
      if (this->source_ != IS_UNDEFINED)
	return true;
    }
  else
    {
      // If the symbol is from a dynamic object, then the final value
      // is not known.
      if (this->object()->is_dynamic())
	return false;

      // If the symbol is not undefined (it is defined or common),
      // then the final value is known.
      if (!this->is_undefined())
	return true;
    }

  // If the symbol is undefined, then whether the final value is known
  // depends on whether we are doing a static link.  If we are doing a
  // dynamic link, then the final value could be filled in at runtime.
  // This could reasonably be the case for a weak undefined symbol.
  return parameters->doing_static_link();
}

// Return the output section where this symbol is defined.

Output_section*
Symbol::output_section() const
{
  switch (this->source_)
    {
    case FROM_OBJECT:
      {
	unsigned int shndx = this->u2_.shndx;
	if (shndx != elfcpp::SHN_UNDEF && this->is_ordinary_shndx_)
	  {
	    gold_assert(!this->u1_.object->is_dynamic());
	    gold_assert(this->u1_.object->pluginobj() == NULL);
	    Relobj* relobj = static_cast<Relobj*>(this->u1_.object);
	    return relobj->output_section(shndx);
	  }
	return NULL;
      }

    case IN_OUTPUT_DATA:
      return this->u1_.output_data->output_section();

    case IN_OUTPUT_SEGMENT:
    case IS_CONSTANT:
    case IS_UNDEFINED:
      return NULL;

    default:
      gold_unreachable();
    }
}

// Set the symbol's output section.  This is used for symbols defined
// in scripts.  This should only be called after the symbol table has
// been finalized.

void
Symbol::set_output_section(Output_section* os)
{
  switch (this->source_)
    {
    case FROM_OBJECT:
    case IN_OUTPUT_DATA:
      gold_assert(this->output_section() == os);
      break;
    case IS_CONSTANT:
      this->source_ = IN_OUTPUT_DATA;
      this->u1_.output_data = os;
      this->u2_.offset_is_from_end = false;
      break;
    case IN_OUTPUT_SEGMENT:
    case IS_UNDEFINED:
    default:
      gold_unreachable();
    }
}

// Set the symbol's output segment.  This is used for pre-defined
// symbols whose segments aren't known until after layout is done
// (e.g., __ehdr_start).

void
Symbol::set_output_segment(Output_segment* os, Segment_offset_base base)
{
  gold_assert(this->is_predefined_);
  this->source_ = IN_OUTPUT_SEGMENT;
  this->u1_.output_segment = os;
  this->u2_.offset_base = base;
}

// Set the symbol to undefined.  This is used for pre-defined
// symbols whose segments aren't known until after layout is done
// (e.g., __ehdr_start).

void
Symbol::set_undefined()
{
  this->source_ = IS_UNDEFINED;
  this->is_predefined_ = false;
}

// Class Symbol_table.

Symbol_table::Symbol_table(unsigned int count,
                           const Version_script_info& version_script)
  : saw_undefined_(0), offset_(0), has_gnu_output_(false), table_(count),
    namepool_(), forwarders_(), commons_(), tls_commons_(), small_commons_(),
    large_commons_(), forced_locals_(), warnings_(),
    version_script_(version_script), gc_(NULL), icf_(NULL),
    target_symbols_()
{
  namepool_.reserve(count);
}

Symbol_table::~Symbol_table()
{
}

// The symbol table key equality function.  This is called with
// Stringpool keys.

inline bool
Symbol_table::Symbol_table_eq::operator()(const Symbol_table_key& k1,
					  const Symbol_table_key& k2) const
{
  return k1.first == k2.first && k1.second == k2.second;
}

bool
Symbol_table::is_section_folded(Relobj* obj, unsigned int shndx) const
{
  return (parameters->options().icf_enabled()
          && this->icf_->is_section_folded(obj, shndx));
}

// For symbols that have been listed with a -u or --export-dynamic-symbol
// option, add them to the work list to avoid gc'ing them.

void 
Symbol_table::gc_mark_undef_symbols(Layout* layout)
{
  for (options::String_set::const_iterator p =
	 parameters->options().undefined_begin();
       p != parameters->options().undefined_end();
       ++p)
    {
      const char* name = p->c_str();
      Symbol* sym = this->lookup(name);
      gold_assert(sym != NULL);
      if (sym->source() == Symbol::FROM_OBJECT 
          && !sym->object()->is_dynamic())
        {
	  this->gc_mark_symbol(sym);
        }
    }

  for (options::String_set::const_iterator p =
	 parameters->options().export_dynamic_symbol_begin();
       p != parameters->options().export_dynamic_symbol_end();
       ++p)
    {
      const char* name = p->c_str();
      Symbol* sym = this->lookup(name);
      // It's not an error if a symbol named by --export-dynamic-symbol
      // is undefined.
      if (sym != NULL
	  && sym->source() == Symbol::FROM_OBJECT 
          && !sym->object()->is_dynamic())
        {
	  this->gc_mark_symbol(sym);
        }
    }

  for (Script_options::referenced_const_iterator p =
	 layout->script_options()->referenced_begin();
       p != layout->script_options()->referenced_end();
       ++p)
    {
      Symbol* sym = this->lookup(p->c_str());
      gold_assert(sym != NULL);
      if (sym->source() == Symbol::FROM_OBJECT
	  && !sym->object()->is_dynamic())
	{
	  this->gc_mark_symbol(sym);
	}
    }
}

void
Symbol_table::gc_mark_symbol(Symbol* sym)
{
  // Add the object and section to the work list.
  bool is_ordinary;
  unsigned int shndx = sym->shndx(&is_ordinary);
  if (is_ordinary && shndx != elfcpp::SHN_UNDEF && !sym->object()->is_dynamic())
    {
      gold_assert(this->gc_!= NULL);
      Relobj* relobj = static_cast<Relobj*>(sym->object());
      this->gc_->worklist().push_back(Section_id(relobj, shndx));
    }
  parameters->target().gc_mark_symbol(this, sym);
}

// When doing garbage collection, keep symbols that have been seen in
// dynamic objects.
inline void 
Symbol_table::gc_mark_dyn_syms(Symbol* sym)
{
  if (sym->in_dyn() && sym->source() == Symbol::FROM_OBJECT
      && !sym->object()->is_dynamic())
    this->gc_mark_symbol(sym);
}

// Make TO a symbol which forwards to FROM.

void
Symbol_table::make_forwarder(Symbol* from, Symbol* to)
{
  gold_assert(from != to);
  gold_assert(!from->is_forwarder() && !to->is_forwarder());
  this->forwarders_[from] = to;
  from->set_forwarder();
}

// Resolve the forwards from FROM, returning the real symbol.

Symbol*
Symbol_table::resolve_forwards(const Symbol* from) const
{
  gold_assert(from->is_forwarder());
  Unordered_map<const Symbol*, Symbol*>::const_iterator p =
    this->forwarders_.find(from);
  gold_assert(p != this->forwarders_.end());
  return p->second;
}

// Look up a symbol by name.

Symbol*
Symbol_table::lookup(const char* name, const char* version) const
{
  Stringpool::Key name_key;
  name = this->namepool_.find(name, &name_key);
  if (name == NULL)
    return NULL;

  Stringpool::Key version_key = 0;
  if (version != NULL)
    {
      version = this->namepool_.find(version, &version_key);
      if (version == NULL)
	return NULL;
    }

  Symbol_table_key key(name_key, version_key);
  Symbol_table::Symbol_table_type::const_iterator p = this->table_.find(key);
  if (p == this->table_.end())
    return NULL;
  return p->second;
}

// Resolve a Symbol with another Symbol.  This is only used in the
// unusual case where there are references to both an unversioned
// symbol and a symbol with a version, and we then discover that that
// version is the default version.  Because this is unusual, we do
// this the slow way, by converting back to an ELF symbol.

template<int size, bool big_endian>
void
Symbol_table::resolve(Sized_symbol<size>* to, const Sized_symbol<size>* from)
{
  unsigned char buf[elfcpp::Elf_sizes<size>::sym_size];
  elfcpp::Sym_write<size, big_endian> esym(buf);
  // We don't bother to set the st_name or the st_shndx field.
  esym.put_st_value(from->value());
  esym.put_st_size(from->symsize());
  esym.put_st_info(from->binding(), from->type());
  esym.put_st_other(from->visibility(), from->nonvis());
  bool is_ordinary;
  unsigned int shndx = from->shndx(&is_ordinary);
  this->resolve(to, esym.sym(), shndx, is_ordinary, shndx, from->object(),
		from->version(), true);
  if (from->in_reg())
    to->set_in_reg();
  if (from->in_dyn())
    to->set_in_dyn();
  if (parameters->options().gc_sections())
    this->gc_mark_dyn_syms(to);
}

// Record that a symbol is forced to be local by a version script or
// by visibility.

void
Symbol_table::force_local(Symbol* sym)
{
  if (!sym->is_defined() && !sym->is_common())
    return;
  if (sym->is_forced_local())
    {
      // We already got this one.
      return;
    }
  sym->set_is_forced_local();
  this->forced_locals_.push_back(sym);
}

// Adjust NAME for wrapping, and update *NAME_KEY if necessary.  This
// is only called for undefined symbols, when at least one --wrap
// option was used.

const char*
Symbol_table::wrap_symbol(const char* name, Stringpool::Key* name_key)
{
  // For some targets, we need to ignore a specific character when
  // wrapping, and add it back later.
  char prefix = '\0';
  if (name[0] == parameters->target().wrap_char())
    {
      prefix = name[0];
      ++name;
    }

  if (parameters->options().is_wrap(name))
    {
      // Turn NAME into __wrap_NAME.
      std::string s;
      if (prefix != '\0')
	s += prefix;
      s += "__wrap_";
      s += name;

      // This will give us both the old and new name in NAMEPOOL_, but
      // that is OK.  Only the versions we need will wind up in the
      // real string table in the output file.
      return this->namepool_.add(s.c_str(), true, name_key);
    }

  const char* const real_prefix = "__real_";
  const size_t real_prefix_length = strlen(real_prefix);
  if (strncmp(name, real_prefix, real_prefix_length) == 0
      && parameters->options().is_wrap(name + real_prefix_length))
    {
      // Turn __real_NAME into NAME.
      std::string s;
      if (prefix != '\0')
	s += prefix;
      s += name + real_prefix_length;
      return this->namepool_.add(s.c_str(), true, name_key);
    }

  return name;
}

// This is called when we see a symbol NAME/VERSION, and the symbol
// already exists in the symbol table, and VERSION is marked as being
// the default version.  SYM is the NAME/VERSION symbol we just added.
// DEFAULT_IS_NEW is true if this is the first time we have seen the
// symbol NAME/NULL.  PDEF points to the entry for NAME/NULL.

template<int size, bool big_endian>
void
Symbol_table::define_default_version(Sized_symbol<size>* sym,
				     bool default_is_new,
				     Symbol_table_type::iterator pdef)
{
  if (default_is_new)
    {
      // This is the first time we have seen NAME/NULL.  Make
      // NAME/NULL point to NAME/VERSION, and mark SYM as the default
      // version.
      pdef->second = sym;
      sym->set_is_default();
    }
  else if (pdef->second == sym)
    {
      // NAME/NULL already points to NAME/VERSION.  Don't mark the
      // symbol as the default if it is not already the default.
    }
  else
    {
      // This is the unfortunate case where we already have entries
      // for both NAME/VERSION and NAME/NULL.  We now see a symbol
      // NAME/VERSION where VERSION is the default version.  We have
      // already resolved this new symbol with the existing
      // NAME/VERSION symbol.

      // It's possible that NAME/NULL and NAME/VERSION are both
      // defined in regular objects.  This can only happen if one
      // object file defines foo and another defines foo@@ver.  This
      // is somewhat obscure, but we call it a multiple definition
      // error.

      // It's possible that NAME/NULL actually has a version, in which
      // case it won't be the same as VERSION.  This happens with
      // ver_test_7.so in the testsuite for the symbol t2_2.  We see
      // t2_2@@VER2, so we define both t2_2/VER2 and t2_2/NULL.  We
      // then see an unadorned t2_2 in an object file and give it
      // version VER1 from the version script.  This looks like a
      // default definition for VER1, so it looks like we should merge
      // t2_2/NULL with t2_2/VER1.  That doesn't make sense, but it's
      // not obvious that this is an error, either.  So we just punt.

      // If one of the symbols has non-default visibility, and the
      // other is defined in a shared object, then they are different
      // symbols.

      // If the two symbols are from different shared objects,
      // they are different symbols.

      // Otherwise, we just resolve the symbols as though they were
      // the same.

      if (pdef->second->version() != NULL)
	gold_assert(pdef->second->version() != sym->version());
      else if (sym->visibility() != elfcpp::STV_DEFAULT
	       && pdef->second->is_from_dynobj())
	;
      else if (pdef->second->visibility() != elfcpp::STV_DEFAULT
	       && sym->is_from_dynobj())
	;
      else if (pdef->second->is_from_dynobj()
	       && sym->is_from_dynobj()
	       && pdef->second->is_defined()
	       && pdef->second->object() != sym->object())
        ;
      else
	{
	  const Sized_symbol<size>* symdef;
	  symdef = this->get_sized_symbol<size>(pdef->second);
	  Symbol_table::resolve<size, big_endian>(sym, symdef);
	  this->make_forwarder(pdef->second, sym);
	  pdef->second = sym;
	  sym->set_is_default();
	}
    }
}

// Add one symbol from OBJECT to the symbol table.  NAME is symbol
// name and VERSION is the version; both are canonicalized.  DEF is
// whether this is the default version.  ST_SHNDX is the symbol's
// section index; IS_ORDINARY is whether this is a normal section
// rather than a special code.

// If IS_DEFAULT_VERSION is true, then this is the definition of a
// default version of a symbol.  That means that any lookup of
// NAME/NULL and any lookup of NAME/VERSION should always return the
// same symbol.  This is obvious for references, but in particular we
// want to do this for definitions: overriding NAME/NULL should also
// override NAME/VERSION.  If we don't do that, it would be very hard
// to override functions in a shared library which uses versioning.

// We implement this by simply making both entries in the hash table
// point to the same Symbol structure.  That is easy enough if this is
// the first time we see NAME/NULL or NAME/VERSION, but it is possible
// that we have seen both already, in which case they will both have
// independent entries in the symbol table.  We can't simply change
// the symbol table entry, because we have pointers to the entries
// attached to the object files.  So we mark the entry attached to the
// object file as a forwarder, and record it in the forwarders_ map.
// Note that entries in the hash table will never be marked as
// forwarders.
//
// ORIG_ST_SHNDX and ST_SHNDX are almost always the same.
// ORIG_ST_SHNDX is the section index in the input file, or SHN_UNDEF
// for a special section code.  ST_SHNDX may be modified if the symbol
// is defined in a section being discarded.

template<int size, bool big_endian>
Sized_symbol<size>*
Symbol_table::add_from_object(Object* object,
			      const char* name,
			      Stringpool::Key name_key,
			      const char* version,
			      Stringpool::Key version_key,
			      bool is_default_version,
			      const elfcpp::Sym<size, big_endian>& sym,
			      unsigned int st_shndx,
			      bool is_ordinary,
			      unsigned int orig_st_shndx)
{
  // Print a message if this symbol is being traced.
  if (parameters->options().is_trace_symbol(name))
    {
      if (orig_st_shndx == elfcpp::SHN_UNDEF)
        gold_info(_("%s: reference to %s"), object->name().c_str(), name);
      else
        gold_info(_("%s: definition of %s"), object->name().c_str(), name);
    }

  // For an undefined symbol, we may need to adjust the name using
  // --wrap.
  if (orig_st_shndx == elfcpp::SHN_UNDEF
      && parameters->options().any_wrap())
    {
      const char* wrap_name = this->wrap_symbol(name, &name_key);
      if (wrap_name != name)
	{
	  // If we see a reference to malloc with version GLIBC_2.0,
	  // and we turn it into a reference to __wrap_malloc, then we
	  // discard the version number.  Otherwise the user would be
	  // required to specify the correct version for
	  // __wrap_malloc.
	  version = NULL;
	  version_key = 0;
	  name = wrap_name;
	}
    }

  Symbol* const snull = NULL;
  std::pair<typename Symbol_table_type::iterator, bool> ins =
    this->table_.insert(std::make_pair(std::make_pair(name_key, version_key),
				       snull));

  std::pair<typename Symbol_table_type::iterator, bool> insdefault =
    std::make_pair(this->table_.end(), false);
  if (is_default_version)
    {
      const Stringpool::Key vnull_key = 0;
      insdefault = this->table_.insert(std::make_pair(std::make_pair(name_key,
								     vnull_key),
						      snull));
    }

  // ins.first: an iterator, which is a pointer to a pair.
  // ins.first->first: the key (a pair of name and version).
  // ins.first->second: the value (Symbol*).
  // ins.second: true if new entry was inserted, false if not.

  Sized_symbol<size>* ret = NULL;
  bool was_undefined_in_reg;
  bool was_common;
  if (!ins.second)
    {
      // We already have an entry for NAME/VERSION.
      ret = this->get_sized_symbol<size>(ins.first->second);
      gold_assert(ret != NULL);

      was_undefined_in_reg = ret->is_undefined() && ret->in_reg();
      // Commons from plugins are just placeholders.
      was_common = ret->is_common() && ret->object()->pluginobj() == NULL;

      this->resolve(ret, sym, st_shndx, is_ordinary, orig_st_shndx, object,
		    version, is_default_version);
      if (parameters->options().gc_sections())
        this->gc_mark_dyn_syms(ret);

      if (is_default_version)
	this->define_default_version<size, big_endian>(ret, insdefault.second,
						       insdefault.first);
      else
	{
	  bool dummy;
	  if (version != NULL
	      && ret->source() == Symbol::FROM_OBJECT
	      && ret->object() == object
	      && is_ordinary
	      && ret->shndx(&dummy) == st_shndx
	      && ret->is_default())
	    {
	      // We have seen NAME/VERSION already, and marked it as the
	      // default version, but now we see a definition for
	      // NAME/VERSION that is not the default version. This can
	      // happen when the assembler generates two symbols for
	      // a symbol as a result of a ".symver foo,foo@VER"
	      // directive. We see the first unversioned symbol and
	      // we may mark it as the default version (from a
	      // version script); then we see the second versioned
	      // symbol and we need to override the first.
	      // In any other case, the two symbols should have generated
	      // a multiple definition error.
	      // (See PR gold/18703.)
	      ret->set_is_not_default();
	      const Stringpool::Key vnull_key = 0;
	      this->table_.erase(std::make_pair(name_key, vnull_key));
	    }
	}
    }
  else
    {
      // This is the first time we have seen NAME/VERSION.
      gold_assert(ins.first->second == NULL);

      if (is_default_version && !insdefault.second)
	{
	  // We already have an entry for NAME/NULL.  If we override
	  // it, then change it to NAME/VERSION.
	  ret = this->get_sized_symbol<size>(insdefault.first->second);

	  // If the existing symbol already has a version,
	  // don't override it with the new symbol.
	  // This should only happen when the new symbol
	  // is from a shared library.
	  if (ret->version() != NULL)
	    {
	      if (!object->is_dynamic())
	        {
		  gold_warning(_("%s: conflicting default version definition"
				 " for %s@@%s"),
			       object->name().c_str(), name, version);
		  if (ret->source() == Symbol::FROM_OBJECT)
		    gold_info(_("%s: %s: previous definition of %s@@%s here"),
			      program_name,
			      ret->object()->name().c_str(),
			      name, ret->version());
	        }
	      ret = NULL;
	      is_default_version = false;
	    }
	  else
	    {
	      was_undefined_in_reg = ret->is_undefined() && ret->in_reg();
	      // Commons from plugins are just placeholders.
	      was_common = (ret->is_common()
			    && ret->object()->pluginobj() == NULL);

	      this->resolve(ret, sym, st_shndx, is_ordinary, orig_st_shndx,
			    object, version, is_default_version);
	      if (parameters->options().gc_sections())
		this->gc_mark_dyn_syms(ret);
	      ins.first->second = ret;
	    }
	}

      if (ret == NULL)
	{
	  was_undefined_in_reg = false;
	  was_common = false;

	  Sized_target<size, big_endian>* target =
	    parameters->sized_target<size, big_endian>();
	  if (!target->has_make_symbol())
	    ret = new Sized_symbol<size>();
	  else
	    {
	      ret = target->make_symbol(name, sym.get_st_type(), object,
					st_shndx, sym.get_st_value());
	      if (ret == NULL)
		{
		  // This means that we don't want a symbol table
		  // entry after all.
		  if (!is_default_version)
		    this->table_.erase(ins.first);
		  else
		    {
		      this->table_.erase(insdefault.first);
		      // Inserting INSDEFAULT invalidated INS.
		      this->table_.erase(std::make_pair(name_key,
							version_key));
		    }
		  return NULL;
		}
	    }

	  ret->init_object(name, version, object, sym, st_shndx, is_ordinary);

	  ins.first->second = ret;
	  if (is_default_version)
	    {
	      // This is the first time we have seen NAME/NULL.  Point
	      // it at the new entry for NAME/VERSION.
	      gold_assert(insdefault.second);
	      insdefault.first->second = ret;
	    }
	}

      if (is_default_version)
	ret->set_is_default();
    }

  // Record every time we see a new undefined symbol, to speed up archive
  // groups. We only care about symbols undefined in regular objects here
  // because undefined symbols only in dynamic objects should't trigger rescans.
  if (!was_undefined_in_reg && ret->is_undefined() && ret->in_reg())
    {
      ++this->saw_undefined_;
      if (parameters->options().has_plugins())
	parameters->options().plugins()->new_undefined_symbol(ret);
    }

  // Keep track of common symbols, to speed up common symbol
  // allocation.  Don't record commons from plugin objects;
  // we need to wait until we see the real symbol in the
  // replacement file.
  if (!was_common && ret->is_common() && ret->object()->pluginobj() == NULL)
    {
      if (ret->type() == elfcpp::STT_TLS)
	this->tls_commons_.push_back(ret);
      else if (!is_ordinary
	       && st_shndx == parameters->target().small_common_shndx())
	this->small_commons_.push_back(ret);
      else if (!is_ordinary
	       && st_shndx == parameters->target().large_common_shndx())
	this->large_commons_.push_back(ret);
      else
	this->commons_.push_back(ret);
    }

  // If we're not doing a relocatable link, then any symbol with
  // hidden or internal visibility is local.
  if ((ret->visibility() == elfcpp::STV_HIDDEN
       || ret->visibility() == elfcpp::STV_INTERNAL)
      && (ret->binding() == elfcpp::STB_GLOBAL
	  || ret->binding() == elfcpp::STB_GNU_UNIQUE
	  || ret->binding() == elfcpp::STB_WEAK)
      && !parameters->options().relocatable())
    this->force_local(ret);

  return ret;
}

// Add all the symbols in a relocatable object to the hash table.

template<int size, bool big_endian>
void
Symbol_table::add_from_relobj(
    Sized_relobj_file<size, big_endian>* relobj,
    const unsigned char* syms,
    size_t count,
    size_t symndx_offset,
    const char* sym_names,
    size_t sym_name_size,
    typename Sized_relobj_file<size, big_endian>::Symbols* sympointers,
    size_t* defined)
{
  *defined = 0;

  gold_assert(size == parameters->target().get_size());

  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  const bool just_symbols = relobj->just_symbols();

  const unsigned char* p = syms;
  for (size_t i = 0; i < count; ++i, p += sym_size)
    {
      (*sympointers)[i] = NULL;

      elfcpp::Sym<size, big_endian> sym(p);

      unsigned int st_name = sym.get_st_name();
      if (st_name >= sym_name_size)
	{
	  relobj->error(_("bad global symbol name offset %u at %zu"),
			st_name, i);
	  continue;
	}

      const char* name = sym_names + st_name;

      if (!parameters->options().relocatable()
	  && name[0] == '_'
	  && name[1] == '_'
	  && strcmp (name + (name[2] == '_'), "__gnu_lto_slim") == 0)
        gold_info(_("%s: plugin needed to handle lto object"),
		  relobj->name().c_str());

      bool is_ordinary;
      unsigned int st_shndx = relobj->adjust_sym_shndx(i + symndx_offset,
						       sym.get_st_shndx(),
						       &is_ordinary);
      unsigned int orig_st_shndx = st_shndx;
      if (!is_ordinary)
	orig_st_shndx = elfcpp::SHN_UNDEF;

      if (st_shndx != elfcpp::SHN_UNDEF)
	++*defined;

      // A symbol defined in a section which we are not including must
      // be treated as an undefined symbol.
      bool is_defined_in_discarded_section = false;
      if (st_shndx != elfcpp::SHN_UNDEF
	  && is_ordinary
	  && !relobj->is_section_included(st_shndx)
          && !this->is_section_folded(relobj, st_shndx))
	{
	  st_shndx = elfcpp::SHN_UNDEF;
	  is_defined_in_discarded_section = true;
	}

      // In an object file, an '@' in the name separates the symbol
      // name from the version name.  If there are two '@' characters,
      // this is the default version.
      const char* ver = strchr(name, '@');
      Stringpool::Key ver_key = 0;
      int namelen = 0;
      // IS_DEFAULT_VERSION: is the version default?
      // IS_FORCED_LOCAL: is the symbol forced local?
      bool is_default_version = false;
      bool is_forced_local = false;

      // FIXME: For incremental links, we don't store version information,
      // so we need to ignore version symbols for now.
      if (parameters->incremental_update() && ver != NULL)
	{
	  namelen = ver - name;
	  ver = NULL;
	}

      if (ver != NULL)
        {
          // The symbol name is of the form foo@VERSION or foo@@VERSION
          namelen = ver - name;
          ++ver;
	  if (*ver == '@')
	    {
	      is_default_version = true;
	      ++ver;
	    }
	  ver = this->namepool_.add(ver, true, &ver_key);
        }
      // We don't want to assign a version to an undefined symbol,
      // even if it is listed in the version script.  FIXME: What
      // about a common symbol?
      else
	{
	  namelen = strlen(name);
	  if (!this->version_script_.empty()
	      && st_shndx != elfcpp::SHN_UNDEF)
	    {
	      // The symbol name did not have a version, but the
	      // version script may assign a version anyway.
	      std::string version;
	      bool is_global;
	      if (this->version_script_.get_symbol_version(name, &version,
							   &is_global))
		{
		  if (!is_global)
		    is_forced_local = true;
		  else if (!version.empty())
		    {
		      ver = this->namepool_.add_with_length(version.c_str(),
							    version.length(),
							    true,
							    &ver_key);
		      is_default_version = true;
		    }
		}
	    }
	}

      elfcpp::Sym<size, big_endian>* psym = &sym;
      unsigned char symbuf[sym_size];
      elfcpp::Sym<size, big_endian> sym2(symbuf);
      if (just_symbols)
	{
	  memcpy(symbuf, p, sym_size);
	  elfcpp::Sym_write<size, big_endian> sw(symbuf);
	  if (orig_st_shndx != elfcpp::SHN_UNDEF
	      && is_ordinary
	      && relobj->e_type() == elfcpp::ET_REL)
	    {
	      // Symbol values in relocatable object files are section
	      // relative.  This is normally what we want, but since here
	      // we are converting the symbol to absolute we need to add
	      // the section address.  The section address in an object
	      // file is normally zero, but people can use a linker
	      // script to change it.
	      sw.put_st_value(sym.get_st_value()
			      + relobj->section_address(orig_st_shndx));
	    }
	  st_shndx = elfcpp::SHN_ABS;
	  is_ordinary = false;
	  psym = &sym2;
	}

      // Fix up visibility if object has no-export set.
      if (relobj->no_export()
	  && (orig_st_shndx != elfcpp::SHN_UNDEF || !is_ordinary))
        {
	  // We may have copied symbol already above.
	  if (psym != &sym2)
	    {
	      memcpy(symbuf, p, sym_size);
	      psym = &sym2;
	    }

	  elfcpp::STV visibility = sym2.get_st_visibility();
	  if (visibility == elfcpp::STV_DEFAULT
	      || visibility == elfcpp::STV_PROTECTED)
	    {
	      elfcpp::Sym_write<size, big_endian> sw(symbuf);
	      unsigned char nonvis = sym2.get_st_nonvis();
	      sw.put_st_other(elfcpp::STV_HIDDEN, nonvis);
	    }
        }

      Stringpool::Key name_key;
      name = this->namepool_.add_with_length(name, namelen, true,
					     &name_key);

      Sized_symbol<size>* res;
      res = this->add_from_object(relobj, name, name_key, ver, ver_key,
				  is_default_version, *psym, st_shndx,
				  is_ordinary, orig_st_shndx);

      if (res == NULL)
	continue;
      
      if (is_forced_local)
	this->force_local(res);

      // Do not treat this symbol as garbage if this symbol will be
      // exported to the dynamic symbol table.  This is true when
      // building a shared library or using --export-dynamic and
      // the symbol is externally visible.
      if (parameters->options().gc_sections()
	  && res->is_externally_visible()
	  && !res->is_from_dynobj()
          && (parameters->options().shared()
	      || parameters->options().export_dynamic()
	      || parameters->options().in_dynamic_list(res->name())))
        this->gc_mark_symbol(res);

      if (is_defined_in_discarded_section)
	res->set_is_defined_in_discarded_section();

      (*sympointers)[i] = res;
    }
}

// Add a symbol from a plugin-claimed file.

template<int size, bool big_endian>
Symbol*
Symbol_table::add_from_pluginobj(
    Sized_pluginobj<size, big_endian>* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<size, big_endian>* sym)
{
  unsigned int st_shndx = sym->get_st_shndx();
  bool is_ordinary = st_shndx < elfcpp::SHN_LORESERVE;

  Stringpool::Key ver_key = 0;
  bool is_default_version = false;
  bool is_forced_local = false;

  if (ver != NULL)
    {
      ver = this->namepool_.add(ver, true, &ver_key);
    }
  // We don't want to assign a version to an undefined symbol,
  // even if it is listed in the version script.  FIXME: What
  // about a common symbol?
  else
    {
      if (!this->version_script_.empty()
          && st_shndx != elfcpp::SHN_UNDEF)
        {
          // The symbol name did not have a version, but the
          // version script may assign a version anyway.
          std::string version;
	  bool is_global;
          if (this->version_script_.get_symbol_version(name, &version,
						       &is_global))
            {
	      if (!is_global)
		is_forced_local = true;
	      else if (!version.empty())
                {
                  ver = this->namepool_.add_with_length(version.c_str(),
                                                        version.length(),
                                                        true,
                                                        &ver_key);
                  is_default_version = true;
                }
            }
        }
    }

  Stringpool::Key name_key;
  name = this->namepool_.add(name, true, &name_key);

  Sized_symbol<size>* res;
  res = this->add_from_object(obj, name, name_key, ver, ver_key,
		              is_default_version, *sym, st_shndx,
			      is_ordinary, st_shndx);

  if (res == NULL)
    return NULL;

  if (is_forced_local)
    this->force_local(res);

  return res;
}

// Add all the symbols in a dynamic object to the hash table.

template<int size, bool big_endian>
void
Symbol_table::add_from_dynobj(
    Sized_dynobj<size, big_endian>* dynobj,
    const unsigned char* syms,
    size_t count,
    const char* sym_names,
    size_t sym_name_size,
    const unsigned char* versym,
    size_t versym_size,
    const std::vector<const char*>* version_map,
    typename Sized_relobj_file<size, big_endian>::Symbols* sympointers,
    size_t* defined)
{
  *defined = 0;

  gold_assert(size == parameters->target().get_size());

  if (dynobj->just_symbols())
    {
      gold_error(_("--just-symbols does not make sense with a shared object"));
      return;
    }

  // FIXME: For incremental links, we don't store version information,
  // so we need to ignore version symbols for now.
  if (parameters->incremental_update())
    versym = NULL;

  if (versym != NULL && versym_size / 2 < count)
    {
      dynobj->error(_("too few symbol versions"));
      return;
    }

  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  // We keep a list of all STT_OBJECT symbols, so that we can resolve
  // weak aliases.  This is necessary because if the dynamic object
  // provides the same variable under two names, one of which is a
  // weak definition, and the regular object refers to the weak
  // definition, we have to put both the weak definition and the
  // strong definition into the dynamic symbol table.  Given a weak
  // definition, the only way that we can find the corresponding
  // strong definition, if any, is to search the symbol table.
  std::vector<Sized_symbol<size>*> object_symbols;

  const unsigned char* p = syms;
  const unsigned char* vs = versym;
  for (size_t i = 0; i < count; ++i, p += sym_size, vs += 2)
    {
      elfcpp::Sym<size, big_endian> sym(p);

      if (sympointers != NULL)
	(*sympointers)[i] = NULL;

      // Ignore symbols with local binding or that have
      // internal or hidden visibility.
      if (sym.get_st_bind() == elfcpp::STB_LOCAL
          || sym.get_st_visibility() == elfcpp::STV_INTERNAL
          || sym.get_st_visibility() == elfcpp::STV_HIDDEN)
	continue;

      // A protected symbol in a shared library must be treated as a
      // normal symbol when viewed from outside the shared library.
      // Implement this by overriding the visibility here.
      // Likewise, an IFUNC symbol in a shared library must be treated
      // as a normal FUNC symbol.
      elfcpp::Sym<size, big_endian>* psym = &sym;
      unsigned char symbuf[sym_size];
      elfcpp::Sym<size, big_endian> sym2(symbuf);
      if (sym.get_st_visibility() == elfcpp::STV_PROTECTED
	  || sym.get_st_type() == elfcpp::STT_GNU_IFUNC)
	{
	  memcpy(symbuf, p, sym_size);
	  elfcpp::Sym_write<size, big_endian> sw(symbuf);
	  if (sym.get_st_visibility() == elfcpp::STV_PROTECTED)
	    sw.put_st_other(elfcpp::STV_DEFAULT, sym.get_st_nonvis());
	  if (sym.get_st_type() == elfcpp::STT_GNU_IFUNC)
	    sw.put_st_info(sym.get_st_bind(), elfcpp::STT_FUNC);
	  psym = &sym2;
	}

      unsigned int st_name = psym->get_st_name();
      if (st_name >= sym_name_size)
	{
	  dynobj->error(_("bad symbol name offset %u at %zu"),
			st_name, i);
	  continue;
	}

      const char* name = sym_names + st_name;

      bool is_ordinary;
      unsigned int st_shndx = dynobj->adjust_sym_shndx(i, psym->get_st_shndx(),
						       &is_ordinary);

      if (st_shndx != elfcpp::SHN_UNDEF)
	++*defined;

      Sized_symbol<size>* res;

      if (versym == NULL)
	{
	  Stringpool::Key name_key;
	  name = this->namepool_.add(name, true, &name_key);
	  res = this->add_from_object(dynobj, name, name_key, NULL, 0,
				      false, *psym, st_shndx, is_ordinary,
				      st_shndx);
	}
      else
	{
	  // Read the version information.

	  unsigned int v = elfcpp::Swap<16, big_endian>::readval(vs);

	  bool hidden = (v & elfcpp::VERSYM_HIDDEN) != 0;
	  v &= elfcpp::VERSYM_VERSION;

	  // The Sun documentation says that V can be VER_NDX_LOCAL,
	  // or VER_NDX_GLOBAL, or a version index.  The meaning of
	  // VER_NDX_LOCAL is defined as "Symbol has local scope."
	  // The old GNU linker will happily generate VER_NDX_LOCAL
	  // for an undefined symbol.  I don't know what the Sun
	  // linker will generate.

	  if (v == static_cast<unsigned int>(elfcpp::VER_NDX_LOCAL)
	      && st_shndx != elfcpp::SHN_UNDEF)
	    {
	      // This symbol should not be visible outside the object.
	      continue;
	    }

	  // At this point we are definitely going to add this symbol.
	  Stringpool::Key name_key;
	  name = this->namepool_.add(name, true, &name_key);

	  if (v == static_cast<unsigned int>(elfcpp::VER_NDX_LOCAL)
	      || v == static_cast<unsigned int>(elfcpp::VER_NDX_GLOBAL))
	    {
	      // This symbol does not have a version.
	      res = this->add_from_object(dynobj, name, name_key, NULL, 0,
					  false, *psym, st_shndx, is_ordinary,
					  st_shndx);
	    }
	  else
	    {
	      if (v >= version_map->size())
		{
		  dynobj->error(_("versym for symbol %zu out of range: %u"),
				i, v);
		  continue;
		}

	      const char* version = (*version_map)[v];
	      if (version == NULL)
		{
		  dynobj->error(_("versym for symbol %zu has no name: %u"),
				i, v);
		  continue;
		}

	      Stringpool::Key version_key;
	      version = this->namepool_.add(version, true, &version_key);

	      // If this is an absolute symbol, and the version name
	      // and symbol name are the same, then this is the
	      // version definition symbol.  These symbols exist to
	      // support using -u to pull in particular versions.  We
	      // do not want to record a version for them.
	      if (st_shndx == elfcpp::SHN_ABS
		  && !is_ordinary
		  && name_key == version_key)
		res = this->add_from_object(dynobj, name, name_key, NULL, 0,
					    false, *psym, st_shndx, is_ordinary,
					    st_shndx);
	      else
		{
		  const bool is_default_version =
		    !hidden && st_shndx != elfcpp::SHN_UNDEF;
		  res = this->add_from_object(dynobj, name, name_key, version,
					      version_key, is_default_version,
					      *psym, st_shndx,
					      is_ordinary, st_shndx);
		}
	    }
	}

      if (res == NULL)
	continue;

      // Note that it is possible that RES was overridden by an
      // earlier object, in which case it can't be aliased here.
      if (st_shndx != elfcpp::SHN_UNDEF
	  && is_ordinary
	  && psym->get_st_type() == elfcpp::STT_OBJECT
	  && res->source() == Symbol::FROM_OBJECT
	  && res->object() == dynobj)
	object_symbols.push_back(res);

      // If the symbol has protected visibility in the dynobj,
      // mark it as such if it was not overridden.
      if (res->source() == Symbol::FROM_OBJECT
          && res->object() == dynobj
          && sym.get_st_visibility() == elfcpp::STV_PROTECTED)
        res->set_is_protected();

      if (sympointers != NULL)
	(*sympointers)[i] = res;
    }

  this->record_weak_aliases(&object_symbols);
}

// Add a symbol from a incremental object file.

template<int size, bool big_endian>
Sized_symbol<size>*
Symbol_table::add_from_incrobj(
    Object* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<size, big_endian>* sym)
{
  unsigned int st_shndx = sym->get_st_shndx();
  bool is_ordinary = st_shndx < elfcpp::SHN_LORESERVE;

  Stringpool::Key ver_key = 0;
  bool is_default_version = false;

  Stringpool::Key name_key;
  name = this->namepool_.add(name, true, &name_key);

  Sized_symbol<size>* res;
  res = this->add_from_object(obj, name, name_key, ver, ver_key,
		              is_default_version, *sym, st_shndx,
			      is_ordinary, st_shndx);

  return res;
}

// This is used to sort weak aliases.  We sort them first by section
// index, then by offset, then by weak ahead of strong.

template<int size>
class Weak_alias_sorter
{
 public:
  bool operator()(const Sized_symbol<size>*, const Sized_symbol<size>*) const;
};

template<int size>
bool
Weak_alias_sorter<size>::operator()(const Sized_symbol<size>* s1,
				    const Sized_symbol<size>* s2) const
{
  bool is_ordinary;
  unsigned int s1_shndx = s1->shndx(&is_ordinary);
  gold_assert(is_ordinary);
  unsigned int s2_shndx = s2->shndx(&is_ordinary);
  gold_assert(is_ordinary);
  if (s1_shndx != s2_shndx)
    return s1_shndx < s2_shndx;

  if (s1->value() != s2->value())
    return s1->value() < s2->value();
  if (s1->binding() != s2->binding())
    {
      if (s1->binding() == elfcpp::STB_WEAK)
	return true;
      if (s2->binding() == elfcpp::STB_WEAK)
	return false;
    }
  return std::string(s1->name()) < std::string(s2->name());
}

// SYMBOLS is a list of object symbols from a dynamic object.  Look
// for any weak aliases, and record them so that if we add the weak
// alias to the dynamic symbol table, we also add the corresponding
// strong symbol.

template<int size>
void
Symbol_table::record_weak_aliases(std::vector<Sized_symbol<size>*>* symbols)
{
  // Sort the vector by section index, then by offset, then by weak
  // ahead of strong.
  std::sort(symbols->begin(), symbols->end(), Weak_alias_sorter<size>());

  // Walk through the vector.  For each weak definition, record
  // aliases.
  for (typename std::vector<Sized_symbol<size>*>::const_iterator p =
	 symbols->begin();
       p != symbols->end();
       ++p)
    {
      if ((*p)->binding() != elfcpp::STB_WEAK)
	continue;

      // Build a circular list of weak aliases.  Each symbol points to
      // the next one in the circular list.

      Sized_symbol<size>* from_sym = *p;
      typename std::vector<Sized_symbol<size>*>::const_iterator q;
      for (q = p + 1; q != symbols->end(); ++q)
	{
	  bool dummy;
	  if ((*q)->shndx(&dummy) != from_sym->shndx(&dummy)
	      || (*q)->value() != from_sym->value())
	    break;

	  this->weak_aliases_[from_sym] = *q;
	  from_sym->set_has_alias();
	  from_sym = *q;
	}

      if (from_sym != *p)
	{
	  this->weak_aliases_[from_sym] = *p;
	  from_sym->set_has_alias();
	}

      p = q - 1;
    }
}

// Create and return a specially defined symbol.  If ONLY_IF_REF is
// true, then only create the symbol if there is a reference to it.
// If this does not return NULL, it sets *POLDSYM to the existing
// symbol if there is one.  This sets *RESOLVE_OLDSYM if we should
// resolve the newly created symbol to the old one.  This
// canonicalizes *PNAME and *PVERSION.

template<int size, bool big_endian>
Sized_symbol<size>*
Symbol_table::define_special_symbol(const char** pname, const char** pversion,
				    bool only_if_ref,
				    elfcpp::STV visibility,
                                    Sized_symbol<size>** poldsym,
				    bool* resolve_oldsym, bool is_forced_local)
{
  *resolve_oldsym = false;
  *poldsym = NULL;

  // If the caller didn't give us a version, see if we get one from
  // the version script.
  std::string v;
  bool is_default_version = false;
  if (!is_forced_local && *pversion == NULL)
    {
      bool is_global;
      if (this->version_script_.get_symbol_version(*pname, &v, &is_global))
	{
	  if (is_global && !v.empty())
	    {
	      *pversion = v.c_str();
	      // If we get the version from a version script, then we
	      // are also the default version.
	      is_default_version = true;
	    }
	}
    }

  Symbol* oldsym;
  Sized_symbol<size>* sym;

  bool add_to_table = false;
  typename Symbol_table_type::iterator add_loc = this->table_.end();
  bool add_def_to_table = false;
  typename Symbol_table_type::iterator add_def_loc = this->table_.end();

  if (only_if_ref)
    {
      oldsym = this->lookup(*pname, *pversion);
      if (oldsym == NULL && is_default_version)
	oldsym = this->lookup(*pname, NULL);
      if (oldsym == NULL)
	return NULL;
      if (!oldsym->is_undefined())
	{
	  // Skip if the old definition is from a regular object.
	  if (!oldsym->is_from_dynobj())
	    return NULL;

	  // If the symbol has hidden or internal visibility, ignore
	  // definition and reference from a dynamic object.
	  if ((visibility == elfcpp::STV_HIDDEN
	       || visibility == elfcpp::STV_INTERNAL)
	      && !oldsym->in_reg())
	    return NULL;
	}

      *pname = oldsym->name();
      if (is_default_version)
	*pversion = this->namepool_.add(*pversion, true, NULL);
      else
	*pversion = oldsym->version();
    }
  else
    {
      // Canonicalize NAME and VERSION.
      Stringpool::Key name_key;
      *pname = this->namepool_.add(*pname, true, &name_key);

      Stringpool::Key version_key = 0;
      if (*pversion != NULL)
	*pversion = this->namepool_.add(*pversion, true, &version_key);

      Symbol* const snull = NULL;
      std::pair<typename Symbol_table_type::iterator, bool> ins =
	this->table_.insert(std::make_pair(std::make_pair(name_key,
							  version_key),
					   snull));

      std::pair<typename Symbol_table_type::iterator, bool> insdefault =
	std::make_pair(this->table_.end(), false);
      if (is_default_version)
	{
	  const Stringpool::Key vnull = 0;
	  insdefault =
	    this->table_.insert(std::make_pair(std::make_pair(name_key,
							      vnull),
					       snull));
	}

      if (!ins.second)
	{
	  // We already have a symbol table entry for NAME/VERSION.
	  oldsym = ins.first->second;
	  gold_assert(oldsym != NULL);

	  if (is_default_version)
	    {
	      Sized_symbol<size>* soldsym =
		this->get_sized_symbol<size>(oldsym);
	      this->define_default_version<size, big_endian>(soldsym,
							     insdefault.second,
							     insdefault.first);
	    }
	}
      else
	{
	  // We haven't seen this symbol before.
	  gold_assert(ins.first->second == NULL);

	  add_to_table = true;
	  add_loc = ins.first;

	  if (is_default_version
	      && !insdefault.second
	      && insdefault.first->second->version() == NULL)
	    {
	      // We are adding NAME/VERSION, and it is the default
	      // version.  We already have an entry for NAME/NULL
	      // that does not already have a version.
	      oldsym = insdefault.first->second;
	      *resolve_oldsym = true;
	    }
	  else
	    {
	      oldsym = NULL;

	      if (is_default_version)
		{
		  add_def_to_table = true;
		  add_def_loc = insdefault.first;
		}
	    }
	}
    }

  const Target& target = parameters->target();
  if (!target.has_make_symbol())
    sym = new Sized_symbol<size>();
  else
    {
      Sized_target<size, big_endian>* sized_target =
	parameters->sized_target<size, big_endian>();
      sym = sized_target->make_symbol(*pname, elfcpp::STT_NOTYPE,
				      NULL, elfcpp::SHN_UNDEF, 0);
      if (sym == NULL)
        return NULL;
    }

  if (add_to_table)
    add_loc->second = sym;
  else
    gold_assert(oldsym != NULL);

  if (add_def_to_table)
    add_def_loc->second = sym;

  *poldsym = this->get_sized_symbol<size>(oldsym);

  return sym;
}

// Define a symbol based on an Output_data.

Symbol*
Symbol_table::define_in_output_data(const char* name,
				    const char* version,
				    Defined defined,
				    Output_data* od,
				    uint64_t value,
				    uint64_t symsize,
				    elfcpp::STT type,
				    elfcpp::STB binding,
				    elfcpp::STV visibility,
				    unsigned char nonvis,
				    bool offset_is_from_end,
				    bool only_if_ref)
{
  if (parameters->target().get_size() == 32)
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
      return this->do_define_in_output_data<32>(name, version, defined, od,
                                                value, symsize, type, binding,
                                                visibility, nonvis,
                                                offset_is_from_end,
                                                only_if_ref);
#else
      gold_unreachable();
#endif
    }
  else if (parameters->target().get_size() == 64)
    {
#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
      return this->do_define_in_output_data<64>(name, version, defined, od,
                                                value, symsize, type, binding,
                                                visibility, nonvis,
                                                offset_is_from_end,
                                                only_if_ref);
#else
      gold_unreachable();
#endif
    }
  else
    gold_unreachable();
}

// Define a symbol in an Output_data, sized version.

template<int size>
Sized_symbol<size>*
Symbol_table::do_define_in_output_data(
    const char* name,
    const char* version,
    Defined defined,
    Output_data* od,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    typename elfcpp::Elf_types<size>::Elf_WXword symsize,
    elfcpp::STT type,
    elfcpp::STB binding,
    elfcpp::STV visibility,
    unsigned char nonvis,
    bool offset_is_from_end,
    bool only_if_ref)
{
  Sized_symbol<size>* sym;
  Sized_symbol<size>* oldsym;
  bool resolve_oldsym;
  const bool is_forced_local = binding == elfcpp::STB_LOCAL;

  if (parameters->target().is_big_endian())
    {
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_64_BIG)
      sym = this->define_special_symbol<size, true>(&name, &version,
						    only_if_ref,
						    visibility,
						    &oldsym,
						    &resolve_oldsym,
						    is_forced_local);
#else
      gold_unreachable();
#endif
    }
  else
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_64_LITTLE)
      sym = this->define_special_symbol<size, false>(&name, &version,
						     only_if_ref,
						     visibility,
						     &oldsym,
						     &resolve_oldsym,
						     is_forced_local);
#else
      gold_unreachable();
#endif
    }

  if (sym == NULL)
    return NULL;

  sym->init_output_data(name, version, od, value, symsize, type, binding,
			visibility, nonvis, offset_is_from_end,
			defined == PREDEFINED);

  if (oldsym == NULL)
    {
      if (is_forced_local || this->version_script_.symbol_is_local(name))
	this->force_local(sym);
      else if (version != NULL)
	sym->set_is_default();
      return sym;
    }

  if (Symbol_table::should_override_with_special(oldsym, type, defined))
    this->override_with_special(oldsym, sym);

  if (resolve_oldsym)
    return sym;
  else
    {
      if (defined == PREDEFINED
	  && (is_forced_local || this->version_script_.symbol_is_local(name)))
	this->force_local(oldsym);
      delete sym;
      return oldsym;
    }
}

// Define a symbol based on an Output_segment.

Symbol*
Symbol_table::define_in_output_segment(const char* name,
				       const char* version,
				       Defined defined,
				       Output_segment* os,
				       uint64_t value,
				       uint64_t symsize,
				       elfcpp::STT type,
				       elfcpp::STB binding,
				       elfcpp::STV visibility,
				       unsigned char nonvis,
				       Symbol::Segment_offset_base offset_base,
				       bool only_if_ref)
{
  if (parameters->target().get_size() == 32)
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
      return this->do_define_in_output_segment<32>(name, version, defined, os,
                                                   value, symsize, type,
                                                   binding, visibility, nonvis,
                                                   offset_base, only_if_ref);
#else
      gold_unreachable();
#endif
    }
  else if (parameters->target().get_size() == 64)
    {
#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
      return this->do_define_in_output_segment<64>(name, version, defined, os,
                                                   value, symsize, type,
                                                   binding, visibility, nonvis,
                                                   offset_base, only_if_ref);
#else
      gold_unreachable();
#endif
    }
  else
    gold_unreachable();
}

// Define a symbol in an Output_segment, sized version.

template<int size>
Sized_symbol<size>*
Symbol_table::do_define_in_output_segment(
    const char* name,
    const char* version,
    Defined defined,
    Output_segment* os,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    typename elfcpp::Elf_types<size>::Elf_WXword symsize,
    elfcpp::STT type,
    elfcpp::STB binding,
    elfcpp::STV visibility,
    unsigned char nonvis,
    Symbol::Segment_offset_base offset_base,
    bool only_if_ref)
{
  Sized_symbol<size>* sym;
  Sized_symbol<size>* oldsym;
  bool resolve_oldsym;
  const bool is_forced_local = binding == elfcpp::STB_LOCAL;

  if (parameters->target().is_big_endian())
    {
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_64_BIG)
      sym = this->define_special_symbol<size, true>(&name, &version,
						    only_if_ref,
						    visibility,
						    &oldsym,
						    &resolve_oldsym,
						    is_forced_local);
#else
      gold_unreachable();
#endif
    }
  else
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_64_LITTLE)
      sym = this->define_special_symbol<size, false>(&name, &version,
						     only_if_ref,
						     visibility,
						     &oldsym,
						     &resolve_oldsym,
						     is_forced_local);
#else
      gold_unreachable();
#endif
    }

  if (sym == NULL)
    return NULL;

  sym->init_output_segment(name, version, os, value, symsize, type, binding,
			   visibility, nonvis, offset_base,
			   defined == PREDEFINED);

  if (oldsym == NULL)
    {
      if (is_forced_local || this->version_script_.symbol_is_local(name))
	this->force_local(sym);
      else if (version != NULL)
	sym->set_is_default();
      return sym;
    }

  if (Symbol_table::should_override_with_special(oldsym, type, defined))
    this->override_with_special(oldsym, sym);

  if (resolve_oldsym)
    return sym;
  else
    {
      if (is_forced_local || this->version_script_.symbol_is_local(name))
	this->force_local(oldsym);
      delete sym;
      return oldsym;
    }
}

// Define a special symbol with a constant value.  It is a multiple
// definition error if this symbol is already defined.

Symbol*
Symbol_table::define_as_constant(const char* name,
				 const char* version,
				 Defined defined,
				 uint64_t value,
				 uint64_t symsize,
				 elfcpp::STT type,
				 elfcpp::STB binding,
				 elfcpp::STV visibility,
				 unsigned char nonvis,
				 bool only_if_ref,
                                 bool force_override)
{
  if (parameters->target().get_size() == 32)
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
      return this->do_define_as_constant<32>(name, version, defined, value,
                                             symsize, type, binding,
                                             visibility, nonvis, only_if_ref,
                                             force_override);
#else
      gold_unreachable();
#endif
    }
  else if (parameters->target().get_size() == 64)
    {
#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
      return this->do_define_as_constant<64>(name, version, defined, value,
                                             symsize, type, binding,
                                             visibility, nonvis, only_if_ref,
                                             force_override);
#else
      gold_unreachable();
#endif
    }
  else
    gold_unreachable();
}

// Define a symbol as a constant, sized version.

template<int size>
Sized_symbol<size>*
Symbol_table::do_define_as_constant(
    const char* name,
    const char* version,
    Defined defined,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    typename elfcpp::Elf_types<size>::Elf_WXword symsize,
    elfcpp::STT type,
    elfcpp::STB binding,
    elfcpp::STV visibility,
    unsigned char nonvis,
    bool only_if_ref,
    bool force_override)
{
  Sized_symbol<size>* sym;
  Sized_symbol<size>* oldsym;
  bool resolve_oldsym;
  const bool is_forced_local = binding == elfcpp::STB_LOCAL;

  if (parameters->target().is_big_endian())
    {
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_64_BIG)
      sym = this->define_special_symbol<size, true>(&name, &version,
						    only_if_ref,
						    visibility,
						    &oldsym,
						    &resolve_oldsym,
						    is_forced_local);
#else
      gold_unreachable();
#endif
    }
  else
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_64_LITTLE)
      sym = this->define_special_symbol<size, false>(&name, &version,
						     only_if_ref,
						     visibility,
						     &oldsym,
						     &resolve_oldsym,
						     is_forced_local);
#else
      gold_unreachable();
#endif
    }

  if (sym == NULL)
    return NULL;

  sym->init_constant(name, version, value, symsize, type, binding, visibility,
		     nonvis, defined == PREDEFINED);

  if (oldsym == NULL)
    {
      // Version symbols are absolute symbols with name == version.
      // We don't want to force them to be local.
      if ((version == NULL
	   || name != version
	   || value != 0)
	  && (is_forced_local || this->version_script_.symbol_is_local(name)))
	this->force_local(sym);
      else if (version != NULL
	       && (name != version || value != 0))
	sym->set_is_default();
      return sym;
    }

  if (force_override
      || Symbol_table::should_override_with_special(oldsym, type, defined))
    this->override_with_special(oldsym, sym);

  if (resolve_oldsym)
    return sym;
  else
    {
      if (is_forced_local || this->version_script_.symbol_is_local(name))
	this->force_local(oldsym);
      delete sym;
      return oldsym;
    }
}

// Define a set of symbols in output sections.

void
Symbol_table::define_symbols(const Layout* layout, int count,
			     const Define_symbol_in_section* p,
			     bool only_if_ref)
{
  for (int i = 0; i < count; ++i, ++p)
    {
      Output_section* os = layout->find_output_section(p->output_section);
      if (os != NULL)
	this->define_in_output_data(p->name, NULL, PREDEFINED, os, p->value,
				    p->size, p->type, p->binding,
				    p->visibility, p->nonvis,
				    p->offset_is_from_end,
				    only_if_ref || p->only_if_ref);
      else
	this->define_as_constant(p->name, NULL, PREDEFINED, 0, p->size,
				 p->type, p->binding, p->visibility, p->nonvis,
				 only_if_ref || p->only_if_ref,
                                 false);
    }
}

// Define a set of symbols in output segments.

void
Symbol_table::define_symbols(const Layout* layout, int count,
			     const Define_symbol_in_segment* p,
			     bool only_if_ref)
{
  for (int i = 0; i < count; ++i, ++p)
    {
      Output_segment* os = layout->find_output_segment(p->segment_type,
						       p->segment_flags_set,
						       p->segment_flags_clear);
      if (os != NULL)
	this->define_in_output_segment(p->name, NULL, PREDEFINED, os, p->value,
				       p->size, p->type, p->binding,
				       p->visibility, p->nonvis,
				       p->offset_base,
				       only_if_ref || p->only_if_ref);
      else
	this->define_as_constant(p->name, NULL, PREDEFINED, 0, p->size,
				 p->type, p->binding, p->visibility, p->nonvis,
				 only_if_ref || p->only_if_ref,
                                 false);
    }
}

// Define CSYM using a COPY reloc.  POSD is the Output_data where the
// symbol should be defined--typically a .dyn.bss section.  VALUE is
// the offset within POSD.

template<int size>
void
Symbol_table::define_with_copy_reloc(
    Sized_symbol<size>* csym,
    Output_data* posd,
    typename elfcpp::Elf_types<size>::Elf_Addr value)
{
  gold_assert(csym->is_from_dynobj());
  gold_assert(!csym->is_copied_from_dynobj());
  Object* object = csym->object();
  gold_assert(object->is_dynamic());
  Dynobj* dynobj = static_cast<Dynobj*>(object);

  // Our copied variable has to override any variable in a shared
  // library.
  elfcpp::STB binding = csym->binding();
  if (binding == elfcpp::STB_WEAK)
    binding = elfcpp::STB_GLOBAL;

  this->define_in_output_data(csym->name(), csym->version(), COPY,
			      posd, value, csym->symsize(),
			      csym->type(), binding,
			      csym->visibility(), csym->nonvis(),
			      false, false);

  csym->set_is_copied_from_dynobj();
  csym->set_needs_dynsym_entry();

  this->copied_symbol_dynobjs_[csym] = dynobj;

  // We have now defined all aliases, but we have not entered them all
  // in the copied_symbol_dynobjs_ map.
  if (csym->has_alias())
    {
      Symbol* sym = csym;
      while (true)
	{
	  sym = this->weak_aliases_[sym];
	  if (sym == csym)
	    break;
	  gold_assert(sym->output_data() == posd);

	  sym->set_is_copied_from_dynobj();
	  this->copied_symbol_dynobjs_[sym] = dynobj;
	}
    }
}

// SYM is defined using a COPY reloc.  Return the dynamic object where
// the original definition was found.

Dynobj*
Symbol_table::get_copy_source(const Symbol* sym) const
{
  gold_assert(sym->is_copied_from_dynobj());
  Copied_symbol_dynobjs::const_iterator p =
    this->copied_symbol_dynobjs_.find(sym);
  gold_assert(p != this->copied_symbol_dynobjs_.end());
  return p->second;
}

// Add any undefined symbols named on the command line.

void
Symbol_table::add_undefined_symbols_from_command_line(Layout* layout)
{
  if (parameters->options().any_undefined()
      || layout->script_options()->any_unreferenced())
    {
      if (parameters->target().get_size() == 32)
	{
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
	  this->do_add_undefined_symbols_from_command_line<32>(layout);
#else
	  gold_unreachable();
#endif
	}
      else if (parameters->target().get_size() == 64)
	{
#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
	  this->do_add_undefined_symbols_from_command_line<64>(layout);
#else
	  gold_unreachable();
#endif
	}
      else
	gold_unreachable();
    }
}

template<int size>
void
Symbol_table::do_add_undefined_symbols_from_command_line(Layout* layout)
{
  for (options::String_set::const_iterator p =
	 parameters->options().undefined_begin();
       p != parameters->options().undefined_end();
       ++p)
    this->add_undefined_symbol_from_command_line<size>(p->c_str());

  for (Script_options::referenced_const_iterator p =
	 layout->script_options()->referenced_begin();
       p != layout->script_options()->referenced_end();
       ++p)
    this->add_undefined_symbol_from_command_line<size>(p->c_str());
}

template<int size>
void
Symbol_table::add_undefined_symbol_from_command_line(const char* name)
{
  if (this->lookup(name) != NULL)
    return;

  const char* version = NULL;

  Sized_symbol<size>* sym;
  Sized_symbol<size>* oldsym;
  bool resolve_oldsym;
  if (parameters->target().is_big_endian())
    {
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_64_BIG)
      sym = this->define_special_symbol<size, true>(&name, &version,
						    false,
						    elfcpp::STV_DEFAULT,
						    &oldsym,
						    &resolve_oldsym,
						    false);
#else
      gold_unreachable();
#endif
    }
  else
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_64_LITTLE)
      sym = this->define_special_symbol<size, false>(&name, &version,
						     false,
						     elfcpp::STV_DEFAULT,
						     &oldsym,
						     &resolve_oldsym,
						     false);
#else
      gold_unreachable();
#endif
    }

  gold_assert(oldsym == NULL);

  sym->init_undefined(name, version, 0, elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
		      elfcpp::STV_DEFAULT, 0);
  ++this->saw_undefined_;
}

// Set the dynamic symbol indexes.  INDEX is the index of the first
// global dynamic symbol.  Pointers to the global symbols are stored
// into the vector SYMS.  The names are added to DYNPOOL.
// This returns an updated dynamic symbol index.

unsigned int
Symbol_table::set_dynsym_indexes(unsigned int index,
				 unsigned int* pforced_local_count,
				 std::vector<Symbol*>* syms,
				 Stringpool* dynpool,
				 Versions* versions)
{
  // First process all the symbols which have been forced to be local,
  // as they must appear before all global symbols.
  unsigned int forced_local_count = 0;
  for (Forced_locals::iterator p = this->forced_locals_.begin();
       p != this->forced_locals_.end();
       ++p)
    {
      Symbol* sym = *p;
      gold_assert(sym->is_forced_local());
      if (sym->has_dynsym_index())
        continue;
      if (!sym->should_add_dynsym_entry(this))
	sym->set_dynsym_index(-1U);
      else
        {
          sym->set_dynsym_index(index);
          ++index;
          ++forced_local_count;
	  dynpool->add(sym->name(), false, NULL);
	  if (sym->type() == elfcpp::STT_GNU_IFUNC)
	    this->set_has_gnu_output();
        }
    }
  *pforced_local_count = forced_local_count;

  // Allow a target to set dynsym indexes.
  if (parameters->target().has_custom_set_dynsym_indexes())
    {
      std::vector<Symbol*> dyn_symbols;
      for (Symbol_table_type::iterator p = this->table_.begin();
           p != this->table_.end();
           ++p)
        {
          Symbol* sym = p->second;
          if (sym->is_forced_local())
	    continue;
          if (!sym->should_add_dynsym_entry(this))
            sym->set_dynsym_index(-1U);
          else
	    {
	      dyn_symbols.push_back(sym);
	      if (sym->type() == elfcpp::STT_GNU_IFUNC
		  || (sym->binding() == elfcpp::STB_GNU_UNIQUE
		      && parameters->options().gnu_unique()))
		this->set_has_gnu_output();
	    }
        }

      return parameters->target().set_dynsym_indexes(&dyn_symbols, index, syms,
                                                     dynpool, versions, this);
    }

  for (Symbol_table_type::iterator p = this->table_.begin();
       p != this->table_.end();
       ++p)
    {
      Symbol* sym = p->second;

      if (sym->is_forced_local())
        continue;

      // Note that SYM may already have a dynamic symbol index, since
      // some symbols appear more than once in the symbol table, with
      // and without a version.

      if (!sym->should_add_dynsym_entry(this))
	sym->set_dynsym_index(-1U);
      else if (!sym->has_dynsym_index())
	{
	  sym->set_dynsym_index(index);
	  ++index;
	  syms->push_back(sym);
	  dynpool->add(sym->name(), false, NULL);
	  if (sym->type() == elfcpp::STT_GNU_IFUNC
	      || (sym->binding() == elfcpp::STB_GNU_UNIQUE
		  && parameters->options().gnu_unique()))
	    this->set_has_gnu_output();

	  // Record any version information, except those from
	  // as-needed libraries not seen to be needed.  Note that the
	  // is_needed state for such libraries can change in this loop.
	  if (sym->version() != NULL)
	    {
	      if (!sym->is_from_dynobj()
		  || !sym->object()->as_needed()
		  || sym->object()->is_needed())
		versions->record_version(this, dynpool, sym);
	      else
		{
		  if (parameters->options().warn_drop_version())
		    gold_warning(_("discarding version information for "
				   "%s@%s, defined in unused shared library %s "
				   "(linked with --as-needed)"),
				 sym->name(), sym->version(),
				 sym->object()->name().c_str());
		  sym->clear_version();
		}
	    }
	}
    }

  // Finish up the versions.  In some cases this may add new dynamic
  // symbols.
  index = versions->finalize(this, index, syms);

  // Process target-specific symbols.
  for (std::vector<Symbol*>::iterator p = this->target_symbols_.begin();
       p != this->target_symbols_.end();
       ++p)
    {
      (*p)->set_dynsym_index(index);
      ++index;
      syms->push_back(*p);
      dynpool->add((*p)->name(), false, NULL);
    }

  return index;
}

// Set the final values for all the symbols.  The index of the first
// global symbol in the output file is *PLOCAL_SYMCOUNT.  Record the
// file offset OFF.  Add their names to POOL.  Return the new file
// offset.  Update *PLOCAL_SYMCOUNT if necessary.  DYNOFF and
// DYN_GLOBAL_INDEX refer to the start of the symbols that will be
// written from the global symbol table in Symtab::write_globals(),
// which will include forced-local symbols.  DYN_GLOBAL_INDEX is
// not necessarily the same as the sh_info field for the .dynsym
// section, which will point to the first real global symbol.

off_t
Symbol_table::finalize(off_t off, off_t dynoff, size_t dyn_global_index,
		       size_t dyncount, Stringpool* pool,
		       unsigned int* plocal_symcount)
{
  off_t ret;

  gold_assert(*plocal_symcount != 0);
  this->first_global_index_ = *plocal_symcount;

  this->dynamic_offset_ = dynoff;
  this->first_dynamic_global_index_ = dyn_global_index;
  this->dynamic_count_ = dyncount;

  if (parameters->target().get_size() == 32)
    {
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_32_LITTLE)
      ret = this->sized_finalize<32>(off, pool, plocal_symcount);
#else
      gold_unreachable();
#endif
    }
  else if (parameters->target().get_size() == 64)
    {
#if defined(HAVE_TARGET_64_BIG) || defined(HAVE_TARGET_64_LITTLE)
      ret = this->sized_finalize<64>(off, pool, plocal_symcount);
#else
      gold_unreachable();
#endif
    }
  else
    gold_unreachable();

  if (this->has_gnu_output_)
    {
      Target* target = const_cast<Target*>(&parameters->target());
      if (target->osabi() == elfcpp::ELFOSABI_NONE)
	target->set_osabi(elfcpp::ELFOSABI_GNU);
    }

  // Now that we have the final symbol table, we can reliably note
  // which symbols should get warnings.
  this->warnings_.note_warnings(this);

  return ret;
}

// SYM is going into the symbol table at *PINDEX.  Add the name to
// POOL, update *PINDEX and *POFF.

template<int size>
void
Symbol_table::add_to_final_symtab(Symbol* sym, Stringpool* pool,
				  unsigned int* pindex, off_t* poff)
{
  sym->set_symtab_index(*pindex);
  if (sym->version() == NULL || !parameters->options().relocatable())
    pool->add(sym->name(), false, NULL);
  else
    pool->add(sym->versioned_name(), true, NULL);
  ++*pindex;
  *poff += elfcpp::Elf_sizes<size>::sym_size;
}

// Set the final value for all the symbols.  This is called after
// Layout::finalize, so all the output sections have their final
// address.

template<int size>
off_t
Symbol_table::sized_finalize(off_t off, Stringpool* pool,
			     unsigned int* plocal_symcount)
{
  off = align_address(off, size >> 3);
  this->offset_ = off;

  unsigned int index = *plocal_symcount;
  const unsigned int orig_index = index;

  // First do all the symbols which have been forced to be local, as
  // they must appear before all global symbols.
  for (Forced_locals::iterator p = this->forced_locals_.begin();
       p != this->forced_locals_.end();
       ++p)
    {
      Symbol* sym = *p;
      gold_assert(sym->is_forced_local());
      if (this->sized_finalize_symbol<size>(sym))
	{
	  this->add_to_final_symtab<size>(sym, pool, &index, &off);
	  ++*plocal_symcount;
	  if (sym->type() == elfcpp::STT_GNU_IFUNC)
	    this->set_has_gnu_output();
	}
    }

  // Now do all the remaining symbols.
  for (Symbol_table_type::iterator p = this->table_.begin();
       p != this->table_.end();
       ++p)
    {
      Symbol* sym = p->second;
      if (this->sized_finalize_symbol<size>(sym))
	{
	  this->add_to_final_symtab<size>(sym, pool, &index, &off);
	  if (sym->type() == elfcpp::STT_GNU_IFUNC
	      || (sym->binding() == elfcpp::STB_GNU_UNIQUE
		  && parameters->options().gnu_unique()))
	    this->set_has_gnu_output();
	}
    }

  // Now do target-specific symbols.
  for (std::vector<Symbol*>::iterator p = this->target_symbols_.begin();
       p != this->target_symbols_.end();
       ++p)
    {
      this->add_to_final_symtab<size>(*p, pool, &index, &off);
    }

  this->output_count_ = index - orig_index;

  return off;
}

// Compute the final value of SYM and store status in location PSTATUS.
// During relaxation, this may be called multiple times for a symbol to
// compute its would-be final value in each relaxation pass.

template<int size>
typename Sized_symbol<size>::Value_type
Symbol_table::compute_final_value(
    const Sized_symbol<size>* sym,
    Compute_final_value_status* pstatus) const
{
  typedef typename Sized_symbol<size>::Value_type Value_type;
  Value_type value;

  switch (sym->source())
    {
    case Symbol::FROM_OBJECT:
      {
	bool is_ordinary;
	unsigned int shndx = sym->shndx(&is_ordinary);

	if (!is_ordinary
	    && shndx != elfcpp::SHN_ABS
	    && !Symbol::is_common_shndx(shndx))
	  {
	    *pstatus = CFVS_UNSUPPORTED_SYMBOL_SECTION;
	    return 0;
	  }

	Object* symobj = sym->object();
	if (symobj->is_dynamic())
	  {
	    value = 0;
	    shndx = elfcpp::SHN_UNDEF;
	  }
	else if (symobj->pluginobj() != NULL)
	  {
	    value = 0;
	    shndx = elfcpp::SHN_UNDEF;
	  }
	else if (shndx == elfcpp::SHN_UNDEF)
	  value = 0;
	else if (!is_ordinary
		 && (shndx == elfcpp::SHN_ABS
		     || Symbol::is_common_shndx(shndx)))
	  value = sym->value();
	else
	  {
	    Relobj* relobj = static_cast<Relobj*>(symobj);
	    Output_section* os = relobj->output_section(shndx);

            if (this->is_section_folded(relobj, shndx))
              {
                gold_assert(os == NULL);
                // Get the os of the section it is folded onto.
                Section_id folded = this->icf_->get_folded_section(relobj,
                                                                   shndx);
                gold_assert(folded.first != NULL);
                Relobj* folded_obj = reinterpret_cast<Relobj*>(folded.first);
		unsigned folded_shndx = folded.second;

                os = folded_obj->output_section(folded_shndx);  
                gold_assert(os != NULL);

		// Replace (relobj, shndx) with canonical ICF input section.
		shndx = folded_shndx;
		relobj = folded_obj;
              }

            uint64_t secoff64 = relobj->output_section_offset(shndx);
 	    if (os == NULL)
	      {
                bool static_or_reloc = (parameters->doing_static_link() ||
                                        parameters->options().relocatable());
                gold_assert(static_or_reloc || sym->dynsym_index() == -1U);

		*pstatus = CFVS_NO_OUTPUT_SECTION;
		return 0;
	      }

            if (secoff64 == -1ULL)
              {
                // The section needs special handling (e.g., a merge section).

	        value = os->output_address(relobj, shndx, sym->value());
	      }
            else
              {
                Value_type secoff =
                  convert_types<Value_type, uint64_t>(secoff64);
	        if (sym->type() == elfcpp::STT_TLS)
	          value = sym->value() + os->tls_offset() + secoff;
	        else
	          value = sym->value() + os->address() + secoff;
	      }
	  }
      }
      break;

    case Symbol::IN_OUTPUT_DATA:
      {
	Output_data* od = sym->output_data();
	value = sym->value();
	if (sym->type() != elfcpp::STT_TLS)
	  value += od->address();
	else
	  {
	    Output_section* os = od->output_section();
	    gold_assert(os != NULL);
	    value += os->tls_offset() + (od->address() - os->address());
	  }
	if (sym->offset_is_from_end())
	  value += od->data_size();
      }
      break;

    case Symbol::IN_OUTPUT_SEGMENT:
      {
	Output_segment* os = sym->output_segment();
	value = sym->value();
        if (sym->type() != elfcpp::STT_TLS)
	  value += os->vaddr();
	switch (sym->offset_base())
	  {
	  case Symbol::SEGMENT_START:
	    break;
	  case Symbol::SEGMENT_END:
	    value += os->memsz();
	    break;
	  case Symbol::SEGMENT_BSS:
	    value += os->filesz();
	    break;
	  default:
	    gold_unreachable();
	  }
      }
      break;

    case Symbol::IS_CONSTANT:
      value = sym->value();
      break;

    case Symbol::IS_UNDEFINED:
      value = 0;
      break;

    default:
      gold_unreachable();
    }

  *pstatus = CFVS_OK;
  return value;
}

// Finalize the symbol SYM.  This returns true if the symbol should be
// added to the symbol table, false otherwise.

template<int size>
bool
Symbol_table::sized_finalize_symbol(Symbol* unsized_sym)
{
  typedef typename Sized_symbol<size>::Value_type Value_type;

  Sized_symbol<size>* sym = static_cast<Sized_symbol<size>*>(unsized_sym);

  // The default version of a symbol may appear twice in the symbol
  // table.  We only need to finalize it once.
  if (sym->has_symtab_index())
    return false;

  if (!sym->in_reg())
    {
      gold_assert(!sym->has_symtab_index());
      sym->set_symtab_index(-1U);
      gold_assert(sym->dynsym_index() == -1U);
      return false;
    }

  // If the symbol is only present on plugin files, the plugin decided we
  // don't need it.
  if (!sym->in_real_elf())
    {
      gold_assert(!sym->has_symtab_index());
      sym->set_symtab_index(-1U);
      return false;
    }

  // Compute final symbol value.
  Compute_final_value_status status;
  Value_type value = this->compute_final_value(sym, &status);

  switch (status)
    {
    case CFVS_OK:
      break;
    case CFVS_UNSUPPORTED_SYMBOL_SECTION:
      {
	bool is_ordinary;
	unsigned int shndx = sym->shndx(&is_ordinary);
	gold_error(_("%s: unsupported symbol section 0x%x"),
		   sym->demangled_name().c_str(), shndx);
      }
      break;
    case CFVS_NO_OUTPUT_SECTION:
      sym->set_symtab_index(-1U);
      return false;
    default:
      gold_unreachable();
    }

  sym->set_value(value);

  if (parameters->options().strip_all()
      || !parameters->options().should_retain_symbol(sym->name()))
    {
      sym->set_symtab_index(-1U);
      return false;
    }

  return true;
}

// Write out the global symbols.

void
Symbol_table::write_globals(const Stringpool* sympool,
			    const Stringpool* dynpool,
			    Output_symtab_xindex* symtab_xindex,
			    Output_symtab_xindex* dynsym_xindex,
			    Output_file* of) const
{
  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      this->sized_write_globals<32, false>(sympool, dynpool, symtab_xindex,
					   dynsym_xindex, of);
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      this->sized_write_globals<32, true>(sympool, dynpool, symtab_xindex,
					  dynsym_xindex, of);
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      this->sized_write_globals<64, false>(sympool, dynpool, symtab_xindex,
					   dynsym_xindex, of);
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      this->sized_write_globals<64, true>(sympool, dynpool, symtab_xindex,
					  dynsym_xindex, of);
      break;
#endif
    default:
      gold_unreachable();
    }
}

// Write out the global symbols.

template<int size, bool big_endian>
void
Symbol_table::sized_write_globals(const Stringpool* sympool,
				  const Stringpool* dynpool,
				  Output_symtab_xindex* symtab_xindex,
				  Output_symtab_xindex* dynsym_xindex,
				  Output_file* of) const
{
  const Target& target = parameters->target();

  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  const unsigned int output_count = this->output_count_;
  const section_size_type oview_size = output_count * sym_size;
  const unsigned int first_global_index = this->first_global_index_;
  unsigned char* psyms;
  if (this->offset_ == 0 || output_count == 0)
    psyms = NULL;
  else
    psyms = of->get_output_view(this->offset_, oview_size);

  const unsigned int dynamic_count = this->dynamic_count_;
  const section_size_type dynamic_size = dynamic_count * sym_size;
  const unsigned int first_dynamic_global_index =
    this->first_dynamic_global_index_;
  unsigned char* dynamic_view;
  if (this->dynamic_offset_ == 0 || dynamic_count == 0)
    dynamic_view = NULL;
  else
    dynamic_view = of->get_output_view(this->dynamic_offset_, dynamic_size);

  for (Symbol_table_type::const_iterator p = this->table_.begin();
       p != this->table_.end();
       ++p)
    {
      Sized_symbol<size>* sym = static_cast<Sized_symbol<size>*>(p->second);

      // Possibly warn about unresolved symbols in shared libraries.
      this->warn_about_undefined_dynobj_symbol(sym);

      unsigned int sym_index = sym->symtab_index();
      unsigned int dynsym_index;
      if (dynamic_view == NULL)
	dynsym_index = -1U;
      else
	dynsym_index = sym->dynsym_index();

      if (sym_index == -1U && dynsym_index == -1U)
	{
	  // This symbol is not included in the output file.
	  continue;
	}

      unsigned int shndx;
      typename elfcpp::Elf_types<size>::Elf_Addr sym_value = sym->value();
      typename elfcpp::Elf_types<size>::Elf_Addr dynsym_value = sym_value;
      elfcpp::STB binding = sym->binding();

      // If --weak-unresolved-symbols is set, change binding of unresolved
      // global symbols to STB_WEAK.
      if (parameters->options().weak_unresolved_symbols()
	  && binding == elfcpp::STB_GLOBAL
	  && sym->is_undefined())
	binding = elfcpp::STB_WEAK;

      // If --no-gnu-unique is set, change STB_GNU_UNIQUE to STB_GLOBAL.
      if (binding == elfcpp::STB_GNU_UNIQUE
	  && !parameters->options().gnu_unique())
	binding = elfcpp::STB_GLOBAL;

      switch (sym->source())
	{
	case Symbol::FROM_OBJECT:
	  {
	    bool is_ordinary;
	    unsigned int in_shndx = sym->shndx(&is_ordinary);

	    if (!is_ordinary
		&& in_shndx != elfcpp::SHN_ABS
		&& !Symbol::is_common_shndx(in_shndx))
	      {
		gold_error(_("%s: unsupported symbol section 0x%x"),
			   sym->demangled_name().c_str(), in_shndx);
		shndx = in_shndx;
	      }
	    else
	      {
		Object* symobj = sym->object();
		if (symobj->is_dynamic())
		  {
		    if (sym->needs_dynsym_value())
		      dynsym_value = target.dynsym_value(sym);
		    shndx = elfcpp::SHN_UNDEF;
		    if (sym->is_undef_binding_weak())
		      binding = elfcpp::STB_WEAK;
		    else
		      binding = elfcpp::STB_GLOBAL;
		  }
		else if (symobj->pluginobj() != NULL)
		  shndx = elfcpp::SHN_UNDEF;
		else if (in_shndx == elfcpp::SHN_UNDEF
			 || (!is_ordinary
			     && (in_shndx == elfcpp::SHN_ABS
				 || Symbol::is_common_shndx(in_shndx))))
		  shndx = in_shndx;
		else
		  {
		    Relobj* relobj = static_cast<Relobj*>(symobj);
		    Output_section* os = relobj->output_section(in_shndx);
                    if (this->is_section_folded(relobj, in_shndx))
                      {
                        // This global symbol must be written out even though
                        // it is folded.
                        // Get the os of the section it is folded onto.
                        Section_id folded =
                             this->icf_->get_folded_section(relobj, in_shndx);
                        gold_assert(folded.first !=NULL);
                        Relobj* folded_obj = 
                          reinterpret_cast<Relobj*>(folded.first);
                        os = folded_obj->output_section(folded.second);  
                        gold_assert(os != NULL);
                      }
		    gold_assert(os != NULL);
		    shndx = os->out_shndx();

		    if (shndx >= elfcpp::SHN_LORESERVE)
		      {
			if (sym_index != -1U)
			  symtab_xindex->add(sym_index, shndx);
			if (dynsym_index != -1U)
			  dynsym_xindex->add(dynsym_index, shndx);
			shndx = elfcpp::SHN_XINDEX;
		      }

		    // In object files symbol values are section
		    // relative.
		    if (parameters->options().relocatable())
		      sym_value -= os->address();
		  }
	      }
	  }
	  break;

	case Symbol::IN_OUTPUT_DATA:
	  {
	    Output_data* od = sym->output_data();

	    shndx = od->out_shndx();
	    if (shndx >= elfcpp::SHN_LORESERVE)
	      {
		if (sym_index != -1U)
		  symtab_xindex->add(sym_index, shndx);
		if (dynsym_index != -1U)
		  dynsym_xindex->add(dynsym_index, shndx);
		shndx = elfcpp::SHN_XINDEX;
	      }

	    // In object files symbol values are section
	    // relative.
	    if (parameters->options().relocatable())
	      {
		Output_section* os = od->output_section();
		gold_assert(os != NULL);
		sym_value -= os->address();
	      }
	  }
	  break;

	case Symbol::IN_OUTPUT_SEGMENT:
	  {
	    Output_segment* oseg = sym->output_segment();
	    Output_section* osect = oseg->first_section();
	    if (osect == NULL)
	      shndx = elfcpp::SHN_ABS;
	    else
	      shndx = osect->out_shndx();
	  }
	  break;

	case Symbol::IS_CONSTANT:
	  shndx = elfcpp::SHN_ABS;
	  break;

	case Symbol::IS_UNDEFINED:
	  shndx = elfcpp::SHN_UNDEF;
	  break;

	default:
	  gold_unreachable();
	}

      if (sym_index != -1U)
	{
	  sym_index -= first_global_index;
	  gold_assert(sym_index < output_count);
	  unsigned char* ps = psyms + (sym_index * sym_size);
	  this->sized_write_symbol<size, big_endian>(sym, sym_value, shndx,
						     binding, sympool, ps);
	}

      if (dynsym_index != -1U)
	{
	  dynsym_index -= first_dynamic_global_index;
	  gold_assert(dynsym_index < dynamic_count);
	  unsigned char* pd = dynamic_view + (dynsym_index * sym_size);
	  this->sized_write_symbol<size, big_endian>(sym, dynsym_value, shndx,
						     binding, dynpool, pd);
          // Allow a target to adjust dynamic symbol value.
          parameters->target().adjust_dyn_symbol(sym, pd);
	}
    }

  // Write the target-specific symbols.
  for (std::vector<Symbol*>::const_iterator p = this->target_symbols_.begin();
       p != this->target_symbols_.end();
       ++p)
    {
      Sized_symbol<size>* sym = static_cast<Sized_symbol<size>*>(*p);

      unsigned int sym_index = sym->symtab_index();
      unsigned int dynsym_index;
      if (dynamic_view == NULL)
	dynsym_index = -1U;
      else
	dynsym_index = sym->dynsym_index();

      unsigned int shndx;
      switch (sym->source())
	{
	case Symbol::IS_CONSTANT:
	  shndx = elfcpp::SHN_ABS;
	  break;
	case Symbol::IS_UNDEFINED:
	  shndx = elfcpp::SHN_UNDEF;
	  break;
	default:
	  gold_unreachable();
	}

      if (sym_index != -1U)
	{
	  sym_index -= first_global_index;
	  gold_assert(sym_index < output_count);
	  unsigned char* ps = psyms + (sym_index * sym_size);
	  this->sized_write_symbol<size, big_endian>(sym, sym->value(), shndx,
						     sym->binding(), sympool,
						     ps);
	}

      if (dynsym_index != -1U)
	{
	  dynsym_index -= first_dynamic_global_index;
	  gold_assert(dynsym_index < dynamic_count);
	  unsigned char* pd = dynamic_view + (dynsym_index * sym_size);
	  this->sized_write_symbol<size, big_endian>(sym, sym->value(), shndx,
						     sym->binding(), dynpool,
						     pd);
	}
    }

  of->write_output_view(this->offset_, oview_size, psyms);
  if (dynamic_view != NULL)
    of->write_output_view(this->dynamic_offset_, dynamic_size, dynamic_view);
}

// Write out the symbol SYM, in section SHNDX, to P.  POOL is the
// strtab holding the name.

template<int size, bool big_endian>
void
Symbol_table::sized_write_symbol(
    Sized_symbol<size>* sym,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    unsigned int shndx,
    elfcpp::STB binding,
    const Stringpool* pool,
    unsigned char* p) const
{
  elfcpp::Sym_write<size, big_endian> osym(p);
  if (sym->version() == NULL || !parameters->options().relocatable())
    osym.put_st_name(pool->get_offset(sym->name()));
  else
    osym.put_st_name(pool->get_offset(sym->versioned_name()));
  osym.put_st_value(value);
  // Use a symbol size of zero for undefined symbols from shared libraries.
  if (shndx == elfcpp::SHN_UNDEF && sym->is_from_dynobj())
    osym.put_st_size(0);
  else
    osym.put_st_size(sym->symsize());
  elfcpp::STT type = sym->type();
  gold_assert(type != elfcpp::STT_GNU_IFUNC || !sym->is_from_dynobj());
  // A version script may have overridden the default binding.
  if (sym->is_forced_local())
    osym.put_st_info(elfcpp::elf_st_info(elfcpp::STB_LOCAL, type));
  else
    osym.put_st_info(elfcpp::elf_st_info(binding, type));
  osym.put_st_other(elfcpp::elf_st_other(sym->visibility(), sym->nonvis()));
  osym.put_st_shndx(shndx);
}

// Check for unresolved symbols in shared libraries.  This is
// controlled by the --allow-shlib-undefined option.

// We only warn about libraries for which we have seen all the
// DT_NEEDED entries.  We don't try to track down DT_NEEDED entries
// which were not seen in this link.  If we didn't see a DT_NEEDED
// entry, we aren't going to be able to reliably report whether the
// symbol is undefined.

// We also don't warn about libraries found in a system library
// directory (e.g., /lib or /usr/lib); we assume that those libraries
// are OK.  This heuristic avoids problems on GNU/Linux, in which -ldl
// can have undefined references satisfied by ld-linux.so.

inline void
Symbol_table::warn_about_undefined_dynobj_symbol(Symbol* sym) const
{
  bool dummy;
  if (sym->source() == Symbol::FROM_OBJECT
      && sym->object()->is_dynamic()
      && sym->shndx(&dummy) == elfcpp::SHN_UNDEF
      && sym->binding() != elfcpp::STB_WEAK
      && !parameters->options().allow_shlib_undefined()
      && !parameters->target().is_defined_by_abi(sym)
      && !sym->object()->is_in_system_directory())
    {
      // A very ugly cast.
      Dynobj* dynobj = static_cast<Dynobj*>(sym->object());
      if (!dynobj->has_unknown_needed_entries())
        gold_undefined_symbol(sym);
    }
}

// Write out a section symbol.  Return the update offset.

void
Symbol_table::write_section_symbol(const Output_section* os,
				   Output_symtab_xindex* symtab_xindex,
				   Output_file* of,
				   off_t offset) const
{
  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      this->sized_write_section_symbol<32, false>(os, symtab_xindex, of,
						  offset);
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      this->sized_write_section_symbol<32, true>(os, symtab_xindex, of,
						 offset);
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      this->sized_write_section_symbol<64, false>(os, symtab_xindex, of,
						  offset);
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      this->sized_write_section_symbol<64, true>(os, symtab_xindex, of,
						 offset);
      break;
#endif
    default:
      gold_unreachable();
    }
}

// Write out a section symbol, specialized for size and endianness.

template<int size, bool big_endian>
void
Symbol_table::sized_write_section_symbol(const Output_section* os,
					 Output_symtab_xindex* symtab_xindex,
					 Output_file* of,
					 off_t offset) const
{
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  unsigned char* pov = of->get_output_view(offset, sym_size);

  elfcpp::Sym_write<size, big_endian> osym(pov);
  osym.put_st_name(0);
  if (parameters->options().relocatable())
    osym.put_st_value(0);
  else
    osym.put_st_value(os->address());
  osym.put_st_size(0);
  osym.put_st_info(elfcpp::elf_st_info(elfcpp::STB_LOCAL,
				       elfcpp::STT_SECTION));
  osym.put_st_other(elfcpp::elf_st_other(elfcpp::STV_DEFAULT, 0));

  unsigned int shndx = os->out_shndx();
  if (shndx >= elfcpp::SHN_LORESERVE)
    {
      symtab_xindex->add(os->symtab_index(), shndx);
      shndx = elfcpp::SHN_XINDEX;
    }
  osym.put_st_shndx(shndx);

  of->write_output_view(offset, sym_size, pov);
}

// Print statistical information to stderr.  This is used for --stats.

void
Symbol_table::print_stats() const
{
#if defined(HAVE_TR1_UNORDERED_MAP) || defined(HAVE_EXT_HASH_MAP)
  fprintf(stderr, _("%s: symbol table entries: %zu; buckets: %zu\n"),
	  program_name, this->table_.size(), this->table_.bucket_count());
#else
  fprintf(stderr, _("%s: symbol table entries: %zu\n"),
	  program_name, this->table_.size());
#endif
  this->namepool_.print_stats("symbol table stringpool");
}

// We check for ODR violations by looking for symbols with the same
// name for which the debugging information reports that they were
// defined in disjoint source locations.  When comparing the source
// location, we consider instances with the same base filename to be
// the same.  This is because different object files/shared libraries
// can include the same header file using different paths, and
// different optimization settings can make the line number appear to
// be a couple lines off, and we don't want to report an ODR violation
// in those cases.

// This struct is used to compare line information, as returned by
// Dwarf_line_info::one_addr2line.  It implements a < comparison
// operator used with std::sort.

struct Odr_violation_compare
{
  bool
  operator()(const std::string& s1, const std::string& s2) const
  {
    // Inputs should be of the form "dirname/filename:linenum" where
    // "dirname/" is optional.  We want to compare just the filename:linenum.

    // Find the last '/' in each string.
    std::string::size_type s1begin = s1.rfind('/');
    std::string::size_type s2begin = s2.rfind('/');
    // If there was no '/' in a string, start at the beginning.
    if (s1begin == std::string::npos)
      s1begin = 0;
    if (s2begin == std::string::npos)
      s2begin = 0;
    return s1.compare(s1begin, std::string::npos,
		      s2, s2begin, std::string::npos) < 0;
  }
};

// Returns all of the lines attached to LOC, not just the one the
// instruction actually came from.
std::vector<std::string>
Symbol_table::linenos_from_loc(const Task* task,
                               const Symbol_location& loc)
{
  // We need to lock the object in order to read it.  This
  // means that we have to run in a singleton Task.  If we
  // want to run this in a general Task for better
  // performance, we will need one Task for object, plus
  // appropriate locking to ensure that we don't conflict with
  // other uses of the object.  Also note, one_addr2line is not
  // currently thread-safe.
  Task_lock_obj<Object> tl(task, loc.object);

  std::vector<std::string> result;
  Symbol_location code_loc = loc;
  parameters->target().function_location(&code_loc);
  // 16 is the size of the object-cache that one_addr2line should use.
  std::string canonical_result = Dwarf_line_info::one_addr2line(
      code_loc.object, code_loc.shndx, code_loc.offset, 16, &result);
  if (!canonical_result.empty())
    result.push_back(canonical_result);
  return result;
}

// OutputIterator that records if it was ever assigned to.  This
// allows it to be used with std::set_intersection() to check for
// intersection rather than computing the intersection.
struct Check_intersection
{
  Check_intersection()
    : value_(false)
  {}

  bool had_intersection() const
  { return this->value_; }

  Check_intersection& operator++()
  { return *this; }

  Check_intersection& operator*()
  { return *this; }

  template<typename T>
  Check_intersection& operator=(const T&)
  {
    this->value_ = true;
    return *this;
  }

 private:
  bool value_;
};

// Check candidate_odr_violations_ to find symbols with the same name
// but apparently different definitions (different source-file/line-no
// for each line assigned to the first instruction).

void
Symbol_table::detect_odr_violations(const Task* task,
				    const char* output_file_name) const
{
  for (Odr_map::const_iterator it = candidate_odr_violations_.begin();
       it != candidate_odr_violations_.end();
       ++it)
    {
      const char* const symbol_name = it->first;

      std::string first_object_name;
      std::vector<std::string> first_object_linenos;

      Unordered_set<Symbol_location, Symbol_location_hash>::const_iterator
          locs = it->second.begin();
      const Unordered_set<Symbol_location, Symbol_location_hash>::const_iterator
          locs_end = it->second.end();
      for (; locs != locs_end && first_object_linenos.empty(); ++locs)
        {
          // Save the line numbers from the first definition to
          // compare to the other definitions.  Ideally, we'd compare
          // every definition to every other, but we don't want to
          // take O(N^2) time to do this.  This shortcut may cause
          // false negatives that appear or disappear depending on the
          // link order, but it won't cause false positives.
          first_object_name = locs->object->name();
          first_object_linenos = this->linenos_from_loc(task, *locs);
        }
      if (first_object_linenos.empty())
	continue;

      // Sort by Odr_violation_compare to make std::set_intersection work.
      std::string first_object_canonical_result = first_object_linenos.back();
      std::sort(first_object_linenos.begin(), first_object_linenos.end(),
                Odr_violation_compare());

      for (; locs != locs_end; ++locs)
        {
          std::vector<std::string> linenos =
              this->linenos_from_loc(task, *locs);
          // linenos will be empty if we couldn't parse the debug info.
          if (linenos.empty())
            continue;
          // Sort by Odr_violation_compare to make std::set_intersection work.
          gold_assert(!linenos.empty());
          std::string second_object_canonical_result = linenos.back();
          std::sort(linenos.begin(), linenos.end(), Odr_violation_compare());

          Check_intersection intersection_result =
              std::set_intersection(first_object_linenos.begin(),
                                    first_object_linenos.end(),
                                    linenos.begin(),
                                    linenos.end(),
                                    Check_intersection(),
                                    Odr_violation_compare());
          if (!intersection_result.had_intersection())
            {
              gold_warning(_("while linking %s: symbol '%s' defined in "
                             "multiple places (possible ODR violation):"),
                           output_file_name, demangle(symbol_name).c_str());
              // This only prints one location from each definition,
              // which may not be the location we expect to intersect
              // with another definition.  We could print the whole
              // set of locations, but that seems too verbose.
              fprintf(stderr, _("  %s from %s\n"),
                      first_object_canonical_result.c_str(),
                      first_object_name.c_str());
              fprintf(stderr, _("  %s from %s\n"),
                      second_object_canonical_result.c_str(),
                      locs->object->name().c_str());
              // Only print one broken pair, to avoid needing to
              // compare against a list of the disjoint definition
              // locations we've found so far.  (If we kept comparing
              // against just the first one, we'd get a lot of
              // redundant complaints about the second definition
              // location.)
              break;
            }
        }
    }
  // We only call one_addr2line() in this function, so we can clear its cache.
  Dwarf_line_info::clear_addr2line_cache();
}

// Warnings functions.

// Add a new warning.

void
Warnings::add_warning(Symbol_table* symtab, const char* name, Object* obj,
		      const std::string& warning)
{
  name = symtab->canonicalize_name(name);
  this->warnings_[name].set(obj, warning);
}

// Look through the warnings and mark the symbols for which we should
// warn.  This is called during Layout::finalize when we know the
// sources for all the symbols.

void
Warnings::note_warnings(Symbol_table* symtab)
{
  for (Warning_table::iterator p = this->warnings_.begin();
       p != this->warnings_.end();
       ++p)
    {
      Symbol* sym = symtab->lookup(p->first, NULL);
      if (sym != NULL
	  && sym->source() == Symbol::FROM_OBJECT
	  && sym->object() == p->second.object)
	sym->set_has_warning();
    }
}

// Issue a warning.  This is called when we see a relocation against a
// symbol for which has a warning.

template<int size, bool big_endian>
void
Warnings::issue_warning(const Symbol* sym,
			const Relocate_info<size, big_endian>* relinfo,
			size_t relnum, off_t reloffset) const
{
  gold_assert(sym->has_warning());

  // We don't want to issue a warning for a relocation against the
  // symbol in the same object file in which the symbol is defined.
  if (sym->object() == relinfo->object)
    return;

  Warning_table::const_iterator p = this->warnings_.find(sym->name());
  gold_assert(p != this->warnings_.end());
  gold_warning_at_location(relinfo, relnum, reloffset,
			   "%s", p->second.text.c_str());
}

// Instantiate the templates we need.  We could use the configure
// script to restrict this to only the ones needed for implemented
// targets.

#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
template
void
Sized_symbol<32>::allocate_common(Output_data*, Value_type);
#endif

#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
template
void
Sized_symbol<64>::allocate_common(Output_data*, Value_type);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Symbol_table::add_from_relobj<32, false>(
    Sized_relobj_file<32, false>* relobj,
    const unsigned char* syms,
    size_t count,
    size_t symndx_offset,
    const char* sym_names,
    size_t sym_name_size,
    Sized_relobj_file<32, false>::Symbols* sympointers,
    size_t* defined);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Symbol_table::add_from_relobj<32, true>(
    Sized_relobj_file<32, true>* relobj,
    const unsigned char* syms,
    size_t count,
    size_t symndx_offset,
    const char* sym_names,
    size_t sym_name_size,
    Sized_relobj_file<32, true>::Symbols* sympointers,
    size_t* defined);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Symbol_table::add_from_relobj<64, false>(
    Sized_relobj_file<64, false>* relobj,
    const unsigned char* syms,
    size_t count,
    size_t symndx_offset,
    const char* sym_names,
    size_t sym_name_size,
    Sized_relobj_file<64, false>::Symbols* sympointers,
    size_t* defined);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Symbol_table::add_from_relobj<64, true>(
    Sized_relobj_file<64, true>* relobj,
    const unsigned char* syms,
    size_t count,
    size_t symndx_offset,
    const char* sym_names,
    size_t sym_name_size,
    Sized_relobj_file<64, true>::Symbols* sympointers,
    size_t* defined);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
Symbol*
Symbol_table::add_from_pluginobj<32, false>(
    Sized_pluginobj<32, false>* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<32, false>* sym);
#endif

#ifdef HAVE_TARGET_32_BIG
template
Symbol*
Symbol_table::add_from_pluginobj<32, true>(
    Sized_pluginobj<32, true>* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<32, true>* sym);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
Symbol*
Symbol_table::add_from_pluginobj<64, false>(
    Sized_pluginobj<64, false>* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<64, false>* sym);
#endif

#ifdef HAVE_TARGET_64_BIG
template
Symbol*
Symbol_table::add_from_pluginobj<64, true>(
    Sized_pluginobj<64, true>* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<64, true>* sym);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Symbol_table::add_from_dynobj<32, false>(
    Sized_dynobj<32, false>* dynobj,
    const unsigned char* syms,
    size_t count,
    const char* sym_names,
    size_t sym_name_size,
    const unsigned char* versym,
    size_t versym_size,
    const std::vector<const char*>* version_map,
    Sized_relobj_file<32, false>::Symbols* sympointers,
    size_t* defined);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Symbol_table::add_from_dynobj<32, true>(
    Sized_dynobj<32, true>* dynobj,
    const unsigned char* syms,
    size_t count,
    const char* sym_names,
    size_t sym_name_size,
    const unsigned char* versym,
    size_t versym_size,
    const std::vector<const char*>* version_map,
    Sized_relobj_file<32, true>::Symbols* sympointers,
    size_t* defined);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Symbol_table::add_from_dynobj<64, false>(
    Sized_dynobj<64, false>* dynobj,
    const unsigned char* syms,
    size_t count,
    const char* sym_names,
    size_t sym_name_size,
    const unsigned char* versym,
    size_t versym_size,
    const std::vector<const char*>* version_map,
    Sized_relobj_file<64, false>::Symbols* sympointers,
    size_t* defined);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Symbol_table::add_from_dynobj<64, true>(
    Sized_dynobj<64, true>* dynobj,
    const unsigned char* syms,
    size_t count,
    const char* sym_names,
    size_t sym_name_size,
    const unsigned char* versym,
    size_t versym_size,
    const std::vector<const char*>* version_map,
    Sized_relobj_file<64, true>::Symbols* sympointers,
    size_t* defined);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
Sized_symbol<32>*
Symbol_table::add_from_incrobj(
    Object* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<32, false>* sym);
#endif

#ifdef HAVE_TARGET_32_BIG
template
Sized_symbol<32>*
Symbol_table::add_from_incrobj(
    Object* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<32, true>* sym);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
Sized_symbol<64>*
Symbol_table::add_from_incrobj(
    Object* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<64, false>* sym);
#endif

#ifdef HAVE_TARGET_64_BIG
template
Sized_symbol<64>*
Symbol_table::add_from_incrobj(
    Object* obj,
    const char* name,
    const char* ver,
    elfcpp::Sym<64, true>* sym);
#endif

#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
template
void
Symbol_table::define_with_copy_reloc<32>(
    Sized_symbol<32>* sym,
    Output_data* posd,
    elfcpp::Elf_types<32>::Elf_Addr value);
#endif

#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
template
void
Symbol_table::define_with_copy_reloc<64>(
    Sized_symbol<64>* sym,
    Output_data* posd,
    elfcpp::Elf_types<64>::Elf_Addr value);
#endif

#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
template
void
Sized_symbol<32>::init_output_data(const char* name, const char* version,
				   Output_data* od, Value_type value,
				   Size_type symsize, elfcpp::STT type,
				   elfcpp::STB binding,
				   elfcpp::STV visibility,
				   unsigned char nonvis,
				   bool offset_is_from_end,
				   bool is_predefined);

template
void
Sized_symbol<32>::init_constant(const char* name, const char* version,
				Value_type value, Size_type symsize,
				elfcpp::STT type, elfcpp::STB binding,
				elfcpp::STV visibility, unsigned char nonvis,
				bool is_predefined);

template
void
Sized_symbol<32>::init_undefined(const char* name, const char* version,
				 Value_type value, elfcpp::STT type,
				 elfcpp::STB binding, elfcpp::STV visibility,
				 unsigned char nonvis);
#endif

#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
template
void
Sized_symbol<64>::init_output_data(const char* name, const char* version,
				   Output_data* od, Value_type value,
				   Size_type symsize, elfcpp::STT type,
				   elfcpp::STB binding,
				   elfcpp::STV visibility,
				   unsigned char nonvis,
				   bool offset_is_from_end,
				   bool is_predefined);

template
void
Sized_symbol<64>::init_constant(const char* name, const char* version,
				Value_type value, Size_type symsize,
				elfcpp::STT type, elfcpp::STB binding,
				elfcpp::STV visibility, unsigned char nonvis,
				bool is_predefined);

template
void
Sized_symbol<64>::init_undefined(const char* name, const char* version,
				 Value_type value, elfcpp::STT type,
				 elfcpp::STB binding, elfcpp::STV visibility,
				 unsigned char nonvis);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Warnings::issue_warning<32, false>(const Symbol* sym,
				   const Relocate_info<32, false>* relinfo,
				   size_t relnum, off_t reloffset) const;
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Warnings::issue_warning<32, true>(const Symbol* sym,
				  const Relocate_info<32, true>* relinfo,
				  size_t relnum, off_t reloffset) const;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Warnings::issue_warning<64, false>(const Symbol* sym,
				   const Relocate_info<64, false>* relinfo,
				   size_t relnum, off_t reloffset) const;
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Warnings::issue_warning<64, true>(const Symbol* sym,
				  const Relocate_info<64, true>* relinfo,
				  size_t relnum, off_t reloffset) const;
#endif

} // End namespace gold.
