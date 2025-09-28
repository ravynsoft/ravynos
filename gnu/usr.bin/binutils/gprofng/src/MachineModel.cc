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
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "DbeSession.h"
#include "Command.h"
#include "Application.h"
#include "MemorySpace.h"
#include "i18n.h"

#define MAXARGS     20

static const char *LIBNAME = "../lib/analyzer/lib/machinemodels";

char *
DbeSession::find_mach_model (char *name)
{
  // Read current working directory to see if it's there
  if (name[0] == '/')
    {
      // Absolute path given
      char *path = dbe_sprintf (NTXT ("%s.ermm"), name);
      if (access (path, R_OK | F_OK) == 0)
	return path;
      free (path);
      // Don't try anywhere else
      return NULL;
    }

  char *path = dbe_sprintf (NTXT ("./%s.ermm"), name);
  if (access (path, R_OK | F_OK) == 0)
    return path;
  free (path);


  // Read the user's home directory to see if it's there
  char *home = getenv (NTXT ("HOME"));
  if (home != NULL)
    {
      path = dbe_sprintf (NTXT ("%s/%s.ermm"), home, name);
      if (access (path, R_OK | F_OK) == 0)
	return path;
      free (path);
    }
  if (strchr (name, (int) '/') != NULL)
    // name has a slash; don't look in system installation directory
    return NULL;

  // Read system installation directory to see if it's there
  path = dbe_sprintf ("%s/%s/%s.ermm", theApplication->get_run_dir (),
		      LIBNAME, name);
  if (access (path, R_OK | F_OK) == 0)
    return path;
  free (path);
  return NULL;
}

//  Handle the definitions from a machinemodel file
//  Return value is NULL if it was OK, or an error message if not
char *
DbeSession::load_mach_model (char *_name)
{
  CmdType cmd_type;
  int arg_count, cparam;
  char *cmd, *end_cmd;
  char *arglist[MAXARGS];
  char *ret = NULL;
  char *path = NULL;
  FILE *fptr = NULL;

  // Does name have .ermm suffix?  If so, strip it away
  char *name = dbe_strdup (_name);
  size_t len = strlen (name);
  if (len > 5 && strcmp (name + len - 5, ".ermm") == 0)
    name[len - 5] = 0;

  if ((mach_model_loaded != NULL) && (strcmp (name, mach_model_loaded) == 0))
    {
      ret = dbe_sprintf (GTXT ("Machine model %s is already loaded\n"), name);
      free (name);
      return ret;
    }
  else if (mach_model_loaded == NULL && len == 0)
    {
      ret = dbe_sprintf (GTXT ("No Machine model is loaded\n"));
      free (name);
      return ret;
    }

  if (len != 0)
    {
      // zero-length just means unload any previously loaded model; only look if non-zero
      path = find_mach_model (name);
      if (path == NULL)
	{
	  ret = dbe_sprintf (GTXT ("Machine model %s not found\n"), name);
	  free (name);
	  return ret;
	}
      fptr = fopen (path, NTXT ("r"));
      if (fptr == NULL)
	{
	  ret = dbe_sprintf (GTXT ("Open of Machine model %s, file %s failed\n"), name, path);
	  free (path);
	  free (name);
	  return ret;
	}
    }

  // We are now committed to make the new machine model the loaded one;
  //   Delete any MemoryObjects from any previously loaded machinemodel
  if (dbeSession->mach_model_loaded != NULL)
    {
      Vector <char *> *oldobjs = MemorySpace::getMachineModelMemObjs
	      (dbeSession->mach_model_loaded);
      for (int i = 0; i < oldobjs->size (); i++)
	MemorySpace::mobj_delete (oldobjs->fetch (i));
      delete oldobjs;
      free (mach_model_loaded);
    }
  if (len == 0)
    {
      mach_model_loaded = NULL;
      free (name);
      // and there's no "loading" to do; just return
      return NULL;
    }
  else
    mach_model_loaded = name;

  int line_no = 0;
  end_cmd = NULL;

  while (!feof (fptr))
    {
      char *script = read_line (fptr);
      if (script == NULL)
	continue;

      line_no++;
      strtok (script, NTXT ("\n"));

      // extract the command
      cmd = strtok (script, NTXT (" \t"));
      if (cmd == NULL || *cmd == '#' || *cmd == '\n')
	{
	  free (script);
	  continue;
	}

      char *remainder = strtok (NULL, NTXT ("\n"));
      // now extract the arguments
      int nargs = 0;
      for (;;)
	{
	  if (nargs >= MAXARGS)
	    {
	      ret = dbe_sprintf (GTXT ("Warning: more than %d arguments to %s command, line %d\n"),
				 MAXARGS, cmd, line_no);
	      continue;
	    }

	  char *nextarg = strtok (remainder, NTXT ("\n"));

	  if (nextarg == NULL || *nextarg == '#')
	      // either the end of the line, or a comment indicator
	      break;
	  arglist[nargs++] = parse_qstring (nextarg, &end_cmd);
	  remainder = end_cmd;
	  if (remainder == NULL)
	    break;

	  // skip any blanks or tabs to get to next argument
	  while ((*remainder == ' ') || (*remainder == '\t'))
	    remainder++;
	}

      cmd_type = Command::get_command (cmd, arg_count, cparam);
      // check for extra arguments
      if (cmd_type != UNKNOWN_CMD && cmd_type != INDXOBJDEF && nargs > arg_count)
	ret = dbe_sprintf (GTXT ("Warning: extra arguments to %s command, line %d\n"),
			   cmd, line_no);
      if (nargs < arg_count)
	{
	  ret = dbe_sprintf (GTXT ("Error: missing arguments to %s command, line %d\n"),
			     cmd, line_no);

	  // ignore this command
	  free (script);
	  continue;
	}

      switch (cmd_type)
	{
	case INDXOBJDEF:
	  {
	    char *err = dbeSession->indxobj_define (arglist[0], NULL,
			    arglist[1], (nargs >= 3) ? PTXT (arglist[2]) : NULL,
			    (nargs >= 4) ? PTXT (arglist[3]) : NULL);
	    if (err != NULL)
	      ret = dbe_sprintf (GTXT ("   %s: line %d `%s %s %s'\n"),
				 err, line_no, cmd, arglist[0], arglist[1]);
	    break;
	  }
	case COMMENT:
	  // ignore the line
	  break;
	default:
	  {
	    // unexpected command in a machinemodel file
	    ret = dbe_sprintf (GTXT ("Unexpected command in machinemodel file %s on line %d: `%.64s'\n"),
			       path, line_no, cmd);
	    break;
	  }
	}
      free (script);
    }
  fclose (fptr);
  return ret;
}

Vector<char*> *
DbeSession::list_mach_models ()
{
  Vector<char*> *model_names = new Vector<char*>();

  // Open the current directory to read the entries there
  DIR *dir = opendir (NTXT ("."));
  if (dir != NULL)
    {
      struct dirent *entry = NULL;
      while ((entry = readdir (dir)) != NULL)
	{
	  size_t len = strlen (entry->d_name);
	  if (len < 5 || strcmp (entry->d_name + len - 5, ".ermm") != 0)
	    continue;
	  char *model = dbe_strdup (entry->d_name);

	  // remove the .ermm suffix
	  model[len - 5] = 0;

	  // add to vector
	  model_names->append (dbe_strdup (model));
	}
      closedir (dir);
    }

  // Read the user's home directory to list the models there
  char *home = getenv ("HOME");
  if (home != NULL)
    {
      dir = opendir (home);
      if (dir != NULL)
	{
	  struct dirent *entry = NULL;
	  while ((entry = readdir (dir)) != NULL)
	    {
	      size_t len = strlen (entry->d_name);
	      if (len < 5 || strcmp (entry->d_name + len - 5, ".ermm") != 0)
		continue;
	      char *model = dbe_strdup (entry->d_name);

	      // remove the .ermm suffix
	      model[len - 5] = 0;

	      // add to vector
	      model_names->append (dbe_strdup (model));
	    }
	  closedir (dir);
	}
    }

  // Read system installation directory to list the models there
  char *sysdir = dbe_sprintf ("%s/%s", theApplication->get_run_dir (), LIBNAME);

  dir = opendir (sysdir);
  if (dir != NULL)
    {
      struct dirent *entry = NULL;
      while ((entry = readdir (dir)) != NULL)
	{
	  size_t len = strlen (entry->d_name);
	  if (len < 5 || strcmp (entry->d_name + len - 5, ".ermm") != 0)
	    continue;
	  char *model = dbe_strdup (entry->d_name);

	  // remove the .ermm suffix
	  model[len - 5] = 0;

	  // add to vector
	  model_names->append (dbe_strdup (model));
	}
      closedir (dir);
    }
  return model_names;
}
