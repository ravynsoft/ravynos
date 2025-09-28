// gold.cc -- main linker functions

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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <algorithm>
#include "libiberty.h"

#include "options.h"
#include "target-select.h"
#include "debug.h"
#include "workqueue.h"
#include "dirsearch.h"
#include "readsyms.h"
#include "symtab.h"
#include "common.h"
#include "object.h"
#include "layout.h"
#include "reloc.h"
#include "defstd.h"
#include "plugin.h"
#include "gc.h"
#include "icf.h"
#include "incremental.h"
#include "timer.h"

namespace gold
{

class Object;

const char* program_name;

static Task*
process_incremental_input(Incremental_binary*, unsigned int, Input_objects*,
			  Symbol_table*, Layout*, Dirsearch*, Mapfile*,
			  Task_token*, Task_token*);

void
gold_exit(Exit_status status)
{
  if (parameters != NULL
      && parameters->options_valid()
      && parameters->options().has_plugins())
    parameters->options().plugins()->cleanup();
  if (status != GOLD_OK && parameters != NULL && parameters->options_valid())
    unlink_if_ordinary(parameters->options().output_file_name());
  exit(status);
}

void
gold_nomem()
{
  // We are out of memory, so try hard to print a reasonable message.
  // Note that we don't try to translate this message, since the
  // translation process itself will require memory.

  // LEN only exists to avoid a pointless warning when write is
  // declared with warn_use_result, as when compiling with
  // -D_USE_FORTIFY on GNU/Linux.  Casting to void does not appear to
  // work, at least not with gcc 4.3.0.

  ssize_t len = write(2, program_name, strlen(program_name));
  if (len >= 0)
    {
      const char* const s = ": out of memory\n";
      len = write(2, s, strlen(s));
    }
  gold_exit(GOLD_ERR);
}

// Handle an unreachable case.

void
do_gold_unreachable(const char* filename, int lineno, const char* function)
{
  fprintf(stderr, _("%s: internal error in %s, at %s:%d\n"),
	  program_name, function, filename, lineno);
  gold_exit(GOLD_ERR);
}

// This class arranges to run the functions done in the middle of the
// link.  It is just a closure.

class Middle_runner : public Task_function_runner
{
 public:
  Middle_runner(const General_options& options,
		const Input_objects* input_objects,
		Symbol_table* symtab,
		Layout* layout, Mapfile* mapfile)
    : options_(options), input_objects_(input_objects), symtab_(symtab),
      layout_(layout), mapfile_(mapfile)
  { }

  void
  run(Workqueue*, const Task*);

 private:
  const General_options& options_;
  const Input_objects* input_objects_;
  Symbol_table* symtab_;
  Layout* layout_;
  Mapfile* mapfile_;
};

void
Middle_runner::run(Workqueue* workqueue, const Task* task)
{
  queue_middle_tasks(this->options_, task, this->input_objects_, this->symtab_,
		     this->layout_, workqueue, this->mapfile_);
}

// This class arranges the tasks to process the relocs for garbage collection.

class Gc_runner : public Task_function_runner
{
  public:
   Gc_runner(const General_options& options,
	     const Input_objects* input_objects,
	     Symbol_table* symtab,
	     Layout* layout, Mapfile* mapfile)
    : options_(options), input_objects_(input_objects), symtab_(symtab),
      layout_(layout), mapfile_(mapfile)
   { }

  void
  run(Workqueue*, const Task*);

 private:
  const General_options& options_;
  const Input_objects* input_objects_;
  Symbol_table* symtab_;
  Layout* layout_;
  Mapfile* mapfile_;
};

void
Gc_runner::run(Workqueue* workqueue, const Task* task)
{
  queue_middle_gc_tasks(this->options_, task, this->input_objects_,
			this->symtab_, this->layout_, workqueue,
			this->mapfile_);
}

// Queue up the initial set of tasks for this link job.

void
queue_initial_tasks(const General_options& options,
		    Dirsearch& search_path,
		    const Command_line& cmdline,
		    Workqueue* workqueue, Input_objects* input_objects,
		    Symbol_table* symtab, Layout* layout, Mapfile* mapfile)
{
  if (cmdline.number_of_input_files() == 0)
    {
      bool is_ok = false;
      if (options.printed_version())
	is_ok = true;
      if (options.print_output_format())
	{
	  print_output_format();
	  is_ok = true;
	}
      if (is_ok)
	gold_exit(GOLD_OK);
      gold_fatal(_("no input files"));
    }

  int thread_count = options.thread_count_initial();
  if (thread_count == 0)
    thread_count = cmdline.number_of_input_files();
  workqueue->set_thread_count(thread_count);

  // For incremental links, the base output file.
  Incremental_binary* ibase = NULL;

  if (parameters->incremental_update())
    {
      Output_file* of = new Output_file(options.output_file_name());
      if (of->open_base_file(options.incremental_base(), true))
	{
	  ibase = open_incremental_binary(of);
	  if (ibase != NULL
	      && ibase->check_inputs(cmdline, layout->incremental_inputs()))
	    ibase->init_layout(layout);
	  else
	    {
	      delete ibase;
	      ibase = NULL;
	      of->close();
	    }
	}
      if (ibase == NULL)
	{
	  if (set_parameters_incremental_full())
	    gold_info(_("linking with --incremental-full"));
	  else
	    gold_fallback(_("restart link with --incremental-full"));
	}
    }

  // Read the input files.  We have to add the symbols to the symbol
  // table in order.  We do this by creating a separate blocker for
  // each input file.  We associate the blocker with the following
  // input file, to give us a convenient place to delete it.
  Task_token* this_blocker = NULL;
  if (ibase == NULL)
    {
      // Normal link.  Queue a Read_symbols task for each input file
      // on the command line.
      for (Command_line::const_iterator p = cmdline.begin();
	   p != cmdline.end();
	   ++p)
	{
	  Task_token* next_blocker = new Task_token(true);
	  next_blocker->add_blocker();
	  workqueue->queue(new Read_symbols(input_objects, symtab, layout,
					    &search_path, 0, mapfile, &*p, NULL,
					    NULL, this_blocker, next_blocker));
	  this_blocker = next_blocker;
	}
    }
  else
    {
      // Incremental update link.  Process the list of input files
      // stored in the base file, and queue a task for each file:
      // a Read_symbols task for a changed file, and an Add_symbols task
      // for an unchanged file.  We need to mark all the space used by
      // unchanged files before we can start any tasks running.
      unsigned int input_file_count = ibase->input_file_count();
      std::vector<Task*> tasks;
      tasks.reserve(input_file_count);
      for (unsigned int i = 0; i < input_file_count; ++i)
	{
	  Task_token* next_blocker = new Task_token(true);
	  next_blocker->add_blocker();
	  Task* t = process_incremental_input(ibase, i, input_objects, symtab,
					      layout, &search_path, mapfile,
					      this_blocker, next_blocker);
	  tasks.push_back(t);
	  this_blocker = next_blocker;
	}
      // Now we can queue the tasks.
      for (unsigned int i = 0; i < tasks.size(); i++)
	workqueue->queue(tasks[i]);
    }

  if (options.has_plugins())
    {
      Task_token* next_blocker = new Task_token(true);
      next_blocker->add_blocker();
      workqueue->queue(new Plugin_hook(options, input_objects, symtab, layout,
				       &search_path, mapfile, this_blocker,
				       next_blocker));
      this_blocker = next_blocker;
    }

  if (options.relocatable()
      && (options.gc_sections() || options.icf_enabled()))
    gold_error(_("cannot mix -r with --gc-sections or --icf"));

  if (options.gc_sections() || options.icf_enabled())
    {
      workqueue->queue(new Task_function(new Gc_runner(options,
						       input_objects,
						       symtab,
						       layout,
						       mapfile),
					 this_blocker,
					 "Task_function Gc_runner"));
    }
  else
    {
      workqueue->queue(new Task_function(new Middle_runner(options,
							   input_objects,
							   symtab,
							   layout,
							   mapfile),
					 this_blocker,
					 "Task_function Middle_runner"));
    }
}

// Process an incremental input file: if it is unchanged from the previous
// link, return a task to add its symbols from the base file's incremental
// info; if it has changed, return a normal Read_symbols task.  We create a
// task for every input file, if only to report the file for rebuilding the
// incremental info.

static Task*
process_incremental_input(Incremental_binary* ibase,
			  unsigned int input_file_index,
			  Input_objects* input_objects,
			  Symbol_table* symtab,
			  Layout* layout,
			  Dirsearch* search_path,
			  Mapfile* mapfile,
			  Task_token* this_blocker,
			  Task_token* next_blocker)
{
  const Incremental_binary::Input_reader* input_reader =
      ibase->get_input_reader(input_file_index);
  Incremental_input_type input_type = input_reader->type();

  // Get the input argument corresponding to this input file, matching on
  // the argument serial number.  If the input file cannot be matched
  // to an existing input argument, synthesize a new one.
  const Input_argument* input_argument =
      ibase->get_input_argument(input_file_index);
  if (input_argument == NULL)
    {
      Input_file_argument file(input_reader->filename(),
			       Input_file_argument::INPUT_FILE_TYPE_FILE,
			       "", false, parameters->options());
      Input_argument* arg = new Input_argument(file);
      arg->set_script_info(ibase->get_script_info(input_file_index));
      input_argument = arg;
    }

  gold_debug(DEBUG_INCREMENTAL, "Incremental object: %s, type %d",
	     input_reader->filename(), input_type);

  if (input_type == INCREMENTAL_INPUT_SCRIPT)
    {
      // Incremental_binary::check_inputs should have cancelled the
      // incremental update if the script has changed.
      gold_assert(!ibase->file_has_changed(input_file_index));
      return new Check_script(layout, ibase, input_file_index, input_reader,
			      this_blocker, next_blocker);
    }

  if (input_type == INCREMENTAL_INPUT_ARCHIVE)
    {
      Incremental_library* lib = ibase->get_library(input_file_index);
      gold_assert(lib != NULL);
      if (lib->filename() == "/group/"
	  || !ibase->file_has_changed(input_file_index))
	{
	  // Queue a task to check that no references have been added to any
	  // of the library's unused symbols.
	  return new Check_library(symtab, layout, ibase, input_file_index,
				   input_reader, this_blocker, next_blocker);
	}
      else
	{
	  // Queue a Read_symbols task to process the archive normally.
	  return new Read_symbols(input_objects, symtab, layout, search_path,
				  0, mapfile, input_argument, NULL, NULL,
				  this_blocker, next_blocker);
	}
    }

  if (input_type == INCREMENTAL_INPUT_ARCHIVE_MEMBER)
    {
      // For archive members, check the timestamp of the containing archive.
      Incremental_library* lib = ibase->get_library(input_file_index);
      gold_assert(lib != NULL);
      // Process members of a --start-lib/--end-lib group as normal objects.
      if (lib->filename() != "/group/")
	{
	  if (ibase->file_has_changed(lib->input_file_index()))
	    {
	      return new Read_member(input_objects, symtab, layout, mapfile,
				     input_reader, this_blocker, next_blocker);
	    }
	  else
	    {
	      // The previous contributions from this file will be kept.
	      // Mark the pieces of output sections contributed by this
	      // object.
	      ibase->reserve_layout(input_file_index);
	      Object* obj = make_sized_incremental_object(ibase,
							  input_file_index,
							  input_type,
							  input_reader);
	      return new Add_symbols(input_objects, symtab, layout,
				     search_path, 0, mapfile, input_argument,
				     obj, lib, NULL, this_blocker,
				     next_blocker);
	    }
	}
    }

  // Normal object file or shared library.  Check if the file has changed
  // since the last incremental link.
  if (ibase->file_has_changed(input_file_index))
    {
      return new Read_symbols(input_objects, symtab, layout, search_path, 0,
			      mapfile, input_argument, NULL, NULL,
			      this_blocker, next_blocker);
    }
  else
    {
      // The previous contributions from this file will be kept.
      // Mark the pieces of output sections contributed by this object.
      ibase->reserve_layout(input_file_index);
      Object* obj = make_sized_incremental_object(ibase,
						  input_file_index,
						  input_type,
						  input_reader);
      return new Add_symbols(input_objects, symtab, layout, search_path, 0,
			     mapfile, input_argument, obj, NULL, NULL,
			     this_blocker, next_blocker);
    }
}

// Queue up a set of tasks to be done before queueing the middle set
// of tasks.  This is only necessary when garbage collection
// (--gc-sections) of unused sections is desired.  The relocs are read
// and processed here early to determine the garbage sections before the
// relocs can be scanned in later tasks.

void
queue_middle_gc_tasks(const General_options& options,
		      const Task* ,
		      const Input_objects* input_objects,
		      Symbol_table* symtab,
		      Layout* layout,
		      Workqueue* workqueue,
		      Mapfile* mapfile)
{
  // Read_relocs for all the objects must be done and processed to find
  // unused sections before any scanning of the relocs can take place.
  Task_token* this_blocker = NULL;
  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      Task_token* next_blocker = new Task_token(true);
      next_blocker->add_blocker();
      workqueue->queue(new Read_relocs(symtab, layout, *p, this_blocker,
				       next_blocker));
      this_blocker = next_blocker;
    }

  // If we are given only archives in input, we have no regular
  // objects and THIS_BLOCKER is NULL here.  Create a dummy
  // blocker here so that we can run the middle tasks immediately.
  if (this_blocker == NULL)
    {
      gold_assert(input_objects->number_of_relobjs() == 0);
      this_blocker = new Task_token(true);
    }

  workqueue->queue(new Task_function(new Middle_runner(options,
						       input_objects,
						       symtab,
						       layout,
						       mapfile),
				     this_blocker,
				     "Task_function Middle_runner"));
}

// Queue up the middle set of tasks.  These are the tasks which run
// after all the input objects have been found and all the symbols
// have been read, but before we lay out the output file.

void
queue_middle_tasks(const General_options& options,
		   const Task* task,
		   const Input_objects* input_objects,
		   Symbol_table* symtab,
		   Layout* layout,
		   Workqueue* workqueue,
		   Mapfile* mapfile)
{
  Timer* timer = parameters->timer();
  if (timer != NULL)
    timer->stamp(0);

  // We have to support the case of not seeing any input objects, and
  // generate an empty file.  Existing builds depend on being able to
  // pass an empty archive to the linker and get an empty object file
  // out.  In order to do this we need to use a default target.
  if (input_objects->number_of_input_objects() == 0
      && layout->incremental_base() == NULL)
    parameters_force_valid_target();

  // Add any symbols named with -u options to the symbol table.
  symtab->add_undefined_symbols_from_command_line(layout);

  // If garbage collection was chosen, relocs have been read and processed
  // at this point by pre_middle_tasks.  Layout can then be done for all
  // objects.
  if (parameters->options().gc_sections())
    {
      // Find the start symbol if any.
      Symbol* sym = symtab->lookup(parameters->entry());
      if (sym != NULL)
	symtab->gc_mark_symbol(sym);
      sym = symtab->lookup(parameters->options().init());
      if (sym != NULL && sym->is_defined() && !sym->is_from_dynobj())
	symtab->gc_mark_symbol(sym);
      sym = symtab->lookup(parameters->options().fini());
      if (sym != NULL && sym->is_defined() && !sym->is_from_dynobj())
	symtab->gc_mark_symbol(sym);
      // Symbols named with -u should not be considered garbage.
      symtab->gc_mark_undef_symbols(layout);
      gold_assert(symtab->gc() != NULL);
      // Do a transitive closure on all references to determine the worklist.
      symtab->gc()->do_transitive_closure();
    }

  // If identical code folding (--icf) is chosen it makes sense to do it
  // only after garbage collection (--gc-sections) as we do not want to
  // be folding sections that will be garbage.
  if (parameters->options().icf_enabled())
    {
      symtab->icf()->find_identical_sections(input_objects, symtab);
    }

  // Call Object::layout for the second time to determine the
  // output_sections for all referenced input sections.  When
  // --gc-sections or --icf is turned on, or when certain input
  // sections have to be mapped to unique segments, Object::layout
  // is called twice.  It is called the first time when symbols
  // are added.
  if (parameters->options().gc_sections()
      || parameters->options().icf_enabled()
      || layout->is_unique_segment_for_sections_specified())
    {
      for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
	   p != input_objects->relobj_end();
	   ++p)
	{
	  Task_lock_obj<Object> tlo(task, *p);
	  (*p)->layout(symtab, layout, NULL);
	}
    }

  // Layout deferred objects due to plugins.
  if (parameters->options().has_plugins())
    {
      Plugin_manager* plugins = parameters->options().plugins();
      gold_assert(plugins != NULL);
      plugins->layout_deferred_objects();
    }

  // Finalize the .eh_frame section.
  layout->finalize_eh_frame_section();

  /* If plugins have specified a section order, re-arrange input sections
     according to a specified section order.  If --section-ordering-file is
     also specified, do not do anything here.  */
  if (parameters->options().has_plugins()
      && layout->is_section_ordering_specified()
      && !parameters->options().section_ordering_file ())
    {
      for (Layout::Section_list::const_iterator p
	     = layout->section_list().begin();
	   p != layout->section_list().end();
	   ++p)
	(*p)->update_section_layout(layout->get_section_order_map());
    }

  if (parameters->options().gc_sections()
      || parameters->options().icf_enabled())
    {
      for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
	   p != input_objects->relobj_end();
	   ++p)
	{
	  // Update the value of output_section stored in rd.
	  Read_relocs_data* rd = (*p)->get_relocs_data();
	  for (Read_relocs_data::Relocs_list::iterator q = rd->relocs.begin();
	       q != rd->relocs.end();
	       ++q)
	    {
	      q->output_section = (*p)->output_section(q->data_shndx);
	      q->needs_special_offset_handling =
		      (*p)->is_output_section_offset_invalid(q->data_shndx);
	    }
	}
    }

  int thread_count = options.thread_count_middle();
  if (thread_count == 0)
    thread_count = std::max(2, input_objects->number_of_input_objects());
  workqueue->set_thread_count(thread_count);

  // Now we have seen all the input files.
  const bool doing_static_link =
    (!input_objects->any_dynamic()
     && !parameters->options().output_is_position_independent());
  set_parameters_doing_static_link(doing_static_link);
  if (!doing_static_link && options.is_static())
    {
      // We print out just the first .so we see; there may be others.
      gold_assert(input_objects->dynobj_begin() != input_objects->dynobj_end());
      gold_error(_("cannot mix -static with dynamic object %s"),
		 (*input_objects->dynobj_begin())->name().c_str());
    }
  if (!doing_static_link && parameters->options().relocatable())
    gold_fatal(_("cannot mix -r with dynamic object %s"),
	       (*input_objects->dynobj_begin())->name().c_str());
  if (!doing_static_link
      && options.oformat_enum() != General_options::OBJECT_FORMAT_ELF)
    gold_fatal(_("cannot use non-ELF output format with dynamic object %s"),
	       (*input_objects->dynobj_begin())->name().c_str());

  if (parameters->options().relocatable())
    {
      Input_objects::Relobj_iterator p = input_objects->relobj_begin();
      if (p != input_objects->relobj_end())
	{
	  bool uses_split_stack = (*p)->uses_split_stack();
	  for (++p; p != input_objects->relobj_end(); ++p)
	    {
	      if ((*p)->uses_split_stack() != uses_split_stack)
		{
		  const char *name1
		    = (*input_objects->relobj_begin())->name().c_str();
		  const char *name2 = (*p)->name().c_str();
		  const char *name_split = uses_split_stack ? name1 : name2;
		  const char *name_nosplit = uses_split_stack ? name2 : name1;
		  gold_fatal(_("cannot mix split-stack '%s' and "
			       "non-split-stack '%s' when using -r"),
			     name_split, name_nosplit);
		}
	    }
	}
    }

  // For incremental updates, record the existing GOT and PLT entries,
  // and the COPY relocations.
  if (parameters->incremental_update())
    {
      Incremental_binary* ibase = layout->incremental_base();
      ibase->process_got_plt(symtab, layout);
      ibase->emit_copy_relocs(symtab);
    }

  if (is_debugging_enabled(DEBUG_SCRIPT))
    layout->script_options()->print(stderr);

  // For each dynamic object, record whether we've seen all the
  // dynamic objects that it depends upon.
  input_objects->check_dynamic_dependencies();

  // Do the --no-undefined-version check.
  if (!parameters->options().undefined_version())
    {
      Script_options* so = layout->script_options();
      so->version_script_info()->check_unmatched_names(symtab);
    }

  // Create any automatic note sections.
  layout->create_notes();

  // Create any output sections required by any linker script.
  layout->create_script_sections();

  // Define some sections and symbols needed for a dynamic link.  This
  // handles some cases we want to see before we read the relocs.
  layout->create_initial_dynamic_sections(symtab);

  // Define symbols from any linker scripts.
  layout->define_script_symbols(symtab);

  // TODO(csilvers): figure out a more principled way to get the target
  Target* target = const_cast<Target*>(&parameters->target());

  // Attach sections to segments.
  layout->attach_sections_to_segments(target);

  if (!parameters->options().relocatable())
    {
      // Predefine standard symbols.
      define_standard_symbols(symtab, layout);

      // Define __start and __stop symbols for output sections where
      // appropriate.
      layout->define_section_symbols(symtab);

      // Define target-specific symbols.
      target->define_standard_symbols(symtab, layout);
    }

  // Make sure we have symbols for any required group signatures.
  layout->define_group_signatures(symtab);

  Task_token* this_blocker = NULL;

  // Allocate common symbols.  We use a blocker to run this before the
  // Scan_relocs tasks, because it writes to the symbol table just as
  // they do.
  if (parameters->options().define_common())
    {
      this_blocker = new Task_token(true);
      this_blocker->add_blocker();
      workqueue->queue(new Allocate_commons_task(symtab, layout, mapfile,
						 this_blocker));
    }

  // If doing garbage collection, the relocations have already been read.
  // Otherwise, read and scan the relocations.
  if (parameters->options().gc_sections()
      || parameters->options().icf_enabled())
    {
      for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
	   p != input_objects->relobj_end();
	   ++p)
	{
	  Task_token* next_blocker = new Task_token(true);
	  next_blocker->add_blocker();
	  workqueue->queue(new Scan_relocs(symtab, layout, *p,
					   (*p)->get_relocs_data(),
					   this_blocker, next_blocker));
	  this_blocker = next_blocker;
	}
    }
  else
    {
      // Read the relocations of the input files.  We do this to find
      // which symbols are used by relocations which require a GOT and/or
      // a PLT entry, or a COPY reloc.  When we implement garbage
      // collection we will do it here by reading the relocations in a
      // breadth first search by references.
      //
      // We could also read the relocations during the first pass, and
      // mark symbols at that time.  That is how the old GNU linker works.
      // Doing that is more complex, since we may later decide to discard
      // some of the sections, and thus change our minds about the types
      // of references made to the symbols.
      for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
	   p != input_objects->relobj_end();
	   ++p)
	{
	  Task_token* next_blocker = new Task_token(true);
	  next_blocker->add_blocker();
	  workqueue->queue(new Read_relocs(symtab, layout, *p, this_blocker,
					   next_blocker));
	  this_blocker = next_blocker;
	}
    }

  if (this_blocker == NULL)
    {
      if (input_objects->number_of_relobjs() == 0)
	{
	  // If we are given only archives in input, we have no regular
	  // objects and THIS_BLOCKER is NULL here.  Create a dummy
	  // blocker here so that we can run the layout task immediately.
	  this_blocker = new Task_token(true);
	}
      else
	{
	  // If we failed to open any input files, it's possible for
	  // THIS_BLOCKER to be NULL here.  There's no real point in
	  // continuing if that happens.
	  gold_assert(parameters->errors()->error_count() > 0);
	  gold_exit(GOLD_ERR);
	}
    }

  // When all those tasks are complete, we can start laying out the
  // output file.
  workqueue->queue(new Task_function(new Layout_task_runner(options,
							    input_objects,
							    symtab,
							    target,
							    layout,
							    mapfile),
				     this_blocker,
				     "Task_function Layout_task_runner"));
}

// Queue up the final set of tasks.  This is called at the end of
// Layout_task.

void
queue_final_tasks(const General_options& options,
		  const Input_objects* input_objects,
		  const Symbol_table* symtab,
		  Layout* layout,
		  Workqueue* workqueue,
		  Output_file* of)
{
  Timer* timer = parameters->timer();
  if (timer != NULL)
    timer->stamp(1);

  int thread_count = options.thread_count_final();
  if (thread_count == 0)
    thread_count = std::max(2, input_objects->number_of_input_objects());
  workqueue->set_thread_count(thread_count);

  bool any_postprocessing_sections = layout->any_postprocessing_sections();

  // Use a blocker to wait until all the input sections have been
  // written out.
  Task_token* input_sections_blocker = NULL;
  if (!any_postprocessing_sections)
    {
      input_sections_blocker = new Task_token(true);
      // Write_symbols_task, Relocate_tasks.
      input_sections_blocker->add_blocker();
      input_sections_blocker->add_blockers(input_objects->number_of_relobjs());
    }

  // Use a blocker to block any objects which have to wait for the
  // output sections to complete before they can apply relocations.
  Task_token* output_sections_blocker = new Task_token(true);
  output_sections_blocker->add_blocker();

  // Use a blocker to block the final cleanup task.
  Task_token* final_blocker = new Task_token(true);
  // Write_symbols_task, Write_sections_task, Write_data_task,
  // Relocate_tasks.
  final_blocker->add_blockers(3);
  final_blocker->add_blockers(input_objects->number_of_relobjs());
  if (!any_postprocessing_sections)
    final_blocker->add_blocker();

  // Queue a task to write out the symbol table.
  workqueue->queue(new Write_symbols_task(layout,
					  symtab,
					  input_objects,
					  layout->sympool(),
					  layout->dynpool(),
					  of,
					  final_blocker));

  // Queue a task to write out the output sections.
  workqueue->queue(new Write_sections_task(layout, of, output_sections_blocker,
					   input_sections_blocker,
					   final_blocker));

  // Queue a task to write out everything else.
  workqueue->queue(new Write_data_task(layout, symtab, of, final_blocker));

  // Queue a task for each input object to relocate the sections and
  // write out the local symbols.
  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    workqueue->queue(new Relocate_task(symtab, layout, *p, of,
				       input_sections_blocker,
				       output_sections_blocker,
				       final_blocker));

  // Queue a task to write out the output sections which depend on
  // input sections.  If there are any sections which require
  // postprocessing, then we need to do this last, since it may resize
  // the output file.
  if (!any_postprocessing_sections)
    {
      Task* t = new Write_after_input_sections_task(layout, of,
						    input_sections_blocker,
						    final_blocker);
      workqueue->queue(t);
    }
  else
    {
      Task_token* new_final_blocker = new Task_token(true);
      new_final_blocker->add_blocker();
      Task* t = new Write_after_input_sections_task(layout, of,
						    final_blocker,
						    new_final_blocker);
      workqueue->queue(t);
      final_blocker = new_final_blocker;
    }

  // Create tasks for tree-style build ID computation, if necessary.
  if (strcmp(options.build_id(), "tree") == 0)
    {
      // Queue a task to compute the build id.  This will be blocked by
      // FINAL_BLOCKER, and will in turn schedule the task to close
      // the output file.
      workqueue->queue(new Task_function(new Build_id_task_runner(&options,
								  layout,
								  of),
					 final_blocker,
					 "Task_function Build_id_task_runner"));
    }
  else
    {
      // Queue a task to close the output file.  This will be blocked by
      // FINAL_BLOCKER.
      workqueue->queue(new Task_function(new Close_task_runner(&options, layout,
							       of, NULL, 0),
					 final_blocker,
					 "Task_function Close_task_runner"));
    }

}

} // End namespace gold.
