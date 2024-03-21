// layout.h -- lay out output file sections for gold  -*- C++ -*-

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

#ifndef GOLD_LAYOUT_H
#define GOLD_LAYOUT_H

#include <cstring>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "script.h"
#include "workqueue.h"
#include "object.h"
#include "dynobj.h"
#include "stringpool.h"

namespace gold
{

class General_options;
class Incremental_inputs;
class Incremental_binary;
class Input_objects;
class Mapfile;
class Symbol_table;
class Output_section_data;
class Output_section;
class Output_section_headers;
class Output_segment_headers;
class Output_file_header;
class Output_segment;
class Output_data;
class Output_data_reloc_generic;
class Output_data_dynamic;
class Output_symtab_xindex;
class Output_reduced_debug_abbrev_section;
class Output_reduced_debug_info_section;
class Eh_frame;
class Gdb_index;
class Target;
struct Timespec;

// Return TRUE if SECNAME is the name of a compressed debug section.
extern bool
is_compressed_debug_section(const char* secname);

// Return the name of the corresponding uncompressed debug section.
extern std::string
corresponding_uncompressed_section_name(std::string secname);

// Maintain a list of free space within a section, segment, or file.
// Used for incremental update links.

class Free_list
{
 public:
  struct Free_list_node
  {
    Free_list_node(off_t start, off_t end)
      : start_(start), end_(end)
    { }
    off_t start_;
    off_t end_;
  };
  typedef std::list<Free_list_node>::const_iterator Const_iterator;

  Free_list()
    : list_(), last_remove_(list_.begin()), extend_(false), length_(0),
      min_hole_(0)
  { }

  // Initialize the free list for a section of length LEN.
  // If EXTEND is true, free space may be allocated past the end.
  void
  init(off_t len, bool extend);

  // Set the minimum hole size that is allowed when allocating
  // from the free list.
  void
  set_min_hole_size(off_t min_hole)
  { this->min_hole_ = min_hole; }

  // Remove a chunk from the free list.
  void
  remove(off_t start, off_t end);

  // Allocate a chunk of space from the free list of length LEN,
  // with alignment ALIGN, and minimum offset MINOFF.
  off_t
  allocate(off_t len, uint64_t align, off_t minoff);

  // Return an iterator for the beginning of the free list.
  Const_iterator
  begin() const
  { return this->list_.begin(); }

  // Return an iterator for the end of the free list.
  Const_iterator
  end() const
  { return this->list_.end(); }

  // Dump the free list (for debugging).
  void
  dump();

  // Print usage statistics.
  static void
  print_stats();

 private:
  typedef std::list<Free_list_node>::iterator Iterator;

  // The free list.
  std::list<Free_list_node> list_;

  // The last node visited during a remove operation.
  Iterator last_remove_;

  // Whether we can extend past the original length.
  bool extend_;

  // The total length of the section, segment, or file.
  off_t length_;

  // The minimum hole size allowed.  When allocating from the free list,
  // we must not leave a hole smaller than this.
  off_t min_hole_;

  // Statistics:
  // The total number of free lists used.
  static unsigned int num_lists;
  // The total number of free list nodes used.
  static unsigned int num_nodes;
  // The total number of calls to Free_list::remove.
  static unsigned int num_removes;
  // The total number of nodes visited during calls to Free_list::remove.
  static unsigned int num_remove_visits;
  // The total number of calls to Free_list::allocate.
  static unsigned int num_allocates;
  // The total number of nodes visited during calls to Free_list::allocate.
  static unsigned int num_allocate_visits;
};

// This task function handles mapping the input sections to output
// sections and laying them out in memory.

class Layout_task_runner : public Task_function_runner
{
 public:
  // OPTIONS is the command line options, INPUT_OBJECTS is the list of
  // input objects, SYMTAB is the symbol table, LAYOUT is the layout
  // object.
  Layout_task_runner(const General_options& options,
		     const Input_objects* input_objects,
		     Symbol_table* symtab,
		     Target* target,
		     Layout* layout,
		     Mapfile* mapfile)
    : options_(options), input_objects_(input_objects), symtab_(symtab),
      target_(target), layout_(layout), mapfile_(mapfile)
  { }

  // Run the operation.
  void
  run(Workqueue*, const Task*);

 private:
  Layout_task_runner(const Layout_task_runner&);
  Layout_task_runner& operator=(const Layout_task_runner&);

  const General_options& options_;
  const Input_objects* input_objects_;
  Symbol_table* symtab_;
  Target* target_;
  Layout* layout_;
  Mapfile* mapfile_;
};

// This class holds information about the comdat group or
// .gnu.linkonce section that will be kept for a given signature.

class Kept_section
{
 private:
  // For a comdat group, we build a mapping from the name of each
  // section in the group to the section index and the size in object.
  // When we discard a group in some other object file, we use this
  // map to figure out which kept section the discarded section is
  // associated with.  We then use that mapping when processing relocs
  // against discarded sections.
  struct Comdat_section_info
  {
    // The section index.
    unsigned int shndx;
    // The section size.
    uint64_t size;

    Comdat_section_info(unsigned int a_shndx, uint64_t a_size)
      : shndx(a_shndx), size(a_size)
    { }
  };

  // Most comdat groups have only one or two sections, so we use a
  // std::map rather than an Unordered_map to optimize for that case
  // without paying too heavily for groups with more sections.
  typedef std::map<std::string, Comdat_section_info> Comdat_group;

 public:
  Kept_section()
    : object_(NULL), shndx_(0), is_comdat_(false), is_group_name_(false)
  { this->u_.linkonce_size = 0; }

  // We need to support copies for the signature map in the Layout
  // object, but we should never copy an object after it has been
  // marked as a comdat section.
  Kept_section(const Kept_section& k)
    : object_(k.object_), shndx_(k.shndx_), is_comdat_(false),
      is_group_name_(k.is_group_name_)
  {
    gold_assert(!k.is_comdat_);
    this->u_.linkonce_size = 0;
  }

  ~Kept_section()
  {
    if (this->is_comdat_)
      delete this->u_.group_sections;
  }

  // The object where this section lives.
  Relobj*
  object() const
  { return this->object_; }

  // Set the object.
  void
  set_object(Relobj* object)
  {
    gold_assert(this->object_ == NULL);
    this->object_ = object;
  }

  // The section index.
  unsigned int
  shndx() const
  { return this->shndx_; }

  // Set the section index.
  void
  set_shndx(unsigned int shndx)
  {
    gold_assert(this->shndx_ == 0);
    this->shndx_ = shndx;
  }

  // Whether this is a comdat group.
  bool
  is_comdat() const
  { return this->is_comdat_; }

  // Set that this is a comdat group.
  void
  set_is_comdat()
  {
    gold_assert(!this->is_comdat_);
    this->is_comdat_ = true;
    this->u_.group_sections = new Comdat_group();
  }

  // Whether this is associated with the name of a group or section
  // rather than the symbol name derived from a linkonce section.
  bool
  is_group_name() const
  { return this->is_group_name_; }

  // Note that this represents a comdat group rather than a single
  // linkonce section.
  void
  set_is_group_name()
  { this->is_group_name_ = true; }

  // Add a section to the group list.
  void
  add_comdat_section(const std::string& name, unsigned int shndx,
		     uint64_t size)
  {
    gold_assert(this->is_comdat_);
    Comdat_section_info sinfo(shndx, size);
    this->u_.group_sections->insert(std::make_pair(name, sinfo));
  }

  // Look for a section name in the group list, and return whether it
  // was found.  If found, returns the section index and size.
  bool
  find_comdat_section(const std::string& name, unsigned int* pshndx,
		      uint64_t* psize) const
  {
    gold_assert(this->is_comdat_);
    Comdat_group::const_iterator p = this->u_.group_sections->find(name);
    if (p == this->u_.group_sections->end())
      return false;
    *pshndx = p->second.shndx;
    *psize = p->second.size;
    return true;
  }

  // If there is only one section in the group list, return true, and
  // return the section index and size.
  bool
  find_single_comdat_section(unsigned int* pshndx, uint64_t* psize) const
  {
    gold_assert(this->is_comdat_);
    if (this->u_.group_sections->size() != 1)
      return false;
    Comdat_group::const_iterator p = this->u_.group_sections->begin();
    *pshndx = p->second.shndx;
    *psize = p->second.size;
    return true;
  }

  // Return the size of a linkonce section.
  uint64_t
  linkonce_size() const
  {
    gold_assert(!this->is_comdat_);
    return this->u_.linkonce_size;
  }

  // Set the size of a linkonce section.
  void
  set_linkonce_size(uint64_t size)
  {
    gold_assert(!this->is_comdat_);
    this->u_.linkonce_size = size;
  }

 private:
  // No assignment.
  Kept_section& operator=(const Kept_section&);

  // The object containing the comdat group or .gnu.linkonce section.
  Relobj* object_;
  // Index of the group section for comdats and the section itself for
  // .gnu.linkonce.
  unsigned int shndx_;
  // True if this is for a comdat group rather than a .gnu.linkonce
  // section.
  bool is_comdat_;
  // The Kept_sections are values of a mapping, that maps names to
  // them.  This field is true if this struct is associated with the
  // name of a comdat or .gnu.linkonce, false if it is associated with
  // the name of a symbol obtained from the .gnu.linkonce.* name
  // through some heuristics.
  bool is_group_name_;
  union
  {
    // If the is_comdat_ field is true, this holds a map from names of
    // the sections in the group to section indexes in object_ and to
    // section sizes.
    Comdat_group* group_sections;
    // If the is_comdat_ field is false, this holds the size of the
    // single section.
    uint64_t linkonce_size;
  } u_;
};

// The ordering for output sections.  This controls how output
// sections are ordered within a PT_LOAD output segment.

enum Output_section_order
{
  // Unspecified.  Used for non-load segments.  Also used for the file
  // and segment headers.
  ORDER_INVALID,

  // The PT_INTERP section should come first, so that the dynamic
  // linker can pick it up quickly.
  ORDER_INTERP,

  // The .note.gnu.property section comes next so that the PT_NOTE
  // segment is on the first page of the executable and it won't be
  // placed between other note sections with different alignments.
  ORDER_PROPERTY_NOTE,

  // Loadable read-only note sections come after the .note.gnu.property
  // section.
  ORDER_RO_NOTE,

  // Put read-only sections used by the dynamic linker early in the
  // executable to minimize paging.
  ORDER_DYNAMIC_LINKER,

  // Put reloc sections used by the dynamic linker after other
  // sections used by the dynamic linker; otherwise, objcopy and strip
  // get confused.
  ORDER_DYNAMIC_RELOCS,

  // Put the PLT reloc section after the other dynamic relocs;
  // otherwise, prelink gets confused.
  ORDER_DYNAMIC_PLT_RELOCS,

  // The .init section.
  ORDER_INIT,

  // The PLT.
  ORDER_PLT,

  // The hot text sections, prefixed by .text.hot.
  ORDER_TEXT_HOT,

  // The regular text sections.
  ORDER_TEXT,

  // The startup text sections, prefixed by .text.startup.
  ORDER_TEXT_STARTUP,

  // The startup text sections, prefixed by .text.startup.
  ORDER_TEXT_EXIT,

  // The unlikely text sections, prefixed by .text.unlikely.
  ORDER_TEXT_UNLIKELY,

  // The .fini section.
  ORDER_FINI,

  // The read-only sections.
  ORDER_READONLY,

  // The exception frame sections.
  ORDER_EHFRAME,

  // The TLS sections come first in the data section.
  ORDER_TLS_DATA,
  ORDER_TLS_BSS,

  // Local RELRO (read-only after relocation) sections come before
  // non-local RELRO sections.  This data will be fully resolved by
  // the prelinker.
  ORDER_RELRO_LOCAL,

  // Non-local RELRO sections are grouped together after local RELRO
  // sections.  All RELRO sections must be adjacent so that they can
  // all be put into a PT_GNU_RELRO segment.
  ORDER_RELRO,

  // We permit marking exactly one output section as the last RELRO
  // section.  We do this so that the read-only GOT can be adjacent to
  // the writable GOT.
  ORDER_RELRO_LAST,

  // Similarly, we permit marking exactly one output section as the
  // first non-RELRO section.
  ORDER_NON_RELRO_FIRST,

  // The regular data sections come after the RELRO sections.
  ORDER_DATA,

  // Large data sections normally go in large data segments.
  ORDER_LARGE_DATA,

  // Group writable notes so that we can have a single PT_NOTE
  // segment.
  ORDER_RW_NOTE,

  // The small data sections must be at the end of the data sections,
  // so that they can be adjacent to the small BSS sections.
  ORDER_SMALL_DATA,

  // The BSS sections start here.

  // The small BSS sections must be at the start of the BSS sections,
  // so that they can be adjacent to the small data sections.
  ORDER_SMALL_BSS,

  // The regular BSS sections.
  ORDER_BSS,

  // The large BSS sections come after the other BSS sections.
  ORDER_LARGE_BSS,

  // Maximum value.
  ORDER_MAX
};

// This class handles the details of laying out input sections.

class Layout
{
 public:
  Layout(int number_of_input_files, Script_options*);

  ~Layout()
  {
    delete this->relaxation_debug_check_;
    delete this->segment_states_;
  }

  // For incremental links, record the base file to be modified.
  void
  set_incremental_base(Incremental_binary* base);

  Incremental_binary*
  incremental_base()
  { return this->incremental_base_; }

  // For incremental links, record the initial fixed layout of a section
  // from the base file, and return a pointer to the Output_section.
  template<int size, bool big_endian>
  Output_section*
  init_fixed_output_section(const char*, elfcpp::Shdr<size, big_endian>&);

  // Given an input section SHNDX, named NAME, with data in SHDR, from
  // the object file OBJECT, return the output section where this
  // input section should go.  RELOC_SHNDX is the index of a
  // relocation section which applies to this section, or 0 if none,
  // or -1U if more than one.  RELOC_TYPE is the type of the
  // relocation section if there is one.  Set *OFFSET to the offset
  // within the output section.
  template<int size, bool big_endian>
  Output_section*
  layout(Sized_relobj_file<size, big_endian> *object, unsigned int shndx,
	 const char* name, const elfcpp::Shdr<size, big_endian>& shdr,
	 unsigned int sh_type, unsigned int reloc_shndx,
	 unsigned int reloc_type, off_t* offset);

  std::map<Section_id, unsigned int>*
  get_section_order_map()
  { return &this->section_order_map_; }

  // Struct to store segment info when mapping some input sections to
  // unique segments using linker plugins.  Mapping an input section to
  // a unique segment is done by first placing such input sections in
  // unique output sections and then mapping the output section to a
  // unique segment.  NAME is the name of the output section.  FLAGS
  // and ALIGN are the extra flags and alignment of the segment.
  struct Unique_segment_info
  {
    // Identifier for the segment.  ELF segments don't have names.  This
    // is used as the name of the output section mapped to the segment.
    const char* name;
    // Additional segment flags.
    uint64_t flags;
    // Segment alignment.
    uint64_t align;
  };

  // Mapping from input section to segment.
  typedef std::map<Const_section_id, Unique_segment_info*>
  Section_segment_map;

  // Maps section SECN to SEGMENT s.
  void
  insert_section_segment_map(Const_section_id secn, Unique_segment_info *s);

  // Some input sections require special ordering, for compatibility
  // with GNU ld.  Given the name of an input section, return -1 if it
  // does not require special ordering.  Otherwise, return the index
  // by which it should be ordered compared to other input sections
  // that require special ordering.
  static int
  special_ordering_of_input_section(const char* name);

  bool
  is_section_ordering_specified()
  { return this->section_ordering_specified_; }

  void
  set_section_ordering_specified()
  { this->section_ordering_specified_ = true; }

  bool
  is_unique_segment_for_sections_specified() const
  { return this->unique_segment_for_sections_specified_; }

  void
  set_unique_segment_for_sections_specified()
  { this->unique_segment_for_sections_specified_ = true; }

  bool
  is_lto_slim_object () const
  { return this->lto_slim_object_; }

  void
  set_lto_slim_object ()
  { this->lto_slim_object_ = true; }

  // For incremental updates, allocate a block of memory from the
  // free list.  Find a block starting at or after MINOFF.
  off_t
  allocate(off_t len, uint64_t align, off_t minoff)
  { return this->free_list_.allocate(len, align, minoff); }

  unsigned int
  find_section_order_index(const std::string&);

  // Read the sequence of input sections from the file specified with
  // linker option --section-ordering-file.
  void
  read_layout_from_file();

  // Layout an input reloc section when doing a relocatable link.  The
  // section is RELOC_SHNDX in OBJECT, with data in SHDR.
  // DATA_SECTION is the reloc section to which it refers.  RR is the
  // relocatable information.
  template<int size, bool big_endian>
  Output_section*
  layout_reloc(Sized_relobj_file<size, big_endian>* object,
	       unsigned int reloc_shndx,
	       const elfcpp::Shdr<size, big_endian>& shdr,
	       Output_section* data_section,
	       Relocatable_relocs* rr);

  // Layout a group section when doing a relocatable link.
  template<int size, bool big_endian>
  void
  layout_group(Symbol_table* symtab,
	       Sized_relobj_file<size, big_endian>* object,
	       unsigned int group_shndx,
	       const char* group_section_name,
	       const char* signature,
	       const elfcpp::Shdr<size, big_endian>& shdr,
	       elfcpp::Elf_Word flags,
	       std::vector<unsigned int>* shndxes);

  // Like layout, only for exception frame sections.  OBJECT is an
  // object file.  SYMBOLS is the contents of the symbol table
  // section, with size SYMBOLS_SIZE.  SYMBOL_NAMES is the contents of
  // the symbol name section, with size SYMBOL_NAMES_SIZE.  SHNDX is a
  // .eh_frame section in OBJECT.  SHDR is the section header.
  // RELOC_SHNDX is the index of a relocation section which applies to
  // this section, or 0 if none, or -1U if more than one.  RELOC_TYPE
  // is the type of the relocation section if there is one.  This
  // returns the output section, and sets *OFFSET to the offset.
  template<int size, bool big_endian>
  Output_section*
  layout_eh_frame(Sized_relobj_file<size, big_endian>* object,
		  const unsigned char* symbols,
		  off_t symbols_size,
		  const unsigned char* symbol_names,
		  off_t symbol_names_size,
		  unsigned int shndx,
		  const elfcpp::Shdr<size, big_endian>& shdr,
		  unsigned int reloc_shndx, unsigned int reloc_type,
		  off_t* offset);

  // After processing all input files, we call this to make sure that
  // the optimized .eh_frame sections have been added to the output
  // section.
  void
  finalize_eh_frame_section();

  // Add .eh_frame information for a PLT.  The FDE must start with a
  // 4-byte PC-relative reference to the start of the PLT, followed by
  // a 4-byte size of PLT.
  void
  add_eh_frame_for_plt(Output_data* plt, const unsigned char* cie_data,
		       size_t cie_length, const unsigned char* fde_data,
		       size_t fde_length);

  // Remove all post-map .eh_frame information for a PLT.
  void
  remove_eh_frame_for_plt(Output_data* plt, const unsigned char* cie_data,
			  size_t cie_length);

  // Scan a .debug_info or .debug_types section, and add summary
  // information to the .gdb_index section.
  template<int size, bool big_endian>
  void
  add_to_gdb_index(bool is_type_unit,
		   Sized_relobj<size, big_endian>* object,
		   const unsigned char* symbols,
		   off_t symbols_size,
		   unsigned int shndx,
		   unsigned int reloc_shndx,
		   unsigned int reloc_type);

  // Handle a GNU stack note.  This is called once per input object
  // file.  SEEN_GNU_STACK is true if the object file has a
  // .note.GNU-stack section.  GNU_STACK_FLAGS is the section flags
  // from that section if there was one.
  void
  layout_gnu_stack(bool seen_gnu_stack, uint64_t gnu_stack_flags,
		   const Object*);

  // Layout a .note.gnu.property section.
  void
  layout_gnu_property(unsigned int note_type,
		      unsigned int pr_type,
		      size_t pr_datasz,
		      const unsigned char* pr_data,
		      const Object* object);

  // Merge per-object properties with program properties.
  void
  merge_gnu_properties(const Object* object);

  // Add a target-specific property for the output .note.gnu.property section.
  void
  add_gnu_property(unsigned int note_type,
		   unsigned int pr_type,
		   size_t pr_datasz,
		   const unsigned char* pr_data);

  // Add an Output_section_data to the layout.  This is used for
  // special sections like the GOT section.  ORDER is where the
  // section should wind up in the output segment.  IS_RELRO is true
  // for relro sections.
  Output_section*
  add_output_section_data(const char* name, elfcpp::Elf_Word type,
			  elfcpp::Elf_Xword flags,
			  Output_section_data*, Output_section_order order,
			  bool is_relro);

  // Increase the size of the relro segment by this much.
  void
  increase_relro(unsigned int s)
  { this->increase_relro_ += s; }

  // Create dynamic sections if necessary.
  void
  create_initial_dynamic_sections(Symbol_table*);

  // Define __start and __stop symbols for output sections.
  void
  define_section_symbols(Symbol_table*);

  // Create automatic note sections.
  void
  create_notes();

  // Create sections for linker scripts.
  void
  create_script_sections()
  { this->script_options_->create_script_sections(this); }

  // Define symbols from any linker script.
  void
  define_script_symbols(Symbol_table* symtab)
  { this->script_options_->add_symbols_to_table(symtab); }

  // Define symbols for group signatures.
  void
  define_group_signatures(Symbol_table*);

  // Return the Stringpool used for symbol names.
  const Stringpool*
  sympool() const
  { return &this->sympool_; }

  // Return the Stringpool used for dynamic symbol names and dynamic
  // tags.
  const Stringpool*
  dynpool() const
  { return &this->dynpool_; }

  // Return the .dynamic output section.  This is only valid after the
  // layout has been finalized.
  Output_section*
  dynamic_section() const
  { return this->dynamic_section_; }

  // Return the symtab_xindex section used to hold large section
  // indexes for the normal symbol table.
  Output_symtab_xindex*
  symtab_xindex() const
  { return this->symtab_xindex_; }

  // Return the dynsym_xindex section used to hold large section
  // indexes for the dynamic symbol table.
  Output_symtab_xindex*
  dynsym_xindex() const
  { return this->dynsym_xindex_; }

  // Return whether a section is a .gnu.linkonce section, given the
  // section name.
  static inline bool
  is_linkonce(const char* name)
  { return strncmp(name, ".gnu.linkonce", sizeof(".gnu.linkonce") - 1) == 0; }

  // Whether we have added an input section.
  bool
  have_added_input_section() const
  { return this->have_added_input_section_; }

  // Return true if a section is a debugging section.
  static inline bool
  is_debug_info_section(const char* name)
  {
    // Debugging sections can only be recognized by name.
    return (strncmp(name, ".debug", sizeof(".debug") - 1) == 0
	    || strncmp(name, ".zdebug", sizeof(".zdebug") - 1) == 0
	    || strncmp(name, ".gnu.linkonce.wi.",
		       sizeof(".gnu.linkonce.wi.") - 1) == 0
	    || strncmp(name, ".line", sizeof(".line") - 1) == 0
	    || strncmp(name, ".stab", sizeof(".stab") - 1) == 0
	    || strncmp(name, ".pdr", sizeof(".pdr") - 1) == 0);
  }

  // Return true if RELOBJ is an input file whose base name matches
  // FILE_NAME.  The base name must have an extension of ".o", and
  // must be exactly FILE_NAME.o or FILE_NAME, one character, ".o".
  static bool
  match_file_name(const Relobj* relobj, const char* file_name);

  // Return whether section SHNDX in RELOBJ is a .ctors/.dtors section
  // with more than one word being mapped to a .init_array/.fini_array
  // section.
  bool
  is_ctors_in_init_array(Relobj* relobj, unsigned int shndx) const;

  // Check if a comdat group or .gnu.linkonce section with the given
  // NAME is selected for the link.  If there is already a section,
  // *KEPT_SECTION is set to point to the signature and the function
  // returns false.  Otherwise, OBJECT, SHNDX,IS_COMDAT, and
  // IS_GROUP_NAME are recorded for this NAME in the layout object,
  // *KEPT_SECTION is set to the internal copy and the function return
  // false.
  bool
  find_or_add_kept_section(const std::string& name, Relobj* object,
			   unsigned int shndx, bool is_comdat,
			   bool is_group_name, Kept_section** kept_section);

  // Finalize the layout after all the input sections have been added.
  off_t
  finalize(const Input_objects*, Symbol_table*, Target*, const Task*);

  // Return whether any sections require postprocessing.
  bool
  any_postprocessing_sections() const
  { return this->any_postprocessing_sections_; }

  // Return the size of the output file.
  off_t
  output_file_size() const
  { return this->output_file_size_; }

  // Return the TLS segment.  This will return NULL if there isn't
  // one.
  Output_segment*
  tls_segment() const
  { return this->tls_segment_; }

  // Return the normal symbol table.
  Output_section*
  symtab_section() const
  {
    gold_assert(this->symtab_section_ != NULL);
    return this->symtab_section_;
  }

  // Return the file offset of the normal symbol table.
  off_t
  symtab_section_offset() const;

  // Return the section index of the normal symbol tabl.e
  unsigned int
  symtab_section_shndx() const;

  // Return the dynamic symbol table.
  Output_section*
  dynsym_section() const
  {
    gold_assert(this->dynsym_section_ != NULL);
    return this->dynsym_section_;
  }

  // Return the dynamic tags.
  Output_data_dynamic*
  dynamic_data() const
  { return this->dynamic_data_; }

  // Write out the output sections.
  void
  write_output_sections(Output_file* of) const;

  // Write out data not associated with an input file or the symbol
  // table.
  void
  write_data(const Symbol_table*, Output_file*) const;

  // Write out output sections which can not be written until all the
  // input sections are complete.
  void
  write_sections_after_input_sections(Output_file* of);

  // Return an output section named NAME, or NULL if there is none.
  Output_section*
  find_output_section(const char* name) const;

  // Return an output segment of type TYPE, with segment flags SET set
  // and segment flags CLEAR clear.  Return NULL if there is none.
  Output_segment*
  find_output_segment(elfcpp::PT type, elfcpp::Elf_Word set,
		      elfcpp::Elf_Word clear) const;

  // Return the number of segments we expect to produce.
  size_t
  expected_segment_count() const;

  // Set a flag to indicate that an object file uses the static TLS model.
  void
  set_has_static_tls()
  { this->has_static_tls_ = true; }

  // Return true if any object file uses the static TLS model.
  bool
  has_static_tls() const
  { return this->has_static_tls_; }

  // Return the options which may be set by a linker script.
  Script_options*
  script_options()
  { return this->script_options_; }

  const Script_options*
  script_options() const
  { return this->script_options_; }

  // Return the object managing inputs in incremental build. NULL in
  // non-incremental builds.
  Incremental_inputs*
  incremental_inputs() const
  { return this->incremental_inputs_; }

  // For the target-specific code to add dynamic tags which are common
  // to most targets.
  void
  add_target_dynamic_tags(bool use_rel, const Output_data* plt_got,
			  const Output_data* plt_rel,
			  const Output_data_reloc_generic* dyn_rel,
			  bool add_debug, bool dynrel_includes_plt,
			  bool custom_relcount);

  // Add a target-specific dynamic tag with constant value.
  void
  add_target_specific_dynamic_tag(elfcpp::DT tag, unsigned int val);

  // Compute and write out the build ID if needed.
  void
  write_build_id(Output_file*, unsigned char*, size_t) const;

  // Rewrite output file in binary format.
  void
  write_binary(Output_file* in) const;

  // Print output sections to the map file.
  void
  print_to_mapfile(Mapfile*) const;

  // Dump statistical information to stderr.
  void
  print_stats() const;

  // A list of segments.

  typedef std::vector<Output_segment*> Segment_list;

  // A list of sections.

  typedef std::vector<Output_section*> Section_list;

  // The list of information to write out which is not attached to
  // either a section or a segment.
  typedef std::vector<Output_data*> Data_list;

  // Store the allocated sections into the section list.  This is used
  // by the linker script code.
  void
  get_allocated_sections(Section_list*) const;

  // Store the executable sections into the section list.
  void
  get_executable_sections(Section_list*) const;

  // Make a section for a linker script to hold data.
  Output_section*
  make_output_section_for_script(const char* name,
				 Script_sections::Section_type section_type);

  // Make a segment.  This is used by the linker script code.
  Output_segment*
  make_output_segment(elfcpp::Elf_Word type, elfcpp::Elf_Word flags);

  // Return the number of segments.
  size_t
  segment_count() const
  { return this->segment_list_.size(); }

  // Map from section flags to segment flags.
  static elfcpp::Elf_Word
  section_flags_to_segment(elfcpp::Elf_Xword flags);

  // Attach sections to segments.
  void
  attach_sections_to_segments(const Target*);

  // For relaxation clean up, we need to know output section data created
  // from a linker script.
  void
  new_output_section_data_from_script(Output_section_data* posd)
  {
    if (this->record_output_section_data_from_script_)
      this->script_output_section_data_list_.push_back(posd);
  }

  // Return section list.
  const Section_list&
  section_list() const
  { return this->section_list_; }

  // Returns TRUE iff NAME (an input section from RELOBJ) will
  // be mapped to an output section that should be KEPT.
  bool
  keep_input_section(const Relobj*, const char*);

  // Add a special output object that will be recreated afresh
  // if there is another relaxation iteration.
  void
  add_relax_output(Output_data* data)
  { this->relax_output_list_.push_back(data); }

  // Clear out (and free) everything added by add_relax_output.
  void
  reset_relax_output();

 private:
  Layout(const Layout&);
  Layout& operator=(const Layout&);

  // Mapping from input section names to output section names.
  struct Section_name_mapping
  {
    const char* from;
    int fromlen;
    const char* to;
    int tolen;
  };
  static const Section_name_mapping section_name_mapping[];
  static const int section_name_mapping_count;
  static const Section_name_mapping text_section_name_mapping[];
  static const int text_section_name_mapping_count;

  // Find section name NAME in map and return the mapped name if found
  // with the length set in PLEN.
  static const char* match_section_name(const Section_name_mapping* map,
					const int count, const char* name,
					size_t* plen);

  // During a relocatable link, a list of group sections and
  // signatures.
  struct Group_signature
  {
    // The group section.
    Output_section* section;
    // The signature.
    const char* signature;

    Group_signature()
      : section(NULL), signature(NULL)
    { }

    Group_signature(Output_section* sectiona, const char* signaturea)
      : section(sectiona), signature(signaturea)
    { }
  };
  typedef std::vector<Group_signature> Group_signatures;

  // Create a note section, filling in the header.
  Output_section*
  create_note(const char* name, int note_type, const char* section_name,
	      size_t descsz, bool allocate, size_t* trailing_padding);

  // Create a note section for gnu program properties.
  void
  create_gnu_properties_note();

  // Create a note section for gold version.
  void
  create_gold_note();

  // Record whether the stack must be executable, and a user-supplied size.
  void
  create_stack_segment();

  // Create a build ID note if needed.
  void
  create_build_id();

  // Create a package metadata note if needed.
  void
  create_package_metadata();

  // Link .stab and .stabstr sections.
  void
  link_stabs_sections();

  // Create .gnu_incremental_inputs and .gnu_incremental_strtab sections needed
  // for the next run of incremental linking to check what has changed.
  void
  create_incremental_info_sections(Symbol_table*);

  // Find the first read-only PT_LOAD segment, creating one if
  // necessary.
  Output_segment*
  find_first_load_seg(const Target*);

  // Count the local symbols in the regular symbol table and the dynamic
  // symbol table, and build the respective string pools.
  void
  count_local_symbols(const Task*, const Input_objects*);

  // Create the output sections for the symbol table.
  void
  create_symtab_sections(const Input_objects*, Symbol_table*,
			 unsigned int, off_t*, unsigned int);

  // Create the .shstrtab section.
  Output_section*
  create_shstrtab();

  // Create the section header table.
  void
  create_shdrs(const Output_section* shstrtab_section, off_t*);

  // Create the dynamic symbol table.
  void
  create_dynamic_symtab(const Input_objects*, Symbol_table*,
			Output_section** pdynstr,
			unsigned int* plocal_dynamic_count,
			unsigned int* pforced_local_dynamic_count,
			std::vector<Symbol*>* pdynamic_symbols,
			Versions* versions);

  // Assign offsets to each local portion of the dynamic symbol table.
  void
  assign_local_dynsym_offsets(const Input_objects*);

  // Finish the .dynamic section and PT_DYNAMIC segment.
  void
  finish_dynamic_section(const Input_objects*, const Symbol_table*);

  // Set the size of the _DYNAMIC symbol.
  void
  set_dynamic_symbol_size(const Symbol_table*);

  // Create the .interp section and PT_INTERP segment.
  void
  create_interp(const Target* target);

  // Create the version sections.
  void
  create_version_sections(const Versions*,
			  const Symbol_table*,
			  unsigned int local_symcount,
			  const std::vector<Symbol*>& dynamic_symbols,
			  const Output_section* dynstr);

  template<int size, bool big_endian>
  void
  sized_create_version_sections(const Versions* versions,
				const Symbol_table*,
				unsigned int local_symcount,
				const std::vector<Symbol*>& dynamic_symbols,
				const Output_section* dynstr);

  // Return whether to include this section in the link.
  template<int size, bool big_endian>
  bool
  include_section(Sized_relobj_file<size, big_endian>* object, const char* name,
		  const elfcpp::Shdr<size, big_endian>&);

  // Return the output section name to use given an input section
  // name.  Set *PLEN to the length of the name.  *PLEN must be
  // initialized to the length of NAME.
  static const char*
  output_section_name(const Relobj*, const char* name, size_t* plen);

  // Return the number of allocated output sections.
  size_t
  allocated_output_section_count() const;

  // Return the output section for NAME, TYPE and FLAGS.
  Output_section*
  get_output_section(const char* name, Stringpool::Key name_key,
		     elfcpp::Elf_Word type, elfcpp::Elf_Xword flags,
		     Output_section_order order, bool is_relro);

  // Clear the input section flags that should not be copied to the
  // output section.
  elfcpp::Elf_Xword
  get_output_section_flags (elfcpp::Elf_Xword input_section_flags);

  // Choose the output section for NAME in RELOBJ.
  Output_section*
  choose_output_section(const Relobj* relobj, const char* name,
			elfcpp::Elf_Word type, elfcpp::Elf_Xword flags,
			bool is_input_section, Output_section_order order,
			bool is_relro, bool is_reloc, bool match_input_spec);

  // Create a new Output_section.
  Output_section*
  make_output_section(const char* name, elfcpp::Elf_Word type,
		      elfcpp::Elf_Xword flags, Output_section_order order,
		      bool is_relro);

  // Attach a section to a segment.
  void
  attach_section_to_segment(const Target*, Output_section*);

  // Get section order.
  Output_section_order
  default_section_order(Output_section*, bool is_relro_local);

  // Attach an allocated section to a segment.
  void
  attach_allocated_section_to_segment(const Target*, Output_section*);

  // Make the .eh_frame section.
  Output_section*
  make_eh_frame_section(const Relobj*);

  // Set the final file offsets of all the segments.
  off_t
  set_segment_offsets(const Target*, Output_segment*, unsigned int* pshndx);

  // Set the file offsets of the sections when doing a relocatable
  // link.
  off_t
  set_relocatable_section_offsets(Output_data*, unsigned int* pshndx);

  // Set the final file offsets of all the sections not associated
  // with a segment.  We set section offsets in three passes: the
  // first handles all allocated sections, the second sections that
  // require postprocessing, and the last the late-bound STRTAB
  // sections (probably only shstrtab, which is the one we care about
  // because it holds section names).
  enum Section_offset_pass
  {
    BEFORE_INPUT_SECTIONS_PASS,
    POSTPROCESSING_SECTIONS_PASS,
    STRTAB_AFTER_POSTPROCESSING_SECTIONS_PASS
  };
  off_t
  set_section_offsets(off_t, Section_offset_pass pass);

  // Set the final section indexes of all the sections not associated
  // with a segment.  Returns the next unused index.
  unsigned int
  set_section_indexes(unsigned int pshndx);

  // Set the section addresses when using a script.
  Output_segment*
  set_section_addresses_from_script(Symbol_table*);

  // Find appropriate places or orphan sections in a script.
  void
  place_orphan_sections_in_script();

  // Return whether SEG1 comes before SEG2 in the output file.
  bool
  segment_precedes(const Output_segment* seg1, const Output_segment* seg2);

  // Use to save and restore segments during relaxation.
  typedef Unordered_map<const Output_segment*, const Output_segment*>
    Segment_states;

  // Save states of current output segments.
  void
  save_segments(Segment_states*);

  // Restore output segment states.
  void
  restore_segments(const Segment_states*);

  // Clean up after relaxation so that it is possible to lay out the
  // sections and segments again.
  void
  clean_up_after_relaxation();

  // Doing preparation work for relaxation.  This is factored out to make
  // Layout::finalized a bit smaller and easier to read.
  void
  prepare_for_relaxation();

  // Main body of the relaxation loop, which lays out the section.
  off_t
  relaxation_loop_body(int, Target*, Symbol_table*, Output_segment**,
		       Output_segment*, Output_segment_headers*,
		       Output_file_header*, unsigned int*);

  // A mapping used for kept comdats/.gnu.linkonce group signatures.
  typedef Unordered_map<std::string, Kept_section> Signatures;

  // Mapping from input section name/type/flags to output section.  We
  // use canonicalized strings here.

  typedef std::pair<Stringpool::Key,
		    std::pair<elfcpp::Elf_Word, elfcpp::Elf_Xword> > Key;

  struct Hash_key
  {
    size_t
    operator()(const Key& k) const;
  };

  typedef Unordered_map<Key, Output_section*, Hash_key> Section_name_map;

  // A comparison class for segments.

  class Compare_segments
  {
   public:
    Compare_segments(Layout* layout)
      : layout_(layout)
    { }

    bool
    operator()(const Output_segment* seg1, const Output_segment* seg2)
    { return this->layout_->segment_precedes(seg1, seg2); }

   private:
    Layout* layout_;
  };

  typedef std::vector<Output_section_data*> Output_section_data_list;

  // Debug checker class.
  class Relaxation_debug_check
  {
   public:
    Relaxation_debug_check()
      : section_infos_()
    { }

    // Check that sections and special data are in reset states.
    void
    check_output_data_for_reset_values(const Layout::Section_list&,
				       const Layout::Data_list& special_outputs,
				       const Layout::Data_list& relax_outputs);

    // Record information of a section list.
    void
    read_sections(const Layout::Section_list&);

    // Verify a section list with recorded information.
    void
    verify_sections(const Layout::Section_list&);

   private:
    // Information we care about a section.
    struct Section_info
    {
      // Output section described by this.
      Output_section* output_section;
      // Load address.
      uint64_t address;
      // Data size.
      off_t data_size;
      // File offset.
      off_t offset;
    };

    // Section information.
    std::vector<Section_info> section_infos_;
  };

  // Program properties from .note.gnu.property sections.
  struct Gnu_property
  {
    size_t pr_datasz;
    unsigned char* pr_data;
  };
  typedef std::map<unsigned int, Gnu_property> Gnu_properties;

  // The number of input files, for sizing tables.
  int number_of_input_files_;
  // Information set by scripts or by command line options.
  Script_options* script_options_;
  // The output section names.
  Stringpool namepool_;
  // The output symbol names.
  Stringpool sympool_;
  // The dynamic strings, if needed.
  Stringpool dynpool_;
  // The list of group sections and linkonce sections which we have seen.
  Signatures signatures_;
  // The mapping from input section name/type/flags to output sections.
  Section_name_map section_name_map_;
  // The list of output segments.
  Segment_list segment_list_;
  // The list of output sections.
  Section_list section_list_;
  // The list of output sections which are not attached to any output
  // segment.
  Section_list unattached_section_list_;
  // The list of unattached Output_data objects which require special
  // handling because they are not Output_sections.
  Data_list special_output_list_;
  // Like special_output_list_, but cleared and recreated on each
  // iteration of relaxation.
  Data_list relax_output_list_;
  // The section headers.
  Output_section_headers* section_headers_;
  // A pointer to the PT_TLS segment if there is one.
  Output_segment* tls_segment_;
  // A pointer to the PT_GNU_RELRO segment if there is one.
  Output_segment* relro_segment_;
  // A pointer to the PT_INTERP segment if there is one.
  Output_segment* interp_segment_;
  // A backend may increase the size of the PT_GNU_RELRO segment if
  // there is one.  This is the amount to increase it by.
  unsigned int increase_relro_;
  // The SHT_SYMTAB output section.
  Output_section* symtab_section_;
  // The SHT_SYMTAB_SHNDX for the regular symbol table if there is one.
  Output_symtab_xindex* symtab_xindex_;
  // The SHT_DYNSYM output section if there is one.
  Output_section* dynsym_section_;
  // The SHT_SYMTAB_SHNDX for the dynamic symbol table if there is one.
  Output_symtab_xindex* dynsym_xindex_;
  // The SHT_DYNAMIC output section if there is one.
  Output_section* dynamic_section_;
  // The _DYNAMIC symbol if there is one.
  Symbol* dynamic_symbol_;
  // The dynamic data which goes into dynamic_section_.
  Output_data_dynamic* dynamic_data_;
  // The exception frame output section if there is one.
  Output_section* eh_frame_section_;
  // The exception frame data for eh_frame_section_.
  Eh_frame* eh_frame_data_;
  // Whether we have added eh_frame_data_ to the .eh_frame section.
  bool added_eh_frame_data_;
  // The exception frame header output section if there is one.
  Output_section* eh_frame_hdr_section_;
  // The data for the .gdb_index section.
  Gdb_index* gdb_index_data_;
  // The space for the build ID checksum if there is one.
  Output_section_data* build_id_note_;
  // The space for the package metadata JSON if there is one.
  Output_section_data* package_metadata_note_;
  // The output section containing dwarf abbreviations
  Output_reduced_debug_abbrev_section* debug_abbrev_;
  // The output section containing the dwarf debug info tree
  Output_reduced_debug_info_section* debug_info_;
  // A list of group sections and their signatures.
  Group_signatures group_signatures_;
  // The size of the output file.
  off_t output_file_size_;
  // Whether we have added an input section to an output section.
  bool have_added_input_section_;
  // Whether we have attached the sections to the segments.
  bool sections_are_attached_;
  // Whether we have seen an object file marked to require an
  // executable stack.
  bool input_requires_executable_stack_;
  // Whether we have seen at least one object file with an executable
  // stack marker.
  bool input_with_gnu_stack_note_;
  // Whether we have seen at least one object file without an
  // executable stack marker.
  bool input_without_gnu_stack_note_;
  // Whether we have seen an object file that uses the static TLS model.
  bool has_static_tls_;
  // Whether any sections require postprocessing.
  bool any_postprocessing_sections_;
  // Whether we have resized the signatures_ hash table.
  bool resized_signatures_;
  // Whether we have created a .stab*str output section.
  bool have_stabstr_section_;
  // True if the input sections in the output sections should be sorted
  // as specified in a section ordering file.
  bool section_ordering_specified_;
  // True if some input sections need to be mapped to a unique segment,
  // after being mapped to a unique Output_section.
  bool unique_segment_for_sections_specified_;
  // In incremental build, holds information check the inputs and build the
  // .gnu_incremental_inputs section.
  Incremental_inputs* incremental_inputs_;
  // Whether we record output section data created in script
  bool record_output_section_data_from_script_;
  // Set if this is a slim LTO object not loaded with a compiler plugin
  bool lto_slim_object_;
  // List of output data that needs to be removed at relaxation clean up.
  Output_section_data_list script_output_section_data_list_;
  // Structure to save segment states before entering the relaxation loop.
  Segment_states* segment_states_;
  // A relaxation debug checker.  We only create one when in debugging mode.
  Relaxation_debug_check* relaxation_debug_check_;
  // Plugins specify section_ordering using this map.  This is set in
  // update_section_order in plugin.cc
  std::map<Section_id, unsigned int> section_order_map_;
  // This maps an input section to a unique segment. This is done by first
  // placing such input sections in unique output sections and then mapping
  // the output section to a unique segment.  Unique_segment_info stores
  // any additional flags and alignment of the new segment.
  Section_segment_map section_segment_map_;
  // Hash a pattern to its position in the section ordering file.
  Unordered_map<std::string, unsigned int> input_section_position_;
  // Vector of glob only patterns in the section_ordering file.
  std::vector<std::string> input_section_glob_;
  // For incremental links, the base file to be modified.
  Incremental_binary* incremental_base_;
  // For incremental links, a list of free space within the file.
  Free_list free_list_;
  // Program properties.
  Gnu_properties gnu_properties_;
};

// This task handles writing out data in output sections which is not
// part of an input section, or which requires special handling.  When
// this is done, it unblocks both output_sections_blocker and
// final_blocker.

class Write_sections_task : public Task
{
 public:
  Write_sections_task(const Layout* layout, Output_file* of,
		      Task_token* output_sections_blocker,
		      Task_token* input_sections_blocker,
		      Task_token* final_blocker)
    : layout_(layout), of_(of),
      output_sections_blocker_(output_sections_blocker),
      input_sections_blocker_(input_sections_blocker),
      final_blocker_(final_blocker)
  { }

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const
  { return "Write_sections_task"; }

 private:
  class Write_sections_locker;

  const Layout* layout_;
  Output_file* of_;
  Task_token* output_sections_blocker_;
  Task_token* input_sections_blocker_;
  Task_token* final_blocker_;
};

// This task handles writing out data which is not part of a section
// or segment.

class Write_data_task : public Task
{
 public:
  Write_data_task(const Layout* layout, const Symbol_table* symtab,
		  Output_file* of, Task_token* final_blocker)
    : layout_(layout), symtab_(symtab), of_(of), final_blocker_(final_blocker)
  { }

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const
  { return "Write_data_task"; }

 private:
  const Layout* layout_;
  const Symbol_table* symtab_;
  Output_file* of_;
  Task_token* final_blocker_;
};

// This task handles writing out the global symbols.

class Write_symbols_task : public Task
{
 public:
  Write_symbols_task(const Layout* layout, const Symbol_table* symtab,
		     const Input_objects* /*input_objects*/,
		     const Stringpool* sympool, const Stringpool* dynpool,
		     Output_file* of, Task_token* final_blocker)
    : layout_(layout), symtab_(symtab),
      sympool_(sympool), dynpool_(dynpool), of_(of),
      final_blocker_(final_blocker)
  { }

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const
  { return "Write_symbols_task"; }

 private:
  const Layout* layout_;
  const Symbol_table* symtab_;
  const Stringpool* sympool_;
  const Stringpool* dynpool_;
  Output_file* of_;
  Task_token* final_blocker_;
};

// This task handles writing out data in output sections which can't
// be written out until all the input sections have been handled.
// This is for sections whose contents is based on the contents of
// other output sections.

class Write_after_input_sections_task : public Task
{
 public:
  Write_after_input_sections_task(Layout* layout, Output_file* of,
				  Task_token* input_sections_blocker,
				  Task_token* final_blocker)
    : layout_(layout), of_(of),
      input_sections_blocker_(input_sections_blocker),
      final_blocker_(final_blocker)
  { }

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const
  { return "Write_after_input_sections_task"; }

 private:
  Layout* layout_;
  Output_file* of_;
  Task_token* input_sections_blocker_;
  Task_token* final_blocker_;
};

// This task function handles computation of the build id.
// When using --build-id=tree, it schedules the tasks that
// compute the hashes for each chunk of the file. This task
// cannot run until we have finalized the size of the output
// file, after the completion of Write_after_input_sections_task.

class Build_id_task_runner : public Task_function_runner
{
 public:
  Build_id_task_runner(const General_options* options, const Layout* layout,
		       Output_file* of)
    : options_(options), layout_(layout), of_(of)
  { }

  // Run the operation.
  void
  run(Workqueue*, const Task*);

 private:
  const General_options* options_;
  const Layout* layout_;
  Output_file* of_;
};

// This task function handles closing the file.

class Close_task_runner : public Task_function_runner
{
 public:
  Close_task_runner(const General_options* options, const Layout* layout,
		    Output_file* of, unsigned char* array_of_hashes,
		    size_t size_of_hashes)
    : options_(options), layout_(layout), of_(of),
      array_of_hashes_(array_of_hashes), size_of_hashes_(size_of_hashes)
  { }

  // Run the operation.
  void
  run(Workqueue*, const Task*);

 private:
  const General_options* options_;
  const Layout* layout_;
  Output_file* of_;
  unsigned char* const array_of_hashes_;
  const size_t size_of_hashes_;
};

// A small helper function to align an address.

inline uint64_t
align_address(uint64_t address, uint64_t addralign)
{
  if (addralign != 0)
    address = (address + addralign - 1) &~ (addralign - 1);
  return address;
}

} // End namespace gold.

#endif // !defined(GOLD_LAYOUT_H)
