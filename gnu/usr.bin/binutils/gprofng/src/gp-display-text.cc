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
#include <unistd.h>     // isatty

#include "gp-print.h"
#include "ipcio.h"
#include "Command.h"
#include "Dbe.h"
#include "DbeApplication.h"
#include "DbeSession.h"
#include "Experiment.h"
#include "Emsg.h"
#include "DbeView.h"
#include "DataObject.h"
#include "Function.h"
#include "Hist_data.h"
#include "PathTree.h"
#include "LoadObject.h"
#include "Function.h"
#include "FilterSet.h"
#include "Filter.h"
#include "MetricList.h"
#include "MemorySpace.h"
#include "Module.h"
#include "util.h"
#include "i18n.h"
#include "StringBuilder.h"
#include "debug.h"
#include "UserLabel.h"

static char *exe_name;
static char **new_argv;

void
reexec ()
{
  if (dbeSession != NULL)
    dbeSession->unlink_tmp_files ();
  execv (exe_name, new_argv);
}

/**
 * Run application under enhance if the following requirements are satisfied:
 * 1. Environment variable GPROFNG_ENHANCE is not set to "no"
 * 2. Standard input is terminal
 * 3. Standard output is terminal
 * 4. /bin/enhance exists and can work on this system
 */
static void
reexec_enhance (int argc, char *argv[])
{
  char *gp_enhance = getenv ("GPROFNG_ENHANCE");
  if (NULL != gp_enhance && 0 == strcasecmp (gp_enhance, "no"))
    return; // Do not enhance
  // Verify that input and output are tty
  if (!isatty (fileno (stdin)))     // stdin is not a terminal
    return; // Do not enhance
  if (!isatty (fileno (stdout)))    // stdout is not a terminal
    return; // Do not enhance
  char *enhance_name = NTXT ("/bin/enhance");
  struct stat sbuf;
  int res = stat (enhance_name, &sbuf); // Check if enhance exists
  if (res == 0)
    res = system (NTXT ("/bin/enhance /bin/true")); // Check if enhance can work
  if (res != 0)
    {
      fflush (stdout);
      printf (GTXT ("Warning: History and command editing is not supported on this system.\n"));
      fflush (stdout);
      return;
    }
  else
    {
      printf (GTXT ("Note: History and command editing is supported on this system.\n"));
      fflush (stdout);
    }
  char **nargv = new char*[argc + 2];
  for (int i = 0; i < argc; i++)
    nargv[i + 1] = argv[i];
  nargv[0] = enhance_name;
  nargv[argc + 1] = NULL;
  putenv (NTXT ("GPROFNG_ENHANCE=no")); // prevent recursion
  execv (enhance_name, nargv);
  // execv failed. Continue to run the program
  delete[] nargv;
}

int
main (int argc, char *argv[])
{
  er_print *erprint;
  int ind = 1;
  if (argc > ind && *argv[ind] == '-')
    {
      int arg_count, cparam;
      if (Command::get_command (argv[ind] + 1, arg_count, cparam) == WHOAMI)
	ind = ind + 1 + arg_count;
    }
  if (argc > ind && argv[ind] != NULL && *argv[ind] != '-')
    reexec_enhance (argc, argv);

  // Save argv for reexec())
  exe_name = argv[0];
  new_argv = argv;

  if (argc > ind && argv[ind] != NULL && strcmp (argv[ind], "-IPC") == 0)
    {
      putenv (NTXT ("LC_NUMERIC=C")); // Use non-localized numeric data in IPC packets
      erprint = new er_print (argc, argv);
      theDbeApplication->rdtMode = false;
      ipc_mainLoop (argc, argv);
    }
  else
    {
      erprint = new er_print (argc, argv);
      erprint->start (argc, argv);
    }

  dbeSession->unlink_tmp_files ();
  if (DUMP_CALL_STACK)
    {
      extern long total_calls_add_stack, total_stacks, total_nodes, call_stack_size[201];
      fprintf (stderr, NTXT ("total_calls_add_stack=%lld\ntotal_stacks=%lld\ntotal_nodes=%lld\n"),
	       (long long) total_calls_add_stack, (long long) total_stacks, (long long) total_nodes);
      for (int i = 0; i < 201; i++)
	if (call_stack_size[i] != 0)
	    fprintf (stderr, NTXT ("   call_stack_size[%d] = %6lld\n"), i,
		     (long long) call_stack_size[i]);
    }
#if defined(DEBUG)
  delete erprint;
#endif
  return 0;
}

er_print::er_print (int argc, char *argv[])
: DbeApplication (argc, argv)
{
  out_fname = GTXT ("<stdout>");
  inp_file = stdin;
  out_file = stdout;
  dis_file = stdout;
  cov_string = NULL;
  limit = 0;
  cstack = new Vector<Histable*>();
  was_QQUIT = false;
}

er_print::~er_print ()
{
  free (cov_string);
  delete cstack;
  if (inp_file != stdin)
    fclose (inp_file);
}

void
er_print::start (int argc, char *argv[])
{
  Vector<String> *res = theDbeApplication->initApplication (NULL, NULL, NULL);
  res->destroy ();
  delete res;

  // Create a view on the session
  dbevindex = dbeSession->createView (0, -1);
  dbev = dbeSession->getView (dbevindex);
  limit = dbev->get_limit ();
  (void) check_args (argc, argv);
  int ngood = dbeSession->ngoodexps ();
  if (ngood == 0)
    {
      fprintf (stderr, GTXT ("No valid experiments loaded; exiting\n"));
      return;
    }
  dbeDetectLoadMachineModel (dbevindex);
  run (argc, argv);
}

bool
er_print::free_memory_before_exit ()
{
  return was_QQUIT;
}

void
er_print::usage ()
{

/*
  Ruud - Isolate this line because it has an argument.  Otherwise it would be at the
  end of the long option list.
*/
  printf ( GTXT (
    "Usage: gprofng display text [OPTION(S)] [COMMAND(S)] [-script <script_file>] EXPERIMENT(S)\n"));

  printf ( GTXT (
    "\n"
    "Print a plain text version of the various displays supported by gprofng.\n"
    "\n"
    "Options:\n"
    "\n"
    " --version           print the version number and exit.\n"
    " --help              print usage information and exit.\n"
    " --verbose {on|off}  enable (on) or disable (off) verbose mode; the default is \"off\".\n"
    "\n"
    " -script <script-file>  execute the commands stored in the script file;\n"
    "                        this feature may be combined with commands specified\n"
    "                        at the command line.\n"
    "\n"
    "Commands:\n"
    "\n"
    "This tool supports a rich set of commands to control the display of the\n"
    "data; instead of, or in addition to, including these commands in a script\n"
    "file, it is also allowed to include such commands at the command line;\n"
    "in this case, the commands need to be prepended with the \"-\" symbol; the\n"
    "commands are processed and interpreted left from right, so the order matters;\n"
    "The gprofng manual documents the commands that are supported.\n"
    "\n"
    "If this tool is invoked without options, commands, or a script file, it starts\n"
    "in interpreter mode. The user can then issue the commands interactively; the\n"
    "session is terminated with the \"exit\" command in the interpreter.\n"
    "\n"
    "Documentation:\n"
    "\n"
    "A getting started guide for gprofng is maintained as a Texinfo manual. If the info and\n"
    "gprofng programs are properly installed at your site, the command \"info gprofng\"\n"
    "should give you access to this document.\n"
    "\n"
    "See also:\n"
    "\n"
    "gprofng(1), gp-archive(1), gp-collect-app(1), gp-display-html(1), gp-display-src(1)\n"));
}

int // returns count of experiments read
er_print::check_args (int argc, char *argv[])
{
  CmdType cmd_type;
  int arg_count;
  int cparam;
  int exp_no;
  error_msg = NULL;

  Emsg *rcmsg = fetch_comments ();
  while (rcmsg != NULL)
    {
      fprintf (stderr, NTXT ("%s: %s\n"), prog_name, rcmsg->get_msg ());
      rcmsg = rcmsg->next;
    }
  delete_comments ();

  // Set up the list of experiments to add after checking the args
  Vector<Vector<char*>*> *exp_list = new Vector<Vector<char*>*>();

  // Prescan the command line arguments, processing only a few
  for (int i = 1; i < argc; i++)
    {
      if (*argv[i] != '-')
	{
	  // we're at the end -- get the list of experiments
	  //  Build the list of experiments, and set the searchpath
	  Vector<char*> *list = dbeSession->get_group_or_expt (argv[i]);
	  if (list->size () > 0)
	    {
	      for (int j = 0, list_sz = list->size (); j < list_sz; j++)
		{
		  char *path = list->fetch (j);
		  if (strlen (path) == 0 || strcmp (path, NTXT ("\\")) == 0)
		    continue;
		  char *p = strrchr (path, '/');
		  if (p)
		    {
		      // there's a directory in front of the name; add it to search path
		      *p = '\0';
		      dbeSession->set_search_path (path, false);
		    }
		}
	      list->destroy ();
	      list->append (dbe_strdup (argv[i]));
	      exp_list->append (list);
	    }
	  else
	    delete list;
	  continue;
	}

      // Not at the end yet, treat the next argument as a command
      switch (cmd_type = Command::get_command (argv[i] + 1, arg_count, cparam))
	{
	case WHOAMI:
	  whoami = argv[i] + 1 + cparam;
	  break;
	case HELP:
	  if (i + 1 + arg_count == argc)
	    {
	      usage();
	      exit (0);
	    }
	  break;
	case HHELP:
	  Command::print_help (whoami, true, false, stdout);
	  fprintf (stdout, "\n");
	  indxo_list (false, stdout);
	  fprintf (stdout, "\n");
	  mo_list (false, stdout);
	  if (!getenv ("_BUILDING_MANPAGE"))
	    fprintf (stdout, GTXT ("\nSee gprofng(1) for more details\n"));
	  exit (0);
	case ADD_EXP:
	case DROP_EXP:
	case OPEN_EXP:
	  printf (GTXT ("Error: command %s can not appear on the command line\n"), argv[i]);
	  exit (2);
	case VERSION_cmd:
	  Application::print_version_info ();
	  exit (0);
	case AMBIGUOUS_CMD:
	  fprintf (stderr, GTXT ("Error: Ambiguous command: %s\n"), argv[i]);
	  exit (2);
	case UNKNOWN_CMD:
	  fprintf (stderr, GTXT ("Error: Invalid command: %s\n"), argv[i]);
	  exit (2);
	  // it's a plausible argument; see if we process now or later
	case SOURCE:
	case DISASM:
	case CSINGLE:
	case CPREPEND:
	case CAPPEND:
	case FSINGLE:
	case SAMPLE_DETAIL:
	case STATISTICS:
	case HEADER:
	  //skip the arguments to that command
	  i += arg_count;
	  if (i >= argc || end_command (argv[i]))
	    i--;
	  break;
	case PRINTMODE:
	case INDXOBJDEF:
	case ADDPATH:
	case SETPATH:
	case PATHMAP:
	case OBJECT_SHOW:
	case OBJECT_HIDE:
	case OBJECT_API:
	case OBJECTS_DEFAULT:
	case EN_DESC:
	  // these are processed in the initial pass over the arguments
	  proc_cmd (cmd_type, cparam, (arg_count > 0) ? argv[i + 1] : NULL,
		    (arg_count > 1) ? argv[i + 2] : NULL,
		    (arg_count > 2) ? argv[i + 3] : NULL,
		    (arg_count > 3) ? argv[i + 4] : NULL);
	  i += arg_count;
	  break;
	default:
	  // any others, we skip for now
	  i += arg_count;
	  break;
	}
    }

  // Make sure some experiments were specified
  exp_no = exp_list->size ();
  if (exp_no == 0)
    { // no experiment name
      fprintf (stderr, GTXT ("%s: Missing experiment directory (use the --help option to get a usage overview)\n"), whoami);
      exit (1);
    }

  // add the experiments to the session
  char *errstr = dbeOpenExperimentList (0, exp_list, false);
  for (long i = 0; i < exp_list->size (); i++)
    {
      Vector<char*>* p = exp_list->get (i);
      Destroy (p);
    }
  delete exp_list;
  if (errstr != NULL)
    {
      fprintf (stderr, NTXT ("%s"), errstr);
      free (errstr);
    }

  return exp_no;
}

int
er_print::is_valid_seg_name (char *lo_name, int prev)
{
  // prev is the loadobject segment index that was last returned
  // search starts following that loadobject
  int index;
  LoadObject *lo;
  char *p_lo_name = lo_name;
  char *name = NULL;

  // strip angle brackets from all but <Unknown> and <Total>
  if (strcmp (lo_name, "<Unknown>") && strcmp (lo_name, "<Total>"))
    {
      if (*lo_name == '<')
	{
	  name = dbe_strdup (lo_name + 1);
	  p_lo_name = name;
	  char *p = strchr (name, '>');
	  if (p)
	    *p = '\0';
	}
    }

  // get the load object list from the session
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  Vec_loop (LoadObject*, lobjs, index, lo)
  {
    if (prev > 0)
      {
	if (lo->seg_idx == prev)    // this is where we left off
	  prev = -1;
	continue;
      }

    // does this one match?
    if (cmp_seg_name (lo->get_pathname (), p_lo_name))
      {
	delete lobjs;
	free (name);
	size_t len = strlen (lo_name);
	if ((len > 7 && streq (lo_name + len - 7, NTXT (".class>"))) ||
	    (len > 6 && streq (lo_name + len - 6, NTXT (".class"))))
	  {
	    fprintf (stderr, GTXT ("Error: Java class `%s' is not selectable\n"), lo_name);
	    return -1;
	  }
	return lo->seg_idx;
      }
  }
  delete lobjs;
  free (name);
  return -1;
}

int
er_print::cmp_seg_name (char *full_name, char *lo_name)
{
  char *cmp_name;
  if (!strchr (lo_name, '/') && (cmp_name = strrchr (full_name, '/')))
    cmp_name++; // basename
  else
    cmp_name = full_name; // full path name
  return !strcmp (lo_name, cmp_name);
}

// processing object_select
//	Note that this does not affect the strings in Settings,
//	unlike object_show, object_hide, and object_api
int
er_print::process_object_select (char *names)
{
  int index;
  LoadObject *lo;
  int no_lobj = 0;
  bool got_err = false;
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  if ((names == NULL) || !strcasecmp (names, Command::ALL_CMD))
    { // full coverage
      Vec_loop (LoadObject*, lobjs, index, lo)
      {
	dbev->set_lo_expand (lo->seg_idx, LIBEX_SHOW);
      }
    }
  else
    { // parsing coverage
      // first, hide functions from all loadobjects
      // except the java ones
      Vec_loop (LoadObject*, lobjs, index, lo)
      {
	char *lo_name = lo->get_name ();
	if (lo_name != NULL)
	  {
	    size_t len = strlen (lo_name);
	    if ((len > 7 && streq (lo_name + len - 7, NTXT (".class>"))) ||
		(len > 6 && streq (lo_name + len - 6, NTXT (".class"))))
	      continue;
	  }
	dbev->set_lo_expand (lo->seg_idx, LIBEX_HIDE);
      }

      Vector <char *> *tokens = split_str (names, ',');
      for (long j = 0, sz = VecSize (tokens); j < sz; j++)
	{
	  // loop over the provided names
	  char *lo_name = tokens->get (j);
	  int seg_idx = -1;
	  seg_idx = is_valid_seg_name (lo_name, seg_idx);
	  while (seg_idx != -1)
	    {
	      dbev->set_lo_expand (seg_idx, LIBEX_SHOW);
	      no_lobj++;
	      seg_idx = is_valid_seg_name (lo_name, seg_idx);
	    }
	  if (no_lobj == 0)
	    {
	      got_err = true;
	      fprintf (stderr, GTXT ("Error: Unknown load object: `%s'\n"), lo_name);
	    }
	  free (lo_name);
	}
      delete tokens;
    }

  if (!got_err)
    { // good coverage string
      free (cov_string);
      cov_string = strdup (names);
    }
  else
    { // bad, restore original coverage
      no_lobj = -1;
      process_object_select (cov_string);
    }
  delete lobjs;
  return no_lobj;
}

int
er_print::set_libexpand (char *cov, enum LibExpand expand)
{
  bool changed = dbev->set_libexpand (cov, expand);
  if (changed == true)
    dbev->update_lo_expands ();
  return 0;
}

int
er_print::set_libdefaults ()
{
  dbev->set_libdefaults ();
  return 0;
}

bool
er_print::end_command (char *cmd)
{
  if (cmd == NULL || *cmd == '-')
    return true;
  size_t len = strlen (cmd);
  if (cmd[len - 1] == '/')
    len--;
  if ((len > 3 && !strncmp (&cmd[len - 3], NTXT (".er"), 3)) ||
      (len > 4 && !strncmp (&cmd[len - 4], NTXT (".erg"), 4)))
    return true;
  return false;
}

// Now actually start processing the arguments
void
er_print::run (int argc, char *argv[])
{
  CmdType cmd_type;
  int arg_count, cparam, i;
  bool got = false;
  char *arg1, *arg2;
  for (i = 1; i < argc; i++)
    {
      if (*argv[i] != '-') // open experiment pointer files
	continue;
      switch (cmd_type = Command::get_command (argv[i] + 1, arg_count, cparam))
	{
	case WHOAMI:
	  whoami = argv[i] + 1 + cparam;
	  break;
	case SCRIPT:
	  got = true;
	  inp_file = fopen (argv[++i], "r");
	  if (inp_file == NULL)
	    {
	      fprintf (stderr, GTXT ("Error: Script file cannot be opened: %s\n"), argv[i]);
	      exit (3);
	    }
	  proc_script ();
	  break;
	case STDIN:
	  got = true;
	  inp_file = stdin;
	  proc_script ();
	  break;
	case SOURCE: // with option arg_count == 2
	case DISASM:
	  got = true;
	  i += arg_count;
	  if ((i >= argc) || end_command (argv[i]))
	    {
	      i--;
	      arg1 = argv[i];
	      arg2 = NTXT ("");
	    }
	  else
	    {
	      arg1 = argv[i - 1];
	      arg2 = argv[i];
	    }
	  proc_cmd (cmd_type, cparam, arg1, arg2, NULL, NULL, true);
	  break;
	case CSINGLE:
	case CPREPEND:
	case CAPPEND:
	case FSINGLE:
	  got = true;
	  i += arg_count;
	  if ((i >= argc) || end_command (argv[i]))
	    {
	      i--;
	      proc_cmd (cmd_type, cparam, argv[i], NTXT ("1"));
	    }
	  else
	    proc_cmd (cmd_type, cparam, argv[i - 1], argv[i]);
	  break;
	case SAMPLE_DETAIL: // with option arg_count == 1
	case STATISTICS:
	case HEADER:
	  got = true;
	  // now fall through to process the command
	case COMPARE:
	  got = true;
	  i += arg_count;
	  if ((i >= argc) || end_command (argv[i]))
	    {
	      i--;
	      proc_cmd (cmd_type, cparam, NULL, NULL);
	    }
	  else
	    proc_cmd (cmd_type, cparam, argv[i], NULL);
	  break;
	case PRINTMODE:
	case INDXOBJDEF:
	case ADDPATH:
	case SETPATH:
	case PATHMAP:
	case OBJECT_SHOW:
	case OBJECT_HIDE:
	case OBJECT_API:
	case OBJECTS_DEFAULT:
	case EN_DESC:
	  got = true;
	  // these have been processed already
	  i += arg_count;
	  break;
	case LIMIT:
	  got = true;
	  proc_cmd (cmd_type, cparam, (arg_count > 0) ? argv[i + 1] : NULL,
		    (arg_count > 1) ? argv[i + 2] : NULL);
	  i += arg_count;
	  break;
	default:
	  got = true;
	  proc_cmd (cmd_type, cparam, (arg_count > 0) ? argv[i + 1] : NULL,
		    (arg_count > 1) ? argv[i + 2] : NULL);
	  i += arg_count;
	  break;
	}
    }
  if (!got) // no command has been specified
    proc_script ();
}

#define MAXARGS 20

void
er_print::proc_script ()
{
  CmdType cmd_type;
  int arg_count, cparam;
  char *cmd, *end_cmd;
  char *script = NULL;
  char *arglist[MAXARGS];
  char *line = NULL;
  int lineno = 0;
  while (!feof (inp_file))
    {
      if (inp_file == stdin)
	printf (NTXT ("(%s) "), get_basename (prog_name));
      free (script);
      script = read_line (inp_file);
      if (script == NULL)
	continue;
      free (line);
      line = dbe_strdup (script);
      lineno++;
      for (int i = 0; i < MAXARGS; i++)
	arglist[i] = NULL;

      // ensure it's terminated by a \n, and remove that character
      strtok (script, NTXT ("\n"));

      // extract the command
      cmd = strtok (script, NTXT (" \t"));
      if (cmd == NULL)
	continue;
      if (*cmd == '#')
	{
	  fprintf (stderr, NTXT ("%s"), line);
	  continue;
	}
      if (*cmd == '\n')
	continue;

      char *remainder = strtok (NULL, NTXT ("\n"));
      // now extract the arguments
      int nargs = 0;
      for (;;)
	{
	  end_cmd = NULL;
	  if (nargs >= MAXARGS)
	    fprintf (stderr, GTXT ("Warning: more than %d arguments to %s command, line %d\n"),
		     MAXARGS, cmd, lineno);
	  char *nextarg = strtok (remainder, NTXT ("\n"));
	  if ((nextarg == NULL) || (*nextarg == '#'))
	    // either the end of the line, or a comment indicator
	    break;
	  if (nargs >= MAXARGS)
	    {
	      parse_qstring (nextarg, &end_cmd);
	      nargs++;
	    }
	  else
	    arglist[nargs++] = parse_qstring (nextarg, &end_cmd);
	  remainder = end_cmd;
	  if (remainder == NULL)
	    break;
	  // skip any blanks or tabs to get to next argument
	  while (*remainder == ' ' || *remainder == '\t')
	    remainder++;
	}

      cmd_type = Command::get_command (cmd, arg_count, cparam);

      // check for extra arguments
      if (cmd_type != UNKNOWN_CMD && cmd_type != INDXOBJDEF && nargs > arg_count)
	fprintf (stderr, GTXT ("Warning: extra arguments to %s command, line %d\n"),
		 cmd, lineno);
      switch (cmd_type)
	{
	case SOURCE:
	case DISASM:
	  // ignore any third parameter
	  // if there was, we have written a warning
	  proc_cmd (cmd_type, cparam, arglist[0], arglist[1], NULL, NULL,
		    (inp_file != stdin));
	  break;
	case QUIT:
	  free (script);
	  free (line);
	  exit (0);
	case QQUIT:
	  was_QQUIT = true;
	  free (script);
	  free (line);
	  return;
	case STDIN:
	  break;
	case COMMENT:
	  fprintf (dis_file, NTXT ("%s"), line);
	  break;
	case AMBIGUOUS_CMD:
	  fprintf (stderr, GTXT ("Error: Ambiguous command: %s\n"), cmd);
	  break;
	case UNKNOWN_CMD:
	  if (*cmd != '\n')
	    fprintf (stderr, GTXT ("Error: Invalid command: %s\n"), cmd);
	  break;
	default:
	  proc_cmd (cmd_type, cparam, arglist[0], arglist[1]);
	  break;
	}
    }
  // free up the input line
  free (script);
  free (line);
}

void
er_print::proc_cmd (CmdType cmd_type, int cparam,
		    char *arg1, char *arg2, char *arg3, char *arg4, bool xdefault)
{
  er_print_common_display *cd;
  FILE *ck_file, *save_file;
  char *name;
  int bgn_index, end_index, index;
  Cmd_status status;
  char *scratch, *scratch1;
  switch (cmd_type)
    {
    case FUNCS:
      print_func (Histable::FUNCTION, MODE_LIST,
		  dbev->get_metric_list (MET_NORMAL), dbev->get_metric_list (MET_NORMAL));
      break;
    case FDETAIL:
      print_func (Histable::FUNCTION, MODE_DETAIL,
		  dbev->get_metric_list (MET_NORMAL), dbev->get_metric_ref (MET_NORMAL));
      break;
    case FSINGLE:
      print_func (Histable::FUNCTION, MODE_DETAIL,
		  dbev->get_metric_list (MET_NORMAL), dbev->get_metric_ref (MET_NORMAL),
		  arg1, arg2);
      break;
    case HOTPCS:
      print_func (Histable::INSTR, MODE_LIST,
		  dbev->get_metric_list (MET_NORMAL), dbev->get_metric_list (MET_NORMAL));
      break;
    case PDETAIL:
      print_func (Histable::INSTR, MODE_DETAIL,
		  dbev->get_metric_list (MET_NORMAL), dbev->get_metric_ref (MET_NORMAL));
      break;
    case HOTLINES:
      print_func (Histable::LINE, MODE_LIST,
		  dbev->get_metric_list (MET_NORMAL), dbev->get_metric_list (MET_NORMAL));
      break;
    case LDETAIL:
      print_func (Histable::LINE, MODE_DETAIL,
		  dbev->get_metric_list (MET_NORMAL), dbev->get_metric_ref (MET_NORMAL));
      break;
    case OBJECTS:
      print_objects ();
      break;
    case OVERVIEW_NEW:
      print_overview ();
      break;
    case LOADOBJECT:
      print_segments ();
      break;
    case GPROF:
      print_func (Histable::FUNCTION, MODE_GPROF,
		  dbev->get_metric_list (MET_CALL), dbev->get_metric_list (MET_NORMAL));
      break;
    case CALLTREE:
      if (dbev->comparingExperiments ())
	{
	  fprintf (out_file, GTXT ("\nNot available when comparing experiments\n\n"));
	  break;
	}
      print_ctree (cmd_type);
      break;
    case CSINGLE:
    case CPREPEND:
    case CAPPEND:
    case CRMFIRST:
    case CRMLAST:
      print_gprof (cmd_type, arg1, arg2);
      break;
    case EXP_LIST:
      exp_list ();
      break;
    case DESCRIBE:
      describe ();
      break;
    case SCOMPCOM:
      status = dbev->proc_compcom (arg1, true, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
      break;
    case STHRESH:
      status = dbev->proc_thresh (arg1, true, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
      break;
    case DCOMPCOM:
      status = dbev->proc_compcom (arg1, false, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
      break;
    case COMPCOM:
      status = dbev->proc_compcom (arg1, true, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
      status = dbev->proc_compcom (arg1, false, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
      break;
    case DTHRESH:
      status = dbev->proc_thresh (arg1, false, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
      break;
    case SOURCE:
    case DISASM:
      {
	if (arg3 != NULL)
	  abort ();
	if (arg1 == NULL)
	  {
	    fprintf (stderr, GTXT ("Error: Invalid function/file setting: \n"));
	    break;
	  }
	char *fcontext = NULL;
	char *arg = parse_fname (arg1, &fcontext);
	if (arg == NULL)
	  {
	    fprintf (stderr, GTXT ("Error: Invalid function/file setting: %s\n"), arg1);
	    free (fcontext);
	    break;
	  }
	if (arg2 && (strlen (arg2) == 0))
	  arg2 = NULL;
	print_anno_file (arg, arg2, fcontext, cmd_type == DISASM,
			 dis_file, inp_file, out_file, dbev, xdefault);
	free (arg);
	free (fcontext);
	break;
      }
    case METRIC_LIST:
      proc_cmd (METRICS, cparam, NULL, NULL);
      dbev->get_metric_ref (MET_NORMAL)->print_metric_list (dis_file,
							    GTXT ("Available metrics:\n"), false);
      break;
    case METRICS:
      if (arg1)
	{
	  char *ret = dbev->setMetrics (arg1, false);
	  if (ret != NULL)
	    {
	      fprintf (stderr, GTXT ("Error: %s\n"), ret);
	      proc_cmd (METRIC_LIST, cparam, NULL, NULL);
	      break;
	    }
	}
      scratch = dbev->get_metric_list (MET_NORMAL)->get_metrics ();
      fprintf (dis_file, GTXT ("Current metrics: %s\n"), scratch);
      free (scratch);
      proc_cmd (SORT, cparam, NULL, NULL);
      break;
    case GMETRIC_LIST:
      scratch = dbev->get_metric_list (MET_CALL)->get_metrics ();
      fprintf (dis_file, GTXT ("Current caller-callee metrics: %s\n"), scratch);
      free (scratch);
      fprintf (dis_file, GTXT ("Current caller-callee sort Metric: %s\n"),
	       dbev->getSort (MET_DATA));
      break;
    case INDX_METRIC_LIST:
      scratch = dbev->get_metric_list (MET_INDX)->get_metrics ();
      fprintf (dis_file, GTXT ("Current index-object metrics: %s\n"), scratch);
      free (scratch);
      scratch = dbev->getSort (MET_INDX);
      fprintf (dis_file, GTXT ("Current index-object sort Metric: %s\n"), scratch);
      free (scratch);
      break;
    case SORT:
      if (arg1)
	{
	  char *ret = dbev->setSort (arg1, MET_NORMAL, false);
	  if (ret != NULL)
	    {
	      fprintf (stderr, GTXT ("Error: %s\n"), ret);
	      proc_cmd (METRICS, cparam, NULL, NULL);
	      break;
	    }
	  dbev->setSort (arg1, MET_SRCDIS, false);
	  dbev->setSort (arg1, MET_CALL, false);
	  dbev->setSort (arg1, MET_DATA, false);
	  dbev->setSort (arg1, MET_INDX, false);
	  dbev->setSort (arg1, MET_CALL_AGR, false);
	  dbev->setSort (arg1, MET_IO, false);
	  dbev->setSort (arg1, MET_HEAP, false);
	}
      scratch = dbev->getSort (MET_NORMAL);
      scratch1 = dbev->getSortCmd (MET_NORMAL);
      fprintf (dis_file,
	       GTXT ("Current Sort Metric: %s ( %s )\n"), scratch, scratch1);
      free (scratch1);
      free (scratch);
      break;
    case OBJECT_SHOW:
      if (arg1)
	set_libexpand (arg1, LIBEX_SHOW);
      obj_list ();
      break;
    case OBJECT_HIDE:
      if (arg1)
	set_libexpand (arg1, LIBEX_HIDE);
      obj_list ();
      break;
    case OBJECT_API:
      if (arg1)
	set_libexpand (arg1, LIBEX_API);
      obj_list ();
      break;
    case OBJECTS_DEFAULT:
      set_libdefaults ();
      obj_list ();
      break;
    case OBJECT_LIST:
      obj_list ();
      break;
    case OBJECT_SELECT:
      if (arg1)
	{
	  if (process_object_select (arg1) != -1)
	    proc_cmd (OBJECT_LIST, cparam, NULL, NULL);
	  else
	    fprintf (stderr, GTXT ("Error: Type \"object_list\" for a list of all load objects.\n"));
	}
      else
	fprintf (stderr, GTXT ("Error: No load object has been specified.\n"));
      break;
    case LOADOBJECT_LIST:
      seg_list ();
      break;
    case LOADOBJECT_SELECT:
      if (arg1)
	{
	  if (process_object_select (arg1) != -1)
	    proc_cmd (LOADOBJECT_LIST, cparam, NULL, NULL);
	  else
	    fprintf (stderr, GTXT ("Error: Type \"segment_list\" for a list of all segments.\n"));
	}
      else
	fprintf (stderr, GTXT ("Error: No segment has been specified.\n"));
      break;
    case SAMPLE_LIST:
      filter_list (SAMPLE_LIST);
      break;
    case SAMPLE_SELECT:
      if (arg1 && !dbev->set_pattern (SAMPLE_FILTER_IDX, arg1))
	fprintf (stderr, GTXT ("Error: Invalid filter pattern specification %s\n"), arg1);
      proc_cmd (SAMPLE_LIST, cparam, NULL, NULL);
      break;
    case THREAD_LIST:
      filter_list (THREAD_LIST);
      break;
    case THREAD_SELECT:
      if (arg1 && !dbev->set_pattern (THREAD_FILTER_IDX, arg1))
	fprintf (stderr, GTXT ("Error: Invalid filter pattern specification %s\n"), arg1);
      proc_cmd (THREAD_LIST, cparam, NULL, NULL);
      break;
    case LWP_LIST:
      filter_list (LWP_LIST);
      break;
    case LWP_SELECT:
      if (arg1 && !dbev->set_pattern (LWP_FILTER_IDX, arg1))
	fprintf (stderr, GTXT ("Error: Invalid filter pattern specification %s\n"), arg1);
      proc_cmd (LWP_LIST, cparam, NULL, NULL);
      break;
    case CPU_LIST:
      filter_list (CPU_LIST);
      break;
    case CPU_SELECT:
      if (arg1 && !dbev->set_pattern (CPU_FILTER_IDX, arg1))
	fprintf (stderr, GTXT ("Error: Invalid filter pattern specification %s\n"), arg1);
      proc_cmd (CPU_LIST, cparam, NULL, NULL);
      break;
    case FILTERS:
      if (arg1 != NULL)
	{
	  if (strcmp (arg1, NTXT ("True")) == 0)
	    scratch = dbev->set_filter (NULL);
	  else
	    scratch = dbev->set_filter (arg1);
	  if (scratch != NULL)
	    fprintf (stderr, GTXT ("Error: %s\n"), scratch);
	}
      scratch = dbev->get_filter ();
      fprintf (dis_file, GTXT ("current filter setting: \"%s\"\n"),
	       scratch == NULL ? GTXT ("<none>") : scratch);
      break;
    case OUTFILE:
      if (arg1)
	{
	  set_outfile (arg1, out_file, false);
	  if (inp_file != stdin)
	    dis_file = out_file;
	}
      break;
    case APPENDFILE:
      if (arg1)
	{
	  set_outfile (arg1, out_file, true);
	  if (inp_file != stdin)
	    dis_file = out_file;
	}
      break;
    case LIMIT:
      if (arg1)
	{
	  limit = (int) strtol (arg1, (char **) NULL, 10);
	  char *res = dbeSetPrintLimit (dbevindex, limit);
	  if (res != NULL)
	    fprintf (stderr, NTXT ("%s\n"), res);
	}

      limit = dbeGetPrintLimit (dbevindex);
      fprintf (stderr, GTXT ("Print limit set to %d\n"), limit);
      break;
    case NAMEFMT:
      if (arg1)
	{
	  status = dbev->set_name_format (arg1);
	  if (status != CMD_OK)
	    fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
	}
      else
	fprintf (stderr, GTXT ("Error: No format has been specified.\n"));
      break;
    case VIEWMODE:
      {
	if (arg1)
	  {
	    status = dbev->set_view_mode (arg1, false);
	    if (status != CMD_OK)
	      fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
	  }
	const char *vname = "unknown";
	int vm = dbev->get_view_mode ();
	switch (vm)
	  {
	  case VMODE_USER:
	    vname = "user";
	    break;
	  case VMODE_EXPERT:
	    vname = "expert";
	    break;
	  case VMODE_MACHINE:
	    vname = "machine";
	    break;
	  }
	fprintf (stderr, GTXT ("Viewmode set to %s\n"), vname);
      }
      break;

      // EN_DESC does not make sense after experiments are read, but it does make sense on the command line,
      //	processed before the experiments are read.
    case EN_DESC:
      if (arg1)
	{
	  status = dbev->set_en_desc (arg1, false);
	  if (status != CMD_OK)
	    fprintf (stderr, GTXT ("Error: %s: %s\n"), Command::get_err_string (status), arg1);
	}
      else
	fprintf (stderr, GTXT ("Error: No descendant processing has been specified.\n"));
      break;
    case SETPATH:
    case ADDPATH:
      if (arg1)
	dbeSession->set_search_path (arg1, (cmd_type == SETPATH));
      fprintf (dis_file, GTXT ("search path:\n"));
      Vec_loop (char*, dbeSession->get_search_path (), index, name)
      {
	fprintf (dis_file, NTXT ("\t%s\n"), name);
      }
      break;
    case PATHMAP:
      {
	Vector<pathmap_t*> *pathMaps = dbeSession->get_pathmaps ();
	if (arg1 != NULL)
	  {
	    if (arg2 == NULL)
	      {
		fprintf (stderr, GTXT ("Error: No replacement path prefix has been specified.\n"));
		break;
	      }
	    // add this mapping to the session
	    char *err = Settings::add_pathmap (pathMaps, arg1, arg2);
	    if (err != NULL)
	      {
		fprintf (stderr, NTXT ("%s"), err);
		free (err);
	      }
	  }
	fprintf (dis_file, GTXT ("Path mappings: from -> to\n"));
	for (int i = 0, sz = pathMaps->size (); i < sz; i++)
	  {
	    pathmap_t *thismap = pathMaps->get (i);
	    fprintf (dis_file, NTXT ("\t`%s' -> `%s'\n"), thismap->old_prefix, thismap->new_prefix);
	  }
      }
      break;
    case SAMPLE_DETAIL:
      if (get_exp_id (arg1, bgn_index, end_index) != -1)
	{
	  cd = new er_print_experiment (dbev, bgn_index, end_index, false,
					false, false, true, true);
	  print_cmd (cd);
	  delete cd;
	}
      break;
    case STATISTICS:
      if (get_exp_id (arg1, bgn_index, end_index) != -1)
	{
	  cd = new er_print_experiment (dbev, bgn_index, end_index, false,
					false, true, true, false);
	  print_cmd (cd);
	  delete cd;
	}
      break;
    case PRINTMODE:
      {
	if (arg1 == NULL)
	  {
	    fprintf (stderr, GTXT ("printmode is set to `%s'\n\n"), dbeGetPrintModeString (dbevindex));
	    break;
	  }
	char *s = dbeSetPrintMode (dbevindex, arg1);
	if (s != NULL)
	  {
	    fprintf (stderr, NTXT ("%s\n"), s);
	    break;
	  }
	fprintf (stderr, GTXT ("printmode is set to `%s'\n\n"), dbeGetPrintModeString (dbevindex));
      }
      break;
    case HEADER:
      if (get_exp_id (arg1, bgn_index, end_index) != -1)
	{
	  cd = new er_print_experiment (dbev, bgn_index, end_index, false,
					true, false, false, false);
	  print_cmd (cd);
	  delete cd;
	}
      break;
    case COMPARE:
      if (arg1 == NULL)
	{
	  fprintf (out_file, GTXT ("The argument to `compare' must be `on', `off', `delta', or `ratio'\n\n"));
	  break;
	}
      else
	{
	  int cmp;
	  if (strcasecmp (arg1, NTXT ("OFF")) == 0 || strcmp (arg1, NTXT ("0")) == 0)
	    cmp = CMP_DISABLE;
	  else if (strcasecmp (arg1, NTXT ("ON")) == 0 || strcmp (arg1, NTXT ("1")) == 0)
	    cmp = CMP_ENABLE;
	  else if (strcasecmp (arg1, NTXT ("DELTA")) == 0)
	    cmp = CMP_DELTA;
	  else if (strcasecmp (arg1, NTXT ("RATIO")) == 0)
	    cmp = CMP_RATIO;
	  else
	    {
	      fprintf (out_file, GTXT ("The argument to `compare' must be `on', `off', `delta', or `ratio'\n\n"));
	      break;
	    }
	  int oldMode = dbev->get_compare_mode ();
	  dbev->set_compare_mode (cmp);
	  if (oldMode != cmp)
	    {
	      dbev->reset_data (false);
	      dbeSession->reset_data ();
	    }
	}
      break;
    case LEAKS:
      if (!dbeSession->is_leaklist_available ())
	{
	  fprintf (out_file, GTXT ("\nHeap trace information was not requested when recording experiments\n\n"));
	  break;
	}
      if (dbev->comparingExperiments ())
	{ // XXXX show warning for compare
	  fprintf (out_file, GTXT ("\nNot available when comparing experiments\n\n"));
	  break;
	}
      cd = new er_print_leaklist (dbev, true, false, dbev->get_limit ());
      print_cmd (cd);
      delete cd;
      break;
    case ALLOCS:
      if (!dbeSession->is_leaklist_available ())
	{
	  fprintf (out_file, GTXT ("\nHeap trace information was not requested when recording experiments\n\n"));
	  break;
	}
      cd = new er_print_leaklist (dbev, false, true, dbev->get_limit ());
      print_cmd (cd);
      delete cd;
      break;
    case HEAP:
      if (!dbeSession->is_heapdata_available ())
	{
	  fprintf (out_file, GTXT ("Heap trace information was not requested when recording experiments\n\n"));
	  break;
	}
      cd = new er_print_heapactivity (dbev, Histable::HEAPCALLSTACK, false, dbev->get_limit ());
      print_cmd (cd);
      delete cd;
      break;
    case HEAPSTAT:
      if (!dbeSession->is_heapdata_available ())
	{
	  fprintf (out_file, GTXT ("Heap trace information was not requested when recording experiments\n\n"));
	  break;
	}
      cd = new er_print_heapactivity (dbev, Histable::HEAPCALLSTACK, true, dbev->get_limit ());
      print_cmd (cd);
      delete cd;
      break;
    case IOACTIVITY:
      if (!dbeSession->is_iodata_available ())
	{
	  fprintf (out_file, GTXT ("I/O trace information was not requested when recording experiments\n\n"));
	  break;
	}
      if (dbev->comparingExperiments ())
	{ // XXXX show warning for compare
	  fprintf (out_file, GTXT ("\nNot available when comparing experiments\n\n"));
	  break;
	}
      cd = new er_print_ioactivity (dbev, Histable::IOACTFILE, false, dbev->get_limit ());
      print_cmd (cd);
      delete cd;
      break;
    case IOVFD:
      if (!dbeSession->is_iodata_available ())
	{
	  fprintf (out_file, GTXT ("I/O trace information was not requested when recording experiments\n\n"));
	  break;
	}
      cd = new er_print_ioactivity (dbev, Histable::IOACTVFD, false, dbev->get_limit ());
      print_cmd (cd);
      delete cd;
      break;
    case IOCALLSTACK:
      if (!dbeSession->is_iodata_available ())
	{
	  fprintf (out_file, GTXT ("I/O trace information was not requested when recording experiments\n\n"));
	  break;
	}
      cd = new er_print_ioactivity (dbev, Histable::IOCALLSTACK, false, dbev->get_limit ());
      print_cmd (cd);
      delete cd;
      break;
    case IOSTAT:
      if (!dbeSession->is_iodata_available ())
	{
	  fprintf (out_file, GTXT ("I/O trace information was not requested when recording experiments\n\n"));
	  break;
	}
      cd = new er_print_ioactivity (dbev, Histable::IOACTVFD, true, dbev->get_limit ());
      print_cmd (cd);
      delete cd;
      break;
    case HELP:
      Command::print_help(whoami, false, true, out_file);
      break;
    case VERSION_cmd:
      Application::print_version_info ();
      break;
    case SCRIPT:
      if (arg1)
	{
	  ck_file = fopen (arg1, NTXT ("r"));
	  if (ck_file == NULL)
	    fprintf (stderr, GTXT ("Error: Script file cannot be opened: %s\n"), arg1);
	  else
	    {
	      save_file = inp_file;
	      inp_file = ck_file;
	      proc_script ();
	      inp_file = save_file;
	    }
	}
      else
	fprintf (stderr, GTXT ("Error: No filename has been specified.\n"));
      break;
    case QUIT:
      exit (0);
      break;

      // commands relating to index Objects
    case INDXOBJ:
      if ((cparam == -1) && (arg1 == NULL))
	{
	  fprintf (stderr, GTXT ("Error: No index object name has been specified.\n"));
	  break;
	}
      // automatically load machine model if applicable
      dbeDetectLoadMachineModel (dbevindex);
      indxobj (arg1, cparam);
      break;
    case INDXOBJLIST:
      // automatically load machine model if applicable
      dbeDetectLoadMachineModel (dbevindex);
      indxo_list (false, out_file);
      break;

      // define a new IndexObject type
    case INDXOBJDEF:
      if (arg1 == NULL)
	{
	  fprintf (stderr, GTXT ("Error: No index object name has been specified.\n"));
	  break;
	}
      if (arg2 == NULL)
	{
	  fprintf (stderr, GTXT ("Error: No index-expr has been specified.\n"));
	  break;
	}
      indxo_define (arg1, arg2, arg3, arg4);
      break;

      // the commands following this are unsupported/hidden
    case IFREQ:
      if (!dbeSession->is_ifreq_available ())
	{
	  fprintf (out_file, GTXT ("\nInstruction frequency data was not requested when recording experiments\n\n"));
	  break;
	}
      ifreq ();
      break;
    case DUMPNODES:
      dump_nodes ();
      break;
    case DUMPSTACKS:
      dump_stacks ();
      break;
    case DUMPUNK:
      dump_unk_pcs ();
      break;
    case DUMPFUNC:
      dump_funcs (arg1);
      break;
    case DUMPDOBJS:
      dump_dataobjects (arg1);
      break;
    case DUMPMAP:
      dump_map ();
      break;
    case DUMPENTITIES:
      dump_entities ();
      break;
    case DUMP_PROFILE:
      dbev->dump_profile (out_file);
      break;
    case DUMP_SYNC:
      dbev->dump_sync (out_file);
      break;
    case DUMP_HWC:
      dbev->dump_hwc (out_file);
      break;
    case DUMP_HEAP:
      if (!dbeSession->is_leaklist_available ())
	{
	  fprintf (out_file, GTXT ("\nHeap trace information was not requested when recording experiments\n\n"));
	  break;
	}
      dbev->dump_heap (out_file);
      break;
    case DUMP_IOTRACE:
      if (!dbeSession->is_iodata_available ())
	{
	  fprintf (out_file, GTXT ("\nI/O trace information was not requested when recording experiments\n\n"));
	  break;
	}
      dbev->dump_iotrace (out_file);
      break;
    case DMEM:
      if (arg1 == NULL)
	fprintf (stderr, GTXT ("Error: No sample has been specified.\n"));
      else
	{
	  Experiment *exp = dbeSession->get_exp (0);
	  if (exp != NULL)
	    exp->DBG_memuse (arg1);
	}
      break;
    case DUMP_GC:
      if (!dbeSession->has_java ())
	{
	  fprintf (out_file, GTXT ("\nJava garbage collection information was not requested when recording experiments\n\n"));
	  break;
	}
      dbev->dump_gc_events (out_file);
      break;
    case DKILL:
      {
	if (arg1 == NULL)
	  {
	    fprintf (stderr, GTXT ("Error: No process has been specified.\n"));
	    break;
	  }
	if (arg2 == NULL)
	  {
	    fprintf (stderr, GTXT ("Error: No signal has been specified.\n"));
	    break;
	  }
	pid_t p = (pid_t) atoi (arg1);
	int signum = atoi (arg2);
	char *ret = dbeSendSignal (p, signum);
	if (ret != NULL)
	  fprintf (stderr, GTXT ("Error: %s"), ret);
      }
      break;
    case PROCSTATS:
      dump_stats ();
      break;
    case ADD_EXP:
    case OPEN_EXP:
      if (arg1 == NULL)
	fprintf (stderr, GTXT ("Error: No experiment name has been specified.\n"));
      else
	{
	  Vector<Vector<char*>*> *groups = new Vector<Vector<char*>*>(1);
	  Vector<char*> *list = new Vector<char*>(1);
	  list->append (arg1);
	  groups->append (list);
	  char *res = dbeOpenExperimentList (dbevindex, groups, cmd_type == OPEN_EXP);
	  if (cmd_type == OPEN_EXP)
	    fprintf (stderr, GTXT ("Previously loaded experiment have been dropped.\n"));
	  if (res != NULL)
	    fprintf (stderr, NTXT ("%s"), res);
	  else
	    fprintf (stderr, GTXT ("Experiment %s has been loaded\n"), arg1);
	  free (res);
	  delete list;
	  delete groups;
	}
      break;
    case DROP_EXP:
      {
	if (arg1 == NULL)
	  fprintf (stderr, GTXT ("Error: No experiment name has been specified.\n"));
	else
	  {
	    int exp_index = dbeSession->find_experiment (arg1);
	    if (exp_index < 0)
	      fprintf (stderr, GTXT ("Error: experiment %s has not been opened.\n"), arg1);
	    else
	      {
		Vector<int> *expid = new Vector<int> (1);
		expid->append (exp_index);
		char *res = dbeDropExperiment (dbevindex, expid);
		if (res != NULL)
		  fprintf (stderr, NTXT ("%s"), res);
		else
		  fprintf (stderr, GTXT ("Experiment %s has been dropped\n"), arg1);
		delete expid;
		free (res);
	      }
	  }
      }
      break;
    case HHELP:
      // automatically load machine model if applicable
      dbeDetectLoadMachineModel (dbevindex);
      Command::print_help (whoami, false, false, out_file);
      fprintf (out_file, NTXT ("\n"));
      indxo_list (false, out_file);
      fprintf (out_file, NTXT ("\n"));
      mo_list (false, out_file);
      if (!getenv ("_BUILDING_MANPAGE"))
	fprintf (out_file, GTXT ("\nSee gprofng(1) for more details\n"));
      break;
    case QQUIT:
      was_QQUIT = true;
      return;
    default:
      fprintf (stderr, GTXT ("Error: Invalid option\n"));
      break;
    }

  // check for any processing error messages
  dump_proc_warnings ();
  fflush (out_file);
}

#define MAX_NUM_HEADER      4

void
er_print::disp_list (int num_header, int size, int align[], char *header[],
		     char **lists[])
{
  size_t maxlen[MAX_NUM_HEADER];
  char fmt[MAX_NUM_HEADER][64];
  if (num_header > MAX_NUM_HEADER)
    abort ();
  for (int i = 0; i < num_header; i++)
    {
      maxlen[i] = strlen (header[i]);
      for (int j = 0; j < size; j++)
	{
	  size_t len = strlen (lists[i][j]);
	  if (maxlen[i] < len)
	    maxlen[i] = len;
	}

      // get format string
      if ((align[i] == -1) && (i == num_header - 1))
	snprintf (fmt[i], sizeof (fmt[i]), NTXT ("%%s "));
      else
	snprintf (fmt[i], sizeof (fmt[i]), NTXT ("%%%ds "), (int) (align[i] * maxlen[i]));

      // write header
      fprintf (out_file, fmt[i], header[i]);
    }
  putc ('\n', out_file);

  // write separator "==="
  size_t np = 0;
  for (int i = 0; (i < num_header) && (np < 132); i++)
    {
      size_t nc = maxlen[i];
      if (nc + np > 132)
	nc = 132 - np;
      for (size_t j = 0; j < nc; j++)
	putc ('=', out_file);
      putc (' ', out_file);
      np += nc + 1;
    }
  putc ('\n', out_file);

  // write lists
  for (int j = 0; j < size; j++)
    {
      for (int i = 0; i < num_header; i++)
	fprintf (out_file, fmt[i], lists[i][j]);
      putc ('\n', out_file);
    }
}

void
er_print::exp_list ()
{
  int size, index;
  int align[MAX_NUM_HEADER];
  char *header[MAX_NUM_HEADER];
  char **lists[MAX_NUM_HEADER];

  align[0] = 1;     // right-justify
  align[1] = 1;     // right-justify
  align[2] = 1;     // right-justify
  align[3] = -1;    // left-justify
  header[0] = GTXT ("ID");
  header[1] = GTXT ("Sel");
  header[2] = GTXT ("PID");
  header[3] = GTXT ("Experiment");

  size = dbeSession->nexps ();
  lists[0] = new char*[size];
  lists[1] = new char*[size];
  lists[2] = new char*[size];
  lists[3] = new char*[size];
  for (index = 0; index < size; index++)
    {
      lists[0][index] = dbe_sprintf (NTXT ("%d"), index + 1);
      lists[1][index] = strdup (dbev->get_exp_enable (index) ? GTXT ("yes") : GTXT ("no"));
      lists[2][index] = dbe_sprintf (NTXT ("%d"), dbeSession->get_exp (index)->getPID ());
      lists[3][index] = strdup (dbeSession->get_exp (index)->get_expt_name ());
    }
  disp_list (4, size, align, header, lists);
  for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < size; j++)
	free (lists[i][j]);
      delete[] lists[i];
    }
}

void
er_print::describe ()
{
  Vector<void*> *res = dbeGetFilterKeywords (dbev->vindex);
  if (res == NULL)
    return;
  Vector <char*> *kwCategoryI18N = (Vector<char*>*) res->fetch (1);
  Vector <char*> *kwKeyword = (Vector<char*>*) res->fetch (3);
  Vector <char*> *kwFormula = (Vector<char*>*) res->fetch (4);
  Vector <char*> *kwDescrip = (Vector<char*>*) res->fetch (5);
  Vector <void*> *kwEnumDescs = (Vector<void*>*) res->fetch (6);
  String sectionFormat = NTXT ("\n------ %s ------\n");
  String categoryFormat = NTXT ("\n%s\n");
  String keywordFormat = NTXT ("   %-20s  %s\n");
  String empty = NTXT ("");
  String previousCategory = empty;

  for (int i = 0; i < kwKeyword->size (); i++)
    {
      if (kwKeyword->fetch (i) == NULL)
	{
	  fprintf (dis_file, sectionFormat, kwCategoryI18N->fetch (i));
	  continue;
	}
      String cat = kwCategoryI18N->fetch (i);
      if (dbe_strcmp (previousCategory, cat) != 0)
	fprintf (dis_file, categoryFormat, cat);
      previousCategory = cat;
      Vector <String> *enumDescs = (Vector <String> *) kwEnumDescs->fetch (i);
      String keyword = kwKeyword->fetch (i);
      if (kwDescrip->fetch (i) != NULL)
	{
	  fprintf (dis_file, keywordFormat, keyword, kwDescrip->fetch (i));
	  keyword = empty;
	}
      if (kwFormula->fetch (i) != NULL)
	{
	  fprintf (dis_file, keywordFormat, keyword, kwFormula->fetch (i));
	  keyword = empty;
	  continue;
	}
      int numEnums = enumDescs != NULL ? enumDescs->size () : 0;
      for (int jj = 0; jj < numEnums; jj++)
	{
	  fprintf (dis_file, keywordFormat, keyword, enumDescs->fetch (jj));
	  keyword = empty;
	}
    }
  destroy (res);
}

void
er_print::obj_list ()
{
  LoadObject *lo;
  int index;
  int align[MAX_NUM_HEADER];
  char *header[MAX_NUM_HEADER];
  char **lists[MAX_NUM_HEADER];
  Vector<LoadObject*> *text_segments = dbeSession->get_text_segments ();
  if (text_segments->size () == 0)
    {
      fprintf (dis_file, GTXT ("There are no load objects in this experiment\n"));
      return;
    }
  align[0] = -1; // left-justify
  align[1] = -1; // left-justify
  align[2] = -1; // left-justify
  align[3] = -1; // left-justify
  header[0] = GTXT ("Sel");
  header[1] = GTXT ("Load Object");
  header[2] = GTXT ("Index");
  header[3] = GTXT ("Path");

  int size = text_segments->size ();
  lists[0] = new char*[size];
  lists[1] = new char*[size];
  lists[2] = new char*[size];
  lists[3] = new char*[size];

  char *lo_name;
  int new_index = 0;
  Vec_loop (LoadObject*, text_segments, index, lo)
  {
    lo_name = lo->get_name ();
    if (lo_name != NULL)
      {
	size_t len = strlen (lo_name);
	if (len > 7 && streq (lo_name + len - 7, NTXT (".class>")))
	  continue;
      }
    LibExpand expand = dbev->get_lo_expand (lo->seg_idx);
    switch (expand)
      {
      case LIBEX_SHOW:
	lists[0][new_index] = dbe_strdup (GTXT ("show"));
	break;
      case LIBEX_HIDE:
	lists[0][new_index] = dbe_strdup (GTXT ("hide"));
	break;
      case LIBEX_API:
	lists[0][new_index] = dbe_strdup (GTXT ("API-only"));
	break;
      }
    lists[1][new_index] = dbe_strdup (lo_name);
    lists[2][new_index] = dbe_sprintf (NTXT ("%d"), lo->seg_idx);
    lists[3][new_index] = dbe_strdup (lo->get_pathname ());
    new_index++;
  }
  disp_list (4, new_index, align, header, lists);
  for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < new_index; j++)
	free (lists[i][j]);
      delete[] lists[i];
    }
  delete text_segments;
}

void
er_print::seg_list ()
{
  LoadObject *lo;
  int index;
  int align[MAX_NUM_HEADER];
  char *header[MAX_NUM_HEADER];
  char **lists[MAX_NUM_HEADER];

  // XXX seg_list only prints text segments; should extend to all
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  if (lobjs->size () == 0)
    {
      fprintf (dis_file, GTXT ("There are no segments in this experiment\n"));
      return;
    }
  align[0] = -1; // left-justify
  align[1] = 1;  // right-justify
  align[2] = -1; // left-justify
  header[0] = GTXT ("Sel");
  header[1] = GTXT ("Size");
  header[2] = GTXT ("Segment");

  int size = lobjs->size ();
  lists[0] = new char*[size];
  lists[1] = new char*[size];
  lists[2] = new char*[size];

  char *lo_name;
  int new_index = 0;
  Vec_loop (LoadObject*, lobjs, index, lo)
  {
    lo_name = lo->get_name ();
    if (lo_name != NULL)
      {
	size_t len = strlen (lo_name);
	if (len > 7 && streq (lo_name + len - 7, NTXT (".class>")))
	  continue;
      }
    bool expand = dbev->get_lo_expand (lo->seg_idx);
    lists[0][new_index] = strdup (expand ? GTXT ("yes") : GTXT ("no"));
    lists[1][new_index] = dbe_sprintf (NTXT ("%lld"), (ll_t) lo->get_size ());
    lists[2][new_index] = strdup (lo->get_pathname ());
    new_index++;
  }

  disp_list (3, new_index, align, header, lists);
  for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < new_index; j++)
	free (lists[i][j]);
      delete[] lists[i];
    }
  delete lobjs;
}

void
er_print::filter_list (CmdType cmd_type)
{
  FilterNumeric *select;
  int index;
  int align[MAX_NUM_HEADER];
  char *header[MAX_NUM_HEADER];
  char **lists[MAX_NUM_HEADER];
  char *pattern;

  // first ensure that the data has been read
  MetricList *mlist = dbev->get_metric_list (MET_INDX);
  Hist_data *data = dbev->get_hist_data (mlist, Histable::INDEXOBJ, 0, Hist_data::ALL);
  delete data;

  align[0] = 1;  // right-justify
  align[1] = -1; // left-justify
  align[2] = 1;  // right-justify
  align[3] = 1;  // right-justify
  header[0] = GTXT ("Exp");
  header[1] = GTXT ("Sel");
  header[2] = GTXT ("Total");
  header[3] = GTXT ("Status");

  int size = dbeSession->nexps ();
  lists[0] = new char*[size];
  lists[1] = new char*[size];
  lists[2] = new char*[size];
  lists[3] = new char*[size];
  int new_index = 0;
  for (index = 0; index < size; index++)
    {
      switch (cmd_type)
	{
	case SAMPLE_LIST:
	  select = dbev->get_FilterNumeric (index, SAMPLE_FILTER_IDX);
	  break;
	case THREAD_LIST:
	  select = dbev->get_FilterNumeric (index, THREAD_FILTER_IDX);
	  break;
	case LWP_LIST:
	  select = dbev->get_FilterNumeric (index, LWP_FILTER_IDX);
	  break;
	case CPU_LIST:
	  select = dbev->get_FilterNumeric (index, CPU_FILTER_IDX);
	  break;
	default:
	  abort (); // internal error
	}
      if (select == NULL)
	continue;
      lists[0][new_index] = dbe_sprintf (NTXT ("%d"), index + 1);
      pattern = dbev->get_exp_enable (index) ? select->get_pattern () : NULL;
      lists[1][new_index] = strdup (pattern && *pattern ? pattern : GTXT ("none"));
      lists[2][new_index] = dbe_sprintf (NTXT ("%lld"), (ll_t) select->nelem ());
      lists[3][new_index] = select->get_status ();
      new_index++;
    }
  disp_list (3, size, align, header, lists);
  for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < new_index; j++)
	free (lists[i][j]);
      delete[] lists[i];
    }
}

int
er_print::check_exp_id (int exp_id, char *sel)
{
  if (exp_id < 0 || exp_id >= dbeSession->nexps ())
    {
      fprintf (stderr, GTXT ("Error: Invalid number entered: %s\nType \"exp_list\" for a list of all experiments.\n"),
	       sel);
      return -1;
    }
  return exp_id;
}

int
er_print::get_exp_id (char *sel, int &bgn_index, int &end_index)
{
  int id, exp_id;
  if (sel == NULL || strcmp (sel, NTXT ("all")) == 0)
    {
      // loop over all experiments
      bgn_index = 0;
      end_index = dbeSession->nexps () - 1;
    }
  else
    {
      id = (int) strtol (sel, (char **) NULL, 10) - 1;
      exp_id = check_exp_id (id, sel);
     if (exp_id == -1)
	return -1;
      bgn_index = end_index = exp_id;
    }
  return 0;
}

void
er_print::print_objects ()
{
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  char *msg = pr_load_objects (lobjs, NTXT (""));
  delete lobjs;
  fprintf (out_file, NTXT ("%s\n"), msg);
  free (msg);
}

void
er_print::print_overview ()
{
  //fprintf(out_file, NTXT("%s\n"), GTXT("Not implemented yet."));//YXXX
  Vector<char*> *status = dbeGetOverviewText (dbevindex);
  StringBuilder sb;
  sb.append (GTXT ("Experiment(s):\n\n"));
  for (int i = 0; i < status->size (); i++)
    sb.appendf (NTXT ("%s\n"), status->fetch (i));
  sb.append (GTXT ("Metrics:\n"));
  sb.toFile (out_file);

  Vector<void*> *data = dbeGetRefMetricTree (dbevindex, false);
  Vector<char *> *metric_cmds = new Vector<char *>();
  Vector<char *> *non_metric_cmds = new Vector<char *>();
  print_overview_nodes (data, 0, metric_cmds, non_metric_cmds);
  Vector<void*> *values = dbeGetRefMetricTreeValues (0, metric_cmds, non_metric_cmds);
  print_overview_tree (data, 0, values, metric_cmds, non_metric_cmds);

  StringBuilder sb2;
  sb2.append (GTXT ("\nNotes: '*' indicates hot metrics, '[X]' indicates currently enabled metrics.\n"));
  sb2.append (GTXT ("       The metrics command can be used to change selections. The metric_list command lists all available metrics.\n"));
  sb2.toFile (out_file);
}

void
er_print::print_overview_nodes (Vector<void*> * data, int level, Vector<char *> *metric_cmds, Vector<char *> *non_metric_cmds)
{
  Vector<void*> *fields = (Vector<void*> *) data->fetch (0);
  Vector<void*> *children = (Vector<void*> *) data->fetch (1);
  char *name = ((Vector<char*> *)fields->fetch (0))->fetch (0);
  int vstyles_capable = ((Vector<int>*) fields->fetch (5))->fetch (0); //bitmask e.g.VAL_TIMEVAL
  bool has_value = ((Vector<bool>*) fields->fetch (10))->fetch (0);
  bool selectable = (vstyles_capable != 0) ? true : false;
  if (selectable)
    metric_cmds->append (name);
  else if (has_value)
    non_metric_cmds->append (name);

  level++;
  for (int i = 0; i < children->size (); i++)
    print_overview_nodes ((Vector<void*> *)(children->fetch (i)), level, metric_cmds, non_metric_cmds);
}

void
er_print::print_overview_tree (Vector<void*> * data, int level, Vector<void*> * values, Vector<char *> *metric_cmds, Vector<char *> *non_metric_cmds)
{
  Vector<void*> * fields = (Vector<void*> *) data->fetch (0);
  Vector<void*> * children = (Vector<void*> *) data->fetch (1);
  char *name = ((Vector<char*> *)fields->fetch (0))->fetch (0);
  char *username = ((Vector<char*> *)fields->fetch (1))->fetch (0);
  int flavors = ((Vector<int>*) fields->fetch (3))->fetch (0); //bitmask e.g. EXCLUSIVE
  int vstyles_capable = ((Vector<int>*) fields->fetch (5))->fetch (0); //bitmask e.g.VAL_TIMEVAL
  //    bool aggregation = ((Vector<bool>*) fields->fetch(9))->fetch(0);
  //    bool has_value = ((Vector<bool>*) fields->fetch(10))->fetch(0);
  char *unit = ((Vector<char*> *) fields->fetch (11))->fetch (0);

  StringBuilder sb;
  for (int i = 0; i < level * 2; i++)
    sb.append (NTXT (" ")); // NOI18N

  bool selectable = (vstyles_capable != 0) ? true : false;
  if (selectable)
    {
      bool isSelected = dbev->get_metric_list (MET_NORMAL)->find_metric_by_name (name) == NULL ? false : true;
      if (isSelected)
	sb.append (NTXT ("[X]"));
      else
	sb.append (NTXT ("[ ]"));
    }
  if ((unit != NULL && dbe_strcmp (unit, UNIT_SECONDS) == 0)
      || (unit == NULL && vstyles_capable & VAL_TIMEVAL))
    unit = GTXT ("Seconds");

  bool isHiddenInOverview = ((flavors & BaseMetric::STATIC) != 0);
  if (name != NULL && dbe_strcmp (name, L1_STATIC) == 0)
    isHiddenInOverview = true;
  if (!dbeSession->has_java () && name != NULL && dbe_strcmp (name, L1_GCDURATION) == 0)
    isHiddenInOverview = true;
  if (isHiddenInOverview)
    return;

  sb.append (username == NULL ? NTXT ("") : username); // NOI18N
  int show = 0;
  if (name == NULL)
    show = 0;
  else if (strstr (name, NTXT ("PROFDATA_TYPE_")) == NULL)
    show = 1;

  if (show)
    {
      sb.append (username == NULL ? NTXT ("") : NTXT (" - ")); // NOI18N
      sb.append (name == NULL ? NTXT ("") : name); // NOI18N
    }

  // "Bugs 16624403 and 19539622" (leave this string intact for searches)
  // add an extra condition for now
  // once we have proper fixes, eliminate test on Bug16624402_extra_condition
  int Bug16624402_extra_condition = 1;
  if (username)
    {
      if (strcmp (username, NTXT ("Block Covered %")) == 0) Bug16624402_extra_condition = 0;
      if (strcmp (username, NTXT ("Instr Covered %")) == 0) Bug16624402_extra_condition = 0;
    }
  if (Bug16624402_extra_condition > 0 && values->size () > 0)
    {
      Vector<void*> * valueColumns = (Vector<void*> *)values->fetch (0);
      Vector<void*> * highlightColumns = (Vector<void*> *)values->fetch (1);
      int jj = 0;
      int found = 0;
      for (jj = 0; jj < valueColumns->size (); jj++)
	{
	  const char *value_name = "";
	  if (jj < metric_cmds->size ())
	    value_name = metric_cmds->fetch (jj);
	  else
	    value_name = non_metric_cmds->fetch (jj - metric_cmds->size ());
	  if (dbe_strcmp (value_name, name) != 0)
	    continue;
	  else
	    {
	      found = 1;
	      break;
	    }
	}
      if (found)
	{
	  Vector<void*> * valueVec = (Vector<void*> *)valueColumns->fetch (jj);
	  Vector<bool> * highlights = (Vector<bool> *)highlightColumns->fetch (jj);
	  for (int kk = 0; kk < valueVec->size (); kk++)
	    {
	      char * value_str;
	      int show_value = 0;
	      switch (valueVec->type ())
		{
		case VEC_INTEGER:
		  value_str = dbe_sprintf (NTXT ("%ld"), (long) (((Vector<int> *)valueVec)->fetch (kk)));
		  show_value = 1;
		  break;
		case VEC_DOUBLE:
		  value_str = dbe_sprintf (NTXT ("%.3f"), (double) (((Vector<double> *)valueVec)->fetch (kk)));
		  show_value = 1;
		  break;
		case VEC_LLONG:
		  value_str = dbe_sprintf (NTXT ("%lld"), (long long) (((Vector<long> *)valueVec)->fetch (kk)));
		  show_value = 1;
		  break;
		case VEC_STRING:
		  value_str = NTXT ("");
		  break;
		default:
		  value_str = NTXT ("");
		}
	      if (show_value)
		{
		  if (kk == 0)
		    {
		      sb.append (unit == NULL ? NTXT ("") : NTXT (" ("));
		      sb.append (unit == NULL ? NTXT ("") : unit);
		      sb.append (unit == NULL ? NTXT ("") : NTXT (")"));
		      sb.append (NTXT (":"));
		    }
		  bool highlight = highlights->fetch (kk);
		  const char * hilite = highlight ? NTXT ("*") : NTXT ("");
		  sb.append (NTXT (" ["));
		  sb.append (hilite);
		  sb.append (value_str);
		  sb.append (NTXT ("]"));
		}
	    }
	}
    }
  sb.append (NTXT ("\n"));
  sb.toFile (out_file);
  level++;
  for (int i = 0; i < children->size (); i++)
    print_overview_tree ((Vector<void*> *)(children->fetch (i)), level, values, metric_cmds, non_metric_cmds);
}

void
er_print::print_segments ()
{
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  char *msg = pr_load_objects (lobjs, NTXT (""));
  delete lobjs;
  fprintf (dis_file, NTXT ("Not implemented yet!\n"));
  free (msg);
}

void
er_print::print_dobj (Print_mode mode, MetricList *mlist1,
		      char *dobj_name, char *sel)
{
  Hist_data *hist_data = NULL;
  char *errstr;
  er_print_common_display *cd;
  int list_limit = limit;
  Histable *sobj = NULL;
  Dprintf (DEBUG_DATAOBJ, NTXT ("er_print::print_dobj(mode=%d,dobj=%s,sel=%s)\n"),
	   mode, (dobj_name == NULL) ? NTXT ("0") : dobj_name, (sel == NULL) ? NTXT ("0") : sel);
  char *name = dbev->getSort (MET_DATA);
  switch (mode)
    {
    case MODE_LIST:
      hist_data = dbev->get_hist_data (mlist1, Histable::DOBJECT, 0, Hist_data::ALL);
      break;
    case MODE_DETAIL:
      // if specified, find the dataobject from the name
      if (dobj_name && strcmp (dobj_name, NTXT ("<All>")))
	{
	  if (!dbeSession->find_obj (dis_file, inp_file, sobj, dobj_name,
				     sel, Histable::DOBJECT, (inp_file != stdin)))
	    return;
	  if (sobj == NULL)
	    { // dataobject/segment not found
	      hist_data = dbev->get_hist_data (mlist1, Histable::DOBJECT, 0, Hist_data::DETAIL);
	      if (!dbeSession->find_obj (dis_file, inp_file, sobj, dobj_name,
					 sel, Histable::DOBJECT, (inp_file != stdin)))
		return;
	      if (sobj == NULL)
		{ // dataobject/segment not found
		  fprintf (stderr, GTXT ("Error: No dataobject with given name `%s' found.\n"),
			   dobj_name);
		  return;
		}
	    }

	  list_limit = 1;
	}
      if (!hist_data)
	hist_data = dbev->get_hist_data (mlist1, Histable::DOBJECT, 0, Hist_data::DETAIL);
      break;
    case MODE_ANNOTATED:
      hist_data = dbev->get_hist_data (mlist1, Histable::DOBJECT, 0, Hist_data::LAYOUT);
      break;
    default: // MODE_GPROF is not relevant for DataObjects
      abort ();
    }

  if (hist_data->get_status () != Hist_data::SUCCESS)
    {
      // XXXX is this error message adequate?
      errstr = DbeView::status_str (DbeView::DBEVIEW_NO_DATA);
      if (errstr)
	{
	  fprintf (stderr, GTXT ("Error: %s\n"), errstr);
	  free (errstr);
	}
      delete hist_data;
      return;
    }
  cd = (er_print_common_display *) new er_print_histogram (dbev, hist_data,
							   hist_data->get_metric_list (), mode, list_limit, name, sobj, false, false);
  free (name);
  print_cmd (cd);

  delete hist_data;
  delete cd;
}

void
er_print::print_func (Histable::Type type, Print_mode mode, MetricList *mlist1,
		      MetricList *mlist2, char *func_name, char *sel)
{
  Hist_data *hist_data;
  Hist_data::HistItem *hitem;
  int index;
  char *errstr;
  int list_limit = limit;
  Histable *sobj = NULL;
  MetricList *mlist;
  StringBuilder sb;
  char *sname = dbev->getSort (MET_NORMAL);
  sb.append (sname);
  free (sname);

  switch (mode)
    {
    case MODE_DETAIL:
      {
	// The first metric list, mlist1, is only used to pick out the sort
	//    mlist2 is the one used to generate the data
	char *prevsort = NULL;
	// if specified, find the function from the function name
	if (func_name && strcmp (func_name, NTXT ("<All>")))
	  {
	    if ((!dbeSession->find_obj (dis_file, inp_file, sobj, func_name,
					sel, Histable::FUNCTION, (inp_file != stdin)) || (sobj == NULL)) &&
		!dbeSession->find_obj (dis_file, inp_file, sobj, func_name,
				       sel, Histable::LOADOBJECT, (inp_file != stdin)))
	      return;
	    if (sobj == NULL)
	      { // function/segment object not found
		fprintf (stderr, GTXT ("Error: No function with given name `%s' found.\n"),
			 func_name);
		return;
	      }
	    list_limit = 1;
	  }
	else
	  {
	    // find the sort metric from the reference list
	    prevsort = mlist2->get_sort_cmd ();

	    // find the current sort metric from the current list
	    char *cursort = mlist1->get_sort_cmd ();

	    // find the corresponding metric in the reference list
	    (void) mlist2->set_sort (cursort, false);
	    free (cursort);
	    // if it fails, nothing is needed
	  }
	hist_data = dbev->get_hist_data (mlist2, type, 0, Hist_data::ALL);

	// restore
	if (sobj == NULL)
	  {
	    if (prevsort == NULL)
	      abort ();
	    (void) mlist2->set_sort (prevsort, false);
	  }
	mlist = mlist2;
	free (prevsort);
	break;
      }
    case MODE_GPROF:
      // if specified, find the function from the function name
      if (func_name && strcmp (func_name, NTXT ("<All>")))
	{
	  if (!dbeSession->find_obj (dis_file, inp_file, sobj, func_name,
				     sel, Histable::FUNCTION, (inp_file != stdin)))
	    return;
	  if (sobj == NULL)
	    { // function/segment object not found
	      fprintf (stderr, GTXT ("Error: No function with given name `%s' found.\n"),
		       func_name);
	      return;
	    }
	  list_limit = 1;
	  sb.setLength (0);
	}
      sb.append (GTXT ("\nCallers and callees sorted by metric: "));
      sname = dbev->getSort (MET_CALL);
      sb.append (sname);
      free (sname);

      // Use mlist2 to generate the sort order.
      // mlist1 is used to generate the data.
      hist_data = dbev->get_hist_data (mlist2, type, 0, Hist_data::ALL);
      mlist = mlist1;
      break;
    default:
      hist_data = dbev->get_hist_data (mlist1, type, 0, Hist_data::ALL);
      mlist = mlist1;
    }

  if (hist_data->get_status () != Hist_data::SUCCESS)
    {
      errstr = DbeView::status_str (DbeView::DBEVIEW_NO_DATA);
      if (errstr)
	{
	  fprintf (stderr, GTXT ("Error: %s\n"), errstr);
	  free (errstr);
	}
      delete hist_data;
      return;
    }

  if (type == Histable::FUNCTION)
    {
      for (index = 0; index < hist_data->size (); index++)
	{
	  hitem = hist_data->fetch (index);
	  if (hitem->obj->get_type () == Histable::FUNCTION)
	    // fetch the name, since that will force a format conversion
	    ((Function *) hitem->obj)->get_name ();
	}
    }

  char *name = sb.toString ();
  er_print_histogram *cd = new er_print_histogram (dbev, hist_data,
						   mlist, mode, list_limit, name, sobj, false, false);
  print_cmd (cd);
  delete hist_data;
  free (name);
  delete cd;
}

void
er_print::print_gprof (CmdType cmd_type, char *func_name, char *sel)
{
  Histable *sobj = NULL;
  if (func_name != NULL)
    {
      if ((!dbeSession->find_obj (dis_file, inp_file, sobj, func_name,
				  sel, Histable::FUNCTION, (inp_file != stdin))
	   || sobj == NULL)
	  && !dbeSession->find_obj (dis_file, inp_file, sobj, func_name,
				    sel, Histable::LOADOBJECT, (inp_file != stdin)))
	return;
      if (sobj == NULL)
	{ // function/segment object not found
	  fprintf (stderr, GTXT ("Error: No function with given name `%s' found.\n"),
		   func_name);
	  return;
	}
    }
  if (cmd_type == CPREPEND)
    {
      if (sobj == NULL)
	{
	  fprintf (stderr, GTXT ("Error: No function name has been specified.\n"));
	  return;
	}
      cstack->insert (0, sobj);
    }
  else if (cmd_type == CAPPEND)
    {
      if (sobj == NULL)
	{
	  fprintf (stderr, GTXT ("Error: No function name has been specified.\n"));
	  return;
	}
      cstack->append (sobj);
    }
  else if (cmd_type == CSINGLE)
    {
      if (sobj != NULL)
	{
	  cstack->reset ();
	  cstack->append (sobj);
	}
      else if (cstack->size () == 0)
	{
	  fprintf (stderr, GTXT ("Error: No function name has been specified.\n"));
	  return;
	}
    }
  else if (cmd_type == CRMFIRST)
    {
      if (cstack->size () <= 1)
	{
	  fprintf (stderr, GTXT ("Warning: there is only one function in the stack segment; cannot remove it.\n"));
	  return;
	}
      cstack->remove (0);
    }
  else if (cmd_type == CRMLAST)
    {
      if (cstack->size () <= 1)
	{
	  fprintf (stderr, GTXT ("Warning: there is only one function in the stack segment; cannot remove it.\n"));
	  return;
	}
      cstack->remove (cstack->size () - 1);
    }

  er_print_gprof *cd = new er_print_gprof (dbev, cstack);
  print_cmd (cd);
  delete cd;
}

/*
 * Method print_ctree() prints Functions Call Tree.
 */
void
er_print::print_ctree (CmdType cmd_type)
{
  if (cmd_type != CALLTREE)
    {
      fprintf (stderr, GTXT ("Error: Invalid command type: %d\n"), cmd_type);
      return;
    }

  Histable *sobj = dbeSession->get_Total_Function ();
  Vector<Histable*> *ctree_cstack = new Vector<Histable*>();
  ctree_cstack->reset ();
  er_print_ctree *cd = new er_print_ctree (dbev, ctree_cstack, sobj, limit);
  print_cmd (cd);
  delete ctree_cstack;
  delete cd;
}

void
er_print::memobj (char *name, int cparam)
{
  int type;
  if (name != NULL)
    {
      // find the memory object index for the name
      MemObjType_t *mot = MemorySpace::findMemSpaceByName (name);
      if (mot == NULL)
	{
	  // unknown type, report the error
	  fprintf (stderr, GTXT ("Error: Unknown Memory Object type: %s\n"), name);
	  return;
	}
      type = mot->type;
    }
  else
    {
      MemObjType_t *mot = MemorySpace::findMemSpaceByIndex (cparam);
      if (mot == NULL)
	{
	  // unknown type, report the error
	  fprintf (stderr, GTXT ("Error: Unknown Memory Object type: %s\n"), name);
	  return;
	}
      type = cparam;
    }
  dbePrintData (0, DSP_MEMOBJ, type, NULL, NULL, out_file);
}

void
er_print::mo_define (char *moname, char *mo_index_exp, char *machmodel, char *short_desc, char *long_desc)
{
  char *ret = MemorySpace::mobj_define (moname, mo_index_exp, machmodel, short_desc, long_desc);
  if (ret != NULL)
    fprintf (stderr, GTXT ("mobj_define for %s failed: %s\n"), moname, ret);
}

void
er_print::mo_list (bool showtab, FILE *outf)
{
  Vector<bool> *mtab = NULL;
  Vector<void*>*res = MemorySpace::getMemObjects ();
  if (showtab)
    mtab = dbev->get_MemTabState ();
  if (res == NULL)
    // Since we checked already, this is an internal error
    abort ();

  // unpack the return
  // Vector<char*> *index = (Vector<int> *)res->fetch(0);  // not used
  Vector<char*> *mo_names = (Vector<char*> *)res->fetch (1);
  // Vector<char*> *mnemonic = (Vector<char> *)res->fetch(2);  // not used
  Vector<char*> *mo_expr = (Vector<char*> *)res->fetch (3);
  Vector<char*> *mo_mach_m = (Vector<char*> *)res->fetch (4);
  // Vector<char*> *tmpOrder = (Vector<int> *)res->fetch(5);  // not used

  int size = mo_names->size ();
  if (size == 0)
    {
      if (!getenv ("_BUILDING_MANPAGE"))
	fprintf (outf, GTXT (" No Memory Object Types Defined\n"));
    }
  else
    {
      if (!getenv ("_BUILDING_MANPAGE"))
	fprintf (outf, GTXT (" Memory Object Types Available:\n"));
      else
	fprintf (outf, GTXT ("*Memory Object Types*\n"));
      for (int i = 0; i < size; i++)
	{
	  if (mtab)
	    fprintf (outf, NTXT ("  %c %s\n"), mtab->fetch (i) ? 'T' : 'F',
		     mo_names->fetch (i));
	  else
	    {
	      if (mo_mach_m->fetch (i) != NULL)
		fprintf (outf, NTXT ("  %s\t\t\"%s\"\t\t(machinemodel: %s)\n"),
			 mo_names->fetch (i), mo_expr->fetch (i), mo_mach_m->fetch (i));
	      else
		fprintf (outf, NTXT ("  %s\t\t\"%s\"\n"),
			 mo_names->fetch (i), mo_expr->fetch (i));
	    }
	}
    }
  delete mo_names;
  delete mo_expr;
  delete mo_mach_m;
  delete res;
}

void
er_print::indxobj (char *name, int cparam)
{
  int type;
  if (name != NULL)
    {
      // find the index object index for the name
      type = dbeSession->findIndexSpaceByName (name);
      if (type < 0)
	{
	  // unknown type, report the error
	  fprintf (stderr, GTXT ("Error: Unknown Index Object type: %s\n"), name);
	  return;
	}
    }
  else
    {
      char *indxname = dbeSession->getIndexSpaceName (cparam);
      if (indxname == NULL)
	{
	  // unknown type, report the error
	  fprintf (stderr, GTXT ("Error: Unknown Index Object type: %d\n"), cparam);
	  return;
	}
      type = cparam;
    }
  dbePrintData (0, DSP_INDXOBJ, type, NULL, NULL, out_file);
}

void
er_print::indxo_define (char *ioname, char *io_index_exp, char *sdesc, char *ldesc)
{
  char *ret = dbeDefineIndxObj (ioname, io_index_exp, sdesc, ldesc);
  if (ret != NULL)
    fprintf (stderr, GTXT ("indxobj_define for %s failed: %s\n"), ioname, ret);
}

void
er_print::indxo_list (bool showtab, FILE *outf)
{
  Vector<bool> *indxtab = NULL;
  char *name;
  char *i18n_name;
  if (!getenv ("_BUILDING_MANPAGE"))
    fprintf (outf, GTXT (" Index Object Types Available:\n"));
  else
    fprintf (outf, GTXT ("*Index Object Types*\n"));
  Vector<void*>*res = dbeGetIndxObjDescriptions (0);
  if (showtab)
    indxtab = dbev->get_IndxTabState ();
  if (res == NULL)  // If none is defined
    return;
  Vector<char*> *indxo_names = (Vector<char*> *)res->fetch (1);
  Vector<char*> *indxo_i18nnames = (Vector<char*> *)res->fetch (3);
  Vector<char*> *indxo_exprlist = (Vector<char*> *)res->fetch (5);
  int size = indxo_names->size ();
  for (int i = 0; i < size; i++)
    {
      name = indxo_names->fetch (i);
      i18n_name = indxo_i18nnames->fetch (i);
      if (indxtab)
	{
	  if ((i18n_name != NULL) && (strcmp (i18n_name, name) != 0))
	    fprintf (outf, NTXT ("  %c %s (%s)\n"), indxtab->fetch (i) ? 'T' : 'F',
		     i18n_name, name);
	  else
	    fprintf (outf, NTXT ("  %c %s\n"), indxtab->fetch (i) ? 'T' : 'F', name);
	}
      else
	{
	  if (i18n_name != NULL && strcmp (i18n_name, indxo_names->fetch (i)) != 0)
	    fprintf (outf, NTXT ("  %s (%s)"), i18n_name, name);
	  else
	    fprintf (outf, NTXT ("  %s"), name);
	}
      char *exprs = indxo_exprlist->fetch (i);
      if (exprs != NULL)
	fprintf (outf, NTXT (" \t%s\n"), exprs);
      else
	fprintf (outf, NTXT ("\n"));
    }
  delete indxo_names;
  if (showtab)
    delete res;
}

void
er_print::ifreq ()
{
  dbev->ifreq (out_file);
}

void
er_print::dump_nodes ()
{
  dbev->dump_nodes (out_file);
}

void
er_print::dump_stacks ()
{
  dbeSession->dump_stacks (out_file);
}

void
er_print::dump_unk_pcs ()
{
  // Dump the nodes associated with the <Unknown> function
  dbev->get_path_tree ()->dumpNodes (out_file, dbeSession->get_Unknown_Function ());

  // Dump the nodes associated with the <no Java callstack recorded> function
  Vector<Function *> *matches = dbeSession->match_func_names ("<no Java callstack recorded>", dbev->get_name_format ());
  if (matches == NULL || matches->size () == 0)
    fprintf (out_file, GTXT ("No %s functions found\n"), "<no Java callstack recorded>");
  else
    {
      Function *fitem;
      int index;
      Vec_loop (Function*, matches, index, fitem)
      {
	dbev->get_path_tree ()->dumpNodes (out_file, fitem);
      }
      delete matches;
    }
}

void
er_print::dump_funcs (char *arg1)
{
  if (arg1 == NULL || strlen (arg1) == 0)
    dbeSession->dump_segments (out_file);
  else
    {
      Vector<Function *> *matches = dbeSession->match_func_names (arg1, dbev->get_name_format ());
      if (matches == NULL)
	{
	  fprintf (stderr, GTXT ("Invalid argument `%s' -- not a regular expression\n"), arg1);
	  return;
	}
      fprintf (out_file, GTXT ("%d Function's match `%s'\n"), (int) matches->size (), arg1);
      Function *fitem;
      int index;
      Vec_loop (Function*, matches, index, fitem)
      {
	fprintf (out_file, NTXT (" %5lld -- %s (%s) [%s]\n"),
		 (ll_t) fitem->id, fitem->get_name (),
		 (fitem->module ? fitem->module->file_name : NTXT ("<unknown>")),
		 ((fitem->module && fitem->module->loadobject) ?
		  get_basename (fitem->module->loadobject->get_name ()) : NTXT ("<unknown>")));
      }
      delete matches;
    }
}

void
er_print::dump_dataobjects (char *arg1)
{
  // Force computation of data objects, to update master table; discard it
  MetricList *mlist1 = dbev->get_metric_list (MET_DATA);
  Hist_data *data = dbev->get_hist_data (mlist1, Histable::DOBJECT, 0, Hist_data::ALL);
  delete data;

  if (arg1 == NULL || strlen (arg1) == 0)
    dbeSession->dump_dataobjects (out_file);
  else
    {
      Vector<DataObject *> *matches = dbeSession->match_dobj_names (arg1);
      if (matches == NULL)
	{
	  fprintf (stderr, GTXT ("Invalid argument `%s' -- not a regular expression\n"), arg1);
	  return;
	}
      fprintf (out_file, GTXT ("%d DataObject's match `%s'\n"), (int) matches->size (), arg1);
      DataObject *ditem;
      int index;
      Vec_loop (DataObject*, matches, index, ditem)
      {
	fprintf (out_file, NTXT (" %5lld -- %s\n"), (ll_t) ditem->id, ditem->get_name ());
      }
      delete matches;
    }
}

void
er_print::dump_map ()
{
  dbeSession->dump_map (out_file);
}

void
er_print::dump_entities ()
{
  int ent_prop_ids[] = {PROP_THRID, PROP_LWPID, PROP_CPUID, PROP_EXPID, -1};

  // loop over experiments
  for (int exp_id = 0; exp_id < dbeSession->nexps (); exp_id++)
    {
      Experiment *exp = dbeSession->get_exp (exp_id);
      fprintf (out_file, GTXT ("Experiment %d (%s)\n"),
	       exp_id, exp->get_expt_name ());

      for (int kk = 0; ent_prop_ids[kk] != -1; kk++)
	{
	  int ent_prop_id = ent_prop_ids[kk];
	  Vector<void*> *elist = dbeGetEntities (0, exp_id, ent_prop_id);
	  if (!elist)
	    continue;
	  Vector<int> *entity_vals = (Vector<int> *) elist->fetch (0);
	  Vector<char*> *jthr_names = (Vector<char*> *)elist->fetch (1);
	  Vector<char*> *jthr_g_names = (Vector<char*> *)elist->fetch (2);
	  Vector<char*> *jthr_p_names = (Vector<char*> *)elist->fetch (3);
	  Vector<char*> *entity_name = (Vector<char*> *)elist->fetch (4);
	  int nent = entity_vals->size ();
	  char *entName = entity_name->fetch (0);
	  if (!entName)
	    entName = NTXT ("<unknown>");
	  fprintf (out_file, GTXT ("  %s\n"), entName);
	  for (int i = 0; i < nent; i++)
	      fprintf (out_file, GTXT ("    %s=%d: %s, %s, %s\n"),
		       entName, entity_vals->fetch (i),
		       jthr_names->fetch (i) != NULL ? jthr_names->fetch (i) : NTXT ("N/A"),
		       jthr_g_names->fetch (i) != NULL ? jthr_g_names->fetch (i) : NTXT ("N/A"),
		       jthr_p_names->fetch (i) != NULL ? jthr_names->fetch (i) : NTXT ("N/A"));
	  destroy (elist);
	}
    }
}

void
er_print::dump_stats ()
{
  Emsg *m = dbev->get_path_tree ()->fetch_stats ();
  while (m != NULL)
    {
      fprintf (out_file, NTXT ("%s\n"), m->get_msg ());
      m = m->next;
    }
  dbev->get_path_tree ()->delete_stats ();
}

void
er_print::dump_proc_warnings ()
{
  PathTree *p = dbev->get_path_tree ();
  if (p == NULL)
    return;
  Emsg *m = p->fetch_warnings ();
  while (m != NULL)
    {
      fprintf (out_file, NTXT ("%s\n"), m->get_msg ());
      m = m->next;
    }
  dbev->get_path_tree ()->delete_warnings ();
}

void
er_print::print_cmd (er_print_common_display *cd)
{
  cd->set_out_file (out_file);
  cd->data_dump ();
}

FILE *
er_print::set_outfile (char *cmd, FILE *&set_file, bool append)
{
  FILE *new_file;
  char *home;
  if (!strcasecmp (cmd, NTXT ("-")))
    {
      new_file = stdout;
      out_fname = NTXT ("<stdout>");
    }
  else if (!strcasecmp (cmd, NTXT ("--")))
    {
      new_file = stderr;
      out_fname = NTXT ("<stderr>");
    }
  else
    {
      char *fname;
      char *path = NULL;
      // Handle ~ in file names
      home = getenv (NTXT ("HOME"));
      if ((fname = strstr (cmd, NTXT ("~/"))) != NULL && home != NULL)
	path = dbe_sprintf (NTXT ("%s/%s"), home, fname + 2);
      else if ((fname = strstr (cmd, NTXT ("~"))) != NULL && home != NULL)
	path = dbe_sprintf (NTXT ("/home/%s"), fname + 1);
      else
	path = strdup (cmd);
      new_file = fopen (path, append ? NTXT ("a") : NTXT ("w"));
      if (new_file == NULL)
	{
	  fprintf (stderr, GTXT ("Error: Unable to open file: %s\n"), cmd);
	  free (path);
	  return NULL;
	}
      out_fname = path;
    }
  if (set_file && set_file != stdout)
    fclose (set_file);
  set_file = new_file;
  return set_file;
}
