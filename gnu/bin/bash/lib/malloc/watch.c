/* watch.c - watchpoint functions for malloc */

/* Copyright (C) 2001-2003 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include "imalloc.h"

#ifdef MALLOC_WATCH
#include "watch.h"

#define WATCH_MAX	32

int		_malloc_nwatch;
static PTR_T	_malloc_watch_list[WATCH_MAX];

static void
watch_warn (addr, file, line, type, data)
     PTR_T addr;
     const char *file;
     int line, type;
     unsigned long data;
{
  char *tag;

  if (type == W_ALLOC)
    tag = "allocated";
  else if (type == W_FREE)
    tag = "freed";
  else if (type == W_REALLOC)
    tag = "requesting resize";
  else if (type == W_RESIZED)
    tag = "just resized";
  else
    tag = "bug: unknown operation";

  fprintf (stderr, "malloc: watch alert: %p %s ", addr, tag);
  if (data != (unsigned long)-1)
    fprintf (stderr, "(size %lu) ", data);
  fprintf (stderr, "from '%s:%d'\n", file ? file : "unknown", line);
}

void
_malloc_ckwatch (addr, file, line, type, data)
     PTR_T addr;
     const char *file;
     int line, type;
     unsigned long data;
{
  register int i;

  for (i = _malloc_nwatch - 1; i >= 0; i--)
    {
      if (_malloc_watch_list[i] == addr)
	{
	  watch_warn (addr, file, line, type, data);
	  return;
	}
    }
}
#endif /* MALLOC_WATCH */

PTR_T
malloc_watch (addr)
     PTR_T addr;
{
  register int i;
  PTR_T ret;

  if (addr == 0)
    return addr;
  ret = (PTR_T)0;

#ifdef MALLOC_WATCH
  for (i = _malloc_nwatch - 1; i >= 0; i--)
    {
      if (_malloc_watch_list[i] == addr)
        break;
    }
  if (i < 0)
    {
      if (_malloc_nwatch == WATCH_MAX)	/* full, take out first */
	{
	  ret = _malloc_watch_list[0];
	  _malloc_nwatch--;
	  for (i = 0; i < _malloc_nwatch; i++)
	    _malloc_watch_list[i] = _malloc_watch_list[i+1];
	}
      _malloc_watch_list[_malloc_nwatch++] = addr;
    }
#endif

  return ret;  
}

/* Remove a watchpoint set on ADDR.  If ADDR is NULL, remove all
   watchpoints.  Returns ADDR if everything went OK, NULL if ADDR was
   not being watched. */
PTR_T
malloc_unwatch (addr)
     PTR_T addr;
{
#ifdef MALLOC_WATCH
  register int i;

  if (addr == 0)
    {
      for (i = 0; i < _malloc_nwatch; i++)
        _malloc_watch_list[i] = (PTR_T)0;
      _malloc_nwatch = 0;
      return ((PTR_T)0);
    }
  else
    {
      for (i = 0; i < _malloc_nwatch; i++)
	{
	  if (_malloc_watch_list[i] == addr)
	    break;
	}
      if (i == _malloc_nwatch)
        return ((PTR_T)0);		/* not found */
      /* shuffle everything from i+1 to end down 1 */
      _malloc_nwatch--;
      for ( ; i < _malloc_nwatch; i++)
        _malloc_watch_list[i] = _malloc_watch_list[i+1];
      return addr;
    }
#else
  return ((PTR_T)0);
#endif
}
