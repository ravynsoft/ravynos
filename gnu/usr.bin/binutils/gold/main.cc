// main.cc -- gold main function.

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

#include <cstdio>
#include <cstring>

#if defined(HAVE_MALLINFO) || defined(HAVE_MALLINFO2)
#include <malloc.h>
#endif

#include "libiberty.h"

#include "script.h"
#include "options.h"
#include "target-select.h"
#include "parameters.h"
#include "errors.h"
#include "mapfile.h"
#include "dirsearch.h"
#include "workqueue.h"
#include "object.h"
#include "archive.h"
#include "symtab.h"
#include "layout.h"
#include "plugin.h"
#include "gc.h"
#include "icf.h"
#include "incremental.h"
#include "gdb-index.h"
#include "timer.h"

using namespace gold;

// This function emits the commandline to a hard-coded file in temp.
// This is useful for debugging since ld is typically invoked by gcc,
// so its commandline is not always easy to extract.  You should be
// able to run 'gcc -B... foo.o -o foo' to invoke this linker the
// first time, and then /tmp/ld-run-foo.sh to invoke it on subsequent
// runes.  "/tmp/ld-run-foo.sh debug" will run the linker inside gdb
// (or whatever value the environment variable GDB is set to), for
// even easier debugging.  Since this is a debugging-only tool, and
// creates files, it is only turned on when the user explicitly asks
// for it, by compiling with -DDEBUG.  Do not do this for release
// versions of the linker!

#ifdef DEBUG
#include <stdio.h>
#include <sys/stat.h>    // for chmod()

static std::string
collect_argv(int argc, char** argv)
{
  // This is used by write_debug_script(), which wants the unedited argv.
  std::string args;
  for (int i = 0; i < argc; ++i)
    {
      args.append(" '");
      // Now append argv[i], but with all single-quotes escaped
      const char* argpos = argv[i];
      while (1)
        {
          const int len = strcspn(argpos, "'");
          args.append(argpos, len);
          if (argpos[len] == '\0')
            break;
          args.append("'\"'\"'");
          argpos += len + 1;
        }
      args.append("'");
    }
  return args;
}

static void
write_debug_script(std::string filename_str,
		   const char* argv_0, const char* args)
{
  size_t slash = filename_str.rfind('/');
  if (slash != std::string::npos)
    filename_str = filename_str.c_str() + slash + 1;
  filename_str = std::string("/tmp/ld-run-") + filename_str + ".sh";
  const char* filename = filename_str.c_str();
  FILE* fp = fopen(filename, "w");
  if (fp)
    {
      fprintf(fp, "[ \"$1\" = debug ]"
              " && PREFIX=\"${GDB-gdb} --annotate=3 --fullname %s --args\""
              " && shift\n",
              argv_0);
      fprintf(fp, "$PREFIX%s $*\n", args);
      fclose(fp);
      chmod(filename, 0755);
    }
  else
    filename = "[none]";
  fprintf(stderr, "Welcome to gold!  Commandline written to %s.\n", filename);
  fflush(stderr);
}

#else // !defined(DEBUG)

static inline std::string
collect_argv(int, char**)
{
  return "";
}

static inline void
write_debug_script(std::string, const char*, const char*)
{
}

#endif // !defined(DEBUG)


int
main(int argc, char** argv)
{
#if defined (HAVE_SETLOCALE) && defined (HAVE_LC_MESSAGES)
  setlocale(LC_MESSAGES, "");
#endif
#if defined (HAVE_SETLOCALE)
  setlocale(LC_CTYPE, "");
#endif
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);

  program_name = argv[0];

  // In libiberty; expands @filename to the args in "filename".
  expandargv(&argc, &argv);

  // This is used by write_debug_script(), which wants the unedited argv.
  std::string args = collect_argv(argc, argv);

  Errors errors(program_name);

  // Initialize the global parameters, to let random code get to the
  // errors object.
  set_parameters_errors(&errors);

  // Handle the command line options.
  Command_line command_line;
  command_line.process(argc - 1, const_cast<const char**>(argv + 1));

  Timer timer;
  if (command_line.options().stats())
    {
      timer.start();
      set_parameters_timer(&timer);
    }

  // Store some options in the globally accessible parameters.
  set_parameters_options(&command_line.options());

  // Do this as early as possible (since it prints a welcome message).
  write_debug_script(command_line.options().output_file_name(),
                     program_name, args.c_str());

  // If the user asked for a map file, open it.
  Mapfile* mapfile = NULL;
  if (command_line.options().user_set_Map())
    {
      mapfile = new Mapfile();
      if (!mapfile->open(command_line.options().Map()))
	{
	  delete mapfile;
	  mapfile = NULL;
	}
    }

  // The GNU linker ignores version scripts when generating
  // relocatable output.  If we are not compatible, then we break the
  // Linux kernel build, which uses a linker script with -r which must
  // not force symbols to be local.  It would actually be useful to
  // permit symbols to be forced local with -r, though, as it would
  // permit some linker optimizations.  Perhaps we need yet another
  // option to control this.  FIXME.
  if (parameters->options().relocatable())
    command_line.script_options().version_script_info()->clear();

  // The work queue.
  Workqueue workqueue(command_line.options());

  // The list of input objects.
  Input_objects input_objects;

  // The Garbage Collection (GC, --gc-sections) Object.
  Garbage_collection gc;

  // The Identical Code Folding (ICF, --icf) Object.
  Icf icf;

  // The symbol table.  We're going to guess here how many symbols
  // we're going to see based on the number of input files.  Even when
  // this is off, it means at worst we don't quite optimize hashtable
  // resizing as well as we could have (perhaps using more memory).
  Symbol_table symtab(command_line.number_of_input_files() * 1024,
                      command_line.version_script());

  if (parameters->options().gc_sections())
    symtab.set_gc(&gc);

  if (parameters->options().icf_enabled())
    symtab.set_icf(&icf);

  // The layout object.
  Layout layout(command_line.number_of_input_files(),
		&command_line.script_options());

  if (layout.incremental_inputs() != NULL)
    layout.incremental_inputs()->report_command_line(argc, argv);

  if (parameters->options().section_ordering_file())
    layout.read_layout_from_file();

  // Load plugin libraries.
  if (command_line.options().has_plugins())
    command_line.options().plugins()->load_plugins(&layout);

  // Get the search path from the -L options.
  Dirsearch search_path;
  search_path.initialize(&workqueue, &command_line.options().library_path());

  // Queue up the first set of tasks.
  queue_initial_tasks(command_line.options(), search_path,
		      command_line, &workqueue, &input_objects,
		      &symtab, &layout, mapfile);

  // Run the main task processing loop.
  workqueue.process(0);

  if (command_line.options().print_output_format())
    print_output_format();

  if (command_line.options().stats())
    {
      timer.stamp(2);
      Timer::TimeStats elapsed = timer.get_pass_time(0);
      fprintf(stderr,
             _("%s: initial tasks run time: " \
               "(user: %ld.%06ld sys: %ld.%06ld wall: %ld.%06ld)\n"),
              program_name,
              elapsed.user / 1000, (elapsed.user % 1000) * 1000,
              elapsed.sys / 1000, (elapsed.sys % 1000) * 1000,
              elapsed.wall / 1000, (elapsed.wall % 1000) * 1000);
      elapsed = timer.get_pass_time(1);
      fprintf(stderr,
             _("%s: middle tasks run time: " \
               "(user: %ld.%06ld sys: %ld.%06ld wall: %ld.%06ld)\n"),
              program_name,
              elapsed.user / 1000, (elapsed.user % 1000) * 1000,
              elapsed.sys / 1000, (elapsed.sys % 1000) * 1000,
              elapsed.wall / 1000, (elapsed.wall % 1000) * 1000);
      elapsed = timer.get_pass_time(2);
      fprintf(stderr,
             _("%s: final tasks run time: " \
               "(user: %ld.%06ld sys: %ld.%06ld wall: %ld.%06ld)\n"),
              program_name,
              elapsed.user / 1000, (elapsed.user % 1000) * 1000,
              elapsed.sys / 1000, (elapsed.sys % 1000) * 1000,
              elapsed.wall / 1000, (elapsed.wall % 1000) * 1000);
      elapsed = timer.get_elapsed_time();
      fprintf(stderr,
             _("%s: total run time: " \
               "(user: %ld.%06ld sys: %ld.%06ld wall: %ld.%06ld)\n"),
              program_name,
              elapsed.user / 1000, (elapsed.user % 1000) * 1000,
              elapsed.sys / 1000, (elapsed.sys % 1000) * 1000,
              elapsed.wall / 1000, (elapsed.wall % 1000) * 1000);

#if defined(HAVE_MALLINFO2)
      struct mallinfo2 m = mallinfo2();
      fprintf(stderr, _("%s: total space allocated by malloc: %lld bytes\n"),
	      program_name, static_cast<long long>(m.arena));
#elif defined(HAVE_MALLINFO)
      struct mallinfo m = mallinfo();
      fprintf(stderr, _("%s: total space allocated by malloc: %lld bytes\n"),
	      program_name, static_cast<long long>(m.arena));
#endif

      File_read::print_stats();
      Archive::print_stats();
      Lib_group::print_stats();
      fprintf(stderr, _("%s: output file size: %lld bytes\n"),
	      program_name, static_cast<long long>(layout.output_file_size()));
      symtab.print_stats();
      layout.print_stats();
      Gdb_index::print_stats();
      Free_list::print_stats();
    }

  // Issue defined symbol report.
  if (command_line.options().user_set_print_symbol_counts())
    input_objects.print_symbol_counts(&symtab);

  // Output cross reference table.
  if (command_line.options().cref())
    input_objects.print_cref(&symtab,
			     mapfile == NULL ? stdout : mapfile->file());

  if (mapfile != NULL)
    mapfile->close();

  if (parameters->options().fatal_warnings()
      && errors.warning_count() > 0
      && errors.error_count() == 0)
    gold_error("treating warnings as errors");

  // If the user used --noinhibit-exec, we force the exit status to be
  // successful.  This is compatible with GNU ld.
  gold_exit((errors.error_count() == 0
	     || parameters->options().noinhibit_exec())
	    ? GOLD_OK
	    : GOLD_ERR);
}
