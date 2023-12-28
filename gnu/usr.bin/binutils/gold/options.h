// options.h -- handle command line options for gold  -*- C++ -*-

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

// General_options (from Command_line::options())
//   All the options (a.k.a. command-line flags)
// Input_argument (from Command_line::inputs())
//   The list of input files, including -l options.
// Command_line
//   Everything we get from the command line -- the General_options
//   plus the Input_arguments.
//
// There are also some smaller classes, such as
// Position_dependent_options which hold a subset of General_options
// that change as options are parsed (as opposed to the usual behavior
// of the last instance of that option specified on the commandline wins).

#ifndef GOLD_OPTIONS_H
#define GOLD_OPTIONS_H

#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "elfcpp.h"
#include "script.h"

namespace gold
{

class Command_line;
class General_options;
class Search_directory;
class Input_file_group;
class Input_file_lib;
class Position_dependent_options;
class Target;
class Plugin_manager;
class Script_info;

// Incremental build action for a specific file, as selected by the user.

enum Incremental_disposition
{
  // Startup files that appear before the first disposition option.
  // These will default to INCREMENTAL_CHECK unless the
  // --incremental-startup-unchanged option is given.
  // (For files added implicitly by gcc before any user options.)
  INCREMENTAL_STARTUP,
  // Determine the status from the timestamp (default).
  INCREMENTAL_CHECK,
  // Assume the file changed from the previous build.
  INCREMENTAL_CHANGED,
  // Assume the file didn't change from the previous build.
  INCREMENTAL_UNCHANGED
};

// The nested namespace is to contain all the global variables and
// structs that need to be defined in the .h file, but do not need to
// be used outside this class.
namespace options
{
typedef std::vector<Search_directory> Dir_list;
typedef Unordered_set<std::string> String_set;

// These routines convert from a string option to various types.
// Each gives a fatal error if it cannot parse the argument.

extern void
parse_bool(const char* option_name, const char* arg, bool* retval);

extern void
parse_int(const char* option_name, const char* arg, int* retval);

extern void
parse_uint(const char* option_name, const char* arg, int* retval);

extern void
parse_uint64(const char* option_name, const char* arg, uint64_t* retval);

extern void
parse_double(const char* option_name, const char* arg, double* retval);

extern void
parse_percent(const char* option_name, const char* arg, double* retval);

extern void
parse_string(const char* option_name, const char* arg, const char** retval);

extern void
parse_optional_string(const char* option_name, const char* arg,
		      const char** retval);

extern void
parse_dirlist(const char* option_name, const char* arg, Dir_list* retval);

extern void
parse_set(const char* option_name, const char* arg, String_set* retval);

extern void
parse_choices(const char* option_name, const char* arg, const char** retval,
	      const char* choices[], int num_choices);

struct Struct_var;

// Most options have both a shortname (one letter) and a longname.
// This enum controls how many dashes are expected for longname access
// -- shortnames always use one dash.  Most longnames will accept
// either one dash or two; the only difference between ONE_DASH and
// TWO_DASHES is how we print the option in --help.  However, some
// longnames require two dashes, and some require only one.  The
// special value DASH_Z means that the option is preceded by "-z".
enum Dashes
{
  ONE_DASH, TWO_DASHES, EXACTLY_ONE_DASH, EXACTLY_TWO_DASHES, DASH_Z
};

// LONGNAME is the long-name of the option with dashes converted to
//    underscores, or else the short-name if the option has no long-name.
//    It is never the empty string.
// DASHES is an instance of the Dashes enum: ONE_DASH, TWO_DASHES, etc.
// SHORTNAME is the short-name of the option, as a char, or '\0' if the
//    option has no short-name.  If the option has no long-name, you
//    should specify the short-name in *both* VARNAME and here.
// DEFAULT_VALUE is the value of the option if not specified on the
//    commandline, as a string.
// HELPSTRING is the descriptive text used with the option via --help
// HELPARG is how you define the argument to the option.
//    --help output is "-shortname HELPARG, --longname HELPARG: HELPSTRING"
//    HELPARG should be NULL iff the option is a bool and takes no arg.
// OPTIONAL_ARG is true if this option takes an optional argument.  An
//    optional argument must be specifid as --OPTION=VALUE, not
//    --OPTION VALUE.
// READER provides parse_to_value, which is a function that will convert
//    a char* argument into the proper type and store it in some variable.
// IS_DEFAULT is true for boolean options that are on by default,
//    and thus should have "(default)" printed with the helpstring.
// A One_option struct initializes itself with the global list of options
// at constructor time, so be careful making one of these.
struct One_option
{
  std::string longname;
  Dashes dashes;
  char shortname;
  const char* default_value;
  const char* helpstring;
  const char* helparg;
  bool optional_arg;
  Struct_var* reader;
  bool is_default;

  One_option(const char* ln, Dashes d, char sn, const char* dv,
	     const char* hs, const char* ha, bool oa, Struct_var* r,
	     bool id)
    : longname(ln), dashes(d), shortname(sn), default_value(dv ? dv : ""),
      helpstring(hs), helparg(ha), optional_arg(oa), reader(r),
      is_default(id)
  {
    // In longname, we convert all underscores to dashes, since GNU
    // style uses dashes in option names.  longname is likely to have
    // underscores in it because it's also used to declare a C++
    // function.
    const char* pos = strchr(this->longname.c_str(), '_');
    for (; pos; pos = strchr(pos, '_'))
      this->longname[pos - this->longname.c_str()] = '-';

    // We only register ourselves if our helpstring is not NULL.  This
    // is to support the "no-VAR" boolean variables, which we
    // conditionally turn on by defining "no-VAR" help text.
    if (this->helpstring)
      this->register_option();
  }

  // This option takes an argument iff helparg is not NULL.
  bool
  takes_argument() const
  { return this->helparg != NULL; }

  // Whether the argument is optional.
  bool
  takes_optional_argument() const
  { return this->optional_arg; }

  // Register this option with the global list of options.
  void
  register_option();

  // Print this option to stdout (used with --help).
  void
  print() const;
};

// All options have a Struct_##varname that inherits from this and
// actually implements parse_to_value for that option.
struct Struct_var
{
  // OPTION: the name of the option as specified on the commandline,
  //    including leading dashes, and any text following the option:
  //    "-O", "--defsym=mysym=0x1000", etc.
  // ARG: the arg associated with this option, or NULL if the option
  //    takes no argument: "2", "mysym=0x1000", etc.
  // CMDLINE: the global Command_line object.  Used by DEFINE_special.
  // OPTIONS: the global General_options object.  Used by DEFINE_special.
  virtual void
  parse_to_value(const char* option, const char* arg,
		 Command_line* cmdline, General_options* options) = 0;
  virtual
  ~Struct_var()  // To make gcc happy.
  { }
};

// This is for "special" options that aren't of any predefined type.
struct Struct_special : public Struct_var
{
  // If you change this, change the parse-fn in DEFINE_special as well.
  typedef void (General_options::*Parse_function)(const char*, const char*,
						  Command_line*);
  Struct_special(const char* varname, Dashes dashes, char shortname,
		 Parse_function parse_function,
		 const char* helpstring, const char* helparg)
    : option(varname, dashes, shortname, "", helpstring, helparg, false, this,
	     false),
      parse(parse_function)
  { }

  void parse_to_value(const char* option, const char* arg,
		      Command_line* cmdline, General_options* options)
  { (options->*(this->parse))(option, arg, cmdline); }

  One_option option;
  Parse_function parse;
};

}  // End namespace options.


// These are helper macros use by DEFINE_uint64/etc below.
// This macro is used inside the General_options_ class, so defines
// var() and set_var() as General_options methods.  Arguments as are
// for the constructor for One_option.  param_type__ is the same as
// type__ for built-in types, and "const type__ &" otherwise.
//
// When we define the linker command option "assert", the macro argument
// varname__ of DEFINE_var below will be replaced by "assert".  On Mac OSX
// assert.h is included implicitly by one of the library headers we use.  To
// avoid unintended macro substitution of "assert()", we need to enclose
// varname__ with parenthese.
#define DEFINE_var(varname__, dashes__, shortname__, default_value__,        \
		   default_value_as_string__, helpstring__, helparg__,       \
		   optional_arg__, type__, param_type__, parse_fn__,         \
		   is_default__)					     \
 public:                                                                     \
  param_type__                                                               \
  (varname__)() const                                                        \
  { return this->varname__##_.value; }                                       \
									     \
  bool                                                                       \
  user_set_##varname__() const                                               \
  { return this->varname__##_.user_set_via_option; }                         \
									     \
  void									     \
  set_user_set_##varname__()						     \
  { this->varname__##_.user_set_via_option = true; }			     \
									     \
  static const bool varname__##is_default = is_default__;		     \
									     \
 private:                                                                    \
  struct Struct_##varname__ : public options::Struct_var                     \
  {                                                                          \
    Struct_##varname__()                                                     \
      : option(#varname__, dashes__, shortname__, default_value_as_string__, \
	       helpstring__, helparg__, optional_arg__, this, is_default__), \
	user_set_via_option(false), value(default_value__)                   \
    { }                                                                      \
									     \
    void                                                                     \
    parse_to_value(const char* option_name, const char* arg,                 \
		   Command_line*, General_options*)                          \
    {                                                                        \
      parse_fn__(option_name, arg, &this->value);                            \
      this->user_set_via_option = true;                                      \
    }                                                                        \
									     \
    options::One_option option;                                              \
    bool user_set_via_option;                                                \
    type__ value;                                                            \
  };                                                                         \
  Struct_##varname__ varname__##_;                                           \
  void                                                                       \
  set_##varname__(param_type__ value)                                        \
  { this->varname__##_.value = value; }

// These macros allow for easy addition of a new commandline option.

// If no_helpstring__ is not NULL, then in addition to creating
// VARNAME, we also create an option called no-VARNAME (or, for a -z
// option, noVARNAME).
#define DEFINE_bool(varname__, dashes__, shortname__, default_value__,   \
		    helpstring__, no_helpstring__)                       \
  DEFINE_var(varname__, dashes__, shortname__, default_value__,          \
	     default_value__ ? "true" : "false", helpstring__, NULL,     \
	     false, bool, bool, options::parse_bool, default_value__)	 \
  struct Struct_no_##varname__ : public options::Struct_var              \
  {                                                                      \
    Struct_no_##varname__() : option((dashes__ == options::DASH_Z	 \
				      ? "no" #varname__			 \
				      : "no-" #varname__),		 \
				     dashes__, '\0',			 \
				     default_value__ ? "false" : "true", \
				     no_helpstring__, NULL, false, this, \
				     !(default_value__))                 \
    { }                                                                  \
									 \
    void                                                                 \
    parse_to_value(const char*, const char*,                             \
		   Command_line*, General_options* options)              \
    {                                                                    \
      options->set_##varname__(false);                                   \
      options->set_user_set_##varname__();                               \
    }                                                                    \
									 \
    options::One_option option;                                          \
  };                                                                     \
  Struct_no_##varname__ no_##varname__##_initializer_

#define DEFINE_bool_ignore(varname__, dashes__, shortname__,		 \
		    helpstring__, no_helpstring__)                       \
  DEFINE_var(varname__, dashes__, shortname__, false,			 \
	     "false", helpstring__, NULL,				 \
	     false, bool, bool, options::parse_bool, false)		 \
  struct Struct_no_##varname__ : public options::Struct_var              \
  {                                                                      \
    Struct_no_##varname__() : option((dashes__ == options::DASH_Z	 \
				      ? "no" #varname__			 \
				      : "no-" #varname__),		 \
				     dashes__, '\0',			 \
				     "false",				 \
				     no_helpstring__, NULL, false, this, \
				     false)				 \
    { }                                                                  \
									 \
    void                                                                 \
    parse_to_value(const char*, const char*,                             \
		   Command_line*, General_options* options)              \
    {                                                                    \
      options->set_##varname__(false);                                   \
      options->set_user_set_##varname__();                               \
    }                                                                    \
									 \
    options::One_option option;                                          \
  };                                                                     \
  Struct_no_##varname__ no_##varname__##_initializer_

#define DEFINE_enable(varname__, dashes__, shortname__, default_value__, \
		      helpstring__, no_helpstring__)                     \
  DEFINE_var(enable_##varname__, dashes__, shortname__, default_value__, \
	     default_value__ ? "true" : "false", helpstring__, NULL,     \
	     false, bool, bool, options::parse_bool, default_value__)	 \
  struct Struct_disable_##varname__ : public options::Struct_var         \
  {                                                                      \
    Struct_disable_##varname__() : option("disable-" #varname__,         \
				     dashes__, '\0',                     \
				     default_value__ ? "false" : "true", \
				     no_helpstring__, NULL, false, this, \
				     !default_value__)                   \
    { }                                                                  \
									 \
    void                                                                 \
    parse_to_value(const char*, const char*,                             \
		   Command_line*, General_options* options)              \
    { options->set_enable_##varname__(false); }                          \
									 \
    options::One_option option;                                          \
  };                                                                     \
  Struct_disable_##varname__ disable_##varname__##_initializer_

#define DEFINE_int(varname__, dashes__, shortname__, default_value__,   \
		   helpstring__, helparg__)                             \
  DEFINE_var(varname__, dashes__, shortname__, default_value__,         \
	     #default_value__, helpstring__, helparg__, false,		\
	     int, int, options::parse_int, false)

#define DEFINE_uint(varname__, dashes__, shortname__, default_value__,  \
		   helpstring__, helparg__)                             \
  DEFINE_var(varname__, dashes__, shortname__, default_value__,         \
	     #default_value__, helpstring__, helparg__, false,		\
	     int, int, options::parse_uint, false)

#define DEFINE_uint64(varname__, dashes__, shortname__, default_value__, \
		      helpstring__, helparg__)                           \
  DEFINE_var(varname__, dashes__, shortname__, default_value__,          \
	     #default_value__, helpstring__, helparg__, false,		 \
	     uint64_t, uint64_t, options::parse_uint64, false)

#define DEFINE_double(varname__, dashes__, shortname__, default_value__, \
		      helpstring__, helparg__)				 \
  DEFINE_var(varname__, dashes__, shortname__, default_value__,		 \
	     #default_value__, helpstring__, helparg__, false,		 \
	     double, double, options::parse_double, false)

#define DEFINE_percent(varname__, dashes__, shortname__, default_value__, \
		       helpstring__, helparg__)				  \
  DEFINE_var(varname__, dashes__, shortname__, default_value__ / 100.0,	  \
	     #default_value__, helpstring__, helparg__, false,		  \
	     double, double, options::parse_percent, false)

#define DEFINE_string(varname__, dashes__, shortname__, default_value__, \
		      helpstring__, helparg__)                           \
  DEFINE_var(varname__, dashes__, shortname__, default_value__,          \
	     default_value__, helpstring__, helparg__, false,		 \
	     const char*, const char*, options::parse_string, false)

// This is like DEFINE_string, but we convert each occurrence to a
// Search_directory and store it in a vector.  Thus we also have the
// add_to_VARNAME() method, to append to the vector.
#define DEFINE_dirlist(varname__, dashes__, shortname__,                  \
			   helpstring__, helparg__)                       \
  DEFINE_var(varname__, dashes__, shortname__, ,                          \
	     "", helpstring__, helparg__, false, options::Dir_list,	  \
	     const options::Dir_list&, options::parse_dirlist, false)     \
  void                                                                    \
  add_to_##varname__(const char* new_value)                               \
  { options::parse_dirlist(NULL, new_value, &this->varname__##_.value); } \
  void                                                                    \
  add_search_directory_to_##varname__(const Search_directory& dir)        \
  { this->varname__##_.value.push_back(dir); }

// This is like DEFINE_string, but we store a set of strings.
#define DEFINE_set(varname__, dashes__, shortname__,                      \
		   helpstring__, helparg__)                               \
  DEFINE_var(varname__, dashes__, shortname__, ,                          \
	     "", helpstring__, helparg__, false, options::String_set,     \
	     const options::String_set&, options::parse_set, false)       \
 public:                                                                  \
  bool                                                                    \
  any_##varname__() const                                                 \
  { return !this->varname__##_.value.empty(); }                           \
									  \
  bool                                                                    \
  is_##varname__(const char* symbol) const                                \
  {                                                                       \
    return (!this->varname__##_.value.empty()                             \
	    && (this->varname__##_.value.find(std::string(symbol))        \
		!= this->varname__##_.value.end()));                      \
  }									  \
									  \
  options::String_set::const_iterator					  \
  varname__##_begin() const						  \
  { return this->varname__##_.value.begin(); }				  \
									  \
  options::String_set::const_iterator					  \
  varname__##_end() const						  \
  { return this->varname__##_.value.end(); }                              \
									  \
  options::String_set::size_type                                          \
  varname__##_size() const                                                \
  { return this->varname__##_.value.size(); }                             \

// When you have a list of possible values (expressed as string)
// After helparg__ should come an initializer list, like
//   {"foo", "bar", "baz"}
#define DEFINE_enum(varname__, dashes__, shortname__, default_value__,   \
		    helpstring__, helparg__, optional_arg__, ...)        \
  DEFINE_var(varname__, dashes__, shortname__, default_value__,          \
	     default_value__, helpstring__, helparg__, optional_arg__,   \
	     const char*, const char*, parse_choices_##varname__, false) \
 private:                                                                \
  static void parse_choices_##varname__(const char* option_name,         \
					const char* arg,                 \
					const char** retval) {           \
    const char* choices[] = __VA_ARGS__;                                 \
    options::parse_choices(option_name, arg, retval,                     \
			   choices, sizeof(choices) / sizeof(*choices)); \
  }

// This is like DEFINE_bool, but VARNAME is the name of a different
// option.  This option becomes an alias for that one.  INVERT is true
// if this option is an inversion of the other one.
#define DEFINE_bool_alias(option__, varname__, dashes__, shortname__,	\
			  helpstring__, no_helpstring__, invert__)	\
 private:								\
  struct Struct_##option__ : public options::Struct_var			\
  {									\
    Struct_##option__()							\
      : option(#option__, dashes__, shortname__, "", helpstring__,	\
	       NULL, false, this,					\
	       General_options::varname__##is_default ^ invert__)	\
    { }									\
									\
    void								\
    parse_to_value(const char*, const char*,				\
		   Command_line*, General_options* options)		\
    {									\
      options->set_##varname__(!invert__);				\
      options->set_user_set_##varname__();				\
    }									\
									\
    options::One_option option;						\
  };									\
  Struct_##option__ option__##_;					\
									\
  struct Struct_no_##option__ : public options::Struct_var		\
  {									\
    Struct_no_##option__()						\
      : option((dashes__ == options::DASH_Z				\
		? "no" #option__					\
		: "no-" #option__),					\
	       dashes__, '\0', "", no_helpstring__,			\
	       NULL, false, this,					\
	       !General_options::varname__##is_default ^ invert__)	\
    { }									\
									\
    void								\
    parse_to_value(const char*, const char*,				\
		   Command_line*, General_options* options)		\
    {									\
      options->set_##varname__(invert__);				\
      options->set_user_set_##varname__();				\
    }									\
									\
    options::One_option option;						\
  };									\
  Struct_no_##option__ no_##option__##_initializer_

// This is like DEFINE_uint64, but VARNAME is the name of a different
// option.  This option becomes an alias for that one.
#define DEFINE_uint64_alias(option__, varname__, dashes__, shortname__,	\
			    helpstring__, helparg__)			\
 private:								\
  struct Struct_##option__ : public options::Struct_var			\
  {									\
    Struct_##option__()							\
      : option(#option__, dashes__, shortname__, "", helpstring__,	\
	       helparg__, false, this, false)				\
    { }									\
									\
    void								\
    parse_to_value(const char* option_name, const char* arg,		\
		   Command_line*, General_options* options)		\
    {									\
      uint64_t value;							\
      options::parse_uint64(option_name, arg, &value);			\
      options->set_##varname__(value);					\
      options->set_user_set_##varname__();				\
    }									\
									\
    options::One_option option;						\
  };									\
  Struct_##option__ option__##_;

// This is used for non-standard flags.  It defines no functions; it
// just calls General_options::parse_VARNAME whenever the flag is
// seen.  We declare parse_VARNAME as a static member of
// General_options; you are responsible for defining it there.
// helparg__ should be NULL iff this special-option is a boolean.
#define DEFINE_special(varname__, dashes__, shortname__,                \
		       helpstring__, helparg__)                         \
 private:                                                               \
  void parse_##varname__(const char* option, const char* arg,           \
			 Command_line* inputs);                         \
  struct Struct_##varname__ : public options::Struct_special            \
  {                                                                     \
    Struct_##varname__()                                                \
      : options::Struct_special(#varname__, dashes__, shortname__,      \
				&General_options::parse_##varname__,    \
				helpstring__, helparg__)                \
    { }                                                                 \
  };                                                                    \
  Struct_##varname__ varname__##_initializer_

// An option that takes an optional string argument.  If the option is
// used with no argument, the value will be the default, and
// user_set_via_option will be true.
#define DEFINE_optional_string(varname__, dashes__, shortname__,	\
			       default_value__,				\
			       helpstring__, helparg__)			\
  DEFINE_var(varname__, dashes__, shortname__, default_value__,		\
	     default_value__, helpstring__, helparg__, true,		\
	     const char*, const char*, options::parse_optional_string,  \
	     false)

// A directory to search.  For each directory we record whether it is
// in the sysroot.  We need to know this so that, if a linker script
// is found within the sysroot, we will apply the sysroot to any files
// named by that script.

class Search_directory
{
 public:
  // We need a default constructor because we put this in a
  // std::vector.
  Search_directory()
    : name_(), put_in_sysroot_(false), is_in_sysroot_(false)
  { }

  // This is the usual constructor.
  Search_directory(const std::string& name, bool put_in_sysroot)
    : name_(name), put_in_sysroot_(put_in_sysroot), is_in_sysroot_(false)
  {
    if (this->name_.empty())
      this->name_ = ".";
  }

  // This is called if we have a sysroot.  The sysroot is prefixed to
  // any entries for which put_in_sysroot_ is true.  is_in_sysroot_ is
  // set to true for any enries which are in the sysroot (this will
  // naturally include any entries for which put_in_sysroot_ is true).
  // SYSROOT is the sysroot, CANONICAL_SYSROOT is the result of
  // passing SYSROOT to lrealpath.
  void
  add_sysroot(const char* sysroot, const char* canonical_sysroot);

  // Get the directory name.
  const std::string&
  name() const
  { return this->name_; }

  // Return whether this directory is in the sysroot.
  bool
  is_in_sysroot() const
  { return this->is_in_sysroot_; }

  // Return whether this is considered a system directory.
  bool
  is_system_directory() const
  { return this->put_in_sysroot_ || this->is_in_sysroot_; }

 private:
  // The directory name.
  std::string name_;
  // True if the sysroot should be added as a prefix for this
  // directory (if there is a sysroot).  This is true for system
  // directories that we search by default.
  bool put_in_sysroot_;
  // True if this directory is in the sysroot (if there is a sysroot).
  // This is true if there is a sysroot and either 1) put_in_sysroot_
  // is true, or 2) the directory happens to be in the sysroot based
  // on a pathname comparison.
  bool is_in_sysroot_;
};

class General_options
{
 private:
  // NOTE: For every option that you add here, also consider if you
  // should add it to Position_dependent_options.
  DEFINE_special(help, options::TWO_DASHES, '\0',
		 N_("Report usage information"), NULL);
  DEFINE_special(version, options::TWO_DASHES, 'v',
		 N_("Report version information"), NULL);
  DEFINE_special(V, options::EXACTLY_ONE_DASH, '\0',
		 N_("Report version and target information"), NULL);

  // These options are sorted approximately so that for each letter in
  // the alphabet, we show the option whose shortname is that letter
  // (if any) and then every longname that starts with that letter (in
  // alphabetical order).  For both, lowercase sorts before uppercase.
  // The -z options come last.

  // a

  DEFINE_bool(add_needed, options::TWO_DASHES, '\0', false,
	      N_("Not supported"),
	      N_("Do not copy DT_NEEDED tags from shared libraries"));

  DEFINE_bool_alias(allow_multiple_definition, muldefs, options::TWO_DASHES,
		    '\0',
		    N_("Allow multiple definitions of symbols"),
		    N_("Do not allow multiple definitions"), false);

  DEFINE_bool(allow_shlib_undefined, options::TWO_DASHES, '\0', false,
	      N_("Allow unresolved references in shared libraries"),
	      N_("Do not allow unresolved references in shared libraries"));

  DEFINE_bool(apply_dynamic_relocs, options::TWO_DASHES, '\0', true,
	      N_("Apply link-time values for dynamic relocations"),
	      N_("(aarch64 only) Do not apply link-time values "
		 "for dynamic relocations"));

  DEFINE_bool(as_needed, options::TWO_DASHES, '\0', false,
	      N_("Use DT_NEEDED only for shared libraries that are used"),
	      N_("Use DT_NEEDED for all shared libraries"));

  DEFINE_enum(assert, options::ONE_DASH, '\0', NULL,
	      N_("Ignored"), N_("[ignored]"), false,
	      {"definitions", "nodefinitions", "nosymbolic", "pure-text"});

  // b

  // This should really be an "enum", but it's too easy for folks to
  // forget to update the list as they add new targets.  So we just
  // accept any string.  We'll fail later (when the string is parsed),
  // if the target isn't actually supported.
  DEFINE_string(format, options::TWO_DASHES, 'b', "elf",
		N_("Set input format"), ("[elf,binary]"));

  DEFINE_bool(be8, options::TWO_DASHES, '\0', false,
	      N_("Output BE8 format image"), NULL);

  DEFINE_optional_string(build_id, options::TWO_DASHES, '\0', "tree",
			 N_("Generate build ID note"),
			 N_("[=STYLE]"));

  DEFINE_uint64(build_id_chunk_size_for_treehash,
		options::TWO_DASHES, '\0', 2 << 20,
		N_("Chunk size for '--build-id=tree'"), N_("SIZE"));

  DEFINE_uint64(build_id_min_file_size_for_treehash, options::TWO_DASHES,
		'\0', 40 << 20,
		N_("Minimum output file size for '--build-id=tree' to work"
		   " differently than '--build-id=sha1'"), N_("SIZE"));

  DEFINE_bool(Bdynamic, options::ONE_DASH, '\0', true,
	      N_("-l searches for shared libraries"), NULL);
  DEFINE_bool_alias(Bstatic, Bdynamic, options::ONE_DASH, '\0',
		    N_("-l does not search for shared libraries"), NULL,
		    true);
  DEFINE_bool_alias(dy, Bdynamic, options::ONE_DASH, '\0',
		    N_("alias for -Bdynamic"), NULL, false);
  DEFINE_bool_alias(dn, Bdynamic, options::ONE_DASH, '\0',
		    N_("alias for -Bstatic"), NULL, true);

  DEFINE_bool(Bgroup, options::ONE_DASH, '\0', false,
	      N_("Use group name lookup rules for shared library"), NULL);

  DEFINE_bool(Bshareable, options::ONE_DASH, '\0', false,
	      N_("Generate shared library (alias for -G/-shared)"), NULL);

  DEFINE_special (Bno_symbolic, options::ONE_DASH, '\0',
		  N_ ("Don't bind default visibility defined symbols locally "
		      "for -shared (default)"),
		  NULL);

  DEFINE_special (Bsymbolic_functions, options::ONE_DASH, '\0',
		  N_ ("Bind default visibility defined function symbols "
		      "locally for -shared"),
		  NULL);

  DEFINE_special (
      Bsymbolic, options::ONE_DASH, '\0',
      N_ ("Bind default visibility defined symbols locally for -shared"),
      NULL);

  // c

  DEFINE_bool(check_sections, options::TWO_DASHES, '\0', true,
	      N_("Check segment addresses for overlaps"),
	      N_("Do not check segment addresses for overlaps"));

  DEFINE_enum(compress_debug_sections, options::TWO_DASHES, '\0', "none",
	      N_("Compress .debug_* sections in the output file"),
	      ("[none,zlib,zlib-gnu,zlib-gabi,zstd]"), false,
	      {"none", "zlib", "zlib-gnu", "zlib-gabi", "zstd"});

  DEFINE_bool(copy_dt_needed_entries, options::TWO_DASHES, '\0', false,
	      N_("Not supported"),
	      N_("Do not copy DT_NEEDED tags from shared libraries"));

  DEFINE_bool(cref, options::TWO_DASHES, '\0', false,
	      N_("Output cross reference table"),
	      N_("Do not output cross reference table"));

  DEFINE_bool(ctors_in_init_array, options::TWO_DASHES, '\0', true,
	      N_("Use DT_INIT_ARRAY for all constructors"),
	      N_("Handle constructors as directed by compiler"));

  // d

  DEFINE_bool(define_common, options::TWO_DASHES, 'd', false,
	      N_("Define common symbols"),
	      N_("Do not define common symbols in relocatable output"));
  DEFINE_bool(dc, options::ONE_DASH, '\0', false,
	      N_("Alias for -d"), NULL);
  DEFINE_bool(dp, options::ONE_DASH, '\0', false,
	      N_("Alias for -d"), NULL);

  DEFINE_string(debug, options::TWO_DASHES, '\0', "",
		N_("Turn on debugging"),
		N_("[all,files,script,task][,...]"));

  DEFINE_special(defsym, options::TWO_DASHES, '\0',
		 N_("Define a symbol"), N_("SYMBOL=EXPRESSION"));

  DEFINE_optional_string(demangle, options::TWO_DASHES, '\0', NULL,
			 N_("Demangle C++ symbols in log messages"),
			 N_("[=STYLE]"));
  DEFINE_bool(no_demangle, options::TWO_DASHES, '\0', false,
	      N_("Do not demangle C++ symbols in log messages"),
	      NULL);

  DEFINE_string(dependency_file, options::TWO_DASHES, '\0', NULL,
		N_("Write a dependency file listing all files read"),
		N_("FILE"));

  DEFINE_bool(detect_odr_violations, options::TWO_DASHES, '\0', false,
	      N_("Look for violations of the C++ One Definition Rule"),
	      N_("Do not look for violations of the C++ One Definition Rule"));

  DEFINE_bool(dynamic_list_data, options::TWO_DASHES, '\0', false,
	      N_("Add data symbols to dynamic symbols"), NULL);

  DEFINE_bool(dynamic_list_cpp_new, options::TWO_DASHES, '\0', false,
	      N_("Add C++ operator new/delete to dynamic symbols"), NULL);

  DEFINE_bool(dynamic_list_cpp_typeinfo, options::TWO_DASHES, '\0', false,
	      N_("Add C++ typeinfo to dynamic symbols"), NULL);

  DEFINE_special(dynamic_list, options::TWO_DASHES, '\0',
		 N_("Read a list of dynamic symbols"), N_("FILE"));

  // e

  DEFINE_bool(emit_stub_syms, options::TWO_DASHES, '\0', true,
	      N_("(PowerPC only) Label linker stubs with a symbol"),
	      N_("(PowerPC only) Do not label linker stubs with a symbol"));

  DEFINE_string(entry, options::TWO_DASHES, 'e', NULL,
		N_("Set program start address"), N_("ADDRESS"));

  DEFINE_bool(eh_frame_hdr, options::TWO_DASHES, '\0', false,
	      N_("Create exception frame header"),
	      N_("Do not create exception frame header"));

  // Alphabetized under 'e' because the option is spelled --enable-new-dtags.
  DEFINE_enable(new_dtags, options::EXACTLY_TWO_DASHES, '\0', true,
		N_("Enable use of DT_RUNPATH"),
		N_("Disable use of DT_RUNPATH"));

  DEFINE_enable(linker_version, options::EXACTLY_TWO_DASHES, '\0', false,
		N_("Put the linker version string into the .comment section"),
		N_("Put the linker version string into the .note.gnu.gold-version section"));

  DEFINE_bool(enum_size_warning, options::TWO_DASHES, '\0', true, NULL,
	      N_("(ARM only) Do not warn about objects with incompatible "
		 "enum sizes"));

  DEFINE_special(exclude_libs, options::TWO_DASHES, '\0',
		 N_("Exclude libraries from automatic export"),
		 N_(("lib,lib ...")));

  DEFINE_bool(export_dynamic, options::TWO_DASHES, 'E', false,
	      N_("Export all dynamic symbols"),
	      N_("Do not export all dynamic symbols"));

  DEFINE_set(export_dynamic_symbol, options::TWO_DASHES, '\0',
	     N_("Export SYMBOL to dynamic symbol table"), N_("SYMBOL"));

  DEFINE_special(EB, options::ONE_DASH, '\0',
		 N_("Link big-endian objects."), NULL);
  DEFINE_special(EL, options::ONE_DASH, '\0',
		 N_("Link little-endian objects."), NULL);

  // f

  DEFINE_set(auxiliary, options::TWO_DASHES, 'f',
	     N_("Auxiliary filter for shared object symbol table"),
	     N_("SHLIB"));

  DEFINE_string(filter, options::TWO_DASHES, 'F', NULL,
		N_("Filter for shared object symbol table"),
		N_("SHLIB"));

  DEFINE_bool(fatal_warnings, options::TWO_DASHES, '\0', false,
	      N_("Treat warnings as errors"),
	      N_("Do not treat warnings as errors"));

  DEFINE_string(fini, options::ONE_DASH, '\0', "_fini",
		N_("Call SYMBOL at unload-time"), N_("SYMBOL"));

  DEFINE_bool(fix_arm1176, options::TWO_DASHES, '\0', true,
	      N_("(ARM only) Fix binaries for ARM1176 erratum"),
	      N_("(ARM only) Do not fix binaries for ARM1176 erratum"));

  DEFINE_bool(fix_cortex_a8, options::TWO_DASHES, '\0', false,
	      N_("(ARM only) Fix binaries for Cortex-A8 erratum"),
	      N_("(ARM only) Do not fix binaries for Cortex-A8 erratum"));

  DEFINE_bool(fix_cortex_a53_843419, options::TWO_DASHES, '\0', false,
	      N_("(AArch64 only) Fix Cortex-A53 erratum 843419"),
	      N_("(AArch64 only) Do not fix Cortex-A53 erratum 843419"));

  DEFINE_bool(fix_cortex_a53_835769, options::TWO_DASHES, '\0', false,
	      N_("(AArch64 only) Fix Cortex-A53 erratum 835769"),
	      N_("(AArch64 only) Do not fix Cortex-A53 erratum 835769"));

  DEFINE_special(fix_v4bx, options::TWO_DASHES, '\0',
		 N_("(ARM only) Rewrite BX rn as MOV pc, rn for ARMv4"),
		 NULL);

  DEFINE_special(fix_v4bx_interworking, options::TWO_DASHES, '\0',
		 N_("(ARM only) Rewrite BX rn branch to ARMv4 interworking "
		    "veneer"),
		 NULL);

  DEFINE_string(fuse_ld, options::ONE_DASH, '\0', "",
		N_("Ignored for GCC linker option compatibility"),
		N_("[gold,bfd]"));

  // g

  DEFINE_bool(g, options::EXACTLY_ONE_DASH, '\0', false,
	      N_("Ignored"), NULL);

  DEFINE_bool(gc_sections, options::TWO_DASHES, '\0', false,
	      N_("Remove unused sections"),
	      N_("Don't remove unused sections"));

  DEFINE_bool(gdb_index, options::TWO_DASHES, '\0', false,
	      N_("Generate .gdb_index section"),
	      N_("Do not generate .gdb_index section"));

  DEFINE_bool(gnu_unique, options::TWO_DASHES, '\0', true,
	      N_("Enable STB_GNU_UNIQUE symbol binding"),
	      N_("Disable STB_GNU_UNIQUE symbol binding"));

  DEFINE_bool(shared, options::ONE_DASH, 'G', false,
	      N_("Generate shared library"), NULL);

  // h

  DEFINE_string(soname, options::ONE_DASH, 'h', NULL,
		N_("Set shared library name"), N_("FILENAME"));

  DEFINE_double(hash_bucket_empty_fraction, options::TWO_DASHES, '\0', 0.0,
		N_("Min fraction of empty buckets in dynamic hash"),
		N_("FRACTION"));

  DEFINE_enum(hash_style, options::TWO_DASHES, '\0', DEFAULT_HASH_STYLE,
	      N_("Dynamic hash style"), N_("[sysv,gnu,both]"), false,
	      {"sysv", "gnu", "both"});

  // i

  DEFINE_bool_alias(i, relocatable, options::EXACTLY_ONE_DASH, '\0',
		    N_("Alias for -r"), NULL, false);

  DEFINE_enum(icf, options::TWO_DASHES, '\0', "none",
	      N_("Identical Code Folding. "
		 "\'--icf=safe\' Folds ctors, dtors and functions whose"
		 " pointers are definitely not taken"),
	      ("[none,all,safe]"), false,
	      {"none", "all", "safe"});

  DEFINE_uint(icf_iterations, options::TWO_DASHES , '\0', 0,
	      N_("Number of iterations of ICF (default 3)"), N_("COUNT"));

  DEFINE_special(incremental, options::TWO_DASHES, '\0',
		 N_("Do an incremental link if possible; "
		    "otherwise, do a full link and prepare output "
		    "for incremental linking"), NULL);

  DEFINE_special(no_incremental, options::TWO_DASHES, '\0',
		 N_("Do a full link (default)"), NULL);

  DEFINE_special(incremental_full, options::TWO_DASHES, '\0',
		 N_("Do a full link and "
		    "prepare output for incremental linking"), NULL);

  DEFINE_special(incremental_update, options::TWO_DASHES, '\0',
		 N_("Do an incremental link; exit if not possible"), NULL);

  DEFINE_string(incremental_base, options::TWO_DASHES, '\0', NULL,
		N_("Set base file for incremental linking"
		   " (default is output file)"),
		N_("FILE"));

  DEFINE_special(incremental_changed, options::TWO_DASHES, '\0',
		 N_("Assume files changed"), NULL);

  DEFINE_special(incremental_unchanged, options::TWO_DASHES, '\0',
		 N_("Assume files didn't change"), NULL);

  DEFINE_special(incremental_unknown, options::TWO_DASHES, '\0',
		 N_("Use timestamps to check files (default)"), NULL);

  DEFINE_special(incremental_startup_unchanged, options::TWO_DASHES, '\0',
		 N_("Assume startup files unchanged "
		    "(files preceding this option)"), NULL);

  DEFINE_percent(incremental_patch, options::TWO_DASHES, '\0', 10,
		 N_("Amount of extra space to allocate for patches "
		    "(default 10)"),
		 N_("PERCENT"));

  DEFINE_string(init, options::ONE_DASH, '\0', "_init",
		N_("Call SYMBOL at load-time"), N_("SYMBOL"));

  DEFINE_string(dynamic_linker, options::TWO_DASHES, 'I', NULL,
		N_("Set dynamic linker path"), N_("PROGRAM"));

  // j

  DEFINE_special(just_symbols, options::TWO_DASHES, '\0',
		 N_("Read only symbol values from FILE"), N_("FILE"));

  // k

  DEFINE_bool(keep_files_mapped, options::TWO_DASHES, '\0', true,
	      N_("Keep files mapped across passes"),
	      N_("Release mapped files after each pass"));

  DEFINE_set(keep_unique, options::TWO_DASHES, '\0',
	     N_("Do not fold this symbol during ICF"), N_("SYMBOL"));

  // l

  DEFINE_special(library, options::TWO_DASHES, 'l',
		 N_("Search for library LIBNAME"), N_("LIBNAME"));

  DEFINE_bool(ld_generated_unwind_info, options::TWO_DASHES, '\0', true,
	      N_("Generate unwind information for PLT"),
	      N_("Do not generate unwind information for PLT"));

  DEFINE_dirlist(library_path, options::TWO_DASHES, 'L',
		 N_("Add directory to search path"), N_("DIR"));

  DEFINE_bool(long_plt, options::TWO_DASHES, '\0', false,
	      N_("(ARM only) Generate long PLT entries"),
	      N_("(ARM only) Do not generate long PLT entries"));

  // m

  DEFINE_string(m, options::EXACTLY_ONE_DASH, 'm', "",
		N_("Set GNU linker emulation; obsolete"), N_("EMULATION"));

  DEFINE_bool(map_whole_files, options::TWO_DASHES, '\0',
	      sizeof(void*) >= 8,
	      N_("Map whole files to memory"),
	      N_("Map relevant file parts to memory"));

  DEFINE_bool(merge_exidx_entries, options::TWO_DASHES, '\0', true,
	      N_("(ARM only) Merge exidx entries in debuginfo"),
	      N_("(ARM only) Do not merge exidx entries in debuginfo"));

  DEFINE_bool(mmap_output_file, options::TWO_DASHES, '\0', true,
	      N_("Map the output file for writing"),
	      N_("Do not map the output file for writing"));

  DEFINE_bool(print_map, options::TWO_DASHES, 'M', false,
	      N_("Write map file on standard output"), NULL);

  DEFINE_string(Map, options::ONE_DASH, '\0', NULL, N_("Write map file"),
		N_("MAPFILENAME"));

  // n

  DEFINE_bool(nmagic, options::TWO_DASHES, 'n', false,
	      N_("Do not page align data"), NULL);
  DEFINE_bool(omagic, options::EXACTLY_TWO_DASHES, 'N', false,
	      N_("Do not page align data, do not make text readonly"),
	      N_("Page align data, make text readonly"));

  DEFINE_bool(no_keep_memory, options::TWO_DASHES, '\0', false,
	      N_("Use less memory and more disk I/O "
		 "(included only for compatibility with GNU ld)"), NULL);

  DEFINE_bool_alias(no_undefined, defs, options::TWO_DASHES, '\0',
		    N_("Report undefined symbols (even with --shared)"),
		    NULL, false);

  DEFINE_bool(noinhibit_exec, options::TWO_DASHES, '\0', false,
	      N_("Create an output file even if errors occur"), NULL);

  DEFINE_bool(nostdlib, options::ONE_DASH, '\0', false,
	      N_("Only search directories specified on the command line"),
	      NULL);

  // o

  DEFINE_string(output, options::TWO_DASHES, 'o', "a.out",
		N_("Set output file name"), N_("FILE"));

  DEFINE_string(oformat, options::EXACTLY_TWO_DASHES, '\0', "elf",
		N_("Set output format"), N_("[binary]"));

  DEFINE_uint(optimize, options::EXACTLY_ONE_DASH, 'O', 0,
	      N_("Optimize output file size"), N_("LEVEL"));

  DEFINE_enum(orphan_handling, options::TWO_DASHES, '\0', "place",
	      N_("Orphan section handling"), N_("[place,discard,warn,error]"),
	      false, {"place", "discard", "warn", "error"});

  // p

  DEFINE_bool(p, options::ONE_DASH, 'p', false,
	      N_("Ignored for ARM compatibility"), NULL);

  DEFINE_optional_string(package_metadata, options::TWO_DASHES, '\0', NULL,
			 N_("Generate package metadata note"),
			 N_("[=JSON]"));

  DEFINE_bool(pie, options::ONE_DASH, '\0', false,
	      N_("Create a position independent executable"),
	      N_("Do not create a position independent executable"));
  DEFINE_bool_alias(pic_executable, pie, options::TWO_DASHES, '\0',
		    N_("Create a position independent executable"),
		    N_("Do not create a position independent executable"),
		    false);

  DEFINE_bool(pic_veneer, options::TWO_DASHES, '\0', false,
	      N_("Force PIC sequences for ARM/Thumb interworking veneers"),
	      NULL);

  DEFINE_bool(pipeline_knowledge, options::ONE_DASH, '\0', false,
	      NULL, N_("(ARM only) Ignore for backward compatibility"));

  DEFINE_var(plt_align, options::TWO_DASHES, '\0', 0, "5",
	     N_("(PowerPC only) Align PLT call stubs to fit cache lines"),
	     N_("[=P2ALIGN]"), true, int, int, options::parse_uint, false);

  DEFINE_bool(plt_localentry, options::TWO_DASHES, '\0', false,
	      N_("(PowerPC64 only) Optimize calls to ELFv2 localentry:0 functions"),
	      N_("(PowerPC64 only) Don't optimize ELFv2 calls"));

  DEFINE_bool(plt_static_chain, options::TWO_DASHES, '\0', false,
	      N_("(PowerPC64 only) PLT call stubs should load r11"),
	      N_("(PowerPC64 only) PLT call stubs should not load r11"));

  DEFINE_bool(plt_thread_safe, options::TWO_DASHES, '\0', false,
	      N_("(PowerPC64 only) PLT call stubs with load-load barrier"),
	      N_("(PowerPC64 only) PLT call stubs without barrier"));

#ifdef ENABLE_PLUGINS
  DEFINE_special(plugin, options::TWO_DASHES, '\0',
		 N_("Load a plugin library"), N_("PLUGIN"));
  DEFINE_special(plugin_opt, options::TWO_DASHES, '\0',
		 N_("Pass an option to the plugin"), N_("OPTION"));
#else
  DEFINE_special(plugin, options::TWO_DASHES, '\0',
		 N_("Load a plugin library (not supported)"), N_("PLUGIN"));
  DEFINE_special(plugin_opt, options::TWO_DASHES, '\0',
		 N_("Pass an option to the plugin (not supported)"),
		 N_("OPTION"));
#endif

  DEFINE_bool(posix_fallocate, options::TWO_DASHES, '\0', true,
	      N_("Use posix_fallocate to reserve space in the output file"),
	      N_("Use fallocate or ftruncate to reserve space"));

  DEFINE_enum(power10_stubs, options::TWO_DASHES, '\0', "yes",
	     N_("(PowerPC64 only) stubs use power10 insns"),
	     N_("[=auto,no,yes]"), true, {"auto", "no", "yes"});
  DEFINE_special(no_power10_stubs, options::TWO_DASHES, '\0',
		 N_("(PowerPC64 only) stubs do not use power10 insns"), NULL);

  DEFINE_bool(preread_archive_symbols, options::TWO_DASHES, '\0', false,
	      N_("Preread archive symbols when multi-threaded"), NULL);

  DEFINE_bool(print_gc_sections, options::TWO_DASHES, '\0', false,
	      N_("List removed unused sections on stderr"),
	      N_("Do not list removed unused sections"));

  DEFINE_bool(print_icf_sections, options::TWO_DASHES, '\0', false,
	      N_("List folded identical sections on stderr"),
	      N_("Do not list folded identical sections"));

  DEFINE_bool(print_output_format, options::TWO_DASHES, '\0', false,
	      N_("Print default output format"), NULL);

  DEFINE_string(print_symbol_counts, options::TWO_DASHES, '\0', NULL,
		N_("Print symbols defined and used for each input"),
		N_("FILENAME"));

  DEFINE_special(push_state, options::TWO_DASHES, '\0',
		 N_("Save the state of flags related to input files"), NULL);
  DEFINE_special(pop_state, options::TWO_DASHES, '\0',
		 N_("Restore the state of flags related to input files"), NULL);

  // q

  DEFINE_bool(emit_relocs, options::TWO_DASHES, 'q', false,
	      N_("Generate relocations in output"), NULL);

  DEFINE_bool(Qy, options::EXACTLY_ONE_DASH, '\0', false,
	      N_("Ignored for SVR4 compatibility"), NULL);

  // r

  DEFINE_bool(relocatable, options::EXACTLY_ONE_DASH, 'r', false,
	      N_("Generate relocatable output"), NULL);

  DEFINE_bool(relax, options::TWO_DASHES, '\0', false,
	      N_("Relax branches on certain targets"),
	      N_("Do not relax branches"));

  DEFINE_string(retain_symbols_file, options::TWO_DASHES, '\0', NULL,
		N_("keep only symbols listed in this file"), N_("FILE"));

  DEFINE_bool(rosegment, options::TWO_DASHES, '\0', false,
	      N_("Put read-only non-executable sections in their own segment"),
	      N_("Do not put read-only non-executable sections in their own segment"));

  DEFINE_uint64(rosegment_gap, options::TWO_DASHES, '\0', -1U,
		N_("Set offset between executable and read-only segments"),
		N_("OFFSET"));

  // -R really means -rpath, but can mean --just-symbols for
  // compatibility with GNU ld.  -rpath is always -rpath, so we list
  // it separately.
  DEFINE_special(R, options::EXACTLY_ONE_DASH, 'R',
		 N_("Add DIR to runtime search path"), N_("DIR"));

  DEFINE_dirlist(rpath, options::ONE_DASH, '\0',
		 N_("Add DIR to runtime search path"), N_("DIR"));

  DEFINE_dirlist(rpath_link, options::TWO_DASHES, '\0',
		 N_("Add DIR to link time shared library search path"),
		 N_("DIR"));

  // s

  DEFINE_bool(strip_all, options::TWO_DASHES, 's', false,
	      N_("Strip all symbols"), NULL);
  DEFINE_bool(strip_debug, options::TWO_DASHES, 'S', false,
	      N_("Strip debugging information"), NULL);
  DEFINE_bool(strip_debug_non_line, options::TWO_DASHES, '\0', false,
	      N_("Emit only debug line number information"), NULL);
  DEFINE_bool(strip_debug_gdb, options::TWO_DASHES, '\0', false,
	      N_("Strip debug symbols that are unused by gdb "
		 "(at least versions <= 7.4)"), NULL);
  DEFINE_bool(strip_lto_sections, options::TWO_DASHES, '\0', true,
	      N_("Strip LTO intermediate code sections"), NULL);

  DEFINE_string(section_ordering_file, options::TWO_DASHES, '\0', NULL,
		N_("Layout sections in the order specified"),
		N_("FILENAME"));

  DEFINE_special(section_start, options::TWO_DASHES, '\0',
		 N_("Set address of section"), N_("SECTION=ADDRESS"));

  DEFINE_bool(secure_plt, options::TWO_DASHES , '\0', true,
	      N_("(PowerPC only) Use new-style PLT"), NULL);

  DEFINE_optional_string(sort_common, options::TWO_DASHES, '\0', NULL,
			 N_("Sort common symbols by alignment"),
			 N_("[={ascending,descending}]"));

  DEFINE_enum(sort_section, options::TWO_DASHES, '\0', "none",
	      N_("Sort sections by name.  \'--no-text-reorder\'"
		 " will override \'--sort-section=name\' for .text"),
	      N_("[none,name]"), false,
	      {"none", "name"});

  DEFINE_uint(spare_dynamic_tags, options::TWO_DASHES, '\0', 5,
	      N_("Dynamic tag slots to reserve (default 5)"),
	      N_("COUNT"));

  DEFINE_int(stub_group_size, options::TWO_DASHES , '\0', 1,
	     N_("(ARM, PowerPC only) The maximum distance from instructions "
		"in a group of sections to their stubs. Negative values mean "
		"stubs are always after the group. 1 means use default size"),
	     N_("SIZE"));

  DEFINE_bool(stub_group_multi, options::TWO_DASHES, '\0', true,
	      N_("(PowerPC only) Allow a group of stubs to serve multiple "
		 "output sections"),
	      N_("(PowerPC only) Each output section has its own stubs"));

  DEFINE_uint(split_stack_adjust_size, options::TWO_DASHES, '\0', 0x100000,
	      N_("Stack size when -fsplit-stack function calls non-split"),
	      N_("SIZE"));

  // This is not actually special in any way, but I need to give it
  // a non-standard accessor-function name because 'static' is a keyword.
  DEFINE_special(static, options::ONE_DASH, '\0',
		 N_("Do not link against shared libraries"), NULL);

  DEFINE_special(start_lib, options::TWO_DASHES, '\0',
		 N_("Start a library"), NULL);
  DEFINE_special(end_lib, options::TWO_DASHES, '\0',
		 N_("End a library "), NULL);

  DEFINE_bool(stats, options::TWO_DASHES, '\0', false,
	      N_("Print resource usage statistics"), NULL);

  DEFINE_string(sysroot, options::TWO_DASHES, '\0', "",
		N_("Set target system root directory"), N_("DIR"));

  // t

  DEFINE_bool(trace, options::TWO_DASHES, 't', false,
	      N_("Print the name of each input file"), NULL);

  DEFINE_bool(target1_abs, options::TWO_DASHES, '\0', false,
	      N_("(ARM only) Force R_ARM_TARGET1 type to R_ARM_ABS32"),
	      NULL);
  DEFINE_bool(target1_rel, options::TWO_DASHES, '\0', false,
	      N_("(ARM only) Force R_ARM_TARGET1 type to R_ARM_REL32"),
	      NULL);
  DEFINE_enum(target2, options::TWO_DASHES, '\0', NULL,
	      N_("(ARM only) Set R_ARM_TARGET2 relocation type"),
	      N_("[rel, abs, got-rel"), false,
	      {"rel", "abs", "got-rel"});

  DEFINE_bool(text_reorder, options::TWO_DASHES, '\0', true,
	      N_("Enable text section reordering for GCC section names"),
	      N_("Disable text section reordering for GCC section names"));

  DEFINE_bool(threads, options::TWO_DASHES, '\0', false,
	      N_("Run the linker multi-threaded"),
	      N_("Do not run the linker multi-threaded"));
  DEFINE_uint(thread_count, options::TWO_DASHES, '\0', 0,
	      N_("Number of threads to use"), N_("COUNT"));
  DEFINE_uint(thread_count_initial, options::TWO_DASHES, '\0', 0,
	      N_("Number of threads to use in initial pass"), N_("COUNT"));
  DEFINE_uint(thread_count_middle, options::TWO_DASHES, '\0', 0,
	      N_("Number of threads to use in middle pass"), N_("COUNT"));
  DEFINE_uint(thread_count_final, options::TWO_DASHES, '\0', 0,
	      N_("Number of threads to use in final pass"), N_("COUNT"));

  DEFINE_bool(tls_optimize, options::TWO_DASHES, '\0', true,
	      N_("(PowerPC/64 only) Optimize GD/LD/IE code to IE/LE"),
	      N_("(PowerPC/64 only) Don'\''t try to optimize TLS accesses"));
  DEFINE_bool(tls_get_addr_optimize, options::TWO_DASHES, '\0', true,
	      N_("(PowerPC/64 only) Use a special __tls_get_addr call"),
	      N_("(PowerPC/64 only) Don't use a special __tls_get_addr call"));

  DEFINE_bool(toc_optimize, options::TWO_DASHES, '\0', true,
	      N_("(PowerPC64 only) Optimize TOC code sequences"),
	      N_("(PowerPC64 only) Don't optimize TOC code sequences"));

  DEFINE_bool(toc_sort, options::TWO_DASHES, '\0', true,
	      N_("(PowerPC64 only) Sort TOC and GOT sections"),
	      N_("(PowerPC64 only) Don't sort TOC and GOT sections"));

  DEFINE_special(script, options::TWO_DASHES, 'T',
		 N_("Read linker script"), N_("FILE"));

  DEFINE_uint64(Tbss, options::ONE_DASH, '\0', -1U,
		N_("Set the address of the bss segment"), N_("ADDRESS"));
  DEFINE_uint64(Tdata, options::ONE_DASH, '\0', -1U,
		N_("Set the address of the data segment"), N_("ADDRESS"));
  DEFINE_uint64(Ttext, options::ONE_DASH, '\0', -1U,
		N_("Set the address of the text segment"), N_("ADDRESS"));
  DEFINE_uint64_alias(Ttext_segment, Ttext, options::ONE_DASH, '\0',
		      N_("Set the address of the text segment"),
		      N_("ADDRESS"));
  DEFINE_uint64(Trodata_segment, options::ONE_DASH, '\0', -1U,
		N_("Set the address of the rodata segment"), N_("ADDRESS"));

  // u

  DEFINE_set(undefined, options::TWO_DASHES, 'u',
	     N_("Create undefined reference to SYMBOL"), N_("SYMBOL"));

  DEFINE_enum(unresolved_symbols, options::TWO_DASHES, '\0', NULL,
	      N_("How to handle unresolved symbols"),
	      ("ignore-all,report-all,ignore-in-object-files,"
	       "ignore-in-shared-libs"), false,
	      {"ignore-all", "report-all", "ignore-in-object-files",
		  "ignore-in-shared-libs"});

  // v

  DEFINE_bool(verbose, options::TWO_DASHES, '\0', false,
	      N_("Alias for --debug=files"), NULL);

  DEFINE_special(version_script, options::TWO_DASHES, '\0',
		 N_("Read version script"), N_("FILE"));

  // w

  DEFINE_bool(warn_common, options::TWO_DASHES, '\0', false,
	      N_("Warn about duplicate common symbols"),
	      N_("Do not warn about duplicate common symbols"));

  DEFINE_bool_ignore(warn_constructors, options::TWO_DASHES, '\0',
		     N_("Ignored"), N_("Ignored"));

  DEFINE_bool(warn_drop_version, options::TWO_DASHES, '\0', false,
	      N_("Warn when discarding version information"),
	      N_("Do not warn when discarding version information"));

  DEFINE_bool(warn_execstack, options::TWO_DASHES, '\0', false,
	      N_("Warn if the stack is executable"),
	      N_("Do not warn if the stack is executable"));

  DEFINE_bool(warn_mismatch, options::TWO_DASHES, '\0', true,
	      NULL, N_("Don't warn about mismatched input files"));

  DEFINE_bool(warn_multiple_gp, options::TWO_DASHES, '\0', false,
	      N_("Ignored"), NULL);

  DEFINE_bool(warn_search_mismatch, options::TWO_DASHES, '\0', true,
	      N_("Warn when skipping an incompatible library"),
	      N_("Don't warn when skipping an incompatible library"));

  DEFINE_bool(warn_shared_textrel, options::TWO_DASHES, '\0', false,
	      N_("Warn if text segment is not shareable"),
	      N_("Do not warn if text segment is not shareable"));

  DEFINE_bool(warn_unresolved_symbols, options::TWO_DASHES, '\0', false,
	      N_("Report unresolved symbols as warnings"),
	      NULL);
  DEFINE_bool_alias(error_unresolved_symbols, warn_unresolved_symbols,
		    options::TWO_DASHES, '\0',
		    N_("Report unresolved symbols as errors"),
		    NULL, true);

  DEFINE_bool(wchar_size_warning, options::TWO_DASHES, '\0', true, NULL,
	      N_("(ARM only) Do not warn about objects with incompatible "
		 "wchar_t sizes"));

  DEFINE_bool(weak_unresolved_symbols, options::TWO_DASHES, '\0', false,
	      N_("Convert unresolved symbols to weak references"),
	      NULL);

  DEFINE_bool(whole_archive, options::TWO_DASHES, '\0', false,
	      N_("Include all archive contents"),
	      N_("Include only needed archive contents"));

  DEFINE_set(wrap, options::TWO_DASHES, '\0',
	     N_("Use wrapper functions for SYMBOL"), N_("SYMBOL"));

  // x

  DEFINE_special(discard_all, options::TWO_DASHES, 'x',
		 N_("Delete all local symbols"), NULL);
  DEFINE_special(discard_locals, options::TWO_DASHES, 'X',
		 N_("Delete all temporary local symbols"), NULL);
  DEFINE_special(discard_none, options::TWO_DASHES, '\0',
		 N_("Keep all local symbols"), NULL);

  // y

  DEFINE_set(trace_symbol, options::TWO_DASHES, 'y',
	     N_("Trace references to symbol"), N_("SYMBOL"));

  DEFINE_bool(undefined_version, options::TWO_DASHES, '\0', true,
	      N_("Allow unused version in script"),
	      N_("Do not allow unused version in script"));

  DEFINE_string(Y, options::EXACTLY_ONE_DASH, 'Y', "",
		N_("Default search path for Solaris compatibility"),
		N_("PATH"));

  // special characters

  DEFINE_special(start_group, options::TWO_DASHES, '(',
		 N_("Start a library search group"), NULL);
  DEFINE_special(end_group, options::TWO_DASHES, ')',
		 N_("End a library search group"), NULL);

  // The -z options.

  DEFINE_bool(combreloc, options::DASH_Z, '\0', true,
	      N_("Sort dynamic relocs"),
	      N_("Do not sort dynamic relocs"));
  DEFINE_uint64(common_page_size, options::DASH_Z, '\0', 0,
		N_("Set common page size to SIZE"), N_("SIZE"));
  DEFINE_bool(defs, options::DASH_Z, '\0', false,
	      N_("Report undefined symbols (even with --shared)"),
	      NULL);
  DEFINE_bool(execstack, options::DASH_Z, '\0', false,
	      N_("Mark output as requiring executable stack"), NULL);
  DEFINE_bool(global, options::DASH_Z, '\0', false,
	      N_("Make symbols in DSO available for subsequently loaded "
		 "objects"), NULL);
  DEFINE_bool(initfirst, options::DASH_Z, '\0', false,
	      N_("Mark DSO to be initialized first at runtime"),
	      NULL);
  DEFINE_bool(interpose, options::DASH_Z, '\0', false,
	      N_("Mark object to interpose all DSOs but executable"),
	      NULL);
  DEFINE_bool(unique, options::DASH_Z, '\0', false,
	      N_("Mark DSO to be loaded at most once, and only in the main namespace"),
	      N_("Do not mark the DSO as one to be loaded only in the main namespace"));
  DEFINE_bool_alias(lazy, now, options::DASH_Z, '\0',
		    N_("Mark object for lazy runtime binding"),
		    NULL, true);
  DEFINE_bool(loadfltr, options::DASH_Z, '\0', false,
	      N_("Mark object requiring immediate process"),
	      NULL);
  DEFINE_uint64(max_page_size, options::DASH_Z, '\0', 0,
		N_("Set maximum page size to SIZE"), N_("SIZE"));
  DEFINE_bool(muldefs, options::DASH_Z, '\0', false,
	      N_("Allow multiple definitions of symbols"),
	      NULL);
  // copyreloc is here in the list because there is only -z
  // nocopyreloc, not -z copyreloc.
  DEFINE_bool(copyreloc, options::DASH_Z, '\0', true,
	      NULL,
	      N_("Do not create copy relocs"));
  DEFINE_bool(nodefaultlib, options::DASH_Z, '\0', false,
	      N_("Mark object not to use default search paths"),
	      NULL);
  DEFINE_bool(nodelete, options::DASH_Z, '\0', false,
	      N_("Mark DSO non-deletable at runtime"),
	      NULL);
  DEFINE_bool(nodlopen, options::DASH_Z, '\0', false,
	      N_("Mark DSO not available to dlopen"),
	      NULL);
  DEFINE_bool(nodump, options::DASH_Z, '\0', false,
	      N_("Mark DSO not available to dldump"),
	      NULL);
  DEFINE_bool(noexecstack, options::DASH_Z, '\0', false,
	      N_("Mark output as not requiring executable stack"), NULL);
  DEFINE_bool(now, options::DASH_Z, '\0', false,
	      N_("Mark object for immediate function binding"),
	      NULL);
  DEFINE_bool(origin, options::DASH_Z, '\0', false,
	      N_("Mark DSO to indicate that needs immediate $ORIGIN "
		 "processing at runtime"), NULL);
  DEFINE_bool(relro, options::DASH_Z, '\0', DEFAULT_LD_Z_RELRO,
	      N_("Where possible mark variables read-only after relocation"),
	      N_("Don't mark variables read-only after relocation"));
  DEFINE_uint64(stack_size, options::DASH_Z, '\0', 0,
		N_("Set PT_GNU_STACK segment p_memsz to SIZE"), N_("SIZE"));
  DEFINE_enum(start_stop_visibility, options::DASH_Z, '\0', "protected",
	      N_("ELF symbol visibility for synthesized "
		 "__start_* and __stop_* symbols"),
	      ("[default,internal,hidden,protected]"), false,
	      {"default", "internal", "hidden", "protected"});
  DEFINE_bool(text, options::DASH_Z, '\0', false,
	      N_("Do not permit relocations in read-only segments"),
	      N_("Permit relocations in read-only segments"));
  DEFINE_bool_alias(textoff, text, options::DASH_Z, '\0',
		    N_("Permit relocations in read-only segments"),
		    NULL, true);
  DEFINE_bool(text_unlikely_segment, options::DASH_Z, '\0', false,
	      N_("Move .text.unlikely sections to a separate segment."),
	      N_("Do not move .text.unlikely sections to a separate "
		 "segment."));
  DEFINE_bool(keep_text_section_prefix, options::DASH_Z, '\0', false,
	      N_("Keep .text.hot, .text.startup, .text.exit and .text.unlikely "
		 "as separate sections in the final binary."),
	      N_("Merge all .text.* prefix sections."));


 public:
  typedef options::Dir_list Dir_list;

  General_options();

  // Does post-processing on flags, making sure they all have
  // non-conflicting values.  Also converts some flags from their
  // "standard" types (string, etc), to another type (enum, DirList),
  // which can be accessed via a separate method.  Dies if it notices
  // any problems.
  void finalize();

  // True if we printed the version information.
  bool
  printed_version() const
  { return this->printed_version_; }

  // The macro defines output() (based on --output), but that's a
  // generic name.  Provide this alternative name, which is clearer.
  const char*
  output_file_name() const
  { return this->output(); }

  // This is not defined via a flag, but combines flags to say whether
  // the output is position-independent or not.
  bool
  output_is_position_independent() const
  { return this->shared() || this->pie(); }

  // Return true if the output is something that can be exec()ed, such
  // as a static executable, or a position-dependent or
  // position-independent executable, but not a dynamic library or an
  // object file.
  bool
  output_is_executable() const
  { return !this->shared() && !this->relocatable(); }

  // This would normally be static(), and defined automatically, but
  // since static is a keyword, we need to come up with our own name.
  bool
  is_static() const
  { return static_; }

  // In addition to getting the input and output formats as a string
  // (via format() and oformat()), we also give access as an enum.
  enum Object_format
  {
    // Ordinary ELF.
    OBJECT_FORMAT_ELF,
    // Straight binary format.
    OBJECT_FORMAT_BINARY
  };

  // Convert a string to an Object_format.  Gives an error if the
  // string is not recognized.
  static Object_format
  string_to_object_format(const char* arg);

  // Convert an Object_format to string.
  static const char*
  object_format_to_string(Object_format);

  // Note: these functions are not very fast.
  Object_format format_enum() const;
  Object_format oformat_enum() const;

  // Return whether FILENAME is in a system directory.
  bool
  is_in_system_directory(const std::string& name) const;

  // RETURN whether SYMBOL_NAME should be kept, according to symbols_to_retain_.
  bool
  should_retain_symbol(const char* symbol_name) const
    {
      if (symbols_to_retain_.empty())    // means flag wasn't specified
	return true;
      return symbols_to_retain_.find(symbol_name) != symbols_to_retain_.end();
    }

  // These are the best way to get access to the execstack state,
  // not execstack() and noexecstack() which are hard to use properly.
  bool
  is_execstack_set() const
  { return this->execstack_status_ != EXECSTACK_FROM_INPUT; }

  bool
  is_stack_executable() const
  { return this->execstack_status_ == EXECSTACK_YES; }

  bool
  icf_enabled() const
  { return this->icf_status_ != ICF_NONE; }

  bool
  icf_safe_folding() const
  { return this->icf_status_ == ICF_SAFE; }

  // The --demangle option takes an optional string, and there is also
  // a --no-demangle option.  This is the best way to decide whether
  // to demangle or not.
  bool
  do_demangle() const
  { return this->do_demangle_; }

  // Returns TRUE if any plugin libraries have been loaded.
  bool
  has_plugins() const
  { return this->plugins_ != NULL; }

  // Return a pointer to the plugin manager.
  Plugin_manager*
  plugins() const
  { return this->plugins_; }

  // True iff SYMBOL was found in the file specified by dynamic-list.
  bool
  in_dynamic_list(const char* symbol) const
  { return this->dynamic_list_.version_script_info()->symbol_is_local(symbol); }

  // True if a --dynamic-list script was provided.
  bool
  have_dynamic_list() const
  { return this->have_dynamic_list_; }

  // Finalize the dynamic list.
  void
  finalize_dynamic_list()
  { this->dynamic_list_.version_script_info()->finalize(); }

  // The mode selected by the --incremental options.
  enum Incremental_mode
  {
    // No incremental linking (--no-incremental).
    INCREMENTAL_OFF,
    // Incremental update only (--incremental-update).
    INCREMENTAL_UPDATE,
    // Force a full link, but prepare for subsequent incremental link
    // (--incremental-full).
    INCREMENTAL_FULL,
    // Incremental update if possible, fallback to full link  (--incremental).
    INCREMENTAL_AUTO
  };

  // The incremental linking mode.
  Incremental_mode
  incremental_mode() const
  { return this->incremental_mode_; }

  // The disposition given by the --incremental-changed,
  // --incremental-unchanged or --incremental-unknown option.  The
  // value may change as we proceed parsing the command line flags.
  Incremental_disposition
  incremental_disposition() const
  { return this->incremental_disposition_; }

  void
  set_incremental_disposition(Incremental_disposition disp)
  { this->incremental_disposition_ = disp; }

  // The disposition to use for startup files (those that precede the
  // first --incremental-changed, etc. option).
  Incremental_disposition
  incremental_startup_disposition() const
  { return this->incremental_startup_disposition_; }

  // Return true if S is the name of a library excluded from automatic
  // symbol export.
  bool
  check_excluded_libs(const std::string &s) const;

  // If an explicit start address was given for section SECNAME with
  // the --section-start option, return true and set *PADDR to the
  // address.  Otherwise return false.
  bool
  section_start(const char* secname, uint64_t* paddr) const;

  // Return whether any --section-start option was used.
  bool
  any_section_start() const
  { return !this->section_starts_.empty(); }

  enum Fix_v4bx
  {
    // Leave original instruction.
    FIX_V4BX_NONE,
    // Replace instruction.
    FIX_V4BX_REPLACE,
    // Generate an interworking veneer.
    FIX_V4BX_INTERWORKING
  };

  Fix_v4bx
  fix_v4bx() const
  { return (this->fix_v4bx_); }

  enum Endianness
  {
    ENDIANNESS_NOT_SET,
    ENDIANNESS_BIG,
    ENDIANNESS_LITTLE
  };

  Endianness
  endianness() const
  { return this->endianness_; }

  enum Bsymbolic_kind
  {
    BSYMBOLIC_NONE,
    BSYMBOLIC_FUNCTIONS,
    BSYMBOLIC_ALL,
  };

  bool
  Bsymbolic() const
  { return this->bsymbolic_ == BSYMBOLIC_ALL; }

  bool
  Bsymbolic_functions() const
  { return this->bsymbolic_ == BSYMBOLIC_FUNCTIONS; }

  bool
  discard_all() const
  { return this->discard_locals_ == DISCARD_ALL; }

  bool
  discard_locals() const
  { return this->discard_locals_ == DISCARD_LOCALS; }

  bool
  discard_sec_merge() const
  { return this->discard_locals_ == DISCARD_SEC_MERGE; }

  enum Orphan_handling
  {
    // Place orphan sections normally (default).
    ORPHAN_PLACE,
    // Discard all orphan sections.
    ORPHAN_DISCARD,
    // Warn when placing orphan sections.
    ORPHAN_WARN,
    // Issue error for orphan sections.
    ORPHAN_ERROR
  };

  Orphan_handling
  orphan_handling_enum() const
  { return this->orphan_handling_enum_; }

  elfcpp::STV
  start_stop_visibility_enum() const
  { return this->start_stop_visibility_enum_; }

  enum Power10_stubs
  {
    // Use Power10 insns on @notoc calls/branches, non-Power10 elsewhere.
    POWER10_STUBS_AUTO,
    // Don't use Power10 insns
    POWER10_STUBS_NO,
    // Always use Power10 insns
    POWER10_STUBS_YES
  };

  Power10_stubs
  power10_stubs_enum() const
  { return this->power10_stubs_enum_; }

 private:
  // Don't copy this structure.
  General_options(const General_options&);
  General_options& operator=(const General_options&);

  // What local symbols to discard.
  enum Discard_locals
  {
    // Locals in merge sections (default).
    DISCARD_SEC_MERGE,
    // None (--discard-none).
    DISCARD_NONE,
    // Temporary locals (--discard-locals/-X).
    DISCARD_LOCALS,
    // All locals (--discard-all/-x).
    DISCARD_ALL
  };

  // Whether to mark the stack as executable.
  enum Execstack
  {
    // Not set on command line.
    EXECSTACK_FROM_INPUT,
    // Mark the stack as executable (-z execstack).
    EXECSTACK_YES,
    // Mark the stack as not executable (-z noexecstack).
    EXECSTACK_NO
  };

  enum Icf_status
  {
    // Do not fold any functions (Default or --icf=none).
    ICF_NONE,
    // All functions are candidates for folding. (--icf=all).
    ICF_ALL,
    // Only ctors and dtors are candidates for folding. (--icf=safe).
    ICF_SAFE
  };

  void
  set_icf_status(Icf_status value)
  { this->icf_status_ = value; }

  void
  set_execstack_status(Execstack value)
  { this->execstack_status_ = value; }

  void
  set_do_demangle(bool value)
  { this->do_demangle_ = value; }

  void
  set_static(bool value)
  { static_ = value; }

  void
  set_orphan_handling_enum(Orphan_handling value)
  { this->orphan_handling_enum_ = value; }

  void
  set_start_stop_visibility_enum(elfcpp::STV value)
  { this->start_stop_visibility_enum_ = value; }

  void
  set_power10_stubs_enum(Power10_stubs value)
  { this->power10_stubs_enum_ = value; }

  // These are called by finalize() to set up the search-path correctly.
  void
  add_to_library_path_with_sysroot(const std::string& arg)
  { this->add_search_directory_to_library_path(Search_directory(arg, true)); }

  // Apply any sysroot to the directory lists.
  void
  add_sysroot();

  // Add a plugin and its arguments to the list of plugins.
  void
  add_plugin(const char* filename);

  // Add a plugin option.
  void
  add_plugin_option(const char* opt);

  void
  copy_from_posdep_options(const Position_dependent_options&);

  // Whether we bind default visibility defined symbols locally for -shared.
  Bsymbolic_kind bsymbolic_;
  // Whether we printed version information.
  bool printed_version_;
  // Whether to mark the stack as executable.
  Execstack execstack_status_;
  // Whether to do code folding.
  Icf_status icf_status_;
  // Whether to do a static link.
  bool static_;
  // Whether to do demangling.
  bool do_demangle_;
  // List of plugin libraries.
  Plugin_manager* plugins_;
  // The parsed output of --dynamic-list files.  For convenience in
  // script.cc, we store this as a Script_options object, even though
  // we only use a single Version_tree from it.
  Script_options dynamic_list_;
  // Whether a --dynamic-list file was provided.
  bool have_dynamic_list_;
  // The incremental linking mode.
  Incremental_mode incremental_mode_;
  // The disposition given by the --incremental-changed,
  // --incremental-unchanged or --incremental-unknown option.  The
  // value may change as we proceed parsing the command line flags.
  Incremental_disposition incremental_disposition_;
  // The disposition to use for startup files (those marked
  // INCREMENTAL_STARTUP).
  Incremental_disposition incremental_startup_disposition_;
  // Whether we have seen one of the options that require incremental
  // build (--incremental-changed, --incremental-unchanged,
  // --incremental-unknown, or --incremental-startup-unchanged).
  bool implicit_incremental_;
  // Libraries excluded from automatic export, via --exclude-libs.
  Unordered_set<std::string> excluded_libs_;
  // List of symbol-names to keep, via -retain-symbol-info.
  Unordered_set<std::string> symbols_to_retain_;
  // Map from section name to address from --section-start.
  std::map<std::string, uint64_t> section_starts_;
  // Whether to process armv4 bx instruction relocation.
  Fix_v4bx fix_v4bx_;
  // Endianness.
  Endianness endianness_;
  // What local symbols to discard.
  Discard_locals discard_locals_;
  // Stack of saved options for --push-state/--pop-state.
  std::vector<Position_dependent_options*> options_stack_;
  // Orphan handling option, decoded to an enum value.
  Orphan_handling orphan_handling_enum_;
  // Symbol visibility for __start_* / __stop_* magic symbols.
  elfcpp::STV start_stop_visibility_enum_;
  // Power10 stubs option
  Power10_stubs power10_stubs_enum_;
};

// The position-dependent options.  We use this to store the state of
// the commandline at a particular point in parsing for later
// reference.  For instance, if we see "ld --whole-archive foo.a
// --no-whole-archive," we want to store the whole-archive option with
// foo.a, so when the time comes to parse foo.a we know we should do
// it in whole-archive mode.  We could store all of General_options,
// but that's big, so we just pick the subset of flags that actually
// change in a position-dependent way.

#define DEFINE_posdep(varname__, type__)        \
 public:                                        \
  type__                                        \
  varname__() const                             \
  { return this->varname__##_; }                \
						\
  void                                          \
  set_##varname__(type__ value)                 \
  { this->varname__##_ = value; }               \
 private:                                       \
  type__ varname__##_

class Position_dependent_options
{
 public:
  Position_dependent_options(const General_options& options
			     = Position_dependent_options::default_options_)
  { copy_from_options(options); }

  void
  copy_from_options(const General_options& options)
  {
    this->set_as_needed(options.as_needed());
    this->set_Bdynamic(options.Bdynamic());
    this->set_format_enum(options.format_enum());
    this->set_whole_archive(options.whole_archive());
    this->set_incremental_disposition(options.incremental_disposition());
  }

  DEFINE_posdep(as_needed, bool);
  DEFINE_posdep(Bdynamic, bool);
  DEFINE_posdep(format_enum, General_options::Object_format);
  DEFINE_posdep(whole_archive, bool);
  DEFINE_posdep(incremental_disposition, Incremental_disposition);

 private:
  // This is a General_options with everything set to its default
  // value.  A Position_dependent_options created with no argument
  // will take its values from here.
  static General_options default_options_;
};


// A single file or library argument from the command line.

class Input_file_argument
{
 public:
  enum Input_file_type
  {
    // A regular file, name used as-is, not searched.
    INPUT_FILE_TYPE_FILE,
    // A library name.  When used, "lib" will be prepended and ".so" or
    // ".a" appended to make a filename, and that filename will be searched
    // for using the -L paths.
    INPUT_FILE_TYPE_LIBRARY,
    // A regular file, name used as-is, but searched using the -L paths.
    INPUT_FILE_TYPE_SEARCHED_FILE
  };

  // name: file name or library name
  // type: the type of this input file.
  // extra_search_path: an extra directory to look for the file, prior
  //         to checking the normal library search path.  If this is "",
  //         then no extra directory is added.
  // just_symbols: whether this file only defines symbols.
  // options: The position dependent options at this point in the
  //         command line, such as --whole-archive.
  Input_file_argument()
    : name_(), type_(INPUT_FILE_TYPE_FILE), extra_search_path_(""),
      just_symbols_(false), options_(), arg_serial_(0)
  { }

  Input_file_argument(const char* name, Input_file_type type,
		      const char* extra_search_path,
		      bool just_symbols,
		      const Position_dependent_options& options)
    : name_(name), type_(type), extra_search_path_(extra_search_path),
      just_symbols_(just_symbols), options_(options), arg_serial_(0)
  { }

  // You can also pass in a General_options instance instead of a
  // Position_dependent_options.  In that case, we extract the
  // position-independent vars from the General_options and only store
  // those.
  Input_file_argument(const char* name, Input_file_type type,
		      const char* extra_search_path,
		      bool just_symbols,
		      const General_options& options)
    : name_(name), type_(type), extra_search_path_(extra_search_path),
      just_symbols_(just_symbols), options_(options), arg_serial_(0)
  { }

  const char*
  name() const
  { return this->name_.c_str(); }

  const Position_dependent_options&
  options() const
  { return this->options_; }

  bool
  is_lib() const
  { return type_ == INPUT_FILE_TYPE_LIBRARY; }

  bool
  is_searched_file() const
  { return type_ == INPUT_FILE_TYPE_SEARCHED_FILE; }

  const char*
  extra_search_path() const
  {
    return (this->extra_search_path_.empty()
	    ? NULL
	    : this->extra_search_path_.c_str());
  }

  // Return whether we should only read symbols from this file.
  bool
  just_symbols() const
  { return this->just_symbols_; }

  // Return whether this file may require a search using the -L
  // options.
  bool
  may_need_search() const
  {
    return (this->is_lib()
	    || this->is_searched_file()
	    || !this->extra_search_path_.empty());
  }

  // Set the serial number for this argument.
  void
  set_arg_serial(unsigned int arg_serial)
  { this->arg_serial_ = arg_serial; }

  // Get the serial number.
  unsigned int
  arg_serial() const
  { return this->arg_serial_; }

 private:
  // We use std::string, not const char*, here for convenience when
  // using script files, so that we do not have to preserve the string
  // in that case.
  std::string name_;
  Input_file_type type_;
  std::string extra_search_path_;
  bool just_symbols_;
  Position_dependent_options options_;
  // A unique index for this file argument in the argument list.
  unsigned int arg_serial_;
};

// A file or library, or a group, from the command line.

class Input_argument
{
 public:
  // Create a file or library argument.
  explicit Input_argument(Input_file_argument file)
    : is_file_(true), file_(file), group_(NULL), lib_(NULL), script_info_(NULL)
  { }

  // Create a group argument.
  explicit Input_argument(Input_file_group* group)
    : is_file_(false), group_(group), lib_(NULL), script_info_(NULL)
  { }

  // Create a lib argument.
  explicit Input_argument(Input_file_lib* lib)
    : is_file_(false), group_(NULL), lib_(lib), script_info_(NULL)
  { }

  // Return whether this is a file.
  bool
  is_file() const
  { return this->is_file_; }

  // Return whether this is a group.
  bool
  is_group() const
  { return !this->is_file_ && this->lib_ == NULL; }

  // Return whether this is a lib.
  bool
  is_lib() const
  { return this->lib_ != NULL; }

  // Return the information about the file.
  const Input_file_argument&
  file() const
  {
    gold_assert(this->is_file_);
    return this->file_;
  }

  // Return the information about the group.
  const Input_file_group*
  group() const
  {
    gold_assert(!this->is_file_);
    return this->group_;
  }

  Input_file_group*
  group()
  {
    gold_assert(!this->is_file_);
    return this->group_;
  }

  // Return the information about the lib.
  const Input_file_lib*
  lib() const
  {
    gold_assert(!this->is_file_);
    gold_assert(this->lib_);
    return this->lib_;
  }

  Input_file_lib*
  lib()
  {
    gold_assert(!this->is_file_);
    gold_assert(this->lib_);
    return this->lib_;
  }

  // If a script generated this argument, store a pointer to the script info.
  // Currently used only for recording incremental link information.
  void
  set_script_info(Script_info* info)
  { this->script_info_ = info; }

  Script_info*
  script_info() const
  { return this->script_info_; }

 private:
  bool is_file_;
  Input_file_argument file_;
  Input_file_group* group_;
  Input_file_lib* lib_;
  Script_info* script_info_;
};

typedef std::vector<Input_argument> Input_argument_list;

// A group from the command line.  This is a set of arguments within
// --start-group ... --end-group.

class Input_file_group
{
 public:
  typedef Input_argument_list::const_iterator const_iterator;

  Input_file_group()
    : files_()
  { }

  // Add a file to the end of the group.
  Input_argument&
  add_file(const Input_file_argument& arg)
  {
    this->files_.push_back(Input_argument(arg));
    return this->files_.back();
  }

  // Iterators to iterate over the group contents.

  const_iterator
  begin() const
  { return this->files_.begin(); }

  const_iterator
  end() const
  { return this->files_.end(); }

 private:
  Input_argument_list files_;
};

// A lib from the command line.  This is a set of arguments within
// --start-lib ... --end-lib.

class Input_file_lib
{
 public:
  typedef Input_argument_list::const_iterator const_iterator;

  Input_file_lib(const Position_dependent_options& options)
    : files_(), options_(options)
  { }

  // Add a file to the end of the lib.
  Input_argument&
  add_file(const Input_file_argument& arg)
  {
    this->files_.push_back(Input_argument(arg));
    return this->files_.back();
  }

  const Position_dependent_options&
  options() const
  { return this->options_; }

  // Iterators to iterate over the lib contents.

  const_iterator
  begin() const
  { return this->files_.begin(); }

  const_iterator
  end() const
  { return this->files_.end(); }

  size_t
  size() const
  { return this->files_.size(); }

 private:
  Input_argument_list files_;
  Position_dependent_options options_;
};

// A list of files from the command line or a script.

class Input_arguments
{
 public:
  typedef Input_argument_list::const_iterator const_iterator;

  Input_arguments()
    : input_argument_list_(), in_group_(false), in_lib_(false), file_count_(0)
  { }

  // Add a file.
  Input_argument&
  add_file(Input_file_argument& arg);

  // Start a group (the --start-group option).
  void
  start_group();

  // End a group (the --end-group option).
  void
  end_group();

  // Start a lib (the --start-lib option).
  void
  start_lib(const Position_dependent_options&);

  // End a lib (the --end-lib option).
  void
  end_lib();

  // Return whether we are currently in a group.
  bool
  in_group() const
  { return this->in_group_; }

  // Return whether we are currently in a lib.
  bool
  in_lib() const
  { return this->in_lib_; }

  // The number of entries in the list.
  int
  size() const
  { return this->input_argument_list_.size(); }

  // Iterators to iterate over the list of input files.

  const_iterator
  begin() const
  { return this->input_argument_list_.begin(); }

  const_iterator
  end() const
  { return this->input_argument_list_.end(); }

  // Return whether the list is empty.
  bool
  empty() const
  { return this->input_argument_list_.empty(); }

  // Return the number of input files.  This may be larger than
  // input_argument_list_.size(), because of files that are part
  // of groups or libs.
  int
  number_of_input_files() const
  { return this->file_count_; }

 private:
  Input_argument_list input_argument_list_;
  bool in_group_;
  bool in_lib_;
  unsigned int file_count_;
};


// All the information read from the command line.  These are held in
// three separate structs: one to hold the options (--foo), one to
// hold the filenames listed on the commandline, and one to hold
// linker script information.  This third is not a subset of the other
// two because linker scripts can be specified either as options (via
// -T) or as a file.

class Command_line
{
 public:
  typedef Input_arguments::const_iterator const_iterator;

  Command_line();

  // Process the command line options.  This will exit with an
  // appropriate error message if an unrecognized option is seen.
  void
  process(int argc, const char** argv);

  // Process one command-line option.  This takes the index of argv to
  // process, and returns the index for the next option.  no_more_options
  // is set to true if argv[i] is "--".
  int
  process_one_option(int argc, const char** argv, int i,
		     bool* no_more_options);

  // Get the general options.
  const General_options&
  options() const
  { return this->options_; }

  // Get the position dependent options.
  const Position_dependent_options&
  position_dependent_options() const
  { return this->position_options_; }

  // Get the linker-script options.
  Script_options&
  script_options()
  { return this->script_options_; }

  // Finalize the version-script options and return them.
  const Version_script_info&
  version_script();

  // Get the input files.
  Input_arguments&
  inputs()
  { return this->inputs_; }

  // The number of input files.
  int
  number_of_input_files() const
  { return this->inputs_.number_of_input_files(); }

  // Iterators to iterate over the list of input files.

  const_iterator
  begin() const
  { return this->inputs_.begin(); }

  const_iterator
  end() const
  { return this->inputs_.end(); }

 private:
  Command_line(const Command_line&);
  Command_line& operator=(const Command_line&);

  // This is a dummy class to provide a constructor that runs before
  // the constructor for the General_options.  The Pre_options constructor
  // is used as a hook to set the flag enabling the options to register
  // themselves.
  struct Pre_options {
    Pre_options();
  };

  // This must come before options_!
  Pre_options pre_options_;
  General_options options_;
  Position_dependent_options position_options_;
  Script_options script_options_;
  Input_arguments inputs_;
};

} // End namespace gold.

#endif // !defined(GOLD_OPTIONS_H)
