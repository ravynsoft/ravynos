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
#include "util.h"
#include "DbeSession.h"
#include "Experiment.h"
#include "DbeFile.h"
#include "ExpGroup.h"
#include "DbeJarFile.h"

DbeFile::DbeFile (const char *filename)
{
  filetype = 0;
  name = dbe_strdup (filename);
  name = canonical_path (name);
  orig_location = NULL;
  location = NULL;
  location_info = NULL;
  jarFile = NULL;
  container = NULL;
  need_refind = true;
  inArchive = false;
  sbuf.st_atim.tv_sec = 0;
  experiment = NULL;
}

DbeFile::~DbeFile ()
{
  free (name);
  free (location);
  free (orig_location);
  free (location_info);
}

void
DbeFile::set_need_refind (bool val)
{
  if (val != need_refind)
    {
      free (location_info);
      location_info = NULL;
      need_refind = val;
    }
}

void
DbeFile::set_location (const char *filename)
{
  free (location);
  location = NULL;
  if (filename)
    {
      if (strncmp (filename, NTXT ("./"), 2) == 0)
	filename += 2;
      location = canonical_path (dbe_strdup (filename));
    }
  free (location_info);
  location_info = NULL;
  set_need_refind (false);
}

char *
DbeFile::get_location_info ()
{
  if (location_info == NULL)
    {
      char *fnm = get_name ();
      char *loc = get_location ();
      Dprintf (DEBUG_DBE_FILE, NTXT ("DbeFile::get_location_info: %s %s\n"),
	       STR (fnm), STR (loc));
      if (loc == NULL)
	{
	  if (filetype & F_FICTION)
	    location_info = dbe_strdup (fnm);
	  else
	    location_info = dbe_sprintf (GTXT ("%s (not found)"),
					 get_relative_path (fnm));
	}
      else
	{
	  char *r_fnm = get_relative_path (fnm);
	  char *r_loc = get_relative_path (loc);
	  if (strcmp (r_fnm, r_loc) == 0)
	    location_info = dbe_strdup (r_fnm);
	  else
	    {
	      char *bname = get_basename (r_fnm);
	      if (strcmp (bname, r_loc) == 0)  // found in current directory
		location_info = dbe_strdup (bname);
	      else
		location_info = dbe_sprintf (GTXT ("%s (found as %s)"), bname, r_loc);
	    }
	}
    }
  return location_info;
}

char *
DbeFile::getResolvedPath ()
{
  if (get_location ())
    return location;
  return name;
}

DbeFile *
DbeFile::getJarDbeFile (char *fnm, int sym)
{
  Dprintf (DEBUG_DBE_FILE, NTXT ("DbeFile::getJarDbeFile: %s fnm='%s' sym=%d\n"),
	   STR (name), STR (fnm), sym);
  DbeFile *df = NULL;
  if (sym)
    {
      char *s = strchr (fnm, sym);
      if (s)
	{
	  s = dbe_strndup (fnm, s - fnm);
	  df = dbeSession->getDbeFile (s, F_JAR_FILE | F_FILE);
	  free (s);
	}
    }
  if (df == NULL)
    df = dbeSession->getDbeFile (fnm, F_JAR_FILE | F_FILE);
  if (df && (df->experiment == NULL))
    df->experiment = experiment;
  return df;
}

char *
DbeFile::get_location (bool find_needed)
{
  Dprintf (DEBUG_DBE_FILE, NTXT ("get_location 0x%x %s\n"), filetype, STR (name));
  if (!find_needed)
    return need_refind ? NULL : location;
  if (location || !need_refind)
    return location;
  set_need_refind (false);
  if ((filetype & F_FICTION) != 0)
    return NULL;
  if (filetype == F_DIR_OR_JAR)
    {
      find_in_archives (name);
      if (location)
	{
	  filetype |= F_JAR_FILE | F_FILE;
	  return location;
	}
      find_in_pathmap (name);
      if (location)
	return location;
      if (check_access (name) == F_DIRECTORY)
	{
	  filetype |= F_DIRECTORY;
	  set_location (name);
	  return location;
	}
    }

  if ((filetype & F_FILE) != 0)
    {
      if (experiment)
	{
	  char *fnm = experiment->checkFileInArchive (name, false);
	  if (fnm)
	    {
	      set_location (fnm);
	      inArchive = true;
	      sbuf.st_mtime = 0; // Don't check timestamps
	      free (fnm);
	      return location;
	    }
	  if ((filetype & F_JAVACLASS) != 0)
	    {
	      if (orig_location)
		{
		  Dprintf (DEBUG_DBE_FILE, NTXT ("DbeFile::get_location:%d name='%s' orig_location='%s'\n"),
			   (int) __LINE__, name, orig_location);
		  // Parse a fileName attribute. There are 4 possibilities:
		  //   file:<Class_Name>
		  //   file:<name_of_jar_or_zip_file>
		  //   jar:file:<name_of_jar_or_zip_file>!<Class_Name>
		  //   zip:<name_of_jar_or_zip_file>!<Class_Name>
		  DbeFile *jar_df = NULL;
		  if (strncmp (orig_location, NTXT ("zip:"), 4) == 0)
		    jar_df = getJarDbeFile (orig_location + 4, '!');
		  else if (strncmp (orig_location, NTXT ("jar:file:"), 9) == 0)
		    jar_df = getJarDbeFile (orig_location + 9, '!');
		  else if (strncmp (orig_location, NTXT ("file:"), 5) == 0
			   && isJarOrZip (orig_location + 5))
		    jar_df = getJarDbeFile (orig_location + 5, 0);
		  if (jar_df)
		    {
		      if (find_in_jar_file (name, jar_df->get_jar_file ()))
			{
			  Dprintf (DEBUG_DBE_FILE, NTXT ("DbeFile::get_location:%d FOUND name='%s' location='%s' jar='%s'\n"),
				   (int) __LINE__, name, STR (location), STR (jar_df->get_location ()));
			  inArchive = jar_df->inArchive;
			  container = jar_df;
			  return location;
			}
		    }
		  if (strncmp (orig_location, NTXT ("file:"), 5) == 0
		      && !isJarOrZip (orig_location + 5))
		    {
		      DbeFile *df = new DbeFile (orig_location + 5);
		      df->filetype = DbeFile::F_FILE;
		      df->experiment = experiment;
		      fnm = df->get_location ();
		      if (fnm)
			{
			  set_location (fnm);
			  inArchive = df->inArchive;
			  sbuf.st_mtime = df->sbuf.st_mtime;
			  Dprintf (DEBUG_DBE_FILE, NTXT ("DbeFile::get_location:%d FOUND name='%s' orig_location='%s' location='%s'\n"),
				   (int) __LINE__, name, orig_location, fnm);
			  delete df;
			  return location;
			}
		      delete df;
		    }
		}
	      fnm = dbe_sprintf (NTXT ("%s/%s/%s"), experiment->get_expt_name (), SP_DYNAMIC_CLASSES, name);
	      if (find_file (fnm))
		{
		  inArchive = true;
		  sbuf.st_mtime = 0; // Don't check timestamps
		  Dprintf (DEBUG_DBE_FILE, NTXT ("DbeFile::get_location:%d FOUND name='%s' location='%s'\n"),
			   (int) __LINE__, name, fnm);
		  free (fnm);
		  return location;
		}
	      free (fnm);
	    }
	}
    }

  if (dbeSession->archive_mode)
    {
      find_file (name);
      if (location)
	return location;
    }

  bool inPathMap = find_in_pathmap (name);
  if (location)
    return location;
  find_in_setpath (name, dbeSession->get_search_path ());
  if (location)
    return location;
  if ((filetype & (F_JAVACLASS | F_JAVA_SOURCE)) != 0)
    {
      find_in_classpath (name, dbeSession->get_classpath ());
      if (location)
	return location;
    }
  if (!inPathMap)
    find_file (name);
  Dprintf (DEBUG_DBE_FILE && (location == NULL),
	   "DbeFile::get_location:%d NOT FOUND name='%s'\n", __LINE__, name);
  return location;
}

int
DbeFile::check_access (const char *filename)
{
  if (filename == NULL)
    return F_NOT_FOUND;
  int st = dbe_stat (filename, &sbuf);
  Dprintf (DEBUG_DBE_FILE, NTXT ("check_access: %d 0x%x %s\n"), st, filetype, filename);
  if (st == 0)
    {
      if (S_ISDIR (sbuf.st_mode))
	return F_DIRECTORY;
      else if (S_ISREG (sbuf.st_mode))
	return F_FILE;
      return F_UNKNOWN; // Symbolic link or unknown type of file
    }
  sbuf.st_atim.tv_sec = 0;
  sbuf.st_mtime = 0; // Don't check timestamps
  return F_NOT_FOUND; // File not found
}

bool
DbeFile::isJarOrZip (const char *fnm)
{
  size_t len = strlen (fnm) - 4;
  return len > 0 && (strcmp (fnm + len, NTXT (".jar")) == 0
		     || strcmp (fnm + len, NTXT (".zip")) == 0);
}

char *
DbeFile::find_file (const char *filename)
{
  switch (check_access (filename))
    {
    case F_DIRECTORY:
      if (filetype == F_DIR_OR_JAR)
	filetype |= F_DIRECTORY;
      if ((filetype & F_DIRECTORY) != 0)
	set_location (filename);
      break;
    case F_FILE:
      if (filetype == F_DIR_OR_JAR)
	{
	  filetype |= F_FILE;
	  if (isJarOrZip (filename))
	    filetype |= F_JAR_FILE;
	}
      if ((filetype & F_DIRECTORY) == 0)
	set_location (filename);
      break;
    }
  return location;
}

DbeJarFile *
DbeFile::get_jar_file ()
{
  if (jarFile == NULL)
    {
      char *fnm = get_location ();
      if (fnm)
	jarFile = dbeSession->get_JarFile (fnm);
    }
  return jarFile;
}

char *
DbeFile::find_package_name (const char *filename, const char *dirname)
{
  char *nm = dbe_sprintf (NTXT ("%s/%s"), dirname, filename);
  if (!find_in_pathmap (nm))
    find_file (nm);
  free (nm);
  return location;
}

char *
DbeFile::find_in_directory (const char *filename, const char *dirname)
{
  if (filename && dirname)
    {
      char *nm = dbe_sprintf (NTXT ("%s/%s"), dirname, filename);
      find_file (nm);
      free (nm);
    }
  return location;
}

char *
DbeFile::find_in_jar_file (const char *filename, DbeJarFile *jfile)
{
  // Read .jar or .zip
  if (jfile == NULL)
    return NULL;
  int entry = jfile->get_entry (filename);
  if (entry >= 0)
    {
      char *fnm = dbeSession->get_tmp_file_name (filename, true);
      long long fsize = jfile->copy (fnm, entry);
      if (fsize >= 0)
	{
	  dbeSession->tmp_files->append (fnm);
	  set_location (fnm);
	  sbuf.st_size = fsize;
	  sbuf.st_mtime = 0; // Don't check timestamps
	  fnm = NULL;
	}
      free (fnm);
    }
  return location;
}

bool
DbeFile::find_in_pathmap (char *filename)
{
  Vector<pathmap_t*> *pathmaps = dbeSession->get_pathmaps ();
  bool inPathMap = false;
  if (strncmp (filename, NTXT ("./"), 2) == 0)
    filename += 2;
  for (int i = 0, sz = pathmaps ? pathmaps->size () : 0; i < sz; i++)
    {
      pathmap_t *pmp = pathmaps->fetch (i);
      size_t len = strlen (pmp->old_prefix);
      if (strncmp (pmp->old_prefix, filename, len) == 0
	  && (filename[len] == '/' || filename[len] == '\0'))
	{
	  inPathMap = true;
	  if (find_in_directory (filename + len, pmp->new_prefix))
	    {
	      return inPathMap;
	    }
	}
    }
  return inPathMap;
}

void
DbeFile::find_in_archives (char *filename)
{
  for (int i1 = 0, sz1 = dbeSession->expGroups->size (); i1 < sz1; i1++)
    {
      ExpGroup *gr = dbeSession->expGroups->fetch (i1);
      if (gr->founder)
	{
	  char *nm = gr->founder->checkFileInArchive (filename, false);
	  if (nm)
	    {
	      find_file (nm);
	      if (location)
		{
		  sbuf.st_mtime = 0; // Don't check timestamps
		  return;
		}
	    }
	}
    }
}

void
DbeFile::find_in_setpath (char *filename, Vector<char*> *searchPath)
{
  char *base = get_basename (filename);
  for (int i = 0, sz = searchPath ? searchPath->size () : 0; i < sz; i++)
    {
      char *spath = searchPath->fetch (i);
      // Check file in each experiment directory
      if (streq (spath, "$") || streq (spath, NTXT ("$expts")))
	{
	  // find only in founders and only LoadObj.
	  for (int i1 = 0, sz1 = dbeSession->expGroups->size (); i1 < sz1; i1++)
	    {
	      ExpGroup *gr = dbeSession->expGroups->fetch (i1);
	      char *exp_name = gr->founder->get_expt_name ();
	      if (gr->founder)
		{
		  if ((filetype & (F_JAVACLASS | F_JAVA_SOURCE)) != 0)
		    {
		      // Find with the package name
		      if (find_in_directory (filename, exp_name))
			 return;
		    }
		  if (find_in_directory (base, exp_name))
		    return;
		}
	    }
	  continue;
	}
      DbeFile *df = dbeSession->getDbeFile (spath, DbeFile::F_DIR_OR_JAR);
      if (df->get_location () == NULL)
	continue;
      if ((filetype & (F_JAVACLASS | F_JAVA_SOURCE)) != 0)
	{
	  if ((df->filetype & F_JAR_FILE) != 0)
	    {
	      if (find_in_jar_file (filename, df->get_jar_file ()))
		{
		  container = df;
		  return;
		}
	      continue;
	    }
	  else if ((df->filetype & F_DIRECTORY) != 0)
	    // Find with the package name
	    if (find_package_name (filename, spath))
	      return;
	}
      if ((df->filetype & F_DIRECTORY) != 0)
	if (find_in_directory (base, df->get_location ()))
	  return;
    }
}

void
DbeFile::find_in_classpath (char *filename, Vector<DbeFile*> *classPath)
{
  for (int i = 0, sz = classPath ? classPath->size () : 0; i < sz; i++)
    {
      DbeFile *df = classPath->fetch (i);
      if (df->get_location () == NULL)
	continue;
      if ((df->filetype & F_JAR_FILE) != 0)
	{
	  if (find_in_jar_file (filename, df->get_jar_file ()))
	    {
	      container = df;
	      return;
	    }
	}
      else if ((df->filetype & F_DIRECTORY) != 0)
	// Find with the package name
	if (find_package_name (filename, df->get_name ()))
	  return;
    }
}

struct stat64 *
DbeFile::get_stat ()
{
  if (sbuf.st_atim.tv_sec == 0)
    {
      int st = check_access (get_location (false));
      if (st == F_NOT_FOUND)
	return NULL;
    }
  return &sbuf;
}

bool
DbeFile::compare (DbeFile *df)
{
  if (df == NULL)
    return false;
  struct stat64 *st1 = get_stat ();
  struct stat64 *st2 = df->get_stat ();
  if (st1 == NULL || st2 == NULL)
    return false;
  if (st1->st_size != st2->st_size)
    return false;
  if (st1->st_mtim.tv_sec != st2->st_mtim.tv_sec)
    return false;
  return true;
}
