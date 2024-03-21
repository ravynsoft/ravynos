/* Parse options for the GNU linker.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "bfdver.h"
#include "libiberty.h"
#include "filenames.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "safe-ctype.h"
#include "getopt.h"
#include "bfdlink.h"
#include "ctf-api.h"
#include "ld.h"
#include "ldmain.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlang.h"
#include <ldgram.h>
#include "ldlex.h"
#include "ldfile.h"
#include "ldver.h"
#include "ldemul.h"
#include "demangle.h"
#if BFD_SUPPORTS_PLUGINS
#include "plugin.h"
#endif /* BFD_SUPPORTS_PLUGINS */

#ifndef PATH_SEPARATOR
#if defined (__MSDOS__) || (defined (_WIN32) && ! defined (__CYGWIN32__))
#define PATH_SEPARATOR ';'
#else
#define PATH_SEPARATOR ':'
#endif
#endif

/* Somewhere above, sys/stat.h got included . . . .  */
#if !defined(S_ISDIR) && defined(S_IFDIR)
#define	S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

static void set_default_dirlist (char *);
static void set_section_start (char *, char *);
static void set_segment_start (const char *, char *);
static void help (void);

/* The long options.  This structure is used for both the option
   parsing and the help text.  */

enum control_enum {
  /* Use one dash before long option name.  */
  ONE_DASH = 1,
  /* Use two dashes before long option name.  */
  TWO_DASHES = 2,
  /* Only accept two dashes before the long option name.
     This is an overloading of the use of this enum, since originally it
     was only intended to tell the --help display function how to display
     the long option name.  This feature was added in order to resolve
     the confusion about the -omagic command line switch.  Is it setting
     the output file name to "magic" or is it setting the NMAGIC flag on
     the output ?  It has been decided that it is setting the output file
     name, and that if you want to set the NMAGIC flag you should use -N
     or --omagic.  */
  EXACTLY_TWO_DASHES,
  /* Don't mention this option in --help output.  */
  NO_HELP
};

struct ld_option
{
  /* The long option information.  */
  struct option opt;
  /* The short option with the same meaning ('\0' if none).  */
  char shortopt;
  /* The name of the argument (NULL if none).  */
  const char *arg;
  /* The documentation string.  If this is NULL, this is a synonym for
     the previous option.  */
  const char *doc;
  enum control_enum control;
};

static const struct ld_option ld_options[] =
{
  { {NULL, required_argument, NULL, '\0'},
    'a', N_("KEYWORD"), N_("Shared library control for HP/UX compatibility"),
    ONE_DASH },
  { {"architecture", required_argument, NULL, 'A'},
    'A', N_("ARCH"), N_("Set architecture") , TWO_DASHES },
  { {"format", required_argument, NULL, 'b'},
    'b', N_("TARGET"), N_("Specify target for following input files"),
    TWO_DASHES },
  { {"mri-script", required_argument, NULL, 'c'},
    'c', N_("FILE"), N_("Read MRI format linker script"), TWO_DASHES },
  { {"dc", no_argument, NULL, 'd'},
    'd', NULL, N_("Force common symbols to be defined"), ONE_DASH },
  { {"dp", no_argument, NULL, 'd'},
    '\0', NULL, NULL, ONE_DASH },
  { {"dependency-file", required_argument, NULL, OPTION_DEPENDENCY_FILE},
    '\0', N_("FILE"), N_("Write dependency file"), TWO_DASHES },
  { {"force-group-allocation", no_argument, NULL,
     OPTION_FORCE_GROUP_ALLOCATION},
    '\0', NULL, N_("Force group members out of groups"), TWO_DASHES },
  { {"entry", required_argument, NULL, 'e'},
    'e', N_("ADDRESS"), N_("Set start address"), TWO_DASHES },
  { {"export-dynamic", no_argument, NULL, OPTION_EXPORT_DYNAMIC},
    'E', NULL, N_("Export all dynamic symbols"), TWO_DASHES },
  { {"no-export-dynamic", no_argument, NULL, OPTION_NO_EXPORT_DYNAMIC},
    '\0', NULL, N_("Undo the effect of --export-dynamic"), TWO_DASHES },
  { {"enable-non-contiguous-regions", no_argument, NULL, OPTION_NON_CONTIGUOUS_REGIONS},
    '\0', NULL, N_("Enable support of non-contiguous memory regions"), TWO_DASHES },
  { {"enable-non-contiguous-regions-warnings", no_argument, NULL, OPTION_NON_CONTIGUOUS_REGIONS_WARNINGS},
    '\0', NULL, N_("Enable warnings when --enable-non-contiguous-regions may cause unexpected behaviour"), TWO_DASHES },
  { {"disable-linker-version", no_argument, NULL, OPTION_DISABLE_LINKER_VERSION},
    '\0', NULL, N_("Disable the LINKER_VERSION linker script directive"), TWO_DASHES },
  { {"enable-linker-version", no_argument, NULL, OPTION_ENABLE_LINKER_VERSION},
    '\0', NULL, N_("Enable the LINKER_VERSION linker script directive"), TWO_DASHES },
  { {"EB", no_argument, NULL, OPTION_EB},
    '\0', NULL, N_("Link big-endian objects"), ONE_DASH },
  { {"EL", no_argument, NULL, OPTION_EL},
    '\0', NULL, N_("Link little-endian objects"), ONE_DASH },
  { {"auxiliary", required_argument, NULL, 'f'},
    'f', N_("SHLIB"), N_("Auxiliary filter for shared object symbol table"),
    TWO_DASHES },
  { {"filter", required_argument, NULL, 'F'},
    'F', N_("SHLIB"), N_("Filter for shared object symbol table"),
    TWO_DASHES },
  { {NULL, no_argument, NULL, '\0'},
    'g', NULL, N_("Ignored"), ONE_DASH },
  { {"gpsize", required_argument, NULL, 'G'},
    'G', N_("SIZE"), N_("Small data size (if no size, same as --shared)"),
    TWO_DASHES },
  { {"soname", required_argument, NULL, OPTION_SONAME},
    'h', N_("FILENAME"), N_("Set internal name of shared library"), ONE_DASH },
  { {"dynamic-linker", required_argument, NULL, OPTION_DYNAMIC_LINKER},
    'I', N_("PROGRAM"), N_("Set PROGRAM as the dynamic linker to use"),
    TWO_DASHES },
  { {"no-dynamic-linker", no_argument, NULL, OPTION_NO_DYNAMIC_LINKER},
    '\0', NULL, N_("Produce an executable with no program interpreter header"),
    TWO_DASHES },
  { {"library", required_argument, NULL, 'l'},
    'l', N_("LIBNAME"), N_("Search for library LIBNAME"), TWO_DASHES },
  { {"library-path", required_argument, NULL, 'L'},
    'L', N_("DIRECTORY"), N_("Add DIRECTORY to library search path"),
    TWO_DASHES },
  { {"sysroot=<DIRECTORY>", required_argument, NULL, OPTION_SYSROOT},
    '\0', NULL, N_("Override the default sysroot location"), TWO_DASHES },
  { {NULL, required_argument, NULL, '\0'},
    'm', N_("EMULATION"), N_("Set emulation"), ONE_DASH },
  { {"print-map", no_argument, NULL, 'M'},
    'M', NULL, N_("Print map file on standard output"), TWO_DASHES },
  { {"nmagic", no_argument, NULL, 'n'},
    'n', NULL, N_("Do not page align data"), TWO_DASHES },
  { {"omagic", no_argument, NULL, 'N'},
    'N', NULL, N_("Do not page align data, do not make text readonly"),
    EXACTLY_TWO_DASHES },
  { {"no-omagic", no_argument, NULL, OPTION_NO_OMAGIC},
    '\0', NULL, N_("Page align data, make text readonly"),
    EXACTLY_TWO_DASHES },
  { {"output", required_argument, NULL, 'o'},
    'o', N_("FILE"), N_("Set output file name"), EXACTLY_TWO_DASHES },
  { {NULL, required_argument, NULL, '\0'},
    'O', NULL, N_("Optimize output file"), ONE_DASH },
  { {"out-implib", required_argument, NULL, OPTION_OUT_IMPLIB},
    '\0', N_("FILE"), N_("Generate import library"), TWO_DASHES },
#if BFD_SUPPORTS_PLUGINS
  { {"plugin", required_argument, NULL, OPTION_PLUGIN},
    '\0', N_("PLUGIN"), N_("Load named plugin"), ONE_DASH },
  { {"plugin-opt", required_argument, NULL, OPTION_PLUGIN_OPT},
    '\0', N_("ARG"), N_("Send arg to last-loaded plugin"), ONE_DASH },
  { {"flto", optional_argument, NULL, OPTION_IGNORE},
    '\0', NULL, N_("Ignored for GCC LTO option compatibility"),
    ONE_DASH },
  { {"flto-partition=", required_argument, NULL, OPTION_IGNORE},
    '\0', NULL, N_("Ignored for GCC LTO option compatibility"),
    ONE_DASH },
#else
  { {"plugin", required_argument, NULL, OPTION_IGNORE},
    '\0', N_("PLUGIN"), N_("Load named plugin (ignored)"), ONE_DASH },
  { {"plugin-opt", required_argument, NULL, OPTION_IGNORE},
    '\0', N_("ARG"), N_("Send arg to last-loaded plugin (ignored)"), ONE_DASH },
#endif /* BFD_SUPPORTS_PLUGINS */
  { {"fuse-ld=", required_argument, NULL, OPTION_IGNORE},
    '\0', NULL, N_("Ignored for GCC linker option compatibility"),
    ONE_DASH },
  { {"map-whole-files", optional_argument, NULL, OPTION_IGNORE},
    '\0', NULL, N_("Ignored for gold option compatibility"),
    TWO_DASHES },
  { {"no-map-whole-files", optional_argument, NULL, OPTION_IGNORE},
    '\0', NULL, N_("Ignored for gold option compatibility"),
    TWO_DASHES },
  { {"Qy", no_argument, NULL, OPTION_IGNORE},
    '\0', NULL, N_("Ignored for SVR4 compatibility"), ONE_DASH },
  { {"emit-relocs", no_argument, NULL, 'q'},
    'q', NULL, "Generate relocations in final output", TWO_DASHES },
  { {"relocatable", no_argument, NULL, 'r'},
    'r', NULL, N_("Generate relocatable output"), TWO_DASHES },
  { {NULL, no_argument, NULL, '\0'},
    'i', NULL, NULL, ONE_DASH },
  { {"just-symbols", required_argument, NULL, 'R'},
    'R', N_("FILE"), N_("Just link symbols (if directory, same as --rpath)"),
    TWO_DASHES },

  { {"remap-inputs-file", required_argument, NULL, OPTION_REMAP_INPUTS_FILE},
    '\0', N_("FILE"), "Provide a FILE containing input remapings", TWO_DASHES },
  { {"remap-inputs", required_argument, NULL, OPTION_REMAP_INPUTS},
    '\0', N_("PATTERN=FILE"), "Remap input files matching PATTERN to FILE", TWO_DASHES },

  { {"strip-all", no_argument, NULL, 's'},
    's', NULL, N_("Strip all symbols"), TWO_DASHES },
  { {"strip-debug", no_argument, NULL, 'S'},
    'S', NULL, N_("Strip debugging symbols"), TWO_DASHES },
  { {"strip-discarded", no_argument, NULL, OPTION_STRIP_DISCARDED},
    '\0', NULL, N_("Strip symbols in discarded sections"), TWO_DASHES },
  { {"no-strip-discarded", no_argument, NULL, OPTION_NO_STRIP_DISCARDED},
    '\0', NULL, N_("Do not strip symbols in discarded sections"), TWO_DASHES },
  { {"trace", no_argument, NULL, 't'},
    't', NULL, N_("Trace file opens"), TWO_DASHES },
  { {"script", required_argument, NULL, 'T'},
    'T', N_("FILE"), N_("Read linker script"), TWO_DASHES },
  { {"default-script", required_argument, NULL, OPTION_DEFAULT_SCRIPT},
    '\0', N_("FILE"), N_("Read default linker script"), TWO_DASHES },
  { {"dT", required_argument, NULL, OPTION_DEFAULT_SCRIPT},
    '\0', NULL, NULL, ONE_DASH },
  { {"undefined", required_argument, NULL, 'u'},
    'u', N_("SYMBOL"), N_("Start with undefined reference to SYMBOL"),
    TWO_DASHES },
  { {"require-defined", required_argument, NULL, OPTION_REQUIRE_DEFINED_SYMBOL},
    '\0', N_("SYMBOL"), N_("Require SYMBOL be defined in the final output"),
    TWO_DASHES },
  { {"unique", optional_argument, NULL, OPTION_UNIQUE},
    '\0', N_("[=SECTION]"),
    N_("Don't merge input [SECTION | orphan] sections"), TWO_DASHES },
  { {"Ur", no_argument, NULL, OPTION_UR},
    '\0', NULL, N_("Build global constructor/destructor tables"), ONE_DASH },
  { {"version", no_argument, NULL, OPTION_VERSION},
    'v', NULL, N_("Print version information"), TWO_DASHES },
  { {NULL, no_argument, NULL, '\0'},
    'V', NULL, N_("Print version and emulation information"), ONE_DASH },
  { {"discard-all", no_argument, NULL, 'x'},
    'x', NULL, N_("Discard all local symbols"), TWO_DASHES },
  { {"discard-locals", no_argument, NULL, 'X'},
    'X', NULL, N_("Discard temporary local symbols (default)"), TWO_DASHES },
  { {"discard-none", no_argument, NULL, OPTION_DISCARD_NONE},
    '\0', NULL, N_("Don't discard any local symbols"), TWO_DASHES },
  { {"trace-symbol", required_argument, NULL, 'y'},
    'y', N_("SYMBOL"), N_("Trace mentions of SYMBOL"), TWO_DASHES },
  { {NULL, required_argument, NULL, '\0'},
    'Y', N_("PATH"), N_("Default search path for Solaris compatibility"),
    ONE_DASH },
  { {"start-group", no_argument, NULL, '('},
    '(', NULL, N_("Start a group"), TWO_DASHES },
  { {"end-group", no_argument, NULL, ')'},
    ')', NULL, N_("End a group"), TWO_DASHES },
  { {"accept-unknown-input-arch", no_argument, NULL,
     OPTION_ACCEPT_UNKNOWN_INPUT_ARCH},
    '\0', NULL,
    N_("Accept input files whose architecture cannot be determined"),
    TWO_DASHES },
  { {"no-accept-unknown-input-arch", no_argument, NULL,
     OPTION_NO_ACCEPT_UNKNOWN_INPUT_ARCH},
    '\0', NULL, N_("Reject input files whose architecture is unknown"),
    TWO_DASHES },

  /* The next two options are deprecated because of their similarity to
     --as-needed and --no-as-needed.  They have been replaced by
     --copy-dt-needed-entries and --no-copy-dt-needed-entries.  */
  { {"add-needed", no_argument, NULL, OPTION_ADD_DT_NEEDED_FOR_DYNAMIC},
    '\0', NULL, NULL, NO_HELP },
  { {"no-add-needed", no_argument, NULL, OPTION_NO_ADD_DT_NEEDED_FOR_DYNAMIC},
    '\0', NULL, NULL, NO_HELP },

  { {"as-needed", no_argument, NULL, OPTION_ADD_DT_NEEDED_FOR_REGULAR},
    '\0', NULL, N_("Only set DT_NEEDED for following dynamic libs if used"),
    TWO_DASHES },
  { {"no-as-needed", no_argument, NULL, OPTION_NO_ADD_DT_NEEDED_FOR_REGULAR},
    '\0', NULL, N_("Always set DT_NEEDED for dynamic libraries mentioned on\n"
		   "                                the command line"),
    TWO_DASHES },
  { {"assert", required_argument, NULL, OPTION_ASSERT},
    '\0', N_("KEYWORD"), N_("Ignored for SunOS compatibility"), ONE_DASH },
  { {"Bdynamic", no_argument, NULL, OPTION_CALL_SHARED},
    '\0', NULL, N_("Link against shared libraries"), ONE_DASH },
  { {"dy", no_argument, NULL, OPTION_CALL_SHARED},
    '\0', NULL, NULL, ONE_DASH },
  { {"call_shared", no_argument, NULL, OPTION_CALL_SHARED},
    '\0', NULL, NULL, ONE_DASH },
  { {"Bstatic", no_argument, NULL, OPTION_NON_SHARED},
    '\0', NULL, N_("Do not link against shared libraries"), ONE_DASH },
  { {"dn", no_argument, NULL, OPTION_NON_SHARED},
    '\0', NULL, NULL, ONE_DASH },
  { {"non_shared", no_argument, NULL, OPTION_NON_SHARED},
    '\0', NULL, NULL, ONE_DASH },
  { {"static", no_argument, NULL, OPTION_NON_SHARED},
    '\0', NULL, NULL, ONE_DASH },
  { {"Bno-symbolic", no_argument, NULL, OPTION_NO_SYMBOLIC},
    '\0', NULL, N_("Don't bind global references locally"), ONE_DASH },
  { {"Bsymbolic", no_argument, NULL, OPTION_SYMBOLIC},
    '\0', NULL, N_("Bind global references locally"), ONE_DASH },
  { {"Bsymbolic-functions", no_argument, NULL, OPTION_SYMBOLIC_FUNCTIONS},
    '\0', NULL, N_("Bind global function references locally"), ONE_DASH },
  { {"check-sections", no_argument, NULL, OPTION_CHECK_SECTIONS},
    '\0', NULL, N_("Check section addresses for overlaps (default)"),
    TWO_DASHES },
  { {"no-check-sections", no_argument, NULL, OPTION_NO_CHECK_SECTIONS},
    '\0', NULL, N_("Do not check section addresses for overlaps"),
    TWO_DASHES },
  { {"copy-dt-needed-entries", no_argument, NULL,
     OPTION_ADD_DT_NEEDED_FOR_DYNAMIC},
    '\0', NULL, N_("Copy DT_NEEDED links mentioned inside DSOs that follow"),
    TWO_DASHES },
  { {"no-copy-dt-needed-entries", no_argument, NULL,
     OPTION_NO_ADD_DT_NEEDED_FOR_DYNAMIC},
    '\0', NULL, N_("Do not copy DT_NEEDED links mentioned inside DSOs that follow"),
    TWO_DASHES },

  { {"cref", no_argument, NULL, OPTION_CREF},
    '\0', NULL, N_("Output cross reference table"), TWO_DASHES },
  { {"defsym", required_argument, NULL, OPTION_DEFSYM},
    '\0', N_("SYMBOL=EXPRESSION"), N_("Define a symbol"), TWO_DASHES },
  { {"demangle", optional_argument, NULL, OPTION_DEMANGLE},
    '\0', N_("[=STYLE]"), N_("Demangle symbol names [using STYLE]"),
    TWO_DASHES },
  { {"disable-multiple-abs-defs", no_argument, NULL,
     OPTION_DISABLE_MULTIPLE_DEFS_ABS},
    '\0', NULL, N_("Do not allow multiple definitions with symbols included\n"
		   "                                in filename invoked by -R "
		   "or --just-symbols"),
    TWO_DASHES},
  { {"embedded-relocs", no_argument, NULL, OPTION_EMBEDDED_RELOCS},
    '\0', NULL, N_("Generate embedded relocs"), TWO_DASHES},
  { {"fatal-warnings", no_argument, NULL, OPTION_WARN_FATAL},
    '\0', NULL, N_("Treat warnings as errors"),
    TWO_DASHES },
  { {"no-fatal-warnings", no_argument, NULL, OPTION_NO_WARN_FATAL},
    '\0', NULL, N_("Do not treat warnings as errors (default)"),
    TWO_DASHES },
  { {"fini", required_argument, NULL, OPTION_FINI},
    '\0', N_("SYMBOL"), N_("Call SYMBOL at unload-time"), ONE_DASH },
  { {"force-exe-suffix", no_argument, NULL, OPTION_FORCE_EXE_SUFFIX},
    '\0', NULL, N_("Force generation of file with .exe suffix"), TWO_DASHES},
  { {"gc-sections", no_argument, NULL, OPTION_GC_SECTIONS},
    '\0', NULL, N_("Remove unused sections (on some targets)"),
    TWO_DASHES },
  { {"no-gc-sections", no_argument, NULL, OPTION_NO_GC_SECTIONS},
    '\0', NULL, N_("Don't remove unused sections (default)"),
    TWO_DASHES },
  { {"print-gc-sections", no_argument, NULL, OPTION_PRINT_GC_SECTIONS},
    '\0', NULL, N_("List removed unused sections on stderr"),
    TWO_DASHES },
  { {"no-print-gc-sections", no_argument, NULL, OPTION_NO_PRINT_GC_SECTIONS},
    '\0', NULL, N_("Do not list removed unused sections"),
    TWO_DASHES },
  { {"gc-keep-exported", no_argument, NULL, OPTION_GC_KEEP_EXPORTED},
    '\0', NULL, N_("Keep exported symbols when removing unused sections"),
    TWO_DASHES },
  { {"hash-size=<NUMBER>", required_argument, NULL, OPTION_HASH_SIZE},
    '\0', NULL, N_("Set default hash table size close to <NUMBER>"),
    TWO_DASHES },
  { {"help", no_argument, NULL, OPTION_HELP},
    '\0', NULL, N_("Print option help"), TWO_DASHES },
  { {"init", required_argument, NULL, OPTION_INIT},
    '\0', N_("SYMBOL"), N_("Call SYMBOL at load-time"), ONE_DASH },
  { {"Map", required_argument, NULL, OPTION_MAP},
    '\0', N_("FILE/DIR"), N_("Write a linker map to FILE or DIR/<outputname>.map"), ONE_DASH },
  { {"no-define-common", no_argument, NULL, OPTION_NO_DEFINE_COMMON},
    '\0', NULL, N_("Do not define Common storage"), TWO_DASHES },
  { {"no-demangle", no_argument, NULL, OPTION_NO_DEMANGLE },
    '\0', NULL, N_("Do not demangle symbol names"), TWO_DASHES },
  { {"no-keep-memory", no_argument, NULL, OPTION_NO_KEEP_MEMORY},
    '\0', NULL, N_("Use less memory and more disk I/O"), TWO_DASHES },
  { {"no-undefined", no_argument, NULL, OPTION_NO_UNDEFINED},
    '\0', NULL, N_("Do not allow unresolved references in object files"),
    TWO_DASHES },
  { {"no-warnings", no_argument, NULL, OPTION_NO_WARNINGS},
    'w', NULL, N_("Do not display any warning or error messages"),
    TWO_DASHES },
  { {"allow-shlib-undefined", no_argument, NULL, OPTION_ALLOW_SHLIB_UNDEFINED},
    '\0', NULL, N_("Allow unresolved references in shared libraries"),
    TWO_DASHES },
  { {"no-allow-shlib-undefined", no_argument, NULL,
     OPTION_NO_ALLOW_SHLIB_UNDEFINED},
    '\0', NULL, N_("Do not allow unresolved references in shared libs"),
    TWO_DASHES },
  { {"allow-multiple-definition", no_argument, NULL,
     OPTION_ALLOW_MULTIPLE_DEFINITION},
    '\0', NULL, N_("Allow multiple definitions"), TWO_DASHES },
#if SUPPORT_ERROR_HANDLING_SCRIPT
  { {"error-handling-script", required_argument, NULL,
     OPTION_ERROR_HANDLING_SCRIPT},
    '\0', N_("SCRIPT"), N_("Provide a script to help with undefined symbol errors"), TWO_DASHES},
#endif
  { {"undefined-version", no_argument, NULL, OPTION_UNDEFINED_VERSION},
    '\0', NULL, N_("Allow undefined version"), EXACTLY_TWO_DASHES },
  { {"no-undefined-version", no_argument, NULL, OPTION_NO_UNDEFINED_VERSION},
    '\0', NULL, N_("Disallow undefined version"), TWO_DASHES },
  { {"default-symver", no_argument, NULL, OPTION_DEFAULT_SYMVER},
    '\0', NULL, N_("Create default symbol version"), TWO_DASHES },
  { {"default-imported-symver", no_argument, NULL,
      OPTION_DEFAULT_IMPORTED_SYMVER},
    '\0', NULL, N_("Create default symbol version for imported symbols"),
    TWO_DASHES },
  { {"no-warn-mismatch", no_argument, NULL, OPTION_NO_WARN_MISMATCH},
    '\0', NULL, N_("Don't warn about mismatched input files"), TWO_DASHES},
  { {"no-warn-search-mismatch", no_argument, NULL,
     OPTION_NO_WARN_SEARCH_MISMATCH},
    '\0', NULL, N_("Don't warn on finding an incompatible library"),
    TWO_DASHES},
  { {"no-whole-archive", no_argument, NULL, OPTION_NO_WHOLE_ARCHIVE},
    '\0', NULL, N_("Turn off --whole-archive"), TWO_DASHES },
  { {"noinhibit-exec", no_argument, NULL, OPTION_NOINHIBIT_EXEC},
    '\0', NULL, N_("Create an output file even if errors occur"),
    TWO_DASHES },
  { {"noinhibit_exec", no_argument, NULL, OPTION_NOINHIBIT_EXEC},
    '\0', NULL, NULL, NO_HELP },
  { {"nostdlib", no_argument, NULL, OPTION_NOSTDLIB},
    '\0', NULL, N_("Only use library directories specified on\n"
		   "                                the command line"),
    ONE_DASH },
  { {"oformat", required_argument, NULL, OPTION_OFORMAT},
    '\0', N_("TARGET"), N_("Specify target of output file"),
    EXACTLY_TWO_DASHES },
  { {"print-output-format", no_argument, NULL, OPTION_PRINT_OUTPUT_FORMAT},
    '\0', NULL, N_("Print default output format"), TWO_DASHES },
  { {"print-sysroot", no_argument, NULL, OPTION_PRINT_SYSROOT},
    '\0', NULL, N_("Print current sysroot"), TWO_DASHES },
  { {"qmagic", no_argument, NULL, OPTION_IGNORE},
    '\0', NULL, N_("Ignored for Linux compatibility"), ONE_DASH },
  { {"reduce-memory-overheads", no_argument, NULL,
     OPTION_REDUCE_MEMORY_OVERHEADS},
    '\0', NULL, N_("Reduce memory overheads, possibly taking much longer"),
    TWO_DASHES },
  { {"max-cache-size=SIZE", required_argument, NULL,
    OPTION_MAX_CACHE_SIZE},
    '\0', NULL, N_("Set the maximum cache size to SIZE bytes"),
    TWO_DASHES },
  { {"relax", no_argument, NULL, OPTION_RELAX},
    '\0', NULL, N_("Reduce code size by using target specific optimizations"), TWO_DASHES },
  { {"no-relax", no_argument, NULL, OPTION_NO_RELAX},
    '\0', NULL, N_("Do not use relaxation techniques to reduce code size"), TWO_DASHES },
  { {"retain-symbols-file", required_argument, NULL,
     OPTION_RETAIN_SYMBOLS_FILE},
    '\0', N_("FILE"), N_("Keep only symbols listed in FILE"), TWO_DASHES },
  { {"rpath", required_argument, NULL, OPTION_RPATH},
    '\0', N_("PATH"), N_("Set runtime shared library search path"), ONE_DASH },
  { {"rpath-link", required_argument, NULL, OPTION_RPATH_LINK},
    '\0', N_("PATH"), N_("Set link time shared library search path"),
    ONE_DASH },
  { {"shared", no_argument, NULL, OPTION_SHARED},
    '\0', NULL, N_("Create a shared library"), ONE_DASH },
  { {"Bshareable", no_argument, NULL, OPTION_SHARED }, /* FreeBSD.  */
    '\0', NULL, NULL, ONE_DASH },
  { {"pie", no_argument, NULL, OPTION_PIE},
    '\0', NULL, N_("Create a position independent executable"), ONE_DASH },
  { {"pic-executable", no_argument, NULL, OPTION_PIE},
    '\0', NULL, NULL, TWO_DASHES },
  { {"no-pie", no_argument, NULL, OPTION_NO_PIE},
    '\0', NULL, N_("Create a position dependent executable (default)"), ONE_DASH },
  { {"sort-common", optional_argument, NULL, OPTION_SORT_COMMON},
    '\0', N_("[=ascending|descending]"),
    N_("Sort common symbols by alignment [in specified order]"),
    TWO_DASHES },
  { {"sort_common", no_argument, NULL, OPTION_SORT_COMMON},
    '\0', NULL, NULL, NO_HELP },
  { {"sort-section", required_argument, NULL, OPTION_SORT_SECTION},
    '\0', N_("name|alignment"),
    N_("Sort sections by name or maximum alignment"), TWO_DASHES },
  { {"spare-dynamic-tags", required_argument, NULL, OPTION_SPARE_DYNAMIC_TAGS},
    '\0', N_("COUNT"), N_("How many tags to reserve in .dynamic section"),
    TWO_DASHES },
  { {"split-by-file", optional_argument, NULL, OPTION_SPLIT_BY_FILE},
    '\0', N_("[=SIZE]"), N_("Split output sections every SIZE octets"),
    TWO_DASHES },
  { {"split-by-reloc", optional_argument, NULL, OPTION_SPLIT_BY_RELOC},
    '\0', N_("[=COUNT]"), N_("Split output sections every COUNT relocs"),
    TWO_DASHES },
  { {"stats", no_argument, NULL, OPTION_STATS},
    '\0', NULL, N_("Print memory usage statistics"), TWO_DASHES },
  { {"target-help", no_argument, NULL, OPTION_TARGET_HELP},
    '\0', NULL, N_("Display target specific options"), TWO_DASHES },
  { {"task-link", required_argument, NULL, OPTION_TASK_LINK},
    '\0', N_("SYMBOL"), N_("Do task level linking"), TWO_DASHES },
  { {"traditional-format", no_argument, NULL, OPTION_TRADITIONAL_FORMAT},
    '\0', NULL, N_("Use same format as native linker"), TWO_DASHES },
  { {"section-start", required_argument, NULL, OPTION_SECTION_START},
    '\0', N_("SECTION=ADDRESS"), N_("Set address of named section"),
    TWO_DASHES },
  { {"Tbss", required_argument, NULL, OPTION_TBSS},
    '\0', N_("ADDRESS"), N_("Set address of .bss section"), ONE_DASH },
  { {"Tdata", required_argument, NULL, OPTION_TDATA},
    '\0', N_("ADDRESS"), N_("Set address of .data section"), ONE_DASH },
  { {"Ttext", required_argument, NULL, OPTION_TTEXT},
    '\0', N_("ADDRESS"), N_("Set address of .text section"), ONE_DASH },
  { {"Ttext-segment", required_argument, NULL, OPTION_TTEXT_SEGMENT},
    '\0', N_("ADDRESS"), N_("Set address of text segment"), ONE_DASH },
  { {"Trodata-segment", required_argument, NULL, OPTION_TRODATA_SEGMENT},
    '\0', N_("ADDRESS"), N_("Set address of rodata segment"), ONE_DASH },
  { {"Tldata-segment", required_argument, NULL, OPTION_TLDATA_SEGMENT},
    '\0', N_("ADDRESS"), N_("Set address of ldata segment"), ONE_DASH },
  { {"unresolved-symbols=<method>", required_argument, NULL,
     OPTION_UNRESOLVED_SYMBOLS},
    '\0', NULL, N_("How to handle unresolved symbols.  <method> is:\n"
		   "                                ignore-all, report-all, ignore-in-object-files,\n"
		   "                                ignore-in-shared-libs"),
    TWO_DASHES },
  { {"verbose", optional_argument, NULL, OPTION_VERBOSE},
    '\0', N_("[=NUMBER]"),
    N_("Output lots of information during link"), TWO_DASHES },
  { {"dll-verbose", no_argument, NULL, OPTION_VERBOSE}, /* Linux.  */
    '\0', NULL, NULL, NO_HELP },
  { {"version-script", required_argument, NULL, OPTION_VERSION_SCRIPT },
    '\0', N_("FILE"), N_("Read version information script"), TWO_DASHES },
  { {"version-exports-section", required_argument, NULL,
     OPTION_VERSION_EXPORTS_SECTION },
    '\0', N_("SYMBOL"), N_("Take export symbols list from .exports, using\n"
			   "                                SYMBOL as the version."),
    TWO_DASHES },
  { {"dynamic-list-data", no_argument, NULL, OPTION_DYNAMIC_LIST_DATA},
    '\0', NULL, N_("Add data symbols to dynamic list"), TWO_DASHES },
  { {"dynamic-list-cpp-new", no_argument, NULL, OPTION_DYNAMIC_LIST_CPP_NEW},
    '\0', NULL, N_("Use C++ operator new/delete dynamic list"), TWO_DASHES },
  { {"dynamic-list-cpp-typeinfo", no_argument, NULL, OPTION_DYNAMIC_LIST_CPP_TYPEINFO},
    '\0', NULL, N_("Use C++ typeinfo dynamic list"), TWO_DASHES },
  { {"dynamic-list", required_argument, NULL, OPTION_DYNAMIC_LIST},
    '\0', N_("FILE"), N_("Read dynamic list"), TWO_DASHES },
  { {"export-dynamic-symbol", required_argument, NULL, OPTION_EXPORT_DYNAMIC_SYMBOL},
    '\0', N_("SYMBOL"), N_("Export the specified symbol"), EXACTLY_TWO_DASHES },
  { {"export-dynamic-symbol-list", required_argument, NULL, OPTION_EXPORT_DYNAMIC_SYMBOL_LIST},
    '\0', N_("FILE"), N_("Read export dynamic symbol list"), EXACTLY_TWO_DASHES },
  { {"warn-common", no_argument, NULL, OPTION_WARN_COMMON},
    '\0', NULL, N_("Warn about duplicate common symbols"), TWO_DASHES },
  { {"warn-constructors", no_argument, NULL, OPTION_WARN_CONSTRUCTORS},
    '\0', NULL, N_("Warn if global constructors/destructors are seen"),
    TWO_DASHES },
  { {"warn-execstack", no_argument, NULL, OPTION_WARN_EXECSTACK},
    '\0', NULL, N_("Warn when creating an executable stack"), TWO_DASHES },
  { {"no-warn-execstack", no_argument, NULL, OPTION_NO_WARN_EXECSTACK},
    '\0', NULL, N_("Do not warn when creating an executable stack"), TWO_DASHES },
  { {"warn-rwx-segments", no_argument, NULL, OPTION_WARN_RWX_SEGMENTS},
    '\0', NULL, N_("Warn when creating executable segments"), TWO_DASHES },
  { {"no-warn-rwx-segments", no_argument, NULL, OPTION_NO_WARN_RWX_SEGMENTS},
    '\0', NULL, N_("Do not warn when creating executable segments"), TWO_DASHES },
  { {"warn-multiple-gp", no_argument, NULL, OPTION_WARN_MULTIPLE_GP},
    '\0', NULL, N_("Warn if the multiple GP values are used"), TWO_DASHES },
  { {"warn-once", no_argument, NULL, OPTION_WARN_ONCE},
    '\0', NULL, N_("Warn only once per undefined symbol"), TWO_DASHES },
  { {"warn-section-align", no_argument, NULL, OPTION_WARN_SECTION_ALIGN},
    '\0', NULL, N_("Warn if start of section changes due to alignment"),
    TWO_DASHES },
  { {"warn-textrel", no_argument, NULL, OPTION_WARN_TEXTREL},
    '\0', NULL,
#if DEFAULT_LD_TEXTREL_CHECK_WARNING
    N_("Warn if output has DT_TEXTREL (default)"),
#else
    N_("Warn if output has DT_TEXTREL"),
#endif
    TWO_DASHES },
  { {"warn-shared-textrel", no_argument, NULL, OPTION_WARN_TEXTREL},
    '\0', NULL, NULL, NO_HELP },
  { {"warn-alternate-em", no_argument, NULL, OPTION_WARN_ALTERNATE_EM},
    '\0', NULL, N_("Warn if an object has alternate ELF machine code"),
    TWO_DASHES },
  { {"warn-unresolved-symbols", no_argument, NULL,
     OPTION_WARN_UNRESOLVED_SYMBOLS},
    '\0', NULL, N_("Report unresolved symbols as warnings"), TWO_DASHES },
  { {"error-unresolved-symbols", no_argument, NULL,
     OPTION_ERROR_UNRESOLVED_SYMBOLS},
    '\0', NULL, N_("Report unresolved symbols as errors"), TWO_DASHES },
  { {"whole-archive", no_argument, NULL, OPTION_WHOLE_ARCHIVE},
    '\0', NULL, N_("Include all objects from following archives"),
    TWO_DASHES },
  { {"wrap", required_argument, NULL, OPTION_WRAP},
    '\0', N_("SYMBOL"), N_("Use wrapper functions for SYMBOL"), TWO_DASHES },
  { {"ignore-unresolved-symbol", required_argument, NULL,
    OPTION_IGNORE_UNRESOLVED_SYMBOL},
    '\0', N_("SYMBOL"),
    N_("Unresolved SYMBOL will not cause an error or warning"), TWO_DASHES },
  { {"push-state", no_argument, NULL, OPTION_PUSH_STATE},
    '\0', NULL, N_("Push state of flags governing input file handling"),
    TWO_DASHES },
  { {"pop-state", no_argument, NULL, OPTION_POP_STATE},
    '\0', NULL, N_("Pop state of flags governing input file handling"),
    TWO_DASHES },
  { {"print-memory-usage", no_argument, NULL, OPTION_PRINT_MEMORY_USAGE},
    '\0', NULL, N_("Report target memory usage"), TWO_DASHES },
  { {"orphan-handling", required_argument, NULL, OPTION_ORPHAN_HANDLING},
    '\0', N_("=MODE"), N_("Control how orphan sections are handled."),
    TWO_DASHES },
  { {"print-map-discarded", no_argument, NULL, OPTION_PRINT_MAP_DISCARDED},
    '\0', NULL, N_("Show discarded sections in map file output (default)"),
    TWO_DASHES },
  { {"no-print-map-discarded", no_argument, NULL, OPTION_NO_PRINT_MAP_DISCARDED},
    '\0', NULL, N_("Do not show discarded sections in map file output"),
    TWO_DASHES },
  { {"print-map-locals", no_argument, NULL, OPTION_PRINT_MAP_LOCALS},
    '\0', NULL, N_("Show local symbols in map file output"),
    TWO_DASHES },
  { {"no-print-map-locals", no_argument, NULL, OPTION_NO_PRINT_MAP_LOCALS},
    '\0', NULL, N_("Do not show local symbols in map file output (default)"),
    TWO_DASHES },
  { {"ctf-variables", no_argument, NULL, OPTION_CTF_VARIABLES},
    '\0', NULL, N_("Emit names and types of static variables in CTF"),
    TWO_DASHES },
  { {"no-ctf-variables", no_argument, NULL, OPTION_NO_CTF_VARIABLES},
    '\0', NULL, N_("Do not emit names and types of static variables in CTF"),
    TWO_DASHES },
  { {"ctf-share-types=<method>", required_argument, NULL,
     OPTION_CTF_SHARE_TYPES},
    '\0', NULL, N_("How to share CTF types between translation units.\n"
		   "                                <method> is: share-unconflicted (default),\n"
		   "                                             share-duplicated"),
    TWO_DASHES },
};

#define OPTION_COUNT ARRAY_SIZE (ld_options)

void
parse_args (unsigned argc, char **argv)
{
  unsigned i;
  int is, il, irl;
  int ingroup = 0;
  char *default_dirlist = NULL;
  char *shortopts;
  struct option *longopts;
  struct option *really_longopts;
  int last_optind;
  enum symbolic_enum
  {
    symbolic_unset = 0,
    symbolic,
    symbolic_functions,
  } opt_symbolic = symbolic_unset;
  enum dynamic_list_enum
  {
    dynamic_list_unset = 0,
    dynamic_list_data,
    dynamic_list
  } opt_dynamic_list = dynamic_list_unset;
  struct bfd_elf_dynamic_list *export_list = NULL;

  shortopts = (char *) xmalloc (OPTION_COUNT * 3 + 2);
  longopts = (struct option *)
      xmalloc (sizeof (*longopts) * (OPTION_COUNT + 1));
  really_longopts = (struct option *)
      xmalloc (sizeof (*really_longopts) * (OPTION_COUNT + 1));

  /* Starting the short option string with '-' is for programs that
     expect options and other ARGV-elements in any order and that care about
     the ordering of the two.  We describe each non-option ARGV-element
     as if it were the argument of an option with character code 1.  */
  shortopts[0] = '-';
  is = 1;
  il = 0;
  irl = 0;
  for (i = 0; i < OPTION_COUNT; i++)
    {
      if (ld_options[i].shortopt != '\0')
	{
	  shortopts[is] = ld_options[i].shortopt;
	  ++is;
	  if (ld_options[i].opt.has_arg == required_argument
	      || ld_options[i].opt.has_arg == optional_argument)
	    {
	      shortopts[is] = ':';
	      ++is;
	      if (ld_options[i].opt.has_arg == optional_argument)
		{
		  shortopts[is] = ':';
		  ++is;
		}
	    }
	}
      if (ld_options[i].opt.name != NULL)
	{
	  if (ld_options[i].control == EXACTLY_TWO_DASHES)
	    {
	      really_longopts[irl] = ld_options[i].opt;
	      ++irl;
	    }
	  else
	    {
	      longopts[il] = ld_options[i].opt;
	      ++il;
	    }
	}
    }
  shortopts[is] = '\0';
  longopts[il].name = NULL;
  really_longopts[irl].name = NULL;

  ldemul_add_options (is, &shortopts, il, &longopts, irl, &really_longopts);

  /* The -G option is ambiguous on different platforms.  Sometimes it
     specifies the largest data size to put into the small data
     section.  Sometimes it is equivalent to --shared.  Unfortunately,
     the first form takes an argument, while the second does not.

     We need to permit the --shared form because on some platforms,
     such as Solaris, gcc -shared will pass -G to the linker.

     To permit either usage, we look through the argument list.  If we
     find -G not followed by a number, we change it into --shared.
     This will work for most normal cases.  */
  for (i = 1; i < argc; i++)
    if (strcmp (argv[i], "-G") == 0
	&& (i + 1 >= argc
	    || ! ISDIGIT (argv[i + 1][0])))
      argv[i] = (char *) "--shared";

  /* Because we permit long options to start with a single dash, and
     we have a --library option, and the -l option is conventionally
     used with an immediately following argument, we can have bad
     results if somebody tries to use -l with a library whose name
     happens to start with "ibrary", as in -li.  We avoid problems by
     simply turning -l into --library.  This means that users will
     have to use two dashes in order to use --library, which is OK
     since that's how it is documented.

     FIXME: It's possible that this problem can arise for other short
     options as well, although the user does always have the recourse
     of adding a space between the option and the argument.  */
  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '-'
	  && argv[i][1] == 'l'
	  && argv[i][2] != '\0')
	{
	  char *n;

	  n = (char *) xmalloc (strlen (argv[i]) + 20);
	  sprintf (n, "--library=%s", argv[i] + 2);
	  argv[i] = n;
	}
    }

  last_optind = -1;
  while (1)
    {
      int longind = 0;
      int optc;
      static unsigned int defsym_count;

      /* Using last_optind lets us avoid calling ldemul_parse_args
	 multiple times on a single option, which would lead to
	 confusion in the internal static variables maintained by
	 getopt.  This could otherwise happen for an argument like
	 -nx, in which the -n is parsed as a single option, and we
	 loop around to pick up the -x.  */
      if (optind != last_optind)
	if (ldemul_parse_args (argc, argv))
	  continue;

      /* getopt_long_only is like getopt_long, but '-' as well as '--'
	 can indicate a long option.  */
      opterr = 0;
      last_optind = optind;
      optc = getopt_long_only (argc, argv, shortopts, longopts, &longind);
      if (optc == '?')
	{
	  optind = last_optind;
	  optc = getopt_long (argc, argv, "-", really_longopts, &longind);
	}
      /* Attempt to detect grouped short options,  eg: "-non-start".
	 Accepting such options is error prone as it is not clear if the user
	 intended "-n -o n-start" or "--non-start".  */
      else if (longind == 0  /* This is a short option.  */
	       && optc > 32  /* It is a valid option.  */
        /* The character is not the second character of argv[last_optind].  */
	       && optc != argv[last_optind][1])
	{
	  if (optarg)
	    einfo (_("%F%P: Error: unable to disambiguate: %s (did you mean -%s ?)\n"),
		   argv[last_optind], argv[last_optind]);
	  else
	    einfo (_("%P: Warning: grouped short command line options are deprecated: %s\n"), argv[last_optind]);
	}

      if (ldemul_handle_option (optc))
	continue;

      if (optc == -1)
	break;

      switch (optc)
	{
	case '?':
	  {
	    /* If the last word on the command line is an option that
	       requires an argument, getopt will refuse to recognise it.
	       Try to catch such options here and issue a more helpful
	       error message than just "unrecognized option".  */
	    int opt;

	    for (opt = ARRAY_SIZE (ld_options); opt--;)
	      if (ld_options[opt].opt.has_arg == required_argument
		  /* FIXME: There are a few short options that do not
		     have long equivalents, but which require arguments.
		     We should handle them too.  */
		  && ld_options[opt].opt.name != NULL
		  && strcmp (argv[last_optind] + ld_options[opt].control, ld_options[opt].opt.name) == 0)
		{
		  einfo (_("%P: %s: missing argument\n"), argv[last_optind]);
		  break;
		}

	    if (opt == -1)
	      einfo (_("%P: unrecognized option '%s'\n"), argv[last_optind]);
	  }
	  /* Fall through.  */

	default:
	  einfo (_("%F%P: use the --help option for usage information\n"));
	  break;

	case 1:			/* File name.  */
	  lang_add_input_file (optarg, lang_input_file_is_file_enum, NULL);
	  break;

	case OPTION_IGNORE:
	  break;
	case 'a':
	  /* For HP/UX compatibility.  Actually -a shared should mean
	     ``use only shared libraries'' but, then, we don't
	     currently support shared libraries on HP/UX anyhow.  */
	  if (strcmp (optarg, "archive") == 0)
	    input_flags.dynamic = false;
	  else if (strcmp (optarg, "shared") == 0
		   || strcmp (optarg, "default") == 0)
	    input_flags.dynamic = true;
	  else
	    einfo (_("%F%P: unrecognized -a option `%s'\n"), optarg);
	  break;
	case OPTION_ASSERT:
	  /* FIXME: We just ignore these, but we should handle them.  */
	  if (strcmp (optarg, "definitions") == 0)
	    ;
	  else if (strcmp (optarg, "nodefinitions") == 0)
	    ;
	  else if (strcmp (optarg, "nosymbolic") == 0)
	    ;
	  else if (strcmp (optarg, "pure-text") == 0)
	    ;
	  else
	    einfo (_("%F%P: unrecognized -assert option `%s'\n"), optarg);
	  break;
	case 'A':
	  ldfile_add_arch (optarg);
	  break;
	case 'b':
	  lang_add_target (optarg);
	  break;
	case 'c':
	  ldfile_open_command_file (optarg);
	  parser_input = input_mri_script;
	  yyparse ();
	  break;
	case OPTION_CALL_SHARED:
	  input_flags.dynamic = true;
	  break;
	case OPTION_NON_SHARED:
	  input_flags.dynamic = false;
	  break;
	case OPTION_CREF:
	  command_line.cref = true;
	  link_info.notice_all = true;
	  break;
	case 'd':
	  command_line.force_common_definition = true;
	  break;
	case OPTION_FORCE_GROUP_ALLOCATION:
	  command_line.force_group_allocation = true;
	  break;
	case OPTION_DEFSYM:
	  lex_string = optarg;
	  lex_redirect (optarg, "--defsym", ++defsym_count);
	  parser_input = input_defsym;
	  yyparse ();
	  lex_string = NULL;
	  break;
	case OPTION_DEMANGLE:
	  demangling = true;
	  if (optarg != NULL)
	    {
	      enum demangling_styles style;

	      style = cplus_demangle_name_to_style (optarg);
	      if (style == unknown_demangling)
		einfo (_("%F%P: unknown demangling style `%s'\n"),
		       optarg);

	      cplus_demangle_set_style (style);
	    }
	  break;
	case 'I':		/* Used on Solaris.  */
	case OPTION_DYNAMIC_LINKER:
	  command_line.interpreter = optarg;
	  link_info.nointerp = 0;
	  break;
	case OPTION_NO_DYNAMIC_LINKER:
	  link_info.nointerp = 1;
	  break;
	case OPTION_SYSROOT:
	  /* Already handled in ldmain.c.  */
	  break;
	case OPTION_EB:
	  command_line.endian = ENDIAN_BIG;
	  break;
	case OPTION_EL:
	  command_line.endian = ENDIAN_LITTLE;
	  break;
	case OPTION_EMBEDDED_RELOCS:
	  command_line.embedded_relocs = true;
	  break;
	case OPTION_EXPORT_DYNAMIC:
	case 'E': /* HP/UX compatibility.  */
	  link_info.export_dynamic = true;
	  break;
	case OPTION_NO_EXPORT_DYNAMIC:
	  link_info.export_dynamic = false;
	  break;
	case OPTION_NON_CONTIGUOUS_REGIONS:
	  link_info.non_contiguous_regions = true;
	  break;
	case OPTION_NON_CONTIGUOUS_REGIONS_WARNINGS:
	  link_info.non_contiguous_regions_warnings = true;
	  break;
	case OPTION_WARN_EXECSTACK:
	  link_info.warn_execstack = 1;
	  break;
	case OPTION_NO_WARN_EXECSTACK:
	  link_info.warn_execstack = 0;
	  break;
	case OPTION_WARN_RWX_SEGMENTS:
	  link_info.no_warn_rwx_segments = 0;
	  link_info.user_warn_rwx_segments = 1;
	  break;
	case OPTION_NO_WARN_RWX_SEGMENTS:
	  link_info.no_warn_rwx_segments = 1;
	  link_info.user_warn_rwx_segments = 1;
	  break;
	case 'e':
	  lang_add_entry (optarg, true);
	  break;
	case 'f':
	  if (command_line.auxiliary_filters == NULL)
	    {
	      command_line.auxiliary_filters = (char **)
		xmalloc (2 * sizeof (char *));
	      command_line.auxiliary_filters[0] = optarg;
	      command_line.auxiliary_filters[1] = NULL;
	    }
	  else
	    {
	      int c;
	      char **p;

	      c = 0;
	      for (p = command_line.auxiliary_filters; *p != NULL; p++)
		++c;
	      command_line.auxiliary_filters = (char **)
		xrealloc (command_line.auxiliary_filters,
			  (c + 2) * sizeof (char *));
	      command_line.auxiliary_filters[c] = optarg;
	      command_line.auxiliary_filters[c + 1] = NULL;
	    }
	  break;
	case 'F':
	  command_line.filter_shlib = optarg;
	  break;
	case OPTION_FORCE_EXE_SUFFIX:
	  command_line.force_exe_suffix = true;
	  break;
	case 'G':
	  {
	    char *end;
	    g_switch_value = strtoul (optarg, &end, 0);
	    if (*end)
	      einfo (_("%F%P: invalid number `%s'\n"), optarg);
	  }
	  break;
	case 'g':
	  /* Ignore.  */
	  break;
	case OPTION_GC_SECTIONS:
	  link_info.gc_sections = true;
	  break;
	case OPTION_PRINT_GC_SECTIONS:
	  link_info.print_gc_sections = true;
	  break;
	case OPTION_GC_KEEP_EXPORTED:
	  link_info.gc_keep_exported = true;
	  break;
	case OPTION_HELP:
	  help ();
	  xexit (0);
	  break;
	case 'L':
	  ldfile_add_library_path (optarg, true);
	  break;
	case 'l':
	  lang_add_input_file (optarg, lang_input_file_is_l_enum, NULL);
	  break;
	case 'M':
	  config.map_filename = "-";
	  break;
	case 'm':
	  /* Ignore.  Was handled in a pre-parse.   */
	  break;
	case OPTION_MAP:
	  config.map_filename = optarg;
	  break;
	case 'N':
	  config.text_read_only = false;
	  config.magic_demand_paged = false;
	  input_flags.dynamic = false;
	  break;
	case OPTION_NO_OMAGIC:
	  config.text_read_only = true;
	  config.magic_demand_paged = true;
	  /* NB/ Does not set input_flags.dynamic to TRUE.
	     Use --call-shared or -Bdynamic for this.  */
	  break;
	case 'n':
	  config.text_read_only = true;
	  config.magic_demand_paged = false;
	  input_flags.dynamic = false;
	  break;
	case OPTION_NO_DEFINE_COMMON:
	  link_info.inhibit_common_definition = true;
	  break;
	case OPTION_NO_DEMANGLE:
	  demangling = false;
	  break;
	case OPTION_NO_GC_SECTIONS:
	  link_info.gc_sections = false;
	  break;
	case OPTION_NO_PRINT_GC_SECTIONS:
	  link_info.print_gc_sections = false;
	  break;
	case OPTION_NO_KEEP_MEMORY:
	  link_info.keep_memory = false;
	  break;
	case OPTION_NO_UNDEFINED:
	  link_info.unresolved_syms_in_objects = RM_DIAGNOSE;
	  break;
	case OPTION_ALLOW_SHLIB_UNDEFINED:
	  link_info.unresolved_syms_in_shared_libs = RM_IGNORE;
	  break;
	case OPTION_NO_ALLOW_SHLIB_UNDEFINED:
	  link_info.unresolved_syms_in_shared_libs = RM_DIAGNOSE;
	  break;
	case OPTION_UNRESOLVED_SYMBOLS:
	  if (strcmp (optarg, "ignore-all") == 0)
	    {
	      link_info.unresolved_syms_in_objects = RM_IGNORE;
	      link_info.unresolved_syms_in_shared_libs = RM_IGNORE;
	    }
	  else if (strcmp (optarg, "report-all") == 0)
	    {
	      link_info.unresolved_syms_in_objects = RM_DIAGNOSE;
	      link_info.unresolved_syms_in_shared_libs = RM_DIAGNOSE;
	    }
	  else if (strcmp (optarg, "ignore-in-object-files") == 0)
	    {
	      link_info.unresolved_syms_in_objects = RM_IGNORE;
	      link_info.unresolved_syms_in_shared_libs = RM_DIAGNOSE;
	    }
	  else if (strcmp (optarg, "ignore-in-shared-libs") == 0)
	    {
	      link_info.unresolved_syms_in_objects = RM_DIAGNOSE;
	      link_info.unresolved_syms_in_shared_libs = RM_IGNORE;
	    }
	  else
	    einfo (_("%F%P: bad --unresolved-symbols option: %s\n"), optarg);
	  break;
	case OPTION_WARN_UNRESOLVED_SYMBOLS:
	  link_info.warn_unresolved_syms = true;
	  break;
	case OPTION_ERROR_UNRESOLVED_SYMBOLS:
	  link_info.warn_unresolved_syms = false;
	  break;
	case OPTION_ALLOW_MULTIPLE_DEFINITION:
	  link_info.allow_multiple_definition = true;
	  break;

#if SUPPORT_ERROR_HANDLING_SCRIPT
	case OPTION_ERROR_HANDLING_SCRIPT:
	  /* FIXME: Should we warn if the script is being overridden by another ?
	     Or maybe they should be chained together ?  */
	  error_handling_script = optarg;
	  break;
#endif

	case OPTION_ENABLE_LINKER_VERSION:
	  enable_linker_version = true;
	  break;
	case OPTION_DISABLE_LINKER_VERSION:
	  enable_linker_version = false;
	  break;

	case OPTION_UNDEFINED_VERSION:
	  link_info.allow_undefined_version = true;
	  break;
	case OPTION_NO_UNDEFINED_VERSION:
	  link_info.allow_undefined_version = false;
	  break;
	case OPTION_DEFAULT_SYMVER:
	  link_info.create_default_symver = true;
	  break;
	case OPTION_DEFAULT_IMPORTED_SYMVER:
	  link_info.default_imported_symver = true;
	  break;
	case OPTION_NO_WARN_MISMATCH:
	  command_line.warn_mismatch = false;
	  break;
	case OPTION_NO_WARN_SEARCH_MISMATCH:
	  command_line.warn_search_mismatch = false;
	  break;
	case OPTION_NOINHIBIT_EXEC:
	  force_make_executable = true;
	  break;
	case OPTION_NOSTDLIB:
	  config.only_cmd_line_lib_dirs = true;
	  break;
	case OPTION_NO_WHOLE_ARCHIVE:
	  input_flags.whole_archive = false;
	  break;
	case 'O':
	  /* FIXME "-O<non-digits> <value>" used to set the address of
	     section <non-digits>.  Was this for compatibility with
	     something, or can we create a new option to do that
	     (with a syntax similar to -defsym)?
	     getopt can't handle two args to an option without kludges.  */

	  /* Enable optimizations of output files.  */
	  link_info.optimize = strtoul (optarg, NULL, 0) != 0;
	  break;
	case 'o':
	  lang_add_output (optarg, 0);
	  break;
	case OPTION_OFORMAT:
	  lang_add_output_format (optarg, NULL, NULL, 0);
	  break;
	case OPTION_OUT_IMPLIB:
	  command_line.out_implib_filename = xstrdup (optarg);
	  break;
	case OPTION_PRINT_SYSROOT:
	  if (*ld_sysroot)
	    puts (ld_sysroot);
	  xexit (0);
	  break;
	case OPTION_PRINT_OUTPUT_FORMAT:
	  command_line.print_output_format = true;
	  break;
#if BFD_SUPPORTS_PLUGINS
	case OPTION_PLUGIN:
	  plugin_opt_plugin (optarg);
	  break;
	case OPTION_PLUGIN_OPT:
	  if (plugin_opt_plugin_arg (optarg))
	    einfo (_("%F%P: bad -plugin-opt option\n"));
	  break;
#endif /* BFD_SUPPORTS_PLUGINS */
	case 'q':
	  link_info.emitrelocations = true;
	  break;
	case 'i':
	case 'r':
	  if (optind == last_optind)
	    /* This can happen if the user put "-rpath,a" on the command
	       line.  (Or something similar.  The comma is important).
	       Getopt becomes confused and thinks that this is a -r option
	       but it cannot parse the text after the -r so it refuses to
	       increment the optind counter.  Detect this case and issue
	       an error message here.  We cannot just make this a warning,
	       increment optind, and continue because getopt is too confused
	       and will seg-fault the next time around.  */
	    einfo(_("%F%P: unrecognised option: %s\n"), argv[optind]);

	  if (bfd_link_pic (&link_info))
	    einfo (_("%F%P: -r and %s may not be used together\n"),
		     bfd_link_dll (&link_info) ? "-shared" : "-pie");

	  link_info.type = type_relocatable;
	  config.build_constructors = false;
	  config.magic_demand_paged = false;
	  config.text_read_only = false;
	  input_flags.dynamic = false;
	  break;
	case 'R':
	  /* The GNU linker traditionally uses -R to mean to include
	     only the symbols from a file.  The Solaris linker uses -R
	     to set the path used by the runtime linker to find
	     libraries.  This is the GNU linker -rpath argument.  We
	     try to support both simultaneously by checking the file
	     named.  If it is a directory, rather than a regular file,
	     we assume -rpath was meant.  */
	  {
	    struct stat s;

	    if (stat (optarg, &s) >= 0
		&& ! S_ISDIR (s.st_mode))
	      {
		lang_add_input_file (optarg,
				     lang_input_file_is_symbols_only_enum,
				     NULL);
		break;
	      }
	  }
	  /* Fall through.  */
	case OPTION_RPATH:
	  if (command_line.rpath == NULL)
	    command_line.rpath = xstrdup (optarg);
	  else
	    {
	      size_t rpath_len = strlen (command_line.rpath);
	      size_t optarg_len = strlen (optarg);
	      char *buf;
	      char *cp = command_line.rpath;

	      /* First see whether OPTARG is already in the path.  */
	      do
		{
		  if (strncmp (optarg, cp, optarg_len) == 0
		      && (cp[optarg_len] == 0
			  || cp[optarg_len] == config.rpath_separator))
		    /* We found it.  */
		    break;

		  /* Not yet found.  */
		  cp = strchr (cp, config.rpath_separator);
		  if (cp != NULL)
		    ++cp;
		}
	      while (cp != NULL);

	      if (cp == NULL)
		{
		  buf = (char *) xmalloc (rpath_len + optarg_len + 2);
		  sprintf (buf, "%s%c%s", command_line.rpath,
			   config.rpath_separator, optarg);
		  free (command_line.rpath);
		  command_line.rpath = buf;
		}
	    }
	  break;
	case OPTION_RPATH_LINK:
	  if (command_line.rpath_link == NULL)
	    command_line.rpath_link = xstrdup (optarg);
	  else
	    {
	      char *buf;

	      buf = (char *) xmalloc (strlen (command_line.rpath_link)
				      + strlen (optarg)
				      + 2);
	      sprintf (buf, "%s%c%s", command_line.rpath_link,
		       config.rpath_separator, optarg);
	      free (command_line.rpath_link);
	      command_line.rpath_link = buf;
	    }
	  break;
	case OPTION_NO_RELAX:
	  DISABLE_RELAXATION;
	  break;
	case OPTION_RELAX:
	  ENABLE_RELAXATION;
	  break;
	case OPTION_RETAIN_SYMBOLS_FILE:
	  add_keepsyms_file (optarg);
	  break;
	case 'S':
	  link_info.strip = strip_debugger;
	  break;
	case 's':
	  link_info.strip = strip_all;
	  break;
	case OPTION_STRIP_DISCARDED:
	  link_info.strip_discarded = true;
	  break;
	case OPTION_NO_STRIP_DISCARDED:
	  link_info.strip_discarded = false;
	  break;
	case OPTION_DISABLE_MULTIPLE_DEFS_ABS:
	  link_info.prohibit_multiple_definition_absolute = true;
	  break;
	case OPTION_SHARED:
	  if (config.has_shared)
	    {
	      if (bfd_link_relocatable (&link_info))
		einfo (_("%F%P: -r and %s may not be used together\n"),
		       "-shared");

	      link_info.type = type_dll;
	      /* When creating a shared library, the default
		 behaviour is to ignore any unresolved references.  */
	      if (link_info.unresolved_syms_in_objects == RM_NOT_YET_SET)
		link_info.unresolved_syms_in_objects = RM_IGNORE;
	      if (link_info.unresolved_syms_in_shared_libs == RM_NOT_YET_SET)
		link_info.unresolved_syms_in_shared_libs = RM_IGNORE;
	    }
	  else
	    einfo (_("%F%P: -shared not supported\n"));
	  break;
	case OPTION_NO_PIE:
	  link_info.type = type_pde;
	  break;
	case OPTION_PIE:
	  if (config.has_shared)
	    {
	      if (bfd_link_relocatable (&link_info))
		einfo (_("%F%P: -r and %s may not be used together\n"), "-pie");

	      link_info.type = type_pie;
	    }
	  else
	    einfo (_("%F%P: -pie not supported\n"));
	  break;
	case 'h':		/* Used on Solaris.  */
	case OPTION_SONAME:
	  if (optarg[0] == '\0' && command_line.soname
	      && command_line.soname[0])
	    einfo (_("%P: SONAME must not be empty string; keeping previous one\n"));
	  else
	    command_line.soname = optarg;
	  break;
	case OPTION_SORT_COMMON:
	  if (optarg == NULL
	      || strcmp (optarg, N_("descending")) == 0)
	    config.sort_common = sort_descending;
	  else if (strcmp (optarg, N_("ascending")) == 0)
	    config.sort_common = sort_ascending;
	  else
	    einfo (_("%F%P: invalid common section sorting option: %s\n"),
		   optarg);
	  break;
	case OPTION_SORT_SECTION:
	  if (strcmp (optarg, N_("name")) == 0)
	    sort_section = by_name;
	  else if (strcmp (optarg, N_("alignment")) == 0)
	    sort_section = by_alignment;
	  else
	    einfo (_("%F%P: invalid section sorting option: %s\n"),
		   optarg);
	  break;
	case OPTION_STATS:
	  config.stats = true;
	  break;
	case OPTION_NO_SYMBOLIC:
	  opt_symbolic = symbolic_unset;
	  break;
	case OPTION_SYMBOLIC:
	  opt_symbolic = symbolic;
	  break;
	case OPTION_SYMBOLIC_FUNCTIONS:
	  opt_symbolic = symbolic_functions;
	  break;
	case 't':
	  ++trace_files;
	  break;
	case 'T':
	  previous_script_handle = saved_script_handle;
	  ldfile_open_script_file (optarg);
	  parser_input = input_script;
	  yyparse ();
	  previous_script_handle = NULL;
	  break;
	case OPTION_DEFAULT_SCRIPT:
	  command_line.default_script = optarg;
	  break;
	case OPTION_SECTION_START:
	  {
	    char *optarg2;
	    char *sec_name;
	    int len;

	    /* Check for <something>=<somthing>...  */
	    optarg2 = strchr (optarg, '=');
	    if (optarg2 == NULL)
	      einfo (_("%F%P: invalid argument to option"
		       " \"--section-start\"\n"));

	    optarg2++;

	    /* So far so good.  Are all the args present?  */
	    if ((*optarg == '\0') || (*optarg2 == '\0'))
	      einfo (_("%F%P: missing argument(s) to option"
		       " \"--section-start\"\n"));

	    /* We must copy the section name as set_section_start
	       doesn't do it for us.  */
	    len = optarg2 - optarg;
	    sec_name = (char *) xmalloc (len);
	    memcpy (sec_name, optarg, len - 1);
	    sec_name[len - 1] = 0;

	    /* Then set it...  */
	    set_section_start (sec_name, optarg2);
	  }
	  break;
	case OPTION_TARGET_HELP:
	  /* Mention any target specific options.  */
	  ldemul_list_emulation_options (stdout);
	  exit (0);
	case OPTION_TBSS:
	  set_segment_start (".bss", optarg);
	  break;
	case OPTION_TDATA:
	  set_segment_start (".data", optarg);
	  break;
	case OPTION_TTEXT:
	  set_segment_start (".text", optarg);
	  break;
	case OPTION_TTEXT_SEGMENT:
	  set_segment_start (".text-segment", optarg);
	  break;
	case OPTION_TRODATA_SEGMENT:
	  set_segment_start (".rodata-segment", optarg);
	  break;
	case OPTION_TLDATA_SEGMENT:
	  set_segment_start (".ldata-segment", optarg);
	  break;
	case OPTION_TRADITIONAL_FORMAT:
	  link_info.traditional_format = true;
	  break;
	case OPTION_TASK_LINK:
	  link_info.task_link = true;
	  /* Fall through.  */
	case OPTION_UR:
	  if (bfd_link_pic (&link_info))
	    einfo (_("%F%P: -r and %s may not be used together\n"),
		     bfd_link_dll (&link_info) ? "-shared" : "-pie");

	  link_info.type = type_relocatable;
	  config.build_constructors = true;
	  config.magic_demand_paged = false;
	  config.text_read_only = false;
	  input_flags.dynamic = false;
	  break;
	case 'u':
	  ldlang_add_undef (optarg, true);
	  break;
	case OPTION_REQUIRE_DEFINED_SYMBOL:
	  ldlang_add_require_defined (optarg);
	  break;
	case OPTION_UNIQUE:
	  if (optarg != NULL)
	    lang_add_unique (optarg);
	  else
	    config.unique_orphan_sections = true;
	  break;
	case OPTION_VERBOSE:
	  ldversion (1);
	  version_printed = true;
	  verbose = true;
	  overflow_cutoff_limit = -2;
	  if (optarg != NULL)
	    {
	      char *end;
	      int level ATTRIBUTE_UNUSED = strtoul (optarg, &end, 0);
	      if (*end)
		einfo (_("%F%P: invalid number `%s'\n"), optarg);
#if BFD_SUPPORTS_PLUGINS
	      report_plugin_symbols = level > 1;
#endif /* BFD_SUPPORTS_PLUGINS */
	    }
	  break;
	case 'v':
	  ldversion (0);
	  version_printed = true;
	  break;
	case 'V':
	  ldversion (1);
	  version_printed = true;
	  break;
	case OPTION_VERSION:
	  ldversion (2);
	  xexit (0);
	  break;
	case OPTION_VERSION_SCRIPT:
	  /* This option indicates a small script that only specifies
	     version information.  Read it, but don't assume that
	     we've seen a linker script.  */
	  {
	    FILE *hold_script_handle;

	    hold_script_handle = saved_script_handle;
	    ldfile_open_command_file (optarg);
	    saved_script_handle = hold_script_handle;
	    parser_input = input_version_script;
	    yyparse ();
	  }
	  break;
	case OPTION_VERSION_EXPORTS_SECTION:
	  /* This option records a version symbol to be applied to the
	     symbols listed for export to be found in the object files
	     .exports sections.  */
	  command_line.version_exports_section = optarg;
	  break;
	case OPTION_DYNAMIC_LIST_DATA:
	  opt_dynamic_list = dynamic_list_data;
	  break;
	case OPTION_DYNAMIC_LIST_CPP_TYPEINFO:
	  lang_append_dynamic_list_cpp_typeinfo ();
	  if (opt_dynamic_list != dynamic_list_data)
	    opt_dynamic_list = dynamic_list;
	  break;
	case OPTION_DYNAMIC_LIST_CPP_NEW:
	  lang_append_dynamic_list_cpp_new ();
	  if (opt_dynamic_list != dynamic_list_data)
	    opt_dynamic_list = dynamic_list;
	  break;
	case OPTION_DYNAMIC_LIST:
	  /* This option indicates a small script that only specifies
	     a dynamic list.  Read it, but don't assume that we've
	     seen a linker script.  */
	  {
	    FILE *hold_script_handle;

	    hold_script_handle = saved_script_handle;
	    ldfile_open_command_file (optarg);
	    saved_script_handle = hold_script_handle;
	    parser_input = input_dynamic_list;
	    current_dynamic_list_p = &link_info.dynamic_list;
	    yyparse ();
	  }
	  if (opt_dynamic_list != dynamic_list_data)
	    opt_dynamic_list = dynamic_list;
	  break;
	case OPTION_EXPORT_DYNAMIC_SYMBOL:
	  {
	    struct bfd_elf_version_expr *expr
	      = lang_new_vers_pattern (NULL, xstrdup (optarg), NULL,
				       false);
	    lang_append_dynamic_list (&export_list, expr);
	  }
	  break;
	case OPTION_EXPORT_DYNAMIC_SYMBOL_LIST:
	  /* This option indicates a small script that only specifies
	     an export list.  Read it, but don't assume that we've
	     seen a linker script.  */
	  {
	    FILE *hold_script_handle;

	    hold_script_handle = saved_script_handle;
	    ldfile_open_command_file (optarg);
	    saved_script_handle = hold_script_handle;
	    parser_input = input_dynamic_list;
	    current_dynamic_list_p = &export_list;
	    yyparse ();
	  }
	  break;
	case OPTION_WARN_COMMON:
	  config.warn_common = true;
	  break;
	case OPTION_WARN_CONSTRUCTORS:
	  config.warn_constructors = true;
	  break;
	case OPTION_WARN_FATAL:
	  config.fatal_warnings = true;
	  break;
	case OPTION_NO_WARN_FATAL:
	  config.fatal_warnings = false;
	  break;
	case OPTION_NO_WARNINGS:
	case 'w':
	  config.no_warnings = true;
	  config.fatal_warnings = false;
	  break;
	case OPTION_WARN_MULTIPLE_GP:
	  config.warn_multiple_gp = true;
	  break;
	case OPTION_WARN_ONCE:
	  config.warn_once = true;
	  break;
	case OPTION_WARN_SECTION_ALIGN:
	  config.warn_section_align = true;
	  break;
	case OPTION_WARN_TEXTREL:
	  link_info.textrel_check = textrel_check_warning;
	  break;
	case OPTION_WARN_ALTERNATE_EM:
	  link_info.warn_alternate_em = true;
	  break;
	case OPTION_WHOLE_ARCHIVE:
	  input_flags.whole_archive = true;
	  break;
	case OPTION_ADD_DT_NEEDED_FOR_DYNAMIC:
	  input_flags.add_DT_NEEDED_for_dynamic = true;
	  break;
	case OPTION_NO_ADD_DT_NEEDED_FOR_DYNAMIC:
	  input_flags.add_DT_NEEDED_for_dynamic = false;
	  break;
	case OPTION_ADD_DT_NEEDED_FOR_REGULAR:
	  input_flags.add_DT_NEEDED_for_regular = true;
	  break;
	case OPTION_NO_ADD_DT_NEEDED_FOR_REGULAR:
	  input_flags.add_DT_NEEDED_for_regular = false;
	  break;
	case OPTION_WRAP:
	  add_wrap (optarg);
	  break;
	case OPTION_IGNORE_UNRESOLVED_SYMBOL:
	  add_ignoresym (&link_info, optarg);
	  break;
	case OPTION_DISCARD_NONE:
	  link_info.discard = discard_none;
	  break;
	case 'X':
	  link_info.discard = discard_l;
	  break;
	case 'x':
	  link_info.discard = discard_all;
	  break;
	case 'Y':
	  if (startswith (optarg, "P,"))
	    optarg += 2;
	  free (default_dirlist);
	  default_dirlist = xstrdup (optarg);
	  break;
	case 'y':
	  add_ysym (optarg);
	  break;
	case OPTION_SPARE_DYNAMIC_TAGS:
	  link_info.spare_dynamic_tags = strtoul (optarg, NULL, 0);
	  break;
	case OPTION_SPLIT_BY_RELOC:
	  if (optarg != NULL)
	    config.split_by_reloc = strtoul (optarg, NULL, 0);
	  else
	    config.split_by_reloc = 32768;
	  break;
	case OPTION_SPLIT_BY_FILE:
	  if (optarg != NULL)
	    config.split_by_file = bfd_scan_vma (optarg, NULL, 0);
	  else
	    config.split_by_file = 1;
	  break;
	case OPTION_CHECK_SECTIONS:
	  command_line.check_section_addresses = 1;
	  break;
	case OPTION_NO_CHECK_SECTIONS:
	  command_line.check_section_addresses = 0;
	  break;
	case OPTION_ACCEPT_UNKNOWN_INPUT_ARCH:
	  command_line.accept_unknown_input_arch = true;
	  break;
	case OPTION_NO_ACCEPT_UNKNOWN_INPUT_ARCH:
	  command_line.accept_unknown_input_arch = false;
	  break;
	case '(':
	  lang_enter_group ();
	  ingroup++;
	  break;
	case ')':
	  if (! ingroup)
	    einfo (_("%F%P: group ended before it began (--help for usage)\n"));

	  lang_leave_group ();
	  ingroup--;
	  break;

	case OPTION_INIT:
	  link_info.init_function = optarg;
	  break;

	case OPTION_FINI:
	  link_info.fini_function = optarg;
	  break;

	case OPTION_REMAP_INPUTS_FILE:
	  if (! ldfile_add_remap_file (optarg))
	    einfo (_("%F%P: failed to add remap file %s\n"), optarg);
	  break;

	case OPTION_REMAP_INPUTS:
	  {
	    char *optarg2 = strchr (optarg, '=');
	    if (optarg2 == NULL)
	      /* FIXME: Should we allow --remap-inputs=@myfile as a synonym
		 for --remap-inputs-file=myfile ?  */
	      einfo (_("%F%P: invalid argument to option --remap-inputs\n"));
	    size_t len = optarg2 - optarg;
	    char * pattern = xmalloc (len + 1);
	    memcpy (pattern, optarg, len);
	    pattern[len] = 0;
	    ldfile_add_remap (pattern, optarg2 + 1);
	    free (pattern);
	  }
	  break;

	case OPTION_REDUCE_MEMORY_OVERHEADS:
	  link_info.reduce_memory_overheads = true;
	  if (config.hash_table_size == 0)
	    config.hash_table_size = 1021;
	  break;

	case OPTION_MAX_CACHE_SIZE:
	  {
	    char *end;
	    bfd_size_type cache_size = strtoul (optarg, &end, 0);
	    if (*end != '\0')
	      einfo (_("%F%P: invalid cache memory size: %s\n"),
		     optarg);
	    link_info.max_cache_size = cache_size;
	  }
	  break;

	case OPTION_HASH_SIZE:
	  {
	    bfd_size_type new_size;

	    new_size = strtoul (optarg, NULL, 0);
	    if (new_size)
	      config.hash_table_size = new_size;
	    else
	      einfo (_("%X%P: --hash-size needs a numeric argument\n"));
	  }
	  break;

	case OPTION_PUSH_STATE:
	  input_flags.pushed = xmemdup (&input_flags,
					sizeof (input_flags),
					sizeof (input_flags));
	  break;

	case OPTION_POP_STATE:
	  if (input_flags.pushed == NULL)
	    einfo (_("%F%P: no state pushed before popping\n"));
	  else
	    {
	      struct lang_input_statement_flags *oldp = input_flags.pushed;
	      memcpy (&input_flags, oldp, sizeof (input_flags));
	      free (oldp);
	    }
	  break;

	case OPTION_PRINT_MEMORY_USAGE:
	  command_line.print_memory_usage = true;
	  break;

	case OPTION_ORPHAN_HANDLING:
	  if (strcasecmp (optarg, "place") == 0)
	    config.orphan_handling = orphan_handling_place;
	  else if (strcasecmp (optarg, "warn") == 0)
	    config.orphan_handling = orphan_handling_warn;
	  else if (strcasecmp (optarg, "error") == 0)
	    config.orphan_handling = orphan_handling_error;
	  else if (strcasecmp (optarg, "discard") == 0)
	    config.orphan_handling = orphan_handling_discard;
	  else
	    einfo (_("%F%P: invalid argument to option"
		     " \"--orphan-handling\"\n"));
	  break;

	case OPTION_NO_PRINT_MAP_DISCARDED:
	  config.print_map_discarded = false;
	  break;

	case OPTION_PRINT_MAP_DISCARDED:
	  config.print_map_discarded = true;
	  break;

	case OPTION_NO_PRINT_MAP_LOCALS:
	  config.print_map_locals = false;
	  break;

	case OPTION_PRINT_MAP_LOCALS:
	  config.print_map_locals = true;
	  break;

	case OPTION_DEPENDENCY_FILE:
	  config.dependency_file = optarg;
	  break;

	case OPTION_CTF_VARIABLES:
	  config.ctf_variables = true;
	  break;

	case OPTION_NO_CTF_VARIABLES:
	  config.ctf_variables = false;
	  break;

	case OPTION_CTF_SHARE_TYPES:
	  if (strcmp (optarg, "share-unconflicted") == 0)
	    config.ctf_share_duplicated = false;
	  else if (strcmp (optarg, "share-duplicated") == 0)
	    config.ctf_share_duplicated = true;
	  else
	    einfo (_("%F%P: bad --ctf-share-types option: %s\n"), optarg);
	  break;
	}
    }

  free (really_longopts);
  free (longopts);
  free (shortopts);

  /* Run a couple of checks on the map filename.  */
  if (config.map_filename)
    {
      char * new_name = NULL;
      char * percent;
      int    res = 0;

      if (config.map_filename[0] == 0)
	{
	  einfo (_("%P: no file/directory name provided for map output; ignored\n"));
	  config.map_filename = NULL;
	}
      else if (strcmp (config.map_filename, "-") == 0)
	; /* Write to stdout.  Handled in main().  */
      else if ((percent = strchr (config.map_filename, '%')) != NULL)
	{
	  /* FIXME: Check for a second % character and issue an error ?  */

	  /* Construct a map file by replacing the % character with the (full)
	     output filename.  If the % character was the last character in
	     the original map filename then add a .map extension.  */
	  percent[0] = 0;
	  res = asprintf (&new_name, "%s%s%s", config.map_filename,
			  output_filename,
			  percent[1] ? percent + 1 : ".map");

	  /* FIXME: Should we ensure that any directory components in new_name exist ?  */
	}
      else
	{
	  struct stat s;

	  /* If the map filename is actually a directory then create
	     a file inside it, based upon the output filename.  */
	  if (stat (config.map_filename, &s) < 0)
	    {
	      if (errno != ENOENT)
		einfo (_("%P: cannot stat linker map file: %E\n"));
	    }
	  else if (S_ISDIR (s.st_mode))
	    {
	      char lastc = config.map_filename[strlen (config.map_filename) - 1];
	      res = asprintf (&new_name, "%s%s%s.map",
			      config.map_filename,
			      IS_DIR_SEPARATOR (lastc) ? "" : "/",
			      lbasename (output_filename));
	    }
	  else if (! S_ISREG (s.st_mode))
	    {
	      einfo (_("%P: linker map file is not a regular file\n"));
	      config.map_filename = NULL;
	    }
	  /* else FIXME: Check write permission ?  */
	}

      if (res < 0)
	{
	  /* If the asprintf failed then something is probably very
	     wrong.  Better to halt now rather than continue on
	     into more problems.  */
	  einfo (_("%P%F: cannot create name for linker map file: %E\n"));
	}
      else if (new_name != NULL)
	{
	  /* This is a trivial memory leak.  */
	  config.map_filename = new_name;
	}
    }

  if (command_line.soname && command_line.soname[0] == '\0')
    {
      einfo (_("%P: SONAME must not be empty string; ignored\n"));
      command_line.soname = NULL;
    }

  while (ingroup)
    {
      einfo (_("%P: missing --end-group; added as last command line option\n"));
      lang_leave_group ();
      ingroup--;
    }

  if (default_dirlist != NULL)
    {
      set_default_dirlist (default_dirlist);
      free (default_dirlist);
    }

  if (link_info.unresolved_syms_in_objects == RM_NOT_YET_SET)
    /* FIXME: Should we allow emulations a chance to set this ?  */
    link_info.unresolved_syms_in_objects = RM_DIAGNOSE;

  if (link_info.unresolved_syms_in_shared_libs == RM_NOT_YET_SET)
    /* FIXME: Should we allow emulations a chance to set this ?  */
    link_info.unresolved_syms_in_shared_libs = RM_DIAGNOSE;

  if (bfd_link_relocatable (&link_info)
      && command_line.check_section_addresses < 0)
    command_line.check_section_addresses = 0;

  if (export_list)
    {
      struct bfd_elf_version_expr *head = export_list->head.list;
      struct bfd_elf_version_expr *next;

      /* For --export-dynamic-symbol[-list]:
	 1. When building executable, treat like --dynamic-list.
	 2. When building shared object:
	    a. If -Bsymbolic or --dynamic-list are used, treat like
	       --dynamic-list.
	    b. Otherwise, ignored.
       */
      if (!bfd_link_relocatable (&link_info)
	  && (bfd_link_executable (&link_info)
	      || opt_symbolic != symbolic_unset
	      || opt_dynamic_list != dynamic_list_unset))
	{
	  /* Append the export list to link_info.dynamic_list.  */
	  if (link_info.dynamic_list)
	    {
	      for (next = head; next->next != NULL; next = next->next)
		;
	      next->next = link_info.dynamic_list->head.list;
	      link_info.dynamic_list->head.list = head;
	    }
	  else
	    link_info.dynamic_list = export_list;

	  if (opt_dynamic_list != dynamic_list_data)
	    opt_dynamic_list = dynamic_list;
	}
      else
	{
	  /* Free the export list.  */
	  for (; head->next != NULL; head = next)
	    {
	      next = head->next;
	      free (head);
	    }
	  free (export_list);
	}
    }

  switch (opt_dynamic_list)
    {
    case dynamic_list_unset:
      break;
    case dynamic_list_data:
      link_info.dynamic_data = true;
      /* Fall through.  */
    case dynamic_list:
      link_info.dynamic = true;
      opt_symbolic = symbolic_unset;
      break;
    }

  /* -Bsymbolic and -Bsymbols-functions are for shared library output.  */
  if (bfd_link_dll (&link_info))
    switch (opt_symbolic)
      {
      case symbolic_unset:
	break;
      case symbolic:
	link_info.symbolic = true;
	if (link_info.dynamic_list)
	  {
	    struct bfd_elf_version_expr *ent, *next;
	    for (ent = link_info.dynamic_list->head.list; ent; ent = next)
	      {
		next = ent->next;
		free (ent);
	      }
	    free (link_info.dynamic_list);
	    link_info.dynamic_list = NULL;
	  }
	break;
      case symbolic_functions:
	link_info.dynamic = true;
	link_info.dynamic_data = true;
	break;
      }

  /* -z nosectionheader implies --strip-all.  */
  if (config.no_section_header)
    {
      if (bfd_link_relocatable (&link_info))
	einfo (_("%F%P: -r and -z nosectionheader may not be used together\n"));

      link_info.strip = strip_all;
    }

  if (!bfd_link_dll (&link_info))
    {
      if (command_line.filter_shlib)
	einfo (_("%F%P: -F may not be used without -shared\n"));
      if (command_line.auxiliary_filters)
	einfo (_("%F%P: -f may not be used without -shared\n"));
    }

  /* Treat ld -r -s as ld -r -S -x (i.e., strip all local symbols).  I
     don't see how else this can be handled, since in this case we
     must preserve all externally visible symbols.  */
  if (bfd_link_relocatable (&link_info) && link_info.strip == strip_all)
    {
      link_info.strip = strip_debugger;
      if (link_info.discard == discard_sec_merge)
	link_info.discard = discard_all;
    }
}

/* Add the (colon-separated) elements of DIRLIST_PTR to the
   library search path.  */

static void
set_default_dirlist (char *dirlist_ptr)
{
  char *p;

  while (1)
    {
      p = strchr (dirlist_ptr, PATH_SEPARATOR);
      if (p != NULL)
	*p = '\0';
      if (*dirlist_ptr != '\0')
	ldfile_add_library_path (dirlist_ptr, true);
      if (p == NULL)
	break;
      dirlist_ptr = p + 1;
    }
}

static void
set_section_start (char *sect, char *valstr)
{
  const char *end;
  bfd_vma val = bfd_scan_vma (valstr, &end, 16);
  if (*end)
    einfo (_("%F%P: invalid hex number `%s'\n"), valstr);
  lang_section_start (sect, exp_intop (val), NULL);
}

static void
set_segment_start (const char *section, char *valstr)
{
  const char *name;
  const char *end;
  segment_type *seg;

  bfd_vma val = bfd_scan_vma (valstr, &end, 16);
  if (*end)
    einfo (_("%F%P: invalid hex number `%s'\n"), valstr);
  /* If we already have an entry for this segment, update the existing
     value.  */
  name = section + 1;
  for (seg = segments; seg; seg = seg->next)
    if (strcmp (seg->name, name) == 0)
      {
	seg->value = val;
	lang_section_start (section, exp_intop (val), seg);
	return;
      }
  /* There was no existing value so we must create a new segment
     entry.  */
  seg = stat_alloc (sizeof (*seg));
  seg->name = name;
  seg->value = val;
  seg->used = false;
  /* Add it to the linked list of segments.  */
  seg->next = segments;
  segments = seg;
  /* Historically, -Ttext and friends set the base address of a
     particular section.  For backwards compatibility, we still do
     that.  If a SEGMENT_START directive is seen, the section address
     assignment will be disabled.  */
  lang_section_start (section, exp_intop (val), seg);
}

static void
elf_shlib_list_options (FILE *file)
{
  fprintf (file, _("\
  --audit=AUDITLIB            Specify a library to use for auditing\n"));
  fprintf (file, _("\
  -Bgroup                     Selects group name lookup rules for DSO\n"));
  fprintf (file, _("\
  --disable-new-dtags         Disable new dynamic tags\n"));
  fprintf (file, _("\
  --enable-new-dtags          Enable new dynamic tags\n"));
  fprintf (file, _("\
  --eh-frame-hdr              Create .eh_frame_hdr section\n"));
  fprintf (file, _("\
  --no-eh-frame-hdr           Do not create .eh_frame_hdr section\n"));
  fprintf (file, _("\
  --exclude-libs=LIBS         Make all symbols in LIBS hidden\n"));
  fprintf (file, _("\
  --hash-style=STYLE          Set hash style to sysv/gnu/both.  Default: "));
  if (DEFAULT_EMIT_SYSV_HASH)
    {
      /* Note - these strings are not translated as
	 they are keywords not descriptive text.  */
      if (DEFAULT_EMIT_GNU_HASH)
	fprintf (file, "both\n");
      else
	fprintf (file, "sysv\n");
    }
  else
    {
      if (DEFAULT_EMIT_GNU_HASH)
	fprintf (file, "gnu\n");
      else
	/* FIXME: Can this happen ?  */
	fprintf (file, "none\n");
    }
  fprintf (file, _("\
  -P AUDITLIB, --depaudit=AUDITLIB\n" "\
                              Specify a library to use for auditing dependencies\n"));
  fprintf (file, _("\
  -z combreloc                Merge dynamic relocs into one section and sort\n"));
  fprintf (file, _("\
  -z nocombreloc              Don't merge dynamic relocs into one section\n"));
  fprintf (file, _("\
  -z global                   Make symbols in DSO available for subsequently\n\
                                loaded objects\n"));
  fprintf (file, _("\
  -z initfirst                Mark DSO to be initialized first at runtime\n"));
  fprintf (file, _("\
  -z interpose                Mark object to interpose all DSOs but executable\n"));
  fprintf (file, _("\
  -z unique                   Mark DSO to be loaded at most once by default, and only in the main namespace\n"));
  fprintf (file, _("\
  -z nounique                 Don't mark DSO as a loadable at most once\n"));
  fprintf (file, _("\
  -z lazy                     Mark object lazy runtime binding (default)\n"));
  fprintf (file, _("\
  -z loadfltr                 Mark object requiring immediate process\n"));
  fprintf (file, _("\
  -z nocopyreloc              Don't create copy relocs\n"));
  fprintf (file, _("\
  -z nodefaultlib             Mark object not to use default search paths\n"));
  fprintf (file, _("\
  -z nodelete                 Mark DSO non-deletable at runtime\n"));
  fprintf (file, _("\
  -z nodlopen                 Mark DSO not available to dlopen\n"));
  fprintf (file, _("\
  -z nodump                   Mark DSO not available to dldump\n"));
  fprintf (file, _("\
  -z now                      Mark object non-lazy runtime binding\n"));
  fprintf (file, _("\
  -z origin                   Mark object requiring immediate $ORIGIN\n\
                                processing at runtime\n"));
#if DEFAULT_LD_Z_RELRO
  fprintf (file, _("\
  -z relro                    Create RELRO program header (default)\n"));
  fprintf (file, _("\
  -z norelro                  Don't create RELRO program header\n"));
#else
  fprintf (file, _("\
  -z relro                    Create RELRO program header\n"));
  fprintf (file, _("\
  -z norelro                  Don't create RELRO program header (default)\n"));
#endif
#if DEFAULT_LD_Z_SEPARATE_CODE
  fprintf (file, _("\
  -z separate-code            Create separate code program header (default)\n"));
  fprintf (file, _("\
  -z noseparate-code          Don't create separate code program header\n"));
#else
  fprintf (file, _("\
  -z separate-code            Create separate code program header\n"));
  fprintf (file, _("\
  -z noseparate-code          Don't create separate code program header (default)\n"));
#endif
  fprintf (file, _("\
  -z common                   Generate common symbols with STT_COMMON type\n"));
  fprintf (file, _("\
  -z nocommon                 Generate common symbols with STT_OBJECT type\n"));
  if (link_info.textrel_check == textrel_check_error)
    fprintf (file, _("\
  -z text                     Treat DT_TEXTREL in output as error (default)\n"));
  else
    fprintf (file, _("\
  -z text                     Treat DT_TEXTREL in output as error\n"));
  if (link_info.textrel_check == textrel_check_none)
    {
      fprintf (file, _("\
  -z notext                   Don't treat DT_TEXTREL in output as error (default)\n"));
      fprintf (file, _("\
  -z textoff                  Don't treat DT_TEXTREL in output as error (default)\n"));
    }
  else
    {
      fprintf (file, _("\
  -z notext                   Don't treat DT_TEXTREL in output as error\n"));
      fprintf (file, _("\
  -z textoff                  Don't treat DT_TEXTREL in output as error\n"));
    }
}

static void
elf_static_list_options (FILE *file)
{
  fprintf (file, _("\
  --build-id[=STYLE]          Generate build ID note\n"));
  fprintf (file, _("\
  --package-metadata[=JSON]   Generate package metadata note\n"));
  fprintf (file, _("\
  --compress-debug-sections=[none|zlib|zlib-gnu|zlib-gabi|zstd]\n\
			      Compress DWARF debug sections\n"));
  fprintf (file, _("\
                                Default: %s\n"),
	   bfd_get_compression_algorithm_name (config.compress_debug));
  fprintf (file, _("\
  -z common-page-size=SIZE    Set common page size to SIZE\n"));
  fprintf (file, _("\
  -z max-page-size=SIZE       Set maximum page size to SIZE\n"));
  fprintf (file, _("\
  -z defs                     Report unresolved symbols in object files\n"));
  fprintf (file, _("\
  -z undefs                   Ignore unresolved symbols in object files\n"));
  fprintf (file, _("\
  -z muldefs                  Allow multiple definitions\n"));
  fprintf (file, _("\
  -z stack-size=SIZE          Set size of stack segment\n"));
  fprintf (file, _("\
  -z execstack                Mark executable as requiring executable stack\n"));
  fprintf (file, _("\
  -z noexecstack              Mark executable as not requiring executable stack\n"));
#if DEFAULT_LD_WARN_EXECSTACK == 1
  fprintf (file, _("\
  --warn-execstack            Generate a warning if the stack is executable (default)\n"));
#else
  fprintf (file, _("\
  --warn-execstack            Generate a warning if the stack is executable\n"));
#endif
#if DEFAULT_LD_WARN_EXECSTACK == 0
  fprintf (file, _("\
  --no-warn-execstack         Do not generate a warning if the stack is executable (default)\n"));
#else
  fprintf (file, _("\
  --no-warn-execstack         Do not generate a warning if the stack is executable\n"));
#endif
#if DEFAULT_LD_WARN_RWX_SEGMENTS
  fprintf (file, _("\
  --warn-rwx-segments         Generate a warning if a LOAD segment has RWX permissions (default)\n"));
  fprintf (file, _("\
  --no-warn-rwx-segments      Do not generate a warning if a LOAD segments has RWX permissions\n"));
#else
  fprintf (file, _("\
  --warn-rwx-segments         Generate a warning if a LOAD segment has RWX permissions\n"));
  fprintf (file, _("\
  --no-warn-rwx-segments      Do not generate a warning if a LOAD segments has RWX permissions (default)\n"));
#endif
  fprintf (file, _("\
  -z unique-symbol            Avoid duplicated local symbol names\n"));
  fprintf (file, _("\
  -z nounique-symbol          Keep duplicated local symbol names (default)\n"));
  fprintf (file, _("\
  -z globalaudit              Mark executable requiring global auditing\n"));
  fprintf (file, _("\
  -z start-stop-gc            Enable garbage collection on __start/__stop\n"));
  fprintf (file, _("\
  -z nostart-stop-gc          Don't garbage collect __start/__stop (default)\n"));
  fprintf (file, _("\
  -z start-stop-visibility=V  Set visibility of built-in __start/__stop symbols\n\
                                to DEFAULT, PROTECTED, HIDDEN or INTERNAL\n"));
  fprintf (file, _("\
  -z sectionheader            Generate section header (default)\n"));
  fprintf (file, _("\
  -z nosectionheader          Do not generate section header\n"));
}

static void
elf_plt_unwind_list_options (FILE *file)
{
  fprintf (file, _("\
  --ld-generated-unwind-info  Generate exception handling info for PLT\n"));
  fprintf (file, _("\
  --no-ld-generated-unwind-info\n\
                              Don't generate exception handling info for PLT\n"));
}

static void
ld_list_options (FILE *file, bool elf, bool shlib, bool plt_unwind)
{
  if (!elf)
    return;
  printf (_("ELF emulations:\n"));
  if (plt_unwind)
    elf_plt_unwind_list_options (file);
  elf_static_list_options (file);
  if (shlib)
    elf_shlib_list_options (file);
}


/* Print help messages for the options.  */

static void
help (void)
{
  unsigned i;
  const char **targets, **pp;
  int len;

  printf (_("Usage: %s [options] file...\n"), program_name);

  printf (_("Options:\n"));
  for (i = 0; i < OPTION_COUNT; i++)
    {
      if (ld_options[i].doc != NULL)
	{
	  bool comma;
	  unsigned j;

	  printf ("  ");

	  comma = false;
	  len = 2;

	  j = i;
	  do
	    {
	      if (ld_options[j].shortopt != '\0'
		  && ld_options[j].control != NO_HELP)
		{
		  printf ("%s-%c", comma ? ", " : "", ld_options[j].shortopt);
		  len += (comma ? 2 : 0) + 2;
		  if (ld_options[j].arg != NULL)
		    {
		      if (ld_options[j].opt.has_arg != optional_argument)
			{
			  printf (" ");
			  ++len;
			}
		      printf ("%s", _(ld_options[j].arg));
		      len += strlen (_(ld_options[j].arg));
		    }
		  comma = true;
		}
	      ++j;
	    }
	  while (j < OPTION_COUNT && ld_options[j].doc == NULL);

	  j = i;
	  do
	    {
	      if (ld_options[j].opt.name != NULL
		  && ld_options[j].control != NO_HELP)
		{
		  int two_dashes =
		    (ld_options[j].control == TWO_DASHES
		     || ld_options[j].control == EXACTLY_TWO_DASHES);

		  printf ("%s-%s%s",
			  comma ? ", " : "",
			  two_dashes ? "-" : "",
			  ld_options[j].opt.name);
		  len += ((comma ? 2 : 0)
			  + 1
			  + (two_dashes ? 1 : 0)
			  + strlen (ld_options[j].opt.name));
		  if (ld_options[j].arg != NULL)
		    {
		      printf (" %s", _(ld_options[j].arg));
		      len += 1 + strlen (_(ld_options[j].arg));
		    }
		  comma = true;
		}
	      ++j;
	    }
	  while (j < OPTION_COUNT && ld_options[j].doc == NULL);

	  if (len >= 30)
	    {
	      printf ("\n");
	      len = 0;
	    }

	  for (; len < 30; len++)
	    putchar (' ');

	  printf ("%s\n", _(ld_options[i].doc));
	}
    }
  printf (_("  @FILE"));
  for (len = strlen ("  @FILE"); len < 30; len++)
    putchar (' ');
  printf (_("Read options from FILE\n"));

  /* Note: Various tools (such as libtool) depend upon the
     format of the listings below - do not change them.  */
  /* xgettext:c-format */
  printf (_("%s: supported targets:"), program_name);
  targets = bfd_target_list ();
  for (pp = targets; *pp != NULL; pp++)
    printf (" %s", *pp);
  free (targets);
  printf ("\n");

  /* xgettext:c-format */
  printf (_("%s: supported emulations: "), program_name);
  ldemul_list_emulations (stdout);
  printf ("\n");

  /* xgettext:c-format */
  printf (_("%s: emulation specific options:\n"), program_name);
  ld_list_options (stdout, ELF_LIST_OPTIONS, ELF_SHLIB_LIST_OPTIONS,
		   ELF_PLT_UNWIND_LIST_OPTIONS);
  ldemul_list_emulation_options (stdout);
  printf ("\n");

  if (REPORT_BUGS_TO[0])
    printf (_("Report bugs to %s\n"), REPORT_BUGS_TO);
}
