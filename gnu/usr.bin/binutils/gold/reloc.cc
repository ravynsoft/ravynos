// reloc.cc -- relocate input files for gold.

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

#include <algorithm>

#include "workqueue.h"
#include "layout.h"
#include "symtab.h"
#include "output.h"
#include "merge.h"
#include "object.h"
#include "target-reloc.h"
#include "reloc.h"
#include "icf.h"
#include "compressed_output.h"
#include "incremental.h"

namespace gold
{

// Read_relocs methods.

// These tasks just read the relocation information from the file.
// After reading it, the start another task to process the
// information.  These tasks requires access to the file.

Task_token*
Read_relocs::is_runnable()
{
  return this->object_->is_locked() ? this->object_->token() : NULL;
}

// Lock the file.

void
Read_relocs::locks(Task_locker* tl)
{
  Task_token* token = this->object_->token();
  if (token != NULL)
    tl->add(this, token);
}

// Read the relocations and then start a Scan_relocs_task.

void
Read_relocs::run(Workqueue* workqueue)
{
  Read_relocs_data* rd = new Read_relocs_data;
  this->object_->read_relocs(rd);
  this->object_->set_relocs_data(rd);
  this->object_->release();

  // If garbage collection or identical comdat folding is desired, we  
  // process the relocs first before scanning them.  Scanning of relocs is
  // done only after garbage or identical sections is identified.
  if (parameters->options().gc_sections()
      || parameters->options().icf_enabled())
    {
      workqueue->queue_next(new Gc_process_relocs(this->symtab_,
                                                  this->layout_, 
                                                  this->object_, rd,
                                                  this->this_blocker_,
						  this->next_blocker_));
    }
  else
    {
      workqueue->queue_next(new Scan_relocs(this->symtab_, this->layout_,
					    this->object_, rd,
                                            this->this_blocker_,
					    this->next_blocker_));
    }
}

// Return a debugging name for the task.

std::string
Read_relocs::get_name() const
{
  return "Read_relocs " + this->object_->name();
}

// Gc_process_relocs methods.

Gc_process_relocs::~Gc_process_relocs()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
}

// These tasks process the relocations read by Read_relocs and
// determine which sections are referenced and which are garbage.
// This task is done only when --gc-sections is used.  This is blocked
// by THIS_BLOCKER_.  It unblocks NEXT_BLOCKER_.

Task_token*
Gc_process_relocs::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  if (this->object_->is_locked())
    return this->object_->token();
  return NULL;
}

void
Gc_process_relocs::locks(Task_locker* tl)
{
  tl->add(this, this->object_->token());
  tl->add(this, this->next_blocker_);
}

void
Gc_process_relocs::run(Workqueue*)
{
  this->object_->gc_process_relocs(this->symtab_, this->layout_, this->rd_);
  this->object_->release();
}

// Return a debugging name for the task.

std::string
Gc_process_relocs::get_name() const
{
  return "Gc_process_relocs " + this->object_->name();
}

// Scan_relocs methods.

Scan_relocs::~Scan_relocs()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
}

// These tasks scan the relocations read by Read_relocs and mark up
// the symbol table to indicate which relocations are required.  We
// use a lock on the symbol table to keep them from interfering with
// each other.

Task_token*
Scan_relocs::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  if (this->object_->is_locked())
    return this->object_->token();
  return NULL;
}

// Return the locks we hold: one on the file, one on the symbol table
// and one blocker.

void
Scan_relocs::locks(Task_locker* tl)
{
  Task_token* token = this->object_->token();
  if (token != NULL)
    tl->add(this, token);
  tl->add(this, this->next_blocker_);
}

// Scan the relocs.

void
Scan_relocs::run(Workqueue*)
{
  this->object_->scan_relocs(this->symtab_, this->layout_, this->rd_);
  delete this->rd_;
  this->rd_ = NULL;
  this->object_->release();
}

// Return a debugging name for the task.

std::string
Scan_relocs::get_name() const
{
  return "Scan_relocs " + this->object_->name();
}

// Relocate_task methods.

// We may have to wait for the output sections to be written.

Task_token*
Relocate_task::is_runnable()
{
  if (this->object_->relocs_must_follow_section_writes()
      && this->output_sections_blocker_->is_blocked())
    return this->output_sections_blocker_;

  if (this->object_->is_locked())
    return this->object_->token();

  return NULL;
}

// We want to lock the file while we run.  We want to unblock
// INPUT_SECTIONS_BLOCKER and FINAL_BLOCKER when we are done.
// INPUT_SECTIONS_BLOCKER may be NULL.

void
Relocate_task::locks(Task_locker* tl)
{
  if (this->input_sections_blocker_ != NULL)
    tl->add(this, this->input_sections_blocker_);
  tl->add(this, this->final_blocker_);
  Task_token* token = this->object_->token();
  if (token != NULL)
    tl->add(this, token);
}

// Run the task.

void
Relocate_task::run(Workqueue*)
{
  this->object_->relocate(this->symtab_, this->layout_, this->of_);

  // This is normally the last thing we will do with an object, so
  // uncache all views.
  this->object_->clear_view_cache_marks();

  this->object_->release();
}

// Return a debugging name for the task.

std::string
Relocate_task::get_name() const
{
  return "Relocate_task " + this->object_->name();
}

// Read the relocs and local symbols from the object file and store
// the information in RD.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_read_relocs(Read_relocs_data* rd)
{
  rd->relocs.clear();

  unsigned int shnum = this->shnum();
  if (shnum == 0)
    return;

  rd->relocs.reserve(shnum / 2);

  const Output_sections& out_sections(this->output_sections());
  const std::vector<Address>& out_offsets(this->section_offsets());

  const unsigned char* pshdrs = this->get_view(this->elf_file_.shoff(),
					       shnum * This::shdr_size,
					       true, true);
  // Skip the first, dummy, section.
  const unsigned char* ps = pshdrs + This::shdr_size;
  for (unsigned int i = 1; i < shnum; ++i, ps += This::shdr_size)
    {
      typename This::Shdr shdr(ps);

      unsigned int sh_type = shdr.get_sh_type();
      if (sh_type != elfcpp::SHT_REL && sh_type != elfcpp::SHT_RELA)
	continue;

      unsigned int shndx = this->adjust_shndx(shdr.get_sh_info());
      if (shndx >= shnum)
	{
	  this->error(_("relocation section %u has bad info %u"),
		      i, shndx);
	  continue;
	}

      Output_section* os = out_sections[shndx];
      if (os == NULL)
	continue;

      // We are scanning relocations in order to fill out the GOT and
      // PLT sections.  Relocations for sections which are not
      // allocated (typically debugging sections) should not add new
      // GOT and PLT entries.  So we skip them unless this is a
      // relocatable link or we need to emit relocations.  FIXME: What
      // should we do if a linker script maps a section with SHF_ALLOC
      // clear to a section with SHF_ALLOC set?
      typename This::Shdr secshdr(pshdrs + shndx * This::shdr_size);
      bool is_section_allocated = ((secshdr.get_sh_flags() & elfcpp::SHF_ALLOC)
				   != 0);
      if (!is_section_allocated
	  && !parameters->options().relocatable()
	  && !parameters->options().emit_relocs()
	  && !parameters->incremental())
	continue;

      if (this->adjust_shndx(shdr.get_sh_link()) != this->symtab_shndx_)
	{
	  this->error(_("relocation section %u uses unexpected "
			"symbol table %u"),
		      i, this->adjust_shndx(shdr.get_sh_link()));
	  continue;
	}

      off_t sh_size = shdr.get_sh_size();

      if (sh_size == 0)
	continue;

      unsigned int reloc_size;
      if (sh_type == elfcpp::SHT_REL)
	reloc_size = elfcpp::Elf_sizes<size>::rel_size;
      else
	reloc_size = elfcpp::Elf_sizes<size>::rela_size;
      if (reloc_size != shdr.get_sh_entsize())
	{
	  this->error(_("unexpected entsize for reloc section %u: %lu != %u"),
		      i, static_cast<unsigned long>(shdr.get_sh_entsize()),
		      reloc_size);
	  continue;
	}

      size_t reloc_count = sh_size / reloc_size;
      if (static_cast<off_t>(reloc_count * reloc_size) != sh_size)
	{
	  this->error(_("reloc section %u size %lu uneven"),
		      i, static_cast<unsigned long>(sh_size));
	  continue;
	}

      rd->relocs.push_back(Section_relocs());
      Section_relocs& sr(rd->relocs.back());
      sr.reloc_shndx = i;
      sr.data_shndx = shndx;
      sr.contents = this->get_lasting_view(shdr.get_sh_offset(), sh_size,
					   true, true);
      sr.sh_type = sh_type;
      sr.reloc_count = reloc_count;
      sr.output_section = os;
      sr.needs_special_offset_handling = out_offsets[shndx] == invalid_address;
      sr.is_data_section_allocated = is_section_allocated;
    }

  // Read the local symbols.
  gold_assert(this->symtab_shndx_ != -1U);
  if (this->symtab_shndx_ == 0 || this->local_symbol_count_ == 0)
    rd->local_symbols = NULL;
  else
    {
      typename This::Shdr symtabshdr(pshdrs
				     + this->symtab_shndx_ * This::shdr_size);
      gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);
      const int sym_size = This::sym_size;
      const unsigned int loccount = this->local_symbol_count_;
      gold_assert(loccount == symtabshdr.get_sh_info());
      off_t locsize = loccount * sym_size;
      rd->local_symbols = this->get_lasting_view(symtabshdr.get_sh_offset(),
						 locsize, true, true);
    }
}

// Process the relocs to generate mappings from source sections to referenced
// sections.  This is used during garbage collection to determine garbage
// sections.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_gc_process_relocs(Symbol_table* symtab,
							  Layout* layout,
							  Read_relocs_data* rd)
{  
  Sized_target<size, big_endian>* target =
    parameters->sized_target<size, big_endian>();

  const unsigned char* local_symbols;
  if (rd->local_symbols == NULL)
    local_symbols = NULL;
  else
    local_symbols = rd->local_symbols->data();

  for (Read_relocs_data::Relocs_list::iterator p = rd->relocs.begin();
       p != rd->relocs.end();
       ++p)
    {
      if (!parameters->options().relocatable())
	  {
	    // As noted above, when not generating an object file, we
	    // only scan allocated sections.  We may see a non-allocated
	    // section here if we are emitting relocs.
	    if (p->is_data_section_allocated)
              target->gc_process_relocs(symtab, layout, this, 
                                        p->data_shndx, p->sh_type, 
                                        p->contents->data(), p->reloc_count, 
                                        p->output_section,
                                        p->needs_special_offset_handling,
                                        this->local_symbol_count_, 
                                        local_symbols);
        }
    }
}


// Scan the relocs and adjust the symbol table.  This looks for
// relocations which require GOT/PLT/COPY relocations.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_scan_relocs(Symbol_table* symtab,
						    Layout* layout,
						    Read_relocs_data* rd)
{
  Sized_target<size, big_endian>* target =
    parameters->sized_target<size, big_endian>();

  const unsigned char* local_symbols;
  if (rd->local_symbols == NULL)
    local_symbols = NULL;
  else
    local_symbols = rd->local_symbols->data();

  // For incremental links, allocate the counters for incremental relocations.
  if (layout->incremental_inputs() != NULL)
    this->allocate_incremental_reloc_counts();

  for (Read_relocs_data::Relocs_list::iterator p = rd->relocs.begin();
       p != rd->relocs.end();
       ++p)
    {
      // When garbage collection is on, unreferenced sections are not included
      // in the link that would have been included normally. This is known only
      // after Read_relocs hence this check has to be done again.
      if (parameters->options().gc_sections()
	  || parameters->options().icf_enabled())
        {
          if (p->output_section == NULL)
            continue;
        }
      if (!parameters->options().relocatable())
	{
	  // As noted above, when not generating an object file, we
	  // only scan allocated sections.  We may see a non-allocated
	  // section here if we are emitting relocs.
	  if (p->is_data_section_allocated)
	    target->scan_relocs(symtab, layout, this, p->data_shndx,
				p->sh_type, p->contents->data(),
				p->reloc_count, p->output_section,
				p->needs_special_offset_handling,
				this->local_symbol_count_,
				local_symbols);
	  if (parameters->options().emit_relocs())
	    this->emit_relocs_scan(symtab, layout, local_symbols, p);
	  if (layout->incremental_inputs() != NULL)
	    this->incremental_relocs_scan(p);
	}
      else
	{
	  Relocatable_relocs* rr = this->relocatable_relocs(p->reloc_shndx);
	  gold_assert(rr != NULL);
	  rr->set_reloc_count(p->reloc_count);
	  target->scan_relocatable_relocs(symtab, layout, this,
					  p->data_shndx, p->sh_type,
					  p->contents->data(),
					  p->reloc_count,
					  p->output_section,
					  p->needs_special_offset_handling,
					  this->local_symbol_count_,
					  local_symbols,
					  rr);
	}

      delete p->contents;
      p->contents = NULL;
    }

  // For incremental links, finalize the allocation of relocations.
  if (layout->incremental_inputs() != NULL)
    this->finalize_incremental_relocs(layout, true);

  if (rd->local_symbols != NULL)
    {
      delete rd->local_symbols;
      rd->local_symbols = NULL;
    }
}

// Scan the input relocations for --emit-relocs.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::emit_relocs_scan(
    Symbol_table* symtab,
    Layout* layout,
    const unsigned char* plocal_syms,
    const Read_relocs_data::Relocs_list::iterator& p)
{
  Sized_target<size, big_endian>* target =
      parameters->sized_target<size, big_endian>();

  Relocatable_relocs* rr = this->relocatable_relocs(p->reloc_shndx);
  gold_assert(rr != NULL);
  rr->set_reloc_count(p->reloc_count);
  target->emit_relocs_scan(
    symtab,
    layout,
    this,
    p->data_shndx,
    p->sh_type,
    p->contents->data(),
    p->reloc_count,
    p->output_section,
    p->needs_special_offset_handling,
    this->local_symbol_count_,
    plocal_syms,
    rr);
}

// Scan the input relocations for --incremental.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::incremental_relocs_scan(
    const Read_relocs_data::Relocs_list::iterator& p)
{
  if (p->sh_type == elfcpp::SHT_REL)
    this->incremental_relocs_scan_reltype<elfcpp::SHT_REL>(p);
  else
    {
      gold_assert(p->sh_type == elfcpp::SHT_RELA);
      this->incremental_relocs_scan_reltype<elfcpp::SHT_RELA>(p);
    }
}

// Scan the input relocation for --incremental, templatized on the
// type of the relocation section.

template<int size, bool big_endian>
template<int sh_type>
void
Sized_relobj_file<size, big_endian>::incremental_relocs_scan_reltype(
    const Read_relocs_data::Relocs_list::iterator& p)
{
  typedef typename Reloc_types<sh_type, size, big_endian>::Reloc Reltype;
  const int reloc_size = Reloc_types<sh_type, size, big_endian>::reloc_size;
  const unsigned char* prelocs = p->contents->data();
  size_t reloc_count = p->reloc_count;

  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Reltype reloc(prelocs);

      if (p->needs_special_offset_handling
	  && !p->output_section->is_input_address_mapped(this, p->data_shndx,
						         reloc.get_r_offset()))
	continue;

      // FIXME: Some targets have a non-standard r_info field.
      typename elfcpp::Elf_types<size>::Elf_WXword r_info = reloc.get_r_info();
      const unsigned int r_sym = elfcpp::elf_r_sym<size>(r_info);

      if (r_sym >= this->local_symbol_count_)
	this->count_incremental_reloc(r_sym - this->local_symbol_count_);
    }
}

// Relocate the input sections and write out the local symbols.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_relocate(const Symbol_table* symtab,
						 const Layout* layout,
						 Output_file* of)
{
  unsigned int shnum = this->shnum();

  // Read the section headers.
  const unsigned char* pshdrs = this->get_view(this->elf_file_.shoff(),
					       shnum * This::shdr_size,
					       true, true);

  Views views;
  views.resize(shnum);

  // Make two passes over the sections.  The first one copies the
  // section data to the output file.  The second one applies
  // relocations.

  this->write_sections(layout, pshdrs, of, &views);

  // To speed up relocations, we set up hash tables for fast lookup of
  // input offsets to output addresses.
  this->initialize_input_to_output_maps();

  // Make the views available through get_output_view() for the duration
  // of this routine.  This RAII class will reset output_views_ to NULL
  // when the views go out of scope.
  struct Set_output_views
  {
    Set_output_views(const Views** ppviews, const Views* pviews)
    {
      ppviews_ = ppviews;
      *ppviews = pviews;
    }

    ~Set_output_views()
    { *ppviews_ = NULL; }

    const Views** ppviews_;
  };
  Set_output_views set_output_views(&this->output_views_, &views);

  // Apply relocations.

  this->relocate_sections(symtab, layout, pshdrs, of, &views);

  // After we've done the relocations, we release the hash tables,
  // since we no longer need them.
  this->free_input_to_output_maps();

  // Write out the accumulated views.
  for (unsigned int i = 1; i < shnum; ++i)
    {
      if (views[i].view != NULL)
	{
	  if (views[i].is_ctors_reverse_view)
	    this->reverse_words(views[i].view, views[i].view_size);
	  if (!views[i].is_postprocessing_view)
	    {
	      if (views[i].is_input_output_view)
		of->write_input_output_view(views[i].offset,
					    views[i].view_size,
					    views[i].view);
	      else
		of->write_output_view(views[i].offset, views[i].view_size,
				      views[i].view);
	    }
	}
    }

  // Write out the local symbols.
  this->write_local_symbols(of, layout->sympool(), layout->dynpool(),
			    layout->symtab_xindex(), layout->dynsym_xindex(),
			    layout->symtab_section_offset());
}

// Sort a Read_multiple vector by file offset.
struct Read_multiple_compare
{
  inline bool
  operator()(const File_read::Read_multiple_entry& rme1,
	     const File_read::Read_multiple_entry& rme2) const
  { return rme1.file_offset < rme2.file_offset; }
};

// Write section data to the output file.  PSHDRS points to the
// section headers.  Record the views in *PVIEWS for use when
// relocating.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::write_sections(const Layout* layout,
						    const unsigned char* pshdrs,
						    Output_file* of,
						    Views* pviews)
{
  unsigned int shnum = this->shnum();
  const Output_sections& out_sections(this->output_sections());
  const std::vector<Address>& out_offsets(this->section_offsets());

  File_read::Read_multiple rm;
  bool is_sorted = true;

  const unsigned char* p = pshdrs + This::shdr_size;
  for (unsigned int i = 1; i < shnum; ++i, p += This::shdr_size)
    {
      View_size* pvs = &(*pviews)[i];

      pvs->view = NULL;

      const Output_section* os = out_sections[i];
      if (os == NULL)
	continue;
      Address output_offset = out_offsets[i];

      typename This::Shdr shdr(p);

      if (shdr.get_sh_type() == elfcpp::SHT_NOBITS)
	continue;

      if ((parameters->options().relocatable()
	   || parameters->options().emit_relocs())
	  && (shdr.get_sh_type() == elfcpp::SHT_REL
	      || shdr.get_sh_type() == elfcpp::SHT_RELA)
	  && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0)
	{
	  // This is a reloc section in a relocatable link or when
	  // emitting relocs.  We don't need to read the input file.
	  // The size and file offset are stored in the
	  // Relocatable_relocs structure.
	  Relocatable_relocs* rr = this->relocatable_relocs(i);
	  gold_assert(rr != NULL);
	  Output_data* posd = rr->output_data();
	  gold_assert(posd != NULL);

	  pvs->offset = posd->offset();
	  pvs->view_size = posd->data_size();
	  pvs->view = of->get_output_view(pvs->offset, pvs->view_size);
	  pvs->address = posd->address();
	  pvs->is_input_output_view = false;
	  pvs->is_postprocessing_view = false;
	  pvs->is_ctors_reverse_view = false;

	  continue;
	}

      // In the normal case, this input section is simply mapped to
      // the output section at offset OUTPUT_OFFSET.

      // However, if OUTPUT_OFFSET == INVALID_ADDRESS, then input data is
      // handled specially--e.g., a .eh_frame section.  The relocation
      // routines need to check for each reloc where it should be
      // applied.  For this case, we need an input/output view for the
      // entire contents of the section in the output file.  We don't
      // want to copy the contents of the input section to the output
      // section; the output section contents were already written,
      // and we waited for them in Relocate_task::is_runnable because
      // relocs_must_follow_section_writes is set for the object.

      // Regardless of which of the above cases is true, we have to
      // check requires_postprocessing of the output section.  If that
      // is false, then we work with views of the output file
      // directly.  If it is true, then we work with a separate
      // buffer, and the output section is responsible for writing the
      // final data to the output file.

      off_t output_section_offset;
      Address output_section_size;
      if (!os->requires_postprocessing())
	{
	  output_section_offset = os->offset();
	  output_section_size = convert_types<Address, off_t>(os->data_size());
	}
      else
	{
	  output_section_offset = 0;
	  output_section_size =
              convert_types<Address, off_t>(os->postprocessing_buffer_size());
	}

      off_t view_start;
      section_size_type view_size;
      bool must_decompress = false;
      if (output_offset != invalid_address)
	{
	  view_start = output_section_offset + output_offset;
	  view_size = convert_to_section_size_type(shdr.get_sh_size());
	  section_size_type uncompressed_size;
	  if (this->section_is_compressed(i, &uncompressed_size))
	    {
	      view_size = uncompressed_size;
	      must_decompress = true;
	    }
	}
      else
	{
	  view_start = output_section_offset;
	  view_size = convert_to_section_size_type(output_section_size);
	}

      if (view_size == 0)
	continue;

      gold_assert(output_offset == invalid_address
		  || output_offset + view_size <= output_section_size);

      unsigned char* view;
      if (os->requires_postprocessing())
	{
	  unsigned char* buffer = os->postprocessing_buffer();
	  view = buffer + view_start;
	  if (output_offset != invalid_address && !must_decompress)
	    {
	      off_t sh_offset = shdr.get_sh_offset();
	      if (!rm.empty() && rm.back().file_offset > sh_offset)
		is_sorted = false;
	      rm.push_back(File_read::Read_multiple_entry(sh_offset,
							  view_size, view));
	    }
	}
      else
	{
	  if (output_offset == invalid_address)
	    view = of->get_input_output_view(view_start, view_size);
	  else
	    {
	      view = of->get_output_view(view_start, view_size);
	      if (!must_decompress)
		{
		  off_t sh_offset = shdr.get_sh_offset();
		  if (!rm.empty() && rm.back().file_offset > sh_offset)
		    is_sorted = false;
		  rm.push_back(File_read::Read_multiple_entry(sh_offset,
							      view_size, view));
		}
	    }
	}

      if (must_decompress)
        {
	  // Read and decompress the section.
          section_size_type len;
	  const unsigned char* p = this->section_contents(i, &len, false);
	  if (!decompress_input_section(p, len, view, view_size,
					size, big_endian,
					shdr.get_sh_flags()))
	    this->error(_("could not decompress section %s"),
			this->section_name(i).c_str());
        }

      pvs->view = view;
      pvs->address = os->address();
      if (output_offset != invalid_address)
	pvs->address += output_offset;
      pvs->offset = view_start;
      pvs->view_size = view_size;
      pvs->is_input_output_view = output_offset == invalid_address;
      pvs->is_postprocessing_view = os->requires_postprocessing();
      pvs->is_ctors_reverse_view =
	(!parameters->options().relocatable()
	 && view_size > size / 8
	 && (strcmp(os->name(), ".init_array") == 0
	     || strcmp(os->name(), ".fini_array") == 0)
	 && layout->is_ctors_in_init_array(this, i));
    }

  // Actually read the data.
  if (!rm.empty())
    {
      if (!is_sorted)
	std::sort(rm.begin(), rm.end(), Read_multiple_compare());
      this->read_multiple(rm);
    }
}

// Relocate section data.  VIEWS points to the section data as views
// in the output file.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_relocate_sections(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews)
{
  this->relocate_section_range(symtab, layout, pshdrs, of, pviews,
			       1, this->shnum() - 1);
}

// Relocate section data for the range of sections START_SHNDX through
// END_SHNDX.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::relocate_section_range(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews,
    unsigned int start_shndx,
    unsigned int end_shndx)
{
  gold_assert(start_shndx >= 1);
  gold_assert(end_shndx < this->shnum());

  if (end_shndx < start_shndx)
    return;

  Sized_target<size, big_endian>* target =
    parameters->sized_target<size, big_endian>();

  const Output_sections& out_sections(this->output_sections());
  const std::vector<Address>& out_offsets(this->section_offsets());

  Relocate_info<size, big_endian> relinfo;
  relinfo.symtab = symtab;
  relinfo.layout = layout;
  relinfo.object = this;

  const unsigned char* p = pshdrs + start_shndx * This::shdr_size;
  for (unsigned int i = start_shndx; i <= end_shndx; ++i, p += This::shdr_size)
    {
      typename This::Shdr shdr(p);

      unsigned int sh_type = shdr.get_sh_type();
      if (sh_type != elfcpp::SHT_REL && sh_type != elfcpp::SHT_RELA)
	continue;

      off_t sh_size = shdr.get_sh_size();
      if (sh_size == 0)
	continue;

      unsigned int index = this->adjust_shndx(shdr.get_sh_info());
      if (index >= this->shnum())
	{
	  this->error(_("relocation section %u has bad info %u"),
		      i, index);
	  continue;
	}

      Output_section* os = out_sections[index];
      if (os == NULL)
	{
	  // This relocation section is against a section which we
	  // discarded.
	  continue;
	}
      Address output_offset = out_offsets[index];

      gold_assert((*pviews)[index].view != NULL);
      if (parameters->options().relocatable())
	gold_assert((*pviews)[i].view != NULL);

      if (this->adjust_shndx(shdr.get_sh_link()) != this->symtab_shndx_)
	{
	  gold_error(_("relocation section %u uses unexpected "
		       "symbol table %u"),
		     i, this->adjust_shndx(shdr.get_sh_link()));
	  continue;
	}

      const unsigned char* prelocs = this->get_view(shdr.get_sh_offset(),
						    sh_size, true, false);

      unsigned int reloc_size;
      if (sh_type == elfcpp::SHT_REL)
	reloc_size = elfcpp::Elf_sizes<size>::rel_size;
      else
	reloc_size = elfcpp::Elf_sizes<size>::rela_size;

      if (reloc_size != shdr.get_sh_entsize())
	{
	  gold_error(_("unexpected entsize for reloc section %u: %lu != %u"),
		     i, static_cast<unsigned long>(shdr.get_sh_entsize()),
		     reloc_size);
	  continue;
	}

      size_t reloc_count = sh_size / reloc_size;
      if (static_cast<off_t>(reloc_count * reloc_size) != sh_size)
	{
	  gold_error(_("reloc section %u size %lu uneven"),
		     i, static_cast<unsigned long>(sh_size));
	  continue;
	}

      gold_assert(output_offset != invalid_address
		  || this->relocs_must_follow_section_writes());

      relinfo.reloc_shndx = i;
      relinfo.reloc_shdr = p;
      relinfo.data_shndx = index;
      relinfo.data_shdr = pshdrs + index * This::shdr_size;
      unsigned char* view = (*pviews)[index].view;
      Address address = (*pviews)[index].address;
      section_size_type view_size = (*pviews)[index].view_size;

      Reloc_symbol_changes* reloc_map = NULL;
      if (this->uses_split_stack() && output_offset != invalid_address)
	{
	  typename This::Shdr data_shdr(pshdrs + index * This::shdr_size);
	  if ((data_shdr.get_sh_flags() & elfcpp::SHF_EXECINSTR) != 0)
	    this->split_stack_adjust(symtab, pshdrs, sh_type, index,
				     prelocs, reloc_count, view, view_size,
				     &reloc_map, target);
	}

      Relocatable_relocs* rr = NULL;
      if (parameters->options().emit_relocs()
	  || parameters->options().relocatable())
	rr = this->relocatable_relocs(i);
      relinfo.rr = rr;

      if (!parameters->options().relocatable())
	{
	  target->relocate_section(&relinfo, sh_type, prelocs, reloc_count, os,
				   output_offset == invalid_address,
				   view, address, view_size, reloc_map);
	  if (parameters->options().emit_relocs())
	    target->relocate_relocs(&relinfo, sh_type, prelocs, reloc_count,
				    os, output_offset,
				    view, address, view_size,
				    (*pviews)[i].view,
				    (*pviews)[i].view_size);
	  if (parameters->incremental())
	    this->incremental_relocs_write(&relinfo, sh_type, prelocs,
					   reloc_count, os, output_offset, of);
	}
      else
	target->relocate_relocs(&relinfo, sh_type, prelocs, reloc_count,
				os, output_offset,
				view, address, view_size,
				(*pviews)[i].view,
				(*pviews)[i].view_size);
    }
}

// Return the output view for section SHNDX.

template<int size, bool big_endian>
unsigned char*
Sized_relobj_file<size, big_endian>::do_get_output_view(
    unsigned int shndx,
    section_size_type* plen) const
{
  gold_assert(this->output_views_ != NULL);
  gold_assert(shndx < this->output_views_->size());
  const View_size& v = (*this->output_views_)[shndx];
  *plen = v.view_size;
  return v.view;
}

// Write the incremental relocs.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::incremental_relocs_write(
    const Relocate_info<size, big_endian>* relinfo,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    Address output_offset,
    Output_file* of)
{
  if (sh_type == elfcpp::SHT_REL)
    this->incremental_relocs_write_reltype<elfcpp::SHT_REL>(
	relinfo,
	prelocs,
	reloc_count,
	output_section,
	output_offset,
	of);
  else
    {
      gold_assert(sh_type == elfcpp::SHT_RELA);
      this->incremental_relocs_write_reltype<elfcpp::SHT_RELA>(
	  relinfo,
	  prelocs,
	  reloc_count,
	  output_section,
	  output_offset,
	  of);
    }
}

// Write the incremental relocs, templatized on the type of the
// relocation section.

template<int size, bool big_endian>
template<int sh_type>
void
Sized_relobj_file<size, big_endian>::incremental_relocs_write_reltype(
    const Relocate_info<size, big_endian>* relinfo,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    Address output_offset,
    Output_file* of)
{
  typedef typename Reloc_types<sh_type, size, big_endian>::Reloc Reloc;
  const unsigned int reloc_size =
      Reloc_types<sh_type, size, big_endian>::reloc_size;
  const unsigned int sizeof_addr = size / 8;
  const unsigned int incr_reloc_size =
      Incremental_relocs_reader<size, big_endian>::reloc_size;

  unsigned int out_shndx = output_section->out_shndx();

  // Get a view for the .gnu_incremental_relocs section.

  Incremental_inputs* inputs = relinfo->layout->incremental_inputs();
  gold_assert(inputs != NULL);
  const off_t relocs_off = inputs->relocs_section()->offset();
  const off_t relocs_size = inputs->relocs_section()->data_size();
  unsigned char* const view = of->get_output_view(relocs_off, relocs_size);

  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Reloc reloc(prelocs);

      // FIXME: Some targets have a non-standard r_info field.
      typename elfcpp::Elf_types<size>::Elf_WXword r_info = reloc.get_r_info();
      const unsigned int r_sym = elfcpp::elf_r_sym<size>(r_info);
      const unsigned int r_type = elfcpp::elf_r_type<size>(r_info);

      if (r_sym < this->local_symbol_count_)
        continue;

      // Get the new offset--the location in the output section where
      // this relocation should be applied.

      Address offset = reloc.get_r_offset();
      if (output_offset != invalid_address)
	offset += output_offset;
      else
	{
          section_offset_type sot_offset =
              convert_types<section_offset_type, Address>(offset);
	  section_offset_type new_sot_offset =
	      output_section->output_offset(relinfo->object,
					    relinfo->data_shndx,
					    sot_offset);
	  gold_assert(new_sot_offset != -1);
	  offset += new_sot_offset;
	}

      // Get the addend.
      typename elfcpp::Elf_types<size>::Elf_Swxword addend;
      if (sh_type == elfcpp::SHT_RELA)
	addend =
	    Reloc_types<sh_type, size, big_endian>::get_reloc_addend(&reloc);
      else
        {
          // FIXME: Get the addend for SHT_REL.
          addend = 0;
        }

      // Get the index of the output relocation.

      unsigned int reloc_index =
          this->next_incremental_reloc_index(r_sym - this->local_symbol_count_);

      // Write the relocation.

      unsigned char* pov = view + reloc_index * incr_reloc_size;
      elfcpp::Swap<32, big_endian>::writeval(pov, r_type);
      elfcpp::Swap<32, big_endian>::writeval(pov + 4, out_shndx);
      elfcpp::Swap<size, big_endian>::writeval(pov + 8, offset);
      elfcpp::Swap<size, big_endian>::writeval(pov + 8 + sizeof_addr, addend);
      of->write_output_view(pov - view, incr_reloc_size, view);
    }
}

// Create merge hash tables for the local symbols.  These are used to
// speed up relocations.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::initialize_input_to_output_maps()
{
  const unsigned int loccount = this->local_symbol_count_;
  for (unsigned int i = 1; i < loccount; ++i)
    {
      Symbol_value<size>& lv(this->local_values_[i]);
      lv.initialize_input_to_output_map(this);
    }
}

// Free merge hash tables for the local symbols.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::free_input_to_output_maps()
{
  const unsigned int loccount = this->local_symbol_count_;
  for (unsigned int i = 1; i < loccount; ++i)
    {
      Symbol_value<size>& lv(this->local_values_[i]);
      lv.free_input_to_output_map();
    }
}

// If an object was compiled with -fsplit-stack, this is called to
// check whether any relocations refer to functions defined in objects
// which were not compiled with -fsplit-stack.  If they were, then we
// need to apply some target-specific adjustments to request
// additional stack space.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::split_stack_adjust(
    const Symbol_table* symtab,
    const unsigned char* pshdrs,
    unsigned int sh_type,
    unsigned int shndx,
    const unsigned char* prelocs,
    size_t reloc_count,
    unsigned char* view,
    section_size_type view_size,
    Reloc_symbol_changes** reloc_map,
    const Sized_target<size, big_endian>* target)
{
  if (sh_type == elfcpp::SHT_REL)
    this->split_stack_adjust_reltype<elfcpp::SHT_REL>(symtab, pshdrs, shndx,
						      prelocs, reloc_count,
						      view, view_size,
						      reloc_map, target);
  else
    {
      gold_assert(sh_type == elfcpp::SHT_RELA);
      this->split_stack_adjust_reltype<elfcpp::SHT_RELA>(symtab, pshdrs, shndx,
							 prelocs, reloc_count,
							 view, view_size,
							 reloc_map, target);
    }
}

// Adjust for -fsplit-stack, templatized on the type of the relocation
// section.

template<int size, bool big_endian>
template<int sh_type>
void
Sized_relobj_file<size, big_endian>::split_stack_adjust_reltype(
    const Symbol_table* symtab,
    const unsigned char* pshdrs,
    unsigned int shndx,
    const unsigned char* prelocs,
    size_t reloc_count,
    unsigned char* view,
    section_size_type view_size,
    Reloc_symbol_changes** reloc_map,
    const Sized_target<size, big_endian>* target)
{
  typedef typename Reloc_types<sh_type, size, big_endian>::Reloc Reltype;
  const int reloc_size = Reloc_types<sh_type, size, big_endian>::reloc_size;

  size_t local_count = this->local_symbol_count();

  std::vector<section_offset_type> non_split_refs;

  const unsigned char* pr = prelocs;
  for (size_t i = 0; i < reloc_count; ++i, pr += reloc_size)
    {
      // Some supported targets have a non-standard r_info field.
      // If this call is too slow, we can move this routine to
      // target-reloc.h and templatize it on Classify_reloc.
      unsigned int r_sym = target->get_r_sym(pr);
      if (r_sym < local_count)
	continue;

      const Symbol* gsym = this->global_symbol(r_sym);
      gold_assert(gsym != NULL);
      if (gsym->is_forwarder())
	gsym = symtab->resolve_forwards(gsym);

      // See if this relocation refers to a function defined in an
      // object compiled without -fsplit-stack.  Note that we don't
      // care about the type of relocation--this means that in some
      // cases we will ask for a large stack unnecessarily, but this
      // is not fatal.  FIXME: Some targets have symbols which are
      // functions but are not type STT_FUNC, e.g., STT_ARM_TFUNC.
      if (!gsym->is_undefined()
	  && gsym->source() == Symbol::FROM_OBJECT
	  && !gsym->object()->uses_split_stack())
	{
	  if (parameters->target().is_call_to_non_split(gsym, pr, view,
							view_size))
	    {
	      Reltype reloc(pr);
	      section_offset_type offset =
		convert_to_section_size_type(reloc.get_r_offset());
	      non_split_refs.push_back(offset);
	    }
	}
    }

  if (non_split_refs.empty())
    return;

  // At this point, every entry in NON_SPLIT_REFS indicates a
  // relocation which refers to a function in an object compiled
  // without -fsplit-stack.  We now have to convert that list into a
  // set of offsets to functions.  First, we find all the functions.

  Function_offsets function_offsets;
  this->find_functions(pshdrs, shndx, &function_offsets);
  if (function_offsets.empty())
    return;

  // Now get a list of the function with references to non split-stack
  // code.

  Function_offsets calls_non_split;
  for (std::vector<section_offset_type>::const_iterator p
	 = non_split_refs.begin();
       p != non_split_refs.end();
       ++p)
    {
      Function_offsets::const_iterator low = function_offsets.lower_bound(*p);
      if (low == function_offsets.end())
	--low;
      else if (low->first == *p)
	;
      else if (low == function_offsets.begin())
	continue;
      else
	--low;

      calls_non_split.insert(*low);
    }
  if (calls_non_split.empty())
    return;

  // Now we have a set of functions to adjust.  The adjustments are
  // target specific.  Besides changing the output section view
  // however, it likes, the target may request a relocation change
  // from one global symbol name to another.

  for (Function_offsets::const_iterator p = calls_non_split.begin();
       p != calls_non_split.end();
       ++p)
    {
      std::string from;
      std::string to;
      parameters->target().calls_non_split(this, shndx, p->first, p->second,
					   prelocs, reloc_count,
					   view, view_size, &from, &to);
      if (!from.empty())
	{
	  gold_assert(!to.empty());
	  Symbol* tosym = NULL;

	  // Find relocations in the relevant function which are for
	  // FROM.
	  pr = prelocs;
	  for (size_t i = 0; i < reloc_count; ++i, pr += reloc_size)
	    {
	      Reltype reloc(pr);

	      unsigned int r_sym = target->get_r_sym(pr);
	      if (r_sym < local_count)
		continue;

	      section_offset_type offset =
		convert_to_section_size_type(reloc.get_r_offset());
	      if (offset < p->first
		  || (offset
		      >= (p->first
			  + static_cast<section_offset_type>(p->second))))
		continue;

	      const Symbol* gsym = this->global_symbol(r_sym);
	      if (from == gsym->name())
		{
		  if (tosym == NULL)
		    {
		      tosym = symtab->lookup(to.c_str());
		      if (tosym == NULL)
			{
			  this->error(_("could not convert call "
					"to '%s' to '%s'"),
				      from.c_str(), to.c_str());
			  break;
			}
		    }

		  if (*reloc_map == NULL)
		    *reloc_map = new Reloc_symbol_changes(reloc_count);
		  (*reloc_map)->set(i, tosym);
		}
	    }
	}
    }
}

// Find all the function in this object defined in section SHNDX.
// Store their offsets in the section in FUNCTION_OFFSETS.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::find_functions(
    const unsigned char* pshdrs,
    unsigned int shndx,
    Sized_relobj_file<size, big_endian>::Function_offsets* function_offsets)
{
  // We need to read the symbols to find the functions.  If we wanted
  // to, we could cache reading the symbols across all sections in the
  // object.
  const unsigned int symtab_shndx = this->symtab_shndx_;
  typename This::Shdr symtabshdr(pshdrs + symtab_shndx * This::shdr_size);
  gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);

  typename elfcpp::Elf_types<size>::Elf_WXword sh_size =
    symtabshdr.get_sh_size();
  const unsigned char* psyms = this->get_view(symtabshdr.get_sh_offset(),
					      sh_size, true, true);

  const int sym_size = This::sym_size;
  const unsigned int symcount = sh_size / sym_size;
  for (unsigned int i = 0; i < symcount; ++i, psyms += sym_size)
    {
      typename elfcpp::Sym<size, big_endian> isym(psyms);

      // FIXME: Some targets can have functions which do not have type
      // STT_FUNC, e.g., STT_ARM_TFUNC.
      if (isym.get_st_type() != elfcpp::STT_FUNC
	  || isym.get_st_size() == 0)
	continue;

      bool is_ordinary;
      Symbol_location loc;
      loc.shndx = this->adjust_sym_shndx(i, isym.get_st_shndx(),
					 &is_ordinary);
      if (!is_ordinary)
	continue;

      loc.object = this;
      loc.offset = isym.get_st_value();
      parameters->target().function_location(&loc);

      if (loc.shndx != shndx)
	continue;

      section_offset_type value =
	convert_to_section_size_type(loc.offset);
      section_size_type fnsize =
	convert_to_section_size_type(isym.get_st_size());

      (*function_offsets)[value] = fnsize;
    }
}

// Reverse the words in a section.  Used for .ctors sections mapped to
// .init_array sections.  See ctors_sections_in_init_array in
// layout.cc.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::reverse_words(unsigned char* view,
						   section_size_type view_size)
{
  typedef typename elfcpp::Swap<size, big_endian>::Valtype Valtype;
  Valtype* vview = reinterpret_cast<Valtype*>(view);
  section_size_type vview_size = view_size / (size / 8);
  for (section_size_type i = 0; i < vview_size / 2; ++i)
    {
      Valtype tmp = vview[i];
      vview[i] = vview[vview_size - 1 - i];
      vview[vview_size - 1 - i] = tmp;
    }
}

// Class Merged_symbol_value.

template<int size>
void
Merged_symbol_value<size>::initialize_input_to_output_map(
    const Relobj* object,
    unsigned int input_shndx)
{
  object->initialize_input_to_output_map<size>(input_shndx,
					       this->output_start_address_,
					       &this->output_addresses_);
}

// Get the output value corresponding to an input offset if we
// couldn't find it in the hash table.

template<int size>
typename elfcpp::Elf_types<size>::Elf_Addr
Merged_symbol_value<size>::value_from_output_section(
    const Relobj* object,
    unsigned int input_shndx,
    typename elfcpp::Elf_types<size>::Elf_Addr input_offset) const
{
  section_offset_type output_offset;
  bool found = object->merge_output_offset(input_shndx, input_offset,
					   &output_offset);

  // If this assertion fails, it means that some relocation was
  // against a portion of an input merge section which we didn't map
  // to the output file and we didn't explicitly discard.  We should
  // always map all portions of input merge sections.
  gold_assert(found);

  if (output_offset == -1)
    return 0;
  else
    return this->output_start_address_ + output_offset;
}

// Track_relocs methods.

// Initialize the class to track the relocs.  This gets the object,
// the reloc section index, and the type of the relocs.  This returns
// false if something goes wrong.

template<int size, bool big_endian>
bool
Track_relocs<size, big_endian>::initialize(
    Object* object,
    unsigned int reloc_shndx,
    unsigned int reloc_type)
{
  // If RELOC_SHNDX is -1U, it means there is more than one reloc
  // section for the .eh_frame section.  We can't handle that case.
  if (reloc_shndx == -1U)
    return false;

  // If RELOC_SHNDX is 0, there is no reloc section.
  if (reloc_shndx == 0)
    return true;

  // Get the contents of the reloc section.
  this->prelocs_ = object->section_contents(reloc_shndx, &this->len_, false);

  if (reloc_type == elfcpp::SHT_REL)
    this->reloc_size_ = elfcpp::Elf_sizes<size>::rel_size;
  else if (reloc_type == elfcpp::SHT_RELA)
    this->reloc_size_ = elfcpp::Elf_sizes<size>::rela_size;
  else
    gold_unreachable();

  if (this->len_ % this->reloc_size_ != 0)
    {
      object->error(_("reloc section size %zu is not a multiple of "
		      "reloc size %d\n"),
		    static_cast<size_t>(this->len_),
		    this->reloc_size_);
      return false;
    }

  return true;
}

// Return the offset of the next reloc, or -1 if there isn't one.

template<int size, bool big_endian>
off_t
Track_relocs<size, big_endian>::next_offset() const
{
  if (this->pos_ >= this->len_)
    return -1;

  // Rel and Rela start out the same, so we can always use Rel to find
  // the r_offset value.
  elfcpp::Rel<size, big_endian> rel(this->prelocs_ + this->pos_);
  return rel.get_r_offset();
}

// Return the index of the symbol referenced by the next reloc, or -1U
// if there aren't any more relocs.

template<int size, bool big_endian>
unsigned int
Track_relocs<size, big_endian>::next_symndx() const
{
  if (this->pos_ >= this->len_)
    return -1U;
  Sized_target<size, big_endian>* target
      = parameters->sized_target<size, big_endian>();
  return target->get_r_sym(this->prelocs_ + this->pos_);
}

// Return the addend of the next reloc, or 0 if there isn't one.

template<int size, bool big_endian>
uint64_t
Track_relocs<size, big_endian>::next_addend() const
{
  if (this->pos_ >= this->len_)
    return 0;
  if (this->reloc_size_ == elfcpp::Elf_sizes<size>::rel_size)
    return 0;
  elfcpp::Rela<size, big_endian> rela(this->prelocs_ + this->pos_);
  return rela.get_r_addend();
}

// Advance to the next reloc whose r_offset is greater than or equal
// to OFFSET.  Return the number of relocs we skip.

template<int size, bool big_endian>
int
Track_relocs<size, big_endian>::advance(off_t offset)
{
  int ret = 0;
  while (this->pos_ < this->len_)
    {
      // Rel and Rela start out the same, so we can always use Rel to
      // find the r_offset value.
      elfcpp::Rel<size, big_endian> rel(this->prelocs_ + this->pos_);
      if (static_cast<off_t>(rel.get_r_offset()) >= offset)
	break;
      // Skip R_*_NONE relocation entries with r_sym of zero
      // without counting.
      if (rel.get_r_info() != 0)
	++ret;
      this->pos_ += this->reloc_size_;
    }
  return ret;
}

// Instantiate the templates we need.

#ifdef HAVE_TARGET_32_LITTLE
template
void
Sized_relobj_file<32, false>::do_read_relocs(Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Sized_relobj_file<32, true>::do_read_relocs(Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Sized_relobj_file<64, false>::do_read_relocs(Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Sized_relobj_file<64, true>::do_read_relocs(Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Sized_relobj_file<32, false>::do_gc_process_relocs(Symbol_table* symtab,
						   Layout* layout,
						   Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Sized_relobj_file<32, true>::do_gc_process_relocs(Symbol_table* symtab,
						  Layout* layout,
						  Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Sized_relobj_file<64, false>::do_gc_process_relocs(Symbol_table* symtab,
						   Layout* layout,
						   Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Sized_relobj_file<64, true>::do_gc_process_relocs(Symbol_table* symtab,
						  Layout* layout,
						  Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Sized_relobj_file<32, false>::do_scan_relocs(Symbol_table* symtab,
					     Layout* layout,
					     Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Sized_relobj_file<32, true>::do_scan_relocs(Symbol_table* symtab,
					    Layout* layout,
					    Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Sized_relobj_file<64, false>::do_scan_relocs(Symbol_table* symtab,
					     Layout* layout,
					     Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Sized_relobj_file<64, true>::do_scan_relocs(Symbol_table* symtab,
					    Layout* layout,
					    Read_relocs_data* rd);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Sized_relobj_file<32, false>::do_relocate(const Symbol_table* symtab,
					  const Layout* layout,
					  Output_file* of);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Sized_relobj_file<32, true>::do_relocate(const Symbol_table* symtab,
					 const Layout* layout,
					 Output_file* of);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Sized_relobj_file<64, false>::do_relocate(const Symbol_table* symtab,
					  const Layout* layout,
					  Output_file* of);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Sized_relobj_file<64, true>::do_relocate(const Symbol_table* symtab,
					 const Layout* layout,
					 Output_file* of);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Sized_relobj_file<32, false>::do_relocate_sections(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews);

template
void
Sized_relobj_file<32, false>::relocate_section_range(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews,
    unsigned int start_shndx,
    unsigned int end_shndx);

template
unsigned char*
Sized_relobj_file<32, false>::do_get_output_view(
    unsigned int shndx,
    section_size_type* plen) const;
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Sized_relobj_file<32, true>::do_relocate_sections(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews);

template
void
Sized_relobj_file<32, true>::relocate_section_range(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews,
    unsigned int start_shndx,
    unsigned int end_shndx);

template
unsigned char*
Sized_relobj_file<32, true>::do_get_output_view(
    unsigned int shndx,
    section_size_type* plen) const;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Sized_relobj_file<64, false>::do_relocate_sections(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews);

template
void
Sized_relobj_file<64, false>::relocate_section_range(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews,
    unsigned int start_shndx,
    unsigned int end_shndx);

template
unsigned char*
Sized_relobj_file<64, false>::do_get_output_view(
    unsigned int shndx,
    section_size_type* plen) const;
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Sized_relobj_file<64, true>::do_relocate_sections(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews);

template
void
Sized_relobj_file<64, true>::relocate_section_range(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    Views* pviews,
    unsigned int start_shndx,
    unsigned int end_shndx);

template
unsigned char*
Sized_relobj_file<64, true>::do_get_output_view(
    unsigned int shndx,
    section_size_type* plen) const;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Sized_relobj_file<32, false>::initialize_input_to_output_maps();

template
void
Sized_relobj_file<32, false>::free_input_to_output_maps();
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Sized_relobj_file<32, true>::initialize_input_to_output_maps();

template
void
Sized_relobj_file<32, true>::free_input_to_output_maps();
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Sized_relobj_file<64, false>::initialize_input_to_output_maps();

template
void
Sized_relobj_file<64, false>::free_input_to_output_maps();
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Sized_relobj_file<64, true>::initialize_input_to_output_maps();

template
void
Sized_relobj_file<64, true>::free_input_to_output_maps();
#endif

#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
template
class Merged_symbol_value<32>;
#endif

#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
template
class Merged_symbol_value<64>;
#endif

#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
template
class Symbol_value<32>;
#endif

#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
template
class Symbol_value<64>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Track_relocs<32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Track_relocs<32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Track_relocs<64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Track_relocs<64, true>;
#endif

} // End namespace gold.
