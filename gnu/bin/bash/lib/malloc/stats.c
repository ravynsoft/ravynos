/* stats.c - malloc statistics */

/*  Copyright (C) 2001-2020 Free Software Foundation, Inc.

    This file is part of GNU Bash, the Bourne-Again SHell.

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

#include "imalloc.h"

#ifdef MALLOC_STATS

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <string.h>

#include "mstats.h"

extern int malloc_free_blocks PARAMS((int));

extern int malloc_mmap_threshold;

extern struct _malstats _mstats;

extern FILE *_imalloc_fopen PARAMS((char *, char *, char *, char *, size_t));

struct bucket_stats
malloc_bucket_stats (size)
     int size;
{
  struct bucket_stats v;

  v.nfree = 0;

  if (size < 0 || size >= NBUCKETS)
    {
      v.blocksize = 0;
      v.nused = v.nmal = v.nmorecore = v.nlesscore = v.nsplit = 0;
      return v;
    }

  v.blocksize = 1 << (size + 3);
  v.nused = _mstats.nmalloc[size];
  v.nmal = _mstats.tmalloc[size];
  v.nmorecore = _mstats.nmorecore[size];
  v.nlesscore = _mstats.nlesscore[size];
  v.nsplit = _mstats.nsplit[size];
  v.ncoalesce = _mstats.ncoalesce[size];

  v.nfree = malloc_free_blocks (size);	/* call back to malloc.c */

  return v;
}

/* Return a copy of _MSTATS, with two additional fields filled in:
   BYTESFREE is the total number of bytes on free lists.  BYTESUSED
   is the total number of bytes in use.  These two fields are fairly
   expensive to compute, so we do it only when asked to. */
struct _malstats
malloc_stats ()
{
  struct _malstats result;
  struct bucket_stats v;
  register int i;

  result = _mstats;
  result.bytesused = result.bytesfree = 0;
  for (i = 0; i < NBUCKETS; i++)
    {
      v = malloc_bucket_stats (i);
      result.bytesfree += v.nfree * v.blocksize;
      result.bytesused += v.nused * v.blocksize;
    }
  return (result);
}

static void
_print_malloc_stats (s, fp)
     char *s;
     FILE *fp;
{
  register int i;
  unsigned long totused, totfree;
  struct bucket_stats v;

  fprintf (fp, "Memory allocation statistics: %s\n    size\tfree\tin use\ttotal\tmorecore lesscore split\tcoalesce\n", s ? s : "");
  for (i = totused = totfree = 0; i < NBUCKETS; i++)
    {
      v = malloc_bucket_stats (i);
      /* Show where the mmap threshold is; sizes greater than this use mmap to
	 allocate and munmap to free (munmap shows up as lesscore). */
      if (i == malloc_mmap_threshold+1)
	fprintf (fp, "--------\n");
      if (v.nmal > 0)
	fprintf (fp, "%8lu\t%4d\t%6d\t%5d%8d\t%8d %5d %8d\n", (unsigned long)v.blocksize, v.nfree, v.nused, v.nmal, v.nmorecore, v.nlesscore, v.nsplit, v.ncoalesce);
      totfree += v.nfree * v.blocksize;
      totused += v.nused * v.blocksize;
    }
  fprintf (fp, "\nTotal bytes in use: %lu, total bytes free: %lu\n",
	   totused, totfree);
  fprintf (fp, "\nTotal bytes requested by application: %lu\n", (unsigned long)_mstats.bytesreq);
  fprintf (fp, "Total mallocs: %d, total frees: %d, total reallocs: %d (%d copies)\n",
	   _mstats.nmal, _mstats.nfre, _mstats.nrealloc, _mstats.nrcopy);
  fprintf (fp, "Total sbrks: %d, total bytes via sbrk: %d\n",
  	   _mstats.nsbrk, _mstats.tsbrk);
  fprintf (fp, "Total mmaps: %d, total bytes via mmap: %d\n",
  	   _mstats.nmmap, _mstats.tmmap);
  fprintf (fp, "Total blocks split: %d, total block coalesces: %d\n",
  	   _mstats.tbsplit, _mstats.tbcoalesce);
}

void
print_malloc_stats (s)
     char *s;
{
  _print_malloc_stats (s, stderr);
}

void
fprint_malloc_stats (s, fp)
     char *s;
     FILE *fp;
{
  _print_malloc_stats (s, fp);
}

#define TRACEROOT "/var/tmp/maltrace/stats."

void
trace_malloc_stats (s, fn)
     char *s, *fn;
{
  FILE *fp;
  char defname[sizeof (TRACEROOT) + 64];
  static char mallbuf[1024];

  fp = _imalloc_fopen (s, fn, TRACEROOT, defname, sizeof (defname));
  if (fp)
    {
      setvbuf (fp, mallbuf, _IOFBF, sizeof (mallbuf));
      _print_malloc_stats (s, fp);
      fflush(fp);
      fclose(fp);
    }
}

#endif /* MALLOC_STATS */

#if defined (MALLOC_STATS) || defined (MALLOC_TRACE)
FILE *
_imalloc_fopen (s, fn, def, defbuf, defsiz)
     char *s;
     char *fn;
     char *def;
     char *defbuf;
     size_t defsiz;
{
  char fname[1024];
  long l;
  FILE *fp;

  l = (long)getpid ();
  if (fn == 0)
    {
      sprintf (defbuf, "%s%ld", def, l);
      fp = fopen(defbuf, "w");
    }
  else
    {
      char *p, *q, *r;
      char pidbuf[32];
      int sp;

      sprintf (pidbuf, "%ld", l);
      if ((strlen (pidbuf) + strlen (fn) + 2) >= sizeof (fname))
	return ((FILE *)0);
      for (sp = 0, p = fname, q = fn; *q; )
	{
	  if (sp == 0 && *q == '%' && q[1] == 'p')
	    {
	      sp = 1;
	      for (r = pidbuf; *r; )
		*p++ = *r++;
	      q += 2;
	    }
	  else
	    *p++ = *q++;
	}
      *p = '\0';
      fp = fopen (fname, "w");
    }

  return fp;
}
#endif /* MALLOC_STATS || MALLOC_TRACE */
