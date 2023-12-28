// plugin.h -- plugin manager for gold      -*- C++ -*-

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

#ifndef GOLD_PLUGIN_H
#define GOLD_PLUGIN_H

#include <list>
#include <string>

#include "object.h"
#include "plugin-api.h"
#include "workqueue.h"

namespace gold
{

class General_options;
class Input_file;
class Input_objects;
class Archive;
class Input_group;
class Symbol;
class Symbol_table;
class Layout;
class Dirsearch;
class Mapfile;
class Task;
class Task_token;
class Pluginobj;
class Plugin_rescan;
class Plugin_recorder;

// This class represents a single plugin library.

class Plugin
{
 public:
  Plugin(const char* filename)
    : handle_(NULL),
      filename_(filename),
      args_(),
      claim_file_handler_(NULL),
      all_symbols_read_handler_(NULL),
      cleanup_handler_(NULL),
      new_input_handler_(NULL),
      cleanup_done_(false)
  { }

  ~Plugin()
  { }

  // Load the library and call its entry point.
  void
  load();

  // Call the claim-file handler.
  bool
  claim_file(struct ld_plugin_input_file* plugin_input_file);

  // Call the all-symbols-read handler.
  void
  all_symbols_read();

  // Call the new_input handler.
  void
  new_input(struct ld_plugin_input_file* plugin_input_file);

  // Call the cleanup handler.
  void
  cleanup();

  // Register a claim-file handler.
  void
  set_claim_file_handler(ld_plugin_claim_file_handler handler)
  { this->claim_file_handler_ = handler; }

  // Register an all-symbols-read handler.
  void
  set_all_symbols_read_handler(ld_plugin_all_symbols_read_handler handler)
  { this->all_symbols_read_handler_ = handler; }

  // Register a claim-file handler.
  void
  set_cleanup_handler(ld_plugin_cleanup_handler handler)
  { this->cleanup_handler_ = handler; }

  // Register a new_input handler.
  void
  set_new_input_handler(ld_plugin_new_input_handler handler)
  { this->new_input_handler_ = handler; }

  // Add an argument
  void
  add_option(const char* arg)
  {
    this->args_.push_back(arg);
  }

  const std::string&
  filename() const
  { return this->filename_; }

 private:
  Plugin(const Plugin&);
  Plugin& operator=(const Plugin&);

  // The shared library handle returned by dlopen.
  void* handle_;
  // The argument string given to --plugin.
  std::string filename_;
  // The list of argument string given to --plugin-opt.
  std::vector<std::string> args_;
  // The plugin's event handlers.
  ld_plugin_claim_file_handler claim_file_handler_;
  ld_plugin_all_symbols_read_handler all_symbols_read_handler_;
  ld_plugin_cleanup_handler cleanup_handler_;
  ld_plugin_new_input_handler new_input_handler_;
  // TRUE if the cleanup handlers have been called.
  bool cleanup_done_;
};

// A manager class for plugins.

class Plugin_manager
{
 public:
  Plugin_manager(const General_options& options)
    : plugins_(), objects_(), deferred_layout_objects_(), input_file_(NULL),
      plugin_input_file_(), rescannable_(), undefined_symbols_(),
      any_claimed_(false), in_replacement_phase_(false), any_added_(false),
      in_claim_file_handler_(false),
      options_(options), workqueue_(NULL), task_(NULL), input_objects_(NULL),
      symtab_(NULL), layout_(NULL), dirpath_(NULL), mapfile_(NULL),
      this_blocker_(NULL), extra_search_path_(), lock_(NULL),
      initialize_lock_(&lock_), defsym_defines_set_(),
      recorder_(NULL)
  { this->current_ = plugins_.end(); }

  ~Plugin_manager();

  // Returns true if the symbol name is used in the LHS of a defsym.
  bool
  is_defsym_def(const char* sym_name) const
  {
    return defsym_defines_set_.find(sym_name) != defsym_defines_set_.end();
  }

  // Add a plugin library.
  void
  add_plugin(const char* filename)
  { this->plugins_.push_back(new Plugin(filename)); }

  // Add an argument to the current plugin.
  void
  add_plugin_option(const char* opt)
  {
    Plugin* last = this->plugins_.back();
    last->add_option(opt);
  }

  // Load all plugin libraries.
  void
  load_plugins(Layout* layout);

  // Call the plugin claim-file handlers in turn to see if any claim the file.
  Pluginobj*
  claim_file(Input_file* input_file, off_t offset, off_t filesize,
             Object* elf_object);

  // Get the object associated with the handle and check if it is an elf object.
  // If it is not a Pluginobj, it is an elf object.
  Object*
  get_elf_object(const void* handle);

  // True if the claim_file handler of the plugins is being called.
  bool
  in_claim_file_handler()
  { return in_claim_file_handler_; }

  // Let the plugin manager save an archive for later rescanning.
  // This takes ownership of the Archive pointer.
  void
  save_archive(Archive*);

  // Let the plugin manager save an input group for later rescanning.
  // This takes ownership of the Input_group pointer.
  void
  save_input_group(Input_group*);

  // Call the all-symbols-read handlers.
  void
  all_symbols_read(Workqueue* workqueue, Task* task,
                   Input_objects* input_objects, Symbol_table* symtab,
                   Dirsearch* dirpath, Mapfile* mapfile,
                   Task_token** last_blocker);

  // Tell the plugin manager that we've a new undefined symbol which
  // may require rescanning.
  void
  new_undefined_symbol(Symbol*);

  // Run deferred layout.
  void
  layout_deferred_objects();

  // Call the cleanup handlers.
  void
  cleanup();

  // Register a claim-file handler.
  void
  set_claim_file_handler(ld_plugin_claim_file_handler handler)
  {
    gold_assert(this->current_ != plugins_.end());
    (*this->current_)->set_claim_file_handler(handler);
  }

  // Register an all-symbols-read handler.
  void
  set_all_symbols_read_handler(ld_plugin_all_symbols_read_handler handler)
  {
    gold_assert(this->current_ != plugins_.end());
    (*this->current_)->set_all_symbols_read_handler(handler);
  }

  // Register a new_input handler.
  void
  set_new_input_handler(ld_plugin_new_input_handler handler)
  {
    gold_assert(this->current_ != plugins_.end());
    (*this->current_)->set_new_input_handler(handler);
  }

  // Register a claim-file handler.
  void
  set_cleanup_handler(ld_plugin_cleanup_handler handler)
  {
    gold_assert(this->current_ != plugins_.end());
    (*this->current_)->set_cleanup_handler(handler);
  }

  // Make a new Pluginobj object.  This is called when the plugin calls
  // the add_symbols API.
  Pluginobj*
  make_plugin_object(unsigned int handle);

  // Return the object associated with the given HANDLE.
  Object*
  object(unsigned int handle) const
  {
    if (handle >= this->objects_.size())
      return NULL;
    return this->objects_[handle];
  }

  // Return TRUE if any input files have been claimed by a plugin
  // and we are still in the initial input phase.
  bool
  should_defer_layout() const
  { return this->any_claimed_ && !this->in_replacement_phase_; }

  // Add a regular object to the deferred layout list.  These are
  // objects whose layout has been deferred until after the
  // replacement files have arrived.
  void
  add_deferred_layout_object(Relobj* obj)
  { this->deferred_layout_objects_.push_back(obj); }

  // Get input file information with an open (possibly re-opened)
  // file descriptor.
  ld_plugin_status
  get_input_file(unsigned int handle, struct ld_plugin_input_file* file);

  ld_plugin_status
  get_view(unsigned int handle, const void **viewp);

  // Release an input file.
  ld_plugin_status
  release_input_file(unsigned int handle);

  // Add a new input file.
  ld_plugin_status
  add_input_file(const char* pathname, bool is_lib);

  // Set the extra library path.
  ld_plugin_status
  set_extra_library_path(const char* path);

  // Return TRUE if we are in the replacement phase.
  bool
  in_replacement_phase() const
  { return this->in_replacement_phase_; }

  Input_objects*
  input_objects() const
  { return this->input_objects_; }

  Symbol_table*
  symtab()
  { return this->symtab_; }

  Layout*
  layout()
  { return this->layout_; }

  Plugin_recorder*
  recorder() const
  { return this->recorder_; }

 private:
  Plugin_manager(const Plugin_manager&);
  Plugin_manager& operator=(const Plugin_manager&);

  // Plugin_rescan is a Task which calls the private rescan method.
  friend class Plugin_rescan;

  // An archive or input group which may have to be rescanned if a
  // plugin adds a new file.
  struct Rescannable
  {
    bool is_archive;
    union
    {
      Archive* archive;
      Input_group* input_group;
    } u;

    Rescannable(Archive* archive)
      : is_archive(true)
    { this->u.archive = archive; }

    Rescannable(Input_group* input_group)
      : is_archive(false)
    { this->u.input_group = input_group; }
  };

  typedef std::list<Plugin*> Plugin_list;
  typedef std::vector<Object*> Object_list;
  typedef std::vector<Relobj*> Deferred_layout_list;
  typedef std::vector<Rescannable> Rescannable_list;
  typedef std::vector<Symbol*> Undefined_symbol_list;

  // Rescan archives for undefined symbols.
  void
  rescan(Task*);

  // See whether the rescannable at index I defines SYM.
  bool
  rescannable_defines(size_t i, Symbol* sym);

  // The list of plugin libraries.
  Plugin_list plugins_;
  // A pointer to the current plugin.  Used while loading plugins.
  Plugin_list::iterator current_;

  // The list of plugin objects.  The index of an item in this list
  // serves as the "handle" that we pass to the plugins.
  Object_list objects_;

  // The list of regular objects whose layout has been deferred.
  Deferred_layout_list deferred_layout_objects_;

  // The file currently up for claim by the plugins.
  Input_file* input_file_;
  struct ld_plugin_input_file plugin_input_file_;

  // A list of archives and input groups being saved for possible
  // later rescanning.
  Rescannable_list rescannable_;

  // A list of undefined symbols found in added files.
  Undefined_symbol_list undefined_symbols_;

  // Whether any input files have been claimed by a plugin.
  bool any_claimed_;

  // Set to true after the all symbols read event; indicates that we
  // are processing replacement files whose symbols should replace the
  // placeholder symbols from the Pluginobj objects.
  bool in_replacement_phase_;

  // Whether any input files or libraries were added by a plugin.
  bool any_added_;

  // Set to true when the claim_file handler of a plugin is called.
  bool in_claim_file_handler_;

  const General_options& options_;
  Workqueue* workqueue_;
  Task* task_;
  Input_objects* input_objects_;
  Symbol_table* symtab_;
  Layout* layout_;
  Dirsearch* dirpath_;
  Mapfile* mapfile_;
  Task_token* this_blocker_;

  // An extra directory to search for the libraries passed by
  // add_input_library.
  std::string extra_search_path_;
  Lock* lock_;
  Initialize_lock initialize_lock_;

  // Keep track of all symbols defined by defsym.
  typedef Unordered_set<std::string> Defsym_defines_set;
  Defsym_defines_set defsym_defines_set_;

  // Class to record plugin actions.
  Plugin_recorder* recorder_;
};


// An object file claimed by a plugin.  This is an abstract base class.
// The implementation is the template class Sized_pluginobj.

class Pluginobj : public Object
{
 public:

  typedef std::vector<Symbol*> Symbols;

  Pluginobj(const std::string& name, Input_file* input_file, off_t offset,
            off_t filesize);

  // Fill in the symbol resolution status for the given plugin symbols.
  ld_plugin_status
  get_symbol_resolution_info(Symbol_table* symtab,
			     int nsyms,
			     ld_plugin_symbol* syms,
			     int version) const;

  // Store the incoming symbols from the plugin for later processing.
  void
  store_incoming_symbols(int nsyms, const struct ld_plugin_symbol* syms)
  {
    this->nsyms_ = nsyms;
    this->syms_ = syms;
  }

  // Return TRUE if the comdat group with key COMDAT_KEY from this object
  // should be kept.
  bool
  include_comdat_group(std::string comdat_key, Layout* layout);

  // Return the filename.
  const std::string&
  filename() const
  { return this->input_file()->filename(); }

  // Return the file descriptor.
  int
  descriptor()
  { return this->input_file()->file().descriptor(); }

  // Return the size of the file or archive member.
  off_t
  filesize()
  { return this->filesize_; }

  // Return the word size of the object file.
  int
  elfsize() const
  { gold_unreachable(); }

  // Return TRUE if this is a big-endian object file.
  bool
  is_big_endian() const
  { gold_unreachable(); }

 protected:
  // Return TRUE if this is an object claimed by a plugin.
  virtual Pluginobj*
  do_pluginobj()
  { return this; }

  // The number of symbols provided by the plugin.
  int nsyms_;

  // The symbols provided by the plugin.
  const struct ld_plugin_symbol* syms_;

  // The entries in the symbol table for the external symbols.
  Symbols symbols_;

 private:
  // Size of the file (or archive member).
  off_t filesize_;
  // Map a comdat key symbol to a boolean indicating whether the comdat
  // group in this object with that key should be kept.
  typedef Unordered_map<std::string, bool> Comdat_map;
  Comdat_map comdat_map_;
};

// A plugin object, size-specific version.

template<int size, bool big_endian>
class Sized_pluginobj : public Pluginobj
{
 public:
  Sized_pluginobj(const std::string& name, Input_file* input_file,
                  off_t offset, off_t filesize);

  // Read the symbols.
  void
  do_read_symbols(Read_symbols_data*);

  // Lay out the input sections.
  void
  do_layout(Symbol_table*, Layout*, Read_symbols_data*);

  // Add the symbols to the symbol table.
  void
  do_add_symbols(Symbol_table*, Read_symbols_data*, Layout*);

  Archive::Should_include
  do_should_include_member(Symbol_table* symtab, Layout*, Read_symbols_data*,
                           std::string* why);

  // Iterate over global symbols, calling a visitor class V for each.
  void
  do_for_all_global_symbols(Read_symbols_data* sd,
			    Library_base::Symbol_visitor_base* v);

  // Iterate over local symbols, calling a visitor class V for each GOT offset
  // associated with a local symbol.
  void
  do_for_all_local_got_entries(Got_offset_list::Visitor* v) const;

  // Get the size of a section.
  uint64_t
  do_section_size(unsigned int shndx);

  // Get the name of a section.
  std::string
  do_section_name(unsigned int shndx) const;

  // Return a view of the contents of a section.
  const unsigned char*
  do_section_contents(unsigned int shndx, section_size_type* plen,
		      bool cache);

  // Return section flags.
  uint64_t
  do_section_flags(unsigned int shndx);

  // Return section entsize.
  uint64_t
  do_section_entsize(unsigned int shndx);

  // Return section address.
  uint64_t
  do_section_address(unsigned int shndx);

  // Return section type.
  unsigned int
  do_section_type(unsigned int shndx);

  // Return the section link field.
  unsigned int
  do_section_link(unsigned int shndx);

  // Return the section link field.
  unsigned int
  do_section_info(unsigned int shndx);

  // Return the section alignment.
  uint64_t
  do_section_addralign(unsigned int shndx);

  // Return the Xindex structure to use.
  Xindex*
  do_initialize_xindex();

  // Get symbol counts.
  void
  do_get_global_symbol_counts(const Symbol_table*, size_t*, size_t*) const;

  // Get global symbols.
  const Symbols*
  do_get_global_symbols() const;

  // Add placeholder symbols from a claimed file.
  ld_plugin_status
  add_symbols_from_plugin(int nsyms, const ld_plugin_symbol* syms);

 protected:

 private:
};

// This Task handles handles the "all symbols read" event hook.
// The plugin may add additional input files at this time, which must
// be queued for reading.

class Plugin_hook : public Task
{
 public:
  Plugin_hook(const General_options& options, Input_objects* input_objects,
	      Symbol_table* symtab, Layout* /*layout*/, Dirsearch* dirpath,
	      Mapfile* mapfile, Task_token* this_blocker,
	      Task_token* next_blocker)
    : options_(options), input_objects_(input_objects), symtab_(symtab),
      dirpath_(dirpath), mapfile_(mapfile),
      this_blocker_(this_blocker), next_blocker_(next_blocker)
  { }

  ~Plugin_hook();

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const
  { return "Plugin_hook"; }

 private:
  const General_options& options_;
  Input_objects* input_objects_;
  Symbol_table* symtab_;
  Dirsearch* dirpath_;
  Mapfile* mapfile_;
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

} // End namespace gold.

#endif // !defined(GOLD_PLUGIN_H)
