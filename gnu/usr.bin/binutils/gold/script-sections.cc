// script-sections.cc -- linker script SECTIONS for gold

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
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
#include <algorithm>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <fnmatch.h>

#include "parameters.h"
#include "object.h"
#include "layout.h"
#include "output.h"
#include "script-c.h"
#include "script.h"
#include "script-sections.h"

// Support for the SECTIONS clause in linker scripts.

namespace gold
{

// A region of memory.
class Memory_region
{
 public:
  Memory_region(const char* name, size_t namelen, unsigned int attributes,
		Expression* start, Expression* length)
    : name_(name, namelen),
      attributes_(attributes),
      start_(start),
      length_(length),
      current_offset_(0),
      vma_sections_(),
      lma_sections_(),
      last_section_(NULL)
  { }

  // Return the name of this region.
  const std::string&
  name() const
  { return this->name_; }

  // Return the start address of this region.
  Expression*
  start_address() const
  { return this->start_; }

  // Return the length of this region.
  Expression*
  length() const
  { return this->length_; }

  // Print the region (when debugging).
  void
  print(FILE*) const;

  // Return true if <name,namelen> matches this region.
  bool
  name_match(const char* name, size_t namelen)
  {
    return (this->name_.length() == namelen
	    && strncmp(this->name_.c_str(), name, namelen) == 0);
  }

  Expression*
  get_current_address() const
  {
    return
      script_exp_binary_add(this->start_,
			    script_exp_integer(this->current_offset_));
  }

  void
  set_address(uint64_t addr, const Symbol_table* symtab, const Layout* layout)
  {
    uint64_t start = this->start_->eval(symtab, layout, false);
    uint64_t len = this->length_->eval(symtab, layout, false);
    if (addr < start || addr >= start + len)
      gold_error(_("address 0x%llx is not within region %s"),
		 static_cast<unsigned long long>(addr),
		 this->name_.c_str());
    else if (addr < start + this->current_offset_)
      gold_error(_("address 0x%llx moves dot backwards in region %s"),
		 static_cast<unsigned long long>(addr),
		 this->name_.c_str());
    this->current_offset_ = addr - start;
  }

  void
  increment_offset(std::string section_name, uint64_t amount,
		   const Symbol_table* symtab, const Layout* layout)
  {
    this->current_offset_ += amount;

    if (this->current_offset_
	> this->length_->eval(symtab, layout, false))
      gold_error(_("section %s overflows end of region %s"),
		 section_name.c_str(), this->name_.c_str());
  }

  // Returns true iff there is room left in this region
  // for AMOUNT more bytes of data.
  bool
  has_room_for(const Symbol_table* symtab, const Layout* layout,
	       uint64_t amount) const
  {
    return (this->current_offset_ + amount
	    < this->length_->eval(symtab, layout, false));
  }

  // Return true if the provided section flags
  // are compatible with this region's attributes.
  bool
  attributes_compatible(elfcpp::Elf_Xword flags, elfcpp::Elf_Xword type) const;

  void
  add_section(Output_section_definition* sec, bool vma)
  {
    if (vma)
      this->vma_sections_.push_back(sec);
    else
      this->lma_sections_.push_back(sec);
  }

  typedef std::vector<Output_section_definition*> Section_list;

  // Return the start of the list of sections
  // whose VMAs are taken from this region.
  Section_list::const_iterator
  get_vma_section_list_start() const
  { return this->vma_sections_.begin(); }

  // Return the start of the list of sections
  // whose LMAs are taken from this region.
  Section_list::const_iterator
  get_lma_section_list_start() const
  { return this->lma_sections_.begin(); }

  // Return the end of the list of sections
  // whose VMAs are taken from this region.
  Section_list::const_iterator
  get_vma_section_list_end() const
  { return this->vma_sections_.end(); }

  // Return the end of the list of sections
  // whose LMAs are taken from this region.
  Section_list::const_iterator
  get_lma_section_list_end() const
  { return this->lma_sections_.end(); }

  Output_section_definition*
  get_last_section() const
  { return this->last_section_; }

  void
  set_last_section(Output_section_definition* sec)
  { this->last_section_ = sec; }

 private:

  std::string name_;
  unsigned int attributes_;
  Expression* start_;
  Expression* length_;
  // The offset to the next free byte in the region.
  // Note - for compatibility with GNU LD we only maintain one offset
  // regardless of whether the region is being used for VMA values,
  // LMA values, or both.
  uint64_t current_offset_;
  // A list of sections whose VMAs are set inside this region.
  Section_list vma_sections_;
  // A list of sections whose LMAs are set inside this region.
  Section_list lma_sections_;
  // The latest section to make use of this region.
  Output_section_definition* last_section_;
};

// Return true if the provided section flags
// are compatible with this region's attributes.

bool
Memory_region::attributes_compatible(elfcpp::Elf_Xword flags,
				     elfcpp::Elf_Xword type) const
{
  unsigned int attrs = this->attributes_;

  // No attributes means that this region is not compatible with anything.
  if (attrs == 0)
    return false;

  bool match = true;
  do
    {
      switch (attrs & - attrs)
	{
	case MEM_EXECUTABLE:
	  if ((flags & elfcpp::SHF_EXECINSTR) == 0)
	    match = false;
	  break;

	case MEM_WRITEABLE:
	  if ((flags & elfcpp::SHF_WRITE) == 0)
	    match = false;
	  break;

	case MEM_READABLE:
	  // All sections are presumed readable.
	  break;

	case MEM_ALLOCATABLE:
	  if ((flags & elfcpp::SHF_ALLOC) == 0)
	    match = false;
	  break;

	case MEM_INITIALIZED:
	  if ((type & elfcpp::SHT_NOBITS) != 0)
	    match = false;
	  break;
	}
      attrs &= ~ (attrs & - attrs);
    }
  while (attrs != 0);

  return match;
}

// Print a memory region.

void
Memory_region::print(FILE* f) const
{
  fprintf(f, "  %s", this->name_.c_str());

  unsigned int attrs = this->attributes_;
  if (attrs != 0)
    {
      fprintf(f, " (");
      do
	{
	  switch (attrs & - attrs)
	    {
	    case MEM_EXECUTABLE:  fputc('x', f); break;
	    case MEM_WRITEABLE:   fputc('w', f); break;
	    case MEM_READABLE:    fputc('r', f); break;
	    case MEM_ALLOCATABLE: fputc('a', f); break;
	    case MEM_INITIALIZED: fputc('i', f); break;
	    default:
	      gold_unreachable();
	    }
	  attrs &= ~ (attrs & - attrs);
	}
      while (attrs != 0);
      fputc(')', f);
    }

  fprintf(f, " : origin = ");
  this->start_->print(f);
  fprintf(f, ", length = ");
  this->length_->print(f);
  fprintf(f, "\n");
}

// Manage orphan sections.  This is intended to be largely compatible
// with the GNU linker.  The Linux kernel implicitly relies on
// something similar to the GNU linker's orphan placement.  We
// originally used a simpler scheme here, but it caused the kernel
// build to fail, and was also rather inefficient.

class Orphan_section_placement
{
 private:
  typedef Script_sections::Elements_iterator Elements_iterator;

 public:
  Orphan_section_placement();

  // Handle an output section during initialization of this mapping.
  void
  output_section_init(const std::string& name, Output_section*,
		      Elements_iterator location);

  // Initialize the last location.
  void
  last_init(Elements_iterator location);

  // Set *PWHERE to the address of an iterator pointing to the
  // location to use for an orphan section.  Return true if the
  // iterator has a value, false otherwise.
  bool
  find_place(Output_section*, Elements_iterator** pwhere);

  // Update PLACE_LAST_ALLOC.
  void
  update_last_alloc(Elements_iterator where);

  // Return the iterator being used for sections at the very end of
  // the linker script.
  Elements_iterator
  last_place() const;

 private:
  // The places that we specifically recognize.  This list is copied
  // from the GNU linker.
  enum Place_index
  {
    PLACE_TEXT,
    PLACE_RODATA,
    PLACE_DATA,
    PLACE_TLS,
    PLACE_TLS_BSS,
    PLACE_BSS,
    PLACE_LAST_ALLOC,
    PLACE_REL,
    PLACE_INTERP,
    PLACE_NONALLOC,
    PLACE_LAST,
    PLACE_MAX
  };

  // The information we keep for a specific place.
  struct Place
  {
    // The name of sections for this place.
    const char* name;
    // Whether we have a location for this place.
    bool have_location;
    // The iterator for this place.
    Elements_iterator location;
  };

  // Initialize one place element.
  void
  initialize_place(Place_index, const char*);

  // The places.
  Place places_[PLACE_MAX];
  // True if this is the first call to output_section_init.
  bool first_init_;
};

// Initialize Orphan_section_placement.

Orphan_section_placement::Orphan_section_placement()
  : first_init_(true)
{
  this->initialize_place(PLACE_TEXT, ".text");
  this->initialize_place(PLACE_RODATA, ".rodata");
  this->initialize_place(PLACE_DATA, ".data");
  this->initialize_place(PLACE_TLS, NULL);
  this->initialize_place(PLACE_TLS_BSS, NULL);
  this->initialize_place(PLACE_BSS, ".bss");
  this->initialize_place(PLACE_LAST_ALLOC, NULL);
  this->initialize_place(PLACE_REL, NULL);
  this->initialize_place(PLACE_INTERP, ".interp");
  this->initialize_place(PLACE_NONALLOC, NULL);
  this->initialize_place(PLACE_LAST, NULL);
}

// Initialize one place element.

void
Orphan_section_placement::initialize_place(Place_index index, const char* name)
{
  this->places_[index].name = name;
  this->places_[index].have_location = false;
}

// While initializing the Orphan_section_placement information, this
// is called once for each output section named in the linker script.
// If we found an output section during the link, it will be passed in
// OS.

void
Orphan_section_placement::output_section_init(const std::string& name,
					      Output_section* os,
					      Elements_iterator location)
{
  bool first_init = this->first_init_;
  this->first_init_ = false;

  // Remember the last allocated section. Any orphan bss sections
  // will be placed after it.
  if (os != NULL
      && (os->flags() & elfcpp::SHF_ALLOC) != 0)
    {
      this->places_[PLACE_LAST_ALLOC].location = location;
      this->places_[PLACE_LAST_ALLOC].have_location = true;
    }

  for (int i = 0; i < PLACE_MAX; ++i)
    {
      if (this->places_[i].name != NULL && this->places_[i].name == name)
	{
	  if (this->places_[i].have_location)
	    {
	      // We have already seen a section with this name.
	      return;
	    }

	  this->places_[i].location = location;
	  this->places_[i].have_location = true;

	  // If we just found the .bss section, restart the search for
	  // an unallocated section.  This follows the GNU linker's
	  // behaviour.
	  if (i == PLACE_BSS)
	    this->places_[PLACE_NONALLOC].have_location = false;

	  return;
	}
    }

  // Relocation sections.
  if (!this->places_[PLACE_REL].have_location
      && os != NULL
      && (os->type() == elfcpp::SHT_REL || os->type() == elfcpp::SHT_RELA)
      && (os->flags() & elfcpp::SHF_ALLOC) != 0)
    {
      this->places_[PLACE_REL].location = location;
      this->places_[PLACE_REL].have_location = true;
    }

  // We find the location for unallocated sections by finding the
  // first debugging or comment section after the BSS section (if
  // there is one).
  if (!this->places_[PLACE_NONALLOC].have_location
      && (name == ".comment" || Layout::is_debug_info_section(name.c_str())))
    {
      // We add orphan sections after the location in PLACES_.  We
      // want to store unallocated sections before LOCATION.  If this
      // is the very first section, we can't use it.
      if (!first_init)
	{
	  --location;
	  this->places_[PLACE_NONALLOC].location = location;
	  this->places_[PLACE_NONALLOC].have_location = true;
	}
    }
}

// Initialize the last location.

void
Orphan_section_placement::last_init(Elements_iterator location)
{
  this->places_[PLACE_LAST].location = location;
  this->places_[PLACE_LAST].have_location = true;
}

// Set *PWHERE to the address of an iterator pointing to the location
// to use for an orphan section.  Return true if the iterator has a
// value, false otherwise.

bool
Orphan_section_placement::find_place(Output_section* os,
				     Elements_iterator** pwhere)
{
  // Figure out where OS should go.  This is based on the GNU linker
  // code.  FIXME: The GNU linker handles small data sections
  // specially, but we don't.
  elfcpp::Elf_Word type = os->type();
  elfcpp::Elf_Xword flags = os->flags();
  Place_index index;
  if ((flags & elfcpp::SHF_ALLOC) == 0
      && !Layout::is_debug_info_section(os->name()))
    index = PLACE_NONALLOC;
  else if ((flags & elfcpp::SHF_ALLOC) == 0)
    index = PLACE_LAST;
  else if (type == elfcpp::SHT_NOTE)
    index = PLACE_INTERP;
  else if ((flags & elfcpp::SHF_TLS) != 0)
    {
      if (type == elfcpp::SHT_NOBITS)
	index = PLACE_TLS_BSS;
      else
	index = PLACE_TLS;
    }
  else if (type == elfcpp::SHT_NOBITS)
    index = PLACE_BSS;
  else if ((flags & elfcpp::SHF_WRITE) != 0)
    index = PLACE_DATA;
  else if (type == elfcpp::SHT_REL || type == elfcpp::SHT_RELA)
    index = PLACE_REL;
  else if ((flags & elfcpp::SHF_EXECINSTR) == 0)
    index = PLACE_RODATA;
  else
    index = PLACE_TEXT;

  // If we don't have a location yet, try to find one based on a
  // plausible ordering of sections.
  if (!this->places_[index].have_location)
    {
      Place_index follow;
      switch (index)
	{
	default:
	  follow = PLACE_MAX;
	  break;
	case PLACE_RODATA:
	  follow = PLACE_TEXT;
	  break;
	case PLACE_DATA:
	  follow = PLACE_RODATA;
	  if (!this->places_[PLACE_RODATA].have_location)
	    follow = PLACE_TEXT;
	  break;
	case PLACE_BSS:
	  follow = PLACE_LAST_ALLOC;
	  break;
	case PLACE_REL:
	  follow = PLACE_TEXT;
	  break;
	case PLACE_INTERP:
	  follow = PLACE_TEXT;
	  break;
	case PLACE_TLS:
	  follow = PLACE_DATA;
	  break;
	case PLACE_TLS_BSS:
	  follow = PLACE_TLS;
	  if (!this->places_[PLACE_TLS].have_location)
	    follow = PLACE_DATA;
	  break;
	}
      if (follow != PLACE_MAX && this->places_[follow].have_location)
	{
	  // Set the location of INDEX to the location of FOLLOW.  The
	  // location of INDEX will then be incremented by the caller,
	  // so anything in INDEX will continue to be after anything
	  // in FOLLOW.
	  this->places_[index].location = this->places_[follow].location;
	  this->places_[index].have_location = true;
	}
    }

  *pwhere = &this->places_[index].location;
  bool ret = this->places_[index].have_location;

  // The caller will set the location.
  this->places_[index].have_location = true;

  return ret;
}

// Update PLACE_LAST_ALLOC.
void
Orphan_section_placement::update_last_alloc(Elements_iterator elem)
{
  Elements_iterator prev = elem;
  --prev;
  if (this->places_[PLACE_LAST_ALLOC].have_location
      && this->places_[PLACE_LAST_ALLOC].location == prev)
    {
      this->places_[PLACE_LAST_ALLOC].have_location = true;
      this->places_[PLACE_LAST_ALLOC].location = elem;
    }
}

// Return the iterator being used for sections at the very end of the
// linker script.

Orphan_section_placement::Elements_iterator
Orphan_section_placement::last_place() const
{
  gold_assert(this->places_[PLACE_LAST].have_location);
  return this->places_[PLACE_LAST].location;
}

// An element in a SECTIONS clause.

class Sections_element
{
 public:
  Sections_element()
  { }

  virtual ~Sections_element()
  { }

  // Return whether an output section is relro.
  virtual bool
  is_relro() const
  { return false; }

  // Record that an output section is relro.
  virtual void
  set_is_relro()
  { }

  // Create any required output sections.  The only real
  // implementation is in Output_section_definition.
  virtual void
  create_sections(Layout*)
  { }

  // Add any symbol being defined to the symbol table.
  virtual void
  add_symbols_to_table(Symbol_table*)
  { }

  // Finalize symbols and check assertions.
  virtual void
  finalize_symbols(Symbol_table*, const Layout*, uint64_t*)
  { }

  // Return the output section name to use for an input file name and
  // section name.  This only real implementation is in
  // Output_section_definition.
  virtual const char*
  output_section_name(const char*, const char*, Output_section***,
		      Script_sections::Section_type*, bool*, bool)
  { return NULL; }

  // Initialize OSP with an output section.
  virtual void
  orphan_section_init(Orphan_section_placement*,
		      Script_sections::Elements_iterator)
  { }

  // Set section addresses.  This includes applying assignments if the
  // expression is an absolute value.
  virtual void
  set_section_addresses(Symbol_table*, Layout*, uint64_t*, uint64_t*,
			uint64_t*)
  { }

  // Check a constraint (ONLY_IF_RO, etc.) on an output section.  If
  // this section is constrained, and the input sections do not match,
  // return the constraint, and set *POSD.
  virtual Section_constraint
  check_constraint(Output_section_definition**)
  { return CONSTRAINT_NONE; }

  // See if this is the alternate output section for a constrained
  // output section.  If it is, transfer the Output_section and return
  // true.  Otherwise return false.
  virtual bool
  alternate_constraint(Output_section_definition*, Section_constraint)
  { return false; }

  // Get the list of segments to use for an allocated section when
  // using a PHDRS clause.  If this is an allocated section, return
  // the Output_section, and set *PHDRS_LIST (the first parameter) to
  // the list of PHDRS to which it should be attached.  If the PHDRS
  // were not specified, don't change *PHDRS_LIST.  When not returning
  // NULL, set *ORPHAN (the second parameter) according to whether
  // this is an orphan section--one that is not mentioned in the
  // linker script.
  virtual Output_section*
  allocate_to_segment(String_list**, bool*)
  { return NULL; }

  // Look for an output section by name and return the address, the
  // load address, the alignment, and the size.  This is used when an
  // expression refers to an output section which was not actually
  // created.  This returns true if the section was found, false
  // otherwise.  The only real definition is for
  // Output_section_definition.
  virtual bool
  get_output_section_info(const char*, uint64_t*, uint64_t*, uint64_t*,
                          uint64_t*) const
  { return false; }

  // Return the associated Output_section if there is one.
  virtual Output_section*
  get_output_section() const
  { return NULL; }

  // Set the section's memory regions.
  virtual void
  set_memory_region(Memory_region*, bool)
  { gold_error(_("Attempt to set a memory region for a non-output section")); }

  // Print the element for debugging purposes.
  virtual void
  print(FILE* f) const = 0;
};

// An assignment in a SECTIONS clause outside of an output section.

class Sections_element_assignment : public Sections_element
{
 public:
  Sections_element_assignment(const char* name, size_t namelen,
			      Expression* val, bool provide, bool hidden)
    : assignment_(name, namelen, false, val, provide, hidden)
  { }

  // Add the symbol to the symbol table.
  void
  add_symbols_to_table(Symbol_table* symtab)
  { this->assignment_.add_to_table(symtab); }

  // Finalize the symbol.
  void
  finalize_symbols(Symbol_table* symtab, const Layout* layout,
		   uint64_t* dot_value)
  {
    this->assignment_.finalize_with_dot(symtab, layout, *dot_value, NULL);
  }

  // Set the section address.  There is no section here, but if the
  // value is absolute, we set the symbol.  This permits us to use
  // absolute symbols when setting dot.
  void
  set_section_addresses(Symbol_table* symtab, Layout* layout,
			uint64_t* dot_value, uint64_t*, uint64_t*)
  {
    this->assignment_.set_if_absolute(symtab, layout, true, *dot_value, NULL);
  }

  // Print for debugging.
  void
  print(FILE* f) const
  {
    fprintf(f, "  ");
    this->assignment_.print(f);
  }

 private:
  Symbol_assignment assignment_;
};

// An assignment to the dot symbol in a SECTIONS clause outside of an
// output section.

class Sections_element_dot_assignment : public Sections_element
{
 public:
  Sections_element_dot_assignment(Expression* val)
    : val_(val)
  { }

  // Finalize the symbol.
  void
  finalize_symbols(Symbol_table* symtab, const Layout* layout,
		   uint64_t* dot_value)
  {
    // We ignore the section of the result because outside of an
    // output section definition the dot symbol is always considered
    // to be absolute.
    *dot_value = this->val_->eval_with_dot(symtab, layout, true, *dot_value,
					   NULL, NULL, NULL, false);
  }

  // Update the dot symbol while setting section addresses.
  void
  set_section_addresses(Symbol_table* symtab, Layout* layout,
			uint64_t* dot_value, uint64_t* dot_alignment,
			uint64_t* load_address)
  {
    *dot_value = this->val_->eval_with_dot(symtab, layout, false, *dot_value,
					   NULL, NULL, dot_alignment, false);
    *load_address = *dot_value;
  }

  // Print for debugging.
  void
  print(FILE* f) const
  {
    fprintf(f, "  . = ");
    this->val_->print(f);
    fprintf(f, "\n");
  }

 private:
  Expression* val_;
};

// An assertion in a SECTIONS clause outside of an output section.

class Sections_element_assertion : public Sections_element
{
 public:
  Sections_element_assertion(Expression* check, const char* message,
			     size_t messagelen)
    : assertion_(check, message, messagelen)
  { }

  // Check the assertion.
  void
  finalize_symbols(Symbol_table* symtab, const Layout* layout, uint64_t*)
  { this->assertion_.check(symtab, layout); }

  // Print for debugging.
  void
  print(FILE* f) const
  {
    fprintf(f, "  ");
    this->assertion_.print(f);
  }

 private:
  Script_assertion assertion_;
};

// An element in an output section in a SECTIONS clause.

class Output_section_element
{
 public:
  // A list of input sections.
  typedef std::list<Output_section::Input_section> Input_section_list;

  Output_section_element()
  { }

  virtual ~Output_section_element()
  { }

  // Return whether this element requires an output section to exist.
  virtual bool
  needs_output_section() const
  { return false; }

  // Add any symbol being defined to the symbol table.
  virtual void
  add_symbols_to_table(Symbol_table*)
  { }

  // Finalize symbols and check assertions.
  virtual void
  finalize_symbols(Symbol_table*, const Layout*, uint64_t*, Output_section**)
  { }

  // Return whether this element matches FILE_NAME and SECTION_NAME.
  // The only real implementation is in Output_section_element_input.
  virtual bool
  match_name(const char*, const char*, bool *) const
  { return false; }

  // Set section addresses.  This includes applying assignments if the
  // expression is an absolute value.
  virtual void
  set_section_addresses(Symbol_table*, Layout*, Output_section*, uint64_t,
			uint64_t*, uint64_t*, Output_section**, std::string*,
			Input_section_list*)
  { }

  // Print the element for debugging purposes.
  virtual void
  print(FILE* f) const = 0;

 protected:
  // Return a fill string that is LENGTH bytes long, filling it with
  // FILL.
  std::string
  get_fill_string(const std::string* fill, section_size_type length) const;
};

std::string
Output_section_element::get_fill_string(const std::string* fill,
					section_size_type length) const
{
  std::string this_fill;
  this_fill.reserve(length);
  while (this_fill.length() + fill->length() <= length)
    this_fill += *fill;
  if (this_fill.length() < length)
    this_fill.append(*fill, 0, length - this_fill.length());
  return this_fill;
}

// A symbol assignment in an output section.

class Output_section_element_assignment : public Output_section_element
{
 public:
  Output_section_element_assignment(const char* name, size_t namelen,
				    Expression* val, bool provide,
				    bool hidden)
    : assignment_(name, namelen, false, val, provide, hidden)
  { }

  // Add the symbol to the symbol table.
  void
  add_symbols_to_table(Symbol_table* symtab)
  { this->assignment_.add_to_table(symtab); }

  // Finalize the symbol.
  void
  finalize_symbols(Symbol_table* symtab, const Layout* layout,
		   uint64_t* dot_value, Output_section** dot_section)
  {
    this->assignment_.finalize_with_dot(symtab, layout, *dot_value,
					*dot_section);
  }

  // Set the section address.  There is no section here, but if the
  // value is absolute, we set the symbol.  This permits us to use
  // absolute symbols when setting dot.
  void
  set_section_addresses(Symbol_table* symtab, Layout* layout, Output_section*,
			uint64_t, uint64_t* dot_value, uint64_t*,
			Output_section** dot_section, std::string*,
			Input_section_list*)
  {
    this->assignment_.set_if_absolute(symtab, layout, true, *dot_value,
				      *dot_section);
  }

  // Print for debugging.
  void
  print(FILE* f) const
  {
    fprintf(f, "    ");
    this->assignment_.print(f);
  }

 private:
  Symbol_assignment assignment_;
};

// An assignment to the dot symbol in an output section.

class Output_section_element_dot_assignment : public Output_section_element
{
 public:
  Output_section_element_dot_assignment(Expression* val)
    : val_(val)
  { }

  // An assignment to dot within an output section is enough to force
  // the output section to exist.
  bool
  needs_output_section() const
  { return true; }

  // Finalize the symbol.
  void
  finalize_symbols(Symbol_table* symtab, const Layout* layout,
		   uint64_t* dot_value, Output_section** dot_section)
  {
    *dot_value = this->val_->eval_with_dot(symtab, layout, true, *dot_value,
					   *dot_section, dot_section, NULL,
					   true);
  }

  // Update the dot symbol while setting section addresses.
  void
  set_section_addresses(Symbol_table* symtab, Layout* layout, Output_section*,
			uint64_t, uint64_t* dot_value, uint64_t*,
			Output_section** dot_section, std::string*,
			Input_section_list*);

  // Print for debugging.
  void
  print(FILE* f) const
  {
    fprintf(f, "    . = ");
    this->val_->print(f);
    fprintf(f, "\n");
  }

 private:
  Expression* val_;
};

// Update the dot symbol while setting section addresses.

void
Output_section_element_dot_assignment::set_section_addresses(
    Symbol_table* symtab,
    Layout* layout,
    Output_section* output_section,
    uint64_t,
    uint64_t* dot_value,
    uint64_t* dot_alignment,
    Output_section** dot_section,
    std::string* fill,
    Input_section_list*)
{
  uint64_t next_dot = this->val_->eval_with_dot(symtab, layout, false,
						*dot_value, *dot_section,
						dot_section, dot_alignment,
						true);
  if (next_dot < *dot_value)
    gold_error(_("dot may not move backward"));
  if (next_dot > *dot_value && output_section != NULL)
    {
      section_size_type length = convert_to_section_size_type(next_dot
							      - *dot_value);
      Output_section_data* posd;
      if (fill->empty())
	posd = new Output_data_zero_fill(length, 0);
      else
	{
	  std::string this_fill = this->get_fill_string(fill, length);
	  posd = new Output_data_const(this_fill, 0);
	}
      output_section->add_output_section_data(posd);
      layout->new_output_section_data_from_script(posd);
    }
  *dot_value = next_dot;
}

// An assertion in an output section.

class Output_section_element_assertion : public Output_section_element
{
 public:
  Output_section_element_assertion(Expression* check, const char* message,
				   size_t messagelen)
    : assertion_(check, message, messagelen)
  { }

  void
  print(FILE* f) const
  {
    fprintf(f, "    ");
    this->assertion_.print(f);
  }

 private:
  Script_assertion assertion_;
};

// We use a special instance of Output_section_data to handle BYTE,
// SHORT, etc.  This permits forward references to symbols in the
// expressions.

class Output_data_expression : public Output_section_data
{
 public:
  Output_data_expression(int size, bool is_signed, Expression* val,
			 const Symbol_table* symtab, const Layout* layout,
			 uint64_t dot_value, Output_section* dot_section)
    : Output_section_data(size, 0, true),
      is_signed_(is_signed), val_(val), symtab_(symtab),
      layout_(layout), dot_value_(dot_value), dot_section_(dot_section)
  { }

 protected:
  // Write the data to the output file.
  void
  do_write(Output_file*);

  // Write the data to a buffer.
  void
  do_write_to_buffer(unsigned char*);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** expression")); }

 private:
  template<bool big_endian>
  void
  endian_write_to_buffer(uint64_t, unsigned char*);

  bool is_signed_;
  Expression* val_;
  const Symbol_table* symtab_;
  const Layout* layout_;
  uint64_t dot_value_;
  Output_section* dot_section_;
};

// Write the data element to the output file.

void
Output_data_expression::do_write(Output_file* of)
{
  unsigned char* view = of->get_output_view(this->offset(), this->data_size());
  this->write_to_buffer(view);
  of->write_output_view(this->offset(), this->data_size(), view);
}

// Write the data element to a buffer.

void
Output_data_expression::do_write_to_buffer(unsigned char* buf)
{
  uint64_t val = this->val_->eval_with_dot(this->symtab_, this->layout_,
					   true, this->dot_value_,
					   this->dot_section_, NULL, NULL,
					   false);

  if (parameters->target().is_big_endian())
    this->endian_write_to_buffer<true>(val, buf);
  else
    this->endian_write_to_buffer<false>(val, buf);
}

template<bool big_endian>
void
Output_data_expression::endian_write_to_buffer(uint64_t val,
					       unsigned char* buf)
{
  switch (this->data_size())
    {
    case 1:
      elfcpp::Swap_unaligned<8, big_endian>::writeval(buf, val);
      break;
    case 2:
      elfcpp::Swap_unaligned<16, big_endian>::writeval(buf, val);
      break;
    case 4:
      elfcpp::Swap_unaligned<32, big_endian>::writeval(buf, val);
      break;
    case 8:
      if (parameters->target().get_size() == 32)
	{
	  val &= 0xffffffff;
	  if (this->is_signed_ && (val & 0x80000000) != 0)
	    val |= 0xffffffff00000000LL;
	}
      elfcpp::Swap_unaligned<64, big_endian>::writeval(buf, val);
      break;
    default:
      gold_unreachable();
    }
}

// A data item in an output section.

class Output_section_element_data : public Output_section_element
{
 public:
  Output_section_element_data(int size, bool is_signed, Expression* val)
    : size_(size), is_signed_(is_signed), val_(val)
  { }

  // If there is a data item, then we must create an output section.
  bool
  needs_output_section() const
  { return true; }

  // Finalize symbols--we just need to update dot.
  void
  finalize_symbols(Symbol_table*, const Layout*, uint64_t* dot_value,
		   Output_section**)
  { *dot_value += this->size_; }

  // Store the value in the section.
  void
  set_section_addresses(Symbol_table*, Layout*, Output_section*, uint64_t,
			uint64_t* dot_value, uint64_t*, Output_section**,
			std::string*, Input_section_list*);

  // Print for debugging.
  void
  print(FILE*) const;

 private:
  // The size in bytes.
  int size_;
  // Whether the value is signed.
  bool is_signed_;
  // The value.
  Expression* val_;
};

// Store the value in the section.

void
Output_section_element_data::set_section_addresses(
    Symbol_table* symtab,
    Layout* layout,
    Output_section* os,
    uint64_t,
    uint64_t* dot_value,
    uint64_t*,
    Output_section** dot_section,
    std::string*,
    Input_section_list*)
{
  gold_assert(os != NULL);
  Output_data_expression* expression =
    new Output_data_expression(this->size_, this->is_signed_, this->val_,
			       symtab, layout, *dot_value, *dot_section);
  os->add_output_section_data(expression);
  layout->new_output_section_data_from_script(expression);
  *dot_value += this->size_;
}

// Print for debugging.

void
Output_section_element_data::print(FILE* f) const
{
  const char* s;
  switch (this->size_)
    {
    case 1:
      s = "BYTE";
      break;
    case 2:
      s = "SHORT";
      break;
    case 4:
      s = "LONG";
      break;
    case 8:
      if (this->is_signed_)
	s = "SQUAD";
      else
	s = "QUAD";
      break;
    default:
      gold_unreachable();
    }
  fprintf(f, "    %s(", s);
  this->val_->print(f);
  fprintf(f, ")\n");
}

// A fill value setting in an output section.

class Output_section_element_fill : public Output_section_element
{
 public:
  Output_section_element_fill(Expression* val)
    : val_(val)
  { }

  // Update the fill value while setting section addresses.
  void
  set_section_addresses(Symbol_table* symtab, Layout* layout, Output_section*,
			uint64_t, uint64_t* dot_value, uint64_t*,
			Output_section** dot_section,
			std::string* fill, Input_section_list*)
  {
    Output_section* fill_section;
    uint64_t fill_val = this->val_->eval_with_dot(symtab, layout, false,
						  *dot_value, *dot_section,
						  &fill_section, NULL, false);
    if (fill_section != NULL)
      gold_warning(_("fill value is not absolute"));
    // FIXME: The GNU linker supports fill values of arbitrary length.
    unsigned char fill_buff[4];
    elfcpp::Swap_unaligned<32, true>::writeval(fill_buff, fill_val);
    fill->assign(reinterpret_cast<char*>(fill_buff), 4);
  }

  // Print for debugging.
  void
  print(FILE* f) const
  {
    fprintf(f, "    FILL(");
    this->val_->print(f);
    fprintf(f, ")\n");
  }

 private:
  // The new fill value.
  Expression* val_;
};

// An input section specification in an output section

class Output_section_element_input : public Output_section_element
{
 public:
  Output_section_element_input(const Input_section_spec* spec, bool keep);

  // Finalize symbols--just update the value of the dot symbol.
  void
  finalize_symbols(Symbol_table*, const Layout*, uint64_t* dot_value,
		   Output_section** dot_section)
  {
    *dot_value = this->final_dot_value_;
    *dot_section = this->final_dot_section_;
  }

  // See whether we match FILE_NAME and SECTION_NAME as an input section.
  // If we do then also indicate whether the section should be KEPT.
  bool
  match_name(const char* file_name, const char* section_name, bool* keep) const;

  // Set the section address.
  void
  set_section_addresses(Symbol_table* symtab, Layout* layout, Output_section*,
			uint64_t subalign, uint64_t* dot_value, uint64_t*,
			Output_section**, std::string* fill,
			Input_section_list*);

  // Print for debugging.
  void
  print(FILE* f) const;

 private:
  // An input section pattern.
  struct Input_section_pattern
  {
    std::string pattern;
    bool pattern_is_wildcard;
    Sort_wildcard sort;

    Input_section_pattern(const char* patterna, size_t patternlena,
			  Sort_wildcard sorta)
      : pattern(patterna, patternlena),
	pattern_is_wildcard(is_wildcard_string(this->pattern.c_str())),
	sort(sorta)
    { }
  };

  typedef std::vector<Input_section_pattern> Input_section_patterns;

  // Filename_exclusions is a pair of filename pattern and a bool
  // indicating whether the filename is a wildcard.
  typedef std::vector<std::pair<std::string, bool> > Filename_exclusions;

  // Return whether STRING matches PATTERN, where IS_WILDCARD_PATTERN
  // indicates whether this is a wildcard pattern.
  static inline bool
  match(const char* string, const char* pattern, bool is_wildcard_pattern)
  {
    return (is_wildcard_pattern
	    ? fnmatch(pattern, string, 0) == 0
	    : strcmp(string, pattern) == 0);
  }

  // See if we match a file name.
  bool
  match_file_name(const char* file_name) const;

  // The file name pattern.  If this is the empty string, we match all
  // files.
  std::string filename_pattern_;
  // Whether the file name pattern is a wildcard.
  bool filename_is_wildcard_;
  // How the file names should be sorted.  This may only be
  // SORT_WILDCARD_NONE or SORT_WILDCARD_BY_NAME.
  Sort_wildcard filename_sort_;
  // The list of file names to exclude.
  Filename_exclusions filename_exclusions_;
  // The list of input section patterns.
  Input_section_patterns input_section_patterns_;
  // Whether to keep this section when garbage collecting.
  bool keep_;
  // The value of dot after including all matching sections.
  uint64_t final_dot_value_;
  // The section where dot is defined after including all matching
  // sections.
  Output_section* final_dot_section_;
};

// Construct Output_section_element_input.  The parser records strings
// as pointers into a copy of the script file, which will go away when
// parsing is complete.  We make sure they are in std::string objects.

Output_section_element_input::Output_section_element_input(
    const Input_section_spec* spec,
    bool keep)
  : filename_pattern_(),
    filename_is_wildcard_(false),
    filename_sort_(spec->file.sort),
    filename_exclusions_(),
    input_section_patterns_(),
    keep_(keep),
    final_dot_value_(0),
    final_dot_section_(NULL)
{
  // The filename pattern "*" is common, and matches all files.  Turn
  // it into the empty string.
  if (spec->file.name.length != 1 || spec->file.name.value[0] != '*')
    this->filename_pattern_.assign(spec->file.name.value,
				   spec->file.name.length);
  this->filename_is_wildcard_ = is_wildcard_string(this->filename_pattern_.c_str());

  if (spec->input_sections.exclude != NULL)
    {
      for (String_list::const_iterator p =
	     spec->input_sections.exclude->begin();
	   p != spec->input_sections.exclude->end();
	   ++p)
	{
	  bool is_wildcard = is_wildcard_string((*p).c_str());
	  this->filename_exclusions_.push_back(std::make_pair(*p,
							      is_wildcard));
	}
    }

  if (spec->input_sections.sections != NULL)
    {
      Input_section_patterns& isp(this->input_section_patterns_);
      for (String_sort_list::const_iterator p =
	     spec->input_sections.sections->begin();
	   p != spec->input_sections.sections->end();
	   ++p)
	isp.push_back(Input_section_pattern(p->name.value, p->name.length,
					    p->sort));
    }
}

// See whether we match FILE_NAME.

bool
Output_section_element_input::match_file_name(const char* file_name) const
{
  if (!this->filename_pattern_.empty())
    {
      // If we were called with no filename, we refuse to match a
      // pattern which requires a file name.
      if (file_name == NULL)
	return false;

      if (!match(file_name, this->filename_pattern_.c_str(),
		 this->filename_is_wildcard_))
	return false;
    }

  if (file_name != NULL)
    {
      // Now we have to see whether FILE_NAME matches one of the
      // exclusion patterns, if any.
      for (Filename_exclusions::const_iterator p =
	     this->filename_exclusions_.begin();
	   p != this->filename_exclusions_.end();
	   ++p)
	{
	  if (match(file_name, p->first.c_str(), p->second))
	    return false;
	}
    }

  return true;
}

// See whether we match FILE_NAME and SECTION_NAME.  If we do then
// KEEP indicates whether the section should survive garbage collection.

bool
Output_section_element_input::match_name(const char* file_name,
					 const char* section_name,
					 bool *keep) const
{
  if (!this->match_file_name(file_name))
    return false;

  *keep = this->keep_;

  // If there are no section name patterns, then we match.
  if (this->input_section_patterns_.empty())
    return true;

  // See whether we match the section name patterns.
  for (Input_section_patterns::const_iterator p =
	 this->input_section_patterns_.begin();
       p != this->input_section_patterns_.end();
       ++p)
    {
      if (match(section_name, p->pattern.c_str(), p->pattern_is_wildcard))
	return true;
    }

  // We didn't match any section names, so we didn't match.
  return false;
}

// Information we use to sort the input sections.

class Input_section_info
{
 public:
  Input_section_info(const Output_section::Input_section& input_section)
    : input_section_(input_section), section_name_(),
      size_(0), addralign_(1)
  { }

  // Return the simple input section.
  const Output_section::Input_section&
  input_section() const
  { return this->input_section_; }

  // Return the object.
  Relobj*
  relobj() const
  { return this->input_section_.relobj(); }

  // Return the section index.
  unsigned int
  shndx()
  { return this->input_section_.shndx(); }

  // Return the section name.
  const std::string&
  section_name() const
  { return this->section_name_; }

  // Set the section name.
  void
  set_section_name(const std::string name)
  {
    if (is_compressed_debug_section(name.c_str()))
      this->section_name_ = corresponding_uncompressed_section_name(name);
    else
      this->section_name_ = name;
  }

  // Return the section size.
  uint64_t
  size() const
  { return this->size_; }

  // Set the section size.
  void
  set_size(uint64_t size)
  { this->size_ = size; }

  // Return the address alignment.
  uint64_t
  addralign() const
  { return this->addralign_; }

  // Set the address alignment.
  void
  set_addralign(uint64_t addralign)
  { this->addralign_ = addralign; }

 private:
  // Input section, can be a relaxed section.
  Output_section::Input_section input_section_;
  // Name of the section.
  std::string section_name_;
  // Section size.
  uint64_t size_;
  // Address alignment.
  uint64_t addralign_;
};

// A class to sort the input sections.

class Input_section_sorter
{
 public:
  Input_section_sorter(Sort_wildcard filename_sort, Sort_wildcard section_sort)
    : filename_sort_(filename_sort), section_sort_(section_sort)
  { }

  bool
  operator()(const Input_section_info&, const Input_section_info&) const;

 private:
  static unsigned long
  get_init_priority(const char*);

  Sort_wildcard filename_sort_;
  Sort_wildcard section_sort_;
};

// Return a relative priority of the section with the specified NAME
// (a lower value meand a higher priority), or 0 if it should be compared
// with others as strings.
// The implementation of this function is copied from ld/ldlang.c.

unsigned long
Input_section_sorter::get_init_priority(const char* name)
{
  char* end;
  unsigned long init_priority;

  // GCC uses the following section names for the init_priority
  // attribute with numerical values 101 and 65535 inclusive. A
  // lower value means a higher priority.
  //
  // 1: .init_array.NNNN/.fini_array.NNNN: Where NNNN is the
  //    decimal numerical value of the init_priority attribute.
  //    The order of execution in .init_array is forward and
  //    .fini_array is backward.
  // 2: .ctors.NNNN/.dtors.NNNN: Where NNNN is 65535 minus the
  //    decimal numerical value of the init_priority attribute.
  //    The order of execution in .ctors is backward and .dtors
  //    is forward.

  if (strncmp(name, ".init_array.", 12) == 0
      || strncmp(name, ".fini_array.", 12) == 0)
    {
      init_priority = strtoul(name + 12, &end, 10);
      return *end ? 0 : init_priority;
    }
  else if (strncmp(name, ".ctors.", 7) == 0
	   || strncmp(name, ".dtors.", 7) == 0)
    {
      init_priority = strtoul(name + 7, &end, 10);
      return *end ? 0 : 65535 - init_priority;
    }

  return 0;
}

bool
Input_section_sorter::operator()(const Input_section_info& isi1,
				 const Input_section_info& isi2) const
{
  if (this->section_sort_ == SORT_WILDCARD_BY_INIT_PRIORITY)
    {
      unsigned long ip1 = get_init_priority(isi1.section_name().c_str());
      unsigned long ip2 = get_init_priority(isi2.section_name().c_str());
      if (ip1 != 0 && ip2 != 0 && ip1 != ip2)
	return ip1 < ip2;
    }
  if (this->section_sort_ == SORT_WILDCARD_BY_NAME
      || this->section_sort_ == SORT_WILDCARD_BY_NAME_BY_ALIGNMENT
      || (this->section_sort_ == SORT_WILDCARD_BY_ALIGNMENT_BY_NAME
	  && isi1.addralign() == isi2.addralign())
      || this->section_sort_ == SORT_WILDCARD_BY_INIT_PRIORITY)
    {
      if (isi1.section_name() != isi2.section_name())
	return isi1.section_name() < isi2.section_name();
    }
  if (this->section_sort_ == SORT_WILDCARD_BY_ALIGNMENT
      || this->section_sort_ == SORT_WILDCARD_BY_NAME_BY_ALIGNMENT
      || this->section_sort_ == SORT_WILDCARD_BY_ALIGNMENT_BY_NAME)
    {
      if (isi1.addralign() != isi2.addralign())
	return isi1.addralign() < isi2.addralign();
    }
  if (this->filename_sort_ == SORT_WILDCARD_BY_NAME)
    {
      if (isi1.relobj()->name() != isi2.relobj()->name())
	return (isi1.relobj()->name() < isi2.relobj()->name());
    }

  // Otherwise we leave them in the same order.
  return false;
}

// Set the section address.  Look in INPUT_SECTIONS for sections which
// match this spec, sort them as specified, and add them to the output
// section.

void
Output_section_element_input::set_section_addresses(
    Symbol_table*,
    Layout* layout,
    Output_section* output_section,
    uint64_t subalign,
    uint64_t* dot_value,
    uint64_t*,
    Output_section** dot_section,
    std::string* fill,
    Input_section_list* input_sections)
{
  // We build a list of sections which match each
  // Input_section_pattern.

  // If none of the patterns specify a sort option, we throw all
  // matching input sections into a single bin, in the order we
  // find them.  Otherwise, we put matching input sections into
  // a separate bin for each pattern, and sort each one as
  // specified.  Thus, an input section spec like this:
  //   *(.foo .bar)
  // will group all .foo and .bar sections in the order seen,
  // whereas this:
  //   *(.foo) *(.bar)
  // will group all .foo sections followed by all .bar sections.
  // This matches Gnu ld behavior.

  // Things get really weird, though, when you add a sort spec
  // on some, but not all, of the patterns, like this:
  //   *(SORT_BY_NAME(.foo) .bar)
  // We do not attempt to match Gnu ld behavior in this case.

  typedef std::vector<std::vector<Input_section_info> > Matching_sections;
  size_t input_pattern_count = this->input_section_patterns_.size();
  size_t bin_count = 1;
  bool any_patterns_with_sort = false;
  for (size_t i = 0; i < input_pattern_count; ++i)
    {
      const Input_section_pattern& isp(this->input_section_patterns_[i]);
      if (isp.sort != SORT_WILDCARD_NONE)
	any_patterns_with_sort = true;
    }
  if (any_patterns_with_sort)
    bin_count = input_pattern_count;
  Matching_sections matching_sections(bin_count);

  // Look through the list of sections for this output section.  Add
  // each one which matches to one of the elements of
  // MATCHING_SECTIONS.

  Input_section_list::iterator p = input_sections->begin();
  while (p != input_sections->end())
    {
      Relobj* relobj = p->relobj();
      unsigned int shndx = p->shndx();
      Input_section_info isi(*p);

      // Calling section_name and section_addralign is not very
      // efficient.

      // Lock the object so that we can get information about the
      // section.  This is OK since we know we are single-threaded
      // here.
      {
	const Task* task = reinterpret_cast<const Task*>(-1);
	Task_lock_obj<Object> tl(task, relobj);

	isi.set_section_name(relobj->section_name(shndx));
	if (p->is_relaxed_input_section())
	  {
	    // We use current data size because relaxed section sizes may not
	    // have finalized yet.
	    isi.set_size(p->relaxed_input_section()->current_data_size());
	    isi.set_addralign(p->relaxed_input_section()->addralign());
	  }
	else
	  {
	    isi.set_size(relobj->section_size(shndx));
	    isi.set_addralign(relobj->section_addralign(shndx));
	  }
      }

      if (!this->match_file_name(relobj->name().c_str()))
	++p;
      else if (this->input_section_patterns_.empty())
	{
	  matching_sections[0].push_back(isi);
	  p = input_sections->erase(p);
	}
      else
	{
	  size_t i;
	  for (i = 0; i < input_pattern_count; ++i)
	    {
	      const Input_section_pattern&
		isp(this->input_section_patterns_[i]);
	      if (match(isi.section_name().c_str(), isp.pattern.c_str(),
			isp.pattern_is_wildcard))
		break;
	    }

	  if (i >= input_pattern_count)
	    ++p;
	  else
	    {
	      if (i >= bin_count)
		i = 0;
	      matching_sections[i].push_back(isi);
	      p = input_sections->erase(p);
	    }
	}
    }

  // Look through MATCHING_SECTIONS.  Sort each one as specified,
  // using a stable sort so that we get the default order when
  // sections are otherwise equal.  Add each input section to the
  // output section.

  uint64_t dot = *dot_value;
  for (size_t i = 0; i < bin_count; ++i)
    {
      if (matching_sections[i].empty())
	continue;

      gold_assert(output_section != NULL);

      const Input_section_pattern& isp(this->input_section_patterns_[i]);
      if (isp.sort != SORT_WILDCARD_NONE
	  || this->filename_sort_ != SORT_WILDCARD_NONE)
	std::stable_sort(matching_sections[i].begin(),
			 matching_sections[i].end(),
			 Input_section_sorter(this->filename_sort_,
					      isp.sort));

      for (std::vector<Input_section_info>::const_iterator p =
	     matching_sections[i].begin();
	   p != matching_sections[i].end();
	   ++p)
	{
	  // Override the original address alignment if SUBALIGN is specified.
	  // We need to make a copy of the input section to modify the
	  // alignment.
	  Output_section::Input_section sis(p->input_section());

	  uint64_t this_subalign = sis.addralign();
	  if (!sis.is_input_section())
	    sis.output_section_data()->finalize_data_size();
	  uint64_t data_size = sis.data_size();
	  if (subalign > 0)
	    {
	      this_subalign = subalign;
	      sis.set_addralign(subalign);
	    }

	  uint64_t address = align_address(dot, this_subalign);

	  if (address > dot && !fill->empty())
	    {
	      section_size_type length =
		convert_to_section_size_type(address - dot);
	      std::string this_fill = this->get_fill_string(fill, length);
	      Output_section_data* posd = new Output_data_const(this_fill, 0);
	      output_section->add_output_section_data(posd);
	      layout->new_output_section_data_from_script(posd);
	    }

	  output_section->add_script_input_section(sis);
	  dot = address + data_size;
	}
    }

  // An SHF_TLS/SHT_NOBITS section does not take up any
  // address space.
  if (output_section == NULL
      || (output_section->flags() & elfcpp::SHF_TLS) == 0
      || output_section->type() != elfcpp::SHT_NOBITS)
    *dot_value = dot;

  this->final_dot_value_ = *dot_value;
  this->final_dot_section_ = *dot_section;
}

// Print for debugging.

void
Output_section_element_input::print(FILE* f) const
{
  fprintf(f, "    ");

  if (this->keep_)
    fprintf(f, "KEEP(");

  if (!this->filename_pattern_.empty())
    {
      bool need_close_paren = false;
      switch (this->filename_sort_)
	{
	case SORT_WILDCARD_NONE:
	  break;
	case SORT_WILDCARD_BY_NAME:
	  fprintf(f, "SORT_BY_NAME(");
	  need_close_paren = true;
	  break;
	default:
	  gold_unreachable();
	}

      fprintf(f, "%s", this->filename_pattern_.c_str());

      if (need_close_paren)
	fprintf(f, ")");
    }

  if (!this->input_section_patterns_.empty()
      || !this->filename_exclusions_.empty())
    {
      fprintf(f, "(");

      bool need_space = false;
      if (!this->filename_exclusions_.empty())
	{
	  fprintf(f, "EXCLUDE_FILE(");
	  bool need_comma = false;
	  for (Filename_exclusions::const_iterator p =
		 this->filename_exclusions_.begin();
	       p != this->filename_exclusions_.end();
	       ++p)
	    {
	      if (need_comma)
		fprintf(f, ", ");
	      fprintf(f, "%s", p->first.c_str());
	      need_comma = true;
	    }
	  fprintf(f, ")");
	  need_space = true;
	}

      for (Input_section_patterns::const_iterator p =
	     this->input_section_patterns_.begin();
	   p != this->input_section_patterns_.end();
	   ++p)
	{
	  if (need_space)
	    fprintf(f, " ");

	  int close_parens = 0;
	  switch (p->sort)
	    {
	    case SORT_WILDCARD_NONE:
	      break;
	    case SORT_WILDCARD_BY_NAME:
	      fprintf(f, "SORT_BY_NAME(");
	      close_parens = 1;
	      break;
	    case SORT_WILDCARD_BY_ALIGNMENT:
	      fprintf(f, "SORT_BY_ALIGNMENT(");
	      close_parens = 1;
	      break;
	    case SORT_WILDCARD_BY_NAME_BY_ALIGNMENT:
	      fprintf(f, "SORT_BY_NAME(SORT_BY_ALIGNMENT(");
	      close_parens = 2;
	      break;
	    case SORT_WILDCARD_BY_ALIGNMENT_BY_NAME:
	      fprintf(f, "SORT_BY_ALIGNMENT(SORT_BY_NAME(");
	      close_parens = 2;
	      break;
	    case SORT_WILDCARD_BY_INIT_PRIORITY:
	      fprintf(f, "SORT_BY_INIT_PRIORITY(");
	      close_parens = 1;
	      break;
	    default:
	      gold_unreachable();
	    }

	  fprintf(f, "%s", p->pattern.c_str());

	  for (int i = 0; i < close_parens; ++i)
	    fprintf(f, ")");

	  need_space = true;
	}

      fprintf(f, ")");
    }

  if (this->keep_)
    fprintf(f, ")");

  fprintf(f, "\n");
}

// An output section.

class Output_section_definition : public Sections_element
{
 public:
  typedef Output_section_element::Input_section_list Input_section_list;

  Output_section_definition(const char* name, size_t namelen,
			    const Parser_output_section_header* header);

  // Finish the output section with the information in the trailer.
  void
  finish(const Parser_output_section_trailer* trailer);

  // Add a symbol to be defined.
  void
  add_symbol_assignment(const char* name, size_t length, Expression* value,
			bool provide, bool hidden);

  // Add an assignment to the special dot symbol.
  void
  add_dot_assignment(Expression* value);

  // Add an assertion.
  void
  add_assertion(Expression* check, const char* message, size_t messagelen);

  // Add a data item to the current output section.
  void
  add_data(int size, bool is_signed, Expression* val);

  // Add a setting for the fill value.
  void
  add_fill(Expression* val);

  // Add an input section specification.
  void
  add_input_section(const Input_section_spec* spec, bool keep);

  // Return whether the output section is relro.
  bool
  is_relro() const
  { return this->is_relro_; }

  // Record that the output section is relro.
  void
  set_is_relro()
  { this->is_relro_ = true; }

  // Create any required output sections.
  void
  create_sections(Layout*);

  // Add any symbols being defined to the symbol table.
  void
  add_symbols_to_table(Symbol_table* symtab);

  // Finalize symbols and check assertions.
  void
  finalize_symbols(Symbol_table*, const Layout*, uint64_t*);

  // Return the output section name to use for an input file name and
  // section name.
  const char*
  output_section_name(const char* file_name, const char* section_name,
		      Output_section***, Script_sections::Section_type*,
		      bool*, bool);

  // Initialize OSP with an output section.
  void
  orphan_section_init(Orphan_section_placement* osp,
		      Script_sections::Elements_iterator p)
  { osp->output_section_init(this->name_, this->output_section_, p); }

  // Set the section address.
  void
  set_section_addresses(Symbol_table* symtab, Layout* layout,
			uint64_t* dot_value, uint64_t*,
			uint64_t* load_address);

  // Check a constraint (ONLY_IF_RO, etc.) on an output section.  If
  // this section is constrained, and the input sections do not match,
  // return the constraint, and set *POSD.
  Section_constraint
  check_constraint(Output_section_definition** posd);

  // See if this is the alternate output section for a constrained
  // output section.  If it is, transfer the Output_section and return
  // true.  Otherwise return false.
  bool
  alternate_constraint(Output_section_definition*, Section_constraint);

  // Get the list of segments to use for an allocated section when
  // using a PHDRS clause.
  Output_section*
  allocate_to_segment(String_list** phdrs_list, bool* orphan);

  // Look for an output section by name and return the address, the
  // load address, the alignment, and the size.  This is used when an
  // expression refers to an output section which was not actually
  // created.  This returns true if the section was found, false
  // otherwise.
  bool
  get_output_section_info(const char*, uint64_t*, uint64_t*, uint64_t*,
                          uint64_t*) const;

  // Return the associated Output_section if there is one.
  Output_section*
  get_output_section() const
  { return this->output_section_; }

  // Print the contents to the FILE.  This is for debugging.
  void
  print(FILE*) const;

  // Return the output section type if specified or Script_sections::ST_NONE.
  Script_sections::Section_type
  section_type() const;

  // Store the memory region to use.
  void
  set_memory_region(Memory_region*, bool set_vma);

  void
  set_section_vma(Expression* address)
  { this->address_ = address; }

  void
  set_section_lma(Expression* address)
  { this->load_address_ = address; }

  const std::string&
  get_section_name() const
  { return this->name_; }

 private:
  static const char*
  script_section_type_name(Script_section_type);

  typedef std::vector<Output_section_element*> Output_section_elements;

  // The output section name.
  std::string name_;
  // The address.  This may be NULL.
  Expression* address_;
  // The load address.  This may be NULL.
  Expression* load_address_;
  // The alignment.  This may be NULL.
  Expression* align_;
  // The input section alignment.  This may be NULL.
  Expression* subalign_;
  // The constraint, if any.
  Section_constraint constraint_;
  // The fill value.  This may be NULL.
  Expression* fill_;
  // The list of segments this section should go into.  This may be
  // NULL.
  String_list* phdrs_;
  // The list of elements defining the section.
  Output_section_elements elements_;
  // The Output_section created for this definition.  This will be
  // NULL if none was created.
  Output_section* output_section_;
  // The address after it has been evaluated.
  uint64_t evaluated_address_;
  // The load address after it has been evaluated.
  uint64_t evaluated_load_address_;
  // The alignment after it has been evaluated.
  uint64_t evaluated_addralign_;
  // The output section is relro.
  bool is_relro_;
  // The output section type if specified.
  enum Script_section_type script_section_type_;
};

// Constructor.

Output_section_definition::Output_section_definition(
    const char* name,
    size_t namelen,
    const Parser_output_section_header* header)
  : name_(name, namelen),
    address_(header->address),
    load_address_(header->load_address),
    align_(header->align),
    subalign_(header->subalign),
    constraint_(header->constraint),
    fill_(NULL),
    phdrs_(NULL),
    elements_(),
    output_section_(NULL),
    evaluated_address_(0),
    evaluated_load_address_(0),
    evaluated_addralign_(0),
    is_relro_(false),
    script_section_type_(header->section_type)
{
}

// Finish an output section.

void
Output_section_definition::finish(const Parser_output_section_trailer* trailer)
{
  this->fill_ = trailer->fill;
  this->phdrs_ = trailer->phdrs;
}

// Add a symbol to be defined.

void
Output_section_definition::add_symbol_assignment(const char* name,
						 size_t length,
						 Expression* value,
						 bool provide,
						 bool hidden)
{
  Output_section_element* p = new Output_section_element_assignment(name,
								    length,
								    value,
								    provide,
								    hidden);
  this->elements_.push_back(p);
}

// Add an assignment to the special dot symbol.

void
Output_section_definition::add_dot_assignment(Expression* value)
{
  Output_section_element* p = new Output_section_element_dot_assignment(value);
  this->elements_.push_back(p);
}

// Add an assertion.

void
Output_section_definition::add_assertion(Expression* check,
					 const char* message,
					 size_t messagelen)
{
  Output_section_element* p = new Output_section_element_assertion(check,
								   message,
								   messagelen);
  this->elements_.push_back(p);
}

// Add a data item to the current output section.

void
Output_section_definition::add_data(int size, bool is_signed, Expression* val)
{
  Output_section_element* p = new Output_section_element_data(size, is_signed,
							      val);
  this->elements_.push_back(p);
}

// Add a setting for the fill value.

void
Output_section_definition::add_fill(Expression* val)
{
  Output_section_element* p = new Output_section_element_fill(val);
  this->elements_.push_back(p);
}

// Add an input section specification.

void
Output_section_definition::add_input_section(const Input_section_spec* spec,
					     bool keep)
{
  Output_section_element* p = new Output_section_element_input(spec, keep);
  this->elements_.push_back(p);
}

// Create any required output sections.  We need an output section if
// there is a data statement here.

void
Output_section_definition::create_sections(Layout* layout)
{
  if (this->output_section_ != NULL)
    return;
  for (Output_section_elements::const_iterator p = this->elements_.begin();
       p != this->elements_.end();
       ++p)
    {
      if ((*p)->needs_output_section())
	{
	  const char* name = this->name_.c_str();
	  this->output_section_ =
	    layout->make_output_section_for_script(name, this->section_type());
	  return;
	}
    }
}

// Add any symbols being defined to the symbol table.

void
Output_section_definition::add_symbols_to_table(Symbol_table* symtab)
{
  for (Output_section_elements::iterator p = this->elements_.begin();
       p != this->elements_.end();
       ++p)
    (*p)->add_symbols_to_table(symtab);
}

// Finalize symbols and check assertions.

void
Output_section_definition::finalize_symbols(Symbol_table* symtab,
					    const Layout* layout,
					    uint64_t* dot_value)
{
  if (this->output_section_ != NULL)
    *dot_value = this->output_section_->address();
  else
    {
      uint64_t address = *dot_value;
      if (this->address_ != NULL)
	{
	  address = this->address_->eval_with_dot(symtab, layout, true,
						  *dot_value, NULL,
						  NULL, NULL, false);
	}
      if (this->align_ != NULL)
	{
	  uint64_t align = this->align_->eval_with_dot(symtab, layout, true,
						       *dot_value, NULL,
						       NULL, NULL, false);
	  address = align_address(address, align);
	}
      *dot_value = address;
    }

  Output_section* dot_section = this->output_section_;
  for (Output_section_elements::iterator p = this->elements_.begin();
       p != this->elements_.end();
       ++p)
    (*p)->finalize_symbols(symtab, layout, dot_value, &dot_section);
}

// Return the output section name to use for an input section name.

const char*
Output_section_definition::output_section_name(
    const char* file_name,
    const char* section_name,
    Output_section*** slot,
    Script_sections::Section_type* psection_type,
    bool* keep,
    bool match_input_spec)
{
  // If the section is a linker-created output section, just look for a match
  // on the output section name.
  if (!match_input_spec && this->name_ != "/DISCARD/")
    {
      if (this->name_ != section_name)
	return NULL;
      *slot = &this->output_section_;
      *psection_type = this->section_type();
      return this->name_.c_str();
    }

  // Ask each element whether it matches NAME.
  for (Output_section_elements::const_iterator p = this->elements_.begin();
       p != this->elements_.end();
       ++p)
    {
      if ((*p)->match_name(file_name, section_name, keep))
	{
	  // We found a match for NAME, which means that it should go
	  // into this output section.
	  *slot = &this->output_section_;
	  *psection_type = this->section_type();
	  return this->name_.c_str();
	}
    }

  // We don't know about this section name.
  return NULL;
}

// Return true if memory from START to START + LENGTH is contained
// within a memory region.

bool
Script_sections::block_in_region(Symbol_table* symtab, Layout* layout,
				 uint64_t start, uint64_t length) const
{
  if (this->memory_regions_ == NULL)
    return false;

  for (Memory_regions::const_iterator mr = this->memory_regions_->begin();
       mr != this->memory_regions_->end();
       ++mr)
    {
      uint64_t s = (*mr)->start_address()->eval(symtab, layout, false);
      uint64_t l = (*mr)->length()->eval(symtab, layout, false);

      if (s <= start
	  && (s + l) >= (start + length))
	return true;
    }

  return false;
}

// Find a memory region that should be used by a given output SECTION.
// If provided set PREVIOUS_SECTION_RETURN to point to the last section
// that used the return memory region.

Memory_region*
Script_sections::find_memory_region(
    Output_section_definition* section,
    bool find_vma_region,
    bool explicit_only,
    Output_section_definition** previous_section_return)
{
  if (previous_section_return != NULL)
    * previous_section_return = NULL;

  // Walk the memory regions specified in this script, if any.
  if (this->memory_regions_ == NULL)
    return NULL;

  // The /DISCARD/ section never gets assigned to any region.
  if (section->get_section_name() == "/DISCARD/")
    return NULL;

  Memory_region* first_match = NULL;

  // First check to see if a region has been assigned to this section.
  for (Memory_regions::const_iterator mr = this->memory_regions_->begin();
       mr != this->memory_regions_->end();
       ++mr)
    {
      if (find_vma_region)
	{
	  for (Memory_region::Section_list::const_iterator s =
		 (*mr)->get_vma_section_list_start();
	       s != (*mr)->get_vma_section_list_end();
	       ++s)
	    if ((*s) == section)
	      {
		(*mr)->set_last_section(section);
		return *mr;
	      }
	}
      else
	{
	  for (Memory_region::Section_list::const_iterator s =
		 (*mr)->get_lma_section_list_start();
	       s != (*mr)->get_lma_section_list_end();
	       ++s)
	    if ((*s) == section)
	      {
		(*mr)->set_last_section(section);
		return *mr;
	      }
	}

      if (!explicit_only)
	{
	  // Make a note of the first memory region whose attributes
	  // are compatible with the section.  If we do not find an
	  // explicit region assignment, then we will return this region.
	  Output_section* out_sec = section->get_output_section();
	  if (first_match == NULL
	      && out_sec != NULL
	      && (*mr)->attributes_compatible(out_sec->flags(),
					      out_sec->type()))
	    first_match = *mr;
	}
    }

  // With LMA computations, if an explicit region has not been specified then
  // we will want to set the difference between the VMA and the LMA of the
  // section were searching for to be the same as the difference between the
  // VMA and LMA of the last section to be added to first matched region.
  // Hence, if it was asked for, we return a pointer to the last section
  // known to be used by the first matched region.
  if (first_match != NULL
      && previous_section_return != NULL)
    *previous_section_return = first_match->get_last_section();

  return first_match;
}

// Set the section address.  Note that the OUTPUT_SECTION_ field will
// be NULL if no input sections were mapped to this output section.
// We still have to adjust dot and process symbol assignments.

void
Output_section_definition::set_section_addresses(Symbol_table* symtab,
						 Layout* layout,
						 uint64_t* dot_value,
						 uint64_t* dot_alignment,
                                                 uint64_t* load_address)
{
  Memory_region* vma_region = NULL;
  Memory_region* lma_region = NULL;
  Script_sections* script_sections =
    layout->script_options()->script_sections();
  uint64_t address;
  uint64_t old_dot_value = *dot_value;
  uint64_t old_load_address = *load_address;

  // If input section sorting is requested via --section-ordering-file or
  // linker plugins, then do it here.  This is important because we want
  // any sorting specified in the linker scripts, which will be done after
  // this, to take precedence.  The final order of input sections is then
  // guaranteed to be according to the linker script specification.
  if (this->output_section_ != NULL
      && this->output_section_->input_section_order_specified())
    this->output_section_->sort_attached_input_sections();

  // Decide the start address for the section.  The algorithm is:
  // 1) If an address has been specified in a linker script, use that.
  // 2) Otherwise if a memory region has been specified for the section,
  //    use the next free address in the region.
  // 3) Otherwise if memory regions have been specified find the first
  //    region whose attributes are compatible with this section and
  //    install it into that region.
  // 4) Otherwise use the current location counter.

  if (this->output_section_ != NULL
      // Check for --section-start.
      && parameters->options().section_start(this->output_section_->name(),
					     &address))
    ;
  else if (this->address_ == NULL)
    {
      vma_region = script_sections->find_memory_region(this, true, false, NULL);
      if (vma_region != NULL)
	address = vma_region->get_current_address()->eval(symtab, layout,
							  false);
      else
	address = *dot_value;
    }
  else
    {
      vma_region = script_sections->find_memory_region(this, true, true, NULL);
      address = this->address_->eval_with_dot(symtab, layout, true,
					      *dot_value, NULL, NULL,
					      dot_alignment, false);
      if (vma_region != NULL)
	vma_region->set_address(address, symtab, layout);
    }

  uint64_t align;
  if (this->align_ == NULL)
    {
      if (this->output_section_ == NULL)
	align = 0;
      else
	align = this->output_section_->addralign();
    }
  else
    {
      Output_section* align_section;
      align = this->align_->eval_with_dot(symtab, layout, true, *dot_value,
					  NULL, &align_section, NULL, false);
      if (align_section != NULL)
	gold_warning(_("alignment of section %s is not absolute"),
		     this->name_.c_str());
      if (this->output_section_ != NULL)
	this->output_section_->set_addralign(align);
    }

  uint64_t subalign;
  if (this->subalign_ == NULL)
    subalign = 0;
  else
    {
      Output_section* subalign_section;
      subalign = this->subalign_->eval_with_dot(symtab, layout, true,
						*dot_value, NULL,
						&subalign_section, NULL,
						false);
      if (subalign_section != NULL)
	gold_warning(_("subalign of section %s is not absolute"),
		     this->name_.c_str());

      // Reserve a value of 0 to mean there is no SUBALIGN property.
      if (subalign == 0)
	subalign = 1;

      // The external alignment of the output section must be at least
      // as large as that of the input sections.  If there is no
      // explicit ALIGN property, we set the output section alignment
      // to match the input section alignment.
      if (align < subalign || this->align_ == NULL)
	{
	  align = subalign;
	  this->output_section_->set_addralign(align);
	}
    }

  address = align_address(address, align);

  uint64_t start_address = address;

  *dot_value = address;

  // Except for NOLOAD sections, the address of non-SHF_ALLOC sections is
  // forced to zero, regardless of what the linker script wants.
  if (this->output_section_ != NULL
      && ((this->output_section_->flags() & elfcpp::SHF_ALLOC) != 0
	  || this->output_section_->is_noload()))
    this->output_section_->set_address(address);

  this->evaluated_address_ = address;
  this->evaluated_addralign_ = align;

  uint64_t laddr;

  if (this->load_address_ == NULL)
    {
      Output_section_definition* previous_section;

      // Determine if an LMA region has been set for this section.
      lma_region = script_sections->find_memory_region(this, false, false,
						       &previous_section);

      if (lma_region != NULL)
	{
	  if (previous_section == NULL)
	    // The LMA address was explicitly set to the given region.
	    laddr = lma_region->get_current_address()->eval(symtab, layout,
							    false);
	  else
	    {
	      // We are not going to use the discovered lma_region, so
	      // make sure that we do not update it in the code below.
	      lma_region = NULL;

	      if (this->address_ != NULL || previous_section == this)
		{
		  // Either an explicit VMA address has been set, or an
		  // explicit VMA region has been set, so set the LMA equal to
		  // the VMA.
		  laddr = address;
		}
	      else
		{
		  // The LMA address was not explicitly or implicitly set.
		  //
		  // We have been given the first memory region that is
		  // compatible with the current section and a pointer to the
		  // last section to use this region.  Set the LMA of this
		  // section so that the difference between its' VMA and LMA
		  // is the same as the difference between the VMA and LMA of
		  // the last section in the given region.
		  laddr = address + (previous_section->evaluated_load_address_
				     - previous_section->evaluated_address_);
		}
	    }

	  if (this->output_section_ != NULL)
	    this->output_section_->set_load_address(laddr);
	}
      else
	{
	  // Do not set the load address of the output section, if one exists.
	  // This allows future sections to determine what the load address
	  // should be.  If none is ever set, it will default to being the
	  // same as the vma address.
	  laddr = address;
	}
    }
  else
    {
      laddr = this->load_address_->eval_with_dot(symtab, layout, true,
						 *dot_value,
						 this->output_section_,
						 NULL, NULL, false);
      if (this->output_section_ != NULL)
        this->output_section_->set_load_address(laddr);
    }

  this->evaluated_load_address_ = laddr;

  std::string fill;
  if (this->fill_ != NULL)
    {
      // FIXME: The GNU linker supports fill values of arbitrary
      // length.
      Output_section* fill_section;
      uint64_t fill_val = this->fill_->eval_with_dot(symtab, layout, true,
						     *dot_value,
						     NULL, &fill_section,
						     NULL, false);
      if (fill_section != NULL)
	gold_warning(_("fill of section %s is not absolute"),
		     this->name_.c_str());
      unsigned char fill_buff[4];
      elfcpp::Swap_unaligned<32, true>::writeval(fill_buff, fill_val);
      fill.assign(reinterpret_cast<char*>(fill_buff), 4);
    }

  Input_section_list input_sections;
  if (this->output_section_ != NULL)
    {
      // Get the list of input sections attached to this output
      // section.  This will leave the output section with only
      // Output_section_data entries.
      address += this->output_section_->get_input_sections(address,
							   fill,
							   &input_sections);
      *dot_value = address;
    }

  Output_section* dot_section = this->output_section_;
  for (Output_section_elements::iterator p = this->elements_.begin();
       p != this->elements_.end();
       ++p)
    (*p)->set_section_addresses(symtab, layout, this->output_section_,
				subalign, dot_value, dot_alignment,
				&dot_section, &fill, &input_sections);

  gold_assert(input_sections.empty());

  if (vma_region != NULL)
    {
      // Update the VMA region being used by the section now that we know how
      // big it is.  Use the current address in the region, rather than
      // start_address because that might have been aligned upwards and we
      // need to allow for the padding.
      Expression* addr = vma_region->get_current_address();
      uint64_t size = *dot_value - addr->eval(symtab, layout, false);

      vma_region->increment_offset(this->get_section_name(), size,
				   symtab, layout);
    }

  // If the LMA region is different from the VMA region, then increment the
  // offset there as well.  Note that we use the same "dot_value -
  // start_address" formula that is used in the load_address assignment below.
  if (lma_region != NULL && lma_region != vma_region)
    lma_region->increment_offset(this->get_section_name(),
				 *dot_value - start_address,
				 symtab, layout);

  // Compute the load address for the following section.
  if (this->output_section_ == NULL)
    *load_address = *dot_value;
  else if (this->load_address_ == NULL)
    {
      if (lma_region == NULL)
	*load_address = *dot_value;
      else
	*load_address =
	  lma_region->get_current_address()->eval(symtab, layout, false);
    }
  else
    *load_address = (this->output_section_->load_address()
                     + (*dot_value - start_address));

  if (this->output_section_ != NULL)
    {
      if (this->is_relro_)
	this->output_section_->set_is_relro();
      else
	this->output_section_->clear_is_relro();

      // If this is a NOLOAD section, keep dot and load address unchanged.
      if (this->output_section_->is_noload())
	{
	  *dot_value = old_dot_value;
	  *load_address = old_load_address;
	}
    }
}

// Check a constraint (ONLY_IF_RO, etc.) on an output section.  If
// this section is constrained, and the input sections do not match,
// return the constraint, and set *POSD.

Section_constraint
Output_section_definition::check_constraint(Output_section_definition** posd)
{
  switch (this->constraint_)
    {
    case CONSTRAINT_NONE:
      return CONSTRAINT_NONE;

    case CONSTRAINT_ONLY_IF_RO:
      if (this->output_section_ != NULL
	  && (this->output_section_->flags() & elfcpp::SHF_WRITE) != 0)
	{
	  *posd = this;
	  return CONSTRAINT_ONLY_IF_RO;
	}
      return CONSTRAINT_NONE;

    case CONSTRAINT_ONLY_IF_RW:
      if (this->output_section_ != NULL
	  && (this->output_section_->flags() & elfcpp::SHF_WRITE) == 0)
	{
	  *posd = this;
	  return CONSTRAINT_ONLY_IF_RW;
	}
      return CONSTRAINT_NONE;

    case CONSTRAINT_SPECIAL:
      if (this->output_section_ != NULL)
	gold_error(_("SPECIAL constraints are not implemented"));
      return CONSTRAINT_NONE;

    default:
      gold_unreachable();
    }
}

// See if this is the alternate output section for a constrained
// output section.  If it is, transfer the Output_section and return
// true.  Otherwise return false.

bool
Output_section_definition::alternate_constraint(
    Output_section_definition* posd,
    Section_constraint constraint)
{
  if (this->name_ != posd->name_)
    return false;

  switch (constraint)
    {
    case CONSTRAINT_ONLY_IF_RO:
      if (this->constraint_ != CONSTRAINT_ONLY_IF_RW)
	return false;
      break;

    case CONSTRAINT_ONLY_IF_RW:
      if (this->constraint_ != CONSTRAINT_ONLY_IF_RO)
	return false;
      break;

    default:
      gold_unreachable();
    }

  // We have found the alternate constraint.  We just need to move
  // over the Output_section.  When constraints are used properly,
  // THIS should not have an output_section pointer, as all the input
  // sections should have matched the other definition.

  if (this->output_section_ != NULL)
    gold_error(_("mismatched definition for constrained sections"));

  this->output_section_ = posd->output_section_;
  posd->output_section_ = NULL;

  if (this->is_relro_)
    this->output_section_->set_is_relro();
  else
    this->output_section_->clear_is_relro();

  return true;
}

// Get the list of segments to use for an allocated section when using
// a PHDRS clause.

Output_section*
Output_section_definition::allocate_to_segment(String_list** phdrs_list,
					       bool* orphan)
{
  // Update phdrs_list even if we don't have an output section. It
  // might be used by the following sections.
  if (this->phdrs_ != NULL)
    *phdrs_list = this->phdrs_;

  if (this->output_section_ == NULL)
    return NULL;
  if ((this->output_section_->flags() & elfcpp::SHF_ALLOC) == 0)
    return NULL;
  *orphan = false;
  return this->output_section_;
}

// Look for an output section by name and return the address, the load
// address, the alignment, and the size.  This is used when an
// expression refers to an output section which was not actually
// created.  This returns true if the section was found, false
// otherwise.

bool
Output_section_definition::get_output_section_info(const char* name,
                                                   uint64_t* address,
                                                   uint64_t* load_address,
                                                   uint64_t* addralign,
                                                   uint64_t* size) const
{
  if (this->name_ != name)
    return false;

  if (this->output_section_ != NULL)
    {
      *address = this->output_section_->address();
      if (this->output_section_->has_load_address())
        *load_address = this->output_section_->load_address();
      else
        *load_address = *address;
      *addralign = this->output_section_->addralign();
      *size = this->output_section_->current_data_size();
    }
  else
    {
      *address = this->evaluated_address_;
      *load_address = this->evaluated_load_address_;
      *addralign = this->evaluated_addralign_;
      *size = 0;
    }

  return true;
}

// Print for debugging.

void
Output_section_definition::print(FILE* f) const
{
  fprintf(f, "  %s ", this->name_.c_str());

  if (this->address_ != NULL)
    {
      this->address_->print(f);
      fprintf(f, " ");
    }

  if (this->script_section_type_ != SCRIPT_SECTION_TYPE_NONE)
      fprintf(f, "(%s) ",
	      this->script_section_type_name(this->script_section_type_));

  fprintf(f, ": ");

  if (this->load_address_ != NULL)
    {
      fprintf(f, "AT(");
      this->load_address_->print(f);
      fprintf(f, ") ");
    }

  if (this->align_ != NULL)
    {
      fprintf(f, "ALIGN(");
      this->align_->print(f);
      fprintf(f, ") ");
    }

  if (this->subalign_ != NULL)
    {
      fprintf(f, "SUBALIGN(");
      this->subalign_->print(f);
      fprintf(f, ") ");
    }

  fprintf(f, "{\n");

  for (Output_section_elements::const_iterator p = this->elements_.begin();
       p != this->elements_.end();
       ++p)
    (*p)->print(f);

  fprintf(f, "  }");

  if (this->fill_ != NULL)
    {
      fprintf(f, " = ");
      this->fill_->print(f);
    }

  if (this->phdrs_ != NULL)
    {
      for (String_list::const_iterator p = this->phdrs_->begin();
	   p != this->phdrs_->end();
	   ++p)
	fprintf(f, " :%s", p->c_str());
    }

  fprintf(f, "\n");
}

Script_sections::Section_type
Output_section_definition::section_type() const
{
  switch (this->script_section_type_)
    {
    case SCRIPT_SECTION_TYPE_NONE:
      return Script_sections::ST_NONE;
    case SCRIPT_SECTION_TYPE_NOLOAD:
      return Script_sections::ST_NOLOAD;
    case SCRIPT_SECTION_TYPE_COPY:
    case SCRIPT_SECTION_TYPE_DSECT:
    case SCRIPT_SECTION_TYPE_INFO:
    case SCRIPT_SECTION_TYPE_OVERLAY:
      // There are not really support so we treat them as ST_NONE.  The
      // parse should have issued errors for them already.
      return Script_sections::ST_NONE;
    default:
      gold_unreachable();
    }
}

// Return the name of a script section type.

const char*
Output_section_definition::script_section_type_name(
    Script_section_type script_section_type)
{
  switch (script_section_type)
    {
    case SCRIPT_SECTION_TYPE_NONE:
      return "NONE";
    case SCRIPT_SECTION_TYPE_NOLOAD:
      return "NOLOAD";
    case SCRIPT_SECTION_TYPE_DSECT:
      return "DSECT";
    case SCRIPT_SECTION_TYPE_COPY:
      return "COPY";
    case SCRIPT_SECTION_TYPE_INFO:
      return "INFO";
    case SCRIPT_SECTION_TYPE_OVERLAY:
      return "OVERLAY";
    default:
      gold_unreachable();
    }
}

void
Output_section_definition::set_memory_region(Memory_region* mr, bool set_vma)
{
  gold_assert(mr != NULL);
  // Add the current section to the specified region's list.
  mr->add_section(this, set_vma);
}

// An output section created to hold orphaned input sections.  These
// do not actually appear in linker scripts.  However, for convenience
// when setting the output section addresses, we put a marker to these
// sections in the appropriate place in the list of SECTIONS elements.

class Orphan_output_section : public Sections_element
{
 public:
  Orphan_output_section(Output_section* os)
    : os_(os)
  { }

  // Return whether the orphan output section is relro.  We can just
  // check the output section because we always set the flag, if
  // needed, just after we create the Orphan_output_section.
  bool
  is_relro() const
  { return this->os_->is_relro(); }

  // Initialize OSP with an output section.  This should have been
  // done already.
  void
  orphan_section_init(Orphan_section_placement*,
		      Script_sections::Elements_iterator)
  { gold_unreachable(); }

  // Set section addresses.
  void
  set_section_addresses(Symbol_table*, Layout*, uint64_t*, uint64_t*,
			uint64_t*);

  // Get the list of segments to use for an allocated section when
  // using a PHDRS clause.
  Output_section*
  allocate_to_segment(String_list**, bool*);

  // Return the associated Output_section.
  Output_section*
  get_output_section() const
  { return this->os_; }

  // Print for debugging.
  void
  print(FILE* f) const
  {
    fprintf(f, "  marker for orphaned output section %s\n",
	    this->os_->name());
  }

 private:
  Output_section* os_;
};

// Set section addresses.

void
Orphan_output_section::set_section_addresses(Symbol_table*, Layout*,
					     uint64_t* dot_value,
					     uint64_t*,
                                             uint64_t* load_address)
{
  typedef std::list<Output_section::Input_section> Input_section_list;

  bool have_load_address = *load_address != *dot_value;

  uint64_t address = *dot_value;
  address = align_address(address, this->os_->addralign());

  // If input section sorting is requested via --section-ordering-file or
  // linker plugins, then do it here.  This is important because we want
  // any sorting specified in the linker scripts, which will be done after
  // this, to take precedence.  The final order of input sections is then
  // guaranteed to be according to the linker script specification.
  if (this->os_ != NULL
      && this->os_->input_section_order_specified())
    this->os_->sort_attached_input_sections();

  // For a relocatable link, all orphan sections are put at
  // address 0.  In general we expect all sections to be at
  // address 0 for a relocatable link, but we permit the linker
  // script to override that for specific output sections.
  if (parameters->options().relocatable())
    {
      address = 0;
      *load_address = 0;
      have_load_address = false;
    }

  if ((this->os_->flags() & elfcpp::SHF_ALLOC) != 0)
    {
      this->os_->set_address(address);
      if (have_load_address)
        this->os_->set_load_address(align_address(*load_address,
                                                  this->os_->addralign()));
    }

  Input_section_list input_sections;
  address += this->os_->get_input_sections(address, "", &input_sections);

  for (Input_section_list::iterator p = input_sections.begin();
       p != input_sections.end();
       ++p)
    {
      uint64_t addralign = p->addralign();
      if (!p->is_input_section())
	p->output_section_data()->finalize_data_size();
      uint64_t size = p->data_size();
      address = align_address(address, addralign);
      this->os_->add_script_input_section(*p);
      address += size;
    }

  if (parameters->options().relocatable())
    {
      // For a relocatable link, reset DOT_VALUE to 0.
      *dot_value = 0;
      *load_address = 0;
    }
  else if (this->os_ == NULL
	   || (this->os_->flags() & elfcpp::SHF_TLS) == 0
	   || this->os_->type() != elfcpp::SHT_NOBITS)
    {
      // An SHF_TLS/SHT_NOBITS section does not take up any address space.
      if (!have_load_address)
	*load_address = address;
      else
	*load_address += address - *dot_value;

      *dot_value = address;
    }
}

// Get the list of segments to use for an allocated section when using
// a PHDRS clause.  If this is an allocated section, return the
// Output_section.  We don't change the list of segments.

Output_section*
Orphan_output_section::allocate_to_segment(String_list**, bool* orphan)
{
  if ((this->os_->flags() & elfcpp::SHF_ALLOC) == 0)
    return NULL;
  *orphan = true;
  return this->os_;
}

// Class Phdrs_element.  A program header from a PHDRS clause.

class Phdrs_element
{
 public:
  Phdrs_element(const char* name, size_t namelen, unsigned int type,
		bool includes_filehdr, bool includes_phdrs,
		bool is_flags_valid, unsigned int flags,
		Expression* load_address)
    : name_(name, namelen), type_(type), includes_filehdr_(includes_filehdr),
      includes_phdrs_(includes_phdrs), is_flags_valid_(is_flags_valid),
      flags_(flags), load_address_(load_address), load_address_value_(0),
      segment_(NULL)
  { }

  // Return the name of this segment.
  const std::string&
  name() const
  { return this->name_; }

  // Return the type of the segment.
  unsigned int
  type() const
  { return this->type_; }

  // Whether to include the file header.
  bool
  includes_filehdr() const
  { return this->includes_filehdr_; }

  // Whether to include the program headers.
  bool
  includes_phdrs() const
  { return this->includes_phdrs_; }

  // Return whether there is a load address.
  bool
  has_load_address() const
  { return this->load_address_ != NULL; }

  // Evaluate the load address expression if there is one.
  void
  eval_load_address(Symbol_table* symtab, Layout* layout)
  {
    if (this->load_address_ != NULL)
      this->load_address_value_ = this->load_address_->eval(symtab, layout,
							    true);
  }

  // Return the load address.
  uint64_t
  load_address() const
  {
    gold_assert(this->load_address_ != NULL);
    return this->load_address_value_;
  }

  // Create the segment.
  Output_segment*
  create_segment(Layout* layout)
  {
    this->segment_ = layout->make_output_segment(this->type_, this->flags_);
    return this->segment_;
  }

  // Return the segment.
  Output_segment*
  segment()
  { return this->segment_; }

  // Release the segment.
  void
  release_segment()
  { this->segment_ = NULL; }

  // Set the segment flags if appropriate.
  void
  set_flags_if_valid()
  {
    if (this->is_flags_valid_)
      this->segment_->set_flags(this->flags_);
  }

  // Print for debugging.
  void
  print(FILE*) const;

 private:
  // The name used in the script.
  std::string name_;
  // The type of the segment (PT_LOAD, etc.).
  unsigned int type_;
  // Whether this segment includes the file header.
  bool includes_filehdr_;
  // Whether this segment includes the section headers.
  bool includes_phdrs_;
  // Whether the flags were explicitly specified.
  bool is_flags_valid_;
  // The flags for this segment (PF_R, etc.) if specified.
  unsigned int flags_;
  // The expression for the load address for this segment.  This may
  // be NULL.
  Expression* load_address_;
  // The actual load address from evaluating the expression.
  uint64_t load_address_value_;
  // The segment itself.
  Output_segment* segment_;
};

// Print for debugging.

void
Phdrs_element::print(FILE* f) const
{
  fprintf(f, "  %s 0x%x", this->name_.c_str(), this->type_);
  if (this->includes_filehdr_)
    fprintf(f, " FILEHDR");
  if (this->includes_phdrs_)
    fprintf(f, " PHDRS");
  if (this->is_flags_valid_)
    fprintf(f, " FLAGS(%u)", this->flags_);
  if (this->load_address_ != NULL)
    {
      fprintf(f, " AT(");
      this->load_address_->print(f);
      fprintf(f, ")");
    }
  fprintf(f, ";\n");
}

// Add a memory region.

void
Script_sections::add_memory_region(const char* name, size_t namelen,
				   unsigned int attributes,
				   Expression* start, Expression* length)
{
  if (this->memory_regions_ == NULL)
    this->memory_regions_ = new Memory_regions();
  else if (this->find_memory_region(name, namelen))
    {
      gold_error(_("region '%.*s' already defined"), static_cast<int>(namelen),
                  name);
      // FIXME: Add a GOLD extension to allow multiple regions with the same
      // name.  This would amount to a single region covering disjoint blocks
      // of memory, which is useful for embedded devices.
    }

  // FIXME: Check the length and start values.  Currently we allow
  // non-constant expressions for these values, whereas LD does not.

  // FIXME: Add a GOLD extension to allow NEGATIVE LENGTHS.  This would
  // describe a region that packs from the end address going down, rather
  // than the start address going up.  This would be useful for embedded
  // devices.

  this->memory_regions_->push_back(new Memory_region(name, namelen, attributes,
						     start, length));
}

// Find a memory region.

Memory_region*
Script_sections::find_memory_region(const char* name, size_t namelen)
{
  if (this->memory_regions_ == NULL)
    return NULL;

  for (Memory_regions::const_iterator m = this->memory_regions_->begin();
       m != this->memory_regions_->end();
       ++m)
    if ((*m)->name_match(name, namelen))
      return *m;

  return NULL;
}

// Find a memory region's origin.

Expression*
Script_sections::find_memory_region_origin(const char* name, size_t namelen)
{
  Memory_region* mr = find_memory_region(name, namelen);
  if (mr == NULL)
    return NULL;

  return mr->start_address();
}

// Find a memory region's length.

Expression*
Script_sections::find_memory_region_length(const char* name, size_t namelen)
{
  Memory_region* mr = find_memory_region(name, namelen);
  if (mr == NULL)
    return NULL;

  return mr->length();
}

// Set the memory region to use for the current section.

void
Script_sections::set_memory_region(Memory_region* mr, bool set_vma)
{
  gold_assert(!this->sections_elements_->empty());
  this->sections_elements_->back()->set_memory_region(mr, set_vma);
}

// Class Script_sections.

Script_sections::Script_sections()
  : saw_sections_clause_(false),
    in_sections_clause_(false),
    sections_elements_(NULL),
    output_section_(NULL),
    memory_regions_(NULL),
    phdrs_elements_(NULL),
    orphan_section_placement_(NULL),
    data_segment_align_start_(),
    saw_data_segment_align_(false),
    saw_relro_end_(false),
    saw_segment_start_expression_(false),
    segments_created_(false)
{
}

// Start a SECTIONS clause.

void
Script_sections::start_sections()
{
  gold_assert(!this->in_sections_clause_ && this->output_section_ == NULL);
  this->saw_sections_clause_ = true;
  this->in_sections_clause_ = true;
  if (this->sections_elements_ == NULL)
    this->sections_elements_ = new Sections_elements;
}

// Finish a SECTIONS clause.

void
Script_sections::finish_sections()
{
  gold_assert(this->in_sections_clause_ && this->output_section_ == NULL);
  this->in_sections_clause_ = false;
}

// Add a symbol to be defined.

void
Script_sections::add_symbol_assignment(const char* name, size_t length,
				       Expression* val, bool provide,
				       bool hidden)
{
  if (this->output_section_ != NULL)
    this->output_section_->add_symbol_assignment(name, length, val,
						 provide, hidden);
  else
    {
      Sections_element* p = new Sections_element_assignment(name, length,
							    val, provide,
							    hidden);
      this->sections_elements_->push_back(p);
    }
}

// Add an assignment to the special dot symbol.

void
Script_sections::add_dot_assignment(Expression* val)
{
  if (this->output_section_ != NULL)
    this->output_section_->add_dot_assignment(val);
  else
    {
      // The GNU linker permits assignments to . to appears outside of
      // a SECTIONS clause, and treats it as appearing inside, so
      // sections_elements_ may be NULL here.
      if (this->sections_elements_ == NULL)
	{
	  this->sections_elements_ = new Sections_elements;
	  this->saw_sections_clause_ = true;
	}

      Sections_element* p = new Sections_element_dot_assignment(val);
      this->sections_elements_->push_back(p);
    }
}

// Add an assertion.

void
Script_sections::add_assertion(Expression* check, const char* message,
			       size_t messagelen)
{
  if (this->output_section_ != NULL)
    this->output_section_->add_assertion(check, message, messagelen);
  else
    {
      Sections_element* p = new Sections_element_assertion(check, message,
							   messagelen);
      this->sections_elements_->push_back(p);
    }
}

// Start processing entries for an output section.

void
Script_sections::start_output_section(
    const char* name,
    size_t namelen,
    const Parser_output_section_header* header)
{
  Output_section_definition* posd = new Output_section_definition(name,
								  namelen,
								  header);
  this->sections_elements_->push_back(posd);
  gold_assert(this->output_section_ == NULL);
  this->output_section_ = posd;
}

// Stop processing entries for an output section.

void
Script_sections::finish_output_section(
    const Parser_output_section_trailer* trailer)
{
  gold_assert(this->output_section_ != NULL);
  this->output_section_->finish(trailer);
  this->output_section_ = NULL;
}

// Add a data item to the current output section.

void
Script_sections::add_data(int size, bool is_signed, Expression* val)
{
  gold_assert(this->output_section_ != NULL);
  this->output_section_->add_data(size, is_signed, val);
}

// Add a fill value setting to the current output section.

void
Script_sections::add_fill(Expression* val)
{
  gold_assert(this->output_section_ != NULL);
  this->output_section_->add_fill(val);
}

// Add an input section specification to the current output section.

void
Script_sections::add_input_section(const Input_section_spec* spec, bool keep)
{
  gold_assert(this->output_section_ != NULL);
  this->output_section_->add_input_section(spec, keep);
}

// This is called when we see DATA_SEGMENT_ALIGN.  It means that any
// subsequent output sections may be relro.

void
Script_sections::data_segment_align()
{
  if (this->saw_data_segment_align_)
    gold_error(_("DATA_SEGMENT_ALIGN may only appear once in a linker script"));
  gold_assert(!this->sections_elements_->empty());
  Sections_elements::iterator p = this->sections_elements_->end();
  --p;
  this->data_segment_align_start_ = p;
  this->saw_data_segment_align_ = true;
}

// This is called when we see DATA_SEGMENT_RELRO_END.  It means that
// any output sections seen since DATA_SEGMENT_ALIGN are relro.

void
Script_sections::data_segment_relro_end()
{
  if (this->saw_relro_end_)
    gold_error(_("DATA_SEGMENT_RELRO_END may only appear once "
		 "in a linker script"));
  this->saw_relro_end_ = true;

  if (!this->saw_data_segment_align_)
    gold_error(_("DATA_SEGMENT_RELRO_END must follow DATA_SEGMENT_ALIGN"));
  else
    {
      Sections_elements::iterator p = this->data_segment_align_start_;
      for (++p; p != this->sections_elements_->end(); ++p)
	(*p)->set_is_relro();
    }
}

// Create any required sections.

void
Script_sections::create_sections(Layout* layout)
{
  if (!this->saw_sections_clause_)
    return;
  for (Sections_elements::iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    (*p)->create_sections(layout);
}

// Add any symbols we are defining to the symbol table.

void
Script_sections::add_symbols_to_table(Symbol_table* symtab)
{
  if (!this->saw_sections_clause_)
    return;
  for (Sections_elements::iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    (*p)->add_symbols_to_table(symtab);
}

// Finalize symbols and check assertions.

void
Script_sections::finalize_symbols(Symbol_table* symtab, const Layout* layout)
{
  if (!this->saw_sections_clause_)
    return;
  uint64_t dot_value = 0;
  for (Sections_elements::iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    (*p)->finalize_symbols(symtab, layout, &dot_value);
}

// Return the name of the output section to use for an input file name
// and section name.

const char*
Script_sections::output_section_name(
    const char* file_name,
    const char* section_name,
    Output_section*** output_section_slot,
    Script_sections::Section_type* psection_type,
    bool* keep,
    bool is_input_section)
{
  for (Sections_elements::const_iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    {
      const char* ret = (*p)->output_section_name(file_name, section_name,
						  output_section_slot,
						  psection_type, keep,
						  is_input_section);

      if (ret != NULL)
	{
	  // The special name /DISCARD/ means that the input section
	  // should be discarded.
	  if (strcmp(ret, "/DISCARD/") == 0)
	    {
	      *output_section_slot = NULL;
	      *psection_type = Script_sections::ST_NONE;
	      return NULL;
	    }
	  return ret;
	}
    }

  // We have an orphan section.
  *output_section_slot = NULL;
  *psection_type = Script_sections::ST_NONE;
  *keep = false;

  General_options::Orphan_handling orphan_handling =
      parameters->options().orphan_handling_enum();
  if (orphan_handling == General_options::ORPHAN_DISCARD)
    return NULL;
  if (orphan_handling == General_options::ORPHAN_ERROR)
    {
      if (file_name == NULL)
	gold_error(_("unplaced orphan section '%s'"), section_name);
      else
	gold_error(_("unplaced orphan section '%s' from '%s'"),
		   section_name, file_name);
      return NULL;
    }
  if (orphan_handling == General_options::ORPHAN_WARN)
    {
      if (file_name == NULL)
	gold_warning(_("orphan section '%s' is being placed in section '%s'"),
		     section_name, section_name);
      else
	gold_warning(_("orphan section '%s' from '%s' is being placed "
		       "in section '%s'"),
		     section_name, file_name, section_name);
    }

  // If we couldn't find a mapping for the name, the output section
  // gets the name of the input section.
  return section_name;
}

// Place a marker for an orphan output section into the SECTIONS
// clause.

void
Script_sections::place_orphan(Output_section* os)
{
  Orphan_section_placement* osp = this->orphan_section_placement_;
  if (osp == NULL)
    {
      // Initialize the Orphan_section_placement structure.
      osp = new Orphan_section_placement();
      for (Sections_elements::iterator p = this->sections_elements_->begin();
	   p != this->sections_elements_->end();
	   ++p)
	(*p)->orphan_section_init(osp, p);
      gold_assert(!this->sections_elements_->empty());
      Sections_elements::iterator last = this->sections_elements_->end();
      --last;
      osp->last_init(last);
      this->orphan_section_placement_ = osp;
    }

  Orphan_output_section* orphan = new Orphan_output_section(os);

  // Look for where to put ORPHAN.
  Sections_elements::iterator* where;
  if (osp->find_place(os, &where))
    {
      if ((**where)->is_relro())
	os->set_is_relro();
      else
	os->clear_is_relro();

      // We want to insert ORPHAN after *WHERE, and then update *WHERE
      // so that the next one goes after this one.
      Sections_elements::iterator p = *where;
      gold_assert(p != this->sections_elements_->end());
      ++p;
      *where = this->sections_elements_->insert(p, orphan);
    }
  else
    {
      os->clear_is_relro();
      // We don't have a place to put this orphan section.  Put it,
      // and all other sections like it, at the end, but before the
      // sections which always come at the end.
      Sections_elements::iterator last = osp->last_place();
      *where = this->sections_elements_->insert(last, orphan);
    }

  if ((os->flags() & elfcpp::SHF_ALLOC) != 0)
    osp->update_last_alloc(*where);
}

// Set the addresses of all the output sections.  Walk through all the
// elements, tracking the dot symbol.  Apply assignments which set
// absolute symbol values, in case they are used when setting dot.
// Fill in data statement values.  As we find output sections, set the
// address, set the address of all associated input sections, and
// update dot.  Return the segment which should hold the file header
// and segment headers, if any.

Output_segment*
Script_sections::set_section_addresses(Symbol_table* symtab, Layout* layout)
{
  gold_assert(this->saw_sections_clause_);

  // Implement ONLY_IF_RO/ONLY_IF_RW constraints.  These are a pain
  // for our representation.
  for (Sections_elements::iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    {
      Output_section_definition* posd;
      Section_constraint failed_constraint = (*p)->check_constraint(&posd);
      if (failed_constraint != CONSTRAINT_NONE)
	{
	  Sections_elements::iterator q;
	  for (q = this->sections_elements_->begin();
	       q != this->sections_elements_->end();
	       ++q)
	    {
	      if (q != p)
		{
		  if ((*q)->alternate_constraint(posd, failed_constraint))
		    break;
		}
	    }

	  if (q == this->sections_elements_->end())
	    gold_error(_("no matching section constraint"));
	}
    }

  // Force the alignment of the first TLS section to be the maximum
  // alignment of all TLS sections.
  Output_section* first_tls = NULL;
  uint64_t tls_align = 0;
  for (Sections_elements::const_iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    {
      Output_section* os = (*p)->get_output_section();
      if (os != NULL && (os->flags() & elfcpp::SHF_TLS) != 0)
	{
	  if (first_tls == NULL)
	    first_tls = os;
	  if (os->addralign() > tls_align)
	    tls_align = os->addralign();
	}
    }
  if (first_tls != NULL)
    first_tls->set_addralign(tls_align);

  // For a relocatable link, we implicitly set dot to zero.
  uint64_t dot_value = 0;
  uint64_t dot_alignment = 0;
  uint64_t load_address = 0;

  // Check to see if we want to use any of -Ttext, -Tdata and -Tbss options
  // to set section addresses.  If the script has any SEGMENT_START
  // expression, we do not set the section addresses.
  bool use_tsection_options =
    (!this->saw_segment_start_expression_
     && (parameters->options().user_set_Ttext()
	 || parameters->options().user_set_Tdata()
	 || parameters->options().user_set_Tbss()));

  for (Sections_elements::iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    {
      Output_section* os = (*p)->get_output_section();

      // Handle -Ttext, -Tdata and -Tbss options.  We do this by looking for
      // the special sections by names and doing dot assignments.
      if (use_tsection_options
	  && os != NULL
	  && (os->flags() & elfcpp::SHF_ALLOC) != 0)
	{
	  uint64_t new_dot_value = dot_value;

	  if (parameters->options().user_set_Ttext()
	      && strcmp(os->name(), ".text") == 0)
	    new_dot_value = parameters->options().Ttext();
	  else if (parameters->options().user_set_Tdata()
	      && strcmp(os->name(), ".data") == 0)
	    new_dot_value = parameters->options().Tdata();
	  else if (parameters->options().user_set_Tbss()
	      && strcmp(os->name(), ".bss") == 0)
	    new_dot_value = parameters->options().Tbss();

	  // Update dot and load address if necessary.
	  if (new_dot_value < dot_value)
	    gold_error(_("dot may not move backward"));
	  else if (new_dot_value != dot_value)
	    {
	      dot_value = new_dot_value;
	      load_address = new_dot_value;
	    }
	}

      (*p)->set_section_addresses(symtab, layout, &dot_value, &dot_alignment,
				  &load_address);
    }

  if (this->phdrs_elements_ != NULL)
    {
      for (Phdrs_elements::iterator p = this->phdrs_elements_->begin();
	   p != this->phdrs_elements_->end();
	   ++p)
	(*p)->eval_load_address(symtab, layout);
    }

  return this->create_segments(layout, dot_alignment);
}

// Sort the sections in order to put them into segments.

class Sort_output_sections
{
 public:
  Sort_output_sections(const Script_sections::Sections_elements* elements)
   : elements_(elements)
  { }

  bool
  operator()(const Output_section* os1, const Output_section* os2) const;

 private:
  int
  script_compare(const Output_section* os1, const Output_section* os2) const;

 private:
  const Script_sections::Sections_elements* elements_;
};

bool
Sort_output_sections::operator()(const Output_section* os1,
				 const Output_section* os2) const
{
  // Sort first by the load address.
  uint64_t lma1 = (os1->has_load_address()
		   ? os1->load_address()
		   : os1->address());
  uint64_t lma2 = (os2->has_load_address()
		   ? os2->load_address()
		   : os2->address());
  if (lma1 != lma2)
    return lma1 < lma2;

  // Then sort by the virtual address.
  if (os1->address() != os2->address())
    return os1->address() < os2->address();

  // If the linker script says which of these sections is first, go
  // with what it says.
  int i = this->script_compare(os1, os2);
  if (i != 0)
    return i < 0;

  // Sort PROGBITS before NOBITS.
  bool nobits1 = os1->type() == elfcpp::SHT_NOBITS;
  bool nobits2 = os2->type() == elfcpp::SHT_NOBITS;
  if (nobits1 != nobits2)
    return nobits2;

  // Sort PROGBITS TLS sections to the end, NOBITS TLS sections to the
  // beginning.
  bool tls1 = (os1->flags() & elfcpp::SHF_TLS) != 0;
  bool tls2 = (os2->flags() & elfcpp::SHF_TLS) != 0;
  if (tls1 != tls2)
    return nobits1 ? tls1 : tls2;

  // Sort non-NOLOAD before NOLOAD.
  if (os1->is_noload() && !os2->is_noload())
    return true;
  if (!os1->is_noload() && os2->is_noload())
    return true;

  // The sections seem practically identical.  Sort by name to get a
  // stable sort.
  return os1->name() < os2->name();
}

// Return -1 if OS1 comes before OS2 in ELEMENTS_, 1 if comes after, 0
// if either OS1 or OS2 is not mentioned.  This ensures that we keep
// empty sections in the order in which they appear in a linker
// script.

int
Sort_output_sections::script_compare(const Output_section* os1,
				     const Output_section* os2) const
{
  if (this->elements_ == NULL)
    return 0;

  bool found_os1 = false;
  bool found_os2 = false;
  for (Script_sections::Sections_elements::const_iterator
	 p = this->elements_->begin();
       p != this->elements_->end();
       ++p)
    {
      if (os2 == (*p)->get_output_section())
	{
	  if (found_os1)
	    return -1;
	  found_os2 = true;
	}
      else if (os1 == (*p)->get_output_section())
	{
	  if (found_os2)
	    return 1;
	  found_os1 = true;
	}
    }

  return 0;
}

// Return whether OS is a BSS section.  This is a SHT_NOBITS section.
// We treat a section with the SHF_TLS flag set as taking up space
// even if it is SHT_NOBITS (this is true of .tbss), as we allocate
// space for them in the file.

bool
Script_sections::is_bss_section(const Output_section* os)
{
  return (os->type() == elfcpp::SHT_NOBITS
	  && (os->flags() & elfcpp::SHF_TLS) == 0);
}

// Return the size taken by the file header and the program headers.

size_t
Script_sections::total_header_size(Layout* layout) const
{
  size_t segment_count = layout->segment_count();
  size_t file_header_size;
  size_t segment_headers_size;
  if (parameters->target().get_size() == 32)
    {
      file_header_size = elfcpp::Elf_sizes<32>::ehdr_size;
      segment_headers_size = segment_count * elfcpp::Elf_sizes<32>::phdr_size;
    }
  else if (parameters->target().get_size() == 64)
    {
      file_header_size = elfcpp::Elf_sizes<64>::ehdr_size;
      segment_headers_size = segment_count * elfcpp::Elf_sizes<64>::phdr_size;
    }
  else
    gold_unreachable();

  return file_header_size + segment_headers_size;
}

// Return the amount we have to subtract from the LMA to accommodate
// headers of the given size.  The complication is that the file
// header have to be at the start of a page, as otherwise it will not
// be at the start of the file.

uint64_t
Script_sections::header_size_adjustment(uint64_t lma,
					size_t sizeof_headers) const
{
  const uint64_t abi_pagesize = parameters->target().abi_pagesize();
  uint64_t hdr_lma = lma - sizeof_headers;
  hdr_lma &= ~(abi_pagesize - 1);
  return lma - hdr_lma;
}

// Create the PT_LOAD segments when using a SECTIONS clause.  Returns
// the segment which should hold the file header and segment headers,
// if any.

Output_segment*
Script_sections::create_segments(Layout* layout, uint64_t dot_alignment)
{
  gold_assert(this->saw_sections_clause_);

  if (parameters->options().relocatable())
    return NULL;

  if (this->saw_phdrs_clause())
    return create_segments_from_phdrs_clause(layout, dot_alignment);

  Layout::Section_list sections;
  layout->get_allocated_sections(&sections);

  // Sort the sections by address.
  std::stable_sort(sections.begin(), sections.end(),
		   Sort_output_sections(this->sections_elements_));

  this->create_note_and_tls_segments(layout, &sections);

  // Walk through the sections adding them to PT_LOAD segments.
  const uint64_t abi_pagesize = parameters->target().abi_pagesize();
  Output_segment* first_seg = NULL;
  Output_segment* current_seg = NULL;
  bool is_current_seg_readonly = true;
  uint64_t last_vma = 0;
  uint64_t last_lma = 0;
  uint64_t last_size = 0;
  bool in_bss = false;
  for (Layout::Section_list::iterator p = sections.begin();
       p != sections.end();
       ++p)
    {
      const uint64_t vma = (*p)->address();
      const uint64_t lma = ((*p)->has_load_address()
			    ? (*p)->load_address()
			    : vma);
      const uint64_t size = (*p)->current_data_size();

      bool need_new_segment;
      if (current_seg == NULL)
	need_new_segment = true;
      else if (lma - vma != last_lma - last_vma)
	{
	  // This section has a different LMA relationship than the
	  // last one; we need a new segment.
	  need_new_segment = true;
	}
      else if (align_address(last_lma + last_size, abi_pagesize)
	       < align_address(lma, abi_pagesize))
	{
	  // Putting this section in the segment would require
	  // skipping a page.
	  need_new_segment = true;
	}
      else if (in_bss && !is_bss_section(*p))
	{
	  // A non-BSS section can not follow a BSS section in the
	  // same segment.
	  need_new_segment = true;
	}
      else if (is_current_seg_readonly
	       && ((*p)->flags() & elfcpp::SHF_WRITE) != 0
	       && !parameters->options().omagic())
	{
	  // Don't put a writable section in the same segment as a
	  // non-writable section.
	  need_new_segment = true;
	}
      else
	{
	  // Otherwise, reuse the existing segment.
	  need_new_segment = false;
	}

      elfcpp::Elf_Word seg_flags =
	Layout::section_flags_to_segment((*p)->flags());

      if (need_new_segment)
	{
	  current_seg = layout->make_output_segment(elfcpp::PT_LOAD,
						    seg_flags);
	  current_seg->set_addresses(vma, lma);
	  current_seg->set_minimum_p_align(dot_alignment);
	  if (first_seg == NULL)
	    first_seg = current_seg;
	  is_current_seg_readonly = true;
	  in_bss = false;
	}

      current_seg->add_output_section_to_load(layout, *p, seg_flags);

      if (((*p)->flags() & elfcpp::SHF_WRITE) != 0)
	is_current_seg_readonly = false;

      if (is_bss_section(*p) && size > 0)
        in_bss = true;

      last_vma = vma;
      last_lma = lma;
      last_size = size;
    }

  // An ELF program should work even if the program headers are not in
  // a PT_LOAD segment.  However, it appears that the Linux kernel
  // does not set the AT_PHDR auxiliary entry in that case.  It sets
  // the load address to p_vaddr - p_offset of the first PT_LOAD
  // segment.  It then sets AT_PHDR to the load address plus the
  // offset to the program headers, e_phoff in the file header.  This
  // fails when the program headers appear in the file before the
  // first PT_LOAD segment.  Therefore, we always create a PT_LOAD
  // segment to hold the file header and the program headers.  This is
  // effectively what the GNU linker does, and it is slightly more
  // efficient in any case.  We try to use the first PT_LOAD segment
  // if we can, otherwise we make a new one.

  if (first_seg == NULL)
    return NULL;

  // -n or -N mean that the program is not demand paged and there is
  // no need to put the program headers in a PT_LOAD segment.
  if (parameters->options().nmagic() || parameters->options().omagic())
    return NULL;

  size_t sizeof_headers = this->total_header_size(layout);

  uint64_t vma = first_seg->vaddr();
  uint64_t lma = first_seg->paddr();

  uint64_t subtract = this->header_size_adjustment(lma, sizeof_headers);

  if ((lma & (abi_pagesize - 1)) >= sizeof_headers)
    {
      first_seg->set_addresses(vma - subtract, lma - subtract);
      return first_seg;
    }

  // If there is no room to squeeze in the headers, then punt.  The
  // resulting executable probably won't run on GNU/Linux, but we
  // trust that the user knows what they are doing.
  if (lma < subtract || vma < subtract)
    return NULL;

  // If memory regions have been specified and the address range
  // we are about to use is not contained within any region then
  // issue a warning message about the segment we are going to
  // create.  It will be outside of any region and so possibly
  // using non-existent or protected memory.  We test LMA rather
  // than VMA since we assume that the headers will never be
  // relocated.
  if (this->memory_regions_ != NULL
      && !this->block_in_region (NULL, layout, lma - subtract, subtract))
    gold_warning(_("creating a segment to contain the file and program"
		   " headers outside of any MEMORY region"));

  Output_segment* load_seg = layout->make_output_segment(elfcpp::PT_LOAD,
							 elfcpp::PF_R);
  load_seg->set_addresses(vma - subtract, lma - subtract);

  return load_seg;
}

// Create a PT_NOTE segment for each SHT_NOTE section and a PT_TLS
// segment if there are any SHT_TLS sections.

void
Script_sections::create_note_and_tls_segments(
    Layout* layout,
    const Layout::Section_list* sections)
{
  gold_assert(!this->saw_phdrs_clause());

  bool saw_tls = false;
  for (Layout::Section_list::const_iterator p = sections->begin();
       p != sections->end();
       ++p)
    {
      if ((*p)->type() == elfcpp::SHT_NOTE)
	{
	  elfcpp::Elf_Word seg_flags =
	    Layout::section_flags_to_segment((*p)->flags());
	  Output_segment* oseg = layout->make_output_segment(elfcpp::PT_NOTE,
							     seg_flags);
	  oseg->add_output_section_to_nonload(*p, seg_flags);

	  // Incorporate any subsequent SHT_NOTE sections, in the
	  // hopes that the script is sensible.
	  Layout::Section_list::const_iterator pnext = p + 1;
	  while (pnext != sections->end()
		 && (*pnext)->type() == elfcpp::SHT_NOTE)
	    {
	      seg_flags = Layout::section_flags_to_segment((*pnext)->flags());
	      oseg->add_output_section_to_nonload(*pnext, seg_flags);
	      p = pnext;
	      ++pnext;
	    }
	}

      if (((*p)->flags() & elfcpp::SHF_TLS) != 0)
	{
	  if (saw_tls)
	    gold_error(_("TLS sections are not adjacent"));

	  elfcpp::Elf_Word seg_flags =
	    Layout::section_flags_to_segment((*p)->flags());
	  Output_segment* oseg = layout->make_output_segment(elfcpp::PT_TLS,
							     seg_flags);
	  oseg->add_output_section_to_nonload(*p, seg_flags);

	  Layout::Section_list::const_iterator pnext = p + 1;
	  while (pnext != sections->end()
		 && ((*pnext)->flags() & elfcpp::SHF_TLS) != 0)
	    {
	      seg_flags = Layout::section_flags_to_segment((*pnext)->flags());
	      oseg->add_output_section_to_nonload(*pnext, seg_flags);
	      p = pnext;
	      ++pnext;
	    }

	  saw_tls = true;
	}

      // If we see a section named .interp then put the .interp section
      // in a PT_INTERP segment.
      // This is for GNU ld compatibility.
      if (strcmp((*p)->name(), ".interp") == 0)
	{
	  elfcpp::Elf_Word seg_flags =
	    Layout::section_flags_to_segment((*p)->flags());
	  Output_segment* oseg = layout->make_output_segment(elfcpp::PT_INTERP,
							     seg_flags);
	  oseg->add_output_section_to_nonload(*p, seg_flags);
	}
    }

    this->segments_created_ = true;
}

// Add a program header.  The PHDRS clause is syntactically distinct
// from the SECTIONS clause, but we implement it with the SECTIONS
// support because PHDRS is useless if there is no SECTIONS clause.

void
Script_sections::add_phdr(const char* name, size_t namelen, unsigned int type,
			  bool includes_filehdr, bool includes_phdrs,
			  bool is_flags_valid, unsigned int flags,
			  Expression* load_address)
{
  if (this->phdrs_elements_ == NULL)
    this->phdrs_elements_ = new Phdrs_elements();
  this->phdrs_elements_->push_back(new Phdrs_element(name, namelen, type,
						     includes_filehdr,
						     includes_phdrs,
						     is_flags_valid, flags,
						     load_address));
}

// Return the number of segments we expect to create based on the
// SECTIONS clause.  This is used to implement SIZEOF_HEADERS.

size_t
Script_sections::expected_segment_count(const Layout* layout) const
{
  // If we've already created the segments, we won't be adding any more.
  if (this->segments_created_)
    return 0;

  if (this->saw_phdrs_clause())
    return this->phdrs_elements_->size();

  Layout::Section_list sections;
  layout->get_allocated_sections(&sections);

  // We assume that we will need two PT_LOAD segments.
  size_t ret = 2;

  bool saw_note = false;
  bool saw_tls = false;
  bool saw_interp = false;
  for (Layout::Section_list::const_iterator p = sections.begin();
       p != sections.end();
       ++p)
    {
      if ((*p)->type() == elfcpp::SHT_NOTE)
	{
	  // Assume that all note sections will fit into a single
	  // PT_NOTE segment.
	  if (!saw_note)
	    {
	      ++ret;
	      saw_note = true;
	    }
	}
      else if (((*p)->flags() & elfcpp::SHF_TLS) != 0)
	{
	  // There can only be one PT_TLS segment.
	  if (!saw_tls)
	    {
	      ++ret;
	      saw_tls = true;
	    }
	}
      else if (strcmp((*p)->name(), ".interp") == 0)
	{
	  // There can only be one PT_INTERP segment.
	  if (!saw_interp)
	    {
	      ++ret;
	      saw_interp = true;
	    }
	}
    }

  return ret;
}

// Create the segments from a PHDRS clause.  Return the segment which
// should hold the file header and program headers, if any.

Output_segment*
Script_sections::create_segments_from_phdrs_clause(Layout* layout,
						   uint64_t dot_alignment)
{
  this->attach_sections_using_phdrs_clause(layout);
  return this->set_phdrs_clause_addresses(layout, dot_alignment);
}

// Create the segments from the PHDRS clause, and put the output
// sections in them.

void
Script_sections::attach_sections_using_phdrs_clause(Layout* layout)
{
  typedef std::map<std::string, Output_segment*> Name_to_segment;
  Name_to_segment name_to_segment;
  for (Phdrs_elements::const_iterator p = this->phdrs_elements_->begin();
       p != this->phdrs_elements_->end();
       ++p)
    name_to_segment[(*p)->name()] = (*p)->create_segment(layout);
  this->segments_created_ = true;

  // Walk through the output sections and attach them to segments.
  // Output sections in the script which do not list segments are
  // attached to the same set of segments as the immediately preceding
  // output section.

  String_list* phdr_names = NULL;
  bool load_segments_only = false;
  for (Sections_elements::const_iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    {
      bool is_orphan;
      String_list* old_phdr_names = phdr_names;
      Output_section* os = (*p)->allocate_to_segment(&phdr_names, &is_orphan);
      if (os == NULL)
	continue;

      elfcpp::Elf_Word seg_flags =
	Layout::section_flags_to_segment(os->flags());

      if (phdr_names == NULL)
	{
	  // Don't worry about empty orphan sections.
	  if (is_orphan && os->current_data_size() > 0)
	    gold_error(_("allocated section %s not in any segment"),
		       os->name());

	  // To avoid later crashes drop this section into the first
	  // PT_LOAD segment.
	  for (Phdrs_elements::const_iterator ppe =
		 this->phdrs_elements_->begin();
	       ppe != this->phdrs_elements_->end();
	       ++ppe)
	    {
	      Output_segment* oseg = (*ppe)->segment();
	      if (oseg->type() == elfcpp::PT_LOAD)
		{
		  oseg->add_output_section_to_load(layout, os, seg_flags);
		  break;
		}
	    }

	  continue;
	}

      // We see a list of segments names.  Disable PT_LOAD segment only
      // filtering.
      if (old_phdr_names != phdr_names)
	load_segments_only = false;

      // If this is an orphan section--one that was not explicitly
      // mentioned in the linker script--then it should not inherit
      // any segment type other than PT_LOAD.  Otherwise, e.g., the
      // PT_INTERP segment will pick up following orphan sections,
      // which does not make sense.  If this is not an orphan section,
      // we trust the linker script.
      if (is_orphan)
	{
	  // Enable PT_LOAD segments only filtering until we see another
	  // list of segment names.
	  load_segments_only = true;
	}

      bool in_load_segment = false;
      for (String_list::const_iterator q = phdr_names->begin();
	   q != phdr_names->end();
	   ++q)
	{
	  Name_to_segment::const_iterator r = name_to_segment.find(*q);
	  if (r == name_to_segment.end())
	    gold_error(_("no segment %s"), q->c_str());
	  else
	    {
	      if (load_segments_only
		  && r->second->type() != elfcpp::PT_LOAD)
		continue;

	      if (r->second->type() != elfcpp::PT_LOAD)
		r->second->add_output_section_to_nonload(os, seg_flags);
	      else
		{
		  r->second->add_output_section_to_load(layout, os, seg_flags);
		  if (in_load_segment)
		    gold_error(_("section in two PT_LOAD segments"));
		  in_load_segment = true;
		}
	    }
	}

      if (!in_load_segment)
	gold_error(_("allocated section not in any PT_LOAD segment"));
    }
}

// Set the addresses for segments created from a PHDRS clause.  Return
// the segment which should hold the file header and program headers,
// if any.

Output_segment*
Script_sections::set_phdrs_clause_addresses(Layout* layout,
					    uint64_t dot_alignment)
{
  Output_segment* load_seg = NULL;
  for (Phdrs_elements::const_iterator p = this->phdrs_elements_->begin();
       p != this->phdrs_elements_->end();
       ++p)
    {
      // Note that we have to set the flags after adding the output
      // sections to the segment, as adding an output segment can
      // change the flags.
      (*p)->set_flags_if_valid();

      Output_segment* oseg = (*p)->segment();

      if (oseg->type() != elfcpp::PT_LOAD)
	{
	  // The addresses of non-PT_LOAD segments are set from the
	  // PT_LOAD segments.
	  if ((*p)->has_load_address())
	    gold_error(_("may only specify load address for PT_LOAD segment"));
	  continue;
	}

      oseg->set_minimum_p_align(dot_alignment);

      // The output sections should have addresses from the SECTIONS
      // clause.  The addresses don't have to be in order, so find the
      // one with the lowest load address.  Use that to set the
      // address of the segment.

      Output_section* osec = oseg->section_with_lowest_load_address();
      if (osec == NULL)
	{
	  oseg->set_addresses(0, 0);
	  continue;
	}

      uint64_t vma = osec->address();
      uint64_t lma = osec->has_load_address() ? osec->load_address() : vma;

      // Override the load address of the section with the load
      // address specified for the segment.
      if ((*p)->has_load_address())
	{
	  if (osec->has_load_address())
	    gold_warning(_("PHDRS load address overrides "
			   "section %s load address"),
			 osec->name());

	  lma = (*p)->load_address();
	}

      bool headers = (*p)->includes_filehdr() && (*p)->includes_phdrs();
      if (!headers && ((*p)->includes_filehdr() || (*p)->includes_phdrs()))
	{
	  // We could support this if we wanted to.
	  gold_error(_("using only one of FILEHDR and PHDRS is "
		       "not currently supported"));
	}
      if (headers)
	{
	  size_t sizeof_headers = this->total_header_size(layout);
	  uint64_t subtract = this->header_size_adjustment(lma,
							   sizeof_headers);
	  if (lma >= subtract && vma >= subtract)
	    {
	      lma -= subtract;
	      vma -= subtract;
	    }
	  else
	    {
	      gold_error(_("sections loaded on first page without room "
			   "for file and program headers "
			   "are not supported"));
	    }

	  if (load_seg != NULL)
	    gold_error(_("using FILEHDR and PHDRS on more than one "
			 "PT_LOAD segment is not currently supported"));
	  load_seg = oseg;
	}

      oseg->set_addresses(vma, lma);
    }

  return load_seg;
}

// Add the file header and segment headers to non-load segments
// specified in the PHDRS clause.

void
Script_sections::put_headers_in_phdrs(Output_data* file_header,
				      Output_data* segment_headers)
{
  gold_assert(this->saw_phdrs_clause());
  for (Phdrs_elements::iterator p = this->phdrs_elements_->begin();
       p != this->phdrs_elements_->end();
       ++p)
    {
      if ((*p)->type() != elfcpp::PT_LOAD)
	{
	  if ((*p)->includes_phdrs())
	    (*p)->segment()->add_initial_output_data(segment_headers);
	  if ((*p)->includes_filehdr())
	    (*p)->segment()->add_initial_output_data(file_header);
	}
    }
}

// Look for an output section by name and return the address, the load
// address, the alignment, and the size.  This is used when an
// expression refers to an output section which was not actually
// created.  This returns true if the section was found, false
// otherwise.

bool
Script_sections::get_output_section_info(const char* name, uint64_t* address,
                                         uint64_t* load_address,
                                         uint64_t* addralign,
                                         uint64_t* size) const
{
  if (!this->saw_sections_clause_)
    return false;
  for (Sections_elements::const_iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    if ((*p)->get_output_section_info(name, address, load_address, addralign,
                                      size))
      return true;
  return false;
}

// Release all Output_segments.  This remove all pointers to all
// Output_segments.

void
Script_sections::release_segments()
{
  if (this->saw_phdrs_clause())
    {
      for (Phdrs_elements::const_iterator p = this->phdrs_elements_->begin();
	   p != this->phdrs_elements_->end();
	   ++p)
	(*p)->release_segment();
    }
  this->segments_created_ = false;
}

// Print the SECTIONS clause to F for debugging.

void
Script_sections::print(FILE* f) const
{
  if (this->phdrs_elements_ != NULL)
    {
      fprintf(f, "PHDRS {\n");
      for (Phdrs_elements::const_iterator p = this->phdrs_elements_->begin();
	   p != this->phdrs_elements_->end();
	   ++p)
	(*p)->print(f);
      fprintf(f, "}\n");
    }

  if (this->memory_regions_ != NULL)
    {
      fprintf(f, "MEMORY {\n");
      for (Memory_regions::const_iterator m = this->memory_regions_->begin();
	   m != this->memory_regions_->end();
	   ++m)
	(*m)->print(f);
      fprintf(f, "}\n");
    }

  if (!this->saw_sections_clause_)
    return;

  fprintf(f, "SECTIONS {\n");

  for (Sections_elements::const_iterator p = this->sections_elements_->begin();
       p != this->sections_elements_->end();
       ++p)
    (*p)->print(f);

  fprintf(f, "}\n");
}

} // End namespace gold.
