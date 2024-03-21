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
#include <unistd.h>

#include "util.h"
#include "DbeSession.h"
#include "LoadObject.h"
#include "ArchiveExp.h"
#include "DbeFile.h"
#include "CallStack.h"
#include "gp-archive.h"
#include "Function.h"
#include "Module.h"

ArchiveExp::ArchiveExp (char *path) : Experiment ()
{
  force_flag = false;
  copyso_flag = false;
  use_fndr_archives = true;
  status = find_expdir (path);
  if (status == SUCCESS)
    read_log_file ();
}

ArchiveExp::~ArchiveExp () { }

void
ArchiveExp::read_data (int s_option)
{
  read_archives ();
  read_map_file ();
  if (read_java_classes_file () == SUCCESS)
    {
      for (int i = 0, sz = loadObjs ? loadObjs->size () : 0; i < sz; i++)
	{
	  LoadObject *lo = loadObjs->get (i);
	  Dprintf (DEBUG_ARCHIVE, NTXT ("%s:%d loadObjs[%d]=%-25s %s\n"),
		   get_basename (__FILE__), (int) __LINE__, i,
		   STR (lo->get_name ()), STR (lo->get_pathname ()));
	  if ((lo->dbeFile->filetype & DbeFile::F_JAVACLASS) == 0)
	    continue;
	  lo->isUsed = true;
	  if ((s_option & ARCH_EXE_ONLY) != 0)
	    continue;
	  lo->sync_read_stabs ();
	}
    }
  if ((s_option & (ARCH_USED_EXE_ONLY | ARCH_USED_SRC_ONLY)) != 0)
    {
      read_frameinfo_file ();
      resolveFrameInfo = true;
      Vector<DataDescriptor*> *ddscr = getDataDescriptors ();
      delete ddscr; // getDataDescriptors() forces reading of experiment data
      CallStack *callStack = callTree ();
      if (callStack)
	{
	  if (DEBUG_ARCHIVE)
	    {
	      Dprintf (DEBUG_ARCHIVE, NTXT ("stacks=%p\n"), callStack);
	      callStack->print (NULL);
	    }
	  for (int n = 0;; n++)
	    {
	      CallStackNode *node = callStack->get_node (n);
	      if (node == NULL)
		break;
	      do
		{
		  Histable *h = node->get_instr ();
		  Histable::Type t = h->get_type ();
		  if (t == Histable::INSTR)
		    {
		      DbeInstr *dbeInstr = (DbeInstr *) h;
		      if (!dbeInstr->isUsed)
			{
			  Function *func = (Function *) dbeInstr->convertto (Histable::FUNCTION);
			  if (!func->isUsed)
			    {
			      func->isUsed = true;
			      func->module->isUsed = true;
			      func->module->loadobject->isUsed = true;
			    }
			  DbeLine *dbeLine = (DbeLine *) dbeInstr->convertto (Histable::LINE);
			  if (dbeLine)
			    dbeLine->sourceFile->isUsed = true;
			}
		    }
		  else if (t == Histable::LINE)
		    {
		      DbeLine * dbeLine = (DbeLine *) h;
		      dbeLine->sourceFile->isUsed = true;
		    }
		  node = node->ancestor;
		}
	      while (node);
	    }
	}
    }
}

char *
ArchiveExp::createLinkToFndrArchive (LoadObject *lo, int /* hide_msg */)
{
  // For example, archives of libc.so will be:
  //  <exp>/archives/<libc.so_check_sum>
  //  <exp>/M_r0.er/archives/libc.so_<hash> -> ../../archives/<libc.so_check_sum>
  if (!create_dir (get_fndr_arch_name ()))
    {
      fprintf (stderr, GTXT ("Unable to create directory `%s'\n"), get_fndr_arch_name ());
      return NULL;
    }
  uint32_t checksum = lo->get_checksum ();
  char *linkName = dbe_sprintf (NTXT ("../../%s/%u"), SP_ARCHIVES_DIR, checksum);
  char *nm = lo->get_pathname ();
  char *symLinkName = getNameInArchive (nm, false);
  if (symlink (linkName, symLinkName) != 0)
    {
      fprintf (stderr, GTXT ("Unable to create link `%s' -> `%s'\n"),
	       symLinkName, linkName);
      free (linkName);
      free (symLinkName);
      return NULL;
    }
  free (linkName);
  free (symLinkName);

  // Return a full path inside founder archive:
  return dbe_sprintf (NTXT ("%s/%u"), get_fndr_arch_name (), checksum);
}
