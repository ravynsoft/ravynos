// readsyms.cc -- read input file symbols for gold

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

#include "elfcpp.h"
#include "options.h"
#include "dirsearch.h"
#include "symtab.h"
#include "object.h"
#include "archive.h"
#include "script.h"
#include "readsyms.h"
#include "plugin.h"
#include "layout.h"
#include "incremental.h"

namespace gold
{

// If we fail to open the object, then we won't create an Add_symbols
// task.  However, we still need to unblock the token, or else the
// link won't proceed to generate more error messages.  We can only
// unblock tokens when the workqueue lock is held, so we need a dummy
// task to do that.  The dummy task has to maintain the right sequence
// of blocks, so we need both this_blocker and next_blocker.

class Unblock_token : public Task
{
 public:
  Unblock_token(Task_token* this_blocker, Task_token* next_blocker)
    : this_blocker_(this_blocker), next_blocker_(next_blocker)
  { }

  ~Unblock_token()
  {
    if (this->this_blocker_ != NULL)
      delete this->this_blocker_;
  }

  Task_token*
  is_runnable()
  {
    if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
      return this->this_blocker_;
    return NULL;
  }

  void
  locks(Task_locker* tl)
  { tl->add(this, this->next_blocker_); }

  void
  run(Workqueue*)
  { }

  std::string
  get_name() const
  { return "Unblock_token"; }

 private:
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

// Class read_symbols.

Read_symbols::~Read_symbols()
{
  // The this_blocker_ and next_blocker_ pointers are passed on to the
  // Add_symbols task.
}

// If appropriate, issue a warning about skipping an incompatible
// file.

void
Read_symbols::incompatible_warning(const Input_argument* input_argument,
				   const Input_file* input_file)
{
  if (parameters->options().warn_search_mismatch())
    gold_warning("skipping incompatible %s while searching for %s",
		 input_file->filename().c_str(),
		 input_argument->file().name());
}

// Requeue a Read_symbols task to search for the next object with the
// same name.

void
Read_symbols::requeue(Workqueue* workqueue, Input_objects* input_objects,
		      Symbol_table* symtab, Layout* layout, Dirsearch* dirpath,
		      int dirindex, Mapfile* mapfile,
		      const Input_argument* input_argument,
		      Input_group* input_group, Task_token* next_blocker)
{
  // Bump the directory search index.
  ++dirindex;

  // We don't need to worry about this_blocker, since we already
  // reached it.  However, we are removing the blocker on next_blocker
  // because the calling task is completing.  So we need to add a new
  // blocker.  Since next_blocker may be shared by several tasks, we
  // need to increment the count with the workqueue lock held.
  workqueue->add_blocker(next_blocker);

  workqueue->queue(new Read_symbols(input_objects, symtab, layout, dirpath,
				    dirindex, mapfile, input_argument,
				    input_group, NULL, NULL, next_blocker));
}

// Return whether a Read_symbols task is runnable.  We can read an
// ordinary input file immediately.  For an archive specified using
// -l, we have to wait until the search path is complete.

Task_token*
Read_symbols::is_runnable()
{
  if (this->input_argument_->is_file()
      && this->input_argument_->file().may_need_search()
      && this->dirpath_->token()->is_blocked())
    return this->dirpath_->token();

  return NULL;
}

// Return a Task_locker for a Read_symbols task.  We don't need any
// locks here.

void
Read_symbols::locks(Task_locker* tl)
{
  if (this->member_ != NULL)
    tl->add(this, this->next_blocker_);
}

// Run a Read_symbols task.

void
Read_symbols::run(Workqueue* workqueue)
{
  // If we didn't queue a new task, then we need to explicitly unblock
  // the token. If the object is a member of a lib group, however,
  // the token was already added to the list of locks for the task,
  // and it will be unblocked automatically at the end of the task.
  if (!this->do_read_symbols(workqueue) && this->member_ == NULL)
    workqueue->queue_soon(new Unblock_token(this->this_blocker_,
					    this->next_blocker_));
}

// Handle a whole lib group. Other than collecting statistics, this just
// mimics what we do for regular object files in the command line.

bool
Read_symbols::do_whole_lib_group(Workqueue* workqueue)
{
  const Input_file_lib* lib_group = this->input_argument_->lib();

  ++Lib_group::total_lib_groups;

  Task_token* this_blocker = this->this_blocker_;
  for (Input_file_lib::const_iterator i = lib_group->begin();
       i != lib_group->end();
       ++i)
    {
      ++Lib_group::total_members;
      ++Lib_group::total_members_loaded;

      const Input_argument* arg = &*i;

      Task_token* next_blocker;
      if (i != lib_group->end() - 1)
        {
          next_blocker = new Task_token(true);
          next_blocker->add_blocker();
        }
      else
        next_blocker = this->next_blocker_;

      workqueue->queue_soon(new Read_symbols(this->input_objects_,
					     this->symtab_, this->layout_,
					     this->dirpath_, this->dirindex_,
					     this->mapfile_, arg, NULL,
					     NULL, this_blocker, next_blocker));
      this_blocker = next_blocker;
    }

  return true;
}

// Handle a lib group. We set Read_symbols Tasks as usual, but have them
// just record the symbol data instead of adding the objects.  We also start
// a Add_lib_group_symbols Task which runs after we've read all the symbols.
// In that task we process the members in a loop until we are done.

bool
Read_symbols::do_lib_group(Workqueue* workqueue)
{
  const Input_file_lib* lib_group = this->input_argument_->lib();

  if (lib_group->options().whole_archive())
    return this->do_whole_lib_group(workqueue);

  Lib_group* lib = new Lib_group(lib_group, this);

  Add_lib_group_symbols* add_lib_group_symbols =
    new Add_lib_group_symbols(this->symtab_, this->layout_,
			      this->input_objects_,
			      lib, this->next_blocker_);


  Task_token* next_blocker = new Task_token(true);
  int j = 0;
  for (Input_file_lib::const_iterator i = lib_group->begin();
       i != lib_group->end();
       ++i, ++j)
    {
      const Input_argument* arg = &*i;
      Archive_member* m = lib->get_member(j);

      next_blocker->add_blocker();

      // Since this Read_symbols will not create an Add_symbols,
      // just pass NULL as this_blocker.
      workqueue->queue_soon(new Read_symbols(this->input_objects_,
					     this->symtab_, this->layout_,
					     this->dirpath_, this->dirindex_,
					     this->mapfile_, arg, NULL,
					     m, NULL, next_blocker));
    }

  add_lib_group_symbols->set_blocker(next_blocker, this->this_blocker_);
  workqueue->queue_soon(add_lib_group_symbols);

  return true;
}

// Open the file and read the symbols.  Return true if a new task was
// queued, false if that could not happen due to some error.

bool
Read_symbols::do_read_symbols(Workqueue* workqueue)
{
  if (this->input_argument_->is_group())
    {
      gold_assert(this->input_group_ == NULL);
      this->do_group(workqueue);
      return true;
    }

  if (this->input_argument_->is_lib())
    return this->do_lib_group(workqueue);

  Input_file* input_file = new Input_file(&this->input_argument_->file());
  if (!input_file->open(*this->dirpath_, this, &this->dirindex_))
    return false;

  // Read enough of the file to pick up the entire ELF header.

  off_t filesize = input_file->file().filesize();

  if (filesize == 0)
    {
      gold_error(_("%s: file is empty"),
		 input_file->file().filename().c_str());
      return false;
    }

  const unsigned char* ehdr;
  int read_size;
  bool is_elf = is_elf_object(input_file, 0, &ehdr, &read_size);

  if (read_size >= Archive::sarmag)
    {
      bool is_thin_archive
          = memcmp(ehdr, Archive::armagt, Archive::sarmag) == 0;
      if (is_thin_archive
          || memcmp(ehdr, Archive::armag, Archive::sarmag) == 0)
	{
	  // This is an archive.
	  Archive* arch = new Archive(this->input_argument_->file().name(),
				      input_file, is_thin_archive,
				      this->dirpath_, this);
	  arch->setup();

	  // Unlock the archive so it can be used in the next task.
	  arch->unlock(this);

	  workqueue->queue_next(new Add_archive_symbols(this->symtab_,
							this->layout_,
							this->input_objects_,
							this->dirpath_,
							this->dirindex_,
							this->mapfile_,
							this->input_argument_,
							arch,
							this->input_group_,
							this->this_blocker_,
							this->next_blocker_));
	  return true;
	}
    }

  Object* elf_obj = NULL;
  bool unconfigured;
  bool* punconfigured = NULL;
  if (is_elf)
    {
      // This is an ELF object.

      unconfigured = false;
      punconfigured = (input_file->will_search_for()
		       ? &unconfigured
		       : NULL);
      elf_obj = make_elf_object(input_file->filename(),
				input_file, 0, ehdr, read_size,
				punconfigured);
    }

  if (parameters->options().has_plugins())
    {
      Pluginobj* obj = parameters->options().plugins()->claim_file(input_file,
                                                                   0, filesize,
								   elf_obj);
      if (obj != NULL)
        {
	  // Delete the elf_obj, this file has been claimed.
	  if (elf_obj != NULL)
	    delete elf_obj;

          // The input file was claimed by a plugin, and its symbols
          // have been provided by the plugin.

          // We are done with the file at this point, so unlock it.
          obj->unlock(this);

          if (this->member_ != NULL)
	    {
	      this->member_->sd_ = NULL;
	      this->member_->obj_ = obj;
	      return true;
	    }

          workqueue->queue_next(new Add_symbols(this->input_objects_,
                                                this->symtab_,
                                                this->layout_,
						this->dirpath_,
						this->dirindex_,
						this->mapfile_,
						this->input_argument_,
                                                obj,
                                                NULL,
						NULL,
                                                this->this_blocker_,
                                                this->next_blocker_));
          return true;
        }
    }

  if (is_elf)
    {
      // This is an ELF object.

      if (elf_obj == NULL)
	{
	  if (unconfigured)
	    {
	      Read_symbols::incompatible_warning(this->input_argument_,
						 input_file);
	      input_file->file().release();
	      input_file->file().unlock(this);
	      delete input_file;
	      ++this->dirindex_;
	      return this->do_read_symbols(workqueue);
	    }
	  return false;
	}

      Read_symbols_data* sd = new Read_symbols_data;
      elf_obj->read_symbols(sd);

      // Opening the file locked it, so now we need to unlock it.  We
      // need to unlock it before queuing the Add_symbols task,
      // because the workqueue doesn't know about our lock on the
      // file.  If we queue the Add_symbols task first, it will be
      // stuck on the end of the file lock, but since the workqueue
      // doesn't know about that lock, it will never release the
      // Add_symbols task.

      input_file->file().unlock(this);

      if (this->member_ != NULL)
        {
          this->member_->sd_ = sd;
          this->member_->obj_ = elf_obj;
          this->member_->arg_serial_ =
              this->input_argument_->file().arg_serial();
          return true;
        }

      // We use queue_next because everything is cached for this
      // task to run right away if possible.

      workqueue->queue_next(new Add_symbols(this->input_objects_,
					    this->symtab_, this->layout_,
					    this->dirpath_,
					    this->dirindex_,
					    this->mapfile_,
					    this->input_argument_,
					    elf_obj,
					    NULL,
					    sd,
					    this->this_blocker_,
					    this->next_blocker_));

      return true;
    }

  // Queue up a task to try to parse this file as a script.  We use a
  // separate task so that the script will be read in order with other
  // objects named on the command line.  Also so that we don't try to
  // read multiple scripts simultaneously, which could lead to
  // unpredictable changes to the General_options structure.

  workqueue->queue_soon(new Read_script(this->symtab_,
					this->layout_,
					this->dirpath_,
					this->dirindex_,
					this->input_objects_,
					this->mapfile_,
					this->input_group_,
					this->input_argument_,
					input_file,
					this->this_blocker_,
					this->next_blocker_));
  return true;
}

// Handle a group.  We need to walk through the arguments over and
// over until we don't see any new undefined symbols.  We do this by
// setting off Read_symbols Tasks as usual, but recording the archive
// entries instead of deleting them.  We also start a Finish_group
// Task which runs after we've read all the symbols.  In that task we
// process the archives in a loop until we are done.

void
Read_symbols::do_group(Workqueue* workqueue)
{
  Input_group* input_group = new Input_group();

  const Input_file_group* group = this->input_argument_->group();
  Task_token* this_blocker = this->this_blocker_;

  Finish_group* finish_group = new Finish_group(this->input_objects_,
						this->symtab_,
						this->layout_,
						this->mapfile_,
						input_group,
						this->next_blocker_);

  Task_token* next_blocker = new Task_token(true);
  next_blocker->add_blocker();
  workqueue->queue_soon(new Start_group(this->symtab_, finish_group,
					this_blocker, next_blocker));
  this_blocker = next_blocker;

  for (Input_file_group::const_iterator p = group->begin();
       p != group->end();
       ++p)
    {
      const Input_argument* arg = &*p;
      gold_assert(arg->is_file());

      next_blocker = new Task_token(true);
      next_blocker->add_blocker();
      workqueue->queue_soon(new Read_symbols(this->input_objects_,
					     this->symtab_, this->layout_,
					     this->dirpath_, this->dirindex_,
					     this->mapfile_, arg, input_group,
					     NULL, this_blocker, next_blocker));
      this_blocker = next_blocker;
    }

  finish_group->set_blocker(this_blocker);

  workqueue->queue_soon(finish_group);
}

// Return a debugging name for a Read_symbols task.

std::string
Read_symbols::get_name() const
{
  if (this->input_argument_->is_group())
    {
      std::string ret("Read_symbols group (");
      bool add_space = false;
      const Input_file_group* group = this->input_argument_->group();
      for (Input_file_group::const_iterator p = group->begin();
           p != group->end();
           ++p)
      {
        if (add_space)
          ret += ' ';
        ret += p->file().name();
        add_space = true;
      }
      return ret + ')';
    }
  else if (this->input_argument_->is_lib())
    {
      std::string ret("Read_symbols lib (");
      bool add_space = false;
      const Input_file_lib* lib = this->input_argument_->lib();
      for (Input_file_lib::const_iterator p = lib->begin();
           p != lib->end();
           ++p)
      {
        if (add_space)
          ret += ' ';
        ret += p->file().name();
        add_space = true;
      }
      return ret + ')';
    }
  else
    {
      std::string ret("Read_symbols ");
      if (this->input_argument_->file().is_lib())
	ret += "-l";
      else if (this->input_argument_->file().is_searched_file())
	ret += "-l:";
      ret += this->input_argument_->file().name();
      return ret;
    }
}

// Class Add_symbols.

Add_symbols::~Add_symbols()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the next
  // input file.
}

// We are blocked by this_blocker_.  We block next_blocker_.  We also
// lock the file.

Task_token*
Add_symbols::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  if (this->object_->is_locked())
    return this->object_->token();
  return NULL;
}

void
Add_symbols::locks(Task_locker* tl)
{
  tl->add(this, this->next_blocker_);
  Task_token* token = this->object_->token();
  if (token != NULL)
    tl->add(this, token);
}

// Add the symbols in the object to the symbol table.

void
Add_symbols::run(Workqueue*)
{
  Pluginobj* pluginobj = this->object_->pluginobj();
  if (pluginobj != NULL)
    {
      this->object_->add_symbols(this->symtab_, this->sd_, this->layout_);
      return;
    }

  if (!this->input_objects_->add_object(this->object_))
    {
      this->object_->discard_decompressed_sections();
      gold_assert(this->sd_ != NULL);
      delete this->sd_;
      this->sd_ = NULL;
      this->object_->release();
      delete this->object_;
    }
  else
    {
      Incremental_inputs* incremental_inputs =
          this->layout_->incremental_inputs();
      if (incremental_inputs != NULL)
	{
          if (this->library_ != NULL && !this->library_->is_reported())
            {
              Incremental_binary* ibase = this->layout_->incremental_base();
              gold_assert(ibase != NULL);
              unsigned int lib_serial = this->library_->arg_serial();
              unsigned int lib_index = this->library_->input_file_index();
	      Script_info* lib_script_info = ibase->get_script_info(lib_index);
	      incremental_inputs->report_archive_begin(this->library_,
						       lib_serial,
						       lib_script_info);
	    }
	  unsigned int arg_serial = this->input_argument_->file().arg_serial();
	  Script_info* script_info = this->input_argument_->script_info();
	  incremental_inputs->report_object(this->object_, arg_serial,
					    this->library_, script_info);
	}
      this->object_->layout(this->symtab_, this->layout_, this->sd_);
      this->object_->add_symbols(this->symtab_, this->sd_, this->layout_);
      this->object_->discard_decompressed_sections();
      delete this->sd_;
      this->sd_ = NULL;
      this->object_->release();
    }
}

// Class Read_member.

Read_member::~Read_member()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the next
  // input file.
}

// Return whether a Read_member task is runnable.

Task_token*
Read_member::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

void
Read_member::locks(Task_locker* tl)
{
  tl->add(this, this->next_blocker_);
}

// Run a Read_member task.

void
Read_member::run(Workqueue*)
{
  // This task doesn't need to do anything for now.  The Read_symbols task
  // that is queued for the archive library will cause the archive to be
  // processed from scratch.
}

// Class Check_script.

Check_script::~Check_script()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the next
  // input file.
}

// Return whether a Check_script task is runnable.

Task_token*
Check_script::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

void
Check_script::locks(Task_locker* tl)
{
  tl->add(this, this->next_blocker_);
}

// Run a Check_script task.

void
Check_script::run(Workqueue*)
{
  Incremental_inputs* incremental_inputs = this->layout_->incremental_inputs();
  gold_assert(incremental_inputs != NULL);
  unsigned int arg_serial = this->input_reader_->arg_serial();
  Script_info* script_info =
      this->ibase_->get_script_info(this->input_file_index_);
  Timespec mtime = this->input_reader_->get_mtime();
  incremental_inputs->report_script(script_info, arg_serial, mtime);
}

// Class Check_library.

Check_library::~Check_library()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the next
  // input file.
}

// Return whether a Check_library task is runnable.

Task_token*
Check_library::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

void
Check_library::locks(Task_locker* tl)
{
  tl->add(this, this->next_blocker_);
}

// Run a Check_library task.

void
Check_library::run(Workqueue*)
{
  Incremental_inputs* incremental_inputs = this->layout_->incremental_inputs();
  gold_assert(incremental_inputs != NULL);
  Incremental_library* lib = this->ibase_->get_library(this->input_file_index_);
  gold_assert(lib != NULL);
  lib->copy_unused_symbols();
  // FIXME: Check that unused symbols remain unused.
  if (!lib->is_reported())
    {
      unsigned int lib_serial = lib->arg_serial();
      unsigned int lib_index = lib->input_file_index();
      Script_info* script_info = this->ibase_->get_script_info(lib_index);
      incremental_inputs->report_archive_begin(lib, lib_serial, script_info);
    }
  incremental_inputs->report_archive_end(lib);
}

// Class Input_group.

// When we delete an Input_group we can delete the archive
// information.

Input_group::~Input_group()
{
  for (Input_group::const_iterator p = this->begin();
       p != this->end();
       ++p)
    delete *p;
}

// Class Start_group.

Start_group::~Start_group()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the first
  // file in the group.
}

// We need to wait for THIS_BLOCKER_ and unblock NEXT_BLOCKER_.

Task_token*
Start_group::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

void
Start_group::locks(Task_locker* tl)
{
  tl->add(this, this->next_blocker_);
}

// Store the number of undefined symbols we see now.

void
Start_group::run(Workqueue*)
{
  this->finish_group_->set_saw_undefined(this->symtab_->saw_undefined());
}

// Class Finish_group.

Finish_group::~Finish_group()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the next
  // input file following the group.
}

// We need to wait for THIS_BLOCKER_ and unblock NEXT_BLOCKER_.

Task_token*
Finish_group::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

void
Finish_group::locks(Task_locker* tl)
{
  tl->add(this, this->next_blocker_);
}

// Loop over the archives until there are no new undefined symbols.

void
Finish_group::run(Workqueue*)
{
  size_t saw_undefined = this->saw_undefined_;
  while (saw_undefined != this->symtab_->saw_undefined())
    {
      saw_undefined = this->symtab_->saw_undefined();

      for (Input_group::const_iterator p = this->input_group_->begin();
	   p != this->input_group_->end();
	   ++p)
	{
	  Task_lock_obj<Archive> tl(this, *p);

	  (*p)->add_symbols(this->symtab_, this->layout_,
			    this->input_objects_, this->mapfile_);
	}
    }

  // Now that we're done with the archives, record the incremental
  // layout information.
  for (Input_group::const_iterator p = this->input_group_->begin();
       p != this->input_group_->end();
       ++p)
    {
      // For an incremental link, finish recording the layout information.
      Incremental_inputs* incremental_inputs =
          this->layout_->incremental_inputs();
      if (incremental_inputs != NULL)
	incremental_inputs->report_archive_end(*p);
    }

  if (parameters->options().has_plugins())
    parameters->options().plugins()->save_input_group(this->input_group_);
  else
    delete this->input_group_;
}

// Class Read_script

Read_script::~Read_script()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the next
  // input file.
}

// We are blocked by this_blocker_.

Task_token*
Read_script::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

// We don't unlock next_blocker_ here.  If the script names any input
// files, then the last file will be responsible for unlocking it.

void
Read_script::locks(Task_locker*)
{
}

// Read the script, if it is a script.

void
Read_script::run(Workqueue* workqueue)
{
  bool used_next_blocker;
  if (!read_input_script(workqueue, this->symtab_, this->layout_,
			 this->dirpath_, this->dirindex_, this->input_objects_,
			 this->mapfile_, this->input_group_,
			 this->input_argument_, this->input_file_,
			 this->next_blocker_, &used_next_blocker))
    {
      // Here we have to handle any other input file types we need.
      gold_error(_("%s: not an object or archive"),
		 this->input_file_->file().filename().c_str());
    }

  if (!used_next_blocker)
    {
      // Queue up a task to unlock next_blocker.  We can't just unlock
      // it here, as we don't hold the workqueue lock.
      workqueue->queue_soon(new Unblock_token(NULL, this->next_blocker_));
    }
}

// Return a debugging name for a Read_script task.

std::string
Read_script::get_name() const
{
  std::string ret("Read_script ");
  if (this->input_argument_->file().is_lib())
    ret += "-l";
  else if (this->input_argument_->file().is_searched_file())
    ret += "-l:";
  ret += this->input_argument_->file().name();
  return ret;
}

} // End namespace gold.
