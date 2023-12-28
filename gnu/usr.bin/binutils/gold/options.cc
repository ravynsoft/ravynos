// options.c -- handle command line options for gold

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
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <iostream>
#include <sys/stat.h>
#include "filenames.h"
#include "libiberty.h"
#include "demangle.h"
#include "../bfd/bfdver.h"

#include "debug.h"
#include "script.h"
#include "target-select.h"
#include "options.h"
#include "plugin.h"

namespace gold
{

General_options
Position_dependent_options::default_options_;

namespace options
{

// This flag is TRUE if we should register the command-line options as they
// are constructed.  It is set after construction of the options within
// class Position_dependent_options.
static bool ready_to_register = false;

// This global variable is set up as General_options is constructed.
static std::vector<const One_option*> registered_options;

// These are set up at the same time -- the variables that accept one
// dash, two, or require -z.  A single variable may be in more than
// one of these data structures.
typedef Unordered_map<std::string, One_option*> Option_map;
static Option_map* long_options = NULL;
static One_option* short_options[128];

void
One_option::register_option()
{
  if (!ready_to_register)
    return;

  registered_options.push_back(this);

  // We can't make long_options a static Option_map because we can't
  // guarantee that will be initialized before register_option() is
  // first called.
  if (long_options == NULL)
    long_options = new Option_map;

  // TWO_DASHES means that two dashes are preferred, but one is ok too.
  if (!this->longname.empty())
    (*long_options)[this->longname] = this;

  const int shortname_as_int = static_cast<int>(this->shortname);
  gold_assert(shortname_as_int >= 0 && shortname_as_int < 128);
  if (this->shortname != '\0')
    {
      gold_assert(short_options[shortname_as_int] == NULL);
      short_options[shortname_as_int] = this;
    }
}

void
One_option::print() const
{
  bool comma = false;
  printf("  ");
  int len = 2;
  if (this->shortname != '\0')
    {
      len += printf("-%c", this->shortname);
      if (this->helparg)
	{
	  // -z takes long-names only.
	  gold_assert(this->dashes != DASH_Z);
	  len += printf(" %s", gettext(this->helparg));
	}
      comma = true;
    }
  if (!this->longname.empty()
      && !(this->longname[0] == this->shortname
	   && this->longname[1] == '\0'))
    {
      if (comma)
	len += printf(", ");
      switch (this->dashes)
	{
	case options::ONE_DASH: case options::EXACTLY_ONE_DASH:
	  len += printf("-");
	  break;
	case options::TWO_DASHES: case options::EXACTLY_TWO_DASHES:
	  len += printf("--");
	  break;
	case options::DASH_Z:
	  len += printf("-z ");
	  break;
	default:
	  gold_unreachable();
	}
      len += printf("%s", this->longname.c_str());
      if (this->helparg)
	{
	  // For most options, we print "--frob FOO".  But for -z
	  // we print "-z frob=FOO".
	  len += printf("%c%s", this->dashes == options::DASH_Z ? '=' : ' ',
			gettext(this->helparg));
	}
    }

  if (len >= 30)
    {
      printf("\n");
      len = 0;
    }
  for (; len < 30; ++len)
    std::putchar(' ');

  printf("%s", gettext(this->helpstring));
  if (this->is_default)
    printf(" (%s)", _("default"));
  printf("\n");
}

void
help()
{
  printf(_("Usage: %s [options] file...\nOptions:\n"), gold::program_name);

  std::vector<const One_option*>::const_iterator it;
  for (it = registered_options.begin(); it != registered_options.end(); ++it)
    (*it)->print();

  // config.guess and libtool.m4 look in ld --help output for the
  // string "supported targets".
  printf(_("%s: supported targets:"), gold::program_name);
  std::vector<const char*> supported_names;
  gold::supported_target_names(&supported_names);
  for (std::vector<const char*>::const_iterator p = supported_names.begin();
       p != supported_names.end();
       ++p)
    printf(" %s", *p);
  printf("\n");

  printf(_("%s: supported emulations:"), gold::program_name);
  supported_names.clear();
  gold::supported_emulation_names(&supported_names);
  for (std::vector<const char*>::const_iterator p = supported_names.begin();
       p != supported_names.end();
       ++p)
    printf(" %s", *p);
  printf("\n");

  // REPORT_BUGS_TO is defined in bfd/bfdver.h.
  const char* report = REPORT_BUGS_TO;
  if (*report != '\0')
    printf(_("Report bugs to %s\n"), report);
}

// For bool, arg will be NULL (boolean options take no argument);
// we always just set to true.
void
parse_bool(const char*, const char*, bool* retval)
{
  *retval = true;
}

void
parse_uint(const char* option_name, const char* arg, int* retval)
{
  char* endptr;
  *retval = strtol(arg, &endptr, 0);
  if (*endptr != '\0' || *retval < 0)
    gold_fatal(_("%s: invalid option value (expected an integer): %s"),
	       option_name, arg);
}

void
parse_int(const char* option_name, const char* arg, int* retval)
{
  char* endptr;
  *retval = strtol(arg, &endptr, 0);
  if (*endptr != '\0')
    gold_fatal(_("%s: invalid option value (expected an integer): %s"),
	       option_name, arg);
}

void
parse_uint64(const char* option_name, const char* arg, uint64_t* retval)
{
  char* endptr;
  *retval = strtoull(arg, &endptr, 0);
  if (*endptr != '\0')
    gold_fatal(_("%s: invalid option value (expected an integer): %s"),
	       option_name, arg);
}

void
parse_double(const char* option_name, const char* arg, double* retval)
{
  char* endptr;
  *retval = strtod(arg, &endptr);
  if (*endptr != '\0')
    gold_fatal(_("%s: invalid option value "
		 "(expected a floating point number): %s"),
	       option_name, arg);
}

void
parse_percent(const char* option_name, const char* arg, double* retval)
{
  char* endptr;
  *retval = strtod(arg, &endptr) / 100.0;
  if (*endptr != '\0')
    gold_fatal(_("%s: invalid option value "
		 "(expected a floating point number): %s"),
	       option_name, arg);
}

void
parse_string(const char* option_name, const char* arg, const char** retval)
{
  if (*arg == '\0')
    gold_fatal(_("%s: must take a non-empty argument"), option_name);
  *retval = arg;
}

void
parse_optional_string(const char*, const char* arg, const char** retval)
{
  *retval = arg;
}

void
parse_dirlist(const char*, const char* arg, Dir_list* retval)
{
  retval->push_back(Search_directory(arg, false));
}

void
parse_set(const char*, const char* arg, String_set* retval)
{
  retval->insert(std::string(arg));
}

void
parse_choices(const char* option_name, const char* arg, const char** retval,
	      const char* choices[], int num_choices)
{
  for (int i = 0; i < num_choices; i++)
    if (strcmp(choices[i], arg) == 0)
      {
	*retval = arg;
	return;
      }

  // If we get here, the user did not enter a valid choice, so we die.
  std::string choices_list;
  for (int i = 0; i < num_choices; i++)
    {
      choices_list += choices[i];
      if (i != num_choices - 1)
	choices_list += ", ";
    }
  gold_fatal(_("%s: must take one of the following arguments: %s"),
	     option_name, choices_list.c_str());
}

} // End namespace options.

// Define the handler for "special" options (set via DEFINE_special).

void
General_options::parse_help(const char*, const char*, Command_line*)
{
  options::help();
  ::exit(EXIT_SUCCESS);
}

void
General_options::parse_version(const char* opt, const char*, Command_line*)
{
  bool print_short = (opt[0] == '-' && opt[1] == 'v');
  gold::print_version(print_short);
  this->printed_version_ = true;
  if (!print_short)
    ::exit(EXIT_SUCCESS);
}

void
General_options::parse_V(const char*, const char*, Command_line*)
{
  gold::print_version(true);
  this->printed_version_ = true;

  printf(_("  Supported targets:\n"));
  std::vector<const char*> supported_names;
  gold::supported_target_names(&supported_names);
  for (std::vector<const char*>::const_iterator p = supported_names.begin();
       p != supported_names.end();
       ++p)
    printf("   %s\n", *p);

  printf(_("  Supported emulations:\n"));
  supported_names.clear();
  gold::supported_emulation_names(&supported_names);
  for (std::vector<const char*>::const_iterator p = supported_names.begin();
       p != supported_names.end();
       ++p)
    printf("   %s\n", *p);
}

void
General_options::parse_Bno_symbolic(const char*, const char*,
				    Command_line*)
{
  this->bsymbolic_ = BSYMBOLIC_NONE;
}

void
General_options::parse_Bsymbolic_functions(const char*, const char*,
					   Command_line*)
{
  this->bsymbolic_ = BSYMBOLIC_FUNCTIONS;
}

void
General_options::parse_Bsymbolic(const char*, const char*,
				 Command_line*)
{
  this->bsymbolic_ = BSYMBOLIC_ALL;
}

void
General_options::parse_defsym(const char*, const char* arg,
			      Command_line* cmdline)
{
  cmdline->script_options().define_symbol(arg);
}

void
General_options::parse_discard_all(const char*, const char*,
				   Command_line*)
{
  this->discard_locals_ = DISCARD_ALL;
}

void
General_options::parse_discard_locals(const char*, const char*,
				      Command_line*)
{
  this->discard_locals_ = DISCARD_LOCALS;
}

void
General_options::parse_discard_none(const char*, const char*,
				    Command_line*)
{
  this->discard_locals_ = DISCARD_NONE;
}

void
General_options::parse_incremental(const char*, const char*,
				   Command_line*)
{
  this->incremental_mode_ = INCREMENTAL_AUTO;
}

void
General_options::parse_no_incremental(const char*, const char*,
				      Command_line*)
{
  this->incremental_mode_ = INCREMENTAL_OFF;
}

void
General_options::parse_incremental_full(const char*, const char*,
					Command_line*)
{
  this->incremental_mode_ = INCREMENTAL_FULL;
}

void
General_options::parse_incremental_update(const char*, const char*,
					  Command_line*)
{
  this->incremental_mode_ = INCREMENTAL_UPDATE;
}

void
General_options::parse_incremental_changed(const char*, const char*,
					   Command_line*)
{
  this->implicit_incremental_ = true;
  this->incremental_disposition_ = INCREMENTAL_CHANGED;
}

void
General_options::parse_incremental_unchanged(const char*, const char*,
					     Command_line*)
{
  this->implicit_incremental_ = true;
  this->incremental_disposition_ = INCREMENTAL_UNCHANGED;
}

void
General_options::parse_incremental_unknown(const char*, const char*,
					   Command_line*)
{
  this->implicit_incremental_ = true;
  this->incremental_disposition_ = INCREMENTAL_CHECK;
}

void
General_options::parse_incremental_startup_unchanged(const char*, const char*,
						     Command_line*)
{
  this->implicit_incremental_ = true;
  this->incremental_startup_disposition_ = INCREMENTAL_UNCHANGED;
}

void
General_options::parse_library(const char*, const char* arg,
			       Command_line* cmdline)
{
  Input_file_argument::Input_file_type type;
  const char* name;
  if (arg[0] == ':')
    {
      type = Input_file_argument::INPUT_FILE_TYPE_SEARCHED_FILE;
      name = arg + 1;
    }
  else
    {
      type = Input_file_argument::INPUT_FILE_TYPE_LIBRARY;
      name = arg;
    }
  Input_file_argument file(name, type, "", false, *this);
  cmdline->inputs().add_file(file);
}

void
General_options::parse_plugin(const char*, const char* arg,
			      Command_line*)
{
  this->add_plugin(arg);
}

// Parse --plugin-opt.

void
General_options::parse_plugin_opt(const char*, const char* arg,
				  Command_line*)
{
  this->add_plugin_option(arg);
}

void
General_options::parse_no_power10_stubs(const char*, const char*,
					Command_line*)
{
  this->set_power10_stubs("no");
  this->set_user_set_power10_stubs();
}

void
General_options::parse_R(const char* option, const char* arg,
			 Command_line* cmdline)
{
  struct stat s;
  if (::stat(arg, &s) != 0 || S_ISDIR(s.st_mode))
    this->add_to_rpath(arg);
  else
    this->parse_just_symbols(option, arg, cmdline);
}

void
General_options::parse_just_symbols(const char*, const char* arg,
				    Command_line* cmdline)
{
  Input_file_argument file(arg, Input_file_argument::INPUT_FILE_TYPE_FILE,
			   "", true, *this);
  cmdline->inputs().add_file(file);
}

// Handle --section-start.

void
General_options::parse_section_start(const char*, const char* arg,
				     Command_line*)
{
  const char* eq = strchr(arg, '=');
  if (eq == NULL)
    {
      gold_error(_("invalid argument to --section-start; "
		   "must be SECTION=ADDRESS"));
      return;
    }

  std::string section_name(arg, eq - arg);

  ++eq;
  const char* val_start = eq;
  if (eq[0] == '0' && (eq[1] == 'x' || eq[1] == 'X'))
    eq += 2;
  if (*eq == '\0')
    {
      gold_error(_("--section-start address missing"));
      return;
    }
  uint64_t addr = 0;
  hex_init();
  for (; *eq != '\0'; ++eq)
    {
      if (!hex_p(*eq))
	{
	  gold_error(_("--section-start argument %s is not a valid hex number"),
		     val_start);
	  return;
	}
      addr <<= 4;
      addr += hex_value(*eq);
    }

  this->section_starts_[section_name] = addr;
}

// Look up a --section-start value.

bool
General_options::section_start(const char* secname, uint64_t* paddr) const
{
  if (this->section_starts_.empty())
    return false;
  std::map<std::string, uint64_t>::const_iterator p =
    this->section_starts_.find(secname);
  if (p == this->section_starts_.end())
    return false;
  *paddr = p->second;
  return true;
}

void
General_options::parse_static(const char*, const char*, Command_line*)
{
  this->set_static(true);
}

void
General_options::parse_script(const char*, const char* arg,
			      Command_line* cmdline)
{
  if (!read_commandline_script(arg, cmdline))
    gold::gold_fatal(_("unable to parse script file %s"), arg);
}

void
General_options::parse_version_script(const char*, const char* arg,
				      Command_line* cmdline)
{
  if (!read_version_script(arg, cmdline))
    gold::gold_fatal(_("unable to parse version script file %s"), arg);
}

void
General_options::parse_dynamic_list(const char*, const char* arg,
				    Command_line* cmdline)
{
  if (!read_dynamic_list(arg, cmdline, &this->dynamic_list_))
    gold::gold_fatal(_("unable to parse dynamic-list script file %s"), arg);
  this->have_dynamic_list_ = true;
}

void
General_options::parse_start_group(const char*, const char*,
				   Command_line* cmdline)
{
  cmdline->inputs().start_group();
}

void
General_options::parse_end_group(const char*, const char*,
				 Command_line* cmdline)
{
  cmdline->inputs().end_group();
}

void
General_options::parse_start_lib(const char*, const char*,
				 Command_line* cmdline)
{
  cmdline->inputs().start_lib(cmdline->position_dependent_options());
}

void
General_options::parse_end_lib(const char*, const char*,
			       Command_line* cmdline)
{
  cmdline->inputs().end_lib();
}

// The function add_excluded_libs() in ld/ldlang.c of GNU ld breaks up a list
// of names separated by commas or colons and puts them in a linked list.
// We implement the same parsing of names here but store names in an unordered
// map to speed up searching of names.

void
General_options::parse_exclude_libs(const char*, const char* arg,
				    Command_line*)
{
  const char* p = arg;

  while (*p != '\0')
    {
      size_t length = strcspn(p, ",:");
      this->excluded_libs_.insert(std::string(p, length));
      p += (p[length] ? length + 1 : length);
    }
}

// The checking logic is based on the function check_excluded_libs() in
// ld/ldlang.c of GNU ld but our implementation is different because we use
// an unordered map instead of a linked list, which is what GNU ld uses.  GNU
// ld searches sequentially in the excluded libs list.  For a given archive,
// a match is found if the archive's name matches exactly one of the list
// entry or if the archive's name is of the form FOO.a and FOO matches exactly
// one of the list entry.  An entry "ALL" in the list is considered as a
// wild-card and matches any given name.

bool
General_options::check_excluded_libs(const std::string &name) const
{
  Unordered_set<std::string>::const_iterator p;

  // Exit early for the most common case.
  if (excluded_libs_.empty())
    return false;

  // If we see "ALL", all archives are excluded from automatic export.
  p = excluded_libs_.find(std::string("ALL"));
  if (p != excluded_libs_.end())
    return true;

  // First strip off any directories in name.
  const char* basename = lbasename(name.c_str());

  // Try finding an exact match.
  p = excluded_libs_.find(std::string(basename));
  if (p != excluded_libs_.end())
    return true;

  // Try matching NAME without ".a" at the end.
  size_t length = strlen(basename);
  if ((length >= 2)
      && (basename[length - 2] == '.')
      && (basename[length - 1] == 'a'))
    {
      p = excluded_libs_.find(std::string(basename, length - 2));
      if (p != excluded_libs_.end())
	return true;
    }

  return false;
}

// Recognize input and output target names.  The GNU linker accepts
// these with --format and --oformat.  This code is intended to be
// minimally compatible.  In practice for an ELF target this would be
// the same target as the input files; that name always start with
// "elf".  Non-ELF targets would be "srec", "symbolsrec", "tekhex",
// "binary", "ihex".

General_options::Object_format
General_options::string_to_object_format(const char* arg)
{
  if (strncmp(arg, "elf", 3) == 0 || strcmp(arg, "default") == 0)
    return gold::General_options::OBJECT_FORMAT_ELF;
  else if (strcmp(arg, "binary") == 0)
    return gold::General_options::OBJECT_FORMAT_BINARY;
  else
    {
      gold::gold_error(_("format '%s' not supported; treating as elf "
			 "(supported formats: elf, binary)"),
		       arg);
      return gold::General_options::OBJECT_FORMAT_ELF;
    }
}

const char*
General_options::object_format_to_string(General_options::Object_format fmt)
{
  switch (fmt)
    {
    case General_options::OBJECT_FORMAT_ELF:
      return "elf";
    case General_options::OBJECT_FORMAT_BINARY:
      return "binary";
    default:
      gold_unreachable();
    }
}

void
General_options::parse_fix_v4bx(const char*, const char*,
				Command_line*)
{
  this->fix_v4bx_ = FIX_V4BX_REPLACE;
}

void
General_options::parse_fix_v4bx_interworking(const char*, const char*,
					     Command_line*)
{
  this->fix_v4bx_ = FIX_V4BX_INTERWORKING;
}

void
General_options::parse_EB(const char*, const char*, Command_line*)
{
  this->endianness_ = ENDIANNESS_BIG;
}

void
General_options::parse_EL(const char*, const char*, Command_line*)
{
  this->endianness_ = ENDIANNESS_LITTLE;
}

void
General_options::copy_from_posdep_options(
    const Position_dependent_options& posdep)
{
  this->set_as_needed(posdep.as_needed());
  this->set_Bdynamic(posdep.Bdynamic());
  this->set_format(
      General_options::object_format_to_string(posdep.format_enum()));
  this->set_whole_archive(posdep.whole_archive());
  this->set_incremental_disposition(posdep.incremental_disposition());
}

void
General_options::parse_push_state(const char*, const char*, Command_line*)
{
  Position_dependent_options* posdep = new Position_dependent_options(*this);
  this->options_stack_.push_back(posdep);
}

void
General_options::parse_pop_state(const char*, const char*, Command_line*)
{
  if (this->options_stack_.empty())
    {
      gold::gold_error(_("unbalanced --push-state/--pop-state"));
      return;
    }
  Position_dependent_options* posdep = this->options_stack_.back();
  this->options_stack_.pop_back();
  this->copy_from_posdep_options(*posdep);
  delete posdep;
}

} // End namespace gold.

namespace
{

void
usage()
{
  fprintf(stderr,
	  _("%s: use the --help option for usage information\n"),
	  gold::program_name);
  ::exit(EXIT_FAILURE);
}

void
usage(const char* msg, const char* opt)
{
  fprintf(stderr,
	  _("%s: %s: %s\n"),
	  gold::program_name, opt, msg);
  usage();
}

// If the default sysroot is relocatable, try relocating it based on
// the prefix FROM.

static char*
get_relative_sysroot(const char* from)
{
  char* path = make_relative_prefix(gold::program_name, from,
				    TARGET_SYSTEM_ROOT);
  if (path != NULL)
    {
      struct stat s;
      if (::stat(path, &s) == 0 && S_ISDIR(s.st_mode))
	return path;
      free(path);
    }

  return NULL;
}

// Return the default sysroot.  This is set by the --with-sysroot
// option to configure.  Note we do not free the return value of
// get_relative_sysroot, which is a small memory leak, but is
// necessary since we store this pointer directly in General_options.

static const char*
get_default_sysroot()
{
  const char* sysroot = TARGET_SYSTEM_ROOT;
  if (*sysroot == '\0')
    return NULL;

  if (TARGET_SYSTEM_ROOT_RELOCATABLE)
    {
      char* path = get_relative_sysroot(BINDIR);
      if (path == NULL)
	path = get_relative_sysroot(TOOLBINDIR);
      if (path != NULL)
	return path;
    }

  return sysroot;
}

// Parse a long option.  Such options have the form
// <-|--><option>[=arg].  If "=arg" is not present but the option
// takes an argument, the next word is taken to the be the argument.
// If equals_only is set, then only the <option>=<arg> form is
// accepted, not the <option><space><arg> form.  Returns a One_option
// struct or NULL if argv[i] cannot be parsed as a long option.  In
// the not-NULL case, *arg is set to the option's argument (NULL if
// the option takes no argument), and *i is advanced past this option.
// NOTE: it is safe for argv and arg to point to the same place.
gold::options::One_option*
parse_long_option(int argc, const char** argv, bool equals_only,
		  const char** arg, int* i)
{
  const char* const this_argv = argv[*i];

  const char* equals = strchr(this_argv, '=');
  const char* option_start = this_argv + strspn(this_argv, "-");
  std::string option(option_start,
		     equals ? equals - option_start : strlen(option_start));

  gold::options::Option_map::iterator it
      = gold::options::long_options->find(option);
  if (it == gold::options::long_options->end())
    return NULL;

  gold::options::One_option* retval = it->second;

  // If the dash-count doesn't match, we fail.
  if (this_argv[0] != '-')  // no dashes at all: had better be "-z <longopt>"
    {
      if (retval->dashes != gold::options::DASH_Z)
	return NULL;
    }
  else if (this_argv[1] != '-')   // one dash
    {
      if (retval->dashes != gold::options::ONE_DASH
	  && retval->dashes != gold::options::EXACTLY_ONE_DASH
	  && retval->dashes != gold::options::TWO_DASHES)
	return NULL;
    }
  else                            // two dashes (or more!)
    {
      if (retval->dashes != gold::options::TWO_DASHES
	  && retval->dashes != gold::options::EXACTLY_TWO_DASHES
	  && retval->dashes != gold::options::ONE_DASH)
	return NULL;
    }

  // Now that we know the option is good (or else bad in a way that
  // will cause us to die), increment i to point past this argv.
  ++(*i);

  // Figure out the option's argument, if any.
  if (!retval->takes_argument())
    {
      if (equals)
	usage(_("unexpected argument"), this_argv);
      else
	*arg = NULL;
    }
  else
    {
      if (equals)
	*arg = equals + 1;
      else if (retval->takes_optional_argument())
	*arg = retval->default_value;
      else if (*i < argc && !equals_only)
	*arg = argv[(*i)++];
      else
	usage(_("missing argument"), this_argv);
    }

  return retval;
}

// Parse a short option.  Such options have the form -<option>[arg].
// If "arg" is not present but the option takes an argument, the next
// word is taken to the be the argument.  If the option does not take
// an argument, it may be followed by another short option.  Returns a
// One_option struct or NULL if argv[i] cannot be parsed as a short
// option.  In the not-NULL case, *arg is set to the option's argument
// (NULL if the option takes no argument), and *i is advanced past
// this option.  This function keeps *i the same if we parsed a short
// option that does not take an argument, that looks to be followed by
// another short option in the same word.
gold::options::One_option*
parse_short_option(int argc, const char** argv, int pos_in_argv_i,
		   const char** arg, int* i)
{
  const char* const this_argv = argv[*i];

  if (this_argv[0] != '-')
    return NULL;

  // We handle -z as a special case.
  static gold::options::One_option dash_z("", gold::options::DASH_Z,
					  'z', "", NULL, "Z-OPTION", false,
					  NULL, false);
  gold::options::One_option* retval = NULL;
  if (this_argv[pos_in_argv_i] == 'z')
    retval = &dash_z;
  else
    {
      const int char_as_int = static_cast<int>(this_argv[pos_in_argv_i]);
      if (char_as_int > 0 && char_as_int < 128)
	retval = gold::options::short_options[char_as_int];
    }

  if (retval == NULL)
    return NULL;

  // Figure out the option's argument, if any.
  if (!retval->takes_argument())
    {
      *arg = NULL;
      // We only advance past this argument if it's the only one in argv.
      if (this_argv[pos_in_argv_i + 1] == '\0')
	++(*i);
    }
  else
    {
      // If we take an argument, we'll eat up this entire argv entry.
      ++(*i);
      if (this_argv[pos_in_argv_i + 1] != '\0')
	*arg = this_argv + pos_in_argv_i + 1;
      else if (retval->takes_optional_argument())
	*arg = retval->default_value;
      else if (*i < argc)
	*arg = argv[(*i)++];
      else
	usage(_("missing argument"), this_argv);
    }

  // If we're a -z option, we need to parse our argument as a
  // long-option, e.g. "-z stacksize=8192".
  if (retval == &dash_z)
    {
      int dummy_i = 0;
      const char* dash_z_arg = *arg;
      retval = parse_long_option(1, arg, true, arg, &dummy_i);
      if (retval == NULL)
	usage(_("unknown -z option"), dash_z_arg);
    }

  return retval;
}

} // End anonymous namespace.

namespace gold
{

General_options::General_options()
  : bsymbolic_(BSYMBOLIC_NONE),
    printed_version_(false),
    execstack_status_(EXECSTACK_FROM_INPUT),
    icf_status_(ICF_NONE),
    static_(false),
    do_demangle_(false),
    plugins_(NULL),
    dynamic_list_(),
    have_dynamic_list_(false),
    incremental_mode_(INCREMENTAL_OFF),
    incremental_disposition_(INCREMENTAL_STARTUP),
    incremental_startup_disposition_(INCREMENTAL_CHECK),
    implicit_incremental_(false),
    excluded_libs_(),
    symbols_to_retain_(),
    section_starts_(),
    fix_v4bx_(FIX_V4BX_NONE),
    endianness_(ENDIANNESS_NOT_SET),
    discard_locals_(DISCARD_SEC_MERGE),
    orphan_handling_enum_(ORPHAN_PLACE),
    start_stop_visibility_enum_(elfcpp::STV_PROTECTED)
{
  // Turn off option registration once construction is complete.
  gold::options::ready_to_register = false;
}

General_options::Object_format
General_options::format_enum() const
{
  return General_options::string_to_object_format(this->format());
}

General_options::Object_format
General_options::oformat_enum() const
{
  return General_options::string_to_object_format(this->oformat());
}

// Add the sysroot, if any, to the search paths.

void
General_options::add_sysroot()
{
  if (this->sysroot() == NULL || this->sysroot()[0] == '\0')
    {
      this->set_sysroot(get_default_sysroot());
      if (this->sysroot() == NULL || this->sysroot()[0] == '\0')
	return;
    }

  char* canonical_sysroot = lrealpath(this->sysroot());

  for (Dir_list::iterator p = this->library_path_.value.begin();
       p != this->library_path_.value.end();
       ++p)
    p->add_sysroot(this->sysroot(), canonical_sysroot);

  free(canonical_sysroot);
}

// Return whether FILENAME is in a system directory.

bool
General_options::is_in_system_directory(const std::string& filename) const
{
  for (Dir_list::const_iterator p = this->library_path_.value.begin();
       p != this->library_path_.value.end();
       ++p)
    {
      // We use a straight string comparison rather than calling
      // FILENAME_CMP because we are only interested in the cases
      // where we found the file in a system directory, which means
      // that we used the directory name as a prefix for a -L search.
      if (p->is_system_directory()
	  && filename.compare(0, p->name().size(), p->name()) == 0)
	return true;
    }
  return false;
}

// Add a plugin to the list of plugins.

void
General_options::add_plugin(const char* filename)
{
  if (this->plugins_ == NULL)
    this->plugins_ = new Plugin_manager(*this);
  this->plugins_->add_plugin(filename);
}

// Add a plugin option to a plugin.

void
General_options::add_plugin_option(const char* arg)
{
  if (this->plugins_ == NULL)
    gold_fatal("--plugin-opt requires --plugin.");
  this->plugins_->add_plugin_option(arg);
}

// Set up variables and other state that isn't set up automatically by
// the parse routine, and ensure options don't contradict each other
// and are otherwise kosher.

void
General_options::finalize()
{
  // Normalize the strip modifiers.  They have a total order:
  // strip_all > strip_debug > strip_non_line > strip_debug_gdb.
  // If one is true, set all beneath it to true as well.
  if (this->strip_all())
    this->set_strip_debug(true);
  if (this->strip_debug())
    this->set_strip_debug_non_line(true);
  if (this->strip_debug_non_line())
    this->set_strip_debug_gdb(true);

  if (this->Bshareable())
    this->set_shared(true);

  // If the user specifies both -s and -r, convert the -s to -S.
  // -r requires us to keep externally visible symbols!
  if (this->strip_all() && this->relocatable())
    {
      this->set_strip_all(false);
      gold_assert(this->strip_debug());
    }

  // For us, -dc and -dp are synonyms for --define-common.
  if (this->dc())
    this->set_define_common(true);
  if (this->dp())
    this->set_define_common(true);

  // We also set --define-common if we're not relocatable, as long as
  // the user didn't explicitly ask for something different.
  if (!this->user_set_define_common())
    this->set_define_common(!this->relocatable());

  // execstack_status_ is a three-state variable; update it based on
  // -z [no]execstack.
  if (this->execstack())
    this->set_execstack_status(EXECSTACK_YES);
  else if (this->noexecstack())
    this->set_execstack_status(EXECSTACK_NO);

  // icf_status_ is a three-state variable; update it based on the
  // value of this->icf().
  if (strcmp(this->icf(), "none") == 0)
    this->set_icf_status(ICF_NONE);
  else if (strcmp(this->icf(), "safe") == 0)
    this->set_icf_status(ICF_SAFE);
  else
    this->set_icf_status(ICF_ALL);

  // Handle the optional argument for --demangle.
  if (this->user_set_demangle())
    {
      this->set_do_demangle(true);
      const char* style = this->demangle();
      if (*style != '\0')
	{
	  enum demangling_styles style_code;

	  style_code = cplus_demangle_name_to_style(style);
	  if (style_code == unknown_demangling)
	    gold_fatal("unknown demangling style '%s'", style);
	  cplus_demangle_set_style(style_code);
	}
    }
  else if (this->user_set_no_demangle())
    this->set_do_demangle(false);
  else
    {
      // Testing COLLECT_NO_DEMANGLE makes our default demangling
      // behaviour identical to that of gcc's linker wrapper.
      this->set_do_demangle(getenv("COLLECT_NO_DEMANGLE") == NULL);
    }

  // Parse the --orphan-handling argument.
  if (this->user_set_orphan_handling())
    {
      if (strcmp(this->orphan_handling(), "place") == 0)
        this->set_orphan_handling_enum(ORPHAN_PLACE);
      else if (strcmp(this->orphan_handling(), "discard") == 0)
        this->set_orphan_handling_enum(ORPHAN_DISCARD);
      else if (strcmp(this->orphan_handling(), "warn") == 0)
        this->set_orphan_handling_enum(ORPHAN_WARN);
      else if (strcmp(this->orphan_handling(), "error") == 0)
        this->set_orphan_handling_enum(ORPHAN_ERROR);
    }

  // Parse the -z start-stop-visibility argument.
  if (this->user_set_start_stop_visibility())
    {
      if (strcmp(this->start_stop_visibility(), "default") == 0)
        this->set_start_stop_visibility_enum(elfcpp::STV_DEFAULT);
      else if (strcmp(this->start_stop_visibility(), "internal") == 0)
        this->set_start_stop_visibility_enum(elfcpp::STV_INTERNAL);
      else if (strcmp(this->start_stop_visibility(), "hidden") == 0)
        this->set_start_stop_visibility_enum(elfcpp::STV_HIDDEN);
      else if (strcmp(this->start_stop_visibility(), "protected") == 0)
        this->set_start_stop_visibility_enum(elfcpp::STV_PROTECTED);
    }

  // Parse the --power10-stubs argument.
  if (!this->user_set_power10_stubs())
    {
      // --power10-stubs without an arg is equivalent to --power10-stubs=yes
      // but not specifying --power10-stubs at all should be equivalent to
      // --power10-stubs=auto.  This doesn't fit into the notion of
      // "default_value", used both as a static initializer and to provide
      // a missing optional arg.  Fix it here.
      this->set_power10_stubs("auto");
      this->set_power10_stubs_enum(POWER10_STUBS_AUTO);
    }
  else
    {
      if (strcmp(this->power10_stubs(), "auto") == 0)
	this->set_power10_stubs_enum(POWER10_STUBS_AUTO);
      else if (strcmp(this->power10_stubs(), "no") == 0)
	this->set_power10_stubs_enum(POWER10_STUBS_NO);
      else if (strcmp(this->power10_stubs(), "yes") == 0)
	this->set_power10_stubs_enum(POWER10_STUBS_YES);
    }

  // -M is equivalent to "-Map -".
  if (this->print_map() && !this->user_set_Map())
    {
      this->set_Map("-");
      this->set_user_set_Map();
    }

  // Using -n or -N implies -static.
  if (this->nmagic() || this->omagic())
    this->set_static(true);

  // If --thread_count is specified, it applies to
  // --thread-count-{initial,middle,final}, though it doesn't override
  // them.
  if (this->thread_count() > 0 && this->thread_count_initial() == 0)
    this->set_thread_count_initial(this->thread_count());
  if (this->thread_count() > 0 && this->thread_count_middle() == 0)
    this->set_thread_count_middle(this->thread_count());
  if (this->thread_count() > 0 && this->thread_count_final() == 0)
    this->set_thread_count_final(this->thread_count());

  // Let's warn if you set the thread-count but we're going to ignore it.
#ifndef ENABLE_THREADS
  if (this->threads())
    {
      gold_warning(_("ignoring --threads: "
		     "%s was compiled without thread support"),
		   program_name);
      this->set_threads(false);
    }
  if (this->thread_count() > 0 || this->thread_count_initial() > 0
      || this->thread_count_middle() > 0 || this->thread_count_final() > 0)
    gold_warning(_("ignoring --thread-count: "
		   "%s was compiled without thread support"),
		 program_name);
#endif

#ifndef ENABLE_PLUGINS
  if (this->has_plugins())
    gold_fatal(_("cannot use --plugin: "
		 "%s was compiled without plugin support"),
	       program_name);
#endif

  std::string libpath;
  if (this->user_set_Y())
    {
      libpath = this->Y();
      if (libpath.compare(0, 2, "P,") == 0)
	libpath.erase(0, 2);
    }
  else if (!this->nostdlib())
    {
#ifndef NATIVE_LINKER
#define NATIVE_LINKER 0
#endif
      const char* p = LIB_PATH;
      if (strcmp(p, "::DEFAULT::") != 0)
	libpath = p;
      else if (NATIVE_LINKER
	       || this->user_set_sysroot()
	       || *TARGET_SYSTEM_ROOT != '\0')
	{
	  this->add_to_library_path_with_sysroot("/lib");
	  this->add_to_library_path_with_sysroot("/usr/lib");
	}
      else
	this->add_to_library_path_with_sysroot(TOOLLIBDIR);
    }

  if (!libpath.empty())
    {
      size_t pos = 0;
      size_t next_pos;
      do
	{
	  next_pos = libpath.find(':', pos);
	  size_t len = (next_pos == std::string::npos
			? next_pos
			: next_pos - pos);
	  if (len != 0)
	    this->add_to_library_path_with_sysroot(libpath.substr(pos, len));
	  pos = next_pos + 1;
	}
      while (next_pos != std::string::npos);
    }

  // Parse the contents of -retain-symbols-file into a set.
  if (this->retain_symbols_file())
    {
      std::ifstream in;
      in.open(this->retain_symbols_file());
      if (!in)
	gold_fatal(_("unable to open -retain-symbols-file file %s: %s"),
		   this->retain_symbols_file(), strerror(errno));
      std::string line;
      std::getline(in, line);   // this chops off the trailing \n, if any
      while (in)
	{
	  if (!line.empty() && line[line.length() - 1] == '\r')   // Windows
	    line.resize(line.length() - 1);
	  this->symbols_to_retain_.insert(line);
	  std::getline(in, line);
	}
    }

  // -Bgroup implies --unresolved-symbols=report-all.
  if (this->Bgroup() && !this->user_set_unresolved_symbols())
    this->set_unresolved_symbols("report-all");

  // -shared implies --allow-shlib-undefined.  Currently
  // ---allow-shlib-undefined controls warnings issued based on the
  // -symbol table.  --unresolved-symbols controls warnings issued
  // -based on relocations.
  if (this->shared() && !this->user_set_allow_shlib_undefined())
    this->set_allow_shlib_undefined(true);

  // Normalize library_path() by adding the sysroot to all directories
  // in the path, as appropriate.
  this->add_sysroot();

  // Now that we've normalized the options, check for contradictory ones.
  if (this->shared() && this->is_static())
    gold_fatal(_("-shared and -static are incompatible"));
  if (this->shared() && this->pie())
    gold_fatal(_("-shared and -pie are incompatible"));
  if (this->pie() && this->is_static())
    gold_fatal(_("-pie and -static are incompatible"));

  if (this->shared() && this->relocatable())
    gold_fatal(_("-shared and -r are incompatible"));
  if (this->pie() && this->relocatable())
    gold_fatal(_("-pie and -r are incompatible"));

  if (!this->shared())
    {
      if (this->filter() != NULL)
	gold_fatal(_("-F/--filter may not used without -shared"));
      if (this->any_auxiliary())
	gold_fatal(_("-f/--auxiliary may not be used without -shared"));
    }

  // TODO: implement support for -retain-symbols-file with -r, if needed.
  if (this->relocatable() && this->retain_symbols_file())
    gold_fatal(_("-retain-symbols-file does not yet work with -r"));

  if (this->oformat_enum() != General_options::OBJECT_FORMAT_ELF
      && (this->shared()
	  || this->pie()
	  || this->relocatable()))
    gold_fatal(_("binary output format not compatible "
		 "with -shared or -pie or -r"));

  if (this->user_set_hash_bucket_empty_fraction()
      && (this->hash_bucket_empty_fraction() < 0.0
	  || this->hash_bucket_empty_fraction() >= 1.0))
    gold_fatal(_("--hash-bucket-empty-fraction value %g out of range "
		 "[0.0, 1.0)"),
	       this->hash_bucket_empty_fraction());

  if (this->implicit_incremental_ && this->incremental_mode_ == INCREMENTAL_OFF)
    gold_fatal(_("Options --incremental-changed, --incremental-unchanged, "
		 "--incremental-unknown require the use of --incremental"));

  // Check for options that are not compatible with incremental linking.
  // Where an option can be disabled without seriously changing the semantics
  // of the link, we turn the option off; otherwise, we issue a fatal error.

  if (this->incremental_mode_ != INCREMENTAL_OFF)
    {
      if (this->relocatable())
	gold_fatal(_("incremental linking is not compatible with -r"));
      if (this->emit_relocs())
	gold_fatal(_("incremental linking is not compatible with "
		     "--emit-relocs"));
      if (this->has_plugins())
	gold_fatal(_("incremental linking is not compatible with --plugin"));
      if (this->relro())
	gold_fatal(_("incremental linking is not compatible with -z relro"));
      if (this->pie())
	gold_fatal(_("incremental linking is not compatible with -pie"));
      if (this->gc_sections())
	{
	  gold_warning(_("ignoring --gc-sections for an incremental link"));
	  this->set_gc_sections(false);
	}
      if (this->icf_enabled())
	{
	  gold_warning(_("ignoring --icf for an incremental link"));
	  this->set_icf_status(ICF_NONE);
	}
      if (strcmp(this->compress_debug_sections(), "none") != 0)
	{
	  gold_warning(_("ignoring --compress-debug-sections for an "
			 "incremental link"));
	  this->set_compress_debug_sections("none");
	}
    }

#ifndef HAVE_ZSTD
  if (strcmp(this->compress_debug_sections(), "zstd") == 0)
    {
      gold_error(_("--compress-debug-sections=zstd: gold is not built with "
		   "zstd support"));
      this->set_compress_debug_sections("none");
    }
#endif

  // --rosegment-gap implies --rosegment.
  if (this->user_set_rosegment_gap())
    this->set_rosegment(true);

  // FIXME: we can/should be doing a lot more sanity checking here.
}

// Search_directory methods.

// This is called if we have a sysroot.  Apply the sysroot if
// appropriate.  Record whether the directory is in the sysroot.

void
Search_directory::add_sysroot(const char* sysroot,
			      const char* canonical_sysroot)
{
  gold_assert(*sysroot != '\0');
  if (this->put_in_sysroot_)
    {
      if (!IS_DIR_SEPARATOR(this->name_[0])
	  && !IS_DIR_SEPARATOR(sysroot[strlen(sysroot) - 1]))
	this->name_ = '/' + this->name_;
      this->name_ = sysroot + this->name_;
      this->is_in_sysroot_ = true;
    }
  else
    {
      // Check whether this entry is in the sysroot.  To do this
      // correctly, we need to use canonical names.  Otherwise we will
      // get confused by the ../../.. paths that gcc tends to use.
      char* canonical_name = lrealpath(this->name_.c_str());
      int canonical_name_len = strlen(canonical_name);
      int canonical_sysroot_len = strlen(canonical_sysroot);
      if (canonical_name_len > canonical_sysroot_len
	  && IS_DIR_SEPARATOR(canonical_name[canonical_sysroot_len]))
	{
	  canonical_name[canonical_sysroot_len] = '\0';
	  if (FILENAME_CMP(canonical_name, canonical_sysroot) == 0)
	    this->is_in_sysroot_ = true;
	}
      free(canonical_name);
    }
}

// Input_arguments methods.

// Add a file to the list.

Input_argument&
Input_arguments::add_file(Input_file_argument& file)
{
  file.set_arg_serial(++this->file_count_);
  if (this->in_group_)
    {
      gold_assert(!this->input_argument_list_.empty());
      gold_assert(this->input_argument_list_.back().is_group());
      return this->input_argument_list_.back().group()->add_file(file);
    }
  if (this->in_lib_)
    {
      gold_assert(!this->input_argument_list_.empty());
      gold_assert(this->input_argument_list_.back().is_lib());
      return this->input_argument_list_.back().lib()->add_file(file);
    }
  this->input_argument_list_.push_back(Input_argument(file));
  return this->input_argument_list_.back();
}

// Start a group.

void
Input_arguments::start_group()
{
  if (this->in_group_)
    gold_fatal(_("May not nest groups"));
  if (this->in_lib_)
    gold_fatal(_("may not nest groups in libraries"));
  Input_file_group* group = new Input_file_group();
  this->input_argument_list_.push_back(Input_argument(group));
  this->in_group_ = true;
}

// End a group.

void
Input_arguments::end_group()
{
  if (!this->in_group_)
    gold_fatal(_("Group end without group start"));
  this->in_group_ = false;
}

// Start a lib.

void
Input_arguments::start_lib(const Position_dependent_options& options)
{
  if (this->in_lib_)
    gold_fatal(_("may not nest libraries"));
  if (this->in_group_)
    gold_fatal(_("may not nest libraries in groups"));
  Input_file_lib* lib = new Input_file_lib(options);
  this->input_argument_list_.push_back(Input_argument(lib));
  this->in_lib_ = true;
}

// End a lib.

void
Input_arguments::end_lib()
{
  if (!this->in_lib_)
    gold_fatal(_("lib end without lib start"));
  this->in_lib_ = false;
}

// Command_line options.

Command_line::Command_line()
{
}

// Pre_options is the hook that sets the ready_to_register flag.

Command_line::Pre_options::Pre_options()
{
  gold::options::ready_to_register = true;
}

// Process the command line options.  For process_one_option, i is the
// index of argv to process next, and must be an option (that is,
// start with a dash).  The return value is the index of the next
// option to process (i+1 or i+2, or argc to indicate processing is
// done).  no_more_options is set to true if (and when) "--" is seen
// as an option.

int
Command_line::process_one_option(int argc, const char** argv, int i,
				 bool* no_more_options)
{
  gold_assert(argv[i][0] == '-' && !(*no_more_options));

  // If we are reading "--", then just set no_more_options and return.
  if (argv[i][1] == '-' && argv[i][2] == '\0')
    {
      *no_more_options = true;
      return i + 1;
    }

  int new_i = i;
  options::One_option* option = NULL;
  const char* arg = NULL;

  // First, try to process argv as a long option.
  option = parse_long_option(argc, argv, false, &arg, &new_i);
  if (option)
    {
      option->reader->parse_to_value(argv[i], arg, this, &this->options_);
      return new_i;
    }

  // Now, try to process argv as a short option.  Since several short
  // options can be combined in one argv, we may have to parse a lot
  // until we're done reading this argv.
  int pos_in_argv_i = 1;
  while (new_i == i)
    {
      option = parse_short_option(argc, argv, pos_in_argv_i, &arg, &new_i);
      if (!option)
	break;
      option->reader->parse_to_value(argv[i], arg, this, &this->options_);
      ++pos_in_argv_i;
    }
  if (option)
    return new_i;

  // I guess it's neither a long option nor a short option.
  usage(_("unknown option"), argv[i]);
  return argc;
}


void
Command_line::process(int argc, const char** argv)
{
  bool no_more_options = false;
  int i = 0;
  while (i < argc)
    {
      this->position_options_.copy_from_options(this->options());
      if (no_more_options || argv[i][0] != '-')
	{
	  Input_file_argument file(argv[i],
				   Input_file_argument::INPUT_FILE_TYPE_FILE,
				   "", false, this->position_options_);
	  this->inputs_.add_file(file);
	  ++i;
	}
      else
	i = process_one_option(argc, argv, i, &no_more_options);
    }

  if (this->inputs_.in_group())
    {
      fprintf(stderr, _("%s: missing group end\n"), program_name);
      usage();
    }

  if (this->inputs_.in_lib())
    {
      fprintf(stderr, _("%s: missing lib end\n"), program_name);
      usage();
    }

  // Normalize the options and ensure they don't contradict each other.
  this->options_.finalize();
}

// Finalize the version script options and return them.

const Version_script_info&
Command_line::version_script()
{
  this->options_.finalize_dynamic_list();
  Version_script_info* vsi = this->script_options_.version_script_info();
  vsi->finalize();
  return *vsi;
}

} // End namespace gold.
