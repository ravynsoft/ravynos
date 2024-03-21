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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "Filter.h"
#include "util.h"
#include "i18n.h"
#include "data_pckts.h"
#include "StringBuilder.h"
#include "Experiment.h"


// ========================================================================
// Subclass: FilterNumeric
// Public Methods

FilterNumeric::FilterNumeric (Experiment *_exp, const char *_cmd,
			      const char *_name)
{
  exp = _exp;
  cmd = dbe_strdup (_cmd);
  name = dbe_strdup (_name);
  pattern = NULL;
  status = NULL;
  items = NULL;
  prop_name = NULL;
  first = (uint64_t) - 1;
  last = (uint64_t) - 1;
  nselected = 0;
  nitems = 0;
}

FilterNumeric::~FilterNumeric ()
{
  free (cmd);
  free (name);
  free (pattern);
  free (status);
  Destroy (items);
}

// sets min and max for this filter; should be called when the range is
//	known -- that comes after the first PathTree build, in the current
//	sequence of things
void
FilterNumeric::set_range (uint64_t findex, uint64_t lindex, uint64_t total)
{
  if (first == findex && last == lindex)
    return;
  first = findex;
  last = lindex;
  nitems = total;
  nselected = nitems;
  if (pattern)
    {
      free (pattern);
      pattern = NULL;
    }
  if (status)
    {
      free (status);
      status = NULL;
    }
}

void
FilterNumeric::update_range ()
{
  if (exp == NULL)
    return;
  if (streq (cmd, NTXT ("sample")))
    set_range (1, (uint64_t) exp->nsamples (), exp->nsamples ());
  else if (streq (cmd, NTXT ("thread")))
    set_range (exp->min_thread, exp->max_thread, exp->thread_cnt);
  else if (streq (cmd, NTXT ("LWP")))
    set_range (exp->min_lwp, exp->max_lwp, exp->lwp_cnt);
  else if (streq (cmd, NTXT ("cpu")))
    {
      if (exp->min_cpu != (uint64_t) - 1)
	set_range (exp->min_cpu, exp->max_cpu, exp->cpu_cnt);
    }
}

// get_advanced_filter -- returns a string matching the current setting
char *
FilterNumeric::get_advanced_filter ()
{
  if (items == NULL)
    return NULL;
  if (items->size () == 0)
    return dbe_strdup (NTXT ("0"));

  StringBuilder sb;
  if (items->size () > 1)
    sb.append ('(');
  for (int i = 0; i < items->size (); i++)
    {
      RangePair *rp = items->fetch (i);
      if (i > 0)
	sb.append (NTXT (" || "));
      sb.append ('(');
      sb.append (prop_name);
      if (rp->first == rp->last)
	{
	  sb.append (NTXT ("=="));
	  sb.append ((long long) rp->first);
	}
      else
	{
	  sb.append (NTXT (">="));
	  sb.append ((long long) rp->first);
	  sb.append (NTXT (" && "));
	  sb.append (prop_name);
	  sb.append (NTXT ("<="));
	  sb.append ((long long) rp->last);
	}
      sb.append (')');
    }
  if (items->size () > 1)
    sb.append (')');
  return sb.toString ();
}


// get_pattern -- returns a string matching the current setting

char *
FilterNumeric::get_pattern ()
{
  update_range ();
  if (pattern)
    return pattern;
  StringBuilder sb;
  if (items == NULL)
    {
      if (last == (uint64_t) - 1 && last == first)
	// neither set; data not available
	sb.append (GTXT ("(data not recorded)"));
      else
	sb.append (GTXT ("all"));
    }
  else if (items->size () == 0)
    sb.append (GTXT ("none"));
  else
    {
      for (int i = 0; i < items->size (); i++)
	{
	  RangePair *rp = items->fetch (i);
	  if (i > 0)
	    sb.append (',');
	  sb.append ((long long) rp->first);
	  if (rp->first != rp->last)
	    {
	      sb.append ('-');
	      sb.append ((long long) rp->last);
	    }
	}
    }
  pattern = sb.toString ();
  return pattern;
}

char *
FilterNumeric::get_status ()
{
  update_range ();
  if (status == NULL)
    update_status ();
  return dbe_strdup (status);
}

// set_pattern -- set the filter to a new pattern
//	set error true/false if there was or was not an error parsing string
//	Returns true/false if the filter changed, implying a rebuild of data
bool
FilterNumeric::set_pattern (char *str, bool *error)
{
  update_range ();
  // save the old filter
  Vector<RangePair *> *olditems = items;
  *error = false;
  if (strcmp (str, NTXT ("all")) == 0)
    // if all, leave items NULL
    items = NULL;
  else if (strcmp (str, NTXT ("none")) == 0)
    // if none, leave items as a zero-length vector
    items = new Vector<RangePair *>(0);
  else
    {
      uint64_t val, val2;
      char *s = str;
      char *nexts = s;
      items = NULL;
      for (bool done = false; done == false;)
	{
	  // tokenize the string
	  // Does it start with a "-" ?
	  if (*nexts == '-')
	    val = first; // yes, set val to first, and see what follows
	  else
	    {
	      // it must start with a number
	      val = get_next_number (s, &nexts, error);
	      if (*error == true)
		break;
	    }

	  // look at the next character
	  switch (*nexts)
	    {
	    case ',':
	      s = ++nexts;
	      *error = include_range (val, val);
	      if (*error == true)
		done = true;
	      break;
	    case '-':
	      s = ++nexts;
	      if (*nexts == ',' || *nexts == '\0')
		val2 = last;
	      else
		{
		  val2 = get_next_number (s, &nexts, error);
		  if (*error == true)
		    {
		      done = true;
		      break;
		    }
		}
	      if (val > val2)
		{
		  *error = true;
		  done = true;
		  break;
		}
	      *error = include_range (val, val2);
	      if (*error == true)
		{
		  done = true;
		  break;
		}
	      if (*nexts == ',')
		{
		  s = ++nexts;
		  break;
		}
	      if (*nexts == '\0')
		{
		  done = true;
		  break;
		}
	      break;
	    case '\0':
	      *error = include_range (val, val);
	      done = true;
	      break;
	    default:
	      *error = true;
	      done = true;
	      break;
	    }
	}
      // if there was a parser error leave old setting
      if (*error == true)
	{
	  if (items)
	    {
	      items->destroy ();
	      delete items;
	    }
	  items = olditems;
	  return false;
	}
    }

  if (first != (uint64_t) - 1 && last != (uint64_t) - 1)
    {
      for (long i = VecSize (items) - 1; i >= 0; i--)
	{
	  RangePair *rp = items->get (i);
	  if ((rp->first > last) || (rp->last < first))
	    {
	      delete rp;
	      items->remove (i);
	      continue;
	    }
	  if (rp->first < first)
	    rp->first = first;
	  if (rp->last > last)
	    rp->last = last;
	}
      if (VecSize (items) == 1)
	{
	  RangePair *rp = items->get (0);
	  if ((rp->first == first) && (rp->last == last))
	    {
	      // All, leave items NULL
	      items->destroy ();
	      delete items;
	      items = NULL;
	    }
	}
    }

  // no error, delete the old setting
  if (olditems != NULL)
    {
      olditems->destroy ();
      delete olditems;
    }

  bool changed;
  // regenerate the pattern
  if (pattern == NULL)
    changed = true;
  else
    {
      char *oldpattern = pattern;
      pattern = NULL; // to force a recompute with new values
      (void) get_pattern ();
      changed = strcmp (pattern, oldpattern) != 0;
      free (oldpattern);
    }
  return changed;
}

//================================================================
// Protected methods

// set_status -- regenerate the status line, describing the current setting
void
FilterNumeric::update_status ()
{
  // regenerate the status line
  free (status);
  nselected = 0;
  if (items == NULL)
    {
      if (last == (uint64_t) - 1 && last == first)
	// neither set; data not available
	status = dbe_sprintf (GTXT ("(data not recorded)"));
      else if (first == (uint64_t) - 1 || last == (uint64_t) - 1)
	// range was not set
	status = dbe_sprintf (GTXT ("(all)"));
      else
	// range was set, compute percentage
	status = dbe_sprintf (GTXT ("total %lld, range: %lld-%lld"),
			      (long long) nitems, (long long) first,
			      (long long) last);
    }
  else
    {
      // some are selected
      int index;
      RangePair *rp;
      Vec_loop (RangePair *, items, index, rp)
      {
	nselected += rp->last - rp->first + 1;
      }
      if (last == (uint64_t) - 1)
	// range was not set
	status = dbe_sprintf (GTXT ("(%lld items selected)"),
			      (long long) nselected);
      else
	// range was set
	status = dbe_sprintf (GTXT ("total %lld, range: %lld-%lld"),
			      (long long) nitems, (long long) first,
			      (long long) last);
    }
}

// Add a range to the filter; called from set_pattern for each index,
//	or index pair
bool
FilterNumeric::include_range (uint64_t findex, uint64_t lindex)
{
  int index;
  RangePair *rp;
  if (findex > lindex)
    return true;

  bool done = false;
  if (items == NULL)
    items = new Vector<RangePair *>(0);

  Vec_loop (RangePair *, items, index, rp)
  {
    if (findex < rp->first)
      {
	// Case where the new pair starts before the old
	if (lindex + 1 < rp->first)
	  {
	    // this pair comes cleanly in front of the current item
	    RangePair *rp2 = new RangePair ();
	    rp2->first = findex;
	    rp2->last = lindex;
	    items->insert (index, rp2);
	    done = true;
	    break;
	  }
	// This new one extends the previous from the front
	rp->first = findex;
chkextend:
	if (lindex <= rp->last)
	  {
	    // but does not extend the back
	    done = true;
	    break;
	  }
	// extend this one out
	rp->last = lindex;

	// does it go into the next range?
	if (index == items->size () - 1)
	  {
	    // this is the last range, so it does not
	    done = true;
	    break;
	  }
	RangePair *next = items->fetch (index + 1);
	if (lindex + 1 < next->first)
	  {
	    // no extension, we're done
	    done = true;
	    break;
	  }
	// it does extend the next one
	next->first = rp->first;
	rp = next;
	// remove the current one, promoting next
	items->remove (index);
	goto chkextend;
      }
    else if (findex > rp->last + 1)
      // the new one is completely beyond the current
      continue;
    else
      {
	// the new one may start at or after the current, but it
	// extends it out;  set the current
	// this pair overlaps the current item
	// rp-> first is OK -- it's equal or less than findex
	goto chkextend;
      }
  }

  if (done != true)
    {
      // fall through -- append to list
      rp = new RangePair ();
      rp->first = findex;
      rp->last = lindex;
      items->append (rp);
    }

  return false;
}

// Scan the filter to see if the number given is filtered in or out
//	return true if number is in, false if it's out
bool
FilterNumeric::is_selected (uint64_t number)
{
  int index;
  RangePair *rp;
  if (items == NULL)
    return true;
  if (items->size () == 0)
    return false;

  Vec_loop (RangePair *, items, index, rp)
  {
    if (number >= rp->first && number <= rp->last)
      return true;
  }
  return false;
}

// get_next_number
//	Called from parser to extract a number from the current string position
//	Sets fail true if there was an error, false otherwise
//	returns the number as parsed
uint64_t
FilterNumeric::get_next_number (char *s, char **e, bool *fail)
{
  errno = 0;
  *fail = false;
  uint64_t val = strtoll (s, e, 10);
  if (errno == EINVAL)
    *fail = true;
  return (val);
}
