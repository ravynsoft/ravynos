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
#include <unistd.h>
#include <fcntl.h>

#include "util.h"
#include "DbeApplication.h"
#include "DbeSession.h"
#include "Function.h"
#include "LoadObject.h"
#include "Module.h"
#include "DbeView.h"
#include "Print.h"
#include "DbeFile.h"
#include "Command.h"

class er_src : public DbeApplication
{
public:
  er_src (int argc, char *argv[]);
  void start (int argc, char *argv[]);

private:

  // override methods in base class
  void usage ();
  int check_args (int argc, char *argv[]);
  void run_args (int argc, char *argv[]);

  enum Obj_Types
  {
    OT_EXE_ELF = 0, OT_JAVA_CLASS, OT_JAR_FILE, OT_UNKNOWN
  };

  void open (char *exe);
  void dump_annotated (char *name, char* sel, char *src, DbeView *dbev,
		       bool is_dis, bool first);
  void checkJavaClass (char *exe);
  void print_header (bool first, const char* text);
  void proc_cmd (CmdType cmd_type, bool first, char *arg1,
		 const char *arg2, const char *arg3 = NULL);
  FILE *set_outfile (char *cmd, FILE *&set_file);

  bool is_java_class ()     { return obj_type == OT_JAVA_CLASS; }

  int dbevindex;
  DbeView *dbev;
  LoadObject *lo;
  Obj_Types obj_type;
  const char *out_fname;
  FILE *out_file;
  bool isDisasm;
  bool isFuncs;
  bool isDFuncs;
  bool isSrc;
  bool v_opt;
  int multiple;
  char *str_compcom;
  bool hex_visible;
  int src_visible;
  int vis_src;
  int vis_dis;
  int threshold_src;
  int threshold_dis;
  int threshold;
  int vis_bits;
};

static int
real_main (int argc, char *argv[])
{
  er_src *src = new er_src (argc, argv);
  src->start (argc, argv);
  delete src;
  return 0;
}

int
main (int argc, char *argv[])
{
  return catch_out_of_memory (real_main, argc, argv);
}

er_src::er_src (int argc, char *argv[])
: DbeApplication (argc, argv)
{
  obj_type = OT_UNKNOWN;
  out_fname = "<stdout>";
  out_file = stdout;
  isDisasm = false;
  isFuncs = false;
  isDFuncs = false;
  isSrc = false;
  v_opt = false;
  multiple = 0;
  lo = NULL;
}

static int
FuncNameCmp (const void *a, const void *b)
{
  Function *item1 = *((Function **) a);
  Function *item2 = *((Function **) b);
  return strcmp (item1->get_mangled_name (), item2->get_mangled_name ());
}

static int
FuncAddrCmp (const void *a, const void *b)
{
  Function *item1 = *((Function **) a);
  Function *item2 = *((Function **) b);
  return (item1->img_offset == item2->img_offset) ?
	  FuncNameCmp (a, b) : item1->img_offset > item2->img_offset ? 1 : -1;
}

void
er_src::usage ()
{

/*
  Ruud - Isolate this line because it has an argument.  Otherwise it would be at the
  end of a long usage list.
*/
  printf ( GTXT (
    "Usage: gprofng display src [OPTION(S)] TARGET-OBJECT\n"));

  printf ( GTXT (
    "\n"
    "Display the source code listing, or source code interleaved with disassembly code,\n"
    "as extracted from the target object (an executable, shared object, object file, or\n"
    "a Java .class file).\n"
    "\n"
    "Options:\n"
    "\n"
    " --version           print the version number and exit.\n"
    " --help              print usage information and exit.\n"
    " --verbose {on|off}  enable (on) or disable (off) verbose mode; the default is \"off\".\n"
    "\n"
    " -func                   list all the functions from the given object.\n"
    "\n"
    " -source item tag    show the source code for item; the tag is used to\n"
    "                     differentiate in case of multiple occurences with\n"
    "                     the same name; the combination of \"all -1\" selects\n"
    "                     all the functions in the object; the default is\n"
    "                     \"-source all -1\".\n"
    "\n"
    " -disasm item tag        show the source code, interleaved with the disassembled\n"
    "                         instructions; the same definitions for item and tag apply.\n"
    "\n"
    " -outfile <filename>     write results to file <filename>; a dash (-) writes to\n"
    "                         stdout; this is also the default; note that this only\n"
    "                         affects options included to the right of this option.\n"
    "\n"
   "Documentation:\n"
    "\n"
    "A getting started guide for gprofng is maintained as a Texinfo manual. If the info and\n"
    "gprofng programs are properly installed at your site, the command \"info gprofng\"\n"
    "should give you access to this document.\n"
    "\n"
    "See also:\n"
    "\n"
    "gprofng(1), gp-archive(1), gp-collect-app(1), gp-display-html(1), gp-display-text(1)\n"));
/*
  printf (GTXT ("Usage: %s [OPTION] a.out/.so/.o/.class\n\n"), whoami);
  printf (GTXT ("    -func                     List all the functions from the given object\n"
		"    -source, -src item tag    Show the annotated source for the listed item\n"
		"    -disasm item tag          Include the disassembly in the listing\n"
		"    -V                        Print the current release version of er_src\n"
		"    -cc, -scc, -dcc com_spec  Define the compiler commentary classes to show\n"
		"    -outfile filename         Open filename for output\n"));
*/
  exit (0);
}

void
er_src::start (int argc, char *argv[])
{
  dbevindex = dbeSession->createView (0, -1);
  dbev = dbeSession->getView (dbevindex);

  // get options
  check_args (argc, argv);
  run_args (argc, argv);
  if (out_file != stdout)
    fclose (out_file);
}

FILE *
er_src::set_outfile (char *cmd, FILE *&set_file)
{
  FILE *new_file;
  if (!strcasecmp (cmd, "-"))
    {
      new_file = stdout;
      out_fname = "<stdout>";
    }
  else
    {
      char *cmdpath;
      char *fname = strstr (cmd, "~/");
      // Handle ~ in file names
      char *home = getenv ("HOME");
      if (fname != NULL && home != NULL)
	cmdpath = dbe_sprintf ("%s/%s", home, fname + 2);
      else if ((fname = strstr (cmd, "~")) != NULL && home != NULL)
	cmdpath = dbe_sprintf ("/home/%s", fname + 1);
      else
	cmdpath = strdup (cmd);
      new_file = fopen (cmdpath, "w");
      if (new_file == NULL)
	{
	  fprintf (stderr, GTXT ("Unable to open file: %s"), cmdpath);
	  free (cmdpath);
	  return NULL;
	}
      out_fname = cmdpath;
    }
  if (set_file && (set_file != stdout))
    fclose (set_file);

  set_file = new_file;
  return set_file;
}

void
er_src::proc_cmd (CmdType cmd_type, bool first, char *arg1,
		  const char *arg2, const char *arg3)
{
  Cmd_status status;
  Module *module;
  Function *fitem;
  int mindex, findex;
  switch (cmd_type)
    {
    case SOURCE:
      dbev->set_view_mode (VMODE_USER);
      print_anno_file (arg1, arg2, arg3, false,
		       stdout, stdin, out_file, dbev, false);
      break;
    case DISASM:
      dbev->set_view_mode (VMODE_MACHINE);
      print_header (first, GTXT ("Annotated disassembly\n"));
      print_anno_file (arg1, arg2, arg3, true,
		       stdout, stdin, out_file, dbev, false);
      break;
    case OUTFILE:
      if (arg1)
	set_outfile (arg1, out_file);
      break;
    case FUNCS:
      print_header (false, GTXT ("Function list\n"));
      fprintf (out_file, GTXT ("Functions sorted in lexicographic order\n"));
      fprintf (out_file, GTXT ("\nLoad Object: %s\n\n"), lo->get_name ());
      if (lo->wsize == W32)
	fprintf (out_file, GTXT ("    Address     Size        Name\n\n"));
      else
	fprintf (out_file, GTXT ("    Address                     Size        Name\n\n"));

      Vec_loop (Module*, lo->seg_modules, mindex, module)
      {
	module->functions->sort (FuncNameCmp);
	const char *fmt = (lo->wsize == W32) ?
		GTXT ("  0x%08llx  %8lld      %s\n") :
		GTXT ("  0x%016llx  %16lld      %s\n");
	Vec_loop (Function*, module->functions, findex, fitem)
	{
	  fprintf (out_file, fmt,
		   (ull_t) fitem->img_offset,
		   (ull_t) fitem->size,
		   fitem->get_name ());
	}
      }
      break;
    case DUMPFUNC:
      lo->functions->sort (FuncAddrCmp);
      print_header (first, GTXT ("Dump functions\n"));
      lo->dump_functions (out_file);
      first = false;
      break;
    case SCOMPCOM:
      status = dbev->proc_compcom (arg1, true, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s"), Command::get_err_string (status));
      break;
    case DCOMPCOM:
      status = dbev->proc_compcom (arg1, false, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s"), Command::get_err_string (status));
      break;
    case COMPCOM:
      status = dbev->proc_compcom (arg1, true, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s: %s"), Command::get_err_string (status), arg1);
      status = dbev->proc_compcom (arg1, false, false);
      if (status != CMD_OK)
	fprintf (stderr, GTXT ("Error: %s: %s"), Command::get_err_string (status), arg1);
      break;
    case HELP:
      usage ();
      break;
    case VERSION_cmd:
      if (out_file != stdout)
// Ruud
	Application::print_version_info ();
/*
	fprintf (out_file, "GNU %s version %s\n", get_basename (prog_name), VERSION);
*/
      break;
    default:
      fprintf (stderr, GTXT ("Invalid option"));
      break;
    }
}

void
er_src::run_args (int argc, char *argv[])
{
  CmdType cmd_type;
  int arg_count, cparam;
  char *arg;
  char *arg1;
  const char *arg2;
  bool first = true;
  bool space;
  Module *module;
  int mindex;

  for (int i = 1; i < argc; i++)
    {
      if (*argv[i] != '-')
	{
	  if (!multiple)
	    { // er_src -V exe
	      space = false;
	      dbev->set_view_mode (VMODE_USER);
	      print_header (first, GTXT ("Annotated source\n"));
	      Vec_loop (Module*, lo->seg_modules, mindex, module)
	      {
		if ((module->flags & MOD_FLAG_UNKNOWN) != 0 ||
		    module->lang_code == Sp_lang_unknown)
		  continue;
		if (space)
		  fprintf (out_file, "\n");
		print_anno_file (module->file_name, "1", NULL, false,
				 stdout, stdin, out_file, dbev, false);
		space = true;
	      }
	    }
	  break;
	}
      if (strncmp (argv[i], NTXT ("--whoami="), 9) == 0)
	{
	  whoami = argv[i] + 9;
	  continue;
	}
      switch (cmd_type = Command::get_command (argv[i] + 1, arg_count, cparam))
	{
	case SOURCE:
	case DISASM:
	  {
	    i += arg_count;
	    multiple++;
	    if (i >= argc || argv[i] == NULL ||
		(*(argv[i]) == '-' && atoi (argv[i]) != -1) || i + 1 == argc)
	      {
		i--;
		arg = argv[i];
		arg2 = "1";
	      }
	    else
	      {
		arg = argv[i - 1];
		if (*(argv[i]) == '-' && atoi (argv[i]) == -1 &&
		    streq (arg, NTXT ("all")))
		  {
		    space = false;
		    if (cmd_type == SOURCE)
		      print_header (first, GTXT ("Annotated source\n"));
		    else
		      print_header (first, GTXT ("Annotated disassembly\n"));
		    Vec_loop (Module*, lo->seg_modules, mindex, module)
		    {
		      if ((module->flags & MOD_FLAG_UNKNOWN) != 0 ||
			  module->lang_code == Sp_lang_unknown)
			continue;
		      if (space)
			fprintf (out_file, "\n");
		      proc_cmd (cmd_type, first, module->file_name, "1");
		      space = true;
		    }
		    first = false;
		    break;
		  }
		arg2 = argv[i];
	      }
	    char *fcontext = NULL;
	    arg1 = parse_fname (arg, &fcontext);
	    if (arg1 == NULL)
	      {
		fprintf (stderr, GTXT ("Error: Invalid function/file setting: %s\n"), arg1);
		free (fcontext);
		break;
	      }
	    proc_cmd (cmd_type, first, arg1, arg2, fcontext);
	    free (arg1);
	    free (fcontext);
	    first = false;
	    break;
	  }
	case OUTFILE:
	case FUNCS:
	case DUMPFUNC:
	case COMPCOM:
	case SCOMPCOM:
	case DCOMPCOM:
	case VERSION_cmd:
	case HELP:
	  proc_cmd (cmd_type, first, (arg_count > 0) ? argv[i + 1] : NULL,
		    (arg_count > 1) ? argv[i + 2] : NULL);
	  i += arg_count;
	  break;
	default:
	  if (streq (argv[i] + 1, NTXT ("all")) || streq (argv[i] + 1, NTXT ("dall")))
	    {
	      first = false;
	      multiple++;
	      if (streq (argv[i] + 1, NTXT ("all")))
		proc_cmd (FUNCS, first, NULL, NULL);
	      else
		proc_cmd (DUMPFUNC, first, NULL, NULL);
	      space = false;
	      print_header (first, GTXT ("Annotated source\n"));
	      Vec_loop (Module*, lo->seg_modules, mindex, module)
	      {
		if ((module->flags & MOD_FLAG_UNKNOWN) != 0 ||
		    module->lang_code == Sp_lang_unknown)
		  continue;
		if (space)
		  fprintf (out_file, "\n");
		proc_cmd (SOURCE, first, module->file_name, "1");
		space = true;
	      }
	      print_header (first, GTXT ("Annotated disassembly\n"));
	      Vec_loop (Module*, lo->seg_modules, mindex, module)
	      {
		if ((module->flags & MOD_FLAG_UNKNOWN) != 0 ||
		    module->lang_code == Sp_lang_unknown)
		  continue;
		if (space)
		  fprintf (out_file, "\n");
		proc_cmd (DISASM, first, module->file_name, "1");
		space = true;
	      }
	    }
	  else
	    {
	      proc_cmd (cmd_type, first, (arg_count > 0) ? argv[i + 1] : NULL,
			(arg_count > 1) ? argv[i + 2] : NULL);
	      i += arg_count;
	      break;
	    }
	}
    }
}

int
er_src::check_args (int argc, char *argv[])
{
  CmdType cmd_type = UNKNOWN_CMD;
  int arg_count, cparam;
  int i;
  char *exe;
  bool first = true;
  if (argc == 1)
    usage ();

  // If any comments from the .rc files, log them to stderr
  Emsg * rcmsg = fetch_comments ();
  while (rcmsg != NULL)
    {
      fprintf (stderr, "%s: %s\n", prog_name, rcmsg->get_msg ());
      rcmsg = rcmsg->next;
    }

  // Parsing the command line
  opterr = 0;
  exe = NULL;
  for (i = 1; i < argc; i++)
    {
      if (*argv[i] != '-')
	{
	  exe = argv[i];
	  if (i == 1)
	    { // er_src exe ?
	      if (!exe)
		usage ();
	      if (argc == 3) // er_src exe file
		usage ();
	    }
	  else if (v_opt && !multiple && !exe && !str_compcom) // just er_src -V
	      exit (0);
	  if (argc < i + 1 || argc > i + 3)
	    usage ();
	  i++;
	  if (argc > i)
	    usage ();
	  open (exe);
	  return i;
	}
      switch (cmd_type = Command::get_command (argv[i] + 1, arg_count, cparam))
	{
	case WHOAMI:
	  whoami = argv[i] + 1 + cparam;
	  break;
	case HELP:
	  i += arg_count;
	  multiple++;
	  usage ();
	  break;
	case VERSION_cmd:
	  if (first)
	    {
// Ruud
	      Application::print_version_info ();
/*
	      printf ("GNU %s version %s\n", get_basename (prog_name), VERSION);
*/
	      v_opt = true;
	      first = false;
	    }
	  break;
	case SOURCE:
	case DISASM:
	  i += arg_count;
	  multiple++;
	  isDisasm = true;
	  if (i >= argc || argv[i] == NULL ||
	      (*(argv[i]) == '-' && atoi (argv[i]) != -1) || (i + 1 == argc))
	    i--;
	  break;
	case DUMPFUNC:
	  i += arg_count;
	  multiple++;
	  break;
	case FUNCS:
	  i += arg_count;
	  multiple++;
	  break;
	case OUTFILE:
	case COMPCOM:
	case SCOMPCOM:
	case DCOMPCOM:
	  i += arg_count;
	  break;
	default:
	  if (!(streq (argv[i] + 1, NTXT ("all")) ||
		streq (argv[i] + 1, NTXT ("dall"))))
	    {
	      fprintf (stderr, "Error: invalid option: `%s'\n", argv[i]);
	      exit (1);
	    }
	}
    }
  if (!exe && !(argc == 2 && cmd_type == VERSION_cmd))
    usage ();
  return i;
}

void
er_src::checkJavaClass (char* exe)
{
  unsigned char cf_buf[4];
  unsigned int magic_number;
  int fd = ::open (exe, O_RDONLY | O_LARGEFILE);
  if (fd == -1)
    return;
  if (sizeof (cf_buf) == read_from_file (fd, cf_buf, sizeof (cf_buf)))
    {
      magic_number = cf_buf[0] << 24;
      magic_number |= cf_buf[1] << 16;
      magic_number |= cf_buf[2] << 8;
      magic_number |= cf_buf[3];
      if (magic_number == 0xcafebabe)
	obj_type = OT_JAVA_CLASS;
    }
  close (fd);
}

void
er_src::print_header (bool first, const char* text)
{
  if (!first)
    fprintf (out_file, "\n");
  if (multiple > 1)
    {
      fprintf (out_file, NTXT ("%s"), text);
      fprintf (out_file, "---------------------------------------\n");
    }
}

void
er_src::dump_annotated (char *name, char *sel, char *src, DbeView *dbevr,
			bool is_dis, bool first)
{
  Module *module;
  bool space;
  int mindex;
  print_header (first, (is_dis) ? ((is_java_class ()) ?
				   GTXT ("Annotated bytecode\n") :
				   GTXT ("Annotated disassembly\n")) :
		GTXT ("Annotated source\n"));
  if (!name)
    {
      space = false;
      Vec_loop (Module*, lo->seg_modules, mindex, module)
      {
	if ((module->flags & MOD_FLAG_UNKNOWN) != 0 ||
	    (!is_dis && module->lang_code == Sp_lang_unknown))
	  continue;
	if (space)
	  fprintf (out_file, "\n");
	print_anno_file (module->file_name, sel, src, is_dis,
			 stdout, stdin, out_file, dbevr, false);
	space = true;
      }
    }
  else
    print_anno_file (name, sel, src, is_dis, stdout, stdin, out_file, dbevr, false);
}

static bool
isFatal (bool isDisasm, LoadObject::Arch_status status)
{
  if (isDisasm)
    {
      switch (status)
	{
	  // non-fatal errors for disassembly
	case LoadObject::ARCHIVE_BAD_STABS:
	case LoadObject::ARCHIVE_NO_STABS:
	  return false;
	default:
	  return true;
	}
    }
  return true;
}

void
er_src::open (char *exe)
{
  LoadObject::Arch_status status;
  char *errstr;
  Module *module;
  Vector<Histable*> *module_lst;

  // Construct the Segment structure
  char *path = strdup (exe);
  lo = dbeSession->createLoadObject (path);
  if (NULL == lo->dbeFile->find_file (lo->dbeFile->get_name ()))
    {
      fprintf (stderr, GTXT ("%s: Error: unable to open file %s\n"), prog_name, lo->dbeFile->get_name ());
      exit (1);
    }
  checkJavaClass (exe);

  if (is_java_class ())
    {
      lo->type = LoadObject::SEG_TEXT;
      lo->set_platform (Java, Wnone);
      lo->id = (uint64_t) - 1; // see AnalyzerSession::ask_which for details
      module = dbeSession->createClassFile (dbe_strdup (exe));
      module->loadobject = lo;
      lo->seg_modules->append (module);
      module->dbeFile->set_location (exe);
      if (module->readFile () != module->AE_OK)
	{
	  Emsg *emsg = module->get_error ();
	  if (emsg)
	    {
	      fprintf (stderr, GTXT ("%s: Error: %s\n"), prog_name, emsg->get_msg ());
	      return;
	    }
	  fprintf (stderr, GTXT ("%s: Error: Could not read class file `%s'\n"), prog_name, exe);
	  return;
	}
      status = lo->sync_read_stabs ();
      if (status != LoadObject::ARCHIVE_SUCCESS)
	{
	  if (status == LoadObject::ARCHIVE_ERR_OPEN)
	    {
	      fprintf (stderr, GTXT ("%s: Error: Could not read class file `%s'\n"), prog_name, exe);
	      return;
	    }
	  else
	    {
	      if (isDisasm)
		if (status == LoadObject::ARCHIVE_NO_STABS)
		  {
		    fprintf (stderr, GTXT ("%s: Error: `%s' is interface; disassembly annotation not available\n"), prog_name, exe);
		    return;
		  }
	    }
	}
    }
  else
    {
      status = lo->sync_read_stabs ();
      if (status != LoadObject::ARCHIVE_SUCCESS)
	{
	  errstr = lo->status_str (status);
	  if (errstr)
	    {
	      fprintf (stderr, "%s: %s\n", prog_name, errstr);
	      free (errstr);
	    }
	  if (isFatal (isDisasm, status))
	    return;
	}
      obj_type = OT_EXE_ELF;

      // if .o file, then set file as the exe name
      if (lo->is_relocatable ())
	{
	  // find the module, if we can
	  module_lst = new Vector<Histable*>;
	  module = dbeSession->map_NametoModule (path, module_lst, 0);
	  if (module == NULL)
	    // Create a module with the right name
	    module = dbeSession->createModule (lo, path);
	}
    }
}
