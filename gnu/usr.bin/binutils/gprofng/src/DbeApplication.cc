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
#include <stdlib.h>
#include <strings.h>

#include "DbeSession.h"
#include "DbeApplication.h"
#include "LoadObject.h"
#include "Experiment.h"
#include "PreviewExp.h"
#include "Function.h"
#include "Hist_data.h"
#include "Module.h"
#include "DataObject.h"
#include "Sample.h"
#include "CallStack.h"
#include "Print.h"
#include "util.h"
#include "libgen.h"
#include "i18n.h"

DbeApplication *theDbeApplication;

DbeApplication::DbeApplication (int argc, char *argv[], char* _run_dir)
: Application (argc, argv, _run_dir)
{
  theDbeApplication = this;
  ipcMode = false;
  rdtMode = false;
  if (argc > 1)
    if (strcmp (argv[1], NTXT ("-IPC")) == 0
	|| strcmp (argv[1], NTXT ("-DIPC")) == 0)
      ipcMode = true;
  lic_found = 0;
  lic_err = NULL;

  // Instantiate a session
  (void) new DbeSession (settings, ipcMode, rdtMode);
}

DbeApplication::~DbeApplication ()
{
  delete dbeSession;
  theDbeApplication = NULL;
}

Vector<char*> *
DbeApplication::initApplication (char *fdhome, char *licpath, ProgressFunc func)
{
  // set the home directory
  if (fdhome != NULL)
    set_run_dir (fdhome);

  // Set progress function
  set_progress_func (func);

  // Get license
  char *license_err = NULL;
  char *sts;
  if (licpath != NULL)
    {
      lic_found = 0;
      if (lic_found == 0)
	{
	  lic_err = dbe_strdup (DbeApplication::get_version ());
	  sts = dbe_strdup (GTXT ("OK"));
	}
      else if (lic_found == 2)
	{
	  lic_err = dbe_strdup (license_err);
	  sts = dbe_strdup (GTXT ("WARN"));
	}
      else if (lic_found == 3)
	{
	  lic_err = dbe_strdup (license_err);
	  sts = dbe_strdup (GTXT ("FATAL"));
	}
      else
	{
	  lic_err = dbe_strdup (license_err);
	  sts = dbe_strdup (GTXT ("ERROR"));
	}
    }
  else
    {
      lic_err = dbe_strdup (DbeApplication::get_version ());
      sts = dbe_strdup (GTXT ("OK"));
    }
  Vector<char*> *data = new Vector<char*>(2);
  data->store (0, sts);
  data->store (1, lic_err);
  return data;
}
