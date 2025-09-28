// layout.cc -- lay out output file sections for gold

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

#include <cerrno>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <fcntl.h>
#include <fnmatch.h>
#include <unistd.h>
#include "libiberty.h"
#include "md5.h"
#include "sha1.h"
#ifdef __MINGW32__
#include <windows.h>
#include <rpcdce.h>
#endif
#ifdef HAVE_JANSSON
#include <jansson.h>
#endif

#include "parameters.h"
#include "options.h"
#include "mapfile.h"
#include "script.h"
#include "script-sections.h"
#include "output.h"
#include "symtab.h"
#include "dynobj.h"
#include "ehframe.h"
#include "gdb-index.h"
#include "compressed_output.h"
#include "reduced_debug_output.h"
#include "object.h"
#include "reloc.h"
#include "descriptors.h"
#include "plugin.h"
#include "incremental.h"
#include "layout.h"

namespace gold
{

// Class Free_list.

// The total number of free lists used.
unsigned int Free_list::num_lists = 0;
// The total number of free list nodes used.
unsigned int Free_list::num_nodes = 0;
// The total number of calls to Free_list::remove.
unsigned int Free_list::num_removes = 0;
// The total number of nodes visited during calls to Free_list::remove.
unsigned int Free_list::num_remove_visits = 0;
// The total number of calls to Free_list::allocate.
unsigned int Free_list::num_allocates = 0;
// The total number of nodes visited during calls to Free_list::allocate.
unsigned int Free_list::num_allocate_visits = 0;

// Initialize the free list.  Creates a single free list node that
// describes the entire region of length LEN.  If EXTEND is true,
// allocate() is allowed to extend the region beyond its initial
// length.

void
Free_list::init(off_t len, bool extend)
{
  this->list_.push_front(Free_list_node(0, len));
  this->last_remove_ = this->list_.begin();
  this->extend_ = extend;
  this->length_ = len;
  ++Free_list::num_lists;
  ++Free_list::num_nodes;
}

// Remove a chunk from the free list.  Because we start with a single
// node that covers the entire section, and remove chunks from it one
// at a time, we do not need to coalesce chunks or handle cases that
// span more than one free node.  We expect to remove chunks from the
// free list in order, and we expect to have only a few chunks of free
// space left (corresponding to files that have changed since the last
// incremental link), so a simple linear list should provide sufficient
// performance.

void
Free_list::remove(off_t start, off_t end)
{
  if (start == end)
    return;
  gold_assert(start < end);

  ++Free_list::num_removes;

  Iterator p = this->last_remove_;
  if (p->start_ > start)
    p = this->list_.begin();

  for (; p != this->list_.end(); ++p)
    {
      ++Free_list::num_remove_visits;
      // Find a node that wholly contains the indicated region.
      if (p->start_ <= start && p->end_ >= end)
	{
	  // Case 1: the indicated region spans the whole node.
	  // Add some fuzz to avoid creating tiny free chunks.
	  if (p->start_ + 3 >= start && p->end_ <= end + 3)
	    p = this->list_.erase(p);
	  // Case 2: remove a chunk from the start of the node.
	  else if (p->start_ + 3 >= start)
	    p->start_ = end;
	  // Case 3: remove a chunk from the end of the node.
	  else if (p->end_ <= end + 3)
	    p->end_ = start;
	  // Case 4: remove a chunk from the middle, and split
	  // the node into two.
	  else
	    {
	      Free_list_node newnode(p->start_, start);
	      p->start_ = end;
	      this->list_.insert(p, newnode);
	      ++Free_list::num_nodes;
	    }
	  this->last_remove_ = p;
	  return;
	}
    }

  // Did not find a node containing the given chunk.  This could happen
  // because a small chunk was already removed due to the fuzz.
  gold_debug(DEBUG_INCREMENTAL,
	     "Free_list::remove(%d,%d) not found",
	     static_cast<int>(start), static_cast<int>(end));
}

// Allocate a chunk of size LEN from the free list.  Returns -1ULL
// if a sufficiently large chunk of free space is not found.
// We use a simple first-fit algorithm.

off_t
Free_list::allocate(off_t len, uint64_t align, off_t minoff)
{
  gold_debug(DEBUG_INCREMENTAL,
	     "Free_list::allocate(%08lx, %d, %08lx)",
	     static_cast<long>(len), static_cast<int>(align),
	     static_cast<long>(minoff));
  if (len == 0)
    return align_address(minoff, align);

  ++Free_list::num_allocates;

  // We usually want to drop free chunks smaller than 4 bytes.
  // If we need to guarantee a minimum hole size, though, we need
  // to keep track of all free chunks.
  const int fuzz = this->min_hole_ > 0 ? 0 : 3;

  for (Iterator p = this->list_.begin(); p != this->list_.end(); ++p)
    {
      ++Free_list::num_allocate_visits;
      off_t start = p->start_ > minoff ? p->start_ : minoff;
      start = align_address(start, align);
      off_t end = start + len;
      if (end > p->end_ && p->end_ == this->length_ && this->extend_)
	{
	  this->length_ = end;
	  p->end_ = end;
	}
      if (end == p->end_ || (end <= p->end_ - this->min_hole_))
	{
	  if (p->start_ + fuzz >= start && p->end_ <= end + fuzz)
	    this->list_.erase(p);
	  else if (p->start_ + fuzz >= start)
	    p->start_ = end;
	  else if (p->end_ <= end + fuzz)
	    p->end_ = start;
	  else
	    {
	      Free_list_node newnode(p->start_, start);
	      p->start_ = end;
	      this->list_.insert(p, newnode);
	      ++Free_list::num_nodes;
	    }
	  return start;
	}
    }
  if (this->extend_)
    {
      off_t start = align_address(this->length_, align);
      this->length_ = start + len;
      return start;
    }
  return -1;
}

// Dump the free list (for debugging).
void
Free_list::dump()
{
  gold_info("Free list:\n     start      end   length\n");
  for (Iterator p = this->list_.begin(); p != this->list_.end(); ++p)
    gold_info("  %08lx %08lx %08lx", static_cast<long>(p->start_),
	      static_cast<long>(p->end_),
	      static_cast<long>(p->end_ - p->start_));
}

// Print the statistics for the free lists.
void
Free_list::print_stats()
{
  fprintf(stderr, _("%s: total free lists: %u\n"),
	  program_name, Free_list::num_lists);
  fprintf(stderr, _("%s: total free list nodes: %u\n"),
	  program_name, Free_list::num_nodes);
  fprintf(stderr, _("%s: calls to Free_list::remove: %u\n"),
	  program_name, Free_list::num_removes);
  fprintf(stderr, _("%s: nodes visited: %u\n"),
	  program_name, Free_list::num_remove_visits);
  fprintf(stderr, _("%s: calls to Free_list::allocate: %u\n"),
	  program_name, Free_list::num_allocates);
  fprintf(stderr, _("%s: nodes visited: %u\n"),
	  program_name, Free_list::num_allocate_visits);
}

// A Hash_task computes the MD5 checksum of an array of char.

class Hash_task : public Task
{
 public:
  Hash_task(Output_file* of,
	    size_t offset,
	    size_t size,
	    unsigned char* dst,
	    Task_token* final_blocker)
    : of_(of), offset_(offset), size_(size), dst_(dst),
      final_blocker_(final_blocker)
  { }

  void
  run(Workqueue*)
  {
    const unsigned char* iv =
	this->of_->get_input_view(this->offset_, this->size_);
    md5_buffer(reinterpret_cast<const char*>(iv), this->size_, this->dst_);
    this->of_->free_input_view(this->offset_, this->size_, iv);
  }

  Task_token*
  is_runnable()
  { return NULL; }

  // Unblock FINAL_BLOCKER_ when done.
  void
  locks(Task_locker* tl)
  { tl->add(this, this->final_blocker_); }

  std::string
  get_name() const
  { return "Hash_task"; }

 private:
  Output_file* of_;
  const size_t offset_;
  const size_t size_;
  unsigned char* const dst_;
  Task_token* const final_blocker_;
};

// Layout::Relaxation_debug_check methods.

// Check that sections and special data are in reset states.
// We do not save states for Output_sections and special Output_data.
// So we check that they have not assigned any addresses or offsets.
// clean_up_after_relaxation simply resets their addresses and offsets.
void
Layout::Relaxation_debug_check::check_output_data_for_reset_values(
    const Layout::Section_list& sections,
    const Layout::Data_list& special_outputs,
    const Layout::Data_list& relax_outputs)
{
  for(Layout::Section_list::const_iterator p = sections.begin();
      p != sections.end();
      ++p)
    gold_assert((*p)->address_and_file_offset_have_reset_values());

  for(Layout::Data_list::const_iterator p = special_outputs.begin();
      p != special_outputs.end();
      ++p)
    gold_assert((*p)->address_and_file_offset_have_reset_values());

  gold_assert(relax_outputs.empty());
}

// Save information of SECTIONS for checking later.

void
Layout::Relaxation_debug_check::read_sections(
    const Layout::Section_list& sections)
{
  for(Layout::Section_list::const_iterator p = sections.begin();
      p != sections.end();
      ++p)
    {
      Output_section* os = *p;
      Section_info info;
      info.output_section = os;
      info.address = os->is_address_valid() ? os->address() : 0;
      info.data_size = os->is_data_size_valid() ? os->data_size() : -1;
      info.offset = os->is_offset_valid()? os->offset() : -1 ;
      this->section_infos_.push_back(info);
    }
}

// Verify SECTIONS using previously recorded information.

void
Layout::Relaxation_debug_check::verify_sections(
    const Layout::Section_list& sections)
{
  size_t i = 0;
  for(Layout::Section_list::const_iterator p = sections.begin();
      p != sections.end();
      ++p, ++i)
    {
      Output_section* os = *p;
      uint64_t address = os->is_address_valid() ? os->address() : 0;
      off_t data_size = os->is_data_size_valid() ? os->data_size() : -1;
      off_t offset = os->is_offset_valid()? os->offset() : -1 ;

      if (i >= this->section_infos_.size())
	{
	  gold_fatal("Section_info of %s missing.\n", os->name());
	}
      const Section_info& info = this->section_infos_[i];
      if (os != info.output_section)
	gold_fatal("Section order changed.  Expecting %s but see %s\n",
		   info.output_section->name(), os->name());
      if (address != info.address
	  || data_size != info.data_size
	  || offset != info.offset)
	gold_fatal("Section %s changed.\n", os->name());
    }
}

// Layout_task_runner methods.

// Lay out the sections.  This is called after all the input objects
// have been read.

void
Layout_task_runner::run(Workqueue* workqueue, const Task* task)
{
  // See if any of the input definitions violate the One Definition Rule.
  // TODO: if this is too slow, do this as a task, rather than inline.
  this->symtab_->detect_odr_violations(task, this->options_.output_file_name());

  Layout* layout = this->layout_;
  off_t file_size = layout->finalize(this->input_objects_,
				     this->symtab_,
				     this->target_,
				     task);

  // Now we know the final size of the output file and we know where
  // each piece of information goes.

  if (this->mapfile_ != NULL)
    {
      this->mapfile_->print_discarded_sections(this->input_objects_);
      layout->print_to_mapfile(this->mapfile_);
    }

  Output_file* of;
  if (layout->incremental_base() == NULL)
    {
      of = new Output_file(parameters->options().output_file_name());
      if (this->options_.oformat_enum() != General_options::OBJECT_FORMAT_ELF)
	of->set_is_temporary();
      of->open(file_size);
    }
  else
    {
      of = layout->incremental_base()->output_file();

      // Apply the incremental relocations for symbols whose values
      // have changed.  We do this before we resize the file and start
      // writing anything else to it, so that we can read the old
      // incremental information from the file before (possibly)
      // overwriting it.
      if (parameters->incremental_update())
	layout->incremental_base()->apply_incremental_relocs(this->symtab_,
							     this->layout_,
							     of);

      of->resize(file_size);
    }

  // Queue up the final set of tasks.
  gold::queue_final_tasks(this->options_, this->input_objects_,
			  this->symtab_, layout, workqueue, of);
}

// Layout methods.

Layout::Layout(int number_of_input_files, Script_options* script_options)
  : number_of_input_files_(number_of_input_files),
    script_options_(script_options),
    namepool_(),
    sympool_(),
    dynpool_(),
    signatures_(),
    section_name_map_(),
    segment_list_(),
    section_list_(),
    unattached_section_list_(),
    special_output_list_(),
    relax_output_list_(),
    section_headers_(NULL),
    tls_segment_(NULL),
    relro_segment_(NULL),
    interp_segment_(NULL),
    increase_relro_(0),
    symtab_section_(NULL),
    symtab_xindex_(NULL),
    dynsym_section_(NULL),
    dynsym_xindex_(NULL),
    dynamic_section_(NULL),
    dynamic_symbol_(NULL),
    dynamic_data_(NULL),
    eh_frame_section_(NULL),
    eh_frame_data_(NULL),
    added_eh_frame_data_(false),
    eh_frame_hdr_section_(NULL),
    gdb_index_data_(NULL),
    build_id_note_(NULL),
    debug_abbrev_(NULL),
    debug_info_(NULL),
    group_signatures_(),
    output_file_size_(-1),
    have_added_input_section_(false),
    sections_are_attached_(false),
    input_requires_executable_stack_(false),
    input_with_gnu_stack_note_(false),
    input_without_gnu_stack_note_(false),
    has_static_tls_(false),
    any_postprocessing_sections_(false),
    resized_signatures_(false),
    have_stabstr_section_(false),
    section_ordering_specified_(false),
    unique_segment_for_sections_specified_(false),
    incremental_inputs_(NULL),
    record_output_section_data_from_script_(false),
    lto_slim_object_(false),
    script_output_section_data_list_(),
    segment_states_(NULL),
    relaxation_debug_check_(NULL),
    section_order_map_(),
    section_segment_map_(),
    input_section_position_(),
    input_section_glob_(),
    incremental_base_(NULL),
    free_list_(),
    gnu_properties_()
{
  // Make space for more than enough segments for a typical file.
  // This is just for efficiency--it's OK if we wind up needing more.
  this->segment_list_.reserve(12);

  // We expect two unattached Output_data objects: the file header and
  // the segment headers.
  this->special_output_list_.reserve(2);

  // Initialize structure needed for an incremental build.
  if (parameters->incremental())
    this->incremental_inputs_ = new Incremental_inputs;

  // The section name pool is worth optimizing in all cases, because
  // it is small, but there are often overlaps due to .rel sections.
  this->namepool_.set_optimize();
}

// For incremental links, record the base file to be modified.

void
Layout::set_incremental_base(Incremental_binary* base)
{
  this->incremental_base_ = base;
  this->free_list_.init(base->output_file()->filesize(), true);
}

// Hash a key we use to look up an output section mapping.

size_t
Layout::Hash_key::operator()(const Layout::Key& k) const
{
 return k.first + k.second.first + k.second.second;
}

// These are the debug sections that are actually used by gdb.
// Currently, we've checked versions of gdb up to and including 7.4.
// We only check the part of the name that follows ".debug_" or
// ".zdebug_".

static const char* gdb_sections[] =
{
  "abbrev",
  "addr",         // Fission extension
  // "aranges",   // not used by gdb as of 7.4
  "frame",
  "gdb_scripts",
  "info",
  "types",
  "line",
  "loc",
  "macinfo",
  "macro",
  // "pubnames",  // not used by gdb as of 7.4
  // "pubtypes",  // not used by gdb as of 7.4
  // "gnu_pubnames",  // Fission extension
  // "gnu_pubtypes",  // Fission extension
  "ranges",
  "str",
  "str_offsets",
};

// This is the minimum set of sections needed for line numbers.

static const char* lines_only_debug_sections[] =
{
  "abbrev",
  // "addr",      // Fission extension
  // "aranges",   // not used by gdb as of 7.4
  // "frame",
  // "gdb_scripts",
  "info",
  // "types",
  "line",
  // "loc",
  // "macinfo",
  // "macro",
  // "pubnames",  // not used by gdb as of 7.4
  // "pubtypes",  // not used by gdb as of 7.4
  // "gnu_pubnames",  // Fission extension
  // "gnu_pubtypes",  // Fission extension
  // "ranges",
  "str",
  "str_offsets",  // Fission extension
};

// These sections are the DWARF fast-lookup tables, and are not needed
// when building a .gdb_index section.

static const char* gdb_fast_lookup_sections[] =
{
  "aranges",
  "pubnames",
  "gnu_pubnames",
  "pubtypes",
  "gnu_pubtypes",
};

// Returns whether the given debug section is in the list of
// debug-sections-used-by-some-version-of-gdb.  SUFFIX is the
// portion of the name following ".debug_" or ".zdebug_".

static inline bool
is_gdb_debug_section(const char* suffix)
{
  // We can do this faster: binary search or a hashtable.  But why bother?
  for (size_t i = 0; i < sizeof(gdb_sections)/sizeof(*gdb_sections); ++i)
    if (strcmp(suffix, gdb_sections[i]) == 0)
      return true;
  return false;
}

// Returns whether the given section is needed for lines-only debugging.

static inline bool
is_lines_only_debug_section(const char* suffix)
{
  // We can do this faster: binary search or a hashtable.  But why bother?
  for (size_t i = 0;
       i < sizeof(lines_only_debug_sections)/sizeof(*lines_only_debug_sections);
       ++i)
    if (strcmp(suffix, lines_only_debug_sections[i]) == 0)
      return true;
  return false;
}

// Returns whether the given section is a fast-lookup section that
// will not be needed when building a .gdb_index section.

static inline bool
is_gdb_fast_lookup_section(const char* suffix)
{
  // We can do this faster: binary search or a hashtable.  But why bother?
  for (size_t i = 0;
       i < sizeof(gdb_fast_lookup_sections)/sizeof(*gdb_fast_lookup_sections);
       ++i)
    if (strcmp(suffix, gdb_fast_lookup_sections[i]) == 0)
      return true;
  return false;
}

// Sometimes we compress sections.  This is typically done for
// sections that are not part of normal program execution (such as
// .debug_* sections), and where the readers of these sections know
// how to deal with compressed sections.  This routine doesn't say for
// certain whether we'll compress -- it depends on commandline options
// as well -- just whether this section is a candidate for compression.
// (The Output_compressed_section class decides whether to compress
// a given section, and picks the name of the compressed section.)

static bool
is_compressible_debug_section(const char* secname)
{
  return (is_prefix_of(".debug", secname));
}

// We may see compressed debug sections in input files.  Return TRUE
// if this is the name of a compressed debug section.

bool
is_compressed_debug_section(const char* secname)
{
  return (is_prefix_of(".zdebug", secname));
}

std::string
corresponding_uncompressed_section_name(std::string secname)
{
  gold_assert(secname[0] == '.' && secname[1] == 'z');
  std::string ret(".");
  ret.append(secname, 2, std::string::npos);
  return ret;
}

// Whether to include this section in the link.

template<int size, bool big_endian>
bool
Layout::include_section(Sized_relobj_file<size, big_endian>*, const char* name,
			const elfcpp::Shdr<size, big_endian>& shdr)
{
  if (!parameters->options().relocatable()
      && (shdr.get_sh_flags() & elfcpp::SHF_EXCLUDE))
    return false;

  elfcpp::Elf_Word sh_type = shdr.get_sh_type();

  if ((sh_type >= elfcpp::SHT_LOOS && sh_type <= elfcpp::SHT_HIOS)
      || (sh_type >= elfcpp::SHT_LOPROC && sh_type <= elfcpp::SHT_HIPROC))
    return parameters->target().should_include_section(sh_type);

  switch (sh_type)
    {
    case elfcpp::SHT_NULL:
    case elfcpp::SHT_SYMTAB:
    case elfcpp::SHT_DYNSYM:
    case elfcpp::SHT_HASH:
    case elfcpp::SHT_DYNAMIC:
    case elfcpp::SHT_SYMTAB_SHNDX:
      return false;

    case elfcpp::SHT_STRTAB:
      // Discard the sections which have special meanings in the ELF
      // ABI.  Keep others (e.g., .stabstr).  We could also do this by
      // checking the sh_link fields of the appropriate sections.
      return (strcmp(name, ".dynstr") != 0
	      && strcmp(name, ".strtab") != 0
	      && strcmp(name, ".shstrtab") != 0);

    case elfcpp::SHT_RELA:
    case elfcpp::SHT_REL:
    case elfcpp::SHT_GROUP:
      // If we are emitting relocations these should be handled
      // elsewhere.
      gold_assert(!parameters->options().relocatable());
      return false;

    case elfcpp::SHT_PROGBITS:
      if (parameters->options().strip_debug()
	  && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0)
	{
	  if (is_debug_info_section(name))
	    return false;
	}
      if (parameters->options().strip_debug_non_line()
	  && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0)
	{
	  // Debugging sections can only be recognized by name.
	  if (is_prefix_of(".debug_", name)
	      && !is_lines_only_debug_section(name + 7))
	    return false;
	  if (is_prefix_of(".zdebug_", name)
	      && !is_lines_only_debug_section(name + 8))
	    return false;
	}
      if (parameters->options().strip_debug_gdb()
	  && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0)
	{
	  // Debugging sections can only be recognized by name.
	  if (is_prefix_of(".debug_", name)
	      && !is_gdb_debug_section(name + 7))
	    return false;
	  if (is_prefix_of(".zdebug_", name)
	      && !is_gdb_debug_section(name + 8))
	    return false;
	}
      if (parameters->options().gdb_index()
	  && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0)
	{
	  // When building .gdb_index, we can strip .debug_pubnames,
	  // .debug_pubtypes, and .debug_aranges sections.
	  if (is_prefix_of(".debug_", name)
	      && is_gdb_fast_lookup_section(name + 7))
	    return false;
	  if (is_prefix_of(".zdebug_", name)
	      && is_gdb_fast_lookup_section(name + 8))
	    return false;
	}
      if (parameters->options().strip_lto_sections()
	  && !parameters->options().relocatable()
	  && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0)
	{
	  // Ignore LTO sections containing intermediate code.
	  if (is_prefix_of(".gnu.lto_", name))
	    return false;
	}
      // The GNU linker strips .gnu_debuglink sections, so we do too.
      // This is a feature used to keep debugging information in
      // separate files.
      if (strcmp(name, ".gnu_debuglink") == 0)
	return false;
      return true;

    default:
      return true;
    }
}

// Return an output section named NAME, or NULL if there is none.

Output_section*
Layout::find_output_section(const char* name) const
{
  for (Section_list::const_iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    if (strcmp((*p)->name(), name) == 0)
      return *p;
  return NULL;
}

// Return an output segment of type TYPE, with segment flags SET set
// and segment flags CLEAR clear.  Return NULL if there is none.

Output_segment*
Layout::find_output_segment(elfcpp::PT type, elfcpp::Elf_Word set,
			    elfcpp::Elf_Word clear) const
{
  for (Segment_list::const_iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    if (static_cast<elfcpp::PT>((*p)->type()) == type
	&& ((*p)->flags() & set) == set
	&& ((*p)->flags() & clear) == 0)
      return *p;
  return NULL;
}

// When we put a .ctors or .dtors section with more than one word into
// a .init_array or .fini_array section, we need to reverse the words
// in the .ctors/.dtors section.  This is because .init_array executes
// constructors front to back, where .ctors executes them back to
// front, and vice-versa for .fini_array/.dtors.  Although we do want
// to remap .ctors/.dtors into .init_array/.fini_array because it can
// be more efficient, we don't want to change the order in which
// constructors/destructors are run.  This set just keeps track of
// these sections which need to be reversed.  It is only changed by
// Layout::layout.  It should be a private member of Layout, but that
// would require layout.h to #include object.h to get the definition
// of Section_id.
static Unordered_set<Section_id, Section_id_hash> ctors_sections_in_init_array;

// Return whether OBJECT/SHNDX is a .ctors/.dtors section mapped to a
// .init_array/.fini_array section.

bool
Layout::is_ctors_in_init_array(Relobj* relobj, unsigned int shndx) const
{
  return (ctors_sections_in_init_array.find(Section_id(relobj, shndx))
	  != ctors_sections_in_init_array.end());
}

// Return the output section to use for section NAME with type TYPE
// and section flags FLAGS.  NAME must be canonicalized in the string
// pool, and NAME_KEY is the key.  ORDER is where this should appear
// in the output sections.  IS_RELRO is true for a relro section.

Output_section*
Layout::get_output_section(const char* name, Stringpool::Key name_key,
			   elfcpp::Elf_Word type, elfcpp::Elf_Xword flags,
			   Output_section_order order, bool is_relro)
{
  elfcpp::Elf_Word lookup_type = type;

  // For lookup purposes, treat INIT_ARRAY, FINI_ARRAY, and
  // PREINIT_ARRAY like PROGBITS.  This ensures that we combine
  // .init_array, .fini_array, and .preinit_array sections by name
  // whatever their type in the input file.  We do this because the
  // types are not always right in the input files.
  if (lookup_type == elfcpp::SHT_INIT_ARRAY
      || lookup_type == elfcpp::SHT_FINI_ARRAY
      || lookup_type == elfcpp::SHT_PREINIT_ARRAY)
    lookup_type = elfcpp::SHT_PROGBITS;

  elfcpp::Elf_Xword lookup_flags = flags;

  // Ignoring SHF_WRITE and SHF_EXECINSTR here means that we combine
  // read-write with read-only sections.  Some other ELF linkers do
  // not do this.  FIXME: Perhaps there should be an option
  // controlling this.
  lookup_flags &= ~(elfcpp::SHF_WRITE | elfcpp::SHF_EXECINSTR);

  const Key key(name_key, std::make_pair(lookup_type, lookup_flags));
  const std::pair<Key, Output_section*> v(key, NULL);
  std::pair<Section_name_map::iterator, bool> ins(
    this->section_name_map_.insert(v));

  if (!ins.second)
    return ins.first->second;
  else
    {
      // This is the first time we've seen this name/type/flags
      // combination.  For compatibility with the GNU linker, we
      // combine sections with contents and zero flags with sections
      // with non-zero flags.  This is a workaround for cases where
      // assembler code forgets to set section flags.  FIXME: Perhaps
      // there should be an option to control this.
      Output_section* os = NULL;

      if (lookup_type == elfcpp::SHT_PROGBITS)
	{
	  if (flags == 0)
	    {
	      Output_section* same_name = this->find_output_section(name);
	      if (same_name != NULL
		  && (same_name->type() == elfcpp::SHT_PROGBITS
		      || same_name->type() == elfcpp::SHT_INIT_ARRAY
		      || same_name->type() == elfcpp::SHT_FINI_ARRAY
		      || same_name->type() == elfcpp::SHT_PREINIT_ARRAY)
		  && (same_name->flags() & elfcpp::SHF_TLS) == 0)
		os = same_name;
	    }
	  else if ((flags & elfcpp::SHF_TLS) == 0)
	    {
	      elfcpp::Elf_Xword zero_flags = 0;
	      const Key zero_key(name_key, std::make_pair(lookup_type,
							  zero_flags));
	      Section_name_map::iterator p =
		  this->section_name_map_.find(zero_key);
	      if (p != this->section_name_map_.end())
		os = p->second;
	    }
	}

      if (os == NULL)
	os = this->make_output_section(name, type, flags, order, is_relro);

      ins.first->second = os;
      return os;
    }
}

// Returns TRUE iff NAME (an input section from RELOBJ) will
// be mapped to an output section that should be KEPT.

bool
Layout::keep_input_section(const Relobj* relobj, const char* name)
{
  if (! this->script_options_->saw_sections_clause())
    return false;

  Script_sections* ss = this->script_options_->script_sections();
  const char* file_name = relobj == NULL ? NULL : relobj->name().c_str();
  Output_section** output_section_slot;
  Script_sections::Section_type script_section_type;
  bool keep;

  name = ss->output_section_name(file_name, name, &output_section_slot,
				 &script_section_type, &keep, true);
  return name != NULL && keep;
}

// Clear the input section flags that should not be copied to the
// output section.

elfcpp::Elf_Xword
Layout::get_output_section_flags(elfcpp::Elf_Xword input_section_flags)
{
  // Some flags in the input section should not be automatically
  // copied to the output section.
  input_section_flags &= ~ (elfcpp::SHF_INFO_LINK
			    | elfcpp::SHF_GROUP
			    | elfcpp::SHF_COMPRESSED
			    | elfcpp::SHF_MERGE
			    | elfcpp::SHF_STRINGS);

  // We only clear the SHF_LINK_ORDER flag in for
  // a non-relocatable link.
  if (!parameters->options().relocatable())
    input_section_flags &= ~elfcpp::SHF_LINK_ORDER;

  return input_section_flags;
}

// Pick the output section to use for section NAME, in input file
// RELOBJ, with type TYPE and flags FLAGS.  RELOBJ may be NULL for a
// linker created section.  IS_INPUT_SECTION is true if we are
// choosing an output section for an input section found in a input
// file.  ORDER is where this section should appear in the output
// sections.  IS_RELRO is true for a relro section.  This will return
// NULL if the input section should be discarded.  MATCH_INPUT_SPEC
// is true if the section name should be matched against input specs
// in a linker script.

Output_section*
Layout::choose_output_section(const Relobj* relobj, const char* name,
			      elfcpp::Elf_Word type, elfcpp::Elf_Xword flags,
			      bool is_input_section, Output_section_order order,
			      bool is_relro, bool is_reloc,
			      bool match_input_spec)
{
  // We should not see any input sections after we have attached
  // sections to segments.
  gold_assert(!is_input_section || !this->sections_are_attached_);

  flags = this->get_output_section_flags(flags);

  if (this->script_options_->saw_sections_clause() && !is_reloc)
    {
      // We are using a SECTIONS clause, so the output section is
      // chosen based only on the name.

      Script_sections* ss = this->script_options_->script_sections();
      const char* file_name = relobj == NULL ? NULL : relobj->name().c_str();
      Output_section** output_section_slot;
      Script_sections::Section_type script_section_type;
      const char* orig_name = name;
      bool keep;
      name = ss->output_section_name(file_name, name, &output_section_slot,
				     &script_section_type, &keep,
				     match_input_spec);

      if (name == NULL)
	{
	  gold_debug(DEBUG_SCRIPT, _("Unable to create output section '%s' "
				     "because it is not allowed by the "
				     "SECTIONS clause of the linker script"),
		     orig_name);
	  // The SECTIONS clause says to discard this input section.
	  return NULL;
	}

      // We can only handle script section types ST_NONE and ST_NOLOAD.
      switch (script_section_type)
	{
	case Script_sections::ST_NONE:
	  break;
	case Script_sections::ST_NOLOAD:
	  flags &= elfcpp::SHF_ALLOC;
	  break;
	default:
	  gold_unreachable();
	}

      // If this is an orphan section--one not mentioned in the linker
      // script--then OUTPUT_SECTION_SLOT will be NULL, and we do the
      // default processing below.

      if (output_section_slot != NULL)
	{
	  if (*output_section_slot != NULL)
	    {
	      (*output_section_slot)->update_flags_for_input_section(flags);
	      return *output_section_slot;
	    }

	  // We don't put sections found in the linker script into
	  // SECTION_NAME_MAP_.  That keeps us from getting confused
	  // if an orphan section is mapped to a section with the same
	  // name as one in the linker script.

	  name = this->namepool_.add(name, false, NULL);

	  Output_section* os = this->make_output_section(name, type, flags,
							 order, is_relro);

	  os->set_found_in_sections_clause();

	  // Special handling for NOLOAD sections.
	  if (script_section_type == Script_sections::ST_NOLOAD)
	    {
	      os->set_is_noload();

	      // The constructor of Output_section sets addresses of non-ALLOC
	      // sections to 0 by default.  We don't want that for NOLOAD
	      // sections even if they have no SHF_ALLOC flag.
	      if ((os->flags() & elfcpp::SHF_ALLOC) == 0
		  && os->is_address_valid())
		{
		  gold_assert(os->address() == 0
			      && !os->is_offset_valid()
			      && !os->is_data_size_valid());
		  os->reset_address_and_file_offset();
		}
	    }

	  *output_section_slot = os;
	  return os;
	}
    }

  // FIXME: Handle SHF_OS_NONCONFORMING somewhere.

  size_t len = strlen(name);
  std::string uncompressed_name;

  // Compressed debug sections should be mapped to the corresponding
  // uncompressed section.
  if (is_compressed_debug_section(name))
    {
      uncompressed_name =
	  corresponding_uncompressed_section_name(std::string(name, len));
      name = uncompressed_name.c_str();
      len = uncompressed_name.length();
    }

  // Turn NAME from the name of the input section into the name of the
  // output section.
  if (is_input_section
      && !this->script_options_->saw_sections_clause()
      && !parameters->options().relocatable())
    {
      const char *orig_name = name;
      name = parameters->target().output_section_name(relobj, name, &len);
      if (name == NULL)
	name = Layout::output_section_name(relobj, orig_name, &len);
    }

  Stringpool::Key name_key;
  name = this->namepool_.add_with_length(name, len, true, &name_key);

  // Find or make the output section.  The output section is selected
  // based on the section name, type, and flags.
  return this->get_output_section(name, name_key, type, flags, order, is_relro);
}

// For incremental links, record the initial fixed layout of a section
// from the base file, and return a pointer to the Output_section.

template<int size, bool big_endian>
Output_section*
Layout::init_fixed_output_section(const char* name,
				  elfcpp::Shdr<size, big_endian>& shdr)
{
  unsigned int sh_type = shdr.get_sh_type();

  // We preserve the layout of PROGBITS, NOBITS, INIT_ARRAY, FINI_ARRAY,
  // PRE_INIT_ARRAY, and NOTE sections.
  // All others will be created from scratch and reallocated.
  if (!can_incremental_update(sh_type))
    return NULL;

  // If we're generating a .gdb_index section, we need to regenerate
  // it from scratch.
  if (parameters->options().gdb_index()
      && sh_type == elfcpp::SHT_PROGBITS
      && strcmp(name, ".gdb_index") == 0)
    return NULL;

  typename elfcpp::Elf_types<size>::Elf_Addr sh_addr = shdr.get_sh_addr();
  typename elfcpp::Elf_types<size>::Elf_Off sh_offset = shdr.get_sh_offset();
  typename elfcpp::Elf_types<size>::Elf_WXword sh_size = shdr.get_sh_size();
  typename elfcpp::Elf_types<size>::Elf_WXword sh_flags =
      this->get_output_section_flags(shdr.get_sh_flags());
  typename elfcpp::Elf_types<size>::Elf_WXword sh_addralign =
      shdr.get_sh_addralign();

  // Make the output section.
  Stringpool::Key name_key;
  name = this->namepool_.add(name, true, &name_key);
  Output_section* os = this->get_output_section(name, name_key, sh_type,
						sh_flags, ORDER_INVALID, false);
  os->set_fixed_layout(sh_addr, sh_offset, sh_size, sh_addralign);
  if (sh_type != elfcpp::SHT_NOBITS)
    this->free_list_.remove(sh_offset, sh_offset + sh_size);
  return os;
}

// Return the index by which an input section should be ordered.  This
// is used to sort some .text sections, for compatibility with GNU ld.

int
Layout::special_ordering_of_input_section(const char* name)
{
  // The GNU linker has some special handling for some sections that
  // wind up in the .text section.  Sections that start with these
  // prefixes must appear first, and must appear in the order listed
  // here.
  static const char* const text_section_sort[] =
  {
    ".text.unlikely",
    ".text.exit",
    ".text.startup",
    ".text.hot",
    ".text.sorted"
  };

  for (size_t i = 0;
       i < sizeof(text_section_sort) / sizeof(text_section_sort[0]);
       i++)
    if (is_prefix_of(text_section_sort[i], name))
      return i;

  return -1;
}

// Return the output section to use for input section SHNDX, with name
// NAME, with header HEADER, from object OBJECT.  RELOC_SHNDX is the
// index of a relocation section which applies to this section, or 0
// if none, or -1U if more than one.  RELOC_TYPE is the type of the
// relocation section if there is one.  Set *OFF to the offset of this
// input section without the output section.  Return NULL if the
// section should be discarded.  Set *OFF to -1 if the section
// contents should not be written directly to the output file, but
// will instead receive special handling.

template<int size, bool big_endian>
Output_section*
Layout::layout(Sized_relobj_file<size, big_endian>* object, unsigned int shndx,
	       const char* name, const elfcpp::Shdr<size, big_endian>& shdr,
	       unsigned int sh_type, unsigned int reloc_shndx,
	       unsigned int, off_t* off)
{
  *off = 0;

  if (!this->include_section(object, name, shdr))
    return NULL;

  // In a relocatable link a grouped section must not be combined with
  // any other sections.
  Output_section* os;
  if (parameters->options().relocatable()
      && (shdr.get_sh_flags() & elfcpp::SHF_GROUP) != 0)
    {
      // Some flags in the input section should not be automatically
      // copied to the output section.
      elfcpp::Elf_Xword sh_flags = (shdr.get_sh_flags()
				    & ~ elfcpp::SHF_COMPRESSED);
      name = this->namepool_.add(name, true, NULL);
      os = this->make_output_section(name, sh_type, sh_flags, ORDER_INVALID,
				     false);
    }
  else
    {
      // Get the section flags and mask out any flags that do not
      // take part in section matching.
      elfcpp::Elf_Xword sh_flags
	  = (this->get_output_section_flags(shdr.get_sh_flags())
	     & ~object->osabi().ignored_sh_flags());

      // All ".text.unlikely.*" sections can be moved to a unique
      // segment with --text-unlikely-segment option.
      bool text_unlikely_segment
	  = (parameters->options().text_unlikely_segment()
	     && is_prefix_of(".text.unlikely",
			     object->section_name(shndx).c_str()));
      if (text_unlikely_segment)
	{
	  Stringpool::Key name_key;
	  const char* os_name = this->namepool_.add(".text.unlikely", true,
						    &name_key);
	  os = this->get_output_section(os_name, name_key, sh_type, sh_flags,
					ORDER_INVALID, false);
	  // Map this output section to a unique segment.  This is done to
	  // separate "text" that is not likely to be executed from "text"
	  // that is likely executed.
	  os->set_is_unique_segment();
	}
      else
	{
	  // Plugins can choose to place one or more subsets of sections in
	  // unique segments and this is done by mapping these section subsets
	  // to unique output sections.  Check if this section needs to be
	  // remapped to a unique output section.
	  Section_segment_map::iterator it
	    = this->section_segment_map_.find(Const_section_id(object, shndx));
	  if (it == this->section_segment_map_.end())
	    {
	      os = this->choose_output_section(object, name, sh_type,
					       sh_flags, true, ORDER_INVALID,
					       false, false, true);
	    }
	  else
	    {
	      // We know the name of the output section, directly call
	      // get_output_section here by-passing choose_output_section.
	      const char* os_name = it->second->name;
	      Stringpool::Key name_key;
	      os_name = this->namepool_.add(os_name, true, &name_key);
	      os = this->get_output_section(os_name, name_key, sh_type,
					    sh_flags, ORDER_INVALID, false);
	      if (!os->is_unique_segment())
		{
		  os->set_is_unique_segment();
		  os->set_extra_segment_flags(it->second->flags);
		  os->set_segment_alignment(it->second->align);
		}
	    }
	  }
      if (os == NULL)
	return NULL;
    }

  // By default the GNU linker sorts input sections whose names match
  // .ctors.*, .dtors.*, .init_array.*, or .fini_array.*.  The
  // sections are sorted by name.  This is used to implement
  // constructor priority ordering.  We are compatible.  When we put
  // .ctor sections in .init_array and .dtor sections in .fini_array,
  // we must also sort plain .ctor and .dtor sections.
  if (!this->script_options_->saw_sections_clause()
      && !parameters->options().relocatable()
      && (is_prefix_of(".ctors.", name)
	  || is_prefix_of(".dtors.", name)
	  || is_prefix_of(".init_array.", name)
	  || is_prefix_of(".fini_array.", name)
	  || (parameters->options().ctors_in_init_array()
	      && (strcmp(name, ".ctors") == 0
		  || strcmp(name, ".dtors") == 0))))
    os->set_must_sort_attached_input_sections();

  // By default the GNU linker sorts some special text sections ahead
  // of others.  We are compatible.
  if (parameters->options().text_reorder()
      && !this->script_options_->saw_sections_clause()
      && !this->is_section_ordering_specified()
      && !parameters->options().relocatable()
      && Layout::special_ordering_of_input_section(name) >= 0)
    os->set_must_sort_attached_input_sections();

  // If this is a .ctors or .ctors.* section being mapped to a
  // .init_array section, or a .dtors or .dtors.* section being mapped
  // to a .fini_array section, we will need to reverse the words if
  // there is more than one.  Record this section for later.  See
  // ctors_sections_in_init_array above.
  if (!this->script_options_->saw_sections_clause()
      && !parameters->options().relocatable()
      && shdr.get_sh_size() > size / 8
      && (((strcmp(name, ".ctors") == 0
	    || is_prefix_of(".ctors.", name))
	   && strcmp(os->name(), ".init_array") == 0)
	  || ((strcmp(name, ".dtors") == 0
	       || is_prefix_of(".dtors.", name))
	      && strcmp(os->name(), ".fini_array") == 0)))
    ctors_sections_in_init_array.insert(Section_id(object, shndx));

  // FIXME: Handle SHF_LINK_ORDER somewhere.

  elfcpp::Elf_Xword orig_flags = os->flags();

  *off = os->add_input_section(this, object, shndx, name, shdr, reloc_shndx,
			       this->script_options_->saw_sections_clause());

  // If the flags changed, we may have to change the order.
  if ((orig_flags & elfcpp::SHF_ALLOC) != 0)
    {
      orig_flags &= (elfcpp::SHF_WRITE | elfcpp::SHF_EXECINSTR);
      elfcpp::Elf_Xword new_flags =
	os->flags() & (elfcpp::SHF_WRITE | elfcpp::SHF_EXECINSTR);
      if (orig_flags != new_flags)
	os->set_order(this->default_section_order(os, false));
    }

  this->have_added_input_section_ = true;

  return os;
}

// Maps section SECN to SEGMENT s.
void
Layout::insert_section_segment_map(Const_section_id secn,
				   Unique_segment_info *s)
{
  gold_assert(this->unique_segment_for_sections_specified_);
  this->section_segment_map_[secn] = s;
}

// Handle a relocation section when doing a relocatable link.

template<int size, bool big_endian>
Output_section*
Layout::layout_reloc(Sized_relobj_file<size, big_endian>*,
		     unsigned int,
		     const elfcpp::Shdr<size, big_endian>& shdr,
		     Output_section* data_section,
		     Relocatable_relocs* rr)
{
  gold_assert(parameters->options().relocatable()
	      || parameters->options().emit_relocs());

  int sh_type = shdr.get_sh_type();

  std::string name;
  if (sh_type == elfcpp::SHT_REL)
    name = ".rel";
  else if (sh_type == elfcpp::SHT_RELA)
    name = ".rela";
  else
    gold_unreachable();
  name += data_section->name();

  // If the output data section already has a reloc section, use that;
  // otherwise, make a new one.
  Output_section* os = data_section->reloc_section();
  if (os == NULL)
    {
      const char* n = this->namepool_.add(name.c_str(), true, NULL);
      os = this->make_output_section(n, sh_type, shdr.get_sh_flags(),
				     ORDER_INVALID, false);
      os->set_should_link_to_symtab();
      os->set_info_section(data_section);
      data_section->set_reloc_section(os);
    }

  Output_section_data* posd;
  if (sh_type == elfcpp::SHT_REL)
    {
      os->set_entsize(elfcpp::Elf_sizes<size>::rel_size);
      posd = new Output_relocatable_relocs<elfcpp::SHT_REL,
					   size,
					   big_endian>(rr);
    }
  else if (sh_type == elfcpp::SHT_RELA)
    {
      os->set_entsize(elfcpp::Elf_sizes<size>::rela_size);
      posd = new Output_relocatable_relocs<elfcpp::SHT_RELA,
					   size,
					   big_endian>(rr);
    }
  else
    gold_unreachable();

  os->add_output_section_data(posd);
  rr->set_output_data(posd);

  return os;
}

// Handle a group section when doing a relocatable link.

template<int size, bool big_endian>
void
Layout::layout_group(Symbol_table* symtab,
		     Sized_relobj_file<size, big_endian>* object,
		     unsigned int,
		     const char* group_section_name,
		     const char* signature,
		     const elfcpp::Shdr<size, big_endian>& shdr,
		     elfcpp::Elf_Word flags,
		     std::vector<unsigned int>* shndxes)
{
  gold_assert(parameters->options().relocatable());
  gold_assert(shdr.get_sh_type() == elfcpp::SHT_GROUP);
  group_section_name = this->namepool_.add(group_section_name, true, NULL);
  Output_section* os = this->make_output_section(group_section_name,
						 elfcpp::SHT_GROUP,
						 shdr.get_sh_flags(),
						 ORDER_INVALID, false);

  // We need to find a symbol with the signature in the symbol table.
  // If we don't find one now, we need to look again later.
  Symbol* sym = symtab->lookup(signature, NULL);
  if (sym != NULL)
    os->set_info_symndx(sym);
  else
    {
      // Reserve some space to minimize reallocations.
      if (this->group_signatures_.empty())
	this->group_signatures_.reserve(this->number_of_input_files_ * 16);

      // We will wind up using a symbol whose name is the signature.
      // So just put the signature in the symbol name pool to save it.
      signature = symtab->canonicalize_name(signature);
      this->group_signatures_.push_back(Group_signature(os, signature));
    }

  os->set_should_link_to_symtab();
  os->set_entsize(4);

  section_size_type entry_count =
    convert_to_section_size_type(shdr.get_sh_size() / 4);
  Output_section_data* posd =
    new Output_data_group<size, big_endian>(object, entry_count, flags,
					    shndxes);
  os->add_output_section_data(posd);
}

// Special GNU handling of sections name .eh_frame.  They will
// normally hold exception frame data as defined by the C++ ABI
// (http://codesourcery.com/cxx-abi/).

template<int size, bool big_endian>
Output_section*
Layout::layout_eh_frame(Sized_relobj_file<size, big_endian>* object,
			const unsigned char* symbols,
			off_t symbols_size,
			const unsigned char* symbol_names,
			off_t symbol_names_size,
			unsigned int shndx,
			const elfcpp::Shdr<size, big_endian>& shdr,
			unsigned int reloc_shndx, unsigned int reloc_type,
			off_t* off)
{
  const unsigned int unwind_section_type =
      parameters->target().unwind_section_type();

  gold_assert(shdr.get_sh_type() == elfcpp::SHT_PROGBITS
	      || shdr.get_sh_type() == unwind_section_type);
  gold_assert((shdr.get_sh_flags() & elfcpp::SHF_ALLOC) != 0);

  Output_section* os = this->make_eh_frame_section(object);
  if (os == NULL)
    return NULL;

  gold_assert(this->eh_frame_section_ == os);

  elfcpp::Elf_Xword orig_flags = os->flags();

  Eh_frame::Eh_frame_section_disposition disp =
      Eh_frame::EH_UNRECOGNIZED_SECTION;
  if (!parameters->incremental())
    {
      disp = this->eh_frame_data_->add_ehframe_input_section(object,
							     symbols,
							     symbols_size,
							     symbol_names,
							     symbol_names_size,
							     shndx,
							     reloc_shndx,
							     reloc_type);
    }

  if (disp == Eh_frame::EH_OPTIMIZABLE_SECTION)
    {
      os->update_flags_for_input_section(shdr.get_sh_flags());

      // A writable .eh_frame section is a RELRO section.
      if ((orig_flags & (elfcpp::SHF_WRITE | elfcpp::SHF_EXECINSTR))
	  != (os->flags() & (elfcpp::SHF_WRITE | elfcpp::SHF_EXECINSTR)))
	{
	  os->set_is_relro();
	  os->set_order(ORDER_RELRO);
	}

      *off = -1;
      return os;
    }

  if (disp == Eh_frame::EH_END_MARKER_SECTION && !this->added_eh_frame_data_)
    {
      // We found the end marker section, so now we can add the set of
      // optimized sections to the output section.  We need to postpone
      // adding this until we've found a section we can optimize so that
      // the .eh_frame section in crtbeginT.o winds up at the start of
      // the output section.
      os->add_output_section_data(this->eh_frame_data_);
      this->added_eh_frame_data_ = true;
     }

  // We couldn't handle this .eh_frame section for some reason.
  // Add it as a normal section.
  bool saw_sections_clause = this->script_options_->saw_sections_clause();
  *off = os->add_input_section(this, object, shndx, ".eh_frame", shdr,
			       reloc_shndx, saw_sections_clause);
  this->have_added_input_section_ = true;

  if ((orig_flags & (elfcpp::SHF_WRITE | elfcpp::SHF_EXECINSTR))
      != (os->flags() & (elfcpp::SHF_WRITE | elfcpp::SHF_EXECINSTR)))
    os->set_order(this->default_section_order(os, false));

  return os;
}

void
Layout::finalize_eh_frame_section()
{
  // If we never found an end marker section, we need to add the
  // optimized eh sections to the output section now.
  if (!parameters->incremental()
      && this->eh_frame_section_ != NULL
      && !this->added_eh_frame_data_)
    {
      this->eh_frame_section_->add_output_section_data(this->eh_frame_data_);
      this->added_eh_frame_data_ = true;
    }
}

// Create and return the magic .eh_frame section.  Create
// .eh_frame_hdr also if appropriate.  OBJECT is the object with the
// input .eh_frame section; it may be NULL.

Output_section*
Layout::make_eh_frame_section(const Relobj* object)
{
  const unsigned int unwind_section_type =
      parameters->target().unwind_section_type();

  Output_section* os = this->choose_output_section(object, ".eh_frame",
						   unwind_section_type,
						   elfcpp::SHF_ALLOC, false,
						   ORDER_EHFRAME, false, false,
						   false);
  if (os == NULL)
    return NULL;

  if (this->eh_frame_section_ == NULL)
    {
      this->eh_frame_section_ = os;
      this->eh_frame_data_ = new Eh_frame();

      // For incremental linking, we do not optimize .eh_frame sections
      // or create a .eh_frame_hdr section.
      if (parameters->options().eh_frame_hdr() && !parameters->incremental())
	{
	  Output_section* hdr_os =
	    this->choose_output_section(NULL, ".eh_frame_hdr",
					unwind_section_type,
					elfcpp::SHF_ALLOC, false,
					ORDER_EHFRAME, false, false,
					false);

	  if (hdr_os != NULL)
	    {
	      Eh_frame_hdr* hdr_posd = new Eh_frame_hdr(os,
							this->eh_frame_data_);
	      hdr_os->add_output_section_data(hdr_posd);

	      hdr_os->set_after_input_sections();

	      if (!this->script_options_->saw_phdrs_clause())
		{
		  Output_segment* hdr_oseg;
		  hdr_oseg = this->make_output_segment(elfcpp::PT_GNU_EH_FRAME,
						       elfcpp::PF_R);
		  hdr_oseg->add_output_section_to_nonload(hdr_os,
							  elfcpp::PF_R);
		}

	      this->eh_frame_data_->set_eh_frame_hdr(hdr_posd);
	    }
	}
    }

  return os;
}

// Add an exception frame for a PLT.  This is called from target code.

void
Layout::add_eh_frame_for_plt(Output_data* plt, const unsigned char* cie_data,
			     size_t cie_length, const unsigned char* fde_data,
			     size_t fde_length)
{
  if (parameters->incremental())
    {
      // FIXME: Maybe this could work some day....
      return;
    }
  Output_section* os = this->make_eh_frame_section(NULL);
  if (os == NULL)
    return;
  this->eh_frame_data_->add_ehframe_for_plt(plt, cie_data, cie_length,
					    fde_data, fde_length);
  if (!this->added_eh_frame_data_)
    {
      os->add_output_section_data(this->eh_frame_data_);
      this->added_eh_frame_data_ = true;
    }
}

// Remove all post-map .eh_frame information for a PLT.

void
Layout::remove_eh_frame_for_plt(Output_data* plt, const unsigned char* cie_data,
				size_t cie_length)
{
  if (parameters->incremental())
    {
      // FIXME: Maybe this could work some day....
      return;
    }
  this->eh_frame_data_->remove_ehframe_for_plt(plt, cie_data, cie_length);
}

// Scan a .debug_info or .debug_types section, and add summary
// information to the .gdb_index section.

template<int size, bool big_endian>
void
Layout::add_to_gdb_index(bool is_type_unit,
			 Sized_relobj<size, big_endian>* object,
			 const unsigned char* symbols,
			 off_t symbols_size,
			 unsigned int shndx,
			 unsigned int reloc_shndx,
			 unsigned int reloc_type)
{
  if (this->gdb_index_data_ == NULL)
    {
      Output_section* os = this->choose_output_section(NULL, ".gdb_index",
						       elfcpp::SHT_PROGBITS, 0,
						       false, ORDER_INVALID,
						       false, false, false);
      if (os == NULL)
	return;

      this->gdb_index_data_ = new Gdb_index(os);
      os->add_output_section_data(this->gdb_index_data_);
      os->set_after_input_sections();
    }

  this->gdb_index_data_->scan_debug_info(is_type_unit, object, symbols,
					 symbols_size, shndx, reloc_shndx,
					 reloc_type);
}

// Add POSD to an output section using NAME, TYPE, and FLAGS.  Return
// the output section.

Output_section*
Layout::add_output_section_data(const char* name, elfcpp::Elf_Word type,
				elfcpp::Elf_Xword flags,
				Output_section_data* posd,
				Output_section_order order, bool is_relro)
{
  Output_section* os = this->choose_output_section(NULL, name, type, flags,
						   false, order, is_relro,
						   false, false);
  if (os != NULL)
    os->add_output_section_data(posd);
  return os;
}

// Map section flags to segment flags.

elfcpp::Elf_Word
Layout::section_flags_to_segment(elfcpp::Elf_Xword flags)
{
  elfcpp::Elf_Word ret = elfcpp::PF_R;
  if ((flags & elfcpp::SHF_WRITE) != 0)
    ret |= elfcpp::PF_W;
  if ((flags & elfcpp::SHF_EXECINSTR) != 0)
    ret |= elfcpp::PF_X;
  return ret;
}

// Make a new Output_section, and attach it to segments as
// appropriate.  ORDER is the order in which this section should
// appear in the output segment.  IS_RELRO is true if this is a relro
// (read-only after relocations) section.

Output_section*
Layout::make_output_section(const char* name, elfcpp::Elf_Word type,
			    elfcpp::Elf_Xword flags,
			    Output_section_order order, bool is_relro)
{
  Output_section* os;
  if ((flags & elfcpp::SHF_ALLOC) == 0
      && strcmp(parameters->options().compress_debug_sections(), "none") != 0
      && is_compressible_debug_section(name))
    os = new Output_compressed_section(&parameters->options(), name, type,
				       flags);
  else if ((flags & elfcpp::SHF_ALLOC) == 0
	   && parameters->options().strip_debug_non_line()
	   && strcmp(".debug_abbrev", name) == 0)
    {
      os = this->debug_abbrev_ = new Output_reduced_debug_abbrev_section(
	  name, type, flags);
      if (this->debug_info_)
	this->debug_info_->set_abbreviations(this->debug_abbrev_);
    }
  else if ((flags & elfcpp::SHF_ALLOC) == 0
	   && parameters->options().strip_debug_non_line()
	   && strcmp(".debug_info", name) == 0)
    {
      os = this->debug_info_ = new Output_reduced_debug_info_section(
	  name, type, flags);
      if (this->debug_abbrev_)
	this->debug_info_->set_abbreviations(this->debug_abbrev_);
    }
  else
    {
      // Sometimes .init_array*, .preinit_array* and .fini_array* do
      // not have correct section types.  Force them here.
      if (type == elfcpp::SHT_PROGBITS)
	{
	  if (is_prefix_of(".init_array", name))
	    type = elfcpp::SHT_INIT_ARRAY;
	  else if (is_prefix_of(".preinit_array", name))
	    type = elfcpp::SHT_PREINIT_ARRAY;
	  else if (is_prefix_of(".fini_array", name))
	    type = elfcpp::SHT_FINI_ARRAY;
	}

      // FIXME: const_cast is ugly.
      Target* target = const_cast<Target*>(&parameters->target());
      os = target->make_output_section(name, type, flags);
    }

  // With -z relro, we have to recognize the special sections by name.
  // There is no other way.
  bool is_relro_local = false;
  if (!this->script_options_->saw_sections_clause()
      && parameters->options().relro()
      && (flags & elfcpp::SHF_ALLOC) != 0
      && (flags & elfcpp::SHF_WRITE) != 0)
    {
      if (type == elfcpp::SHT_PROGBITS)
	{
	  if ((flags & elfcpp::SHF_TLS) != 0)
	    is_relro = true;
	  else if (strcmp(name, ".data.rel.ro") == 0)
	    is_relro = true;
	  else if (strcmp(name, ".data.rel.ro.local") == 0)
	    {
	      is_relro = true;
	      is_relro_local = true;
	    }
	  else if (strcmp(name, ".ctors") == 0
		   || strcmp(name, ".dtors") == 0
		   || strcmp(name, ".jcr") == 0)
	    is_relro = true;
	}
      else if (type == elfcpp::SHT_INIT_ARRAY
	       || type == elfcpp::SHT_FINI_ARRAY
	       || type == elfcpp::SHT_PREINIT_ARRAY)
	is_relro = true;
    }

  if (is_relro)
    os->set_is_relro();

  if (order == ORDER_INVALID && (flags & elfcpp::SHF_ALLOC) != 0)
    order = this->default_section_order(os, is_relro_local);

  os->set_order(order);

  parameters->target().new_output_section(os);

  this->section_list_.push_back(os);

  // The GNU linker by default sorts some sections by priority, so we
  // do the same.  We need to know that this might happen before we
  // attach any input sections.
  if (!this->script_options_->saw_sections_clause()
      && !parameters->options().relocatable()
      && (strcmp(name, ".init_array") == 0
	  || strcmp(name, ".fini_array") == 0
	  || (!parameters->options().ctors_in_init_array()
	      && (strcmp(name, ".ctors") == 0
		  || strcmp(name, ".dtors") == 0))))
    os->set_may_sort_attached_input_sections();

  // The GNU linker by default sorts .text.{unlikely,exit,startup,hot}
  // sections before other .text sections.  We are compatible.  We
  // need to know that this might happen before we attach any input
  // sections.
  if (parameters->options().text_reorder()
      && !this->script_options_->saw_sections_clause()
      && !this->is_section_ordering_specified()
      && !parameters->options().relocatable()
      && strcmp(name, ".text") == 0)
    os->set_may_sort_attached_input_sections();

  // GNU linker sorts section by name with --sort-section=name.
  if (strcmp(parameters->options().sort_section(), "name") == 0)
      os->set_must_sort_attached_input_sections();

  // Check for .stab*str sections, as .stab* sections need to link to
  // them.
  if (type == elfcpp::SHT_STRTAB
      && !this->have_stabstr_section_
      && strncmp(name, ".stab", 5) == 0
      && strcmp(name + strlen(name) - 3, "str") == 0)
    this->have_stabstr_section_ = true;

  // During a full incremental link, we add patch space to most
  // PROGBITS and NOBITS sections.  Flag those that may be
  // arbitrarily padded.
  if ((type == elfcpp::SHT_PROGBITS || type == elfcpp::SHT_NOBITS)
      && order != ORDER_INTERP
      && order != ORDER_INIT
      && order != ORDER_PLT
      && order != ORDER_FINI
      && order != ORDER_RELRO_LAST
      && order != ORDER_NON_RELRO_FIRST
      && strcmp(name, ".eh_frame") != 0
      && strcmp(name, ".ctors") != 0
      && strcmp(name, ".dtors") != 0
      && strcmp(name, ".jcr") != 0)
    {
      os->set_is_patch_space_allowed();

      // Certain sections require "holes" to be filled with
      // specific fill patterns.  These fill patterns may have
      // a minimum size, so we must prevent allocations from the
      // free list that leave a hole smaller than the minimum.
      if (strcmp(name, ".debug_info") == 0)
	os->set_free_space_fill(new Output_fill_debug_info(false));
      else if (strcmp(name, ".debug_types") == 0)
	os->set_free_space_fill(new Output_fill_debug_info(true));
      else if (strcmp(name, ".debug_line") == 0)
	os->set_free_space_fill(new Output_fill_debug_line());
    }

  // If we have already attached the sections to segments, then we
  // need to attach this one now.  This happens for sections created
  // directly by the linker.
  if (this->sections_are_attached_)
    this->attach_section_to_segment(&parameters->target(), os);

  return os;
}

// Return the default order in which a section should be placed in an
// output segment.  This function captures a lot of the ideas in
// ld/scripttempl/elf.sc in the GNU linker.  Note that the order of a
// linker created section is normally set when the section is created;
// this function is used for input sections.

Output_section_order
Layout::default_section_order(Output_section* os, bool is_relro_local)
{
  gold_assert((os->flags() & elfcpp::SHF_ALLOC) != 0);
  bool is_write = (os->flags() & elfcpp::SHF_WRITE) != 0;
  bool is_execinstr = (os->flags() & elfcpp::SHF_EXECINSTR) != 0;
  bool is_bss = false;

  switch (os->type())
    {
    default:
    case elfcpp::SHT_PROGBITS:
      break;
    case elfcpp::SHT_NOBITS:
      is_bss = true;
      break;
    case elfcpp::SHT_RELA:
    case elfcpp::SHT_REL:
      if (!is_write)
	return ORDER_DYNAMIC_RELOCS;
      break;
    case elfcpp::SHT_HASH:
    case elfcpp::SHT_DYNAMIC:
    case elfcpp::SHT_SHLIB:
    case elfcpp::SHT_DYNSYM:
    case elfcpp::SHT_GNU_HASH:
    case elfcpp::SHT_GNU_verdef:
    case elfcpp::SHT_GNU_verneed:
    case elfcpp::SHT_GNU_versym:
      if (!is_write)
	return ORDER_DYNAMIC_LINKER;
      break;
    case elfcpp::SHT_NOTE:
      return is_write ? ORDER_RW_NOTE : ORDER_RO_NOTE;
    }

  if ((os->flags() & elfcpp::SHF_TLS) != 0)
    return is_bss ? ORDER_TLS_BSS : ORDER_TLS_DATA;

  if (!is_bss && !is_write)
    {
      if (is_execinstr)
	{
	  if (strcmp(os->name(), ".init") == 0)
	    return ORDER_INIT;
	  else if (strcmp(os->name(), ".fini") == 0)
	    return ORDER_FINI;
	  else if (parameters->options().keep_text_section_prefix())
	    {
	      // -z,keep-text-section-prefix introduces additional
	      // output sections.
	      if (strcmp(os->name(), ".text.hot") == 0)
		return ORDER_TEXT_HOT;
	      else if (strcmp(os->name(), ".text.startup") == 0)
		return ORDER_TEXT_STARTUP;
	      else if (strcmp(os->name(), ".text.exit") == 0)
		return ORDER_TEXT_EXIT;
	      else if (strcmp(os->name(), ".text.unlikely") == 0)
		return ORDER_TEXT_UNLIKELY;
	    }
	}
      return is_execinstr ? ORDER_TEXT : ORDER_READONLY;
    }

  if (os->is_relro())
    return is_relro_local ? ORDER_RELRO_LOCAL : ORDER_RELRO;

  if (os->is_small_section())
    return is_bss ? ORDER_SMALL_BSS : ORDER_SMALL_DATA;
  if (os->is_large_section())
    return is_bss ? ORDER_LARGE_BSS : ORDER_LARGE_DATA;

  return is_bss ? ORDER_BSS : ORDER_DATA;
}

// Attach output sections to segments.  This is called after we have
// seen all the input sections.

void
Layout::attach_sections_to_segments(const Target* target)
{
  for (Section_list::iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    this->attach_section_to_segment(target, *p);

  this->sections_are_attached_ = true;
}

// Attach an output section to a segment.

void
Layout::attach_section_to_segment(const Target* target, Output_section* os)
{
  if ((os->flags() & elfcpp::SHF_ALLOC) == 0)
    this->unattached_section_list_.push_back(os);
  else
    this->attach_allocated_section_to_segment(target, os);
}

// Attach an allocated output section to a segment.

void
Layout::attach_allocated_section_to_segment(const Target* target,
					    Output_section* os)
{
  elfcpp::Elf_Xword flags = os->flags();
  gold_assert((flags & elfcpp::SHF_ALLOC) != 0);

  if (parameters->options().relocatable())
    return;

  // If we have a SECTIONS clause, we can't handle the attachment to
  // segments until after we've seen all the sections.
  if (this->script_options_->saw_sections_clause())
    return;

  gold_assert(!this->script_options_->saw_phdrs_clause());

  // This output section goes into a PT_LOAD segment.

  elfcpp::Elf_Word seg_flags = Layout::section_flags_to_segment(flags);

  // If this output section's segment has extra flags that need to be set,
  // coming from a linker plugin, do that.
  seg_flags |= os->extra_segment_flags();

  // Check for --section-start.
  uint64_t addr;
  bool is_address_set = parameters->options().section_start(os->name(), &addr);

  // In general the only thing we really care about for PT_LOAD
  // segments is whether or not they are writable or executable,
  // so that is how we search for them.
  // Large data sections also go into their own PT_LOAD segment.
  // People who need segments sorted on some other basis will
  // have to use a linker script.

  Segment_list::const_iterator p;
  if (!os->is_unique_segment())
    {
      for (p = this->segment_list_.begin();
	   p != this->segment_list_.end();
	   ++p)
	{
	  if ((*p)->type() != elfcpp::PT_LOAD)
	    continue;
	  if ((*p)->is_unique_segment())
	    continue;
	  if (!parameters->options().omagic()
	      && ((*p)->flags() & elfcpp::PF_W) != (seg_flags & elfcpp::PF_W))
	    continue;
	  if ((target->isolate_execinstr() || parameters->options().rosegment())
	      && ((*p)->flags() & elfcpp::PF_X) != (seg_flags & elfcpp::PF_X))
	    continue;
	  // If -Tbss was specified, we need to separate the data and BSS
	  // segments.
	  if (parameters->options().user_set_Tbss())
	    {
	      if ((os->type() == elfcpp::SHT_NOBITS)
		  == (*p)->has_any_data_sections())
		continue;
	    }
	  if (os->is_large_data_section() && !(*p)->is_large_data_segment())
	    continue;

	  if (is_address_set)
	    {
	      if ((*p)->are_addresses_set())
		continue;

	      (*p)->add_initial_output_data(os);
	      (*p)->update_flags_for_output_section(seg_flags);
	      (*p)->set_addresses(addr, addr);
	      break;
	    }

	  (*p)->add_output_section_to_load(this, os, seg_flags);
	  break;
	}
    }

  if (p == this->segment_list_.end()
      || os->is_unique_segment())
    {
      Output_segment* oseg = this->make_output_segment(elfcpp::PT_LOAD,
						       seg_flags);
      if (os->is_large_data_section())
	oseg->set_is_large_data_segment();
      oseg->add_output_section_to_load(this, os, seg_flags);
      if (is_address_set)
	oseg->set_addresses(addr, addr);
      // Check if segment should be marked unique.  For segments marked
      // unique by linker plugins, set the new alignment if specified.
      if (os->is_unique_segment())
	{
	  oseg->set_is_unique_segment();
	  if (os->segment_alignment() != 0)
	    oseg->set_minimum_p_align(os->segment_alignment());
	}
    }

  // If we see a loadable SHT_NOTE section, we create a PT_NOTE
  // segment.
  if (os->type() == elfcpp::SHT_NOTE)
    {
      uint64_t os_align = os->addralign();

      // See if we already have an equivalent PT_NOTE segment.
      for (p = this->segment_list_.begin();
	   p != segment_list_.end();
	   ++p)
	{
	  if ((*p)->type() == elfcpp::PT_NOTE
	      && (*p)->align() == os_align
	      && (((*p)->flags() & elfcpp::PF_W)
		  == (seg_flags & elfcpp::PF_W)))
	    {
	      (*p)->add_output_section_to_nonload(os, seg_flags);
	      break;
	    }
	}

      if (p == this->segment_list_.end())
	{
	  Output_segment* oseg = this->make_output_segment(elfcpp::PT_NOTE,
							   seg_flags);
	  oseg->add_output_section_to_nonload(os, seg_flags);
	  oseg->set_align(os_align);
	}
    }

  // If we see a loadable SHF_TLS section, we create a PT_TLS
  // segment.  There can only be one such segment.
  if ((flags & elfcpp::SHF_TLS) != 0)
    {
      if (this->tls_segment_ == NULL)
	this->make_output_segment(elfcpp::PT_TLS, seg_flags);
      this->tls_segment_->add_output_section_to_nonload(os, seg_flags);
    }

  // If -z relro is in effect, and we see a relro section, we create a
  // PT_GNU_RELRO segment.  There can only be one such segment.
  if (os->is_relro() && parameters->options().relro())
    {
      gold_assert(seg_flags == (elfcpp::PF_R | elfcpp::PF_W));
      if (this->relro_segment_ == NULL)
	this->make_output_segment(elfcpp::PT_GNU_RELRO, seg_flags);
      this->relro_segment_->add_output_section_to_nonload(os, seg_flags);
    }

  // If we see a section named .interp, put it into a PT_INTERP
  // segment.  This seems broken to me, but this is what GNU ld does,
  // and glibc expects it.
  if (strcmp(os->name(), ".interp") == 0
      && !this->script_options_->saw_phdrs_clause())
    {
      if (this->interp_segment_ == NULL)
	this->make_output_segment(elfcpp::PT_INTERP, seg_flags);
      else
	gold_warning(_("multiple '.interp' sections in input files "
		       "may cause confusing PT_INTERP segment"));
      this->interp_segment_->add_output_section_to_nonload(os, seg_flags);
    }
}

// Make an output section for a script.

Output_section*
Layout::make_output_section_for_script(
    const char* name,
    Script_sections::Section_type section_type)
{
  name = this->namepool_.add(name, false, NULL);
  elfcpp::Elf_Xword sh_flags = elfcpp::SHF_ALLOC;
  if (section_type == Script_sections::ST_NOLOAD)
    sh_flags = 0;
  Output_section* os = this->make_output_section(name, elfcpp::SHT_PROGBITS,
						 sh_flags, ORDER_INVALID,
						 false);
  os->set_found_in_sections_clause();
  if (section_type == Script_sections::ST_NOLOAD)
    os->set_is_noload();
  return os;
}

// Return the number of segments we expect to see.

size_t
Layout::expected_segment_count() const
{
  size_t ret = this->segment_list_.size();

  // If we didn't see a SECTIONS clause in a linker script, we should
  // already have the complete list of segments.  Otherwise we ask the
  // SECTIONS clause how many segments it expects, and add in the ones
  // we already have (PT_GNU_STACK, PT_GNU_EH_FRAME, etc.)

  if (!this->script_options_->saw_sections_clause())
    return ret;
  else
    {
      const Script_sections* ss = this->script_options_->script_sections();
      return ret + ss->expected_segment_count(this);
    }
}

// Handle the .note.GNU-stack section at layout time.  SEEN_GNU_STACK
// is whether we saw a .note.GNU-stack section in the object file.
// GNU_STACK_FLAGS is the section flags.  The flags give the
// protection required for stack memory.  We record this in an
// executable as a PT_GNU_STACK segment.  If an object file does not
// have a .note.GNU-stack segment, we must assume that it is an old
// object.  On some targets that will force an executable stack.

void
Layout::layout_gnu_stack(bool seen_gnu_stack, uint64_t gnu_stack_flags,
			 const Object* obj)
{
  if (!seen_gnu_stack)
    {
      this->input_without_gnu_stack_note_ = true;
      if (parameters->options().warn_execstack()
	  && parameters->target().is_default_stack_executable())
	gold_warning(_("%s: missing .note.GNU-stack section"
		       " implies executable stack"),
		     obj->name().c_str());
    }
  else
    {
      this->input_with_gnu_stack_note_ = true;
      if ((gnu_stack_flags & elfcpp::SHF_EXECINSTR) != 0)
	{
	  this->input_requires_executable_stack_ = true;
	  if (parameters->options().warn_execstack())
	    gold_warning(_("%s: requires executable stack"),
			 obj->name().c_str());
	}
    }
}

// Read a value with given size and endianness.

static inline uint64_t
read_sized_value(size_t size, const unsigned char* buf, bool is_big_endian,
		 const Object* object)
{
  uint64_t val = 0;
  if (size == 4)
    {
      if (is_big_endian)
	val = elfcpp::Swap<32, true>::readval(buf);
      else
	val = elfcpp::Swap<32, false>::readval(buf);
    }
  else if (size == 8)
    {
      if (is_big_endian)
	val = elfcpp::Swap<64, true>::readval(buf);
      else
	val = elfcpp::Swap<64, false>::readval(buf);
    }
  else
    {
      gold_warning(_("%s: in .note.gnu.property section, "
		     "pr_datasz must be 4 or 8"),
		   object->name().c_str());
    }
  return val;
}

// Write a value with given size and endianness.

static inline void
write_sized_value(uint64_t value, size_t size, unsigned char* buf,
		  bool is_big_endian)
{
  if (size == 4)
    {
      if (is_big_endian)
	elfcpp::Swap<32, true>::writeval(buf, static_cast<uint32_t>(value));
      else
	elfcpp::Swap<32, false>::writeval(buf, static_cast<uint32_t>(value));
    }
  else if (size == 8)
    {
      if (is_big_endian)
	elfcpp::Swap<64, true>::writeval(buf, value);
      else
	elfcpp::Swap<64, false>::writeval(buf, value);
    }
  else
    {
      // We will have already complained about this.
    }
}

// Handle the .note.gnu.property section at layout time.

void
Layout::layout_gnu_property(unsigned int note_type,
			    unsigned int pr_type,
			    size_t pr_datasz,
			    const unsigned char* pr_data,
			    const Object* object)
{
  // We currently support only the one note type.
  gold_assert(note_type == elfcpp::NT_GNU_PROPERTY_TYPE_0);

  if (pr_type >= elfcpp::GNU_PROPERTY_LOPROC
      && pr_type < elfcpp::GNU_PROPERTY_HIPROC)
    {
      // Target-dependent property value; call the target to record.
      const int size = parameters->target().get_size();
      const bool is_big_endian = parameters->target().is_big_endian();
      if (size == 32)
	{
	  if (is_big_endian)
	    {
#ifdef HAVE_TARGET_32_BIG
	      parameters->sized_target<32, true>()->
		  record_gnu_property(note_type, pr_type, pr_datasz, pr_data,
				      object);
#else
	      gold_unreachable();
#endif
	    }
	  else
	    {
#ifdef HAVE_TARGET_32_LITTLE
	      parameters->sized_target<32, false>()->
		  record_gnu_property(note_type, pr_type, pr_datasz, pr_data,
				      object);
#else
	      gold_unreachable();
#endif
	    }
	}
      else if (size == 64)
	{
	  if (is_big_endian)
	    {
#ifdef HAVE_TARGET_64_BIG
	      parameters->sized_target<64, true>()->
		  record_gnu_property(note_type, pr_type, pr_datasz, pr_data,
				      object);
#else
	      gold_unreachable();
#endif
	    }
	  else
	    {
#ifdef HAVE_TARGET_64_LITTLE
	      parameters->sized_target<64, false>()->
		  record_gnu_property(note_type, pr_type, pr_datasz, pr_data,
				      object);
#else
	      gold_unreachable();
#endif
	    }
	}
      else
	gold_unreachable();
      return;
    }

  Gnu_properties::iterator pprop = this->gnu_properties_.find(pr_type);
  if (pprop == this->gnu_properties_.end())
    {
      Gnu_property prop;
      prop.pr_datasz = pr_datasz;
      prop.pr_data = new unsigned char[pr_datasz];
      memcpy(prop.pr_data, pr_data, pr_datasz);
      this->gnu_properties_[pr_type] = prop;
    }
  else
    {
      const bool is_big_endian = parameters->target().is_big_endian();
      switch (pr_type)
	{
	case elfcpp::GNU_PROPERTY_STACK_SIZE:
	  // Record the maximum value seen.
	  {
	    uint64_t val1 = read_sized_value(pprop->second.pr_datasz,
					     pprop->second.pr_data,
					     is_big_endian, object);
	    uint64_t val2 = read_sized_value(pr_datasz, pr_data,
					     is_big_endian, object);
	    if (val2 > val1)
	      write_sized_value(val2, pprop->second.pr_datasz,
				pprop->second.pr_data, is_big_endian);
	  }
	  break;
	case elfcpp::GNU_PROPERTY_NO_COPY_ON_PROTECTED:
	  // No data to merge.
	  break;
	default:
	  gold_warning(_("%s: unknown program property type %d "
			 "in .note.gnu.property section"),
		       object->name().c_str(), pr_type);
	}
    }
}

// Merge per-object properties with program properties.
// This lets the target identify objects that are missing certain
// properties, in cases where properties must be ANDed together.

void
Layout::merge_gnu_properties(const Object* object)
{
  const int size = parameters->target().get_size();
  const bool is_big_endian = parameters->target().is_big_endian();
  if (size == 32)
    {
      if (is_big_endian)
	{
#ifdef HAVE_TARGET_32_BIG
	  parameters->sized_target<32, true>()->merge_gnu_properties(object);
#else
	  gold_unreachable();
#endif
	}
      else
	{
#ifdef HAVE_TARGET_32_LITTLE
	  parameters->sized_target<32, false>()->merge_gnu_properties(object);
#else
	  gold_unreachable();
#endif
	}
    }
  else if (size == 64)
    {
      if (is_big_endian)
	{
#ifdef HAVE_TARGET_64_BIG
	  parameters->sized_target<64, true>()->merge_gnu_properties(object);
#else
	  gold_unreachable();
#endif
	}
      else
	{
#ifdef HAVE_TARGET_64_LITTLE
	  parameters->sized_target<64, false>()->merge_gnu_properties(object);
#else
	  gold_unreachable();
#endif
	}
    }
  else
    gold_unreachable();
}

// Add a target-specific property for the output .note.gnu.property section.

void
Layout::add_gnu_property(unsigned int note_type,
			 unsigned int pr_type,
			 size_t pr_datasz,
			 const unsigned char* pr_data)
{
  gold_assert(note_type == elfcpp::NT_GNU_PROPERTY_TYPE_0);

  Gnu_property prop;
  prop.pr_datasz = pr_datasz;
  prop.pr_data = new unsigned char[pr_datasz];
  memcpy(prop.pr_data, pr_data, pr_datasz);
  this->gnu_properties_[pr_type] = prop;
}

// Create automatic note sections.

void
Layout::create_notes()
{
  this->create_gnu_properties_note();
  this->create_gold_note();
  this->create_stack_segment();
  this->create_build_id();
  this->create_package_metadata();
}

// Create the dynamic sections which are needed before we read the
// relocs.

void
Layout::create_initial_dynamic_sections(Symbol_table* symtab)
{
  if (parameters->doing_static_link())
    return;

  this->dynamic_section_ = this->choose_output_section(NULL, ".dynamic",
						       elfcpp::SHT_DYNAMIC,
						       (elfcpp::SHF_ALLOC
							| elfcpp::SHF_WRITE),
						       false, ORDER_RELRO,
						       true, false, false);

  // A linker script may discard .dynamic, so check for NULL.
  if (this->dynamic_section_ != NULL)
    {
      this->dynamic_symbol_ =
	symtab->define_in_output_data("_DYNAMIC", NULL,
				      Symbol_table::PREDEFINED,
				      this->dynamic_section_, 0, 0,
				      elfcpp::STT_OBJECT, elfcpp::STB_LOCAL,
				      elfcpp::STV_HIDDEN, 0, false, false);

      this->dynamic_data_ =  new Output_data_dynamic(&this->dynpool_);

      this->dynamic_section_->add_output_section_data(this->dynamic_data_);
    }
}

// For each output section whose name can be represented as C symbol,
// define __start and __stop symbols for the section.  This is a GNU
// extension.

void
Layout::define_section_symbols(Symbol_table* symtab)
{
  const elfcpp::STV visibility = parameters->options().start_stop_visibility_enum();
  for (Section_list::const_iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      const char* const name = (*p)->name();
      if (is_cident(name))
	{
	  const std::string name_string(name);
	  const std::string start_name(cident_section_start_prefix
				       + name_string);
	  const std::string stop_name(cident_section_stop_prefix
				      + name_string);

	  symtab->define_in_output_data(start_name.c_str(),
					NULL, // version
					Symbol_table::PREDEFINED,
					*p,
					0, // value
					0, // symsize
					elfcpp::STT_NOTYPE,
					elfcpp::STB_GLOBAL,
					visibility,
					0, // nonvis
					false, // offset_is_from_end
					true); // only_if_ref

	  symtab->define_in_output_data(stop_name.c_str(),
					NULL, // version
					Symbol_table::PREDEFINED,
					*p,
					0, // value
					0, // symsize
					elfcpp::STT_NOTYPE,
					elfcpp::STB_GLOBAL,
					visibility,
					0, // nonvis
					true, // offset_is_from_end
					true); // only_if_ref
	}
    }
}

// Define symbols for group signatures.

void
Layout::define_group_signatures(Symbol_table* symtab)
{
  for (Group_signatures::iterator p = this->group_signatures_.begin();
       p != this->group_signatures_.end();
       ++p)
    {
      Symbol* sym = symtab->lookup(p->signature, NULL);
      if (sym != NULL)
	p->section->set_info_symndx(sym);
      else
	{
	  // Force the name of the group section to the group
	  // signature, and use the group's section symbol as the
	  // signature symbol.
	  if (strcmp(p->section->name(), p->signature) != 0)
	    {
	      const char* name = this->namepool_.add(p->signature,
						     true, NULL);
	      p->section->set_name(name);
	    }
	  p->section->set_needs_symtab_index();
	  p->section->set_info_section_symndx(p->section);
	}
    }

  this->group_signatures_.clear();
}

// Find the first read-only PT_LOAD segment, creating one if
// necessary.

Output_segment*
Layout::find_first_load_seg(const Target* target)
{
  Output_segment* best = NULL;
  for (Segment_list::const_iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    {
      if ((*p)->type() == elfcpp::PT_LOAD
	  && ((*p)->flags() & elfcpp::PF_R) != 0
	  && (parameters->options().omagic()
	      || ((*p)->flags() & elfcpp::PF_W) == 0)
	  && (!target->isolate_execinstr()
	      || ((*p)->flags() & elfcpp::PF_X) == 0))
	{
	  if (best == NULL || this->segment_precedes(*p, best))
	    best = *p;
	}
    }
  if (best != NULL)
    return best;

  gold_assert(!this->script_options_->saw_phdrs_clause());

  Output_segment* load_seg = this->make_output_segment(elfcpp::PT_LOAD,
						       elfcpp::PF_R);
  return load_seg;
}

// Save states of all current output segments.  Store saved states
// in SEGMENT_STATES.

void
Layout::save_segments(Segment_states* segment_states)
{
  for (Segment_list::const_iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    {
      Output_segment* segment = *p;
      // Shallow copy.
      Output_segment* copy = new Output_segment(*segment);
      (*segment_states)[segment] = copy;
    }
}

// Restore states of output segments and delete any segment not found in
// SEGMENT_STATES.

void
Layout::restore_segments(const Segment_states* segment_states)
{
  // Go through the segment list and remove any segment added in the
  // relaxation loop.
  this->tls_segment_ = NULL;
  this->relro_segment_ = NULL;
  Segment_list::iterator list_iter = this->segment_list_.begin();
  while (list_iter != this->segment_list_.end())
    {
      Output_segment* segment = *list_iter;
      Segment_states::const_iterator states_iter =
	  segment_states->find(segment);
      if (states_iter != segment_states->end())
	{
	  const Output_segment* copy = states_iter->second;
	  // Shallow copy to restore states.
	  *segment = *copy;

	  // Also fix up TLS and RELRO segment pointers as appropriate.
	  if (segment->type() == elfcpp::PT_TLS)
	    this->tls_segment_ = segment;
	  else if (segment->type() == elfcpp::PT_GNU_RELRO)
	    this->relro_segment_ = segment;

	  ++list_iter;
	}
      else
	{
	  list_iter = this->segment_list_.erase(list_iter);
	  // This is a segment created during section layout.  It should be
	  // safe to remove it since we should have removed all pointers to it.
	  delete segment;
	}
    }
}

// Clean up after relaxation so that sections can be laid out again.

void
Layout::clean_up_after_relaxation()
{
  // Restore the segments to point state just prior to the relaxation loop.
  Script_sections* script_section = this->script_options_->script_sections();
  script_section->release_segments();
  this->restore_segments(this->segment_states_);

  // Reset section addresses and file offsets
  for (Section_list::iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      (*p)->restore_states();

      // If an input section changes size because of relaxation,
      // we need to adjust the section offsets of all input sections.
      // after such a section.
      if ((*p)->section_offsets_need_adjustment())
	(*p)->adjust_section_offsets();

      (*p)->reset_address_and_file_offset();
    }

  // Reset special output object address and file offsets.
  for (Data_list::iterator p = this->special_output_list_.begin();
       p != this->special_output_list_.end();
       ++p)
    (*p)->reset_address_and_file_offset();

  // A linker script may have created some output section data objects.
  // They are useless now.
  for (Output_section_data_list::const_iterator p =
	 this->script_output_section_data_list_.begin();
       p != this->script_output_section_data_list_.end();
       ++p)
    delete *p;
  this->script_output_section_data_list_.clear();

  // Special-case fill output objects are recreated each time through
  // the relaxation loop.
  this->reset_relax_output();
}

void
Layout::reset_relax_output()
{
  for (Data_list::const_iterator p = this->relax_output_list_.begin();
       p != this->relax_output_list_.end();
       ++p)
    delete *p;
  this->relax_output_list_.clear();
}

// Prepare for relaxation.

void
Layout::prepare_for_relaxation()
{
  // Create an relaxation debug check if in debugging mode.
  if (is_debugging_enabled(DEBUG_RELAXATION))
    this->relaxation_debug_check_ = new Relaxation_debug_check();

  // Save segment states.
  this->segment_states_ = new Segment_states();
  this->save_segments(this->segment_states_);

  for(Section_list::const_iterator p = this->section_list_.begin();
      p != this->section_list_.end();
      ++p)
    (*p)->save_states();

  if (is_debugging_enabled(DEBUG_RELAXATION))
    this->relaxation_debug_check_->check_output_data_for_reset_values(
	this->section_list_, this->special_output_list_,
	this->relax_output_list_);

  // Also enable recording of output section data from scripts.
  this->record_output_section_data_from_script_ = true;
}

// If the user set the address of the text segment, that may not be
// compatible with putting the segment headers and file headers into
// that segment.  For isolate_execinstr() targets, it's the rodata
// segment rather than text where we might put the headers.
static inline bool
load_seg_unusable_for_headers(const Target* target)
{
  const General_options& options = parameters->options();
  if (target->isolate_execinstr())
    return (options.user_set_Trodata_segment()
	    && options.Trodata_segment() % target->abi_pagesize() != 0);
  else
    return (options.user_set_Ttext()
	    && options.Ttext() % target->abi_pagesize() != 0);
}

// Relaxation loop body:  If target has no relaxation, this runs only once
// Otherwise, the target relaxation hook is called at the end of
// each iteration.  If the hook returns true, it means re-layout of
// section is required.
//
// The number of segments created by a linking script without a PHDRS
// clause may be affected by section sizes and alignments.  There is
// a remote chance that relaxation causes different number of PT_LOAD
// segments are created and sections are attached to different segments.
// Therefore, we always throw away all segments created during section
// layout.  In order to be able to restart the section layout, we keep
// a copy of the segment list right before the relaxation loop and use
// that to restore the segments.
//
// PASS is the current relaxation pass number.
// SYMTAB is a symbol table.
// PLOAD_SEG is the address of a pointer for the load segment.
// PHDR_SEG is a pointer to the PHDR segment.
// SEGMENT_HEADERS points to the output segment header.
// FILE_HEADER points to the output file header.
// PSHNDX is the address to store the output section index.

off_t inline
Layout::relaxation_loop_body(
    int pass,
    Target* target,
    Symbol_table* symtab,
    Output_segment** pload_seg,
    Output_segment* phdr_seg,
    Output_segment_headers* segment_headers,
    Output_file_header* file_header,
    unsigned int* pshndx)
{
  // If this is not the first iteration, we need to clean up after
  // relaxation so that we can lay out the sections again.
  if (pass != 0)
    this->clean_up_after_relaxation();

  // If there is a SECTIONS clause, put all the input sections into
  // the required order.
  Output_segment* load_seg;
  if (this->script_options_->saw_sections_clause())
    load_seg = this->set_section_addresses_from_script(symtab);
  else if (parameters->options().relocatable())
    load_seg = NULL;
  else
    load_seg = this->find_first_load_seg(target);

  if (parameters->options().oformat_enum()
      != General_options::OBJECT_FORMAT_ELF)
    load_seg = NULL;

  if (load_seg_unusable_for_headers(target))
    {
      load_seg = NULL;
      phdr_seg = NULL;
    }

  gold_assert(phdr_seg == NULL
	      || load_seg != NULL
	      || this->script_options_->saw_sections_clause());

  // If the address of the load segment we found has been set by
  // --section-start rather than by a script, then adjust the VMA and
  // LMA downward if possible to include the file and section headers.
  uint64_t header_gap = 0;
  if (load_seg != NULL
      && load_seg->are_addresses_set()
      && !this->script_options_->saw_sections_clause()
      && !parameters->options().relocatable())
    {
      file_header->finalize_data_size();
      segment_headers->finalize_data_size();
      size_t sizeof_headers = (file_header->data_size()
			       + segment_headers->data_size());
      const uint64_t abi_pagesize = target->abi_pagesize();
      uint64_t hdr_paddr = load_seg->paddr() - sizeof_headers;
      hdr_paddr &= ~(abi_pagesize - 1);
      uint64_t subtract = load_seg->paddr() - hdr_paddr;
      if (load_seg->paddr() < subtract || load_seg->vaddr() < subtract)
	load_seg = NULL;
      else
	{
	  load_seg->set_addresses(load_seg->vaddr() - subtract,
				  load_seg->paddr() - subtract);
	  header_gap = subtract - sizeof_headers;
	}
    }

  // Lay out the segment headers.
  if (!parameters->options().relocatable())
    {
      gold_assert(segment_headers != NULL);
      if (header_gap != 0 && load_seg != NULL)
	{
	  Output_data_zero_fill* z = new Output_data_zero_fill(header_gap, 1);
	  load_seg->add_initial_output_data(z);
	}
      if (load_seg != NULL)
	load_seg->add_initial_output_data(segment_headers);
      if (phdr_seg != NULL)
	phdr_seg->add_initial_output_data(segment_headers);
    }

  // Lay out the file header.
  if (load_seg != NULL)
    load_seg->add_initial_output_data(file_header);

  if (this->script_options_->saw_phdrs_clause()
      && !parameters->options().relocatable())
    {
      // Support use of FILEHDRS and PHDRS attachments in a PHDRS
      // clause in a linker script.
      Script_sections* ss = this->script_options_->script_sections();
      ss->put_headers_in_phdrs(file_header, segment_headers);
    }

  // We set the output section indexes in set_segment_offsets and
  // set_section_indexes.
  *pshndx = 1;

  // Set the file offsets of all the segments, and all the sections
  // they contain.
  off_t off;
  if (!parameters->options().relocatable())
    off = this->set_segment_offsets(target, load_seg, pshndx);
  else
    off = this->set_relocatable_section_offsets(file_header, pshndx);

   // Verify that the dummy relaxation does not change anything.
  if (is_debugging_enabled(DEBUG_RELAXATION))
    {
      if (pass == 0)
	this->relaxation_debug_check_->read_sections(this->section_list_);
      else
	this->relaxation_debug_check_->verify_sections(this->section_list_);
    }

  *pload_seg = load_seg;
  return off;
}

// Search the list of patterns and find the position of the given section
// name in the output section.  If the section name matches a glob
// pattern and a non-glob name, then the non-glob position takes
// precedence.  Return 0 if no match is found.

unsigned int
Layout::find_section_order_index(const std::string& section_name)
{
  Unordered_map<std::string, unsigned int>::iterator map_it;
  map_it = this->input_section_position_.find(section_name);
  if (map_it != this->input_section_position_.end())
    return map_it->second;

  // Absolute match failed.  Linear search the glob patterns.
  std::vector<std::string>::iterator it;
  for (it = this->input_section_glob_.begin();
       it != this->input_section_glob_.end();
       ++it)
    {
       if (fnmatch((*it).c_str(), section_name.c_str(), FNM_NOESCAPE) == 0)
	 {
	   map_it = this->input_section_position_.find(*it);
	   gold_assert(map_it != this->input_section_position_.end());
	   return map_it->second;
	 }
    }
  return 0;
}

// Read the sequence of input sections from the file specified with
// option --section-ordering-file.

void
Layout::read_layout_from_file()
{
  const char* filename = parameters->options().section_ordering_file();
  std::ifstream in;
  std::string line;

  in.open(filename);
  if (!in)
    gold_fatal(_("unable to open --section-ordering-file file %s: %s"),
	       filename, strerror(errno));

  File_read::record_file_read(filename);

  std::getline(in, line);   // this chops off the trailing \n, if any
  unsigned int position = 1;
  this->set_section_ordering_specified();

  while (in)
    {
      if (!line.empty() && line[line.length() - 1] == '\r')   // Windows
	line.resize(line.length() - 1);
      // Ignore comments, beginning with '#'
      if (line[0] == '#')
	{
	  std::getline(in, line);
	  continue;
	}
      this->input_section_position_[line] = position;
      // Store all glob patterns in a vector.
      if (is_wildcard_string(line.c_str()))
	this->input_section_glob_.push_back(line);
      position++;
      std::getline(in, line);
    }
}

// Finalize the layout.  When this is called, we have created all the
// output sections and all the output segments which are based on
// input sections.  We have several things to do, and we have to do
// them in the right order, so that we get the right results correctly
// and efficiently.

// 1) Finalize the list of output segments and create the segment
// table header.

// 2) Finalize the dynamic symbol table and associated sections.

// 3) Determine the final file offset of all the output segments.

// 4) Determine the final file offset of all the SHF_ALLOC output
// sections.

// 5) Create the symbol table sections and the section name table
// section.

// 6) Finalize the symbol table: set symbol values to their final
// value and make a final determination of which symbols are going
// into the output symbol table.

// 7) Create the section table header.

// 8) Determine the final file offset of all the output sections which
// are not SHF_ALLOC, including the section table header.

// 9) Finalize the ELF file header.

// This function returns the size of the output file.

off_t
Layout::finalize(const Input_objects* input_objects, Symbol_table* symtab,
		 Target* target, const Task* task)
{
  unsigned int local_dynamic_count = 0;
  unsigned int forced_local_dynamic_count = 0;

  target->finalize_sections(this, input_objects, symtab);

  this->count_local_symbols(task, input_objects);

  this->link_stabs_sections();

  Output_segment* phdr_seg = NULL;
  if (!parameters->options().relocatable() && !parameters->doing_static_link())
    {
      // There was a dynamic object in the link.  We need to create
      // some information for the dynamic linker.

      // Create the PT_PHDR segment which will hold the program
      // headers.
      if (!this->script_options_->saw_phdrs_clause())
	phdr_seg = this->make_output_segment(elfcpp::PT_PHDR, elfcpp::PF_R);

      // Create the dynamic symbol table, including the hash table.
      Output_section* dynstr;
      std::vector<Symbol*> dynamic_symbols;
      Versions versions(*this->script_options()->version_script_info(),
			&this->dynpool_);
      this->create_dynamic_symtab(input_objects, symtab, &dynstr,
				  &local_dynamic_count,
				  &forced_local_dynamic_count,
				  &dynamic_symbols,
				  &versions);

      // Create the .interp section to hold the name of the
      // interpreter, and put it in a PT_INTERP segment.  Don't do it
      // if we saw a .interp section in an input file.
      if ((!parameters->options().shared()
	   || parameters->options().dynamic_linker() != NULL)
	  && this->interp_segment_ == NULL)
	this->create_interp(target);

      // Finish the .dynamic section to hold the dynamic data, and put
      // it in a PT_DYNAMIC segment.
      this->finish_dynamic_section(input_objects, symtab);

      // We should have added everything we need to the dynamic string
      // table.
      this->dynpool_.set_string_offsets();

      // Create the version sections.  We can't do this until the
      // dynamic string table is complete.
      this->create_version_sections(&versions, symtab,
				    (local_dynamic_count
				     + forced_local_dynamic_count),
				    dynamic_symbols, dynstr);

      // Set the size of the _DYNAMIC symbol.  We can't do this until
      // after we call create_version_sections.
      this->set_dynamic_symbol_size(symtab);
    }

  // Create segment headers.
  Output_segment_headers* segment_headers =
    (parameters->options().relocatable()
     ? NULL
     : new Output_segment_headers(this->segment_list_));

  // Lay out the file header.
  Output_file_header* file_header = new Output_file_header(target, symtab,
							   segment_headers);

  this->special_output_list_.push_back(file_header);
  if (segment_headers != NULL)
    this->special_output_list_.push_back(segment_headers);

  // Find approriate places for orphan output sections if we are using
  // a linker script.
  if (this->script_options_->saw_sections_clause())
    this->place_orphan_sections_in_script();

  Output_segment* load_seg;
  off_t off;
  unsigned int shndx;
  int pass = 0;

  // Take a snapshot of the section layout as needed.
  if (target->may_relax())
    this->prepare_for_relaxation();

  // Run the relaxation loop to lay out sections.
  do
    {
      off = this->relaxation_loop_body(pass, target, symtab, &load_seg,
				       phdr_seg, segment_headers, file_header,
				       &shndx);
      pass++;
    }
  while (target->may_relax()
	 && target->relax(pass, input_objects, symtab, this, task));

  // If there is a load segment that contains the file and program headers,
  // provide a symbol __ehdr_start pointing there.
  // A program can use this to examine itself robustly.
  Symbol *ehdr_start = symtab->lookup("__ehdr_start");
  if (ehdr_start != NULL && ehdr_start->is_predefined())
    {
      if (load_seg != NULL)
	ehdr_start->set_output_segment(load_seg, Symbol::SEGMENT_START);
      else
	ehdr_start->set_undefined();
    }

  // Set the file offsets of all the non-data sections we've seen so
  // far which don't have to wait for the input sections.  We need
  // this in order to finalize local symbols in non-allocated
  // sections.
  off = this->set_section_offsets(off, BEFORE_INPUT_SECTIONS_PASS);

  // Set the section indexes of all unallocated sections seen so far,
  // in case any of them are somehow referenced by a symbol.
  shndx = this->set_section_indexes(shndx);

  // Create the symbol table sections.
  this->create_symtab_sections(input_objects, symtab, shndx, &off,
			       local_dynamic_count);
  if (!parameters->doing_static_link())
    this->assign_local_dynsym_offsets(input_objects);

  // Process any symbol assignments from a linker script.  This must
  // be called after the symbol table has been finalized.
  this->script_options_->finalize_symbols(symtab, this);

  // Create the incremental inputs sections.
  if (this->incremental_inputs_)
    {
      this->incremental_inputs_->finalize();
      this->create_incremental_info_sections(symtab);
    }

  // Create the .shstrtab section.
  Output_section* shstrtab_section = this->create_shstrtab();

  // Set the file offsets of the rest of the non-data sections which
  // don't have to wait for the input sections.
  off = this->set_section_offsets(off, BEFORE_INPUT_SECTIONS_PASS);

  // Now that all sections have been created, set the section indexes
  // for any sections which haven't been done yet.
  shndx = this->set_section_indexes(shndx);

  // Create the section table header.
  this->create_shdrs(shstrtab_section, &off);

  // If there are no sections which require postprocessing, we can
  // handle the section names now, and avoid a resize later.
  if (!this->any_postprocessing_sections_)
    {
      off = this->set_section_offsets(off,
				      POSTPROCESSING_SECTIONS_PASS);
      off =
	  this->set_section_offsets(off,
				    STRTAB_AFTER_POSTPROCESSING_SECTIONS_PASS);
    }

  file_header->set_section_info(this->section_headers_, shstrtab_section);

  // Now we know exactly where everything goes in the output file
  // (except for non-allocated sections which require postprocessing).
  Output_data::layout_complete();

  this->output_file_size_ = off;

  return off;
}

// Create a note header following the format defined in the ELF ABI.
// NAME is the name, NOTE_TYPE is the type, SECTION_NAME is the name
// of the section to create, DESCSZ is the size of the descriptor.
// ALLOCATE is true if the section should be allocated in memory.
// This returns the new note section.  It sets *TRAILING_PADDING to
// the number of trailing zero bytes required.

Output_section*
Layout::create_note(const char* name, int note_type,
		    const char* section_name, size_t descsz,
		    bool allocate, size_t* trailing_padding)
{
  // Authorities all agree that the values in a .note field should
  // be aligned on 4-byte boundaries for 32-bit binaries.  However,
  // they differ on what the alignment is for 64-bit binaries.
  // The GABI says unambiguously they take 8-byte alignment:
  //    http://sco.com/developers/gabi/latest/ch5.pheader.html#note_section
  // Other documentation says alignment should always be 4 bytes:
  //    http://www.netbsd.org/docs/kernel/elf-notes.html#note-format
  // GNU ld and GNU readelf both support the latter (at least as of
  // version 2.16.91), and glibc always generates the latter for
  // .note.ABI-tag (as of version 1.6), so that's the one we go with
  // here.
#ifdef GABI_FORMAT_FOR_DOTNOTE_SECTION   // This is not defined by default.
  const int size = parameters->target().get_size();
#else
  const int size = 32;
#endif
  // The NT_GNU_PROPERTY_TYPE_0 note is aligned to the pointer size.
  const int addralign = ((note_type == elfcpp::NT_GNU_PROPERTY_TYPE_0
			 ? parameters->target().get_size()
			 : size) / 8);

  // The contents of the .note section.
  size_t namesz = strlen(name) + 1;
  size_t aligned_namesz = align_address(namesz, size / 8);
  size_t aligned_descsz = align_address(descsz, size / 8);

  size_t notehdrsz = 3 * (size / 8) + aligned_namesz;

  unsigned char* buffer = new unsigned char[notehdrsz];
  memset(buffer, 0, notehdrsz);

  bool is_big_endian = parameters->target().is_big_endian();

  if (size == 32)
    {
      if (!is_big_endian)
	{
	  elfcpp::Swap<32, false>::writeval(buffer, namesz);
	  elfcpp::Swap<32, false>::writeval(buffer + 4, descsz);
	  elfcpp::Swap<32, false>::writeval(buffer + 8, note_type);
	}
      else
	{
	  elfcpp::Swap<32, true>::writeval(buffer, namesz);
	  elfcpp::Swap<32, true>::writeval(buffer + 4, descsz);
	  elfcpp::Swap<32, true>::writeval(buffer + 8, note_type);
	}
    }
  else if (size == 64)
    {
      if (!is_big_endian)
	{
	  elfcpp::Swap<64, false>::writeval(buffer, namesz);
	  elfcpp::Swap<64, false>::writeval(buffer + 8, descsz);
	  elfcpp::Swap<64, false>::writeval(buffer + 16, note_type);
	}
      else
	{
	  elfcpp::Swap<64, true>::writeval(buffer, namesz);
	  elfcpp::Swap<64, true>::writeval(buffer + 8, descsz);
	  elfcpp::Swap<64, true>::writeval(buffer + 16, note_type);
	}
    }
  else
    gold_unreachable();

  memcpy(buffer + 3 * (size / 8), name, namesz);

  elfcpp::Elf_Xword flags = 0;
  Output_section_order order = ORDER_INVALID;
  if (allocate)
    {
      flags = elfcpp::SHF_ALLOC;
      order = (note_type == elfcpp::NT_GNU_PROPERTY_TYPE_0
	       ?  ORDER_PROPERTY_NOTE : ORDER_RO_NOTE);
    }
  Output_section* os = this->choose_output_section(NULL, section_name,
						   elfcpp::SHT_NOTE,
						   flags, false, order, false,
						   false, true);
  if (os == NULL)
    return NULL;

  Output_section_data* posd = new Output_data_const_buffer(buffer, notehdrsz,
							   addralign,
							   "** note header");
  os->add_output_section_data(posd);

  *trailing_padding = aligned_descsz - descsz;

  return os;
}

// Create a .note.gnu.property section to record program properties
// accumulated from the input files.

void
Layout::create_gnu_properties_note()
{
  parameters->target().finalize_gnu_properties(this);

  if (this->gnu_properties_.empty())
    return;

  const unsigned int size = parameters->target().get_size();
  const bool is_big_endian = parameters->target().is_big_endian();

  // Compute the total size of the properties array.
  size_t descsz = 0;
  for (Gnu_properties::const_iterator prop = this->gnu_properties_.begin();
       prop != this->gnu_properties_.end();
       ++prop)
    {
      descsz = align_address(descsz + 8 + prop->second.pr_datasz, size / 8);
    }

  // Create the note section.
  size_t trailing_padding;
  Output_section* os = this->create_note("GNU", elfcpp::NT_GNU_PROPERTY_TYPE_0,
					 ".note.gnu.property", descsz,
					 true, &trailing_padding);
  if (os == NULL)
    return;
  gold_assert(trailing_padding == 0);

  // Allocate and fill the properties array.
  unsigned char* desc = new unsigned char[descsz];
  unsigned char* p = desc;
  for (Gnu_properties::const_iterator prop = this->gnu_properties_.begin();
       prop != this->gnu_properties_.end();
       ++prop)
    {
      size_t datasz = prop->second.pr_datasz;
      size_t aligned_datasz = align_address(prop->second.pr_datasz, size / 8);
      write_sized_value(prop->first, 4, p, is_big_endian);
      write_sized_value(datasz, 4, p + 4, is_big_endian);
      memcpy(p + 8, prop->second.pr_data, datasz);
      if (aligned_datasz > datasz)
	memset(p + 8 + datasz, 0, aligned_datasz - datasz);
      p += 8 + aligned_datasz;
    }
  Output_section_data* posd = new Output_data_const(desc, descsz, 4);
  os->add_output_section_data(posd);
}

// For an executable or shared library, create a note to record the
// version of gold used to create the binary.

void
Layout::create_gold_note()
{
  if (parameters->options().relocatable()
      || parameters->incremental_update())
    return;

  std::string desc = std::string("gold ") + gold::get_version_string();

  Output_section* os;
  Output_section_data* posd;

  if (!parameters->options().enable_linker_version())
    {
      size_t trailing_padding;

      os = this->create_note("GNU", elfcpp::NT_GNU_GOLD_VERSION,
			     ".note.gnu.gold-version", desc.size(),
			     false, &trailing_padding);
      if (os == NULL)
	return;

      posd = new Output_data_const(desc, 4);
      os->add_output_section_data(posd);

      if (trailing_padding > 0)
	{
	  posd = new Output_data_zero_fill(trailing_padding, 0);
	  os->add_output_section_data(posd);
	}
    }
  else
    {
      os = this->choose_output_section(NULL, ".comment",
				       elfcpp::SHT_PROGBITS, 0,
				       false, ORDER_INVALID,
				       false, false, false);
      if (os == NULL)
	return;

      posd = new Output_data_const(desc, 1);
      os->add_output_section_data(posd);
    }
}

// Record whether the stack should be executable.  This can be set
// from the command line using the -z execstack or -z noexecstack
// options.  Otherwise, if any input file has a .note.GNU-stack
// section with the SHF_EXECINSTR flag set, the stack should be
// executable.  Otherwise, if at least one input file a
// .note.GNU-stack section, and some input file has no .note.GNU-stack
// section, we use the target default for whether the stack should be
// executable.  If -z stack-size was used to set a p_memsz value for
// PT_GNU_STACK, we generate the segment regardless.  Otherwise, we
// don't generate a stack note.  When generating a object file, we
// create a .note.GNU-stack section with the appropriate marking.
// When generating an executable or shared library, we create a
// PT_GNU_STACK segment.

void
Layout::create_stack_segment()
{
  bool is_stack_executable;
  if (parameters->options().is_execstack_set())
    {
      is_stack_executable = parameters->options().is_stack_executable();
      if (!is_stack_executable
	  && this->input_requires_executable_stack_
	  && parameters->options().warn_execstack())
	gold_warning(_("one or more inputs require executable stack, "
		       "but -z noexecstack was given"));
    }
  else if (!this->input_with_gnu_stack_note_
	   && (!parameters->options().user_set_stack_size()
	       || parameters->options().relocatable()))
    return;
  else
    {
      if (this->input_requires_executable_stack_)
	is_stack_executable = true;
      else if (this->input_without_gnu_stack_note_)
	is_stack_executable =
	  parameters->target().is_default_stack_executable();
      else
	is_stack_executable = false;
    }

  if (parameters->options().relocatable())
    {
      const char* name = this->namepool_.add(".note.GNU-stack", false, NULL);
      elfcpp::Elf_Xword flags = 0;
      if (is_stack_executable)
	flags |= elfcpp::SHF_EXECINSTR;
      this->make_output_section(name, elfcpp::SHT_PROGBITS, flags,
				ORDER_INVALID, false);
    }
  else
    {
      if (this->script_options_->saw_phdrs_clause())
	return;
      int flags = elfcpp::PF_R | elfcpp::PF_W;
      if (is_stack_executable)
	flags |= elfcpp::PF_X;
      Output_segment* seg =
	this->make_output_segment(elfcpp::PT_GNU_STACK, flags);
      seg->set_size(parameters->options().stack_size());
      // BFD lets targets override this default alignment, but the only
      // targets that do so are ones that Gold does not support so far.
      seg->set_minimum_p_align(16);
    }
}

// If --build-id was used, set up the build ID note.

void
Layout::create_build_id()
{
  if (!parameters->options().user_set_build_id())
    return;

  const char* style = parameters->options().build_id();
  if (strcmp(style, "none") == 0)
    return;

  // Set DESCSZ to the size of the note descriptor.  When possible,
  // set DESC to the note descriptor contents.
  size_t descsz;
  std::string desc;
  if (strcmp(style, "md5") == 0)
    descsz = 128 / 8;
  else if ((strcmp(style, "sha1") == 0) || (strcmp(style, "tree") == 0))
    descsz = 160 / 8;
  else if (strcmp(style, "uuid") == 0)
    {
#ifndef __MINGW32__
      const size_t uuidsz = 128 / 8;

      char buffer[uuidsz];
      memset(buffer, 0, uuidsz);

      int descriptor = open_descriptor(-1, "/dev/urandom", O_RDONLY);
      if (descriptor < 0)
	gold_error(_("--build-id=uuid failed: could not open /dev/urandom: %s"),
		   strerror(errno));
      else
	{
	  ssize_t got = ::read(descriptor, buffer, uuidsz);
	  release_descriptor(descriptor, true);
	  if (got < 0)
	    gold_error(_("/dev/urandom: read failed: %s"), strerror(errno));
	  else if (static_cast<size_t>(got) != uuidsz)
	    gold_error(_("/dev/urandom: expected %zu bytes, got %zd bytes"),
		       uuidsz, got);
	}

      desc.assign(buffer, uuidsz);
      descsz = uuidsz;
#else // __MINGW32__
      UUID uuid;
      typedef RPC_STATUS (RPC_ENTRY *UuidCreateFn)(UUID *Uuid);

      HMODULE rpc_library = LoadLibrary("rpcrt4.dll");
      if (!rpc_library)
	gold_error(_("--build-id=uuid failed: could not load rpcrt4.dll"));
      else
	{
	  UuidCreateFn uuid_create = reinterpret_cast<UuidCreateFn>(
	      GetProcAddress(rpc_library, "UuidCreate"));
	  if (!uuid_create)
	    gold_error(_("--build-id=uuid failed: could not find UuidCreate"));
	  else if (uuid_create(&uuid) != RPC_S_OK)
	    gold_error(_("__build_id=uuid failed: call UuidCreate() failed"));
	  FreeLibrary(rpc_library);
	}
      desc.assign(reinterpret_cast<const char *>(&uuid), sizeof(UUID));
      descsz = sizeof(UUID);
#endif // __MINGW32__
    }
  else if (strncmp(style, "0x", 2) == 0)
    {
      hex_init();
      const char* p = style + 2;
      while (*p != '\0')
	{
	  if (hex_p(p[0]) && hex_p(p[1]))
	    {
	      char c = (hex_value(p[0]) << 4) | hex_value(p[1]);
	      desc += c;
	      p += 2;
	    }
	  else if (*p == '-' || *p == ':')
	    ++p;
	  else
	    gold_fatal(_("--build-id argument '%s' not a valid hex number"),
		       style);
	}
      descsz = desc.size();
    }
  else
    gold_fatal(_("unrecognized --build-id argument '%s'"), style);

  // Create the note.
  size_t trailing_padding;
  Output_section* os = this->create_note("GNU", elfcpp::NT_GNU_BUILD_ID,
					 ".note.gnu.build-id", descsz, true,
					 &trailing_padding);
  if (os == NULL)
    return;

  if (!desc.empty())
    {
      // We know the value already, so we fill it in now.
      gold_assert(desc.size() == descsz);

      Output_section_data* posd = new Output_data_const(desc, 4);
      os->add_output_section_data(posd);

      if (trailing_padding != 0)
	{
	  posd = new Output_data_zero_fill(trailing_padding, 0);
	  os->add_output_section_data(posd);
	}
    }
  else
    {
      // We need to compute a checksum after we have completed the
      // link.
      gold_assert(trailing_padding == 0);
      this->build_id_note_ = new Output_data_zero_fill(descsz, 4);
      os->add_output_section_data(this->build_id_note_);
    }
}

// If --package-metadata was used, set up the package metadata note.
// https://systemd.io/ELF_PACKAGE_METADATA/

void
Layout::create_package_metadata()
{
  if (!parameters->options().user_set_package_metadata())
    return;

  const char* desc = parameters->options().package_metadata();
  if (strcmp(desc, "") == 0)
    return;

#ifdef HAVE_JANSSON
  json_error_t json_error;
  json_t *json = json_loads(desc, 0, &json_error);
  if (json)
    json_decref(json);
  else
    {
      gold_fatal(_("error: --package-metadata=%s does not contain valid "
		   "JSON: %s\n"),
		 desc, json_error.text);
    }
#endif

  // Create the note.
  size_t trailing_padding;
  // Ensure the trailing NULL byte is always included, as per specification.
  size_t descsz = strlen(desc) + 1;
  Output_section* os = this->create_note("FDO", elfcpp::FDO_PACKAGING_METADATA,
					 ".note.package", descsz, true,
					 &trailing_padding);
  if (os == NULL)
    return;

  Output_section_data* posd = new Output_data_const(desc, descsz, 4);
  os->add_output_section_data(posd);

  if (trailing_padding != 0)
    {
      posd = new Output_data_zero_fill(trailing_padding, 0);
      os->add_output_section_data(posd);
    }
}

// If we have both .stabXX and .stabXXstr sections, then the sh_link
// field of the former should point to the latter.  I'm not sure who
// started this, but the GNU linker does it, and some tools depend
// upon it.

void
Layout::link_stabs_sections()
{
  if (!this->have_stabstr_section_)
    return;

  for (Section_list::iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      if ((*p)->type() != elfcpp::SHT_STRTAB)
	continue;

      const char* name = (*p)->name();
      if (strncmp(name, ".stab", 5) != 0)
	continue;

      size_t len = strlen(name);
      if (strcmp(name + len - 3, "str") != 0)
	continue;

      std::string stab_name(name, len - 3);
      Output_section* stab_sec;
      stab_sec = this->find_output_section(stab_name.c_str());
      if (stab_sec != NULL)
	stab_sec->set_link_section(*p);
    }
}

// Create .gnu_incremental_inputs and related sections needed
// for the next run of incremental linking to check what has changed.

void
Layout::create_incremental_info_sections(Symbol_table* symtab)
{
  Incremental_inputs* incr = this->incremental_inputs_;

  gold_assert(incr != NULL);

  // Create the .gnu_incremental_inputs, _symtab, and _relocs input sections.
  incr->create_data_sections(symtab);

  // Add the .gnu_incremental_inputs section.
  const char* incremental_inputs_name =
    this->namepool_.add(".gnu_incremental_inputs", false, NULL);
  Output_section* incremental_inputs_os =
    this->make_output_section(incremental_inputs_name,
			      elfcpp::SHT_GNU_INCREMENTAL_INPUTS, 0,
			      ORDER_INVALID, false);
  incremental_inputs_os->add_output_section_data(incr->inputs_section());

  // Add the .gnu_incremental_symtab section.
  const char* incremental_symtab_name =
    this->namepool_.add(".gnu_incremental_symtab", false, NULL);
  Output_section* incremental_symtab_os =
    this->make_output_section(incremental_symtab_name,
			      elfcpp::SHT_GNU_INCREMENTAL_SYMTAB, 0,
			      ORDER_INVALID, false);
  incremental_symtab_os->add_output_section_data(incr->symtab_section());
  incremental_symtab_os->set_entsize(4);

  // Add the .gnu_incremental_relocs section.
  const char* incremental_relocs_name =
    this->namepool_.add(".gnu_incremental_relocs", false, NULL);
  Output_section* incremental_relocs_os =
    this->make_output_section(incremental_relocs_name,
			      elfcpp::SHT_GNU_INCREMENTAL_RELOCS, 0,
			      ORDER_INVALID, false);
  incremental_relocs_os->add_output_section_data(incr->relocs_section());
  incremental_relocs_os->set_entsize(incr->relocs_entsize());

  // Add the .gnu_incremental_got_plt section.
  const char* incremental_got_plt_name =
    this->namepool_.add(".gnu_incremental_got_plt", false, NULL);
  Output_section* incremental_got_plt_os =
    this->make_output_section(incremental_got_plt_name,
			      elfcpp::SHT_GNU_INCREMENTAL_GOT_PLT, 0,
			      ORDER_INVALID, false);
  incremental_got_plt_os->add_output_section_data(incr->got_plt_section());

  // Add the .gnu_incremental_strtab section.
  const char* incremental_strtab_name =
    this->namepool_.add(".gnu_incremental_strtab", false, NULL);
  Output_section* incremental_strtab_os = this->make_output_section(incremental_strtab_name,
							elfcpp::SHT_STRTAB, 0,
							ORDER_INVALID, false);
  Output_data_strtab* strtab_data =
      new Output_data_strtab(incr->get_stringpool());
  incremental_strtab_os->add_output_section_data(strtab_data);

  incremental_inputs_os->set_after_input_sections();
  incremental_symtab_os->set_after_input_sections();
  incremental_relocs_os->set_after_input_sections();
  incremental_got_plt_os->set_after_input_sections();

  incremental_inputs_os->set_link_section(incremental_strtab_os);
  incremental_symtab_os->set_link_section(incremental_inputs_os);
  incremental_relocs_os->set_link_section(incremental_inputs_os);
  incremental_got_plt_os->set_link_section(incremental_inputs_os);
}

// Return whether SEG1 should be before SEG2 in the output file.  This
// is based entirely on the segment type and flags.  When this is
// called the segment addresses have normally not yet been set.

bool
Layout::segment_precedes(const Output_segment* seg1,
			 const Output_segment* seg2)
{
  // In order to produce a stable ordering if we're called with the same pointer
  // return false.
  if (seg1 == seg2)
    return false;

  elfcpp::Elf_Word type1 = seg1->type();
  elfcpp::Elf_Word type2 = seg2->type();

  // The single PT_PHDR segment is required to precede any loadable
  // segment.  We simply make it always first.
  if (type1 == elfcpp::PT_PHDR)
    {
      gold_assert(type2 != elfcpp::PT_PHDR);
      return true;
    }
  if (type2 == elfcpp::PT_PHDR)
    return false;

  // The single PT_INTERP segment is required to precede any loadable
  // segment.  We simply make it always second.
  if (type1 == elfcpp::PT_INTERP)
    {
      gold_assert(type2 != elfcpp::PT_INTERP);
      return true;
    }
  if (type2 == elfcpp::PT_INTERP)
    return false;

  // We then put PT_LOAD segments before any other segments.
  if (type1 == elfcpp::PT_LOAD && type2 != elfcpp::PT_LOAD)
    return true;
  if (type2 == elfcpp::PT_LOAD && type1 != elfcpp::PT_LOAD)
    return false;

  // We put the PT_TLS segment last except for the PT_GNU_RELRO
  // segment, because that is where the dynamic linker expects to find
  // it (this is just for efficiency; other positions would also work
  // correctly).
  if (type1 == elfcpp::PT_TLS
      && type2 != elfcpp::PT_TLS
      && type2 != elfcpp::PT_GNU_RELRO)
    return false;
  if (type2 == elfcpp::PT_TLS
      && type1 != elfcpp::PT_TLS
      && type1 != elfcpp::PT_GNU_RELRO)
    return true;

  // We put the PT_GNU_RELRO segment last, because that is where the
  // dynamic linker expects to find it (as with PT_TLS, this is just
  // for efficiency).
  if (type1 == elfcpp::PT_GNU_RELRO && type2 != elfcpp::PT_GNU_RELRO)
    return false;
  if (type2 == elfcpp::PT_GNU_RELRO && type1 != elfcpp::PT_GNU_RELRO)
    return true;

  const elfcpp::Elf_Word flags1 = seg1->flags();
  const elfcpp::Elf_Word flags2 = seg2->flags();

  // The order of non-PT_LOAD segments is unimportant.  We simply sort
  // by the numeric segment type and flags values.  There should not
  // be more than one segment with the same type and flags, except
  // when a linker script specifies such.
  if (type1 != elfcpp::PT_LOAD)
    {
      if (type1 != type2)
	return type1 < type2;
      uint64_t align1 = seg1->align();
      uint64_t align2 = seg2->align();
      // Place segments with larger alignments first.
      if (align1 != align2)
	return align1 > align2;
      gold_assert(flags1 != flags2
		  || this->script_options_->saw_phdrs_clause());
      return flags1 < flags2;
    }

  // If the addresses are set already, sort by load address.
  if (seg1->are_addresses_set())
    {
      if (!seg2->are_addresses_set())
	return true;

      unsigned int section_count1 = seg1->output_section_count();
      unsigned int section_count2 = seg2->output_section_count();
      if (section_count1 == 0 && section_count2 > 0)
	return true;
      if (section_count1 > 0 && section_count2 == 0)
	return false;

      uint64_t paddr1 =	(seg1->are_addresses_set()
			 ? seg1->paddr()
			 : seg1->first_section_load_address());
      uint64_t paddr2 =	(seg2->are_addresses_set()
			 ? seg2->paddr()
			 : seg2->first_section_load_address());

      if (paddr1 != paddr2)
	return paddr1 < paddr2;
    }
  else if (seg2->are_addresses_set())
    return false;

  // A segment which holds large data comes after a segment which does
  // not hold large data.
  if (seg1->is_large_data_segment())
    {
      if (!seg2->is_large_data_segment())
	return false;
    }
  else if (seg2->is_large_data_segment())
    return true;

  // Otherwise, we sort PT_LOAD segments based on the flags.  Readonly
  // segments come before writable segments.  Then writable segments
  // with data come before writable segments without data.  Then
  // executable segments come before non-executable segments.  Then
  // the unlikely case of a non-readable segment comes before the
  // normal case of a readable segment.  If there are multiple
  // segments with the same type and flags, we require that the
  // address be set, and we sort by virtual address and then physical
  // address.
  if ((flags1 & elfcpp::PF_W) != (flags2 & elfcpp::PF_W))
    return (flags1 & elfcpp::PF_W) == 0;
  if ((flags1 & elfcpp::PF_W) != 0
      && seg1->has_any_data_sections() != seg2->has_any_data_sections())
    return seg1->has_any_data_sections();
  if ((flags1 & elfcpp::PF_X) != (flags2 & elfcpp::PF_X))
    return (flags1 & elfcpp::PF_X) != 0;
  if ((flags1 & elfcpp::PF_R) != (flags2 & elfcpp::PF_R))
    return (flags1 & elfcpp::PF_R) == 0;

  // We shouldn't get here--we shouldn't create segments which we
  // can't distinguish.  Unless of course we are using a weird linker
  // script or overlapping --section-start options.  We could also get
  // here if plugins want unique segments for subsets of sections.
  gold_assert(this->script_options_->saw_phdrs_clause()
	      || parameters->options().any_section_start()
	      || this->is_unique_segment_for_sections_specified()
	      || parameters->options().text_unlikely_segment());
  return false;
}

// Increase OFF so that it is congruent to ADDR modulo ABI_PAGESIZE.

static off_t
align_file_offset(off_t off, uint64_t addr, uint64_t abi_pagesize)
{
  uint64_t unsigned_off = off;
  uint64_t aligned_off = ((unsigned_off & ~(abi_pagesize - 1))
			  | (addr & (abi_pagesize - 1)));
  if (aligned_off < unsigned_off)
    aligned_off += abi_pagesize;
  return aligned_off;
}

// On targets where the text segment contains only executable code,
// a non-executable segment is never the text segment.

static inline bool
is_text_segment(const Target* target, const Output_segment* seg)
{
  elfcpp::Elf_Xword flags = seg->flags();
  if ((flags & elfcpp::PF_W) != 0)
    return false;
  if ((flags & elfcpp::PF_X) == 0)
    return !target->isolate_execinstr();
  return true;
}

// Set the file offsets of all the segments, and all the sections they
// contain.  They have all been created.  LOAD_SEG must be laid out
// first.  Return the offset of the data to follow.

off_t
Layout::set_segment_offsets(const Target* target, Output_segment* load_seg,
			    unsigned int* pshndx)
{
  // Sort them into the final order.  We use a stable sort so that we
  // don't randomize the order of indistinguishable segments created
  // by linker scripts.
  std::stable_sort(this->segment_list_.begin(), this->segment_list_.end(),
		   Layout::Compare_segments(this));

  // Find the PT_LOAD segments, and set their addresses and offsets
  // and their section's addresses and offsets.
  uint64_t start_addr;
  if (parameters->options().user_set_Ttext())
    start_addr = parameters->options().Ttext();
  else if (parameters->options().output_is_position_independent())
    start_addr = 0;
  else
    start_addr = target->default_text_segment_address();

  uint64_t addr = start_addr;
  off_t off = 0;

  // If LOAD_SEG is NULL, then the file header and segment headers
  // will not be loadable.  But they still need to be at offset 0 in
  // the file.  Set their offsets now.
  if (load_seg == NULL)
    {
      for (Data_list::iterator p = this->special_output_list_.begin();
	   p != this->special_output_list_.end();
	   ++p)
	{
	  off = align_address(off, (*p)->addralign());
	  (*p)->set_address_and_file_offset(0, off);
	  off += (*p)->data_size();
	}
    }

  unsigned int increase_relro = this->increase_relro_;
  if (this->script_options_->saw_sections_clause())
    increase_relro = 0;

  const bool check_sections = parameters->options().check_sections();
  Output_segment* last_load_segment = NULL;

  unsigned int shndx_begin = *pshndx;
  unsigned int shndx_load_seg = *pshndx;

  for (Segment_list::iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    {
      if ((*p)->type() == elfcpp::PT_LOAD)
	{
	  if (target->isolate_execinstr())
	    {
	      // When we hit the segment that should contain the
	      // file headers, reset the file offset so we place
	      // it and subsequent segments appropriately.
	      // We'll fix up the preceding segments below.
	      if (load_seg == *p)
		{
		  if (off == 0)
		    load_seg = NULL;
		  else
		    {
		      off = 0;
		      shndx_load_seg = *pshndx;
		    }
		}
	    }
	  else
	    {
	      // Verify that the file headers fall into the first segment.
	      if (load_seg != NULL && load_seg != *p)
		gold_unreachable();
	      load_seg = NULL;
	    }

	  bool are_addresses_set = (*p)->are_addresses_set();
	  if (are_addresses_set)
	    {
	      // When it comes to setting file offsets, we care about
	      // the physical address.
	      addr = (*p)->paddr();
	    }
	  else if (parameters->options().user_set_Ttext()
		   && (parameters->options().omagic()
		       || is_text_segment(target, *p)))
	    {
	      are_addresses_set = true;
	    }
	  else if (parameters->options().user_set_Trodata_segment()
		   && ((*p)->flags() & (elfcpp::PF_W | elfcpp::PF_X)) == 0)
	    {
	      addr = parameters->options().Trodata_segment();
	      are_addresses_set = true;
	    }
	  else if (parameters->options().user_set_Tdata()
		   && ((*p)->flags() & elfcpp::PF_W) != 0
		   && (!parameters->options().user_set_Tbss()
		       || (*p)->has_any_data_sections()))
	    {
	      addr = parameters->options().Tdata();
	      are_addresses_set = true;
	    }
	  else if (parameters->options().user_set_Tbss()
		   && ((*p)->flags() & elfcpp::PF_W) != 0
		   && !(*p)->has_any_data_sections())
	    {
	      addr = parameters->options().Tbss();
	      are_addresses_set = true;
	    }

	  uint64_t orig_addr = addr;
	  uint64_t orig_off = off;

	  uint64_t aligned_addr = 0;
	  uint64_t abi_pagesize = target->abi_pagesize();
	  uint64_t common_pagesize = target->common_pagesize();

	  if (!parameters->options().nmagic()
	      && !parameters->options().omagic())
	    (*p)->set_minimum_p_align(abi_pagesize);

	  if (!are_addresses_set)
	    {
	      // Skip the address forward one page, maintaining the same
	      // position within the page.  This lets us store both segments
	      // overlapping on a single page in the file, but the loader will
	      // put them on different pages in memory. We will revisit this
	      // decision once we know the size of the segment.

	      uint64_t max_align = (*p)->maximum_alignment();
	      if (max_align > abi_pagesize)
		addr = align_address(addr, max_align);
	      aligned_addr = addr;

	      if (load_seg == *p)
		{
		  // This is the segment that will contain the file
		  // headers, so its offset will have to be exactly zero.
		  gold_assert(orig_off == 0);

		  // If the target wants a fixed minimum distance from the
		  // text segment to the read-only segment, move up now.
		  uint64_t min_addr =
		    start_addr + (parameters->options().user_set_rosegment_gap()
				  ? parameters->options().rosegment_gap()
				  : target->rosegment_gap());
		  if (addr < min_addr)
		    addr = min_addr;

		  // But this is not the first segment!  To make its
		  // address congruent with its offset, that address better
		  // be aligned to the ABI-mandated page size.
		  addr = align_address(addr, abi_pagesize);
		  aligned_addr = addr;
		}
	      else
		{
		  if ((addr & (abi_pagesize - 1)) != 0)
		    addr = addr + abi_pagesize;

		  off = orig_off + ((addr - orig_addr) & (abi_pagesize - 1));
		}
	    }

	  if (!parameters->options().nmagic()
	      && !parameters->options().omagic())
	    {
	      // Here we are also taking care of the case when
	      // the maximum segment alignment is larger than the page size.
	      off = align_file_offset(off, addr,
				      std::max(abi_pagesize,
					       (*p)->maximum_alignment()));
	    }
	  else
	    {
	      // This is -N or -n with a section script which prevents
	      // us from using a load segment.  We need to ensure that
	      // the file offset is aligned to the alignment of the
	      // segment.  This is because the linker script
	      // implicitly assumed a zero offset.  If we don't align
	      // here, then the alignment of the sections in the
	      // linker script may not match the alignment of the
	      // sections in the set_section_addresses call below,
	      // causing an error about dot moving backward.
	      off = align_address(off, (*p)->maximum_alignment());
	    }

	  unsigned int shndx_hold = *pshndx;
	  bool has_relro = false;
	  uint64_t new_addr = (*p)->set_section_addresses(target, this,
							  false, addr,
							  &increase_relro,
							  &has_relro,
							  &off, pshndx);

	  // Now that we know the size of this segment, we may be able
	  // to save a page in memory, at the cost of wasting some
	  // file space, by instead aligning to the start of a new
	  // page.  Here we use the real machine page size rather than
	  // the ABI mandated page size.  If the segment has been
	  // aligned so that the relro data ends at a page boundary,
	  // we do not try to realign it.

	  if (!are_addresses_set
	      && !has_relro
	      && aligned_addr != addr
	      && !parameters->incremental())
	    {
	      uint64_t first_off = (common_pagesize
				    - (aligned_addr
				       & (common_pagesize - 1)));
	      uint64_t last_off = new_addr & (common_pagesize - 1);
	      if (first_off > 0
		  && last_off > 0
		  && ((aligned_addr & ~ (common_pagesize - 1))
		      != (new_addr & ~ (common_pagesize - 1)))
		  && first_off + last_off <= common_pagesize)
		{
		  *pshndx = shndx_hold;
		  addr = align_address(aligned_addr, common_pagesize);
		  addr = align_address(addr, (*p)->maximum_alignment());
		  if ((addr & (abi_pagesize - 1)) != 0)
		    addr = addr + abi_pagesize;
		  off = orig_off + ((addr - orig_addr) & (abi_pagesize - 1));
		  off = align_file_offset(off, addr, abi_pagesize);

		  increase_relro = this->increase_relro_;
		  if (this->script_options_->saw_sections_clause())
		    increase_relro = 0;
		  has_relro = false;

		  new_addr = (*p)->set_section_addresses(target, this,
							 true, addr,
							 &increase_relro,
							 &has_relro,
							 &off, pshndx);
		}
	    }

	  addr = new_addr;

	  // Implement --check-sections.  We know that the segments
	  // are sorted by LMA.
	  if (check_sections && last_load_segment != NULL)
	    {
	      gold_assert(last_load_segment->paddr() <= (*p)->paddr());
	      if (last_load_segment->paddr() + last_load_segment->memsz()
		  > (*p)->paddr())
		{
		  unsigned long long lb1 = last_load_segment->paddr();
		  unsigned long long le1 = lb1 + last_load_segment->memsz();
		  unsigned long long lb2 = (*p)->paddr();
		  unsigned long long le2 = lb2 + (*p)->memsz();
		  gold_error(_("load segment overlap [0x%llx -> 0x%llx] and "
			       "[0x%llx -> 0x%llx]"),
			     lb1, le1, lb2, le2);
		}
	    }
	  last_load_segment = *p;
	}
    }

  if (load_seg != NULL && target->isolate_execinstr())
    {
      // Process the early segments again, setting their file offsets
      // so they land after the segments starting at LOAD_SEG.
      off = align_file_offset(off, 0, target->abi_pagesize());

      this->reset_relax_output();

      for (Segment_list::iterator p = this->segment_list_.begin();
	   *p != load_seg;
	   ++p)
	{
	  if ((*p)->type() == elfcpp::PT_LOAD)
	    {
	      // We repeat the whole job of assigning addresses and
	      // offsets, but we really only want to change the offsets and
	      // must ensure that the addresses all come out the same as
	      // they did the first time through.
	      bool has_relro = false;
	      const uint64_t old_addr = (*p)->vaddr();
	      const uint64_t old_end = old_addr + (*p)->memsz();
	      uint64_t new_addr = (*p)->set_section_addresses(target, this,
							      true, old_addr,
							      &increase_relro,
							      &has_relro,
							      &off,
							      &shndx_begin);
	      gold_assert(new_addr == old_end);
	    }
	}

      gold_assert(shndx_begin == shndx_load_seg);
    }

  // Handle the non-PT_LOAD segments, setting their offsets from their
  // section's offsets.
  for (Segment_list::iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    {
      // PT_GNU_STACK was set up correctly when it was created.
      if ((*p)->type() != elfcpp::PT_LOAD
	  && (*p)->type() != elfcpp::PT_GNU_STACK)
	(*p)->set_offset((*p)->type() == elfcpp::PT_GNU_RELRO
			 ? increase_relro
			 : 0);
    }

  // Set the TLS offsets for each section in the PT_TLS segment.
  if (this->tls_segment_ != NULL)
    this->tls_segment_->set_tls_offsets();

  return off;
}

// Set the offsets of all the allocated sections when doing a
// relocatable link.  This does the same jobs as set_segment_offsets,
// only for a relocatable link.

off_t
Layout::set_relocatable_section_offsets(Output_data* file_header,
					unsigned int* pshndx)
{
  off_t off = 0;

  file_header->set_address_and_file_offset(0, 0);
  off += file_header->data_size();

  for (Section_list::iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      // We skip unallocated sections here, except that group sections
      // have to come first.
      if (((*p)->flags() & elfcpp::SHF_ALLOC) == 0
	  && (*p)->type() != elfcpp::SHT_GROUP)
	continue;

      off = align_address(off, (*p)->addralign());

      // The linker script might have set the address.
      if (!(*p)->is_address_valid())
	(*p)->set_address(0);
      (*p)->set_file_offset(off);
      (*p)->finalize_data_size();
      if ((*p)->type() != elfcpp::SHT_NOBITS)
	off += (*p)->data_size();

      (*p)->set_out_shndx(*pshndx);
      ++*pshndx;
    }

  return off;
}

// Set the file offset of all the sections not associated with a
// segment.

off_t
Layout::set_section_offsets(off_t off, Layout::Section_offset_pass pass)
{
  off_t startoff = off;
  off_t maxoff = off;

  for (Section_list::iterator p = this->unattached_section_list_.begin();
       p != this->unattached_section_list_.end();
       ++p)
    {
      // The symtab section is handled in create_symtab_sections.
      if (*p == this->symtab_section_)
	continue;

      // If we've already set the data size, don't set it again.
      if ((*p)->is_offset_valid() && (*p)->is_data_size_valid())
	continue;

      if (pass == BEFORE_INPUT_SECTIONS_PASS
	  && (*p)->requires_postprocessing())
	{
	  (*p)->create_postprocessing_buffer();
	  this->any_postprocessing_sections_ = true;
	}

      if (pass == BEFORE_INPUT_SECTIONS_PASS
	  && (*p)->after_input_sections())
	continue;
      else if (pass == POSTPROCESSING_SECTIONS_PASS
	       && (!(*p)->after_input_sections()
		   || (*p)->type() == elfcpp::SHT_STRTAB))
	continue;
      else if (pass == STRTAB_AFTER_POSTPROCESSING_SECTIONS_PASS
	       && (!(*p)->after_input_sections()
		   || (*p)->type() != elfcpp::SHT_STRTAB))
	continue;

      if (!parameters->incremental_update())
	{
	  off = align_address(off, (*p)->addralign());
	  (*p)->set_file_offset(off);
	  (*p)->finalize_data_size();
	}
      else
	{
	  // Incremental update: allocate file space from free list.
	  (*p)->pre_finalize_data_size();
	  off_t current_size = (*p)->current_data_size();
	  off = this->allocate(current_size, (*p)->addralign(), startoff);
	  if (off == -1)
	    {
	      if (is_debugging_enabled(DEBUG_INCREMENTAL))
		this->free_list_.dump();
	      gold_assert((*p)->output_section() != NULL);
	      gold_fallback(_("out of patch space for section %s; "
			      "relink with --incremental-full"),
			    (*p)->output_section()->name());
	    }
	  (*p)->set_file_offset(off);
	  (*p)->finalize_data_size();
	  if ((*p)->data_size() > current_size)
	    {
	      gold_assert((*p)->output_section() != NULL);
	      gold_fallback(_("%s: section changed size; "
			      "relink with --incremental-full"),
			    (*p)->output_section()->name());
	    }
	  gold_debug(DEBUG_INCREMENTAL,
		     "set_section_offsets: %08lx %08lx %s",
		     static_cast<long>(off),
		     static_cast<long>((*p)->data_size()),
		     ((*p)->output_section() != NULL
		      ? (*p)->output_section()->name() : "(special)"));
	}

      off += (*p)->data_size();
      if (off > maxoff)
	maxoff = off;

      // At this point the name must be set.
      if (pass != STRTAB_AFTER_POSTPROCESSING_SECTIONS_PASS)
	this->namepool_.add((*p)->name(), false, NULL);
    }
  return maxoff;
}

// Set the section indexes of all the sections not associated with a
// segment.

unsigned int
Layout::set_section_indexes(unsigned int shndx)
{
  for (Section_list::iterator p = this->unattached_section_list_.begin();
       p != this->unattached_section_list_.end();
       ++p)
    {
      if (!(*p)->has_out_shndx())
	{
	  (*p)->set_out_shndx(shndx);
	  ++shndx;
	}
    }
  return shndx;
}

// Set the section addresses according to the linker script.  This is
// only called when we see a SECTIONS clause.  This returns the
// program segment which should hold the file header and segment
// headers, if any.  It will return NULL if they should not be in a
// segment.

Output_segment*
Layout::set_section_addresses_from_script(Symbol_table* symtab)
{
  Script_sections* ss = this->script_options_->script_sections();
  gold_assert(ss->saw_sections_clause());
  return this->script_options_->set_section_addresses(symtab, this);
}

// Place the orphan sections in the linker script.

void
Layout::place_orphan_sections_in_script()
{
  Script_sections* ss = this->script_options_->script_sections();
  gold_assert(ss->saw_sections_clause());

  // Place each orphaned output section in the script.
  for (Section_list::iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      if (!(*p)->found_in_sections_clause())
	ss->place_orphan(*p);
    }
}

// Count the local symbols in the regular symbol table and the dynamic
// symbol table, and build the respective string pools.

void
Layout::count_local_symbols(const Task* task,
			    const Input_objects* input_objects)
{
  // First, figure out an upper bound on the number of symbols we'll
  // be inserting into each pool.  This helps us create the pools with
  // the right size, to avoid unnecessary hashtable resizing.
  unsigned int symbol_count = 0;
  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    symbol_count += (*p)->local_symbol_count();

  // Go from "upper bound" to "estimate."  We overcount for two
  // reasons: we double-count symbols that occur in more than one
  // object file, and we count symbols that are dropped from the
  // output.  Add it all together and assume we overcount by 100%.
  symbol_count /= 2;

  // We assume all symbols will go into both the sympool and dynpool.
  this->sympool_.reserve(symbol_count);
  this->dynpool_.reserve(symbol_count);

  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      Task_lock_obj<Object> tlo(task, *p);
      (*p)->count_local_symbols(&this->sympool_, &this->dynpool_);
    }
}

// Create the symbol table sections.  Here we also set the final
// values of the symbols.  At this point all the loadable sections are
// fully laid out.  SHNUM is the number of sections so far.

void
Layout::create_symtab_sections(const Input_objects* input_objects,
			       Symbol_table* symtab,
			       unsigned int shnum,
			       off_t* poff,
			       unsigned int local_dynamic_count)
{
  int symsize;
  unsigned int align;
  if (parameters->target().get_size() == 32)
    {
      symsize = elfcpp::Elf_sizes<32>::sym_size;
      align = 4;
    }
  else if (parameters->target().get_size() == 64)
    {
      symsize = elfcpp::Elf_sizes<64>::sym_size;
      align = 8;
    }
  else
    gold_unreachable();

  // Compute file offsets relative to the start of the symtab section.
  off_t off = 0;

  // Save space for the dummy symbol at the start of the section.  We
  // never bother to write this out--it will just be left as zero.
  off += symsize;
  unsigned int local_symbol_index = 1;

  // Add STT_SECTION symbols for each Output section which needs one.
  for (Section_list::iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      if (!(*p)->needs_symtab_index())
	(*p)->set_symtab_index(-1U);
      else
	{
	  (*p)->set_symtab_index(local_symbol_index);
	  ++local_symbol_index;
	  off += symsize;
	}
    }

  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      unsigned int index = (*p)->finalize_local_symbols(local_symbol_index,
							off, symtab);
      off += (index - local_symbol_index) * symsize;
      local_symbol_index = index;
    }

  unsigned int local_symcount = local_symbol_index;
  gold_assert(static_cast<off_t>(local_symcount * symsize) == off);

  off_t dynoff;
  size_t dyncount;
  if (this->dynsym_section_ == NULL)
    {
      dynoff = 0;
      dyncount = 0;
    }
  else
    {
      off_t locsize = local_dynamic_count * this->dynsym_section_->entsize();
      dynoff = this->dynsym_section_->offset() + locsize;
      dyncount = (this->dynsym_section_->data_size() - locsize) / symsize;
      gold_assert(static_cast<off_t>(dyncount * symsize)
		  == this->dynsym_section_->data_size() - locsize);
    }

  off_t global_off = off;
  off = symtab->finalize(off, dynoff, local_dynamic_count, dyncount,
			 &this->sympool_, &local_symcount);

  if (!parameters->options().strip_all())
    {
      this->sympool_.set_string_offsets();

      const char* symtab_name = this->namepool_.add(".symtab", false, NULL);
      Output_section* osymtab = this->make_output_section(symtab_name,
							  elfcpp::SHT_SYMTAB,
							  0, ORDER_INVALID,
							  false);
      this->symtab_section_ = osymtab;

      Output_section_data* pos = new Output_data_fixed_space(off, align,
							     "** symtab");
      osymtab->add_output_section_data(pos);

      // We generate a .symtab_shndx section if we have more than
      // SHN_LORESERVE sections.  Technically it is possible that we
      // don't need one, because it is possible that there are no
      // symbols in any of sections with indexes larger than
      // SHN_LORESERVE.  That is probably unusual, though, and it is
      // easier to always create one than to compute section indexes
      // twice (once here, once when writing out the symbols).
      if (shnum >= elfcpp::SHN_LORESERVE)
	{
	  const char* symtab_xindex_name = this->namepool_.add(".symtab_shndx",
							       false, NULL);
	  Output_section* osymtab_xindex =
	    this->make_output_section(symtab_xindex_name,
				      elfcpp::SHT_SYMTAB_SHNDX, 0,
				      ORDER_INVALID, false);

	  size_t symcount = off / symsize;
	  this->symtab_xindex_ = new Output_symtab_xindex(symcount);

	  osymtab_xindex->add_output_section_data(this->symtab_xindex_);

	  osymtab_xindex->set_link_section(osymtab);
	  osymtab_xindex->set_addralign(4);
	  osymtab_xindex->set_entsize(4);

	  osymtab_xindex->set_after_input_sections();

	  // This tells the driver code to wait until the symbol table
	  // has written out before writing out the postprocessing
	  // sections, including the .symtab_shndx section.
	  this->any_postprocessing_sections_ = true;
	}

      const char* strtab_name = this->namepool_.add(".strtab", false, NULL);
      Output_section* ostrtab = this->make_output_section(strtab_name,
							  elfcpp::SHT_STRTAB,
							  0, ORDER_INVALID,
							  false);

      Output_section_data* pstr = new Output_data_strtab(&this->sympool_);
      ostrtab->add_output_section_data(pstr);

      off_t symtab_off;
      if (!parameters->incremental_update())
	symtab_off = align_address(*poff, align);
      else
	{
	  symtab_off = this->allocate(off, align, *poff);
	  if (off == -1)
	    gold_fallback(_("out of patch space for symbol table; "
			    "relink with --incremental-full"));
	  gold_debug(DEBUG_INCREMENTAL,
		     "create_symtab_sections: %08lx %08lx .symtab",
		     static_cast<long>(symtab_off),
		     static_cast<long>(off));
	}

      symtab->set_file_offset(symtab_off + global_off);
      osymtab->set_file_offset(symtab_off);
      osymtab->finalize_data_size();
      osymtab->set_link_section(ostrtab);
      osymtab->set_info(local_symcount);
      osymtab->set_entsize(symsize);

      if (symtab_off + off > *poff)
	*poff = symtab_off + off;
    }
}

// Create the .shstrtab section, which holds the names of the
// sections.  At the time this is called, we have created all the
// output sections except .shstrtab itself.

Output_section*
Layout::create_shstrtab()
{
  // FIXME: We don't need to create a .shstrtab section if we are
  // stripping everything.

  const char* name = this->namepool_.add(".shstrtab", false, NULL);

  Output_section* os = this->make_output_section(name, elfcpp::SHT_STRTAB, 0,
						 ORDER_INVALID, false);

  if (strcmp(parameters->options().compress_debug_sections(), "none") != 0)
    {
      // We can't write out this section until we've set all the
      // section names, and we don't set the names of compressed
      // output sections until relocations are complete.  FIXME: With
      // the current names we use, this is unnecessary.
      os->set_after_input_sections();
    }

  Output_section_data* posd = new Output_data_strtab(&this->namepool_);
  os->add_output_section_data(posd);

  return os;
}

// Create the section headers.  SIZE is 32 or 64.  OFF is the file
// offset.

void
Layout::create_shdrs(const Output_section* shstrtab_section, off_t* poff)
{
  Output_section_headers* oshdrs;
  oshdrs = new Output_section_headers(this,
				      &this->segment_list_,
				      &this->section_list_,
				      &this->unattached_section_list_,
				      &this->namepool_,
				      shstrtab_section);
  off_t off;
  if (!parameters->incremental_update())
    off = align_address(*poff, oshdrs->addralign());
  else
    {
      oshdrs->pre_finalize_data_size();
      off = this->allocate(oshdrs->data_size(), oshdrs->addralign(), *poff);
      if (off == -1)
	  gold_fallback(_("out of patch space for section header table; "
			  "relink with --incremental-full"));
      gold_debug(DEBUG_INCREMENTAL,
		 "create_shdrs: %08lx %08lx (section header table)",
		 static_cast<long>(off),
		 static_cast<long>(off + oshdrs->data_size()));
    }
  oshdrs->set_address_and_file_offset(0, off);
  off += oshdrs->data_size();
  if (off > *poff)
    *poff = off;
  this->section_headers_ = oshdrs;
}

// Count the allocated sections.

size_t
Layout::allocated_output_section_count() const
{
  size_t section_count = 0;
  for (Segment_list::const_iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    section_count += (*p)->output_section_count();
  return section_count;
}

// Create the dynamic symbol table.
// *PLOCAL_DYNAMIC_COUNT will be set to the number of local symbols
// from input objects, and *PFORCED_LOCAL_DYNAMIC_COUNT will be set
// to the number of global symbols that have been forced local.
// We need to remember the former because the forced-local symbols are
// written along with the global symbols in Symtab::write_globals().

void
Layout::create_dynamic_symtab(const Input_objects* input_objects,
			      Symbol_table* symtab,
			      Output_section** pdynstr,
			      unsigned int* plocal_dynamic_count,
			      unsigned int* pforced_local_dynamic_count,
			      std::vector<Symbol*>* pdynamic_symbols,
			      Versions* pversions)
{
  // Count all the symbols in the dynamic symbol table, and set the
  // dynamic symbol indexes.

  // Skip symbol 0, which is always all zeroes.
  unsigned int index = 1;

  // Add STT_SECTION symbols for each Output section which needs one.
  for (Section_list::iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      if (!(*p)->needs_dynsym_index())
	(*p)->set_dynsym_index(-1U);
      else
	{
	  (*p)->set_dynsym_index(index);
	  ++index;
	}
    }

  // Count the local symbols that need to go in the dynamic symbol table,
  // and set the dynamic symbol indexes.
  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      unsigned int new_index = (*p)->set_local_dynsym_indexes(index);
      index = new_index;
    }

  unsigned int local_symcount = index;
  unsigned int forced_local_count = 0;

  index = symtab->set_dynsym_indexes(index, &forced_local_count,
				     pdynamic_symbols, &this->dynpool_,
				     pversions);

  *plocal_dynamic_count = local_symcount;
  *pforced_local_dynamic_count = forced_local_count;

  int symsize;
  unsigned int align;
  const int size = parameters->target().get_size();
  if (size == 32)
    {
      symsize = elfcpp::Elf_sizes<32>::sym_size;
      align = 4;
    }
  else if (size == 64)
    {
      symsize = elfcpp::Elf_sizes<64>::sym_size;
      align = 8;
    }
  else
    gold_unreachable();

  // Create the dynamic symbol table section.

  Output_section* dynsym = this->choose_output_section(NULL, ".dynsym",
						       elfcpp::SHT_DYNSYM,
						       elfcpp::SHF_ALLOC,
						       false,
						       ORDER_DYNAMIC_LINKER,
						       false, false, false);

  // Check for NULL as a linker script may discard .dynsym.
  if (dynsym != NULL)
    {
      Output_section_data* odata = new Output_data_fixed_space(index * symsize,
							       align,
							       "** dynsym");
      dynsym->add_output_section_data(odata);

      dynsym->set_info(local_symcount + forced_local_count);
      dynsym->set_entsize(symsize);
      dynsym->set_addralign(align);

      this->dynsym_section_ = dynsym;
    }

  Output_data_dynamic* const odyn = this->dynamic_data_;
  if (odyn != NULL)
    {
      odyn->add_section_address(elfcpp::DT_SYMTAB, dynsym);
      odyn->add_constant(elfcpp::DT_SYMENT, symsize);
    }

  // If there are more than SHN_LORESERVE allocated sections, we
  // create a .dynsym_shndx section.  It is possible that we don't
  // need one, because it is possible that there are no dynamic
  // symbols in any of the sections with indexes larger than
  // SHN_LORESERVE.  This is probably unusual, though, and at this
  // time we don't know the actual section indexes so it is
  // inconvenient to check.
  if (this->allocated_output_section_count() >= elfcpp::SHN_LORESERVE)
    {
      Output_section* dynsym_xindex =
	this->choose_output_section(NULL, ".dynsym_shndx",
				    elfcpp::SHT_SYMTAB_SHNDX,
				    elfcpp::SHF_ALLOC,
				    false, ORDER_DYNAMIC_LINKER, false, false,
				    false);

      if (dynsym_xindex != NULL)
	{
	  this->dynsym_xindex_ = new Output_symtab_xindex(index);

	  dynsym_xindex->add_output_section_data(this->dynsym_xindex_);

	  dynsym_xindex->set_link_section(dynsym);
	  dynsym_xindex->set_addralign(4);
	  dynsym_xindex->set_entsize(4);

	  dynsym_xindex->set_after_input_sections();

	  // This tells the driver code to wait until the symbol table
	  // has written out before writing out the postprocessing
	  // sections, including the .dynsym_shndx section.
	  this->any_postprocessing_sections_ = true;
	}
    }

  // Create the dynamic string table section.

  Output_section* dynstr = this->choose_output_section(NULL, ".dynstr",
						       elfcpp::SHT_STRTAB,
						       elfcpp::SHF_ALLOC,
						       false,
						       ORDER_DYNAMIC_LINKER,
						       false, false, false);
  *pdynstr = dynstr;
  if (dynstr != NULL)
    {
      Output_section_data* strdata = new Output_data_strtab(&this->dynpool_);
      dynstr->add_output_section_data(strdata);

      if (dynsym != NULL)
	dynsym->set_link_section(dynstr);
      if (this->dynamic_section_ != NULL)
	this->dynamic_section_->set_link_section(dynstr);

      if (odyn != NULL)
	{
	  odyn->add_section_address(elfcpp::DT_STRTAB, dynstr);
	  odyn->add_section_size(elfcpp::DT_STRSZ, dynstr);
	}
    }

  // Create the hash tables.  The Gnu-style hash table must be
  // built first, because it changes the order of the symbols
  // in the dynamic symbol table.

  if (strcmp(parameters->options().hash_style(), "gnu") == 0
      || strcmp(parameters->options().hash_style(), "both") == 0)
    {
      unsigned char* phash;
      unsigned int hashlen;
      Dynobj::create_gnu_hash_table(*pdynamic_symbols,
				    local_symcount + forced_local_count,
				    &phash, &hashlen);

      Output_section* hashsec =
	this->choose_output_section(NULL, ".gnu.hash", elfcpp::SHT_GNU_HASH,
				    elfcpp::SHF_ALLOC, false,
				    ORDER_DYNAMIC_LINKER, false, false,
				    false);

      Output_section_data* hashdata = new Output_data_const_buffer(phash,
								   hashlen,
								   align,
								   "** hash");
      if (hashsec != NULL && hashdata != NULL)
	hashsec->add_output_section_data(hashdata);

      if (hashsec != NULL)
	{
	  if (dynsym != NULL)
	    hashsec->set_link_section(dynsym);

	  // For a 64-bit target, the entries in .gnu.hash do not have
	  // a uniform size, so we only set the entry size for a
	  // 32-bit target.
	  if (parameters->target().get_size() == 32)
	    hashsec->set_entsize(4);

	  if (odyn != NULL)
	    odyn->add_section_address(elfcpp::DT_GNU_HASH, hashsec);
	}
    }

  if (strcmp(parameters->options().hash_style(), "sysv") == 0
      || strcmp(parameters->options().hash_style(), "both") == 0)
    {
      unsigned char* phash;
      unsigned int hashlen;
      Dynobj::create_elf_hash_table(*pdynamic_symbols,
				    local_symcount + forced_local_count,
				    &phash, &hashlen);

      Output_section* hashsec =
	this->choose_output_section(NULL, ".hash", elfcpp::SHT_HASH,
				    elfcpp::SHF_ALLOC, false,
				    ORDER_DYNAMIC_LINKER, false, false,
				    false);

      Output_section_data* hashdata = new Output_data_const_buffer(phash,
								   hashlen,
								   align,
								   "** hash");
      if (hashsec != NULL && hashdata != NULL)
	hashsec->add_output_section_data(hashdata);

      if (hashsec != NULL)
	{
	  if (dynsym != NULL)
	    hashsec->set_link_section(dynsym);
	  hashsec->set_entsize(parameters->target().hash_entry_size() / 8);
	}

      if (odyn != NULL)
	odyn->add_section_address(elfcpp::DT_HASH, hashsec);
    }
}

// Assign offsets to each local portion of the dynamic symbol table.

void
Layout::assign_local_dynsym_offsets(const Input_objects* input_objects)
{
  Output_section* dynsym = this->dynsym_section_;
  if (dynsym == NULL)
    return;

  off_t off = dynsym->offset();

  // Skip the dummy symbol at the start of the section.
  off += dynsym->entsize();

  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      unsigned int count = (*p)->set_local_dynsym_offset(off);
      off += count * dynsym->entsize();
    }
}

// Create the version sections.

void
Layout::create_version_sections(const Versions* versions,
				const Symbol_table* symtab,
				unsigned int local_symcount,
				const std::vector<Symbol*>& dynamic_symbols,
				const Output_section* dynstr)
{
  if (!versions->any_defs() && !versions->any_needs())
    return;

  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      this->sized_create_version_sections<32, false>(versions, symtab,
						     local_symcount,
						     dynamic_symbols, dynstr);
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      this->sized_create_version_sections<32, true>(versions, symtab,
						    local_symcount,
						    dynamic_symbols, dynstr);
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      this->sized_create_version_sections<64, false>(versions, symtab,
						     local_symcount,
						     dynamic_symbols, dynstr);
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      this->sized_create_version_sections<64, true>(versions, symtab,
						    local_symcount,
						    dynamic_symbols, dynstr);
      break;
#endif
    default:
      gold_unreachable();
    }
}

// Create the version sections, sized version.

template<int size, bool big_endian>
void
Layout::sized_create_version_sections(
    const Versions* versions,
    const Symbol_table* symtab,
    unsigned int local_symcount,
    const std::vector<Symbol*>& dynamic_symbols,
    const Output_section* dynstr)
{
  Output_section* vsec = this->choose_output_section(NULL, ".gnu.version",
						     elfcpp::SHT_GNU_versym,
						     elfcpp::SHF_ALLOC,
						     false,
						     ORDER_DYNAMIC_LINKER,
						     false, false, false);

  // Check for NULL since a linker script may discard this section.
  if (vsec != NULL)
    {
      unsigned char* vbuf;
      unsigned int vsize;
      versions->symbol_section_contents<size, big_endian>(symtab,
							  &this->dynpool_,
							  local_symcount,
							  dynamic_symbols,
							  &vbuf, &vsize);

      Output_section_data* vdata = new Output_data_const_buffer(vbuf, vsize, 2,
								"** versions");

      vsec->add_output_section_data(vdata);
      vsec->set_entsize(2);
      vsec->set_link_section(this->dynsym_section_);
    }

  Output_data_dynamic* const odyn = this->dynamic_data_;
  if (odyn != NULL && vsec != NULL)
    odyn->add_section_address(elfcpp::DT_VERSYM, vsec);

  if (versions->any_defs())
    {
      Output_section* vdsec;
      vdsec = this->choose_output_section(NULL, ".gnu.version_d",
					  elfcpp::SHT_GNU_verdef,
					  elfcpp::SHF_ALLOC,
					  false, ORDER_DYNAMIC_LINKER, false,
					  false, false);

      if (vdsec != NULL)
	{
	  unsigned char* vdbuf;
	  unsigned int vdsize;
	  unsigned int vdentries;
	  versions->def_section_contents<size, big_endian>(&this->dynpool_,
							   &vdbuf, &vdsize,
							   &vdentries);

	  Output_section_data* vddata =
	    new Output_data_const_buffer(vdbuf, vdsize, 4, "** version defs");

	  vdsec->add_output_section_data(vddata);
	  vdsec->set_link_section(dynstr);
	  vdsec->set_info(vdentries);

	  if (odyn != NULL)
	    {
	      odyn->add_section_address(elfcpp::DT_VERDEF, vdsec);
	      odyn->add_constant(elfcpp::DT_VERDEFNUM, vdentries);
	    }
	}
    }

  if (versions->any_needs())
    {
      Output_section* vnsec;
      vnsec = this->choose_output_section(NULL, ".gnu.version_r",
					  elfcpp::SHT_GNU_verneed,
					  elfcpp::SHF_ALLOC,
					  false, ORDER_DYNAMIC_LINKER, false,
					  false, false);

      if (vnsec != NULL)
	{
	  unsigned char* vnbuf;
	  unsigned int vnsize;
	  unsigned int vnentries;
	  versions->need_section_contents<size, big_endian>(&this->dynpool_,
							    &vnbuf, &vnsize,
							    &vnentries);

	  Output_section_data* vndata =
	    new Output_data_const_buffer(vnbuf, vnsize, 4, "** version refs");

	  vnsec->add_output_section_data(vndata);
	  vnsec->set_link_section(dynstr);
	  vnsec->set_info(vnentries);

	  if (odyn != NULL)
	    {
	      odyn->add_section_address(elfcpp::DT_VERNEED, vnsec);
	      odyn->add_constant(elfcpp::DT_VERNEEDNUM, vnentries);
	    }
	}
    }
}

// Create the .interp section and PT_INTERP segment.

void
Layout::create_interp(const Target* target)
{
  gold_assert(this->interp_segment_ == NULL);

  const char* interp = parameters->options().dynamic_linker();
  if (interp == NULL)
    {
      interp = target->dynamic_linker();
      gold_assert(interp != NULL);
    }

  size_t len = strlen(interp) + 1;

  Output_section_data* odata = new Output_data_const(interp, len, 1);

  Output_section* osec = this->choose_output_section(NULL, ".interp",
						     elfcpp::SHT_PROGBITS,
						     elfcpp::SHF_ALLOC,
						     false, ORDER_INTERP,
						     false, false, false);
  if (osec != NULL)
    osec->add_output_section_data(odata);
}

// Add dynamic tags for the PLT and the dynamic relocs.  This is
// called by the target-specific code.  This does nothing if not doing
// a dynamic link.

// USE_REL is true for REL relocs rather than RELA relocs.

// If PLT_GOT is not NULL, then DT_PLTGOT points to it.

// If PLT_REL is not NULL, it is used for DT_PLTRELSZ, and DT_JMPREL,
// and we also set DT_PLTREL.  We use PLT_REL's output section, since
// some targets have multiple reloc sections in PLT_REL.

// If DYN_REL is not NULL, it is used for DT_REL/DT_RELA,
// DT_RELSZ/DT_RELASZ, DT_RELENT/DT_RELAENT.  Again we use the output
// section.

// If ADD_DEBUG is true, we add a DT_DEBUG entry when generating an
// executable.

void
Layout::add_target_dynamic_tags(bool use_rel, const Output_data* plt_got,
				const Output_data* plt_rel,
				const Output_data_reloc_generic* dyn_rel,
				bool add_debug, bool dynrel_includes_plt,
				bool custom_relcount)
{
  Output_data_dynamic* odyn = this->dynamic_data_;
  if (odyn == NULL)
    return;

  if (plt_got != NULL && plt_got->output_section() != NULL)
    odyn->add_section_address(elfcpp::DT_PLTGOT, plt_got);

  if (plt_rel != NULL && plt_rel->output_section() != NULL)
    {
      odyn->add_section_size(elfcpp::DT_PLTRELSZ, plt_rel->output_section());
      odyn->add_section_address(elfcpp::DT_JMPREL, plt_rel->output_section());
      odyn->add_constant(elfcpp::DT_PLTREL,
			 use_rel ? elfcpp::DT_REL : elfcpp::DT_RELA);
    }

  if ((dyn_rel != NULL && dyn_rel->output_section() != NULL)
      || (dynrel_includes_plt
	  && plt_rel != NULL
	  && plt_rel->output_section() != NULL))
    {
      bool have_dyn_rel = dyn_rel != NULL && dyn_rel->output_section() != NULL;
      bool have_plt_rel = plt_rel != NULL && plt_rel->output_section() != NULL;
      odyn->add_section_address(use_rel ? elfcpp::DT_REL : elfcpp::DT_RELA,
				(have_dyn_rel
				 ? dyn_rel->output_section()
				 : plt_rel->output_section()));
      elfcpp::DT size_tag = use_rel ? elfcpp::DT_RELSZ : elfcpp::DT_RELASZ;
      if (have_dyn_rel && have_plt_rel && dynrel_includes_plt)
	odyn->add_section_size(size_tag,
			       dyn_rel->output_section(),
			       plt_rel->output_section());
      else if (have_dyn_rel)
	odyn->add_section_size(size_tag, dyn_rel->output_section());
      else
	odyn->add_section_size(size_tag, plt_rel->output_section());
      const int size = parameters->target().get_size();
      elfcpp::DT rel_tag;
      int rel_size;
      if (use_rel)
	{
	  rel_tag = elfcpp::DT_RELENT;
	  if (size == 32)
	    rel_size = Reloc_types<elfcpp::SHT_REL, 32, false>::reloc_size;
	  else if (size == 64)
	    rel_size = Reloc_types<elfcpp::SHT_REL, 64, false>::reloc_size;
	  else
	    gold_unreachable();
	}
      else
	{
	  rel_tag = elfcpp::DT_RELAENT;
	  if (size == 32)
	    rel_size = Reloc_types<elfcpp::SHT_RELA, 32, false>::reloc_size;
	  else if (size == 64)
	    rel_size = Reloc_types<elfcpp::SHT_RELA, 64, false>::reloc_size;
	  else
	    gold_unreachable();
	}
      odyn->add_constant(rel_tag, rel_size);

      if (parameters->options().combreloc() && have_dyn_rel)
	{
	  size_t c = dyn_rel->relative_reloc_count();
	  if (c != 0)
	    {
	      elfcpp::DT tag
		= use_rel ? elfcpp::DT_RELCOUNT : elfcpp::DT_RELACOUNT;
	      if (custom_relcount)
		odyn->add_custom(tag);
	      else
		odyn->add_constant(tag, c);
	    }
	}
    }

  if (add_debug && !parameters->options().shared())
    {
      // The value of the DT_DEBUG tag is filled in by the dynamic
      // linker at run time, and used by the debugger.
      odyn->add_constant(elfcpp::DT_DEBUG, 0);
    }
}

void
Layout::add_target_specific_dynamic_tag(elfcpp::DT tag, unsigned int val)
{
  Output_data_dynamic* odyn = this->dynamic_data_;
  if (odyn == NULL)
    return;
  odyn->add_constant(tag, val);
}

// Finish the .dynamic section and PT_DYNAMIC segment.

void
Layout::finish_dynamic_section(const Input_objects* input_objects,
			       const Symbol_table* symtab)
{
  if (!this->script_options_->saw_phdrs_clause()
      && this->dynamic_section_ != NULL)
    {
      Output_segment* oseg = this->make_output_segment(elfcpp::PT_DYNAMIC,
						       (elfcpp::PF_R
							| elfcpp::PF_W));
      oseg->add_output_section_to_nonload(this->dynamic_section_,
					  elfcpp::PF_R | elfcpp::PF_W);
    }

  Output_data_dynamic* const odyn = this->dynamic_data_;
  if (odyn == NULL)
    return;

  for (Input_objects::Dynobj_iterator p = input_objects->dynobj_begin();
       p != input_objects->dynobj_end();
       ++p)
    {
      if (!(*p)->is_needed() && (*p)->as_needed())
	{
	  // This dynamic object was linked with --as-needed, but it
	  // is not needed.
	  continue;
	}

      odyn->add_string(elfcpp::DT_NEEDED, (*p)->soname());
    }

  if (parameters->options().shared())
    {
      const char* soname = parameters->options().soname();
      if (soname != NULL)
	odyn->add_string(elfcpp::DT_SONAME, soname);
    }

  Symbol* sym = symtab->lookup(parameters->options().init());
  if (sym != NULL && sym->is_defined() && !sym->is_from_dynobj())
    odyn->add_symbol(elfcpp::DT_INIT, sym);

  sym = symtab->lookup(parameters->options().fini());
  if (sym != NULL && sym->is_defined() && !sym->is_from_dynobj())
    odyn->add_symbol(elfcpp::DT_FINI, sym);

  // Look for .init_array, .preinit_array and .fini_array by checking
  // section types.
  for(Layout::Section_list::const_iterator p = this->section_list_.begin();
      p != this->section_list_.end();
      ++p)
    switch((*p)->type())
      {
      case elfcpp::SHT_FINI_ARRAY:
	odyn->add_section_address(elfcpp::DT_FINI_ARRAY, *p);
	odyn->add_section_size(elfcpp::DT_FINI_ARRAYSZ, *p);
	break;
      case elfcpp::SHT_INIT_ARRAY:
	odyn->add_section_address(elfcpp::DT_INIT_ARRAY, *p);
	odyn->add_section_size(elfcpp::DT_INIT_ARRAYSZ, *p);
	break;
      case elfcpp::SHT_PREINIT_ARRAY:
	odyn->add_section_address(elfcpp::DT_PREINIT_ARRAY, *p);
	odyn->add_section_size(elfcpp::DT_PREINIT_ARRAYSZ, *p);
	break;
      default:
	break;
      }

  // Add a DT_RPATH entry if needed.
  const General_options::Dir_list& rpath(parameters->options().rpath());
  if (!rpath.empty())
    {
      std::string rpath_val;
      for (General_options::Dir_list::const_iterator p = rpath.begin();
	   p != rpath.end();
	   ++p)
	{
	  if (rpath_val.empty())
	    rpath_val = p->name();
	  else
	    {
	      // Eliminate duplicates.
	      General_options::Dir_list::const_iterator q;
	      for (q = rpath.begin(); q != p; ++q)
		if (q->name() == p->name())
		  break;
	      if (q == p)
		{
		  rpath_val += ':';
		  rpath_val += p->name();
		}
	    }
	}

      if (!parameters->options().enable_new_dtags())
	odyn->add_string(elfcpp::DT_RPATH, rpath_val);
      else
	odyn->add_string(elfcpp::DT_RUNPATH, rpath_val);
    }

  // Look for text segments that have dynamic relocations.
  bool have_textrel = false;
  if (!this->script_options_->saw_sections_clause())
    {
      for (Segment_list::const_iterator p = this->segment_list_.begin();
	   p != this->segment_list_.end();
	   ++p)
	{
	  if ((*p)->type() == elfcpp::PT_LOAD
	      && ((*p)->flags() & elfcpp::PF_W) == 0
	      && (*p)->has_dynamic_reloc())
	    {
	      have_textrel = true;
	      break;
	    }
	}
    }
  else
    {
      // We don't know the section -> segment mapping, so we are
      // conservative and just look for readonly sections with
      // relocations.  If those sections wind up in writable segments,
      // then we have created an unnecessary DT_TEXTREL entry.
      for (Section_list::const_iterator p = this->section_list_.begin();
	   p != this->section_list_.end();
	   ++p)
	{
	  if (((*p)->flags() & elfcpp::SHF_ALLOC) != 0
	      && ((*p)->flags() & elfcpp::SHF_WRITE) == 0
	      && (*p)->has_dynamic_reloc())
	    {
	      have_textrel = true;
	      break;
	    }
	}
    }

  if (parameters->options().filter() != NULL)
    odyn->add_string(elfcpp::DT_FILTER, parameters->options().filter());
  if (parameters->options().any_auxiliary())
    {
      for (options::String_set::const_iterator p =
	     parameters->options().auxiliary_begin();
	   p != parameters->options().auxiliary_end();
	   ++p)
	odyn->add_string(elfcpp::DT_AUXILIARY, *p);
    }

  // Add a DT_FLAGS entry if necessary.
  unsigned int flags = 0;
  if (have_textrel)
    {
      // Add a DT_TEXTREL for compatibility with older loaders.
      odyn->add_constant(elfcpp::DT_TEXTREL, 0);
      flags |= elfcpp::DF_TEXTREL;

      if (parameters->options().text())
	gold_error(_("read-only segment has dynamic relocations"));
      else if (parameters->options().warn_shared_textrel()
	       && parameters->options().shared())
	gold_warning(_("shared library text segment is not shareable"));
    }
  if (parameters->options().shared() && this->has_static_tls())
    flags |= elfcpp::DF_STATIC_TLS;
  if (parameters->options().origin())
    flags |= elfcpp::DF_ORIGIN;
  if (parameters->options().Bsymbolic()
      && !parameters->options().have_dynamic_list())
    {
      flags |= elfcpp::DF_SYMBOLIC;
      // Add DT_SYMBOLIC for compatibility with older loaders.
      odyn->add_constant(elfcpp::DT_SYMBOLIC, 0);
    }
  if (parameters->options().now())
    flags |= elfcpp::DF_BIND_NOW;
  if (flags != 0)
    odyn->add_constant(elfcpp::DT_FLAGS, flags);

  flags = 0;
  if (parameters->options().global())
    flags |= elfcpp::DF_1_GLOBAL;
  if (parameters->options().initfirst())
    flags |= elfcpp::DF_1_INITFIRST;
  if (parameters->options().interpose())
    flags |= elfcpp::DF_1_INTERPOSE;
  if (parameters->options().loadfltr())
    flags |= elfcpp::DF_1_LOADFLTR;
  if (parameters->options().nodefaultlib())
    flags |= elfcpp::DF_1_NODEFLIB;
  if (parameters->options().nodelete())
    flags |= elfcpp::DF_1_NODELETE;
  if (parameters->options().nodlopen())
    flags |= elfcpp::DF_1_NOOPEN;
  if (parameters->options().nodump())
    flags |= elfcpp::DF_1_NODUMP;
  if (!parameters->options().shared())
    flags &= ~(elfcpp::DF_1_INITFIRST
	       | elfcpp::DF_1_NODELETE
	       | elfcpp::DF_1_NOOPEN);
  if (parameters->options().origin())
    flags |= elfcpp::DF_1_ORIGIN;
  if (parameters->options().now())
    flags |= elfcpp::DF_1_NOW;
  if (parameters->options().Bgroup())
    flags |= elfcpp::DF_1_GROUP;
  if (parameters->options().pie())
    flags |= elfcpp::DF_1_PIE;
  if (flags != 0)
    odyn->add_constant(elfcpp::DT_FLAGS_1, flags);

  flags = 0;
  if (parameters->options().unique())
    flags |= elfcpp::DF_GNU_1_UNIQUE;
  if (flags != 0)
    odyn->add_constant(elfcpp::DT_GNU_FLAGS_1, flags);
}

// Set the size of the _DYNAMIC symbol table to be the size of the
// dynamic data.

void
Layout::set_dynamic_symbol_size(const Symbol_table* symtab)
{
  Output_data_dynamic* const odyn = this->dynamic_data_;
  if (odyn == NULL)
    return;
  odyn->finalize_data_size();
  if (this->dynamic_symbol_ == NULL)
    return;
  off_t data_size = odyn->data_size();
  const int size = parameters->target().get_size();
  if (size == 32)
    symtab->get_sized_symbol<32>(this->dynamic_symbol_)->set_symsize(data_size);
  else if (size == 64)
    symtab->get_sized_symbol<64>(this->dynamic_symbol_)->set_symsize(data_size);
  else
    gold_unreachable();
}

// The mapping of input section name prefixes to output section names.
// In some cases one prefix is itself a prefix of another prefix; in
// such a case the longer prefix must come first.  These prefixes are
// based on the GNU linker default ELF linker script.

#define MAPPING_INIT(f, t) { f, sizeof(f) - 1, t, sizeof(t) - 1 }
#define MAPPING_INIT_EXACT(f, t) { f, 0, t, sizeof(t) - 1 }
const Layout::Section_name_mapping Layout::section_name_mapping[] =
{
  MAPPING_INIT(".text.", ".text"),
  MAPPING_INIT(".rodata.", ".rodata"),
  MAPPING_INIT(".data.rel.ro.local.", ".data.rel.ro.local"),
  MAPPING_INIT_EXACT(".data.rel.ro.local", ".data.rel.ro.local"),
  MAPPING_INIT(".data.rel.ro.", ".data.rel.ro"),
  MAPPING_INIT_EXACT(".data.rel.ro", ".data.rel.ro"),
  MAPPING_INIT(".data.", ".data"),
  MAPPING_INIT(".bss.", ".bss"),
  MAPPING_INIT(".tdata.", ".tdata"),
  MAPPING_INIT(".tbss.", ".tbss"),
  MAPPING_INIT(".init_array.", ".init_array"),
  MAPPING_INIT(".fini_array.", ".fini_array"),
  MAPPING_INIT(".sdata.", ".sdata"),
  MAPPING_INIT(".sbss.", ".sbss"),
  // FIXME: In the GNU linker, .sbss2 and .sdata2 are handled
  // differently depending on whether it is creating a shared library.
  MAPPING_INIT(".sdata2.", ".sdata"),
  MAPPING_INIT(".sbss2.", ".sbss"),
  MAPPING_INIT(".lrodata.", ".lrodata"),
  MAPPING_INIT(".ldata.", ".ldata"),
  MAPPING_INIT(".lbss.", ".lbss"),
  MAPPING_INIT(".gcc_except_table.", ".gcc_except_table"),
  MAPPING_INIT(".gnu.linkonce.d.rel.ro.local.", ".data.rel.ro.local"),
  MAPPING_INIT(".gnu.linkonce.d.rel.ro.", ".data.rel.ro"),
  MAPPING_INIT(".gnu.linkonce.t.", ".text"),
  MAPPING_INIT(".gnu.linkonce.r.", ".rodata"),
  MAPPING_INIT(".gnu.linkonce.d.", ".data"),
  MAPPING_INIT(".gnu.linkonce.b.", ".bss"),
  MAPPING_INIT(".gnu.linkonce.s.", ".sdata"),
  MAPPING_INIT(".gnu.linkonce.sb.", ".sbss"),
  MAPPING_INIT(".gnu.linkonce.s2.", ".sdata"),
  MAPPING_INIT(".gnu.linkonce.sb2.", ".sbss"),
  MAPPING_INIT(".gnu.linkonce.wi.", ".debug_info"),
  MAPPING_INIT(".gnu.linkonce.td.", ".tdata"),
  MAPPING_INIT(".gnu.linkonce.tb.", ".tbss"),
  MAPPING_INIT(".gnu.linkonce.lr.", ".lrodata"),
  MAPPING_INIT(".gnu.linkonce.l.", ".ldata"),
  MAPPING_INIT(".gnu.linkonce.lb.", ".lbss"),
  MAPPING_INIT(".ARM.extab", ".ARM.extab"),
  MAPPING_INIT(".gnu.linkonce.armextab.", ".ARM.extab"),
  MAPPING_INIT(".ARM.exidx", ".ARM.exidx"),
  MAPPING_INIT(".gnu.linkonce.armexidx.", ".ARM.exidx"),
  MAPPING_INIT(".gnu.build.attributes.", ".gnu.build.attributes"),
};

// Mapping for ".text" section prefixes with -z,keep-text-section-prefix.
const Layout::Section_name_mapping Layout::text_section_name_mapping[] =
{
  MAPPING_INIT(".text.hot.", ".text.hot"),
  MAPPING_INIT_EXACT(".text.hot", ".text.hot"),
  MAPPING_INIT(".text.unlikely.", ".text.unlikely"),
  MAPPING_INIT_EXACT(".text.unlikely", ".text.unlikely"),
  MAPPING_INIT(".text.startup.", ".text.startup"),
  MAPPING_INIT_EXACT(".text.startup", ".text.startup"),
  MAPPING_INIT(".text.exit.", ".text.exit"),
  MAPPING_INIT_EXACT(".text.exit", ".text.exit"),
  MAPPING_INIT(".text.", ".text"),
};
#undef MAPPING_INIT
#undef MAPPING_INIT_EXACT

const int Layout::section_name_mapping_count =
  (sizeof(Layout::section_name_mapping)
   / sizeof(Layout::section_name_mapping[0]));

const int Layout::text_section_name_mapping_count =
  (sizeof(Layout::text_section_name_mapping)
   / sizeof(Layout::text_section_name_mapping[0]));

// Find section name NAME in PSNM and return the mapped name if found
// with the length set in PLEN.
const char *
Layout::match_section_name(const Layout::Section_name_mapping* psnm,
			   const int count,
			   const char* name, size_t* plen)
{
  for (int i = 0; i < count; ++i, ++psnm)
    {
      if (psnm->fromlen > 0)
	{
	  if (strncmp(name, psnm->from, psnm->fromlen) == 0)
	    {
	      *plen = psnm->tolen;
	      return psnm->to;
	    }
	}
      else
	{
	  if (strcmp(name, psnm->from) == 0)
	    {
	      *plen = psnm->tolen;
	      return psnm->to;
	    }
	}
    }
  return NULL;
}

// Choose the output section name to use given an input section name.
// Set *PLEN to the length of the name.  *PLEN is initialized to the
// length of NAME.

const char*
Layout::output_section_name(const Relobj* relobj, const char* name,
			    size_t* plen)
{
  // gcc 4.3 generates the following sorts of section names when it
  // needs a section name specific to a function:
  //   .text.FN
  //   .rodata.FN
  //   .sdata2.FN
  //   .data.FN
  //   .data.rel.FN
  //   .data.rel.local.FN
  //   .data.rel.ro.FN
  //   .data.rel.ro.local.FN
  //   .sdata.FN
  //   .bss.FN
  //   .sbss.FN
  //   .tdata.FN
  //   .tbss.FN

  // The GNU linker maps all of those to the part before the .FN,
  // except that .data.rel.local.FN is mapped to .data, and
  // .data.rel.ro.local.FN is mapped to .data.rel.ro.  The sections
  // beginning with .data.rel.ro.local are grouped together.

  // For an anonymous namespace, the string FN can contain a '.'.

  // Also of interest: .rodata.strN.N, .rodata.cstN, both of which the
  // GNU linker maps to .rodata.

  // The .data.rel.ro sections are used with -z relro.  The sections
  // are recognized by name.  We use the same names that the GNU
  // linker does for these sections.

  // It is hard to handle this in a principled way, so we don't even
  // try.  We use a table of mappings.  If the input section name is
  // not found in the table, we simply use it as the output section
  // name.

  if (parameters->options().keep_text_section_prefix()
      && is_prefix_of(".text", name))
    {
      const char* match = match_section_name(text_section_name_mapping,
					     text_section_name_mapping_count,
					     name, plen);
      if (match != NULL)
	return match;
    }

  const char* match = match_section_name(section_name_mapping,
					 section_name_mapping_count, name, plen);
  if (match != NULL)
    return match;

  // As an additional complication, .ctors sections are output in
  // either .ctors or .init_array sections, and .dtors sections are
  // output in either .dtors or .fini_array sections.
  if (is_prefix_of(".ctors.", name) || is_prefix_of(".dtors.", name))
    {
      if (parameters->options().ctors_in_init_array())
	{
	  *plen = 11;
	  return name[1] == 'c' ? ".init_array" : ".fini_array";
	}
      else
	{
	  *plen = 6;
	  return name[1] == 'c' ? ".ctors" : ".dtors";
	}
    }
  if (parameters->options().ctors_in_init_array()
      && (strcmp(name, ".ctors") == 0 || strcmp(name, ".dtors") == 0))
    {
      // To make .init_array/.fini_array work with gcc we must exclude
      // .ctors and .dtors sections from the crtbegin and crtend
      // files.
      if (relobj == NULL
	  || (!Layout::match_file_name(relobj, "crtbegin")
	      && !Layout::match_file_name(relobj, "crtend")))
	{
	  *plen = 11;
	  return name[1] == 'c' ? ".init_array" : ".fini_array";
	}
    }

  return name;
}

// Return true if RELOBJ is an input file whose base name matches
// FILE_NAME.  The base name must have an extension of ".o", and must
// be exactly FILE_NAME.o or FILE_NAME, one character, ".o".  This is
// to match crtbegin.o as well as crtbeginS.o without getting confused
// by other possibilities.  Overall matching the file name this way is
// a dreadful hack, but the GNU linker does it in order to better
// support gcc, and we need to be compatible.

bool
Layout::match_file_name(const Relobj* relobj, const char* match)
{
  const std::string& file_name(relobj->name());
  const char* base_name = lbasename(file_name.c_str());
  size_t match_len = strlen(match);
  if (strncmp(base_name, match, match_len) != 0)
    return false;
  size_t base_len = strlen(base_name);
  if (base_len != match_len + 2 && base_len != match_len + 3)
    return false;
  return memcmp(base_name + base_len - 2, ".o", 2) == 0;
}

// Check if a comdat group or .gnu.linkonce section with the given
// NAME is selected for the link.  If there is already a section,
// *KEPT_SECTION is set to point to the existing section and the
// function returns false.  Otherwise, OBJECT, SHNDX, IS_COMDAT, and
// IS_GROUP_NAME are recorded for this NAME in the layout object,
// *KEPT_SECTION is set to the internal copy and the function returns
// true.

bool
Layout::find_or_add_kept_section(const std::string& name,
				 Relobj* object,
				 unsigned int shndx,
				 bool is_comdat,
				 bool is_group_name,
				 Kept_section** kept_section)
{
  // It's normal to see a couple of entries here, for the x86 thunk
  // sections.  If we see more than a few, we're linking a C++
  // program, and we resize to get more space to minimize rehashing.
  if (this->signatures_.size() > 4
      && !this->resized_signatures_)
    {
      reserve_unordered_map(&this->signatures_,
			    this->number_of_input_files_ * 64);
      this->resized_signatures_ = true;
    }

  Kept_section candidate;
  std::pair<Signatures::iterator, bool> ins =
    this->signatures_.insert(std::make_pair(name, candidate));

  if (kept_section != NULL)
    *kept_section = &ins.first->second;
  if (ins.second)
    {
      // This is the first time we've seen this signature.
      ins.first->second.set_object(object);
      ins.first->second.set_shndx(shndx);
      if (is_comdat)
	ins.first->second.set_is_comdat();
      if (is_group_name)
	ins.first->second.set_is_group_name();
      return true;
    }

  // We have already seen this signature.

  if (ins.first->second.is_group_name())
    {
      // We've already seen a real section group with this signature.
      // If the kept group is from a plugin object, and we're in the
      // replacement phase, accept the new one as a replacement.
      if (ins.first->second.object() == NULL
	  && parameters->options().plugins()->in_replacement_phase())
	{
	  ins.first->second.set_object(object);
	  ins.first->second.set_shndx(shndx);
	  return true;
	}
      return false;
    }
  else if (is_group_name)
    {
      // This is a real section group, and we've already seen a
      // linkonce section with this signature.  Record that we've seen
      // a section group, and don't include this section group.
      ins.first->second.set_is_group_name();
      return false;
    }
  else
    {
      // We've already seen a linkonce section and this is a linkonce
      // section.  These don't block each other--this may be the same
      // symbol name with different section types.
      return true;
    }
}

// Store the allocated sections into the section list.

void
Layout::get_allocated_sections(Section_list* section_list) const
{
  for (Section_list::const_iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    if (((*p)->flags() & elfcpp::SHF_ALLOC) != 0)
      section_list->push_back(*p);
}

// Store the executable sections into the section list.

void
Layout::get_executable_sections(Section_list* section_list) const
{
  for (Section_list::const_iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    if (((*p)->flags() & (elfcpp::SHF_ALLOC | elfcpp::SHF_EXECINSTR))
	== (elfcpp::SHF_ALLOC | elfcpp::SHF_EXECINSTR))
      section_list->push_back(*p);
}

// Create an output segment.

Output_segment*
Layout::make_output_segment(elfcpp::Elf_Word type, elfcpp::Elf_Word flags)
{
  gold_assert(!parameters->options().relocatable());
  Output_segment* oseg = new Output_segment(type, flags);
  this->segment_list_.push_back(oseg);

  if (type == elfcpp::PT_TLS)
    this->tls_segment_ = oseg;
  else if (type == elfcpp::PT_GNU_RELRO)
    this->relro_segment_ = oseg;
  else if (type == elfcpp::PT_INTERP)
    this->interp_segment_ = oseg;

  return oseg;
}

// Return the file offset of the normal symbol table.

off_t
Layout::symtab_section_offset() const
{
  if (this->symtab_section_ != NULL)
    return this->symtab_section_->offset();
  return 0;
}

// Return the section index of the normal symbol table.  It may have
// been stripped by the -s/--strip-all option.

unsigned int
Layout::symtab_section_shndx() const
{
  if (this->symtab_section_ != NULL)
    return this->symtab_section_->out_shndx();
  return 0;
}

// Write out the Output_sections.  Most won't have anything to write,
// since most of the data will come from input sections which are
// handled elsewhere.  But some Output_sections do have Output_data.

void
Layout::write_output_sections(Output_file* of) const
{
  for (Section_list::const_iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      if (!(*p)->after_input_sections())
	(*p)->write(of);
    }
}

// Write out data not associated with a section or the symbol table.

void
Layout::write_data(const Symbol_table* symtab, Output_file* of) const
{
  if (!parameters->options().strip_all())
    {
      const Output_section* symtab_section = this->symtab_section_;
      for (Section_list::const_iterator p = this->section_list_.begin();
	   p != this->section_list_.end();
	   ++p)
	{
	  if ((*p)->needs_symtab_index())
	    {
	      gold_assert(symtab_section != NULL);
	      unsigned int index = (*p)->symtab_index();
	      gold_assert(index > 0 && index != -1U);
	      off_t off = (symtab_section->offset()
			   + index * symtab_section->entsize());
	      symtab->write_section_symbol(*p, this->symtab_xindex_, of, off);
	    }
	}
    }

  const Output_section* dynsym_section = this->dynsym_section_;
  for (Section_list::const_iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      if ((*p)->needs_dynsym_index())
	{
	  gold_assert(dynsym_section != NULL);
	  unsigned int index = (*p)->dynsym_index();
	  gold_assert(index > 0 && index != -1U);
	  off_t off = (dynsym_section->offset()
		       + index * dynsym_section->entsize());
	  symtab->write_section_symbol(*p, this->dynsym_xindex_, of, off);
	}
    }

  // Write out the Output_data which are not in an Output_section.
  for (Data_list::const_iterator p = this->special_output_list_.begin();
       p != this->special_output_list_.end();
       ++p)
    (*p)->write(of);

  // Write out the Output_data which are not in an Output_section
  // and are regenerated in each iteration of relaxation.
  for (Data_list::const_iterator p = this->relax_output_list_.begin();
       p != this->relax_output_list_.end();
       ++p)
    (*p)->write(of);
}

// Write out the Output_sections which can only be written after the
// input sections are complete.

void
Layout::write_sections_after_input_sections(Output_file* of)
{
  // Determine the final section offsets, and thus the final output
  // file size.  Note we finalize the .shstrab last, to allow the
  // after_input_section sections to modify their section-names before
  // writing.
  if (this->any_postprocessing_sections_)
    {
      off_t off = this->output_file_size_;
      off = this->set_section_offsets(off, POSTPROCESSING_SECTIONS_PASS);

      // Now that we've finalized the names, we can finalize the shstrab.
      off =
	this->set_section_offsets(off,
				  STRTAB_AFTER_POSTPROCESSING_SECTIONS_PASS);

      if (off > this->output_file_size_)
	{
	  of->resize(off);
	  this->output_file_size_ = off;
	}
    }

  for (Section_list::const_iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    {
      if ((*p)->after_input_sections())
	(*p)->write(of);
    }

  this->section_headers_->write(of);
}

// If a tree-style build ID was requested, the parallel part of that computation
// is already done, and the final hash-of-hashes is computed here.  For other
// types of build IDs, all the work is done here.

void
Layout::write_build_id(Output_file* of, unsigned char* array_of_hashes,
		       size_t size_of_hashes) const
{
  if (this->build_id_note_ == NULL)
    return;

  unsigned char* ov = of->get_output_view(this->build_id_note_->offset(),
					  this->build_id_note_->data_size());

  if (array_of_hashes == NULL)
    {
      const size_t output_file_size = this->output_file_size();
      const unsigned char* iv = of->get_input_view(0, output_file_size);
      const char* style = parameters->options().build_id();

      // If we get here with style == "tree" then the output must be
      // too small for chunking, and we use SHA-1 in that case.
      if ((strcmp(style, "sha1") == 0) || (strcmp(style, "tree") == 0))
	sha1_buffer(reinterpret_cast<const char*>(iv), output_file_size, ov);
      else if (strcmp(style, "md5") == 0)
	md5_buffer(reinterpret_cast<const char*>(iv), output_file_size, ov);
      else
	gold_unreachable();

      of->free_input_view(0, output_file_size, iv);
    }
  else
    {
      // Non-overlapping substrings of the output file have been hashed.
      // Compute SHA-1 hash of the hashes.
      sha1_buffer(reinterpret_cast<const char*>(array_of_hashes),
		  size_of_hashes, ov);
      delete[] array_of_hashes;
    }

  of->write_output_view(this->build_id_note_->offset(),
			this->build_id_note_->data_size(),
			ov);
}

// Write out a binary file.  This is called after the link is
// complete.  IN is the temporary output file we used to generate the
// ELF code.  We simply walk through the segments, read them from
// their file offset in IN, and write them to their load address in
// the output file.  FIXME: with a bit more work, we could support
// S-records and/or Intel hex format here.

void
Layout::write_binary(Output_file* in) const
{
  gold_assert(parameters->options().oformat_enum()
	      == General_options::OBJECT_FORMAT_BINARY);

  // Get the size of the binary file.
  uint64_t max_load_address = 0;
  for (Segment_list::const_iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    {
      if ((*p)->type() == elfcpp::PT_LOAD && (*p)->filesz() > 0)
	{
	  uint64_t max_paddr = (*p)->paddr() + (*p)->filesz();
	  if (max_paddr > max_load_address)
	    max_load_address = max_paddr;
	}
    }

  Output_file out(parameters->options().output_file_name());
  out.open(max_load_address);

  for (Segment_list::const_iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    {
      if ((*p)->type() == elfcpp::PT_LOAD && (*p)->filesz() > 0)
	{
	  const unsigned char* vin = in->get_input_view((*p)->offset(),
							(*p)->filesz());
	  unsigned char* vout = out.get_output_view((*p)->paddr(),
						    (*p)->filesz());
	  memcpy(vout, vin, (*p)->filesz());
	  out.write_output_view((*p)->paddr(), (*p)->filesz(), vout);
	  in->free_input_view((*p)->offset(), (*p)->filesz(), vin);
	}
    }

  out.close();
}

// Print the output sections to the map file.

void
Layout::print_to_mapfile(Mapfile* mapfile) const
{
  for (Segment_list::const_iterator p = this->segment_list_.begin();
       p != this->segment_list_.end();
       ++p)
    (*p)->print_sections_to_mapfile(mapfile);
  for (Section_list::const_iterator p = this->unattached_section_list_.begin();
       p != this->unattached_section_list_.end();
       ++p)
    (*p)->print_to_mapfile(mapfile);
}

// Print statistical information to stderr.  This is used for --stats.

void
Layout::print_stats() const
{
  this->namepool_.print_stats("section name pool");
  this->sympool_.print_stats("output symbol name pool");
  this->dynpool_.print_stats("dynamic name pool");

  for (Section_list::const_iterator p = this->section_list_.begin();
       p != this->section_list_.end();
       ++p)
    (*p)->print_merge_stats();
}

// Write_sections_task methods.

// We can always run this task.

Task_token*
Write_sections_task::is_runnable()
{
  return NULL;
}

// We need to unlock both OUTPUT_SECTIONS_BLOCKER and FINAL_BLOCKER
// when finished.

void
Write_sections_task::locks(Task_locker* tl)
{
  tl->add(this, this->output_sections_blocker_);
  if (this->input_sections_blocker_ != NULL)
    tl->add(this, this->input_sections_blocker_);
  tl->add(this, this->final_blocker_);
}

// Run the task--write out the data.

void
Write_sections_task::run(Workqueue*)
{
  this->layout_->write_output_sections(this->of_);
}

// Write_data_task methods.

// We can always run this task.

Task_token*
Write_data_task::is_runnable()
{
  return NULL;
}

// We need to unlock FINAL_BLOCKER when finished.

void
Write_data_task::locks(Task_locker* tl)
{
  tl->add(this, this->final_blocker_);
}

// Run the task--write out the data.

void
Write_data_task::run(Workqueue*)
{
  this->layout_->write_data(this->symtab_, this->of_);
}

// Write_symbols_task methods.

// We can always run this task.

Task_token*
Write_symbols_task::is_runnable()
{
  return NULL;
}

// We need to unlock FINAL_BLOCKER when finished.

void
Write_symbols_task::locks(Task_locker* tl)
{
  tl->add(this, this->final_blocker_);
}

// Run the task--write out the symbols.

void
Write_symbols_task::run(Workqueue*)
{
  this->symtab_->write_globals(this->sympool_, this->dynpool_,
			       this->layout_->symtab_xindex(),
			       this->layout_->dynsym_xindex(), this->of_);
}

// Write_after_input_sections_task methods.

// We can only run this task after the input sections have completed.

Task_token*
Write_after_input_sections_task::is_runnable()
{
  if (this->input_sections_blocker_->is_blocked())
    return this->input_sections_blocker_;
  return NULL;
}

// We need to unlock FINAL_BLOCKER when finished.

void
Write_after_input_sections_task::locks(Task_locker* tl)
{
  tl->add(this, this->final_blocker_);
}

// Run the task.

void
Write_after_input_sections_task::run(Workqueue*)
{
  this->layout_->write_sections_after_input_sections(this->of_);
}

// Build IDs can be computed as a "flat" sha1 or md5 of a string of bytes,
// or as a "tree" where each chunk of the string is hashed and then those
// hashes are put into a (much smaller) string which is hashed with sha1.
// We compute a checksum over the entire file because that is simplest.

void
Build_id_task_runner::run(Workqueue* workqueue, const Task*)
{
  Task_token* post_hash_tasks_blocker = new Task_token(true);
  const Layout* layout = this->layout_;
  Output_file* of = this->of_;
  const size_t filesize = (layout->output_file_size() <= 0 ? 0
			   : static_cast<size_t>(layout->output_file_size()));
  unsigned char* array_of_hashes = NULL;
  size_t size_of_hashes = 0;

  if (strcmp(this->options_->build_id(), "tree") == 0
      && this->options_->build_id_chunk_size_for_treehash() > 0
      && filesize > 0
      && (filesize >= this->options_->build_id_min_file_size_for_treehash()))
    {
      static const size_t MD5_OUTPUT_SIZE_IN_BYTES = 16;
      const size_t chunk_size =
	  this->options_->build_id_chunk_size_for_treehash();
      const size_t num_hashes = ((filesize - 1) / chunk_size) + 1;
      post_hash_tasks_blocker->add_blockers(num_hashes);
      size_of_hashes = num_hashes * MD5_OUTPUT_SIZE_IN_BYTES;
      array_of_hashes = new unsigned char[size_of_hashes];
      unsigned char *dst = array_of_hashes;
      for (size_t i = 0, src_offset = 0; i < num_hashes;
	   i++, dst += MD5_OUTPUT_SIZE_IN_BYTES, src_offset += chunk_size)
	{
	  size_t size = std::min(chunk_size, filesize - src_offset);
	  workqueue->queue(new Hash_task(of,
					 src_offset,
					 size,
					 dst,
					 post_hash_tasks_blocker));
	}
    }

  // Queue the final task to write the build id and close the output file.
  workqueue->queue(new Task_function(new Close_task_runner(this->options_,
							   layout,
							   of,
							   array_of_hashes,
							   size_of_hashes),
				     post_hash_tasks_blocker,
				     "Task_function Close_task_runner"));
}

// Close_task_runner methods.

// Finish up the build ID computation, if necessary, and write a binary file,
// if necessary.  Then close the output file.

void
Close_task_runner::run(Workqueue*, const Task*)
{
  // At this point the multi-threaded part of the build ID computation,
  // if any, is done.  See Build_id_task_runner.
  this->layout_->write_build_id(this->of_, this->array_of_hashes_,
				this->size_of_hashes_);

  // If we've been asked to create a binary file, we do so here.
  if (this->options_->oformat_enum() != General_options::OBJECT_FORMAT_ELF)
    this->layout_->write_binary(this->of_);

  if (this->options_->dependency_file())
    File_read::write_dependency_file(this->options_->dependency_file(),
				     this->options_->output_file_name());

  this->of_->close();
}

// Instantiate the templates we need.  We could use the configure
// script to restrict this to only the ones for implemented targets.

#ifdef HAVE_TARGET_32_LITTLE
template
Output_section*
Layout::init_fixed_output_section<32, false>(
    const char* name,
    elfcpp::Shdr<32, false>& shdr);
#endif

#ifdef HAVE_TARGET_32_BIG
template
Output_section*
Layout::init_fixed_output_section<32, true>(
    const char* name,
    elfcpp::Shdr<32, true>& shdr);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
Output_section*
Layout::init_fixed_output_section<64, false>(
    const char* name,
    elfcpp::Shdr<64, false>& shdr);
#endif

#ifdef HAVE_TARGET_64_BIG
template
Output_section*
Layout::init_fixed_output_section<64, true>(
    const char* name,
    elfcpp::Shdr<64, true>& shdr);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
Output_section*
Layout::layout<32, false>(Sized_relobj_file<32, false>* object,
			  unsigned int shndx,
			  const char* name,
			  const elfcpp::Shdr<32, false>& shdr,
			  unsigned int, unsigned int, unsigned int, off_t*);
#endif

#ifdef HAVE_TARGET_32_BIG
template
Output_section*
Layout::layout<32, true>(Sized_relobj_file<32, true>* object,
			 unsigned int shndx,
			 const char* name,
			 const elfcpp::Shdr<32, true>& shdr,
			 unsigned int, unsigned int, unsigned int, off_t*);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
Output_section*
Layout::layout<64, false>(Sized_relobj_file<64, false>* object,
			  unsigned int shndx,
			  const char* name,
			  const elfcpp::Shdr<64, false>& shdr,
			  unsigned int, unsigned int, unsigned int, off_t*);
#endif

#ifdef HAVE_TARGET_64_BIG
template
Output_section*
Layout::layout<64, true>(Sized_relobj_file<64, true>* object,
			 unsigned int shndx,
			 const char* name,
			 const elfcpp::Shdr<64, true>& shdr,
			 unsigned int, unsigned int, unsigned int, off_t*);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
Output_section*
Layout::layout_reloc<32, false>(Sized_relobj_file<32, false>* object,
				unsigned int reloc_shndx,
				const elfcpp::Shdr<32, false>& shdr,
				Output_section* data_section,
				Relocatable_relocs* rr);
#endif

#ifdef HAVE_TARGET_32_BIG
template
Output_section*
Layout::layout_reloc<32, true>(Sized_relobj_file<32, true>* object,
			       unsigned int reloc_shndx,
			       const elfcpp::Shdr<32, true>& shdr,
			       Output_section* data_section,
			       Relocatable_relocs* rr);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
Output_section*
Layout::layout_reloc<64, false>(Sized_relobj_file<64, false>* object,
				unsigned int reloc_shndx,
				const elfcpp::Shdr<64, false>& shdr,
				Output_section* data_section,
				Relocatable_relocs* rr);
#endif

#ifdef HAVE_TARGET_64_BIG
template
Output_section*
Layout::layout_reloc<64, true>(Sized_relobj_file<64, true>* object,
			       unsigned int reloc_shndx,
			       const elfcpp::Shdr<64, true>& shdr,
			       Output_section* data_section,
			       Relocatable_relocs* rr);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Layout::layout_group<32, false>(Symbol_table* symtab,
				Sized_relobj_file<32, false>* object,
				unsigned int,
				const char* group_section_name,
				const char* signature,
				const elfcpp::Shdr<32, false>& shdr,
				elfcpp::Elf_Word flags,
				std::vector<unsigned int>* shndxes);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Layout::layout_group<32, true>(Symbol_table* symtab,
			       Sized_relobj_file<32, true>* object,
			       unsigned int,
			       const char* group_section_name,
			       const char* signature,
			       const elfcpp::Shdr<32, true>& shdr,
			       elfcpp::Elf_Word flags,
			       std::vector<unsigned int>* shndxes);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Layout::layout_group<64, false>(Symbol_table* symtab,
				Sized_relobj_file<64, false>* object,
				unsigned int,
				const char* group_section_name,
				const char* signature,
				const elfcpp::Shdr<64, false>& shdr,
				elfcpp::Elf_Word flags,
				std::vector<unsigned int>* shndxes);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Layout::layout_group<64, true>(Symbol_table* symtab,
			       Sized_relobj_file<64, true>* object,
			       unsigned int,
			       const char* group_section_name,
			       const char* signature,
			       const elfcpp::Shdr<64, true>& shdr,
			       elfcpp::Elf_Word flags,
			       std::vector<unsigned int>* shndxes);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
Output_section*
Layout::layout_eh_frame<32, false>(Sized_relobj_file<32, false>* object,
				   const unsigned char* symbols,
				   off_t symbols_size,
				   const unsigned char* symbol_names,
				   off_t symbol_names_size,
				   unsigned int shndx,
				   const elfcpp::Shdr<32, false>& shdr,
				   unsigned int reloc_shndx,
				   unsigned int reloc_type,
				   off_t* off);
#endif

#ifdef HAVE_TARGET_32_BIG
template
Output_section*
Layout::layout_eh_frame<32, true>(Sized_relobj_file<32, true>* object,
				  const unsigned char* symbols,
				  off_t symbols_size,
				  const unsigned char* symbol_names,
				  off_t symbol_names_size,
				  unsigned int shndx,
				  const elfcpp::Shdr<32, true>& shdr,
				  unsigned int reloc_shndx,
				  unsigned int reloc_type,
				  off_t* off);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
Output_section*
Layout::layout_eh_frame<64, false>(Sized_relobj_file<64, false>* object,
				   const unsigned char* symbols,
				   off_t symbols_size,
				   const unsigned char* symbol_names,
				   off_t symbol_names_size,
				   unsigned int shndx,
				   const elfcpp::Shdr<64, false>& shdr,
				   unsigned int reloc_shndx,
				   unsigned int reloc_type,
				   off_t* off);
#endif

#ifdef HAVE_TARGET_64_BIG
template
Output_section*
Layout::layout_eh_frame<64, true>(Sized_relobj_file<64, true>* object,
				  const unsigned char* symbols,
				  off_t symbols_size,
				  const unsigned char* symbol_names,
				  off_t symbol_names_size,
				  unsigned int shndx,
				  const elfcpp::Shdr<64, true>& shdr,
				  unsigned int reloc_shndx,
				  unsigned int reloc_type,
				  off_t* off);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Layout::add_to_gdb_index(bool is_type_unit,
			 Sized_relobj<32, false>* object,
			 const unsigned char* symbols,
			 off_t symbols_size,
			 unsigned int shndx,
			 unsigned int reloc_shndx,
			 unsigned int reloc_type);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Layout::add_to_gdb_index(bool is_type_unit,
			 Sized_relobj<32, true>* object,
			 const unsigned char* symbols,
			 off_t symbols_size,
			 unsigned int shndx,
			 unsigned int reloc_shndx,
			 unsigned int reloc_type);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Layout::add_to_gdb_index(bool is_type_unit,
			 Sized_relobj<64, false>* object,
			 const unsigned char* symbols,
			 off_t symbols_size,
			 unsigned int shndx,
			 unsigned int reloc_shndx,
			 unsigned int reloc_type);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Layout::add_to_gdb_index(bool is_type_unit,
			 Sized_relobj<64, true>* object,
			 const unsigned char* symbols,
			 off_t symbols_size,
			 unsigned int shndx,
			 unsigned int reloc_shndx,
			 unsigned int reloc_type);
#endif

} // End namespace gold.
