// gdb-index.cc -- generate .gdb_index section for fast debug lookup

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
// Written by Cary Coutant <ccoutant@google.com>.

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

#include "gdb-index.h"
#include "dwarf_reader.h"
#include "dwarf.h"
#include "object.h"
#include "output.h"
#include "demangle.h"

namespace gold
{

const int gdb_index_version = 7;

// Sizes of various records in the .gdb_index section.
const int gdb_index_offset_size = 4;
const int gdb_index_hdr_size = 6 * gdb_index_offset_size;
const int gdb_index_cu_size = 16;
const int gdb_index_tu_size = 24;
const int gdb_index_addr_size = 16 + gdb_index_offset_size;
const int gdb_index_sym_size = 2 * gdb_index_offset_size;

// This class manages the hashed symbol table for the .gdb_index section.
// It is essentially equivalent to the hashtab implementation in libiberty,
// but is copied into gdb sources and here for compatibility because its
// data structure is exposed on disk.

template <typename T>
class Gdb_hashtab
{
 public:
  Gdb_hashtab()
    : size_(0), capacity_(0), hashtab_(NULL)
  { }

  ~Gdb_hashtab()
  {
    for (size_t i = 0; i < this->capacity_; ++i)
      if (this->hashtab_[i] != NULL)
	delete this->hashtab_[i];
    delete[] this->hashtab_;
  }

  // Add a symbol.
  T*
  add(T* symbol)
  {
    // Resize the hash table if necessary.
    if (4 * this->size_ / 3 >= this->capacity_)
      this->expand();

    T** slot = this->find_slot(symbol);
    if (*slot == NULL)
      {
	++this->size_;
	*slot = symbol;
      }

    return *slot;
  }

  // Return the current size.
  size_t
  size() const
  { return this->size_; }

  // Return the current capacity.
  size_t
  capacity() const
  { return this->capacity_; }

  // Return the contents of slot N.
  T*
  operator[](size_t n)
  { return this->hashtab_[n]; }

 private:
  // Find a symbol in the hash table, or return an empty slot if
  // the symbol is not in the table.
  T**
  find_slot(T* symbol)
  {
    unsigned int index = symbol->hash() & (this->capacity_ - 1);
    unsigned int step = ((symbol->hash() * 17) & (this->capacity_ - 1)) | 1;

    for (;;)
      {
	if (this->hashtab_[index] == NULL
	    || this->hashtab_[index]->equal(symbol))
	  return &this->hashtab_[index];
	index = (index + step) & (this->capacity_ - 1);
      }
  }

  // Expand the hash table.
  void
  expand()
  {
    if (this->capacity_ == 0)
      {
	// Allocate the hash table for the first time.
	this->capacity_ = Gdb_hashtab::initial_size;
	this->hashtab_ = new T*[this->capacity_];
	memset(this->hashtab_, 0, this->capacity_ * sizeof(T*));
      }
    else
      {
	// Expand and rehash.
	unsigned int old_cap = this->capacity_;
	T** old_hashtab = this->hashtab_;
	this->capacity_ *= 2;
	this->hashtab_ = new T*[this->capacity_];
	memset(this->hashtab_, 0, this->capacity_ * sizeof(T*));
	for (size_t i = 0; i < old_cap; ++i)
	  {
	    if (old_hashtab[i] != NULL)
	      {
		T** slot = this->find_slot(old_hashtab[i]);
		*slot = old_hashtab[i];
	      }
	  }
	delete[] old_hashtab;
      }
  }

  // Initial size of the hash table; must be a power of 2.
  static const int initial_size = 1024;
  size_t size_;
  size_t capacity_;
  T** hashtab_;
};

// The hash function for strings in the mapped index.  This is copied
// directly from gdb/dwarf2read.c.

static unsigned int
mapped_index_string_hash(const unsigned char* str)
{
  unsigned int r = 0;
  unsigned char c;

  while ((c = *str++) != 0)
    {
      if (gdb_index_version >= 5)
	c = tolower (c);
      r = r * 67 + c - 113;
    }

  return r;
}

// A specialization of Dwarf_info_reader, for building the .gdb_index.

class Gdb_index_info_reader : public Dwarf_info_reader
{
 public:
  Gdb_index_info_reader(bool is_type_unit,
			Relobj* object,
			const unsigned char* symbols,
			off_t symbols_size,
			unsigned int shndx,
			unsigned int reloc_shndx,
			unsigned int reloc_type,
			Gdb_index* gdb_index)
    : Dwarf_info_reader(is_type_unit, object, symbols, symbols_size, shndx,
			reloc_shndx, reloc_type),
      gdb_index_(gdb_index), cu_index_(0), cu_language_(0)
  { }

  ~Gdb_index_info_reader()
  { this->clear_declarations(); }

  // Print usage statistics.
  static void
  print_stats();

 protected:
  // Visit a compilation unit.
  virtual void
  visit_compilation_unit(off_t cu_offset, off_t cu_length, Dwarf_die*);

  // Visit a type unit.
  virtual void
  visit_type_unit(off_t tu_offset, off_t tu_length, off_t type_offset,
		  uint64_t signature, Dwarf_die*);

 private:
  // A map for recording DIEs we've seen that may be referred to be
  // later DIEs (via DW_AT_specification or DW_AT_abstract_origin).
  // The map is indexed by a DIE offset within the compile unit.
  // PARENT_OFFSET_ is the offset of the DIE that represents the
  // outer context, and NAME_ is a pointer to a component of the
  // fully-qualified name.
  // Normally, the names we point to are in a string table, so we don't
  // have to manage them, but when we have a fully-qualified name
  // computed, we put it in the table, and set PARENT_OFFSET_ to -1
  // indicate a string that we are managing.
  struct Declaration_pair
  {
    Declaration_pair(off_t parent_offset, const char* name)
      : parent_offset_(parent_offset), name_(name)
    { }

    off_t parent_offset_;
    const char* name_; 
  };
  typedef Unordered_map<off_t, Declaration_pair> Declaration_map;

  // Visit a top-level DIE.
  void
  visit_top_die(Dwarf_die* die);

  // Visit the children of a DIE.
  void
  visit_children(Dwarf_die* die, Dwarf_die* context);

  // Visit a DIE.
  void
  visit_die(Dwarf_die* die, Dwarf_die* context);

  // Visit the children of a DIE.
  void
  visit_children_for_decls(Dwarf_die* die);

  // Visit a DIE.
  void
  visit_die_for_decls(Dwarf_die* die, Dwarf_die* context);

  // Guess a fully-qualified name for a class type, based on member function
  // linkage names.
  std::string
  guess_full_class_name(Dwarf_die* die);

  // Add a declaration DIE to the table of declarations.
  void
  add_declaration(Dwarf_die* die, Dwarf_die* context);

  // Add a declaration whose fully-qualified name is already known.
  void
  add_declaration_with_full_name(Dwarf_die* die, const char* full_name);

  // Return the context for a DIE whose parent is at DIE_OFFSET.
  std::string
  get_context(off_t die_offset);

  // Construct a fully-qualified name for DIE.
  std::string
  get_qualified_name(Dwarf_die* die, Dwarf_die* context);

  // Record the address ranges for a compilation unit.
  void
  record_cu_ranges(Dwarf_die* die);

  // Wrapper for read_pubtable.
  bool
  read_pubnames_and_pubtypes(Dwarf_die* die);

  // Read the .debug_pubnames and .debug_pubtypes tables.
  bool
  read_pubtable(Dwarf_pubnames_table* table, off_t offset);

  // Clear the declarations map.
  void
  clear_declarations();

  // The Gdb_index section.
  Gdb_index* gdb_index_;
  // The current CU index (negative for a TU).
  int cu_index_;
  // The language of the current CU or TU.
  unsigned int cu_language_;
  // Map from DIE offset to (parent offset, name) pair,
  // for DW_AT_specification.
  Declaration_map declarations_;

  // Statistics.
  // Total number of DWARF compilation units processed.
  static unsigned int dwarf_cu_count;
  // Number of DWARF compilation units with pubnames/pubtypes.
  static unsigned int dwarf_cu_nopubnames_count;
  // Total number of DWARF type units processed.
  static unsigned int dwarf_tu_count;
  // Number of DWARF type units with pubnames/pubtypes.
  static unsigned int dwarf_tu_nopubnames_count;
};

// Total number of DWARF compilation units processed.
unsigned int Gdb_index_info_reader::dwarf_cu_count = 0;
// Number of DWARF compilation units without pubnames/pubtypes.
unsigned int Gdb_index_info_reader::dwarf_cu_nopubnames_count = 0;
// Total number of DWARF type units processed.
unsigned int Gdb_index_info_reader::dwarf_tu_count = 0;
// Number of DWARF type units without pubnames/pubtypes.
unsigned int Gdb_index_info_reader::dwarf_tu_nopubnames_count = 0;

// Process a compilation unit and parse its child DIE.

void
Gdb_index_info_reader::visit_compilation_unit(off_t cu_offset, off_t cu_length,
					      Dwarf_die* root_die)
{
  ++Gdb_index_info_reader::dwarf_cu_count;
  this->cu_index_ = this->gdb_index_->add_comp_unit(cu_offset, cu_length);
  this->visit_top_die(root_die);
}

// Process a type unit and parse its child DIE.

void
Gdb_index_info_reader::visit_type_unit(off_t tu_offset, off_t,
				       off_t type_offset, uint64_t signature,
				       Dwarf_die* root_die)
{
  ++Gdb_index_info_reader::dwarf_tu_count;
  // Use a negative index to flag this as a TU instead of a CU.
  this->cu_index_ = -1 - this->gdb_index_->add_type_unit(tu_offset, type_offset,
							 signature);
  this->visit_top_die(root_die);
}

// Process a top-level DIE.
// For compile_unit DIEs, record the address ranges.  For all
// interesting tags, add qualified names to the symbol table
// and process interesting children.  We may need to process
// certain children just for saving declarations that might be
// referenced by later DIEs with a DW_AT_specification attribute.

void
Gdb_index_info_reader::visit_top_die(Dwarf_die* die)
{
  this->clear_declarations();

  switch (die->tag())
    {
      case elfcpp::DW_TAG_compile_unit:
      case elfcpp::DW_TAG_type_unit:
	this->cu_language_ = die->int_attribute(elfcpp::DW_AT_language);
	if (die->tag() == elfcpp::DW_TAG_compile_unit)
	  this->record_cu_ranges(die);
	// If there is a pubnames and/or pubtypes section for this
	// compilation unit, use those; otherwise, parse the DWARF
	// info to extract the names.
	if (!this->read_pubnames_and_pubtypes(die))
	  {
	    // Check for languages that require specialized knowledge to
	    // construct fully-qualified names, that we don't yet support.
	    if (this->cu_language_ == elfcpp::DW_LANG_Ada83
		|| this->cu_language_ == elfcpp::DW_LANG_Fortran77
		|| this->cu_language_ == elfcpp::DW_LANG_Fortran90
		|| this->cu_language_ == elfcpp::DW_LANG_Java
		|| this->cu_language_ == elfcpp::DW_LANG_Ada95
		|| this->cu_language_ == elfcpp::DW_LANG_Fortran95
		|| this->cu_language_ == elfcpp::DW_LANG_Fortran03
		|| this->cu_language_ == elfcpp::DW_LANG_Fortran08)
	      {
		gold_warning(_("%s: --gdb-index currently supports "
			       "only C and C++ languages"),
			     this->object()->name().c_str());
		return;
	      }
	    if (die->tag() == elfcpp::DW_TAG_compile_unit)
	      ++Gdb_index_info_reader::dwarf_cu_nopubnames_count;
	    else
	      ++Gdb_index_info_reader::dwarf_tu_nopubnames_count;
	    this->visit_children(die, NULL);
	  }
	break;
      default:
	// The top level DIE should be one of the above.
	gold_warning(_("%s: top level DIE is not DW_TAG_compile_unit "
		       "or DW_TAG_type_unit"),
		     this->object()->name().c_str());
	return;
    }
}

// Visit the children of PARENT, looking for symbols to add to the index.
// CONTEXT points to the DIE to use for constructing the qualified name --
// NULL if PARENT is the top-level DIE; otherwise it is the same as PARENT.

void
Gdb_index_info_reader::visit_children(Dwarf_die* parent, Dwarf_die* context)
{
  off_t next_offset = 0;
  for (off_t die_offset = parent->child_offset();
       die_offset != 0;
       die_offset = next_offset)
    {
      Dwarf_die die(this, die_offset, parent);
      if (die.tag() == 0)
	break;
      this->visit_die(&die, context);
      next_offset = die.sibling_offset();
    }
}

// Visit a child DIE, looking for symbols to add to the index.
// CONTEXT is the parent DIE, used for constructing the qualified name;
// it is NULL if the parent DIE is the top-level DIE.

void
Gdb_index_info_reader::visit_die(Dwarf_die* die, Dwarf_die* context)
{
  switch (die->tag())
    {
      case elfcpp::DW_TAG_subprogram:
      case elfcpp::DW_TAG_constant:
      case elfcpp::DW_TAG_variable:
      case elfcpp::DW_TAG_enumerator:
      case elfcpp::DW_TAG_base_type:
	if (die->is_declaration())
	  this->add_declaration(die, context);
	else
	  {
	    // If the DIE is not a declaration, add it to the index.
	    std::string full_name = this->get_qualified_name(die, context);
	    if (!full_name.empty())
	      this->gdb_index_->add_symbol(this->cu_index_,
                                           full_name.c_str(), 0);
	  }
	break;
      case elfcpp::DW_TAG_typedef:
      case elfcpp::DW_TAG_union_type:
      case elfcpp::DW_TAG_class_type:
      case elfcpp::DW_TAG_interface_type:
      case elfcpp::DW_TAG_structure_type:
      case elfcpp::DW_TAG_enumeration_type:
      case elfcpp::DW_TAG_subrange_type:
      case elfcpp::DW_TAG_namespace:
	{
	  std::string full_name;
	  
	  // For classes at the top level, we need to look for a
	  // member function with a linkage name in order to get
	  // the properly-canonicalized name.
	  if (context == NULL
	      && (die->tag() == elfcpp::DW_TAG_class_type
		  || die->tag() == elfcpp::DW_TAG_structure_type
		  || die->tag() == elfcpp::DW_TAG_union_type))
	    full_name.assign(this->guess_full_class_name(die));

	  // Because we will visit the children, we need to add this DIE
	  // to the declarations table.
	  if (full_name.empty())
	    this->add_declaration(die, context);
	  else
	    this->add_declaration_with_full_name(die, full_name.c_str());

	  // If the DIE is not a declaration, add it to the index.
	  // Gdb stores a namespace in the index even when it is
	  // a declaration.
	  if (die->tag() == elfcpp::DW_TAG_namespace
	      || !die->is_declaration())
	    {
	      if (full_name.empty())
		full_name = this->get_qualified_name(die, context);
	      if (!full_name.empty())
		this->gdb_index_->add_symbol(this->cu_index_,
					     full_name.c_str(), 0);
	    }

	  // We're interested in the children only for namespaces and
	  // enumeration types.  For enumeration types, we do not include
	  // the enumeration tag as part of the full name.  For other tags,
	  // visit the children only to collect declarations.
	  if (die->tag() == elfcpp::DW_TAG_namespace
	      || die->tag() == elfcpp::DW_TAG_enumeration_type)
	    this->visit_children(die, die);
	  else
	    this->visit_children_for_decls(die);
	}
	break;
      default:
	break;
    }
}

// Visit the children of PARENT, looking only for declarations that
// may be referenced by later specification DIEs.

void
Gdb_index_info_reader::visit_children_for_decls(Dwarf_die* parent)
{
  off_t next_offset = 0;
  for (off_t die_offset = parent->child_offset();
       die_offset != 0;
       die_offset = next_offset)
    {
      Dwarf_die die(this, die_offset, parent);
      if (die.tag() == 0)
	break;
      this->visit_die_for_decls(&die, parent);
      next_offset = die.sibling_offset();
    }
}

// Visit a child DIE, looking only for declarations that
// may be referenced by later specification DIEs.

void
Gdb_index_info_reader::visit_die_for_decls(Dwarf_die* die, Dwarf_die* context)
{
  switch (die->tag())
    {
      case elfcpp::DW_TAG_subprogram:
      case elfcpp::DW_TAG_constant:
      case elfcpp::DW_TAG_variable:
      case elfcpp::DW_TAG_enumerator:
      case elfcpp::DW_TAG_base_type:
	{
	  if (die->is_declaration())
	    this->add_declaration(die, context);
	}
	break;
      case elfcpp::DW_TAG_typedef:
      case elfcpp::DW_TAG_union_type:
      case elfcpp::DW_TAG_class_type:
      case elfcpp::DW_TAG_interface_type:
      case elfcpp::DW_TAG_structure_type:
      case elfcpp::DW_TAG_enumeration_type:
      case elfcpp::DW_TAG_subrange_type:
      case elfcpp::DW_TAG_namespace:
	{
	  if (die->is_declaration())
	    this->add_declaration(die, context);
	  this->visit_children_for_decls(die);
	}
	break;
      default:
	break;
    }
}

// Extract the class name from the linkage name of a member function.
// This code is adapted from ../gdb/cp-support.c.

#define d_left(dc) (dc)->u.s_binary.left
#define d_right(dc) (dc)->u.s_binary.right

static char*
class_name_from_linkage_name(const char* linkage_name)
{
  void* storage;
  struct demangle_component* tree =
      cplus_demangle_v3_components(linkage_name, DMGL_NO_OPTS, &storage);
  if (tree == NULL)
    return NULL;

  int done = 0;

  // First strip off any qualifiers, if we have a function or
  // method.
  while (!done)
    switch (tree->type)
      {
	case DEMANGLE_COMPONENT_CONST:
	case DEMANGLE_COMPONENT_RESTRICT:
	case DEMANGLE_COMPONENT_VOLATILE:
	case DEMANGLE_COMPONENT_CONST_THIS:
	case DEMANGLE_COMPONENT_RESTRICT_THIS:
	case DEMANGLE_COMPONENT_VOLATILE_THIS:
	case DEMANGLE_COMPONENT_VENDOR_TYPE_QUAL:
	  tree = d_left(tree);
	  break;
	default:
	  done = 1;
	  break;
      }

  // If what we have now is a function, discard the argument list.
  if (tree->type == DEMANGLE_COMPONENT_TYPED_NAME)
    tree = d_left(tree);

  // If what we have now is a template, strip off the template
  // arguments.  The left subtree may be a qualified name.
  if (tree->type == DEMANGLE_COMPONENT_TEMPLATE)
    tree = d_left(tree);

  // What we have now should be a name, possibly qualified.
  // Additional qualifiers could live in the left subtree or the right
  // subtree.  Find the last piece.
  done = 0;
  struct demangle_component* prev_comp = NULL;
  struct demangle_component* cur_comp = tree;
  while (!done)
    switch (cur_comp->type)
      {
	case DEMANGLE_COMPONENT_QUAL_NAME:
	case DEMANGLE_COMPONENT_LOCAL_NAME:
	  prev_comp = cur_comp;
	  cur_comp = d_right(cur_comp);
	  break;
	case DEMANGLE_COMPONENT_TEMPLATE:
	case DEMANGLE_COMPONENT_NAME:
	case DEMANGLE_COMPONENT_CTOR:
	case DEMANGLE_COMPONENT_DTOR:
	case DEMANGLE_COMPONENT_OPERATOR:
	case DEMANGLE_COMPONENT_EXTENDED_OPERATOR:
	  done = 1;
	  break;
	default:
	  done = 1;
	  cur_comp = NULL;
	  break;
      }

  char* ret = NULL;
  if (cur_comp != NULL && prev_comp != NULL)
    {
      // We want to discard the rightmost child of PREV_COMP.
      *prev_comp = *d_left(prev_comp);
      size_t allocated_size;
      ret = cplus_demangle_print(DMGL_NO_OPTS, tree, 30, &allocated_size);
    }

  free(storage);
  return ret;
}

// Guess a fully-qualified name for a class type, based on member function
// linkage names.  This is needed for class/struct/union types at the
// top level, because GCC does not always properly embed them within
// the namespace.  As in gdb, we look for a member function with a linkage
// name and extract the qualified name from the demangled name.

std::string
Gdb_index_info_reader::guess_full_class_name(Dwarf_die* die)
{
  std::string full_name;
  off_t next_offset = 0;
  
  // This routine scans ahead in the DIE structure, possibly advancing
  // the relocation tracker beyond the current DIE.  We need to checkpoint
  // the tracker and reset it when we're done.
  uint64_t checkpoint = this->get_reloc_checkpoint();

  for (off_t child_offset = die->child_offset();
       child_offset != 0;
       child_offset = next_offset)
    {
      Dwarf_die child(this, child_offset, die);
      if (child.tag() == 0)
	break;
      if (child.tag() == elfcpp::DW_TAG_subprogram)
        {
          const char* linkage_name = child.linkage_name();
	  if (linkage_name != NULL)
	    {
	      char* guess = class_name_from_linkage_name(linkage_name);
	      if (guess != NULL)
	        {
		  full_name.assign(guess);
		  free(guess);
		  break;
	        }
	    }
        }
      next_offset = child.sibling_offset();
    }

  this->reset_relocs(checkpoint);
  return full_name;
}

// Add a declaration DIE to the table of declarations.

void
Gdb_index_info_reader::add_declaration(Dwarf_die* die, Dwarf_die* context)
{
  const char* name = die->name();

  off_t parent_offset = context != NULL ? context->offset() : 0;

  // If this DIE has a DW_AT_specification or DW_AT_abstract_origin
  // attribute, use the parent and name from the earlier declaration.
  off_t spec = die->specification();
  if (spec == 0)
    spec = die->abstract_origin();
  if (spec > 0)
    {
      Declaration_map::iterator it = this->declarations_.find(spec);
      if (it != this->declarations_.end())
        {
	  parent_offset = it->second.parent_offset_;
	  name = it->second.name_;
        }
    }

  if (name == NULL)
    {
      if (die->tag() == elfcpp::DW_TAG_namespace)
        name = "(anonymous namespace)";
      else if (die->tag() == elfcpp::DW_TAG_union_type)
        name = "(anonymous union)";
      else
        name = "(unknown)";
    }

  Declaration_pair decl(parent_offset, name);
  this->declarations_.insert(std::make_pair(die->offset(), decl));
}

// Add a declaration whose fully-qualified name is already known.
// In the case where we had to get the canonical name by demangling
// a linkage name, this ensures we use that name instead of the one
// provided in DW_AT_name.

void
Gdb_index_info_reader::add_declaration_with_full_name(
    Dwarf_die* die,
    const char* full_name)
{
  // We need to copy the name.
  int len = strlen(full_name);
  char* copy = new char[len + 1];
  memcpy(copy, full_name, len + 1);

  // Flag that we now manage the memory this points to.
  Declaration_pair decl(-1, copy);
  this->declarations_.insert(std::make_pair(die->offset(), decl));
}

// Return the context for a DIE whose parent is at DIE_OFFSET.

std::string
Gdb_index_info_reader::get_context(off_t die_offset)
{
  std::string context;
  Declaration_map::iterator it = this->declarations_.find(die_offset);
  if (it != this->declarations_.end())
    {
      off_t parent_offset = it->second.parent_offset_;
      if (parent_offset > 0)
	{
	  context = get_context(parent_offset);
	  context.append("::");
	}
      if (it->second.name_ != NULL)
        context.append(it->second.name_);
    }
  return context;
}

// Construct the fully-qualified name for DIE.

std::string
Gdb_index_info_reader::get_qualified_name(Dwarf_die* die, Dwarf_die* context)
{
  std::string full_name;
  const char* name = die->name();

  off_t parent_offset = context != NULL ? context->offset() : 0;

  // If this DIE has a DW_AT_specification or DW_AT_abstract_origin
  // attribute, use the parent and name from the earlier declaration.
  off_t spec = die->specification();
  if (spec == 0)
    spec = die->abstract_origin();
  if (spec > 0)
    {
      Declaration_map::iterator it = this->declarations_.find(spec);
      if (it != this->declarations_.end())
        {
	  parent_offset = it->second.parent_offset_;
	  name = it->second.name_;
        }
    }

  if (name == NULL && die->tag() == elfcpp::DW_TAG_namespace)
    name = "(anonymous namespace)";
  else if (name == NULL)
    return full_name;

  // If this is an enumerator constant, skip the immediate parent,
  // which is the enumeration tag.
  if (die->tag() == elfcpp::DW_TAG_enumerator)
    {
      Declaration_map::iterator it = this->declarations_.find(parent_offset);
      if (it != this->declarations_.end())
	parent_offset = it->second.parent_offset_;
    }

  if (parent_offset > 0)
    {
      full_name.assign(this->get_context(parent_offset));
      full_name.append("::");
    }
  full_name.append(name);

  return full_name;
}

// Record the address ranges for a compilation unit.

void
Gdb_index_info_reader::record_cu_ranges(Dwarf_die* die)
{
  unsigned int shndx;
  unsigned int shndx2;

  off_t ranges_offset = die->ref_attribute(elfcpp::DW_AT_ranges, &shndx);
  if (ranges_offset != -1)
    {
      Dwarf_range_list* ranges = this->read_range_list(shndx, ranges_offset);
      if (ranges != NULL)
	this->gdb_index_->add_address_range_list(this->object(),
						 this->cu_index_, ranges);
      return;
    }

  off_t low_pc = die->address_attribute(elfcpp::DW_AT_low_pc, &shndx);
  off_t high_pc = die->address_attribute(elfcpp::DW_AT_high_pc, &shndx2);
  if (high_pc == -1)
    {
      high_pc = die->uint_attribute(elfcpp::DW_AT_high_pc);
      high_pc += low_pc;
      shndx2 = shndx;
    }
  if ((low_pc != 0 || high_pc != 0) && low_pc != -1)
    {
      if (shndx != shndx2)
        {
	  gold_warning(_("%s: DWARF info may be corrupt; low_pc and high_pc "
			 "are in different sections"),
		       this->object()->name().c_str());
	  return;
	}
      if (shndx == 0 || this->object()->is_section_included(shndx))
        {
	  Dwarf_range_list* ranges = new Dwarf_range_list();
	  ranges->add(shndx, low_pc, high_pc);
	  this->gdb_index_->add_address_range_list(this->object(),
						   this->cu_index_, ranges);
        }
    }
}

// Read table and add the relevant names to the index.  Returns true
// if any names were added.

bool
Gdb_index_info_reader::read_pubtable(Dwarf_pubnames_table* table, off_t offset)
{
  // If we couldn't read the section when building the cu_pubname_map,
  // then we won't find any pubnames now.
  if (table == NULL)
    return false;

  if (!table->read_header(offset))
    return false;
  while (true)
    {
      uint8_t flag_byte;
      const char* name = table->next_name(&flag_byte);
      if (name == NULL)
        break;

      this->gdb_index_->add_symbol(this->cu_index_, name, flag_byte);
    }
  return true;
}

// Read the .debug_pubnames and .debug_pubtypes tables for the CU or TU.
// Returns TRUE if either a pubnames or pubtypes section was found.

bool
Gdb_index_info_reader::read_pubnames_and_pubtypes(Dwarf_die* die)
{
  // If this is a skeleton debug-type die (generated via
  // -gsplit-dwarf), then the associated pubnames should have been
  // read along with the corresponding CU.  In any case, there isn't
  // enough info inside to build a gdb index entry.
  if (die->tag() == elfcpp::DW_TAG_type_unit
      && die->string_attribute(elfcpp::DW_AT_GNU_dwo_name))
    return true;

  // We use stmt_list_off as a unique identifier for the
  // compilation unit and its associated type units.
  unsigned int shndx;
  off_t stmt_list_off = die->ref_attribute (elfcpp::DW_AT_stmt_list,
                                            &shndx);
  // Look for the attr as either a flag or a ref.
  off_t offset = die->ref_attribute(elfcpp::DW_AT_GNU_pubnames, &shndx);

  // Newer versions of GCC generate CUs, but not TUs, with
  // DW_AT_FORM_flag_present.
  unsigned int flag = die->uint_attribute(elfcpp::DW_AT_GNU_pubnames);
  if (offset == -1 && flag == 0)
    {
      // Didn't find the attribute.
      if (die->tag() == elfcpp::DW_TAG_type_unit)
        {
          // If die is a TU, then it might correspond to a CU which we
          // have read. If it does, then no need to read the pubnames.
          // If it doesn't, then the caller will have to parse the
          // dies manually to find the names.
          return this->gdb_index_->pubnames_read(this->object(),
                                                 stmt_list_off);
        }
      else
        {
          // No attribute on the CU means that no pubnames were read.
          return false;
        }
    }

  // We found the attribute, so we can check if the corresponding
  // pubnames have been read.
  if (this->gdb_index_->pubnames_read(this->object(), stmt_list_off))
    return true;

  this->gdb_index_->set_pubnames_read(this->object(), stmt_list_off);

  // We have an attribute, and the pubnames haven't been read, so read
  // them.
  bool names = false;
  // In some of the cases, we could rely on the previous value of
  // offset here, but sorting out which cases complicates the logic
  // enough that it isn't worth it. So just look up the offset again.
  offset = this->gdb_index_->find_pubname_offset(this->cu_offset());
  names = this->read_pubtable(this->gdb_index_->pubnames_table(), offset);

  bool types = false;
  offset = this->gdb_index_->find_pubtype_offset(this->cu_offset());
  types = this->read_pubtable(this->gdb_index_->pubtypes_table(), offset);
  return names || types;
}

// Clear the declarations map.
void
Gdb_index_info_reader::clear_declarations()
{
  // Free strings in memory we manage.
  for (Declaration_map::iterator it = this->declarations_.begin();
       it != this->declarations_.end();
       ++it)
    {
      if (it->second.parent_offset_ == -1)
	delete[] it->second.name_;
    }

  this->declarations_.clear();
}

// Print usage statistics.
void
Gdb_index_info_reader::print_stats()
{
  fprintf(stderr, _("%s: DWARF CUs: %u\n"),
          program_name, Gdb_index_info_reader::dwarf_cu_count);
  fprintf(stderr, _("%s: DWARF CUs without pubnames/pubtypes: %u\n"),
          program_name, Gdb_index_info_reader::dwarf_cu_nopubnames_count);
  fprintf(stderr, _("%s: DWARF TUs: %u\n"),
          program_name, Gdb_index_info_reader::dwarf_tu_count);
  fprintf(stderr, _("%s: DWARF TUs without pubnames/pubtypes: %u\n"),
          program_name, Gdb_index_info_reader::dwarf_tu_nopubnames_count);
}

// Class Gdb_index.

// Construct the .gdb_index section.

Gdb_index::Gdb_index(Output_section* gdb_index_section)
  : Output_section_data(4),
    pubnames_table_(NULL),
    pubtypes_table_(NULL),
    gdb_index_section_(gdb_index_section),
    comp_units_(),
    type_units_(),
    ranges_(),
    cu_vector_list_(),
    cu_vector_offsets_(NULL),
    stringpool_(),
    tu_offset_(0),
    addr_offset_(0),
    symtab_offset_(0),
    cu_pool_offset_(0),
    stringpool_offset_(0),
    pubnames_object_(NULL),
    stmt_list_offset_(-1)
{
  this->gdb_symtab_ = new Gdb_hashtab<Gdb_symbol>();
}

Gdb_index::~Gdb_index()
{
  // Free the memory used by the symbol table.
  delete this->gdb_symtab_;
  // Free the memory used by the CU vectors.
  for (unsigned int i = 0; i < this->cu_vector_list_.size(); ++i)
    delete this->cu_vector_list_[i];
}


// Scan the pubnames and pubtypes sections and build a map of the
// various cus and tus they refer to, so we can process the entries
// when we encounter the die for that cu or tu.
// Return the just-read table so it can be cached.

Dwarf_pubnames_table*
Gdb_index::map_pubtable_to_dies(unsigned int attr,
                                Gdb_index_info_reader* dwinfo,
                                Relobj* object,
                                const unsigned char* symbols,
                                off_t symbols_size)
{
  uint64_t section_offset = 0;
  Dwarf_pubnames_table* table;
  Pubname_offset_map* map;

  if (attr == elfcpp::DW_AT_GNU_pubnames)
    {
      table = new Dwarf_pubnames_table(dwinfo, false);
      map = &this->cu_pubname_map_;
    }
  else
    {
      table = new Dwarf_pubnames_table(dwinfo, true);
      map = &this->cu_pubtype_map_;
    }

  map->clear();
  if (!table->read_section(object, symbols, symbols_size))
    return NULL;

  while (table->read_header(section_offset))
    {
      map->insert(std::make_pair(table->cu_offset(), section_offset));
      section_offset += table->subsection_size();
    }

  return table;
}

// Wrapper for map_pubtable_to_dies

void
Gdb_index::map_pubnames_and_types_to_dies(Gdb_index_info_reader* dwinfo,
                                          Relobj* object,
                                          const unsigned char* symbols,
                                          off_t symbols_size)
{
  // This is a new object, so reset the relevant variables.
  this->pubnames_object_ = object;
  this->stmt_list_offset_ = -1;

  delete this->pubnames_table_;
  this->pubnames_table_
      = this->map_pubtable_to_dies(elfcpp::DW_AT_GNU_pubnames, dwinfo,
                                   object, symbols, symbols_size);
  delete this->pubtypes_table_;
  this->pubtypes_table_
      = this->map_pubtable_to_dies(elfcpp::DW_AT_GNU_pubtypes, dwinfo,
                                   object, symbols, symbols_size);
}

// Given a cu_offset, find the associated section of the pubnames
// table.

off_t
Gdb_index::find_pubname_offset(off_t cu_offset)
{
  Pubname_offset_map::iterator it = this->cu_pubname_map_.find(cu_offset);
  if (it != this->cu_pubname_map_.end())
    return it->second;
  return -1;
}

// Given a cu_offset, find the associated section of the pubnames
// table.

off_t
Gdb_index::find_pubtype_offset(off_t cu_offset)
{
  Pubname_offset_map::iterator it = this->cu_pubtype_map_.find(cu_offset);
  if (it != this->cu_pubtype_map_.end())
    return it->second;
  return -1;
}

// Scan a .debug_info or .debug_types input section.

void
Gdb_index::scan_debug_info(bool is_type_unit,
			   Relobj* object,
			   const unsigned char* symbols,
			   off_t symbols_size,
			   unsigned int shndx,
			   unsigned int reloc_shndx,
			   unsigned int reloc_type)
{
  Gdb_index_info_reader dwinfo(is_type_unit, object,
			       symbols, symbols_size,
			       shndx, reloc_shndx,
			       reloc_type, this);
  if (object != this->pubnames_object_)
    map_pubnames_and_types_to_dies(&dwinfo, object, symbols, symbols_size);
  dwinfo.parse();
}

// Add a symbol.

void
Gdb_index::add_symbol(int cu_index, const char* sym_name, uint8_t flags)
{
  unsigned int hash = mapped_index_string_hash(
      reinterpret_cast<const unsigned char*>(sym_name));
  Gdb_symbol* sym = new Gdb_symbol();
  this->stringpool_.add(sym_name, true, &sym->name_key);
  sym->hashval = hash;
  sym->cu_vector_index = 0;

  Gdb_symbol* found = this->gdb_symtab_->add(sym);
  if (found == sym)
    {
      // New symbol -- allocate a new CU index vector.
      found->cu_vector_index = this->cu_vector_list_.size();
      this->cu_vector_list_.push_back(new Cu_vector());
    }
  else
    {
      // Found an existing symbol -- append to the existing
      // CU index vector.
      delete sym;
    }

  // Add the CU index to the vector list for this symbol,
  // if it's not already on the list.  We only need to
  // check the last added entry.
  Cu_vector* cu_vec = this->cu_vector_list_[found->cu_vector_index];
  if (cu_vec->size() == 0
      || cu_vec->back().first != cu_index
      || cu_vec->back().second != flags)
    cu_vec->push_back(std::make_pair(cu_index, flags));
}

// Return TRUE if we have already processed the pubnames associated
// with the statement list at the given OFFSET.

bool
Gdb_index::pubnames_read(const Relobj* object, off_t offset)
{
  bool ret = (this->pubnames_object_ == object
	      && this->stmt_list_offset_ == offset);
  return ret;
}

// Record that we have processed the pubnames associated with the
// statement list for OBJECT at the given OFFSET.

void
Gdb_index::set_pubnames_read(const Relobj* object, off_t offset)
{
  this->pubnames_object_ = object;
  this->stmt_list_offset_ = offset;
}

// Set the size of the .gdb_index section.

void
Gdb_index::set_final_data_size()
{
  // Finalize the string pool.
  this->stringpool_.set_string_offsets();

  // Compute the total size of the CU vectors.
  // For each CU vector, include one entry for the count at the
  // beginning of the vector.
  unsigned int cu_vector_count = this->cu_vector_list_.size();
  unsigned int cu_vector_size = 0;
  this->cu_vector_offsets_ = new off_t[cu_vector_count];
  for (unsigned int i = 0; i < cu_vector_count; ++i)
    {
      Cu_vector* cu_vec = this->cu_vector_list_[i];
      cu_vector_offsets_[i] = cu_vector_size;
      cu_vector_size += gdb_index_offset_size * (cu_vec->size() + 1);
    }

  // Assign relative offsets to each portion of the index,
  // and find the total size of the section.
  section_size_type data_size = gdb_index_hdr_size;
  data_size += this->comp_units_.size() * gdb_index_cu_size;
  this->tu_offset_ = data_size;
  data_size += this->type_units_.size() * gdb_index_tu_size;
  this->addr_offset_ = data_size;
  for (unsigned int i = 0; i < this->ranges_.size(); ++i)
    data_size += this->ranges_[i].ranges->size() * gdb_index_addr_size;
  this->symtab_offset_ = data_size;
  data_size += this->gdb_symtab_->capacity() * gdb_index_sym_size;
  this->cu_pool_offset_ = data_size;
  data_size += cu_vector_size;
  this->stringpool_offset_ = data_size;
  data_size += this->stringpool_.get_strtab_size();

  this->set_data_size(data_size);
}

// Write the data to the file.

void
Gdb_index::do_write(Output_file* of)
{
  const off_t off = this->offset();
  const off_t oview_size = this->data_size();
  unsigned char* const oview = of->get_output_view(off, oview_size);
  unsigned char* pov = oview;

  // Write the file header.
  // (1) Version number.
  elfcpp::Swap<32, false>::writeval(pov, gdb_index_version);
  pov += 4;
  // (2) Offset of the CU list.
  elfcpp::Swap<32, false>::writeval(pov, gdb_index_hdr_size);
  pov += 4;
  // (3) Offset of the types CU list.
  elfcpp::Swap<32, false>::writeval(pov, this->tu_offset_);
  pov += 4;
  // (4) Offset of the address area.
  elfcpp::Swap<32, false>::writeval(pov, this->addr_offset_);
  pov += 4;
  // (5) Offset of the symbol table.
  elfcpp::Swap<32, false>::writeval(pov, this->symtab_offset_);
  pov += 4;
  // (6) Offset of the constant pool.
  elfcpp::Swap<32, false>::writeval(pov, this->cu_pool_offset_);
  pov += 4;

  gold_assert(pov - oview == gdb_index_hdr_size);

  // Write the CU list.
  unsigned int comp_units_count = this->comp_units_.size();
  for (unsigned int i = 0; i < comp_units_count; ++i)
    {
      const Comp_unit& cu = this->comp_units_[i];
      elfcpp::Swap<64, false>::writeval(pov, cu.cu_offset);
      elfcpp::Swap<64, false>::writeval(pov + 8, cu.cu_length);
      pov += 16;
    }

  gold_assert(pov - oview == this->tu_offset_);

  // Write the types CU list.
  for (unsigned int i = 0; i < this->type_units_.size(); ++i)
    {
      const Type_unit& tu = this->type_units_[i];
      elfcpp::Swap<64, false>::writeval(pov, tu.tu_offset);
      elfcpp::Swap<64, false>::writeval(pov + 8, tu.type_offset);
      elfcpp::Swap<64, false>::writeval(pov + 16, tu.type_signature);
      pov += 24;
    }

  gold_assert(pov - oview == this->addr_offset_);

  // Write the address area.
  for (unsigned int i = 0; i < this->ranges_.size(); ++i)
    {
      int cu_index = this->ranges_[i].cu_index;
      // Translate negative indexes, which refer to a TU, to a
      // logical index into a concatenated CU/TU list.
      if (cu_index < 0)
        cu_index = comp_units_count + (-1 - cu_index);
      Relobj* object = this->ranges_[i].object;
      const Dwarf_range_list& ranges = *this->ranges_[i].ranges;
      for (unsigned int j = 0; j < ranges.size(); ++j)
        {
	  const Dwarf_range_list::Range& range = ranges[j];
	  uint64_t base = 0;
	  if (range.shndx > 0)
	    {
	      const Output_section* os = object->output_section(range.shndx);
	      base = (os->address()
		      + object->output_section_offset(range.shndx));
	    }
	  elfcpp::Swap_aligned32<64, false>::writeval(pov, base + range.start);
	  elfcpp::Swap_aligned32<64, false>::writeval(pov + 8,
						      base + range.end);
	  elfcpp::Swap<32, false>::writeval(pov + 16, cu_index);
	  pov += 20;
	}
    }

  gold_assert(pov - oview == this->symtab_offset_);

  // Write the symbol table.
  for (unsigned int i = 0; i < this->gdb_symtab_->capacity(); ++i)
    {
      const Gdb_symbol* sym = (*this->gdb_symtab_)[i];
      section_offset_type name_offset = 0;
      unsigned int cu_vector_offset = 0;
      if (sym != NULL)
	{
	  name_offset = (this->stringpool_.get_offset_from_key(sym->name_key)
			 + this->stringpool_offset_ - this->cu_pool_offset_);
	  cu_vector_offset = this->cu_vector_offsets_[sym->cu_vector_index];
	}
      elfcpp::Swap<32, false>::writeval(pov, name_offset);
      elfcpp::Swap<32, false>::writeval(pov + 4, cu_vector_offset);
      pov += 8;
    }

  gold_assert(pov - oview == this->cu_pool_offset_);

  // Write the CU vectors into the constant pool.
  for (unsigned int i = 0; i < this->cu_vector_list_.size(); ++i)
    {
      Cu_vector* cu_vec = this->cu_vector_list_[i];
      elfcpp::Swap<32, false>::writeval(pov, cu_vec->size());
      pov += 4;
      for (unsigned int j = 0; j < cu_vec->size(); ++j)
	{
	  int cu_index = (*cu_vec)[j].first;
          uint8_t flags = (*cu_vec)[j].second;
	  if (cu_index < 0)
	    cu_index = comp_units_count + (-1 - cu_index);
          cu_index |= flags << 24;
	  elfcpp::Swap<32, false>::writeval(pov, cu_index);
	  pov += 4;
	}
    }

  gold_assert(pov - oview == this->stringpool_offset_);

  // Write the strings into the constant pool.
  this->stringpool_.write_to_buffer(pov, oview_size - this->stringpool_offset_);

  of->write_output_view(off, oview_size, oview);
}

// Print usage statistics.
void
Gdb_index::print_stats()
{
  if (parameters->options().gdb_index())
    Gdb_index_info_reader::print_stats();
}

} // End namespace gold.
