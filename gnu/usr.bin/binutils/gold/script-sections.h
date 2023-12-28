// script-sections.h -- linker script SECTIONS for gold   -*- C++ -*-

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

// This is for the support of the SECTIONS clause in linker scripts.

#ifndef GOLD_SCRIPT_SECTIONS_H
#define GOLD_SCRIPT_SECTIONS_H

#include <cstdio>
#include <list>
#include <vector>

namespace gold
{

struct Parser_output_section_header;
struct Parser_output_section_trailer;
struct Input_section_spec;
class Expression;
class Sections_element;
class Memory_region;
class Phdrs_element;
class Output_data;
class Output_section_definition;
class Output_section;
class Output_segment;
class Orphan_section_placement;

class Script_sections
{
 public:
  // This is a list, not a vector, because we insert orphan sections
  // in the middle.
  typedef std::list<Sections_element*> Sections_elements;

  // Logical script section types.  We map section types returned by the
  // parser into these since some section types have the same semantics.
  enum Section_type
  {
    // No section type specified.
    ST_NONE,
    // Section is NOLOAD.  We allocate space in the output but section
    // is not loaded in runtime.
    ST_NOLOAD,
    // No space is allocated to section.
    ST_NOALLOC
  };

  Script_sections();

  // Start a SECTIONS clause.
  void
  start_sections();

  // Finish a SECTIONS clause.
  void
  finish_sections();

  // Return whether we ever saw a SECTIONS clause.  If we did, then
  // all section layout needs to go through this class.
  bool
  saw_sections_clause() const
  { return this->saw_sections_clause_; }

  // Return whether we are currently processing a SECTIONS clause.
  bool
  in_sections_clause() const
  { return this->in_sections_clause_; }

  // Return whether we ever saw a PHDRS clause.  We ignore the PHDRS
  // clause unless we also saw a SECTIONS clause.
  bool
  saw_phdrs_clause() const
  { return this->saw_sections_clause_ && this->phdrs_elements_ != NULL; }

  // Start processing entries for an output section.
  void
  start_output_section(const char* name, size_t namelen,
		       const Parser_output_section_header*);

  // Finish processing entries for an output section.
  void
  finish_output_section(const Parser_output_section_trailer*);

  // Add a data item to the current output section.
  void
  add_data(int size, bool is_signed, Expression* val);

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

  // Add a setting for the fill value.
  void
  add_fill(Expression* val);

  // Add an input section specification.
  void
  add_input_section(const Input_section_spec* spec, bool keep);

  // Saw DATA_SEGMENT_ALIGN.
  void
  data_segment_align();

  // Saw DATA_SEGMENT_RELRO_END.
  void
  data_segment_relro_end();

  // Create any required sections.
  void
  create_sections(Layout*);

  // Add any symbols we are defining to the symbol table.
  void
  add_symbols_to_table(Symbol_table*);

  // Finalize symbol values and check assertions.
  void
  finalize_symbols(Symbol_table* symtab, const Layout* layout);

  // Find the name of the output section to use for an input file name
  // and section name.  This returns a name, and sets
  // *OUTPUT_SECTION_SLOT to point to the address where the actual
  // output section may be stored.
  // 1) If the input section should be discarded, this returns NULL
  //    and sets *OUTPUT_SECTION_SLOT to NULL.
  // 2) If the input section is mapped by the SECTIONS clause, this
  //    returns the name to use for the output section (in permanent
  //    storage), and sets *OUTPUT_SECTION_SLOT to point to where the
  //    output section should be stored.  **OUTPUT_SECTION_SLOT will be
  //    non-NULL if we have seen this output section already.
  // 3) If the input section is not mapped by the SECTIONS clause,
  //    this returns SECTION_NAME, and sets *OUTPUT_SECTION_SLOT to
  //    NULL.
  // PSCRIPT_SECTION_TYPE points to a location for returning the section
  // type specified in script.  This can be SCRIPT_SECTION_TYPE_NONE if
  // no type is specified.
  // *KEEP indicates whether the section should survive garbage collection.
  // MATCH_INPUT_SPEC indicates whether the section should be matched
  // with input section specs or simply against the output section name
  // (i.e., for linker-created sections like .dynamic).
  const char*
  output_section_name(const char* file_name, const char* section_name,
		      Output_section*** output_section_slot,
		      Section_type* pscript_section_type,
		      bool* keep, bool match_input_spec);

  // Place a marker for an orphan output section into the SECTIONS
  // clause.
  void
  place_orphan(Output_section* os);

  // Set the addresses of all the output sections.  Return the segment
  // which holds the file header and segment headers, if any.
  Output_segment*
  set_section_addresses(Symbol_table*, Layout*);

  // Add a program header definition.
  void
  add_phdr(const char* name, size_t namelen, unsigned int type,
	   bool filehdr, bool phdrs, bool is_flags_valid, unsigned int flags,
	   Expression* load_address);

  // Return the number of segments we expect to create based on the
  // SECTIONS clause.
  size_t
  expected_segment_count(const Layout*) const;

  // Add the file header and segment header to non-load segments as
  // specified by the PHDRS clause.
  void
  put_headers_in_phdrs(Output_data* file_header, Output_data* segment_headers);

  // Look for an output section by name and return the address, the
  // load address, the alignment, and the size.  This is used when an
  // expression refers to an output section which was not actually
  // created.  This returns true if the section was found, false
  // otherwise.
  bool
  get_output_section_info(const char* name, uint64_t* address,
                          uint64_t* load_address, uint64_t* addralign,
                          uint64_t* size) const;

  // Release all Output_segments.  This is used in relaxation.
  void
  release_segments();

  // Whether we ever saw a SEGMENT_START expression, the presence of which
  // changes the behaviour of -Ttext, -Tdata and -Tbss options.
  bool
  saw_segment_start_expression() const
  { return this->saw_segment_start_expression_; }

  // Set the flag which indicates whether we saw a SEGMENT_START expression.
  void
  set_saw_segment_start_expression(bool value)
  { this->saw_segment_start_expression_ = value; }

  // Add a memory region.
  void
  add_memory_region(const char*, size_t, unsigned int,
		    Expression*, Expression*);

  // Find a memory region's origin.
  Expression*
  find_memory_region_origin(const char*, size_t);

  // Find a memory region's length.
  Expression*
  find_memory_region_length(const char*, size_t);

  // Find a memory region by name.
  Memory_region*
  find_memory_region(const char*, size_t);

  // Find a memory region that should be used by a given output section.
  Memory_region*
  find_memory_region(Output_section_definition*, bool, bool,
		     Output_section_definition**);

  // Returns true if the provide block of memory is contained
  // within a memory region.
  bool
  block_in_region(Symbol_table*, Layout*, uint64_t, uint64_t) const;
    
  // Set the memory region of the section.
  void
  set_memory_region(Memory_region*, bool);

  // Print the contents to the FILE.  This is for debugging.
  void
  print(FILE*) const;

  // Used for orphan sections.
  typedef Sections_elements::iterator Elements_iterator;

 private:
  typedef std::vector<Memory_region*> Memory_regions;
  typedef std::vector<Phdrs_element*> Phdrs_elements;

  // Create segments.
  Output_segment*
  create_segments(Layout*, uint64_t);

  // Create PT_NOTE and PT_TLS segments.
  void
  create_note_and_tls_segments(Layout*, const std::vector<Output_section*>*);

  // Return whether the section is a BSS section.
  static bool
  is_bss_section(const Output_section*);

  // Return the total size of the headers.
  size_t
  total_header_size(Layout* layout) const;

  // Return the amount we have to subtract from the LMA to accommodate
  // headers of the given size.
  uint64_t
  header_size_adjustment(uint64_t lma, size_t sizeof_headers) const;

  // Create the segments from a PHDRS clause.
  Output_segment*
  create_segments_from_phdrs_clause(Layout* layout, uint64_t);

  // Attach sections to segments from a PHDRS clause.
  void
  attach_sections_using_phdrs_clause(Layout*);

  // Set addresses of segments from a PHDRS clause.
  Output_segment*
  set_phdrs_clause_addresses(Layout*, uint64_t);

  // True if we ever saw a SECTIONS clause.
  bool saw_sections_clause_;
  // True if we are currently processing a SECTIONS clause.
  bool in_sections_clause_;
  // The list of elements in the SECTIONS clause.
  Sections_elements* sections_elements_;
  // The current output section, if there is one.
  Output_section_definition* output_section_;
  // The list of memory regions in the MEMORY clause.
  Memory_regions* memory_regions_;
  // The list of program headers in the PHDRS clause.
  Phdrs_elements* phdrs_elements_;
  // Where to put orphan sections.
  Orphan_section_placement* orphan_section_placement_;
  // A pointer to the last Sections_element when we see
  // DATA_SEGMENT_ALIGN.
  Sections_elements::iterator data_segment_align_start_;
  // Whether we have seen DATA_SEGMENT_ALIGN.
  bool saw_data_segment_align_;
  // Whether we have seen DATA_SEGMENT_RELRO_END.
  bool saw_relro_end_;
  // Whether we have seen SEGMENT_START.
  bool saw_segment_start_expression_;
  // Whether we have created all necessary segments.
  bool segments_created_;
};

// Attributes for memory regions.
enum
{
  MEM_EXECUTABLE   = (1 << 0),
  MEM_WRITEABLE    = (1 << 1),
  MEM_READABLE     = (1 << 2),
  MEM_ALLOCATABLE  = (1 << 3),
  MEM_INITIALIZED  = (1 << 4),
  MEM_ATTR_MASK    = (1 << 5) - 1
};

} // End namespace gold.

#endif // !defined(GOLD_SCRIPT_SECTIONS_H
