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

/*
 * The DbeApplication class is the base class for all C++ executables
 *  that read Experiments
 *
 *  It is derived from the Application class, and differs from it
 *  only in that it instantiates a DbeSession (q.v.) to manage
 *  the reading and processing of Experiments
 */

#ifndef _DBEAPPLICATION_H
#define _DBEAPPLICATION_H

#include "Application.h"

template <class ITEM> class Vector;

class DbeApplication : public Application
{
public:
  DbeApplication (int argc, char *argv[], char *_run_dir = NULL);
  ~DbeApplication ();
  Vector<char*> *initApplication (char *fdhome, char *licpath, ProgressFunc func);

  bool rdtMode;
  bool ipcMode;
};

extern DbeApplication *theDbeApplication;

#endif  /* _DBEAPPLICATION_H */
