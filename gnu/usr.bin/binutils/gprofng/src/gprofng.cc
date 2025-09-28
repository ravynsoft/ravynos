/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "Application.h"
#include "i18n.h"
#include "util.h"

static int verbose = 0;

class Gprofng : Application
{
public:
  Gprofng (int _argc, char *_argv[]);
  ~Gprofng ();
  void start();
  void usage();

private:
  void exec_cmd(char *tool_name, int argc, char **argv);
  int argc;
  char **argv;
};

int
main (int argc, char *argv[])
{
  Gprofng *gprofng = new Gprofng (argc, argv);
  gprofng->start();
  delete gprofng;
  return 0;
}

Gprofng::Gprofng (int _argc, char *_argv[]) : Application(_argc, _argv, NULL)
{
  argc = _argc;
  argv = _argv;
}

Gprofng::~Gprofng () { }

void
Gprofng::usage ()
{
  /*
   * Isolate the first line because it has an argument.
   * Otherwise it would be at the end of this long list.
   */
  printf ( GTXT (
    "Usage: %s [OPTION(S)] COMMAND [KEYWORD] [ARGUMENTS]\n"), whoami);

  printf ( GTXT (
    "\n"
    "This is the driver for the GPROFNG tools suite to gather and analyze performance data.\n"
    "\n"
    "Options:\n"
    "\n"
    " --version           print the version number and exit.\n"
    " --help              print usage information and exit.\n"
    " --check             verify if the hardware and software environment is supported.\n"
    " --verbose {on|off}  enable (on) or disable (off) verbose mode; the default is \"off\".\n"
    "\n"
    "Commands:\n"
    "\n"
    "The driver supports various commands. These are listed below.\n"
    "\n"
    "It is also possible to invoke the lower level commands directly, but since these \n"
    "are subject to change, in particular the options, we recommend to use the driver.\n"
    "\n"
    "The man pages for the commands below can be viewed using the command name with\n"
    "\"gprofng\" replaced by \"gp\" and the spaces replaced by a dash (\"-\"). For\n"
    "example the man page name for \"gprofng collect app\" is \"gp-collect-app\".\n"
    "\n"
    "The following combination of commands and keywords are supported:\n"
    "\n"
    "Collect performance data\n"
    "\n"
    " gprofng collect app     collect application performance data.\n"
    "\n"
    "Display the performance results\n"
    "\n"
    " gprofng display text    display the performance data in ASCII format.\n"
    " gprofng display html    generate an HTML file from one or more experiments.\n"
/*
    " gprofng display gui     invoke the GUI to graphically analyze the results.\n"
*/
    " gprofng display src     display source or disassembly with compiler annotations.\n"
    "\n"
    "Miscellaneous commands\n"
    "\n"
    " gprofng archive         include binaries and source code in an experiment directory.\n"
    "\n"
    "Environment:\n"
    "\n"
    "The following environment variables are supported:\n"
    "\n"
    " GPROFNG_MAX_CALL_STACK_DEPTH  set the depth of the call stack (default is 256).\n"
    "\n"
    " GPROFNG_USE_JAVA_OPTIONS      may be set when profiling a C/C++ application\n"
    "                               that uses dlopen() to execute Java code.\n"
    "\n"
    " GPROFNG_SSH_REMOTE_DISPLAY    use this variable to define the ssh command\n"
    "                               executed by the remote display tool.\n"
    "\n"
    " GPROFNG_SKIP_VALIDATION       set this variable to disable checking hardware,\n"
    "                               system, and Java versions.\n"
    "\n"
    " GPROFNG_ALLOW_CORE_DUMP       set this variable to allow a core file to be\n"
    "                               generated; otherwise an error report is created on /tmp.\n"
    "\n"
    " GPROFNG_ARCHIVE               use this variable to define the settings for automatic\n"
    "                               archiving upon experiment recording completion.\n"
    "\n"
    " GPROFNG_ARCHIVE_COMMON_DIR    set this variable to the location of the common archive.\n"
    "\n"
    " GPROFNG_JAVA_MAX_CALL_STACK_DEPTH  set the depth of the Java call stack; the default\n"
    "                                    is 256; set to 0 to disable capturing of call stacks.\n"
    "\n"
    " GPROFNG_JAVA_NATIVE_MAX_CALL_STACK_DEPTH  set the depth of the Java native call stack;\n"
    "                                           the default is 256; set to 0 to disable capturing\n"
    "                                           of call stacks (JNI and assembly call stacks\n"
    "                                           are not captured).\n"
    "\n"
    "Documentation:\n"
    "\n"
    "A getting started guide for gprofng is maintained as a Texinfo manual. If the info and\n"
    "gprofng programs are properly installed at your site, the command \"info gprofng\"\n"
    "should give you access to this document.\n"
    "\n"
    "See also:\n"
    "\n"
    "gp-archive(1), gp-collect-app(1), gp-display-html(1), gp-display-src(1), gp-display-text(1)\n"));

/*
  printf ( GTXT (
    "Usage: %s [--verbose] [--version] [--help] <tool-name> [<keyword>] <args>\n"
    "\n%s\n"
    "   archive         Archive binaries and sources\n"
    "   collect [app]   Collect performance data\n"
    "   display [text]  Print an ASCII report\n"
    "   display gui     Graphical tool for analyzing an experiment\n"
    "   display html    Generate an HTML file from an experiment\n"
    "   display src     Print source or dissasembly\n"),
    whoami, getenv ("_BUILDING_MANPAGE")
	   ? "*Available subcommands*"
	   : "Available Subcommands");
*/
}

void
Gprofng::exec_cmd (char *tool_name, int argc, char **argv)
{
  static const struct
  {
    const char *tool_name;
    const char *keyword;
    const char *app_name;
  } app_names [] = {
    { "archive", NULL, "gp-archive"},
    { "collect", "app", "gp-collect-app"},
    { "collect", "kernel", "gp-collect-kernel"},
    { "display", "text", "gp-display-text"},
    { "display", "gui", "gp-display-gui"},
    { "display", "html", "gp-display-html"},
    { "display", "src", "gp-display-src"},
    { NULL, NULL}
  };

  const char *keyword = argc > 1 ? argv[1] : "";
  int first = -1;
  int find_tool_name = -1;
  for (int i = 0; app_names[i].tool_name; i++)
    if (!strcmp (tool_name, app_names[i].tool_name))
      {
	if (app_names[i].keyword == NULL)
	  {
	    first = i;
	    break;
	  }
	if (!strcmp (keyword, app_names[i].keyword))
	  {
	    first = i;
	    argc--;
	    argv++;
	    break;
	  }
	if (find_tool_name == -1)
	  find_tool_name = i;
      }

  if (first == -1)
    {
      if (find_tool_name == -1)
	fprintf (stderr, GTXT ("%s: error: keyword '%s' is not supported\n"),
		 get_basename (get_name ()), tool_name);
      else if (*keyword == 0)
	fprintf (stderr, GTXT ("%s %s: error: no qualifier\n"),
		 get_basename (get_name ()), tool_name);
      else
	fprintf (stderr, GTXT ("%s %s: error: qualifier '%s' is not supported\n"),
		 get_basename (get_name ()), tool_name, keyword);
      exit (1);
    }

  const char *aname = app_names[first].app_name;;

  char **arr = (char **) malloc ((argc + 3) * sizeof (char *));
  int n = 0;
  char *pname = get_name ();
  arr[n++] = dbe_sprintf ("%.*s%s", (int) (get_basename (pname) - pname),
			    pname, aname);
  if (app_names[first].keyword)
    arr[n++] = dbe_sprintf ("--whoami=%s %s %s", whoami, tool_name,
			    app_names[first].keyword);
  else
    arr[n++] = dbe_sprintf ("--whoami=%s %s", whoami, tool_name);
  for (int i = 1; i < argc; i++)
    arr[n++] = argv[i];
  arr[n] = NULL;
  if (verbose)
    {
      printf ("gprofng::exec\n");
      for (int i = 0; arr[i]; i++)
	printf ("%5d: %s\n", i, arr[i]);
      printf("\n");
    }
  execv (arr[0], arr);

  // If execv returns, it must have failed.
  fprintf (stderr, GTXT ("%s failed: %s\n"), arr[0], STR (strerror (errno)));
  exit(1);
}

void
Gprofng::start ()
{
  if (argc == 1)
    {
      usage ();
      exit (0);
    }
  for (int i = 1; i < argc; i++)
    {
      char *s = argv[i];
      if (*s != '-')
	{
	  exec_cmd(s, argc - i, argv + i);
	  return;
	}
      else if (!strcmp (s, "--help"))
	{
	  usage ();
	  exit (0);
	}
      else if (!strcmp (s, "--version") || !strcmp (s, "-v"))
	{
	   Application::print_version_info ();
	   exit (0);
	}
      else if (!strcmp (s, "--verbose"))
	verbose = 1;
      else if (!strcmp (s, "--check"))
	{
	  fprintf (stderr, GTXT ("%s: error: --check is not implemented yet\n"),
		   get_basename (get_name ()));
	  exit (1);
	}
      else
	{
	  fprintf (stderr, GTXT ("%s: error: unknown option %s\n"),
		   get_basename (get_name ()), s);
	  exit(1);
	}
    }
  fprintf (stderr, GTXT ("%s: error: expected argument after options\n"),
	   get_basename (get_name ()));
}
