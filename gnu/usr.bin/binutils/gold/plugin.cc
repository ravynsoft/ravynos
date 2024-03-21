// plugin.cc -- plugin manager for gold      -*- C++ -*-

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
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

#include <cerrno>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include "libiberty.h"

#ifdef ENABLE_PLUGINS
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#elif defined (HAVE_WINDOWS_H)
#include <windows.h>
#else
#error Unknown how to handle dynamic-load-libraries.
#endif

#if !defined (HAVE_DLFCN_H) && defined (HAVE_WINDOWS_H)

#define RTLD_NOW 0      /* Dummy value.  */
static void *
dlopen(const char *file, int mode ATTRIBUTE_UNUSED)
{
  return LoadLibrary(file);
}

static void *
dlsym(void *handle, const char *name)
{
  return reinterpret_cast<void *>(
     GetProcAddress(static_cast<HMODULE>(handle),name));
}

static const char *
dlerror(void)
{
  return "unable to load dll";
}

#endif /* !defined (HAVE_DLFCN_H) && defined (HAVE_WINDOWS_H)  */
#endif /* ENABLE_PLUGINS */

#include "parameters.h"
#include "debug.h"
#include "errors.h"
#include "fileread.h"
#include "layout.h"
#include "options.h"
#include "plugin.h"
#include "target.h"
#include "readsyms.h"
#include "symtab.h"
#include "descriptors.h"
#include "elfcpp.h"

namespace gold
{

#ifdef ENABLE_PLUGINS

// The linker's exported interfaces.

extern "C"
{

static enum ld_plugin_status
register_claim_file(ld_plugin_claim_file_handler handler);

static enum ld_plugin_status
register_all_symbols_read(ld_plugin_all_symbols_read_handler handler);

static enum ld_plugin_status
register_cleanup(ld_plugin_cleanup_handler handler);

static enum ld_plugin_status
add_symbols(void *handle, int nsyms, const struct ld_plugin_symbol *syms);

static enum ld_plugin_status
get_input_file(const void *handle, struct ld_plugin_input_file *file);

static enum ld_plugin_status
get_view(const void *handle, const void **viewp);

static enum ld_plugin_status
release_input_file(const void *handle);

static enum ld_plugin_status
get_symbols(const void *handle, int nsyms, struct ld_plugin_symbol *syms);

static enum ld_plugin_status
get_symbols_v2(const void *handle, int nsyms, struct ld_plugin_symbol *syms);

static enum ld_plugin_status
get_symbols_v3(const void *handle, int nsyms, struct ld_plugin_symbol *syms);

static enum ld_plugin_status
add_input_file(const char *pathname);

static enum ld_plugin_status
add_input_library(const char *pathname);

static enum ld_plugin_status
set_extra_library_path(const char *path);

static enum ld_plugin_status
message(int level, const char *format, ...);

static enum ld_plugin_status
get_input_section_count(const void* handle, unsigned int* count);

static enum ld_plugin_status
get_input_section_type(const struct ld_plugin_section section,
                       unsigned int* type);

static enum ld_plugin_status
get_input_section_name(const struct ld_plugin_section section,
                       char** section_name_ptr);

static enum ld_plugin_status
get_input_section_contents(const struct ld_plugin_section section,
                           const unsigned char** section_contents,
		           size_t* len);

static enum ld_plugin_status
update_section_order(const struct ld_plugin_section *section_list,
		     unsigned int num_sections);

static enum ld_plugin_status
allow_section_ordering();

static enum ld_plugin_status
allow_unique_segment_for_sections();

static enum ld_plugin_status
unique_segment_for_sections(const char* segment_name,
			    uint64_t flags,
			    uint64_t align,
			    const struct ld_plugin_section *section_list,
			    unsigned int num_sections);

static enum ld_plugin_status
get_input_section_alignment(const struct ld_plugin_section section,
                            unsigned int* addralign);

static enum ld_plugin_status
get_input_section_size(const struct ld_plugin_section section,
                       uint64_t* secsize);

static enum ld_plugin_status
register_new_input(ld_plugin_new_input_handler handler);

static enum ld_plugin_status
get_wrap_symbols(uint64_t *num_symbols, const char ***wrap_symbol_list);

};

#endif // ENABLE_PLUGINS

static Pluginobj* make_sized_plugin_object(const std::string& filename,
					   Input_file* input_file,
                                           off_t offset, off_t filesize);

// Plugin methods.

// Load one plugin library.

void
Plugin::load()
{
#ifdef ENABLE_PLUGINS
  // Load the plugin library.
  // FIXME: Look for the library in standard locations.
  this->handle_ = dlopen(this->filename_.c_str(), RTLD_NOW);
  if (this->handle_ == NULL)
    {
      gold_error(_("%s: could not load plugin library: %s"),
                 this->filename_.c_str(), dlerror());
      return;
    }

  // Find the plugin's onload entry point.
  void* ptr = dlsym(this->handle_, "onload");
  if (ptr == NULL)
    {
      gold_error(_("%s: could not find onload entry point"),
                 this->filename_.c_str());
      return;
    }
  ld_plugin_onload onload;
  gold_assert(sizeof(onload) == sizeof(ptr));
  memcpy(&onload, &ptr, sizeof(ptr));

  // Get the linker's version number.
  const char* ver = get_version_string();
  int major = 0;
  int minor = 0;
  sscanf(ver, "%d.%d", &major, &minor);

  // Allocate and populate a transfer vector.
  const int tv_fixed_size = 31;

  int tv_size = this->args_.size() + tv_fixed_size;
  ld_plugin_tv* tv = new ld_plugin_tv[tv_size];

  // Put LDPT_MESSAGE at the front of the list so the plugin can use it
  // while processing subsequent entries.
  int i = 0;
  tv[i].tv_tag = LDPT_MESSAGE;
  tv[i].tv_u.tv_message = message;

  ++i;
  tv[i].tv_tag = LDPT_API_VERSION;
  tv[i].tv_u.tv_val = LD_PLUGIN_API_VERSION;

  ++i;
  tv[i].tv_tag = LDPT_GOLD_VERSION;
  tv[i].tv_u.tv_val = major * 100 + minor;

  ++i;
  tv[i].tv_tag = LDPT_LINKER_OUTPUT;
  if (parameters->options().relocatable())
    tv[i].tv_u.tv_val = LDPO_REL;
  else if (parameters->options().shared())
    tv[i].tv_u.tv_val = LDPO_DYN;
  else if (parameters->options().pie())
    tv[i].tv_u.tv_val = LDPO_PIE;
  else
    tv[i].tv_u.tv_val = LDPO_EXEC;

  ++i;
  tv[i].tv_tag = LDPT_OUTPUT_NAME;
  tv[i].tv_u.tv_string = parameters->options().output();

  for (unsigned int j = 0; j < this->args_.size(); ++j)
    {
      ++i;
      tv[i].tv_tag = LDPT_OPTION;
      tv[i].tv_u.tv_string = this->args_[j].c_str();
    }

  ++i;
  tv[i].tv_tag = LDPT_REGISTER_CLAIM_FILE_HOOK;
  tv[i].tv_u.tv_register_claim_file = register_claim_file;

  ++i;
  tv[i].tv_tag = LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK;
  tv[i].tv_u.tv_register_all_symbols_read = register_all_symbols_read;

  ++i;
  tv[i].tv_tag = LDPT_REGISTER_CLEANUP_HOOK;
  tv[i].tv_u.tv_register_cleanup = register_cleanup;

  ++i;
  tv[i].tv_tag = LDPT_ADD_SYMBOLS;
  tv[i].tv_u.tv_add_symbols = add_symbols;

  ++i;
  tv[i].tv_tag = LDPT_GET_INPUT_FILE;
  tv[i].tv_u.tv_get_input_file = get_input_file;

  ++i;
  tv[i].tv_tag = LDPT_GET_VIEW;
  tv[i].tv_u.tv_get_view = get_view;

  ++i;
  tv[i].tv_tag = LDPT_RELEASE_INPUT_FILE;
  tv[i].tv_u.tv_release_input_file = release_input_file;

  ++i;
  tv[i].tv_tag = LDPT_GET_SYMBOLS;
  tv[i].tv_u.tv_get_symbols = get_symbols;

  ++i;
  tv[i].tv_tag = LDPT_GET_SYMBOLS_V2;
  tv[i].tv_u.tv_get_symbols = get_symbols_v2;

  ++i;
  tv[i].tv_tag = LDPT_GET_SYMBOLS_V3;
  tv[i].tv_u.tv_get_symbols = get_symbols_v3;

  ++i;
  tv[i].tv_tag = LDPT_ADD_INPUT_FILE;
  tv[i].tv_u.tv_add_input_file = add_input_file;

  ++i;
  tv[i].tv_tag = LDPT_ADD_INPUT_LIBRARY;
  tv[i].tv_u.tv_add_input_library = add_input_library;

  ++i;
  tv[i].tv_tag = LDPT_SET_EXTRA_LIBRARY_PATH;
  tv[i].tv_u.tv_set_extra_library_path = set_extra_library_path;

  ++i;
  tv[i].tv_tag = LDPT_GET_INPUT_SECTION_COUNT;
  tv[i].tv_u.tv_get_input_section_count = get_input_section_count;

  ++i;
  tv[i].tv_tag = LDPT_GET_INPUT_SECTION_TYPE;
  tv[i].tv_u.tv_get_input_section_type = get_input_section_type;

  ++i;
  tv[i].tv_tag = LDPT_GET_INPUT_SECTION_NAME;
  tv[i].tv_u.tv_get_input_section_name = get_input_section_name;

  ++i;
  tv[i].tv_tag = LDPT_GET_INPUT_SECTION_CONTENTS;
  tv[i].tv_u.tv_get_input_section_contents = get_input_section_contents;

  ++i;
  tv[i].tv_tag = LDPT_UPDATE_SECTION_ORDER;
  tv[i].tv_u.tv_update_section_order = update_section_order;

  ++i;
  tv[i].tv_tag = LDPT_ALLOW_SECTION_ORDERING;
  tv[i].tv_u.tv_allow_section_ordering = allow_section_ordering;

  ++i;
  tv[i].tv_tag = LDPT_ALLOW_UNIQUE_SEGMENT_FOR_SECTIONS;
  tv[i].tv_u.tv_allow_unique_segment_for_sections
    = allow_unique_segment_for_sections;

  ++i;
  tv[i].tv_tag = LDPT_UNIQUE_SEGMENT_FOR_SECTIONS;
  tv[i].tv_u.tv_unique_segment_for_sections = unique_segment_for_sections;

  ++i;
  tv[i].tv_tag = LDPT_GET_INPUT_SECTION_ALIGNMENT;
  tv[i].tv_u.tv_get_input_section_alignment = get_input_section_alignment;

  ++i;
  tv[i].tv_tag = LDPT_GET_INPUT_SECTION_SIZE;
  tv[i].tv_u.tv_get_input_section_size = get_input_section_size;

  ++i;
  tv[i].tv_tag = LDPT_REGISTER_NEW_INPUT_HOOK;
  tv[i].tv_u.tv_register_new_input = register_new_input;

  ++i;
  tv[i].tv_tag = LDPT_GET_WRAP_SYMBOLS;
  tv[i].tv_u.tv_get_wrap_symbols = get_wrap_symbols;

  ++i;
  tv[i].tv_tag = LDPT_NULL;
  tv[i].tv_u.tv_val = 0;

  gold_assert(i == tv_size - 1);

  // Call the onload entry point.
  (*onload)(tv);

  delete[] tv;
#endif // ENABLE_PLUGINS
}

// Call the plugin claim-file handler.

inline bool
Plugin::claim_file(struct ld_plugin_input_file* plugin_input_file)
{
  int claimed = 0;

  if (this->claim_file_handler_ != NULL)
    {
      (*this->claim_file_handler_)(plugin_input_file, &claimed);
      if (claimed)
        return true;
    }
  return false;
}

// Call the all-symbols-read handler.

inline void
Plugin::all_symbols_read()
{
  if (this->all_symbols_read_handler_ != NULL)
    (*this->all_symbols_read_handler_)();
}

// Call the new_input handler.

inline void
Plugin::new_input(struct ld_plugin_input_file* plugin_input_file)
{
  if (this->new_input_handler_ != NULL)
    (*this->new_input_handler_)(plugin_input_file);
}

// Call the cleanup handler.

inline void
Plugin::cleanup()
{
  if (this->cleanup_handler_ != NULL && !this->cleanup_done_)
    {
      // Set this flag before calling to prevent a recursive plunge
      // in the event that a plugin's cleanup handler issues a
      // fatal error.
      this->cleanup_done_ = true;
      (*this->cleanup_handler_)();
    }
}

// This task is used to rescan archives as needed.

class Plugin_rescan : public Task
{
 public:
  Plugin_rescan(Task_token* this_blocker, Task_token* next_blocker)
    : this_blocker_(this_blocker), next_blocker_(next_blocker)
  { }

  ~Plugin_rescan()
  {
    delete this->this_blocker_;
  }

  Task_token*
  is_runnable()
  {
    if (this->this_blocker_->is_blocked())
      return this->this_blocker_;
    return NULL;
  }

  void
  locks(Task_locker* tl)
  { tl->add(this, this->next_blocker_); }

  void
  run(Workqueue*)
  { parameters->options().plugins()->rescan(this); }

  std::string
  get_name() const
  { return "Plugin_rescan"; }

 private:
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

// Plugin_recorder logs plugin actions and saves intermediate files
// for later replay.

class Plugin_recorder
{
 public:
  Plugin_recorder() : file_count_(0), tempdir_(NULL), logfile_(NULL)
  { }

  bool
  init();

  void
  claimed_file(const std::string& obj_name, off_t offset, off_t filesize,
	       const std::string& plugin_name);

  void
  unclaimed_file(const std::string& obj_name, off_t offset, off_t filesize);

  void
  replacement_file(const char* name, bool is_lib);

  void
  record_symbols(const Object* obj, int nsyms,
		 const struct ld_plugin_symbol* syms);

  void
  finish()
  { ::fclose(this->logfile_); }

 private:
  unsigned int file_count_;
  const char* tempdir_;
  FILE* logfile_;
};

bool
Plugin_recorder::init()
{
  // Create a temporary directory where we can stash the log and
  // copies of replacement files.
  char dir_template[] = "gold-recording-XXXXXX";
#ifdef HAVE_MKDTEMP
  if (mkdtemp(dir_template) == NULL)
    return false;
#else
  if (mktemp(dir_template) == NULL)
    return false;
#if defined (_WIN32) && !defined (__CYGWIN32__)
  if (mkdir(dir_template) != 0)
    return false;
#else
  if (mkdir(dir_template, 0700) != 0)
    return false;
#endif
#endif

  size_t len = strlen(dir_template) + 1;
  char* tempdir = new char[len];
  memcpy(tempdir, dir_template, len);

  // Create the log file.
  std::string logname(tempdir);
  logname.append("/log");
  FILE* logfile = ::fopen(logname.c_str(), "w");
  if (logfile == NULL)
    return false;

  this->tempdir_ = tempdir;
  this->logfile_ = logfile;

  gold_info(_("%s: recording to %s"), program_name, this->tempdir_);

  return true;
}

void
Plugin_recorder::claimed_file(const std::string& obj_name,
			      off_t offset,
			      off_t filesize,
			      const std::string& plugin_name)
{
  fprintf(this->logfile_, "PLUGIN: %s\n", plugin_name.c_str());
  fprintf(this->logfile_, "CLAIMED: %s", obj_name.c_str());
  if (offset > 0)
    fprintf(this->logfile_, " @%ld", static_cast<long>(offset));
  fprintf(this->logfile_, " %ld\n", static_cast<long>(filesize));
}

void
Plugin_recorder::unclaimed_file(const std::string& obj_name,
				off_t offset,
				off_t filesize)
{
  fprintf(this->logfile_, "UNCLAIMED: %s", obj_name.c_str());
  if (offset > 0)
    fprintf(this->logfile_, " @%ld", static_cast<long>(offset));
  fprintf(this->logfile_, " %ld\n", static_cast<long>(filesize));
}

// Make a hard link to INNAME from OUTNAME, if possible.
// If not, copy the file.

static bool
link_or_copy_file(const char* inname, const char* outname)
{
  static char buf[4096];

#ifdef HAVE_LINK
  if (::link(inname, outname) == 0)
    return true;
#endif

  int in = ::open(inname, O_RDONLY);
  if (in < 0)
    {
      gold_warning(_("%s: can't open (%s)"), inname, strerror(errno));
      return false;
    }
  int out = ::open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0600);
  if (out < 0)
    {
      gold_warning(_("%s: can't create (%s)"), outname, strerror(errno));
      ::close(in);
      return false;
    }
  ssize_t len;
  while ((len = ::read(in, buf, sizeof(buf))) > 0)
    {
      if (::write(out, buf, len) != len)
	{
	  gold_warning(_("%s: write error while making copy of file (%s)"),
		       inname, strerror(errno));
	  break;
        }
    }
  ::close(in);
  ::close(out);
  return true;
}

void
Plugin_recorder::replacement_file(const char* name, bool is_lib)
{
  fprintf(this->logfile_, "REPLACEMENT: %s", name);
  if (is_lib)
    fprintf(this->logfile_, "(lib)");
  else
    {
      char counter[10];
      const char* basename = lbasename(name);
      snprintf(counter, sizeof(counter), "%05d", this->file_count_);
      ++this->file_count_;
      std::string outname(this->tempdir_);
      outname.append("/");
      outname.append(counter);
      outname.append("-");
      outname.append(basename);
      if (link_or_copy_file(name, outname.c_str()))
        fprintf(this->logfile_, " -> %s", outname.c_str());
    }
  fprintf(this->logfile_, "\n");
}

void
Plugin_recorder::record_symbols(const Object* obj, int nsyms,
				const struct ld_plugin_symbol* syms)
{
  fprintf(this->logfile_, "SYMBOLS: %d %s\n", nsyms, obj->name().c_str());
  for (int i = 0; i < nsyms; ++i)
    {
      const struct ld_plugin_symbol* isym = &syms[i];

      const char* def;
      switch (isym->def)
        {
        case LDPK_DEF:
          def = "D";
          break;
        case LDPK_WEAKDEF:
          def = "WD";
          break;
        case LDPK_UNDEF:
          def = "U";
          break;
        case LDPK_WEAKUNDEF:
          def = "WU";
          break;
        case LDPK_COMMON:
          def = "C";
          break;
        default:
          def = "?";
          break;
        }

      char vis;
      switch (isym->visibility)
        {
        case LDPV_PROTECTED:
          vis = 'P';
          break;
        case LDPV_INTERNAL:
          vis = 'I';
          break;
        case LDPV_HIDDEN:
          vis = 'H';
          break;
        case LDPV_DEFAULT:
          vis = 'D';
          break;
        default:
          vis = '?';
          break;
        }

      fprintf(this->logfile_, " %5d: %-2s %c %s", i, def, vis, isym->name);
      if (isym->version != NULL && isym->version[0] != '\0')
	fprintf(this->logfile_, "@%s", isym->version);
      if (isym->comdat_key != NULL && isym->comdat_key[0] != '\0')
	{
	  if (strcmp(isym->name, isym->comdat_key) == 0)
	    fprintf(this->logfile_, " [comdat]");
	  else
	    fprintf(this->logfile_, " [comdat: %s]", isym->comdat_key);
	}
      fprintf(this->logfile_, "\n");
    }
}

// Plugin_manager methods.

Plugin_manager::~Plugin_manager()
{
  for (Plugin_list::iterator p = this->plugins_.begin();
       p != this->plugins_.end();
       ++p)
    delete *p;
  this->plugins_.clear();
  for (Object_list::iterator obj = this->objects_.begin();
       obj != this->objects_.end();
       ++obj)
    delete *obj;
  this->objects_.clear();
  delete this->lock_;
  delete this->recorder_;
}

// Load all plugin libraries.

void
Plugin_manager::load_plugins(Layout* layout)
{
  this->layout_ = layout;

  if (is_debugging_enabled(DEBUG_PLUGIN))
    {
      this->recorder_ = new Plugin_recorder();
      this->recorder_->init();
    }

  for (this->current_ = this->plugins_.begin();
       this->current_ != this->plugins_.end();
       ++this->current_)
    (*this->current_)->load();
}

// Call the plugin claim-file handlers in turn to see if any claim the file.

Pluginobj*
Plugin_manager::claim_file(Input_file* input_file, off_t offset,
                           off_t filesize, Object* elf_object)
{
  bool lock_initialized = this->initialize_lock_.initialize();

  gold_assert(lock_initialized);
  Hold_lock hl(*this->lock_);

  unsigned int handle = this->objects_.size();
  this->input_file_ = input_file;
  this->plugin_input_file_.name = input_file->filename().c_str();
  this->plugin_input_file_.fd = input_file->file().descriptor();
  this->plugin_input_file_.offset = offset;
  this->plugin_input_file_.filesize = filesize;
  this->plugin_input_file_.handle = reinterpret_cast<void*>(handle);
  if (elf_object != NULL)
    this->objects_.push_back(elf_object);
  this->in_claim_file_handler_ = true;

  for (Plugin_list::iterator p = this->plugins_.begin();
       p != this->plugins_.end();
       ++p)
    {
      // If we aren't yet in replacement phase, allow plugins to claim input
      // files, otherwise notify the plugin of the new input file, if needed.
      if (!this->in_replacement_phase_)
	{
	  if ((*p)->claim_file(&this->plugin_input_file_))
	    {
	      this->any_claimed_ = true;
              this->in_claim_file_handler_ = false;

	      if (this->recorder_ != NULL)
		{
		  const std::string& objname = (elf_object == NULL
						? input_file->filename()
						: elf_object->name());
		  this->recorder_->claimed_file(objname,
						offset, filesize,
						(*p)->filename());
		}

              if (this->objects_.size() > handle
                  && this->objects_[handle]->pluginobj() != NULL)
                return this->objects_[handle]->pluginobj();

              // If the plugin claimed the file but did not call the
              // add_symbols callback, we need to create the Pluginobj now.
              Pluginobj* obj = this->make_plugin_object(handle);
              return obj;
            }
        }
      else
        {
	  (*p)->new_input(&this->plugin_input_file_);
        }
    }

  this->in_claim_file_handler_ = false;

  if (this->recorder_ != NULL)
    this->recorder_->unclaimed_file(input_file->filename(), offset, filesize);

  return NULL;
}

// Save an archive.  This is used so that a plugin can add a file
// which refers to a symbol which was not previously referenced.  In
// that case we want to pretend that the symbol was referenced before,
// and pull in the archive object.

void
Plugin_manager::save_archive(Archive* archive)
{
  if (this->in_replacement_phase_ || !this->any_claimed_)
    delete archive;
  else
    this->rescannable_.push_back(Rescannable(archive));
}

// Save an Input_group.  This is like save_archive.

void
Plugin_manager::save_input_group(Input_group* input_group)
{
  if (this->in_replacement_phase_ || !this->any_claimed_)
    delete input_group;
  else
    this->rescannable_.push_back(Rescannable(input_group));
}

// Call the all-symbols-read handlers.

void
Plugin_manager::all_symbols_read(Workqueue* workqueue, Task* task,
                                 Input_objects* input_objects,
	                         Symbol_table* symtab,
	                         Dirsearch* dirpath, Mapfile* mapfile,
	                         Task_token** last_blocker)
{
  this->in_replacement_phase_ = true;
  this->workqueue_ = workqueue;
  this->task_ = task;
  this->input_objects_ = input_objects;
  this->symtab_ = symtab;
  this->dirpath_ = dirpath;
  this->mapfile_ = mapfile;
  this->this_blocker_ = NULL;

  // Set symbols used in defsym expressions as seen in real ELF.
  Layout *layout = parameters->options().plugins()->layout();
  layout->script_options()->set_defsym_uses_in_real_elf(symtab);
  layout->script_options()->find_defsym_defs(this->defsym_defines_set_);

  for (Plugin_list::iterator p = this->plugins_.begin();
       p != this->plugins_.end();
       ++p)
    (*p)->all_symbols_read();

  if (this->any_added_)
    {
      Task_token* next_blocker = new Task_token(true);
      next_blocker->add_blocker();
      workqueue->queue(new Plugin_rescan(this->this_blocker_, next_blocker));
      this->this_blocker_ = next_blocker;
    }

  *last_blocker = this->this_blocker_;
}

// This is called when we see a new undefined symbol.  If we are in
// the replacement phase, this means that we may need to rescan some
// archives we have previously seen.

void
Plugin_manager::new_undefined_symbol(Symbol* sym)
{
  if (this->in_replacement_phase_)
    this->undefined_symbols_.push_back(sym);
}

// Rescan archives as needed.  This handles the case where a new
// object file added by a plugin has an undefined reference to some
// symbol defined in an archive.

void
Plugin_manager::rescan(Task* task)
{
  size_t rescan_pos = 0;
  size_t rescan_size = this->rescannable_.size();
  while (!this->undefined_symbols_.empty())
    {
      if (rescan_pos >= rescan_size)
	{
	  this->undefined_symbols_.clear();
	  return;
	}

      Undefined_symbol_list undefs;
      undefs.reserve(this->undefined_symbols_.size());
      this->undefined_symbols_.swap(undefs);

      size_t min_rescan_pos = rescan_size;

      for (Undefined_symbol_list::const_iterator p = undefs.begin();
	   p != undefs.end();
	   ++p)
	{
	  if (!(*p)->is_undefined())
	    continue;

	  this->undefined_symbols_.push_back(*p);

	  // Find the first rescan archive which defines this symbol,
	  // starting at the current rescan position.  The rescan position
	  // exists so that given -la -lb -lc we don't look for undefined
	  // symbols in -lb back in -la, but instead get the definition
	  // from -lc.  Don't bother to look past the current minimum
	  // rescan position.
	  for (size_t i = rescan_pos; i < min_rescan_pos; ++i)
	    {
	      if (this->rescannable_defines(i, *p))
		{
		  min_rescan_pos = i;
		  break;
		}
	    }
	}

      if (min_rescan_pos >= rescan_size)
	{
	  // We didn't find any rescannable archives which define any
	  // undefined symbols.
	  return;
	}

      const Rescannable& r(this->rescannable_[min_rescan_pos]);
      if (r.is_archive)
	{
	  Task_lock_obj<Archive> tl(task, r.u.archive);
	  r.u.archive->add_symbols(this->symtab_, this->layout_,
				   this->input_objects_, this->mapfile_);
	}
      else
	{
	  size_t next_saw_undefined = this->symtab_->saw_undefined();
	  size_t saw_undefined;
	  do
	    {
	      saw_undefined = next_saw_undefined;

	      for (Input_group::const_iterator p = r.u.input_group->begin();
		   p != r.u.input_group->end();
		   ++p)
		{
		  Task_lock_obj<Archive> tl(task, *p);

		  (*p)->add_symbols(this->symtab_, this->layout_,
				    this->input_objects_, this->mapfile_);
		}

	      next_saw_undefined = this->symtab_->saw_undefined();
	    }
	  while (saw_undefined != next_saw_undefined);
	}

      for (size_t i = rescan_pos; i < min_rescan_pos + 1; ++i)
	{
	  if (this->rescannable_[i].is_archive)
	    delete this->rescannable_[i].u.archive;
	  else
	    delete this->rescannable_[i].u.input_group;
	}

      rescan_pos = min_rescan_pos + 1;
    }
}

// Return whether the rescannable at index I defines SYM.

bool
Plugin_manager::rescannable_defines(size_t i, Symbol* sym)
{
  const Rescannable& r(this->rescannable_[i]);
  if (r.is_archive)
    return r.u.archive->defines_symbol(sym);
  else
    {
      for (Input_group::const_iterator p = r.u.input_group->begin();
	   p != r.u.input_group->end();
	   ++p)
	{
	  if ((*p)->defines_symbol(sym))
	    return true;
	}
      return false;
    }
}

// Layout deferred objects.

void
Plugin_manager::layout_deferred_objects()
{
  Deferred_layout_list::iterator obj;

  for (obj = this->deferred_layout_objects_.begin();
       obj != this->deferred_layout_objects_.end();
       ++obj)
    {
      // Lock the object so we can read from it.  This is only called
      // single-threaded from queue_middle_tasks, so it is OK to lock.
      // Unfortunately we have no way to pass in a Task token.
      const Task* dummy_task = reinterpret_cast<const Task*>(-1);
      Task_lock_obj<Object> tl(dummy_task, *obj);
      (*obj)->layout_deferred_sections(this->layout_);
    }
}

// Call the cleanup handlers.

void
Plugin_manager::cleanup()
{
  if (this->any_added_)
    {
      // If any input files were added, close all the input files.
      // This is because the plugin may want to remove them, and on
      // Windows you are not allowed to remove an open file.
      close_all_descriptors();
    }

  for (Plugin_list::iterator p = this->plugins_.begin();
       p != this->plugins_.end();
       ++p)
    (*p)->cleanup();
}

// Make a new Pluginobj object.  This is called when the plugin calls
// the add_symbols API.

Pluginobj*
Plugin_manager::make_plugin_object(unsigned int handle)
{
  // Make sure we aren't asked to make an object for the same handle twice.
  if (this->objects_.size() != handle
      && this->objects_[handle]->pluginobj() != NULL)
    return NULL;

  const std::string* filename = &this->input_file_->filename();

  // If the elf object for this file was pushed into the objects_ vector,
  // use its filename, then delete it to make room for the Pluginobj as
  // this file is claimed.
  if (this->objects_.size() != handle)
    {
      filename = &this->objects_.back()->name();
      this->objects_.pop_back();
    }

  Pluginobj* obj = make_sized_plugin_object(*filename,
					    this->input_file_,
                                            this->plugin_input_file_.offset,
                                            this->plugin_input_file_.filesize);



  this->objects_.push_back(obj);
  return obj;
}

// Get the input file information with an open (possibly re-opened)
// file descriptor.

ld_plugin_status
Plugin_manager::get_input_file(unsigned int handle,
                               struct ld_plugin_input_file* file)
{
  Pluginobj* obj = this->object(handle)->pluginobj();
  if (obj == NULL)
    return LDPS_BAD_HANDLE;

  obj->lock(this->task_);
  file->name = obj->filename().c_str();
  file->fd = obj->descriptor();
  file->offset = obj->offset();
  file->filesize = obj->filesize();
  file->handle = reinterpret_cast<void*>(handle);
  return LDPS_OK;
}

// Release the input file.

ld_plugin_status
Plugin_manager::release_input_file(unsigned int handle)
{
  if (this->object(handle) == NULL)
    return LDPS_BAD_HANDLE;

  Pluginobj* obj = this->object(handle)->pluginobj();

  if (obj == NULL)
    return LDPS_BAD_HANDLE;

  obj->unlock(this->task_);
  return LDPS_OK;
}

// Get the elf object corresponding to the handle. Return NULL if we
// found a Pluginobj instead.

Object*
Plugin_manager::get_elf_object(const void* handle)
{
  Object* obj = this->object(
      static_cast<unsigned int>(reinterpret_cast<intptr_t>(handle)));

  // The object should not be a Pluginobj.
  if (obj == NULL
      || obj->pluginobj() != NULL)
    return NULL;

  return obj;
}

ld_plugin_status
Plugin_manager::get_view(unsigned int handle, const void **viewp)
{
  off_t offset;
  size_t filesize;
  Input_file *input_file;
  if (this->in_claim_file_handler_)
    {
      // We are being called from the claim_file hook.
      const struct ld_plugin_input_file &f = this->plugin_input_file_;
      offset = f.offset;
      filesize = f.filesize;
      input_file = this->input_file_;
    }
  else
    {
      // An already claimed file.
      if (this->object(handle) == NULL)
        return LDPS_BAD_HANDLE;
      Pluginobj* obj = this->object(handle)->pluginobj();
      if (obj == NULL)
        return LDPS_BAD_HANDLE;
      offset = obj->offset();
      filesize = obj->filesize();
      input_file = obj->input_file();
    }
  *viewp = (void*) input_file->file().get_view(offset, 0, filesize, false,
                                               false);
  return LDPS_OK;
}

// Add a new library path.

ld_plugin_status
Plugin_manager::set_extra_library_path(const char* path)
{
  this->extra_search_path_ = std::string(path);
  return LDPS_OK;
}

// Add a new input file.

ld_plugin_status
Plugin_manager::add_input_file(const char* pathname, bool is_lib)
{
  Input_file_argument file(pathname,
                           (is_lib
                            ? Input_file_argument::INPUT_FILE_TYPE_LIBRARY
                            : Input_file_argument::INPUT_FILE_TYPE_FILE),
                           (is_lib
                            ? this->extra_search_path_.c_str()
                            : ""),
                           false,
                           this->options_);
  Input_argument* input_argument = new Input_argument(file);
  Task_token* next_blocker = new Task_token(true);
  next_blocker->add_blocker();
  if (parameters->incremental())
    gold_error(_("input files added by plug-ins in --incremental mode not "
		 "supported yet"));

  if (this->recorder_ != NULL)
    this->recorder_->replacement_file(pathname, is_lib);

  this->workqueue_->queue_soon(new Read_symbols(this->input_objects_,
                                                this->symtab_,
                                                this->layout_,
                                                this->dirpath_,
						0,
                                                this->mapfile_,
                                                input_argument,
                                                NULL,
                                                NULL,
                                                this->this_blocker_,
                                                next_blocker));
  this->this_blocker_ = next_blocker;
  this->any_added_ = true;
  return LDPS_OK;
}

// Class Pluginobj.

Pluginobj::Pluginobj(const std::string& name, Input_file* input_file,
                     off_t offset, off_t filesize)
  : Object(name, input_file, false, offset),
    nsyms_(0), syms_(NULL), symbols_(), filesize_(filesize), comdat_map_()
{
}

// Return TRUE if a defined symbol is referenced from outside the
// universe of claimed objects.  Only references from relocatable,
// non-IR (unclaimed) objects count as a reference.  References from
// dynamic objects count only as "visible".

static inline bool
is_referenced_from_outside(Symbol* lsym)
{
  if (lsym->in_real_elf())
    return true;
  if (parameters->options().relocatable())
    return true;
  if (parameters->options().is_undefined(lsym->name()))
    return true;
  return false;
}

// Return TRUE if a defined symbol might be reachable from outside the
// load module.

static inline bool
is_visible_from_outside(Symbol* lsym)
{
  if (lsym->in_dyn())
    return true;
  if (parameters->options().export_dynamic() || parameters->options().shared()
      || parameters->options().in_dynamic_list(lsym->name())
      || parameters->options().is_export_dynamic_symbol(lsym->name()))
    return lsym->is_externally_visible();
  return false;
}

// Get symbol resolution info.

ld_plugin_status
Pluginobj::get_symbol_resolution_info(Symbol_table* symtab,
				      int nsyms,
				      ld_plugin_symbol* syms,
				      int version) const
{
  // For version 1 of this interface, we cannot use
  // LDPR_PREVAILING_DEF_IRONLY_EXP, so we return LDPR_PREVAILING_DEF
  // instead.
  const ld_plugin_symbol_resolution ldpr_prevailing_def_ironly_exp
      = (version > 1
	 ? LDPR_PREVAILING_DEF_IRONLY_EXP
	 : LDPR_PREVAILING_DEF);

  if (nsyms > this->nsyms_)
    return LDPS_NO_SYMS;

  if (static_cast<size_t>(nsyms) > this->symbols_.size())
    {
      // We never decided to include this object. We mark all symbols as
      // preempted.
      gold_assert(this->symbols_.size() == 0);
      for (int i = 0; i < nsyms; i++)
        syms[i].resolution = LDPR_PREEMPTED_REG;
      return version > 2 ? LDPS_NO_SYMS : LDPS_OK;
    }

  Plugin_manager* plugins = parameters->options().plugins();
  for (int i = 0; i < nsyms; i++)
    {
      ld_plugin_symbol* isym = &syms[i];
      Symbol* lsym = this->symbols_[i];
      if (lsym->is_forwarder())
        lsym = symtab->resolve_forwards(lsym);
      ld_plugin_symbol_resolution res = LDPR_UNKNOWN;

      if (plugins->is_defsym_def(lsym->name()))
	{
	  // The symbol is redefined via defsym.
	  res = LDPR_PREEMPTED_REG;
	}
      else if (lsym->is_undefined())
	{
          // The symbol remains undefined.
          res = LDPR_UNDEF;
	}
      else if (isym->def == LDPK_UNDEF
               || isym->def == LDPK_WEAKUNDEF
               || isym->def == LDPK_COMMON)
        {
          // The original symbol was undefined or common.
          if (lsym->source() != Symbol::FROM_OBJECT)
            res = LDPR_RESOLVED_EXEC;
          else if (lsym->object()->pluginobj() == this)
	    {
	      if (is_referenced_from_outside(lsym))
		res = LDPR_PREVAILING_DEF;
	      else if (is_visible_from_outside(lsym))
		res = ldpr_prevailing_def_ironly_exp;
	      else
		res = LDPR_PREVAILING_DEF_IRONLY;
	    }
          else if (lsym->object()->pluginobj() != NULL)
            res = LDPR_RESOLVED_IR;
          else if (lsym->object()->is_dynamic())
            res = LDPR_RESOLVED_DYN;
          else
            res = LDPR_RESOLVED_EXEC;
        }
      else
        {
          // The original symbol was a definition.
          if (lsym->source() != Symbol::FROM_OBJECT)
            res = LDPR_PREEMPTED_REG;
          else if (lsym->object() == static_cast<const Object*>(this))
	    {
	      if (is_referenced_from_outside(lsym))
		res = LDPR_PREVAILING_DEF;
	      else if (is_visible_from_outside(lsym))
		res = ldpr_prevailing_def_ironly_exp;
	      else
		res = LDPR_PREVAILING_DEF_IRONLY;
	    }
          else
            res = (lsym->object()->pluginobj() != NULL
                   ? LDPR_PREEMPTED_IR
                   : LDPR_PREEMPTED_REG);
        }
      isym->resolution = res;
    }
  return LDPS_OK;
}

// Return TRUE if the comdat group with key COMDAT_KEY from this object
// should be kept.

bool
Pluginobj::include_comdat_group(std::string comdat_key, Layout* layout)
{
  std::pair<Comdat_map::iterator, bool> ins =
    this->comdat_map_.insert(std::make_pair(comdat_key, false));

  // If this is the first time we've seen this comdat key, ask the
  // layout object whether it should be included.
  if (ins.second)
    ins.first->second = layout->find_or_add_kept_section(comdat_key,
							 NULL, 0, true,
							 true, NULL);

  return ins.first->second;
}

// Class Sized_pluginobj.

template<int size, bool big_endian>
Sized_pluginobj<size, big_endian>::Sized_pluginobj(
    const std::string& name,
    Input_file* input_file,
    off_t offset,
    off_t filesize)
  : Pluginobj(name, input_file, offset, filesize)
{
}

// Read the symbols.  Not used for plugin objects.

template<int size, bool big_endian>
void
Sized_pluginobj<size, big_endian>::do_read_symbols(Read_symbols_data*)
{
  gold_unreachable();
}

// Lay out the input sections.  Not used for plugin objects.

template<int size, bool big_endian>
void
Sized_pluginobj<size, big_endian>::do_layout(Symbol_table*, Layout*,
                                             Read_symbols_data*)
{
  gold_unreachable();
}

// Add the symbols to the symbol table.

template<int size, bool big_endian>
void
Sized_pluginobj<size, big_endian>::do_add_symbols(Symbol_table* symtab,
                                                  Read_symbols_data*,
                                                  Layout* layout)
{
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  unsigned char symbuf[sym_size];
  elfcpp::Sym_write<size, big_endian> osym(symbuf);

  Plugin_recorder* recorder = parameters->options().plugins()->recorder();
  if (recorder != NULL)
    recorder->record_symbols(this, this->nsyms_, this->syms_);

  this->symbols_.resize(this->nsyms_);

  for (int i = 0; i < this->nsyms_; ++i)
    {
      const struct ld_plugin_symbol* isym = &this->syms_[i];
      const char* name = isym->name;
      const char* ver = isym->version;
      elfcpp::Elf_Half shndx;
      elfcpp::STB bind;
      elfcpp::STV vis;

      if (name != NULL && name[0] == '\0')
        name = NULL;
      if (ver != NULL && ver[0] == '\0')
        ver = NULL;

      switch (isym->def)
        {
        case LDPK_WEAKDEF:
        case LDPK_WEAKUNDEF:
          bind = elfcpp::STB_WEAK;
          break;
        case LDPK_DEF:
        case LDPK_UNDEF:
        case LDPK_COMMON:
        default:
          bind = elfcpp::STB_GLOBAL;
          break;
        }

      switch (isym->def)
        {
        case LDPK_DEF:
        case LDPK_WEAKDEF:
          // We use an arbitrary section number for a defined symbol.
          shndx = 1;
          break;
        case LDPK_COMMON:
          shndx = elfcpp::SHN_COMMON;
          break;
        case LDPK_UNDEF:
        case LDPK_WEAKUNDEF:
        default:
          shndx = elfcpp::SHN_UNDEF;
          break;
        }

      switch (isym->visibility)
        {
        case LDPV_PROTECTED:
          vis = elfcpp::STV_PROTECTED;
          break;
        case LDPV_INTERNAL:
          vis = elfcpp::STV_INTERNAL;
          break;
        case LDPV_HIDDEN:
          vis = elfcpp::STV_HIDDEN;
          break;
        case LDPV_DEFAULT:
        default:
          vis = elfcpp::STV_DEFAULT;
          break;
        }

      if (isym->comdat_key != NULL
          && isym->comdat_key[0] != '\0'
          && !this->include_comdat_group(isym->comdat_key, layout))
        shndx = elfcpp::SHN_UNDEF;

      osym.put_st_name(0);
      osym.put_st_value(0);
      osym.put_st_size(0);
      osym.put_st_info(bind, elfcpp::STT_NOTYPE);
      osym.put_st_other(vis, 0);
      osym.put_st_shndx(shndx);

      elfcpp::Sym<size, big_endian> sym(symbuf);
      this->symbols_[i] =
        symtab->add_from_pluginobj<size, big_endian>(this, name, ver, &sym);
    }
}

template<int size, bool big_endian>
Archive::Should_include
Sized_pluginobj<size, big_endian>::do_should_include_member(
    Symbol_table* symtab,
    Layout* layout,
    Read_symbols_data*,
    std::string* why)
{
  char* tmpbuf = NULL;
  size_t tmpbuflen = 0;

  for (int i = 0; i < this->nsyms_; ++i)
    {
      const struct ld_plugin_symbol& sym = this->syms_[i];
      if (sym.def == LDPK_UNDEF || sym.def == LDPK_WEAKUNDEF)
        continue;
      const char* name = sym.name;
      Symbol* symbol;
      Archive::Should_include t = Archive::should_include_member(symtab,
								 layout,
								 name,
								 &symbol, why,
								 &tmpbuf,
								 &tmpbuflen);
      if (t == Archive::SHOULD_INCLUDE_YES)
	{
	  if (tmpbuf != NULL)
	    free(tmpbuf);
	  return t;
	}
    }
  if (tmpbuf != NULL)
    free(tmpbuf);
  return Archive::SHOULD_INCLUDE_UNKNOWN;
}

// Iterate over global symbols, calling a visitor class V for each.

template<int size, bool big_endian>
void
Sized_pluginobj<size, big_endian>::do_for_all_global_symbols(
    Read_symbols_data*,
    Library_base::Symbol_visitor_base* v)
{
  for (int i = 0; i < this->nsyms_; ++i)
    {
      const struct ld_plugin_symbol& sym = this->syms_[i];
      if (sym.def != LDPK_UNDEF)
	v->visit(sym.name);
    }
}

// Iterate over local symbols, calling a visitor class V for each GOT offset
// associated with a local symbol.
template<int size, bool big_endian>
void
Sized_pluginobj<size, big_endian>::do_for_all_local_got_entries(
    Got_offset_list::Visitor*) const
{
  gold_unreachable();
}

// Get the size of a section.  Not used for plugin objects.

template<int size, bool big_endian>
uint64_t
Sized_pluginobj<size, big_endian>::do_section_size(unsigned int)
{
  gold_unreachable();
  return 0;
}

// Get the name of a section.  Not used for plugin objects.

template<int size, bool big_endian>
std::string
Sized_pluginobj<size, big_endian>::do_section_name(unsigned int) const
{
  gold_unreachable();
  return std::string();
}

// Return a view of the contents of a section.  Not used for plugin objects.

template<int size, bool big_endian>
const unsigned char*
Sized_pluginobj<size, big_endian>::do_section_contents(
    unsigned int,
    section_size_type*,
    bool)
{
  gold_unreachable();
  return NULL;
}

// Return section flags.  Not used for plugin objects.

template<int size, bool big_endian>
uint64_t
Sized_pluginobj<size, big_endian>::do_section_flags(unsigned int)
{
  gold_unreachable();
  return 0;
}

// Return section entsize.  Not used for plugin objects.

template<int size, bool big_endian>
uint64_t
Sized_pluginobj<size, big_endian>::do_section_entsize(unsigned int)
{
  gold_unreachable();
  return 0;
}

// Return section address.  Not used for plugin objects.

template<int size, bool big_endian>
uint64_t
Sized_pluginobj<size, big_endian>::do_section_address(unsigned int)
{
  gold_unreachable();
  return 0;
}

// Return section type.  Not used for plugin objects.

template<int size, bool big_endian>
unsigned int
Sized_pluginobj<size, big_endian>::do_section_type(unsigned int)
{
  gold_unreachable();
  return 0;
}

// Return the section link field.  Not used for plugin objects.

template<int size, bool big_endian>
unsigned int
Sized_pluginobj<size, big_endian>::do_section_link(unsigned int)
{
  gold_unreachable();
  return 0;
}

// Return the section link field.  Not used for plugin objects.

template<int size, bool big_endian>
unsigned int
Sized_pluginobj<size, big_endian>::do_section_info(unsigned int)
{
  gold_unreachable();
  return 0;
}

// Return the section alignment.  Not used for plugin objects.

template<int size, bool big_endian>
uint64_t
Sized_pluginobj<size, big_endian>::do_section_addralign(unsigned int)
{
  gold_unreachable();
  return 0;
}

// Return the Xindex structure to use.  Not used for plugin objects.

template<int size, bool big_endian>
Xindex*
Sized_pluginobj<size, big_endian>::do_initialize_xindex()
{
  gold_unreachable();
  return NULL;
}

// Get symbol counts.  Don't count plugin objects; the replacement
// files will provide the counts.

template<int size, bool big_endian>
void
Sized_pluginobj<size, big_endian>::do_get_global_symbol_counts(
    const Symbol_table*,
    size_t* defined,
    size_t* used) const
{
  *defined = 0;
  *used = 0;
}

// Get symbols.  Not used for plugin objects.

template<int size, bool big_endian>
const Object::Symbols*
Sized_pluginobj<size, big_endian>::do_get_global_symbols() const
{
  gold_unreachable();
}

// Class Plugin_finish.  This task runs after all replacement files have
// been added.  For now, it's a placeholder for a possible plugin API
// to allow the plugin to release most of its resources.  The cleanup
// handlers must be called later, because they can remove the temporary
// object files that are needed until the end of the link.

class Plugin_finish : public Task
{
 public:
  Plugin_finish(Task_token* this_blocker, Task_token* next_blocker)
    : this_blocker_(this_blocker), next_blocker_(next_blocker)
  { }

  ~Plugin_finish()
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
  {
    Plugin_manager* plugins = parameters->options().plugins();
    gold_assert(plugins != NULL);
    // We could call early cleanup handlers here.
    if (plugins->recorder())
      plugins->recorder()->finish();
  }

  std::string
  get_name() const
  { return "Plugin_finish"; }

 private:
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

// Class Plugin_hook.

Plugin_hook::~Plugin_hook()
{
}

// Return whether a Plugin_hook task is runnable.

Task_token*
Plugin_hook::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

// Return a Task_locker for a Plugin_hook task.  We don't need any
// locks here.

void
Plugin_hook::locks(Task_locker*)
{
}

// Run the "all symbols read" plugin hook.

void
Plugin_hook::run(Workqueue* workqueue)
{
  gold_assert(this->options_.has_plugins());
  Symbol* start_sym = this->symtab_->lookup(parameters->entry());
  if (start_sym != NULL)
    start_sym->set_in_real_elf();

  this->options_.plugins()->all_symbols_read(workqueue,
                                             this,
                                             this->input_objects_,
                                             this->symtab_,
                                             this->dirpath_,
                                             this->mapfile_,
                                             &this->this_blocker_);
  workqueue->queue_soon(new Plugin_finish(this->this_blocker_,
					  this->next_blocker_));
}

// The C interface routines called by the plugins.

#ifdef ENABLE_PLUGINS

// Register a claim-file handler.

static enum ld_plugin_status
register_claim_file(ld_plugin_claim_file_handler handler)
{
  gold_assert(parameters->options().has_plugins());
  parameters->options().plugins()->set_claim_file_handler(handler);
  return LDPS_OK;
}

// Register an all-symbols-read handler.

static enum ld_plugin_status
register_all_symbols_read(ld_plugin_all_symbols_read_handler handler)
{
  gold_assert(parameters->options().has_plugins());
  parameters->options().plugins()->set_all_symbols_read_handler(handler);
  return LDPS_OK;
}

// Register a cleanup handler.

static enum ld_plugin_status
register_cleanup(ld_plugin_cleanup_handler handler)
{
  gold_assert(parameters->options().has_plugins());
  parameters->options().plugins()->set_cleanup_handler(handler);
  return LDPS_OK;
}

// Add symbols from a plugin-claimed input file.

static enum ld_plugin_status
add_symbols(void* handle, int nsyms, const ld_plugin_symbol* syms)
{
  gold_assert(parameters->options().has_plugins());
  Pluginobj* obj = parameters->options().plugins()->make_plugin_object(
      static_cast<unsigned int>(reinterpret_cast<intptr_t>(handle)));
  if (obj == NULL)
    return LDPS_ERR;
  obj->store_incoming_symbols(nsyms, syms);
  return LDPS_OK;
}

// Get the input file information with an open (possibly re-opened)
// file descriptor.

static enum ld_plugin_status
get_input_file(const void* handle, struct ld_plugin_input_file* file)
{
  gold_assert(parameters->options().has_plugins());
  unsigned int obj_index =
      static_cast<unsigned int>(reinterpret_cast<intptr_t>(handle));
  return parameters->options().plugins()->get_input_file(obj_index, file);
}

// Release the input file.

static enum ld_plugin_status
release_input_file(const void* handle)
{
  gold_assert(parameters->options().has_plugins());
  unsigned int obj_index =
      static_cast<unsigned int>(reinterpret_cast<intptr_t>(handle));
  return parameters->options().plugins()->release_input_file(obj_index);
}

static enum ld_plugin_status
get_view(const void *handle, const void **viewp)
{
  gold_assert(parameters->options().has_plugins());
  unsigned int obj_index =
      static_cast<unsigned int>(reinterpret_cast<intptr_t>(handle));
  return parameters->options().plugins()->get_view(obj_index, viewp);
}

// Get the symbol resolution info for a plugin-claimed input file.

static enum ld_plugin_status
get_symbols(const void* handle, int nsyms, ld_plugin_symbol* syms)
{
  gold_assert(parameters->options().has_plugins());
  Plugin_manager* plugins = parameters->options().plugins();
  Object* obj = plugins->object(
    static_cast<unsigned int>(reinterpret_cast<intptr_t>(handle)));
  if (obj == NULL)
    return LDPS_ERR;
  Pluginobj* plugin_obj = obj->pluginobj();
  if (plugin_obj == NULL)
    return LDPS_ERR;
  Symbol_table* symtab = plugins->symtab();
  return plugin_obj->get_symbol_resolution_info(symtab, nsyms, syms, 1);
}

// Version 2 of the above.  The only difference is that this version
// is allowed to return the resolution code LDPR_PREVAILING_DEF_IRONLY_EXP.

static enum ld_plugin_status
get_symbols_v2(const void* handle, int nsyms, ld_plugin_symbol* syms)
{
  gold_assert(parameters->options().has_plugins());
  Plugin_manager* plugins = parameters->options().plugins();
  Object* obj = plugins->object(
    static_cast<unsigned int>(reinterpret_cast<intptr_t>(handle)));
  if (obj == NULL)
    return LDPS_ERR;
  Pluginobj* plugin_obj = obj->pluginobj();
  if (plugin_obj == NULL)
    return LDPS_ERR;
  Symbol_table* symtab = plugins->symtab();
  return plugin_obj->get_symbol_resolution_info(symtab, nsyms, syms, 2);
}

// Version 3 of the above.  The only difference from v2 is that it
// returns LDPS_NO_SYMS instead of LDPS_OK for the objects we never
// decided to include.

static enum ld_plugin_status
get_symbols_v3(const void* handle, int nsyms, ld_plugin_symbol* syms)
{
  gold_assert(parameters->options().has_plugins());
  Plugin_manager* plugins = parameters->options().plugins();
  Object* obj = plugins->object(
    static_cast<unsigned int>(reinterpret_cast<intptr_t>(handle)));
  if (obj == NULL)
    return LDPS_ERR;
  Pluginobj* plugin_obj = obj->pluginobj();
  if (plugin_obj == NULL)
    return LDPS_ERR;
  Symbol_table* symtab = plugins->symtab();
  return plugin_obj->get_symbol_resolution_info(symtab, nsyms, syms, 3);
}

// Add a new (real) input file generated by a plugin.

static enum ld_plugin_status
add_input_file(const char* pathname)
{
  gold_assert(parameters->options().has_plugins());
  return parameters->options().plugins()->add_input_file(pathname, false);
}

// Add a new (real) library required by a plugin.

static enum ld_plugin_status
add_input_library(const char* pathname)
{
  gold_assert(parameters->options().has_plugins());
  return parameters->options().plugins()->add_input_file(pathname, true);
}

// Set the extra library path to be used by libraries added via
// add_input_library

static enum ld_plugin_status
set_extra_library_path(const char* path)
{
  gold_assert(parameters->options().has_plugins());
  return parameters->options().plugins()->set_extra_library_path(path);
}

// Issue a diagnostic message from a plugin.

static enum ld_plugin_status
message(int level, const char* format, ...)
{
  va_list args;
  va_start(args, format);

  switch (level)
    {
    case LDPL_INFO:
      parameters->errors()->info(format, args);
      break;
    case LDPL_WARNING:
      parameters->errors()->warning(format, args);
      break;
    case LDPL_ERROR:
    default:
      parameters->errors()->error(format, args);
      break;
    case LDPL_FATAL:
      parameters->errors()->fatal(format, args);
      break;
    }

  va_end(args);
  return LDPS_OK;
}

// Get the section count of the object corresponding to the handle.  This
// plugin interface can only be called in the claim_file handler of the plugin.

static enum ld_plugin_status
get_input_section_count(const void* handle, unsigned int* count)
{
  gold_assert(parameters->options().has_plugins());

  if (!parameters->options().plugins()->in_claim_file_handler())
    return LDPS_ERR;

  Object* obj = parameters->options().plugins()->get_elf_object(handle);

  if (obj == NULL)
    return LDPS_ERR;

  *count = obj->shnum();
  return LDPS_OK;
}

// Get the type of the specified section in the object corresponding
// to the handle.  This plugin interface can only be called in the
// claim_file handler of the plugin.

static enum ld_plugin_status
get_input_section_type(const struct ld_plugin_section section,
                       unsigned int* type)
{
  gold_assert(parameters->options().has_plugins());

  if (!parameters->options().plugins()->in_claim_file_handler())
    return LDPS_ERR;

  Object* obj
    = parameters->options().plugins()->get_elf_object(section.handle); 

  if (obj == NULL)
    return LDPS_BAD_HANDLE;

  *type = obj->section_type(section.shndx);
  return LDPS_OK;
}

// Get the name of the specified section in the object corresponding
// to the handle.  This plugin interface can only be called in the
// claim_file handler of the plugin.

static enum ld_plugin_status
get_input_section_name(const struct ld_plugin_section section,
                       char** section_name_ptr)
{
  gold_assert(parameters->options().has_plugins());

  if (!parameters->options().plugins()->in_claim_file_handler())
    return LDPS_ERR;

  Object* obj
    = parameters->options().plugins()->get_elf_object(section.handle); 

  if (obj == NULL)
    return LDPS_BAD_HANDLE;

  // Check if the object is locked before getting the section name.
  gold_assert(obj->is_locked());

  const std::string section_name = obj->section_name(section.shndx);
  *section_name_ptr = static_cast<char*>(malloc(section_name.length() + 1));
  memcpy(*section_name_ptr, section_name.c_str(), section_name.length() + 1);
  return LDPS_OK;
}

// Get the contents of the specified section in the object corresponding
// to the handle.  This plugin interface can only be called in the
// claim_file handler of the plugin.

static enum ld_plugin_status
get_input_section_contents(const struct ld_plugin_section section,
			   const unsigned char** section_contents_ptr,
			   size_t* len)
{
  gold_assert(parameters->options().has_plugins());

  if (!parameters->options().plugins()->in_claim_file_handler())
    return LDPS_ERR;

  Object* obj
    = parameters->options().plugins()->get_elf_object(section.handle); 

  if (obj == NULL)
    return LDPS_BAD_HANDLE;

  // Check if the object is locked before getting the section contents.
  gold_assert(obj->is_locked());

  section_size_type plen;
  *section_contents_ptr
      = obj->section_contents(section.shndx, &plen, false);
  *len = plen;
  return LDPS_OK;
}

// Get the alignment of the specified section in the object corresponding
// to the handle.  This plugin interface can only be called in the
// claim_file handler of the plugin.

static enum ld_plugin_status
get_input_section_alignment(const struct ld_plugin_section section,
                            unsigned int* addralign)
{
  gold_assert(parameters->options().has_plugins());

  if (!parameters->options().plugins()->in_claim_file_handler())
    return LDPS_ERR;

  Object* obj
    = parameters->options().plugins()->get_elf_object(section.handle);

  if (obj == NULL)
    return LDPS_BAD_HANDLE;

  *addralign = obj->section_addralign(section.shndx);
  return LDPS_OK;
}

// Get the size of the specified section in the object corresponding
// to the handle.  This plugin interface can only be called in the
// claim_file handler of the plugin.

static enum ld_plugin_status
get_input_section_size(const struct ld_plugin_section section,
                       uint64_t* secsize)
{
  gold_assert(parameters->options().has_plugins());

  if (!parameters->options().plugins()->in_claim_file_handler())
    return LDPS_ERR;

  Object* obj
    = parameters->options().plugins()->get_elf_object(section.handle);

  if (obj == NULL)
    return LDPS_BAD_HANDLE;

  *secsize = obj->section_size(section.shndx);
  return LDPS_OK;
}

static enum ld_plugin_status
get_wrap_symbols(uint64_t *count, const char ***wrap_symbols)
{
  gold_assert(parameters->options().has_plugins());
  *count = parameters->options().wrap_size();

  if (*count == 0)
    return LDPS_OK;

  *wrap_symbols = new const char *[*count];
  int i = 0;
  for (options::String_set::const_iterator
       it = parameters->options().wrap_begin();
       it != parameters->options().wrap_end(); ++it, ++i) {
    (*wrap_symbols)[i] = it->c_str();
  }
  return LDPS_OK;
}


// Specify the ordering of sections in the final layout. The sections are
// specified as (handle,shndx) pairs in the two arrays in the order in
// which they should appear in the final layout.

static enum ld_plugin_status
update_section_order(const struct ld_plugin_section* section_list,
		     unsigned int num_sections)
{
  gold_assert(parameters->options().has_plugins());

  if (num_sections == 0)
    return LDPS_OK;

  if (section_list == NULL)
    return LDPS_ERR;

  Layout* layout = parameters->options().plugins()->layout();
  gold_assert (layout != NULL);

  std::map<Section_id, unsigned int>* order_map
    = layout->get_section_order_map();

  /* Store the mapping from Section_id to section position in layout's
     order_map to consult after output sections are added.  */
  for (unsigned int i = 0; i < num_sections; ++i)
    {
      Object* obj = parameters->options().plugins()->get_elf_object(
          section_list[i].handle);
      if (obj == NULL || obj->is_dynamic())
	return LDPS_BAD_HANDLE;
      unsigned int shndx = section_list[i].shndx;
      Section_id secn_id(static_cast<Relobj*>(obj), shndx);
      (*order_map)[secn_id] = i + 1;
    }

  return LDPS_OK;
}

// Let the linker know that the sections could be reordered.

static enum ld_plugin_status
allow_section_ordering()
{
  gold_assert(parameters->options().has_plugins());
  Layout* layout = parameters->options().plugins()->layout();
  layout->set_section_ordering_specified();
  return LDPS_OK;
}

// Let the linker know that a subset of sections could be mapped
// to a unique segment.

static enum ld_plugin_status
allow_unique_segment_for_sections()
{
  gold_assert(parameters->options().has_plugins());
  Layout* layout = parameters->options().plugins()->layout();
  layout->set_unique_segment_for_sections_specified();
  return LDPS_OK;
}

// This function should map the list of sections specified in the
// SECTION_LIST to a unique segment.  ELF segments do not have names
// and the NAME is used to identify Output Section which should contain
// the list of sections.  This Output Section will then be mapped to
// a unique segment.  FLAGS is used to specify if any additional segment
// flags need to be set.  For instance, a specific segment flag can be
// set to identify this segment.  Unsetting segment flags is not possible.
// ALIGN specifies the alignment of the segment.

static enum ld_plugin_status
unique_segment_for_sections(const char* segment_name,
			    uint64_t flags,
			    uint64_t align,
			    const struct ld_plugin_section* section_list,
			    unsigned int num_sections)
{
  gold_assert(parameters->options().has_plugins());

  if (num_sections == 0)
    return LDPS_OK;

  if (section_list == NULL)
    return LDPS_ERR;

  Layout* layout = parameters->options().plugins()->layout();
  gold_assert (layout != NULL);

  Layout::Unique_segment_info* s = new Layout::Unique_segment_info;
  s->name = segment_name;
  s->flags = flags;
  s->align = align;

  for (unsigned int i = 0; i < num_sections; ++i)
    {
      Object* obj = parameters->options().plugins()->get_elf_object(
          section_list[i].handle);
      if (obj == NULL || obj->is_dynamic())
	return LDPS_BAD_HANDLE;
      unsigned int shndx = section_list[i].shndx;
      Const_section_id secn_id(static_cast<Relobj*>(obj), shndx);
      layout->insert_section_segment_map(secn_id, s);
    }

  return LDPS_OK;
}

// Register a new_input handler.

static enum ld_plugin_status
register_new_input(ld_plugin_new_input_handler handler)
{
  gold_assert(parameters->options().has_plugins());
  parameters->options().plugins()->set_new_input_handler(handler);
  return LDPS_OK;
}

#endif // ENABLE_PLUGINS

// Allocate a Pluginobj object of the appropriate size and endianness.

static Pluginobj*
make_sized_plugin_object(const std::string& filename,
			 Input_file* input_file, off_t offset, off_t filesize)
{
  Pluginobj* obj = NULL;

  parameters_force_valid_target();
  const Target& target(parameters->target());

  if (target.get_size() == 32)
    {
      if (target.is_big_endian())
#ifdef HAVE_TARGET_32_BIG
        obj = new Sized_pluginobj<32, true>(filename, input_file,
					    offset, filesize);
#else
        gold_error(_("%s: not configured to support "
		     "32-bit big-endian object"),
		   filename.c_str());
#endif
      else
#ifdef HAVE_TARGET_32_LITTLE
        obj = new Sized_pluginobj<32, false>(filename, input_file,
                                             offset, filesize);
#else
        gold_error(_("%s: not configured to support "
		     "32-bit little-endian object"),
		   filename.c_str());
#endif
    }
  else if (target.get_size() == 64)
    {
      if (target.is_big_endian())
#ifdef HAVE_TARGET_64_BIG
        obj = new Sized_pluginobj<64, true>(filename, input_file,
                                            offset, filesize);
#else
        gold_error(_("%s: not configured to support "
		     "64-bit big-endian object"),
		   filename.c_str());
#endif
      else
#ifdef HAVE_TARGET_64_LITTLE
        obj = new Sized_pluginobj<64, false>(filename, input_file,
                                             offset, filesize);
#else
        gold_error(_("%s: not configured to support "
		     "64-bit little-endian object"),
		   filename.c_str());
#endif
    }

  gold_assert(obj != NULL);
  return obj;
}

} // End namespace gold.
