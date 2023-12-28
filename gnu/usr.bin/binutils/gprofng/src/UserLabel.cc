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
#include <time.h>

#include "DbeSession.h"
#include "Expression.h"
#include "StringBuilder.h"
#include "util.h"
#include "UserLabel.h"
#include "debug.h"

int UserLabel::last_id = 0;

UserLabel::UserLabel (char *_name)
{
  name = dbe_strdup (_name);
  comment = str_expr = all_times = hostname = NULL;
  start_f = stop_f = false;
  expr = NULL;
  start_tv.tv_sec = 0;
  start_tv.tv_usec = 0;
  atime = timeStart = timeStop = start_sec = start_hrtime = 0;
  relative = REL_TIME;
  id = ++last_id;
}

UserLabel::~UserLabel ()
{
  free (name);
  free (comment);
  free (all_times);
  free (hostname);
  free (str_expr);
  delete expr;
}

void
UserLabel::gen_expr ()
{
  if (!start_f && !stop_f)
    return;
  StringBuilder sb;
  sb.append ('(');
  if (str_expr)
    {
      sb.append (str_expr);
      sb.append (NTXT (" || ("));
    }
  if (start_f)
    {
      sb.append (NTXT ("TSTAMP"));
      sb.append (NTXT (">="));
      sb.append (timeStart);
      if (stop_f)
	{
	  sb.append (NTXT (" && "));
	}
    }
  if (stop_f)
    {
      sb.append (NTXT ("TSTAMP"));
      sb.append ('<');
      sb.append (timeStop);
    }
  sb.append (')');
  if (str_expr)
    {
      sb.append (')');
      delete str_expr;
    }
  str_expr = sb.toString ();
  start_f = stop_f = false;
}

void
UserLabel::register_user_label (int groupId)
{
  gen_expr ();
  if (str_expr)
    {
      char *old_str = str_expr;
      str_expr = dbe_sprintf (NTXT ("(EXPGRID==%d && %s)"), groupId, old_str);
      delete old_str;
      UserLabel *ulbl = dbeSession->findUserLabel (name);
      if (ulbl)
	{
	  old_str = ulbl->str_expr;
	  ulbl->str_expr = dbe_sprintf (NTXT ("(%s || %s)"), old_str, str_expr);
	  delete old_str;
	  if (comment)
	    {
	      if (ulbl->comment)
		{
		  old_str = ulbl->comment;
		  ulbl->comment = dbe_sprintf (NTXT ("%s; %s"), old_str, comment);
		  delete old_str;
		}
	      else
		ulbl->comment = dbe_strdup (comment);
	    }
	  delete ulbl->expr;
	  ulbl->expr = dbeSession->ql_parse (ulbl->str_expr);
	}
      else
	{
	  expr = dbeSession->ql_parse (str_expr);
	  dbeSession->append (this);
	}
    }
}

char *
UserLabel::dump ()
{
  StringBuilder sb;
  sb.append (name);
  if (str_expr)
    {
      sb.append (NTXT ("  str_expr='"));
      sb.append (str_expr);
      sb.append ('\'');
    }
  if (all_times)
    {
      sb.append (NTXT (" atime="));
      sb.append ((unsigned int) (atime / NANOSEC));
      sb.append ('.');
      char buf[128];
      snprintf (buf, sizeof (buf), NTXT ("%09llu"), (unsigned long long) (atime % NANOSEC));
      sb.append (buf);
      sb.append (NTXT ("  all_times='"));
      sb.append (all_times);
      sb.append ('\'');
    }
  if (comment)
    {
      sb.append (NTXT ("  comment='"));
      sb.append (comment);
      sb.append ('\'');
    }
  return sb.toString ();
}

void
UserLabel::dump (const char *msg, Vector<UserLabel*> *labels)
{
  if (!DUMP_USER_LABELS)
    return;
  if (msg)
    fprintf (stderr, NTXT ("%s\n"), msg);
  for (int i = 0, sz = labels ? labels->size () : 0; i < sz; i++)
    {
      UserLabel *lbl = labels->fetch (i);
      char *s = lbl->dump ();
      fprintf (stderr, NTXT ("%2d %s\n"), i, s);
      delete s;
    }
}
