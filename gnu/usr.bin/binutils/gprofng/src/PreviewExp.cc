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

#include "PreviewExp.h"
#include "Data_window.h"
#include "DbeSession.h"
#include "Emsg.h"
#include "Print.h"
#include "i18n.h"

PreviewExp::PreviewExp (): Experiment () { }

PreviewExp::~PreviewExp () { }//~PreviewExp

Experiment::Exp_status
PreviewExp::experiment_open (char *path)
{
  // Find experiment directory
  if ((status = find_expdir (path)) != SUCCESS)
    {
      size_t len = strlen (path);
      is_group = ((len > 4) && !strcmp (&path[len - 4], NTXT (".erg")));
      return status;
    }
  else
    is_group = 0;

  read_log_file ();
  if (status == FAILURE)
    return status;

  if (status == INCOMPLETE && resume_ts != MAX_TIME)
    // experiment is incomplete and "resumed" (non-paused)
    // PreviewExp does not process all the packets, therefore...
    //    ... last_event does not reflect reality
    //    ... we don't know the duration or the end.
    last_event = ZERO_TIME; // mark last_event as uninitialized

  read_notes_file ();
  return status;
}

Vector<char*> *
PreviewExp::preview_info ()
{
  Vector<char*> *info = new Vector<char*>;
  if (is_group)
    info->append (GTXT ("Experiment Group"));
  else
    info->append (GTXT ("Experiment"));
  info->append (expt_name);

  if (status == FAILURE /* != SUCCESS */)
    {
      if (is_group)
	{
	  Vector<char*> *grp_list = dbeSession->get_group_or_expt (expt_name);
	  for (int i = 0, grp_sz = grp_list->size (); i < grp_sz; i++)
	    {
	      char *nm = grp_list->fetch (i);
	      char *str = dbe_sprintf (GTXT ("Exp.#%d"), i + 1);
	      info->append (str);
	      info->append (nm);
	    }
	  delete grp_list;
	}
      else
	{
	  info->append (GTXT ("Error message"));
	  info->append (mqueue_str (errorq, GTXT ("No errors\n")));
	}
      return info;
    }
  info->append (GTXT ("Experiment header"));
  info->append (mqueue_str (commentq, GTXT ("Empty header\n")));
  info->append (GTXT ("Error message"));
  info->append (mqueue_str (errorq, GTXT ("No errors\n")));
  info->append (GTXT ("Warning message"));
  info->append (mqueue_str (warnq, GTXT ("No warnings\n")));
  info->append (GTXT ("Notes"));
  info->append (mqueue_str (notesq, GTXT ("\n")));
  return info;
}

char *
PreviewExp::mqueue_str (Emsgqueue *msgqueue, char *null_str)
{
  char *mesgs = pr_mesgs (msgqueue->fetch (), null_str, "");
  char *last = mesgs + strlen (mesgs) - 1;
  if (*last == '\n')
    *last = '\0';
  return mesgs;
}
