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
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#include "util.h"
#include "StringMap.h"
#include "LoadObject.h"
#include "DbeSession.h"
#include "DbeFile.h"
#include "SourceFile.h"
#include "Elf.h"
#include "gp-archive.h"
#include "ArchiveExp.h"
#include "Print.h"
#include "Module.h"

er_archive::er_archive (int argc, char *argv[]) : DbeApplication (argc, argv)
{
  force = 0;
  common_archive_dir = NULL;
  quiet = 0;
  descendant = 1;
  use_relative_path = 0;
  s_option = ARCH_EXE_ONLY;
  mask = NULL;
}

er_archive::~er_archive ()
{
  if (mask)
    {
      for (long i = 0, sz = mask->size (); i < sz; i++)
	{
	  regex_t *regex_desc = mask->get (i);
	  regfree (regex_desc);
	  delete regex_desc;
	}
      delete mask;
    }
  delete common_archive_dir;
}

int
er_archive::mask_is_on (const char *str)
{
  if (mask == NULL)
    return 1;
  for (long i = 0, sz = mask->size (); i < sz; i++)
    {
      regex_t *regex_desc = mask->get (i);
      if (regexec (regex_desc, str, 0, NULL, 0) == 0)
	return 1;
    }
  return 0;
}

void
er_archive::usage ()
{
/*
  fprintf (stderr, GTXT ("Usage: %s [-nqFV] [-a on|ldobjects|src|usedldobjects|usedsrc|off] [-m regexp] experiment\n"), whoami);
*/

/*
  Ruud - Isolate this line because it has an argument.  Otherwise it would be at the
  end of this long list.
*/
  printf ( GTXT (
    "Usage: gprofng archive [OPTION(S)] EXPERIMENT\n"));

  printf ( GTXT (
    "\n"
    "Archive the associated application binaries and source files in a gprofng\n"
    "experiment to make it self contained and portable.\n"
    "\n"
    "Options:\n"
    "\n"
    " --version           print the version number and exit.\n"
    " --help              print usage information and exit.\n"
    " --verbose {on|off}  enable (on) or disable (off) verbose mode; the default is \"off\".\n"
    "\n"
    " -a {off|on|ldobjects|src|usedldobjects|usedsrc}  specify archiving of binaries and other files;\n"
    "                    in addition to disable this feature (off), or enable archiving off all\n"
    "                    loadobjects and sources (on), the other options support a more\n"
    "                    refined selection. All of these options enable archiving, but the\n"
    "                    keyword controls what exactly is selected: all load objects (ldobjects),\n"
    "                    all source files (src), the loadobjects asscoiated with a program counter\n"
    "                    (usedldobjects), or the source files associated with a program counter\n"
    "                    (usedsrc); the default is \"-a ldobjects\".\n"
    "\n"
    " -n                 archive the named experiment only, not any of its descendants.\n"
    "\n"
    " -q                 do not write any warnings to stderr; messages are archived and\n"
    "                    can be retrieved later.\n"
    "\n"
    " -F                 force writing or rewriting of the archive; ignored with the -n\n"
    "                    or -m options, or if this is a subexperiment.\n"
    "\n"
    " -d <path>          specifies the location of a common archive; this is a directory that\n"
    "                    contains archived files.\n"
    "\n"
    " -m <regex>         archive only those source, object, and debug info files whose full\n"
    "                    path name matches the given POSIX compliant regular expression.\n"
    "\n"
    "Limitations:\n"
    "\n"
    "Default archiving does not occur in case the application profiled terminates prematurely,\n"
    "or if archiving is disabled when collecting the performance data. In such cases, this\n"
    "tool can be used to afterwards archive the information, but it has to run on the same \n"
    "system where the profiling data was recorded.\n"
    "\n"
    "Documentation:\n"
    "\n"
    "A getting started guide for gprofng is maintained as a Texinfo manual. If the info and\n"
    "gprofng programs are properly installed at your site, the command \"info gprofng\"\n"
    "should give you access to this document.\n"
    "\n"
    "See also:\n"
    "\n"
    "gprofng(1), gp-collect-app(1), gp-display-html(1), gp-display-src(1), gp-display-text(1)\n"));
// Ruud
/*
  fprintf (stderr, GTXT ("GNU %s version %s\n"), get_basename (prog_name), VERSION);
*/
  exit (1);
}

Vector <LoadObject*> *
er_archive::get_loadObjs ()
{
  Vector <LoadObject*> *objs = new Vector<LoadObject*>();
  Vector <LoadObject*> *loadObjs = dbeSession->get_text_segments ();
  if (s_option != ARCH_NOTHING)
    {
      for (long i = 0, sz = VecSize(loadObjs); i < sz; i++)
	{
	  LoadObject *lo = loadObjs->get (i);
	  if ((lo->flags & SEG_FLAG_DYNAMIC) != 0)
	    continue;
	  DbeFile *df = lo->dbeFile;
	  if (df && ((df->filetype & DbeFile::F_FICTION) != 0))
	    continue;
	  if (!lo->isUsed && ((s_option & (ARCH_USED_EXE_ONLY | ARCH_USED_SRC_ONLY)) != 0))
	    continue;
	  objs->append (lo);
	}
    }
  if (DEBUG_ARCHIVE)
    {
      Dprintf (DEBUG_ARCHIVE, NTXT ("get_text_segments(): %d\n"),
	       (int) (loadObjs ? loadObjs->size () : -1));
      for (long i = 0, sz = loadObjs ? loadObjs->size () : 0; i < sz; i++)
	{
	  LoadObject *lo = loadObjs->get (i);
	  Dprintf (DEBUG_ARCHIVE, NTXT ("%s:%d  [%2ld] %s\n"),
		   get_basename (__FILE__), (int) __LINE__, i, STR (lo->dump ()));
	}
      Dprintf (DEBUG_ARCHIVE, NTXT ("\nget_loadObjs(): %d\n"),
	       (int) (objs ? objs->size () : -1));
      for (long i = 0, sz = VecSize(objs); i < sz; i++)
	{
	  LoadObject *lo = objs->get (i);
	  Dprintf (DEBUG_ARCHIVE, NTXT ("%s:%d  [%2ld] %s\n"),
		   get_basename (__FILE__), (int) __LINE__, i, STR (lo->dump ()));
	}
    }
  delete loadObjs;
  return objs;
}

/**
 * Clean old archive
 * Except the following cases:
 * 1. Founder experiment is an MPI experiment
 * 2. "-n" option is passed (do not archive descendants)
 * 3. "-m" option is passed (partial archiving)
 * 4. Experiment name is not the founder experiment (it is a sub-experiment)
 * @param expname
 * @param founder_exp
 * @return 0 - success
 */
int
er_archive::clean_old_archive (char *expname, ArchiveExp *founder_exp)
{
  if (0 == descendant)
    { // do not archive descendants
      fprintf (stderr, GTXT ("Warning: Option -F is ignored because -n option is specified (do not archive descendants)\n"));
      return 1;
    }
  if (NULL != mask)
    { // partial archiving
      fprintf (stderr, GTXT ("Warning: Option -F is ignored because -m option is specified\n"));
      return 1;
    }
  // Check if the experiment is the founder
  char *s1 = dbe_strdup (expname);
  char *s2 = dbe_strdup (founder_exp->get_expt_name ());
  if (!s1 || !s2)
    {
      fprintf (stderr, GTXT ("Cannot allocate memory\n"));
      exit (1);
    }
  // remove trailing slashes
  for (int n = strlen (s1); n > 0; n--)
    {
      if ('/' != s1[n - 1])
	break;
      s1[n - 1] = 0;
    }
  for (int n = strlen (s2); n > 0; n--)
    {
      if ('/' != s2[n - 1])
	break;
      s2[n - 1] = 0;
    }
  if (strcmp (s1, s2) != 0)
    { // not founder
      fprintf (stderr, GTXT ("Warning: Option -F is ignored because specified experiment name %s does not match founder experiment name %s\n"), s1, s2);
      free (s1);
      free (s2);
      return 1;
    }
  // Remove old "archives"
  char *arch = founder_exp->get_arch_name ();
  fprintf (stderr, GTXT ("INFO: removing existing archive: %s\n"), arch);
  if (dbe_stat (arch, NULL) == 0)
    {
      char *cmd = dbe_sprintf ("/bin/rm -rf %s", arch);
      system (cmd);
      free (cmd);
      if (dbe_stat (arch, NULL) != 0)
	{ // create "archives"
	  if (!founder_exp->create_dir (founder_exp->get_arch_name ()))
	    {
	      fprintf (stderr, GTXT ("Unable to create directory `%s'\n"), founder_exp->get_arch_name ());
	      exit (1);
	    }
	}
    }
  free (s1);
  free (s2);
  return 0;
} // clean_old_archive_if_necessary

void
er_archive::start (int argc, char *argv[])
{
  int last = argc - 1;
  if (check_args (argc, argv) != last)
    usage ();
  check_env_var ();
  if (s_option == ARCH_NOTHING)
    return;

  ArchiveExp *founder_exp = new ArchiveExp (argv[last]);
  if (founder_exp->get_status () == Experiment::FAILURE)
    {
      if (!quiet)
	fprintf (stderr, GTXT ("er_archive: %s: %s\n"), argv[last],
		 pr_mesgs (founder_exp->fetch_errors (), NTXT (""), NTXT ("")));
      exit (1);
    }
  if (!founder_exp->create_dir (founder_exp->get_arch_name ()))
    {
      fprintf (stderr, GTXT ("Unable to create directory `%s'\n"), founder_exp->get_arch_name ());
      exit (1);
    }
  if (!common_archive_dir)
    common_archive_dir = dbe_strdup (getenv ("GPROFNG_ARCHIVE_COMMON_DIR"));
  if (common_archive_dir)
    {
      if (!founder_exp->create_dir (common_archive_dir))
	if (dbe_stat (common_archive_dir, NULL) != 0)
	  {
	    fprintf (stderr, GTXT ("Unable to create directory for common archive `%s'\n"), common_archive_dir);
	    exit (1);
	  }
    }
  // Clean old archives if necessary
  if (force)
    clean_old_archive (argv[last], founder_exp);
  Vector<ArchiveExp*> *exps = new Vector<ArchiveExp*>();
  exps->append (founder_exp);
  if (descendant)
    {
      Vector<char*> *exp_names = founder_exp->get_descendants_names ();
      if (exp_names)
	{
	  for (long i = 0, sz = exp_names->size (); i < sz; i++)
	    {
	      char *exp_path = exp_names->get (i);
	      ArchiveExp *exp = new ArchiveExp (exp_path);
	      if (exp->get_status () == Experiment::FAILURE)
		{
		  if (!quiet)
		    fprintf (stderr, GTXT ("er_archive: %s: %s\n"), exp_path,
			     pr_mesgs (exp->fetch_errors (), NTXT (""), NTXT ("")));
		  delete exp;
		  continue;
		}
	      exps->append (exp);
	    }
	  exp_names->destroy ();
	  delete exp_names;
	}
    }
  for (long i = 0, sz = exps->size (); i < sz; i++)
    {
      ArchiveExp *exp = exps->get (i);
      exp->read_data (s_option);
    }

  Vector <DbeFile*> *copy_files = new Vector<DbeFile*>();
  Vector <LoadObject*> *loadObjs = get_loadObjs ();
  for (long i = 0, sz = VecSize(loadObjs); i < sz; i++)
    {
      LoadObject *lo = loadObjs->get (i);
      if (strcmp (lo->get_pathname (), "LinuxKernel") == 0)
	continue;
      DbeFile *df = lo->dbeFile;
      if ((df->filetype & DbeFile::F_FICTION) != 0)
	continue;
      if (df->get_location () == NULL)
	{
	  copy_files->append (df);
	  continue;
	}
      if ((df->filetype & DbeFile::F_JAVACLASS) != 0)
	{
	  if (df->container)
	    { // Found in .jar file
	      copy_files->append (df->container);
	    }
	  copy_files->append (df);
	  if ((s_option & ARCH_EXE_ONLY) != 0)
	    continue;
	}
      lo->sync_read_stabs ();
      Elf *elf = lo->get_elf ();
      if (elf && (lo->checksum != 0) && (lo->checksum != elf->elf_checksum ()))
	{
	  if (!quiet)
	    fprintf (stderr, GTXT ("er_archive: '%s' has an unexpected checksum value; perhaps it was rebuilt. File ignored\n"),
		       df->get_location ());
	  continue;
	}
      copy_files->append (df);
      if (elf)
	{
	  Elf *f = elf->find_ancillary_files (lo->get_pathname ());
	  if (f)
	    copy_files->append (f->dbeFile);
	  for (long i1 = 0, sz1 = VecSize(elf->ancillary_files); i1 < sz1; i1++)
	    {
	      Elf *ancElf = elf->ancillary_files->get (i1);
	      copy_files->append (ancElf->dbeFile);
	    }
	}
      Vector<Module*> *modules = lo->seg_modules;
      for (long i1 = 0, sz1 = VecSize(modules); i1 < sz1; i1++)
	{
	  Module *mod = modules->get (i1);
	  if ((mod->flags & MOD_FLAG_UNKNOWN) != 0)
	    continue;
	  else if ((s_option & (ARCH_USED_EXE_ONLY | ARCH_USED_SRC_ONLY)) != 0 &&
		   !mod->isUsed)
	    continue;
	  if ((s_option & ARCH_ALL) != 0)
	    mod->read_stabs (false); // Find all Sources
	  if (mod->dot_o_file && mod->dot_o_file->dbeFile)
	    copy_files->append (mod->dot_o_file->dbeFile);
	}
    }
  delete loadObjs;

  int bmask = DbeFile::F_LOADOBJ | DbeFile::F_JAVACLASS | DbeFile::F_JAR_FILE |
	  DbeFile::F_DOT_O | DbeFile::F_DEBUG_FILE;
  if ((s_option & (ARCH_USED_SRC_ONLY | ARCH_ALL)) != 0)
    {
      bmask |= DbeFile::F_JAVA_SOURCE | DbeFile::F_SOURCE;
      Vector<SourceFile*> *sources = dbeSession->get_sources ();
      for (long i = 0, sz = VecSize(sources); i < sz; i++)
	{
	  SourceFile *src = sources->get (i);
	  if ((src->flags & SOURCE_FLAG_UNKNOWN) != 0)
	    continue;
	  else if ((s_option & ARCH_USED_SRC_ONLY) != 0)
	    {
	      if ((src->dbeFile->filetype & DbeFile::F_JAVA_SOURCE) != 0 &&
		  !src->isUsed)
		continue;
	    }
	  if (src->dbeFile)
	    copy_files->append (src->dbeFile);
	}
    }

  Vector <DbeFile*> *notfound_files = new Vector<DbeFile*>();
  for (long i = 0, sz = VecSize(copy_files); i < sz; i++)
    {
      DbeFile *df = copy_files->get (i);
      char *fnm = df->get_location ();
      char *nm = df->get_name ();
      Dprintf (DEBUG_ARCHIVE,
	       "%s::%d copy_files[%ld] filetype=%4d inArchive=%d '%s' --> '%s'\n",
	       get_basename (__FILE__), (int) __LINE__, i,
	       df->filetype, df->inArchive ? 1 : 0, STR (nm), STR (fnm));
      Dprintf (DEBUG_ARCHIVE && df->container,
	       "    copy_files[%ld]: Found '%s' in '%s'\n",
	       i, STR (nm), STR (df->container->get_name ()));
      if (fnm == NULL)
	{
	  if (!quiet)
	    notfound_files->append (df);
	  continue;
	}
      else if (df->inArchive)
	{
	  Dprintf (DEBUG_ARCHIVE,
		   "  NOT COPIED: copy_files[%ld]: inArchive=1 '%s'\n",
		   i, STR (nm));
	  continue;
	}
      else if ((df->filetype & bmask) == 0)
	{
	  Dprintf (DEBUG_ARCHIVE,
		   "  NOT COPIED: copy_files[%ld]: container=%p filetype=%d bmask=%d '%s'\n",
		   i, df->container, df->filetype, bmask, STR (nm));
	  continue;
	}
      else if (df->container &&
	       (df->filetype & (DbeFile::F_JAVA_SOURCE | DbeFile::F_SOURCE)) == 0)
	{
	  Dprintf (DEBUG_ARCHIVE,
		   "  NOT COPIED: copy_files[%ld]: container=%p filetype=%d bmask=%d '%s'\n",
		   i, df->container, df->filetype, bmask, STR (nm));
	  continue;
	}
      else if (!mask_is_on (df->get_name ()))
	{
	  Dprintf (DEBUG_ARCHIVE,
		   "  NOT COPIED: copy_files[%ld]: mask is off for '%s'\n",
		   i, STR (nm));
	  continue;
	}
      char *anm = founder_exp->getNameInArchive (nm, false);
      if (force)
	unlink (anm);
      int res = founder_exp->copy_file (fnm, anm, quiet, common_archive_dir, use_relative_path);
      if (0 == res)  // file successfully archived
	df->inArchive = 1;
      delete anm;
    }
  delete copy_files;

  if (notfound_files->size () > 0)
    {
      for (long i = 0, sz = notfound_files->size (); i < sz; i++)
	{
	  DbeFile *df = notfound_files->get (i);
	  fprintf (stderr, GTXT ("er_archive: Cannot find file: `%s'\n"), df->get_name ());
	}
      fprintf (stderr, GTXT ("\n If you know the correct location of the missing file(s)"
			     " you can help %s to find them by manually editing the .gprofng.rc file."
			     " See %s man pages for more details.\n"),
	       whoami, whoami);
    }
  delete notfound_files;
}

int
er_archive::check_args (int argc, char *argv[])
{
  int opt;
  int rseen = 0;
  int dseen = 0;
  // Parsing the command line
  opterr = 0;
  optind = 1;
  static struct option long_options[] = {
    {"help",    no_argument,        0, 'h'},
    {"version", no_argument,        0, 'V'},
    {"whoami",  required_argument,  0, 'w'},
    {"outfile", required_argument,  0, 'O'},
    {NULL, 0, 0, 0}
  };
  while (1)
    {
      int option_index = 0;
      opt = getopt_long (argc, argv, NTXT (":VFa:d:qnr:m:"),
			 long_options, &option_index);
      if (opt == EOF)
	break;
      switch (opt)
	{
	case 'F':
	  force = 1;
	  break;
	case 'd': // Common archive directory (absolute path)
	  if (rseen)
	    {
	      fprintf (stderr, GTXT ("Error: invalid combination of options: -r and -d are in conflict.\n"));
	      return -1;
	    }
	  if (dseen)
	    fprintf (stderr, GTXT ("Warning: option -d was specified several times. Last value is used.\n"));
	  free (common_archive_dir);
	  common_archive_dir = strdup (optarg);
	  dseen = 1;
	  break;
	case 'q':
	  quiet = 1;
	  break;
	case 'n':
	  descendant = 0;
	  break;
	case 'r': // Common archive directory (relative path)
	  if (dseen)
	    {
	      fprintf (stderr, GTXT ("Error: invalid combination of options: -d and -r are in conflict.\n"));
	      return -1;
	    }
	  if (rseen)
	    fprintf (stderr, GTXT ("Warning: option -r was specified several times. Last value is used.\n"));
	  free (common_archive_dir);
	  common_archive_dir = strdup (optarg);
	  use_relative_path = 1;
	  rseen = 1;
	  break;
	case 'a':
	  if (strcmp (optarg, "off") == 0)
	    s_option = ARCH_NOTHING;
	  else if (strcmp (optarg, "on") == 0 ||
		   strcmp (optarg, "ldobjects") == 0)
	    s_option = ARCH_EXE_ONLY;
	  else if (strcmp (optarg, "usedldobjects") == 0)
	    s_option = ARCH_USED_EXE_ONLY;
	  else if (strcmp (optarg, "usedsrc") == 0)
	    s_option = ARCH_USED_EXE_ONLY | ARCH_USED_SRC_ONLY;
	  else if (strcmp (optarg, "all") == 0 || strcmp (optarg, "src") == 0)
	    s_option = ARCH_ALL;
	  else
	    {
	      fprintf (stderr, GTXT ("Error: invalid option: `-%c %s'\n"),
		       optopt, optarg);
	      return -1;
	    }
	  break;
	case 'm':
	  {
	    regex_t *regex_desc = new regex_t ();
	    if (regcomp (regex_desc, optarg, REG_EXTENDED | REG_NOSUB | REG_NEWLINE))
	      {
		delete regex_desc;
		fprintf (stderr, GTXT ("Error: invalid option: `-%c %s'\n"),
			 optopt, optarg);
		return -1;
	      }
	    if (mask == NULL)
	      mask = new Vector<regex_t *>();
	    mask->append (regex_desc);
	    break;
	  }
	case 'O':
	  {
	    int fd = open (optarg, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	    if (fd == -1)
	      {
		fprintf (stderr, GTXT ("er_archive: Can't open %s: %s\n"),
			 optarg, strerror (errno));
		break;
	      }
	    if (dup2 (fd, 2) == -1)
	      {
		close (fd);
		fprintf (stderr, GTXT ("er_archive: Can't divert stderr: %s\n"),
			 strerror (errno));
		break;
	      }
	    if (dup2 (fd, 1) == -1)
	      {
		close (fd);
		fprintf (stderr, GTXT ("er_archive: Can't divert stdout: %s\n"),
			 strerror (errno));
		break;
	      }
	    close (fd);
	    struct timeval tp;
	    gettimeofday (&tp, NULL);
	    fprintf (stderr, "### Start %s#", ctime (&tp.tv_sec));
	    for (int i = 0; i < argc; i++)
	      fprintf (stderr, " %s", argv[i]);
	    fprintf (stderr, "\n");
	    break;
	  }
	case 'V':
// Ruud
	  Application::print_version_info ();
/*
	  printf (GTXT ("GNU %s version %s\n"), get_basename (prog_name), VERSION);
*/
	  exit (0);
	case 'w':
	  whoami = optarg;
	  break;
	case 'h':
	  usage ();
	  exit (0);
	case ':': // -s -m without operand
	  fprintf (stderr, GTXT ("Option -%c requires an operand\n"), optopt);
	  return -1;
	case '?':
	default:
	  fprintf (stderr, GTXT ("Unrecognized option: -%c\n"), optopt);
	  return -1;
	}
    }
  return optind;
}

void
er_archive::check_env_var ()
{
  char *ename = NTXT ("GPROFNG_ARCHIVE");
  char *var = getenv (ename);
  if (var == NULL)
    return;
  var = dbe_strdup (var);
  Vector<char*> *opts = new Vector<char*>();
  opts->append (ename);
  for (char *s = var;;)
    {
      while (*s && isblank (*s))
	s++;
      if (*s == 0)
	break;
      opts->append (s);
      while (*s && !isblank (*s))
	s++;
      if (*s == 0)
	break;
      *s = 0;
      s++;
    }
  if (opts->size () > 0)
    {
      char **arr = (char **) malloc (sizeof (char *) *opts->size ());
      for (long i = 0; i < opts->size (); i++)
	arr[i] = opts->get (i);
      if (-1 == check_args (opts->size (), arr))
	fprintf (stderr, GTXT ("Error: Wrong SP_ER_ARCHIVE: '%s'\n"), var);
      free (arr);
    }
  delete opts;
  free (var);
}

static int
real_main (int argc, char *argv[])
{
  er_archive *archive = new er_archive (argc, argv);
  dbeSession->archive_mode = 1;
  archive->start (argc, argv);
  dbeSession->unlink_tmp_files ();
  return 0;
}

/**
 * Call catch_out_of_memory(int (*real_main)(int, char*[]), int argc, char *argv[]) which will call real_main()
 * @param argc
 * @param argv
 * @return
 */
int
main (int argc, char *argv[])
{
  return catch_out_of_memory (real_main, argc, argv);
}
