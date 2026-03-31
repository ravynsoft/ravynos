/* table.c - bookkeeping functions for allocated memory */

/* Copyright (C) 2001-2020 Free Software Foundation, Inc.

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
#include <string.h>

#include "imalloc.h"
#include "table.h"

#ifdef SHELL
extern int running_trap;
extern int signal_is_trapped PARAMS((int));
#endif

extern int malloc_register;

#ifdef MALLOC_REGISTER

extern FILE *_imalloc_fopen PARAMS((char *, char *, char *, char *, size_t));

#define FIND_ALLOC	0x01	/* find slot for new allocation */
#define FIND_EXIST	0x02	/* find slot for existing entry for free() or search */

static int table_count = 0;
static int table_allocated = 0;
static int table_bucket_index = REG_TABLE_SIZE-1;
static mr_table_t mem_table[REG_TABLE_SIZE];
static mr_table_t mem_overflow;

#ifndef STREQ
#define STREQ(a, b) ((a)[0] == (b)[0] && strcmp(a, b) == 0)
#endif

static int location_table_index = 0;
static int location_table_count = 0;
static ma_table_t mlocation_table[REG_TABLE_SIZE];

/*
 * NOTE: taken from dmalloc (http://dmalloc.com) and modified.
 */
static unsigned int
mt_hash (key)
     const PTR_T key;
{
  unsigned int a, b, c;
  unsigned long x;

  /* set up the internal state */
  a = 0x9e3779b9;	/* the golden ratio; an arbitrary value */
  x = (unsigned long)key;		/* truncation is OK */
  b = x >> 8;
  c = x >> 3;				/* XXX - was >> 4 */

  HASH_MIX(a, b, c);
  return c;
}

#if 0
static unsigned int
which_bucket (mem)
     PTR_T mem;
{
  return (mt_hash ((unsigned char *)mem) & (REG_TABLE_SIZE-1));
}

#else
#define which_bucket(mem) (mt_hash ((unsigned char *)(mem)) & (REG_TABLE_SIZE-1));

#define next_bucket()	((table_bucket_index + 1) & (REG_TABLE_SIZE-1))
#define next_entry(mem)	((mem == mem_table + REG_TABLE_SIZE - 1) ? mem_table : ++mem)

#define prev_bucket()	(table_bucket_index == 0 ? REG_TABLE_SIZE-1 : table_bucket_index-1)
#define prev_entry(mem)	((mem == mem_table) ? mem_table + REG_TABLE_SIZE - 1 : mem - 1)
#endif

static mr_table_t *
find_entry (mem, flags)
     PTR_T mem;
     int flags;
{
  unsigned int bucket;
  register mr_table_t *tp;
  mr_table_t *endp;

  if (mem_overflow.mem == mem)
    return (&mem_overflow);

  /* If we want to insert an allocation entry just use the next slot */
  if (flags & FIND_ALLOC)
    {
      table_bucket_index = next_bucket();
      table_count++;
      tp = mem_table + table_bucket_index;
      memset(tp, 0, sizeof (mr_table_t));	/* overwrite next existing entry */
      return tp;
    }
    
  tp = endp = mem_table + table_bucket_index;

  /* search for last allocation corresponding to MEM, return entry pointer */
  while (1)
    {
      if (tp->mem == mem)
	return (tp);

      tp = prev_entry (tp);

      /* if we went all the way around and didn't find it, return NULL */
      if (tp == endp)
        return ((mr_table_t *)NULL);
    }

  return (mr_table_t *)NULL;
}

mr_table_t *
mr_table_entry (mem)
     PTR_T mem;
{
  return (find_entry (mem, FIND_EXIST));
}

void
mregister_describe_mem (mem, fp)
     PTR_T mem;
     FILE *fp;
{
  mr_table_t *entry;

  entry = find_entry (mem, FIND_EXIST);
  if (entry == 0)
    return;
  fprintf (fp, "malloc: %p: %s: last %s from %s:%d\n",
  		mem,
		(entry->flags & MT_ALLOC) ? "allocated" : "free",
		(entry->flags & MT_ALLOC) ? "allocated" : "freed",
		entry->file ? entry->file : "unknown",
		entry->line);
}

void
mregister_alloc (tag, mem, size, file, line)
     const char *tag;
     PTR_T mem;
     size_t size;
     const char *file;
     int line;
{
  mr_table_t *tentry;
  sigset_t set, oset;
  int blocked_sigs;

  /* Block all signals in case we are executed from a signal handler. */
  blocked_sigs = 0;
#ifdef SHELL
  if (running_trap || signal_is_trapped (SIGINT) || signal_is_trapped (SIGCHLD))
#endif
    {
      _malloc_block_signals (&set, &oset);
      blocked_sigs = 1;
    }

  mlocation_register_alloc (file, line);

  tentry = find_entry (mem, FIND_ALLOC);

  if (tentry == 0)
    {
      /* oops.  table is full.  punt. */
      fprintf (stderr, _("register_alloc: alloc table is full with FIND_ALLOC?\n"));
      if (blocked_sigs)
	_malloc_unblock_signals (&set, &oset);
      return;
    }
  
  if (tentry->flags & MT_ALLOC)
    {
      /* oops.  bad bookkeeping. ignore for now */
      fprintf (stderr, _("register_alloc: %p already in table as allocated?\n"), mem);
    }

  tentry->mem = mem;
  tentry->size = size;
  tentry->func = tag;
  tentry->flags = MT_ALLOC;
  tentry->file = file;
  tentry->line = line;
  tentry->nalloc++;

  if (tentry != &mem_overflow)
    table_allocated++;

  if (blocked_sigs)
    _malloc_unblock_signals (&set, &oset);
}

void
mregister_free (mem, size, file, line)
     PTR_T mem;
     int size;
     const char *file;
     int line;
{
  mr_table_t *tentry;
  sigset_t set, oset;
  int blocked_sigs;

  /* Block all signals in case we are executed from a signal handler. */
  blocked_sigs = 0;
#ifdef SHELL
  if (running_trap || signal_is_trapped (SIGINT) || signal_is_trapped (SIGCHLD))
#endif
    {
      _malloc_block_signals (&set, &oset);
      blocked_sigs = 1;
    }

  tentry = find_entry (mem, FIND_EXIST);
  if (tentry == 0)
    {
      /* oops.  not found. */
#if 0
      fprintf (stderr, "register_free: %p not in allocation table?\n", mem);
#endif
      if (blocked_sigs)
	_malloc_unblock_signals (&set, &oset);
      return;
    }
  if (tentry->flags & MT_FREE)
    {
      /* oops.  bad bookkeeping. ignore for now */
      fprintf (stderr, _("register_free: %p already in table as free?\n"), mem);
    }
    	
  tentry->flags = MT_FREE;
  tentry->func = "free";
  tentry->file = file;
  tentry->line = line;
  tentry->nfree++;

  if (tentry != &mem_overflow)
    table_allocated--;

  if (blocked_sigs)
    _malloc_unblock_signals (&set, &oset);
}

/* If we ever add more flags, this will require changes. */
static char *
_entry_flags(x)
     int x;
{
  if (x & MT_FREE)
    return "free";
  else if (x & MT_ALLOC)
    return "allocated";
  else
    return "undetermined?";
}

static void
_register_dump_table(fp)
     FILE *fp;
{
  register int i;
  mr_table_t entry;

  for (i = 0; i < REG_TABLE_SIZE; i++)
    {
      entry = mem_table[i];
      if (entry.mem)
	fprintf (fp, "%s[%d] %p:%zu:%s:%s:%s:%d:%d:%d\n",
						(i == table_bucket_index) ? "*" : "",
						i,
						entry.mem, entry.size,
						_entry_flags(entry.flags),
						entry.func ? entry.func : "unknown",
						entry.file ? entry.file : "unknown",
						entry.line,
						entry.nalloc, entry.nfree);
    }
}
 
void
mregister_dump_table()
{
  _register_dump_table (stderr);
}

void
mregister_table_init ()
{
  memset (mem_table, 0, sizeof(mr_table_t) * REG_TABLE_SIZE);
  memset (&mem_overflow, 0, sizeof (mr_table_t));
  table_count = 0;
}

/* Simple for now */

static ma_table_t *
find_location_entry (file, line)
     const char *file;
     int line;
{
  register ma_table_t *tp, *endp;

  endp = mlocation_table + location_table_count;
  for (tp = mlocation_table; tp <= endp; tp++)
    {
      if (tp->line == line && STREQ (file, tp->file))
        return tp;
    }
  return (ma_table_t *)NULL;
}

void
mlocation_register_alloc (file, line)
     const char *file;
     int line;
{
  ma_table_t *lentry;
  const char *nfile;

  if (file == 0)
    {
      mlocation_table[0].nalloc++;
      return;
    }

  nfile = strrchr (file, '/');
  if (nfile)
    nfile++;
  else
    nfile = file;

  lentry = find_location_entry (nfile, line);
  if (lentry == 0)
    {
      location_table_index++;
      if (location_table_index == REG_TABLE_SIZE)
        location_table_index = 1;	/* slot 0 reserved */
      lentry = mlocation_table + location_table_index;
      lentry->file = nfile;
      lentry->line = line;
      lentry->nalloc = 1;
      if (location_table_count < REG_TABLE_SIZE)
	location_table_count++;		/* clamp at REG_TABLE_SIZE for now */
    }
  else
    lentry->nalloc++;
}

static void
_location_dump_table (fp)
     FILE *fp;
{
  register ma_table_t *tp, *endp;

  endp = mlocation_table + location_table_count;
  for (tp = mlocation_table; tp < endp; tp++)
    fprintf (fp, "%s:%d\t%d\n", tp->file ? tp->file : "unknown",
				tp->line ? tp->line : 0,
				tp->nalloc);
}

void
mlocation_dump_table ()
{
  _location_dump_table (stderr);
}

#define LOCROOT "/var/tmp/maltrace/locations."

void
mlocation_write_table ()
{
  FILE *fp;
  char defname[sizeof (LOCROOT) + 64];

  fp = _imalloc_fopen ((char *)NULL, (char *)NULL, LOCROOT, defname, sizeof (defname));
  if (fp == 0)
    return;		/* XXX - no error message yet */
  _location_dump_table (fp);
  fclose (fp);
}

void
mlocation_table_init ()
{
  memset (mlocation_table, 0, sizeof (ma_table_t) * REG_TABLE_SIZE);
  mlocation_table[0].file = "";		/* reserve slot 0 for unknown locations */
  mlocation_table[0].line = 0;
  mlocation_table[0].nalloc = 0;
  location_table_count = 1;
}

#endif /* MALLOC_REGISTER */

int
malloc_set_register(n)
     int n;
{
  int old;

  old = malloc_register;
  malloc_register = n;
  return old;
}
