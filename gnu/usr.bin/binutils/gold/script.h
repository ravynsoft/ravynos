// script.h -- handle linker scripts for gold   -*- C++ -*-

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

// We implement a subset of the original GNU ld linker script language
// for compatibility.  The goal is not to implement the entire
// language.  It is merely to implement enough to handle common uses.
// In particular we need to handle /usr/lib/libc.so on a typical
// GNU/Linux system, and we want to handle linker scripts used by the
// Linux kernel build.

#ifndef GOLD_SCRIPT_H
#define GOLD_SCRIPT_H

#include <cstdio>
#include <string>
#include <vector>

#include "elfcpp.h"
#include "script-sections.h"

namespace gold
{

class General_options;
class Command_line;
class Symbol_table;
class Layout;
class Mapfile;
class Input_argument;
class Input_arguments;
class Input_objects;
class Input_group;
class Input_file;
class Output_segment;
class Task_token;
class Workqueue;
struct Version_dependency_list;
struct Version_expression_list;
struct Version_tree;
struct Version_expression;
class Lazy_demangler;
class Incremental_script_entry;

// This class represents an expression in a linker script.

class Expression
{
 protected:
  // These should only be created by child classes.
  Expression()
  { }

 public:
  virtual ~Expression()
  { }

  // Return the value of the expression which is not permitted to
  // refer to the dot symbol.  CHECK_ASSERTIONS is true if we should
  // check whether assertions are true.
  uint64_t
  eval(const Symbol_table*, const Layout*, bool check_assertions);

  // Return the value of an expression which is permitted to refer to
  // the dot symbol.  DOT_VALUE is the absolute value of the dot
  // symbol.  DOT_SECTION is the section in which dot is defined; it
  // should be NULL if the dot symbol has an absolute value (e.g., is
  // defined in a SECTIONS clause outside of any output section
  // definition).  This sets *RESULT_SECTION to indicate where the
  // value is defined.  If the value is absolute *RESULT_SECTION will
  // be NULL.  Note that the returned value is still an absolute
  // value; to get a section relative value the caller must subtract
  // the section address.  If RESULT_ALIGNMENT is not NULL, this sets
  // *RESULT_ALIGNMENT to the alignment of the value of that alignment
  // is larger than *RESULT_ALIGNMENT; this will only be non-zero if
  // this is an ALIGN expression.  If IS_SECTION_DOT_ASSIGMENT is true,
  // we are evaluating an assignment to dot within an output section,
  // and an absolute value should be interpreted as an offset within
  // the section.
  uint64_t
  eval_with_dot(const Symbol_table*, const Layout*, bool check_assertions,
		uint64_t dot_value, Output_section* dot_section,
		Output_section** result_section, uint64_t* result_alignment,
		bool is_section_dot_assignment);

  // Return the value of an expression which may or may not be
  // permitted to refer to the dot symbol, depending on
  // is_dot_available.  If IS_SECTION_DOT_ASSIGMENT is true,
  // we are evaluating an assignment to dot within an output section,
  // and an absolute value should be interpreted as an offset within
  // the section.
  uint64_t
  eval_maybe_dot(const Symbol_table*, const Layout*, bool check_assertions,
		 bool is_dot_available, uint64_t dot_value,
		 Output_section* dot_section,
		 Output_section** result_section, uint64_t* result_alignment,
		 elfcpp::STT* type, elfcpp::STV* vis, unsigned char* nonvis,
		 bool is_section_dot_assignment, bool* is_valid_pointer);

  // Print the expression to the FILE.  This is for debugging.
  virtual void
  print(FILE*) const = 0;

 protected:
  struct Expression_eval_info;

 public:
  // Compute the value of the expression (implemented by child class).
  // This is public rather than protected because it is called
  // directly by children of Expression on other Expression objects.
  virtual uint64_t
  value(const Expression_eval_info*) = 0;

  // Sets all symbols used in expressions as seen in a real ELF object.
  virtual void
  set_expr_sym_in_real_elf(Symbol_table*) const
  { return; }

 private:
  // May not be copied.
  Expression(const Expression&);
  Expression& operator=(const Expression&);
};

// Version_script_info stores information parsed from the version
// script, either provided by --version-script or as part of a linker
// script.  A single Version_script_info object per target is owned by
// Script_options.

class Version_script_info
{
 public:
  // The languages which can be specified in a versionn script.
  enum Language
  {
    LANGUAGE_C,		// No demangling.
    LANGUAGE_CXX,	// C++ demangling.
    LANGUAGE_JAVA,	// Java demangling.
    LANGUAGE_COUNT
  };

  Version_script_info();

  ~Version_script_info();

  // Clear everything.
  void
  clear();

  // Finalize the version control information.
  void
  finalize();

  // Return whether the information is finalized.
  bool
  is_finalized() const
  { return this->is_finalized_; }

  // Return whether any version were defined in the version script.
  bool
  empty() const
  { return this->version_trees_.empty(); }

  // If there is a version associated with SYMBOL, return true, and
  // set *VERSION to the version, and *IS_GLOBAL to whether the symbol
  // should be global.  Otherwise, return false.
  bool
  get_symbol_version(const char* symbol, std::string* version,
		     bool* is_global) const;

  // Return whether this symbol matches the local: section of some
  // version.
  bool
  symbol_is_local(const char* symbol) const
  {
    bool is_global;
    return (this->get_symbol_version(symbol, NULL, &is_global)
	    && !is_global);
  }

  // Return the names of versions defined in the version script.
  std::vector<std::string>
  get_versions() const;

  // Return the list of dependencies for this version.
  std::vector<std::string>
  get_dependencies(const char* version) const;

  // The following functions should only be used by the bison helper
  // functions.  They allocate new structs whose memory belongs to
  // Version_script_info.  The bison functions copy the information
  // from the version script into these structs.
  struct Version_dependency_list*
  allocate_dependency_list();

  struct Version_expression_list*
  allocate_expression_list();

  struct Version_tree*
  allocate_version_tree();

  // Build the lookup tables after all data have been read.
  void
  build_lookup_tables();

  // Give an error if there are any unmatched names in the version
  // script.
  void
  check_unmatched_names(const Symbol_table*) const;

  // Print contents to the FILE.  This is for debugging.
  void
  print(FILE*) const;

 private:
  void
  print_expression_list(FILE* f, const Version_expression_list*) const;

  bool
  get_symbol_version_helper(const char* symbol,
			    bool check_global,
			    std::string* pversion) const;

  // Fast lookup information for a given language.

  // We map from exact match strings to Version_tree's.  Historically
  // version scripts sometimes have the same symbol multiple times,
  // which is ambiguous.  We warn about that case by storing the
  // second Version_tree we see.
  struct Version_tree_match
  {
    Version_tree_match(const Version_tree* r, bool ig,
		       const Version_expression* e)
      : real(r), is_global(ig), expression(e), ambiguous(NULL)
    { }

    // The Version_tree that we return.
    const Version_tree* real;
    // True if this is a global match for the REAL member, false if it
    // is a local match.
    bool is_global;
    // Point back to the Version_expression for which we created this
    // match.
    const Version_expression* expression;
    // If not NULL, another Version_tree that defines the symbol.
    const Version_tree* ambiguous;
  };

  // Map from an exact match string to a Version_tree.

  typedef Unordered_map<std::string, Version_tree_match> Exact;

  // Fast lookup information for a glob pattern.
  struct Glob
  {
    Glob()
      : expression(NULL), version(NULL), is_global(false)
    { }

    Glob(const Version_expression* e, const Version_tree* v, bool ig)
      : expression(e), version(v), is_global(ig)
    { }

    // A pointer to the version expression holding the pattern to
    // match and the language to use for demangling the symbol before
    // doing the match.
    const Version_expression* expression;
    // The Version_tree we use if this pattern matches.
    const Version_tree* version;
    // True if this is a global symbol.
    bool is_global;
  };

  typedef std::vector<Glob> Globs;

  bool
  unquote(std::string*) const;

  void
  add_exact_match(const std::string&, const Version_tree*, bool is_global,
		  const Version_expression*, Exact*);

  void
  build_expression_list_lookup(const Version_expression_list*,
			       const Version_tree*, bool);

  const char*
  get_name_to_match(const char*, int,
		    Lazy_demangler*, Lazy_demangler*) const;

  // All the version dependencies we allocate.
  std::vector<Version_dependency_list*> dependency_lists_;
  // All the version expressions we allocate.
  std::vector<Version_expression_list*> expression_lists_;
  // The list of versions.
  std::vector<Version_tree*> version_trees_;
  // Exact matches for global symbols, by language.
  Exact* exact_[LANGUAGE_COUNT];
  // A vector of glob patterns mapping to Version_trees.
  Globs globs_;
  // The default version to use, if there is one.  This is from a
  // pattern of "*".
  const Version_tree* default_version_;
  // True if the default version is global.
  bool default_is_global_;
  // Whether this has been finalized.
  bool is_finalized_;
};

// This class manages assignments to symbols.  These can appear in
// three different locations in scripts: outside of a SECTIONS clause,
// within a SECTIONS clause, and within an output section definition
// within a SECTIONS clause.  This can also appear on the command line
// via the --defsym command line option.

class Symbol_assignment
{
 public:
  Symbol_assignment(const char* name, size_t namelen, bool is_defsym,
		    Expression* val, bool provide, bool hidden)
    : name_(name, namelen), val_(val), is_defsym_(is_defsym),
      provide_(provide), hidden_(hidden), sym_(NULL)
  { }

  // Add the symbol to the symbol table.
  void
  add_to_table(Symbol_table*);

  // Finalize the symbol value.
  void
  finalize(Symbol_table*, const Layout*);

  bool
  is_defsym() const
  { return is_defsym_; }

  Expression *
  value() const
  { return val_; }

  // Finalize the symbol value when it can refer to the dot symbol.
  void
  finalize_with_dot(Symbol_table*, const Layout*, uint64_t dot_value,
		    Output_section* dot_section);

  // Set the symbol value, but only if the value is absolute or relative to
  // DOT_SECTION.  This is used while processing a SECTIONS clause.
  // We assume that dot is an absolute value here.  We do not check assertions.
  void
  set_if_absolute(Symbol_table*, const Layout*, bool is_dot_available,
		  uint64_t dot_value, Output_section* dot_section);

  const std::string&
  name() const
  { return this->name_; }

  // Print the assignment to the FILE.  This is for debugging.
  void
  print(FILE*) const;

 private:
  // Shared by finalize and finalize_with_dot.
  void
  finalize_maybe_dot(Symbol_table*, const Layout*, bool is_dot_available,
		     uint64_t dot_value, Output_section* dot_section);

  // Sized version of finalize.
  template<int size>
  void
  sized_finalize(Symbol_table*, const Layout*, bool is_dot_available,
		 uint64_t dot_value, Output_section*);

  // Symbol name.
  std::string name_;
  // Expression to assign to symbol.
  Expression* val_;
  // True if this symbol is defined by a --defsym, false if it is
  // defined in a linker script.
  bool is_defsym_;
  // Whether the assignment should be provided (only set if there is
  // an undefined reference to the symbol.
  bool provide_;
  // Whether the assignment should be hidden.
  bool hidden_;
  // The entry in the symbol table.
  Symbol* sym_;
};

// This class manages assertions in linker scripts.  These can appear
// in all the places where a Symbol_assignment can appear.

class Script_assertion
{
 public:
  Script_assertion(Expression* check, const char* message,
		   size_t messagelen)
    : check_(check), message_(message, messagelen)
  { }

  // Check the assertion.
  void
  check(const Symbol_table*, const Layout*);

  // Print the assertion to the FILE.  This is for debugging.
  void
  print(FILE*) const;

 private:
  // The expression to check.
  Expression* check_;
  // The message to issue if the expression fails.
  std::string message_;
};

// We can read a linker script in two different contexts: when
// initially parsing the command line, and when we find an input file
// which is actually a linker script.  Also some of the data which can
// be set by a linker script can also be set via command line options
// like -e and --defsym.  This means that we have a type of data which
// can be set both during command line option parsing and while
// reading input files.  We store that data in an instance of this
// object.  We will keep pointers to that instance in both the
// Command_line and Layout objects.

class Script_options
{
 public:
  Script_options();

  // Add a symbol to be defined.
  void
  add_symbol_assignment(const char* name, size_t length, bool is_defsym,
			Expression* value, bool provide, bool hidden);

  // Look for an assigned symbol.
  bool
  is_pending_assignment(const char* name);
  
  // Add a reference to a symbol.
  void
  add_symbol_reference(const char* name, size_t length);

  // Add an assertion.
  void
  add_assertion(Expression* check, const char* message, size_t messagelen);

  // Define a symbol from the command line.
  bool
  define_symbol(const char* definition);

  // Populates the set with symbol names used in LHS of defsym.
  void
  find_defsym_defs(Unordered_set<std::string>&);

  // Set symbols used in defsym expressions as seen in a real ELF object.
  void set_defsym_uses_in_real_elf(Symbol_table*) const;

  // Create sections required by any linker scripts.
  void
  create_script_sections(Layout*);

  // Add all symbol definitions to the symbol table.
  void
  add_symbols_to_table(Symbol_table*);

  // Used to iterate over symbols which are referenced in expressions
  // but not defined.
  typedef Unordered_set<std::string>::const_iterator referenced_const_iterator;

  referenced_const_iterator
  referenced_begin() const
  { return this->symbol_references_.begin(); }

  referenced_const_iterator
  referenced_end() const
  { return this->symbol_references_.end(); }

  // Return whether a symbol is referenced but not defined.
  bool
  is_referenced(const std::string& name) const
  {
    return (this->symbol_references_.find(name)
	    != this->symbol_references_.end());
  }

  // Return whether there are any symbols which were referenced but
  // not defined.
  bool
  any_unreferenced() const
  { return !this->symbol_references_.empty(); }

  // Finalize the symbol values.  Also check assertions.
  void
  finalize_symbols(Symbol_table*, const Layout*);

  // Version information parsed from a version script.  Everything
  // else has a pointer to this object.
  Version_script_info*
  version_script_info()
  { return &this->version_script_info_; }

  const Version_script_info*
  version_script_info() const
  { return &this->version_script_info_; }

  // A SECTIONS clause parsed from a linker script.  Everything else
  // has a pointer to this object.
  Script_sections*
  script_sections()
  { return &this->script_sections_; }

  const Script_sections*
  script_sections() const
  { return &this->script_sections_; }

  // Whether we saw a SECTIONS clause.
  bool
  saw_sections_clause() const
  { return this->script_sections_.saw_sections_clause(); }

  // Whether we saw a PHDRS clause.
  bool
  saw_phdrs_clause() const
  { return this->script_sections_.saw_phdrs_clause(); }

  // Set section addresses using a SECTIONS clause.  Return the
  // segment which should hold the file header and segment headers;
  // this may return NULL, in which case the headers are not in a
  // loadable segment.
  Output_segment*
  set_section_addresses(Symbol_table*, Layout*);

  // Print the script to the FILE.  This is for debugging.
  void
  print(FILE*) const;

 private:
  // We keep a list of symbol assignments which occur outside of a
  // SECTIONS clause.
  typedef std::vector<Symbol_assignment*> Symbol_assignments;

  // We keep a list of all assertions which occur outside of a
  // SECTIONS clause.
  typedef std::vector<Script_assertion*> Assertions;

  // The entry address.  This will be empty if not set.
  std::string entry_;
  // Symbols to set.
  Symbol_assignments symbol_assignments_;
  // Symbols defined in an expression, for faster lookup.
  Unordered_set<std::string> symbol_definitions_;
  // Symbols referenced in an expression.
  Unordered_set<std::string> symbol_references_;
  // Assertions to check.
  Assertions assertions_;
  // Version information parsed from a version script.
  Version_script_info version_script_info_;
  // Information from any SECTIONS clauses.
  Script_sections script_sections_;
};

// FILE was found as an argument on the command line, but was not
// recognized as an ELF file.  Try to read it as a script.  Return
// true if the file was handled.  This has to handle /usr/lib/libc.so
// on a GNU/Linux system.  *USED_NEXT_BLOCKER is set to indicate
// whether the function took over NEXT_BLOCKER.

bool
read_input_script(Workqueue*, Symbol_table*, Layout*, Dirsearch*, int,
		  Input_objects*, Mapfile*, Input_group*,
		  const Input_argument*, Input_file*,
		  Task_token* next_blocker, bool* used_next_blocker);

// FILE was found as an argument to --script (-T).
// Read it as a script, and execute its contents immediately.

bool
read_commandline_script(const char* filename, Command_line* cmdline);

// FILE was found as an argument to --version-script.  Read it as a
// version script, and store its contents in
// cmdline->script_options()->version_script_info().

bool
read_version_script(const char* filename, Command_line* cmdline);

// FILENAME was found as an argument to --dynamic-list.  Read it as a
// version script (actually, a versym_node from a version script), and
// store its contents in DYNAMIC_LIST.

bool
read_dynamic_list(const char* filename, Command_line* cmdline,
                  Script_options* dynamic_list);

} // End namespace gold.

#endif // !defined(GOLD_SCRIPT_H)
