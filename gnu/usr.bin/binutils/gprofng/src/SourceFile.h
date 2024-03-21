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

#ifndef _SOURCEFILE_H
#define _SOURCEFILE_H

#include "Histable.h"
#include "Map.h"

template <typename Key_t, typename Value_t> class Map;

#define SOURCE_FLAG_UNKNOWN 0x01

class SourceFile : public HistableFile
{
public:

  enum OpenStatus
  {
    OS_OK,
    OS_NOTREAD,
    OS_NOSRC,
    OS_TIMESRC
  };

  SourceFile (const char *file_name);
  virtual ~SourceFile ();
  virtual void set_name (char *);
  virtual char *get_name (NameFormat = NA);

  bool readSource ();
  Vector<Function *> *get_functions ();
  DbeLine *find_dbeline (Function *func, int lineno);
  char *getLine (int lineno);

  int
  getLineCount ()
  {
    return srcLines ? srcLines->size () : 0;
  }

  ino64_t
  getInode ()
  {
    return srcInode;
  }

  time_t
  getMTime ()
  {
    return srcMTime;
  }

  void
  setMTime (time_t tm)
  {
    srcMTime = tm;
  }

  bool
  isTmp ()
  {
    return isTmpFile;
  }

  void
  setTmp (bool set)
  {
    isTmpFile = set;
  }

  Histable_type
  get_type ()
  {
    return SOURCEFILE;
  }

  DbeLine *
  find_dbeline (int lineno)
  {
    return find_dbeline (NULL, lineno);
  }

  unsigned int flags;

private:
  static int curId;         // object id
  OpenStatus status;
  ino64_t srcInode;         // Inode number of source file
  time_t srcMTime;          // Creating time for source
  Vector<char *> *srcLines; // array of pointers to lines in source
  bool isTmpFile;           // Temporary src file to be deleted

  Vector<DbeLine*> *lines;
  Map<int, DbeLine*> *dbeLines;
  Map<Function *, Function *> *functions;
  bool read_stabs;
};

#endif
