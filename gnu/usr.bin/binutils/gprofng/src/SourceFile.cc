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

#include "util.h"
#include "DbeSession.h"
#include "Function.h"
#include "SourceFile.h"
#include "DefaultMap.h"
#include "DbeFile.h"
#include "LoadObject.h"
#include "Module.h"

int SourceFile::curId = 0;

SourceFile::SourceFile (const char *file_name)
{
  status = OS_NOTREAD;
  srcLines = NULL;
  srcInode = -1;
  lines = NULL;
  dbeLines = NULL;
  functions = new DefaultMap<Function *, Function *>();
  dbeFile = new DbeFile (file_name);
  dbeFile->filetype |= DbeFile::F_SOURCE | DbeFile::F_FILE;
  set_name ((char *) file_name);
  srcMTime = (time_t) 0;
  isTmpFile = false;
  flags = 0;
  read_stabs = false;
  id = (uint64_t) ((Histable::SOURCEFILE << 24) + curId) << 32;
  curId++;
}

SourceFile::~SourceFile ()
{
  destroy_map (DbeLine *, dbeLines);
  delete functions;
  delete dbeFile;
  if (lines)
    {
      lines->destroy ();
      delete lines;
    }
  if (srcLines)
    {
      free (srcLines->get (0));
      delete srcLines;
    }
  if (isTmpFile)
    unlink (name);
}

void
SourceFile::set_name (char* _name)
{
  name = dbe_strdup (_name);
}

char*
SourceFile::get_name (NameFormat)
{
  return name;
}

bool
SourceFile::readSource ()
{
  if (srcLines)
    return true;
  status = OS_NOSRC;
  char *location = dbeFile->get_location ();
  if (location == NULL)
    return false;
  if (!isTmpFile)
    srcMTime = dbeFile->sbuf.st_mtime;
  srcInode = dbeFile->sbuf.st_ino;
  size_t srcLen = dbeFile->sbuf.st_size;
  int fd = open64 (location, O_RDONLY);
  if (fd == -1)
    {
      status = OS_NOSRC;
      return false;
    }
  char *srcMap = (char *) malloc (srcLen + 1);
  int64_t sz = read_from_file (fd, srcMap, srcLen);
  if (sz != (int64_t) srcLen)
    append_msg (CMSG_ERROR, GTXT ("%s: Can read only %lld bytes instead %lld"),
		location, (long long) sz, (long long) srcLen);
  srcMap[sz] = 0;
  close (fd);

  // Count the number of lines in the file, converting <nl> to zero
  srcLines = new Vector<char*>();
  srcLines->append (srcMap);
  for (int64_t i = 0; i < sz; i++)
    {
      if (srcMap[i] == '\r')
	{ // Window style
	  srcMap[i] = 0;
	  if (i + 1 < sz && srcMap[i + 1] != '\n')
	    srcLines->append (srcMap + i + 1);
	}
      else if (srcMap[i] == '\n')
	{
	  srcMap[i] = '\0';
	  if (i + 1 < sz)
	    srcLines->append (srcMap + i + 1);
	}
    }
  if (dbeLines)
    {
      Vector<DbeLine *> *v = dbeLines->values ();
      for (long i = 0, sz1 = v ? v->size () : 0; i < sz1; i++)
	{
	  DbeLine *p = v->get (i);
	  if (p->lineno >= srcLines->size ())
	    append_msg (CMSG_ERROR, GTXT ("Wrong line number %d. '%s' has only %d lines"),
			p->lineno, dbeFile->get_location (), srcLines->size ());
	}
      delete v;
    }
  status = OS_OK;
  return true;
}

char *
SourceFile::getLine (int lineno)
{
  assert (srcLines != NULL);
  if (lineno > 0 && lineno <= srcLines->size ())
    return srcLines->get (lineno - 1);
  return NTXT ("");
}

DbeLine *
SourceFile::find_dbeline (Function *func, int lineno)
{
  if (lineno < 0 || (lineno == 0 && func == NULL))
    return NULL;
  DbeLine *dbeLine = NULL;
  if (lines)
    { // the source is available
      if (lineno > lines->size ())
	{
	  if (dbeLines)
	    dbeLine = dbeLines->get (lineno);
	  if (dbeLine == NULL)
	    append_msg (CMSG_ERROR,
			GTXT ("Wrong line number %d. '%s' has only %d lines"),
			lineno, dbeFile->get_location (), lines->size ());
	}
      else
	{
	  dbeLine = lines->fetch (lineno);
	  if (dbeLine == NULL)
	    {
	      dbeLine = new DbeLine (NULL, this, lineno);
	      lines->store (lineno, dbeLine);
	    }
	}
    }
  if (dbeLine == NULL)
    { // the source is not yet read or lineno is wrong
      if (dbeLines == NULL)
	dbeLines = new DefaultMap<int, DbeLine *>();
      dbeLine = dbeLines->get (lineno);
      if (dbeLine == NULL)
	{
	  dbeLine = new DbeLine (NULL, this, lineno);
	  dbeLines->put (lineno, dbeLine);
	}
    }

  for (DbeLine *last = dbeLine;; last = last->dbeline_func_next)
    {
      if (last->func == func)
	return last;
      if (last->dbeline_func_next == NULL)
	{
	  DbeLine *dl = new DbeLine (func, this, lineno);
	  if (functions->get (func) == NULL)
	    functions->put (func, func);
	  last->dbeline_func_next = dl;
	  dl->dbeline_base = dbeLine;
	  return dl;
	}
    }
}

Vector<Function *> *
SourceFile::get_functions ()
{
  if (!read_stabs)
    {
      // Create all DbeLines for this Source
      read_stabs = true;
      Vector<LoadObject *> *lobjs = dbeSession->get_LoadObjects ();
      for (long i = 0, sz = VecSize (lobjs); i < sz; i++)
	{
	  LoadObject *lo = lobjs->get (i);
	  for (long i1 = 0, sz1 = VecSize (lo->seg_modules); i1 < sz1; i1++)
	    {
	      Module *mod = lo->seg_modules->get (i1);
	      mod->read_stabs ();
	    }
	}
    }
  return functions->keySet ();
}
