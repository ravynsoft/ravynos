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
#include <stdlib.h>
#include <strings.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Application.h"
#include "Settings.h"
#include "i18n.h"
#include "util.h"

Application::ProgressFunc Application::progress_func = NULL;
Application *theApplication;

Application::Application (int argc, char *argv[], char *fdhome)
{
  theApplication = this;
  cur_dir = NULL;
  prog_version = dbe_strdup (VERSION);
  set_name (strchr (argv[0], '/') ? argv[0] : NULL);
  whoami = get_basename (get_name ());

  // set up a queue for comments
  commentq = new Emsgqueue (NTXT ("app_commentq"));

  // Locate where the binaries are installed
  set_run_dir (fdhome);

  // Initialize I18N
  init_locale (run_dir);

  // Initialize licensing data
  lic_found = 0;
  lic_err = NULL;

  // Initialize worker threads
  number_of_worker_threads = 1;
#if DEBUG
  char *use_worker_threads = getenv (NTXT ("SP_USE_WORKER_THREADS"));
  if ((NULL != use_worker_threads) && (0 == strcasecmp (use_worker_threads, NTXT ("no"))))
    {
      number_of_worker_threads = 0;
    }
#endif /* DEBUG */
  settings = new Settings (this);
}

Application::~Application ()
{
  delete commentq;
  delete settings;
  free (prog_version);
  free (cur_dir);
  free (prog_name);
  free (run_dir);
}

// Set the name of the application (for messages)
void
Application::set_name (const char *_name)
{
  prog_name = get_realpath (_name);
}

char *
Application::get_realpath (const char *_name)
{
  if (_name == NULL)
    _name = "/proc/self/exe";
  char *exe_name = realpath (_name, NULL);
  if (exe_name)
    return exe_name;
  if (strchr (_name, '/') == NULL)
    {
      char *path = getenv ("PATH");
      if (path)
	for (char *s = path;; s++)
	  if (*s == ':' || *s == 0)
	    {
	      if (path != s)
		{
		  char *nm = dbe_sprintf (NTXT ("%.*s/%s"), (int) (path - s - 1), path, _name);
		  exe_name = realpath (nm, NULL);
		  free (nm);
		  if (exe_name)
		    return exe_name;
		}
	      if (*s == 0)
		break;
	      path = s + 1;
	    }
    }
  return strdup (_name);
}

// Set the directory where all binaries are found
void
Application::set_run_dir (char *fdhome)
{
  run_dir_with_spaces = NULL;
  if (fdhome)
    {
      char *path = dbe_sprintf ("%s/bin", fdhome);
      struct stat sbuf;
      if (stat (path, &sbuf) != -1)
	run_dir = path;
      else
	{
	  free (path);
	  run_dir = dbe_strdup (fdhome);
	}
    }
  else
    {
      run_dir = realpath (prog_name, NULL);
      if (run_dir == NULL)
	{
	  fprintf (stderr, // I18N won't work here -- not catopen yet.
		   GTXT ("Can't find location of %s\n"), prog_name);
	  run_dir = dbe_strdup (get_cur_dir ());
	}
      else
	{
	  char *d = strrchr (run_dir, '/');
	  if (d)
	    *d = 0;
	  // Check if the installation path contains spaces
	  if (strchr (run_dir, ' ') != NULL)
	    {
	      // Create a symbolic link without spaces
	      const char *dir = NTXT ("/tmp/.gprofngLinks");
	      char *symbolic_link = dbe_create_symlink_to_path (run_dir, dir);
	      if (NULL != symbolic_link)
		{
		  // Save old path to avoid memory leak
		  run_dir_with_spaces = run_dir;
		  // Use the path through symbolic link
		  run_dir = symbolic_link;
		}
	    }
	}
    }
}

char *
Application::get_cur_dir ()
{
  if (cur_dir == NULL)
    {
      char cwd[MAXPATHLEN];
      if (getcwd (cwd, sizeof (cwd)) == NULL)
	{
	  perror (prog_name);
	  exit (1);
	}
      cur_dir = dbe_strdup (canonical_path (cwd));
    }
  return cur_dir;
}

/**
 * Get number of worker threads
 * This is used to decide if it is ok to use worker threads for stat()
 * and other actions that can hang for a long time
 * @return number_of_worker_threads
 */
int
Application::get_number_of_worker_threads ()
{
  return number_of_worker_threads;
}

int
Application::check_args (int argc, char *argv[])
{
  int opt;
  // Parsing the command line
  opterr = 0;
  while ((opt = getopt (argc, argv, "V")) != EOF)
    switch (opt)
      {
      case 'V':
// Ruud
	Application::print_version_info ();
/*
	printf (NTXT ("GNU %s version %s\n"), get_basename (prog_name), VERSION);
*/
	exit (0);
      default:
	usage ();
      }
  return optind;
}

Emsg *
Application::fetch_comments ()
{
  if (commentq == NULL)
    return NULL;
  return commentq->fetch ();
}

void
Application::queue_comment (Emsg *m)
{
  commentq->append (m);
}

void
Application::delete_comments ()
{
  if (commentq != NULL)
    {
      delete commentq;
      commentq = new Emsgqueue (NTXT ("app_commentq"));
    }
}

int
Application::set_progress (int percentage, const char *proc_str)
{
  if (progress_func != NULL)
    return progress_func (percentage, proc_str);
  return 0;
}

// Ruud
void
Application::print_version_info ()
{
  printf ( GTXT (
    "GNU %s binutils version %s\n"
    "Copyright (C) 2023 Free Software Foundation, Inc.\n"
    "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n"),
    get_basename (prog_name), VERSION);
}
